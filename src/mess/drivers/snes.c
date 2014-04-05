// license:?
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
    - Fix mouse & superscope support
    - Add multitap support
    - Add support for other controllers

***************************************************************************/

#include "emu.h"
#include "audio/snes_snd.h"
#include "cpu/spc700/spc700.h"
#include "includes/snes.h"
#include "machine/snescx4.h"

#include "crsshair.h"

#include "bus/snes/snes_slot.h"
#include "bus/snes/snes_carts.h"

struct snes_mouse
{
	INT16 x, y, oldx, oldy;
	UINT16 buttons;
	UINT8 deltax, deltay;
	int speed;
};

struct snes_superscope
{
	INT16 x, y;
	UINT8 buttons;
	int turbo_lock, pause_lock, fire_lock;
	int offscreen;
};


class snes_console_state : public snes_state
{
public:
	enum
	{
		TIMER_LIGHTGUN_TICK = TIMER_SNES_LAST
	};

	snes_console_state(const machine_config &mconfig, device_type type, const char *tag)
			: snes_state(mconfig, type, tag)
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
	TIMER_CALLBACK_MEMBER( lightgun_tick );

	// input related
	virtual DECLARE_WRITE8_MEMBER(io_read);
	virtual UINT8 oldjoy1_read(int latched);
	virtual UINT8 oldjoy2_read(int latched);

	// pad inputs
	void input_read_joy(int port, bool multitap);
	UINT8 input_serial_pad(int port, int latched, bool multitap);

	// mouse inputs
	void input_read_mouse(int port);
	UINT8 input_serial_mouse(int port, int latched);

	// superscope inputs
	DECLARE_CUSTOM_INPUT_MEMBER( sscope_offscreen_input );
	void input_read_sscope(int port);
	UINT8 input_serial_sscope(int port, int latched);
	void gun_latch(INT16 x, INT16 y);

	virtual void machine_start();
	virtual void machine_reset();
	int m_type;
	optional_device<sns_cart_slot_device> m_cartslot;

protected:

