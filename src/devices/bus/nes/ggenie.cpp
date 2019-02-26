// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Galoob Game Genie PCBs


 Here we emulate the following PCBs

 * Galoob Game Genie, passthrough hacking cart

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


nes_ggenie_device::nes_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_GGENIE, tag, owner, clock)
	, m_ggslot(*this, "gg_slot")
	, m_gg_bypass(0)
{
}


void nes_ggenie_device::device_start()
{
	common_start();
	save_item(NAME(m_gg_bypass));
}

void nes_ggenie_device::pcb_start(running_machine &machine, uint8_t *ciram_ptr, bool cart_mounted)
{
	device_nes_cart_interface::pcb_start(machine, ciram_ptr, cart_mounted);
	if (m_ggslot->m_cart)
		m_ggslot->pcb_start(m_ciram);
}

void nes_ggenie_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	set_nt_mirroring(PPU_MIRROR_LOW);
	m_gg_bypass = 0;

	if (m_ggslot->m_cart)
		m_ggslot->m_cart->pcb_reset();
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Game Genie board emulation

 -------------------------------------------------*/

void nes_ggenie_device::write_h(offs_t offset, uint8_t data)
{
//  LOG_MMC(("axrom write_h, offset: %04x, data: %02x\n", offset, data));
	if (!m_gg_bypass)
	{
		// From blargg: Codes are written to $8001-800C, starting at $800C and going down to $8001.
		// Next, two values are written to $8000. The first specify the kind of codes which have
		// been inserted, the second write to $8000 is always zero (all 8 bits).
		// This probably disables the boot ROM (the code is executing from RAM at this point).
		// Once done, the code jumps to ($FFFC) to begin the game.
		// All 12 bytes from $8001-800C are written regardless of how many codes are inserted.
		// The value written to $8000 is the only clue as to what codes are actually valid and which
		// ones have compare values.
		if (offset)
		{
			int code;
			offset -= 1;
			code = (offset & 0xc) >> 2;
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
			return;
		}

		if (offset == 0 && data == 0)
		{
			m_gg_bypass = 1;
			reset_cpu();
		}
		else
		{
			// bit 0 is always set (GG enable?)
			m_gg_is_comp[0] = BIT(data, 1);
			m_gg_is_comp[1] = BIT(data, 2);
			m_gg_is_comp[2] = BIT(data, 3);
			m_gg_disable[0] = BIT(data, 4);
			m_gg_disable[1] = BIT(data, 5);
			m_gg_disable[2] = BIT(data, 6);
			// bit 7 is always clear
			logerror("Game Genie Summary:\n");
			for (int i = 0; i < 3; i++)
			{
				logerror("Code %d: %s\n", i, m_gg_disable[i] ? "No" : "Yes");
				if (!m_gg_disable[i])
				{
					logerror("\tAddr: 0x%X\n", m_gg_addr[i]);
					logerror("\tValue: 0x%X\n", m_gg_repl[i]);
					if (m_gg_is_comp[i])
						logerror("\t if equals: 0x%X\n", m_gg_comp[i]);

				}
			}
		}
	}
	else
		m_ggslot->write_h(offset, data);
}

void nes_ggenie_device::write_m(offs_t offset, uint8_t data)
{
	if (m_gg_bypass && m_ggslot)
		m_ggslot->write_m(offset, data);
}

void nes_ggenie_device::write_l(offs_t offset, uint8_t data)
{
	if (m_gg_bypass && m_ggslot)
		m_ggslot->write_l(offset, data);
}

uint8_t nes_ggenie_device::read_h(offs_t offset)
{
	if (m_gg_bypass && m_ggslot->m_cart)
	{
		uint8_t rom_value = m_ggslot->m_cart->hi_access_rom(offset);

		// check if GG code has to act on this address
		for (int i = 0; i < 3; i++)
		{
			if (!m_gg_disable[i] && offset == m_gg_addr[i])
			{
				if (!m_gg_is_comp[i] || (m_gg_is_comp[i] && m_gg_comp[i] == rom_value))
					return m_gg_repl[i];
			}
		}

		return  rom_value;
	}
	return hi_access_rom(offset);
}

uint8_t nes_ggenie_device::read_m(offs_t offset)
{
	if (m_gg_bypass && m_ggslot->m_cart)
		return m_ggslot->m_cart->read_m(offset);

	return 0xff;
}

uint8_t nes_ggenie_device::read_l(offs_t offset)
{
	if (m_gg_bypass && m_ggslot->m_cart)
		return m_ggslot->m_cart->read_l(offset);

	return 0xff;
}

void nes_ggenie_device::chr_w(offs_t offset, uint8_t data)
{
	int bank = offset >> 10;

	if (m_gg_bypass && m_ggslot->m_cart)
	{
		m_ggslot->m_cart->chr_w(offset, data);
		return;
	}

	if (m_chr_src[bank] == CHRRAM)
		m_chr_access[bank][offset & 0x3ff] = data;
}

uint8_t nes_ggenie_device::chr_r(offs_t offset)
{
	int bank = offset >> 10;

	if (m_gg_bypass && m_ggslot->m_cart)
		return  m_ggslot->m_cart->chr_r(offset);

	return m_chr_access[bank][offset & 0x3ff];
}


void nes_ggenie_device::nt_w(offs_t offset, uint8_t data)
{
	int page = ((offset & 0xc00) >> 10);

	if (m_gg_bypass && m_ggslot->m_cart)
	{
		m_ggslot->m_cart->nt_w(offset, data);
		return;
	}

	if (!m_nt_writable[page])
		return;

	m_nt_access[page][offset & 0x3ff] = data;
}

uint8_t nes_ggenie_device::nt_r(offs_t offset)
{
	int page = ((offset & 0xc00) >> 10);

	if (m_gg_bypass && m_ggslot->m_cart)
		return m_ggslot->m_cart->nt_r(offset);

	return m_nt_access[page][offset & 0x3ff];
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_ggenie_device::device_add_mconfig(machine_config &config)
{
	NES_CART_SLOT(config, "gg_slot", DERIVED_CLOCK(1, 1), nes_cart, nullptr).set_must_be_loaded(false);
}
