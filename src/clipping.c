/*
 * clipping.c --
 *
 * Implementation of routines for clipping lines within boxes.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#ifndef _MUPLOT_CLIPPING_C
#define _MUPLOT_CLIPPING_C 1

#include "muPlot.h"

#define JOIN(a,b)     a##b
#define JOIN2(a,b)    JOIN(a,b)

#define T                     float
#define SFX                   Flt
#define BOX                   MpBoxFlt
#define MAPPING               MpMappingFlt
#define CLIP_STATE            MpClipStateFlt
#define CLIP_INIT             MpInitializeClipFlt
#define CLIP_RESTART          MpRestartClipFlt
#define CLIP_NEXT             MpClipNextFlt
#define CLIP_SEGMENT          MpClipSegmentFlt
#define CLIP_POLYLINE         MpClipPolylineFlt
#define CLIP_SEGMENTS         MpClipSegmentsFlt
#define DRAW_CLIPPED_SEGMENT  MpDrawClippedSegmentFlt
#define DRAW_CLIPPED_POLYLINE MpDrawClippedPolylineFlt
#define DRAW_CLIPPED_SEGMENTS MpDrawClippedSegmentsFlt
#include __FILE__

#define T                     double
#define SFX                   Dbl
#define BOX                   MpBoxDbl
#define MAPPING               MpMappingDbl
#define CLIP_STATE            MpClipStateDbl
#define CLIP_INIT             MpInitializeClipDbl
#define CLIP_RESTART          MpRestartClipDbl
#define CLIP_NEXT             MpClipNextDbl
#define CLIP_SEGMENT          MpClipSegmentDbl
#define CLIP_POLYLINE         MpClipPolylineDbl
#define CLIP_SEGMENTS         MpClipSegmentsDbl
#define DRAW_CLIPPED_SEGMENT  MpDrawClippedSegmentDbl
#define DRAW_CLIPPED_POLYLINE MpDrawClippedPolylineDbl
#define DRAW_CLIPPED_SEGMENTS MpDrawClippedSegmentsDbl
#include __FILE__

#else /* _MUPLOT_CLIPPING_C defined */

#ifdef CLIP_INIT
void
CLIP_INIT(CLIP_STATE* w, T x, T y, const BOX* box)
{
    MP_INITIALIZE_CLIP_PTR(w, x, y, box);
}
#endif /* CLIP_INIT */

#ifdef CLIP_RESTART
void
CLIP_RESTART(CLIP_STATE* w, T x, T y)
{
    MP_RESTART_CLIP_PTR(w, x, y);
}
#endif /* CLIP_INIT */

#ifdef CLIP_NEXT
int
CLIP_NEXT(CLIP_STATE* w, T x, T y)
{
    /* Make the former second point the first one. */
    w->c1 = w->c2;
    w->x1 = w->x2;
    w->y1 = w->y2;

    /* Register the coordinates of the new end-point. */
    w->c2 = MP_CLIP_TBRL(x, y, w->xmin, w->xmax, w->ymin, w->ymax);
    w->x2 = x;
    w->y2 = y;

    if (w->c1 == 0 && w->c2 == 0) {
        /* Accept unclipped segment. */
        return 1;
    } else if ((w->c1 & w->c2) == 0) {
        /* The segment is not trivially outside box, try to find
           intersections of the segment with box edges. */
        T dx = w->x2 - w->x1;
        T dy = w->y2 - w->y1;
        if (w->c1 == 0) {
            /* No needs to move first point. */
            w->x1c = w->x1;
            w->y1c = w->y1;
        } else {
            /* Try to move first point at box edges. */
            MP_CLIP_INTERSECT(T, goto reject, w->x1c, w->y1c, dx, dy,
                              w->x1, w->y1, w->x2, w->y2,
                              w->xmin, w->xmax, w->ymin, w->ymax);
        }
        if (w->c2 == 0) {
            /* No needs to move second point. */
            w->x2c = w->x2;
            w->y2c = w->y2;
        } else {
            /* Try to move second point at box edges. */
            MP_CLIP_INTERSECT(T, goto reject, w->x2c, w->y2c, dx, dy,
                              w->x2, w->y2, w->x1, w->y1,
                              w->xmin, w->xmax, w->ymin, w->ymax);
        }
        /* Accept the clipped segment. */
        return 2;
    }

    /* No interesection of the segment with box edges was
       found by moving this end-point of the segment. */
 reject:
    return 0;
}
#endif /* CLIP_NEXT */