	snes_mouse            m_mouse[2];
	snes_superscope       m_scope[2];

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
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
	AM_RANGE(0x000000, 0x7dffff) AM_READWRITE(snes20_lo_r, snes20_lo_w)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM                                     /* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xffffff) AM_READWRITE(snes20_hi_r, snes20_hi_w)
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

void snes_console_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_LIGHTGUN_TICK:
		lightgun_tick(ptr, param);
		break;
	default:
		snes_state::device_timer(timer, id, param, ptr);
		break;
	}
}

TIMER_CALLBACK_MEMBER( snes_console_state::lightgun_tick )
{
	if ((ioport("CTRLSEL")->read() & 0x0f) == 0x03 || (ioport("CTRLSEL")->read() & 0x0f) == 0x04)   {
		/* enable lightpen crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_NONE);
	}

	if ((ioport("CTRLSEL")->read() & 0xf0) == 0x30 || (ioport("CTRLSEL")->read() & 0xf0) == 0x40)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_NONE);
	}
}

static INPUT_PORTS_START( snes_joypads )

	PORT_START("JOY1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P1 Select") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start") PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button A") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button X") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button L") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button R") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)

	PORT_START("JOY2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P2 Select") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button X") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button L") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button R") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
// temp hack to share the same port both for P2 alone and P2 through MP5 adapter
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P2 Select") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button X") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button L") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button R") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)

	PORT_START("JOY3")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P3 Button B") PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P3 Button Y") PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P3 Select") PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("P3 Start") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P3 Button A") PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P3 Button X") PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P3 Button L") PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P3 Button R") PORT_PLAYER(3) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)

	PORT_START("JOY4")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P4 Button B") PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P4 Button Y") PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P4 Select") PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START4 ) PORT_NAME("P4 Start") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P4 Button A") PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P4 Button X") PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P4 Button L") PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P4 Button R") PORT_PLAYER(4) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)

	PORT_START("JOY5")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P5 Button B") PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P5 Button Y") PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("P5 Select") PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START5 ) PORT_NAME("P5 Start") PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P5 Button A") PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P5 Button X") PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P5 Button L") PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P5 Button R") PORT_PLAYER(5) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x50)

INPUT_PORTS_END

static INPUT_PORTS_START( snes_mouse )
	PORT_START("MOUSE1")
	/* bits 0,3 = mouse signature (must be 1) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	/* bits 4,5 = mouse speed: 0 = slow, 1 = normal, 2 = fast, 3 = unused */
	PORT_BIT( 0x0030, IP_ACTIVE_HIGH, IPT_UNUSED )
	/* bits 6,7 = mouse buttons */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Mouse Button Left") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Mouse Button Right") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be 0!

	PORT_START("MOUSE1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("MOUSE1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("MOUSE2")
	/* bits 0,3 = mouse signature (must be 1) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	/* bits 4,5 = mouse speed: 0 = slow, 1 = normal, 2 = fast, 3 = unused */
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	/* bits 6,7 = mouse buttons */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Mouse Button Left") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Mouse Button Right") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)

	PORT_START("MOUSE2_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)

	PORT_START("MOUSE2_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
INPUT_PORTS_END

static INPUT_PORTS_START( snes_superscope )
	PORT_START("SUPERSCOPE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )    // Noise
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snes_console_state, sscope_offscreen_input, (void *)0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Port1 Superscope Pause") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Port1 Superscope Turbo") PORT_TOGGLE PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Port1 Superscope Cursor") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Port1 Superscope Fire") PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)

	PORT_START("SUPERSCOPE1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Port1 Superscope X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)

	PORT_START("SUPERSCOPE1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_NAME("Port1 Superscope Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x03)

	PORT_START("SUPERSCOPE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )    // Noise
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, snes_console_state, sscope_offscreen_input, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Port2 Superscope Pause") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Port2 Superscope Turbo") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Port2 Superscope Cursor") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Port2 Superscope Fire") PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)

	PORT_START("SUPERSCOPE2_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Port2 Superscope X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)

	PORT_START("SUPERSCOPE2_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_NAME("Port2 Superscope Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x30)
INPUT_PORTS_END

static INPUT_PORTS_START( snes )
	PORT_START("CTRLSEL")  /* Select Controller Type */
	PORT_CONFNAME( 0x0f, 0x01, "P1 Controller")
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x01, "Gamepad" )
	PORT_CONFSETTING(  0x02, "Mouse" )
	PORT_CONFSETTING(  0x03, "Superscope" )
//  PORT_CONFSETTING(  0x04, "Justfier" )
//  PORT_CONFSETTING(  0x05, "Multitap" )
	PORT_CONFNAME( 0xf0, 0x10, "P2 Controller")
	PORT_CONFSETTING(  0x00, "Unconnected" )
	PORT_CONFSETTING(  0x10, "Gamepad" )
	PORT_CONFSETTING(  0x20, "Mouse" )
	PORT_CONFSETTING(  0x30, "Superscope" )
//  PORT_CONFSETTING(  0x40, "Justfier" )
	PORT_CONFSETTING(  0x50, "Multitap" )

	PORT_INCLUDE(snes_joypads)
	PORT_INCLUDE(snes_mouse)
	PORT_INCLUDE(snes_superscope)

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

// Mouse handling

