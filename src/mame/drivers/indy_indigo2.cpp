// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI IP22 Indigo2/Indy workstation
*
*   Todo: Fix tod clock set problem
*         Fix NVRAM saving
*         Fix SCSI DMA to handle chains properly
*         Probably many more things
*
*  Memory map:
*
*  18000000 - 1effffff      RESERVED - Unused
*  1f000000 - 1f3fffff      GIO - GFX
*  1f400000 - 1f5fffff      GIO - EXP0
*  1f600000 - 1f9fffff      GIO - EXP1 - Unused
*  1fa00000 - 1fa02047      Memory Controller
*  1fb00000 - 1fb1a7ff      HPC3 CHIP1
*  1fb80000 - 1fb9a7ff      HPC3 CHIP0
*  1fc00000 - 1fc7ffff      BIOS
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
* Gentoo LiveCD r5 boot instructions:
*     mess -cdrom gentoor5.chd ip225015
*     enter the command interpreter and type "sashARCS".  press enter and
*     it'll autoboot.
*
* IRIX boot instructions:
*     mess -cdrom irix656inst1.chd ip225015
*     at the menu, choose either "run diagnostics" or "install system software"
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

#include "screen.h"
#include "speaker.h"

class ip22_state : public driver_device
{
public:
	ip22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_sgi_mc(*this, "sgi_mc")
		, m_newport(*this, "newport")
		, m_dac(*this, "dac")
		, m_scsi(*this, "wd33c93")
		, m_hal2(*this, HAL2_TAG)
		, m_hpc3(*this, HPC3_TAG)
		, m_ioc2(*this, "ioc2")
		, m_rtc(*this, RTC_TAG)
	{
	}

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE32_MEMBER(ip22_write_ram);

	void init_ip225015();
	void ip225015(machine_config &config);
	void ip224613(machine_config &config);
	void ip244415(machine_config &config);
	void ip225015_map(address_map &map);

	static void cdrom_config(device_t *device);

	static const char* HAL2_TAG;
	static const char* HPC3_TAG;
	static const char* RTC_TAG;

protected:
	required_device<mips3_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<sgi_mc_device> m_sgi_mc;
	required_device<newport_video_device> m_newport;
	required_device<dac_word_interface> m_dac;
	required_device<wd33c93_device> m_scsi;
	required_device<hal2_device> m_hal2;
	required_device<hpc3_device> m_hpc3;
	required_device<ioc2_device> m_ioc2;
	required_device<ds1386_device> m_rtc;

	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ... );
};

/*static*/ const char* ip22_state::HAL2_TAG = "hal2";
/*static*/ const char* ip22_state::HPC3_TAG = "hpc3";
/*static*/ const char* ip22_state::RTC_TAG = "rtc";

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

// a bit hackish, but makes the memory detection work properly and allows a big cleanup of the mapping
WRITE32_MEMBER(ip22_state::ip22_write_ram)
{
	// if banks 2 or 3 are enabled, do nothing, we don't support that much memory
	if (m_sgi_mc->read(space, 0xc8/4, 0xffffffff) & 0x10001000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}

	// if banks 0 or 1 have 2 membanks, also kill it, we only want 128 MB
	if (m_sgi_mc->read(space, 0xc0/4, 0xffffffff) & 0x40004000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}
	COMBINE_DATA(&m_mainram[offset]);
}

void ip22_state::ip225015_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).bankrw("bank1");    /* mirror of first 512k of main RAM */
	map(0x08000000, 0x0fffffff).share("mainram").ram().w(this, FUNC(ip22_state::ip22_write_ram));     /* 128 MB of main RAM */
	map(0x1f0f0000, 0x1f0f1fff).rw(m_newport, FUNC(newport_video_device::rex3_r), FUNC(newport_video_device::rex3_w));
	map(0x1fa00000, 0x1fa1ffff).rw(m_sgi_mc, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fb90000, 0x1fb9ffff).rw(m_hpc3, FUNC(hpc3_device::hd_enet_r), FUNC(hpc3_device::hd_enet_w));
	map(0x1fbb0000, 0x1fbb0003).ram();   /* unknown, but read a lot and discarded */
	map(0x1fbc0000, 0x1fbc7fff).rw(m_hpc3, FUNC(hpc3_device::hd0_r), FUNC(hpc3_device::hd0_w));
	map(0x1fbc8000, 0x1fbcffff).rw(m_hpc3, FUNC(hpc3_device::unkpbus0_r), FUNC(hpc3_device::unkpbus0_w)).share("unkpbus0");
	map(0x1fb80000, 0x1fb8ffff).rw(m_hpc3, FUNC(hpc3_device::pbusdma_r), FUNC(hpc3_device::pbusdma_w));
	map(0x1fbd8000, 0x1fbd83ff).rw(m_hal2, FUNC(hal2_device::read), FUNC(hal2_device::write));
	map(0x1fbd8400, 0x1fbd87ff).ram(); /* hack */
	map(0x1fbd9000, 0x1fbd93ff).rw(m_hpc3, FUNC(hpc3_device::pbus4_r), FUNC(hpc3_device::pbus4_w));
	map(0x1fbd9800, 0x1fbd9bff).rw(m_ioc2, FUNC(ioc2_device::read), FUNC(ioc2_device::write));
	map(0x1fbdc000, 0x1fbdc7ff).ram();
	map(0x1fbdd000, 0x1fbdd3ff).ram();
	map(0x1fbe0000, 0x1fbe04ff).rw(m_rtc, FUNC(ds1386_device::data_r), FUNC(ds1386_device::data_w)).umask32(0x000000ff);
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
	map(0x20000000, 0x27ffffff).share("mainram").ram().w(this, FUNC(ip22_state::ip22_write_ram));
}

