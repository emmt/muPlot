/*
 * muPlotXForms.h --
 *
 * Definitions for 2-dimensional affine transforms implementing coordinate
 * transforms.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#ifndef _MUPLOT_XFORMS_H
#define _MUPLOT_XFORMS_H 1

#include "muPlot.h"

_MP_BEGIN_DECLS

/**
 * Affine 2-dimensional coordinate transform.
 *
 * This structure is used to store the coefficients of a 2-dimensional affine
 * transform suitable for 2-dimensional coordinate transforms.  Transforming
 * coordinates `(x,y)` to `(xp,yp)` via an affine transform `A` writes:
 *
 * <pre>
 * xp = A.xx*x + A.xy*y + A.x;
 * yp = A.yx*x + A.yy*y + A.y;
 * </pre>
 *
 * This can also be accomplished with a macro:
 *
 * <pre>
 * xp = MP_XFORM_APPLY_X(E,A,x,y);
 * yp = MP_XFORM_APPLY_Y(E,A,x,y);
 * </pre>
 *
 * where `E` is a macro such that `E(A,m)` yields the member `m` of
 * `A`. Usually, `E` is `MP_GET_FIELD` or `MP_GET_FIELD_PTR`.
 *
 * The simple layout of the coefficients make possible the casting an array of
 * 6 floating-point values as a pointer to an affine transform (and
 * conversely).  For example:
 *
 * <pre>
 * float a[6] = {Axx, Axy, Ax,
 *               Ayx, Ayy, Ay};
 * MpAffineTransformFlt* A = (MpAffineTransformFlt*)a;
 * </pre>
 *
 * The applying `A` is equivalent to:
 *
 * <pre>
 * xp = a[0]*x + a[1]*y + a[2];
 * yp = a[3]*x + a[4]*y + a[5];
 * </pre>
 *
 * Structure @ref MpAffineTransformDbl is for double precision floating-point
 * coefficients.
 *
 * Many operations are available for affine coordinates transforms:
 *
 * - Call MpComposeAffineTransformsFlt(), MpComposeAffineTransformsDbl(),
 *   MpComposeAffineTransformsAltFlt() or MpComposeAffineTransformsAltDbl() to
 *   compose 2 affine transforms.
 *
 * - Call MpLeftScaleAffineTransformFlt() or MpLeftScaleAffineTransformDbl() to
 *   post-scale an affine transform by a multiplier.
 *
 * - Call MpRightScaleAffineTransformFlt() or MpRightScaleAffineTransformDbl()
 *   to pre-scale an affine transform by a multiplier.
 *
 * - Call MpLeftTranslateAffineTransformFlt() or
 *   MpLeftTranslateAffineTransformDbl() to post-translate an affine transform
 *   by a vector.
 *
 * - Call MpRightTranslateAffineTransformFlt() or
 *   MpRightTranslateAffineTransformDbl() to pre-translate an affine transform
 *   by a vector.
 *
 * - Call MpLeftRotateAffineTransformFlt() or MpLeftRotateAffineTransformDbl()
 *   to post-rotate an affine transform by an angle.
 *
 * - Call MpRightRotateAffineTransformFlt() or
 *   MpRightRotateAffineTransformDbl() to pre-rotate an affine transform by an
 *   angle.
 *
 * - Call MpDeterminantAffineTransformFlt() or
 *   MpDeterminantAffineTransformDbl() to compute the determinant of an affine
 *   transform.
 *
 * - Call MpInverseAffineTransformFlt() or MpInverseAffineTransformFlt() to
 *   compute the inverse of an affine transform.
 *
 * - Call MpLeftDivideAffineTransformsFlt() or MpLeftDivideAffineTransformsDbl()
 *   to left-divide affine transforms.
 *
 * - Call MpRightDivideAffineTransformsFlt() or
 *   MpRightDivideAffineTransformsDbl() to right-divide affine transforms.
 *
 * - Call MpInterceptAffineTransformFlt() or MpInterceptAffineTransformFlt() to
 *   compute the intercept of an affine transform.
 */
typedef struct _MpAffineTransformFlt {
    float xx, xy, x;
    float yx, yy, y;
} MpAffineTransformFlt;

