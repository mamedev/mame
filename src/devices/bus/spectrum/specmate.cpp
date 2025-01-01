// license:BSD-3-Clause
// copyright-holders:TwistedTom
/*********************************************************************

    Spec-Mate back-up interface

    (A, T & Y Computing Ltd.)

    A back-up/snapshot type device similar to multiface.
    No on-screen UI, feedback is given by border colour changes.
    Supports saving to Beta-disk, Microdrive, Wafadrive, tape (normal & double-speed modes)

    Only one rom is known, although magazine adverts stated:
    "Opus users £2 extra, Opus & Sprint users contact us for details", suggesting other roms may exist.
    (Sprint was a high-speed tape deck, x4 recording on a standard cassette)

    Operation:

    Press freeze button...

    Black border mode:   (select device)
    t = save to tape
    m = save to microdrive
    d = save to Beta disk
    w = save to wafadrive
    f = save to tape, double-speed mode
    0 = remove attributes *1

    Red border mode:   (save mode)
    n = normal save
    s = screenless save                (game starts with blank screen!)
    a = normal save with a 2nd screen  (1st part of 2-part operation)  *2
    b = normal save with a 2nd screen  (2nd part of 2-part operation)  *2

    Blue border mode:   (select which part of vram is used as specmate working ram)
    1 = use top 3rd of screen
    2 = use middle 3rd of screen
    3 = use bottom 3rd of screen

    Magenta border mode:  (enter filename)
    Type a filename, press Enter to save

    System should auto-unfreeze after successful save...


    Notes:
    *1 Removes colour from the frozen screen so that essential data "hidden" in vram can be seen (some games do this as a copy protection),
       user can then avoid disturbing this data with choice made in blue-border mode.
    *2 Saves a 2nd screen (eg. a nice loading screen) just for fun. 2-part operation, so load game, freeze, save 1st screen,
       reset system, re-load game, freeze, save main game and 2nd screen.
    *  Saves are created with stand-alone loader code so are not dependant on the interface for reuse.
    *  Pokes can be added to the save loader: press BREAK as soon as the screen goes black, INK 9, POKE aaaaa,ddd


*********************************************************************/

#include "emu.h"
#include "specmate.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_SPECMATE, spectrum_specmate_device, "spectrum_specmate", "AT&Y Spec-Mate")


//-------------------------------------------------
//  INPUT_PORTS( specmate )
//-------------------------------------------------

INPUT_PORTS_START( specmate )
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Freeze Button") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spectrum_specmate_device::freeze_button), 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_specmate_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(specmate);
}

//-------------------------------------------------
//  ROM( specmate )
//-------------------------------------------------

ROM_START(specmate)
	ROM_REGION(0x2000, "rom", 0)  // unknown ver
	ROM_LOAD("specmate.rom", 0x0000, 0x2000, CRC(8d74b19c) SHA1(c8f128610eeb5142d4d80f28e5dd07b7a5ab6b84))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_specmate_device::device_add_mconfig(machine_config &config)
{
	/* passthru */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

const tiny_rom_entry *spectrum_specmate_device::device_rom_region() const
{
	return ROM_NAME(specmate);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_specmate_device - constructor
//-------------------------------------------------

spectrum_specmate_device::spectrum_specmate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_SPECMATE, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_exp(*this, "exp")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_specmate_device::device_start()
{
	save_item(NAME(m_romcs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_specmate_device::device_reset()
{
	m_romcs = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_specmate_device::romcs()
{
	return m_romcs || m_exp->romcs();
}

void spectrum_specmate_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xdfff)  // a13 isn't used in the decoding
		{
		case 0x0066:
			m_romcs = 1;
			break;
		case 0x0071:
			m_romcs = 0;
			break;
		}
	}
}

uint8_t spectrum_specmate_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		data = m_rom->base()[offset & 0x1fff];
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}
