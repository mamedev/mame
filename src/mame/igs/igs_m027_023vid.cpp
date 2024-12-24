// license:BSD-3-Clause
// copyright-holders:

/*
IGS ARM7 (IGS027A) based mahjong / gambling platform(s),
with IGS customs normally used on PGM hardware.
Contrary to proper PGM hardware, these don't have a M68K.

Main components for the PCB-0457-03-GS are:
- IGS 027A (ARM7-based MCU)
- 33 MHz XTAL
- IGS 023 graphics chip
- ICS2115V Wavefront sound chip
- IGS 026B I/O chip
- 3 banks of 8 DIP switches

Notes:
- Press Bookkeeping (0) while the game is running to access the
  bookkeeping and setup menus (default password is Start eight times).
- Press Test (F2) while the game is running to view DIP switch settings.
- Hold Test (F2) while the game boots to access the I/O test.
- Press Test (F2) and Bookkeeping (0) simultaneously at the I/O test to
  access the sound test (Button 1/A to increment, Button 2/B to
  decrement, Button 3/C to toggle looping, Start to play sound).
- Press Test (F2) and Bookkeeping (0) simultaneously at the sound test
  to access the display test (Button 1/A to cycle pattern type,
  Button 2/B to cycle pattern variant, Start to return to I/O test).
*/

#include "emu.h"

#include "igs023_video.h"
#include "igs027a.h"
#include "igsmahjong.h"
#include "pgmcrypt.h"

#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/ics2115.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "endianness.h"

#include <algorithm>


