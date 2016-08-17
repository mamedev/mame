// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        PK-8000

        18/07/2009 Mostly working driver
        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "imagedev/cassette.h"
#include "formats/fmsx_cas.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "includes/pk8000.h"
#include "machine/ram.h"


class pk8000_state : public pk8000_base_state
{
public:
	pk8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: pk8000_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_speaker(*this, "speaker")
		, m_region_maincpu(*this, "maincpu")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_bank5(*this, "bank5")
		, m_bank6(*this, "bank6")
		, m_bank7(*this, "bank7")
		, m_bank8(*this, "bank8")
		, m_io_joy1(*this, "JOY1")
		, m_io_joy2(*this, "JOY2")
	{ }

	UINT8 m_keyboard_line;
	DECLARE_READ8_MEMBER(pk8000_joy_1_r);
	DECLARE_READ8_MEMBER(pk8000_joy_2_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_pk8000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pk8000_interrupt);
	DECLARE_WRITE8_MEMBER(pk8000_80_porta_w);
	DECLARE_READ8_MEMBER(pk8000_80_portb_r);
	DECLARE_WRITE8_MEMBER(pk8000_80_portc_w);

	IRQ_CALLBACK_MEMBER(pk8000_irq_callback);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_bank m_bank5;
	required_memory_bank m_bank6;
	required_memory_bank m_bank7;
	required_memory_bank m_bank8;
	required_ioport m_io_joy1;
	required_ioport m_io_joy2;
	ioport_port *m_io_port[10];

	void pk8000_set_bank(UINT8 data);
};



void pk8000_state::pk8000_set_bank(UINT8 data)
{
	UINT8 *rom = m_region_maincpu->base();
	UINT8 *ram = m_ram->pointer();
	UINT8 block1 = data & 3;
	UINT8 block2 = (data >> 2) & 3;
	UINT8 block3 = (data >> 4) & 3;
	UINT8 block4 = (data >> 6) & 3;

	switch(block1) {
		case 0:
				m_bank1->set_base(rom + 0x10000);
				m_bank5->set_base(ram);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_bank1->set_base(ram);
				m_bank5->set_base(ram);
				break;
	}

	switch(block2) {
		case 0:
				m_bank2->set_base(rom + 0x14000);
				m_bank6->set_base(ram + 0x4000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_bank2->set_base(ram + 0x4000);
				m_bank6->set_base(ram + 0x4000);
				break;
	}
	switch(block3) {
		case 0:
				m_bank3->set_base(rom + 0x18000);
				m_bank7->set_base(ram + 0x8000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_bank3->set_base(ram + 0x8000);
				m_bank7->set_base(ram + 0x8000);
				break;
	}
	switch(block4) {
		case 0:
				m_bank4->set_base(rom + 0x1c000);
				m_bank8->set_base(ram + 0xc000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				m_bank4->set_base(ram + 0xc000);
				m_bank8->set_base(ram + 0xc000);
				break;
	}
}
WRITE8_MEMBER(pk8000_state::pk8000_80_porta_w)
{
	pk8000_set_bank(data);
}

READ8_MEMBER(pk8000_state::pk8000_80_portb_r)
{
	if(m_keyboard_line>9) {
		return 0xff;
	}
	return m_io_port[m_keyboard_line]->read();
}

WRITE8_MEMBER(pk8000_state::pk8000_80_portc_w)
{
	m_keyboard_line = data & 0x0f;

	m_speaker->level_w(BIT(data, 7));

	m_cassette->change_state((BIT(data, 4)) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_cassette->output((BIT(data, 6)) ? +1.0 : 0.0);
}

READ8_MEMBER(pk8000_state::pk8000_joy_1_r)
{
	UINT8 retVal = (m_cassette->input() > 0.0038 ? 0x80 : 0);
	retVal |= m_io_joy1->read() & 0x7f;
	return retVal;
}
READ8_MEMBER(pk8000_state::pk8000_joy_2_r)
{
	UINT8 retVal = (m_cassette->input() > 0.0038 ? 0x80 : 0);
	retVal |= m_io_joy2->read() & 0x7f;
	return retVal;
}

static ADDRESS_MAP_START(pk8000_mem, AS_PROGRAM, 8, pk8000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bank1") AM_WRITE_BANK("bank5")
	AM_RANGE( 0x4000, 0x7fff ) AM_READ_BANK("bank2") AM_WRITE_BANK("bank6")
	AM_RANGE( 0x8000, 0xbfff ) AM_READ_BANK("bank3") AM_WRITE_BANK("bank7")
	AM_RANGE( 0xc000, 0xffff ) AM_READ_BANK("bank4") AM_WRITE_BANK("bank8")
ADDRESS_MAP_END

static ADDRESS_MAP_START( pk8000_io , AS_IO, 8, pk8000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
	AM_RANGE(0x88, 0x88) AM_READWRITE(pk8000_video_color_r,pk8000_video_color_w)
	AM_RANGE(0x8c, 0x8c) AM_READ(pk8000_joy_1_r)
	AM_RANGE(0x8d, 0x8d) AM_READ(pk8000_joy_2_r)
	AM_RANGE(0x90, 0x90) AM_READWRITE(pk8000_text_start_r,pk8000_text_start_w)
	AM_RANGE(0x91, 0x91) AM_READWRITE(pk8000_chargen_start_r,pk8000_chargen_start_w)
	AM_RANGE(0x92, 0x92) AM_READWRITE(pk8000_video_start_r,pk8000_video_start_w)
	AM_RANGE(0x93, 0x93) AM_READWRITE(pk8000_color_start_r,pk8000_color_start_w)
	AM_RANGE(0xa0, 0xbf) AM_READWRITE(pk8000_color_r,pk8000_color_w)
ADDRESS_MAP_END

/*   Input ports */
static INPUT_PORTS_START( pk8000 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH)
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("|") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rg") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Upr") PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graf") PORT_CODE(KEYCODE_F10)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alf") PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("???") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_F12)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("VSh") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sel") PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_START("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Probel") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cls") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_START("LINE9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up-Left") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down-Right") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Menu") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Begin") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End page") PORT_CODE(KEYCODE_PGDN)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Begin page") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("JOY1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("JOY2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(pk8000_state::pk8000_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

IRQ_CALLBACK_MEMBER(pk8000_state::pk8000_irq_callback)
{
	return 0xff;
}


void pk8000_state::machine_start()
{
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8", "LINE9" };

	for ( int i = 0; i < 10; i++ )
	{
		m_io_port[i] = ioport(keynames[i]);
	}
}

void pk8000_state::machine_reset()
{
	pk8000_set_bank(0);
}

void pk8000_state::video_start()
{
}

UINT32 pk8000_state::screen_update_pk8000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return pk8000_video_update(screen, bitmap, cliprect, m_ram->pointer());
}

