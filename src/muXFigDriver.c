/*
 * muXFigDriver.c --
 *
 * Implementation of the XFig driver for µPlot.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include "muPlotPriv.h"

#define true (!0)
#define false (!true)

#define MILLIMETERS_PER_INCH 25.4
#define MP_A4_PAPER_WIDTH     210
#define MP_A4_PAPER_HEIGHT    297

#define MP_FLIP_X   (1 << 0)
#define MP_FLIP_Y   (1 << 1)

typedef struct _MpColor MpColor;
struct _MpColor {
    MpReal red;
    MpReal green;
    MpReal blue;
};

typedef enum {
    XFIG_COLOR_OBJECT_TYPE    = 0,
    XFIG_ELLIPSE_OBJECT_TYPE  = 1,
    XFIG_POLYLINE_OBJECT_TYPE = 2,
    XFIG_SPLINE_OBJECT_TYPE   = 3,
    XFIG_TEXT_OBJECT_TYPE     = 4,
    XFIG_ARC_OBJECT_TYPE      = 5,
    XFIG_COMPOUND_OBJECT_TYPE = 6,
} XFigObjectType;

/* Sub-types for type 2 objects (polyline). */
typedef enum {
    XFIG_POLYLINE_SUBTYPE = 1,
    XFIG_BOX_SUBTYPE      = 2,
    XFIG_POLYGON_SUBTYPE  = 3,
    XFIG_ARCBOX_SUBTYPE   = 4,
    XFIG_PICTURE_SUBTYPE  = 5, // imported-picture bounding-box
} XFigPolylineSubtype;

typedef enum {
    XFIG_DEFAULT_LINE_STYLE            = -1,
    XFIG_SOLID_LINE_STYLE              =  0,
    XFIG_DASHED_LINE_STYLE             =  1,
    XFIG_DOTTED_LINE_STYLE             =  2,
    XFIG_DASH_DOTTED_LINE_STYLE        =  3,
    XFIG_DASH_DOUBLE_DOTTED_LINE_STYLE =  4,
    XFIG_DASH_TRIPLE_DOTTED_LINE_STYLE =  5,
} XFigLineStyle;

typedef enum {
    XFIG_MITER_JOIN_STYLE = 0,
    XFIG_ROUND_JOIN_STYLE = 1,
    XFIG_BEVEL_JOIN_STYLE = 2,
} XFigJoinStyle;

typedef enum {
    XFIG_BUTT_CAP_STYLE       = 0,
    XFIG_ROUND_CAP_STYLE      = 1,
    XFIG_PROJECTING_CAP_STYLE = 2,
} XFigCapStyle;

typedef enum {
    XFIG_STICK_ARROW_TYPE         = 0,
    XFIG_STRAIGHT_BUTT_ARROW_TYPE = 1,
    XFIG_INDENTED_BUTT_ARROW_TYPE = 2,
    XFIG_POINTED_BUTT_ARROW_TYPE  = 3,
} XFigArrowType;

typedef enum {
    XFIG_HOLLOW_ARROW_STYLE = 0,
    XFIG_FILLED_ARROW_TYPE  = 1,
} XFigArrowStyle;


#define XFIG_C0MIN  -1
#define XFIG_C0MAX  31 /* Gold */
#define XFIG_C1MIN  32
#define XFIG_C1MAX 543
#define XFIG_CMIN    0
#define XFIG_CMAX  XFIG_C1MAX
#define XFIG_NCOLORS (XFIG_CMAX + 1 - XFIG_CMIN)

typedef struct _XFigDevice XFigDevice;

struct _XFigDevice {
    MpDevice base;
    MpColor colors[XFIG_NCOLORS];
    int stage; /* Initially 0, becomes 1 after the first graphic object has
                  been written; user defined colors can only be written at
                  stage = 0. */

    XFigLineStyle lineStyle;
    int lineWidth;
    MpColorIndex colorIndex; // FIXME: In base?
    MpColorIndex fillColor;
    int penStyle; // pen style, not used
    int areaFill; // enumeration
    double styleVal; // 1/80 inch, specification for dash/dotted lines
    XFigJoinStyle        joinStyle;   // enumeration type)
    XFigCapStyle       capStyle;    // enumeration type, only used for POLYLINE
    int           radius;        // 1/80 inch, radius of arc-boxes)
    int     forwardArrow; // forwardArrow (0: off, 1: on)
    int    backwardArrow; // backwardArrow (0: off, 1: on)
    int pictureOrientation;
    const char* pictureName;
    FILE* file;
};

