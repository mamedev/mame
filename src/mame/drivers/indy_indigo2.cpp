// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI IP22 Indigo2/Indy workstation
*
*  Known Issues:
*  - The proper hookup for the MAC address is unknown, requiring
*    a fake MAC to be set up before any IRIX installers will proceed.
*  - The IRIX 6.5.x installer kernel-panics on startup.
*  - The IRIX 5.3 installer hangs after loading.
*  - The Gentoo Linux live CD hangs on starting the kernel.
*  - The disk formatting/partitioning utility for IRIX, fx, has
*    various issues, from the disk formatting too quickly to hanging
*    when exercising the disk.
*  - Disk accesses frequently result in a "SYNC negotiation error"
*    message.
*
*  Memory map:
*
*  00000000 - 0007ffff      Alias for first 512kbyte of RAM
*  00080000 - 0008ffff      EISA I/O space (pullups on Indy)
*  00090000 - 0009ffff      EISA I/O space Alias (pullups on Indy)
*  000a0000 - 07ffffff      EISA Memory
*  08000000 - 17ffffff      Low System Memory
*  18000000 - 1effffff      RESERVED - Unused
*  1f000000 - 1f3fffff      GIO - GFX
*  1f400000 - 1f5fffff      GIO - EXP0
*  1f600000 - 1f9fffff      GIO - EXP1 - Unused
*  1fa00000 - 1fa1ffff      Memory Controller
*  1fb00000 - 1fb1a7ff      HPC3 CHIP1
*  1fb80000 - 1fb9a7ff      HPC3 CHIP0
*  1fc00000 - 1fffffff      BIOS
*  20000000 - 2fffffff      High System Memory
*  30000000 - 7fffffff      Reserved
*  80000000 - ffffffff      EISA Memory
*
*  IP22/24 has 2 pieces of PC-compatible hardware: the 8042 PS/2 keyboard/mouse
*  interface and the 8254 PIT.  Both are licensed cores embedded in the IOC custom chip.
*
*  References used:
*    MipsLinux: http://www.mips-linux.org/
*      linux-2.6.6/include/newport.h
*      linux-2.6.6/include/asm-mips/sgi/gio.h
*      linux-2.6.6/include/asm-mips/sgi/mc.h
*      linux-2.6.6/include/asm-mips/sgi/hpc3.h
*    NetBSD: http://www.netbsd.org/
*    gxemul: http://gavare.se/gxemul/
*
*  Gentoo LiveCD r5 boot instructions:
*  - Specify an appropriate LiveCD image at the command line.
*  - Enter the command interpreter and type "sashARCS". Press enter and
*    it will autoboot.
*
*  IRIX boot instructions:
*  - Specify an appropriate IRIX CD image at the command line.
*  - At the menu, choose either "run diagnostics" or "install
*    system software".
*
\*********************************************************************/

#include "emu.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "bus/scsi/scsihd.h"

#include "cpu/mips/mips3.h"

#include "machine/ds1386.h"
#include "machine/hal2.h"
#include "machine/hpc3.h"
#include "machine/ioc2.h"
#include "machine/sgi.h"

#include "sound/cdda.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "video/newport.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class ip22_state : public driver_device
{
public:
	ip22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_mem_ctrl(*this, "memctrl")
		, m_scsi_ctrl(*this, "wd33c93")
		, m_ldac(*this, "ldac")
		, m_rdac(*this, "rdac")
		, m_newport(*this, "newport")
		, m_hal2(*this, HAL2_TAG)
		, m_hpc3(*this, HPC3_TAG)
		, m_ioc2(*this, IOC2_TAG)
		, m_rtc(*this, RTC_TAG)
	{
	}

	void ip22_base(machine_config &config);
	void ip225015(machine_config &config);
	void ip224613(machine_config &config);

protected:
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(unknown_r);
	DECLARE_READ32_MEMBER(enet_r);
	DECLARE_WRITE32_MEMBER(enet_w);
	DECLARE_READ32_MEMBER(eisa_io_r);

	DECLARE_WRITE32_MEMBER(ip22_write_ram);

	void ip22_map(address_map &map);

	static void cdrom_config(device_t *device);

	static char const *const HAL2_TAG;
	static char const *const HPC3_TAG;
	static char const *const IOC2_TAG;
	static char const *const RTC_TAG;

	required_device<mips3_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<sgi_mc_device> m_mem_ctrl;
	required_device<wd33c93_device> m_scsi_ctrl;
	required_device<dac_16bit_r2r_twos_complement_device> m_ldac;
	required_device<dac_16bit_r2r_twos_complement_device> m_rdac;
	required_device<newport_video_device> m_newport;
	required_device<hal2_device> m_hal2;
	required_device<hpc3_device> m_hpc3;
	required_device<ioc2_device> m_ioc2;
	required_device<ds1386_device> m_rtc;

	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ... );
};

