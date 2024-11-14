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

#include "bus/vtech/ioexp/ioexp.h"
#include "bus/vtech/memexp/memexp.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "sound/spkrdev.h"
#include "video/mc6847.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/vt_cas.h"
#include "multibyte.h"

#define LOG_VTECH1_LATCH (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

/***************************************************************************
    CONSTANTS & MACROS
***************************************************************************/

#define VTECH1_CLK        3579500
#define VZ300_XTAL1_CLK   XTAL(17'734'470)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class vtech1_base_state : public driver_device
{
public:
	vtech1_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_vbank(*this, "vbank")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_ioexp(*this, "io")
		, m_memexp(*this, "mem")
	{
	}

	void vtech1(machine_config &config);

	void laser110_mem(address_map &map) ATTR_COLD;
	void laser210_mem(address_map &map) ATTR_COLD;
	void laser310_mem(address_map &map) ATTR_COLD;
	void vtech1_io(address_map &map) ATTR_COLD;

protected:
	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_crtc;
	memory_bank_creator m_vbank;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<8> m_io_keyboard;
	required_device<vtech_ioexp_slot_device> m_ioexp;
	required_device<vtech_memexp_slot_device> m_memexp;

	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	uint8_t keyboard_r(offs_t offset);
	virtual void latch_w(uint8_t data);
	uint8_t vram_r(memory_share_creator<uint8_t> &vram, offs_t offset);

	static const uint8_t VZ_BASIC = 0xf0;
	static const uint8_t VZ_MCODE = 0xf1;
};

class vtech1_state : public vtech1_base_state
{
public:
	vtech1_state(const machine_config &mconfig, device_type type, const char *tag)
		: vtech1_base_state(mconfig, type, tag)
		, m_vram(*this, "videoram", 0x800, ENDIANNESS_LITTLE)
	{ }

	void laser310(machine_config &config);
	void laser200(machine_config &config);
	void laser110(machine_config &config);
	void laser210(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_vram;

	uint8_t mc6847_videoram_r(offs_t offset);
};

class laser310h_state : public vtech1_base_state
{
public:
	laser310h_state(const machine_config &mconfig, device_type type, const char *tag)
		: vtech1_base_state(mconfig, type, tag)
		, m_vram(*this, "videoram", 0x2000, ENDIANNESS_LITTLE)
	{ }

