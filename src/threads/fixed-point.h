#ifndef FIXED-POINT-H
#define FIXED-POINT-H

#define f (1 << 14)

typedef int real;

 
#define int_to_real(n) ((real)(n * f))
#define real_truncate(n) (n / f)
#define real_round(n) (n>=0? (n+f /2)/f : (n-f /2)/f)

#define add_real_real(x,y) (x+y)
#define sub_real_real(x,y) (x-y)
#define add_real_int(x,n) (x + n*f)
#define sub_real_int(x,n) (x - n*f)
#define mul_real_real(x,y) (((int64_t) x) * y / f)
#define mul_real_int(x,n) (x*n)
#define div_real_real(x,y) (((int64_t) x) * f / y)
#define div_real_int(x,n) (x/n)

#endif
