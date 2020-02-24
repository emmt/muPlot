/*
 * muPlot.h --
 *
 * Definitions for the µPlot plotting library.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#ifndef _MUPLOT_H_
#define _MUPLOT_H_ 1

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
#  define _MP_BEGIN_DECLS extern "C" {
#  define _MP_END_DECLS }
#else
#  define _MP_BEGIN_DECLS
#  define _MP_END_DECLS
#endif

/**
 * @def MP_IS_SINGLE_PRECISION(T)
 *
 * Check whether floating-point type `T` is single precision.  The macro must be
 * evaluated by the compiler (i.e. not the preprocessor), the result is
 * constant so it will probably be eliminated by the optimizer.
 */
#define MP_IS_SINGLE_PRECISION(T) (sizeof(T) <= sizeof(float))

/**
 * @def MP_IS_FINITE(x)
 *
 * Macro to check whether `x` is finite.  Argument is evaluated twice, it is
 * better if it is a variable, not an expression.
 */
#define MP_IS_FINITE(x)  (((x) - (x)) == 0)

/**
 * @def MP_IS_NAN(x)
 *
 * Macro to check whether `x` is a NaN (not-a-number).  Argument is evaluated
 * twice, it is better if it is a variable, not an expression.
 */
#define MP_IS_NAN(x)   ((x) != (x))

/**
 * @def MP_NEW(T)
 *
 * Allocate a memory for an object of type `T`.  The returned memory is
 * zero-filled.
 */
#define MP_NEW(T)   ((T*)calloc(1, sizeof(T)))

/**
 * @def MP_OFFSET_OF(T)
 *
 * This macro yields the offset (in bytes) of member `M` in a structure of type
 * `T`.
 */
#define MP_OFFSET_OF(T, M)  (((char*)&(((T*)0)->M)) - ((char*)0))

/**
 * @def MP_GET_FIELD(A,M)
 *
 * This macro yields (as an L-value) the member `M` of a structured object `A`.
 */
#define MP_GET_FIELD(A,M)      ((A).M)

/**
 * @def MP_GET_FIELD_PTR(A,M)
 *
 * This macro yields (as an L-value) the member `M` of a structured object
 * given a pointer `A`.
 */
#define MP_GET_FIELD_PTR(A,M)    ((A)->M)

_MP_BEGIN_DECLS

typedef float   MpReal;
typedef long    MpInt;
typedef bool    MpBool;
typedef MpInt   MpColorIndex;
typedef int16_t MpPoint; /* for device coordinates, sufficient for giga-pixel images */

/**
 * @defgroup Errors Mangement of errors.
 */

/**
 * @addtogroup Errors
 *
 * @{
 */

/* List of status symbolic names awith corresponding code and message.  The
   macro `_MP_STATUS(a,b,c)` shall be defined appropriately before using this
   list. */
#define _MP_STATUS_LIST                                                 \
    _MP_STATUS(MP_OK,                0, "successful operation")         \
    _MP_STATUS(MP_ASSERTION_FAILED,  1, "assertion failed")             \
    _MP_STATUS(MP_BAD_ADDRESS,       2, "invalid address")              \
    _MP_STATUS(MP_BAD_ARGUMENT,      3, "invalid argument")             \
    _MP_STATUS(MP_BAD_DEVICE,        4, "invalid device")               \
    _MP_STATUS(MP_BAD_FILENAME,      5, "invalid filename")             \
    _MP_STATUS(MP_BAD_IDENTIFIER,    6, "invalid identifier")           \
    _MP_STATUS(MP_BAD_METHOD,        7, "invalid method")               \
    _MP_STATUS(MP_BAD_SETTINGS,      8, "invalid settings")             \
    _MP_STATUS(MP_BAD_SIZE,          9, "invalid size")                 \
    _MP_STATUS(MP_NOT_FOUND,        10, "not found")                    \
    _MP_STATUS(MP_NOT_IMPLEMENTED,  11, "not implemented")              \
    _MP_STATUS(MP_NOT_PERMITTED,    12, "forbidden operation")          \
    _MP_STATUS(MP_NO_MEMORY,        13, "insufficient memory")          \
    _MP_STATUS(MP_OUT_OF_RANGE,     14, "out of range value or index")  \
    _MP_STATUS(MP_READ_ONLY,        15, "read only parameter")          \
    _MP_STATUS(MP_SINGULAR,         16, "singular system of equations")

