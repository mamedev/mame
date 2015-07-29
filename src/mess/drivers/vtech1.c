// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/***************************************************************************

    Video Technology Laser 110
    Video Technology Laser 200
      Salora Fellow
      Texet TX-8000 (?)
      Video Technology VZ-200
    Video Technology Laser 210
      Dick Smith Electronics VZ-200
    Video Technology Laser 310
      Dick Smith Electronics VZ-300

Thanks go to:

    - Guy Thomason
    - Jason Oakley
    - Bushy Maunder
    - and anybody else on the vzemu list :)
    - Davide Moretti for the detailed description of the colors
    - Leslie Milburn

Todo:

    - Figure out which machines were shipped with which ROM version
      where not known (currently only a guess)
    - Lightpen support
    - Rewrite floppy

***************************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "cpu/z80/z80.h"
#include "video/mc6847.h"
#include "bus/vtech/ioexp/ioexp.h"
#include "bus/vtech/memexp/memexp.h"
#include "sound/wave.h"
#include "sound/speaker.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "formats/vt_cas.h"


/***************************************************************************
    CONSTANTS & MACROS
***************************************************************************/

#define LOG_VTECH1_LATCH 0

#define VTECH1_CLK        3579500
#define VZ300_XTAL1_CLK   XTAL_17_73447MHz


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class vtech1_state : public driver_device
{
public:
	vtech1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_mc6847(*this, "mc6847"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_ioexp(*this, "io"),
		m_memexp(*this, "mem")
	{
	}

	DECLARE_DRIVER_INIT(vtech1);
	DECLARE_DRIVER_INIT(vtech1h);

	DECLARE_READ8_MEMBER(vtech1_lightpen_r);
	DECLARE_READ8_MEMBER(vtech1_keyboard_r);
	DECLARE_WRITE8_MEMBER(vtech1_latch_w);

	DECLARE_WRITE8_MEMBER(vtech1_video_bank_w);
	DECLARE_READ8_MEMBER(mc6847_videoram_r);

	DECLARE_SNAPSHOT_LOAD_MEMBER( vtech1 );

private:
	static const UINT8 VZ_BASIC = 0xf0;
	static const UINT8 VZ_MCODE = 0xf1;

	required_shared_ptr<UINT8> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_mc6847;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<ioexp_slot_device> m_ioexp;
	required_device<memexp_slot_device> m_memexp;
};


/***************************************************************************
    SNAPSHOT LOADING
***************************************************************************/

SNAPSHOT_LOAD_MEMBER( vtech1_state, vtech1 )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 header[24];
	char pgmname[18];

	// get the header
	image.fread(&header, sizeof(header));

	// get image name
	for (int i = 0; i < 16; i++)
		pgmname[i] = header[i+4];
	pgmname[16] = '\0';

	// get start and end addresses
	UINT16 start = pick_integer_le(header, 22, 2);
	UINT16 end = start + snapshot_size - sizeof(header);
	UINT16 size = end - start;

	// write it to ram
	UINT8 *ptr = (UINT8 *)image.ptr() + sizeof(header);

	for (UINT16 addr = start; addr <= end; addr++, ptr++)
	{
		UINT8 to_write = *ptr;
		space.write_byte(addr, to_write);

		// verify
		if (space.read_byte(addr) != to_write)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Insufficient RAM to load snapshot");
			image.message("Insufficient RAM to load snapshot (%d bytes needed) [%s]", size, pgmname);

			return IMAGE_INIT_FAIL;
		}
	}

	// patch variables depending on snapshot type
	switch (header[21])
	{
	case VZ_BASIC:
		space.write_byte(0x78a4, start % 256); /* start of basic program */
		space.write_byte(0x78a5, start / 256);
		space.write_byte(0x78f9, end % 256); /* end of basic program */
		space.write_byte(0x78fa, end / 256);
		space.write_byte(0x78fb, end % 256); /* start variable table */
		space.write_byte(0x78fc, end / 256);
		space.write_byte(0x78fd, end % 256); /* start free mem, end variable table */
		space.write_byte(0x78fe, end / 256);
		image.message(" %s (B)\nsize=%04X : start=%04X : end=%04X", pgmname, size, start, end);
		break;

	case VZ_MCODE:
		space.write_byte(0x788e, start % 256); /* usr subroutine address */
		space.write_byte(0x788f, start / 256);
		image.message(" %s (M)\nsize=%04X : start=%04X : end=%04X", pgmname, size, start, end);
		m_maincpu->set_pc(start);              /* start program */
		break;

	default:
		image.seterror(IMAGE_ERROR_UNSUPPORTED, "Snapshot format not supported.");
		image.message("Snapshot format not supported.");
		return IMAGE_INIT_FAIL;
	}

	return IMAGE_INIT_PASS;
}


