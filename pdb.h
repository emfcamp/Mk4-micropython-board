/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PDB_INCLUDE_H
#define PDB_INCLUDE_H

typedef struct PDB_Object * PDB_Handle;

typedef void (*PDB_NotifyFxn)(void * arg);

extern PDB_Handle PDB_create(PDB_NotifyFxn fxn, void * arg, uint32_t pin);
extern void PDB_start(PDB_Handle pdb);
extern void PDB_stop(PDB_Handle pdb);
extern void PDB_delete(PDB_Handle pdb);

#endif
