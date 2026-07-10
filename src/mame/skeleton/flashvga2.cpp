// license:BSD-3-Clause
// copyright-holders: Angelo Salese

/**************************************************************************************************

"Flash VGA 2" hardware (from TourVisión?) for video-slots.

TODO:
- Currently fails on timekeeper check ("error en reloj"), does extensive checks in the empty
  dump area;

===================================================================================================

                               _________
     _________________________|  VGA   |_________________________________
    |           ···CN6···     |________|        _______________         |
    |    ____      _____    ______   ______    |K6R4016C10-JC10         |
    |  TDA2030A   SN7407   74HC5740 74HC5740   |______________|         |
 ___|                                                              ···  |
|___         _____    ______                     _________         ···  |
|___       TLC7524C  |LM358|                    |LATTICE |         CN3  |
|___                                            |ISPLSI  |         ···  |
|___       _________     Xtal                   |1032EA  |         ···  |
|___     SCC2692AC1A44  3.6864 MHz              |C424AH03|              |
|___      |Philips |                                _____________       |
|___      |        |           _____   _________   |Intel       | Xtal  |
|___      |________|         74HC04D  |LATTICE |   |N80C186XL25 |50 MHz |
|___   __________                     |ISPLSI  |   |            |       |
|___  |ULN2803A |  ______    ______   |2032A   |   |            |       |
|___   __________ 74HC273D  74HC273D  |80LJ44__|   |____________|       |
|___  |ULN2803A |  ______    ______  ______   ______   ______   ______  |
|___              74HC273D  |HC138| 74HC245D  HC573A  74HC245D  HC573A  |
|___               ______     ____     ::::::::::::CN1?:::::::::::::    |
|___              74HC245D   |NE555          ______         ______      |
|___               ______    ______         |FLASH|        |FLASH|      |
    |             74HC245D  ADM691AARW      M29F032D       M29F032D     |
    |                                       |_____|        |_____|      |
    |  __________          ________   ___    __________     __________  |
    | OMRON G5V-2         |_ST232C|  24256B |CY62256LL|    |CY62256LL|  |
    |                                        ______________             |
    |    ···CN2···                   CR2032 |M48T18-150PC1|   Switch    |
    |               _________         BATT  |_____________|    (o)      |
    |______________|  DE-9  |___________________________________________|
                   |________|

    CN1?: 15x2-pin male connector (black socket)
    CN2: 10-pin male connector
    CN3: 5x2-pin male connector (blue socket)
    CN4: VGA connector
    CN5: DE-9 UART connector
    CN6: 6-pin male connector
    no dips on (this) board

**************************************************************************************************/

#include "emu.h"

#include "cpu/i86/i186.h"
#include "machine/i2cmem.h"
#include "machine/mc68681.h"
#include "machine/timekpr.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"


namespace {

class flashvga2_state : public driver_device
{
public:
	flashvga2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_program_bank(*this, "program_bank")
		, m_vram(*this, "vram")
		, m_eeprom(*this, "eeprom")
	{ }

	void flashvga2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_memory_bank m_program_bank;
	required_shared_ptr<u16> m_vram;
	required_device<i2c_24c256_device> m_eeprom;

	void mem_map(address_map &map);
	void io_map(address_map &map);
};

uint32_t flashvga2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 *vram = (u8 *)(m_vram.target());

	// palette is at top of VRAM, vectors provided by $be800 bank=7 ROM reads
	// and translated from RAMDAC format to RGB565
	// TODO: cache in write handler, is the resulting garbage displayable or VRAM offsets?
	for (int i = 0; i < 0x100; i ++)
	{
		const u16 datax = m_vram[i];
		const u8 r = (datax >> 11) & 0x1f;
		const u8 g = (datax >> 5) & 0x3f;
		const u8 b = (datax >> 0) & 0x1f;

		m_palette->set_pen_color(i, pal5bit(r), pal6bit(g), pal5bit(b));
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 base_address = y * 640;
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const u8 pen = vram[base_address + x];
			bitmap.pix(y, x) = m_palette->pen(pen);
		}
	}

	return 0;
}

void flashvga2_state::mem_map(address_map &map)
{
	map(0x00000, 0x30fff).ram();
	// TODO: check limits and parallelism (access in 8-bit only?)
	map(0x30000, 0x31fff).rw("rtc", FUNC(m48t02_device::read), FUNC(m48t02_device::write)).umask16(0x00ff);
	map(0x32000, 0x3ffff).ram();
	map(0x40000, 0x7ffff).ram().share("vram");
	// TODO: several zero writes in this area (flash ROM side-effect?)
	map(0x80000, 0xbffff).bankr("program_bank").lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
		if (data)
			LOG("Warning: write at [%05x] %04x & %04x\n", offset * 2 + 0x80000, data, mem_mask);
	}));
	map(0xc0000, 0xfffff).rom().region("program_rom", 0x7c0000);
}