/**
 * Affine 2-dimensional coordinate transform.
 *
 * The structure `MpAffineTransformDbl` is the same as @ref MpAffineTransformFlt
 * but with double precision floating-point coefficients.
 */
typedef struct _MpAffineTransformDbl {
    double xx, xy, x;
    double yx, yy, y;
} MpAffineTransformDbl;


/*
 * The following macros yield the abscissa `X` or the ordinate `Y` resulting
 * from applying an affine transform `A` to coordinates `(X,Y)`. Argument `E`
 * is a macro such that `E(A,M)` yields the member `M` of `A`.  Usually, `E` is
 * `MP_GET_FIELD` or `MP_GET_FIELD_PTR`.
 */
#define MP_XFORM_APPLY_X(E,A,X,Y)  (E(A,xx)*(X) + E(A,xy)*(Y) + E(A,x))
#define MP_XFORM_APPLY_Y(E,A,X,Y)  (E(A,yx)*(X) + E(A,yy)*(Y) + E(A,y))

/*--------------------------------------------------------------------------*/
/* Compose affine transforms. */

/**
 * Compose affine transforms.
 *
 * This function stores in `dst` the affine transform `A⋅B` resulting from the
 * composition of affine transforms `A` and `B`, that is the affine transform
 * whose effect is equivalent to applying `B` and then `A`.
 *
 * @param dst  The destination.
 * @param A    The left operand.
 * @param B    The right operand.
 *
 * @warning The operation cannot be done in-place, that is `dst` must be
 * different from `A` and `B`.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus
MpComposeAffineTransformsFlt(MpAffineTransformFlt* dst,
                             const MpAffineTransformFlt* A,
                             const MpAffineTransformFlt* B);

/**
 * Compose affine transforms.
 *
 * This is the same as @ref MpComposeAffineTransformsFlt but with double
 * precision floating-point coefficients.
 */
extern MpStatus
MpComposeAffineTransformsDbl(MpAffineTransformDbl* C,
                             const MpAffineTransformDbl* A,
                             const MpAffineTransformDbl* B);

/**
 * Compose affine transforms.
 *
 * This function yields the affine transform `A⋅B` resulting from the
 * composition of affine transforms `A` and `B`, that is the affine transform
 * whose effect is equivalent to applying `B` and then `A`.
 *
 * This version is similar to @ref MpComposeAffineTransformsFlt except that
 * argument are passed by value and that the destination is automatically
 * allocated (on the stack).
 *
 * @param A    The left operand.
 * @param B    The right operand.
 *
 * @return The affine transform `A⋅B`.
 */
extern MpAffineTransformFlt
MpComposeAffineTransformsAltFlt(MpAffineTransformFlt A,
                                MpAffineTransformFlt B);

/**
 * Compose affine transforms.
 *
 * This is the same as @ref MpComposeAffineTransformsAltFlt but with double
 * precision floating-point coefficients.
 */
extern MpAffineTransformDbl
MpComposeAffineTransformsAltDbl(MpAffineTransformDbl A,
                                MpAffineTransformDbl B);

/*
 * The following macros yield the components of the composition `A⋅B` of affine
 * transforms `A` and `B`.  Argument `E` is a macro such that `E(A,M)` and
 * `E(B,M)` yield the member `M` of `A` and `B`.  Usually, `E` is
 * `MP_GET_FIELD` or `MP_GET_FIELD_PTR`.
 */
