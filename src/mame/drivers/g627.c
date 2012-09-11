/******************************************************************************

    Pinball
    Midway A084-91313-G627
           A080-91313-G627
           A082-91320-C000

    Only one of its kind

    This is a cocktail pinball game, for up to 4 players. The board is round.
    When it is another player's turn, the playboard will turn around to face
    him. And so, the system has a motor and an infrared shaft-locating system.
    If this system does not return the expected data, the machine will refuse
    to start.

    Schematic and PinMAME used as references

ToDo:
- Battery backup
- Inputs
- Outputs
- Displays / Layout
- Diagnostic keyboard
- Possibility of a rom missing (most likely it is optional)

*******************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8155.h"
#include "sound/astrocde.h"
#include "g627.lh"


class g627_state : public driver_device
{
public:
	g627_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE8_MEMBER(disp_w);

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_seg[6];
};


static ADDRESS_MAP_START( g627_map, AS_PROGRAM, 8, g627_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_DEVREADWRITE("i8156", i8155_device, memory_r, memory_w)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM // battery backed
ADDRESS_MAP_END

static ADDRESS_MAP_START( g627_io, AS_IO, 8, g627_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x02) AM_WRITE(disp_w)
	//AM_RANGE(0x03, 0x07) AM_WRITE(port_0x_w)
	AM_RANGE(0x10, 0x17) AM_DEVWRITE_LEGACY("astrocade", astrocade_sound_w)
	AM_RANGE(0x20, 0x27) AM_DEVREADWRITE("i8156", i8155_device, io_r, io_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( g627 )
INPUT_PORTS_END

void g627_state::machine_reset()
{
}

READ8_MEMBER( g627_state::porta_r )
{
	return 0;
}

READ8_MEMBER( g627_state::portb_r )
{
	return 0;
}

// write the 6 digits of 1 display panel
WRITE8_MEMBER( g627_state::portc_w )
{
	if (data < 7)
		for (int i = 0; i < 6; i++)
			output_set_digit_value(data * 10 + i, m_seg[i]);
}

// save segments until we can write the digits
WRITE8_MEMBER( g627_state::disp_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	offset <<= 1;
	m_seg[offset]=patterns[data>>4];
	m_seg[++offset]=patterns[data&15];
}

static I8156_INTERFACE(i8156_intf)
{
	DEVCB_DRIVER_MEMBER(g627_state,porta_r), // Port A in
	DEVCB_NULL, // Port A out
	DEVCB_DRIVER_MEMBER(g627_state,portb_r), // Port B in
	DEVCB_NULL, // Port B out
	DEVCB_NULL, // Port C in
	DEVCB_DRIVER_MEMBER(g627_state,portc_w), // Port C out
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_NMI) // timer out
};

static MACHINE_CONFIG_START( g627, g627_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 14138000/8)
	MCFG_CPU_PROGRAM_MAP(g627_map)
	MCFG_CPU_IO_MAP(g627_io)
	MCFG_I8156_ADD("i8156", 14138000/8, i8156_intf)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("astrocade",  ASTROCADE, 14138000/8) // 0066-117XX audio chip
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_g627)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Rotation VIII (09/1978)
/-------------------------------------------------------------------*/
ROM_START(rotation)
	ROM_REGION(0x10000, "maincpu", 0) // schematic shows 4x 2716 roms
	ROM_LOAD("rot-a117.dat", 0x0000, 0x0800, CRC(7bb6beb3) SHA1(5ee62246032158c68d426c11a4a9a889ee7655d7))
	ROM_LOAD("rot-b117.dat", 0x0800, 0x0800, CRC(538e37b2) SHA1(d283ac4d0024388b92b6494fcde63957b705bf48))
	ROM_LOAD("rot-c117.dat", 0x1000, 0x0800, CRC(3321ff08) SHA1(d6d94fea27ef58ca648b2829b32d62fcec108c9b))
ROM_END


GAME(1978,  rotation,  0,  g627,  g627, driver_device,  0,  ROT0,  "Midway", "Rotation VIII", GAME_MECHANICAL | GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD)
