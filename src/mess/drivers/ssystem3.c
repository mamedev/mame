// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************

  ssystem3.c

Driver file to handle emulation of the Chess Champion Super System III / Chess
Champion MK III
by PeT mess@utanet.at November 2000, march 2008

Hardware descriptions:
- A 6502 CPU
- A 6522 VIA ($6000?) (PB6 and PB7 are tied)
- 2 x 2114  1kx4 SRAM to provide 1KB of RAM ($0000)
- 2xXXKB ROM (both connected to the same pins!,
  look into mess source mess/messroms/rddil24.c for notes)
  - signetics 7947e c19081e ss-3-lrom
  - signetics 7945e c19082 ss-3-hrom ($d000??)

optional printer (special serial connection)
optional board display (special serial connection)
internal expansion/cartridge port
 (special power saving pack)


todo:
not sure about lcd signs and their assignments
not sure about all buttons and switches
playfield displayment currently based on simulation and on screen
not check for audio yet
convert to new artwork system

needed:
artwork for board display
backup of playfield rom and picture/description of its board
*/

#include "emu.h"

#include "includes/ssystem3.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"


// in my opinion own cpu to display lcd field and to handle own buttons
void ssystem3_state::ssystem3_playfield_getfigure(int x, int y, int *figure, int *black)
{
	int d;
	if (x&1)
	d=m_playfield.u.s.field[y][x/2]&0xf;
	else
	d=m_playfield.u.s.field[y][x/2]>>4;

	*figure=d&7;
	*black=d&8;
}

void ssystem3_state::ssystem3_playfield_reset()
{
	memset(&m_playfield, 0, sizeof(m_playfield));
	m_playfield.signal=FALSE;
	//  m_playfield.on=TRUE; //m_configuration->read()&1;
}

void ssystem3_state::ssystem3_playfield_write(int reset, int signal)
{
	int d=FALSE;

	if (!reset) {
	m_playfield.count=0;
	m_playfield.bit=0;
	m_playfield.started=FALSE;
	m_playfield.signal=signal;
	m_playfield.time=machine().time();
	}
	if (!signal && m_playfield.signal) {
	attotime t=machine().time();
	m_playfield.high_time=t - m_playfield.time;
	m_playfield.time=t;

	//    logerror("%.4x playfield %d lowtime %s hightime %s\n",(int)activecpu_get_pc(), m_playfield.count,
	//       m_playfield.low_time.as_string(7), m_playfield.high_time.as_string(7) );

	if (m_playfield.started) {
		// 0 twice as long low
		// 1 twice as long high
		if (m_playfield.low_time > m_playfield.high_time) d=TRUE;

		m_playfield.data&=~(1<<(m_playfield.bit^7));
		if (d) m_playfield.data|=1<<(m_playfield.bit^7);
		m_playfield.bit++;
		if (m_playfield.bit==8) {
	logerror("%.4x playfield wrote %d %02x\n", (int)m_maincpu->pc(), m_playfield.count, m_playfield.data);
	m_playfield.u.data[m_playfield.count]=m_playfield.data;
	m_playfield.bit=0;
	m_playfield.count=(m_playfield.count+1)%ARRAY_LENGTH(m_playfield.u.data);
	if (m_playfield.count==0) m_playfield.started=FALSE;
		}
	}

	} else if (signal && !m_playfield.signal) {
	attotime t=machine().time();
	m_playfield.low_time= t - m_playfield.time;
	m_playfield.time=t;
	m_playfield.started=TRUE;
	}
	m_playfield.signal=signal;
}

void ssystem3_state::ssystem3_playfield_read(int *on, int *ready)
{
	*on = !(m_configuration->read() & 1);
	//  *on=!m_playfield.on;
	*ready=FALSE;
}

WRITE8_MEMBER(ssystem3_state::ssystem3_via_write_a)
{
	m_porta=data;
	//  logerror("%.4x via port a write %02x\n",(int)activecpu_get_pc(), data);
}

