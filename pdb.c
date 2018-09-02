/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/GPIO.h>

#include "pdb.h"

#define CHECK_MSEC    5    // Read hardware every 5 msec
#define PRESS_MSEC    15   // Stable time before registering pressed
#define RELEASE_MSEC  60  // Stable time before registering released

typedef struct PDB_Object {
    Clock_Handle clock;
    void (*notifyFxn)(void * arg);
    void * notifyArg;
    uint32_t pin;
    uint32_t count;
    uint32_t period;
    bool pressed;
} PDB_Object;

static bool debounce(PDB_Handle pdb, bool * pressed)
{
    bool raw;
    bool changed = false;

    if (pdb->period-- == 0) {
        Clock_stop(pdb->clock);
        return false;
    }

    *pressed = pdb->pressed;
    raw = GPIO_read(pdb->pin);
    if (raw == pdb->pressed) {
        if (pdb->pressed) {
            pdb->count = RELEASE_MSEC / CHECK_MSEC;
        }
        else {
            pdb->count = PRESS_MSEC /CHECK_MSEC;
        }
    }
    else {
        if (--pdb->count == 0) {
            pdb->pressed = raw;
            changed = true;
            *pressed = pdb->pressed;
        }
    }

    return changed;
}

static void updatePin(UArg arg)
{
    PDB_Handle pdb = (PDB_Handle)arg;
    bool pressed;

    if (debounce(pdb, &pressed)) {
        pdb->notifyFxn(pdb->notifyArg);
    }
}

PDB_Handle PDB_create(PDB_NotifyFxn fxn, void * arg, uint32_t pin)
{
    PDB_Handle pdb;

    if ((pdb = calloc(sizeof(PDB_Object), 1))) {
        pdb->notifyFxn = fxn;
        pdb->notifyArg = arg;
        pdb->pin = pin;

        Clock_Params params;
        Clock_Params_init(&params);
        params.period = CHECK_MSEC;
        params.arg = (UArg)pdb;
        if ((pdb->clock = Clock_create(updatePin, 1, &params, NULL)) == NULL) {
            free(pdb);
            pdb = NULL;
        }
    }

    return pdb;
}

void PDB_delete(PDB_Handle pdb)
{
    if (pdb) {
        if (pdb->clock) {
            Clock_stop(pdb->clock);
            Clock_delete(&pdb->clock);
        }
        free(pdb);
    }
}

void PDB_start(PDB_Handle pdb)
{
    pdb->pressed = true;
    pdb->period = 150;
    pdb->count = RELEASE_MSEC / CHECK_MSEC;
    Clock_start(pdb->clock);
}

void PDB_stop(PDB_Handle pdb)
{
    Clock_stop(pdb->clock);
}
