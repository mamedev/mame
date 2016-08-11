// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Tektronix 440x "AI Workstations"

    skeleton by R. Belmont

    Hardware overview:
        * 68010 (4404) or 68020 (4405) with custom MMU
        * Intelligent floppy subsystem with 6502 driving a uPD765 controller
        * NS32081 FPU
        * 6551 debug console AICA
        * SN76496 PSG for sound
        * MC146818 RTC
        * MC68681 DUART / timer (3.6864 MHz clock) (serial channel A = keyboard, channel B = RS-232 port)
        * AM9513 timer (source of timer IRQ)
        * NCR5385 SCSI controller

        Video is a 640x480 1bpp window on a 1024x1024 VRAM area; smooth panning around that area
        is possible as is flat-out changing the scanout address.

    IRQ levels:
        7 = Debug (NMI)
        6 = VBL
        5 = UART
        4 = Spare (exp slots)
        3 = SCSI
        2 = DMA
        1 = Timer
        0 = Unused

    MMU info:
        Map control register (location unk): bit 15 = VM enable, bits 10-8 = process ID

        Map entries:
            bit 15 = dirty
            bit 14 = write enable
            bit 13-11 = process ID
            bits 10-0 = address bits 22-12 in the final address

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6551.h"    // debug tty
#include "machine/mc146818.h"
#include "bus/rs232/rs232.h"

class tek440x_state : public driver_device
{
public:
	tek440x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fdccpu(*this, "fdccpu"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram")
	{}

	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<m68010_device> m_maincpu;
	required_device<m6502_device> m_fdccpu;
	required_shared_ptr<UINT16> m_mainram;
	required_shared_ptr<UINT16> m_vram;
};

/*************************************
 *
 *  Machine start
 *
 *************************************/

void tek440x_state::machine_start()
{
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void tek440x_state::machine_reset()
{
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 *RAM = (UINT8 *)m_mainram.target();

	memcpy(RAM, ROM, 256);

	m_maincpu->reset();
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

UINT32 tek440x_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT16 *video_ram;
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	for (y = 0; y < 480; y++)
	{
		line = &bitmap.pix16(y);
		video_ram = &m_vram[y * 64];

		for (x = 0; x < 640; x += 16)
		{
			word = *(video_ram++);
			for (b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}

	return 0;
}



/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 16, tek440x_state )
	AM_RANGE(0x000000, 0x1fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x600000, 0x61ffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x740000, 0x747fff) AM_ROM AM_REGION("maincpu", 0)
	// 760000 - optional debug ROM
	AM_RANGE(0x780000, 0x781fff) AM_RAM // map registers
	// 782000-783fff: video address registers
	// 784000-785fff: video control registers
	// 788000-789fff: SN76496 audio
	// 78a000-78bfff: NS32081 FPU
	AM_RANGE(0x78c000, 0x78c007) AM_DEVREADWRITE8("aica", mos6551_device, read, write, 0xff00)
	// 7b1000-7b2fff: diagnostic registers
	// 7b2000-7b3fff: Centronics printer data
	// 7b4000-7b5fff: 68681 DUART
	// 7b6000-7b7fff: Mouse
	// 7b8000-7b9fff: AM9513 timer
	// 7ba000-7bbfff: MC146818 RTC
	// 7bc000-7bdfff: SCSI bus address registers
	// 7be000-7bffff: SCSI (NCR 5385)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fdccpu_map, AS_PROGRAM, 8, tek440x_state )
	AM_RANGE(0x0000, 0x1000) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("fdccpu", 0)
ADDRESS_MAP_END

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( tek4404 )
INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( tek4404, tek440x_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68010, 166666666)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)

	MCFG_CPU_ADD("fdccpu", M6502, 1000000)
	MCFG_CPU_PROGRAM_MAP(fdccpu_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(tek440x_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("aica", MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("aica", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("aica", mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("aica", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("aica", mos6551_device, write_cts))
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tek4404 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tek_u158.bin", 0x000000, 0x004000, CRC(9939e660) SHA1(66b4309e93e4ff20c1295dc2ec2a8d6389b2578c) )
	ROM_LOAD16_BYTE( "tek_u163.bin", 0x000001, 0x004000, CRC(a82dcbb1) SHA1(a7e4545e9ea57619faacc1556fa346b18f870084) )

	ROM_REGION( 0x1000, "fdccpu", 0 )
	ROM_LOAD( "tek_u130.bin", 0x000000, 0x001000, CRC(2c11a3f1) SHA1(b29b3705692d50f15f7e8bbba12a24c69817d52e) )

	ROM_REGION( 0x2000, "scsimfm", 0 )
	ROM_LOAD( "scsi_mfm.bin", 0x000000, 0x002000, CRC(b4293435) SHA1(5e2b96c19c4f5c63a5afa2de504d29fe64a4c908) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
/*    YEAR  NAME      PARENT    COMPAT   MACHINE   INPUT   DEVICE       INIT    COMPANY    FULLNAME  FLAGS*/
COMP( 1984, tek4404,    0,      0,       tek4404, tek4404, driver_device,  0,   "Tektronix", "4404", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