void snes_console_state::input_read_mouse(int port)
{
	INT16 var;
	static const char *const portnames[2][3] =
	{
		{ "MOUSE1", "MOUSE1_X", "MOUSE1_Y" },
		{ "MOUSE2", "MOUSE2_X", "MOUSE2_Y" },
	};

	m_mouse[port].buttons = ioport(portnames[port][0])->read();
	m_mouse[port].x = ioport(portnames[port][1])->read();
	m_mouse[port].y = ioport(portnames[port][2])->read();

	var = m_mouse[port].x - m_mouse[port].oldx;

	if (var < -127)
	{
		m_mouse[port].deltax = 0xff;
		m_mouse[port].oldx -= 127;
	}
	else if (var < 0)
	{
		m_mouse[port].deltax = 0x80 | (-var);
		m_mouse[port].oldx = m_mouse[port].x;
	}
	else if (var > 127)
	{
		m_mouse[port].deltax = 0x7f;
		m_mouse[port].oldx += 127;
	}
	else
	{
		m_mouse[port].deltax = var & 0xff;
		m_mouse[port].oldx = m_mouse[port].x;
	}

	var = m_mouse[port].y - m_mouse[port].oldy;

	if (var < -127)
	{
		m_mouse[port].deltay = 0xff;
		m_mouse[port].oldy -= 127;
	}
	else if (var < 0)
	{
		m_mouse[port].deltay = 0x80 | (-var);
		m_mouse[port].oldy = m_mouse[port].y;
	}
	else if (var > 127)
	{
		m_mouse[port].deltay = 0x7f;
		m_mouse[port].oldy += 127;
	}
	else
	{
		m_mouse[port].deltay = var & 0xff;
		m_mouse[port].oldy = m_mouse[port].y;
	}

	m_data1[port] = m_mouse[port].buttons;
	m_data2[port] = 0;
}

UINT8 snes_console_state::input_serial_mouse(int port, int latched)
{
	UINT8 res = 0;

	if (latched)
	{
		m_mouse[port].speed = (m_mouse[port].speed + 1) % 3;
		return res;
	}

	if (m_read_idx[port] >= 32)
		res |= 0x01;
	else if (m_read_idx[port] >= 24)
		res |= (m_mouse[port].deltax >> (31 - m_read_idx[port]++)) & 0x01;
	else if (m_read_idx[port] >= 16)
		res |= (m_mouse[port].deltay >> (23 - m_read_idx[port]++)) & 0x01;
	else if (m_read_idx[port] == 11)
	{
		res |= (m_mouse[port].speed >> 0) & 0x01;
		m_read_idx[port]++;
	}
	else if (m_read_idx[port] == 10)
	{
		res |= (m_mouse[port].speed >> 1) & 0x01;
		m_read_idx[port]++;
	}
	else
		res |= (m_mouse[port].buttons >> (15 - m_read_idx[port]++)) & 0x01;

	return res;
}

// Superscope handling

CUSTOM_INPUT_MEMBER( snes_console_state::sscope_offscreen_input )
{
	int port = (FPTR)param;
	static const char *const portnames[2][3] =
	{
		{ "SUPERSCOPE1", "SUPERSCOPE1_X", "SUPERSCOPE1_Y" },
		{ "SUPERSCOPE2", "SUPERSCOPE2_X", "SUPERSCOPE2_Y" },
	};

	INT16 x = ioport(portnames[port][1])->read();
	INT16 y = ioport(portnames[port][2])->read();

	/* these are the theoretical boundaries, but we currently are always onscreen... */
	if (x < 0 || x >= SNES_SCR_WIDTH || y < 0 || y >= m_ppu.m_beam.last_visible_line)
		m_scope[port].offscreen = 1;
	else
		m_scope[port].offscreen = 0;

	return m_scope[port].offscreen;
}


void snes_console_state::gun_latch(INT16 x, INT16 y)
{
	/* these are the theoretical boundaries */
	if (x < 0)
		x = 0;
	if (x > (SNES_SCR_WIDTH - 1))
		x = SNES_SCR_WIDTH - 1;

	if (y < 0)
		y = 0;
	if (y > (m_ppu.m_beam.last_visible_line - 1))
		y = m_ppu.m_beam.last_visible_line - 1;

	m_ppu.m_beam.latch_horz = x;
	m_ppu.m_beam.latch_vert = y;
	m_ppu.m_stat78 |= 0x40;
}

