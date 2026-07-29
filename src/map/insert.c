#include "insert.h"

#include <assert.h>
#include <stdlib.h>

#include "arena.h"

#include "../edit.h"
#include "../geometry.h"
#include "../map.h"
#include "logging.h"
#include "remove.h"
#include "triangulate.h"
#include "util.h"
#include "query.h"
#include "utils.h"

#define MAX_LINES_PER_SECTOR 1024
#define SAFE_LINE_HEADROOM 256
#define EXPECTED_BOUNDED_ORIENTATION CW_ORIENT

static Arena workArena = { 0 };

typedef struct SplitPoint
{
    Frac t;
    FVec2 pos;
    MapVertex *vertex;
} SplitPoint;

typedef struct LineWork
{
    MapLine *originalLine;
    line_t original;
    SplitPoint *items;
    size_t count, capacity;
    bool consumed;
} LineWork;

typedef struct WorkSet
{
    LineWork *items;
    size_t count, capacity;
} WorkSet;

typedef struct LineSectorData
{
    SectorData data;
    bool wasFront;
} LineSectorData;

typedef struct PendingLineSector
{
    LineSectorData *items;
    size_t count, capacity;
} PendingLineSector;

typedef struct TouchedLine
{
    MapLine *line;
    PendingLineSector seeds;
} TouchedLine;

typedef struct TouchedLines
{
    TouchedLine *items;
    size_t count, capacity;
} TouchedLines;

static bool includes(size_t num, void *elements[static num], void *v)
{
    for(size_t i = 0; i < num; ++i)
    {
        if(elements[i] == v)
            return true;
    }
    return false;
}

static void insert(size_t size, size_t *num, void *elements[static size], void *element)
{
    if(!includes(*num, elements, element))
    {
        assert(*num+1 < size);
        elements[(*num)++] = element;
    }
}

static bool LoopIsValidHole(polygon_t *outerPoly, size_t n, MapLine *loop[static n])
{
    for(size_t i = 0; i < n; ++i)
    {
        if(!PointInPolygon(outerPoly, loop[i]->a->pos))
            return false;
        if(!PointInPolygon(outerPoly, loop[i]->b->pos))
            return false;
    }
    return true;
}

static orientation_t LoopIsBoundedFace(size_t numLines, MapLine *lines[static numLines])
{
    FVec2 verts[MAX_LINES_PER_SECTOR];

    MapVertex *v, *nextV;
    GetLoopStart(lines[0], lines[1], &v, &nextV);
    verts[0] = v->pos;
    for(size_t i = 1; i < numLines; ++i)
    {
        MapLine *l = lines[i];
        bool front = l->a == nextV;
        v = front ? l->a : l->b;
        nextV = front ? l->b : l->a;
        verts[i] = v->pos;
    }

    return LineLoopOrientation(numLines, verts);
}

static bool TraceTargetsAreFree(size_t numLines, MapLine *lines[static numLines], bool otherSide)
{
    MapVertex *vertex, *nextVertex;
    GetLoopStart(lines[0], lines[1], &vertex, &nextVertex);

    bool front0 = lines[0]->a == vertex;
    MapSector *sector = (front0 != otherSide) ? lines[0]->frontSector : lines[0]->backSector;
    if(sector != NULL)
        return false;

    for(size_t i = 1; i < numLines; ++i)
    {
        MapLine *line = lines[i];
        bool front = line->a == nextVertex;
        MapSector *existing = (front != otherSide) ? line->frontSector : line->backSector;
        if(existing != NULL)
            return false;

        nextVertex = front ? line->b : line->a;
    }
    return true;
}

static bool LoopHasRealBoundary(size_t numLines, MapLine *lines[static numLines])
{
    for(size_t i = 0; i < numLines; ++i)
    {
        size_t count = 0;
        for(size_t j = 0; j < numLines; ++j)
            if(lines[i] == lines[j])
                count++;
        if(count == 1)
            return true;
    }
    return false;
}