namespace {

class igs_m027_023vid_state : public driver_device
{
public:
	igs_m027_023vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_external_rom(*this, "user1"),
		m_nvram(*this, "nvram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video(*this, "igs023"),
		m_ics(*this, "ics"),
		m_hopper(*this, "hopper"),
		m_io_clearmem(*this, "CLEARMEM"),
		m_io_in(*this, "IN%u", 0U),
		m_io_out(*this, "OUT%u", 0U),
		m_io_dsw(*this, "DSW%u", 1U),
		m_io_kbd(*this, "KEY%u", 0U)
	{ }

	void m027_023vid(machine_config &config) ATTR_COLD;

	void init_mxsqy() ATTR_COLD;

	ioport_value kbd_r();

	template <unsigned N> void counter_w(int state) { machine().bookkeeping().coin_counter_w(N, state); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_region_ptr<u32> m_external_rom;
	required_shared_ptr<u32> m_nvram;

	required_device<igs027a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<igs023_video_device> m_video;
	required_device<ics2115_device> m_ics;
	required_device<hopper_device> m_hopper;

	required_ioport m_io_clearmem;
	required_ioport_array<3> m_io_in;
	required_ioport_array<2> m_io_out;
	required_ioport_array<3> m_io_dsw;
	required_ioport_array<5> m_io_kbd;

	u32 m_xor_table[0x100];
	bool m_irq_source;

	u8 m_gpio_out;
	u8 m_kbd_sel;

	bool m_first_start;

	u32 external_rom_r(offs_t offset);
	void xor_table_w(offs_t offset, u8 data);

	template <unsigned N> u16 in_r() { return m_io_in[N]->read(); }
	template <unsigned N> void out_w(u16 data) { return m_io_out[N]->write(data); }
	u16 dsw_r();
	void kbd_w(u16 data);

	void gpio_w(u8 data);

	u16 sprites_r(offs_t offset);
	void screen_vblank(int state);

	void m027_map(address_map &map) ATTR_COLD;

	template <unsigned N>
	void irq_w(int state);
	u32 gpio_r();
};


void igs_m027_023vid_state::machine_start()
{
	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);
	m_irq_source = 0;
	m_gpio_out = 0x1f;
	m_kbd_sel = 0x1f;
	m_first_start = true;

	save_item(NAME(m_xor_table));
	save_item(NAME(m_irq_source));
	save_item(NAME(m_gpio_out));
	save_item(NAME(m_kbd_sel));
	save_item(NAME(m_first_start));
}

void igs_m027_023vid_state::machine_reset()
{
	if (m_first_start)
	{
		if (!BIT(m_io_clearmem->read(), 0))
			std::fill(std::begin(m_nvram), std::end(m_nvram), 0);
		m_first_start = false;
	}
}


void igs_m027_023vid_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		// first 0xa00 of main ram = sprites, seems to be buffered, DMA?
		m_video->get_sprites();

		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
	}
	else
	{
		// this handles palette transfers, what scanline should it happen on? incorrect frame timing causes some graphics to be missing in service mode
		igs_m027_023vid_state::irq_w<0>(ASSERT_LINE);
	}
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027_023vid_state::m027_map(address_map &map)
{
	map(0x0800'0000, 0x0807'ffff).r(FUNC(igs_m027_023vid_state::external_rom_r)); // Game ROM

	map(0x1800'0000, 0x1800'7fff).ram().share(m_nvram);

	map(0x2800'0000, 0x2800'0fff).ram();

	map(0x3890'0000, 0x3890'7fff).rw(m_video, FUNC(igs023_video_device::videoram_r), FUNC(igs023_video_device::videoram_w)).umask32(0xffffffff);
	map(0x38a0'0000, 0x38a0'11ff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x38b0'0000, 0x38b0'ffff).rw(m_video, FUNC(igs023_video_device::videoregs_r), FUNC(igs023_video_device::videoregs_w)).umask32(0xffffffff);

	map(0x4800'0000, 0x4800'0001).r(FUNC(igs_m027_023vid_state::in_r<0>));
	map(0x4800'0002, 0x4800'0003).r(FUNC(igs_m027_023vid_state::in_r<1>));
	map(0x4800'0004, 0x4800'0005).r(FUNC(igs_m027_023vid_state::in_r<2>));
	map(0x4800'0006, 0x4800'0007).rw(FUNC(igs_m027_023vid_state::dsw_r), FUNC(igs_m027_023vid_state::out_w<0>));

	map(0x5000'0000, 0x5000'03ff).umask32(0x0000'00ff).w(FUNC(igs_m027_023vid_state::xor_table_w)); // uploads XOR table to external ROM here

	map(0x5800'0000, 0x5800'0007).rw(m_ics, FUNC(ics2115_device::read), FUNC(ics2115_device::write)).umask32(0x00ff00ff);

	map(0x6800'0000, 0x6800'0001).w(FUNC(igs_m027_023vid_state::kbd_w));
	map(0x6800'0002, 0x6800'0003).w(FUNC(igs_m027_023vid_state::out_w<1>));
}


/***************************************************************************

    Input Ports

***************************************************************************/

INPUT_PORTS_START( mxsqy )
	IGS_MAHJONG_MATRIX_CONDITIONAL("DSW1", 0x01, 0x00)

	PORT_START("IN0")
	PORT_BIT(0x07ff, IP_ACTIVE_LOW,  IPT_UNKNOWN)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_BIT(0x003f, IP_ACTIVE_HIGH, IPT_CUSTOM)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  PORT_CUSTOM_MEMBER(FUNC(igs_m027_023vid_state::kbd_r))
	PORT_BIT(0x0040, IP_ACTIVE_LOW,  IPT_CUSTOM)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // 哈巴
	PORT_SERVICE_NO_TOGGLE(0x0080, IP_ACTIVE_LOW)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  // 测试
	PORT_BIT(0x0100, IP_ACTIVE_LOW,  IPT_COIN1)                 PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  // 投币
	PORT_BIT(0x0200, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT)         PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  // 洗分
	PORT_BIT(0x0400, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK)           PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  // 査帐
	PORT_BIT(0xf800, IP_ACTIVE_LOW,  IPT_UNKNOWN)

	PORT_START("IN1")
	PORT_BIT(0x0007, IP_ACTIVE_LOW,  IPT_UNKNOWN)
	PORT_BIT(0x0008, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK)           PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 査帐
	PORT_SERVICE_NO_TOGGLE(0x0010, IP_ACTIVE_LOW)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 测试
	PORT_BIT(0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_BIT(0x0040, IP_ACTIVE_LOW,  IPT_COIN1)                 PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 投币
	PORT_BIT(0x0078, IP_ACTIVE_LOW,  IPT_UNKNOWN)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_BIT(0x0080, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN)                                                      // 开分
	PORT_BIT(0x0100, IP_ACTIVE_LOW,  IPT_START1)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 开始
	PORT_BIT(0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP)           PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 上
	PORT_BIT(0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN)         PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 下
	PORT_BIT(0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT)         PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 左
	PORT_BIT(0x1000, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT)        PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 右
	PORT_BIT(0x2000, IP_ACTIVE_LOW,  IPT_BUTTON1)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // S1
	PORT_BIT(0x4000, IP_ACTIVE_LOW,  IPT_BUTTON2)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // S2
	PORT_BIT(0x8000, IP_ACTIVE_LOW,  IPT_BUTTON3)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // S3
	PORT_BIT(0xff00, IP_ACTIVE_LOW,  IPT_UNKNOWN)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)

	PORT_START("IN2")
	PORT_BIT(0x0001, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT)                                                     // 退币
	PORT_BIT(0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT)         PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  // 洗分
	PORT_BIT(0x0004, IP_ACTIVE_LOW,  IPT_CUSTOM)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // HPSW.
	PORT_BIT(0x0006, IP_ACTIVE_LOW,  IPT_UNKNOWN)               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_BIT(0xfff8, IP_ACTIVE_LOW,  IPT_UNKNOWN)

	PORT_START("OUT0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT )               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  PORT_WRITE_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::motor_w))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  PORT_WRITE_LINE_MEMBER(FUNC(igs_m027_023vid_state::counter_w<0>)) // coin or key-in
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)  PORT_WRITE_LINE_MEMBER(FUNC(igs_m027_023vid_state::counter_w<2>)) // payout or key-out

	PORT_START("OUT1")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_WRITE_LINE_MEMBER(FUNC(igs_m027_023vid_state::counter_w<0>)) // coin
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_WRITE_LINE_MEMBER(FUNC(igs_m027_023vid_state::counter_w<2>)) // payout
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OUTPUT)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_WRITE_LINE_MEMBER(FUNC(igs_m027_023vid_state::counter_w<1>)) // key-in
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OUTPUT)                PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_WRITE_LINE_MEMBER(FUNC(igs_m027_023vid_state::counter_w<3>)) // key-out
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OUTPUT )               PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_WRITE_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::motor_w))

	PORT_START("CLEARMEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE

	PORT_START("DSW1")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Controls))             PORT_DIPLOCATION("SW1:1")          // PLAY MODE
	PORT_DIPSETTING(   0x01, DEF_STR(Joystick))                                                // JAMMA
	PORT_DIPSETTING(   0x00, "Mahjong")                                                        // MJ
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/


