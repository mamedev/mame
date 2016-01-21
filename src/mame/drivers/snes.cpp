// license:BSD-3-Clause
// copyright-holders:Angelo Salese, R. Belmont, Anthony Kruize, Fabio Priuli
/***************************************************************************

  snes.c

  Driver file to handle emulation of the Nintendo Super NES.

  Based on original MESS driver by Lee Hammerton (aka Savoury Snax),
  later contributions by
   R. Belmont
   Anthony Kruize
   Angelo Salese
   Fabio Priuli
   byuu (extensive RE of both SNES and add-on chips)


  Todo (in no particular order):
    - Fix sync between 5A22, SPC700 and PPU
    - Fix remaining sound and video bugs
    - Fix vertical mosaic effects
    - Add support for real CX4 and ST018 CPUs
    - Add support for SA-1 and SuperGB add-ons
    - Fix superscope support
    - Add support for other controllers

***************************************************************************/

#include "emu.h"
#include "audio/snes_snd.h"
#include "cpu/spc700/spc700.h"
#include "includes/snes.h"
#include "machine/snescx4.h"

#include "bus/snes/snes_slot.h"
#include "bus/snes/snes_carts.h"
#include "bus/snes_ctrl/ctrl.h"
#include "softlist.h"

class snes_console_state : public snes_state
{
public:
	snes_console_state(const machine_config &mconfig, device_type type, const char *tag)
			: snes_state(mconfig, type, tag)
			, m_ctrl1(*this, "ctrl1")
			, m_ctrl2(*this, "ctrl2")
			, m_cartslot(*this, "snsslot")
	{ }

	DECLARE_READ8_MEMBER( snes20_hi_r );
	DECLARE_WRITE8_MEMBER( snes20_hi_w );
	DECLARE_READ8_MEMBER( snes20_lo_r );
	DECLARE_WRITE8_MEMBER( snes20_lo_w );
	DECLARE_READ8_MEMBER( snes21_lo_r );
	DECLARE_WRITE8_MEMBER( snes21_lo_w );
	DECLARE_READ8_MEMBER( snes21_hi_r );
	DECLARE_WRITE8_MEMBER( snes21_hi_w );
	DECLARE_READ8_MEMBER( snessfx_hi_r );
	DECLARE_READ8_MEMBER( snessfx_lo_r );
	DECLARE_WRITE8_MEMBER( snessfx_hi_w );
	DECLARE_WRITE8_MEMBER( snessfx_lo_w );
	DECLARE_READ8_MEMBER( snessa1_hi_r );
	DECLARE_READ8_MEMBER( snessa1_lo_r );
	DECLARE_WRITE8_MEMBER( snessa1_hi_w );
	DECLARE_WRITE8_MEMBER( snessa1_lo_w );
	DECLARE_READ8_MEMBER( snes7110_hi_r );
	DECLARE_READ8_MEMBER( snes7110_lo_r );
	DECLARE_WRITE8_MEMBER( snes7110_hi_w );
	DECLARE_WRITE8_MEMBER( snes7110_lo_w );
	DECLARE_READ8_MEMBER( snessdd1_lo_r );
	DECLARE_WRITE8_MEMBER( snessdd1_lo_w );
	DECLARE_READ8_MEMBER( snessdd1_hi_r );
	DECLARE_WRITE8_MEMBER( snessdd1_hi_w );
	DECLARE_READ8_MEMBER( snesbsx_hi_r );
	DECLARE_WRITE8_MEMBER( snesbsx_hi_w );
	DECLARE_READ8_MEMBER( snesbsx_lo_r );
	DECLARE_WRITE8_MEMBER( snesbsx_lo_w );
	DECLARE_READ8_MEMBER( snessgb_hi_r );
	DECLARE_READ8_MEMBER( snessgb_lo_r );
	DECLARE_WRITE8_MEMBER( snessgb_hi_w );
	DECLARE_WRITE8_MEMBER( snessgb_lo_w );
	DECLARE_READ8_MEMBER( pfest94_hi_r );
	DECLARE_WRITE8_MEMBER( pfest94_hi_w );
	DECLARE_READ8_MEMBER( pfest94_lo_r );
	DECLARE_WRITE8_MEMBER( pfest94_lo_w );

	DECLARE_READ8_MEMBER( spc_ram_100_r );
	DECLARE_WRITE8_MEMBER( spc_ram_100_w );

	// input related
	SNESCTRL_ONSCREEN_CB(onscreen_cb);
	SNESCTRL_GUNLATCH_CB(gun_latch_cb);
	virtual DECLARE_WRITE8_MEMBER(io_read) override;
	virtual UINT8 oldjoy1_read(int latched) override;
	virtual UINT8 oldjoy2_read(int latched) override;
	virtual void write_joy_latch(UINT8 data) override;
	virtual void wrio_write(UINT8 data) override;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	int m_type;
	required_device<snes_control_port_device> m_ctrl1;
	required_device<snes_control_port_device> m_ctrl2;
	optional_device<sns_cart_slot_device> m_cartslot;
};


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

// SPC access

READ8_MEMBER(snes_console_state::spc_ram_100_r )
{
	return m_spc700->spc_ram_r(space, offset + 0x100);
}

