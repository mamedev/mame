// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#ifndef THOMFLOP_H_
#define THOMFLOP_H_

#include "image.h"
#include "imagedev/flopdrv.h"
#include "machine/mc6843.h"
#include "machine/mc6854.h"

extern UINT8 to7_controller_type; /* set during init */
extern UINT8 to7_floppy_bank;

/* number of external floppy controller ROM banks */
#define TO7_NB_FLOP_BANK 9

/* external floppy / network controller active */
#define THOM_FLOPPY_EXT (to7_controller_type >= 1)

/* internal floppy controller active (no or network extension) */
#define THOM_FLOPPY_INT (to7_controller_type == 0 || to7_controller_type > 4)


/* external controllers */
/* TO9 internal (WD2793) & external controllers */
/* TO8 internal (THMFC1) controller */

#endif /* THOMFLOP_H_ */
