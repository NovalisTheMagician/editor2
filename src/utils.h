#pragma once

#include <math.h>

#define EPSILON 0.000001

#define COUNT_OF(arr) (sizeof(arr)/sizeof(0[arr]))

#define min(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); a_ < b_ ? a_ : b_; })
#define max(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); a_ > b_ ? a_ : b_; })
#define clamp(mi, ma, v) ({ typeof(mi) mi_ = (mi); typeof(ma) ma_ = (ma); typeof(v) v_ = (v); max(mi_, min(ma_, v_)); })
#define mag(v) ({ typeof(v) v_ = (v); sqrtf(v_.x*v_.x + v_.y*v_.y); })
#define mag2(v) ({ typeof(v) v_ = (v); v_.x*v_.x + v_.y*v_.y; })

#define eq(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); fabs(a_ - b_) <= EPSILON; })
#define gt(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); (a_ + EPSILON) > b_; })
#define lt(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); (a_ - EPSILON) < b_; })
#define gte(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); (a_ + EPSILON) >= b_; })
#define lte(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); (a_ - EPSILON) <= b_; })
