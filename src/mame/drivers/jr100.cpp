// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        JR-100 National / Panasonic

        23/08/2010 Initial driver version

        TODO:
        - beeper doesn't work correctly when a key is pressed (should have
          more time but it's quickly shut off), presumably there are M6802
          timing bugs that causes this quirk.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"
#include "sound/wave.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class jr100_state : public driver_device
{
public:
	jr100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_pcg(*this, "pcg"),
		m_vram(*this, "vram"),
		m_via(*this, "via"),
		m_cassette(*this, "cassette"),
		m_beeper(*this, "beeper"),
		m_speaker(*this, "speaker"),
		m_region_maincpu(*this, "maincpu"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3"),
		m_line4(*this, "LINE4"),
		m_line5(*this, "LINE5"),
		m_line6(*this, "LINE6"),
		m_line7(*this, "LINE7"),
		m_line8(*this, "LINE8") ,
		m_maincpu(*this, "maincpu")
	{ }

	void jr100(machine_config &config);

private:
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_pcg;
	required_shared_ptr<uint8_t> m_vram;
	uint8_t m_keyboard_line;
	bool m_use_pcg;
	uint8_t m_speaker_data;
	uint16_t m_t1latch;
	uint8_t m_beep_en;
	DECLARE_WRITE8_MEMBER(jr100_via_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_jr100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(sound_tick);
	DECLARE_READ8_MEMBER(jr100_via_read_b);
	DECLARE_WRITE8_MEMBER(jr100_via_write_a);
	DECLARE_WRITE8_MEMBER(jr100_via_write_b);
	DECLARE_WRITE_LINE_MEMBER(jr100_via_write_cb2);
	uint32_t readByLittleEndian(uint8_t *buf,int pos);
	DECLARE_QUICKLOAD_LOAD_MEMBER(jr100);


	void jr100_mem(address_map &map);

	required_device<via6522_device> m_via;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_ioport m_line5;
	required_ioport m_line6;
	required_ioport m_line7;
	required_ioport m_line8;
	required_device<m6802_cpu_device> m_maincpu;

	emu_timer *m_sound_timer;
};


WRITE8_MEMBER(jr100_state::jr100_via_w)
{
	/* ACR: beeper masking */
	if(offset == 0x0b)
	{
		//printf("BEEP %s\n",((data & 0xe0) == 0xe0) ? "ON" : "OFF");
		m_beep_en = ((data & 0xe0) == 0xe0);

		if(!m_beep_en)
			m_beeper->set_state(0);
	}

	/* T1L-L */
	if(offset == 0x04)
	{
		m_t1latch = (m_t1latch & 0xff00) | (data & 0xff);
		//printf("BEEP T1CL %02x\n",data);
	}

	/* T1L-H */
	if(offset == 0x05)
	{
		m_t1latch = (m_t1latch & 0xff) | ((data & 0xff) << 8);
		//printf("BEEP T1CH %02x\n",data);

		/* writing here actually enables the beeper, if above masking condition is satisfied */
		if(m_beep_en)
		{
			m_beeper->set_state(1);
			m_beeper->set_clock(894886.25 / (double)(m_t1latch) / 2.0);
		}
	}
	m_via->write(offset,data);
}

void jr100_state::jr100_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).ram().share("ram");
	map(0xc000, 0xc0ff).ram().share("pcg");
	map(0xc100, 0xc3ff).ram().share("vram");
	map(0xc800, 0xc80f).r(m_via, FUNC(via6522_device::read)).w(FUNC(jr100_state::jr100_via_w));
	map(0xe000, 0xffff).rom();
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

void jr100_state::video_start()
{
}

uint32_t jr100_state::screen_update_jr100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,xi,yi;

	uint8_t *rom_pcg = m_region_maincpu->base() + 0xe000;
	for (y = 0; y < 24; y++)
	{
		for (x = 0; x < 32; x++)
		{
			uint8_t tile = m_vram[x + y*32];
			uint8_t attr = tile >> 7;
			// ATTR is inverted for normal char or use PCG in case of CMODE1
			uint8_t *gfx_data = rom_pcg;
			if (m_use_pcg && attr) {
				gfx_data = m_pcg;
				attr = 0; // clear attr so bellow code stay same
			}
			tile &= 0x7f;
			for(yi=0;yi<8;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					uint8_t pen = (gfx_data[(tile*8)+yi]>>(7-xi) & 1);
					bitmap.pix16(y*8+yi, x*8+xi) = attr ^ pen;
				}
			}
		}
	}

	return 0;
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_jr100 )
	GFXDECODE_ENTRY( "maincpu", 0xe000, tiles8x8_layout, 0, 1 )