typedef enum {
#define _MP_STATUS(a,b,c) a = b,
    _MP_STATUS_LIST
#undef _MP_STATUS
} MpStatus;

/**
 * Get error message.
 *
 * Call this routine to retrieve the message associated with a given status.
 *
 * @param status  Value returned by one of the MpPlot routines.
 *
 * @return A literal message.
 */
extern const char* MpGetReason(MpStatus status);

/**
 * Report system error.
 *
 * This function reports a system error based on the current value of `errno`.
 *
 * @return The error status corresponding to the last system error.
 */
extern MpStatus MpSystemError(void);

/**
 * @}
 */

/*---------------------------------------------------------------------------*/
/* BOXES AND (SIMPLE) COORDINATE MAPPINGS */

/**
 * @defgroup Mappings Boxes and coordinate mappings.
 */

/**
 * @addtogroup Mappings
 *
 * @{
 */
/**
 * @struct MpBoxFlt
 *
 * A rectangular box whose edges are aligned with the coordinate axes.
 */
typedef struct _MpBoxFlt {
    float xmin, xmax, ymin, ymax;
} MpBoxFlt;

/**
 * @struct MpBoxDbl
 *
 * A rectangular box whose edges are aligned with the coordinate axes.
 */
typedef struct _MpBoxDbl {
    double xmin, xmax, ymin, ymax;
} MpBoxDbl;

/**
 * @struct MpMappingFlt
 *
 * A simple coordinate mapping for single precision coordinates.
 */
typedef struct _MpMappingFlt {
    float xx, x;
    float yy, y;
} MpMappingFlt;

/**
 * @struct MpMappingDbl
 *
 * A simple coordinate mapping for double precision coordinates.
 */
typedef struct _MpMappingDbl {
    double xx, x;
    double yy, y;
} MpMappingDbl;

#define MP_APPLY_SIMPLE_MAPPING_X(E,A,X,Y) (E(A,xx)*(X) + E(A,x))
#define MP_APPLY_SIMPLE_MAPPING_Y(E,A,X,Y) (E(A,yy)*(Y) + E(A,y))

extern MpStatus MpCheckBoxFlt(const MpBoxFlt* box);
extern MpStatus MpCheckBoxDbl(const MpBoxDbl* box);

/* Assume that box has been checked. */
extern MpBool MpIsEmptyBoxFlt(const MpBoxFlt* box);
extern MpBool MpIsEmptyBoxDbl(const MpBoxDbl* box);

extern MpStatus MpCheckMappingFlt(const MpMappingFlt* map);
extern MpStatus MpCheckMappingDbl(const MpMappingDbl* map);

#define MP_FLIP_X        (1 << 0)
#define MP_FLIP_Y        (1 << 1)
#define MP_FLIP_X_AND_Y  (MP_FLIP_X | MP_FLIP_Y)
#define MP_FLIP_NONE     0
#define MP_FLIP_BOTH     MP_FLIP_X_AND_Y

/**
 * Define the mapping between two boxes.
 *
 * This function defines the mapping from an input box to an output box.  The
 * resulting mapping is complete in the sense that any point of the input box
 * is mapped to a point inside the output box and, conversely, any point of the
 * output box has a counterpart in the input box.  An error is reported if this
 * does not hold.
 *
 * @param dst   The destination structure.
 * @param inp   The input box.
 * @param out   The output box.
 * @param flip  Bit mask indicating whether the X and/or Y axes are flipped.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpDefineMappingFlt(MpMappingFlt* dst,
                                   const MpBoxFlt* inp,
                                   const MpBoxFlt* out,
                                   unsigned flip);

/**
 * Define the mapping between two boxes.
 *
 * This function is identical to MpDefineMappingFlt() but for double precision
 * coordinates.
 */
extern MpStatus MpDefineMappingDbl(MpMappingDbl* dst,
                                   const MpBoxDbl* inp,
                                   const MpBoxDbl* out,
                                   unsigned flip);

