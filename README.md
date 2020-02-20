# µPlot is a micro-library for scientific plotting

You can have a look at the code but this is largely a work in progress.


## Guidelines

At the lowest level, plotting is done by device drivers via a unified
interface.  There may be various *drivers* (e.g. for Postscript, X11, etc.) and
*devices* are producing plots to a given out put (there may be several devices
using the same driver).

A typical plotting session is:

```c
// Install XFig driver.
MpInstallDriver("xfig", MpOpenXFigDevice);

// Open a device for drawing and XFig figure.
MpDevice* dev;
MpOpenDevice(&dev, "xfig", "plot.fig");

// Plot something.
...

// Close the device (this also set `dev` to `NULL`).
MpCloseDevice(&dev);
```

Here `MpDevice` is an (opaque to the end-user?) structure which stores anything
needed by the user-level interface.  Its contents is called the *public* part
of the device structure.  Specific devices *inherit* from `MpDevice` as
follows:

```c
typedef struct {
    // Public part common to all devices (must be the first member):
    MpDevice pub;

    // Device specific (private) part:
    type1 privData1;
    type2 privData2;
    ...
} SomeSpecificDevice;
```

A pointer to an instance of such a structure can be *safely* recast as a pointer
to a `MpDevice` structure.


### Graphic contexts

At the intermediate level, all plotting operations are redirected to a single
device.  To spare providing all parameters for each drawing operation (like
drawing a polyline), these operations are always performed with the current
device settings.  Hence, drawing only requires to provide the data (e.g., the
arrays `x` and `y` to draw a polyline).

Quick example:

```c
// Configure device `dev` for plotting:
MpSetCoordinateTransform(dev, &A); // set the data to NDC transform (see below)
MpSetColorIndex(dev, ci); // set the color index for drawing
MpSetLineStyle(dev, ls); // set the line style for drawing
MpSetLineWidth(dev, lw); // set the line width for drawing
...

// Eventually draw the polyline on device `dev` with the current settings
// (`x` and `y` are arrays storing the abscissae and ordinates of the points
// defined the polyline and `n` is the number of such points):
MpDrawPolyline(dev, x, y, n);
```

Configure operations can be done in any order.  For maximum efficiency, it is
only necessary to apply settings that do change (including the coordinate
transform) between calls to drawing routines for a given device.  The
user-level part of µPlot attempts to cache settings and to only notify the
driver when something really change.


### Multi-threading

µPlot user interface is intended to be single threaded.  However plotting
drivers (notably for interactive devices) may explot multi-threading to remain
responsive for redrawing damaged regions, zooming, etc.

Ensuring the integrity of the shared data is done by simple mechanisms.  The
driver shall only change the value of members in the common part (the *public*
part) of the `MpDevice` structure as a result of a user call and before the
called routine returns.  The driver can change the contents of the specific
part of the device structure (the *private* part) at any time.  This may
involve locks to ensure the integrity of the private data, but these locks are
managed by the driver specific routines and the *methods* implementing the
common interface.


### Coordinate transforms

To minimize the number of operations, affine coordinate transforms are used to
implement all necessary change of 2-dimensional coordinates.  At any time,
converting from one coordinate system to another involves at most one such
transform.  For instance, to convert any user-defined coordinates to device
coordinates (e.g., *points* or *pixels*) a transform, say `A`, is defined to
convert the user-defined coordinates into *normalized device coordinates*
(NDC), whereas the device driver maintains another transform, say `B`, to to
convert the NDC into device coordinates.  Then to convert the user defined
coordinates into appropriate device coordinates it is sufficient to define the
coordinate transform `C = B⋅A` which amounts to applying `A` and then `B`.

Setting the coordinate transform is done by:

```c
MpCoordinateTransform A;
MpSetCoordinateTransform(&A);
```

triggers recomputing the user-to-device coordinate transform as follows:

```c
MpCoordinateTransform C;
MpComposeCoordinateTransforms(&C, &B, &A);
```

That is compute the coefficienst of a coordinate transform `C` which amounts to
applying `A` first (user-to-NDC) and then `B` (NDC-to-device).

Note that `MpCoordinateTransform` is an alias to `MpAffineTransformDbl` as
double precision floating point is used for coordinate transforms.  For faster
operations, the drivers may convert the coordinate transform coefficienst to
single precision and and account for specific case (i.e., when the coordinate
transform implements no shear nor rotation).

The high-level interface takes care of setting the user-coordinate transform
only when it is appropriate: that is when it changes and when a device is
selected.



## Roadmap

- Write doc. for affine transforms.

- Write code for clipping a polyline.

- Implement Herschey fonts.  Start simple, then deal with UTF-8 encoding.
