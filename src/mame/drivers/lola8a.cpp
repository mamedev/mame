// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Lola 8A

        Ivo Lola Ribar Institute

        2013-08-28 Skeleton driver.


    BASIC commands :

    LET NEXT IF GOTO GOSUB RETURN READ DATA FOR CLS INPUT DIM STOP END RESTORE
    REM CLEAR PUSH POKE PRINT OUT ERROR USR CURSOR NORMAL INVERSE PLOT UNPLOT
    ELSE WIPE COLOUR CENTRE RANGE DRAW CIRCLE LOAD SAVE VERIFY HLOAD HSAVE HVERIFY
    DLOAD DSAVE DVERIFY MERGE CAT RUN NEW ON LIST DEF MON GWIND TWIND UNDER
    SPC OFF TAB THEN TO STEP AND OR XOR NOT ABS LEN SQR INT ASC CHR VAL STR MID
    ARG CALL RND LEFT RIGHT DOT SGN SIN FREE PI FN TAN COS POP PEEK INP LN EXP ATN

    COLOUR x (x = 0 to 3) there's no known colour ram, unable to determine
    how colours can be displayed. Therefore we only show black and white.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define AY8910_TAG "g12"
#define HD46505SP_TAG "h45"


class lola8a_state : public driver_device
{
public:
	lola8a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_palette(*this, "palette")
		, m_p_videoram(*this, "videoram")
	{ }

	void lola8a(machine_config &config);

private:
	DECLARE_READ8_MEMBER(lola8a_port_a_r);
	DECLARE_WRITE8_MEMBER(lola8a_port_b_w);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync);
	DECLARE_READ_LINE_MEMBER(cass_r);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	MC6845_UPDATE_ROW(crtc_update_row);

	void lola8a_io(address_map &map);
	void lola8a_mem(address_map &map);

	uint8_t m_portb;
	virtual void machine_reset() override { m_maincpu->set_pc(0x8000); }
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_p_videoram;
};

void lola8a_state::lola8a_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram(); // 6264 at G45
	map(0x2000, 0x3fff).ram(); // 6264 at F45
										// empty place for 6264 at E45
										// empty place for 6264 at D45
	map(0x8000, 0x9fff).rom(); // 2764A at B45
	map(0xa000, 0xbfff).rom(); // 2764A at C45
	map(0xc000, 0xdfff).rom(); // 2764A at H67
	map(0xe000, 0xffff).ram().share("videoram"); // 6264 at G67
}

void lola8a_state::lola8a_io(address_map &map)
{
	map.unmap_value_high();
	map(0x80, 0x80).w(AY8910_TAG, FUNC(ay8910_device::address_w));
	map(0x84, 0x84).rw(AY8910_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x88, 0x88).r(FUNC(lola8a_state::keyboard_r));

	map(0x90, 0x90).rw(HD46505SP_TAG, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x92, 0x92).rw(HD46505SP_TAG, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

}

/* Input ports */
static INPUT_PORTS_START( lola8a )
	PORT_START("line_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)// Z"
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // C"
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) // =
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // S"
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // C'
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // ;
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)// /
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // ????
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // :
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("line_9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) // return
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED) // ????
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // ????
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // ????
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)// @
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
INPUT_PORTS_END

MC6845_UPDATE_ROW( lola8a_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 x,gfx;
	u16 mem;
	u32 *p = &bitmap.pix32(y);
	ma &= 0x7ff;

	for (x = 0; x < x_count; x++)
	{
		mem = (x+ma)*8 + ra;
		gfx = m_p_videoram[mem] ^ ((cursor_x == x) ? 0xff : 0);

		*p++ = palette[BIT(gfx, 7) ? 7 : 0];
		*p++ = palette[BIT(gfx, 6) ? 7 : 0];
		*p++ = palette[BIT(gfx, 5) ? 7 : 0];
		*p++ = palette[BIT(gfx, 4) ? 7 : 0];
		*p++ = palette[BIT(gfx, 3) ? 7 : 0];
		*p++ = palette[BIT(gfx, 2) ? 7 : 0];
		*p++ = palette[BIT(gfx, 1) ? 7 : 0];
		*p++ = palette[BIT(gfx, 0) ? 7 : 0];
	}
}


READ8_MEMBER(lola8a_state::lola8a_port_a_r)
{
	logerror("lola8a_port_a_r\n");
	return 0x00;
}

WRITE8_MEMBER(lola8a_state::lola8a_port_b_w)
{
	m_portb = data;
}

READ_LINE_MEMBER( lola8a_state::cass_r )
{
	return (m_cass->input() < 0.03);
}

WRITE_LINE_MEMBER( lola8a_state::cass_w )
{
	m_cass->output(state ? -1.0 : +1.0);
}

READ8_MEMBER(lola8a_state::keyboard_r)
{
	static const char *const keynames[] =
	{
		"line_0", "line_1", "line_2", "line_3",
		"line_4", "line_5", "line_6", "line_7",
		"line_8", "line_9", nullptr, nullptr,
		nullptr,nullptr,nullptr,nullptr
	};

	if( keynames[m_portb & 0x0f])
		return ioport(keynames[m_portb & 0x0f])->read();
	return 0xff;
}

WRITE_LINE_MEMBER(lola8a_state::crtc_vsync)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state? ASSERT_LINE : CLEAR_LINE);
}

void lola8a_state::lola8a(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &lola8a_state::lola8a_mem);
	m_maincpu->set_addrmap(AS_IO, &lola8a_state::lola8a_io);
	m_maincpu->in_sid_func().set(FUNC(lola8a_state::cass_r));
	m_maincpu->out_sod_func().set(FUNC(lola8a_state::cass_w));

	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, AY8910_TAG, XTAL(4'915'200) / 4));
	aysnd.port_a_read_callback().set(FUNC(lola8a_state::lola8a_port_a_r));
	aysnd.port_b_write_callback().set(FUNC(lola8a_state::lola8a_port_b_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(HD46505SP_TAG, FUNC(hd6845_device::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);

	hd6845_device &crtc(HD6845(config, HD46505SP_TAG, XTAL(8'000'000) / 8)); // HD6845 == HD46505S
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(lola8a_state::crtc_update_row), this);
	crtc.out_vsync_callback().set(FUNC(lola8a_state::crtc_vsync));

	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	/* Cassette */
	CASSETTE(config, m_cass);
	WAVE(config, "wave", m_cass).add_route(ALL_OUTPUTS, "mono", 0.25);
}

/* ROM definition */
ROM_START( lola8a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lola 8a r0 w06 22.11.86.b45", 0x8000, 0x2000, CRC(aca1fc08) SHA1(f7076d937bb53b0addcba2a5b7c05ab75d6d0d93))
	ROM_LOAD( "lola 8a r1 w06 22.11.86.c45", 0xa000, 0x2000, CRC(99f8ec9b) SHA1(88eafd09c479f177525fa0039cf04d74bae39dab))
	ROM_LOAD( "lola 8a r2 w06 22.11.86.h67", 0xc000, 0x2000, CRC(1e7cd46b) SHA1(048b2583ee7baeb9621e629b79ed64583ac5d554))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                    FULLNAME   FLAGS
COMP( 1986, lola8a, 0,      0,      lola8a,  lola8a, lola8a_state, empty_init, "Institut Ivo Lola Ribar", "Lola 8A", MACHINE_NOT_WORKING )
