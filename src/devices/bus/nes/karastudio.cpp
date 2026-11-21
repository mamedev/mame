// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bandai Karaoke Studio


 Here we emulate the following PCBs Bandai Karaoke Studio [mapper 188]

 The Karaoke Studio cart consist of a large connector which fits the FC cart slot, with a microphone
 connected. The game data is in the connector itself. The microphone has two buttons on it, and the
 game uses these only to navigate through the menus (the two buttons are not read through the controller
 port, which is not accessible from the cart, but from $6000-$7fff). Part of the connector body can be
 removed to be replaced by an expansion cart containing new songs (we emulate this by adding a -cart2 slot).


 Register and microphone port layout follow NESdev INES Mapper 188. The main PRG stays in the
 main unit; the expansion ROM is selected at $8000-$BFFF with R = 0, and reads open bus when no
 expansion cartridge is present. 256 KiB iNES images hold the 128 KiB internal ROM followed by
 the 128 KiB expansion ROM, so the expansion half is used when no -cart2 is mounted.

 TODO:
 - the polarity of the 1-bit microphone ADC input is not documented

 ***********************************************************************************************************/


#include "emu.h"
#include "karastudio.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE 0
#endif
#include "logmacro.h"


//-----------------------------------------
//
//  Karaoke Studio Cartslot implementation
//
//-----------------------------------------

//-------------------------------------------------
//  sub-cart interface
//-------------------------------------------------

kstudio_cart_interface::kstudio_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "kstudiocart")
	, m_rom(nullptr), m_bank(0)
{
}

kstudio_cart_interface::~kstudio_cart_interface()
{
}

uint8_t kstudio_cart_interface::read(offs_t offset)
{
	return m_rom[(m_bank * 0x4000) + (offset & 0x3fff)];
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_KSEXPANSION_SLOT, nes_kstudio_slot_device, "nes_ks_slot", "NES Karaoke Studio Expansion Slot")

nes_kstudio_slot_device::nes_kstudio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NES_KSEXPANSION_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<kstudio_cart_interface>(mconfig, *this)
	, m_cart(nullptr)
{
}

nes_kstudio_slot_device::~nes_kstudio_slot_device()
{
}


void nes_kstudio_slot_device::device_start()
{
	m_cart = get_card_device();
}

uint8_t nes_kstudio_slot_device::read(offs_t offset)
{
	if (m_cart)
		return m_cart->read(offset);
	else
		return 0xff;
}

std::pair<std::error_condition, std::string> nes_kstudio_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *const ROM = m_cart->get_cart_base();
		if (!ROM)
			return std::make_pair(image_error::INTERNAL, std::string());

		// Existing expansion carts are all 128K, so we only load files of this size
		if (!loaded_through_softlist())
		{
			if (length() != 0x20000)
				return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 128K)");

			fread(ROM, 0x20000);
		}
		else
		{
			if (get_software_region_length("rom") != 0x20000)
				return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 128K)");

			memcpy(ROM, get_software_region("rom"), 0x20000);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}


std::string nes_kstudio_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("ks_exp");
}


//-----------------------------------------------
//
//  Karaoke Studio Expansion cart implementation
//
//-----------------------------------------------

ROM_START( ks_exp_rom )
	ROM_REGION(0x20000, "exrom", ROMREGION_ERASEFF)
ROM_END

DEFINE_DEVICE_TYPE(NES_KSEXPANSION_ROM, nes_kstudio_rom_device, "nes_ks_rom", "NES Karaoke Studio Expansion ROM")

nes_kstudio_rom_device::nes_kstudio_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NES_KSEXPANSION_ROM, tag, owner, clock)
	, kstudio_cart_interface( mconfig, *this )
{
}

void nes_kstudio_rom_device::device_start()
{
	m_rom = (uint8_t*)memregion("exrom")->base();
	save_item(NAME(m_bank));
}

void nes_kstudio_rom_device::device_reset()
{
	m_bank = 0;
}

const tiny_rom_entry *nes_kstudio_rom_device::device_rom_region() const
{
	return ROM_NAME( ks_exp_rom );
}

