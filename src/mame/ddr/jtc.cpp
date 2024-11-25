// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/***************************************************************************************

Jugend+Technik CompJU+TEr

2009-07-15 Skeleton driver.

2018-09: Made mostly working

To Do:
- Figure out how to use the so-called "Basic", all documents are in German.
- Fix any remaining CPU bugs
- On jtces40, the use of ALT key will usually freeze the system. Normal, or a bug?
- On jtces40, no backspace?
- On jtces40, is there a way to type lower case?
- On jtces40, hires gfx and colours to fix.

****************************************************************************************/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "cpu/z8/z8.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "imagedev/snapquik.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "utf8.h"


namespace {

#define UB8830D_TAG     "ub8830d"
#define CENTRONICS_TAG  "centronics"

#define JTC_ES40_VIDEORAM_SIZE  0x2000

class jtc_state : public driver_device
{
public:
	jtc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, UB8830D_TAG)
		, m_ram(*this, RAM_TAG)
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_centronics(*this, CENTRONICS_TAG)
		, m_video_ram(*this, "videoram")
	{ }

	void basic(machine_config &config);
	void jtc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	void p2_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	int m_centronics_busy = 0;
	void write_centronics_busy(int state);
	required_device<z8_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_centronics;
	optional_shared_ptr<u8> m_video_ram;

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void jtc_mem(address_map &map) ATTR_COLD;
};

class jtces88_state : public jtc_state
{
public:
	using jtc_state::jtc_state;
	void jtces88(machine_config &config);
};


class jtces23_state : public jtc_state
{
public:
	using jtc_state::jtc_state;
	void jtces23(machine_config &config);
private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void jtc_es23_mem(address_map &map) ATTR_COLD;
};


class jtces40_state : public jtc_state
{
public:
	jtces40_state(const machine_config &mconfig, device_type type, const char *tag)
		: jtc_state(mconfig, type, tag)
		, m_video_ram_40(*this, "videoram40", JTC_ES40_VIDEORAM_SIZE, ENDIANNESS_BIG)
		, m_color_ram_r(*this, "color_ram_r", JTC_ES40_VIDEORAM_SIZE, ENDIANNESS_BIG)
		, m_color_ram_g(*this, "color_ram_g", JTC_ES40_VIDEORAM_SIZE, ENDIANNESS_BIG)
		, m_color_ram_b(*this, "color_ram_b", JTC_ES40_VIDEORAM_SIZE, ENDIANNESS_BIG)
	{ }
	void jtces40(machine_config &config);
private:
	virtual void video_start() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u8 videoram_r(offs_t offset);
	void videoram_w(offs_t offset, u8 data);
	void banksel_w(u8 data);
	u8 m_video_bank = 0U;

	memory_share_creator<uint8_t> m_video_ram_40;
	memory_share_creator<uint8_t> m_color_ram_r;
	memory_share_creator<uint8_t> m_color_ram_g;
	memory_share_creator<uint8_t> m_color_ram_b;
	void jtc_es40_mem(address_map &map) ATTR_COLD;
	void es40_palette(palette_device &palette) const;
};


/* Read/Write Handlers */

void jtc_state::p2_w(u8 data)
{
	/*

	    bit     description

	    P20
	    P21
	    P22
	    P23
	    P24
	    P25     centronics strobe output
	    P26     V4093 pins 1,2
	    P27     DL299 pin 18

	*/

	m_centronics->write_strobe(BIT(data, 5));
}

void jtc_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

u8 jtc_state::p3_r()
{
	/*

	    bit     description

	    P30     tape input
	    P31
	    P32
	    P33     centronics busy input

	*/

	u8 data = 0;

	data |= ((m_cassette)->input() < 0.0) ? 1 : 0;
	data |= m_centronics_busy << 3;

	return data;
}

void jtc_state::p3_w(u8 data)
{
	/*

	    bit     description

	    P34
	    P35
	    P36     tape output
	    P37     speaker output

	*/

	/* tape */
	if (BIT(data, 7))
		m_cassette->output( BIT(data, 6) ? +1.0 : -1.0);
	else
	/* speaker */
		m_speaker->level_w(BIT(data, 6));
}