void snes_console_state::input_read_sscope(int port)
{
	static const char *const portnames[2][3] =
	{
		{ "SUPERSCOPE1", "SUPERSCOPE1_X", "SUPERSCOPE1_Y" },
		{ "SUPERSCOPE2", "SUPERSCOPE2_X", "SUPERSCOPE2_Y" },
	};
	UINT8 input;

	/* first read input bits */
	m_scope[port].x = ioport(portnames[port][1])->read();
	m_scope[port].y = ioport(portnames[port][2])->read();
	input = ioport(portnames[port][0])->read();

	/* then start elaborating input bits: only keep old turbo value */
	m_scope[port].buttons &= 0x20;

	/* set onscreen/offscreen */
	m_scope[port].buttons |= BIT(input, 1);

	/* turbo is a switch; toggle is edge sensitive */
	if (BIT(input, 5) && !m_scope[port].turbo_lock)
	{
		m_scope[port].buttons ^= 0x20;
		m_scope[port].turbo_lock = 1;
	}
	else if (!BIT(input, 5))
		m_scope[port].turbo_lock = 0;

	/* fire is a button; if turbo is active, trigger is level sensitive; otherwise it is edge sensitive */
	if (BIT(input, 7) && (BIT(m_scope[port].buttons, 5) || !m_scope[port].fire_lock))
	{
		m_scope[port].buttons |= 0x80;
		m_scope[port].fire_lock = 1;
	}
	else if (!BIT(input, 7))
		m_scope[port].fire_lock = 0;

	/* cursor is a button; it is always level sensitive */
	m_scope[port].buttons |= BIT(input, 6);

	/* pause is a button; it is always edge sensitive */
	if (BIT(input, 4) && !m_scope[port].pause_lock)
	{
		m_scope[port].buttons |= 0x10;
		m_scope[port].pause_lock = 1;
	}
	else if (!BIT(input, 4))
		m_scope[port].pause_lock = 0;

	/* If we have pressed fire or cursor and we are on-screen and SuperScope is in Port2, then latch video signal.
	 Notice that we only latch Port2 because its IOBit pin is connected to bit7 of the IO Port, while Port1 has
	 IOBit pin connected to bit6 of the IO Port, and the latter is not detected by the H/V Counters. In other
	 words, you can connect SuperScope to Port1, but there is no way SNES could detect its on-screen position */
	if ((m_scope[port].buttons & 0xc0) && !(m_scope[port].buttons & 0x02) && port == 1)
		gun_latch(m_scope[port].x, m_scope[port].y);

	m_data1[port] = 0xff | (m_scope[port].buttons << 8);
	m_data2[port] = 0;
}

UINT8 snes_console_state::input_serial_sscope(int port, int latched)
{
	UINT8 res = 0;

	if (m_read_idx[port] >= 8)
		res |= 0x01;
	else
		res |= (m_scope[port].buttons >> (7 - m_read_idx[port]++)) & 0x01;

	return res;
}

// Joypad + Multitap handling
// input_read_joy is used both for standard joys and for the MP5 multitap

void snes_console_state::input_read_joy( int port, bool multitap )
{
	static const char *const portnames[4][2] =
			{
				{ "JOY1", "JOY3" },
				{ "JOY2", "JOY3" },
				{ "JOY4", "JOY5" },
				{ "JOY4", "JOY5" }
			};

	if (!multitap)
	{
		m_data1[port] = ioport(portnames[port][0])->read();
		m_data2[port] = 0;
		// avoid sending signals that could crash games
		// if left, no right
		if (m_data1[port] & 0x200)
			m_data1[port] &= ~0x100;
		// if up, no down
		if (m_data1[port] & 0x800)
			m_data1[port] &= ~0x400;
		// if left, no right
		if (m_data2[port] & 0x200)
			m_data2[port] &= ~0x100;
		// if up, no down
		if (m_data2[port] & 0x800)
			m_data2[port] &= ~0x400;
	}
	else
	{
		m_data1[port] = ioport(portnames[port][0])->read();
		m_data2[port] = ioport(portnames[port][1])->read();
		m_data1[port + 2] = ioport(portnames[port + 2][0])->read();
		m_data2[port + 2] = ioport(portnames[port + 2][1])->read();
		// avoid sending signals that could crash games
		// if left, no right
		if (m_data1[port] & 0x200)
			m_data1[port] &= ~0x100;
		// if up, no down
		if (m_data1[port] & 0x800)
			m_data1[port] &= ~0x400;
		// if left, no right
		if (m_data2[port] & 0x200)
			m_data2[port] &= ~0x100;
		// if up, no down
		if (m_data2[port] & 0x800)
			m_data2[port] &= ~0x400;
		// if left, no right
		if (m_data1[port + 2] & 0x200)
			m_data1[port + 2] &= ~0x100;
		// if up, no down
		if (m_data1[port + 2] & 0x800)
			m_data1[port + 2] &= ~0x400;
		// if left, no right
		if (m_data2[port + 2] & 0x200)
			m_data2[port + 2] &= ~0x100;
		// if up, no down
		if (m_data2[port + 2] & 0x800)
			m_data2[port + 2] &= ~0x400;
	}
}