u32 igs_m027_023vid_state::external_rom_r(offs_t offset)
{
	return m_external_rom[offset] ^ m_xor_table[offset & 0x00ff];
}

void igs_m027_023vid_state::xor_table_w(offs_t offset, u8 data)
{
	m_xor_table[offset] = (u32(data) << 24) | (u32(data) << 8);
}


ioport_value igs_m027_023vid_state::kbd_r()
{
	ioport_value result = 0x3f;
	for (unsigned i = 0; m_io_kbd.size() > i; ++i)
	{
		if (!BIT(m_kbd_sel, i))
			result &= m_io_kbd[i]->read();
	}
	return result;
}

u16 igs_m027_023vid_state::dsw_r()
{
	u16 result = 0x00ff;
	for (unsigned i = 0; m_io_dsw.size() > i; ++i)
	{
		if (!BIT(m_gpio_out, i))
			result &= m_io_dsw[i]->read();
	}
	return 0xff00 | result;
}

void igs_m027_023vid_state::kbd_w(u16 data)
{
	m_kbd_sel = data & 0x001f;
}


u32 igs_m027_023vid_state::gpio_r()
{
	// The function that reads this only looks at the low eight bits.
	// If bit 0 is clear, the FIQ handler sits in a tight loop at
	// 0x0000'3154 waiting for it to be set.
	// The IRQ handler checks bit 1.
	return
			0xffffc | // unused
			(m_irq_source ? 0x00002 : 0x00000) | // checked by IRQ handler - clear for scan line interrupt?
			0x00001; // FIQ handler sits in a loop if this is clear
}

