// license:BSD
// copyright-holders:Robbbert
/***************************************************************************

Goldstar Famicom FC-100

2014/04/20 Skeleton driver.

TODO:
- Everything

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6847.h"


class fc100_state : public driver_device
{
public:
	fc100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s68047p(*this, "s68047p")
		, m_videoram(*this, "videoram")
	{ }

	DECLARE_READ8_MEMBER( mc6847_videoram_r );

	UINT8 *m_p_chargen;
	static UINT8 get_char_rom(running_machine &machine, UINT8 ch, int line)
	{
		fc100_state *state = machine.driver_data<fc100_state>();
		return state->m_p_chargen[(ch&0x7F)*16+line];
	}
private:
	virtual void machine_start();
	virtual void machine_reset();

	required_device<cpu_device> m_maincpu;
	required_device<s68047_device> m_s68047p;
	required_shared_ptr<UINT8> m_videoram;

	// graphics signals
	UINT8 m_ag;
	UINT8 m_gm2;
	UINT8 m_gm1;
	UINT8 m_gm0;
	UINT8 m_as;
	UINT8 m_css;
	UINT8 m_intext;
	UINT8 m_inv;
};


static ADDRESS_MAP_START( fc100_mem, AS_PROGRAM, 8, fc100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM AM_REGION("roms", 0)
	AM_RANGE( 0x6000, 0xBFFF ) AM_RAM
	AM_RANGE( 0xc000, 0xffff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( fc100_io, AS_IO, 8, fc100_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( fc100 )
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("Left 1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Left 4") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("Left 7") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left *") // Guess
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Right 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Right 4") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Right 7") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Right *") // Guess

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("Left 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("Left 5") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("Left 8") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("Left 0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Right 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Right 5") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Right 8") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Right 0")

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("Left 3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("Left 6") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("Left 9") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("Left #") // Guess
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Right 3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Right 6") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Right 9") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Right #") // Guess

	PORT_START("JOY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left Right")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left Left")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_NAME("Left Down")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_NAME("Left Up")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right Right")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("Right Left")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("Right Down")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("Right Up")
INPUT_PORTS_END


void fc100_state::machine_start()
{
	m_ag = 0;
	m_gm2 = 0;
	m_gm1 = 0;
	m_gm0 = 0;
	m_as = 0;
	m_css = 0;
	m_intext = 0;
	m_inv = 0;

	save_item(NAME(m_ag));
	save_item(NAME(m_gm2));
	save_item(NAME(m_gm1));
	save_item(NAME(m_gm0));
	save_item(NAME(m_as));
	save_item(NAME(m_css));
	save_item(NAME(m_intext));
	save_item(NAME(m_inv));
}


void fc100_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
}

#if 0
WRITE8_MEMBER( fc100_state::ay_port_a_w )
{
	//logerror("ay_port_a_w: %02X\n", data);

	// Lacking schematics, these are all wild guesses
	// Having bit 1 set makes black display as blue??
	m_ag = BIT(data, 4);
	m_gm2 = BIT(data, 6);
	m_gm1 = BIT(data, 3);
	m_gm0 = BIT(data, 3);
	m_css = m_ag;

	m_s68047p->ag_w( m_ag ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->gm2_w( m_gm2 ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->gm1_w( m_gm1 ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->gm0_w( m_gm0 ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->css_w( m_css ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->hack_black_becomes_blue( BIT(data, 1) );
}


WRITE8_MEMBER( fc100_state::ay_port_b_w )
{
	//logerror("ay_port_b_w: %02X\n", data);
}


static const ay8910_interface fc100_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_DRIVER_MEMBER(fc100_state, ay_port_a_r),
	DEVCB_DRIVER_MEMBER(fc100_state, ay_port_b_r),
	DEVCB_DRIVER_MEMBER(fc100_state, ay_port_a_w),
	DEVCB_DRIVER_MEMBER(fc100_state, ay_port_b_w)
};
#endif

READ8_MEMBER( fc100_state::mc6847_videoram_r )
{
	if (offset == ~0) return 0xff;

	if ( m_ag )
	{
		if ( m_gm2 )
		{
			// 256 x 192 / 6KB
			offset = ( ( offset & 0x1fc0 ) >> 1 ) | ( offset & 0x1f );
			return m_videoram[offset % 0xc00];
		}
		else
		{
			// 256 x 96 / 3KB
			return m_videoram[offset % 0xc00];
		}
	}

	// Standard text
	UINT8 data = m_videoram[offset];
	UINT8 attr = m_videoram[offset+0x200];

	m_s68047p->inv_w( BIT( attr, 0 ));

	return data;
}


static const mc6847_interface fc100_mc6847_interface =
{
	"screen",
	DEVCB_DRIVER_MEMBER(fc100_state,mc6847_videoram_r),   // data fetch

	DEVCB_NULL,                 /* AG */
	DEVCB_NULL,                 /* GM2 */
	DEVCB_NULL,                 /* GM1 */
	DEVCB_NULL,                 /* GM0 */
	DEVCB_NULL,                 /* CSS */
	DEVCB_NULL,                 /* AS */
	DEVCB_LINE_VCC,             /* INTEXT */
	DEVCB_NULL,                 /* INV */

	&fc100_state::get_char_rom
};


static MACHINE_CONFIG_START( fc100, fc100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, 9159090/2)
	MCFG_CPU_PROGRAM_MAP(fc100_mem)
	MCFG_CPU_IO_MAP(fc100_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fc100_state,  irq0_line_hold)

	/* video hardware */
	MCFG_MC6847_ADD("s68047p", S68047, 9159090/3, fc100_mc6847_interface )  // Clock not verified
	MCFG_SCREEN_MC6847_NTSC_ADD("screen", "s68047p")

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fc100 )
	ROM_REGION( 0x6000, "roms", 0 )
	ROM_LOAD( "08-01.u48", 0x0000, 0x2000, CRC(24e78e75) SHA1(13121706544256a702635448ed2950a75c13f491) )
	ROM_LOAD( "08-02.u49", 0x2000, 0x2000, CRC(e14fc7e9) SHA1(9c5821e65c1efe698e25668d24c36929ea4c3ad7) )
	ROM_LOAD( "06-03.u50", 0x4000, 0x2000, CRC(d783c84e) SHA1(6d1bf53995e08724d5ecc24198cdda4442eb2eb9) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "cgen.u53",  0x0000, 0x1000, CRC(2de75b7f) SHA1(464369d98cbae92ffa322ebaa4404cf5b26825f1) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS          INIT    COMPANY   FULLNAME       STATUS                     FLAGS */
CONS( 1982, fc100,  0,      0,       fc100,   fc100,  driver_device,   0,   "Goldstar", "Famicom FC-100", GAME_IS_SKELETON )
