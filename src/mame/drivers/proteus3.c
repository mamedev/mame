// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************

    Proteus III computer.

    2015-10-02 Skeleton [Robbbert]

    Chips:
    6800 @ 894kHz
    6850 (TTY interface)
    6850 (Cassette interface)
    6820 (PIA for Keyboard and video
    6844 DMA
    MC14411 baud rate generator
    CRT96364 CRTC @1008kHz

    To Do:
    - Everything

******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/keyboard.h"


class proteus3_state : public driver_device
{
public:
	proteus3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
	{ }

	DECLARE_WRITE_LINE_MEMBER(ca2_w);
	DECLARE_WRITE8_MEMBER(video_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_video_data;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	//required_device<acia6850_device> m_acia;
};




/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(proteus3_mem, AS_PROGRAM, 8, proteus3_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8004, 0x8007) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0x8008, 0x8008) AM_DEVREADWRITE("acia1", acia6850_device, status_r, control_w)
	AM_RANGE(0x8009, 0x8009) AM_DEVREADWRITE("acia1", acia6850_device, data_r, data_w)
	AM_RANGE(0x8010, 0x8010) AM_DEVREADWRITE("acia2", acia6850_device, status_r, control_w)
	AM_RANGE(0x8011, 0x8011) AM_DEVREADWRITE("acia2", acia6850_device, data_r, data_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(proteus3)
INPUT_PORTS_END

WRITE8_MEMBER( proteus3_state::kbd_put )
{
	m_pia->portb_w(data);
	m_pia->cb1_w(1);
	m_pia->cb1_w(0);
}

WRITE8_MEMBER( proteus3_state::video_w )
{
	m_video_data = data;
}

WRITE_LINE_MEMBER( proteus3_state::ca2_w )
{
	if (state)
		printf("%c", m_video_data);
}

void proteus3_state::machine_reset()
{
}


/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( proteus3, proteus3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_3_579545MHz)  /* Divided by 4 internally */
	MCFG_CPU_PROGRAM_MAP(proteus3_mem)


	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(proteus3_state, video_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(proteus3_state, ca2_w))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD ("acia1", ACIA6850, 0)
	MCFG_DEVICE_ADD ("acia2", ACIA6850, 0)
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(proteus3_state, kbd_put))
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(proteus3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "proteus3_basic8k.m0", 0xe000, 0x2000, CRC(7d9111c2) SHA1(3c032c9c7f87d22a1a9819b3b812be84404d2ad2) )
	ROM_RELOAD( 0xc000, 0x2000 )

	ROM_REGION(0x400, "chargen", 0)
	ROM_LOAD( "proteus3_font.m25",   0x0000, 0x0400, CRC(6a3a30a5) SHA1(ab39bf09722928483e497b87ac2dbd870828893b) )

	ROM_REGION(0xf000, "user1", 0) // roms not used yet
	// Proteus III - pbug
	ROM_LOAD( "proteus3_pbug.bin", 0x0000, 0x0800, CRC(1118694d) SHA1(2dfc08d405e8f2936f5b0bd1c4007995151abbba) )
	// Proteus III - 14k basic - single-rom
	ROM_LOAD( "proteus3_basic14k.bin", 0x0800, 0x3800, CRC(e0626ff5) SHA1(755e8c1aa71342a0f78ff3ba8b93d038b5b1301e) )
	// Proteus III - 14k basic - split roms
	ROM_LOAD( "bas1.bin",     0x4000, 0x0800, CRC(016bf2d6) SHA1(89605dbede3b6fd101ee0548e5c545a0824fcfd3) )
	ROM_LOAD( "bas2.bin",     0x4800, 0x0800, CRC(39d3e543) SHA1(dd0fe220e3c2a48ce84936301311cbe9f1597ca7) )
	ROM_LOAD( "bas3.bin",     0x5000, 0x0800, CRC(3a41617d) SHA1(175406f4732389e226bc50d27ada39e6ea48de34) )
	ROM_LOAD( "bas4.bin",     0x5800, 0x0800, CRC(ee9d77ee) SHA1(f7e60a1ab88a3accc8ffdc545657c071934d09d2) )
	ROM_LOAD( "bas5.bin",     0x6000, 0x0800, CRC(bd81bb34) SHA1(6325735e5750a9536e63b67048f74711fae1fa42) )
	ROM_LOAD( "bas6.bin",     0x6800, 0x0800, CRC(60cd006b) SHA1(28354f78490da1eb5116cbbc43eaca0670f7f398) )
	ROM_LOAD( "bas7.bin",     0x7000, 0x0800, CRC(84c3dc22) SHA1(8fddba61b5f0270ca2daef32ab5edfd60300c776) )
	// Micro-Systemes I - 8k basic - single rom
	ROM_LOAD( "ms1_basic8k.bin", 0x7800, 0x2000, CRC(b5476e28) SHA1(c8c2366d549b2645c740be4ab4237e05c3cab4a9) )
	// Micro-Systemes I - 8k basic - split roms
	ROM_LOAD( "eprom1",       0xa000, 0x0480, CRC(de20c8a2) SHA1(2c62410888c8418990ff773578d350358e73b505) )
	ROM_LOAD( "eprom1b",      0xa800, 0x0480, CRC(a168830c) SHA1(c39dd955295b1b18e21374d0bc361fb95f8767cd) )
	ROM_LOAD( "eprom2",       0xb000, 0x0480, CRC(fa13fa1e) SHA1(f5e5aab9dc2eecdb587520bcac8a61eb10b96ba2) )
	ROM_LOAD( "eprom3",       0xb800, 0x0480, CRC(6fbf88d1) SHA1(d949159c8a7201b38fae5955e8a79fac9beb873c) )
	ROM_LOAD( "eprom4",       0xc000, 0x0480, CRC(84e0d659) SHA1(d49119d8338ce060e10c43f5c3ada792de48bf68) )
	ROM_LOAD( "eprom5",       0xc800, 0x0480, CRC(e550fb62) SHA1(ebef2e3f76eeca422c896e186cc6ad82185c1db9) )
	ROM_LOAD( "eprom6",       0xd000, 0x0480, CRC(c816d374) SHA1(b56db1e673273bc5cc4d5efaa257314d11d9b625) )
	ROM_LOAD( "eprom7",       0xd800, 0x0480, CRC(d863a23f) SHA1(f23b0d5e81f85890f50fda5778adc86354330a7f) )
	ROM_LOAD( "eprom8",       0xe000, 0x0480, CRC(034f2295) SHA1(80916c2fcd38ce474d47c75e82f1c5e578fb5f26) )
	ROM_LOAD( "eprom8b",      0xe800, 0x0480, CRC(b8144b85) SHA1(ee0d6f495f52d2dc846d8d29cf37c72454ddd6c2) )
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT     CLASS          INIT      COMPANY                     FULLNAME        FLAGS */
COMP( 1978, proteus3,   0,          0,      proteus3,   proteus3, driver_device,   0,     "Proteus International", "Proteus III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
