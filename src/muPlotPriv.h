/*
 * muPlotPriv.h --
 *
 * Private definitions for the µPlot plotting library.  In partular, this file
 * defines the structures that are *opaque* at the user-level.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#ifndef _MUPLOT_PRIVATE_H_
#define _MUPLOT_PRIVATE_H_ 1

#include <muPlot.h>
#include <muPlotXForms.h>

_MP_BEGIN_DECLS

struct _MpDevice {
    /* The following members are set once and must never be changed. */
    const char* driver; /* Driver name */

    /* The following members may be managed by the high-level interface. */
    MpInt groupLevel; /* Index of group (for grouping elements) */
    MpInt pageNumber; /* Index of group (for grouping elements) */

    /* The following members are considered as read-only by the high-level
       interface (after initialization). */
    MpReal            pageWidth; /* Page width in millimeters */
    MpReal           pageHeight; /* Page height in millimeters */
    MpReal horizontalResolution; /* Number of horizontal samples per millimeter.
                                    non-zero but may be < 0 if flipped. */
    MpReal   verticalResolution; /* Number of vertical samples per millimeter.
                                    non-zero but may be < 0 if flipped. */
    MpInt     horizontalSamples; /* Number of horizontal samples (always > 0). */
    MpInt       verticalSamples; /* Number of vertical samples (always > 0). */

    /* Current (cached) plotting settings. */
    MpColorIndex         colorIndex;
    MpLineStyle           lineStyle;
    MpReal                lineWidth;
    MpCoordinateTransform dataToNDC; /* data to NDC coordinate transform */
    MpColorIndex      colormapSize0; /* Number of colors in the colormap 0 */
    MpColorIndex      colormapSize1; /* Number of colors in the colormap 1 */
    MpColorIndex       colormapSize; /* Total number of colors */


    /* Methods can assume checked arguments. */
    MpStatus (*close)(MpDevice* dev);
    MpStatus (*select)(MpDevice* dev);
    MpStatus (*setPageSize)(MpDevice* dev, MpReal w, MpReal h);
    MpStatus (*setResolution)(MpDevice* dev, MpReal xpmm, MpReal ypmm);
    MpStatus (*startBuffering)(MpDevice* dev);
    MpStatus (*stopBuffering)(MpDevice* dev);
    MpStatus (*beginPage)(MpDevice* dev);
    MpStatus (*endPage)(MpDevice* dev);
    MpStatus (*setColorIndex)(MpDevice* dev, MpColorIndex ci);
    MpStatus (*getColorIndex)(MpDevice* dev, MpColorIndex* ci);
    MpStatus (*setColor)(MpDevice* dev, MpColorIndex ci, MpReal rd, MpReal gr, MpReal bl);
    MpStatus (*getColor)(MpDevice* dev, MpColorIndex ci, MpReal* rd, MpReal* gr, MpReal* bl);
    MpStatus (*setLineWidth)(MpDevice* dev, MpReal lw);
    MpStatus (*getLineWidth)(MpDevice* dev, MpReal* lw);
    MpStatus (*drawPoint)(MpDevice* dev, MpPoint x, MpPoint y);
    MpStatus (*drawRectangle)(MpDevice* dev, MpPoint x0, MpPoint y0, MpPoint x1, MpPoint y1);
    MpStatus (*drawPolyline)(MpDevice* dev, const MpPoint* x, const MpPoint* y, MpInt n);
    MpStatus (*drawPolygon)(MpDevice* dev, const MpPoint* x, const MpPoint* y, MpInt n);
    MpStatus (*drawCells)(MpDevice* dev, const MpColorIndex* z,
                          MpInt n1, MpInt n2, MpInt stride,
                          MpPoint x0, MpPoint y0, MpPoint x1, MpPoint y1);
};

/*
  calll `dev->select(dev)` when device becomes active
  setPageSize may be NULL
  getPageSize yields page size in millimeters

  drawRectangle dwar rectange (x0,y0,x1,y1) including (x0,y0) but not (x1,y1) to allow for drawing
  adjacent rectangles without recovering.
  Alternatives:
  1. device can give size of point (in each direction),
  2. device drawing routines all take integer coordinates, of type say MpPoint, and high level takes
  care of converting.

  drawCells can be set to MpDrawCells to call drawRectangle for each cell. */

_MP_END_DECLS

#endif /* _MUPLOT_PRIVATE_H_ */