UINT8 snes_console_state::input_serial_pad(int port, int latched, bool multitap)
{
	UINT8 res = 0;

	// multitap signature? Super Bomberman 3-5 do not like this at all...
	if (multitap)
		res |= 2;
	if (latched)
		return res;

	if (!multitap)
	{
		if (m_read_idx[port] >= 16)
			res |= 0x01;
		else
		{
			res |= (BIT(m_data1[port], (15 - m_read_idx[port])));
			res |= (BIT(m_data2[port], (15 - m_read_idx[port])) << 1);
			m_read_idx[port]++;
		}
	}
	else
	{
		int shift = !(SNES_CPU_REG(WRIO) & 0x80) ? 2 : 0;
		if (m_read_idx[port + shift] >= 16)
			res |= 0x03;
		else
		{
			res |= (BIT(m_data1[port + shift], (15 - m_read_idx[port + shift])));
			res |= (BIT(m_data2[port + shift], (15 - m_read_idx[port + shift])) << 1);
			m_read_idx[port + shift]++;
		}
	}
	return res;
}


// input handling from the system side

WRITE8_MEMBER(snes_console_state::io_read)
{
	UINT8 ctrl1 = ioport("CTRLSEL")->read() & 0x0f;
	UINT8 ctrl2 = (ioport("CTRLSEL")->read() & 0xf0) >> 4;
	bool multitap0 = FALSE;
	bool multitap1 = FALSE;

	// Check if lightgun has been chosen as input: if so, enable crosshair
	timer_set(attotime::zero, TIMER_LIGHTGUN_TICK);

	switch (ctrl1)
	{
		case 1: // SNES joypad
			input_read_joy(0, FALSE);
			break;
		case 2: // SNES Mouse
			input_read_mouse(0);
			break;
		case 3: // SNES Superscope
			input_read_sscope(0);
			break;
		case 5: // SNES joypad(s) through MP5 multitap
			input_read_joy(0, TRUE);
			multitap0 = TRUE;
			break;
		case 0: // empty port1
		default:
			m_data1[0] = 0;
			m_data2[0] = 0;
			break;
	}

	switch (ctrl2)
	{
		case 1: // SNES joypad
			input_read_joy(1, FALSE);
			break;
		case 2: // SNES Mouse
			input_read_mouse(1);
			break;
		case 3: // SNES Superscope
			input_read_sscope(1);
			break;
		case 5: // SNES joypad(s) through MP5 multitap
			input_read_joy(1, TRUE);
			multitap1 = TRUE;
			break;
		case 0: // empty port2
		default:
			m_data1[1] = 0;
			m_data2[1] = 0;
			break;
	}

	// is automatic reading on? if so, copy port data1/data2 to joy1l->joy4h
	// this actually works like reading the first 16bits from oldjoy1/2 in reverse order
	if (SNES_CPU_REG(NMITIMEN) & 1)
	{
		int shift0 = (multitap0 && !(SNES_CPU_REG(WRIO) & 0x80)) ? 2 : 0;
		int shift1 = (multitap1 && !(SNES_CPU_REG(WRIO) & 0x80)) ? 2 : 0;

		SNES_CPU_REG(JOY1L) = (m_data1[0 + shift0] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY1H) = (m_data1[0 + shift0] & 0xff00) >> 8;
		SNES_CPU_REG(JOY2L) = (m_data1[1 + shift1] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY2H) = (m_data1[1 + shift1] & 0xff00) >> 8;
		SNES_CPU_REG(JOY3L) = (m_data2[0 + shift0] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY3H) = (m_data2[0 + shift0] & 0xff00) >> 8;
		SNES_CPU_REG(JOY4L) = (m_data2[1 + shift1] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY4H) = (m_data2[1 + shift1] & 0xff00) >> 8;

		// make sure read_idx starts returning all 1s because the auto-read reads it :-)
		m_read_idx[0 + shift0] = 16;
		m_read_idx[1 + shift1] = 16;
	}
}