static bool LoopIsOutermost(polygon_t *innerPoly, size_t n, MapLine *loop[static n], size_t numPotentialLines, MapLine *potentialLines[static numPotentialLines])
{
    for(size_t i = 0; i < numPotentialLines; ++i)
    {
        MapLine *candidate = potentialLines[i];
        if(includes(n, (void**)loop, candidate))
            continue;
        
        bool aShared = false, bShared = false;
        for(size_t j = 0; j < n; ++j)
        {
            MapVertex *va = loop[j]->a, *vb = loop[j]->b;
            if(candidate->a == va || candidate->a == vb) aShared = true;
            if(candidate->b == va || candidate->b == vb) bShared = true;
        }

        if(!aShared && !bShared)
            continue;

        bool escapes = false;
        if(!aShared && !PointInPolygon(innerPoly, candidate->a->pos)) escapes = true;
        if(!bShared && !PointInPolygon(innerPoly, candidate->b->pos)) escapes = true;

        if(escapes)
            return false;
    }
    return true;
}

MapSector* MakeMapSector(Map *map, MapLine *startLine, bool reversed, SectorData data)
{
    MapLine *sectorLines[MAX_LINES_PER_SECTOR] = { 0 };
    size_t numLines = FindOuterLineLoop(startLine, sectorLines, MAX_LINES_PER_SECTOR, reversed);
    if(numLines < 3)
        return NULL;

    orientation_t orientation = LoopIsBoundedFace(numLines, sectorLines);
    if(orientation == DEGENERATE_ORIENT || orientation != EXPECTED_BOUNDED_ORIENTATION)
        return NULL;

    if(MapSector *s = FindEquivalentSector(map, numLines, sectorLines))
        return s;

    if(!TraceTargetsAreFree(numLines, sectorLines, false))
        return NULL;

    polygon_t *poly = PolygonFromMapLines(numLines, sectorLines);

    Arena arena = { 0 };

    size_t numInnerLineLoops = 0, sizeInnerLineLoops = MAX_LINES_PER_SECTOR, usedLinesTop = 0, usedLinesSize = 4096, numPotentialLines = 0, sizePotentialLines = 1024;
    MapLine ***innerLines = arena_alloc(&arena, sizeInnerLineLoops * sizeof *innerLines);
    size_t *innerLinesNum = arena_alloc(&arena, sizeInnerLineLoops * sizeof *innerLinesNum);
    MapLine **usedLines = arena_alloc(&arena, usedLinesSize * sizeof *usedLines);
    MapLine **potentialLines = arena_alloc(&arena, sizePotentialLines * sizeof *potentialLines);
    bool *innerOtherSide = arena_alloc(&arena, sizeInnerLineLoops * sizeof *innerOtherSide);

    for(MapLine *line = map->headLine; line; line = line->next)
    {
        if(includes(numLines, (void**)sectorLines, line))
            continue;

        bool aIn = PointInPolygon(poly, line->a->pos);
        bool bIn = PointInPolygon(poly, line->b->pos);
        if(aIn && bIn)
            insert(sizePotentialLines, &numPotentialLines, (void**)potentialLines, line);
    }

    if(numPotentialLines >= 3) // need at least 3 lines to form a sector
    {
        while(numPotentialLines > 0)
        {
            numPotentialLines--;

            size_t id = numInnerLineLoops++;
            Arena_Mark mark = arena_snapshot(&arena);
            innerLines[id] = arena_alloc(&arena, MAX_LINES_PER_SECTOR * sizeof **innerLines);
            MapLine *potentialLine = potentialLines[numPotentialLines];

            if(!potentialLine)
            {
                numInnerLineLoops--;
                arena_rewind(&arena, mark);
                continue;
            }

            size_t n = FindInnerLineLoop(potentialLine, innerLines[id], MAX_LINES_PER_SECTOR, false);
            bool usedReversed = false;
            bool valid = n > 0 && LoopHasRealBoundary(n, innerLines[id]) && LoopIsValidHole(poly, n, innerLines[id]);

            if(!valid)
            {
                n = FindInnerLineLoop(potentialLine, innerLines[id], MAX_LINES_PER_SECTOR, true);
                usedReversed = true;
                valid = n > 0 && LoopHasRealBoundary(n, innerLines[id]) && LoopIsValidHole(poly, n, innerLines[id]);
            }

            if(!valid) // couldnt find a loop
            {
                numInnerLineLoops--;
                insert(usedLinesSize, &usedLinesTop, (void**)usedLines, potentialLine);
                arena_rewind(&arena, mark);
            }
            else
            {
                bool rejected = false;
                for(size_t i = 0; i < n; ++i)
                {
                    MapLine *line = innerLines[id][i];
                    if(includes(usedLinesTop, (void**)usedLines, line))
                    {
                        rejected = true;
                        break;
                    }
                }

                if(!rejected && !TraceTargetsAreFree(n, innerLines[id], !usedReversed))
                    rejected = true;

                polygon_t *innerPoly = PolygonFromMapLines(n, innerLines[id]);
                if(!rejected)
                {
                    if(!LoopIsOutermost(innerPoly, n, innerLines[id], numPotentialLines, potentialLines))
                        rejected = true;
                }

                if(rejected)
                {
                    numInnerLineLoops--;
                    insert(usedLinesSize, &usedLinesTop, (void**)usedLines, potentialLine);
                    arena_rewind(&arena, mark);
                    free(innerPoly);
                }
                else
                {
                    for(size_t i = 0; i < n; ++i)
                    {
                        MapLine *line = innerLines[id][i];
                        insert(usedLinesSize, &usedLinesTop, (void**)usedLines, line);
                    }
                    innerLinesNum[id] = n;
                    innerOtherSide[id] = LoopIsBoundedFace(n, innerLines[id]) == CW_ORIENT;

                    for(size_t k = 0; k < numPotentialLines;)
                    {
                        MapLine *candidate = potentialLines[k];
                        if(!includes(n, (void**)innerLines[id], candidate) && PointInPolygon(innerPoly, candidate->a->pos) && PointInPolygon(innerPoly, candidate->b->pos))
                        {
                            potentialLines[k] = potentialLines[--numPotentialLines];
                            continue;
                        }
                        ++k;
                    }
                    free(innerPoly);
                }
            }
        }
    }

    free(poly);
    MapSector *sector = EditAddSector(map, numLines, sectorLines, numInnerLineLoops, innerLinesNum, innerLines, innerOtherSide, data);

    arena_free(&arena);

    return sector;
}