u8 jtces40_state::videoram_r(offs_t offset)
{
	u8 data = 0;

	if (BIT(m_video_bank, 7)) data |= m_color_ram_r[offset];
	if (BIT(m_video_bank, 6)) data |= m_color_ram_g[offset];
	if (BIT(m_video_bank, 5)) data |= m_color_ram_b[offset];
	if (BIT(m_video_bank, 4)) data |= m_video_ram[offset];

	return data;
}

void jtces40_state::videoram_w(offs_t offset, u8 data)
{
	if (BIT(m_video_bank, 7)) m_color_ram_r[offset] = data;
	if (BIT(m_video_bank, 6)) m_color_ram_g[offset] = data;
	if (BIT(m_video_bank, 5)) m_color_ram_b[offset] = data;
	if (BIT(m_video_bank, 4)) m_video_ram_40[offset] = data;
}

void jtces40_state::banksel_w(u8 data)
{
	m_video_bank = data;
}

/* Memory Maps */

void jtc_state::jtc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0800, 0x0fff).rom();
	map(0x2000, 0x27ff).rom();
	map(0x7001, 0x7001).mirror(0x0ff0).portr("Y1");
	map(0x7002, 0x7002).mirror(0x0ff0).portr("Y2");
	map(0x7003, 0x7003).mirror(0x0ff0).portr("Y3");
	map(0x7004, 0x7004).mirror(0x0ff0).portr("Y4");
	map(0x7005, 0x7005).mirror(0x0ff0).portr("Y5");
	map(0x7006, 0x7006).mirror(0x0ff0).portr("Y6");
	map(0x7007, 0x7007).mirror(0x0ff0).portr("Y7");
	map(0x7008, 0x7008).mirror(0x0ff0).portr("Y8");
	map(0x7009, 0x7009).mirror(0x0ff0).portr("Y9");
	map(0x700a, 0x700a).mirror(0x0ff0).portr("Y10");
	map(0x700b, 0x700b).mirror(0x0ff0).portr("Y11");
	map(0x700c, 0x700c).mirror(0x0ff0).portr("Y12");
	map(0x700d, 0x700d).mirror(0x0ff0).portr("Y13");
	map(0x700e, 0x700e).mirror(0x0ff0).portr("Y14");
	map(0x700f, 0x700f).mirror(0x0ff0).portr("Y15");
	map(0xe000, 0xfdff).ram();
	map(0xfe00, 0xffff).ram().share("videoram");
}

void jtces23_state::jtc_es23_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0800, 0x17ff).rom();
	map(0x7000, 0x7000).mirror(0x0ff0).portr("Y0");
	map(0x7001, 0x7001).mirror(0x0ff0).portr("Y1");
	map(0x7002, 0x7002).mirror(0x0ff0).portr("Y2");
	map(0x7003, 0x7003).mirror(0x0ff0).portr("Y3");
	map(0x7004, 0x7004).mirror(0x0ff0).portr("Y4");
	map(0x7005, 0x7005).mirror(0x0ff0).portr("Y5");
	map(0x7006, 0x7006).mirror(0x0ff0).portr("Y6");
	map(0x7007, 0x7007).mirror(0x0ff0).portr("Y7");
	map(0x7008, 0x7008).mirror(0x0ff0).portr("Y8");
	map(0x7009, 0x7009).mirror(0x0ff0).portr("Y9");
	map(0x700a, 0x700a).mirror(0x0ff0).portr("Y10");
	map(0x700b, 0x700b).mirror(0x0ff0).portr("Y11");
	map(0x700c, 0x700c).mirror(0x0ff0).portr("Y12");
	map(0x700d, 0x700d).mirror(0x0ff0).portr("Y13");
	map(0x700e, 0x700e).mirror(0x0ff0).portr("Y14");
	map(0x700f, 0x700f).mirror(0x0ff0).portr("Y15");
	map(0xe000, 0xf7ff).ram();
	map(0xf800, 0xffff).ram().share("videoram");
}

