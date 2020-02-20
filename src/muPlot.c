/*
 * muPlot.c --
 *
 * Basic plotting routines.
 *
 *------------------------------------------------------------------------------
 *
 * This file if part of the µPlot software licensed under the MIT license
 * (https://github.com/emmt/muPlot.jl).
 *
 * Copyright (C) 2020, Éric Thiébaut.
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "muPlotPriv.h"

typedef struct _MpDriver MpDriver;
struct _MpDriver {
    MpStatus (*open)(MpDevice** dev, const char* ident, const char* arg);
    MpDriver* next;
    const char ident[1]; /* Driver identifier. */
};

MpStatus
MpSystemError()
{
    return -(1 + errno);
}

const char*
MpGetReason(MpStatus status)
{
    if (status < 0) {
        int errnum = -(1 + status);
        const char* mesg = strerror(errnum);
        return (mesg != NULL ? mesg : "unknown system error number");
    }
    switch (status) {
#define _MP_STATUS(a,b,c) case a: return c;
        _MP_STATUS_LIST
#undef _MP_STATUS
    default: return "unknown status";
    }
}

/* Chained list of installed drivers. */
static MpDriver* firstDriver = NULL;

MpStatus
MpInstallDriver(const char* ident,
                MpStatus (*open)(MpDevice** dev,
                                 const char* ident,
                                 const char* arg))
{
    /* Check arguments. */
    if (ident == NULL || ident[0] == '\0') {
        return MP_BAD_IDENTIFIER;
    }
    if (open == NULL) {
        return MP_BAD_METHOD;
    }

    /* Replace existing driver if found. */
    for (MpDriver* drv = firstDriver; drv != NULL; drv = drv->next) {
        if (strcmp(drv->ident, ident) == 0) {
            drv->open = open;
            return MP_OK;
        }
    }

    /* Insert a new driver in the list. */
    size_t len = strlen(ident);
    size_t siz = MP_OFFSET_OF(MpDriver, ident) + len + 1;
    MpDriver* drv = (MpDriver*)malloc(siz);
    if (drv == NULL) {
        return MP_NO_MEMORY;
    }
    memset((void*)drv, 0, siz);
    drv->open = open;
    drv->next = firstDriver;
    memcpy((void*)&drv->ident[0], ident, len);
    firstDriver = drv;
    return MP_OK;
}

MpStatus
MpUninstallDriver(const char* ident)
{
    /* Check argument. */
    if (ident == NULL || ident[0] == '\0') {
        return MP_BAD_IDENTIFIER;
    }

    /* Uninstall existing driver if found. */
    MpDriver* prev = NULL;
    for (MpDriver* drv = firstDriver; drv != NULL; drv = drv->next) {
        if (strcmp(drv->ident, ident) == 0) {
            if (prev == NULL) {
                firstDriver = drv->next;
            } else {
                prev->next = drv->next;
            }
            free((void*)drv);
            return MP_OK;
        }
        prev = drv;
    }
    return MP_NOT_FOUND;
}

MpStatus
MpUninstallAllDrivers(void)
{
    while (firstDriver != NULL) {
        MpUninstallDriver(firstDriver->ident);
    }
    return MP_OK;
}

MpStatus
MpListDrivers(MpInt* argc, char*** argv)
{
    /* Preser outputs in case of errors. */
    if (argc != NULL) {
        *argc = 0;
    }
    if (argv != NULL) {
        *argv = NULL;
    }

    /* Check argument. */
    if (argc == NULL || argv == NULL) {
        return MP_BAD_ADDRESS;
    }
    MpInt cnt = 0;
    size_t len = 0;
    for (MpDriver* drv = firstDriver; drv != NULL; drv = drv->next) {
        cnt += 1;
        len += strlen(drv->ident) + 1;
    }
    size_t siz = (cnt + 1)*sizeof(char*) + len;
    void* buf = malloc(siz);
    if (buf == NULL) {
        return MP_NO_MEMORY;
    }
    memset(buf, 0, siz);
    char** lst = (char**)buf;
    char* str = ((char*)buf) + (cnt + 1)*sizeof(char*);
    MpInt idx = 0;
    for (MpDriver* drv = firstDriver; drv != NULL; drv = drv->next) {
        if (idx < cnt) {
            len = strlen(drv->ident);
            memcpy(str, drv->ident, len);
            str[len] = '\0';
            lst[idx] = str;
            str += len + 1;
            idx += 1;
        }
    }
    if (idx != cnt) {
        free(buf);
        return MP_ASSERTION_FAILED;
    }
    lst[cnt] = NULL;
    *argc = cnt;
    *argv = lst;
    return MP_OK;
}

