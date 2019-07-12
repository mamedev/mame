// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Play Mechanix / Right Hand Tech "VP50", "VP100" and "VP101" platforms
    (PCBs are also marked "Raw Thrills" but all RT games appear to be on PC hardware)

    Boards:
        - VP101: Johnny Nero.  The original (?)
        - VP100: Special Forces Elite Training.  A not-quite-complete VP101; missing ATA DMA.
        - VP50 : Zoofari.  Cost-reduced (?) with TX4925 SoC, much less complex FPGA.
               : Rhythm Nation.

    Preliminary driver by R. Belmont

    TODO:
        - All games: that formidable sounding 3D accelerator mentioned below.
        - All games: the sound system (the POST plays some example sounds)
        - Zoofari's framebuffer is 256 color but I don't know where the CLUT comes from.

    To make the games go into a POST test, hold down START 1 while resetting.

    VP101 Features from http://web.archive.org/web/20041016000248/http://www.righthandtech.com/projects.htm

    MIPS VR5500 CPU
        The VR5500 operates at either at 300 or 400 MHz with 120MHz external bus
        MIPS 64-bit RISC architecture
        Two-way super-scalar super pipeline
        On-chip floating-point unit (FPU)
        High-speed translation look-aside buffer (TLB)(48 double-entries)
        On-chip primary cache memory (instruction/data: 32 KB each)
        2-way set associative, Supports line lock feature
        Conforms to MIPS I, II, III, and IV instruction sets. Also supports product-sum operation instruction, rotate instruction, register scan instruction
        Six execution units (ALU0, ALU1, FPU, FPU/MAC, BRU, and LSU)
        Employment of out-of-order execution mechanism
        Branch prediction mechanism - Branch history table with 4K entries
        Support for CPU emulator connection via JTAG/n-Wire port

    Unified Memory Architecture - DDR SDRAM bank
        Arbitrating DDR SDRAM Memory controller
        128Mbyte to 512Mbyte memory capacity
        120/240 MHz @ 64 bits - ~2GBytes/sec bandwidth

    3D Render Engine
        True color and 8-bit palette lookup textures
        8K byte texel cache for accelerated source texel selection.
        Perspective corrected rendering
        Bi-linear filter for source texel scaling
        256 Color Palette Lookup (888 RGB plus 8 bit Source Palette Alpha)
        True Color Source Textures (888 RGB plus 8 bit Alpha)
        24 bit Z-buffer structure in DDR SDRAM buffer
        Per-vertex colored lighting
        Alpha channel structure in DDR SDRAM buffer
        Pixel processing effects (fog, night, etc.)
        888 RGB Video DAC output section.
        Bitmap structure in DDR SDRAM with DMA for screen update
        Flexible CRT controller with X/Y gun interface counters

    Game I/O
        Standard JAMMA I/O interface, including player 3 and 4 connectors
        4 channel general purpose A to D interface (steering wheel and control pedals)
        100baseT Ethernet interface for debugging and/or inter game communications
        Forced-feedback “Wheel Driver Interface” for driving games
        High-current drivers for lamps or solenoids
        Gun interface I/O tightly coupled to the CRT controller

    Sound System
        AC97 codec for low cost of implementation and development
        TDA7375 40 Watt Integrated Amplifier
        Codec fed from the DDR bank via a 16 channel (8 channels of stereo) DMA engine.

    ATA/IDE Disk Drive Interface
        Standard ATA/IDE interface
        Ultra DMA 33/66/100/133 to the DDR SDRAM memory

    Video DAC
        RGB values at 8 bits per color
        RGB voltage level adjustable from 0-1.0 Vp-p to 0-4.0 Vp-p

    Flash Memory
        Minimum of 1MB of Flash memory – expandable to 4 MB
        Updateable Boot ROM
        Updateable FPGA configuration

    Battery Backed Up RAM
        32K bytes of non-volatile memory for static game configuration and high score table
        Non-volatile Real-Time clock

    Small Footprint
        Small outline design for easy kit retrofitting of existing cabinet
        12.2 in x 14.96 in

    Security Interface
        Security processor provides for a means to “unlock” the FPGA functions
        Enabled for software protection against piracy and unwarranted game updates

Full populated and tested board is less than $500, including IDE hard disk.
Small outline design for easy kit retrofitting of existing cabinets.

****************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "machine/ataintf.h"
#include "machine/nvram.h"
#include "imagedev/harddriv.h"
#include "screen.h"

