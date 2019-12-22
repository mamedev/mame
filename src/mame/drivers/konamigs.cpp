// license:BSD-3-Clause
// copyright-holders:MetalliC
/*************************************************************************

	Konami GSAN1 hardware
	(c) 2000

	mostly preliminary driver
	
	CPU: Hitachi HD6417709 SH-3
	GPU: Hitachi HD64413AF 'Q2SD' Quick 2D Graphics Renderer with Synchronous DRAM Interface
	SPU: Yamaha YMZ280B-F
	Misc:
	     Altera Max EPM3256ATC144-10
		 Altera Max EPM3064ATC100-10
		 Epson RTC-4553

	Known games:
	 Dance Dance Revolution Kids (not dumped)
	 Muscle Ranking Spray Hitter

	TODOs:
	 - interrupt sources
	 - I/O
	 - GPU

**************************************************************************/

#include "emu.h"
#include "cpu/sh/sh3comn.h"
#include "cpu/sh/sh4.h"
#include "bus/ata/ataintf.h"
#include "machine/ataflash.h"
#include "machine/s3520cf.h"
#include "sound/ymz280b.h"
#include "speaker.h"
#include "screen.h"

class gsan_state : public driver_device
{
public:
	gsan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ata(*this, "ata")
		, m_vram(*this, "vram")
		, m_rtc_r(*this, "RTCR")
		, m_rtc_w(*this, "RTCW")
		{ }

	void gsan(machine_config &config);
	void init_gsan();

private:
	required_device<sh34_base_device> m_maincpu;
	required_device<ata_interface_device> m_ata;
	required_shared_ptr<uint64_t> m_vram;
	required_ioport m_rtc_r;
	required_ioport m_rtc_w;

	void main_map(address_map &map);
	void main_port(address_map &map);
	void ymz280b_map(address_map &map);

	DECLARE_READ16_MEMBER(cf_regs_r);
	DECLARE_WRITE16_MEMBER(cf_regs_w);
	DECLARE_READ16_MEMBER(cf_data_r);
	DECLARE_WRITE16_MEMBER(cf_data_w);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_READ16_MEMBER(gpu_r);
	DECLARE_WRITE16_MEMBER(gpu_w);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

u32 gsan_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// display something
	u32 base_offs = 0;
	for (int y = 0; y < 240; y++)
	{
		for (int x = 0; x < 320; x++)
		{
			u32 idx = (x & 0xf) | ((y & 0xf) << 4) | ((x & ~0xf) << 4) | ((y & ~0xf) << 9);
			u64 pix = (m_vram[base_offs + idx / 4] >> ((~idx & 3) * 16)) & 0xffff;
			bitmap.pix32(y, x) = pal555(pix, 0, 5, 10);
		}
	}
	return 0;
}

static INPUT_PORTS_START( gsan )
	PORT_START("PORT_L")
	PORT_BIT( 0x00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // DIP switches ?

	PORT_START("RTCW")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_dir_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_cs_line)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, write_bit)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_clock_line)

	PORT_START("RTCR")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rtc", rtc4553_device, read_bit)
INPUT_PORTS_END

// CF interface, looks like standard memory-mapped ATA layout, probably should be devicified
READ16_MEMBER(gsan_state::cf_regs_r)
{
	offset *= 2;
	u16 data = m_ata->read_cs0(offset, 0xff) & 0xff;
	data |= (m_ata->read_cs0(offset + 1, 0xff) << 8);
	return data;
}
WRITE16_MEMBER(gsan_state::cf_regs_w)
{
	offset *= 2;
	m_ata->write_cs0(offset, data & 0xff, 0xff);
	m_ata->write_cs0(offset + 1, data >> 8, 0xff);
}
READ16_MEMBER(gsan_state::cf_data_r)
{
	u16 data = m_ata->read_cs0(0, 0xffff);
	return data;
}
WRITE16_MEMBER(gsan_state::cf_data_w)
{
	m_ata->write_cs0(0, data, 0xffff);
}

