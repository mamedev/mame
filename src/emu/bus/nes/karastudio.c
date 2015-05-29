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
   * bankswicth writes with bit3=0 (to access expansion) when no expansion is present should do
     nothing

 ***********************************************************************************************************/


#include "emu.h"
#include "karastudio.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-----------------------------------------
//
//  Karaoke Studio Cartslot implementation
//
//-----------------------------------------

//-------------------------------------------------
//  sub-cart interface
//-------------------------------------------------

kstudio_cart_interface::kstudio_cart_interface(const machine_config &mconfig, device_t &device)
					: device_slot_card_interface(mconfig, device),
					m_rom(NULL)
{
}

kstudio_cart_interface::~kstudio_cart_interface()
{
}

READ8_MEMBER(kstudio_cart_interface::read)
{
	return m_rom[(m_bank * 0x4000) + (offset & 0x3fff)];
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

const device_type NES_KSEXPANSION_SLOT = &device_creator<nes_kstudio_slot_device>;

nes_kstudio_slot_device::nes_kstudio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
							device_t(mconfig, NES_KSEXPANSION_SLOT, "NES Karaoke Studio Expansion Slot", tag, owner, clock, "nes_ks_slot", __FILE__),
							device_image_interface(mconfig, *this),
							device_slot_interface(mconfig, *this)
{
}

nes_kstudio_slot_device::~nes_kstudio_slot_device()
{
}


void nes_kstudio_slot_device::device_start()
{
	m_cart = dynamic_cast<kstudio_cart_interface *>(get_card_device());
}

READ8_MEMBER(nes_kstudio_slot_device::read)
{
	if (m_cart)
		return m_cart->read(space, offset, mem_mask);

	return 0xff;
}

bool nes_kstudio_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM = m_cart->get_cart_base();

		if (!ROM)
			return IMAGE_INIT_FAIL;

		// Existing exapnsion carts are all 128K, so we only load files of this size
		if (software_entry() == NULL)
		{
			if (length() != 0x20000)
				return IMAGE_INIT_FAIL;

			fread(&ROM, 0x20000);
		}
		else
		{
			if (get_software_region_length("rom") != 0x20000)
				return IMAGE_INIT_FAIL;

			memcpy(ROM, get_software_region("rom"), 0x20000);
		}
	}

	return IMAGE_INIT_PASS;
}


bool nes_kstudio_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}

void nes_kstudio_slot_device::get_default_card_software(std::string &result)
{
	software_get_default_slot(result, "ks_exp");
}


//-----------------------------------------------
//
//  Karaoke Studio Expansion cart implementation
//
//-----------------------------------------------

ROM_START( ks_exp_rom )
	ROM_REGION(0x20000, "exrom", ROMREGION_ERASEFF)
ROM_END

const device_type NES_KSEXPANSION_ROM = &device_creator<nes_kstudio_rom_device>;

nes_kstudio_rom_device::nes_kstudio_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
							: device_t(mconfig, NES_KSEXPANSION_ROM, "NES Karaoke Studio Expansion ROM", tag, owner, clock, "nes_ks_rom", __FILE__),
								kstudio_cart_interface( mconfig, *this )
{
}

void nes_kstudio_rom_device::device_start()
{
	m_rom = (UINT8*)memregion("exrom")->base();
	save_item(NAME(m_bank));
}

void nes_kstudio_rom_device::device_reset()
{
	m_bank = 0;
}

const rom_entry *nes_kstudio_rom_device::device_rom_region() const
{
	return ROM_NAME( ks_exp_rom );
}

UINT8 *nes_kstudio_rom_device::get_cart_base()
{
	return m_rom;
}


//------------------------------------------
//
//  Karaoke Studio Base Cart implementation
//
//------------------------------------------

const device_type NES_KARAOKESTUDIO = &device_creator<nes_karaokestudio_device>;


nes_karaokestudio_device::nes_karaokestudio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KARAOKESTUDIO, "NES Cart Bandai Karaoke Studio PCB", tag, owner, clock, "nes_karaoke", __FILE__),
					m_subslot(*this, "exp_slot"),
					m_mic_ipt(*this, "MIC")
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

READ8_MEMBER(nes_karaokestudio_device::read_m)
{
	LOG_MMC(("karaoke studio read_m, offset: %04x\n", offset));
	return m_mic_ipt->read();
}

READ8_MEMBER(nes_karaokestudio_device::read_h)
{
	LOG_MMC(("karaoke studio read_h, offset: %04x\n", offset));
	// this shall be the proper code, but it's a bit slower, so we access directly the subcart below
	//return m_subslot->read(space, offset, mem_mask);

	// access expansion cart only if all of the followings are verified
	// * we are in $8000-$bfff range
	// * there has been a bankswitch write to map the expansion to such range
	// * there actually is an expansion cart mounted
	if (offset < 0x4000 && m_exp_active && m_subslot->m_cart)
		return m_subslot->m_cart->read(space, offset, mem_mask);
	else
		return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_karaokestudio_device::write_h)
{
	LOG_MMC(("karaoke studio write_h, offset: %04x, data: %02x\n", offset, data));
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

static SLOT_INTERFACE_START(karaoke_studio_cart)
	SLOT_INTERFACE_INTERNAL("ks_exp", NES_KSEXPANSION_ROM)
SLOT_INTERFACE_END


MACHINE_CONFIG_FRAGMENT( karaoke_studio )
	MCFG_KSTUDIO_MINICART_ADD("exp_slot", karaoke_studio_cart)
MACHINE_CONFIG_END

machine_config_constructor nes_karaokestudio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( karaoke_studio );
}