void ip22_state::machine_reset()
{
	// set up low RAM mirror
	membank("bank1")->set_base(m_mainram);

	m_maincpu->mips3drc_set_options(MIPS3DRC_COMPATIBLE_OPTIONS | MIPS3DRC_CHECK_OVERFLOWS);
}

void ip22_state::machine_start()
{
}

void ip22_state::init_ip225015()
{
	// IP22 uses 2 pieces of PC-compatible hardware: the 8042 PS/2 keyboard/mouse
	// interface and the 8254 PIT.  Both are licensed cores embedded in the IOC custom chip.
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

MACHINE_CONFIG_START(ip22_state::ip225015)
	MCFG_DEVICE_ADD("maincpu", R5000BE, 50000000*3)
	//MCFG_MIPS3_ICACHE_SIZE(32768)
	//MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_DEVICE_PROGRAM_MAP(ip225015_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1280+64, 1024+64)
	MCFG_SCREEN_VISIBLE_AREA(0, 1279, 0, 1023)
	MCFG_SCREEN_UPDATE_DEVICE("newport", newport_video_device, screen_update)

	MCFG_PALETTE_ADD("palette", 65536)

	MCFG_NEWPORT_ADD("newport")

	MCFG_DEVICE_ADD("sgi_mc", SGI_MC, 0)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("dac", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_1)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "cdrom", SCSICD, SCSI_ID_4)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("cdrom", cdrom_config)

	MCFG_DEVICE_ADD("wd33c93", WD33C93, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_WD33C93_IRQ_CB(WRITELINE(HPC3_TAG, hpc3_device, scsi_irq))

	MCFG_SGI_HAL2_ADD(HAL2_TAG)

	MCFG_IOC2_GUINNESS_ADD("ioc2")
	MCFG_IOC2_CPU("maincpu")

	MCFG_SGI_HPC3_ADD(HPC3_TAG)
	MCFG_HPC3_CPU_TAG("maincpu")
	MCFG_HPC3_SCSI_TAG("wd33c93")
	MCFG_HPC3_IOC2_TAG("ioc2")

	MCFG_DS1386_8K_ADD(RTC_TAG, 32768)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(ip22_state::ip224613)
	ip225015(config);
	MCFG_DEVICE_REPLACE("maincpu", R4600BE, 133333333)
	//MCFG_MIPS3_ICACHE_SIZE(32768)
	//MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_DEVICE_PROGRAM_MAP( ip225015_map)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(ip22_state::ip244415)
	ip225015(config);
	MCFG_DEVICE_REPLACE("maincpu", R4600BE, 150000000)
	//MCFG_MIPS3_ICACHE_SIZE(32768)
	//MCFG_MIPS3_DCACHE_SIZE(32768)
	MCFG_DEVICE_PROGRAM_MAP(ip225015_map)
MACHINE_CONFIG_END

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

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS       INIT           COMPANY                 FULLNAME                   FLAGS
COMP( 1993, ip225015, 0,      0,      ip225015, ip225015, ip22_state, init_ip225015, "Silicon Graphics Inc", "Indy (R5000, 150MHz)",    MACHINE_NOT_WORKING )
COMP( 1993, ip224613, 0,      0,      ip224613, ip225015, ip22_state, init_ip225015, "Silicon Graphics Inc", "Indy (R4600, 133MHz)",    MACHINE_NOT_WORKING )
COMP( 1994, ip244415, 0,      0,      ip244415, ip225015, ip22_state, init_ip225015, "Silicon Graphics Inc", "Indigo2 (R4400, 150MHz)", MACHINE_NOT_WORKING )