MpStatus
MpFreeDriverList(char** argv)
{
    if (argv != NULL) {
        free((void*)argv);
    }
    return MP_OK;
}

MpDevice*
MpAllocateDevice(size_t size)
{
    if (size < sizeof(MpDevice)) {
        size = sizeof(MpDevice);
    }
    void* buf = malloc(size);
    if (buf == NULL) {
        return NULL;
    }
    memset(buf, 0, size);
    return (MpDevice*)buf;
}

MpStatus
MpCheckPageSettings(MpDevice* dev)
{
    /* Minimal check. */
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }

    /* Checks values that may have been set by the driver.  The horizontal and
       vertical resolutions must have been set to finite non-zero values (they
       may be negative to indicate reverse orientation).  Pager size and/or
       number of samples may have been set or left to their uninitialized
       value (that is zero). */
    if (! MP_IS_FINITE(dev->horizontalResolution) ||
        ! MP_IS_FINITE(dev->verticalResolution) ||
        ! MP_IS_FINITE(dev->pageWidth) || dev->pageWidth < 0 ||
        ! MP_IS_FINITE(dev->pageHeight) || dev->pageHeight < 0) {
        return MP_BAD_SETTINGS;
    }

    /* Attempt to fix page size. */
    if (dev->pageWidth == 0 && dev->horizontalSamples > 0) {
        dev->pageWidth = dev->horizontalSamples/fabs(dev->horizontalResolution);
        if (! MP_IS_FINITE(dev->pageWidth)) {
            return MP_BAD_SETTINGS;
        }
    }
    if (dev->pageHeight == 0 && dev->verticalSamples > 0) {
        dev->pageHeight = dev->verticalSamples/fabs(dev->verticalResolution);
        if (! MP_IS_FINITE(dev->pageHeight)) {
            return MP_BAD_SETTINGS;
        }
    }

    /* Attempt to fix number of samples. */
    if (dev->horizontalSamples == 0 && dev->pageWidth > 0) {
        dev->horizontalSamples = round(dev->pageWidth*fabs(dev->horizontalResolution));
    }
    if (dev->verticalSamples == 0 && dev->pageHeight > 0) {
        dev->verticalSamples = round(dev->pageHeight*fabs(dev->verticalResolution));
    }

    /* Final check. */
    if (dev->pageWidth <= 0 || dev->horizontalSamples < 1 ||
        fabs(dev->horizontalSamples -
             dev->pageWidth*fabs(dev->horizontalResolution)) >= 1 ||
        dev->pageHeight <= 0 || dev->verticalSamples < 1 ||
        fabs(dev->verticalSamples -
             dev->pageHeight*fabs(dev->verticalResolution)) >= 1) {
        return MP_BAD_SETTINGS;
    }

    return MP_OK;
}

static MpStatus
doNothing(MpDevice* dev)
{
    return MP_OK;
}

static MpStatus
cannotSetPageSize(MpDevice* dev, MpReal w, MpReal h)
{
    return MP_NOT_PERMITTED;
}

static MpStatus
cannotSetResolution(MpDevice* dev, MpReal xpmm, MpReal ypmm)
{
    return MP_NOT_PERMITTED;
}

static MpStatus
defaultSetColorIndex(MpDevice* dev, MpColorIndex ci)
{
    /* Note: Arguments have bee checked. */
    dev->colorIndex = ci;
    return MP_OK;
}

static MpStatus
defaultSetColor(MpDevice* dev, MpColorIndex ci, MpReal rd, MpReal gr, MpReal bl)
{
    /* Note: Arguments have bee checked. */
    dev->colormap[ci].red   = rd;
    dev->colormap[ci].green = gr;
    dev->colormap[ci].blue  = bl;
    return MP_OK;
}

static MpStatus
defaultSetLineStyle(MpDevice* dev, MpLineStyle ls)
{
    /* Note: Arguments have bee checked. */
    dev->lineStyle = ls;
    return MP_OK;
}

