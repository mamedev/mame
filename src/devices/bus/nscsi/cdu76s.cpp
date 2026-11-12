// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/*********************************************************************

    Sony CDU-76S CD-ROM drive

    The firmware is not executed; the drive is emulated at the SCSI
    command level.

*********************************************************************/

#include "emu.h"
#include "cdu76s.h"

DEFINE_DEVICE_TYPE(NSCSI_CDU76S, nscsi_cdu76s_device, "cdu76s", "Sony CDU-76S CD-ROM Drive")

ROM_START( cdu76s )
	ROM_REGION( 0x10000, "eprom", 0 )
	ROM_DEFAULT_BIOS("v11c")
	ROM_SYSTEM_BIOS( 0, "v11c", "CDU76S firmware 1.1c" )
	ROMX_LOAD( "76s.1.1c.ic206", 0x00000, 0x10000, CRC(8a233f19) SHA1(5f3c3f1ec181cb7f18daea5c8c592f6910aac3b2), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v11a", "CDU76S firmware 1.1a" )
	ROMX_LOAD( "76s.1.1a.ic206", 0x00000, 0x10000, CRC(1df044f9) SHA1(8d1f6ccd6fb5e04b987b9aa21e602a21df7505a4), ROM_BIOS(1) ) // 95.06.01
ROM_END

nscsi_cdu76s_device::nscsi_cdu76s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDU76S, tag, owner, "Sony", "CDU-76S", "1.0", 0x00, 0x05)
{
}

const tiny_rom_entry *nscsi_cdu76s_device::device_rom_region() const
{
	return ROM_NAME( cdu76s );
}