#define MP_XFORM_COMPOSE_XX(E,A,B)  (E(A,xx)*E(B,xx) + E(A,xy)*E(B,yx)        )
#define MP_XFORM_COMPOSE_XY(E,A,B)  (E(A,xx)*E(B,xy) + E(A,xy)*E(B,yy)        )
#define MP_XFORM_COMPOSE_X( E,A,B)  (E(A,xx)*E(B,x)  + E(A,xy)*E(B,y) + E(A,x))
#define MP_XFORM_COMPOSE_YX(E,A,B)  (E(A,yx)*E(B,xx) + E(A,yy)*E(B,yx)        )
#define MP_XFORM_COMPOSE_YY(E,A,B)  (E(A,yx)*E(B,xy) + E(A,yy)*E(B,yy)        )
#define MP_XFORM_COMPOSE_Y( E,A,B)  (E(A,yx)*E(B,x)  + E(A,yy)*E(B,y) + E(A,y))
#define MP_XFORM_COMPOSE(E,DST,A,B)             \
    do {                                        \
        E(DST,xx) = MP_XFORM_COMPOSE_XX(E,A,B); \
        E(DST,xy) = MP_XFORM_COMPOSE_XY(E,A,B); \
        E(DST,x ) = MP_XFORM_COMPOSE_X( E,A,B); \
        E(DST,yx) = MP_XFORM_COMPOSE_YX(E,A,B); \
        E(DST,yy) = MP_XFORM_COMPOSE_YX(E,A,B); \
        E(DST,y ) = MP_XFORM_COMPOSE_Y( E,A,B); \
    } while (0)

/*--------------------------------------------------------------------------*/
/* Scaling an affine transform. */

/**
 * Left-scaling an affine transforms.
 *
 * This function stores in `dst` the result of left-multiplying an affine
 * transform `A` by a multiplier `alpha`.  Left-scaling amounts to multiplying
 * by `alpha` the output of the transform `A`.
 *
 * The operation can be done in-place, that is `dst` and `A` can be the same
 * object.
 *
 * @param dst    The destination.
 * @param alpha  The multiplier.
 * @param A      The operand.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus
MpLeftScaleAffineTransformFlt(MpAffineTransformFlt* dst,
                              float alpha,
                              const MpAffineTransformFlt* A);

/**
 * Left-scaling an affine transforms.
 *
 * This is the same as @ref MpLeftScaleAffineTransformFlt but with double
 * precision floating-point coefficients.
 */
extern MpStatus
MpLeftScaleAffineTransformDbl(MpAffineTransformDbl* dst,
                              double alpha,
                              const MpAffineTransformDbl* A);

/**
 * Right-scaling an affine transforms.
 *
 * This function stores in `dst` the result of right-multiplying an affine
 * transform `A` by a multiplier `alpha`.  Right-scaling amounts to multiplying
 * by `alpha` the input of the transform `A`.
 *
 * The operation can be done in-place, that is `dst` and `A` can be the same
 * object.
 *
 * @param dst    The destination.
 * @param alpha  The multiplier.
 * @param A      The operand.
 *
 * FIXME:
 */
extern MpStatus
MpRightScaleAffineTransformFlt(MpAffineTransformFlt* dst,
                               const MpAffineTransformFlt* A,
                               float alpha);

/**
 * Right-scaling an affine transforms.
 *
 * This is the same as @ref MpRightScaleAffineTransformFlt but with double
 * precision floating-point coefficients.
 */
extern MpStatus
MpRightScaleAffineTransformDbl(MpAffineTransformDbl* dst,
                               const MpAffineTransformDbl* A,
                               double alpha);


/*
 * The following macros are to left-scale an affine transform `A` by a
 * multiplier `ALPHA`.  Left-scaling amounts to scaling the output of the
 * transform.
 */
#define MP_XFORM_LEFT_SCALE_XX(E,ALPHA,A)  ((ALPHA)*E(A,xx))
#define MP_XFORM_LEFT_SCALE_XY(E,ALPHA,A)  ((ALPHA)*E(A,xy))
#define MP_XFORM_LEFT_SCALE_X( E,ALPHA,A)  ((ALPHA)*E(A,x) )
#define MP_XFORM_LEFT_SCALE_YX(E,ALPHA,A)  ((ALPHA)*E(A,yx))
#define MP_XFORM_LEFT_SCALE_YY(E,ALPHA,A)  ((ALPHA)*E(A,yy))
#define MP_XFORM_LEFT_SCALE_Y( E,ALPHA,A)  ((ALPHA)*E(A,y) )
#define MP_XFORM_LEFT_SCALE(E,DST,ALPHA,A)              \
    do {                                                \
        E(DST,xx) = MP_XFORM_LEFT_SCALE_XX(E,ALPHA,A);  \
        E(DST,xy) = MP_XFORM_LEFT_SCALE_XY(E,ALPHA,A);  \
        E(DST,x ) = MP_XFORM_LEFT_SCALE_X( E,ALPHA,A);  \
        E(DST,yx) = MP_XFORM_LEFT_SCALE_YX(E,ALPHA,A);  \
        E(DST,yy) = MP_XFORM_LEFT_SCALE_YY(E,ALPHA,A);  \
        E(DST,y ) = MP_XFORM_LEFT_SCALE_Y( E,ALPHA,A);  \
    } while (0)

