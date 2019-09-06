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
#include "cpu/mcs48/mcs48.h"
#include "machine/am9513.h"
#include "machine/bankdev.h"
#include "machine/mos6551.h"    // debug tty
#include "machine/mc146818.h"
#include "machine/mc68681.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class tek440x_state : public driver_device
{
public:
	tek440x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fdccpu(*this, "fdccpu"),
		m_vm(*this, "vm"),
		m_snsnd(*this, "snsnd"),
		m_prom(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram")
	{ }

	void tek4404(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 memory_r(offs_t offset, u16 mem_mask);
	void memory_w(offs_t offset, u16 data, u16 mem_mask);
	void sound_w(u8 data);

	void logical_map(address_map &map);
	void physical_map(address_map &map);
	void fdccpu_map(address_map &map);

	required_device<m68010_device> m_maincpu;
	required_device<m6502_device> m_fdccpu;
	required_device<address_map_bank_device> m_vm;
	required_device<sn76496_device> m_snsnd;
	required_region_ptr<u16> m_prom;
	required_shared_ptr<u16> m_mainram;
	required_shared_ptr<u16> m_vram;

	bool m_boot;
};

/*************************************
 *
 *  Machine start
 *
 *************************************/

void tek440x_state::machine_start()
{
	save_item(NAME(m_boot));
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void tek440x_state::machine_reset()
{
	m_boot = true;
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

u32 tek440x_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 *video_ram;
	u16 word;
	u16 *line;
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

u16 tek440x_state::memory_r(offs_t offset, u16 mem_mask)
{
	if (m_boot)
		return m_prom[offset & 0x3fff];

	// TODO: banking
	return m_vm->read16(offset, mem_mask);
}

void tek440x_state::memory_w(offs_t offset, u16 data, u16 mem_mask)
{
	// TODO: banking
	m_vm->write16(offset, data, mem_mask);
}

void tek440x_state::sound_w(u8 data)
{
	m_snsnd->write(data);
	m_boot = false;
}

void tek440x_state::logical_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(tek440x_state::memory_r), FUNC(tek440x_state::memory_w));
}

void tek440x_state::physical_map(address_map &map)
{
	map(0x000000, 0x1fffff).ram().share("mainram");
	map(0x600000, 0x61ffff).ram().share("vram");
	map(0x740000, 0x747fff).rom().mirror(0x8000).region("maincpu", 0);
	map(0x760000, 0x760fff).ram().mirror(0xf000); // debug RAM
	map(0x780000, 0x781fff).ram(); // map registers
	// 782000-783fff: video address registers
	// 784000-785fff: video control registers
	map(0x788000, 0x788000).w(FUNC(tek440x_state::sound_w));
	// 78a000-78bfff: NS32081 FPU
	map(0x78c000, 0x78c007).rw("aica", FUNC(mos6551_device::read), FUNC(mos6551_device::write)).umask16(0xff00);
	// 7b1000-7b2fff: diagnostic registers
	// 7b2000-7b3fff: Centronics printer data
	map(0x7b4000, 0x7b401f).rw("duart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0xff00);
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
	m_maincpu->set_addrmap(AS_PROGRAM, &tek440x_state::logical_map);

	ADDRESS_MAP_BANK(config, m_vm);
	m_vm->set_addrmap(0, &tek440x_state::physical_map);
	m_vm->set_data_width(16);
	m_vm->set_addr_width(23);
	m_vm->set_endianness(ENDIANNESS_BIG);

	M6502(config, m_fdccpu, 16_MHz_XTAL / 8);
	m_fdccpu->set_addrmap(AS_PROGRAM, &tek440x_state::fdccpu_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_raw(25.2_MHz_XTAL, 800, 0, 640, 525, 0, 480); // 31.5 kHz horizontal (guessed), 60 Hz vertical
	screen.set_screen_update(FUNC(tek440x_state::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);

	mos6551_device &aica(MOS6551(config, "aica", 40_MHz_XTAL / 4 / 10));
	aica.set_xtal(1.8432_MHz_XTAL);
	aica.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	aica.irq_handler().set_inputline(m_maincpu, M68K_IRQ_7);

	mc68681_device &duart(MC68681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set_inputline(m_maincpu, M68K_IRQ_5);

	I8048(config, "kbdmcu", 4.608_MHz_XTAL).set_disable();

	AM9513(config, "timer", 40_MHz_XTAL / 4 / 10); // from CPU E output

	//MC146818(config, "calendar", 32.768_MHz_XTAL);
	//NCR5385(config, "scsic", 40_MHz_XTAL / 4);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("aica", FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set("aica", FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set("aica", FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set("aica", FUNC(mos6551_device::write_cts));

	SPEAKER(config, "mono").front_center();

	SN76496(config, m_snsnd, 25.2_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.80);
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

	ROM_REGION( 0x400, "kbdmcu", 0 )
	ROM_LOAD( "keytronic_8x48.bin", 0x000, 0x400, NO_DUMP )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME                               FLAGS
COMP( 1984, tek4404, 0,      0,      tek4404, tek4404, tek440x_state, empty_init, "Tektronix", "4404 Artificial Intelligence System", MACHINE_NOT_WORKING )