#ifdef CLIP_SEGMENT
int
CLIP_SEGMENT(T* x1c, T* y1c, T* x2c, T* y2c, const BOX* box,
             T x1, T y1, T x2, T y2)
{
    unsigned c1, c2;
    T xmin, xmax, ymin, ymax;
    if (box->xmin <= box->xmax) {
        xmin = box->xmin;
        xmax = box->xmax;
    } else {
        xmin = box->xmax;
        xmax = box->xmin;
    }
    if (box->ymin <= box->ymax) {
        ymin = box->ymin;
        ymax = box->ymax;
    } else {
        ymin = box->ymax;
        ymax = box->ymin;
    }
    c1 = MP_CLIP_TBRL(x1, y1, xmin, xmax, ymin, ymax);
    c2 = MP_CLIP_TBRL(x2, y2, xmin, xmax, ymin, ymax);
    if (c1 == 0 && c2 == 0) {
        /* Accept unclipped segment. */
        *x1c = x1;
        *y1c = y1;
        *x2c = x2;
        *y2c = y2;
        return 1;
    } else if ((c1 & c2) == 0) {
        /* The segment is not trivially outside box, try to find
           intersections of the segment with box edges. */
        T dx = x2 - x1;
        T dy = y2 - y1;
        if (c1 == 0) {
            /* No needs to move first point. */
            *x1c = x1;
            *y1c = y1;
        } else {
            /* Try to move first point at box edges. */
            MP_CLIP_INTERSECT(T, goto reject, *x1c, *y1c, dx, dy,
                              x1, y1, x2, y2,
                              box->xmin, box->xmax,
                              box->ymin, box->ymax);
        }
        if (c2 == 0) {
            /* No needs to move second point. */
            *x2c = x2;
            *y2c = y2;
        } else {
            /* Try to move second point at box edges. */
            MP_CLIP_INTERSECT(T, goto reject, *x2c, *y2c, dx, dy,
                              x2, y2, x1, y1,
                              box->xmin, box->xmax,
                              box->ymin, box->ymax);
        }
        /* Accept the clipped segment. */
        return 2;
    }

    /* No interesection of the segment with box edges was
       found by moving this end-point of the segment. */
 reject:
    return 0;
}
#endif /* CLIP_SEGMENT */

#ifdef CLIP_POLYLINE
MpInt
CLIP_POLYLINE(T* xc, T* yc, const BOX* box,
              const T* x, const T* y, MpInt n)
{
    MpInt i, j = 0;
    if (n >= 2) {
        CLIP_STATE w;
        CLIP_INIT(&w, x[0], y[0], box);
        for (i = 1; i < n; ++i) {
            switch (CLIP_NEXT(&w, x[i], y[i])) {
            case 1:
                /* Append the unclipped segment. */
                xc[j]   = w.x1;
                yc[j]   = w.y1;
                xc[j+1] = w.x2;
                yc[j+1] = w.y2;
                j += 2;
                break;
            case 2:
                /* Append the clipped segment. */
                xc[j]   = w.x1c;
                yc[j]   = w.y1c;
                xc[j+1] = w.x2c;
                yc[j+1] = w.y2c;
                j += 2;
                break;
            }
        }
    }
    return (j >> 1);
}
#endif /* CLIP_POLYLINE */