/*
 * The following macros are to right-scale an affine transform `A` by a
 * multiplier `ALPHA`.  Right-scaling amounts to scaling the input of the
 * transform.
 */
#define MP_XFORM_RIGHT_SCALE_XX(E,A,ALPHA) ((ALPHA)*E(A,xx))
#define MP_XFORM_RIGHT_SCALE_XY(E,A,ALPHA) ((ALPHA)*E(A,xy))
#define MP_XFORM_RIGHT_SCALE_X( E,A,ALPHA) (        E(A,x) )
#define MP_XFORM_RIGHT_SCALE_YX(E,A,ALPHA) ((ALPHA)*E(A,yx))
#define MP_XFORM_RIGHT_SCALE_YY(E,A,ALPHA) ((ALPHA)*E(A,yy))
#define MP_XFORM_RIGHT_SCALE_Y( E,A,ALPHA) (        E(A,y) )
#define MP_XFORM_RIGHT_SCALE(E,DST,A,ALPHA)             \
    do {                                                \
        E(DST,xx) = MP_XFORM_RIGHT_SCALE_XX(E,A,ALPHA); \
        E(DST,xy) = MP_XFORM_RIGHT_SCALE_XY(E,A,ALPHA); \
        E(DST,x ) = MP_XFORM_RIGHT_SCALE_X( E,A,ALPHA); \
        E(DST,yx) = MP_XFORM_RIGHT_SCALE_YX(E,A,ALPHA); \
        E(DST,yy) = MP_XFORM_RIGHT_SCALE_YY(E,A,ALPHA); \
        E(DST,y ) = MP_XFORM_RIGHT_SCALE_Y( E,A,ALPHA); \
    } while (0)

/*--------------------------------------------------------------------------*/
/* Translating an affine transform. */

extern MpStatus
MpLeftTranslateAffineTransformFlt(MpAffineTransformFlt* dst,
                                  float x, float y,
                                  const MpAffineTransformFlt* A);

extern MpStatus
MpLeftTranslateAffineTransformDbl(MpAffineTransformDbl* dst,
                                  double x, double y,
                                  const MpAffineTransformDbl* A);

extern MpStatus
MpRightTranslateAffineTransformFlt(MpAffineTransformFlt* dst,
                                   const MpAffineTransformFlt* A,
                                   float x, float y);

extern MpStatus
MpRightTranslateAffineTransformDbl(MpAffineTransformDbl* dst,
                                   const MpAffineTransformDbl* A,
                                   double x, double y);

/*
 * The following macros are to left-translate an affine transform `A` by a
 * vector `(X,Y)`.  Left-translating amounts to translating the output of the
 * transform.
 */
#define MP_XFORM_LEFT_TRANSLATE_XX(E,X,Y,A) (E(A,xx)     )
#define MP_XFORM_LEFT_TRANSLATE_XY(E,X,Y,A) (E(A,xy)     )
#define MP_XFORM_LEFT_TRANSLATE_X( E,X,Y,A) (E(A,x) + (X))
#define MP_XFORM_LEFT_TRANSLATE_YX(E,X,Y,A) (E(A,yx)     )
#define MP_XFORM_LEFT_TRANSLATE_YY(E,X,Y,A) (E(A,yy)     )
#define MP_XFORM_LEFT_TRANSLATE_Y( E,X,Y,A) (E(A,y) + (Y))
#define MP_XFORM_LEFT_TRANSLATE(E,DST,X,Y,A)                    \
    do {                                                        \
        E(DST,xx) = MP_XFORM_LEFT_TRANSLATE_XX(E,X,Y,A);        \
        E(DST,xy) = MP_XFORM_LEFT_TRANSLATE_XY(E,X,Y,A);        \
        E(DST,x ) = MP_XFORM_LEFT_TRANSLATE_X( E,X,Y,A);        \
        E(DST,yx) = MP_XFORM_LEFT_TRANSLATE_YX(E,X,Y,A);        \
        E(DST,yy) = MP_XFORM_LEFT_TRANSLATE_YY(E,X,Y,A);        \
        E(DST,y ) = MP_XFORM_LEFT_TRANSLATE_Y( E,X,Y,A);        \
    } while (0)