UINT8 snes_console_state::oldjoy1_read(int latched)
{
	UINT8 ctrl1 = ioport("CTRLSEL")->read() & 0x0f;

	switch (ctrl1)
	{
		case 1: // SNES joypad
			return input_serial_pad(0, latched, FALSE);

		case 2: // SNES Mouse
			return input_serial_mouse(0, latched);

		case 3: // SNES Superscope
			return input_serial_sscope(0, latched);

		case 5: // SNES multipad
			return input_serial_pad(0, latched, TRUE);

		case 0: // empty port1
		default:
			return 0;
	}
}

UINT8 snes_console_state::oldjoy2_read(int latched)
{
	UINT8 ctrl2 = (ioport("CTRLSEL")->read() & 0xf0) >> 4;

	switch (ctrl2)
	{
		case 1: // SNES joypad
			return input_serial_pad(1, latched, FALSE);

		case 2: // SNES Mouse
			return input_serial_mouse(1, latched);

		case 3: // SNES Superscope
			return input_serial_sscope(1, latched);

		case 5: // SNES multipad
			return input_serial_pad(1, latched, TRUE);

		case 0: // empty port1
		default:
			return 0;
	}
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void snes_console_state::machine_start()
{
	snes_state::machine_start();

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
//          m_maincpu->space(AS_PROGRAM).install_read_handler(0x808000, 0x80ffff, 0, 0x780000, read8_delegate(FUNC(base_sns_cart_slot_device::chip_read),(base_sns_cart_slot_device*)m_cartslot));
//          m_maincpu->space(AS_PROGRAM).install_write_handler(0x808000, 0x80ffff, 0, 0x780000, write8_delegate(FUNC(base_sns_cart_slot_device::chip_write),(base_sns_cart_slot_device*)m_cartslot));
//          m_maincpu->set_5a22_map();
			break;
	}

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_mouse[i].x), i);
		save_item(NAME(m_mouse[i].oldx), i);
		save_item(NAME(m_mouse[i].y), i);
		save_item(NAME(m_mouse[i].oldy), i);
		save_item(NAME(m_mouse[i].buttons), i);
		save_item(NAME(m_mouse[i].deltax), i);
		save_item(NAME(m_mouse[i].deltay), i);
		save_item(NAME(m_mouse[i].speed), i);
		save_item(NAME(m_scope[i].x), i);
		save_item(NAME(m_scope[i].y), i);
		save_item(NAME(m_scope[i].buttons), i);
		save_item(NAME(m_scope[i].turbo_lock), i);
		save_item(NAME(m_scope[i].pause_lock), i);
		save_item(NAME(m_scope[i].fire_lock), i);
		save_item(NAME(m_scope[i].offscreen), i);
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

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("spc700", SNES, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_SNS_CARTRIDGE_ADD("snsslot", snes_cart, NULL)
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
CONS( 1989, snes,      0,      0,     snes,      snes, driver_device,  0,    "Nintendo", "Super Nintendo Entertainment System / Super Famicom (NTSC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
CONS( 1991, snespal,   snes,   0,     snespal,   snes, driver_device,  0,    "Nintendo", "Super Nintendo Entertainment System (PAL)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