/**
 * Compose two coordinate mapping.
 *
 * This function yields the mapping `A.B` that results from the composition of
 * two mappings`A` and `B`.
 *
 * @param dst   The destination structure.
 * @param A     The left mapping.
 * @param B     The right mapping.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpComposeMappingsFlt(MpMappingFlt* dst,
                                     const MpMappingFlt* A,
                                     const MpMappingFlt* B);

/**
 * Compose two coordinate mapping.
 *
 *
 * This function is identical to MpComposeMappingsFlt() but for double
 * precision coordinates.
 */
extern MpStatus MpComposeMappingsDbl(MpMappingDbl* dst,
                                     const MpMappingDbl* A,
                                     const MpMappingDbl* B);

/**
 * Invert a coordinate mapping.
 *
 * This function yields the inverse mapping of `A`.
 *
 * @param dst   The destination structure.
 * @param A     The mapping to invert.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpInvertMappingFlt(MpMappingFlt* dst,
                                   const MpMappingFlt* src);
/**
 * Invert a coordinate mapping.
 *
 * This function is identical to MpInvertMappingFlt() but for double precision
 * coordinates.
 */
extern MpStatus MpInvertMappingDbl(MpMappingDbl* dst,
                                   const MpMappingDbl* src);

#define _MP_IS_EMPTY_BOX(E,B) (E(B,xmin) > E(box,xmax) || \
                               E(B,ymin) > E(box,ymax))

#define MP_IS_EMPTY_BOX(B)     _MP_IS_EMPTY_BOX(MP_GET_FIELD,B)
#define MP_IS_EMPTY_BOX_PTR(B) _MP_IS_EMPTY_BOX(MP_GET_FIELD_PTR,B)

#define _MP_CHECK_BOX(E,B) ((MP_IS_FINITE(E(B,xmin)) && \
                             MP_IS_FINITE(E(B,xmax)) && \
                             MP_IS_FINITE(E(B,ymin)) && \
                             MP_IS_FINITE(E(B,ymax))) ? MP_OK : MP_SINGULAR)

#define MP_CHECK_BOX(B)     _MP_CHECK_BOX(MP_GET_FIELD,B)
#define MP_CHECK_BOX_PTR(B) _MP_CHECK_BOX(MP_GET_FIELD_PTR,B)

#define _MP_CHECK_MAPPING(E,A) ((MP_IS_FINITE(E(A,xx)) && \
                                 MP_IS_FINITE(E(A,x))  && \
                                 MP_IS_FINITE(E(A,yy)) && \
                                 MP_IS_FINITE(E(A,y))) ? MP_OK : MP_SINGULAR)

#define MP_CHECK_MAPPING(A)     _MP_CHECK_MAPPING(MP_GET_FIELD,A)
#define MP_CHECK_MAPPING_PTR(A) _MP_CHECK_MAPPING(MP_GET_FIELD_PTR,A)

/**
 * @}
 */

/*---------------------------------------------------------------------------*/
/* CLIPPING */

/**
 * @defgroup Clipping Clipping within boxes.
 */

/**
 * @addtogroup Clipping
 *
 * @{
 */

/**
 * Clip a line segment
 *
 * This function clips a line segment defined by its two end-points within
 * the region defined by a box.
 *
 * @param x1c    Address to store the clipped abscissa of the first point.
 * @param y1c    Address to store the clipped ordinate of the first point.
 * @param x2c    Address to store the clipped abscissa of the second point.
 * @param y2c    Address to store the clipped ordinate of the second point.
 * @parma box    The clipping box.  It is assumed that its limits are finite
 *               and have been ordered (i.e., `box->xmin ≤ box->xmax` and
 *               `box->ymin ≤ box->ymax` hold).
 * @param x1     Abscissa of the first point.
 * @param y1     Ordinate of the first point.
 * @param x2     Abscissa of the second point.
 * @param y2     Ordinate of the second point.
 *
 * @return An integer code: 1 if the unclipped segment fits in the box, 2
 * if the segment intercepts the box, 0 if the segment is outside the box.
 */
extern int
MpClipSegmentFlt(float* x1c, float* y1c, float* x2c, float* y2c,
                 const MpBoxFlt* box,
                 float x1, float y1, float x2, float y2);

/**
 * Clip a line segment
 *
 * This function is identical to MpClipSegmentFlt() but for double
 * precision coordinates.
 */
extern int
MpClipSegmentDbl(double* x1c, double* y1c, double* x2c, double* y2c,
                 const MpBoxDbl* box,
                 double x1, double y1, double x2, double y2);

