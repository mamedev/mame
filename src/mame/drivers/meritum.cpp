// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Mera-Elzab Meritum II (Poland)

Meritum I was a basic unit with no expansion, expensive, and so very rare.
Meritum II had all the standard TRS80 expansions, but the i/o is different
and thus not compatible.
Meritum III was planned, but seems to have been cancelled before release.

Split from trs80.cpp on 2018-07-15

It is quite similar to the TRS80 Model 1 Level II, however the extra chips
are quite different, being Intel ones (i8251, i8253, i8255, i8272, etc).

There's no lowercase, so the shift key will select Polish characters if
available, as well as the usual standard punctuation.
The control key appears to do nothing.
There's also a Reset key, a NMI key, and 2 blank ones.

Status:
- Starts up, runs Basic. Cassette works. Quickload mostly works.
- Some quickloads have corrupt text due to no lowercase.
- Some quickloads don't run at all. Some may crash the emulator.
- Intel chips need adding, along with the peripherals they control.
- A speaker has been included (which works), but real machine might not have
  one at that address. To be checked.
- On meritum_net, type NET to activate the networking features.
- Add Reset key and 2 blank keys.
- Need software specific to test the hardware.
- Need boot disks (MER-DOS, CP/M 2.2)

Depending on which website you read, the following might be for II, or
perhaps III:
- Add 4-colour mode, 4 shades of grey mode, and 512x192 monochrome.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "bus/rs232/rs232.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "sound/spkrdev.h"
#include "sound/wave.h"
#include "screen.h"
#include "speaker.h"
#include "emupal.h"

class meritum_state : public driver_device
{
public:
	meritum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "LINE%u", 0)
	{ }

	void meritum(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(port_ff_w);
	DECLARE_READ8_MEMBER(port_ff_r);
	DECLARE_READ8_MEMBER(keyboard_r);

	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	DECLARE_QUICKLOAD_LOAD_MEMBER(trs80_cmd);
	uint32_t screen_update_meritum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	bool m_mode;
	bool m_size_store;
	bool m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<8> m_io_keyboard;
};

void meritum_state::mem_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x3800, 0x38ff).mirror(0x300).r(FUNC(meritum_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0xffff).ram();
}

void meritum_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("audiopit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf0, 0xf3).rw("flopppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf4, 0xf7).rw("mainppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf8, 0xfb).rw("mainpit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xfc, 0xfd).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	// map(0xfe, 0xfe) audio interface
	map(0xff, 0xff).rw(FUNC(meritum_state::port_ff_r), FUNC(meritum_state::port_ff_w));
}


static INPUT_PORTS_START( meritum )
	PORT_START("LINE0")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('@')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A', 'a') PORT_CHAR(0x0104, 0x0105)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('B', 'b')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('C', 'c') PORT_CHAR(0x0106, 0x0107)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('D', 'd')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('E', 'e') PORT_CHAR(0x0118, 0x0119)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('F', 'f')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('G', 'g')

	PORT_START("LINE1")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('H', 'h')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('I', 'i')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('J', 'j')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('K', 'k')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('L', 'l') PORT_CHAR(0x0141, 0x0142)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('M', 'm')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('N', 'n') PORT_CHAR(0x0143, 0x0144)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('O', 'o') PORT_CHAR(0x00d3, 0x00f3)

	PORT_START("LINE2")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('P', 'p')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q', 'q')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('R', 'r')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('S', 's') PORT_CHAR(0x015a, 0x015b)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('T', 't')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('U', 'u')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('V', 'v')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('W', 'w')

	PORT_START("LINE3")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('X', 'x') PORT_CHAR(0x0179, 0x017a)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y', 'y')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z', 'z') PORT_CHAR(0x017b, 0x017c)
	PORT_BIT(0xF8, 0x00, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)      PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)       PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_TAB)       PORT_CHAR(UCHAR_MAMEKEY(UP), '[')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')

	PORT_START("LINE7")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0xFC, 0x00, IPT_UNUSED)

	PORT_START("NMI")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("NMI") PORT_CODE(KEYCODE_BACKSPACE) PORT_WRITE_LINE_DEVICE_MEMBER("nmigate", input_merger_device, in_w<1>)
INPUT_PORTS_END

uint32_t meritum_state::screen_update_meritum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
/* lores characters are in the character generator. Each character is 6x12 (basic characters are 6x7 excluding descenders/ascenders). */
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;
	uint8_t cols = m_mode ? 32 : 64;
	uint8_t skip = m_mode ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x+=skip)
			{
				chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[(chr<<4) | ra];

				/* Display a scanline of a character (6 pixels) */
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}

TIMER_CALLBACK_MEMBER(meritum_state::cassette_data_callback)
{
	double new_val = (m_cassette->input());

	/* Check for HI-LO transition */
	if ( m_old_cassette_val > -0.2 && new_val < -0.2 )
		m_cassette_data = true;        /* 500 baud */

	m_old_cassette_val = new_val;
}

