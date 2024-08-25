#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdalign.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "glad/gl.h"
#include <cglm/struct.h>

#include "utils/pstring.h"
#include "utils/hash.h"
#include "logging.h"

#define COUNT_OF(arr) (sizeof(arr)/sizeof(0[arr]))

#define min(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_ < b_ ? a_ : b_; })
#define max(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_ > b_ ? a_ : b_; })
#define clamp(mi, ma, v) ({ __typeof__(mi) mi_ = (mi); __typeof__(ma) ma_ = (ma); __typeof__(v) v_ = (v); max(mi_, min(ma_, v_)); })

#define SHADER_VERSION "#version 460 core\n"
#define REQ_GL_MAJOR 4
#define REQ_GL_MINOR 6

#include "utils/debug.h"