/**
 * Clip several line segments
 *
 * This function clips a list of disjoint line segments defined by their two
 * end-points within the region defined by a box.
 *
 * @param xc     Address to store the clipped abscissae of the segments.
 *
 * @param yc     Address to store the clipped ordinates of the segments.
 *
 * @parma box    The clipping box.  It is assumed that its limits are finite
 *               and have been ordered (i.e., `box->xmin ≤ box->xmax` and
 *               `box->ymin ≤ box->ymax` hold).
 *
 * @param x      Input abscissae of the segments.
 *
 * @param y      Input ordinates of the segments.
 *
 * @param n      Number of segments.  Arrays `xc`, `yc`, `x` and `y` must have
 *               (at least) `2*n` elements.  The operation can be done
 *               in-place, that is `xc` and `yc` can be the same as `x` and
 *               `y`.  The segments are defined, on input, by end-points at
 *               coordinates `(x[2*i],y[2*i])` and `(x[2*i+1],y[2*i+1])` for `i
 *               = 0, ..., n-1` and, on output, by end-points at coordinates
 *               `(xc[2*j],yc[2*j])` and `(xc[2*j+1],yc[2*j+1])` for `j = 0,
 *               ..., nc-1` with `nc` the number of segments returned by the
 *               function.
 *
 * @return The number of segments stored in `xc` and `yc`.  This is the number
 *         of input segments that are not completely outside the box.
 */
extern MpInt
MpClipSegmentsFlt(float* xc, float* yc, const MpBoxFlt* box,
                  const float* x, const float* y, MpInt n);

/**
 * Clip several line segments
 *
 * This function is identical to MpClipSegmentsFlt() but for double
 * precision coordinates.
 */
extern MpInt
MpClipSegmentsDbl(double* xc, double* yc, const MpBoxDbl* box,
                  const double* x, const double* y, MpInt n);

/**
 * Clip a polyline within a box.
 *
 * There must be at least `2n - 1` values in `xc` and `yc`.
 *
 * @param xc     Array to store the abscissae of the segments to draw.
 * @param yc     Array to store the ordinates of the segments to draw.
 * @param box    The clipping box.
 * @param x      Abscissae of the points defining the polyline.
 * @param y      Ordinates of the points defining the polyline.
 * @param n      Number of points in the polyline.
 *
 * @return The number of segments to draw.
 */
extern MpInt
MpClipPolylineFlt(float* xc, float* yc, const MpBoxFlt* box,
                  const float* x, const float* y, MpInt n);

extern MpInt
MpClipPolylineDbl(double* xc, double* yc, const MpBoxDbl* box,
                  const double* x, const double* y, MpInt n);

extern MpStatus
MpDrawClippedSegmentFlt(void* ctx,
                        MpStatus (*move)(void* ctx, float x, float y),
                        MpStatus (*draw)(void* ctx, float x, float y),
                        const MpBoxFlt* box,
                        float x1, float y1, float x2, float y2);

extern MpStatus
MpDrawClippedSegmentDbl(void* ctx,
                        MpStatus (*move)(void* ctx, double x, double y),
                        MpStatus (*draw)(void* ctx, double x, double y),
                        const MpBoxDbl* box,
                        double x1, double y1, double x2, double y2);

extern MpStatus
MpDrawClippedPolylineFlt(void* ctx,
                         MpStatus (*move)(void* ctx, float x, float y),
                         MpStatus (*draw)(void* ctx, float x, float y),
                         const MpBoxFlt* box,
                         const float* x, const float* y, MpInt n);

extern MpStatus
MpDrawClippedPolylineDbl(void* ctx,
                         MpStatus (*move)(void* ctx, double x, double y),
                         MpStatus (*draw)(void* ctx, double x, double y),
                         const MpBoxDbl* box,
                         const double* x, const double* y, MpInt n);

extern MpStatus
MpDrawClippedSegmentsFlt(void* ctx,
                         MpStatus (*move)(void* ctx, float x, float y),
                         MpStatus (*draw)(void* ctx, float x, float y),
                         const MpBoxFlt* box,
                         const float* x, const float* y, MpInt n);

extern MpStatus
MpDrawClippedSegmentsDbl(void* ctx,
                         MpStatus (*move)(void* ctx, double x, double y),
                         MpStatus (*draw)(void* ctx, double x, double y),
                         const MpBoxDbl* box,
                         const double* x, const double* y, MpInt n);