WRITE8_MEMBER(snes_console_state::spc_ram_100_w )
{
	m_spc700->spc_ram_w(space, offset + 0x100, data);
}

// Memory access for the various types of carts

//---------------------------------------------------------------------------------
// LoROM & LoROM + BSX slot & LoROM + some add-on chips
//---------------------------------------------------------------------------------

// In general LoROM games have perfect mirror between 0x00-0x7d and 0x80-0xff
// But BSX+LoROM games use different read handlers (to access ROM beyond 2MB)
// so we use two different set of handlers...

// Also we have here LoROM + CX4, until the Hitachi CPU is emulated,
// and the LoROM + Seta DSP, because their chip_read/chip_write need global offset

READ8_MEMBER( snes_console_state::snes20_hi_r )
{
	UINT16 address = offset & 0xffff;

	// take care of add-on IO
	if ((m_cartslot->get_type() == SNES_ST010 || m_cartslot->get_type() == SNES_ST011)
		&& (offset >= 0x600000 && offset < 0x680000 && (offset & 0xffff) < 0x4000))
		return m_cartslot->chip_read(space, offset);
	else if ((m_cartslot->get_type() == SNES_ST010 || m_cartslot->get_type() == SNES_ST011)
				&& (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x8000))
		return m_cartslot->chip_read(space, offset);
	else if (m_cartslot->get_type() == SNES_CX4
				&& (offset < 0x400000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))    // hack until we emulate the real CPU
		return CX4_read((offset & 0xffff) - 0x6000);

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
			return snes_r_io(space, address);
		else if (address < 0x8000)
			return snes_open_bus_r(space, 0);
		else
			return m_cartslot->read_h(space, offset);
	}
	else if (offset < 0x700000)
	{
		if (address < 0x8000)
			return snes_open_bus_r(space, 0);
		else
			return m_cartslot->read_h(space, offset);
	}
	else
	{
		if (m_type == SNES_SUFAMITURBO && address >= 0x8000 && offset < 0x740000)
			return m_cartslot->read_h(space, offset);

		// here usually there is SRAM mirrored in the whole range, but if ROM is very large then arrives here too (see tokimeki and wizardg4)
		if (m_cartslot->m_cart->get_rom_size() > 0x200000 && address >= 0x8000)
			return m_cartslot->read_h(space, offset);
		else
		{
			if (m_cartslot->m_cart->get_nvram_size() > 0x8000)
			{
				// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
				offset = ((offset - 0x700000) / 0x10000) * 0x8000 + (offset & 0x7fff);
				return m_cartslot->read_ram(space, offset);
			}
			else if (m_cartslot->m_cart->get_nvram_size() > 0)
				return m_cartslot->read_ram(space, offset);
			else
				return snes_open_bus_r(space, 0);
		}
	}
}

WRITE8_MEMBER( snes_console_state::snes20_hi_w )
{
	UINT16 address = offset & 0xffff;

	// take care of add-on IO
	if ((m_cartslot->get_type() == SNES_ST010 || m_cartslot->get_type() == SNES_ST011)
		&& (offset >= 0x600000 && offset < 0x680000 && (offset & 0xffff) < 0x4000))
	{ m_cartslot->chip_write(space, offset, data); return; }
	else if ((m_cartslot->get_type() == SNES_ST010 || m_cartslot->get_type() == SNES_ST011)
				&& (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x8000))
	{ m_cartslot->chip_write(space, offset, data); return; }
	else if (m_cartslot->get_type() == SNES_CX4
				&& (offset < 0x400000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))    // hack until we emulate the real CPU
	{ CX4_write(space.machine(), (offset & 0xffff) - 0x6000, data); return; }
	else if (m_type == SNES_SUFAMITURBO
				&& address >= 0x8000 && ((offset >= 0x600000 && offset < 0x640000) || (offset >= 0x700000 && offset < 0x740000)))
	{ m_cartslot->write_h(space, offset, data); return; }

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
			snes_w_io(space, address, data);
	}
	else if (offset >= 0x700000 && (m_cartslot->m_cart->get_rom_size() <= 0x200000 || address < 0x8000))    // NVRAM access
	{
		if (m_cartslot->m_cart->get_nvram_size() > 0x8000)
		{
			// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
			offset = ((offset - 0x700000) / 0x10000) * 0x8000 + (offset & 0x7fff);
			m_cartslot->write_ram(space, offset, data);
		}
		else if (m_cartslot->m_cart->get_nvram_size() > 0)
			m_cartslot->write_ram(space, offset, data);
	}
}

