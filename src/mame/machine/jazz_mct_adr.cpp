// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the MCT-ADR device found in Microsoft Jazz/MIPS
 * ARCSystem 100 architecture systems. This device was originally designed
 * by Microsoft, and then implemented and used in various forms by MIPS,
 * Olivetti, LSI Logic, NEC, Acer and others.
 *
 * Specific implementations/derivatives include:
 *
 *   LSI Logic R4030/R4230
 *   NEC μPD31432
 *   ALI M6101-A1
 *
 * References:
 *
 *   https://datasheet.datasheetarchive.com/originals/scans/Scans-054/DSAIH000102184.pdf
 *   https://github.com/torvalds/linux/tree/master/arch/mips/jazz/
 *   http://cvsweb.netbsd.org/bsdweb.cgi/src/sys/arch/arc/jazz/
 *
 * TODO
 *   - everything (skeleton only)
 */

#include "emu.h"
#include "jazz_mct_adr.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(JAZZ_MCT_ADR, jazz_mct_adr_device, "jazz_mct_adr", "Jazz MCT-ADR")

jazz_mct_adr_device::jazz_mct_adr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, JAZZ_MCT_ADR, tag, owner, clock)
{
}

void jazz_mct_adr_device::map(address_map &map)
{
	map(0x000, 0x003).lr32("config",   [](){ return 0x00000000; });
	map(0x008, 0x00b).lr32("revision", [](){ return 0xf0000000; });
}

void jazz_mct_adr_device::device_start()
{
}

void jazz_mct_adr_device::device_reset()
{
}
