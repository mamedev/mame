// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix2000.cpp (Super XaviX)

    The dies for these are marked

    SSD 2000 NEC 85605-621
    and possibly
    SSD 2002 NEC 85054-611 (although this might use even more opcodes and need it's own file)

    6502 with custom opcodes
    integrated gfx / sound

    special notes

    see xavix.cpp for basic notes

    the machines using the '2000' series chips seem to have more extra
    opcodes, so presumably these were only available in those models.
    of note, push x / push y and pull x / pull y

    Dirt Rebel MX, which is confirmed as 2000 type however seems to require
    some of the documented but 'undocumented' opcodes on the 6502 to have additonal
    meaning for one specific function on startup

    this just seems to be some very buggy checksum code where the games don't even care about the result...
    (actually it looks like some games might be using it as a PRNG seed?)

    a is 80 when entering here?

    01BC37: A0 3F       ldy #$3f
    01BC39: B2          clr a     // clear acculuator
    01BC3A: 1B          spa0 a    // store 'accumulator' into byte 0 of PA 'address' register
    01BC3B: 9B          spa2 a    // store 'accumulator' into byte 2 of PA 'address' register

    -- loop point 2
    01BC3C: 98          tya       // y -> a  (3f on first run of loop)
    01BC3D: 5B          spa1 a    // store 'accumulator' into byte 1 of PA 'address' register  (003f00 on first loop?)

    -- loop point 1
    01BC3E: A3          ldal0 a   // read byte 0 of 32-bit 'long' register into accumulator
    01BC3F: 73          adcpa     // adc ($Address PA), y
    01BC40: 83          stal0 a   // store accumulator back in byte 0 of 32-bit 'long' register (even byte checksum?)
    01BC41: FB          incpa     // increase 'address' register PA
    01BC42: A7          ldal1 a   // read byte 1 of 32-bit 'long' register into accumulator
    01BC43: 73          adcpa     // adc ($Address PA), y
    01BC44: 87          stal1 a   // store accumulator back in byte 0 of 32-bit 'long' register (odd byte checksum?)
    01BC45: FB          incpa     // increase 'address' register PA
    01BC46: D0 F6       bne $1bc3e // (branch based on PA increase, so PA must set flags?, probably overflows after 0xffff if upper byte is 'bank'? or at 0xff if this really is a mirror of the function below

    01BC48: 88          dey          // decrease y, which contained 3f at the start
    01BC49: 10 F1       bpl $1bc3c  // branch back to loop point 2 to reload counter

    // contains the odd byte checksum once we drop out the loop
    01BC4B: 8D FB 00    sta $00fb // store it in zero page memory
    01BC4E: A3          ldal0 a  // get the even byte checksum from byte 0 of 32-bit 'long' register
    01BC4F: 8D FA 00    sta $00fa // store it in zero page memory
    01BC52: 07          oral1 a   // why do we want to do this? (routine below does it too)
    01BC53: D0 03       bne $1bc58
    01BC55: CE FA 00    dec $00fa
    01BC58: 80          retf

    this is presumably meant to be similar to the function found in Namco
    Nostalgia 2

    // A is 80 on entry
    09FFD8: A9 3F       lda #$3f
    09FFDA: 85 01       sta $01   // set upper bit of pointer at 0x0000 to 3f  (it completely fails to initialize the ram at 00, we currently init it to ff, but should probably be 0?)
    09FFDC: A0 00       ldy #$00  // clear inner loop counter
    -- loop point 1 and 2
    09FFDE: AD FA 00    lda $00fa     // read current even byte checksum value from zero page ram fa (game also completely fails to initialize this)
    09FFE1: 71 00       adc ($00), y  // so 3f00 + y first outer loop, 3e00 + y second outer loop with y increasing in inner loop (add byte read to even checksum byte?)
    09FFE3: 8D FA 00    sta $00fa     // store even checksum at 0xfa in zero page ram
    09FFE6: C8          iny           // increase counter y
    09FFE7: AD FB 00    lda $00fb     // read current odd byte checksum value from zero page ram fb (likewise never initialized!)
    09FFEA: 71 00       adc ($00), y  // 3f00 + y (add byte read to odd checksum byte)
    09FFEC: 8D FB 00    sta $00fb     // store odd checksum byte at 0xfb in zero page ram
    09FFEF: C8          iny           // increase y
    09FFF0: D0 EC       bne $9ffde    // branch back if y hasn't overflowed (so work on 0x100 bytes, looping back 0x7f times)
    09FFF2: C6 01       dec $01       // decrease accumulator (started at 3f) (inner loop counter already 0 because it overflowed)
    09FFF4: 10 E8       bpl $9ffde    // branch back

    // checksums are already in place after loop so no need to copy them like in above
    09FFF6: 0D FA 00    ora $00fa  // same weird 'or' call
    --
    09FFF9: D0 03       bne $9fffe
    09FFFB: CE FA 00    dec $00fa
    09FFFE: 80          retf


***************************************************************************/

#include "emu.h"
#include "xavix2000.h"
#include "xavix2000d.h"

DEFINE_DEVICE_TYPE(XAVIX2000, xavix2000_device, "xavix2000", "XaviX (SSD 2000 / 2002)")

xavix2000_device::xavix2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_device(mconfig, XAVIX2000, tag, owner, clock)
{
	program_config.m_addr_width = 24;
	program_config.m_logaddr_width = 24;
	sprogram_config.m_addr_width = 24;
	sprogram_config.m_logaddr_width = 24;
}

void xavix2000_device::device_start()
{
	xavix_device::device_start();

	state_add(SXAVIX_L, "L", m_l).callimport().formatstr("%8s");;
	state_add(SXAVIX_PA, "PA", m_pa).callimport().formatstr("%8s");
	state_add(SXAVIX_PB, "PB", m_pb).callimport().formatstr("%8s");
}

std::unique_ptr<util::disasm_interface> xavix2000_device::create_disassembler()
{
	return std::make_unique<xavix2000_disassembler>();
}


void xavix2000_device::state_import(const device_state_entry &entry)
{
	xavix_device::state_import(entry);

	switch(entry.index())
	{
	case SXAVIX_L:
		break;
	case SXAVIX_PA:
		break;
	case SXAVIX_PB:
		break;
	}
}

void xavix2000_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	xavix_device::state_string_export(entry, str);

	switch(entry.index())
	{
	case SXAVIX_L:
		str = string_format("%08x", m_l);
		break;
	case SXAVIX_PA:
		str = string_format("%08x", m_pa);
		break;
	case SXAVIX_PB:
		str = string_format("%08x", m_pb);
		break;
	}
}


#include "cpu/m6502/xavix2000.hxx"
