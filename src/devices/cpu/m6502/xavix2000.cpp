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

	a is 80 when entering here?

	01BC37: A0 3F       ldy #$3f
	01BC39: B2          clrl      // clear 32-bit 'long' register (part of it might be the accumulator?) (or clear accumulator?)
	01BC3A: 1B          spa0 a    // store 'accumulator' into byte 0 of PA 'address' register
	01BC3B: 9B          spa2 a    // store 'accumulator' into byte 2 of PA 'address' register
	
	-- loop point 2
	01BC3C: 98          tya       // y -> a  (3f on first run of loop)
	01BC3D: 5B          spa1 a    // store 'accumulator' into byte 1 of PA 'address' register  (803f80 on first loop? 003f00 if we clear accumulator with clrl instead) maybe should be 3f0000 ? (would make more sense for a ram test)
	
	-- loop point 1
	01BC3E: A3          ldal0 a   // read byte 0 of 32-bit 'long' register into accumulator
	01BC3F: 73          adcpa     // adc ($Address PA), y
	01BC40: 83          stal0 a   // store accumulator back in byte 0 of 32-bit 'long' register (even byte checksum?)
	01BC41: FB          incpa     // increase 'address' register PA
	01BC42: A7          ldal1 a   // read byte 1 of 32-bit 'long' register into accumulator
	01BC43: 73          adcpa     // adc ($Address PA), y
	01BC44: 87          stal1 a   // store accumulator back in byte 0 of 32-bit 'long' register (odd byte checksum?)
	01BC45: FB          incpa     // increase 'address' register PA
	01BC46: D0 F6       bne $1bc3e // (branch based on PA increase, so PA must set flags?, probably overflows at 0xffff if upper byte is 'bank'?)

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

    09FFD8: A9 3F       lda #$3f
    09FFDA: 85 01       sta $01
    09FFDC: A0 00       ldy #$00
    09FFDE: AD FA 00    lda $00fa
    09FFE1: 71 00       adc ($00), y
    09FFE3: 8D FA 00    sta $00fa
    09FFE6: C8          iny
    09FFE7: AD FB 00    lda $00fb
    09FFEA: 71 00       adc ($00), y
    09FFEC: 8D FB 00    sta $00fb
    09FFEF: C8          iny
    09FFF0: D0 EC       bne $c
    09FFF2: C6 01       dec $01
    09FFF4: 10 E8       bpl $9ffde
    09FFF6: 0D FA 00    ora $00fa
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


std::unique_ptr<util::disasm_interface> xavix2000_device::create_disassembler()
{
	return std::make_unique<xavix2000_disassembler>();
}


#include "cpu/m6502/xavix2000.hxx"