class vp10x_state : public driver_device
{
public:
	vp10x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mainram(*this, "mainram"),
			m_ata(*this, "ata"),
			m_in0(*this, "IN0")
	{ }

	void vp50(machine_config &config);
	void vp101(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;

	DECLARE_READ32_MEMBER(tty_ready_r);
	DECLARE_WRITE32_MEMBER(tty_w);
	DECLARE_READ32_MEMBER(test_r) { return 0xffffffff; }

	DECLARE_READ32_MEMBER(pic_r);
	DECLARE_WRITE32_MEMBER(pic_w);

	DECLARE_WRITE32_MEMBER(dmaaddr_w);

	DECLARE_WRITE_LINE_MEMBER(dmarq_w);

	DECLARE_READ32_MEMBER(tty_4925_rdy_r) { return 0x2; }

	DECLARE_READ32_MEMBER(spi_status_r) { return 0x8007; }

	DECLARE_READ32_MEMBER(spi_r);
	DECLARE_WRITE32_MEMBER(spi_w);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t vp50_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void vp50_map(address_map &map);

	// devices
	required_device<mips3_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<ata_interface_device> m_ata;
	required_ioport m_in0;

	// driver_device overrides
	virtual void video_start() override;
	int pic_cmd;
	int pic_state;
	int m_dmarq_state;
	uint32_t m_dma_ptr;
	uint32_t m_spi_select;
};

void vp10x_state::machine_reset()
{
	m_dmarq_state = 0;
	pic_cmd = pic_state = 0;
}

void vp10x_state::machine_start()
{
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS);
//  m_maincpu->add_fastram(0x00000000, 0x03ffffff, false, m_mainram);
}

WRITE32_MEMBER(vp10x_state::dmaaddr_w)
{
	m_dma_ptr = (data & 0x07ffffff);
}

WRITE_LINE_MEMBER(vp10x_state::dmarq_w)
{
	if (state != m_dmarq_state)
	{
		m_dmarq_state = state;

		if (state)
		{
			uint16_t *RAMbase = (uint16_t *)&m_mainram[0];
			uint16_t *RAM = &RAMbase[m_dma_ptr>>1];

			m_ata->write_dmack(ASSERT_LINE);

			while (m_dmarq_state)
			{
				*RAM++ = m_ata->read_dma();
				m_dma_ptr += 2; // pointer must advance
			}

			m_ata->write_dmack(CLEAR_LINE);
		}
	}
}

READ32_MEMBER(vp10x_state::pic_r)
{
	static const uint8_t vers[5] = { 0x00, 0x01, 0x00, 0x00, 0x00 };
	static const uint8_t serial[10] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };
	static const uint8_t magic[10] = { 0xaa, 0x55, 0x18, 0x18, 0xc0, 0x03, 0xf0, 0x0f, 0x09, 0x0a };

	switch (pic_cmd)
	{
		case 0x20:
			return vers[pic_state++];

		case 0x21:
		case 0x22:
			return serial[pic_state++];

		case 0x23:  // this is the same for jnero and specfrce.  great security!
			return magic[pic_state++];
	}

	return 0;
}

WRITE32_MEMBER(vp10x_state::pic_w)
{
	//printf("%02x to pic_cmd\n", data&0xff);
	if ((data & 0xff) == 0)
	{
		return;
	}
	pic_cmd = data & 0xff;
	pic_state = 0;
}

READ32_MEMBER(vp10x_state::spi_r)
{
	return 0xffffffff;
}

WRITE32_MEMBER(vp10x_state::spi_w)
{
//  printf("%d to SPI select\n", data);
	m_spi_select = data;
}

void vp10x_state::video_start()
{
}

uint32_t vp10x_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 240; y++)
	{
		uint32_t *line = &bitmap.pix32(y);
		const uint32_t *video_ram = (const uint32_t *) &m_mainram[(0x7400000/4) + (y * (0x1000/4)) + 4];

		for (int x = 0; x < 320; x++)
		{
			uint32_t word = *(video_ram++);
			video_ram++;
			*line++ = word;
		}
	}
	return 0;
}

// TODO: Palette is not at 0, where is it?
uint32_t vp10x_state::vp50_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint16_t *pal_ram = (const uint16_t *) &m_mainram[0];

	for (int y = 0; y < 240; y++)
	{
		uint32_t *line = &bitmap.pix32(y);
		const uint8_t *video_ram = (const uint8_t *) &m_mainram[(0x10000/4)+(y * 100)];

		for (int x = 0; x < 400; x++)
		{
			// assume 565
			int r = pal_ram[video_ram[x]] >> 11;
			int g = (pal_ram[video_ram[x]] >> 5) & 0x3f;
			int b = pal_ram[video_ram[x]] & 0x1f;

			*line++ = (r << 19) | (g << 10) | (b << 3);
		}
	}
	return 0;
}

