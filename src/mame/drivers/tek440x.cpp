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
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "machine/am9513.h"
#include "machine/mos6551.h"    // debug tty
#include "machine/mc146818.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define VIDEO_CLOCK XTAL(25'200'000)

class tek440x_state : public driver_device
{
public:
	tek440x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fdccpu(*this, "fdccpu"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram")
	{ }

	void tek4404(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<m68010_device> m_maincpu;
	required_device<m6502_device> m_fdccpu;
	required_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_vram;
	void fdccpu_map(address_map &map);
	void maincpu_map(address_map &map);
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
	uint8_t *ROM = memregion("maincpu")->base();
	uint8_t *RAM = (uint8_t *)m_mainram.target();

	memcpy(RAM, ROM, 256);

	m_maincpu->reset();
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

uint32_t tek440x_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint16_t *video_ram;
	uint16_t word;
	uint16_t *line;
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

void tek440x_state::maincpu_map(address_map &map)
{
	map(0x000000, 0x1fffff).ram().share("mainram");
	map(0x600000, 0x61ffff).ram().share("vram");
	map(0x740000, 0x747fff).rom().region("maincpu", 0);
	// 760000 - optional debug ROM
	map(0x780000, 0x781fff).ram(); // map registers
	// 782000-783fff: video address registers
	// 784000-785fff: video control registers
	map(0x788000, 0x788000).w("snsnd", FUNC(sn76496_device::write));
	// 78a000-78bfff: NS32081 FPU
	map(0x78c000, 0x78c007).rw("aica", FUNC(mos6551_device::read), FUNC(mos6551_device::write)).umask16(0xff00);
	// 7b1000-7b2fff: diagnostic registers
	// 7b2000-7b3fff: Centronics printer data
	// 7b4000-7b5fff: 68681 DUART
	// 7b6000-7b7fff: Mouse
	map(0x7b8000, 0x7b8003).mirror(0x100).rw("timer", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	// 7ba000-7bbfff: MC146818 RTC
	// 7bc000-7bdfff: SCSI bus address registers
	// 7be000-7bffff: SCSI (NCR 5385)
}

void tek440x_state::fdccpu_map(address_map &map)
{
	map(0x0000, 0x1000).ram();
	map(0xf000, 0xffff).rom().region("fdccpu", 0);
}

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

void tek440x_state::tek4404(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, 40_MHz_XTAL / 4); // MC68010L10
	m_maincpu->set_addrmap(AS_PROGRAM, &tek440x_state::maincpu_map);

	M6502(config, m_fdccpu, 1000000);
	m_fdccpu->set_addrmap(AS_PROGRAM, &tek440x_state::fdccpu_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update(FUNC(tek440x_state::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);

	mos6551_device &aica(MOS6551(config, "aica", 0));
	aica.set_xtal(1.8432_MHz_XTAL);
	aica.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	AM9513(config, "timer", 40_MHz_XTAL / 4 / 10); // from CPU E output

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("aica", FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set("aica", FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set("aica", FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set("aica", FUNC(mos6551_device::write_cts));

	SPEAKER(config, "mono").front_center();

	SN76496(config, "snsnd", VIDEO_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.80);
}



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
//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME                               FLAGS
COMP( 1984, tek4404, 0,      0,      tek4404, tek4404, tek440x_state, empty_init, "Tektronix", "4404 Artificial Intelligence System", MACHINE_NOT_WORKING )
