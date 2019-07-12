// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Robbbert
/***************************************************************************

JR-100 National / Panasonic

2010-08-23 Initial driver version

CPU: User manual for JR100U states it is a MN1800 (==MC6802), but the photo
shows it to be a Fujitsu MB8861H, which is a MC6800 with extra instructions.
The manual also states the clock to be 890kHz but in fact it is 894kHz.
MB14392 custom LSI generates the various clock signals for the computer.

List of extra instructions:
0x71 NIM And Immediate with memory    data & M -> M       Index, 8,3   71,data,index
     V is reset; if result = 0, Z is set, N is reset; else Z is reset, N is set
0x72 OIM Or Immediate with memory     data | M -> M       Index, 8,3   72,data,index
     V is reset; if result = 0, Z is set, N is reset; else Z is reset, N is set
0x75 XIM Xor Immediate with memory    data ^ M -> M       Index, 8,3   75,data,index
     if result = 0, Z is set, N is reset; else Z is reset, N is set
0x7B TMM Test under mask with memory  (data & M) ^ data   Index, 7,3   7B,data,index
     flags not stated
0xEC ADX Add Index Register           X + data -> X       Immed,4,2
     "adds 1 byte to the index register"?  flags not stated
0xFC ADX Add Index Register           X + (M) -> X        Extend,7,3
     "adds 2 bytes to the index register"?  flags not stated. One page says result goes to M.

There is also an MB8861 equivalent of the MC6802 which is called MB8871. Behaviour
of undefined instructions is not the same as the Motorola parts. Variants:
MB8861N : 1MHz
MB8861E : 1.3MHz
MB8861H : 2MHz

Similar to MC6875, the MB8867 could be used as a clock oscillator and divide by 8
circuit to drive the CPU directly.

CC02 joystick input register (active high)
bit 7,6: undefined
bit 5: 0
bit 4: button
bit 3: down
bit 2: up
bit 1: left
bit 0: right

TODO:
- Cassette loading doesn't work. Save is verified good.
- Need software.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class jr100_state : public driver_device
{
public:
	jr100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "ram")
		, m_pcg(*this, "pcg")
		, m_vram(*this, "vram")
		, m_rom(*this, "rom")
		, m_via(*this, "via")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_region_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "LINE%u", 0)
		, m_maincpu(*this, "maincpu")
	{ }

	void jr100(machine_config &config);

private:
	uint8_t m_keyboard_line;
	bool m_use_pcg;
	bool m_pb7;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_jr100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(sound_tick);
	DECLARE_READ8_MEMBER(pb_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_WRITE_LINE_MEMBER(cb2_w);
	uint32_t readByLittleEndian(uint8_t *buf,int pos);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void mem_map(address_map &map);

	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_pcg;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_rom;
	required_device<via6522_device> m_via;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	required_ioport_array<9> m_io_keyboard;
	required_device<m6800_cpu_device> m_maincpu;

	emu_timer *m_sound_timer;
};


void jr100_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).ram().share("ram");
	//map(0x4000, 0x7fff).ram();   expansion ram
	//map(0x8000, 0xbfff).rom();   expansion rom
	map(0xc000, 0xc0ff).ram().share("pcg").region("maincpu", 0xc000);
	map(0xc100, 0xc3ff).ram().share("vram");
	map(0xc800, 0xc80f).m(m_via, FUNC(via6522_device::map));
	//map(0xcc00, 0xcfff).;   expansion i/o
	//map(0xd000, 0xd7ff).rom();   expansion rom for printer control
	//map(0xd800, 0xdfff).rom();   expansion rom
	map(0xe000, 0xffff).rom().share("rom");
}

/* Input ports */
INPUT_PORTS_START( jr100 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) // Z
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) // X
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) // C
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) // A
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) // S
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) // D
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) // F
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) // G
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) // Q
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) // W
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) // E
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) // R
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) // T
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) // 1
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) // 2
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) // 3
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) // 4
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) // 5
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) // 6
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) // 7
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) // 8
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) // 9
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) // 0
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) // Y
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) // U
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) // I
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) // O
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) // P
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) // H
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) // J
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) // K
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) // L
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) // ;
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) // V
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) // B
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) // N
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) // M
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) // ,
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) // .
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) // space
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) // :
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) // enter
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) // -
	PORT_BIT(0xE0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void jr100_state::machine_start()
{
	if (!m_sound_timer)
		m_sound_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(jr100_state::sound_tick), this));
}

void jr100_state::machine_reset()
{
	attotime timer_period = attotime::from_hz(XTAL(14'318'181) / 16);
	m_sound_timer->adjust(timer_period, 0, timer_period);
}

uint32_t jr100_state::screen_update_jr100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool attr;
	u8 y,ra,gfx,chr;
	u16 x,sy=0,ma=0;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);
			for (x = ma; x < ma + 32; x++)
			{
				chr = m_vram[x];
				attr = BIT(chr, 7);
				chr &= 0x7f;
				// ATTR is inverted for normal char or use PCG in case of CMODE1
				if (m_use_pcg && attr && (chr < 32))
					gfx = m_pcg[(chr<<3) | ra];
				else
					gfx = m_rom[(chr<<3) | ra] ^ (attr ? 0xff : 0);

				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma += 32;
	}

	return 0;
}