void igs_m027_023vid_state::gpio_w(u8 data)
{
	// bits 0-2 select DIP switch banks 1-3, respectively
	// the game pulses bit 3 after every IRQ
	m_gpio_out = data;
}

u16 igs_m027_023vid_state::sprites_r(offs_t offset)
{
	// there does seem to be a spritelist at the start of mainram like PGM
	// it is also copied to a secondary RAM area, which seems to be our datasource in this case

	address_space &mem = m_maincpu->space(AS_PROGRAM);
	u16 const sprdata = mem.read_word(0x28000000 + offset * 2);
	return sprdata;
}

template <unsigned N>
void igs_m027_023vid_state::irq_w(int state)
{
	if (state)
	{
		m_irq_source = N;
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time());
	}
}


void igs_m027_023vid_state::m027_023vid(machine_config &config)
{
	IGS027A(config, m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027_023vid_state::m027_map);
	m_maincpu->in_port().set(FUNC(igs_m027_023vid_state::gpio_r));
	m_maincpu->out_port().set(FUNC(igs_m027_023vid_state::gpio_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1000));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 448-1, 0, 224-1);
	m_screen->set_screen_update(m_video, FUNC(igs023_video_device::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(igs_m027_023vid_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1200/2);

	// PGM video
	IGS023_VIDEO(config, m_video, 0);
	m_video->set_palette(m_palette);
	m_video->read_spriteram_callback().set(FUNC(igs_m027_023vid_state::sprites_r));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ICS2115(config, m_ics, 33.8688_MHz_XTAL);
	m_ics->irq().set(FUNC(igs_m027_023vid_state::irq_w<1>));
	m_ics->add_route(ALL_OUTPUTS, "mono", 5.0);

	HOPPER(config, m_hopper, attotime::from_msec(50));
}


/***************************************************************************

    ROMs Loading

***************************************************************************/


// 明星三缺一 (Míngxīng sān quē yī)
ROM_START( mxsqy )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "a8_027a.u41", 0x00000, 0x4000, CRC(f9ada8c4) SHA1(0715fdc3d15ae2d1af4e9c7d25f6410ae7c22d42) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "igs_m2401.u39", 0x000000, 0x80000, CRC(32e69540) SHA1(e5bc44700ba965fae433c3b39afa95bc753e3f2a) )

	ROM_REGION( 0x80000, "igs023",  0 )
	ROM_LOAD( "igs_l2405.u38", 0x00000, 0x80000, CRC(2f20eade) SHA1(aa11d26cb51483af5fdd4b181dff0f222baeaaff) )

	ROM_REGION16_LE( 0x400000, "igs023:sprcol", 0 )
	ROM_LOAD( "igs_l2404.u23", 0x000000, 0x400000, CRC(dc8ff7ae) SHA1(4609b5543d8bea7a8dea4e744f81c407688a96ee) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION16_LE( 0x400000, "igs023:sprmask", 0 )
	ROM_LOAD( "igs_m2403.u22", 0x000000, 0x400000, CRC(53940332) SHA1(3c703cbdc51dfb100f3ce10452a81091305dee01) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "igs_s2402.u21", 0x000000, 0x400000, CRC(a3e3b2e0) SHA1(906e5839ab62e570d9716e01b49e5b067e041269) )
ROM_END


void igs_m027_023vid_state::init_mxsqy()
{
	mxsqy_decrypt(machine());
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

// internal ROM is 2003
GAME( 2003, mxsqy, 0, m027_023vid, mxsqy, igs_m027_023vid_state, init_mxsqy, ROT0, "IGS", "Mingxing San Que Yi (China, V201CN)", MACHINE_IMPERFECT_GRAPHICS )