READ8_MEMBER( snes_console_state::snes20_lo_r )
{
	UINT16 address = offset & 0xffff;

	// take care of add-on IO
	if ((m_cartslot->get_type() == SNES_ST010 /*|| m_cartslot->get_type() == SNES_ST011*/) // why does this break moritash?
		&& (offset >= 0x600000 && offset < 0x680000 && (offset & 0xffff) < 0x4000))
		return m_cartslot->chip_read(space, offset);
	else if ((m_cartslot->get_type() == SNES_ST010 || m_cartslot->get_type() == SNES_ST011)
				&& (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x8000))
		return m_cartslot->chip_read(space, offset);
	else if (m_cartslot->get_type() == SNES_CX4
				&& (offset < 0x400000 && (offset & 0xffff) >= 0x6000 && (offset & 0xffff) < 0x8000))    // hack until we emulate the real CPU
		return CX4_read((offset & 0xffff) - 0x6000);

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
			return snes_r_io(space, address);
		else if (address < 0x8000)
			return snes_open_bus_r(space, 0);
		else
			return m_cartslot->read_l(space, offset);
	}
	else if (offset < 0x700000)
	{
		if (address < 0x8000)
			return snes_open_bus_r(space, 0);
		else
			return m_cartslot->read_l(space, offset);
	}
	else
	{
		if (m_type == SNES_SUFAMITURBO && address >= 0x8000 && offset < 0x740000)
			return m_cartslot->read_l(space, offset);

		// here usually there is SRAM mirrored in the whole range, but if ROM is very large then arrives here too (see tokimeki and wizardg4)
		if (m_cartslot->m_cart->get_rom_size() > 0x200000 && address >= 0x8000)
			return m_cartslot->read_l(space, offset);
		else
		{
			if (m_cartslot->m_cart->get_nvram_size() > 0x8000)
			{
				// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
				offset = ((offset - 0x700000) / 0x10000) * 0x8000 + (offset & 0x7fff);
				return m_cartslot->read_ram(space, offset);
			}
			else if (m_cartslot->m_cart->get_nvram_size() > 0)
				return m_cartslot->read_ram(space, offset);
			else
				return snes_open_bus_r(space, 0);
		}
	}
}

WRITE8_MEMBER( snes_console_state::snes20_lo_w )
{
	if (m_type == SNES_SUFAMITURBO
		&& (offset & 0xffff) >= 0x8000 && ((offset >= 0x600000 && offset < 0x640000) || (offset >= 0x700000 && offset < 0x740000)))
	{ m_cartslot->write_l(space, offset, data); return; }

	// other add-on writes matches the hi handler
	snes20_hi_w(space, offset, data, 0xff);
}


//---------------------------------------------------------------------------------
// HiROM & HiROM + BSX slot
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::snes21_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000 && address < 0x8000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
			return snes_r_io(space, address);
		else
		{
			if (m_type == SNES_BSXHI && m_cartslot->m_cart->get_nvram_size() && offset >= 0x200000)
			{
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff;
				return m_cartslot->read_ram(space, (offset - 0x6000) & mask);
			}

			if (m_cartslot->m_cart->get_nvram_size() && offset >= 0x300000)
			{
				/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
				/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff; /* Limit SRAM size to what's actually present */
				return m_cartslot->read_ram(space, (offset - 0x6000) & mask);
			}
			else
				return snes_open_bus_r(space, 0);
		}
	}

	// ROM access
	return m_cartslot->read_l(space, offset);
}

WRITE8_MEMBER( snes_console_state::snes21_lo_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000 && address < 0x8000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
			snes_w_io(space, address, data);
		else
		{
			if (m_type == SNES_BSXHI && m_cartslot->m_cart->get_nvram_size() && offset >= 0x200000)
			{
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff;
				m_cartslot->write_ram(space, (offset - 0x6000) & mask, data);
				return;
			}
			if (m_cartslot->m_cart->get_nvram_size() && offset >= 0x300000)
			{
				/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
				/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff; /* Limit SRAM size to what's actually present */
				m_cartslot->write_ram(space, (offset - 0x6000) & mask, data);
			}
		}
	}
}

READ8_MEMBER( snes_console_state::snes21_hi_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000 && address < 0x8000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
			return snes_r_io(space, address);
		else
		{
			if (m_type == SNES_BSXHI && m_cartslot->m_cart->get_nvram_size() && offset >= 0x200000)
			{
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff;
				return m_cartslot->read_ram(space, (offset - 0x6000) & mask);
			}

			if (m_cartslot->m_cart->get_nvram_size() && offset >= 0x300000)
			{
				/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
				/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff; /* Limit SRAM size to what's actually present */
				return m_cartslot->read_ram(space, (offset - 0x6000) & mask);
			}
			else
				return snes_open_bus_r(space, 0);
		}
	}

	// ROM access
	return m_cartslot->read_h(space, offset);
}

WRITE8_MEMBER( snes_console_state::snes21_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000 && address < 0x8000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
			snes_w_io(space, address, data);
		else
		{
			if (m_type == SNES_BSXHI && m_cartslot->m_cart->get_nvram_size() && offset >= 0x200000)
			{
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff;
				m_cartslot->write_ram(space, (offset - 0x6000) & mask, data);
				return;
			}
			if (m_cartslot->m_cart->get_nvram_size() && offset >= 0x300000)
			{
				/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
				/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
				int mask = (m_cartslot->m_cart->get_nvram_size() - 1) & 0x7fff; /* Limit SRAM size to what's actually present */
				m_cartslot->write_ram(space, (offset - 0x6000) & mask, data);
			}
		}
	}
}

//---------------------------------------------------------------------------------
// LoROM + SuperFX / GSU
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::snessfx_hi_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			if (address >= 0x3000 && address < 0x3300)
				return m_cartslot->chip_read(space, offset);
			else
				return snes_r_io(space, address);
		}
		else if (address < 0x8000)
			return m_cartslot->read_ram(space, offset & 0x1fff);
		else
			return m_cartslot->read_h(space, offset);
	}
	else if (offset < 0x600000)
		return m_cartslot->read_h(space, offset);
	else
		return m_cartslot->read_ram(space, offset);
}

