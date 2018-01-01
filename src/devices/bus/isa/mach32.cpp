// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mach32.c
 *
 *  Implementation of the ATi mach32 and mach64 video chips
 *  Based on ati_vga and mach8
 *
 *  Created on: 16/05/2014
 */

#include "emu.h"
#include "mach32.h"

#include "screen.h"

DEFINE_DEVICE_TYPE(ATIMACH32,       mach32_device,       "mach32",       "ATi mach32")
DEFINE_DEVICE_TYPE(ATIMACH32_8514A, mach32_8514a_device, "mach32_8514a", "ATi mach32 (2D acceleration module)")
DEFINE_DEVICE_TYPE(ATIMACH64,       mach64_device,       "mach64",       "ATi mach64")
DEFINE_DEVICE_TYPE(ATIMACH64_8514A, mach64_8514a_device, "mach64_8514a", "ATi mach64 (2D acceleration module)")


/*
 *  mach32
 */

// 8514/A device
mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach32_8514a_device(mconfig, ATIMACH32_8514A, tag, owner, clock)
{
}

mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach8_device(mconfig, type, tag, owner, clock), m_chip_ID(0), m_membounds(0)
{
}


// SVGA device
mach32_device::mach32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach32_device(mconfig, ATIMACH32, tag, owner, clock)
{
}

mach32_device::mach32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ati_vga_device(mconfig, type, tag, owner, clock), m_8514a(*this,"8514a")
{
}

MACHINE_CONFIG_MEMBER( mach32_device::device_add_mconfig )
	MCFG_DEVICE_ADD("8514a", ATIMACH32_8514A, 0)
	MCFG_EEPROM_SERIAL_93C56_ADD("ati_eeprom")
MACHINE_CONFIG_END

void mach32_8514a_device::device_config_complete()
{
	m_vga = dynamic_cast<svga_device*>(owner());
}

void mach32_8514a_device::device_start()
{
	mach8_device::device_start();
	//    017h  68800-AX
	//    177h  68800-LX
	//    2F7h  68800-6
	//  The 68800-3 appears to return 0 for this field (undocumented)
	m_chip_ID = 0x17;
	m_membounds = 0;
}

// Configuration Status Register 1 (read only)
// bit 0:     Disable VGA: 0=VGA+8514/A, 1=8514/A only
// bits 1-3:  Bus Type:  0=16-bit ISA. 1=EISA, 2=16-bit MCA, 3=32-bit MCA, 4=LBus 386SX
//                       5=LBus 386DX, 6=LBus 486. 7=PCI
// bits 4-6:  RAM Type:  3=256Kx16 DRAM
// bit 7:     Chip Disable
// bit 8:     TST_VCTR_ENA:  1=delay memory write by 1/2 MCLK to test vector generation
// bits 9-11: DAC Type:  0=ATI68830, 1=SC11483, 2=ATI68875, 3=Bt476, 4=Bt481, 5=ATI68860 (68800AX or higher)
//                       The Graphics Ultra Pro has an ATI68875
// bit 12:    Enable internal uC address decode
// bit 13-15: Card ID:  ID when using multiple controllers
READ16_MEMBER(mach32_8514a_device::mach32_config1_r)
{
	return 0x0430;  // enable VGA, 16-bit ISA, 256Kx16 DRAM, ATI68875
}

// mach32 Hardware Pointer
WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_l_w)
{
	if(offset == 1)
		m_cursor_address = (m_cursor_address & 0xf0000) | data;
}

WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_h_w)
{
	if(offset == 1)
	{
		m_cursor_address = (m_cursor_address & 0x0ffff) | ((data & 0x000f) << 16);
		m_cursor_enable = data & 0x8000;
		if(m_cursor_enable) popmessage("mach32 Hardware Cursor enabled");
	}
}

WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_pos_h)
{
	if(offset == 1)
		m_cursor_horizontal = data & 0x07ff;
}

WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_pos_v)
{
	if(offset == 1)
		m_cursor_vertical = data & 0x0fff;
}

WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_colour_b_w)
{
	if(offset == 1)
	{
		m_cursor_colour0_b = data & 0xff;
		m_cursor_colour1_b = data >> 8;
	}
}

WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_colour_0_w)
{
	if(offset == 1)
	{
		m_cursor_colour0_g = data & 0xff;
		m_cursor_colour0_r = data >> 8;
	}
}

WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_colour_1_w)
{
	if(offset == 1)
	{
		m_cursor_colour1_g = data & 0xff;
		m_cursor_colour1_r = data >> 8;
	}
}

WRITE16_MEMBER(mach32_8514a_device::mach32_cursor_offset_w)
{
	if(offset == 1)
	{
		m_cursor_horizontal = data & 0x00ff;
		m_cursor_vertical = data >> 8;
	}
}

void mach32_8514a_device::device_reset()
{
}

void mach32_device::device_start()
{
	ati_vga_device::device_start();
	ati.vga_chip_id = 0x00;  // correct?
}

void mach32_device::device_reset()
{
	ati_vga_device::device_reset();
}

/*
 *   mach64
 */

// 8514/A device
mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach64_8514a_device(mconfig, ATIMACH64_8514A, tag, owner, clock)
{
}

mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach32_8514a_device(mconfig, type, tag, owner, clock)
{
}


// SVGA device
mach64_device::mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach64_device(mconfig, ATIMACH64, tag, owner, clock)
{
}

mach64_device::mach64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach32_device(mconfig, type, tag, owner, clock), m_8514a(*this, "8514a")
{
}

MACHINE_CONFIG_MEMBER( mach64_device::device_add_mconfig )
	MCFG_DEVICE_ADD("8514a", ATIMACH64_8514A, 0)
	MCFG_EEPROM_SERIAL_93C56_ADD("ati_eeprom")
MACHINE_CONFIG_END

void mach64_8514a_device::device_config_complete()
{
	m_vga = dynamic_cast<svga_device*>(owner());
}

void mach64_8514a_device::device_start()
{
	mach32_8514a_device::device_start();
	//    017h  68800-AX
	//    177h  68800-LX
	//    2F7h  68800-6
	//  The 68800-3 appears to return 0 for this field (undocumented)
	m_chip_ID = 0x0000;  // value is unknown for mach64
	m_membounds = 0;
}

void mach64_8514a_device::device_reset()
{
}

void mach64_device::device_start()
{
	mach32_device::device_start();
}

void mach64_device::device_reset()
{
	mach32_device::device_reset();
}