uint8_t *nes_kstudio_rom_device::get_cart_base()
{
	return m_rom;
}


//------------------------------------------
//
//  Karaoke Studio Base Cart implementation
//
//------------------------------------------

DEFINE_DEVICE_TYPE(NES_KARAOKESTUDIO, nes_karaokestudio_device, "nes_karaoke", "NES Cart Bandai Karaoke Studio PCB")


nes_karaokestudio_device::nes_karaokestudio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_KARAOKESTUDIO, tag, owner, clock)
	, m_exp_active(0)
	, m_exp_bank(0)
	, m_subslot(*this, "exp_slot")
	, m_mic_ipt(*this, "MIC")
{
}


void nes_karaokestudio_device::device_start()
{
	common_start();
	save_item(NAME(m_exp_active));
	save_item(NAME(m_exp_bank));
}

void nes_karaokestudio_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(7);   // $C000-$FFFF: last bank of the 128 KiB internal ROM
	chr8(0, m_chr_source);
	m_exp_active = 0;
	m_exp_bank = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bandai Karaoke Studio board emulation

 Games: Karaoke Studio + expansion carts with
 additional songs


 iNES: mapper 188

 -------------------------------------------------*/

uint8_t nes_karaokestudio_device::read_m(offs_t offset)
{
	LOG("karaoke studio read_m, offset: %04x\n", offset);
	// [.... .MBA]: A and B buttons read 0 when pressed, M is the 1-bit microphone ADC, the rest is open bus
	return (get_open_bus() & 0xf8) | (m_mic_ipt->read() & 0x07);
}

uint8_t nes_karaokestudio_device::read_h(offs_t offset)
{
	LOG("karaoke studio read_h, offset: %04x\n", offset);
	// this shall be the proper code, but it's a bit slower, so we access directly the subcart below
	//return m_subslot->read(offset);

	// access expansion cart only if all of the followings are verified
	// * we are in $8000-$bfff range
	// * there has been a bankswitch write to map the expansion to such range
	// * there actually is an expansion cart mounted
	if (offset < 0x4000 && m_exp_active)
	{
		if (m_subslot->m_cart)
			return m_subslot->m_cart->read(offset);
		// 256 KiB images are the 128 KiB internal ROM followed by the 128 KiB expansion ROM (NESdev)
		if (m_prg_size > 0x20000)
			return m_prg[0x20000 + ((m_exp_bank * 0x4000 + offset) % (m_prg_size - 0x20000))];
		return get_open_bus();   // no expansion cartridge: nothing drives the bus
	}
	return hi_access_rom(offset);
}

void nes_karaokestudio_device::write_h(offs_t offset, uint8_t data)
{
	LOG("karaoke studio write_h, offset: %04x, data: %02x\n", offset, data);
	// [.LXR BBBB] (NESdev INES Mapper 188): B = 16 KiB bank at $8000-$BFFF, R = 1 internal / 0 external ROM,
	// X = CIRAM A10 from PPU A10 (0, vertical mirroring) or PPU A11 (1, horizontal mirroring), L = unused latch;
	// the board has bus conflicts
	data = account_bus_conflict(offset, data);
	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	if (BIT(data, 4))
	{
		m_exp_active = 0;
		prg16_89ab(data & 0x0f);
	}
	else
	{
		m_exp_active = 1;
		m_exp_bank = data & 0x0f;
		m_subslot->write_prg_bank(m_exp_bank);
	}
}


static INPUT_PORTS_START( karaoke_mic )
	PORT_START("MIC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("A (Mic Select)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("B (Mic Start)") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Microphone (?)") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor nes_karaokestudio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( karaoke_mic );
}


//-------------------------------------------------
//  CART SLOT
//-------------------------------------------------

static void karaoke_studio_cart(device_slot_interface &device)
{
	device.option_add_internal("ks_exp", NES_KSEXPANSION_ROM);
}


void nes_karaokestudio_device::device_add_mconfig(machine_config &config)
{
	NES_KSEXPANSION_SLOT(config, m_subslot, karaoke_studio_cart);
}
