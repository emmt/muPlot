/*
 * mappings.c --
 *
 * Implementation of simple coordinate mappings and rectangular boxes.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#ifndef _MUPLOT_MAPPINGS_C
#define _MUPLOT_MAPPINGS_C 1

#include "muPlot.h"

#define T                   float
#define SFX                 Flt
#define BOX                 MpBoxFlt
#define MAPPING             MpMappingFlt
#define IS_EMPTY_BOX        MpIsEmptyBoxFlt
#define CHECK_BOX           MpCheckBoxFlt
#define REORDER_BOX_LIMITS  MpReorderBoxLimitsFlt
#define CHECK_MAPPING       MpCheckMappingFlt
#define DEFINE_MAPPING      MpDefineMappingFlt
#define COMPOSE_MAPPINGS    MpComposeMappingsFlt
#define INVERT_MAPPING      MpInvertMappingFlt
#include __FILE__

#define T                   double
#define SFX                 Dbl
#define BOX                 MpBoxDbl
#define MAPPING             MpMappingDbl
#define IS_EMPTY_BOX        MpIsEmptyBoxDbl
#define CHECK_BOX           MpCheckBoxDbl
#define REORDER_BOX_LIMITS  MpReorderBoxLimitsDbl
#define CHECK_MAPPING       MpCheckMappingDbl
#define DEFINE_MAPPING      MpDefineMappingDbl
#define COMPOSE_MAPPINGS    MpComposeMappingsDbl
#define INVERT_MAPPING      MpInvertMappingDbl
#include __FILE__

#else /* _MUPLOT_MAPPINGS_C defined */

#ifdef IS_EMPTY_BOX
MpBool
IS_EMPTY_BOX(const BOX* box)
{
    return (MP_IS_EMPTY_BOX_PTR(box) ? true : false);
}
#endif /* IS_EMPTY_BOX */

#ifdef CHECK_BOX
MpStatus
CHECK_BOX(const BOX* box)
{
    return MP_CHECK_BOX_PTR(box);
}
#endif /* CHECK_BOX */

#ifdef REORDER_BOX_LIMITS
MpStatus
REORDER_BOX_LIMITS(BOX* dst, const BOX* src)
{
    MpStatus status = MP_CHECK_BOX_PTR(src);
    if (status == MP_OK) {
        if (dst == src) {
            /* In-place operation. */
            if (dst->xmin > dst->xmax) {
                T xtmp = dst->xmin;
                dst->xmin = dst->xmax;
                dst->xmax = xtmp;
            }
            if (dst->ymin > dst->ymax) {
                T ytmp = dst->ymin;
                dst->ymin = dst->ymax;
                dst->ymax = ytmp;
            }
        } else {
            /* Out-of-place operation. */
            if (src->xmin <= src->xmax) {
                dst->xmin = src->xmin;
                dst->xmax = src->xmax;
            } else {
                dst->xmin = src->xmax;
                dst->xmax = src->xmin;
            }
            if (src->ymin <= src->ymax) {
                dst->ymin = src->ymin;
                dst->ymax = src->ymax;
            } else {
                dst->ymin = src->ymax;
                dst->ymax = src->ymin;
            }
        }
    }
    return status;
}
#endif /* REORDER_BOX_LIMITS */

#ifdef CHECK_MAPPING
MpStatus
CHECK_MAPPING(const MAPPING* map)
{
    return MP_CHECK_MAPPING_PTR(map);
}
#endif /* CHECK_MAPPING */

#ifdef DEFINE_MAPPING
MpStatus
DEFINE_MAPPING(MAPPING* dst, const BOX* inp, const BOX* out, unsigned flip)
{
    T out_xmin, out_xmax, out_ymin, out_ymax;
    T dx = inp->xmax - inp->xmin;
    T dy = inp->ymax - inp->ymin;
    if (dx == 0 || dy == 0) {
        return MP_SINGULAR;
    }
    if ((flip & MP_FLIP_X) == 0) {
        out_xmin = out->xmin;
        out_xmax = out->xmax;
    } else {
        out_xmin = out->xmin;
        out_xmax = out->xmax;
    }
    dst->xx = (out_xmax - out_xmin)/dx;
    dst->x = (inp->xmax*out_xmin - inp->xmin*out_xmax)/dx;
    if ((flip & MP_FLIP_Y) == 0) {
        out_ymin = out->ymin;
        out_ymax = out->ymax;
    } else {
        out_ymin = out->ymin;
        out_ymax = out->ymax;
    }
    dst->yy = (out_ymax - out_ymin)/dy;
    dst->y = (inp->ymax*out_ymin - inp->ymin*out_ymax)/dy;
    return CHECK_MAPPING(dst);
}
#endif /* DEFINE_MAPPING */

#ifdef COMPOSE_MAPPINGS
MpStatus
COMPOSE_MAPPINGS(MAPPING* dst,
                     const MAPPING* A,
                     const MAPPING* B)
{
    /* Order of operations is important to allow for
       in-place application. */
    dst->x = A->xx*B->x + A->x;
    dst->xx = A->xx*B->xx;
    dst->y = A->yy*B->y + A->y;
    dst->yy = A->yy*B->yy;
    return CHECK_MAPPING(dst);
}
#endif /* COMPOSE_MAPPINGS */

#ifdef INVERT_MAPPING
MpStatus
INVERT_MAPPING(MAPPING* dst, const MAPPING* A)
{
    if (MP_IS_NAN(A->xx) || A->xx == 0 ||
        MP_IS_NAN(A->yy) || A->yy == 0) {
        return MP_SINGULAR;
    }
    /* Order of operations is important to allow for
       in-place application. */
    dst->x = -A->x/A->xx;
    dst->xx = 1/A->xx;
    dst->y = -A->y/A->yy;
    dst->yy = 1/A->yy;
    return CHECK_MAPPING(dst);
}
#endif /* INVERT_MAPPING */

#undef T
#undef SFX
#undef BOX
#undef MAPPING
#undef IS_EMPTY_BOX
#undef CHECK_BOX
#undef REORDER_BOX_LIMITS
#undef CHECK_MAPPING
#undef DEFINE_MAPPING
#undef COMPOSE_MAPPINGS
#undef INVERT_MAPPING

#endif /* _MUPLOT_MAPPINGS_C */
