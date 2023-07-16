#include "editor.h"

void ResetSettings(struct EdSettings *settings)
{
    settings->colors[BACKGROUND][0] = 0.45f;
    settings->colors[BACKGROUND][1] = 0.55f;
    settings->colors[BACKGROUND][2] = 0.60f;
    settings->colors[BACKGROUND][3] = 1.00f;
}

bool LoadSettings(const char *settingsPath, struct EdSettings *settings)
{
    return false;
}

void SaveSettings(const char *settingsPath, const struct EdSettings *settings)
{

}