typedef struct _MpClipStateFlt {
    unsigned c1; /* Clip bits for 1st point. */
    unsigned c2; /* Clip bits for 2nd point. */
    float x1, y1; /* Coordinates of 1st point. */
    float x2, y2; /* Coordinates of 2nd point. */
    float xmin, xmax, ymin, ymax; /* Clipping box. */
    float x1c, y1c; /* Clipped coordinates of 1st point. */
    float x2c, y2c; /* Clipped coordinates of 2nd point. */
} MpClipStateFlt;

typedef struct _MpClipStateDbl {
    unsigned c1; /* Clip bits for 1st point. */
    unsigned c2; /* Clip bits for 2nd point. */
    double x1, y1; /* Coordinates of 1st point. */
    double x2, y2; /* Coordinates of 2nd point. */
    double xmin, xmax, ymin, ymax; /* Clipping box. */
    double x1c, y1c; /* Clipped coordinates of 1st point. */
    double x2c, y2c; /* Clipped coordinates of 2nd point. */
} MpClipStateDbl;

/**
 * Intialize clipping of consecutive segments (polyline)
 *
 * This function initializes the state structure used to clip the consecutive
 * segments that make a polyline.
 *
 * @param w   Address of clipping state structure to initialize.
 *
 * @param x   Abscissa of the first point in the polyline.
 *
 * @param y   Abscissa of the first point in the polyline.
 *
 * @param box The clipping box.
 */
extern void MpInitializeClipFlt(MpClipStateFlt* w, float x, float y,
                                const MpBoxFlt* box);

/**
 * Intialize clipping of consecutive segments (polyline)
 *
 * This function is identical to MpInitializeClipFlt() but for double precision
 * coordinates.
 */
extern void MpInitializeClipDbl(MpClipStateDbl* w, double x, double y,
                                const MpBoxDbl* box);

extern void MpRestartClipFlt(MpClipStateFlt* w, float x, float y);

extern void MpRestartClipDbl(MpClipStateDbl* w, double x, double y);

/**
 * Clip next segment in a polyline
 *
 * This function clips the next segment in a polyline given the coordinates of
 * the next end-point.
 *
 * @param w   Address of clipping state structure (initialized by
 *            MpInitializeClipFlt()).
 *
 * @param x   Abscissa of the next point in the polyline.
 *
 * @param y   Abscissa of the next point in the polyline.
 *
 * @return The function update the state structure and returns one of:
 *
 * - `0` if the segment is outside the clipping box.
 *
 * - `1` if both end-points are inside the clipping box.  The segment to draw
 *   is defined by the points `(w->x1,w->y1)` and `(w->x2,w->y2)`.
 *
 * - `2` if the segment is partially inside the clipping box.  The segment to
 *   draw is defined by the points `(w->x1c,w->y1c)` and `(w->x2c,w->y2c)`.
 */
extern int MpClipNextFlt(MpClipStateFlt* w, float x, float y);

/**
 * Clip next segment in a polyline
 *
 * This function is identical to MpClipNextFlt() but for double precision
 * coordinates.
 */
extern int MpClipNextDbl(MpClipStateDbl* w, double x, double y);

/**
 * @def MP_CLIP_TBRL(X, Y, XMIN, XMAX, YMIN, YMAX)
 *
 * This macro yields the Top-Bottom-Right-Left bits of a point at coordinates
 * `(X,Y)` relative to a window defined by `XMIN ≤ X ≤ XMAX`, `YMIN ≤ Y ≤
 * YMAX`.  This assumes that the window (inclusive) limits are correctly
 * sorted.  The bits are set as follow:
 *
 * - if `X < XMIN`, bit `0001` is set;
 * - if `X > XMAX`, bit `0010` is set;
 * - if `Y < YMIN`, bit `0100` is set;
 * - if `Y > YMAX`, bit `1000` is set.
 */
#define MP_CLIP_TBRL(X, Y, XMIN, XMAX, YMIN, YMAX)      \
    (((X) < (XMIN) ? 1 :                                \
      ((X) > (XMAX) ? 2 : 0)) |                         \
     ((Y) < (YMIN) ? 4 :                                \
      ((Y) > (YMAX) ? 8 : 0)))

