/***************************************************************************

        Samsung SPC-1000 driver by Miodrag Milanovic

        10/05/2009 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6847.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"


class spc1000_state : public driver_device
{
public:
	spc1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_vdg(*this, "mc6847") {}

	required_device<mc6847_base_device> m_vdg;
	UINT8 m_IPLK;
	UINT8 m_GMODE;
	UINT8 m_video_ram[0x2000];

	static UINT8 get_char_rom(running_machine &machine, UINT8 ch, int line)
	{
		spc1000_state *state = machine.driver_data<spc1000_state>();
		return state->m_video_ram[0x1000+(ch&0x7F)*16+line];
	}
	DECLARE_WRITE8_MEMBER(spc1000_iplk_w);
	DECLARE_READ8_MEMBER(spc1000_iplk_r);
	DECLARE_WRITE8_MEMBER(spc1000_video_ram_w);
	DECLARE_READ8_MEMBER(spc1000_video_ram_r);
	DECLARE_READ8_MEMBER(spc1000_keyboard_r);
	virtual void machine_reset();
	DECLARE_WRITE8_MEMBER(spc1000_gmode_w);
	DECLARE_READ8_MEMBER(spc1000_gmode_r);
	DECLARE_READ8_MEMBER(spc1000_mc6847_videoram_r);
};



static ADDRESS_MAP_START(spc1000_mem, AS_PROGRAM, 8, spc1000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x7fff ) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE( 0x8000, 0xffff ) AM_READ_BANK("bank3") AM_WRITE_BANK("bank4")
ADDRESS_MAP_END

WRITE8_MEMBER(spc1000_state::spc1000_iplk_w)
{
	m_IPLK = m_IPLK ? 0 : 1;
	if (m_IPLK == 1) {
		UINT8 *mem = memregion("maincpu")->base();
		membank("bank1")->set_base(mem);
		membank("bank3")->set_base(mem);
	} else {
		UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();
		membank("bank1")->set_base(ram);
		membank("bank3")->set_base(ram + 0x8000);
	}
}

READ8_MEMBER(spc1000_state::spc1000_iplk_r)
{
	m_IPLK = m_IPLK ? 0 : 1;
	if (m_IPLK == 1) {
		UINT8 *mem = memregion("maincpu")->base();
		membank("bank1")->set_base(mem);
		membank("bank3")->set_base(mem);
	} else {
		UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();
		membank("bank1")->set_base(ram);
		membank("bank3")->set_base(ram + 0x8000);
	}
	return 0;
}



WRITE8_MEMBER(spc1000_state::spc1000_video_ram_w)
{
	m_video_ram[offset] = data;
}

READ8_MEMBER(spc1000_state::spc1000_video_ram_r)
{
	return m_video_ram[offset];
}

READ8_MEMBER(spc1000_state::spc1000_keyboard_r){
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4",
		"LINE5", "LINE6", "LINE7", "LINE8", "LINE9"
	};
	return ioport(keynames[offset])->read();
}

WRITE8_MEMBER(spc1000_state::spc1000_gmode_w)
{
	m_GMODE = data;

	// m_GMODE layout: CSS|NA|PS2|PS1|~A/G|GM0|GM1|NA
	//  [PS2,PS1] is used to set screen 0/1 pages
	m_vdg->gm1_w(BIT(data, 1));
	m_vdg->gm0_w(BIT(data, 2));
	m_vdg->ag_w(BIT(data, 3));
	m_vdg->css_w(BIT(data, 7));
}

READ8_MEMBER(spc1000_state::spc1000_gmode_r)
{
	return m_GMODE;
}

static ADDRESS_MAP_START( spc1000_io , AS_IO, 8, spc1000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(spc1000_video_ram_r, spc1000_video_ram_w)
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(spc1000_gmode_r, spc1000_gmode_w)
	AM_RANGE(0x8000, 0x8009) AM_READ(spc1000_keyboard_r)
	AM_RANGE(0xA000, 0xA000) AM_READWRITE(spc1000_iplk_r, spc1000_iplk_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE_LEGACY("ay8910", ay8910_address_w)
	AM_RANGE(0x4001, 0x4001) AM_DEVREADWRITE_LEGACY("ay8910", ay8910_r, ay8910_data_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( spc1000 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_RCONTROL) PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_START("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_START("LINE9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
INPUT_PORTS_END


void spc1000_state::machine_reset()
{
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *mem = memregion("maincpu")->base();
	UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();

	space.install_read_bank(0x0000, 0x7fff, "bank1");
	space.install_read_bank(0x8000, 0xffff, "bank3");

	space.install_write_bank(0x0000, 0x7fff, "bank2");
	space.install_write_bank(0x8000, 0xffff, "bank4");

	membank("bank1")->set_base(mem);
	membank("bank2")->set_base(ram);
	membank("bank3")->set_base(mem);
	membank("bank4")->set_base(ram + 0x8000);

	m_IPLK = 1;
}

READ8_MEMBER(spc1000_state::spc1000_mc6847_videoram_r)
{
	if (offset == ~0) return 0xff;

	// m_GMODE layout: CSS|NA|PS2|PS1|~A/G|GM0|GM1|NA
	//  [PS2,PS1] is used to set screen 0/1 pages
	if ( !BIT(m_GMODE, 3) ) {   // text mode (~A/G set to A)
		unsigned int page = (BIT(m_GMODE, 5) << 1) | BIT(m_GMODE, 4);
		m_vdg->inv_w(BIT(m_video_ram[offset+page*0x200+0x800], 0));
		m_vdg->css_w(BIT(m_video_ram[offset+page*0x200+0x800], 1));
		m_vdg->as_w(BIT(m_video_ram[offset+page*0x200+0x800], 2));
		m_vdg->intext_w(BIT(m_video_ram[offset+page*0x200+0x800], 3));
		return m_video_ram[offset+page*0x200];
	} else {    // graphics mode: uses full 6KB of VRAM
		return m_video_ram[offset];
	}
}

static const ay8910_interface spc1000_ay_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL
};

static const cassette_interface spc1000_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED),
	NULL,
	NULL
};

static const mc6847_interface spc1000_mc6847_intf =
{
	"screen",
	DEVCB_DRIVER_MEMBER(spc1000_state,spc1000_mc6847_videoram_r),   // data fetch
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,                 /* AG */
	DEVCB_LINE_VCC,             /* GM2 */
	DEVCB_NULL,                 /* GM1 */
	DEVCB_NULL,                 /* GM0 */
	DEVCB_NULL,                 /* CSS */
	DEVCB_NULL,                 /* AS */
	DEVCB_NULL,                 /* INTEXT */
	DEVCB_NULL,                 /* INV */

	&spc1000_state::get_char_rom
};

static MACHINE_CONFIG_START( spc1000, spc1000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(spc1000_mem)
	MCFG_CPU_IO_MAP(spc1000_io)


	/* video hardware */
	MCFG_SCREEN_MC6847_NTSC_ADD("screen", "mc6847")
	MCFG_MC6847_ADD("mc6847", MC6847_NTSC, XTAL_3_579545MHz, spc1000_mc6847_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_4MHz / 1)
	MCFG_SOUND_CONFIG(spc1000_ay_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CASSETTE_ADD( CASSETTE_TAG, spc1000_cassette_interface )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( spc1000 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "spcall.rom", 0x0000, 0x8000, CRC(19638fc9) SHA1(489f1baa7aebf3c8c660325fb1fd790d84203284))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1982, spc1000,  0,       0,   spc1000,    spc1000, driver_device,  0,  "Samsung",   "SPC-1000",       GAME_NOT_WORKING)
