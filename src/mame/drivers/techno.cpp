// license:BSD-3-Clause
// copyright-holders:Robbbert
/**********************************************************************************

    PINBALL
    Technoplay "2-2C 8008 LS" (68000 CPU)
    Schematic and PinMAME used as references

ToDo:
- Once you press the credit button, nothing responds (game requires 4 balls)
- Sliding display is too fast to read (much better if cpu xtal changed to 4MHz)
- No sound due to missing roms

***********************************************************************************/


#include "machine/genpin.h"
#include "cpu/m68000/m68000.h"
#include "techno.lh"


class techno_state : public driver_device
{
public:
	techno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_switch(*this, "SWITCH")
	{ }

	DECLARE_READ16_MEMBER(key_r);
	DECLARE_READ16_MEMBER(rtrg_r);
	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE16_MEMBER(disp1_w);
	DECLARE_WRITE16_MEMBER(disp2_w);
	DECLARE_WRITE16_MEMBER(lamp1_w);
	DECLARE_WRITE16_MEMBER(lamp2_w);
	DECLARE_WRITE16_MEMBER(setout_w);
	DECLARE_WRITE16_MEMBER(sol1_w);
	DECLARE_WRITE16_MEMBER(sol2_w);
	DECLARE_WRITE16_MEMBER(sound_w);
	INTERRUPT_GEN_MEMBER(techno_intgen);
private:
	bool m_digwait;
	UINT8 m_keyrow;
	UINT16 m_digit;
	UINT8 m_vector;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_ioport_array<8> m_switch;
};


static ADDRESS_MAP_START( techno_map, AS_PROGRAM, 16, techno_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x03fff) AM_ROM
	AM_RANGE(0x04000, 0x04fff) AM_RAM AM_SHARE("nvram") // battery backed-up
	AM_RANGE(0x06000, 0x0ffff) AM_ROM
	AM_RANGE(0x14000, 0x147ff) AM_READWRITE(key_r,lamp1_w)
	AM_RANGE(0x14800, 0x14fff) AM_READWRITE(sound_r,lamp2_w)
	AM_RANGE(0x15000, 0x157ff) AM_READWRITE(rtrg_r,sol1_w)
	AM_RANGE(0x15800, 0x15fff) AM_READNOP AM_WRITE(sol2_w) // reads from 15800, but shown as not connected
	AM_RANGE(0x16000, 0x167ff) AM_WRITE(sound_w)
	AM_RANGE(0x16800, 0x16fff) AM_WRITE(disp1_w)
	AM_RANGE(0x17000, 0x177ff) AM_WRITE(disp2_w)
	AM_RANGE(0x17800, 0x17fff) AM_WRITE(setout_w)
ADDRESS_MAP_END

//static ADDRESS_MAP_START( techno_sub_map, AS_IO, 8, techno_state )
//       no ram here, must be internal to the cpu
//  AM_RANGE(0x0000, 0x3fff) AM_READ(rd_r) // to TKY2016A audio processor which has its own 3.58MHz clock
//  AM_RANGE(0x4000, 0x7fff) AM_WRITE(wr_w) // A11=LED;A12=WR2 (DAC) ;A13=WR1 (TKY2016A as above)
//  AM_RANGE(0x4000, 0xbfff) AM_ROM // 4000-7FFF is same as 8000-BFFF; 4x 16k ROMS bankswitched
//  AM_RANGE(0xc000, 0xffff) AM_ROM // another 16k ROM
//ADDRESS_MAP_END

WRITE16_MEMBER( techno_state::disp1_w )
{
	output_set_digit_value(m_digit, BITSWAP16(data, 12, 10, 8, 14, 13, 9, 11, 15, 7, 6, 5, 4, 3, 2, 1, 0));
}

WRITE16_MEMBER( techno_state::disp2_w )
{
	output_set_digit_value(m_digit+30, BITSWAP16(data, 12, 10, 8, 14, 13, 9, 11, 15, 7, 6, 5, 4, 3, 2, 1, 0));
}

WRITE16_MEMBER( techno_state::sound_w )
{
/*
d0..d7 : to sound board
d8     : strobe to display board
d9     : reset (unknown purpose)
d10    : data clock to display board
d11-d15: AUX outputs
*/

// this code derived from PinMAME
	if (m_digwait)
		m_digit = (m_digit+1) % 16;

	if (BIT(data, 10))
	{
		m_digwait = 1;
		m_digit = 0;
	}
}

