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


 TODO:
 - verify expansion slot emulation for the Senyou Cassettes:
   not much documentation exists about the expansion carts (except for few paragraphs
   at Enri's FC webpage), so I implemented it based on "common sense"
   * expansion carts do not contain the required game data => main PRG must be in the main cart
     so to remain connected even when an expansion is inserted (differently from Datach, where
     the base unit contains no PRG)
   * bankswitch writes with bit3=0 (to access expansion) when no expansion is present should do
     nothing

 ***********************************************************************************************************/


#include "emu.h"
#include "karastudio.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
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
	, device_image_interface(mconfig, *this)
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

image_init_result nes_kstudio_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *ROM = m_cart->get_cart_base();

		if (!ROM)
			return image_init_result::FAIL;

		// Existing expansion carts are all 128K, so we only load files of this size
		if (!loaded_through_softlist())
		{
			if (length() != 0x20000)
				return image_init_result::FAIL;

			fread(&ROM, 0x20000);
		}
		else
		{
			if (get_software_region_length("rom") != 0x20000)
				return image_init_result::FAIL;

			memcpy(ROM, get_software_region("rom"), 0x20000);
		}
	}

	return image_init_result::PASS;
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
	, m_subslot(*this, "exp_slot")
	, m_mic_ipt(*this, "MIC")
{
}


void nes_karaokestudio_device::device_start()
{
	common_start();
	save_item(NAME(m_exp_active));
}

void nes_karaokestudio_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef((m_prg_chunks - 1) ^ 0x08);
	chr8(0, m_chr_source);
	m_exp_active = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bandai Karaoke Studio board emulation

 Games: Karaoke Studio + expansion carts with
 additional songs

 Note: we currently do not emulate properly the
 expansion slot

 iNES: mapper 188

 -------------------------------------------------*/

uint8_t nes_karaokestudio_device::read_m(offs_t offset)
{
	LOG("karaoke studio read_m, offset: %04x\n", offset);
	return m_mic_ipt->read();
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
	if (offset < 0x4000 && m_exp_active && m_subslot->m_cart)
		return m_subslot->m_cart->read(offset);
	else
		return hi_access_rom(offset);
}

void nes_karaokestudio_device::write_h(offs_t offset, uint8_t data)
{
	LOG("karaoke studio write_h, offset: %04x, data: %02x\n", offset, data);
	// bit3 1 = M ROM (main unit), 0=E ROM (expansion)
	// HACK(?): currently it is not clear how the unit acknowledges the presence of the expansion
	// cart (when expansion is present, code keeps switching both from the expansion rom and from
	// the main ROM)
	// my guess is that writes with bit3=0 and no expansion just do nothing, but it shall be verified

	if (offset >= 04000)
	{
		if (BIT(data, 3))
		{
			m_exp_active = 0;
			prg16_89ab(data & 7);
		}
		else    // expansion cart
		{
			m_exp_active = 1;
			m_subslot->write_prg_bank(data & 7);
		}
	}
}


static INPUT_PORTS_START( karaoke_mic )
	PORT_START("MIC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A (Mic Select)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B (Mic Start)") PORT_CODE(KEYCODE_Z)
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