READ8_MEMBER( meritum_state::port_ff_r )
{
/* ModeSel and cassette data
    d7 cassette data from tape
    d6 modesel setting */

	return (m_mode ? 0 : 0x40) | (m_cassette_data ? 0x80 : 0) | 0x3f;
}

WRITE8_MEMBER( meritum_state::port_ff_w )
{
/* Standard output port of Model I
    d3 ModeSel bit
    d2 Relay
    d1, d0 Cassette output */

	static const double levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	static bool init = 0;

	m_cassette->change_state(BIT(data, 2) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR );
	m_cassette->output(levels[data & 3]);
	m_cassette_data = false;

	m_mode = BIT(data, 3);

	if (!init)
	{
		init = 1;
		static int16_t speaker_levels[4] = { 0, -32767, 0, 32767 };
		m_speaker->set_levels(4, speaker_levels);

	}
}

READ8_MEMBER( meritum_state::keyboard_r )
{
	u8 i, result = 0;

	for (i = 0; i < 8; i++)
		if (BIT(offset, i))
			result |= m_io_keyboard[i]->read();

	return result;
}

void meritum_state::machine_start()
{
	m_cassette_data_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(meritum_state::cassette_data_callback),this));
	m_cassette_data_timer->adjust( attotime::zero, 0, attotime::from_hz(11025) );
}

void meritum_state::machine_reset()
{
	m_cassette_data = false;
	m_size_store = true;
	m_mode = false;
}

#define CMD_TYPE_OBJECT_CODE                            0x01
#define CMD_TYPE_TRANSFER_ADDRESS                       0x02
#define CMD_TYPE_END_OF_PARTITIONED_DATA_SET_MEMBER     0x04
#define CMD_TYPE_LOAD_MODULE_HEADER                     0x05
#define CMD_TYPE_PARTITIONED_DATA_SET_HEADER            0x06
#define CMD_TYPE_PATCH_NAME_HEADER                      0x07
#define CMD_TYPE_ISAM_DIRECTORY_ENTRY                   0x08
#define CMD_TYPE_END_OF_ISAM_DIRECTORY_ENTRY            0x0a
#define CMD_TYPE_PDS_DIRECTORY_ENTRY                    0x0c
#define CMD_TYPE_END_OF_PDS_DIRECTORY_ENTRY             0x0e
#define CMD_TYPE_YANKED_LOAD_BLOCK                      0x10
#define CMD_TYPE_COPYRIGHT_BLOCK                        0x1f

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

QUICKLOAD_LOAD_MEMBER( meritum_state, trs80_cmd )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	uint8_t type, length;
	uint8_t data[0x100];
	uint8_t addr[2];
	void *ptr;

	while (!image.image_feof())
	{
		image.fread( &type, 1);
		image.fread( &length, 1);

		length -= 2;
		int block_length = length ? length : 256;

		switch (type)
		{
		case CMD_TYPE_OBJECT_CODE:
			{
				image.fread( &addr, 2);
				uint16_t address = (addr[1] << 8) | addr[0];
				ptr = program.get_write_ptr(address);
				image.fread( ptr, block_length);
			}
			break;

		case CMD_TYPE_TRANSFER_ADDRESS:
			{
				image.fread( &addr, 2);
				uint16_t address = (addr[1] << 8) | addr[0];
				m_maincpu->set_state_int(Z80_PC, address);
			}
			break;

		case CMD_TYPE_LOAD_MODULE_HEADER:
			image.fread( &data, block_length);
			break;

		case CMD_TYPE_COPYRIGHT_BLOCK:
			image.fread( &data, block_length);
			break;

		default:
			image.fread( &data, block_length);
			logerror("/CMD unsupported block type %u!\n", type);
		}
	}

	return image_init_result::PASS;
}


/**************************** F4 CHARACTER DISPLAYER ***********************************************************/
static const gfx_layout meritum_charlayout =
{
	6, 12,          /* 6 x 12 characters */
	256,            /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16           /* every char takes 16 bytes (unused scanlines are blank) */
};

static GFXDECODE_START(gfx_meritum)
	GFXDECODE_ENTRY( "chargen", 0, meritum_charlayout, 0, 1 )
GFXDECODE_END



