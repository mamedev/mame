// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
	Symbolics 3600
	TODO: add credits, backstory, history, etc here
	Front-end Processor dumped only, so far, plds/proms/pals not dumped yet
******************************************************************************/

/* Core includes */
#include "cpu/m68000/m68000.h"

class symbolics_state : public driver_device
{
public:

	symbolics_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	required_device<m68000_base_device> m_maincpu;
	DECLARE_DRIVER_INIT(symbolics);

//protected:
//	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


/******************************************************************************
 Address Maps
******************************************************************************/
/*
Address maps (x = ignored; * = selects address within this range, ? = unknown, 1/0 = decodes only when this bit is set to 1/0)
68k address map:
a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  (a0 via UDS/LDS)
?   ?   ?   ?   ?   0   0   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   0       R   ROM 00H
?   ?   ?   ?   ?   0   0   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   1       R   ROM 00L
?   ?   ?   ?   ?   0   0   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   0       R   ROM 04H
?   ?   ?   ?   ?   0   0   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   1       R   ROM 04L
?   ?   ?   ?   ?   0   0   1   0   x   x   x   x   x   x   x   x   x   x   x   x   x   x   0       R   Open bus?
?   ?   ?   ?   ?   0   0   1   0   x   x   x   x   x   x   x   x   x   x   x   x   x   x   1       R   Open bus?
?   ?   ?   ?   ?   0   0   1   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   0       R   ROM 10H
?   ?   ?   ?   ?   0   0   1   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   1       R   ROM 10L
?   ?   ?   ?   ?   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW  RAM <- recheck this, might be a weird hole between 20000-20fff ?
?   ?   ?   ?   ?   1   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   *       ?   Something maps here... 
              |               |               |               |               |
*/

static ADDRESS_MAP_START(m68k_mem, AS_PROGRAM, 16, symbolics_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x00ffff) AM_ROM /* ROM */
	// 10000-17fff open bus here?
	AM_RANGE(0x018000, 0x01ffff) AM_ROM /* ROM */
	AM_RANGE(0x020000, 0x03ffff) AM_RAM /* Local FEP ram seems to be here? */
	// there is stuff mapped at 40000 and ffxxx as well, ffxxx may be the main lisp cpu's microcode and macrocode areas
ADDRESS_MAP_END

static ADDRESS_MAP_START(m68k_io, AS_IO, 16, symbolics_state )
ADDRESS_MAP_END

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( symbolics )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/
/*void symbolics_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_OUTFIFO_READ:
		outfifo_read_cb(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in symbolics_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(symbolics_state::outfifo_read_cb)
{
	UINT16 data;
}
*/

/* Driver init: stuff that needs setting up which isn't directly affected by reset */
DRIVER_INIT_MEMBER(symbolics_state,symbolics)
{
}

static MACHINE_CONFIG_START( symbolics, symbolics_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000) /* PLACEHOLDER CLOCK, TODO: add correct clock */
	MCFG_CPU_PROGRAM_MAP(m68k_mem)
	MCFG_CPU_IO_MAP(m68k_io)

MACHINE_CONFIG_END

/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( s3600 )
	ROM_REGION16_BE(0x40000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v127", "Symbolics 3600 L-Machine FEP V127")
	ROMX_LOAD("00h.127.27128.d13", 0x00000, 0x4000, CRC(b8d7c8da) SHA1(663a09359f5db63beeac00e5c2783ccc25b94250), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "00H.127"
	ROMX_LOAD("00l.127.27c128.d7", 0x00001, 0x4000, CRC(cc7bae9a) SHA1(057538eb821c4d00dde19cfe5136ccc0aee43800), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "00L.127"
	ROMX_LOAD("04h.127.27c128.d15", 0x08000, 0x4000, CRC(e01a717b) SHA1(b87a670f7be13553485ce88fad5fcf90f01473c4), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "04H.127"
	ROMX_LOAD("04l.127.27c128.d9", 0x08001, 0x4000, CRC(68d169fa) SHA1(d6fab3132fca332a9bedb174fddf5fc8c69d05b6), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "04L.127"
	ROMX_LOAD("10h.127.27c128.d16", 0x18000, 0x4000, CRC(2ea7a70d) SHA1(61cc97aada028612c24d788d946d77e82116cf30), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "10H.127"
	ROMX_LOAD("10l.127.27128.d10", 0x18001, 0x4000, CRC(b8ddb3c8) SHA1(e6c3b96340c5c767ef18abf48b73fa8e5d7353b9), ROM_SKIP(1) | ROM_BIOS(1)) // Label: "10L.127"
	// note: load all the PLAs, PALs and PROMs here
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       STATE            INIT        COMPANY      FULLNAME  FLAGS */
COMP( 1984, s3600,      0,      0,      symbolics,  symbolics,  symbolics_state, symbolics,  "Symbolics", "3600",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