READ8_MEMBER(ssystem3_state::ssystem3_via_read_a)
{
	UINT8 data=0xff;
#if 1 // time switch
	if (!(m_porta&0x10)) data&=m_matrix[0]->read()|0xf1;
	if (!(m_porta&0x20)) data&=m_matrix[1]->read()|0xf1;
	if (!(m_porta&0x40)) data&=m_matrix[2]->read()|0xf1;
	if (!(m_porta&0x80)) data&=m_matrix[3]->read()|0xf1;
#else
	if (!(m_porta&0x10)) data&=m_matrix[0]->read()|0xf0;
	if (!(m_porta&0x20)) data&=m_matrix[1]->read()|0xf0;
	if (!(m_porta&0x40)) data&=m_matrix[2]->read()|0xf0;
	if (!(m_porta&0x80)) data&=m_matrix[3]->read()|0xf0;
#endif
	if (!(m_porta&1)) {
	if (!(m_matrix[0]->read()&1)) data&=~0x10;
	if (!(m_matrix[1]->read()&1)) data&=~0x20;
	if (!(m_matrix[2]->read()&1)) data&=~0x40;
	if (!(m_matrix[3]->read()&1)) data&=~0x80;
	}
	if (!(m_porta&2)) {
	if (!(m_matrix[0]->read()&2)) data&=~0x10;
	if (!(m_matrix[1]->read()&2)) data&=~0x20;
	if (!(m_matrix[2]->read()&2)) data&=~0x40;
	if (!(m_matrix[3]->read()&2)) data&=~0x80;
	}
	if (!(m_porta&4)) {
	if (!(m_matrix[0]->read()&4)) data&=~0x10;
	if (!(m_matrix[1]->read()&4)) data&=~0x20;
	if (!(m_matrix[2]->read()&4)) data&=~0x40;
	if (!(m_matrix[3]->read()&4)) data&=~0x80;
	}
	if (!(m_porta&8)) {
	if (!(m_matrix[0]->read()&8)) data&=~0x10;
	if (!(m_matrix[1]->read()&8)) data&=~0x20;
	if (!(m_matrix[2]->read()&8)) data&=~0x40;
	if (!(m_matrix[3]->read()&8)) data&=~0x80;
	}
	//  logerror("%.4x via port a read %02x\n",(int)activecpu_get_pc(), data);
	return data;
}


/*
  port b
   bit 0: output opt device reset?

    hi speed serial 1 (d7d7 transfers 40 bit $2e)
   bit 1: output data
   bit 2: output clock (hi data is taken)

   bit 3: output opt data read
   bit 4: input low opt data available
   bit 5: input low opt device available


    bit 6: input clocks!?

  port a:
   bit 7: input low x/$37 2
   bit 6: input low x/$37 3
   bit 5: input low x/$37 4 (else 1)

 */
READ8_MEMBER(ssystem3_state::ssystem3_via_read_b)
{
	UINT8 data=0xff;
	int on, ready;
	ssystem3_playfield_read(&on, &ready);
	if (!on) data&=~0x20;
	if (!ready) data&=~0x10;
	return data;
}

WRITE8_MEMBER(ssystem3_state::ssystem3_via_write_b)
{
	ssystem3_playfield_write(data & 1, data & 8);
	ssystem3_lcd_write(data & 4, data & 2);

	// TODO: figure out what this is trying to achieve
	UINT8 d = ssystem3_via_read_b(space, 0, mem_mask) & ~0x40;
	if (data & 0x80) d |= 0x40;
	//  d&=~0x8f;
	m_via6522_0->write_pb0((d >> 0) & 1);
	m_via6522_0->write_pb1((d >> 1) & 1);
	m_via6522_0->write_pb2((d >> 2) & 1);
	m_via6522_0->write_pb3((d >> 3) & 1);
	m_via6522_0->write_pb4((d >> 4) & 1);
	m_via6522_0->write_pb5((d >> 5) & 1);
	m_via6522_0->write_pb6((d >> 6) & 1);
	m_via6522_0->write_pb7((d >> 7) & 1);
}

DRIVER_INIT_MEMBER(ssystem3_state,ssystem3)
{
	ssystem3_playfield_reset();
	ssystem3_lcd_reset();
}

static ADDRESS_MAP_START( ssystem3_map , AS_PROGRAM, 8, ssystem3_state )
	AM_RANGE( 0x0000, 0x03ff) AM_RAM
					/*
67-de playfield ($40 means white, $80 black)
					*/
