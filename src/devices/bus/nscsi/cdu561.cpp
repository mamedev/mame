// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Sony CDU561-25 CD-ROM drive.

    PCB: MAIN 1-644-812-13

    Microcontroller:
    - Mitsubishi M37732S4AFP

    Memories:
    - TI TMS27C010A-12 EPROM or equivalent
    - Mosel MS62256CLL-10FC or Sony CXK58257AM-10L 32,768x8 Static RAM
    - Mosel MS514256A-70VC 262,144x4 Fast Page Mode DRAM (x2; near CXD1198)
    - Fujitsu MB81C79A-45 or Sony CXK5971AM-35 8,192x9 Static RAM (near CXD1186)

    Other major digital ICs:
    - Sony CXD1185CQ SCSI Controller
    - Sony CXD1186Q CD-ROM Decoder
    - Sony CXD1198AQ CD-ROM Subcode Decoder
    - Sony CXD2500BQ CD Digital Signal Processor
    - Sony CXD8416Q

    Linear ICs:
    - Sony CXA1372AQ RF Signal Processing Servo Amplifier for CD Player
    - Sony CXA1571M RF Amplifier for CD Player
    - Sony CXD2568M (apparently a RF amplifier, similar to CXA1821M but with
      hold and AGC controls)
    - Toshiba TA8406P Dual Power Operational Amplifier (x3)

    XTALs:
    - D169 (near CXD2500B)
    - 24.00MX (near CXD1198)

*******************************************************************************/

#include "emu.h"
#include "cdu561.h"

#include "cpu/m37710/m37710.h"
#include "machine/cxd1185.h"

DEFINE_DEVICE_TYPE(CDU561_25, cdu561_25_device, "cdu561_25", "Sony CDU561-25")

cdu561_25_device::cdu561_25_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CDU561_25, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "scsic")
{
}

void cdu561_25_device::device_start()
{
}

void cdu561_25_device::mem_map(address_map &map)
{
	map(0x000880, 0x007fff).ram();
	map(0x008000, 0x01ffff).rom().region("eprom", 0x8000);
	map(0x020000, 0x02ffff).rom().region("eprom", 0);
	map(0x030000, 0x03000f).noprw(); // CXD1186?
	map(0x050000, 0x05000f).m("scsic", FUNC(cxd1185_device::map));
	map(0x070000, 0x070001).nopr(); // ?
}

void cdu561_25_device::device_add_mconfig(machine_config &config)
{
	m37732s4_device &mcu(M37732S4(config, "mcu", 24_MHz_XTAL / 2));
	mcu.set_addrmap(AS_PROGRAM, &cdu561_25_device::mem_map);

	CXD1185(config, "scsic", 24_MHz_XTAL / 2);
}

ROM_START(cdu561_25)
	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "19a", "apl1.9a")
	ROMX_LOAD("apl1.9a_83fb.ic302", 0x00000, 0x20000, CRC(0efc50eb) SHA1(8bfd6ebc0863017914808e8282a5914cdc828f56), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "18f", "apl1.8F")
	ROMX_LOAD("apl1.8f_d905.ic302", 0x00000, 0x20000, CRC(3ea92e48) SHA1(2f409fd59c5f09d22e00b39f4b0b57e16316090d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "17w", "apl1.7w")
	ROMX_LOAD("apl_1.7w.bin", 0x00000, 0x20000, CRC(12ba5843) SHA1(70aa550693020431ccbd374ff85e4de8809431df), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *cdu561_25_device::device_rom_region() const
{
	return ROM_NAME(cdu561_25);
}