/***************************************************************************
    INPUTS
***************************************************************************/

READ8_MEMBER( vtech1_state::vtech1_lightpen_r )
{
	logerror("vtech1_lightpen_r(%d)\n", offset);
	return 0xff;
}

READ8_MEMBER( vtech1_state::vtech1_keyboard_r )
{
	UINT8 result = 0x3f;

	// bit 0 to 5, keyboard input
	if (!BIT(offset, 0)) result &= ioport("keyboard_0")->read();
	if (!BIT(offset, 1)) result &= ioport("keyboard_1")->read();
	if (!BIT(offset, 2)) result &= ioport("keyboard_2")->read();
	if (!BIT(offset, 3)) result &= ioport("keyboard_3")->read();
	if (!BIT(offset, 4)) result &= ioport("keyboard_4")->read();
	if (!BIT(offset, 5)) result &= ioport("keyboard_5")->read();
	if (!BIT(offset, 6)) result &= ioport("keyboard_6")->read();
	if (!BIT(offset, 7)) result &= ioport("keyboard_7")->read();

	// bit 6, cassette input
	result |= ((m_cassette->input()) > 0 ? 1 : 0) << 6;

	// bit 7, field sync
	result |= m_mc6847->fs_r() << 7;

	return result;
}


/***************************************************************************
    I/O LATCH
***************************************************************************/

WRITE8_MEMBER( vtech1_state::vtech1_latch_w )
{
	if (LOG_VTECH1_LATCH)
		logerror("vtech1_latch_w $%02X\n", data);

	// bit 1, SHRG mod (if installed)
	if (m_videoram.bytes() == 0x2000)
	{
		m_mc6847->gm0_w(BIT(data, 1));
		m_mc6847->gm2_w(BIT(data, 1));
	}

	// bit 2, cassette out (actually bits 1 and 2 perform this function, so either can be used)
	m_cassette->output( BIT(data, 2) ? -1.0 : +1.0);

	// bit 3 and 4, vdc mode control lines
	m_mc6847->ag_w(BIT(data, 3));
	m_mc6847->css_w(BIT(data, 4));

	// bit 0 and 5, speaker
	m_speaker->level_w((BIT(data, 5) << 1) | BIT(data, 0));
}


/***************************************************************************
    MEMORY BANKING
***************************************************************************/

WRITE8_MEMBER( vtech1_state::vtech1_video_bank_w )
{
	membank("bank4")->set_entry(data & 0x03);
}


/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

READ8_MEMBER( vtech1_state::mc6847_videoram_r )
{
	if (offset == ~0) return 0xff;

	m_mc6847->inv_w(BIT(m_videoram[offset], 6));
	m_mc6847->as_w(BIT(m_videoram[offset], 7));

	return m_videoram[offset];
}


/***************************************************************************
    DRIVER INIT
***************************************************************************/

DRIVER_INIT_MEMBER( vtech1_state, vtech1 )
{
	// setup expansion slots
	m_ioexp->set_io_space(&m_maincpu->space(AS_IO));
	m_memexp->set_program_space(&m_maincpu->space(AS_PROGRAM));
	m_memexp->set_io_space(&m_maincpu->space(AS_IO));
}