void flashvga2_state::io_map(address_map &map)
{
	map(0x2000, 0x201f).rw("uart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x2176, 0x2177).nopw();
	map(0x2178, 0x2179).portr("IN0").nopw();
	map(0x217a, 0x217b).portr("IN1").nopw();
	map(0x2180, 0x2180).lw8(NAME([this] (offs_t offset, u8 data) {
		// assumed, currently selects pages 7 and 8 only
		m_program_bank->set_entry(data & 0x1f);
		if (data & 0xe0)
			LOG("$2180: write %02x\n", data);
	}));
	map(0x2184, 0x2184).lrw8(
		NAME([this] (offs_t offset) {
			return m_eeprom->read_sda();
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_eeprom->write_scl(BIT(data, 1));
			m_eeprom->write_sda(BIT(data, 0));
		})
	);
}

static INPUT_PORTS_START(ruletamag)
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" ) // asks to "turn off the machine" after "error en reloj"
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) // likely unused

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) // likely unused

INPUT_PORTS_END

void flashvga2_state::machine_start()
{
	m_program_bank->configure_entries(0, 0x20, memregion("program_rom")->base(), 0x40000);
}

void flashvga2_state::machine_reset()
{
	m_program_bank->set_entry(0);
}

void flashvga2_state::flashvga2(machine_config &config)
{
	I80186(config, m_maincpu, 50_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &flashvga2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &flashvga2_state::io_map);

	scn2681_device &uart(SCN2681(config, "uart", 3.6864_MHz_XTAL)); // Philips SCC2692AC1A44
	uart.irq_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);

	M48T02(config, "rtc"); // ST M48T18-150PC1

	I2C_24C256(config, m_eeprom);

	// TODO: refine, is it really 60 Hz?
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(flashvga2_state::screen_update));
	screen.set_size(911, 525);
	screen.set_visarea(0, 640-1, 0, 400-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_palette("palette");
	screen.screen_vblank().set("uart", FUNC(scn2681_device::ip0_w)).invert();

	PALETTE(config, "palette").set_entries(256);

	// Sound hardware
	SPEAKER(config, "mono").front_center();
}

/* Ruleta Mágica from Codere. Same hardware as Ruleta Mágica Mini, the only difference is the cabinet.
   VGA screen, plus a big 7 digits 7-segments (plus dot) display.
   The manual can be downloaded from: https://www.recreativas.org/manuales/tragaperras
   Video of the actual machine booting: https://youtu.be/xUARqw1_N_A  */
ROM_START( ruletamag )
	ROM_REGION16_LE( 0x800000, "program_rom", 0 )
	ROM_LOAD16_BYTE( "m29f032d.u100", 0x000000, 0x400000, CRC(04bf20c2) SHA1(fc4be2c22dc266d6a460aeca257b449be5ab630f) )
	ROM_LOAD16_BYTE( "m29f032d.u101", 0x000001, 0x400000, CRC(2bd85284) SHA1(36f4b918d1d9b57bf382fa940180b3a1aac9780f) )

	ROM_REGION( 0x8000, "eeprom", 0 )
	ROM_LOAD( "m24256bf.u31",  0x000000, 0x008000, CRC(af9adcae) SHA1(ac6274edc4240d5cf397455868009263264ffc6e) )

	/* With an unintialized NVRAM/timekeeper, the machine won't work and Will output just a "ERROR EN RELOJ" message.
	       With the included dump (corrupted), it will output the message "ERROR EN MODULO", but still won't boot.
	       Maybe there's a way to initialize the NVRAM, but there's nothing about it on the manual. */
	ROM_REGION( 0x2000, "rtc", 0 )
	ROM_LOAD( "m48t18.u38",    0x000000, 0x002000, BAD_DUMP CRC(025fb8c2) SHA1(61c90ecad8565cfd20674034a5917b0225edbfe5) ) // Corrupted
	// patch checks at PC=d3299 onward
	ROM_FILL( 0x0044 >> 1, 1, 0x55 )
	ROM_FILL( 0x0046 >> 1, 1, 0xaa )
	// checksum for 0x48 ~ 0x5c
	ROM_FILL( 0x005e >> 1, 1, 0xf4 )
	ROM_FILL( 0x0060 >> 1, 1, 0x0b )

	// does further checks if this skipped (going back to "error en reloj")
//	ROM_FILL( 0x00b0 >> 1, 1, 0x55 )
//	ROM_FILL( 0x00b2 >> 1, 1, 0xaa )

//	ROM_FILL( 0x0008 >> 1, 1, 0x71 )
//	ROM_FILL( 0x000a >> 1, 1, 0xac )
ROM_END


} // Anonymous namespace

//    YEAR  NAME       PARENT  MACHINE    INPUT      CLASS            INIT        ROT   COMPANY   FULLNAME           FLAGS
GAME( 2005, ruletamag, 0,      flashvga2, ruletamag, flashvga2_state, empty_init, ROT0, "Codere", u8"Ruleta Mágica", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // "Mini Ruleta M. SP v.-1.00 Flash/Vga" as sticker on 186