READ8_MEMBER( snes_console_state::snessfx_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			if (address >= 0x3000 && address < 0x3300)
				return m_cartslot->chip_read(space, offset);
			else
				return snes_r_io(space, address);
		}
		else if (address < 0x8000)
			return m_cartslot->read_ram(space, offset & 0x1fff);
		else
			return m_cartslot->read_l(space, offset);
	}
	else if (offset < 0x600000)
		return m_cartslot->read_l(space, offset);
	else
		return m_cartslot->read_ram(space, offset);
}

WRITE8_MEMBER( snes_console_state::snessfx_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
		{
			if (address >= 0x3000 && address < 0x3300)
				m_cartslot->chip_write(space, offset, data);
			else
				snes_w_io(space, address, data);
		}
		else if (address < 0x8000)
			m_cartslot->write_ram(space, offset & 0x1fff, data);
	}
	else if (offset >= 0x600000)
		m_cartslot->write_ram(space, offset, data);
}

WRITE8_MEMBER( snes_console_state::snessfx_lo_w )
{
	snessfx_hi_w(space, offset, data, 0xff);
}

//---------------------------------------------------------------------------------
// LoROM + SA-1
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::snessa1_hi_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			if (address >= 0x2200 && address < 0x2400)
				return m_cartslot->chip_read(space, offset);    // SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				return m_cartslot->chip_read(space, offset);    // Internal SA-1 RAM (2K)
			else
				return snes_r_io(space, address);
		}
		else if (address < 0x8000)
			return m_cartslot->chip_read(space, offset);        // SA-1 BWRAM
		else
			return m_cartslot->read_h(space, offset);
	}
	else
		return m_cartslot->read_h(space, offset);
}

READ8_MEMBER( snes_console_state::snessa1_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			if (address >= 0x2200 && address < 0x2400)
				return m_cartslot->chip_read(space, offset);    // SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				return m_cartslot->chip_read(space, offset);    // Internal SA-1 RAM (2K)
			else
				return snes_r_io(space, address);
		}
		else if (address < 0x8000)
			return m_cartslot->chip_read(space, offset);        // SA-1 BWRAM
		else
			return m_cartslot->read_l(space, offset);
	}
	else if (offset < 0x500000)
		return m_cartslot->chip_read(space, offset);        // SA-1 BWRAM (not mirrored above!)
	else
		return snes_r_io(space, address);                   // nothing mapped here!
}

WRITE8_MEMBER( snes_console_state::snessa1_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
		{
			if (address >= 0x2200 && address < 0x2400)
				m_cartslot->chip_write(space, offset, data);    // SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				m_cartslot->chip_write(space, offset, data);    // Internal SA-1 RAM (2K)
			else
				snes_w_io(space, address, data);
		}
		else if (address < 0x8000)
			m_cartslot->chip_write(space, offset, data);        // SA-1 BWRAM
	}
}

WRITE8_MEMBER( snes_console_state::snessa1_lo_w )
{
	if (offset >= 0x400000 && offset < 0x500000)
		m_cartslot->chip_write(space, offset, data);        // SA-1 BWRAM (not mirrored above!)
	else
		snessa1_hi_w(space, offset, data, 0xff);
}

//---------------------------------------------------------------------------------
// HiROM + SPC-7110
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::snes7110_hi_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			UINT16 limit = (m_cartslot->get_type() == SNES_SPC7110_RTC) ? 0x4843 : 0x4840;
			if (address >= 0x4800 && address < limit)
				return m_cartslot->chip_read(space, address);

			return snes_r_io(space, address);
		}
		else if (address < 0x8000)
		{
			if (offset < 0x10000)
				return m_cartslot->read_ram(space, offset);
			if (offset >= 0x300000 && offset < 0x310000)
				return m_cartslot->read_ram(space, offset);
		}
		else
			return m_cartslot->read_h(space, offset);
	}
	return m_cartslot->read_h(space, offset);
}

READ8_MEMBER( snes_console_state::snes7110_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			UINT16 limit = (m_cartslot->get_type() == SNES_SPC7110_RTC) ? 0x4843 : 0x4840;
			if (address >= 0x4800 && address < limit)
				return m_cartslot->chip_read(space, address);

			return snes_r_io(space, address);
		}
		else if (address < 0x8000)
		{
			if (offset < 0x10000)
				return m_cartslot->read_ram(space, offset);
			if (offset >= 0x300000 && offset < 0x310000)
				return m_cartslot->read_ram(space, offset);
		}
		else
			return m_cartslot->read_l(space, offset);
	}
	if (offset >= 0x500000 && offset < 0x510000)
		return m_cartslot->chip_read(space, 0x4800);

	return snes_open_bus_r(space, 0);
}

WRITE8_MEMBER( snes_console_state::snes7110_hi_w )
{
	snes7110_lo_w(space, offset, data, 0xff);
}