void jtces40_state::jtc_es40_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0800, 0x1fff).rom();
	map(0x4000, 0x5fff).rw(FUNC(jtces40_state::videoram_r), FUNC(jtces40_state::videoram_w));
	map(0x6000, 0x63ff).w(FUNC(jtces40_state::banksel_w));
	map(0x7000, 0x7000).mirror(0x0ff0).portr("Y0");
	map(0x7001, 0x7001).mirror(0x0ff0).portr("Y1");
	map(0x7002, 0x7002).mirror(0x0ff0).portr("Y2");
	map(0x7003, 0x7003).mirror(0x0ff0).portr("Y3");
	map(0x7004, 0x7004).mirror(0x0ff0).portr("Y4");
	map(0x7005, 0x7005).mirror(0x0ff0).portr("Y5");
	map(0x7006, 0x7006).mirror(0x0ff0).portr("Y6");
	map(0x7007, 0x7007).mirror(0x0ff0).portr("Y7");
	map(0x7008, 0x7008).mirror(0x0ff0).portr("Y8");
	map(0x7009, 0x7009).mirror(0x0ff0).portr("Y9");
	map(0x700a, 0x700a).mirror(0x0ff0).portr("Y10");
	map(0x700b, 0x700b).mirror(0x0ff0).portr("Y11");
	map(0x700c, 0x700c).mirror(0x0ff0).portr("Y12");
	map(0x700d, 0x700d).mirror(0x0ff0).portr("Y13");
	map(0x700e, 0x700e).mirror(0x0ff0).portr("Y14");
	map(0x700f, 0x700f).mirror(0x0ff0).portr("Y15");
	map(0x8000, 0xffff).ram();//BANK(1)
}

/* Input Ports */

static INPUT_PORTS_START( jtc )
	PORT_START("Y1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91 \xE2\x86\x93") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90 \xE2\x86\x92") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("Y3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("Y4")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("Y5")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("Y6")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("Y7")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("Y8")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(0x2030) // per mille
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("Y9")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("Y10")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("Y11")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR('*') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("Y12")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('+') PORT_CHAR(';')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("Y13")
	PORT_START("Y14")
	PORT_START("Y15")
INPUT_PORTS_END

static INPUT_PORTS_START( jtces23 )
	PORT_START("Y0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("Y2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("Y3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("Y4")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("Y5")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("Y6")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("Y7")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("Y8")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('[')

	PORT_START("Y9")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(']')

	PORT_START("Y10")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("Y11")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('+') PORT_CHAR('\\')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RET") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("Y12")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) //PORT_CODE() PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) //PORT_CODE() PORT_CHAR('*') PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y13")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // DBS
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // DEL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // SPA

	PORT_START("Y14")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // INS
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // HOM
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("Y15")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // CLS
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // SOL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // RET
INPUT_PORTS_END

static INPUT_PORTS_START( jtces40 )
	PORT_START("Y0")

	PORT_START("Y1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHT3") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHT2") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHT1") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("Y3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y4")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("Y5")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("Y6")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("Y7")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("Y8")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("Y9")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('[') PORT_CHAR('{')

	PORT_START("Y10")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("Y11")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR('=') PORT_CHAR('_')

	PORT_START("Y12")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('<') PORT_CHAR('>') PORT_CHAR('^')
	// can't find this: PORT_NAME("CLR") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('+') PORT_CHAR('-') PORT_CHAR('\\')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('*') PORT_CHAR('/') PORT_CHAR('|')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RET") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_CHAR('?') PORT_CHAR('~')

	PORT_START("Y13")
	PORT_START("Y14")
	PORT_START("Y15")
INPUT_PORTS_END

QUICKLOAD_LOAD_MEMBER(jtc_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	u16 quick_addr;
	std::vector<u8> quick_data;

	int quick_length = image.length();
	if (image.is_filetype("jtc"))
	{
		if (quick_length < 0x0088)
			return std::make_pair(image_error::INVALIDLENGTH, "File too short");
		else if (quick_length > 0x8000)
			return std::make_pair(image_error::INVALIDLENGTH, "File too long");

		quick_data.resize(quick_length+1);
		int const read_ = image.fread(&quick_data[0], quick_length);
		if (read_ != quick_length)
			return std::make_pair(image_error::UNSPECIFIED, "Cannot read the file");

		quick_addr = quick_data[0x12] * 256 + quick_data[0x11];
		quick_length = quick_data[0x14] * 256 + quick_data[0x13] - quick_addr + 0x81;
		if (image.length() != quick_length)
			return std::make_pair(image_error::INVALIDIMAGE, "Invalid file header");

		for (int i = 0x80; i < image.length(); i++)
			space.write_byte(quick_addr+i-0x80, quick_data[i]);

		// display a message about the loaded quickload
		image.message(" Quickload: size=%04X : loaded at %04X",quick_length,quick_addr);

		return std::make_pair(std::error_condition(), std::string());
	}
	else if (image.is_filetype("bin"))
	{
		quick_addr = 0xe000;
		if (quick_length > 0x8000)
			return std::make_pair(image_error::INVALIDLENGTH, "File too long");

		quick_data.resize(quick_length+1);
		u16 read_ = image.fread( &quick_data[0], quick_length);
		if (read_ != quick_length)
			return std::make_pair(image_error::UNSPECIFIED, "Cannot read the file");

		for (int i = 0; i < image.length(); i++)
			space.write_byte(quick_addr+i, quick_data[i]);

		// display a message about the loaded quickload
		image.message(" Quickload: size=%04X : loaded at %04X",quick_length,quick_addr);

		m_maincpu->set_pc(quick_addr);

		return std::make_pair(std::error_condition(), std::string());
	}

	return std::make_pair(image_error::UNSUPPORTED, std::string());
}


