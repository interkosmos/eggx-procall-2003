#ifndef _TINY_COMPLEX_H
#define _TINY_COMPLEX_H 1

#ifdef complex
#error "Do not use tiny_complex.h with C99 complex.h"
#endif

#define complex _Complex
#define _Complex_I (__extension__ 1.0iF)
#define I _Complex_I

#define creal(cv) (__real__ (cv))
#define cimag(cv) (__imag__ (cv))

#define cexp(cv) (exp(creal(cv))*cos(cimag(cv)) + I * exp(creal(cv))*sin(cimag(cv)))
#define ccos(cv) (cos(creal(cv))*cosh(cimag(cv)) - I * sin(creal(cv))*sinh(cimag(cv)))
#define csin(cv) (sin(creal(cv))*cosh(cimag(cv)) + I * cos(creal(cv))*sinh(cimag(cv)))
#define conj(cv) (creal(cv) - I * cimag(cv))

#define cabs(cv) (hypot(creal(cv), cimag(cv)))

#define carg(cv) (atan2(cimag(cv), creal(cv)))

#endif	/* _TINY_COMPLEX_H */
