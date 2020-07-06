// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    Skeleton driver for "EuroByte Electronics & Multimedia IASA" EuroPlay
    (http://www.eurobyte.com.gr/gb_europlay.htm), sold in Spain by Sleic / Petaco
    as Star Touch / EuroPlay 2001.

    Hardware overview:
    MB Soyo M5EH V1.2 (1MB cache) or similar (e.g. Biostar M5ATD)
    16384 KB RAM
    Intel Pentium MMX 233 MHz

    MicroTouch ISA (BIOS 5.6)
    ExpertColor Med3931 v1.0 ISA sound card (or other 82C931-based similar card, e.g. BTC 1817DS OPTi ISA)
    S3 Trio64V2/DX PCI VGA (86C775, 512KB RAM)
    Parallel port dongle HASP4
    Creative Video Blaster camera (parallel port)
    HDD Samsung SV0322A or other IDE HDD with similar capacity.

*******************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "speaker.h"


class startouch_state : public driver_device
{
public:
	startouch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void europl01(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void startouch_state::mem_map(address_map &map)
{
}

void startouch_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START(europl01)
INPUT_PORTS_END

void startouch_state::europl01(machine_config &config)
{
	PENTIUM_MMX(config, m_maincpu, 233'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &startouch_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &startouch_state::io_map);
}

ROM_START(europl01)
	ROM_REGION(0x20000, "mb_bios", 0)
	ROM_LOAD( "award_pci-pnp_586_222951562_v4.51pg.u2", 0x00000, 0x20000, CRC(5bb1bcbc) SHA1(6e2a7b5b3fc892ed20d0b12a1a533231c8953177) )

	ROM_REGION(0x8000, "vga_bios", 0)
	ROM_LOAD( "s3_86c775-86c785_video_bios_v1.01.04.u5", 0x0000, 0x8000, CRC(e718418f) SHA1(1288ce51bb732a346eb7c61d5bdf80ea22454d45) )

	ROM_REGION(0x20000, "hd_firmware", 0)
	ROM_LOAD( "jk200-35.bin", 0x00000, 0x20000, CRC(601fa709) SHA1(13ded4826a64209faac8bc81708172b81195ab96) )

	ROM_REGION(0x66da4, "dongle", 0)
	ROM_LOAD( "9b91f19d.hsp", 0x00000, 0x66da4, CRC(0cf78908) SHA1(c596f415accd6b91be85ea8c1b89ea380d0dc6c8) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "sleic-petaco_startouch_2001_v2.0", 0, SHA1(3164a5786d6b9bb0dd9910b4d27a77a6b746dedf) ) // labeled startouch_2001 but when run game title is EuroPlay 2001
ROM_END


GAME(2001?, europl01, 0, europl01, europl01, startouch_state, empty_init, ROT0, "Sleic / Petaco", "EuroPlay 2001", MACHINE_IS_SKELETON)