#ifdef CLIP_SEGMENTS
MpInt
CLIP_SEGMENTS(T* xc, T* yc, const BOX* box,
              const T* x, const T* y, MpInt n)
{
    MpInt i, j = 0;
    T xmin, xmax, ymin, ymax;
    if (box->xmin <= box->xmax) {
        xmin = box->xmin;
        xmax = box->xmax;
    } else {
        xmin = box->xmax;
        xmax = box->xmin;
    }
    if (box->ymin <= box->ymax) {
        ymin = box->ymin;
        ymax = box->ymax;
    } else {
        ymin = box->ymax;
        ymax = box->ymin;
    }
    for (i = 1; i < n; ++i) {
        T x1 = x[2*i];
        T y1 = y[2*i];
        T x2 = x[2*i+1];
        T y2 = y[2*i+1];
        unsigned c1 = MP_CLIP_TBRL(x1, y1, xmin, xmax, ymin, ymax);
        unsigned c2 = MP_CLIP_TBRL(x2, y2, xmin, xmax, ymin, ymax);
        if (c1 == 0 && c2 == 0) {
            /* Accept unclipped segment. */
            xc[2*j]   = x1;
            yc[2*j]   = y1;
            xc[2*j+1] = x2;
            yc[2*j+1] = y2;
            ++j;
        } else if ((c1 & c2) == 0) {
            /* The segment is not trivially outside box, try to find
               intersections of the segment with box edges. */
            T dx = x2 - x1;
            T dy = y2 - y1;
            if (c1 == 0) {
                /* No needs to move first point. */
                xc[2*j] = x1;
                yc[2*j] = y1;
            } else {
                /* Try to move first point at box edges. */
                MP_CLIP_INTERSECT(T, goto reject, xc[2*j], yc[2*j], dx, dy,
                                  x1, y1, x2, y2, xmin, xmax, ymin, ymax);
            }
            if (c2 == 0) {
                /* No needs to move second point. */
                xc[2*j+1] = x2;
                yc[2*j+1] = y2;
            } else {
                /* Try to move second point at box edges. */
                MP_CLIP_INTERSECT(T, goto reject, xc[2*j+1], yc[2*j+1], dx, dy,
                                  x2, y2, x1, y1, xmin, xmax, ymin, ymax);
            }
            /* Accept the clipped segment. */
            ++j;
        }
    reject:
        continue;
    }
    return j;
}
#endif /* CLIP_SEGMENTS */

#ifdef DRAW_CLIPPED_SEGMENT
MpStatus
DRAW_CLIPPED_SEGMENT(void* ctx,
                     MpStatus (*move)(void* ctx, T x, T y),
                     MpStatus (*draw)(void* ctx, T x, T y),
                     const BOX* box, T x1, T y1, T x2, T y2)
{
    T x1c = x1, y1c = y1, x2c = x2, y2c = y2;
    T xmin, xmax, ymin, ymax;
    MpStatus status = MP_OK;
    unsigned c1, c2;
    if (box->xmin <= box->xmax) {
        xmin = box->xmin;
        xmax = box->xmax;
    } else {
        xmin = box->xmax;
        xmax = box->xmin;
    }
    if (box->ymin <= box->ymax) {
        ymin = box->ymin;
        ymax = box->ymax;
    } else {
        ymin = box->ymax;
        ymax = box->ymin;
    }
    c1 = MP_CLIP_TBRL(x1, y1, xmin, xmax, ymin, ymax);
    c2 = MP_CLIP_TBRL(x2, y2, xmin, xmax, ymin, ymax);
    if (c1 == 0 && c2 == 0) {
        /* Draw Segment. */
    accept:
        status = move(ctx, x1c, y1c);
        if (status == MP_OK) {
            status = draw(ctx, x2c, y2c);
        }
    } else if ((c1 & c2) == 0) {
        /* The segment is not trivially outside box, try to find
           intersections of the segment with box edges. */
        T dx = x2 - x1;
        T dy = y2 - y1;
        if (c1 != 0) {
            /* Try to move first point at box edges. */
            MP_CLIP_INTERSECT(T, goto reject, x1c, y1c, dx, dy,
                              x1, y1, x2, y2, xmin, xmax, ymin, ymax);
        }
        if (c2 != 0) {
            /* Try to move second point at box edges. */
           MP_CLIP_INTERSECT(T, goto reject, x2c, y2c, dx, dy,
                             x2, y2, x1, y1, xmin, xmax, ymin, ymax);
        }
        goto accept;
    }
 reject:
    return status;
}
#endif /* DRAW_CLIPPED_SEGMENT */

