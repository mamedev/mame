// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*

Othello (version 3.0) - Success 1984
-------------------------------------

driver by Tomasz Slanina

CPU Board:
 D780C          - main CPU (Z80)
 HD46505SP      - CRTC
 D780-C         - Sound CPU (Z80)
 AY-3-8910 x2   - Sound
 D7751C         - ADPCM "Speech processor"
 D8243          - I/O Expander for D7751C (8048 based)

Video Board:
 almost empty - 3/4 soldering pins not populated


Todo:
- correct colors (based on the color DAC (24 resistors) on pcb
- cocktail mode
- map a bunch of unknown read/writes (related to above I think)

Notes:

DSw 1:2
Limit for help/undo (matta):
- when it's off, you can use each of them twice every time you
  win and advance to the next game
- when it's on, you can only use them twice throughout the game

*/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8243.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

constexpr uint8_t TILE_WIDTH = 6;


class othello_state : public driver_device
{
public:
	othello_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_ay(*this, "ay%u", 0U),
		m_upd7751(*this, "upd7751"),
		m_i8243(*this, "upd7751_8243"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_gfx_data(*this, "gfx"),
		m_upd7751_data(*this, "upd7751data")
	{ }

	void othello(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	int m_tile_bank = 0;

	/* misc */
	int m_ay_select = 0;
	int m_ack_data = 0;
	uint8_t m_upd7751_command = 0;
	int m_sound_addr = 0;
	int m_upd7751_busy = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device_array<ay8910_device, 2> m_ay;
	required_device<upd7751_device> m_upd7751;
	required_device<i8243_device> m_i8243;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_region_ptr<uint8_t> m_gfx_data;
	required_region_ptr<uint8_t> m_upd7751_data;

	uint8_t unk_87_r();
	void unk_8a_w(uint8_t data);
	void upd7751_command_w(uint8_t data);
	uint8_t upd7751_busy_r();
	uint8_t sound_ack_r();
	void unk_8f_w(uint8_t data);
	void tilebank_w(uint8_t data);
	uint8_t latch_r();
	void ay_select_w(uint8_t data);
	void ack_w(uint8_t data);
	void ay_address_w(uint8_t data);
	void ay_data_w(uint8_t data);
	uint8_t upd7751_rom_r();
	uint8_t upd7751_command_r();
	void upd7751_p2_w(uint8_t data);
	template<int Shift> void upd7751_rom_addr_w(uint8_t data);
	void upd7751_rom_select_w(uint8_t data);

	void othello_palette(palette_device &palette) const;
	MC6845_UPDATE_ROW(crtc_update_row);

	void audio_map(address_map &map) ATTR_COLD;
	void audio_portmap(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
};


MC6845_UPDATE_ROW( othello_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t const *const gfx = m_gfx_data;

	for(int cx = 0; cx < x_count; ++cx)
	{
		uint32_t address = ((m_videoram[ma + cx] + m_tile_bank) << 4) | ra;
		uint32_t tile_data = gfx[address] | (gfx[address + 0x2000] << 8) | (gfx[address + 0x4000] << 16);

		for(int x = 0; x < TILE_WIDTH; ++x)
		{
			bitmap.pix(y, (cx * TILE_WIDTH + x) ^ 1) = palette[tile_data & 0x0f];
			tile_data >>= 4;
		}
	}
}

void othello_state::othello_palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
		palette.set_pen_color(i, rgb_t(0xff, 0x00, 0xff));

	// only colors  2,3,7,9,c,d,f are used
	palette.set_pen_color(0x02, rgb_t(0x00, 0xff, 0x00));
	palette.set_pen_color(0x03, rgb_t(0xff, 0x7f, 0x00));
	palette.set_pen_color(0x07, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(0x09, rgb_t(0xff, 0x00, 0x00));
	palette.set_pen_color(0x0c, rgb_t(0x00, 0x00, 0xff));
	palette.set_pen_color(0x0d, rgb_t(0x7f, 0x7f, 0x00));
	palette.set_pen_color(0x0f, rgb_t(0xff, 0xff, 0xff));
}

void othello_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x97ff).noprw(); /* not populated */
	map(0x9800, 0x9fff).ram().share(m_videoram);
	map(0xf000, 0xffff).ram();
}

uint8_t othello_state::unk_87_r()
{
	// bit 7 = ack/status from device connected to port 8a?
	return 0;
}

void othello_state::unk_8a_w(uint8_t data)
{
	logerror("8a -> %x\n", data);
}

void othello_state::upd7751_command_w(uint8_t data)
{
	m_upd7751->set_input_line(0, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
	m_upd7751_command = data;
}

uint8_t othello_state::upd7751_busy_r()
{
	return m_upd7751_busy;
}

uint8_t othello_state::sound_ack_r()
{
	return m_ack_data;
}

void othello_state::unk_8f_w(uint8_t data)
{
	logerror("8f -> %x\n", data);
}

void othello_state::tilebank_w(uint8_t data)
{
	m_tile_bank = (data == 0x0f) ? 0x100 : 0x00;
	logerror("tilebank -> %x\n", data);
}

void othello_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x08, 0x08).w("crtc", FUNC(mc6845_device::address_w));
	map(0x09, 0x09).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x80, 0x80).portr("INP");
	map(0x81, 0x81).portr("SYSTEM");
	map(0x83, 0x83).portr("DSW");
	map(0x86, 0x86).w(FUNC(othello_state::tilebank_w));
	map(0x87, 0x87).r(FUNC(othello_state::unk_87_r));
	map(0x8a, 0x8a).w(FUNC(othello_state::unk_8a_w));
	map(0x8c, 0x8c).rw(FUNC(othello_state::upd7751_busy_r), FUNC(othello_state::upd7751_command_w));
	map(0x8d, 0x8d).r(FUNC(othello_state::sound_ack_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x8f, 0x8f).w(FUNC(othello_state::unk_8f_w));
}

uint8_t othello_state::latch_r()
{
	uint8_t retval = m_soundlatch->read();
	if (!machine().side_effects_disabled())
		m_soundlatch->clear_w();

	return retval;
}

void othello_state::ay_select_w(uint8_t data)
{
	m_ay_select = data;
}

void othello_state::ack_w(uint8_t data)
{
	m_ack_data = data;
}

void othello_state::ay_address_w(uint8_t data)
{
	if (m_ay_select & 1) m_ay[0]->address_w(data);
	if (m_ay_select & 2) m_ay[1]->address_w(data);
}

void othello_state::ay_data_w(uint8_t data)
{
	if (m_ay_select & 1) m_ay[0]->data_w(data);
	if (m_ay_select & 2) m_ay[1]->data_w(data);
}

void othello_state::audio_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x83ff).ram();
}