/* Video */

u32 jtc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u8 y = 0; y < 64; y++)
	{
		for (u8 sx = 0; sx < 8; sx++)
		{
			u8 const data = m_video_ram[(y * 8) + sx];

			for (u8 i = 0; i < 8; i++)
				bitmap.pix(y, (sx * 8) + i) = BIT(data, 7-i);
		}
	}

	return 0;
}

u32 jtces23_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u8 y = 0; y < 128; y++)
	{
		for (u8 sx = 0; sx < 16; sx++)
		{
			u8 const data = m_video_ram[(y * 16) + sx];

			for (u8 i = 0; i < 8; i++)
				bitmap.pix(y, (sx * 8) + i) = BIT(data, 7-i);
		}
	}

	return 0;
}

void jtces40_state::es40_palette(palette_device &palette) const
{
	for (u8 i = 8; i < 16; i++)
		palette.set_pen_color(i, rgb_t(BIT(i, 0) ? 0xc0 : 0, BIT(i, 1) ? 0xc0 : 0, BIT(i, 2) ? 0xc0 : 0));
}

void jtces40_state::video_start()
{
	/* register for state saving */
	save_item(NAME(m_video_bank));
	save_item(NAME(m_centronics_busy));
}

u32 jtces40_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 ma = 0;

	for (int y = 0; y < 64; y++)
	{
		for (int z = 0; z < 3; z++)
		{
			for (int x = 0; x < 40; x++)
			{
				u8 const data = m_video_ram_40[ma + x];
				u8 const r = ~m_color_ram_r[ma + x];
				u8 const g = ~m_color_ram_g[ma + x];
				u8 const b = ~m_color_ram_b[ma + x];

				for (int i = 0; i < 8; i++)
					bitmap.pix(y*3+z, (x * 8) + 7 - i) = (BIT(r, i) << 0) | (BIT(g, i) << 1) | (BIT(b, i) << 2) | (BIT(data, i) << 3);
			}
			ma+=40;
		}
		ma+=8;
	}

	return 0;
}

/* Machine Initialization */

void jtc_state::machine_start()
{
	/* register for state saving */
}

/* Machine Driver */

/* F4 Character Displayer */
static const gfx_layout jtces23_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	64,                 /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout jtces40_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_jtces23 )
	GFXDECODE_ENTRY( UB8830D_TAG, 0x1000, jtces23_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_jtces40 )
	GFXDECODE_ENTRY( UB8830D_TAG, 0x1000, jtces40_charlayout, 0, 8 )
GFXDECODE_END

