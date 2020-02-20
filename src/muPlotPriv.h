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
    MpColorIndex      colormapSize1; /* Number of colors in the primary colormap */
    MpColorIndex      colormapSize2; /* Number of colors in the secondary colormap */
    MpColorIndex       colormapSize; /* Total number of colors in color table */
    MpColor*               colormap; /* Colormap (freed automatically on close
                                        if non-NULL */


    /* Methods can assume checked arguments.
     *
     * Note: These methods may assume that all arguments have been checked.
     *
     * - initialize() is called after the device has been open by the driver
     *   and the device structure allocated (with its colormap).  This method
     *   is the oportunity for the device to set initial settings (like the
     *   colors, etc.).  This method is never called again.
     */
    MpStatus (*initialize)(MpDevice* dev);
    /*
     * - finalize() is called to close the device and free associated
     *   ressources (resources in the public part of the device like the
     *   colormap are however automacally destroyed if not yet done on return
     *   of this method).  This method is never called again.
     */
    MpStatus (*finalize)(MpDevice* dev);
    /*
     * - setPageSize() is called to set the page size in millimeters.
     */
    MpStatus (*setPageSize)(MpDevice* dev, MpReal w, MpReal h);
    /*
     * - setResolution() is called to set the horizontal and vertical
     *   resolution is number of samples per millimeter.
     */
    MpStatus (*setResolution)(MpDevice* dev, MpReal xpmm, MpReal ypmm);
    /*
     * - startBuffering() is called to start buffering graphical output.
     *   Buffering may be useful to speed-up drawing operations or to make sure
     *   that a graphic is complete before showing it.
     */
    MpStatus (*startBuffering)(MpDevice* dev);
    /*
     * - stopBuffering() is called to stop buffering graphical output.
     */
    MpStatus (*stopBuffering)(MpDevice* dev);
    /*
     * - beginPage() is called to begin a new page of graphics.
     */
    MpStatus (*beginPage)(MpDevice* dev);
    /*
     * - endPage() is called to end the current page of graphics.
     */
    MpStatus (*endPage)(MpDevice* dev);
    /*
     * - setColormapSizes() is called to set the number of colors in the
     *   colormaps.
     */
    MpStatus (*setColormapSizes)(MpDevice* dev, MpInt n1, MpInt n2);
    /*
     * - setColorIndex() is called to set the current color index.
     */
    MpStatus (*setColorIndex)(MpDevice* dev, MpColorIndex ci);
    /*
     * - setColor() is called to define a given color.
     */
    MpStatus (*setColor)(MpDevice* dev, MpColorIndex ci,
                         MpReal rd, MpReal gr, MpReal bl);
    /*
     * - setLineStyle() is called to set the current line style.
     */
    MpStatus (*setLineStyle)(MpDevice* dev, MpLineStyle ls);
    /*
     * - setLineWidth() is called to set the current line width.
     */
    MpStatus (*setLineWidth)(MpDevice* dev, MpReal lw);
    /*
     * - drawPoint() is called to draw a point using the current settings.
     */
    MpStatus (*drawPoint)(MpDevice* dev, MpPoint x, MpPoint y);
    /*
     * - drawRectangle() is called to draw a (filled) rectangle using the
     *   current settings.
     */
    MpStatus (*drawRectangle)(MpDevice* dev,
                              MpPoint x0, MpPoint y0, MpPoint x1, MpPoint y1);
    /*
     * - drawPolyline() is called to draw an open polyline using the current
     *   settings.
     */
    MpStatus (*drawPolyline)(MpDevice* dev,
                             const MpPoint* x, const MpPoint* y, MpInt n);
    /*
     * - drawPolygon() is called to draw a closed polygon using the current
     *   settings.
     */
    MpStatus (*drawPolygon)(MpDevice* dev,
                            const MpPoint* x, const MpPoint* y, MpInt n);
    /*
     * - drawCells() is called to draw colored cells using the current
     *   settings.
     */
    MpStatus (*drawCells)(MpDevice* dev,
                          const MpColorIndex* z, MpInt n1, MpInt n2, MpInt stride,
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