WRITE8_MEMBER( snes_console_state::snes7110_lo_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
		{
			UINT16 limit = (m_cartslot->get_type() == SNES_SPC7110_RTC) ? 0x4843 : 0x4840;
			if (address >= 0x4800 && address < limit)
			{
				m_cartslot->chip_write(space, address, data);
				return;
			}
			snes_w_io(space, address, data);
		}
		else if (address < 0x8000)
		{
			if (offset < 0x10000)
				m_cartslot->write_ram(space, offset, data);
			if (offset >= 0x300000 && offset < 0x310000)
				m_cartslot->write_ram(space, offset, data);
		}
	}
}


//---------------------------------------------------------------------------------
// LoROM + S-DD1
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::snessdd1_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			if (address >= 0x4800 && address < 0x4808)
				return m_cartslot->chip_read(space, address);

			return snes_r_io(space, address);
		}
		else if (address < 0x8000)
			return snes_open_bus_r(space, 0);
		else
			return m_cartslot->read_l(space, offset);
	}
	else if (offset >= 0x700000 && address < 0x8000 && m_cartslot->m_cart->get_nvram_size())    // NVRAM access
		return m_cartslot->read_ram(space, offset);
	else    // ROM access
		return m_cartslot->read_l(space, offset);
}

READ8_MEMBER( snes_console_state::snessdd1_hi_r )
{
	if (offset >= 0x400000)
		return m_cartslot->read_h(space, offset);
	else
		return snessdd1_lo_r(space, offset, 0xff);
}

WRITE8_MEMBER( snes_console_state::snessdd1_lo_w )
{
	snessdd1_hi_w(space, offset, data, 0xff);
}

WRITE8_MEMBER( snes_console_state::snessdd1_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
		{
			if (address >= 0x4300 && address < 0x4380)
			{
				m_cartslot->chip_write(space, address, data);
				// here we don't return, but we let the w_io happen...
			}
			if (address >= 0x4800 && address < 0x4808)
			{
				m_cartslot->chip_write(space, address, data);
				return;
			}
			snes_w_io(space, address, data);
		}
	}
	if (offset >= 0x700000 && address < 0x8000 && m_cartslot->m_cart->get_nvram_size())
		return m_cartslot->write_ram(space, offset, data);
}


//---------------------------------------------------------------------------------
// LoROM + BS-X (Base unit)
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::snesbsx_hi_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			if (address >= 0x2188 && address < 0x21a0)
				return m_cartslot->chip_read(space, offset);
			if (address >= 0x5000)
				return m_cartslot->chip_read(space, offset);
			return snes_r_io(space, address);
		}
		else if (address < 0x8000)
		{
			if (offset >= 0x200000)
				return m_cartslot->read_h(space, offset);
			else
				return snes_open_bus_r(space, 0);
		}
		else
			return m_cartslot->read_h(space, offset);
	}
	return m_cartslot->read_h(space, offset);
}

WRITE8_MEMBER( snes_console_state::snesbsx_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
		{
			if (address >= 0x2188 && address < 0x21a0)
			{
				m_cartslot->chip_write(space, offset, data);
				return;
			}
			if (address >= 0x5000)
			{
				m_cartslot->chip_write(space, offset, data);
				return;
			}
			snes_w_io(space, address, data);
		}
		else if (address < 0x8000)
		{
			if (offset >= 0x200000)
				return m_cartslot->write_l(space, offset, data);
		}
		else
			return m_cartslot->write_l(space, offset, data);
	}
	return m_cartslot->write_l(space, offset, data);
}

READ8_MEMBER( snes_console_state::snesbsx_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
		{
			if (address >= 0x2188 && address < 0x21a0)
				return m_cartslot->chip_read(space, offset);
			if (address >= 0x5000)
				return m_cartslot->chip_read(space, offset);
			return snes_r_io(space, address);
		}
		else if (address < 0x8000)
		{
			if (offset >= 0x200000)
				return m_cartslot->read_l(space, offset);
			else
				return snes_open_bus_r(space, 0);
		}
		else
			return m_cartslot->read_l(space, offset);
	}
	return m_cartslot->read_l(space, offset);
}

WRITE8_MEMBER( snes_console_state::snesbsx_lo_w )
{
	snesbsx_hi_w(space, offset, data, 0xff);
}


//---------------------------------------------------------------------------------
// LoROM + SuperGB
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::snessgb_hi_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
			return snes_r_io(space, address);
		else if (address < 0x8000)
			return m_cartslot->chip_read(space, offset);
		else
			return m_cartslot->read_h(space, offset);
	}
	else if (address >= 0x8000)
		return m_cartslot->read_h(space, offset);

	return snes_open_bus_r(space, 0);
}

READ8_MEMBER( snes_console_state::snessgb_lo_r )
{
	return snessgb_hi_r(space, offset, 0xff);
}

WRITE8_MEMBER( snes_console_state::snessgb_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
			snes_w_io(space, address, data);
		else if (address < 0x8000)
			m_cartslot->chip_write(space, offset, data);
	}
}

WRITE8_MEMBER( snes_console_state::snessgb_lo_w )
{
	snessgb_hi_w(space, offset, data, 0xff);
}

//---------------------------------------------------------------------------------
// Powerfest '94 event cart
//---------------------------------------------------------------------------------