/*static*/ char const *const ip22_state::HAL2_TAG = "hal2";
/*static*/ char const *const ip22_state::HPC3_TAG = "hpc3";
/*static*/ char const *const ip22_state::IOC2_TAG = "ioc2";
/*static*/ char const *const ip22_state::RTC_TAG = "rtc";

class ip24_state : public ip22_state
{
public:
	ip24_state(const machine_config &mconfig, device_type type, const char *tag)
		: ip22_state(mconfig, type, tag)
		, m_scsi_ctrl2(*this, "wd33c93_2")
	{
	}

	void ip244415(machine_config &config);

private:
	void ip24_map(address_map &map);

	required_device<wd33c93_device> m_scsi_ctrl2;
};

#define VERBOSE_LEVEL ( 0 )

inline void ATTR_PRINTF(3,4) ip22_state::verboselog(int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%08x: %s", m_maincpu->pc(), buf);
	}
}

READ32_MEMBER(ip22_state::enet_r)
{
	switch (offset)
	{
		case 0x000/4:
			logerror("%s: enet_r: Read MAC Address bytes 0-3, 0x80675309 & %08x\n", machine().describe_context(), mem_mask);
			return 0x80675309;
		default:
			logerror("%s: enet_r: Read Unknown Register %08x & %08x\n", machine().describe_context(), 0x1fbd4000 + (offset << 2), mem_mask);
			return 0;
	}
}

WRITE32_MEMBER(ip22_state::enet_w)
{
	switch (offset)
	{
		default:
			logerror("%s: enet_w: Write Unknown Register %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd4000 + (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(ip22_state::eisa_io_r)
{
	return 0xffffffff;
}

// a bit hackish, but makes the memory detection work properly and allows a big cleanup of the mapping
WRITE32_MEMBER(ip22_state::ip22_write_ram)
{
	// if banks 2 or 3 are enabled, do nothing, we don't support that much memory
	if (m_mem_ctrl->read(space, 0xc8/4, 0xffffffff) & 0x10001000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}

	// if banks 0 or 1 have 2 membanks, also kill it, we only want 128 MB
	if (m_mem_ctrl->read(space, 0xc0/4, 0xffffffff) & 0x40004000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}
	COMBINE_DATA(&m_mainram[offset]);
}

READ32_MEMBER(ip22_state::unknown_r)
{
	return 0x043b2800;
}

void ip22_state::ip22_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).bankrw("bank1");    /* mirror of first 512k of main RAM */
	map(0x00080000, 0x0009ffff).r(FUNC(ip22_state::eisa_io_r));
	map(0x08000000, 0x0fffffff).share("mainram").ram().w(FUNC(ip22_state::ip22_write_ram));     /* 128 MB of main RAM */
	map(0x1f0f0000, 0x1f0f1fff).rw(m_newport, FUNC(newport_video_device::rex3_r), FUNC(newport_video_device::rex3_w));
	//map(0x1f4f0100, 0x1f4f0103).r(FUNC(ip22_state::unknown_r));
	map(0x1fa00000, 0x1fa1ffff).rw(m_mem_ctrl, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));

	map(0x1fb80000, 0x1fb8ffff).rw(m_hpc3, FUNC(hpc3_device::pbusdma_r), FUNC(hpc3_device::pbusdma_w));
	map(0x1fb90000, 0x1fb9ffff).rw(m_hpc3, FUNC(hpc3_device::hd_enet_r), FUNC(hpc3_device::hd_enet_w));
	map(0x1fbb0000, 0x1fbb0003).ram();   /* unknown, but read a lot and discarded */
	map(0x1fbc0000, 0x1fbc7fff).rw(m_hpc3, FUNC(hpc3_device::hd_r<0>), FUNC(hpc3_device::hd_w<0>));
	map(0x1fbd4000, 0x1fbd44ff).rw(FUNC(ip22_state::enet_r), FUNC(ip22_state::enet_w));
	map(0x1fbd8000, 0x1fbd83ff).rw(m_hal2, FUNC(hal2_device::read), FUNC(hal2_device::write));
	map(0x1fbd8400, 0x1fbd87ff).ram(); /* hack */
	map(0x1fbd9000, 0x1fbd93ff).rw(m_hpc3, FUNC(hpc3_device::pbus4_r), FUNC(hpc3_device::pbus4_w));
	map(0x1fbd9800, 0x1fbd9bff).rw(m_ioc2, FUNC(ioc2_device::read), FUNC(ioc2_device::write));
	map(0x1fbdc000, 0x1fbdcfff).rw(m_hpc3, FUNC(hpc3_device::dma_config_r), FUNC(hpc3_device::dma_config_w));
	map(0x1fbdd000, 0x1fbddfff).rw(m_hpc3, FUNC(hpc3_device::pio_config_r), FUNC(hpc3_device::pio_config_w));

	map(0x1fbe0000, 0x1fbe04ff).rw(m_rtc, FUNC(ds1386_device::data_r), FUNC(ds1386_device::data_w)).umask32(0x000000ff);
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
	map(0x20000000, 0x27ffffff).share("mainram").ram().w(FUNC(ip22_state::ip22_write_ram));
}

