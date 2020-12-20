#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#define shift (1 << 14)

typedef int real;

 
#define int_to_real(n) ((real)(n * shift))
#define real_truncate(n) (n / shift)
#define real_round(n) (n>=0? ((n+ shift/2)/shift) : ((n- shift/2)/shift))

#define add_real_real(x,y) (x+y)
#define sub_real_real(x,y) (x-y)
#define add_real_int(x,n) (x + n*shift)
#define sub_real_int(x,n) (x - n*shift)
#define mul_real_real(x,y) (((int64_t) x) * y / shift)
#define mul_real_int(x,n) (x*n)
#define div_real_real(x,y) (((int64_t) x) * shift / y)
#define div_real_int(x,n) (x/n)

#endif