READ8_MEMBER( snes_console_state::pfest94_hi_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
			return snes_r_io(space, address);
		else if (address < 0x8000)
		{
			if (offset < 0x100000)    // DSP access
				return m_cartslot->chip_read(space, offset);
			else if (offset == 0x106000)    // menu access
				return m_cartslot->chip_read(space, offset + 0x8000);
			else if (offset >= 0x300000 && m_cartslot->m_cart->get_nvram_size())    // NVRAM access
				return m_cartslot->read_ram(space, offset);
			else
				return snes_open_bus_r(space, 0);
		}
		else
			return m_cartslot->read_h(space, offset);
	}
	return m_cartslot->read_h(space, offset);
}

WRITE8_MEMBER( snes_console_state::pfest94_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x2000)
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)
			snes_w_io(space, address, data);
		else if (address < 0x8000)
		{
			if (offset < 0x100000)    // DSP access
				m_cartslot->chip_write(space, offset, data);
			else if (offset == 0x206000)    // menu access
				m_cartslot->chip_write(space, offset + 0x8000, data);
			else if (offset >= 0x300000 && m_cartslot->m_cart->get_nvram_size())    // NVRAM access
				m_cartslot->write_ram(space, offset, data);
		}
	}
}

READ8_MEMBER( snes_console_state::pfest94_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)
			return space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)
			return snes_r_io(space, address);
		else if (address < 0x8000)
		{
			if (offset < 0x100000)    // DSP access
				return m_cartslot->chip_read(space, offset);
			else if (offset == 0x106000)    // menu access
				return m_cartslot->chip_read(space, offset + 0x8000);
			else if (offset >= 0x300000 && m_cartslot->m_cart->get_nvram_size())    // NVRAM access
				return m_cartslot->read_ram(space, offset);
			else
				return snes_open_bus_r(space, 0);
		}
		else
			return m_cartslot->read_l(space, offset);
	}
	return 0xff;    // or open_bus?
}

WRITE8_MEMBER( snes_console_state::pfest94_lo_w )
{
	pfest94_hi_w(space, offset, data, 0xff);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( snes_map, AS_PROGRAM, 8, snes_console_state )
//  AM_RANGE(0x000000, 0x7dffff) AM_READWRITE(snes20_lo_r, snes20_lo_w)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM                                     /* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
//  AM_RANGE(0x800000, 0xffffff) AM_READWRITE(snes20_hi_r, snes20_hi_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spc_map, AS_PROGRAM, 8, snes_console_state )
	AM_RANGE(0x0000, 0x00ef) AM_DEVREADWRITE("spc700", snes_sound_device, spc_ram_r, spc_ram_w) /* lower 32k ram */
	AM_RANGE(0x00f0, 0x00ff) AM_DEVREADWRITE("spc700", snes_sound_device, spc_io_r, spc_io_w)   /* spc io */
	AM_RANGE(0x0100, 0xffff) AM_READWRITE(spc_ram_100_r, spc_ram_100_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( snes )
	// input devices go through slot options
	PORT_START("OPTIONS")
	PORT_CONFNAME( 0x01, 0x00, "Hi-Res pixels blurring (TV effect)")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

#if SNES_LAYER_DEBUG
	PORT_START("DEBUG1")
	PORT_CONFNAME( 0x03, 0x00, "Select BG1 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x01, "BG1B (lower) only" )
	PORT_CONFSETTING(    0x02, "BG1A (higher) only" )
	PORT_CONFNAME( 0x0c, 0x00, "Select BG2 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x04, "BG2B (lower) only" )
	PORT_CONFSETTING(    0x08, "BG2A (higher) only" )
	PORT_CONFNAME( 0x30, 0x00, "Select BG3 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "BG3B (lower) only" )
	PORT_CONFSETTING(    0x20, "BG3A (higher) only" )
	PORT_CONFNAME( 0xc0, 0x00, "Select BG4 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x40, "BG4B (lower) only" )
	PORT_CONFSETTING(    0x80, "BG4A (higher) only" )

	PORT_START("DEBUG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 1") PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 2") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 3") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 4") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Objects") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Main/Sub") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Color Math") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Windows") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_START("DEBUG3")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mosaic") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x70, 0x00, "Select OAM priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "OAM0 only" )
	PORT_CONFSETTING(    0x20, "OAM1 only" )
	PORT_CONFSETTING(    0x30, "OAM2 only" )
	PORT_CONFSETTING(    0x40, "OAM3 only" )
	PORT_CONFNAME( 0x80, 0x00, "Draw sprite in reverse order" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DEBUG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 0 draw") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 1 draw") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 2 draw") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 3 draw") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 4 draw") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 5 draw") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 6 draw") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 7 draw") PORT_TOGGLE
#endif
INPUT_PORTS_END


/*************************************
 *
 *  Input callbacks
 *
 *************************************/

