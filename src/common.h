#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdalign.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "glad/gl.h"
#include "cglm/struct.h"

#include "imguidef.h"

#include "utils/pstring.h"
#include "utils/hash.h"
#include "logging.h"

#define COUNT_OF(arr) (sizeof(arr)/sizeof(0[arr]))

#define min(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); a_ < b_ ? a_ : b_; })
#define max(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); a_ > b_ ? a_ : b_; })
#define clamp(mi, ma, v) ({ typeof(mi) mi_ = (mi); typeof(ma) ma_ = (ma); typeof(v) v_ = (v); max(mi_, min(ma_, v_)); })

#define SHADER_VERSION "#version 460 core\n"
#define REQ_GL_MAJOR 4
#define REQ_GL_MINOR 6

#if defined(_DEBUG)
#include "utils/debug.h"
#endif