void jtc_state::basic(machine_config &config)
{
	/* basic machine hardware */
	UB8830D(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &jtc_state::jtc_mem);
	m_maincpu->p2_out_cb().set(FUNC(jtc_state::p2_w));
	m_maincpu->p3_in_cb().set(FUNC(jtc_state::p3_r));
	m_maincpu->p3_out_cb().set(FUNC(jtc_state::p3_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(jtc_state::write_centronics_busy));

	/* quickload */
	QUICKLOAD(config, "quickload", "jtc,bin", attotime::from_seconds(2)).set_load_callback(FUNC(jtc_state::quickload_cb));
}

void jtc_state::jtc(machine_config &config)
{
	basic(config);
	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(jtc_state::screen_update));
	screen.set_size(64, 64);
	screen.set_visarea(0, 64-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* internal ram */
	RAM(config, m_ram).set_default_size("2K");
}

void jtces88_state::jtces88(machine_config &config)
{
	jtc(config);

	/* internal ram */
	m_ram->set_default_size("4K");
}

void jtces23_state::jtces23(machine_config &config)
{
	basic(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &jtces23_state::jtc_es23_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(jtces23_state::screen_update));
	screen.set_size(128, 128);
	screen.set_visarea(0, 128-1, 0, 128-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_jtces23);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("4K");
}

void jtces40_state::jtces40(machine_config &config)
{
	basic(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &jtces40_state::jtc_es40_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(jtces40_state::screen_update));
	screen.set_size(320, 192);
	screen.set_visarea(0, 320-1, 0, 192-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_jtces40);
	PALETTE(config, "palette", FUNC(jtces40_state::es40_palette), 16);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("8K").set_extra_options("16K,32K");
}

/* ROMs */

ROM_START( jtc )
	ROM_REGION( 0x2800, UB8830D_TAG, 0 )
	ROM_LOAD( "u883rom.bin", 0x0000, 0x0800, CRC(2453c8c1) SHA1(816f5d08f8064b69b1779eb6661fde091aa58ba8) )
	ROM_LOAD( "os2k_0800.bin", 0x0800, 0x0800, CRC(c81a2e19) SHA1(97c3b36c7b555081e084403e8f800fc9dbf5e68d) ) // u2716c1.bin
	ROM_LOAD( "u2716c2.bin", 0x2000, 0x0800, NO_DUMP ) // doesn't seem to be needed?
ROM_END

ROM_START( jtces88 )
	ROM_REGION( 0x2800, UB8830D_TAG, 0 )
	ROM_LOAD( "u883rom.bin", 0x0000, 0x0800, CRC(2453c8c1) SHA1(816f5d08f8064b69b1779eb6661fde091aa58ba8) )
	ROM_LOAD( "es1988_0800.bin", 0x0800, 0x0800, CRC(af3e882f) SHA1(65af0d0f5f882230221e9552707d93ed32ba794d) )
	ROM_LOAD( "es1988_2000.bin", 0x2000, 0x0800, CRC(5ff87c1e) SHA1(fbd2793127048bd9706970b7bce84af2cb258dc5) )
ROM_END

ROM_START( jtces23 )
	ROM_REGION( 0x2800, UB8830D_TAG, 0 )
	ROM_LOAD( "u883rom.bin", 0x0000, 0x0800, CRC(2453c8c1) SHA1(816f5d08f8064b69b1779eb6661fde091aa58ba8) )
	ROM_LOAD( "es23_0800.bin", 0x0800, 0x1000, CRC(16128b64) SHA1(90fb0deeb5660f4a2bb38d51981cc6223d5ddf6b) )
ROM_END

ROM_START( jtces40 )
	ROM_REGION( 0x2800, UB8830D_TAG, 0 )
	ROM_LOAD( "u883rom.bin", 0x0000, 0x0800, CRC(2453c8c1) SHA1(816f5d08f8064b69b1779eb6661fde091aa58ba8) )
	ROM_LOAD( "es40_0800.bin", 0x0800, 0x1800, CRC(770c87ce) SHA1(1a5227ba15917f2a572cb6c27642c456f5b32b90) )
ROM_END

} // anonymous namespace


/* System Drivers */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY           FULLNAME                    FLAGS */
COMP( 1987, jtc,     0,      0,      jtc,     jtc,     jtc_state,     empty_init, "Jugend+Technik", "CompJU+TEr",               MACHINE_SUPPORTS_SAVE )
COMP( 1988, jtces88, jtc,    0,      jtces88, jtc,     jtces88_state, empty_init, "Jugend+Technik", "CompJU+TEr (EMR-ES 1988)", MACHINE_SUPPORTS_SAVE )
COMP( 1989, jtces23, jtc,    0,      jtces23, jtces23, jtces23_state, empty_init, "Jugend+Technik", "CompJU+TEr (ES 2.3)",      MACHINE_SUPPORTS_SAVE )
COMP( 1990, jtces40, jtc,    0,      jtces40, jtces40, jtces40_state, empty_init, "Jugend+Technik", "CompJU+TEr (ES 4.0)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