WRITE8_MEMBER(snes_console_state::io_read)
{
	// is automatic reading on? if so, read 16bits from oldjoy1/2
	if (SNES_CPU_REG(NMITIMEN) & 1)
	{
		UINT16 joy1 = 0, joy2 = 0, joy3 = 0, joy4 = 0;
		m_ctrl1->port_poll();
		m_ctrl2->port_poll();

		for (int i = 0; i < 16; i++)
		{
			joy1 |= ((m_ctrl1->read_pin4() & 1) << (15 - i));
			joy2 |= ((m_ctrl2->read_pin4() & 1) << (15 - i));
			joy3 |= ((m_ctrl1->read_pin5() & 1) << (15 - i));
			joy4 |= ((m_ctrl2->read_pin5() & 1) << (15 - i));
		}

		SNES_CPU_REG(JOY1L) = (joy1 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY1H) = (joy1 & 0xff00) >> 8;
		SNES_CPU_REG(JOY2L) = (joy2 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY2H) = (joy2 & 0xff00) >> 8;
		SNES_CPU_REG(JOY3L) = (joy3 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY3H) = (joy3 & 0xff00) >> 8;
		SNES_CPU_REG(JOY4L) = (joy4 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY4H) = (joy4 & 0xff00) >> 8;
	}
}

UINT8 snes_console_state::oldjoy1_read(int latched)
{
	UINT8 ret = 0;
	ret |= m_ctrl1->read_pin4();
	ret |= (m_ctrl1->read_pin5() << 1);
	return ret;
}

UINT8 snes_console_state::oldjoy2_read(int latched)
{
	UINT8 ret = 0;
	ret |= m_ctrl2->read_pin4();
	ret |= (m_ctrl2->read_pin5() << 1);
	return ret;
}

void snes_console_state::write_joy_latch(UINT8 data)
{
	m_ctrl1->write_strobe(data);
	m_ctrl2->write_strobe(data);
}

void snes_console_state::wrio_write(UINT8 data)
{
	if (!(SNES_CPU_REG(WRIO) & 0x80) && (data & 0x80))
	{
		// external latch
		m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
	}

	m_ctrl1->write_pin6(BIT(data, 6));
	m_ctrl2->write_pin6(BIT(data, 7));

}

SNESCTRL_GUNLATCH_CB(snes_console_state::gun_latch_cb)
{
	// these are the theoretical boundaries
	if (x < 0)
		x = 0;
	if (x > (SNES_SCR_WIDTH - 1))
		x = SNES_SCR_WIDTH - 1;

	if (y < 0)
		y = 0;
	if (y > (m_ppu->m_beam.last_visible_line - 1))
		y = m_ppu->m_beam.last_visible_line - 1;

//  m_ppu->set_latch_hv(x, y);  // it would be more accurate to write twice to WRIO register, first with bit7 = 0 and then with bit7 = 1
	m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
}

SNESCTRL_ONSCREEN_CB(snes_console_state::onscreen_cb)
{
	// these are the theoretical boundaries, but we currently are always onscreen due to the
	// way IPT_LIGHTGUNs work... investigate more on this!
	if (x < 0 || x >= SNES_SCR_WIDTH || y < 0 || y >= m_ppu->m_beam.last_visible_line)
		return false;
	else
		return true;
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void snes_console_state::machine_start()
{
	snes_state::machine_start();

	if (m_cartslot && m_cartslot->exists())
	{
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snes20_lo_r),this), write8_delegate(FUNC(snes_console_state::snes20_lo_w),this));
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snes20_hi_r),this), write8_delegate(FUNC(snes_console_state::snes20_hi_w),this));
		m_maincpu->set_5a22_map();

		m_type = m_cartslot->get_type();

		switch (m_type)
		{
			// LoROM & LoROM + addons
			case SNES_MODE20:
			case SNES_BSXLO:
			case SNES_SUFAMITURBO:
			case SNES_CX4:      // this still uses the old simulation instead of emulating the CPU
			case SNES_ST010:    // this requires two diff kinds of chip access, so we handle it in snes20_lo/hi_r/w
			case SNES_ST011:    // this requires two diff kinds of chip access, so we handle it in snes20_lo/hi_r/w
			case SNES_ST018:    // still unemulated
				break;
			case SNES_Z80GB:      // skeleton support
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snessgb_lo_r),this), write8_delegate(FUNC(snes_console_state::snessgb_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snessgb_hi_r),this), write8_delegate(FUNC(snes_console_state::snessgb_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			case SNES_SA1:      // skeleton support
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snessa1_lo_r),this), write8_delegate(FUNC(snes_console_state::snessa1_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snessa1_hi_r),this), write8_delegate(FUNC(snes_console_state::snessa1_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			case SNES_DSP:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x208000, 0x20ffff, 0, 0x9f0000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x208000, 0x20ffff, 0, 0x9f0000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_DSP_2MB:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x600000, 0x607fff, 0, 0x8f0000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x600000, 0x607fff, 0, 0x8f0000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_DSP4:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x308000, 0x30ffff, 0, 0x8f0000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x308000, 0x30ffff, 0, 0x8f0000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_OBC1:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x006000, 0x007fff, 0, 0xbf0000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x006000, 0x007fff, 0, 0xbf0000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_SFX:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snessfx_lo_r),this), write8_delegate(FUNC(snes_console_state::snessfx_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snessfx_hi_r),this), write8_delegate(FUNC(snes_console_state::snessfx_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			case SNES_SDD1:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snessdd1_lo_r),this), write8_delegate(FUNC(snes_console_state::snessdd1_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snessdd1_hi_r),this), write8_delegate(FUNC(snes_console_state::snessdd1_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			case SNES_BSX:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snesbsx_lo_r),this), write8_delegate(FUNC(snes_console_state::snesbsx_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snesbsx_hi_r),this), write8_delegate(FUNC(snes_console_state::snesbsx_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			// HiROM & HiROM + addons
			case SNES_MODE21:
			case SNES_BSXHI:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snes21_lo_r),this), write8_delegate(FUNC(snes_console_state::snes21_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snes21_hi_r),this), write8_delegate(FUNC(snes_console_state::snes21_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			case SNES_DSP_MODE21:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snes21_lo_r),this), write8_delegate(FUNC(snes_console_state::snes21_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snes21_hi_r),this), write8_delegate(FUNC(snes_console_state::snes21_hi_w),this));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x006000, 0x007fff, 0, 0x9f0000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x006000, 0x007fff, 0, 0x9f0000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->set_5a22_map();
				break;
			case SNES_SRTC:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snes21_lo_r),this), write8_delegate(FUNC(snes_console_state::snes21_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snes21_hi_r),this), write8_delegate(FUNC(snes_console_state::snes21_hi_w),this));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x002800, 0x002800, 0, 0xbf0000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x002801, 0x002801, 0, 0xbf0000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->set_5a22_map();
				break;
			case SNES_SPC7110:
			case SNES_SPC7110_RTC:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::snes7110_lo_r),this), write8_delegate(FUNC(snes_console_state::snes7110_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::snes7110_hi_r),this), write8_delegate(FUNC(snes_console_state::snes7110_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			case SNES_PFEST94:
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7dffff, read8_delegate(FUNC(snes_console_state::pfest94_lo_r),this), write8_delegate(FUNC(snes_console_state::pfest94_lo_w),this));
				m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x800000, 0xffffff, read8_delegate(FUNC(snes_console_state::pfest94_hi_r),this), write8_delegate(FUNC(snes_console_state::pfest94_hi_w),this));
				m_maincpu->set_5a22_map();
				break;
			// pirate 'mappers'
			case SNES_POKEMON:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x800000, 0x80ffff, 0, 0x780000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x800000, 0x80ffff, 0, 0x780000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_TEKKEN2:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x808000, 0x8087ff, 0, 0x3f0000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x808000, 0x8087ff, 0, 0x3f0000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_MCPIR1:
			case SNES_MCPIR2:
				m_maincpu->space(AS_PROGRAM).install_write_handler(0xffff00, 0xffffff, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_20COL:
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x008000, 0x008fff, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
				break;
			case SNES_SOULBLAD:
				// reads from xxx0-xxx3in range [80-bf] return a fixed sequence of 4bits; reads in range [c0-ff] return open bus
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x808000, 0x808003, 0, 0x3f7ff0, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0xc00000, 0xffffff, read8_delegate(FUNC(snes_console_state::snes_open_bus_r),this));
				break;
			case SNES_BUGS:
			case SNES_BANANA:
