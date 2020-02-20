/*
 * muPlot.h --
 *
 * Definitions for the µPlot plotting libraru.
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

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
#  define _MP_BEGIN_DECLS extern "C" {
#  define _MP_END_DECLS }
#else
#  define _MP_BEGIN_DECLS
#  define _MP_END_DECLS
#endif

_MP_BEGIN_DECLS

typedef float   MpReal;
typedef long    MpInt;
typedef int     MpBool; /* FIXME: see bool.h */
typedef MpInt   MpColorIndex;
typedef int16_t MpPoint; /* for device coordinates, sufficient for giga-pixel images */

/* The following macro must be evaluated by the compiler (i.e. not the
   preprocessor), the result is constant so it will probably be eliminated by
   the optimizer. */
#define MP_SINGLE_PRECISION (sizeof(MpReal) <= sizeof(float))

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
    _MP_STATUS(MP_NOT_FOUND,         9, "not found")                    \
    _MP_STATUS(MP_NOT_IMPLEMENTED,  10, "not implemented")              \
    _MP_STATUS(MP_NOT_PERMITTED,    11, "forbidden operation")          \
    _MP_STATUS(MP_NO_MEMORY,        12, "insufficient memory")          \
    _MP_STATUS(MP_OUT_OF_RANGE,     13, "out of range value or index")  \
    _MP_STATUS(MP_READ_ONLY,        14, "read only parameter")          \
    _MP_STATUS(MP_SINGULAR,         15, "singular system of equations")

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
extern MpStatus MpSystemError(void);

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
 * @param dev       The address to store the new device structure.
 * @param ident     The identifier of the graphic device.
 * @param arg       Supplied argument like the filename.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpOpenDevice(MpDevice** dev, const char* ident, const char* arg);

/**
 * Close graphic device.
 *
 * Call this routine to close a graphic device.  This releases all associated
 * ressources.  As a consequence, the device can no longer be used.
 *
 * @param dev     The graphic device.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpCloseDevice(MpDevice* dev);

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

/**
 * Select device.
 *
 * This function is called to redirect all plotting calls to a given device.
 *
 * @param dev    The graphic device.
 *
 * @return A standard status: `MP_OK` on success, an error code on failure.
 */
extern MpStatus MpSelect(MpDevice* dev);

extern MpStatus MpDrawCellsHelper(MpDevice* dev, const MpColorIndex* z,
                                  MpInt n1, MpInt n2, MpInt stride,
                                  MpPoint x0, MpPoint y0, MpPoint x1, MpPoint y1);

_MP_END_DECLS

#endif /* _MUPLOT_H_ */