/*
 * The following macros are to right-translate an affine transform `A` by a
 * vector `(X,Y)`.  Right-translating amounts to translating the input of the
 * transform.
 */
#define MP_XFORM_RIGHT_TRANSLATE_XX(E,A,X,Y) (E(A,xx)                           )
#define MP_XFORM_RIGHT_TRANSLATE_XY(E,A,X,Y) (E(A,xy)                           )
#define MP_XFORM_RIGHT_TRANSLATE_X( E,A,X,Y) (E(A,xx)*(X) + E(A,xy)*(Y) + E(A,x))
#define MP_XFORM_RIGHT_TRANSLATE_YX(E,A,X,Y) (E(A,yx)                           )
#define MP_XFORM_RIGHT_TRANSLATE_YY(E,A,X,Y) (E(A,yy)                           )
#define MP_XFORM_RIGHT_TRANSLATE_Y( E,A,X,Y) (E(A,yx)*(X) + E(A,yy)*(Y) + E(A,y))
#define MP_XFORM_RIGHT_TRANSLATE(E,DST,A,X,Y)                   \
    do {                                                        \
        E(DST,xx) = MP_XFORM_RIGHT_TRANSLATE_XX(E,A,X,Y);       \
        E(DST,xy) = MP_XFORM_RIGHT_TRANSLATE_XY(E,A,X,Y);       \
        E(DST,x ) = MP_XFORM_RIGHT_TRANSLATE_X( E,A,X,Y);       \
        E(DST,yx) = MP_XFORM_RIGHT_TRANSLATE_YX(E,A,X,Y);       \
        E(DST,yy) = MP_XFORM_RIGHT_TRANSLATE_YY(E,A,X,Y);       \
        E(DST,y ) = MP_XFORM_RIGHT_TRANSLATE_Y( E,A,X,Y);       \
    } while (0)

/*--------------------------------------------------------------------------*/
/* Rotating an affine transform. */

extern MpStatus
MpLeftRotateAffineTransformFlt(MpAffineTransformFlt* dst,
                               float theta,
                               const MpAffineTransformFlt* A);

extern MpStatus
MpLeftRotateAffineTransformDbl(MpAffineTransformDbl* dst,
                               double theta,
                               const MpAffineTransformDbl* A);

extern MpStatus
MpRightRotateAffineTransformFlt(MpAffineTransformFlt* dst,
                                const MpAffineTransformFlt* A,
                                float theta);

extern MpStatus
MpRightRotateAffineTransformDbl(MpAffineTransformDbl* dst,
                                const MpAffineTransformDbl* A,
                                double theta);

/*
 * The following macros are to left-rotate an affine transform `A` by an
 * angle.  Left-rotating amounts to rotating the output of the transform.
 * Arguments `SN` and `CS` are respectively the sine and the cosine of the
 * rotation angle.
 */
#define MP_XFORM_LEFT_ROTATE_XX(E,SN,CS,A)  ((CS)*E(A,xx) - (SN)*E(A,yx))
#define MP_XFORM_LEFT_ROTATE_XY(E,SN,CS,A)  ((CS)*E(A,xy) - (SN)*E(A,yy))
#define MP_XFORM_LEFT_ROTATE_X( E,SN,CS,A)  ((CS)*E(A,x)  - (SN)*E(A,y) )
#define MP_XFORM_LEFT_ROTATE_YX(E,SN,CS,A)  ((CS)*E(A,yx) + (SN)*E(A,xx))
#define MP_XFORM_LEFT_ROTATE_YY(E,SN,CS,A)  ((CS)*E(A,yy) + (SN)*E(A,xy))
#define MP_XFORM_LEFT_ROTATE_Y( E,SN,CS,A)  ((CS)*E(A,y)  + (SN)*E(A,x) )
#define MP_XFORM_LEFT_ROTATE(E,DST,SN,CS,A)             \
    do {                                                \
        E(DST,xx) = MP_XFORM_LEFT_ROTATE_XX(E,SN,CS,A); \
        E(DST,xy) = MP_XFORM_LEFT_ROTATE_XY(E,SN,CS,A); \
        E(DST,x ) = MP_XFORM_LEFT_ROTATE_X( E,SN,CS,A); \
        E(DST,yx) = MP_XFORM_LEFT_ROTATE_YX(E,SN,CS,A); \
        E(DST,yy) = MP_XFORM_LEFT_ROTATE_YY(E,SN,CS,A); \
        E(DST,y ) = MP_XFORM_LEFT_ROTATE_Y( E,SN,CS,A); \
    } while (0)

