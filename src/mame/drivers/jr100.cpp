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
#include "machine/6522via.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "sound/speaker.h"
#include "sound/beep.h"
#include "imagedev/snapquik.h"


class jr100_state : public driver_device
{
public:
	jr100_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
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
		m_maincpu(*this, "maincpu") { }

	required_shared_ptr<UINT8> m_ram;
	required_shared_ptr<UINT8> m_pcg;
	required_shared_ptr<UINT8> m_vram;
	UINT8 m_keyboard_line;
	bool m_use_pcg;
	UINT8 m_speaker_data;
	UINT16 m_t1latch;
	UINT8 m_beep_en;
	DECLARE_WRITE8_MEMBER(jr100_via_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_jr100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_tick);
	DECLARE_READ8_MEMBER(jr100_via_read_b);
	DECLARE_WRITE8_MEMBER(jr100_via_write_a);
	DECLARE_WRITE8_MEMBER(jr100_via_write_b);
	DECLARE_WRITE_LINE_MEMBER(jr100_via_write_cb2);
	UINT32 readByLittleEndian(UINT8 *buf,int pos);
	DECLARE_QUICKLOAD_LOAD_MEMBER(jr100);


protected:
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
	required_device<cpu_device> m_maincpu;
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
			m_beeper->set_frequency(894886.25 / (double)(m_t1latch) / 2.0);
		}
	}
	m_via->write(space,offset,data);
}

static ADDRESS_MAP_START(jr100_mem, AS_PROGRAM, 8, jr100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xc000, 0xc0ff) AM_RAM AM_SHARE("pcg")
	AM_RANGE(0xc100, 0xc3ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xc800, 0xc80f) AM_DEVREAD("via", via6522_device, read) AM_WRITE(jr100_via_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

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
	m_beeper->set_frequency(0);
	m_beeper->set_state(0);
}

void jr100_state::machine_reset()
{
}

void jr100_state::video_start()
{
}

UINT32 jr100_state::screen_update_jr100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,xi,yi;

	UINT8 *rom_pcg = m_region_maincpu->base() + 0xe000;
	for (y = 0; y < 24; y++)
	{
		for (x = 0; x < 32; x++)
		{
			UINT8 tile = m_vram[x + y*32];
			UINT8 attr = tile >> 7;
			// ATTR is inverted for normal char or use PCG in case of CMODE1
			UINT8 *gfx_data = rom_pcg;
			if (m_use_pcg && attr) {
				gfx_data = m_pcg;
				attr = 0; // clear attr so bellow code stay same
			}
			tile &= 0x7f;
			for(yi=0;yi<8;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					UINT8 pen = (gfx_data[(tile*8)+yi]>>(7-xi) & 1);
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

static GFXDECODE_START( jr100 )
	GFXDECODE_ENTRY( "maincpu", 0xe000, tiles8x8_layout, 0, 1 )
GFXDECODE_END

READ8_MEMBER(jr100_state::jr100_via_read_b)
{
	UINT8 val = 0x1f;
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
	m_use_pcg = (data & 0x20) ? TRUE : FALSE;
	m_speaker_data = data>>7;
}

WRITE_LINE_MEMBER(jr100_state::jr100_via_write_cb2)
{
	m_cassette->output(state ? -1.0 : +1.0);
}

TIMER_DEVICE_CALLBACK_MEMBER(jr100_state::sound_tick)
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

UINT32 jr100_state::readByLittleEndian(UINT8 *buf,int pos)
{
	return buf[pos] + (buf[pos+1] << 8) + (buf[pos+2] << 16) + (buf[pos+3] << 24);
}

QUICKLOAD_LOAD_MEMBER( jr100_state,jr100)
{
	int quick_length;
	UINT8 buf[0x10000];
	int read_;
	quick_length = image.length();
	if (quick_length >= 0xffff)
		return IMAGE_INIT_FAIL;
	read_ = image.fread(buf, quick_length);
	if (read_ != quick_length)
		return IMAGE_INIT_FAIL;

	if (buf[0]!=0x50 || buf[1]!=0x52 || buf[2]!=0x4F || buf[3]!=0x47) {
		// this is not PRG
		return IMAGE_INIT_FAIL;
	}
	int pos = 4;
	if (readByLittleEndian(buf,pos)!=1) {
		// not version 1 of PRG file
		return IMAGE_INIT_FAIL;
	}
	pos += 4;
	UINT32 len =readByLittleEndian(buf,pos); pos+= 4;
	pos += len; // skip name
	UINT32 start_address = readByLittleEndian(buf,pos); pos+= 4;
	UINT32 code_length   = readByLittleEndian(buf,pos); pos+= 4;
	UINT32 flag          = readByLittleEndian(buf,pos); pos+= 4;

	UINT32 end_address = start_address + code_length - 1;
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

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( jr100, jr100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6802, XTAL_14_31818MHz / 4) // clock devided internaly by 4
	MCFG_CPU_PROGRAM_MAP(jr100_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 192) /* border size not accurate */
	MCFG_SCREEN_VISIBLE_AREA(0, 256 - 1, 0, 192 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(jr100_state, screen_update_jr100)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jr100)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_DEVICE_ADD("via", VIA6522, XTAL_14_31818MHz / 16)
	MCFG_VIA6522_READPB_HANDLER(READ8(jr100_state,jr100_via_read_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(jr100_state,jr100_via_write_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(jr100_state,jr100_via_write_b))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(jr100_state, jr100_via_write_cb2))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("sound_tick", jr100_state, sound_tick, attotime::from_hz(XTAL_14_31818MHz / 16))

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", jr100_state, jr100, "prg", 2)
MACHINE_CONFIG_END


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

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT  INIT        COMPANY   FULLNAME       FLAGS */
COMP( 1981, jr100,  0,        0,    jr100,  jr100, driver_device,    0,       "National",   "JR-100",       0)
COMP( 1981, jr100u, jr100,    0,    jr100,  jr100, driver_device,    0,       "Panasonic",   "JR-100U",     0)