DRIVER_INIT_MEMBER( vtech1_state, vtech1h )
{
	DRIVER_INIT_CALL(vtech1);

	// the SHRG mod replaces the standard videoram chip with an 8k chip
	m_videoram.allocate(0x2000);

	m_maincpu->space(AS_PROGRAM).install_readwrite_bank(0x7000, 0x77ff, "bank4");
	membank("bank4")->configure_entries(0, 4, m_videoram, 0x800);
	membank("bank4")->set_entry(0);
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( laser110_mem, AS_PROGRAM, 8, vtech1_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM // basic rom
	AM_RANGE(0x6800, 0x6fff) AM_READWRITE(vtech1_keyboard_r, vtech1_latch_w)
	AM_RANGE(0x7000, 0x77ff) AM_RAM AM_SHARE("videoram") // 6847
	AM_RANGE(0x7800, 0x7fff) AM_RAM // 2k user ram
ADDRESS_MAP_END

static ADDRESS_MAP_START( laser210_mem, AS_PROGRAM, 8, vtech1_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM // basic rom
	AM_RANGE(0x6800, 0x6fff) AM_READWRITE(vtech1_keyboard_r, vtech1_latch_w)
	AM_RANGE(0x7000, 0x77ff) AM_RAM AM_SHARE("videoram") // 6847
	AM_RANGE(0x7800, 0x8fff) AM_RAM // 6k user ram
ADDRESS_MAP_END

static ADDRESS_MAP_START( laser310_mem, AS_PROGRAM, 8, vtech1_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM // basic rom
	AM_RANGE(0x6800, 0x6fff) AM_READWRITE(vtech1_keyboard_r, vtech1_latch_w)
	AM_RANGE(0x7000, 0x77ff) AM_RAM AM_SHARE("videoram") // 6847
	AM_RANGE(0x7800, 0xb7ff) AM_RAM // 16k user ram
ADDRESS_MAP_END

static ADDRESS_MAP_START( vtech1_io, AS_IO, 8, vtech1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x4f) AM_READ(vtech1_lightpen_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vtech1_shrg_io, AS_IO, 8, vtech1_state )
	AM_IMPORT_FROM(vtech1_io)
	AM_RANGE(0xd0, 0xdf) AM_WRITE(vtech1_video_bank_w)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START(vtech1)
	PORT_START("keyboard_0")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R       RETURN  LEFT$")   PORT_CODE(KEYCODE_R)     PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q       FOR     CHR$")    PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E       NEXT    LEN(")    PORT_CODE(KEYCODE_E)     PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W       TO      VAL(")    PORT_CODE(KEYCODE_W)     PORT_CHAR('W')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T       THEN    MID$")    PORT_CODE(KEYCODE_T)     PORT_CHAR('T')

	PORT_START("keyboard_1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F       GOSUB   RND(")    PORT_CODE(KEYCODE_F)     PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A       MODE(   ASC(")    PORT_CODE(KEYCODE_A)     PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D       DIM     RESTORE") PORT_CODE(KEYCODE_D)     PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")                    PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S       STEP    STR$(")   PORT_CODE(KEYCODE_S)     PORT_CHAR('S')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G       GOTO    STOP")    PORT_CODE(KEYCODE_G)     PORT_CHAR('G')

	PORT_START("keyboard_2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V       LPRINT  USR")     PORT_CODE(KEYCODE_V)     PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z       PEEK(   INP")     PORT_CODE(KEYCODE_Z)     PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C       CONT    COPY")    PORT_CODE(KEYCODE_C)     PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")                   PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X       POKE    OUT")     PORT_CODE(KEYCODE_X)     PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B       LLIST   SOUND")   PORT_CODE(KEYCODE_B)     PORT_CHAR('B')

	PORT_START("keyboard_3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $    VERIFY  ATN(")    PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !    CSAVE   SIN(")    PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #    CRUN    TAN(")    PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"    CLOAD   COS(")   PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %    LIST    LOG(")    PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("keyboard_4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  \\    \xE2\x86\x90")   PORT_CODE(KEYCODE_M)     PORT_CHAR('M') PORT_CHAR('\\') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE   \xE2\x86\x93")    PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <    \xE2\x86\x92")    PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >    \xE2\x86\x91")    PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  ^    COLOR   USING")   PORT_CODE(KEYCODE_N)     PORT_CHAR('N') PORT_CHAR('^')

	PORT_START("keyboard_5")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '    END     SGN(")    PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  @    DATA    INT(")    PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (    NEW     SQR(")    PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =    [Break]")         PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')  PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )    READ    ABS(")    PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &    RUN     EXP(")    PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("keyboard_6")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U       IF      INKEY$")  PORT_CODE(KEYCODE_U)     PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P  ]    PRINT   NOT")     PORT_CODE(KEYCODE_P)     PORT_CHAR('P') PORT_CHAR(']')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I       INPUT   AND")     PORT_CODE(KEYCODE_I)     PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN  [Function]")      PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  [    LET     OR")      PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('[')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y       ELSE    RIGHT$(") PORT_CODE(KEYCODE_Y)     PORT_CHAR('Y')

	PORT_START("keyboard_7")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J       REM     RESET")   PORT_CODE(KEYCODE_J)     PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +    [Rubout]")        PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')  PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  /    TAB(    POINT")   PORT_CODE(KEYCODE_K)     PORT_CHAR('K') PORT_CHAR('/')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *    [Inverse]")       PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')  PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  ?    [Insert]")        PORT_CODE(KEYCODE_L)     PORT_CHAR('L') PORT_CHAR('?')  PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H       CLS     SET")     PORT_CODE(KEYCODE_H)     PORT_CHAR('H')

	/* Enhanced options not available on real hardware */
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Autorun on Quickload")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x01, DEF_STR(Yes))
//  PORT_CONFNAME( 0x08, 0x08, "Cassette Speaker")
//  PORT_CONFSETTING(    0x08, DEF_STR(On))
//  PORT_CONFSETTING(    0x00, DEF_STR(Off))
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static const INT16 speaker_levels[] = {-32768, 0, 32767, 0};