//              m_maincpu->space(AS_PROGRAM).install_read_handler(0x808000, 0x80ffff, 0, 0x780000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
//              m_maincpu->space(AS_PROGRAM).install_write_handler(0x808000, 0x80ffff, 0, 0x780000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
//              m_maincpu->set_5a22_map();
				break;
		}
		m_cartslot->save_ram();
	}
}

void snes_console_state::machine_reset()
{
	snes_state::machine_reset();
}


static MACHINE_CONFIG_START( snes, snes_console_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", _5A22, MCLK_NTSC)   /* 2.68 MHz, also 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(snes_map)

	MCFG_CPU_ADD("soundcpu", SPC700, 1024000)   /* 1.024 MHz */
	MCFG_CPU_PROGRAM_MAP(spc_map)

	//MCFG_QUANTUM_TIME(attotime::from_hz(48000))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC)
	MCFG_SCREEN_UPDATE_DRIVER( snes_state, screen_update )

	MCFG_DEVICE_ADD("ppu", SNES_PPU, 0)
	MCFG_SNES_PPU_OPENBUS_CB(READ8(snes_state, snes_open_bus_r))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_SNES_CONTROL_PORT_ADD("ctrl1", snes_control_port_devices, "joypad")
	MCFG_SNESCTRL_ONSCREEN_CB(snes_console_state, onscreen_cb)
	MCFG_SNES_CONTROL_PORT_ADD("ctrl2", snes_control_port_devices, "joypad")
	MCFG_SNESCTRL_ONSCREEN_CB(snes_console_state, onscreen_cb)
	MCFG_SNESCTRL_GUNLATCH_CB(snes_console_state, gun_latch_cb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("spc700", SNES, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_SNS_CARTRIDGE_ADD("snsslot", snes_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","snes")
	MCFG_SOFTWARE_LIST_ADD("bsx_list","snes_bspack")
	MCFG_SOFTWARE_LIST_ADD("st_list","snes_strom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snespal, snes )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( MCLK_PAL )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_PAL, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_PAL, 0, SNES_SCR_HEIGHT_PAL)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( snes )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */
ROM_END

#define rom_snespal rom_snes

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/*    YEAR  NAME       PARENT  COMPAT MACHINE    INPUT                 INIT  COMPANY     FULLNAME                                      FLAGS */
CONS( 1989, snes,      0,      0,     snes,      snes, driver_device,  0,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 1991, snespal,   snes,   0,     snespal,   snes, driver_device,  0,    "Nintendo", "Super Nintendo Entertainment System (PAL)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