static MpStatus
closeXFigDevice(MpDevice* dev)
{
    MpStatus status = MP_OK;
    XFigDevice* xfig = (XFigDevice*)dev;
    if (xfig->file != NULL) {
        if (fclose(xfig->file) != 0 && status == MP_OK) {
            status = MpSystemError();
        }
        xfig->file = NULL;
    }
    return status;
}

static MpStatus
selectXFigDevice(MpDevice* dev)
{
    return MP_OK;
}

static MpStatus getXFigColorRanges(MpDevice* dev,
                                   MpColorIndex* c0min, MpColorIndex* c0max,
                                   MpColorIndex* c1min, MpColorIndex* c1max)
{
    *c0min = XFIG_C0MIN; /* Default color read-only */
    *c0max = XFIG_C0MAX;
    *c1min = XFIG_C1MIN;
    *c1max = XFIG_C1MAX;
    return MP_OK;
}

static MpStatus setXFigColorIndex(MpDevice* dev, MpColorIndex ci)
{
    MpStatus status = MP_OK;
    XFigDevice* xfig = (XFigDevice*)dev;
    if (ci < 0) {
        /* Use default color. */
        ci = -1;
    }
    if (ci >= XFIG_NCOLORS) {
        ci = XFIG_NCOLORS - 1;
    }
    xfig->colorIndex = ci;
    return status;
}

static MpStatus getXFigColorIndex(MpDevice* dev, MpColorIndex* ci)
{
    MpStatus status = MP_OK;
    XFigDevice* xfig = (XFigDevice*)dev;
    *ci = xfig->colorIndex;
    return status;
}

static MpStatus setXFigColor(MpDevice* dev, MpColorIndex ci, MpReal rd, MpReal gr, MpReal bl)
{
    XFigDevice* xfig = (XFigDevice*)dev;
    if (ci < XFIG_C0MIN || ci > XFIG_CMAX) {
        return MP_OUT_OF_RANGE;
    }
    if (xfig->stage > 0 || ci < XFIG_C1MIN) {
        return MP_READ_ONLY;
    }
    /* Note: Color levels have already been clamped. */
    xfig->colors[ci].red   = rd;
    xfig->colors[ci].green = gr;
    xfig->colors[ci].blue  = bl;
    return MP_OK;
}

static MpStatus getXFigColor(MpDevice* dev, MpColorIndex ci, MpReal* rd, MpReal* gr, MpReal* bl)
{
    XFigDevice* xfig = (XFigDevice*)dev;
    if (ci < XFIG_C0MIN || ci > XFIG_CMAX) {
        return MP_OUT_OF_RANGE;
    }
    if (ci < 0) {
        /* Assume default color is black. */
        *rd = 0;
        *gr = 0;
        *bl = 0;
    } else {
        *rd = xfig->colors[ci].red;
        *gr = xfig->colors[ci].green;
        *bl = xfig->colors[ci].blue;
    }
    return MP_OK;
}

static MpColor
MpEncodeColor(MpReal red, MpReal green, MpReal blue)
{
    MpColor color; // = {.red = red, .green = green, .blue = blue};
    color.red   = red;
    color.green = green;
    color.blue  = blue;
    return color;
}

#define COLOR(r,g,b) MpEncodeColor((MpReal)(r)/(MpReal)255,   \
                                   (MpReal)(g)/(MpReal)255,   \
                                   (MpReal)(b)/(MpReal)255)

static unsigned
colorant(MpReal val)
{
    return (val <= (MpReal)0 ? (unsigned)0 :
            (val >= (MpReal)255 ? (unsigned)255 :
             (MP_IS_SINGLE_PRECISION(val) ?
              (unsigned)roundf((float)val*(float)255) :
              (unsigned)round((double)val*(double)255))));
}

static void
bumpXFigStage(XFigDevice* xfig)
{
    if (xfig->stage == 0) {
        /* Write definitions of arbitrary user defined colors beyond the 32
           standard colors.  The color objects must be defined before any other
           Fig objects. */
        for (int ci = XFIG_C1MIN; ci <= XFIG_C1MAX; ++ci) {
            fprintf(xfig->file, "%d %d #%02x%02x%02x\n",
                    XFIG_COLOR_OBJECT_TYPE, ci,
                    colorant(xfig->colors[ci].red),
                    colorant(xfig->colors[ci].green),
                    colorant(xfig->colors[ci].blue));
        }
        xfig->stage = 1;
    }
}

