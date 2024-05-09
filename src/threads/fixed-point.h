#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#define S 16
#define F (1 << (S))
#define Fby2 (1 << (S - 1))

#define CONVERT_N_TO_FIXED_POINT(n) ((n * F))
#define CONVERT_X_TO_INTEGER_ROUNDING_TOWARD_ZERO(x) (x / F)
#define CONVERT_X_TO_INTEGER_ROUNDING_TO_NEAREST(x)                            \
  (x >= 0 ? ((x + (1 * Fby2)) / F) : ((x - Fby2) / F))

#define FIXED_POINT_ADD(x, y) (x + y)
#define FIXED_POINT_SUB(x, y) (x - y)
#define FIXED_POINT_MUL(x, y) ((((int64_t)x) * y / F))
#define FIXED_POINT_DIV(x, y) (((((int64_t)x) * F) / y))
#define ADD_INT_TO_FP(x, y) (x + (y * F))
#define SUB_INT_FROM_FP(x, y) (x - (y * F))
#define MUL_FP_BY_INT(x, y) (x * y)
#define DIV_FP_BY_INT(x, y) (x / y)

#endif