READ32_MEMBER(vp10x_state::tty_ready_r)
{
	return 0x60;    // must return &0x20 for output at tty_w to continue
}

WRITE32_MEMBER(vp10x_state::tty_w)  // set breakpoint at bfc01430 to catch when it's printing things
{
// uncomment to see startup messages - it says "RAM OK" and "EPI RSS Ver 4.5.1" followed by "<RSS active>" and then lots of dots
// Special Forces also says "<inited tv_cap> = 00000032"
//  printf("%c", data);
}

void vp10x_state::main_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram().share("mainram");
	map(0x14000000, 0x14000003).r(FUNC(vp10x_state::test_r));
	map(0x1c000000, 0x1c000003).w(FUNC(vp10x_state::tty_w));        // RSS OS code uses this one
	map(0x1c000014, 0x1c000017).r(FUNC(vp10x_state::tty_ready_r));
	map(0x1c400000, 0x1c400003).w(FUNC(vp10x_state::tty_w));        // boot ROM code uses this one
	map(0x1c400014, 0x1c400017).r(FUNC(vp10x_state::tty_ready_r));
	map(0x1ca0000c, 0x1ca0000f).portr("IN0");
	map(0x1ca00010, 0x1ca00013).r(FUNC(vp10x_state::test_r));        // bits here cause various test mode stuff
	map(0x1cf00000, 0x1cf00003).noprw().nopr();
	map(0x1d000030, 0x1d000033).w(FUNC(vp10x_state::dmaaddr_w));    // ATA DMA destination address
	map(0x1d000040, 0x1d00005f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask32(0x0000ffff);
	map(0x1d000060, 0x1d00007f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)).umask32(0x0000ffff);
	map(0x1f200000, 0x1f200003).rw(FUNC(vp10x_state::pic_r), FUNC(vp10x_state::pic_w));
	map(0x1f807000, 0x1f807fff).ram().share("nvram");
	map(0x1fc00000, 0x1fffffff).rom().region("maincpu", 0);
}

void vp10x_state::vp50_map(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram().share("mainram");
	map(0x1f000010, 0x1f00001f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));
	map(0x1f000020, 0x1f00002f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x1f400000, 0x1f400003).noprw(); // FPGA bitstream download?
	map(0x1f400800, 0x1f400bff).ram().share("nvram");
	map(0x1fc00000, 0x1fffffff).rom().region("maincpu", 0);

	// TX4925 peripherals
	map(0xff1ff40c, 0xff1ff40f).r(FUNC(vp10x_state::tty_4925_rdy_r));
	map(0xff1ff41c, 0xff1ff41f).w(FUNC(vp10x_state::tty_w));
	map(0xff1ff500, 0xff1ff503).noprw();
	map(0xff1ff814, 0xff1ff817).r(FUNC(vp10x_state::spi_status_r));
	map(0xff1ff818, 0xff1ff81b).rw(FUNC(vp10x_state::spi_r), FUNC(vp10x_state::spi_w));
}

static INPUT_PORTS_START( vp101 )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000008,  IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0xfffffff0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( vp50 )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000008,  IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0xfffffff0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