static TouchedLine* FindOrCreateTouched(TouchedLines *touched, MapLine *line)
{
    for(size_t i = 0; i < touched->count; ++i)
        if(touched->items[i].line == line)
            return &touched->items[i];

    TouchedLine t = { .line = line };
    arena_da_append(&workArena, touched, t);
    return &touched->items[touched->count - 1];
}

static void MarkTouched(TouchedLines *touched, MapLine *line)
{
    FindOrCreateTouched(touched, line);
}

static void MarkTouchedWithSector(TouchedLines *touched, MapLine *line, SectorData data, bool wasFront)
{
    TouchedLine *t = FindOrCreateTouched(touched, line);
    LineSectorData sdata = { .data = data, .wasFront = wasFront };
    arena_da_append(&workArena, &t->seeds, sdata);
}

static SplitResult DoSplit(Map *map, TouchedLines *touched, MapLine *line, MapVertex *vertex)
{
    if(vertex == line->a || vertex == line->b)
        return (SplitResult){ .left = line, .right = line };

    SectorData frontData = DefaultSectorData();
    SectorData backData = DefaultSectorData();

    MapSector *frontSector = line->frontSector;
    MapSector *backSector = line->backSector;

    bool hasFrontSector = frontSector != NULL;
    bool hasBackSector = backSector != NULL && backSector != frontSector;
    bool hasSectorsAttached = hasFrontSector || hasBackSector;

    if(hasFrontSector)
    {
        frontData = CopySectorData(line->frontSector->data);
        MapSector *sector = line->frontSector;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = sector->outerLines[i];
            if(sline == line) continue;
            bool wasFront = sline->frontSector == sector;
            MarkTouchedWithSector(touched, sline, frontData, wasFront);
        }
        RemoveSector(map, sector);
    }
    if(hasBackSector)
    {
        backData = CopySectorData(line->backSector->data);
        MapSector *sector = line->backSector;
        for(size_t i = 0; i < sector->numOuterLines; ++i)
        {
            MapLine *sline = sector->outerLines[i];
            if(sline == line) continue;
            bool wasFront = sline->frontSector == sector;
            MarkTouchedWithSector(touched, sline, backData, wasFront);
        }
        RemoveSector(map, sector);
    }

    SplitResult result = SplitMapLine(map, line, vertex);

    for(size_t i = 0; i < touched->count; ++i)
    {
        if(touched->items[i].line == line)
        {
            touched->items[i].line = result.left;
            break;
        }
    }

    if(hasFrontSector)
    {
        MarkTouchedWithSector(touched, result.left, frontData, true);
        MarkTouchedWithSector(touched, result.right, frontData, true);
    }
    if(hasBackSector)
    {
        MarkTouchedWithSector(touched, result.left, backData, false);
        MarkTouchedWithSector(touched, result.right, backData, false);
    }
    if(!hasSectorsAttached)
    {
        MarkTouched(touched, result.left);
        MarkTouched(touched, result.right);
    }

    return result;
}