/*
 * The following macros are to right-rotate an affine transform `A` by an
 * angle.  Right-rotating amounts to rotating the input of the transform.
 * Arguments `SN` and `CS` are respectively the sine and the cosine of the
 * rotation angle.
 */
#define MP_XFORM_RIGHT_ROTATE_XX(E,A,SN,CS) ((CS)*E(A,xx) + (SN)*E(A,xy))
#define MP_XFORM_RIGHT_ROTATE_XY(E,A,SN,CS) ((CS)*E(A,xy) - (SN)*E(A,xx))
#define MP_XFORM_RIGHT_ROTATE_X( E,A,SN,CS) (E(A,x)                     )
#define MP_XFORM_RIGHT_ROTATE_YX(E,A,SN,CS) ((CS)*E(A,yx) + (SN)*E(A,yy))
#define MP_XFORM_RIGHT_ROTATE_YY(E,A,SN,CS) ((CS)*E(A,yy) - (SN)*E(A,yx))
#define MP_XFORM_RIGHT_ROTATE_Y( E,A,SN,CS) (E(A,y)                     )
#define MP_XFORM_RIGHT_ROTATE(E,DST,A,SN,CS)                    \
    do {                                                        \
        E(DST,xx) = MP_XFORM_RIGHT_ROTATE_XX(E,A,SN,CS);        \
        E(DST,xy) = MP_XFORM_RIGHT_ROTATE_XY(E,A,SN,CS);        \
        E(DST,x ) = MP_XFORM_RIGHT_ROTATE_X( E,A,SN,CS);        \
        E(DST,yx) = MP_XFORM_RIGHT_ROTATE_YX(E,A,SN,CS);        \
        E(DST,yy) = MP_XFORM_RIGHT_ROTATE_YY(E,A,SN,CS);        \
        E(DST,y ) = MP_XFORM_RIGHT_ROTATE_Y( E,A,SN,CS);        \
    } while (0)

/*--------------------------------------------------------------------------*/
/* Determinant of an affine transform. */

extern float MpDeterminantAffineTransformFlt(const MpAffineTransformFlt* A);

extern double MpDeterminantAffineTransformDblt(const MpAffineTransformFlt* A);

/*
 * The following macro yields the determinant of the linear part of the affine
 * transform `A`.  Argument `E` is a macro such that `E(A,M)` yields the member
 * `M` of `A`.  Usually, `E` is `MP_GET_FIELD` or `MP_GET_FIELD_PTR`.
 */
#define MP_XFORM_DETERMINANT(E,A) (E(A,xx)*E(A,yy) - E(A,xy)*E(A,yx))

/*--------------------------------------------------------------------------*/
/* Inversion of an affine transform. */

extern MpStatus
MpInverseAffineTransformFlt(MpAffineTransformFlt* dst,
                            const MpAffineTransformFlt* A);

extern MpStatus
MpInverseAffineTransformDbl(MpAffineTransformDbl* dst,
                            const MpAffineTransformDbl* A);

/*
 * The following macro is to store in `DST` the coefficients of the inverse of
 * the affine transform `A`.  Argument `T` is the floating-point type of the
 * coefficients and argument `E` is a macro such that `E(A,M)` yields the
 * member `M` of `A` (and similarly for `DST`).  Usually, `E` is
 * `MP_GET_FIELD` or `MP_GET_FIELD_PTR`.
 */