static void
setXFigInitialColors(XFigDevice* xfig)
{
    xfig->colors[0] = MpEncodeColor(0,0,0); /* 0 = Black */
    xfig->colors[1] = MpEncodeColor(0,0,1); /* 1 = Blue */
    xfig->colors[2] = MpEncodeColor(0,1,0); /* 2 = Green */
    xfig->colors[3] = MpEncodeColor(0,1,1); /* 3 = Cyan */
    xfig->colors[4] = MpEncodeColor(1,0,0); /* 4 = Red */
    xfig->colors[5] = MpEncodeColor(1,0,1); /* 5 = Magenta */
    xfig->colors[6] = MpEncodeColor(1,1,0); /* 6 = Yellow */
    xfig->colors[7] = MpEncodeColor(1,1,1); /* 7 = White */
    /*    8-11 = four shades of blue (dark to lighter) */
    /*   12-14 = three shades of green (dark to lighter) */
    /*   15-17 = three shades of cyan (dark to lighter) */
    /*   18-20 = three shades of red (dark to lighter) */
    /*   21-23 = three shades of magenta (dark to lighter) */
    /*   24-26 = three shades of brown (dark to lighter) */
    /*   27-30 = four shades of pink (dark to lighter) */
    xfig->colors[31] = COLOR(255, 215, 0); /* 31 = Gold */

    /* Initialize colormap CMAP1 with a ramp of grays. */
    MpReal a = (MpReal)1/(MpReal)(XFIG_C1MAX - XFIG_C1MIN);
    for (MpInt ci = XFIG_C1MIN; ci <= XFIG_C1MAX; ++ci) {
        MpReal g = (ci - XFIG_C1MIN)*a;
        xfig->colors[ci] = MpEncodeColor(g,g,g);
    }
}

static MpStatus
drawXFigPolylineObject(XFigDevice* xfig, int subType, int depth,
                       const MpPoint* x, const MpPoint* y, MpInt n,
                       MpBool closed)
{
    if (n < 1) {
        return MP_OK;
    }
    if (depth < 0) {
        depth = 0;
    } else if (depth > 999) {
        depth = 999;
    }
    bumpXFigStage(xfig);
    fprintf(xfig->file, "2 %d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %ld\n",
            XFIG_POLYLINE_OBJECT_TYPE,
            subType,
            xfig->lineStyle,
            xfig->lineWidth,
            (int)xfig->colorIndex,
            (int)xfig->fillColor,
            depth,
            xfig->penStyle, // pen style, not used
            xfig->areaFill, // enumeration
            xfig->styleVal, // 1/80 inch, specification for dash/dotted lines
            xfig->joinStyle,   // enumeration type)
            xfig->capStyle,    // enumeration type, only used for POLYLINE
            xfig->radius,        // 1/80 inch, radius of arc-boxes)
            xfig->forwardArrow, // forwardArrow (0: off, 1: on)
            xfig->backwardArrow, // backwardArrow (0: off, 1: on)
            (long)(closed ? n + 1 : n)); // number of points
    if (xfig->forwardArrow) {
        /* Write forward-arraow specifications. */
    }
    if (xfig->backwardArrow) {
        /* Write backward-arraow specifications. */
    }
    if (subType == XFIG_PICTURE_SUBTYPE) {
        /* Write linked picture specifications. */
        fprintf(xfig->file, "%d\n%s\n",
                xfig->pictureOrientation, // orientation = normal (0) or flipped (1)
                xfig->pictureName); // name of picture file to import
    }
    for (MpInt i = 0; i < n; ++i) {
        fprintf(xfig->file, "%s%d %d",
                ((i%6) == 0 ? (i == 0 ? "        " : "\n        ") : " "),
                (int)x[i], (int)y[i]);
    }
    if (closed) {
        fprintf(xfig->file, "%s%d %d\n",
                ((n%6) == 0 ? "\n        " : " "),
                (int)x[0], (int)y[0]);
    } else {
        fputs("\n", xfig->file);
    }
    return MP_OK;
}

static MpStatus
drawXFigPoint(MpDevice* dev, MpPoint x, MpPoint y)
{
    XFigDevice* xfig = (XFigDevice*)dev;
    return drawXFigPolylineObject(xfig, XFIG_POLYLINE_SUBTYPE,
                                  dev->groupLevel, &x, &y, 1, true);
}

static MpStatus
drawXFigRectangle(MpDevice* dev, MpPoint x0, MpPoint y0, MpPoint x1, MpPoint y1)
{
    XFigDevice* xfig = (XFigDevice*)dev;
    MpPoint x[4] = {x0, x0, x1, x1};
    MpPoint y[4] = {y0, y1, y1, y0};
    return drawXFigPolylineObject(xfig, XFIG_BOX_SUBTYPE,
                                  dev->groupLevel, x, y, 4, true);
}