static LineWork* GetOrCreateWork(WorkSet *ws, MapLine *originalLine, line_t original)
{
    for(size_t i = 0; i < ws->count; ++i)
        if(ws->items[i].originalLine == originalLine)
            return &ws->items[i];

    LineWork w = { .originalLine = originalLine, .original = original };
    arena_da_append(&workArena, ws, w);
    return &ws->items[ws->count - 1];
}

static void AddSplitPoint(LineWork *w, Frac t, FVec2 pos)
{
    for(size_t i = 0; i < w->count; ++i)
        if(t.num == w->items[i].t.num && t.den == w->items[i].t.den)
            return;

    SplitPoint sp = { .t = t, .pos = pos };
    arena_da_append(&workArena, w, sp);
}

static void ReverseVertexOrder(size_t numVerts, FVec2 vertices[static numVerts])
{
    size_t half = numVerts / 2;
    for(size_t i = 0; i < half; ++i)
    {
        FVec2 tmp = vertices[i];
        vertices[i] = vertices[numVerts - i - 1];
        vertices[numVerts - i - 1] = tmp;
    }
}

static void SnapInputVertices(Map *map, size_t numVerts, FVec2 vertices[static numVerts])
{
    for(size_t i = 0; i < numVerts; ++i)
    {
        MapVertex *existing = EditGetClosestVertex(map, vertices[i], STITCHING_DIST);
        if(existing) vertices[i] = existing->pos;
    }
}

static bool TrySnapEndpointToLine(WorkSet *ws, LineWork *nw, bool useA, MapLine *mapLine, line_t mline)
{
    FVec2 endpoint = useA ? nw->original.a : nw->original.b;
    real_t dist = MinDistToLine(mline.a, mline.b, endpoint);
    if(fixed_from_real(dist) > STITCHING_DIST) 
        return false;

    FVec2 dir = fvec2_sub(mline.b, mline.a);
    Frac t = frac_make(fvec2_dotw(fvec2_sub(endpoint, mline.a), dir), fvec2_dotw(dir, dir));
    if(frac_lt_zero(t) || frac_gt_one(t))
        return false;

    FVec2 exactPoint = Materialize(mline.a, dir, t);

    if(frac_is_zero(t) || frac_is_one(t) || fvec2_eq(exactPoint, mline.a) || fvec2_eq(exactPoint, mline.b))
    {
        FVec2 target = frac_is_zero(t) ? mline.a : mline.b;
        if(useA)
            nw->original.a = target;
        else
            nw->original.b = target;
        return true;
    }

    LineWork *mw = GetOrCreateWork(ws, mapLine, mline);
    AddSplitPoint(mw, t, exactPoint);

    if(useA) 
        nw->original.a = exactPoint;
    else
        nw->original.b = exactPoint;
    return true;
}

static void SpawnLeftover(WorkSet *ws, size_t *newLineIdx, size_t *numNewLines, size_t cap, FVec2 a, FVec2 b)
{
    if(fvec2_eq(a, b))
        return;
    assert(*numNewLines < cap);

    LineWork w = { .original = { a, b } };

    arena_da_append(&workArena, ws, w);
    newLineIdx[(*numNewLines)++] = ws->count - 1;
}

