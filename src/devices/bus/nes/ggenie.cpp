// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Galoob Game Genie PCBs


 Here we emulate the following PCBs

 * Galoob Game Genie, pass-through cheat cartridge (NESdev: Game Genie)

 Menu mode: the 4 KiB ROM at $C000-$FFFF (mirrored), open bus at $8000-$BFFF,
 pattern tables generated from PPU A2/A4-A7, code registers at $8000-$800C.
 Game mode (until power off): everything on the cartridge connector passes
 through to the cartridge plugged into the Game Genie, except the reads that
 the three codes replace.

 Bus conflict (NESdev): for a replaced read in $C020-$FFFF the Game Genie keeps
 the plugged cartridge's /ROMSEL high, so the cartridge sees a read of
 $4020-$7FFF and whatever it has mapped there drives the bus as well. Emulated
 for cartridge RAM at $6000-$7FFF (codes at $E000-$FFFF), as the bitwise AND
 of both drivers; what a board maps at $4020-$5FFF is board specific and is
 not modelled.

 Not emulated: cartridges that decode CHR from PPU /A13 alone (MMC5) show a
 corrupted menu on hardware, as the Game Genie ignores that line.

 ***********************************************************************************************************/


#include "emu.h"
#include "ggenie.h"
#include "bus/nes/nes_carts.h"

#define LOG_HIFREQ (1U << 1)
#define LOG_INFO   (1U << 2)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_INFO | LOG_GENERAL)
#else
#define VERBOSE (LOG_INFO)
#endif
#include "logmacro.h"


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
	LOGMASKED(LOG_HIFREQ, "ggenie write_h, offset: %04x, data: %02x\n", offset, data);

	if (m_gg_bypass)
	{
		m_ggslot->write_h(offset, data);
		return;
	}

	// The ROM writes the three codes to $8001-$800C, then $8000 with bit 0 set (game mode), then $00 to $8000,
	// which reaches the plugged cartridge because the Game Genie is already in game mode, and jumps to ($FFFC).
	// The ROM also writes 0 to $FFF0, $FFF1 and $FFF0; what those do is unknown (NESdev), so they change nothing.
	if ((offset > 0) && (offset <= 0x0c))
	{
		offset -= 1;
		int const code = BIT(offset, 2, 2);

		switch (offset & 3)
		{
			case 0: // address bits 8-14 (bit 15 is always set)
				m_gg_addr[code] = (m_gg_addr[code] & 0x00ff) | ((data & 0x7f) << 8);
				break;
			case 1: // address bits 0-7
				m_gg_addr[code] = (m_gg_addr[code] & 0x7f00) | data;
				break;
			case 2:
				m_gg_comp[code] = data;
				break;
			case 3:
				m_gg_repl[code] = data;
				break;
		}
	}
	else if (offset == 0) // 0x8000
	{
		m_gg_bypass = BIT(data, 0);
		for (int i = 0; i < 3; i++)
		{
			m_gg_is_comp[i] = BIT(data, i + 1);
			m_gg_enable[i] = !BIT(data, i + 4);
		}
		// bit 7 is unused and always zero

		LOGMASKED(LOG_INFO, "Game Genie Summary:\n");
		for (int i = 0; i < 3; i++)
		{
			LOGMASKED(LOG_INFO, "Code %d: %s\n", i, m_gg_enable[i] ? "Yes" : "No");
			if (m_gg_enable[i])
			{
				LOGMASKED(LOG_INFO, "\tAddr: 0x%X\n", m_gg_addr[i]);
				LOGMASKED(LOG_INFO, "\tValue: 0x%X\n", m_gg_repl[i]);
				if (m_gg_is_comp[i])
					LOGMASKED(LOG_INFO, "\t if equals: 0x%X\n", m_gg_comp[i]);
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
				{
					// a replaced read at $E000-$FFFF is also seen by the plugged cartridge as a read of $6000-$7FFF:
					// its RAM there drives the bus too
					if ((offset >= 0x6000) && (m_ggslot->m_cart->get_prgram_size() || m_ggslot->m_cart->get_battery_size()))
						return m_gg_repl[i] & m_ggslot->read_m(offset - 0x6000);
					return m_gg_repl[i];
				}
			}
		}

		return rom_value;
	}

	// menu mode: the 4 KiB ROM at $C000-$FFFF, nothing drives the bus at $8000-$BFFF
	if (offset < 0x4000)
		return get_open_bus();
	return m_prg[offset & 0xfff];
}

u8 nes_ggenie_device::read_m(offs_t offset)
{
	// in menu mode the plugged cartridge is not passed through, and the Game Genie has nothing here
	if (m_gg_bypass)
		return m_ggslot->read_m(offset);

	return get_open_bus();
}

u8 nes_ggenie_device::read_l(offs_t offset)
{
	if (m_gg_bypass)
		return m_ggslot->read_l(offset);

	return get_open_bus();
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
	nes_cart_slot_device &slot(NES_CART_SLOT(config, "gg_slot", DERIVED_CLOCK(1, 1), nes_cart, nullptr));
	slot.set_must_be_loaded(false);
	slot.irq_callback().set(FUNC(nes_ggenie_device::gg_irq_w));   // the plugged cartridge's IRQ reaches the console through the Game Genie
	slot.add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0);   // the cartridge plugged into the Game Genie is heard through it
}