// lamps & keymatrix
WRITE16_MEMBER( techno_state::lamp1_w )
{
// Work out key row
	for (int i = 8; i < 16; i++)
		if (BIT(data, i))
			m_keyrow = i-8;
}

// more lamps
WRITE16_MEMBER( techno_state::lamp2_w )
{
}

// solenoids
WRITE16_MEMBER( techno_state::sol1_w )
{
}

// more solenoids
WRITE16_MEMBER( techno_state::sol2_w )
{
}

// unknown
WRITE16_MEMBER( techno_state::setout_w )
{
}

// inputs
READ16_MEMBER( techno_state::key_r )
{
	return m_switch[m_keyrow]->read();
}

// unknown
READ16_MEMBER( techno_state::rtrg_r )
{
	return 0xffff;
}

// feedback from sound board, and some AUX inputs
READ16_MEMBER( techno_state::sound_r )
{
	return 0;
}

static INPUT_PORTS_START( techno )
	PORT_START("SWITCH.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Letter select+") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Test-") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Test+") PORT_CODE(KEYCODE_F8)
	PORT_START("SWITCH.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix top target right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Horizontal rail right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Mini Post") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Letter select+") PORT_CODE(KEYCODE_F6)
	PORT_START("SWITCH.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Inner Canal Left") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Exit Canal Right") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Out Hole") PORT_CODE(KEYCODE_X)
	PORT_START("SWITCH.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Top Bumper") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bottom Bumper") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Top Right Kicker") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right Kicker") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left Kicker") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Exit Canal Left") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Inner Canal Right") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("SWITCH.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix left target bottom") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left rollover") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Barrier 1 Target") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Barrier 2 Target") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Barrier 3 Target") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left Bumper") PORT_CODE(KEYCODE_H)
	PORT_START("SWITCH.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix right target top") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix right target middle-top") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinning Target") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fixed contact") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Special Target") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix left target top") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix left target centre") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_START("SWITCH.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix top left target left") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 1 Bridge")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 2 Bridge")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 3 Bridge")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ball 4 Bridge")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix right target middle-bottom") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix right target bottom") PORT_CODE(KEYCODE_V)
	PORT_START("SWITCH.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix top target middle-right") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix top target middle-left") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix top target Left") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Horizontal Rail Left") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Chopper Exit") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Chopper Entry") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix top left target right") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fix top left target middle") PORT_CODE(KEYCODE_EQUALS)
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(techno_state::techno_intgen)
{
	// vectors change per int: 88-8F, 98-9F)
	if ((m_vector & 7) == 7)
		m_vector = (m_vector ^ 0x10) & 0x97;
	m_vector++;
	// core doesn't support clearing of irq via hardware
	generic_pulse_irq_line_and_vector(device.execute(), 1, m_vector, 1);
}

void techno_state::machine_reset()
{
	m_vector = 0x88;
	m_digit = 0;
}

static MACHINE_CONFIG_START( techno, techno_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(techno_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(techno_state, techno_intgen,  XTAL_8MHz/256) // 31250Hz
	MCFG_NVRAM_ADD_0FILL("nvram")
	//MCFG_CPU_ADD("cpu2", TMS7000, XTAL_4MHz)
	//MCFG_CPU_PROGRAM_MAP(techno_sub_map)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_techno)
MACHINE_CONFIG_END

ROM_START(xforce)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE("ic15", 0x0001, 0x8000, CRC(fb8d2853) SHA1(0b0004abfe32edfd3ac15d66f90695d264c97eba))
	ROM_LOAD16_BYTE("ic17", 0x0000, 0x8000, CRC(122ef649) SHA1(0b425f81869bc359841377a91c39f44395502bff))

	//ROM_REGION(0x20000), "cpu2", 0)
	// 5 x 27256 roms are undumped
ROM_END

GAME(1987,  xforce,  0,  techno,  techno, driver_device,  0,  ROT0,  "Tecnoplay", "X Force", MACHINE_IS_SKELETON_MECHANICAL)
