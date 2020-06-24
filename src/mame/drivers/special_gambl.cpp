// license:BSD-3-Clause
// copyright-holders:MetalliC

/*
  Gambling hardware based on "Specialist MX" computer
  (c) 199? Dinaris

  Main components:
   Z80 CPU,
   27256 ROM,
   2x KR580VV55(i8255 PPI),
   KR580VI53(i8253 PIT),
   8x KR565RU5(4164) DRAM,
   3x KR573RU10(HM6516) SRAM

  Work in progress driver

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/special.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class dinaris_state : public driver_device
{
public:
	dinaris_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi8255")
		, m_ppi2(*this, "ppi82552")
		, m_pit(*this, "pit8253")
		, m_palette(*this, "palette")
		, m_nvram(*this, "nvram")
		, m_vram(*this, "vram")
		, m_hopper(*this, "hopper")
	{ }

	void dice(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(test_mode);

protected:
	void machine_start() override;
	void dice_palette(palette_device &palette) const;
	u32 screen_update_dice(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void dice_mem(address_map &map);
	void dice_io(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<i8255_device> m_ppi2;
	required_device<pit8253_device> m_pit;
	required_device<palette_device> m_palette;
	required_device<nvram_device> m_nvram;
	required_shared_ptr<u8> m_vram;
	required_device<hopper_device> m_hopper;

	std::unique_ptr<u8[]> m_sram;
};


static INPUT_PORTS_START(dice)
	PORT_START("P0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW)    // 6 or less / double less
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP)   // 12/11
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET)    // bet / field
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HALF)   // point/7
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH)   // 8 or more / double more
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL)   // start / double
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT) // payout / take
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON6)       // 0 - clear SRAM at boot

	PORT_START("P1")
	PORT_DIPNAME(0x01, 0x00, DEF_STR(Language))
	PORT_DIPSETTING(0x00, DEF_STR(English))
	PORT_DIPSETTING(0x01, "Russian")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) // CODE ?
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) // ????
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3) // reset
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON5)

	PORT_START("NMI")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE2) PORT_CHANGED_MEMBER(DEVICE_SELF, dinaris_state, test_mode, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(dinaris_state::test_mode)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

void dinaris_state::machine_start()
{
	constexpr int size = 0x800; // actually used only 256 bytes
	m_sram = std::make_unique<u8[]>(size);
	m_nvram->set_base(&m_sram[0], size);
	save_pointer(NAME(m_sram), size);
}

void dinaris_state::dice_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram(); // SRAM
	map(0x9000, 0xffff).ram().share("vram"); // DRAM
}

void dinaris_state::dice_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

// borrowed from Specialist MX, probably wrong and expected to be the cause of visual glitches
static constexpr rgb_t specimx_pens[16] = {
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0x00, 0x00, 0xaa }, // 1
	{ 0x00, 0xaa, 0x00 }, // 2
	{ 0x00, 0xaa, 0xaa }, // 3
	{ 0xaa, 0x00, 0x00 }, // 4
	{ 0xaa, 0x00, 0xaa }, // 5
	{ 0xaa, 0x55, 0x00 }, // 6
	{ 0xaa, 0xaa, 0xaa }, // 7
	{ 0x55, 0x55, 0x55 }, // 8
	{ 0x55, 0x55, 0xff }, // 9
	{ 0x55, 0xff, 0x55 }, // A
	{ 0x55, 0xff, 0xff }, // B
	{ 0xff, 0x55, 0x55 }, // C
	{ 0xff, 0x55, 0xff }, // D
	{ 0xff, 0xff, 0x55 }, // E
	{ 0xff, 0xff, 0xff }  // F
};

void dinaris_state::dice_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, specimx_pens);
}

u32 dinaris_state::screen_update_dice(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x = 0; x < 48; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			u8 const code = m_vram[0x0000 + y + x * 256];
			u8 const color = m_vram[0x4000 + y + x * 256];
			for (int b = 7; b >= 0; b--)
				bitmap.pix16(y, x * 8 + (7 - b)) = BIT(code, b) ? (color & 0xf) : (color >> 4);
		}
	}
	return 0;
}

void dinaris_state::dice(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 2000000); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &dinaris_state::dice_mem);
	m_maincpu->set_addrmap(AS_IO, &dinaris_state::dice_io);
	m_maincpu->set_vblank_int("screen", FUNC(dinaris_state::irq0_line_hold));

	// Video
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(384, 256);
	screen.set_visarea(0, 384 - 1, 0, 256 - 1);
	screen.set_screen_update(FUNC(dinaris_state::screen_update_dice));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(dinaris_state::dice_palette), 16);

	// Sound
	SPEAKER(config, "speaker").front_center();
	SPECIMX_SND(config, "custom", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(2000000); // guess
	m_pit->out_handler<0>().set("custom", FUNC(specimx_sound_device::set_input_ch0));
	m_pit->set_clk<1>(2000000); // guess
	m_pit->out_handler<1>().set("custom", FUNC(specimx_sound_device::set_input_ch1));
	m_pit->set_clk<2>(2000000); // guess
	m_pit->out_handler<2>().set("custom", FUNC(specimx_sound_device::set_input_ch2));

	// Devices
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("P0");
	m_ppi->in_pb_callback().set_ioport("P1");
	// TODO port C outputs

	I8255(config, m_ppi2);
	m_ppi2->in_pa_callback().set([this] () { return m_sram[m_ppi2->pb_r()]; });
	m_ppi2->out_pc_callback().set([this](u8 data) { if (!BIT(data, 0)) m_sram[m_ppi2->pb_r()] = m_ppi2->pa_r(); });
	// TODO identify other port C bits

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	HOPPER(config, m_hopper, attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW);
}

ROM_START(dindice)
	ROM_REGION( 0x8000, "maincpu", 0)
	ROM_LOAD( "27256.bin", 0x0000, 0x8000, CRC(511f8ba8) SHA1(e75a2cab80ac6b08a19d1adb8ba9bb321aa5e7a8))
ROM_END

GAME( 199?, dindice, 0,    dice,     dice,     dinaris_state, empty_init, ROT0,  "Dinaris",   "Dice game", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_GRAPHICS)