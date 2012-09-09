/**********************************************************************************

    Pinball
    Technoplay "2-2C 8008 LS" (68000 CPU)
    Schematic and PinMAME used as references

***********************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "techno.lh"
// schematic says 8MHz but it goes too fast
#define TECHNO_MAINCLK 4e6

class techno_state : public driver_device
{
public:
	techno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
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
	UINT16 m_digit_1;
	UINT16 m_digit_2;
	UINT16 m_segment_1;
	UINT16 m_segment_2;
	UINT8 m_vector;
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
private:
	bool m_digwait;
public:
	DECLARE_DRIVER_INIT(techno);
};


static ADDRESS_MAP_START( techno_map, AS_PROGRAM, 16, techno_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x03fff) AM_ROM
	AM_RANGE(0x04000, 0x04fff) AM_RAM // battery backed-up
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
//	AM_RANGE(0x0000, 0x3fff) AM_READ(rd_r) // to TKY2016A audio processor which has its own 3.58MHz clock
//	AM_RANGE(0x4000, 0x7fff) AM_WRITE(wr_w) // A11=LED;A12=WR2 (DAC) ;A13=WR1 (TKY2016A as above)
//	AM_RANGE(0x4000, 0xbfff) AM_ROM // 4000-7FFF is same as 8000-BFFF; 4x 16k ROMS bankswitched
//	AM_RANGE(0xc000, 0xffff) AM_ROM // another 16k ROM
//ADDRESS_MAP_END

WRITE16_MEMBER( techno_state::disp1_w )
{
	m_segment_1 = BITSWAP16(data, 12, 10, 8, 14, 13, 9, 11, 15, 7, 6, 5, 4, 3, 2, 1, 0);
	output_set_digit_value(m_digit_1, m_segment_1);
}

WRITE16_MEMBER( techno_state::disp2_w )
{
	m_segment_2 = BITSWAP16(data, 12, 10, 8, 14, 13, 9, 11, 15, 7, 6, 5, 4, 3, 2, 1, 0);
	output_set_digit_value(m_digit_1+30, m_segment_2);
}

WRITE16_MEMBER( techno_state::sound_w )
{
	if (m_digwait)
		m_digit_1 = (m_digit_1+1) % 16;

	if (BIT(data, 10))
	{
		m_digwait = 1;
		m_digit_1 = 0;
	}
}

// lamps & keymatrix
WRITE16_MEMBER( techno_state::lamp1_w )
{
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
	return 0;
}

// unknown
READ16_MEMBER( techno_state::rtrg_r )
{
	return 0xffff;
}

// feedback from sound board, and some AUX inputs
READ16_MEMBER( techno_state::sound_r )
{
	return 0xffff;
}

static INPUT_PORTS_START( techno )
INPUT_PORTS_END

static INTERRUPT_GEN( techno_intgen )
{
	techno_state *state = device->machine().driver_data<techno_state>();
	// vectors change per int: 88-8F, 98-9F)
	if ((state->m_vector & 7) == 7)
		state->m_vector = (state->m_vector ^ 0x10) & 0x97;
	state->m_vector++;
	// core doesn't support clearing of irq via hardware
	generic_pulse_irq_line_and_vector(device, 1, state->m_vector, 1);
}

void techno_state::machine_reset()
{
	m_vector = 0x88;
}

DRIVER_INIT_MEMBER(techno_state,techno)
{
}

static MACHINE_CONFIG_START( techno, techno_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, TECHNO_MAINCLK)
	MCFG_CPU_PROGRAM_MAP(techno_map)
	MCFG_CPU_PERIODIC_INT(techno_intgen, TECHNO_MAINCLK/256) // 31250Hz
	//MCFG_CPU_ADD("cpu2", TMS7000, 4000000)
	//MCFG_CPU_PROGRAM_MAP(techno_sub_map)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_techno)
MACHINE_CONFIG_END

ROM_START(xforce)
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD16_BYTE("ic15", 0x000001, 0x8000, CRC(fb8d2853) SHA1(0b0004abfe32edfd3ac15d66f90695d264c97eba))
	ROM_LOAD16_BYTE("ic17", 0x000000, 0x8000, CRC(122ef649) SHA1(0b425f81869bc359841377a91c39f44395502bff))

	//ROM_REGION(0x20000), "cpu2", 0)
	// 5 x 27256 roms are undumped
ROM_END

GAME(1987,  xforce,  0,  techno,  techno, techno_state,  techno,  ROT0,  "Tecnoplay", "X Force", GAME_IS_SKELETON_MECHANICAL)
