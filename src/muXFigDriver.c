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

#define MILLIMETERS_PER_INCH 25.4
#define MP_A4_PAPER_WIDTH     210
#define MP_A4_PAPER_HEIGHT    297

#define MP_FLIP_X   (1 << 0)
#define MP_FLIP_Y   (1 << 1)

/*
 * Color indices of primary colors for XFig.  All these (approximately) map to the
 * 10 standard colors of µPlot.
 */
#define XFIG_COLOR_DEFAULT   -1
#define XFIG_COLOR_BLACK      0
#define XFIG_COLOR_BLUE       1
#define XFIG_COLOR_GREEN      2
#define XFIG_COLOR_CYAN       3
#define XFIG_COLOR_RED        4
#define XFIG_COLOR_MAGENTA    5
#define XFIG_COLOR_YELLOW     6
#define XFIG_COLOR_WHITE      7

/*
 * Correspondance between color indices.
 *
 * XFig has 32 read-only colors at indices 0-31 and a maximum of 512 possible
 * user defined colors at indices 32-543.  Clearly, the first 32 colors of XFig
 * belong to the 1st colormap of µPlot (intended for distinctive colors) while
 * the user defined colors belong to the 2nd colormap of µPlot (intended for
 * continuous colors).  Out of these, the first ones (XFig color indices -1 to
 * 7) match the 10 standard µPlot colors (at indices 0-9) assuming "Background"
 * for µPlot is "White" for XFig and "Foreground" for µPlot is "Default" for
 * XFig.  Remaining low color indices 8-31 (24 in total) of XFig can also be
 * considered as non-standard colors of the 1st colormap.  Hence the size of
 * the 1st colormap is 34 (10 + 24) for XFig while the maximum size for the
 * second colormap is 512.  Except for the 10 first color indices of µPlot
 * which require a special mapping, all µPlot color indices `ci` greater or
 * equal 10 correspond to color index `ci - 2` in XFig.
 *
 * To simplify the code, we allocate the maximum number of colors and keep
 * track of the number of user defined colors.
 */
#define XFIG_COLORMAP_SIZE_1  34
#define XFIG_COLORMAP_SIZE_2 512 /* This is the maximum value. */
#define XFIG_COLOR_INDEX(ci)  ((ci) < 10 ? standardColors[ci] : (int)(ci) - 1)

/*
 * The following table maps standard µPlot color indices to XFig colors.
 */