GFXDECODE_END

READ8_MEMBER(jr100_state::jr100_via_read_b)
{
	uint8_t val = 0x1f;
	switch ( m_keyboard_line )
	{
		case 0: val = m_line0->read(); break;
		case 1: val = m_line1->read(); break;
		case 2: val = m_line2->read(); break;
		case 3: val = m_line3->read(); break;
		case 4: val = m_line4->read(); break;
		case 5: val = m_line5->read(); break;
		case 6: val = m_line6->read(); break;
		case 7: val = m_line7->read(); break;
		case 8: val = m_line8->read(); break;
	}
	return val;
}

WRITE8_MEMBER(jr100_state::jr100_via_write_a)
{
	m_keyboard_line = data & 0x0f;
}

WRITE8_MEMBER(jr100_state::jr100_via_write_b)
{
	m_use_pcg = (data & 0x20) ? true : false;
	m_speaker_data = data>>7;
}

WRITE_LINE_MEMBER(jr100_state::jr100_via_write_cb2)
{
	m_cassette->output(state ? -1.0 : +1.0);
}

TIMER_CALLBACK_MEMBER(jr100_state::sound_tick)
{
	m_speaker->level_w(m_speaker_data);
	m_speaker_data = 0;

	double level = (m_cassette->input());
	if (level > 0.0) {
		m_via->write_ca1(0);
		m_via->write_cb1(0);
	} else {
		m_via->write_ca1(1);
		m_via->write_cb1(1);
	}
}

uint32_t jr100_state::readByLittleEndian(uint8_t *buf,int pos)
{
	return buf[pos] + (buf[pos+1] << 8) + (buf[pos+2] << 16) + (buf[pos+3] << 24);
}

QUICKLOAD_LOAD_MEMBER( jr100_state,jr100)
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

	if (buf[0]!=0x50 || buf[1]!=0x52 || buf[2]!=0x4F || buf[3]!=0x47) {
		// this is not PRG
		return image_init_result::FAIL;
	}
	int pos = 4;
	if (readByLittleEndian(buf,pos)!=1) {
		// not version 1 of PRG file
		return image_init_result::FAIL;
	}
	pos += 4;
	uint32_t len =readByLittleEndian(buf,pos); pos+= 4;
	pos += len; // skip name
	uint32_t start_address = readByLittleEndian(buf,pos); pos+= 4;
	uint32_t code_length   = readByLittleEndian(buf,pos); pos+= 4;
	uint32_t flag          = readByLittleEndian(buf,pos); pos+= 4;

	uint32_t end_address = start_address + code_length - 1;
	// copy code
	memcpy(m_ram + start_address,buf + pos,code_length);
	if (flag == 0) {
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
	M6802(config, m_maincpu, XTAL(14'318'181) / 4); // clock divided internally by 4
	m_maincpu->set_addrmap(AS_PROGRAM, &jr100_state::jr100_mem);

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

	VIA6522(config, m_via, XTAL(14'318'181) / 16);
	m_via->readpb_handler().set(FUNC(jr100_state::jr100_via_read_b));
	m_via->writepa_handler().set(FUNC(jr100_state::jr100_via_write_a));
	m_via->writepb_handler().set(FUNC(jr100_state::jr100_via_write_b));
	m_via->cb2_handler().set(FUNC(jr100_state::jr100_via_write_cb2));

	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);
	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette, 0);
	m_cassette->set_default_state((cassette_state)(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED));

	/* quickload */
	quickload_image_device &quickload(QUICKLOAD(config, "quickload"));
	quickload.set_handler(snapquick_load_delegate(&QUICKLOAD_LOAD_NAME(jr100_state, jr100), this), "prg", attotime::from_seconds(2));
}


/* ROM definition */
ROM_START( jr100 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "jr100.rom", 0xe000, 0x2000, CRC(951d08a1) SHA1(edae3daaa94924e444bbe485ac2bcd5cb5b22ca2))
ROM_END

ROM_START( jr100u )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "jr100u.rom", 0xe000, 0x2000, CRC(f589dd8d) SHA1(78a51f2ae055bf4dc1b0887a6277f5dbbd8ba512))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME   FLAGS
COMP( 1981, jr100,  0,      0,      jr100,   jr100, jr100_state, empty_init, "National",  "JR-100",  0 )
COMP( 1981, jr100u, jr100,  0,      jr100,   jr100, jr100_state, empty_init, "Panasonic", "JR-100U", 0 )