	void laser310h(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_vram;

	void latch_w(uint8_t data) override;
	void video_bank_w(uint8_t data);
	uint8_t mc6847_videoram_r(offs_t offset);

	void vtech1_shrg_mem(address_map &map) ATTR_COLD;
	void vtech1_shrg_io(address_map &map) ATTR_COLD;
};


/***************************************************************************
    SNAPSHOT LOADING
***************************************************************************/

SNAPSHOT_LOAD_MEMBER(vtech1_base_state::snapshot_cb)
{
	// get the header
	uint8_t header[24];
	if (image.fread(&header, sizeof(header)) != sizeof(header))
	{
		return std::make_pair(image_error::UNSPECIFIED, std::string());
	}

	// get image name
	char pgmname[17];
	for (int i = 0; i < 16; i++)
		pgmname[i] = header[i+4];
	pgmname[16] = '\0';

	// get start and end addresses
	uint16_t const start = get_u16le(&header[22]);
	uint16_t const end = start + image.length() - sizeof(header);
	uint16_t const size = end - start;

	// write it to RAM
	auto buf = std::make_unique<uint8_t []>(size);
	if (image.fread(buf.get(), size) != size)
	{
		return std::make_pair(image_error::UNSPECIFIED, std::string());
	}
	uint8_t *ptr = &buf[0];

	// check for supported format before overwriting memory
	switch (header[21])
	{
	case VZ_BASIC:
	case VZ_MCODE:
		break;

	default:
		return std::make_pair(image_error::UNSUPPORTED, "Snapshot format not supported");
	}

	address_space &space = m_maincpu->space(AS_PROGRAM);
	for (uint16_t addr = start; addr < end; addr++, ptr++)
	{
		uint8_t to_write = *ptr;
		space.write_byte(addr, to_write);

		// verify
		if (space.read_byte(addr) != to_write)
		{
			return std::make_pair(
					image_error::INVALIDIMAGE,
					util::string_format("Insufficient RAM to load snapshot program '%s' (%d bytes needed)", pgmname, size));
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
	}

	return std::make_pair(std::error_condition(), std::string());
}


/***************************************************************************
    INPUTS
***************************************************************************/

uint8_t vtech1_base_state::keyboard_r(offs_t offset)
{
	uint8_t result = 0x3f;

	// bit 0 to 5, keyboard input
	for (u8 i = 0; i < 8; i++)
		if (!BIT(offset, i)) result &= m_io_keyboard[i]->read();

	// bit 6, cassette input
	result |= (m_cassette->input() > 0.04) ? 0 : 0x40;

	// bit 7, field sync
	result |= m_crtc->fs_r() << 7;

	return result;
}


/***************************************************************************
    I/O LATCH
***************************************************************************/

void vtech1_base_state::latch_w(uint8_t data)
{
	LOGMASKED(LOG_VTECH1_LATCH, "vtech1_latch_w $%02X\n", data);

	// bit 2, cassette out (actually bits 1 and 2 perform this function, so either can be used)
	m_cassette->output( BIT(data, 2) ? 1.0 : -1.0);

	// bit 3 and 4, vdc mode control lines
	m_crtc->ag_w(BIT(data, 3));
	m_crtc->css_w(BIT(data, 4));

	// bit 0 and 5, speaker
	m_speaker->level_w((BIT(data, 5) << 1) | BIT(data, 0));
}

void laser310h_state::latch_w(uint8_t data)
{
	vtech1_base_state::latch_w(data);

	m_crtc->gm0_w(BIT(data, 1));
	m_crtc->gm2_w(BIT(data, 1));
}


/***************************************************************************
    MEMORY BANKING
***************************************************************************/

void laser310h_state::video_bank_w(uint8_t data)
{
	m_vbank->set_entry(data & 0x03);
}


/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

uint8_t vtech1_base_state::vram_r(memory_share_creator<uint8_t> &vram, offs_t offset)
{
	if (offset == ~0) return 0xff;

	m_crtc->inv_w(BIT(vram[offset], 6));
	m_crtc->as_w(BIT(vram[offset], 7));

	return vram[offset];
}

uint8_t vtech1_state::mc6847_videoram_r(offs_t offset)
{
	return vram_r(m_vram, offset);
}

uint8_t laser310h_state::mc6847_videoram_r(offs_t offset)
{
	return vram_r(m_vram, offset);
}


/***************************************************************************
    DRIVER INIT
***************************************************************************/

void vtech1_state::machine_start()
{
	m_vbank->configure_entries(0, 1, m_vram, 0x800);
	m_vbank->set_entry(0);
}

void laser310h_state::machine_start()
{
	// the SHRG mod replaces the standard videoram chip with an 8k chip
	m_vbank->configure_entries(0, 4, m_vram, 0x800);
	m_vbank->set_entry(0);
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void vtech1_base_state::laser110_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom(); // basic rom
	map(0x4000, 0x67ff).noprw(); // cartridge space
	map(0x6800, 0x6fff).rw(FUNC(vtech1_base_state::keyboard_r), FUNC(vtech1_base_state::latch_w));
	map(0x7000, 0x77ff).bankrw("vbank");
	map(0x7800, 0x7fff).ram(); // 2k user ram
	map(0x8000, 0xffff).noprw(); // expansion ram
}

void vtech1_base_state::laser210_mem(address_map &map)
{
	laser110_mem(map);
	map(0x7800, 0x8fff).ram(); // 6k user ram
}

void vtech1_base_state::laser310_mem(address_map &map)
{
	laser110_mem(map);
	map(0x7800, 0xb7ff).ram(); // 16k user ram
}

void vtech1_base_state::vtech1_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0xff).noprw(); // completely handled by expansion devices
}

void laser310h_state::vtech1_shrg_mem(address_map &map)
{
	laser310_mem(map);
	map(0x6800, 0x6fff).w(FUNC(laser310h_state::latch_w));
}

void laser310h_state::vtech1_shrg_io(address_map &map)
{
	map.global_mask(0xff);
	vtech1_io(map);
	map(0xd0, 0xdf).w(FUNC(laser310h_state::video_bank_w));
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START(vtech1)
	PORT_START("X0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R       RETURN  LEFT$")   PORT_CODE(KEYCODE_R)     PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q       FOR     CHR$")    PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E       NEXT    LEN(")    PORT_CODE(KEYCODE_E)     PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W       TO      VAL(")    PORT_CODE(KEYCODE_W)     PORT_CHAR('W')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T       THEN    MID$")    PORT_CODE(KEYCODE_T)     PORT_CHAR('T')

	PORT_START("X1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F       GOSUB   RND(")    PORT_CODE(KEYCODE_F)     PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A       MODE(   ASC(")    PORT_CODE(KEYCODE_A)     PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D       DIM     RESTORE") PORT_CODE(KEYCODE_D)     PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")                    PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S       STEP    STR$(")   PORT_CODE(KEYCODE_S)     PORT_CHAR('S')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G       GOTO    STOP")    PORT_CODE(KEYCODE_G)     PORT_CHAR('G')