void othello_state::audio_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(othello_state::latch_r));
	map(0x01, 0x01).w(FUNC(othello_state::ay_data_w));
	map(0x03, 0x03).w(FUNC(othello_state::ay_address_w));
	map(0x04, 0x04).w(FUNC(othello_state::ack_w));
	map(0x08, 0x08).w(FUNC(othello_state::ay_select_w));
}

template<int Shift>
void othello_state::upd7751_rom_addr_w(uint8_t data)
{
	// P4 - address lines 0-3
	// P5 - address lines 4-7
	// P6 - address lines 8-11
	m_sound_addr = (m_sound_addr & ~(0x00f << Shift)) | ((data & 0x0f) << Shift);
}

void othello_state::upd7751_rom_select_w(uint8_t data)
{
	// P7 - ROM selects
	m_sound_addr &= 0xfff;

	if (!BIT(data, 0)) m_sound_addr |= 0x0000;
	if (!BIT(data, 1)) m_sound_addr |= 0x1000;
	if (!BIT(data, 2)) m_sound_addr |= 0x2000;
	if (!BIT(data, 3)) m_sound_addr |= 0x3000;
}

uint8_t othello_state::upd7751_rom_r()
{
	return m_upd7751_data[m_sound_addr];
}

uint8_t othello_state::upd7751_command_r()
{
	return m_upd7751_command << 4 | 0x0f;
}

void othello_state::upd7751_p2_w(uint8_t data)
{
	/* write to P2; low 4 bits go to 8243 */
	m_i8243->p2_w(data & 0x0f);

	/* output of bit $80 indicates we are ready (1) or busy (0) */
	/* no other outputs are used */
	m_upd7751_busy = data & 0x80;
}

