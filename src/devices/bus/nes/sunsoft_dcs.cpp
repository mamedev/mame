// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Sunsoft DCS PCBs


 Here we emulate the Sunsoft Dual Cassette System (DCS) PCB used by Nantettate!! Baseball + subslot


 ***********************************************************************************************************/


#include "emu.h"
#include "sunsoft_dcs.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-----------------------------------------------
//
//  Nantettate!! Baseball Cartslot implementation
//
//-----------------------------------------------

//-------------------------------------------------
//  sub-cart interface
//-------------------------------------------------

ntb_cart_interface::ntb_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_rom(nullptr)
{
}

ntb_cart_interface::~ntb_cart_interface()
{
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NTB_SLOT, nes_ntb_slot_device, "nes_ntb_slot", "NES NTB Cartridge Slot")

nes_ntb_slot_device::nes_ntb_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NES_NTB_SLOT, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, device_slot_interface(mconfig, *this)
	, m_cart(nullptr)
{
}

nes_ntb_slot_device::~nes_ntb_slot_device()
{
}


void nes_ntb_slot_device::device_start()
{
	m_cart = dynamic_cast<ntb_cart_interface *>(get_card_device());
}

uint8_t nes_ntb_slot_device::read(offs_t offset)
{
	if (m_cart)
		return m_cart->read(offset);

	return 0xff;
}


image_init_result nes_ntb_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *ROM = m_cart->get_cart_base();

		if (!ROM)
			return image_init_result::FAIL;

		if (!loaded_through_softlist())
		{
			if (length() != 0x4000)
				return image_init_result::FAIL;

			fread(&ROM, 0x4000);
		}
		else
		{
			if (get_software_region_length("rom") != 0x4000)
				return image_init_result::FAIL;

			memcpy(ROM, get_software_region("rom"), 0x4000);
		}
	}

	return image_init_result::PASS;
}


std::string nes_ntb_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("ntbrom");
}

//-----------------------------------------------
//
//  Nantettate!! Baseball Minicart implementation
//
//-----------------------------------------------

ROM_START( ntb_rom )
	ROM_REGION(0x4000, "ntbrom", ROMREGION_ERASEFF)
ROM_END

DEFINE_DEVICE_TYPE(NES_NTB_ROM, nes_ntb_rom_device, "nes_ntbrom", "NES NTB ROM")

nes_ntb_rom_device::nes_ntb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NES_NTB_ROM, tag, owner, clock)
	, ntb_cart_interface(mconfig, *this)
{
}

void nes_ntb_rom_device::device_start()
{
	m_rom = (uint8_t*)memregion("ntbrom")->base();
}

const tiny_rom_entry *nes_ntb_rom_device::device_rom_region() const
{
	return ROM_NAME( ntb_rom );
}

uint8_t *nes_ntb_rom_device::get_cart_base()
{
	return m_rom;
}



//------------------------------------------------
//
//  Nantettate!! Baseball base cart implementation
//  a.k.a. Sunsoft Dual Cassette System
//  (variant of Sunsoft-4 PCB)
//
//------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SUNSOFT_DCS, nes_sunsoft_dcs_device, "nes_dcs", "NES Cart Sunsoft DCS PCB")


nes_sunsoft_dcs_device::nes_sunsoft_dcs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sunsoft_4_device(mconfig, NES_SUNSOFT_DCS, tag, owner, clock), m_timer_on(0), m_exrom_enable(0)
	, m_subslot(*this, "ntb_slot"), ntb_enable_timer(nullptr)
{
}


void nes_sunsoft_dcs_device::device_start()
{
	common_start();
	ntb_enable_timer = timer_alloc(TIMER_PROTECT);
	ntb_enable_timer->reset();
	timer_freq = clocks_to_attotime(107520);

	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
	save_item(NAME(m_reg));
	save_item(NAME(m_wram_enable));
	save_item(NAME(m_exrom_enable));
	save_item(NAME(m_timer_on));
}

void nes_sunsoft_dcs_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_reg = 0;
	m_latch1 = 0;
	m_latch2 = 0;
	m_wram_enable = 0;
	m_exrom_enable = 0;
	m_timer_on = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Sunsoft-DCS board emulation

 Games: Nantettatte!! Baseball

 This board feature a subslot to load minicart containing
 updated data?

 iNES: mapper 68

 -------------------------------------------------*/

void nes_sunsoft_dcs_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("Sunsoft DCS write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x7000:
			prg16_89ab(data & 0x07);
			m_exrom_enable = !BIT(data, 3);
			m_wram_enable = BIT(data, 4);
			break;
		default:
			sun4_write(offset, data);
			break;
	}
}

uint8_t nes_sunsoft_dcs_device::read_h(offs_t offset)
{
	LOG_MMC(("Sunsoft DCS read_h, offset: %04x\n", offset));

	if (m_exrom_enable && m_subslot->m_cart && offset < 0x4000)
	{
		if (m_timer_on)
			return m_subslot->m_cart->read(offset);
		else
			return get_open_bus();   // after the timer is off, this returns open bus...
	}
	else
		return hi_access_rom(offset);
}

void nes_sunsoft_dcs_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("Sunsoft DCS write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_battery.empty() && m_wram_enable)
		m_battery[offset & (m_battery.size() - 1)] = data;
	if (!m_prgram.empty() && m_wram_enable)
		m_prgram[offset & (m_prgram.size() - 1)] = data;
	if (!m_wram_enable && !m_timer_on)
	{
		m_timer_on = 1;
		// start NTB-ROM timer
		ntb_enable_timer->adjust(timer_freq);
	}
}

uint8_t nes_sunsoft_dcs_device::read_m(offs_t offset)
{
	LOG_MMC(("Sunsoft DCS read_m, offset: %04x\n", offset));

	if (!m_battery.empty() && m_wram_enable)
		return m_battery[offset & (m_battery.size() - 1)];
	if (!m_prgram.empty() && m_wram_enable)
		return m_prgram[offset & (m_prgram.size() - 1)];

	return get_open_bus();   // open bus
}


static void ntb_cart(device_slot_interface &device)
{
	device.option_add_internal("ntbrom", NES_NTB_ROM);
}




//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_sunsoft_dcs_device::device_add_mconfig(machine_config &config)
{
	NES_NTB_SLOT(config, m_subslot, ntb_cart);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void nes_sunsoft_dcs_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_PROTECT)
	{
		m_timer_on = 0;
		ntb_enable_timer->reset();
	}
}