READ8_MEMBER(gsan_state::rtc_r)
{
	return m_rtc_r->read();
}
WRITE8_MEMBER(gsan_state::rtc_w)
{
	m_rtc_w->write(data);
}

// Q2SD GPU
READ16_MEMBER(gsan_state::gpu_r)
{
	switch (offset)
	{
	case 1: // status
		return machine().rand(); // code must go on
	default:
		//logerror("GPU read %03X\n", offset);
		return 0;
	}
}
WRITE16_MEMBER(gsan_state::gpu_w)
{
	// here be dragons...
	//logerror("GPU write %03X %04X\n", offset, data);
}


void gsan_state::main_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("maincpu", 0);
	map(0x08000000, 0x083fffff).ram().share("vram");
	map(0x0c000000, 0x0c3fffff).ram().share("main_ram");
	map(0x10000000, 0x100007ff).rw(FUNC(gsan_state::gpu_r), FUNC(gsan_state::gpu_w));
	// misc I/O
	map(0x14000800, 0x14000807).rw(FUNC(gsan_state::cf_regs_r), FUNC(gsan_state::cf_regs_w));
	map(0x14000c00, 0x14000c03).rw(FUNC(gsan_state::cf_data_r), FUNC(gsan_state::cf_data_w));
	map(0x14001019, 0x14001019).w(FUNC(gsan_state::rtc_w));
	map(0x14001039, 0x14001039).r(FUNC(gsan_state::rtc_r));

	map(0x18000000, 0x18000001).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0x1f000000, 0x1f000fff).ram(); // cache RAM-mode (SH3 internal), actually should be 7Fxxxxxx, but current SH3 core doesn't like 7Fxxxxxx
	map(0xa0000000, 0xa000ffff).rom().region("maincpu", 0); // uncached mirror, otherwise no disassembly can bee seen in debugger (bug?)
}

void gsan_state::main_port(address_map &map)
{
	map(SH3_PORT_L, SH3_PORT_L + 7).portr("PORT_L");
}

void gsan_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x3fffff);
	map(0x000000, 0x3fffff).ram();
}

static void gsan_devices(device_slot_interface &device)
{
	device.option_add("cfcard", ATA_FLASH_PCCARD);
}


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void gsan_state::gsan(machine_config &config)
{
	// basic machine hardware
	// SH7709 is earlier version of SH7709S (cv1k), not exact same, have minor differences
	SH3BE(config, m_maincpu, 32_MHz_XTAL * 2); // not verified
	m_maincpu->set_md(0, 0);  // none of this is verified
	m_maincpu->set_md(1, 0);  // (the sh3 is different to the sh4 anyway, should be changed)
	m_maincpu->set_md(2, 0);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(32_MHz_XTAL * 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gsan_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &gsan_state::main_port);

	ATA_INTERFACE(config, m_ata).options(gsan_devices, "cfcard", nullptr, true);

	RTC4553(config, "rtc");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(gsan_state::screen_update));
	screen.set_size(320, 240);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_refresh_hz(60);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16.9344_MHz_XTAL));
	ymz.set_addrmap(0, &gsan_state::ymz280b_map);
	ymz.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void gsan_state::init_gsan()
{
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0000ffff, 0, memregion("maincpu")->base());
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0c3fffff, 1, memshare("main_ram")->ptr());
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( musclhit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gsan5-a.u17", 0x00000, 0x10000, CRC(6ae1d1e8) SHA1(3224e4b8198aa38c094088456281cbd62c085407) )

	DISK_REGION( "ata:0:cfcard:image" )
	DISK_IMAGE( "gsan6_a-213", 0, SHA1(d9e7a350428d1621fc70e81561390c01837a94c0) )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

GAME( 2000, musclhit,      0, gsan, gsan, gsan_state, init_gsan, ROT0, "Konami / TBS", "Muscle Ranking Kinniku Banzuke Spray Hitter", MACHINE_IS_SKELETON )