static int standardColors[] = {
    XFIG_COLOR_WHITE,   /* MP_COLOR_BACKGROUND */
    XFIG_COLOR_DEFAULT, /* MP_COLOR_FOREGROUND */
    XFIG_COLOR_RED,
    XFIG_COLOR_GREEN,
    XFIG_COLOR_BLUE,
    XFIG_COLOR_CYAN,
    XFIG_COLOR_MAGENTA,
    XFIG_COLOR_YELLOW,
    XFIG_COLOR_BLACK,
    XFIG_COLOR_WHITE,
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


#define XFIG_C0MIN   0
#define XFIG_C0MAX  31 /* Gold */
#define XFIG_C1MIN  32
#define XFIG_C1MAX 543
#define XFIG_CMIN    0
#define XFIG_CMAX  XFIG_C1MAX
#define XFIG_NCOLORS (XFIG_CMAX + 1 - XFIG_CMIN)

typedef struct _XFigDevice XFigDevice;

struct _XFigDevice {
    MpDevice pub;

    int stage; /* Initially 0, becomes 1 after the first graphic object has
                  been written; user defined colors can only be written at
                  stage = 0. */
    int          dotsPerInch;
    XFigLineStyle  lineStyle;
    int            lineWidth;
    MpColorIndex   fillColor;
    int             penStyle; // pen style, not used
    int             areaFill; // enumeration
    double          styleVal; // 1/80 inch, specification for dash/dotted lines
    XFigJoinStyle  joinStyle;   // enumeration type)
    XFigCapStyle    capStyle;    // enumeration type, only used for POLYLINE
    int               radius;        // 1/80 inch, radius of arc-boxes)
    int         forwardArrow; // forwardArrow (0: off, 1: on)
    int        backwardArrow; // backwardArrow (0: off, 1: on)
    int   pictureOrientation;
    const char*  pictureName;
    const char*    paperSize;
    FILE* file;
};



/* Encode RBG with colorants in bytes */
#define encodeColor(dst,r,g,b)                                          \
    MpEncodeColor(dst, (MpReal)(r)/(MpReal)255,                         \
                      (MpReal)(g)/(MpReal)255, (MpReal)(b)/(MpReal)255)

static void
warnColorMismatch(const char* A, const char* B)
{
    fprintf(stderr,
            "Warning: µPlot \"%s\" color does not match XFig \"%s\" color.\n",
            A, B);
}

#define CHECK_COLOR(A, B)                                       \
    do {                                                        \
        if (XFIG_COLOR_INDEX(MP_COLOR_##A) != XFIG_COLOR_##B) { \
            warnColorMismatch(#A, #B);                          \
        }                                                       \
    } while (0)

static MpStatus
initializeXFigDevice(MpDevice* dev)
{
    MpStatus status = MP_OK;

    if (dev->colormapSize != XFIG_NCOLORS) {
        return MP_BAD_SIZE;
    }

    /* Check mapping of color indices. */
    CHECK_COLOR(BACKGROUND, WHITE);
    CHECK_COLOR(FOREGROUND, DEFAULT);
    CHECK_COLOR(RED,        RED);
    CHECK_COLOR(GREEN,      GREEN);
    CHECK_COLOR(BLUE,       BLUE);
    CHECK_COLOR(CYAN,       CYAN);
    CHECK_COLOR(MAGENTA,    MAGENTA);
    CHECK_COLOR(YELLOW,     YELLOW);
    CHECK_COLOR(BLACK,      BLACK);
    CHECK_COLOR(WHITE,      WHITE);

    /* FIXME: need to define the followinh XFig colors. */
    /*    8-11 = four shades of blue (dark to lighter) */
    /*   12-14 = three shades of green (dark to lighter) */
    /*   15-17 = three shades of cyan (dark to lighter) */
    /*   18-20 = three shades of red (dark to lighter) */
    /*   21-23 = three shades of magenta (dark to lighter) */
    /*   24-26 = three shades of brown (dark to lighter) */
    /*   27-30 = four shades of pink (dark to lighter) */
    if (dev->colormapSize1 > 31+2) {
        /* 31 = Gold, +2 to match µPlot color index */
        MpEncodeColor(&dev->colormap[31+2], 255, 215, 0);
    }

    /* Initialize colormap CMAP1 with a ramp of grays. */
    if (dev->colormapSize2 > 100) {
        /* Use at most 100 colors by default. */
        dev->colormapSize2 = 100;
        dev->colormapSize = dev->colormapSize1 + dev->colormapSize2;

    }
    if (dev->colormapSize2 > 1) {
        MpReal a = (MpReal)1/(MpReal)(dev->colormapSize2 - 1);
        for (MpInt i = 0; i < dev->colormapSize2; ++i) {
            MpReal g = a*i;
            MpEncodeColor(&dev->colormap[dev->colormapSize1 + i], g,g,g);
        }
    }
    return status;
}

static MpStatus
finalizeXFigDevice(MpDevice* dev)
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
setXFigColormapSizes(MpDevice* dev, MpInt n1, MpInt n2)
{
    /* Note: Arguments have bee checked and this method is only called if at
       least of n1 or n2 is different from the current settings. */
    XFigDevice* xfig = (XFigDevice*)dev;
    if (xfig->stage > 0) {
        /* Too late to chage colormap sizes. */
        return MP_READ_ONLY;
    }
    MpStatus status = MP_OK;
    if (n1 != dev->colormapSize1) {
        /* The size of 1st colormap cannot be changed. */
        status = MP_BAD_SIZE;
    }
    if (n2 < 0) {
        n2 = 0;
        status = MP_BAD_SIZE;
    } else if (n2 > XFIG_COLORMAP_SIZE_2) {
        n2 = XFIG_COLORMAP_SIZE_2;
        status = MP_BAD_SIZE;
    }
    if (n2 != dev->colormapSize2) {
        dev->colormapSize2 = n2;
        dev->colormapSize = dev->colormapSize1 + dev->colormapSize2;
    }
    return status;
}

/* FIXME: This is the default behavior? */
static MpStatus setXFigColorIndex(MpDevice* dev, MpColorIndex ci)
{
    /* Note: Arguments have been checked. */
    dev->colorIndex = ci;
    return MP_OK;
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
    dev->colormap[ci].red   = rd;
    dev->colormap[ci].green = gr;
    dev->colormap[ci].blue  = bl;
    return MP_OK;
}

static unsigned
colorant(MpReal val)
{
    return (val <= (MpReal)0 ? (unsigned)0 :
            (val >= (MpReal)255 ? (unsigned)255 :
             (MP_IS_SINGLE_PRECISION(val) ?
              (unsigned)roundf((float)val*(float)255) :
              (unsigned)round((double)val*(double)255))));
}

static MpStatus
bumpXFigStage(XFigDevice* xfig)
{
    if (xfig->stage == 0) {
        /* Write XFig header information. */
        fprintf(xfig->file, "#FIG 3.2\n");
        fprintf(xfig->file, "%s\n",
                xfig->pub.pageWidth <= xfig->pub.pageHeight ? "Portrait" : "Landscape");
        fprintf(xfig->file, "Center\n"); /* "Center" or "Flush Left" */
        fprintf(xfig->file, "Metric\n"); /* "Metric" or "Inches" */
        fprintf(xfig->file, "%s\n", xfig->paperSize); /* papersize */
        fprintf(xfig->file, "%.2f\n", 100.0); /* magnification */
        fprintf(xfig->file, "Single\n"); /* multiple-page ("Single" or "Multiple" pages) */
        fprintf(xfig->file, "%d\n", -2); /*  transparent color (color number for
                                             transparent color for GIF
                                             export. -3=background, -2=None,
                                             -1=Default, 0-31 for standard colors
                                             or 32- for user colors) */
        fprintf(xfig->file, "# Created by muPlot.\n"); /* comment (An optional set of comments may be here,
                                                          which are associated with the whole figure) */
        fprintf(xfig->file, "%d %d\n", xfig->dotsPerInch, 2);
        /* resolution coord_system (Fig units/inch and coordinate system:
           1: origin at lower left corner (NOT USED)
           2: upper left) */

        /* Write definitions of arbitrary user defined colors beyond the 32
           standard colors.  The color objects must be defined before any other
           Fig objects. */
        for (int ci = XFIG_C1MIN; ci <= XFIG_C1MAX; ++ci) {
            fprintf(xfig->file, "%d %d #%02x%02x%02x\n",
                    XFIG_COLOR_OBJECT_TYPE, ci,
                    colorant(xfig->pub.colormap[ci].red),
                    colorant(xfig->pub.colormap[ci].green),
                    colorant(xfig->pub.colormap[ci].blue));
        }
        xfig->stage = 1;
    }
    return MP_OK;
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
    MpStatus status = bumpXFigStage(xfig);
    if (status != MP_OK) {
        return status;
    }
    fprintf(xfig->file, "2 %d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %ld\n",
            XFIG_POLYLINE_OBJECT_TYPE,
            subType,
            xfig->lineStyle,
            xfig->lineWidth,
            XFIG_COLOR_INDEX(xfig->pub.colorIndex),
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
    return status;
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

    /* Allocate structure and instanciate methods. */
    MpDevice* dev = MpAllocateDevice(sizeof(XFigDevice));
    *devptr = dev;
    if (dev == NULL) {
        return MP_NO_MEMORY;
    }
    dev->initialize = initializeXFigDevice;
    dev->finalize = finalizeXFigDevice;
    dev->setColorIndex = setXFigColorIndex;
    dev->setColormapSizes = setXFigColormapSizes;
    dev->setColor = setXFigColor;
    dev->drawPoint = drawXFigPoint;
    dev->drawRectangle = drawXFigRectangle;
    dev->drawPolyline = drawXFigPolyline;
    dev->drawPolygon = drawXFigPolygon;

    /* Open output file. */
    XFigDevice* xfig = (XFigDevice*)dev;
    xfig->file = fopen(arg, "w");
    if (xfig->file == NULL) {
        free((void*)dev);
        return MpSystemError();
    }

    /* Initialize private settings (assuming A4 paper). */
    xfig->paperSize = "A4";
    xfig->dotsPerInch = 1200;

    /* Initialize some public device settings.  Set the size of the secondary
       colormap to be the maximum possible. */
    double dotsPerMillimeter = (double)xfig->dotsPerInch/MILLIMETERS_PER_INCH;
    dev->pageWidth = MP_A4_PAPER_WIDTH;
    dev->pageWidth = MP_A4_PAPER_HEIGHT;
    dev->horizontalResolution = dotsPerMillimeter;
    dev->verticalResolution = dotsPerMillimeter;
    dev->horizontalSamples = round(dev->pageWidth*dev->horizontalResolution);
    dev->verticalSamples = round(dev->pageHeight*dev->verticalResolution);
    dev->colormapSize1 = XFIG_COLORMAP_SIZE_1;
    dev->colormapSize2 = XFIG_COLORMAP_SIZE_2;
    dev->colorIndex = MP_COLOR_FOREGROUND;
    return MP_OK;
}
