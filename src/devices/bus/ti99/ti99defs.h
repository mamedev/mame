// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Common definitions for TI family
    Should be included in each component.

    Michael Zapf

*****************************************************************************/

#ifndef MAME_BUS_TI99_TI99DEFS_H
#define MAME_BUS_TI99_TI99DEFS_H

#pragma once

// TI-99/4(A)
#define TI99_GROMPORT_TAG    "gromport"
#define TI99_GROM0_TAG       "console_grom_0"
#define TI99_GROM1_TAG       "console_grom_1"
#define TI99_GROM2_TAG       "console_grom_2"
#define TI99_IOPORT_TAG      "ioport"
#define TI99_EVPC_CONN_TAG   "evpc_conn"
#define TI99_DATAMUX_TAG     "datamux_16_8"
#define TI99_PADRAM_TAG      "scratchpad"
#define TI99_EXPRAM_TAG      "internal_32k_mod"
#define TI99_DSRROM          "dsrrom"
#define TI99_CONSOLEROM      "console_rom"
#define TI99_CONSOLEGROM     "cons_grom"

// Various systems (ti99, geneve)
#define TI_SOUNDCHIP_TAG   "soundchip"
#define TI_TMS9901_TAG     "tms9901"
#define TI_PERIBOX_TAG     "peb"
#define TI_JOYPORT_TAG     "joyport"
#define TI_VDP_TAG         "vdp"
#define TI_SCREEN_TAG      "screen"
#define TI_HEXBUS_TAG   "hexbus"
#define TI_CASSETTE     "cassette"
#define TI992_VDC_TAG   "vdc"

#define TI_VDPFREQ XTAL(10'738'635)

// v9938
#define COLORBUS_TAG     "colorbus"

#endif // MAME_BUS_TI99_TI99DEFS_H
