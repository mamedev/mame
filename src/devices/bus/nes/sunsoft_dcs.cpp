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
					: device_slot_card_interface(mconfig, device),
						m_rom(nullptr)
{
}

ntb_cart_interface::~ntb_cart_interface()
{
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

const device_type NES_NTB_SLOT = &device_creator<nes_ntb_slot_device>;

nes_ntb_slot_device::nes_ntb_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, NES_NTB_SLOT, "NES NTB Cartridge Slot", tag, owner, clock, "nes_ntb_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this), m_cart(nullptr)
{
}

nes_ntb_slot_device::~nes_ntb_slot_device()
{
}


void nes_ntb_slot_device::device_start()
{
	m_cart = dynamic_cast<ntb_cart_interface *>(get_card_device());
}

READ8_MEMBER(nes_ntb_slot_device::read)
{
	if (m_cart)
		return m_cart->read(space, offset, mem_mask);

	return 0xff;
}


bool nes_ntb_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM = m_cart->get_cart_base();

		if (!ROM)
			return IMAGE_INIT_FAIL;

		if (software_entry() == nullptr)
		{
			if (length() != 0x4000)
				return IMAGE_INIT_FAIL;

			fread(&ROM, 0x4000);
		}
		else
		{
			if (get_software_region_length("rom") != 0x4000)
				return IMAGE_INIT_FAIL;

			memcpy(ROM, get_software_region("rom"), 0x4000);
		}
	}

	return IMAGE_INIT_PASS;
}


bool nes_ntb_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}

std::string nes_ntb_slot_device::get_default_card_software()
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

const device_type NES_NTB_ROM = &device_creator<nes_ntb_rom_device>;

nes_ntb_rom_device::nes_ntb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, NES_NTB_ROM, "NES NTB ROM", tag, owner, clock, "nes_ntbrom", __FILE__),
						ntb_cart_interface( mconfig, *this )
{
}

void nes_ntb_rom_device::device_start()
{
	m_rom = (UINT8*)memregion("ntbrom")->base();
}

const rom_entry *nes_ntb_rom_device::device_rom_region() const
{
	return ROM_NAME( ntb_rom );
}

UINT8 *nes_ntb_rom_device::get_cart_base()
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

const device_type NES_SUNSOFT_DCS = &device_creator<nes_sunsoft_dcs_device>;


nes_sunsoft_dcs_device::nes_sunsoft_dcs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_sunsoft_4_device(mconfig, NES_SUNSOFT_DCS, "NES Cart Sunsoft DCS PCB", tag, owner, clock, "nes_dcs", __FILE__), m_timer_on(0), m_exrom_enable(0),
						m_subslot(*this, "ntb_slot"), ntb_enable_timer(nullptr)
				{
}


void nes_sunsoft_dcs_device::device_start()
{
	common_start();
	ntb_enable_timer = timer_alloc(TIMER_PROTECT);
	ntb_enable_timer->reset();
	timer_freq = machine().device<cpu_device>("maincpu")->cycles_to_attotime(107520);

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

WRITE8_MEMBER(nes_sunsoft_dcs_device::write_h)
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
			sun4_write(space, offset, data, mem_mask);
			break;
	}
}

READ8_MEMBER(nes_sunsoft_dcs_device::read_h)
{
	LOG_MMC(("Sunsoft DCS read_h, offset: %04x\n", offset));

	if (m_exrom_enable && m_subslot->m_cart && offset < 0x4000)
	{
		if (m_timer_on)
			return m_subslot->m_cart->read(space, offset, mem_mask);
		else
			return m_open_bus;   // after the timer is off, this returns open bus...
	}
	else
		return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_sunsoft_dcs_device::write_m)
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

READ8_MEMBER(nes_sunsoft_dcs_device::read_m)
{
	LOG_MMC(("Sunsoft DCS read_m, offset: %04x\n", offset));

	if (!m_battery.empty() && m_wram_enable)
		return m_battery[offset & (m_battery.size() - 1)];
	if (!m_prgram.empty() && m_wram_enable)
		return m_prgram[offset & (m_prgram.size() - 1)];

	return m_open_bus;   // open bus
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sub_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START(ntb_cart)
	SLOT_INTERFACE_INTERNAL("ntbrom", NES_NTB_ROM)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( sub_slot )
	MCFG_NTB_MINICART_ADD("ntb_slot", ntb_cart)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nes_sunsoft_dcs_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sub_slot );
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