static MpStatus
defaultSetLineWidth(MpDevice* dev, MpReal lw)
{
    /* Note: Arguments have bee checked. */
    dev->lineWidth = lw;
    return MP_OK;
}

MpStatus
MpCheckMethods(MpDevice* dev)
{
    /* Minimal check. */
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }

    /* Provide substitutes (if possible). */
#define SUBSTITUTE_METHOD(M,A) if (M == NULL) M = A
    SUBSTITUTE_METHOD(dev->close,          doNothing);
    SUBSTITUTE_METHOD(dev->select,         doNothing);
    SUBSTITUTE_METHOD(dev->setPageSize,    cannotSetPageSize);
    SUBSTITUTE_METHOD(dev->setResolution,  cannotSetResolution);
    SUBSTITUTE_METHOD(dev->startBuffering, doNothing);
    SUBSTITUTE_METHOD(dev->stopBuffering,  doNothing);
    SUBSTITUTE_METHOD(dev->beginPage,      doNothing);
    SUBSTITUTE_METHOD(dev->endPage,        doNothing);
    SUBSTITUTE_METHOD(dev->setColorIndex,  defaultSetColorIndex);
    SUBSTITUTE_METHOD(dev->setColor,       defaultSetColor);
    SUBSTITUTE_METHOD(dev->setLineWidth,   defaultSetLineWidth);
    SUBSTITUTE_METHOD(dev->setLineStyle,   defaultSetLineStyle);
    SUBSTITUTE_METHOD(dev->drawCells,      MpDrawCellsHelper);
#undef SUBSTITUTE_METHOD

    /* Make sure that all methods are defined. */
    if (dev->close          == NULL ||
        dev->select         == NULL ||
        dev->setPageSize    == NULL ||
        dev->setResolution  == NULL ||
        dev->startBuffering == NULL ||
        dev->stopBuffering  == NULL ||
        dev->beginPage      == NULL ||
        dev->endPage        == NULL ||
        dev->setColorIndex  == NULL ||
        dev->setColor       == NULL ||
        dev->setLineStyle   == NULL ||
        dev->setLineWidth   == NULL ||
        dev->drawPoint      == NULL ||
        dev->drawRectangle  == NULL ||
        dev->drawPolyline   == NULL ||
        dev->drawPolygon    == NULL ||
        dev->drawCells      == NULL) {
        return MP_BAD_METHOD;
    }
    return MP_OK;
}

MpStatus
MpCheckColors(MpDevice* dev)
{
    /* First attempt to fix total number of colors. */
    if (dev->colormapSize == 0) {
        dev->colormapSize = dev->colormapSize0 + dev->colormapSize1;
    }
    if (dev->colormapSize0 < 2 || dev->colormapSize1 < 0 ||
        dev->colormapSize != dev->colormapSize0 + dev->colormapSize1) {
        return MP_BAD_SETTINGS;
    }
    if (dev->colormap == NULL) {
        dev->colormap = (MpColor*)calloc(dev->colormapSize, sizeof(MpColor));
        if (dev->colormap == NULL) {
            return MP_NO_MEMORY;
        }
    }
    MpStatus status = MP_OK;
#if 0 // FIXME: provide means to initially set colors and color index
    for (MpColorIndex ci = 0; status == MP_OK && ci < dev->colormapSize; ++ci) {
        status = dev->setColor(dev, ci,
                               &dev->colormap[ci].red,
                               &dev->colormap[ci].green,
                               &dev->colormap[ci].blue);
    }
#endif
    return status;
}

MpStatus
MpOpenDevice(MpDevice** devptr, const char* ident, const char* arg)
{
    /* Check arguments and set `*devptr` to `NULL` in case of errors. */
    if (devptr == NULL) {
        return MP_BAD_ADDRESS;
    }
    *devptr = NULL;
    if (ident == NULL || ident[0] == '\0') {
        return MP_BAD_IDENTIFIER;
    }

    /* Find driver. */
    MpStatus status = MP_NOT_FOUND;
    for (MpDriver* drv = firstDriver; drv != NULL; drv = drv->next) {
        if (strcmp(drv->ident, ident) == 0) {
            status = drv->open(devptr, ident, arg);
            break;
        }
    }
    if (status == MP_OK) {
        /* Fix/check settings. */
        if (*devptr == NULL) {
            status = MP_BAD_ADDRESS;
        }
        if (status == MP_OK) {
            status = MpCheckPageSettings(*devptr);
        }
        if (status == MP_OK) {
            status = MpCheckMethods(*devptr);
        }
        if (status == MP_OK) {
            status = MpCheckColors(*devptr);
        }
        if (status != MP_OK) {
            MpCloseDevice(devptr);
        }
    }
    return status;
}

