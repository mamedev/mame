// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Datel Action Replay

    Freezer cartridge for Amiga 500 and Amiga 2000

    Skeleton device, just loads the ROMs and generates the NMI
    for now.

    Hardware notes:
    - http://www.mways.co.uk/amiga/howtocode/text/actionreplay.php

***************************************************************************/

#include "emu.h"
#include "action_replay.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ZORRO_ACTION_REPLAY_MK1, bus::amiga::zorro::action_replay_mk1_device, "zorro_ar1", "Datel Action Replay MK-I")
DEFINE_DEVICE_TYPE(ZORRO_ACTION_REPLAY_MK2, bus::amiga::zorro::action_replay_mk2_device, "zorro_ar2", "Datel Action Replay MK-II")
DEFINE_DEVICE_TYPE(ZORRO_ACTION_REPLAY_MK3, bus::amiga::zorro::action_replay_mk3_device, "zorro_ar3", "Datel Action Replay MK-III")


namespace bus::amiga::zorro {

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( ar_button )
	PORT_START("freeze")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(action_replay_device_base::freeze), 0)
INPUT_PORTS_END

ioport_constructor action_replay_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( ar_button );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( ar_mk1 )
	ROM_REGION(0x10000, "firmware", 0)
	ROM_DEFAULT_BIOS("v150")
	ROM_SYSTEM_BIOS(0, "v100", "Version 1.00")
	ROMX_LOAD("ar1_v100.bin", 0x0000, 0x10000, BAD_DUMP CRC(2d921771) SHA1(1ead9dda2dad29146441f5ef7218375022e01248), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v150", "Version 1.50")
	ROMX_LOAD("ar1_v150.bin", 0x0000, 0x10000, BAD_DUMP CRC(f82c4258) SHA1(843b433b2c56640e045d5fdc854dc6b1a4964e7c), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *action_replay_mk1_device::device_rom_region() const
{
	return ROM_NAME( ar_mk1 );
}

ROM_START( ar_mk2 )
	ROM_REGION(0x20000, "firmware", 0)
	ROM_DEFAULT_BIOS("v214")
	ROM_SYSTEM_BIOS(0, "v205", "Version 2.05")
	ROMX_LOAD("ar2_v205.bin", 0x0000, 0x20000, BAD_DUMP CRC(4051eef8) SHA1(9df22b1d3285b522c223697c83d144d04e961a4a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v212", "Version 2.12")
	ROMX_LOAD("ar2_v212.bin", 0x0000, 0x20000, BAD_DUMP CRC(d29bdd86) SHA1(76c2900457badf22b742f0af48b78937e8b67694), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v214", "Version 2.14")
	ROMX_LOAD("ar2_v214.bin", 0x0000, 0x20000, BAD_DUMP CRC(1bb3d0a8) SHA1(14b1f5a69efb6f4e2331970e6ca0f33c0f04ac91), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *action_replay_mk2_device::device_rom_region() const
{
	return ROM_NAME( ar_mk2 );
}

ROM_START( ar_mk3 )
	ROM_REGION(0x40000, "firmware", 0)
	ROM_DEFAULT_BIOS("v309")
	ROM_SYSTEM_BIOS(0, "v309", "Version 3.09")
	ROMX_LOAD("ar3_v309.evn", 0x00000, 0x20000, CRC(2b84519f) SHA1(7841873bf009d8341dfa2794b3751bacf86adcc8), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("ar3_v309.odd", 0x00001, 0x20000, CRC(1d35bd56) SHA1(6464be1626b519499e76e4e3409e8016515d48b6), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v317", "Version 3.17")
	ROMX_LOAD("ar3_v314.bin", 0x0000, 0x40000, BAD_DUMP CRC(009f7768) SHA1(0439d6ccc2a0e5c2e83fcf2389dc4d4a440a4c62), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *action_replay_mk3_device::device_rom_region() const
{
	return ROM_NAME( ar_mk3 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  action_replay_device_base - constructor
//-------------------------------------------------

action_replay_device_base::action_replay_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_exp_card_interface(mconfig, *this),
	m_button(*this, "freeze")
{
}

action_replay_mk1_device::action_replay_mk1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	action_replay_device_base(mconfig, ZORRO_ACTION_REPLAY_MK1, tag, owner, clock)
{
}

action_replay_mk2_device::action_replay_mk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	action_replay_device_base(mconfig, ZORRO_ACTION_REPLAY_MK2, tag, owner, clock)
{
}

action_replay_mk3_device::action_replay_mk3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	action_replay_device_base(mconfig, ZORRO_ACTION_REPLAY_MK3, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void action_replay_device_base::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void action_replay_device_base::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

INPUT_CHANGED_MEMBER( action_replay_device_base::freeze )
{
	// pushing the freeze button generates an nmi
	m_slot->ipl_w(newval == 1 ? 7 : 0);
}

} // namespace bus::amiga::zorro