void ip24_state::ip24_map(address_map &map)
{
	ip22_map(map);
	map(0x1fbc8000, 0x1fbcffff).rw(m_hpc3, FUNC(hpc3_device::hd_r<1>), FUNC(hpc3_device::hd_w<1>));
}

void ip22_state::machine_reset()
{
	// set up low RAM mirror
	membank("bank1")->set_base(m_mainram);

	m_maincpu->mips3drc_set_options(MIPS3DRC_COMPATIBLE_OPTIONS | MIPS3DRC_CHECK_OVERFLOWS);
}

static INPUT_PORTS_START( ip225015 )
	PORT_START("IN0")   // unused IN0
	PORT_START("DSW0")  // unused IN1
	PORT_START("DSW1")  // unused IN2
	PORT_START("DSW2")  // unused IN3
	PORT_INCLUDE( at_keyboard )     /* IN4 - IN11 */
INPUT_PORTS_END

void ip22_state::cdrom_config(device_t *device)
{
	device = device->subdevice("cdda");
	MCFG_SOUND_ROUTE(0, ":lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, ":rspeaker", 1.0)
}

void ip22_state::ip22_base(machine_config &config)
{
	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1280+64, 1024+64);
	screen.set_visarea(0, 1279, 0, 1023);
	screen.set_screen_update("newport", FUNC(newport_video_device::screen_update));
	screen.screen_vblank().set(m_newport, FUNC(newport_video_device::vblank_w));

	PALETTE(config, "palette", 65536);

	NEWPORT_VIDEO(config, m_newport, m_maincpu, m_ioc2);

	SGI_MC(config, m_mem_ctrl);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0);
	m_ldac->add_route(ALL_OUTPUTS, "lspeaker", 0.25);

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0);
	m_rdac->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	voltage_regulator_device &vreg = VOLTAGE_REGULATOR(config, "vref");
	vreg.set_output(5.0);
	vreg.add_route(0, "ldac",  1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "rdac",  1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vreg.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);

	scsi_port_device &scsi(SCSI_PORT(config, "scsi"));
	scsi.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	scsi.set_slot_device(2, "cdrom", SCSICD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));
	scsi.slot(2).set_option_machine_config("cdrom", cdrom_config);

	WD33C93(config, m_scsi_ctrl);
	m_scsi_ctrl->set_scsi_port("scsi");
	m_scsi_ctrl->irq_cb().set(m_hpc3, FUNC(hpc3_device::scsi0_irq));
	//m_scsi_ctrl->drq_cb().set(m_hpc3, FUNC(hpc3_device::scsi0_drq));

	SGI_HAL2(config, m_hal2);
	SGI_IOC2_GUINNESS(config, m_ioc2, m_maincpu);

	DS1386_8K(config, m_rtc, 32768);
}

void ip22_state::ip225015(machine_config &config)
{
	ip22_base(config);

	R5000BE(config, m_maincpu, 50000000*3);
	m_maincpu->set_icache_size(32768);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip22_state::ip22_map);

	SGI_HPC3(config, m_hpc3, m_maincpu, m_scsi_ctrl, m_ioc2, m_ldac, m_rdac);
}

void ip22_state::ip224613(machine_config &config)
{
	ip22_base(config);

	R4600BE(config, m_maincpu, 33333333*4);
	m_maincpu->set_icache_size(32768);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip22_state::ip22_map);

	SGI_HPC3(config, m_hpc3, m_maincpu, m_scsi_ctrl, m_ioc2, m_ldac, m_rdac);
}