static bool CollectIntersections(Map *map, WorkSet *ws, size_t numNewLines, size_t newLineIdx[static numNewLines], size_t newLineCapacity, bool isLoop)
{
    for(size_t i = 0; i < numNewLines; ++i)
    {
        LineWork *nw = &ws->items[newLineIdx[i]];

        for(MapLine *mapLine = map->headLine; mapLine; mapLine = mapLine->next)
        {
            line_t mline = { .a = mapLine->a->pos, .b = mapLine->b->pos };

            //if(!BoundsNear(nw->original, mline, STITCHING_DIST)) continue;

            intersection_res_t res;
            if(LineOverlap(mline, nw->original, &res))
            {
                LineWork *mw = GetOrCreateWork(ws, mapLine, mline);

                bool uInside = !frac_lte_zero(res.u) && !frac_gte_one(res.u);
                bool vInside = !frac_lte_zero(res.v) && !frac_gte_one(res.v);

                if(uInside)
                    AddSplitPoint(mw, res.u, res.p0);
                if(vInside)
                    AddSplitPoint(mw, res.v, res.p1);

                if(frac_lte_zero(res.u))
                    SpawnLeftover(ws, newLineIdx, &numNewLines, newLineCapacity, nw->original.a, mline.a);
                else if(frac_gte_one(res.u))
                    SpawnLeftover(ws, newLineIdx, &numNewLines, newLineCapacity, mline.b, nw->original.a);

                if(frac_gte_one(res.v))
                    SpawnLeftover(ws, newLineIdx, &numNewLines, newLineCapacity, mline.b, nw->original.b);
                else if(frac_lte_zero(res.v))
                    SpawnLeftover(ws, newLineIdx, &numNewLines, newLineCapacity, nw->original.b, mline.a);

                nw->consumed = true;
                break;
            }
            else if(LineIntersection(mline, nw->original, &res))
            {
                bool uInside = !frac_is_zero(res.u) && !frac_is_one(res.u);
                bool vInside = !frac_is_zero(res.v) && !frac_is_one(res.v);

                if(uInside)
                {
                    LineWork *mw = GetOrCreateWork(ws, mapLine, mline);
                    AddSplitPoint(mw, res.u, res.p0);
                }
                if(vInside)
                    AddSplitPoint(nw, res.v, res.p0);
            }
            else
            {
                TrySnapEndpointToLine(ws, nw, true, mapLine, mline);
                TrySnapEndpointToLine(ws, nw, false, mapLine, mline);
            }
        }

        for(size_t j = i + 1; j < numNewLines; ++j)
        {
            if(j == i + 1 || (i == 0 && j == numNewLines - 1 && isLoop))
                continue;

            LineWork *newLine = &ws->items[newLineIdx[j]];

            intersection_res_t res;
            if(LineOverlap(nw->original, newLine->original, &res))
            {
                AddSplitPoint(nw, res.u, res.p0);
                AddSplitPoint(newLine, res.v, res.p1);
            }
            else if(LineIntersection(nw->original, newLine->original, &res))
            {
                AddSplitPoint(nw, res.u, res.p0);
                AddSplitPoint(newLine, res.v, res.p0);
            }
        }
    }
    return true;
}

static void ResolveSplitPoints(Map *map, WorkSet *ws)
{
    for(size_t i = 0; i < ws->count; ++i)
        for(size_t j = 0; j < ws->items[i].count; ++j)
        {
            SplitPoint *sp = &ws->items[i].items[j];
            sp->vertex = EditAddVertex(map, sp->pos);
        }
}

static int CompareSplitPoints(const void *a, const void *b)
{
    const SplitPoint *sa = a, *sb = b;
    double ta = (double)sa->t.num / (double)sa->t.den;
    double tb = (double)sb->t.num / (double)sb->t.den;
    if(ta < tb) return -1;
    if(ta > tb) return 1;
    return (sa->vertex > sb->vertex) - (sa->vertex < sb->vertex);
}