void vp10x_state::vp101(machine_config &config)
{
	VR5500LE(config, m_maincpu, 400000000);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_system_clock(100000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vp10x_state::main_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(vp10x_state::screen_update));
	screen.set_size(320, 240);
	screen.set_visarea(0, 319, 0, 239);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->dmarq_handler().set(FUNC(vp10x_state::dmarq_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void vp10x_state::vp50(machine_config &config)
{
	TX4925LE(config, m_maincpu, 200000000);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_system_clock(100000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vp10x_state::vp50_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(vp10x_state::vp50_screen_update));
	screen.set_size(400, 240);
	screen.set_visarea(0, 399, 0, 239);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

ROM_START(jnero)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "d710.05523.bin", 0x000000, 0x100000, CRC(6054a066) SHA1(58e68b7d86e6f24c79b99c8406e86e3c14387726) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 program - read-protected, need dumped */
	ROM_LOAD( "8722a-1206.bin", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )    /* ideally an IDENTIFY page from a real drive should be the IDTN metadata,
	                                       but even factory-new boardsets came with a variety of HDD makes and models */
	DISK_IMAGE_READONLY("jn010108", 0, SHA1(5a27990478b65fca801c3a6518c519c5b4ca934d) )
ROM_END

ROM_START(specfrce)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_SYSTEM_BIOS(0, "default", "rev. 3.6")
	ROMX_LOAD( "boot 3.6.u4.27c801", 0x000000, 0x100000, CRC(b1628dd9) SHA1(5970d31b0cf3d0c1ab4b10ee8e54d2696fafde24), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "r35", "rev. 3.5")
	ROMX_LOAD( "special_forces_boot_v3.5.u4", 0x000000, 0x100000, CRC(ae8dfdf0) SHA1(d64130e710d0c70095ad8ebd4e2194b8c461be4a), ROM_BIOS(1) ) /* Newer, but keep both in driver */
	ROM_SYSTEM_BIOS(2, "r34", "rev. 3.4")
	ROMX_LOAD( "special_forces_boot_v3.4.u4", 0x000000, 0x100000, CRC(db4862ac) SHA1(a1e886d424cf7d26605e29d972d48e8d44ae2d58), ROM_BIOS(2) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 I/P program - read-protected, need dumped */
	ROM_LOAD( "special_forces_et_u7_rev1.2.u7", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("sf010200", 0, SHA1(33c35fd5e110ff06330e0f0313fcd75d5c64a090) )
ROM_END

ROM_START(specfrceo)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_SYSTEM_BIOS(0, "default", "rev. 3.6")
	ROMX_LOAD( "boot 3.6.u4.27c801", 0x000000, 0x100000, CRC(b1628dd9) SHA1(5970d31b0cf3d0c1ab4b10ee8e54d2696fafde24), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "r35", "rev. 3.5")
	ROMX_LOAD( "special_forces_boot_v3.5.u4", 0x000000, 0x100000, CRC(ae8dfdf0) SHA1(d64130e710d0c70095ad8ebd4e2194b8c461be4a), ROM_BIOS(1) ) /* Newer, but keep both in driver */
	ROM_SYSTEM_BIOS(2, "r34", "rev. 3.4")
	ROMX_LOAD( "special_forces_boot_v3.4.u4", 0x000000, 0x100000, CRC(db4862ac) SHA1(a1e886d424cf7d26605e29d972d48e8d44ae2d58), ROM_BIOS(2) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 I/P program - read-protected, need dumped */
	ROM_LOAD( "special_forces_et_u7_rev1.2.u7", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("sf010101", 0, SHA1(59b5e3d8e1d5537204233598830be2066aad0556) )
ROM_END

ROM_START(zoofari)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "zf_boot_rel.u13", 0x000000, 0x400000, CRC(e629689a) SHA1(7352d033c638040c3e51a453e2440a7f38a1b406) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c422 program - read-protected, need dumped */
	ROM_LOAD( "8777z-568.bin", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("zoofari", 0, SHA1(8fb9cfb1ab2660f40b643fcd772243903bd69a6c) )
ROM_END

ROM_START(rhnation)
	ROM_REGION(0x400000, "maincpu", 0)  /* Boot ROM */
	ROM_LOAD( "rhythm_nation_rev_3.1.5_m27v322.u13", 0x000000, 0x400000, CRC(456f043d) SHA1(cc166897fdbdaa3583e44816da9dfbbf303f5c61) )

	ROM_REGION(0x80000, "pic", 0)       /* PIC18c242 program - read-protected, need dumped */
	ROM_LOAD( " pic18c242-i-sp.u22", 0x000000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY("rhn010104", 0, SHA1(5bc2e5817b29bf42ec483414242795fd76d749d9) )
ROM_END

GAME( 2002,  specfrce,  0,          vp101,  vp101, vp10x_state, empty_init, ROT0, "ICE/Play Mechanix",    "Special Forces Elite Training (v01.02.00)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002,  specfrceo, specfrce,   vp101,  vp101, vp10x_state, empty_init, ROT0, "ICE/Play Mechanix",    "Special Forces Elite Training (v01.01.01)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003,  rhnation,  0,          vp50,   vp50,  vp10x_state, empty_init, ROT0, "ICE/Play Mechanix",    "Rhythm Nation (v01.00.04)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2004,  jnero,     0,          vp101,  vp101, vp10x_state, empty_init, ROT0, "ICE/Play Mechanix",    "Johnny Nero Action Hero (v01.01.08)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2006,  zoofari,   0,          vp50,   vp50,  vp10x_state, empty_init, ROT0, "ICE/Play Mechanix",    "Zoofari",                                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