static MACHINE_CONFIG_START( laser110, vtech1_state )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, VTECH1_CLK)  /* 3.57950 MHz */
	MCFG_CPU_PROGRAM_MAP(laser110_mem)
	MCFG_CPU_IO_MAP(vtech1_io)

	// video hardware
	MCFG_SCREEN_MC6847_PAL_ADD("screen", "mc6847")

	MCFG_DEVICE_ADD("mc6847", MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_FSYNC_CALLBACK(INPUTLINE("maincpu", 0)) MCFG_DEVCB_INVERT
	MCFG_MC6847_INPUT_CALLBACK(READ8(vtech1_state, mc6847_videoram_r))
	MCFG_MC6847_BW(true)
	MCFG_MC6847_FIXED_MODE(MC6847_MODE_GM1)
	// GM2 = GND, GM0 = GND, INTEXT = GND
	// other lines not connected

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	// peripheral and memory expansion slots
	MCFG_IOEXP_SLOT_ADD("io")
	MCFG_MEMEXP_SLOT_ADD("mem")

	// snapshot
	MCFG_SNAPSHOT_ADD("snapshot", vtech1_state, vtech1, "vz", 1.5)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(vtech1_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)
	MCFG_CASSETTE_INTERFACE("vtech1_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list", "vz_cass")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( laser200, laser110 )
	MCFG_DEVICE_REMOVE("mc6847")
	MCFG_DEVICE_ADD("mc6847", MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_FSYNC_CALLBACK(INPUTLINE("maincpu", 0)) MCFG_DEVCB_INVERT
	MCFG_MC6847_INPUT_CALLBACK(READ8(vtech1_state, mc6847_videoram_r))
	MCFG_MC6847_FIXED_MODE(MC6847_MODE_GM1)
	// GM2 = GND, GM0 = GND, INTEXT = GND
	// other lines not connected
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( laser210, laser200 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(laser210_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( laser310, laser200 )
	MCFG_CPU_REPLACE("maincpu", Z80, VZ300_XTAL1_CLK / 5)  /* 3.546894 MHz */
	MCFG_CPU_PROGRAM_MAP(laser310_mem)
	MCFG_CPU_IO_MAP(vtech1_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( laser310h, laser310 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(vtech1_shrg_io)

	MCFG_DEVICE_REMOVE("mc6847")
	MCFG_DEVICE_ADD("mc6847", MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_FSYNC_CALLBACK(INPUTLINE("maincpu", 0)) MCFG_DEVCB_INVERT
	MCFG_MC6847_INPUT_CALLBACK(READ8(vtech1_state, mc6847_videoram_r))
	MCFG_MC6847_FIXED_MODE(MC6847_MODE_GM1)
	// INTEXT = GND
	// other lines not connected
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( laser110 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("vtechv12.u09", 0x0000, 0x2000, CRC(99412d43) SHA1(6aed8872a0818be8e1b08ecdfd92acbe57a3c96d))
	ROM_LOAD("vtechv12.u10", 0x2000, 0x2000, CRC(e4c24e8b) SHA1(9d8fb3d24f3d4175b485cf081a2d5b98158ab2fb))
ROM_END

// The VZ-200 sold in Germany and the Netherlands came with BASIC V1.1, which is currently not dumped.
ROM_START( vz200de )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("vtechv11.u09", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD("vtechv11.u10", 0x2000, 0x2000, NO_DUMP)
ROM_END

#define rom_laser200    rom_laser110
#define rom_fellow      rom_laser110

// It's possible that the Texet TX-8000 came with BASIC V1.0, but this needs to be verified
#define rom_tx8000  rom_laser110

ROM_START( laser210 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("vtechv20.u09", 0x0000, 0x2000, CRC(cc854fe9) SHA1(6e66a309b8e6dc4f5b0b44e1ba5f680467353d66))
	ROM_LOAD("vtechv20.u10", 0x2000, 0x2000, CRC(7060f91a) SHA1(8f3c8f24f97ebb98f3c88d4e4ba1f91ffd563440))
ROM_END

ROM_START( vz200 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "basic20", "BASIC V2.0")
	ROMX_LOAD("vtechv20.u09",  0x0000, 0x2000, CRC(cc854fe9) SHA1(6e66a309b8e6dc4f5b0b44e1ba5f680467353d66), ROM_BIOS(1))
	ROMX_LOAD("vtechv20.u10",  0x2000, 0x2000, CRC(7060f91a) SHA1(8f3c8f24f97ebb98f3c88d4e4ba1f91ffd563440), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "enhanced", "VZ-200 Enhanced BASIC V1.01")
	ROMX_LOAD("vz200_v101.u9", 0x0000, 0x2000, CRC(70340b97) SHA1(eb3f3c8cf0cfa7acd646e89a90a3edf9e556cab6), ROM_BIOS(2))
	ROMX_LOAD("vtechv20.u10",  0x2000, 0x2000, CRC(7060f91a) SHA1(8f3c8f24f97ebb98f3c88d4e4ba1f91ffd563440), ROM_BIOS(2))
ROM_END

ROM_START( laser310 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "basic20", "BASIC V2.0")
	ROMX_LOAD("vtechv20.u12", 0x0000, 0x4000, CRC(613de12c) SHA1(f216c266bc09b0dbdbad720796e5ea9bc7d91e53), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "basic21", "BASIC V2.1 (hack)")
	ROMX_LOAD("vtechv21.u12", 0x0000, 0x4000, CRC(f7df980f) SHA1(5ba14a7a2eedca331b033901080fa5d205e245ea), ROM_BIOS(2))
ROM_END

#define rom_vz300       rom_laser310
#define rom_laser310h   rom_laser310


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT   INIT                   COMPANY                   FULLNAME                          FLAGS
COMP( 1983, laser110,  0,        0,      laser110,  vtech1, vtech1_state, vtech1,  "Video Technology",       "Laser 110",                      0 )
COMP( 1983, laser200,  0,        0,      laser200,  vtech1, vtech1_state, vtech1,  "Video Technology",       "Laser 200",                      0 )
COMP( 1983, vz200de,   laser200, 0,      laser200,  vtech1, vtech1_state, vtech1,  "Video Technology",       "VZ-200 (Germany & Netherlands)", MACHINE_NOT_WORKING )
COMP( 1983, fellow,    laser200, 0,      laser200,  vtech1, vtech1_state, vtech1,  "Salora",                 "Fellow (Finland)",               0 )
COMP( 1983, tx8000,    laser200, 0,      laser200,  vtech1, vtech1_state, vtech1,  "Texet",                  "TX-8000 (UK)",                   0 )
COMP( 1984, laser210,  0,        0,      laser210,  vtech1, vtech1_state, vtech1,  "Video Technology",       "Laser 210",                      0 )
COMP( 1984, vz200,     laser210, 0,      laser210,  vtech1, vtech1_state, vtech1,  "Dick Smith Electronics", "VZ-200 (Oceania)",               0 )
COMP( 1984, laser310,  0,        0,      laser310,  vtech1, vtech1_state, vtech1,  "Video Technology",       "Laser 310",                      0 )
COMP( 1984, vz300,     laser310, 0,      laser310,  vtech1, vtech1_state, vtech1,  "Dick Smith Electronics", "VZ-300 (Oceania)",               0 )
COMP( 1984, laser310h, laser310, 0,      laser310h, vtech1, vtech1_state, vtech1h, "Video Technology",       "Laser 310 (SHRG)",               MACHINE_UNOFFICIAL)
