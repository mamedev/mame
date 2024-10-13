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

   How to enter statistics mode:
    - press Test button, "CODE" text will appear
    - repeat 4x times
      - press Test button (code_digit - 1) times
      - press and hold Test button until "INPUT" text appears
      - release Test button
    Currently we use 0000 hardware code, in such case game will accept any code sequence.
   Now you may press "Clear stats" key few times to clear all the non-volatile play statistics.
   Or press "Clear stats" once, then hold for a several seconds, and then press again - game will clear NVRAM and also raise few flags, effect is not known.

*/

#include "emu.h"
#include "specialsound.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

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
		, m_lamps(*this, "lamp%u", 0U)
		, m_sram_en(false)
		, m_cold_boot(0)
	{ }

	void dice(machine_config &config);

	ioport_value boot_r() { return m_cold_boot; }
	DECLARE_INPUT_CHANGED_MEMBER(ram_test) { m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	void machine_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_boot_flag);

	void palette_init(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void dice_mem(address_map &map) ATTR_COLD;
	void dice_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<i8255_device> m_ppi2;
	required_device<pit8253_device> m_pit;
	required_device<palette_device> m_palette;
	required_device<nvram_device> m_nvram;
	required_shared_ptr<u8> m_vram;
	required_device<hopper_device> m_hopper;
	output_finder<8> m_lamps;

	void lamps_w(u8 data);
	u8 ppi2a_r();
	void ppi2c_w(u8 data);
	u8 code_r(offs_t offset);

	std::unique_ptr<u8[]> m_sram;
	bool m_sram_en;
	int m_cold_boot;
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
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(dinaris_state, boot_r)

	PORT_START("P1")
	PORT_DIPNAME(0x01, 0x00, DEF_STR(Language))
	PORT_DIPSETTING(0x00, DEF_STR(English))
	PORT_DIPSETTING(0x01, "Russian")
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Clear stats")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_NAME("Reset")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("NMI")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE3) PORT_NAME("RAM Test") PORT_CHANGED_MEMBER(DEVICE_SELF, dinaris_state, ram_test, 0)
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(dinaris_state::update_boot_flag)
{
	m_cold_boot = 1;
}

void dinaris_state::machine_start()
{
	m_lamps.resolve();

	constexpr int size = 0x800; // actually used only 256 bytes
	m_sram = std::make_unique<u8[]>(size);
	m_nvram->set_base(&m_sram[0], size);

	save_pointer(NAME(m_sram), size);
	save_item(NAME(m_sram_en));
	save_item(NAME(m_cold_boot));

	emu_timer *cold_boot_timer = timer_alloc(FUNC(dinaris_state::update_boot_flag), this);
	cold_boot_timer->adjust(attotime::from_msec(100));
}

void dinaris_state::dice_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram(); // SRAM
	map(0x9000, 0xffff).ram().share("vram"); // DRAM
}

void dinaris_state::dice_io(address_map &map)
{
	map.global_mask(0x000f);
	map(0x00, 0x03).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0c, 0x0d).r(FUNC(dinaris_state::code_r)).mirror(0x02);
}

// borrowed from Specialist MX, probably wrong
static constexpr rgb_t specimx_pens[16] = {
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0x00, 0x00, 0xaa }, // 1
	{ 0x00, 0xaa, 0x00 }, // 2
	{ 0x00, 0xaa, 0xaa }, // 3
	{ 0xaa, 0x00, 0x00 }, // 4
	{ 0xaa, 0x00, 0xaa }, // 5
	{ 0xaa, 0xaa, 0x00 }, // 6
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

void dinaris_state::palette_init(palette_device &palette) const
{
	palette.set_pen_colors(0, specimx_pens);
}

u32 dinaris_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// note: attribute area bytes set colors for 4x2 pix screen elements, not 8x1 like Specialist MX.
	for (int x = 0; x < 48; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			u8 const code = m_vram[0x0000 + y + x * 256];
			u8 const color1 = m_vram[0x4000 + (y & 0xfe) + x * 256];
			u8 const color2 = m_vram[0x4000 + (y | 0x01) + x * 256];
			for (int b = 7; b >= 4; b--)
				bitmap.pix(y, x * 8 + (7 - b)) = BIT(code, b) ? (color1 & 0xf) : (color1 >> 4);
			for (int b = 3; b >= 0; b--)
				bitmap.pix(y, x * 8 + (7 - b)) = BIT(code, b) ? (color2 & 0xf) : (color2 >> 4);
		}
	}
	return 0;
}

void dinaris_state::lamps_w(u8 data)
{
	// TODO identify each lamp
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
}

u8 dinaris_state::ppi2a_r()
{
	return m_sram_en ? m_sram[m_ppi2->pb_r()] : 0xff;
}

void dinaris_state::ppi2c_w(u8 data)
{
	if (!BIT(data, 2))
		m_sram_en = true;

	if (!BIT(data, 1))
		m_sram_en = false;

	if (!BIT(data, 0) && m_sram_en)
		m_sram[m_ppi2->pb_r()] = m_ppi2->pa_r();

	m_hopper->motor_w(((data & 0x68) == 0x60) ? 1 : 0);
}

u8 dinaris_state::code_r(offs_t offset)
{
	// it is not clear how statistics mode code was set, related components was removed from PCB
	// currently we return 0 so game will accept any code sequence
	return 0x0000 >> (offset * 8);
}

void dinaris_state::dice(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000) / 4); // not confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &dinaris_state::dice_mem);
	m_maincpu->set_addrmap(AS_IO, &dinaris_state::dice_io);
	m_maincpu->set_vblank_int("screen", FUNC(dinaris_state::irq0_line_hold));

	// Video
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(8'000'000), 512, 0, 384, 312, 0, 256);
	screen.set_screen_update(FUNC(dinaris_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(dinaris_state::palette_init), 16);

	// Sound
	SPEAKER(config, "speaker").front_center();
	SPECIMX_SND(config, "custom", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(XTAL(8'000'000) / 4);
	m_pit->out_handler<0>().set("custom", FUNC(specimx_sound_device::set_input_ch0));
	m_pit->set_clk<1>(XTAL(8'000'000) / 4);
	m_pit->out_handler<1>().set("custom", FUNC(specimx_sound_device::set_input_ch1));
	m_pit->set_clk<2>(XTAL(8'000'000) / 4);
	m_pit->out_handler<2>().set("custom", FUNC(specimx_sound_device::set_input_ch2));

	// Devices
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("P0");
	m_ppi->in_pb_callback().set_ioport("P1");
	m_ppi->out_pc_callback().set(FUNC(dinaris_state::lamps_w));

	I8255(config, m_ppi2);
	m_ppi2->in_pa_callback().set(FUNC(dinaris_state::ppi2a_r));
	m_ppi2->out_pc_callback().set(FUNC(dinaris_state::ppi2c_w));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100));
}

ROM_START(dindice)
	ROM_REGION( 0x8000, "maincpu", 0)
	ROM_LOAD( "27256.bin", 0x0000, 0x8000, CRC(511f8ba8) SHA1(e75a2cab80ac6b08a19d1adb8ba9bb321aa5e7a8))
ROM_END

} // anonymous namespace

GAME( 199?, dindice, 0,    dice,     dice,     dinaris_state, empty_init, ROT0,  "Dinaris",   "Dice game", MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_COLORS)