MACHINE_CONFIG_START(meritum_state::meritum)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, 10_MHz_XTAL / 4) // U880D @ 2.5 MHz or 1.67 MHz by jumper selection
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	i8251_device &usart(I8251(config, "usart", 10_MHz_XTAL / 4)); // same as CPU clock
	usart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("usart", FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set("usart", FUNC(i8251_device::write_cts));

	INPUT_MERGER_ALL_HIGH(config, "nmigate").output_handler().set("mainpit", FUNC(pit8253_device::write_gate2)).invert();

	pit8253_device &pit(PIT8253(config, "mainpit", 0));
	pit.set_clk<0>(10_MHz_XTAL / 5); // 2 MHz
	pit.set_clk<1>(10_MHz_XTAL / 10); // 1 MHz
	pit.set_clk<2>(10_MHz_XTAL / 4); // same as CPU clock
	pit.out_handler<0>().set("usart", FUNC(i8251_device::write_txc));
	pit.out_handler<0>().append("usart", FUNC(i8251_device::write_rxc));
	// Channel 1 generates INT pulse through 123 monostable
	pit.out_handler<2>().set_inputline(m_maincpu, INPUT_LINE_NMI);

	i8255_device &mainppi(I8255(config, "mainppi")); // parallel interface
	mainppi.out_pc_callback().set("nmigate", FUNC(input_merger_device::in_w<0>)).bit(7).invert();

	MCFG_DEVICE_ADD("flopppi", I8255, 0) // floppy disk interface
	MCFG_DEVICE_ADD("audiopit", PIT8253, 0) // optional audio interface

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(10_MHz_XTAL, 107 * 6, 0, 64 * 6, 312, 0, 192)
	MCFG_SCREEN_UPDATE_DRIVER(meritum_state, screen_update_meritum)
	MCFG_SCREEN_PALETTE("palette")

	GFXDECODE(config, "gfxdecode", "palette", gfx_meritum);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.05);

	/* devices */
	CASSETTE(config, m_cassette);
	MCFG_QUICKLOAD_ADD("quickload", meritum_state, trs80_cmd, "cmd", attotime::from_seconds(1))
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( meritum)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD( "01.bin", 0x0000, 0x0800, CRC(ed705a47) SHA1(dae8b14eb2ddb2a8b4458215180ebc0fb781816a))
	ROM_LOAD( "02.bin", 0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD( "03.bin", 0x1000, 0x0800, CRC(a21d0d62) SHA1(6dfdf3806ed2b6502e09a1b6922f21494134cc05))
	ROM_LOAD( "04.bin", 0x1800, 0x0800, CRC(3610bdda) SHA1(602f0ba1e1267f24620f993acac019ac6342a594))
	ROM_LOAD( "05.bin", 0x2000, 0x0800, CRC(461fbf0d) SHA1(bd19187dd992168af43bd68055343d515f152624))
	ROM_LOAD( "06.bin", 0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))
	ROM_LOAD( "07.bin", 0x3000, 0x0800, CRC(044b1459) SHA1(faace7353ffbef6587b1b9e7f8b312e0892e3427))

	ROM_REGION(0x1000, "chargen", ROMREGION_INVERT)
	ROM_LOAD( "chargen.bin", 0x0000, 0x1000, CRC(3dfc6439) SHA1(6e45a27f68c3491c403b4eafe45a108f348dd2fd))
ROM_END

ROM_START( meritum_net )
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD( "01_447_m07_015m.bin", 0x0000, 0x0800, CRC(6d30cb49) SHA1(558241340a84eebcbbf8d92540e028e9164b6f8a))
	ROM_LOAD( "02_440_m08_01.bin",   0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD( "03_440_m09_015m.bin", 0x1000, 0x0800, CRC(88e267da) SHA1(9cb8626801f8e969f35291de43c1b643c809a3c3))
	ROM_LOAD( "04_447_m10_015m.bin", 0x1800, 0x0800, CRC(e51991e4) SHA1(a7d42436da1af405970f9f99ab34b6d9abd05adf))
	ROM_LOAD( "05_440_m11_02.bin",   0x2000, 0x0800, CRC(461fbf0d) SHA1(bd19187dd992168af43bd68055343d515f152624))
	ROM_LOAD( "06_440_m12_01.bin",   0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))
	ROM_LOAD( "07_447_m13_015m.bin", 0x3000, 0x0800, CRC(789f6964) SHA1(9b2231ca7ffd82bbca1f53988a7df833290ddbf2))

	ROM_REGION(0x1000, "chargen", ROMREGION_INVERT)
	ROM_LOAD( "char.bin", 0x0000, 0x1000, CRC(2c09a5a7) SHA1(146891b3ddfc2de95e6a5371536394a657880054))
ROM_END


//    YEAR  NAME         PARENT    COMPAT    MACHINE   INPUT      CLASS          INIT             COMPANY              FULLNAME                FLAGS
COMP( 1985, meritum,     0,        trs80l2,  meritum,  meritum,   meritum_state, empty_init,  "Mera-Elzab", "Meritum I (Model 2)",             0 )
COMP( 1985, meritum_net, meritum,  0,        meritum,  meritum,   meritum_state, empty_init,  "Mera-Elzab", "Meritum I (Model 2) (network)",   0 )