	PORT_START("X2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V       LPRINT  USR")     PORT_CODE(KEYCODE_V)     PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z       PEEK(   INP")     PORT_CODE(KEYCODE_Z)     PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C       CONT    COPY")    PORT_CODE(KEYCODE_C)     PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")                   PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X       POKE    OUT")     PORT_CODE(KEYCODE_X)     PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B       LLIST   SOUND")   PORT_CODE(KEYCODE_B)     PORT_CHAR('B')

	PORT_START("X3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $    VERIFY  ATN(")    PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !    CSAVE   SIN(")    PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #    CRUN    TAN(")    PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"    CLOAD   COS(")   PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %    LIST    LOG(")    PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("X4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  \\    \xE2\x86\x90")   PORT_CODE(KEYCODE_M)     PORT_CHAR('M') PORT_CHAR('\\') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE   \xE2\x86\x93")    PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR('~')  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <    \xE2\x86\x92")    PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >    \xE2\x86\x91")    PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  ^    COLOR   USING")   PORT_CODE(KEYCODE_N)     PORT_CHAR('N') PORT_CHAR('^')

	PORT_START("X5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '    END     SGN(")    PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  @    DATA    INT(")    PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (    NEW     SQR(")    PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =    [Break]")         PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')  PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )    READ    ABS(")    PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &    RUN     EXP(")    PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("X6")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U       IF      INKEY$")  PORT_CODE(KEYCODE_U)     PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P  ]    PRINT   NOT")     PORT_CODE(KEYCODE_P)     PORT_CHAR('P') PORT_CHAR(']')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I       INPUT   AND")     PORT_CODE(KEYCODE_I)     PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN  [Function]")      PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  [    LET     OR")      PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('[')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y       ELSE    RIGHT$(") PORT_CODE(KEYCODE_Y)     PORT_CHAR('Y')

	PORT_START("X7")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J       REM     RESET")   PORT_CODE(KEYCODE_J)     PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +    [Rubout]")        PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')  PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  /    TAB(    POINT")   PORT_CODE(KEYCODE_K)     PORT_CHAR('K') PORT_CHAR('/')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *    [Inverse]")       PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')  PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  ?    [Insert]")        PORT_CODE(KEYCODE_L)     PORT_CHAR('L') PORT_CHAR('?')  PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H       CLS     SET")     PORT_CODE(KEYCODE_H)     PORT_CHAR('H')
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static const double speaker_levels[] = { 0.0, 1.0, -1.0, 0.0 };

