// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Galoob Game Genie PCBs


 Here we emulate the following PCBs

 * Galoob Game Genie, passthrough hacking cart

 TODO: emulate bugs/quirks/bus conflicts. See NesDev wiki.

 ***********************************************************************************************************/


#include "emu.h"
#include "ggenie.h"
#include "bus/nes/nes_carts.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_GGENIE, nes_ggenie_device, "nes_ggenie", "NES Cart Game Genie PCB")


nes_ggenie_device::nes_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_GGENIE, tag, owner, clock)
	, m_ggslot(*this, "gg_slot")
	, m_gg_bypass(false)
{
}


void nes_ggenie_device::device_start()
{
	common_start();
	save_item(NAME(m_gg_bypass));
	save_item(NAME(m_gg_addr));
	save_item(NAME(m_gg_repl));
	save_item(NAME(m_gg_comp));
	save_item(NAME(m_gg_enable));
	save_item(NAME(m_gg_is_comp));
}

void nes_ggenie_device::pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted)
{
	device_nes_cart_interface::pcb_start(machine, ciram_ptr, cart_mounted);
	m_ggslot->pcb_start(m_ciram);

	prg32(0);
	m_gg_bypass = false;

	for (int i = 0; i < 3; i++)
	{
		m_gg_addr[i] = 0;
		m_gg_repl[i] = 0;
		m_gg_comp[i] = 0;
		m_gg_enable[i] = false;
		m_gg_is_comp[i] = false;
	}
}

void nes_ggenie_device::pcb_reset()
{
	// Game Genie does NOT reset to its interface once in game mode
	m_ggslot->pcb_reset();
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Game Genie board emulation

 -------------------------------------------------*/

void nes_ggenie_device::write_h(offs_t offset, u8 data)
{
//  LOG_MMC(("ggenie write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_gg_bypass)
	{
		m_ggslot->write_h(offset, data);
		return;
	}

	// From blargg: Codes are written to $8001-800C, starting at $800C and going down to $8001.
	// Next, two values are written to $8000. The first specify the kind of codes which have
	// been inserted, and disables the boot ROM. The second write to $8000 is always 0x00, and
	// goes to the slave cart for unknown reasons.
	// Once done, the code jumps to ($FFFC) to begin the game.
	// All 12 bytes from $8001-800C are written regardless of how many codes are inserted.
	// The value written to $8000 is the only clue as to what codes are actually valid and which
	// ones have compare values.
	if (offset)
	{
		offset -= 1;
		int code = BIT(offset, 2, 2);
		if (code == 3)
			return; // how did we end up here? the GG is not expected to write to $800d-800f!

		switch (offset & 3)
		{
			case 0:
				m_gg_addr[code] |= (data & 0x7f) << 8;
				break;
			case 1:
				m_gg_addr[code] = data;
				break;
			case 2:
				m_gg_comp[code] = data;
				break;
			case 3:
				m_gg_repl[code] = data;
				break;
		}
	}
	else // 0x8000
	{
		m_gg_bypass = BIT(data, 0);
		for (int i = 0; i < 3; i++)
		{
			m_gg_is_comp[i] = BIT(data, i + 1);
			m_gg_enable[i] = !BIT(data, i + 4);
		}
		// bit 7 is unused and always zero

		logerror("Game Genie Summary:\n");
		for (int i = 0; i < 3; i++)
		{
			logerror("Code %d: %s\n", i, m_gg_enable[i] ? "Yes" : "No");
			if (m_gg_enable[i])
			{
				logerror("\tAddr: 0x%X\n", m_gg_addr[i]);
				logerror("\tValue: 0x%X\n", m_gg_repl[i]);
				if (m_gg_is_comp[i])
					logerror("\t if equals: 0x%X\n", m_gg_comp[i]);
			}
		}
	}
}

void nes_ggenie_device::write_m(offs_t offset, u8 data)
{
	if (m_gg_bypass)
		m_ggslot->write_m(offset, data);
}

void nes_ggenie_device::write_l(offs_t offset, u8 data)
{
	if (m_gg_bypass)
		m_ggslot->write_l(offset, data);
}

u8 nes_ggenie_device::read_h(offs_t offset)
{
	if (m_gg_bypass && m_ggslot->m_cart)
	{
		u8 rom_value = m_ggslot->m_cart->hi_access_rom(offset);

		// check if GG code has to act on this address
		for (int i = 0; i < 3; i++)
		{
			if (m_gg_enable[i] && offset == m_gg_addr[i])
			{
				if (!m_gg_is_comp[i] || m_gg_comp[i] == rom_value)
					return m_gg_repl[i];
			}
		}

		return rom_value;
	}
	return m_prg[offset & 0xfff]; // ROM is only 4K
}

u8 nes_ggenie_device::read_m(offs_t offset)
{
	if (m_gg_bypass)
		return m_ggslot->read_m(offset);

	return 0xff;
}

u8 nes_ggenie_device::read_l(offs_t offset)
{
	if (m_gg_bypass)
		return m_ggslot->read_l(offset);

	return 0xff;
}

void nes_ggenie_device::chr_w(offs_t offset, u8 data)
{
	if (m_gg_bypass && m_ggslot->m_cart)
		m_ggslot->m_cart->chr_w(offset, data);
}

u8 nes_ggenie_device::chr_r(offs_t offset)
{
	if (m_gg_bypass && m_ggslot->m_cart)
		return m_ggslot->m_cart->chr_r(offset);
	else
	{
		// there is no on-board CHRROM, 1 of 4 values are generated based on PPU A2, A4, A5, A6, A7, resulting in 16 tiles
		static constexpr u8 chr_lut[4] = { 0x00, 0xf0, 0x0f, 0xff };
		return chr_lut[BIT(offset, BIT(offset, 2) ? 4 : 6, 2)];
	}
}


void nes_ggenie_device::nt_w(offs_t offset, u8 data)
{
	if (m_gg_bypass && m_ggslot->m_cart)
		m_ggslot->m_cart->nt_w(offset, data);
	else
		device_nes_cart_interface::nt_w(offset, data);
}

u8 nes_ggenie_device::nt_r(offs_t offset)
{
	if (m_gg_bypass && m_ggslot->m_cart)
		return m_ggslot->m_cart->nt_r(offset);
	else
		return device_nes_cart_interface::nt_r(offset);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_ggenie_device::device_add_mconfig(machine_config &config)
{
	NES_CART_SLOT(config, "gg_slot", DERIVED_CLOCK(1, 1), nes_cart, nullptr).set_must_be_loaded(false);
}
