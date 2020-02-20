#include <stdio.h>
#include <stdlib.h>
#include "muPlot.h"

static MpStatus
openDummyDevice(MpDevice** dev, const char* ident, const char* arg)
{
    return MP_NOT_IMPLEMENTED;
}

static void
printDriverList(int cnt, char** lst)
{
    printf("%d installed graphic driver(s):\n", cnt);
    for (int i = 0; i < cnt; ++i) {
        printf(" - %d: %s\n", i, lst[i]);
    }
    if (lst[cnt] != NULL) {
        fprintf(stderr, "non-NULL final pointer in list\n");
        exit(1);
    }
}

int main(int argc, char** argv)
{
    MpInt cnt;
    char** lst;
    MpStatus status;

    status = MpListDrivers(&cnt, &lst);
    printf("MpListDrivers -> %d: %s\n", (int)status, MpGetReason(status));
    if (status != MP_OK) {
        return 1;
    }
    printDriverList(cnt, lst);
    MpFreeDriverList(lst);

    status = MpInstallDriver("dummy1", openDummyDevice);
    printf("MpInstallDriver -> %d: %s\n", (int)status, MpGetReason(status));
    if (status != MP_OK) {
        return 1;
    }

    status = MpInstallDriver("dummy2", openDummyDevice);
    printf("MpInstallDriver -> %d: %s\n", (int)status, MpGetReason(status));
    if (status != MP_OK) {
        return 1;
    }

    status = MpListDrivers(&cnt, &lst);
    printf("MpListDrivers -> %d: %s\n", (int)status, MpGetReason(status));
    if (status != MP_OK) {
        return 1;
    }
    printDriverList(cnt, lst);
    MpFreeDriverList(lst);

    return 0;
}
