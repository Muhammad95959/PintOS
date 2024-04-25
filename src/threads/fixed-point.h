#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#define P 17;
#define Q 14;
#define F (1 << (Q))

#define CONVERT_N_TO_FIXED_POINT(n) (((n) * (F)))
#define CONVERT_X_TO_INTEGER_ROUNDING_TOWARD_ZERO(x) ((x) / (F))
#define CONVERT_X_TO_INTEGER_ROUNDING_TO_NEAREST(x)                            \
  ((x) >= 0 ? ((x) + (F) / 2) / (F) : ((x) - (F) / 2) / (F))

#define FIXED_POINT_ADD(x, y) ((x) + (y))
#define FIXED_POINT_SUB(x, y) ((x) - (y))
#define ADD_INT_TO_FP(x, n) ((x) + (n) * (F))
#define SUB_INT_FROM_FP(x, n) ((x) - (n) * (F))
#define FIXED_POINT_MUL(x, y) (((int64_t)(x)) * (y) / (F))
#define MUL_FP_BY_INT(x,n) ((x) * (n))
#define FIXED_POINT_DIV(x,y) ((int64_t)(x) * (F) / (y))
#define DIV_FP_BY_INT(x,n) ((x) / (n))

#endif // !FIXED_POINT_H