#ifdef DRAW_CLIPPED_POLYLINE
MpStatus
DRAW_CLIPPED_POLYLINE(void* ctx,
                      MpStatus (*move)(void* ctx, T x, T y),
                      MpStatus (*draw)(void* ctx, T x, T y),
                      const BOX* box,
                      const T* x, const T* y, MpInt n)
{
    MpStatus status = MP_OK;
    if (n >= 2) {
        MpInt i;
        CLIP_STATE w;
        bool flag = true;
        T x2p = 0, y2p = 0; /* Coordinates of previous second end-point. */
        CLIP_INIT(&w, x[0], y[0], box);
        for (i = 1; i < n; ++i) {
            int state = CLIP_NEXT(&w, x[i], y[i]);
            if (state != 0) {
                T x1, y1, x2, y2;
                if (state == 1) {
                    x1 = w.x1;
                    y1 = w.y1;
                    x2 = w.x2;
                    y2 = w.y2;
                } else {
                    x1 = w.x1c;
                    y1 = w.y1c;
                    x2 = w.x2c;
                    y2 = w.y2c;
                }
                if (flag || x1 != x2p || y1 != y2p) {
                    status = move(ctx, x1, y1);
                    if (status != MP_OK) {
                        return status;
                    }
                }
                status = draw(ctx, x2, y2);
                if (status != MP_OK) {
                    return status;
                }
                flag = false;
                x2p = x2;
                y2p = y2;
            }
        }
    }
    return status;
}
#endif /* DRAW_CLIPPED_POLYLINE */

#ifdef DRAW_CLIPPED_SEGMENTS
MpStatus
DRAW_CLIPPED_SEGMENTS(void* ctx,
                      MpStatus (*move)(void* ctx, T x, T y),
                      MpStatus (*draw)(void* ctx, T x, T y),
                      const BOX* box,
                      const T* x, const T* y, MpInt n)
{
    MpStatus status = MP_OK;
    MpInt i;
    T x1c = 0, y1c = 0, x2c = 0, y2c = 0;
    T xmin, xmax, ymin, ymax;
    if (box->xmin <= box->xmax) {
        xmin = box->xmin;
        xmax = box->xmax;
    } else {
        xmin = box->xmax;
        xmax = box->xmin;
    }
    if (box->ymin <= box->ymax) {
        ymin = box->ymin;
        ymax = box->ymax;
    } else {
        ymin = box->ymax;
        ymax = box->ymin;
    }
    for (i = 1; i < n; ++i) {
        T x1 = x[2*i];
        T y1 = y[2*i];
        T x2 = x[2*i+1];
        T y2 = y[2*i+1];
        unsigned c1 = MP_CLIP_TBRL(x1, y1, xmin, xmax, ymin, ymax);
        unsigned c2 = MP_CLIP_TBRL(x2, y2, xmin, xmax, ymin, ymax);
        if (c1 == 0 && c2 == 0) {
            /* Accept unclipped segment. */
            x1c = x1;
            y1c = y1;
            x2c = x2;
            y2c = y2;
        } else if ((c1 & c2) != 0) {
            /* The segment is trivially outside box. */
        reject:
            continue;
        } else if ((c1 & c2) == 0) {
            /* The segment is not trivially outside box, try to find
               intersections of the segment with box edges. */
            T dx = x2 - x1;
            T dy = y2 - y1;
            if (c1 == 0) {
                /* No needs to move first point. */
                x1c = x1;
                y1c = y1;
            } else {
                /* Try to move first point at box edges. */
                MP_CLIP_INTERSECT(T, goto reject, x1c, y1c, dx, dy,
                                  x1, y1, x2, y2, xmin, xmax, ymin, ymax);
            }
            if (c2 == 0) {
                /* No needs to move second point. */
                x2c = x2;
                y2c = y2;
            } else {
                /* Try to move second point at box edges. */
                MP_CLIP_INTERSECT(T, goto reject, x2c, y2c, dx, dy,
                                  x2, y2, x1, y1, xmin, xmax, ymin, ymax);
            }
        }
        status = move(ctx, x1c, y1c);
        if (status != MP_OK) {
            break;
        }
        status = draw(ctx, x2c, y2c);
        if (status != MP_OK) {
            break;
        }
    }
    return status;
}
#endif /* DRAW_CLIPPED_SEGMENTS */

#undef T
#undef SFX
#undef BOX
#undef MAPPING
#undef CLIP_STATE
#undef CLIP_INIT
#undef CLIP_RESTART
#undef CLIP_NEXT
#undef CLIP_SEGMENT
#undef CLIP_POLYLINE
#undef CLIP_SEGMENTS
#undef DRAW_CLIPPED_SEGMENT
#undef DRAW_CLIPPED_POLYLINE
#undef DRAW_CLIPPED_SEGMENTS

#endif /* _MUPLOT_CLIPPING_C */