void ip24_state::ip244415(machine_config &config)
{
	ip22_base(config);

	R4400BE(config, m_maincpu, 50000000*3);
	m_maincpu->set_icache_size(32768);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);

	SCSI_PORT(config, "scsi2");

	WD33C93(config, m_scsi_ctrl2);
	m_scsi_ctrl2->set_scsi_port("scsi2");
	m_scsi_ctrl->irq_cb().set(m_hpc3, FUNC(hpc3_device::scsi1_irq));
	//m_scsi_ctrl->drq_cb().set(m_hpc3, FUNC(hpc3_device::scsi1_drq));

	SGI_HPC3(config, m_hpc3, m_maincpu, m_scsi_ctrl, m_scsi_ctrl2, m_ioc2, m_ldac, m_rdac);
}

/* SCC init ip225015
 * Channel A
 * 09 <- c0 Master Interrup Control: Force HW reset + enable SWI INTACK
 * 04 <- 44 Clocks: x16 mode, 1 stop bits, no parity
 * 03 <- c0 Receiver: 8 bit data, auto enables, Rx disabled
 * 05 <- e2 Transmitter: DTR set, 8 bit data, RTS set, Tx disabled
 * 0b <- 50 Clock Mode: TRxC: XTAL output, TRxC: Output, TxC from BRG, RxC from BRG
 * 0c <- 0a Low const BRG  3.6864Mhz CLK => 9600 baud
 * 0d <- 00 High Const BRG = (CLK / (2 x Desired Rate x BR Clock period)) - 2
 * 0e <- 01 Mics: BRG enable
 * 03 <- c1 Receiver: as above + Receiver enable
 * 05 <- ea Transmitter: as above + Transmitter enable
 *
 * Channel A and B init - only BRG low const differs
 * 09 <- 80 channel A reset
 * 04 <- 44 Clocks: x16 mode, 1 stop bits, no parity
 * 0f <- 81 External/Status Control: Break/Abort enabled, WR7 prime enabled
 * 07p<- 40 External read enable (RR9=WR3, RR4=WR4, RR5=WR5, RR14=WR7 and RR11=WR10)
 * 03 <- c0 Receiver: 8 bit data, auto enables, Rx disabled
 * 05 <- e2 Transmitter: DTR set, 8 bit data, RTS set, Tx disabled
 * 0b <- 50 Clock Mode: TRxC: XTAL output, TRxC: Output, TxC from BRG, RxC from BRG
 * 0e <- 00 Mics: BRG disable
 * 0c <- 0a/04 Low const BRG, 3.6864Mhz CLK => Chan A:9600 Chan B:38400
 * 0d <- 00 High Const BRG = (CLK / (2 x Desired Rate x BR Clock period)) - 2
 * 0e <- 01 Mics: BRG enable
 * 03 <- c1 Receiver: as above + Receiver enable
 * 05 <- ea Transmitter: as above + Transmitetr enable
 * 00 <- 10 Reset External/status IE
*/
ROM_START( ip225015 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip225015.bin", 0x000000, 0x080000, CRC(aee5502e) SHA1(9243fef0a3508790651e0d6d2705c887629b1280) )
ROM_END

ROM_START( ip224613 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip224613.bin", 0x000000, 0x080000, CRC(f1868b5b) SHA1(0dcbbd776e671785b9b65f3c6dbd609794a40157) )
ROM_END

ROM_START( ip244415 )
	ROM_REGION( 0x80000, "user1", 0 )
	ROM_LOAD( "ip244415.bin", 0x000000, 0x080000, CRC(2f37825a) SHA1(0d48c573b53a307478820b85aacb57b868297ca3) )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS       INIT        COMPANY                 FULLNAME                   FLAGS
COMP( 1993, ip225015, 0,      0,      ip225015, ip225015, ip22_state, empty_init, "Silicon Graphics Inc", "Indy (R5000, 150MHz)",    MACHINE_NOT_WORKING )
COMP( 1993, ip224613, 0,      0,      ip224613, ip225015, ip22_state, empty_init, "Silicon Graphics Inc", "Indy (R4600, 133MHz)",    MACHINE_NOT_WORKING )
COMP( 1994, ip244415, 0,      0,      ip244415, ip225015, ip24_state, empty_init, "Silicon Graphics Inc", "Indigo2 (R4400, 150MHz)", MACHINE_NOT_WORKING )