/**
 * @def MP_CLIP_INTERSECT(T,REJECT,X1C,Y1C,DX,DY,X1,Y1,X2,Y2,
 *                        XMIN,XMAX,YMIN,YMAX)
 *
 * This macro attempts to find the intersection of a segment with a window
 * limits by moving one end-point of the segment along the line defined by the
 * segment.
 *
 * The window is defined by `XMIN ≤ X ≤ XMAX`, `YMIN ≤ Y ≤ YMAX`.  This assumes
 * that the window limits are correctly sorted.
 *
 * Arguments `(X1,Y1)` and `(X2,Y2)` are the (unclipped) coordinates of the
 * segment, `(X1,Y1)` being the end-point to move inside the window.  `DX` and
 * `DY` should be respectively equal to `X2 - X1` and `Y2 - Y1` (or to `X1 -
 * X2` and `Y1 - Y2` as only the ratio of these two values is used, see below).
 *
 * Argument `T` is the floating-point type for computations.  All coordinates
 * are assumed to be of this type.
 *
 * Argument `REJECT` is the statement to execute if no valid intersection has
 * been found.
 *
 * Arguments `X1C` and `Y1C` must be L-values.  On return, if a valid
 * intersection has been found, `X1C` and `Y1C` are set with the coordinates of
 * the intersecting point.  If no valid intersection has been found, `X1C` and
 * `Y1C` are left unchanged.
 *
 * For a maximum of precision, the move is carried out relatively to the other
 * (unclipped) end-point.  The intersection `(X,Y)` is then such that:
 *
 *     (X - X2, Y - Y2)^(DX, DY) = 0
 *
 * where ^ denotes vector cross product.  Hence, the intersection is given
 * by one of the following:
 *
 *     X1C = X2 + (YLIM - Y2)*(DX/DY)
 *     Y1C = YLIM
 *
 * if `Y1` was outside the bound ylim, or:
 *
 *     X1C = XLIM
 *     Y1C = Y2 + (XLIM - X2)*(DY/DX)
 *
 * if `X1` was outside the bound xlim.
 */
#define MP_CLIP_INTERSECT(T, REJECT, X1C, Y1C, DX, DY, X1, Y1, X2, Y2,  \
                          XMIN, XMAX, YMIN, YMAX)                       \
    do {                                                                \
        T _x1 = (X1);                                                   \
        T _y1 = (Y1);                                                   \
        /* First impose the Y-limits. */                                \
        if (_y1 > (YMAX)) {                                             \
            _y1 = (YMAX);                                               \
            _x1 = (X2) + (_y1 - (Y2))*((DX)/(DY));                      \
        } else if (_y1 < (YMIN)) {                                      \
            _y1 = (YMIN);                                               \
            _x1 = (X2) + (_y1 - (Y2))*((DX)/(DY));                      \
        }                                                               \
        /* Then impose the X-limits. */                                 \
        if (_x1 > (XMAX)) {                                             \
            _x1 = (XMAX);                                               \
            _y1 = (Y2) + (_x1 - (X2))*((DY)/(DX));                      \
        } else if (_x1 < (XMIN)) {                                      \
            _x1 = (XMIN);                                               \
            _y1 = (Y2) + (_x1 - (X2))*((DY)/(DX));                      \
        }                                                               \
        /* Last move has left the point inside the window? */           \
        if (_y1 > (YMIN) || _y1 > (YMAX)) {                             \
            REJECT;                                                     \
        }                                                               \
        X1C = _x1;                                                      \
        Y1C = _y1;                                                      \
    } while (0)

#define _MP_INITIALIZE_CLIP(E,W,X,Y,B)                  \
    do {                                                \
        if (E(B,xmin) <= E(B,xmax)) {                   \
            E(W,xmin) = E(B,xmin);                      \
            E(W,xmax) = E(B,xmax);                      \
        } else {                                        \
            E(W,xmin) = E(B,xmax);                      \
            E(W,xmax) = E(B,xmin);                      \
        }                                               \
        if (E(B,ymin) <= E(B,ymax)) {                   \
            E(W,ymin) = E(B,ymin);                      \
            E(W,ymax) = E(B,ymax);                      \
        } else {                                        \
            E(W,ymin) = E(B,ymax);                      \
            E(W,ymax) = E(B,ymin);                      \
        }                                               \
        E(w,c2) = MP_CLIP_TBRL(X, Y,                    \
                               E(w,xmin), E(w,xmax),    \
                               E(w,ymin), E(w,ymax));   \
        E(w,x2) = (X);                                  \
        E(w,y2) = (Y);                                  \
    } while (0)

