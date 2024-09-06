#pragma once

#define COUNT_OF(arr) (sizeof(arr)/sizeof(0[arr]))

#define min(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); a_ < b_ ? a_ : b_; })
#define max(a, b) ({ typeof(a) a_ = (a); typeof(b) b_ = (b); a_ > b_ ? a_ : b_; })
#define clamp(mi, ma, v) ({ typeof(mi) mi_ = (mi); typeof(ma) ma_ = (ma); typeof(v) v_ = (v); max(mi_, min(ma_, v_)); })