static MACHINE_CONFIG_START( pk8000, pk8000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, 1780000)
	MCFG_CPU_PROGRAM_MAP(pk8000_mem)
	MCFG_CPU_IO_MAP(pk8000_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pk8000_state,  pk8000_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(pk8000_state,pk8000_irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256+32, 192+32)
	MCFG_SCREEN_VISIBLE_AREA(0, 256+32-1, 0, 192+32-1)
	MCFG_SCREEN_UPDATE_DRIVER(pk8000_state, screen_update_pk8000)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(pk8000_base_state, pk8000)

	MCFG_DEVICE_ADD("ppi8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pk8000_state, pk8000_80_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(pk8000_state, pk8000_80_portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pk8000_state, pk8000_80_portc_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(pk8000_base_state, pk8000_84_porta_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pk8000_base_state, pk8000_84_porta_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pk8000_base_state,pk8000_84_portc_w))

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(fmsx_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vesta )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vesta.rom", 0x10000, 0x4000, CRC(fbf7e2cc) SHA1(4bc5873066124bd926c3c6aa2fd1a062c87af339))
ROM_END

ROM_START( hobby )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "hobby.rom", 0x10000, 0x4000, CRC(a25b4b2c) SHA1(0d86e6e4be8d1aa389bfa9dd79e3604a356729f7))
ROM_END

ROM_START( pk8002 )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pk8002.rom", 0x10000, 0x4000, CRC(07b9ae71) SHA1(2137a41cc095c7aba58b7b109fce63f30a4568b2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1987, vesta,  0,       0,     pk8000,     pk8000, driver_device,   0,   "BP EVM",   "PK8000 Vesta",       0)
COMP( 1987, hobby,  vesta,   0,     pk8000,     pk8000, driver_device,   0,   "BP EVM",   "PK8000 Sura/Hobby",  0)
COMP( 1987, pk8002, vesta,   0,     pk8000,     pk8000, driver_device,   0,   "<unknown>",   "PK8002 Elf",  MACHINE_NOT_WORKING)