static const gfx_layout tilesrom_layout =
{
	8,8,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilesram_layout =
{
	8,8,
	32,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_jr100 )
	GFXDECODE_ENTRY( "maincpu", 0xe000, tilesrom_layout, 0, 1 )   // inside rom
	GFXDECODE_ENTRY( "maincpu", 0xc000, tilesram_layout, 0, 1 )   // user defined
GFXDECODE_END

READ8_MEMBER(jr100_state::pb_r)
{
	uint8_t data = 0x1f;
	if (m_keyboard_line < 9)
		data = m_io_keyboard[m_keyboard_line]->read() & 0xbf;
	data |= (m_pb7 ? 0x40 : 0);
	return data;
}

WRITE8_MEMBER(jr100_state::pa_w)
{
	m_keyboard_line = data & 0x0f;
}

WRITE8_MEMBER(jr100_state::pb_w)
{
	m_use_pcg = BIT(data, 5);
	m_pb7 = BIT(data, 7);
	m_speaker->level_w(m_pb7);
	m_via->write_pb6(m_pb7);
}

WRITE_LINE_MEMBER(jr100_state::cb2_w)
{
	m_cassette->output(state ? -1.0 : +1.0);
}

// not working; code doesn't seem right
TIMER_CALLBACK_MEMBER(jr100_state::sound_tick)
{
	double level = (m_cassette->input());
	if (level > 0.0)
	{
		m_via->write_ca1(0);
		m_via->write_cb1(0);
	}
	else
	{
		m_via->write_ca1(1);
		m_via->write_cb1(1);
	}
}

uint32_t jr100_state::readByLittleEndian(uint8_t *buf,int pos)
{
	return buf[pos] + (buf[pos+1] << 8) + (buf[pos+2] << 16) + (buf[pos+3] << 24);
}

QUICKLOAD_LOAD_MEMBER(jr100_state::quickload_cb)
{
	int quick_length;
	uint8_t buf[0x10000];
	int read_;
	quick_length = image.length();
	if (quick_length >= 0xffff)
		return image_init_result::FAIL;
	read_ = image.fread(buf, quick_length);
	if (read_ != quick_length)
		return image_init_result::FAIL;

	if (buf[0]!=0x50 || buf[1]!=0x52 || buf[2]!=0x4F || buf[3]!=0x47)
		// this is not PRG
		return image_init_result::FAIL;

	int pos = 4;
	if (readByLittleEndian(buf,pos)!=1)
		// not version 1 of PRG file
		return image_init_result::FAIL;

	pos += 4;
	uint32_t len =readByLittleEndian(buf,pos); pos+= 4;
	pos += len; // skip name
	uint32_t start_address = readByLittleEndian(buf,pos); pos+= 4;
	uint32_t code_length   = readByLittleEndian(buf,pos); pos+= 4;
	uint32_t flag          = readByLittleEndian(buf,pos); pos+= 4;

	uint32_t end_address = start_address + code_length - 1;
	// copy code
	memcpy(m_ram + start_address,buf + pos,code_length);
	if (flag == 0)
	{
		m_ram[end_address + 1] =  0xdf;
		m_ram[end_address + 2] =  0xdf;
		m_ram[end_address + 3] =  0xdf;
		m_ram[6 ] = (end_address >> 8 & 0xFF);
		m_ram[7 ] = (end_address & 0xFF);
		m_ram[8 ] = ((end_address + 1) >> 8 & 0xFF);
		m_ram[9 ] = ((end_address + 1) & 0xFF);
		m_ram[10] = ((end_address + 2) >> 8 & 0xFF);
		m_ram[11] = ((end_address + 2) & 0xFF);
		m_ram[12] = ((end_address + 3) >> 8 & 0xFF);
		m_ram[13] = ((end_address + 3) & 0xFF);
	}

	return image_init_result::PASS;
}

void jr100_state::jr100(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(14'318'181) / 16);  //actually MB8861
	m_maincpu->set_addrmap(AS_PROGRAM, &jr100_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(256, 192); /* border size not accurate */
	screen.set_visarea(0, 256 - 1, 0, 192 - 1);
	screen.set_screen_update(FUNC(jr100_state::screen_update_jr100));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_jr100);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	VIA6522(config, m_via, XTAL(14'318'181) / 32);   // this divider produces the correct cassette save frequencies
	m_via->readpb_handler().set(FUNC(jr100_state::pb_r));
	m_via->writepa_handler().set(FUNC(jr100_state::pa_w));
	m_via->writepb_handler().set(FUNC(jr100_state::pb_w));
	m_via->cb2_handler().set(FUNC(jr100_state::cb2_w));
	m_via->irq_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	CASSETTE(config, m_cassette, 0);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* quickload */
	QUICKLOAD(config, "quickload", "prg", attotime::from_seconds(2)).set_load_callback(FUNC(jr100_state::quickload_cb), this);
}


/* ROM definition */
ROM_START( jr100 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jr100.ic5", 0xe000, 0x2000, CRC(951d08a1) SHA1(edae3daaa94924e444bbe485ac2bcd5cb5b22ca2))
ROM_END

ROM_START( jr100u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jr100u.ic5", 0xe000, 0x2000, CRC(f589dd8d) SHA1(78a51f2ae055bf4dc1b0887a6277f5dbbd8ba512))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME   FLAGS
COMP( 1981, jr100,  0,      0,      jr100,   jr100, jr100_state, empty_init, "National",  "JR-100",  0 )
COMP( 1981, jr100u, jr100,  0,      jr100,   jr100, jr100_state, empty_init, "Panasonic", "JR-100U", 0 )