#define MP_INITIALIZE_CLIP(W,X,Y,B) \
    _MP_INITIALIZE_CLIP(MP_GET_FIELD,W,X,Y,B)
#define MP_INITIALIZE_CLIP_PTR(W,X,Y,B) \
    _MP_INITIALIZE_CLIP(MP_GET_FIELD_PTR,W,X,Y,B)

#define _MP_RESTART_CLIP(E,W,X,Y)                       \
    do {                                                \
        E(w,c2) = MP_CLIP_TBRL(X, Y,                    \
                               E(w,xmin), E(w,xmax),    \
                               E(w,ymin), E(w,ymax));   \
        E(w,x2) = (X);                                  \
        E(w,y2) = (Y);                                  \
    } while (0)

#define MP_RESTART_CLIP(W,X,Y) \
    _MP_RESTART_CLIP(MP_GET_FIELD,W,X,Y)
#define MP_RESTART_CLIP_PTR(W,X,Y) \
    _MP_RESTART_CLIP(MP_GET_FIELD_PTR,W,X,Y)

/**
 * @}
 */

/*---------------------------------------------------------------------------*/

/*
 * Structure to store an (R,G,B) color representation.  Colorant values are in
 * the range [0,1].  0 means dark, 1 means bright.  Values outside this range
 * are clippled to [0,1].
 */
typedef struct _MpColor MpColor;
struct _MpColor {
    MpReal red;
    MpReal green;
    MpReal blue;
};

/*
 * There are 10 predefined colors in µPlot.  Altough colors can be redefined by
 * the user, device drivers should start with these colors.
 */
#define MP_COLOR_BACKGROUND 0
#define MP_COLOR_FOREGROUND 1
#define MP_COLOR_RED        2
#define MP_COLOR_GREEN      3
#define MP_COLOR_BLUE       4
#define MP_COLOR_CYAN       5
#define MP_COLOR_MAGENTA    6
#define MP_COLOR_YELLOW     7
#define MP_COLOR_BLACK      8
#define MP_COLOR_WHITE      9

typedef enum {
    MP_SOLID_LINE              =  0,
    MP_DASHED_LINE             =  1,
    MP_DOTTED_LINE             =  2,
    MP_DASH_DOTTED_LINE        =  3,
    MP_DASH_DOUBLE_DOTTED_LINE =  4,
    MP_DASH_TRIPLE_DOTTED_LINE =  5,
} MpLineStyle;

/*
 * Opaque device structure.
 */
typedef struct _MpDevice MpDevice;

/*
 * Coordinate transform (must match opaque definition in <muPlotXForms.h>).
 */
typedef struct _MpAffineTransformDbl MpCoordinateTransform;