static INPUT_PORTS_START( othello )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, "Limit for Matta" )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )     PORT_DIPLOCATION("SW1:5") /* stored at $fd1e */
	PORT_DIPNAME( 0x60, 0x60, "Timer (seconds)" )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x40, "8" )
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )

	PORT_START("INP")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )  PORT_PLAYER(2)

	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

void othello_state::machine_start()
{
	save_item(NAME(m_tile_bank));
	save_item(NAME(m_ay_select));
	save_item(NAME(m_ack_data));
	save_item(NAME(m_upd7751_command));
	save_item(NAME(m_sound_addr));
	save_item(NAME(m_upd7751_busy));
}

void othello_state::machine_reset()
{
	m_tile_bank = 0;
	m_ay_select = 0;
	m_ack_data = 0;
	m_upd7751_command = 0;
	m_sound_addr = 0;
	m_upd7751_busy = 0;
}

void othello_state::othello(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &othello_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &othello_state::main_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(othello_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(3'579'545)));
	audiocpu.set_addrmap(AS_PROGRAM, &othello_state::audio_map);
	audiocpu.set_addrmap(AS_IO, &othello_state::audio_portmap);

	UPD7751(config, m_upd7751, XTAL(6'000'000));
	m_upd7751->t1_in_cb().set_constant(0); // labelled as "TEST", connected to ground
	m_upd7751->p2_in_cb().set(FUNC(othello_state::upd7751_command_r));
	m_upd7751->bus_in_cb().set(FUNC(othello_state::upd7751_rom_r));
	m_upd7751->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_upd7751->p2_out_cb().set(FUNC(othello_state::upd7751_p2_w));
	m_upd7751->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));

	config.set_perfect_quantum(m_maincpu);

	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(othello_state::upd7751_rom_addr_w<0>));
	m_i8243->p5_out_cb().set(FUNC(othello_state::upd7751_rom_addr_w<4>));
	m_i8243->p6_out_cb().set(FUNC(othello_state::upd7751_rom_addr_w<8>));
	m_i8243->p7_out_cb().set(FUNC(othello_state::upd7751_rom_select_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*6, 64*8);
	screen.set_visarea(0*8, 64*6-1, 0*8, 64*8-1);
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette, FUNC(othello_state::othello_palette), 0x10);

	hd6845s_device &crtc(HD6845S(config, "crtc", 1000000 /* ? MHz */));   /* HD46505SP @ CPU clock */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(TILE_WIDTH);
	crtc.set_update_row_callback(FUNC(othello_state::crtc_update_row));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay[0], 2000000).add_route(ALL_OUTPUTS, "speaker", 0.25);
	AY8910(config, m_ay[1], 2000000).add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_8BIT_R2R(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}

ROM_START( othello )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4.ic59",   0x0000, 0x2000, CRC(9f82fe14) SHA1(59600264ccce787383827fc5aa0f2c23728f6946))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3.ic32",   0x0000, 0x2000, CRC(2bb4f75d) SHA1(29a659031acf0d50f374f440b8d353bcf98145a0))

	ROM_REGION( 0x1000, "upd7751", 0 ) /* 1k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin", 0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) )

	ROM_REGION( 0x4000, "upd7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "1.ic48",   0x0000, 0x2000, CRC(c3807dea) SHA1(d6339380e1239f3e20bcca2fbc673ad72e9ca608))
	ROM_LOAD( "2.ic49",   0x2000, 0x2000, CRC(a945f3e7) SHA1(ea18efc18fda63ce1747287bbe2a9704b08daff8))

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "5.ic40",   0x0000, 0x2000, CRC(45fdc1ab) SHA1(f30f6002e3f34a647effac8b0116c8ed064e226a))
	ROM_LOAD( "6.ic41",   0x2000, 0x2000, CRC(467a731f) SHA1(af80e854522e53fb1b9af7945b2c803a654c6f65))
	ROM_LOAD( "7.ic42",   0x4000, 0x2000, CRC(a76705f7) SHA1(b7d2a65d65d065732ddd0b3b738749369b382b48))
ROM_END

} // anonymous namespace


GAME( 1984, othello, 0, othello, othello, othello_state, empty_init, ROT0, "Success", "Othello (version 3.0)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