static MpStatus
finalizeDevice(MpDevice* dev)
{
    MpStatus status = MP_OK;
    if (dev != NULL) {
        /* Device not yet closed. */
        status = dev->close(dev);
        if (dev->colormap != NULL) {
            free((void*)dev->colormap);
            dev->colormap = NULL;
            dev->colormapSize = 0;
        }
        free((void*)dev);
    }
    return status;
}

MpStatus
MpCloseDevice(MpDevice** devptr)
{
    if (devptr == NULL) {
        return MP_BAD_ADDRESS;
    }
    MpStatus status = finalizeDevice(*devptr);
    *devptr = NULL;
    return status;
}

MpStatus
MpSetPageSize(MpDevice* dev, MpReal width, MpReal height)
{
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }
    if (width < 1 || height < 1) {
        return MP_BAD_ARGUMENT;
    }
    if (width == dev->pageWidth && height == dev->pageHeight) {
        return MP_OK;
    }
    return dev->setPageSize(dev, width, height);
}

MpStatus
MpGetPageSize(MpDevice* dev, MpReal* width, MpReal* height)
{
    if (dev == NULL || width == NULL || height == NULL) {
        return MP_BAD_ADDRESS;
    }
    *width  = dev->pageWidth;
    *height = dev->pageHeight;
    return MP_OK;
}

MpStatus
MpSetResolution(MpDevice* dev, MpReal xpmm, MpReal ypmm)
{
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }
    if (! MP_IS_FINITE(xpmm) || xpmm <= 0 || ! MP_IS_FINITE(ypmm) || ypmm <= 0) {
        return MP_BAD_ARGUMENT;
    }
    if (xpmm == dev->horizontalResolution && ypmm == dev->verticalResolution) {
        return MP_OK;
    }
    return dev->setResolution(dev, xpmm, ypmm);
}

MpStatus
MpGetResolution(MpDevice* dev, MpReal* xpmm, MpReal* ypmm)
{
    if (dev == NULL || xpmm == NULL || ypmm == NULL) {
        return MP_BAD_ADDRESS;
    }
    *xpmm = dev->horizontalResolution;
    *ypmm = dev->verticalResolution;
    return MP_OK;
}

MpStatus
MpGetNumberOfSamples(MpDevice* dev, MpPoint* width, MpPoint* height)
{
    if (dev == NULL || width == NULL || height == NULL) {
        return MP_BAD_ADDRESS;
    }
    *width  = dev->horizontalSamples;
    *height = dev->verticalSamples;
    return MP_OK;
}

MpStatus
MpSelect(MpDevice* dev)
{
    return (dev == NULL ? MP_BAD_ADDRESS : dev->select(dev));
}

MpStatus
MpDrawCellsHelper(MpDevice* dev, const MpColorIndex* z,
                  MpInt n1, MpInt n2, MpInt stride,
                  MpPoint x0, MpPoint y0, MpPoint x1, MpPoint y1)
{
    MpStatus status = MP_OK;
    MpColorIndex ci0, cip, ci;

    /* Early return if nothing to do. */
    if (n1 < 1 || n2 < 1) {
        return status;
    }

    /* Save initial color index. */
    status = MpGetColorIndex(dev, &ci0);
    if (status != MP_OK) {
        return status;
    }

    /* Draw all cells. */
    float dx = (float)(x1 - x0)/n1;
    float dy = (float)(y1 - y0)/n2;
    cip = -1;
    for (MpInt i2 = 0; i2 < n2; ++i2) {
        const MpColorIndex* c =  z + i2*stride;
        MpPoint cy0 = y0 + (MpPoint)roundf(i2*dy); /* FIXME: replace by integer code */
        MpPoint cy1 = cy0 + dy;
        for (MpInt i1 = 0; i1 < n1; ++i1) {
            /* Set color index if it has changed. */
            ci = c[i1];
            if (ci != cip) {
                status = dev->setColorIndex(dev, c[i1]);
                if (status != MP_OK) {
                    return status;
                }
                cip = ci;
            }
            /* Draw rectangle. */
            MpPoint cx0 = x0 + i1*dx;
            MpPoint cx1 = cx0 + dx;
            status = dev->drawRectangle(dev, cx0, cy0, cx1, cy1);
            if (status != MP_OK) {
                return status;
            }
        }
    }

    /* Restore initial color index. */
    return MpSetColorIndex(dev, ci0);
}