/**
 * Install a graphic driver.
 *
 * Call this routine to install a graphic driver.  Installed drivers may be
 * uninstalled by calling MpUninstallDriver() or MpUninstallAllDrivers().
 *
 * @param ident  The identifier of the graphic driver.

 * @param open   The method called to open a new device managed by this driver.
 *               This method takes two string arguments: the identifier of the
 *               driver and an additional argument.  This method shall return
 *               a standard status.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpInstallDriver(const char* ident,
                                MpStatus (*open)(MpDevice** dev,
                                                 const char* ident,
                                                 const char* arg));

extern MpStatus MpUninstallDriver(const char* ident);
extern MpStatus MpUninstallAllDrivers(void);
extern MpStatus MpListDrivers(MpInt* argc, char*** argv);
extern MpStatus MpFreeDriverList(char** argv);

extern MpStatus MpCheckPageSettings(MpDevice* dev);
extern MpStatus MpCheckMethods(MpDevice* dev);
extern MpDevice* MpAllocateDevice(size_t size);

/**
 * Open a new graphic device.
 *
 * Call this routine to open a new graphic device.  The caller is responsible
 * of eventually calling MpCloseDevice() to close the graphic device and free
 * all associated ressources.
 *
 * @param devptr    The address to store the new device structure.
 * @param ident     The identifier of the graphic device.
 * @param arg       Supplied argument like the filename.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpOpenDevice(MpDevice** devptr, const char* ident, const char* arg);

/**
 * Close graphic device.
 *
 * Call this routine to close a graphic device.  This releases all associated
 * ressources.  As a consequence, the device can no longer be used.
 *
 * @param devptr    The graphic device.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpCloseDevice(MpDevice** devptr);

/**
 * Set page size.
 *
 * @param dev     The graphic device.
 * @param width   The page width in millimeters.
 * @param height  The page height in millimeters.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpSetPageSize(MpDevice* dev, MpReal width, MpReal height);

/**
 * Get page size.
 *
 * @param dev     The graphic device.
 * @param width   The address to store the page width in millimeters.
 * @param height  The address to store the page height in millimeters.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpGetPageSize(MpDevice* dev, MpReal* width, MpReal* height);

/**
 * Set device resolution.
 *
 * @param dev    The graphic device.
 * @param xpmm   The number of horizontal samples per millimeter.
 * @param ypmm   The number of vertical samples per millimeter.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpSetResolution(MpDevice* dev, MpReal xpmm, MpReal ypmm);

/**
 * Get device resolution.
 *
 * @param dev    The graphic device.
 * @param xpmm   The address to store the number of horizontal samples per
 *               millimeter.
 * @param ypmm   The address to store number of vertical samples per
 *               millimeter.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpGetResolution(MpDevice* dev, MpReal* xpmm, MpReal* ypmm);

/**
 * Get the number of samples in a page.
 *
 * @param dev     The graphic device.
 * @param width   The address to store the number of horizontal samples.
 * @param height  The address to store number of vertical samples.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpGetNumberOfSamples(MpDevice* dev, MpPoint* width, MpPoint* height);

extern MpStatus MpDrawCellsHelper(MpDevice* dev, const MpColorIndex* z,
                                  MpInt n1, MpInt n2, MpInt stride,
                                  MpPoint x0, MpPoint y0, MpPoint x1, MpPoint y1);

extern MpStatus MpSetColorIndex(MpDevice* dev, MpColorIndex ci);
extern MpStatus MpGetColorIndex(MpDevice* dev, MpColorIndex* ci);

extern MpStatus MpSetColor(MpDevice* dev, MpColorIndex ci,
                           MpReal rd, MpReal gr, MpReal bl);
extern MpStatus MpGetColor(MpDevice* dev, MpColorIndex ci,
                           MpReal* rd, MpReal* gr, MpReal* bl);

extern MpStatus MpSetLineStyle(MpDevice* dev, MpLineStyle ls);
extern MpStatus MpGetLineStyle(MpDevice* dev, MpLineStyle* ls);

extern MpStatus MpSetLineWidth(MpDevice* dev, MpReal lw);
extern MpStatus MpGetLineWidth(MpDevice* dev, MpReal* lw);

/**
 * Define standard colors.
 *
 * This function defines the 10 standard colors in the primary colormap of a
 * given device.  This function should only be called at initialization time.
 *
 * @param dev     The graphic device.
 * @param dark    True to define a dark (i.e., black) background.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure
 * (e.g., `MP_BAD_ADDRESS` is the colormap is not allocated or `MP_BAD_SIZE` is
 * the size of the primary colormap is insufficient.
 */
extern MpStatus MpDefineStandardColors(MpDevice* dev, MpBool dark);

extern void MpEncodeColor(MpColor* dst, MpReal rd, MpReal gr, MpReal bl);

/**
 * Set the number of colors.
 *
 * This function attempts to change the number of colors in the primary and/or
 * secondary colormaps.
 *
 * Unsuccessful operation does not mean that nothing has been done: in case of
 * error, the user may call MpGetColormapSizes() to figure out the actual
 * number of colors that have been allocated.
 *
 * @param dev     The graphic device.
 *
 * @param n1      The number of colors for the primary colormap (for most
 *                devices this cannot be changed).
 *
 * @param n2      The number of colors for the secondary colormap.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpSetColormapSizes(MpDevice* dev, MpInt n1, MpInt n2);

extern MpStatus MpGetColormapSizes(MpDevice* dev, MpInt* n1, MpInt* n2);

_MP_END_DECLS

#endif /* _MUPLOT_H_ */