static bool ApplyLineWork(Map *map, TouchedLines *touched, LineWork *w)
{
    if(w->consumed)
        return true;

    if(w->count == 0)
    {
        if(w->originalLine)
            return true;

        MapVertex *a = EditAddVertex(map, w->original.a);
        MapVertex *b = EditAddVertex(map, w->original.b);

        MapLine *nl = EditAddLine(map, a, b, DefaultLineData());
        if(!nl)
            return false;
        MarkTouched(touched, nl);
        return true;
    }

    // sort then dedup
    qsort(w->items, w->count, sizeof *w->items, CompareSplitPoints);
    size_t uniqueCount = 0;
    for(size_t i = 0; i < w->count; ++i)
        if(uniqueCount == 0 || w->items[uniqueCount-1].vertex != w->items[i].vertex)
            w->items[uniqueCount++] = w->items[i];

    MapVertex *startVertex = EditAddVertex(map, w->original.a);
    MapVertex *endVertex = EditAddVertex(map, w->original.b);

    if(w->originalLine)
    {
        MapLine *remaining = w->originalLine;
        for(size_t i = 0; i < uniqueCount; ++i)
        {
            SplitResult result = DoSplit(map, touched, remaining, w->items[i].vertex);
            remaining = result.right;
        }
    }
    else
    {
        MapVertex *prev = startVertex;
        for(size_t i = 0; i < uniqueCount; ++i)
        {
            if(prev != w->items[i].vertex)
            {
                MapLine *nl = EditAddLine(map, prev, w->items[i].vertex, DefaultLineData());
                if(!nl) return false;
                MarkTouched(touched, nl);
            }
            prev = w->items[i].vertex;
        }
        if(prev != endVertex)
        {
            MapLine *nl = EditAddLine(map, prev, endVertex, DefaultLineData());
            if(!nl) return false;
            MarkTouched(touched, nl);
        }
    }

    return true;
}

static void RebuildTouchedSectors(Map *map, TouchedLines *touched)
{
    for(size_t i = 0; i < touched->count; ++i)
    {
        TouchedLine *t = &touched->items[i];

        if(t->seeds.count > 0)
        {
            for(size_t s = 0; s < t->seeds.count; ++s)
            {
                LineSectorData *sdata = &t->seeds.items[s];

                MapSector *current = sdata->wasFront ? t->line->frontSector : t->line->backSector;
                if(current != NULL)
                    continue;

                MakeMapSector(map, t->line, !sdata->wasFront, sdata->data);
            }
        }
        else
        {
            if(t->line->frontSector == NULL)
                MakeMapSector(map, t->line, false, DefaultSectorData());
            if(t->line->backSector == NULL)
                MakeMapSector(map, t->line, true, DefaultSectorData());
        }
    }
}

static void DetectEnclosingSectors(Map *map, TouchedLines *touched)
{
    size_t originalCount = touched->count;
    for(size_t i = 0; i < originalCount; ++i)
    {
        MapLine *rep = touched->items[i].line;
        if(rep->frontSector || rep->backSector)
            continue;
        
        FVec2 midpoint = fvec2_scale(fvec2_add(rep->a->pos, rep->b->pos), fixed_from_real(0.5f));
        for(MapSector *s = map->headSector; s; s = s->next)
        {
            if(!PointInSector2(s, midpoint))
                continue;

            MapLine *outerStart = s->outerLines[0];
            SectorData data = CopySectorData(s->data);
            bool wasFront = outerStart->frontSector == s;
            RemoveSector(map, s);
            MarkTouchedWithSector(touched, outerStart, data, wasFront);
            break;
        }
    }
}

bool InsertLinesIntoMap(Map *map, size_t numVerts, FVec2 vertices[static numVerts], bool isLoop)
{
    size_t end = isLoop ? numVerts : numVerts - 1;

    if(isLoop && LineLoopOrientation(numVerts, vertices) == CCW_ORIENT)
        ReverseVertexOrder(numVerts, vertices);

    SnapInputVertices(map, numVerts, vertices);
    WorkSet ws = { 0 };

    size_t newLineCapacity = end + SAFE_LINE_HEADROOM;
    size_t *newLineIdx = arena_alloc(&workArena, newLineCapacity * sizeof *newLineIdx);
    size_t numNewLines = 0;
    for(size_t i = 0; i < end; ++i)
    {
        line_t seg = { vertices[i], vertices[(i+1) % numVerts] };
        if(fvec2_eq(seg.a, seg.b)) continue;
        LineWork w = { .original = seg };
        arena_da_append(&workArena, &ws, w);
        newLineIdx[numNewLines++] = ws.count - 1;
    }

    if(!CollectIntersections(map, &ws, numNewLines, newLineIdx, newLineCapacity, isLoop))
        return false;
    ResolveSplitPoints(map, &ws);

    TouchedLines touched = { 0 };
    for(size_t i = 0; i < ws.count; ++i)
        if(!ApplyLineWork(map, &touched, &ws.items[i]))
            return false;

    DetectEnclosingSectors(map, &touched);
    RebuildTouchedSectors(map, &touched);

    arena_reset(&workArena);
    return true;
}