MpStatus
MpSetColorIndex(MpDevice* dev, MpColorIndex ci)
{
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }
    if (ci == dev->colorIndex) {
        return MP_OK;
    }
    if (ci < 0 || ci >= dev->colormapSize) {
        return MP_OUT_OF_RANGE;
    }
    return dev->setColorIndex(dev, ci); // FXIME: check that dev->colorIndex = ci
}

MpStatus
MpGetColorIndex(MpDevice* dev, MpColorIndex* ci)
{
    if (dev == NULL || ci == NULL) {
        return MP_BAD_ADDRESS;
    }
    *ci = dev->colorIndex;
    return MP_OK;
}

/*
 * The following macro clamp a colorant value to the range [0,1].  Argument
 * `lval` is an L-value.  Argument `label` is the label to go to if `lval` is
 * not-a-number.
 */
#define CLAMP_COLORANT(lval, label)             \
    do {                                        \
        if (MP_IS_NAN(lval)) {                  \
            goto label;                         \
        }                                       \
        if (lval < 0) {                         \
            lval = 0;                           \
        }  else if (lval > 1) {                 \
            lval = 1;                           \
        }                                       \
    } while (0)

MpStatus
MpSetColor(MpDevice* dev, MpColorIndex ci, MpReal rd, MpReal gr, MpReal bl)
{
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }
    if (ci < 0 || ci >= dev->colormapSize) {
        return MP_OUT_OF_RANGE;
    }
    CLAMP_COLORANT(rd, badValue);
    CLAMP_COLORANT(gr, badValue);
    CLAMP_COLORANT(bl, badValue);
    if (dev->colormap[ci].red   == rd &&
        dev->colormap[ci].green == gr &&
        dev->colormap[ci].blue  == bl) {
        return MP_OK;
    }
    return dev->setColor(dev, ci, rd, gr, bl);

 badValue:
    return MP_BAD_SETTINGS;
}

MpStatus
MpGetColor(MpDevice* dev, MpColorIndex ci,
           MpReal* rd, MpReal* gr, MpReal* bl)
{
    if (dev == NULL || rd == NULL || gr == NULL || bl == NULL) {
        return MP_BAD_ADDRESS;
    }
    if (ci < 0 || ci >= dev->colormapSize) {
        return MP_OUT_OF_RANGE;
    }
    *rd = dev->colormap[ci].red;
    *gr = dev->colormap[ci].green;
    *bl = dev->colormap[ci].blue;
    return MP_OK;
}

MpStatus
MpSetLineStyle(MpDevice* dev, MpLineStyle ls)
{
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }
    if (ls == dev->lineStyle) {
        return MP_OK;
    }
    if (ls < 0 || ls > MP_DASH_TRIPLE_DOTTED_LINE) {
        return MP_OUT_OF_RANGE;
    }
    return dev->setLineStyle(dev, ls);
}

MpStatus
MpGetLineStyle(MpDevice* dev, MpLineStyle* ls)
{
    if (dev == NULL || ls == NULL) {
        return MP_BAD_ADDRESS;
    }
    *ls = dev->lineStyle;
    return MP_OK;
}

#define MAX_LINE_WIDTH 100 // FIXME:

MpStatus
MpSetLineWidth(MpDevice* dev, MpReal lw)
{
    if (dev == NULL) {
        return MP_BAD_ADDRESS;
    }
    if (lw == dev->lineWidth) {
        return MP_OK;
    }
    if (MP_IS_NAN(lw) || lw < 0 || lw > MAX_LINE_WIDTH) {
        return MP_BAD_SETTINGS;
    }
    return dev->setLineWidth(dev, lw);
}

MpStatus
MpGetLineWidth(MpDevice* dev, MpReal* lw)
{
    if (dev == NULL || lw == NULL) {
        return MP_BAD_ADDRESS;
    }
    *lw = dev->lineWidth;
    return MP_OK;
}