static MpStatus
drawXFigPolyline(MpDevice* dev, const MpPoint* x, const MpPoint* y, MpInt n)
{
    if (n > 1) { /* FIXME: At least two points are needed. */
        XFigDevice* xfig = (XFigDevice*)dev;
        return drawXFigPolylineObject(xfig, XFIG_POLYLINE_SUBTYPE,
                                      dev->groupLevel, x, y, n, false);
    }
    return MP_OK;
}

static MpStatus
drawXFigPolygon(MpDevice* dev, const MpPoint* x, const MpPoint* y, MpInt n)
{
    if (n > 1) { /* FIXME: At least two points are needed. */
        XFigDevice* xfig = (XFigDevice*)dev;
        return drawXFigPolylineObject(xfig, XFIG_POLYGON_SUBTYPE,
                                      dev->groupLevel, x, y, n, false);
    }
    return MP_OK;
}

MpStatus
MpOpenXFigDevice(MpDevice** devptr, const char* ident, const char* arg)
{
    /* Note: Arguments have been checked but `arg` may be `NULL` or an empty
       string. */
    if (arg == NULL || arg[0] == '\0') {
        return MP_BAD_FILENAME;
    }

    /* Allocate structure and instanciate it. */
    MpDevice* dev = MpAllocateDevice(sizeof(XFigDevice));
    *devptr = dev;
    if (dev == NULL) {
        return MP_NO_MEMORY;
    }
    dev->select = selectXFigDevice;
    dev->setColorIndex = setXFigColorIndex;
    dev->setColor = setXFigColor;
    dev->drawPoint = drawXFigPoint;
    dev->drawRectangle = drawXFigRectangle;
    dev->drawPolyline = drawXFigPolyline;
    dev->drawPolygon = drawXFigPolygon;
    dev->colormapSize0 = (XFIG_C0MAX + 1 - XFIG_C0MIN);
    dev->colormapSize1 = (XFIG_C1MAX + 1 - XFIG_C1MIN);
    // FIXME: dev->colormapSize = dev->colormapSize0 + dev->colormapSize1;

    XFigDevice* xfig = (XFigDevice*)dev;
    xfig->file = fopen(arg, "w");
    if (xfig->file == NULL) {
        free((void*)dev);
        return MpSystemError();
    }
    xfig->colorIndex = XFIG_C0MIN;
    setXFigInitialColors(xfig);


    /* Page settings (assuming A4 paper). */
    const char* paperSize = "A4";
    int dotsPerInch = 1200;
    double dotsPerMillimeter = (double)dotsPerInch/MILLIMETERS_PER_INCH;
    dev->pageWidth = MP_A4_PAPER_WIDTH;
    dev->pageWidth = MP_A4_PAPER_HEIGHT;
    dev->horizontalResolution = dotsPerMillimeter;
    dev->verticalResolution = dotsPerMillimeter;
    dev->horizontalSamples = round(dev->pageWidth*dev->horizontalResolution);
    dev->verticalSamples = round(dev->pageHeight*dev->verticalResolution);

    /* Write XFig header information. */
    fprintf(xfig->file, "#FIG 3.2\n");
    fprintf(xfig->file, "%s\n",
            dev->pageWidth <= dev->pageHeight ? "Portrait" : "Landscape");
    fprintf(xfig->file, "Center\n"); /* "Center" or "Flush Left" */
    fprintf(xfig->file, "Metric\n"); /* "Metric" or "Inches" */
    fprintf(xfig->file, "%s\n", paperSize); /* papersize */
    fprintf(xfig->file, "%.2f\n", 100.0); /* magnification */
    fprintf(xfig->file, "Single\n"); /* multiple-page ("Single" or "Multiple" pages) */
    fprintf(xfig->file, "%d\n", -2); /*  transparent color (color number for
                                         transparent color for GIF
                                         export. -3=background, -2=None,
                                         -1=Default, 0-31 for standard colors
                                         or 32- for user colors) */
    fprintf(xfig->file, "# Created by muPlot.\n"); /* comment (An optional set of comments may be here,
                                                      which are associated with the whole figure) */
    fprintf(xfig->file, "%d %d\n", dotsPerInch, 2); /* resolution coord_system (Fig units/inch and coordinate system:
                                                       1: origin at lower left corner (NOT USED)
                                                       2: upper left) */
    return MP_OK;
}