void vtech1_base_state::vtech1(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, VTECH1_CLK);  /* 3.57950 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &vtech1_base_state::laser110_mem);
	m_maincpu->set_addrmap(AS_IO, &vtech1_base_state::vtech1_io);

	// video hardware
	MC6847_PAL(config, m_crtc, XTAL(4'433'619));
	m_crtc->set_screen("screen");
	m_crtc->fsync_wr_callback().set_inputline(m_maincpu, 0).invert();
	m_crtc->set_get_fixed_mode(mc6847_pal_device::MODE_GM1);
	// GM2 = GND, GM0 = GND, INTEXT = GND
	// other lines not connected

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.75);

	// peripheral and memory expansion slots
	VTECH_IOEXP_SLOT(config, m_ioexp);
	m_ioexp->set_iospace(m_maincpu, AS_IO);

	VTECH_MEMEXP_SLOT(config, m_memexp);
	m_memexp->set_memspace(m_maincpu, AS_PROGRAM);
	m_memexp->set_iospace(m_maincpu, AS_IO);

	// snapshot
	snapshot_image_device &snapshot(SNAPSHOT(config, "snapshot", "vz"));
	snapshot.set_delay(attotime::from_double(2.0));
	snapshot.set_load_callback(FUNC(vtech1_base_state::snapshot_cb));
	snapshot.set_interface("vzsnap");

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(vtech1_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("vtech1_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("vz_cass");
	SOFTWARE_LIST(config, "snap_list").set_original("vz_snap");
}

void vtech1_state::laser110(machine_config &config)
{
	vtech1(config);

	m_crtc->input_callback().set(FUNC(vtech1_state::mc6847_videoram_r));
	m_crtc->set_black_and_white(true);
}

void vtech1_state::laser200(machine_config &config)
{
	laser110(config);

	m_crtc->set_black_and_white(false);
}

void vtech1_state::laser210(machine_config &config)
{
	laser200(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &vtech1_base_state::laser210_mem);
}

void vtech1_state::laser310(machine_config &config)
{
	laser200(config);

	m_maincpu->set_clock(VZ300_XTAL1_CLK / 5);  /* 3.546894 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &vtech1_base_state::laser310_mem);
}

void laser310h_state::laser310h(machine_config &config)
{
	vtech1(config);

	m_maincpu->set_clock(VZ300_XTAL1_CLK / 5);  /* 3.546894 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &laser310h_state::vtech1_shrg_mem);
	m_maincpu->set_addrmap(AS_IO, &laser310h_state::vtech1_shrg_io);

	m_crtc->input_callback().set(FUNC(laser310h_state::mc6847_videoram_r));
	m_crtc->set_get_fixed_mode(mc6847_pal_device::MODE_GM1);
	// INTEXT = GND
	// other lines not connected
}


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
	ROMX_LOAD("vtechv20.u09",  0x0000, 0x2000, CRC(cc854fe9) SHA1(6e66a309b8e6dc4f5b0b44e1ba5f680467353d66), ROM_BIOS(0))
	ROMX_LOAD("vtechv20.u10",  0x2000, 0x2000, CRC(7060f91a) SHA1(8f3c8f24f97ebb98f3c88d4e4ba1f91ffd563440), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "enhanced", "VZ-200 Enhanced BASIC V1.01")
	ROMX_LOAD("vz200_v101.u9", 0x0000, 0x2000, CRC(70340b97) SHA1(eb3f3c8cf0cfa7acd646e89a90a3edf9e556cab6), ROM_BIOS(1))
	ROMX_LOAD("vtechv20.u10",  0x2000, 0x2000, CRC(7060f91a) SHA1(8f3c8f24f97ebb98f3c88d4e4ba1f91ffd563440), ROM_BIOS(1))
ROM_END

ROM_START( laser310 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "basic20", "BASIC V2.0")
	ROMX_LOAD("vtechv20.u12", 0x0000, 0x4000, CRC(613de12c) SHA1(f216c266bc09b0dbdbad720796e5ea9bc7d91e53), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "basic21", "BASIC V2.1 (hack)")
	ROMX_LOAD("vtechv21.u12", 0x0000, 0x4000, CRC(f7df980f) SHA1(5ba14a7a2eedca331b033901080fa5d205e245ea), ROM_BIOS(1))
ROM_END

#define rom_vz300       rom_laser310
#define rom_laser310h   rom_laser310

} // anonymous namespace


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT   CLASS            INIT        COMPANY                   FULLNAME                          FLAGS
COMP( 1983, laser110,  0,        0,      laser110,  vtech1, vtech1_state,    empty_init, "Video Technology",       "Laser 110",                      MACHINE_SUPPORTS_SAVE )
COMP( 1983, laser200,  0,        0,      laser200,  vtech1, vtech1_state,    empty_init, "Video Technology",       "Laser 200",                      MACHINE_SUPPORTS_SAVE )
COMP( 1983, vz200de,   laser200, 0,      laser200,  vtech1, vtech1_state,    empty_init, "Video Technology",       "VZ-200 (Germany & Netherlands)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1983, fellow,    laser200, 0,      laser200,  vtech1, vtech1_state,    empty_init, "Salora",                 "Fellow (Finland)",               MACHINE_SUPPORTS_SAVE )
COMP( 1983, tx8000,    laser200, 0,      laser200,  vtech1, vtech1_state,    empty_init, "Texet",                  "TX-8000 (UK)",                   MACHINE_SUPPORTS_SAVE )
COMP( 1984, laser210,  0,        0,      laser210,  vtech1, vtech1_state,    empty_init, "Video Technology",       "Laser 210",                      MACHINE_SUPPORTS_SAVE )
COMP( 1984, vz200,     laser210, 0,      laser210,  vtech1, vtech1_state,    empty_init, "Dick Smith Electronics", "VZ-200 (Oceania)",               MACHINE_SUPPORTS_SAVE )
COMP( 1984, laser310,  0,        0,      laser310,  vtech1, vtech1_state,    empty_init, "Video Technology",       "Laser 310",                      MACHINE_SUPPORTS_SAVE )
COMP( 1984, vz300,     laser310, 0,      laser310,  vtech1, vtech1_state,    empty_init, "Dick Smith Electronics", "VZ-300 (Oceania)",               MACHINE_SUPPORTS_SAVE )
COMP( 1984, laser310h, laser310, 0,      laser310h, vtech1, laser310h_state, empty_init, "Video Technology",       "Laser 310 (SHRG)",               MACHINE_UNOFFICIAL | MACHINE_SUPPORTS_SAVE )
