// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    tmsops.cpp

    TMS57002 "DASP" emulator.

***************************************************************************/

#include "emu.h"
#include "tms57002.h"

// Kept separate from tms57002.cpp to avoid the optimizer eating all the memory

#define CINTRP
#include "cpu/tms57002/tms57002.hxx"
#undef CINTRP