//  AM_RANGE( 0x4000, 0x40ff) AM_NOP
/*
  probably zusatzger??t memory (battery powered ram 256x4? at 0x4000)
  $40ff low nibble ram if playfield module (else init with normal playfield)
 */
	AM_RANGE( 0x6000, 0x600f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
#if 1
	AM_RANGE( 0xc000, 0xdfff) AM_ROM
	AM_RANGE( 0xf000, 0xffff) AM_ROM
#else
	AM_RANGE( 0xc000, 0xffff) AM_ROM
#endif
ADDRESS_MAP_END

static INPUT_PORTS_START( ssystem3 )
/*
  switches: light(hardware?) sound time power(hardware!)

  new game (hardware?)
*/


	PORT_START( "Switches" )
//PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NEW GAME") PORT_CODE(KEYCODE_F3) // seems to be direct wired to reset
//  PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?CLEAR") PORT_CODE(KEYCODE_F1)
//  PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_START( "matrix.0" )
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?1") PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9   C SQ     EP") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER?") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0   C BOARD  MD") PORT_CODE(KEYCODE_0)
	PORT_START( "matrix.1" )
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?2") PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 F springer zeitvorgabe") PORT_CODE(KEYCODE_6)  PORT_CODE(KEYCODE_F)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 E laeufer ruecknahme") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_E)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CE  interrupt") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_START( "matrix.2" )
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?3") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 G bauer zugvorschlaege") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_G)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 D turm #") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_D)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 A white") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_A)
	PORT_START( "matrix.3" )
		PORT_DIPNAME( 0x01, 0, "Time") PORT_CODE(KEYCODE_T) PORT_TOGGLE PORT_DIPSETTING( 0, DEF_STR(Off) ) PORT_DIPSETTING( 0x01, DEF_STR( On ) )
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 H black") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 C dame #50") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_C)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 B koenig FP") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_B)
	PORT_START( "Configuration" )
		PORT_DIPNAME( 0x0001, 0, "Schachbrett") PORT_TOGGLE
		PORT_DIPSETTING( 0, DEF_STR( Off ) )
		PORT_DIPSETTING( 1, "angeschlossen" )
#if 0
	PORT_DIPNAME( 0x0002, 0, "Memory") PORT_TOGGLE
	PORT_DIPSETTING( 0, DEF_STR( Off ) )
	PORT_DIPSETTING( 2, "angeschlossen" )
	PORT_DIPNAME( 0x0004, 0, "Drucker") PORT_TOGGLE
	PORT_DIPSETTING( 0, DEF_STR( Off ) )
	PORT_DIPSETTING( 4, "angeschlossen" )
#endif
INPUT_PORTS_END



static MACHINE_CONFIG_START( ssystem3, ssystem3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1000000)
	MCFG_CPU_PROGRAM_MAP(ssystem3_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(LCD_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(728, 437)
	MCFG_SCREEN_VISIBLE_AREA(0, 728-1, 0, 437-1)
	MCFG_SCREEN_UPDATE_DRIVER(ssystem3_state, screen_update_ssystem3)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 242 + 32768)
	MCFG_PALETTE_INIT_OWNER(ssystem3_state, ssystem3)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(READ8(ssystem3_state,ssystem3_via_read_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(ssystem3_state,ssystem3_via_read_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(ssystem3_state,ssystem3_via_write_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(ssystem3_state,ssystem3_via_write_b))
MACHINE_CONFIG_END


ROM_START(ssystem3)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("ss3lrom", 0xc000, 0x1000, CRC(9ea46ed3) SHA1(34eef85b356efbea6ddac1d1705b104fc8e2731a) )
//  ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("ss3hrom", 0xf000, 0x1000, CRC(52741e0b) SHA1(2a7b950f9810c5a14a1b9d5e6b2bd93da621662e) )
	ROM_RELOAD(0xd000, 0x1000)
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT        COMPANY     FULLNAME */
CONS( 1979, ssystem3, 0,        0,      ssystem3, ssystem3, ssystem3_state, ssystem3,   "NOVAG Industries Ltd",  "Chess Champion Super System III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
//chess champion MK III in germany
