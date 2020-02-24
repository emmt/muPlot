/*
 * muXForms.c --
 *
 * Implementation of 2-dimensional affine transforms for coordinate transforms.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#include <math.h>
#include "muPlotXForms.h"

/*--------------------------------------------------------------------------*/
/* Compose affine transforms. */

#define ENCODE(T, SFX)                                                  \
                                                                        \
    MpAffineTransform##SFX                                              \
    MpComposeAffineTransformsAlt##SFX(MpAffineTransform##SFX A,         \
                                      MpAffineTransform##SFX B)         \
    {                                                                   \
        MpAffineTransform##SFX dst;                                     \
        MP_XFORM_COMPOSE(MP_GET_FIELD, dst, A, B);                      \
        return dst;                                                     \
    }                                                                   \
                                                                        \
    MpStatus                                                            \
    MpComposeAffineTransforms##SFX(MpAffineTransform##SFX* dst,         \
                                   const MpAffineTransform##SFX* A,     \
                                   const MpAffineTransform##SFX* B)     \
    {                                                                   \
        MP_XFORM_COMPOSE(MP_GET_FIELD_PTR, dst, A, B);                  \
        return MP_OK;                                                   \
    }

ENCODE(float,  Flt)
ENCODE(double, Dbl)

#undef ENCODE

/*----------------------------------------------------------------------------*/
/* Scaling of an affine transform. */

#define ENCODE(T, SFX)                                                  \
                                                                        \
    MpStatus                                                            \
    MpLeftScaleAffineTransform##SFX(MpAffineTransform##SFX* dst,        \
                                    T alpha,                            \
                                    const MpAffineTransform##SFX* A)    \
    {                                                                   \
        MP_XFORM_LEFT_SCALE(MP_GET_FIELD_PTR, dst, alpha, A);           \
        return MP_OK;                                                   \
    }                                                                   \
                                                                        \
    MpStatus                                                            \
    MpRightScaleAffineTransform##SFX(MpAffineTransform##SFX* dst,       \
                                     const MpAffineTransform##SFX* A,   \
                                     T alpha)                           \
    {                                                                   \
        MP_XFORM_RIGHT_SCALE(MP_GET_FIELD_PTR, dst, A, alpha);          \
        return MP_OK;                                                   \
    }

ENCODE(float,Flt)
ENCODE(double,Dbl)

#undef ENCODE

/*----------------------------------------------------------------------------*/
/* Translating an affine transform. */

#define ENCODE(T, SFX)                                                    \
    MpStatus                                                              \
    MpLeftTranslateAffineTransform##SFX(MpAffineTransform##SFX* dst,      \
                                        T x, T y,                         \
                                        const MpAffineTransform##SFX* A)  \
    {                                                                     \
        MP_XFORM_LEFT_TRANSLATE(MP_GET_FIELD_PTR, dst, x, y, A);          \
        return MP_OK;                                                     \
    }                                                                     \
                                                                          \
    MpStatus                                                              \
    MpRightTranslateAffineTransform##SFX(MpAffineTransform##SFX* dst,     \
                                         const MpAffineTransform##SFX* A, \
                                         T x, T y)                        \
    {                                                                     \
        MP_XFORM_LEFT_TRANSLATE(MP_GET_FIELD_PTR, dst, x, y, A);          \
        return MP_OK;                                                     \
    }

ENCODE(float,  Flt)
ENCODE(double, Dbl)

#undef ENCODE

/*---------------------------------------------------------------------------*/
/* Rotating an affine transform. */

#define ENCODE(T, SFX, SIN, COS)                                        \
                                                                        \
    MpStatus                                                            \
    MpLeftRotateAffineTransform##SFX(MpAffineTransform##SFX* dst,       \
                                     T theta,                           \
                                     const MpAffineTransform##SFX* A)   \
    {                                                                   \
        T sn = SIN(theta);                                              \
        T cs = COS(theta);                                              \
        MP_XFORM_LEFT_ROTATE(MP_GET_FIELD_PTR, dst, sn, cs, A);         \
        return MP_OK;                                                   \
    }                                                                   \
                                                                        \
    MpStatus                                                            \
    MpRightRotateAffineTransform##SFX(MpAffineTransform##SFX* dst,      \
                                      const MpAffineTransform##SFX* A,  \
                                      T theta)                          \
    {                                                                   \
        T sn = SIN(theta);                                              \
        T cs = COS(theta);                                              \
        MP_XFORM_RIGHT_ROTATE(MP_GET_FIELD_PTR, dst, A, sn, cs);        \
        return MP_OK;                                                   \
    }

ENCODE(float,  Flt, sinf, cosf)
ENCODE(double, Dbl, sin,  cos)

#undef ENCODE

/*--------------------------------------------------------------------------*/
/* Determinant of an affine transform. */

#define ENCODE(T, SFX)                                                  \
                                                                        \
    T                                                                   \
    MpDeterminantAffineTransform##SFX(const MpAffineTransform##SFX* A)  \
    {                                                                   \
        return MP_XFORM_DETERMINANT(MP_GET_FIELD_PTR, A);               \
    }

ENCODE(float,  Flt)
ENCODE(double, Dbl)

#undef ENCODE

/*--------------------------------------------------------------------------*/
/* Inversion of an affine transform. */

#define ENCODE(T, SFX)                                                  \
                                                                        \
    MpStatus                                                            \
    MpInverseAffineTransform##SFX(MpAffineTransform##SFX* dst,          \
                                  const MpAffineTransform##SFX* A)      \
    {                                                                   \
        MP_XFORM_INVERSE(T, MP_GET_FIELD_PTR, dst, A);                  \
        return MP_OK;                                                   \
    }

ENCODE(float,  Flt)
ENCODE(double, Dbl)

#undef ENCODE

/*--------------------------------------------------------------------------*/
/* Division of affine transforms. */

#define ENCODE(T, SFX)                                                  \
                                                                        \
    MpStatus                                                            \
    MpLeftDivideAffineTransforms##SFX(MpAffineTransform##SFX* dst,      \
                                      const MpAffineTransform##SFX* A,  \
                                      const MpAffineTransform##SFX* B)  \
    {                                                                   \
        MP_XFORM_LEFT_DIVIDE(T, MP_GET_FIELD_PTR, dst, A, B);           \
        return MP_OK;                                                   \
    }                                                                   \
                                                                        \
    MpStatus                                                            \
    MpRightDivideAffineTransforms##SFX(MpAffineTransform##SFX* dst,     \
                                       const MpAffineTransform##SFX* A, \
                                       const MpAffineTransform##SFX* B) \
    {                                                                   \
        MP_XFORM_RIGHT_DIVIDE(T, MP_GET_FIELD_PTR, dst, A, B);          \
        return MP_OK;                                                   \
    }

ENCODE(float,  Flt)
ENCODE(double, Dbl)

#undef ENCODE

/*--------------------------------------------------------------------------*/
/* Intercept of an affine transform. */

#define ENCODE(T, SFX)                                                  \
                                                                        \
    MpStatus                                                            \
    MpInterceptAffineTransform##SFX(T* xptr, T* yptr,                   \
                                    const MpAffineTransform##SFX* A)    \
    {                                                                   \
        MP_XFORM_INTERCEPT(T, MP_GET_FIELD_PTR, *xptr, *yptr, A);       \
        return MP_OK;                                                   \
    }

ENCODE(float,  Flt)
ENCODE(double, Dbl)

#undef ENCODE