#define MP_XFORM_INVERSE(T,E,DST,A)                     \
    do {                                                \
        T _delta = MP_XFORM_DETERMINANT(E,A);           \
        if (_delta == 0) {                              \
            return MP_SINGULAR;                         \
        }                                               \
        T _alpha = 1/_delta;                            \
        T _Rxx =  _alpha*E(A,yy);                       \
        T _Rxy = -_alpha*E(A,xy);                       \
        T _Ryx = -_alpha*E(A,yx);                       \
        T _Ryy = _alpha*E(A,xx);                        \
        E(DST,xx) =   _Rxx;                             \
        E(DST,xy) =   _Rxy;                             \
        E(DST,x ) = -(_Rxy*E(A,y) + _Rxx*E(A,x));       \
        E(DST,yx) =   _Ryx;                             \
        E(DST,yy) =   _Ryy;                             \
        E(DST,y ) = -(_Ryx*E(A,x) + _Ryy*E(A,y));       \
    } while(0)

/*--------------------------------------------------------------------------*/
/* Division of affine transforms. */

/**
 * Left-divide affine transforms.
 *
 * This function stores in `dst` the affine transform `A\\B` resulting from the
 * left division of the affine transform `B` by the affine transform `A`.
 *
 * @param dst  The destination.
 * @param A    The left operand.
 * @param B    The right operand.
 *
 * @warning The operation cannot be done in-place, that is `dst` must be
 * different from `A` and `B`.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus
MpLeftDivideAffineTransformsFlt(MpAffineTransformFlt* dst,
                                const MpAffineTransformFlt* A,
                                const MpAffineTransformFlt* B);

/**
 * Left-divide affine transforms.
 *
 * This is the same as @ref MpLeftDivideAffineTransformsFlt but with double
 * precision floating-point coefficients.
 */
extern MpStatus
MpLeftDivideAffineTransformsDbl(MpAffineTransformDbl* dst,
                                const MpAffineTransformDbl* A,
                                const MpAffineTransformDbl* B);

/**
 * Right-divide affine transforms.
 *
 * This function stores in `dst` the affine transform `A/B` resulting from the
 * right division of the affine transform `A` by the affine transform `B`.
 *
 * @param dst  The destination.
 * @param A    The left operand.
 * @param B    The right operand.
 *
 * @warning The operation cannot be done in-place, that is `dst` must be
 * different from `A` and `B`.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus
MpRightDivideAffineTransformsFlt(MpAffineTransformFlt* dst,
                                 const MpAffineTransformFlt* A,
                                 const MpAffineTransformFlt* B);

/**
 * Right-divide affine transforms.
 *
 * This is the same as @ref MpRightDivideAffineTransformsFlt but with double
 * precision floating-point coefficients.
 */
extern MpStatus
MpRightDivideAffineTransformsDbl(MpAffineTransformDbl* dst,
                                 const MpAffineTransformDbl* A,
                                 const MpAffineTransformDbl* B);

/*
 * The following macro is to store in `DST` the coefficients of `A\\B`, the
 * left division of the affine transform `B` by the affine transform `A`.
 * Argument `T` is the floating-point type of the coefficients and argument `E`
 * is a macro such that `E(A,M)` yields the member `M` of `A` (and similarly
 * for `DST` and `B`).  Usually, `E` is `MP_GET_FIELD` or
 * `MP_GET_FIELD_PTR`.
 */
#define MP_XFORM_LEFT_DIVIDE(T,E,DST,A,B)               \
    do {                                                \
        T _delta = MP_XFORM_DETERMINANT(E,A);           \
        if (_delta == 0) {                              \
            return MP_SINGULAR;                         \
        }                                               \
        T _alpha = 1/_delta;                            \
        T _Txx = _alpha*E(A,yy);                        \
        T _Txy = _alpha*E(A,xy);                        \
        T _Tyx = _alpha*E(A,yx);                        \
        T _Tyy = _alpha*E(A,xx);                        \
        T _Tx = E(B,x) - E(A,x);                        \
        T _Ty = E(B,y) - E(A,y);                        \
        E(DST,xx) = _Txx*E(B,xx) - _Txy*E(B,yx);        \
        E(DST,xy) = _Txx*E(B,xy) - _Txy*E(B,yy);        \
        E(DST,x ) = _Txx*_Tx     - _Txy*_Ty;            \
        E(DST,yx) = _Tyy*E(B,yx) - _Tyx*E(B,xx);        \
        E(DST,yy) = _Tyy*E(B,yy) - _Tyx*E(B,xy);        \
        E(DST,y ) = _Tyy*_Ty     - _Tyx*_Tx;            \
    } while (0)

/*
 * The following macro is to store in `DST` the coefficients of `A/B`, the
 * right division of the affine transform `A` by the affine transform `B`.
 * Argument `T` is the floating-point type of the coefficients and argument `E`
 * is a macro such that `E(A,M)` yields the member `M` of `A` (and similarly
 * for `DST` and `B`).  Usually, `E` is `MP_GET_FIELD` or
 * `MP_GET_FIELD_PTR`.
 */
#define MP_XFORM_RIGHT_DIVIDE(T,E,DST,A,B)                      \
    do {                                                        \
        T _delta = MP_XFORM_DETERMINANT(E,A);                   \
        if (_delta == 0) {                                      \
            return MP_SINGULAR;                                 \
        }                                                       \
        T _alpha = 1/_delta;                                    \
        T _Rxx = _alpha*(E(A,xx)*E(B,yy) - E(A,xy)*E(B,yx));    \
        T _Rxy = _alpha*(E(A,xy)*E(B,xx) - E(A,xx)*E(B,xy));    \
        T _Ryx = _alpha*(E(A,yx)*E(B,yy) - E(A,yy)*E(B,yx));    \
        T _Ryy = _alpha*(E(A,yy)*E(B,xx) - E(A,yx)*E(B,xy));    \
        E(DST,xx) = _Rxx;                                       \
        E(DST,xy) = _Rxy;                                       \
        E(DST,x ) = E(A,x) - (_Rxx*E(B,x) + _Rxy*E(B,y));       \
        E(DST,yx) = _Ryx;                                       \
        E(DST,yy) = _Ryy;                                       \
        E(DST,y ) = E(A,y) - (_Ryx*E(B,y) + _Ryy*E(B,y));       \
    } while (0)

/*--------------------------------------------------------------------------*/
/* Intercept of an affine transform. */

/**
 * Intercept of an affine transforms.
 *
 * This function yields the coordinates `(x,y)` of the intercept point such
 * that applying tha affine transform `A` to the coordinates `(x,y)` yields
 * `(0,0)`.
 *
 * @param xptr  The address to store the abscissa of the intercept.
 * @param yptr  The address to store the ordinate of the intercept.
 * @param A     The affine transform.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure
 * (e.g. `MP_SINGULAR` if the transform in non-invertible).
 */
extern MpStatus
MpInterceptAffineTransformFlt(float* xptr, float* yptr,
                              const MpAffineTransformFlt* A);

/**
 * Intercept of an affine transforms.
 *
 * This is the same as @ref MpInterceptAffineTransformFlt but with double
 * precision floating-point coefficients.
 */
extern MpStatus
MpInterceptAffineTransformDbl(double* xptr, double* yptr,
                              const MpAffineTransformDbl* A);

/*
 * The following macro is to compute `(X,Y)` such that `A⋅(X,Y) = (0,0)`.
 * Argument `T` is the floating-point type of the coefficients and argument `E`
 * is a macro such that `E(A,M)` yields the member `M` of `A`.  Usually, `E` is
 * `MP_GET_FIELD` or `MP_GET_FIELD_PTR`.
 */
#define MP_XFORM_INTERCEPT(T,E,X,Y,A)                   \
    do {                                                \
        T _delta = MP_XFORM_DETERMINANT(E,A);           \
        if (_delta == 0) {                              \
            return MP_SINGULAR;                         \
        }                                               \
        T _alpha = (T)1/_delta;                         \
        (X) = _alpha*(E(A,xy)*E(A,y) - E(A,yy)*E(A,x)); \
        (Y) = _alpha*(E(A,yx)*E(A,x) - E(A,xx)*E(A,y)); \
    } while (0)

_MP_END_DECLS

#endif /* _MUPLOT_XFORMS_H */
