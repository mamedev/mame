// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

    Fidelity Electronics 68000 based board driver
    
    TODO:
    - how does dual-CPU work?
    - the EAG manual mentions optional voice(speech)
    - where does the cartridge slot map to?
    - IRQ level/timing is unknown

******************************************************************************

Elite Avant Garde (EAG, model 6114)
-----------------------------------

There are 5 versions of model 6114(V1 to V5). The one emulated here came from a V2,
but is practically emulated as a V4.

V1: 128KB DRAM, no EEPROM
V2: 128KB DRAM
V3: 512KB DRAM
V4: 1MB DRAM
V5: 128KB+64KB DRAM, dual-CPU! (2*68K @ 16MHz)

V6-V11 are on model 6117. Older 1986 model 6081 uses a 6502 CPU.

- MC68HC000P12F 16MHz CPU, 16MHz XTAL
- MB1422A DRAM Controller, 25MHz XTAL near, 4 DRAM slots(V2: slot 1 and 2 64KB)
- 2*27C512 64KB EPROM, 2*KM6264AL-10 8KB SRAM, 2*AT28C64X 8KB EEPROM
- external module slot, no dumps yet
- OKI M82C51A-2 USART, 4.9152MHz XTAL, assume it's used for factory test/debug
- other special: Chessboard squares are magnet sensors

IRQ source is unknown. Several possibilities:
- NE555 timer IC
- MM/SN74HC4060 binary counter IC (near the M82C51A)
- one of the XTALs, plus divider of course

Memory map: (of what is known)
-----------
000000-01FFFF: 128KB ROM
104000-107FFF: 16KB SRAM
200000-2FFFFF: hashtable DRAM (max. 1MB)
300000-30000F W hi d0: NE591: 7seg data
300000-30000F W lo d0: NE591: LED data
300000-30000F R lo d7: 74259: keypad rows 0-7
400000-400001 W lo d0-d3: 74145/7442: led/keypad mux, buzzer out
700002-700003 R lo d7: 74251: keypad row 8
604000-607FFF: 16KB EEPROM


******************************************************************************

Elite Avant Garde (EAG, model 6117)
-----------------------------------

There are 6 versions of model 6114(V6 to V11). The one emulated here came from a V7.
From a programmer's point of view, the hardware is very similar to model 6114.

V6: 68020, 512KB hashtable RAM
V7: 68020, 1MB h.RAM
V8: 2*68020, 512KB+128KB h.RAM
V9: 68030, 1MB h.RAM
V10: 68040, 1MB h.RAM
V11: 68060, 2MB h.RAM, high speed

- MC68020RC25E CPU, QFP 25MHz XTAL, 2*GAL16V8C
- 4*AS7C164-20PC 8KB SRAM, 2*KM684000ALG-7L 512KB CMOS SRAM
- 2*27C512? 64KB EPROM, 2*HM6264LP-15 8KB SRAM, 2*AT28C64B 8KB EEPROM
- same as 6114: M82C51A, SN74HC4060, module slot?, chessboard

Memory map:
-----------
000000-01FFFF: 128KB ROM
104000-107FFF: 16KB SRAM (unused?)
200000-2FFFFF: hashtable SRAM
300000-30000F: see model 6114
400000-400007: see model 6114
700000-700003: see model 6114
604000-607FFF: 16KB EEPROM
800000-807FFF: 32KB SRAM

******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"

#include "includes/fidelz80.h"

// internal artwork
#include "fidel_eag.lh" // clickable


class fidel68k_state : public fidelz80base_state
{
public:
	fidel68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: fidelz80base_state(mconfig, type, tag)
	{ }

	// EAG(6114)
	void eag_prepare_display();
	DECLARE_READ8_MEMBER(eag_input1_r);
	DECLARE_WRITE8_MEMBER(eag_leds_w);
	DECLARE_WRITE8_MEMBER(eag_7seg_w);
	DECLARE_WRITE8_MEMBER(eag_mux_w);
	DECLARE_READ8_MEMBER(eag_input2_r);
};



// Devices, I/O

/******************************************************************************
    EAG
******************************************************************************/

// misc handlers

void fidel68k_state::eag_prepare_display()
{
	// 8*7seg leds, (8+1)*8 chessboard leds
	UINT8 seg_data = BITSWAP8(m_7seg_data,0,1,3,2,7,5,6,4);
	set_display_segmask(0x1ef, 0x7f);
	display_matrix(16, 9, m_led_data << 8 | seg_data, m_inp_mux);
}


// TTL

READ8_MEMBER(fidel68k_state::eag_input1_r)
{
	// a1-a3,d7: multiplexed inputs (active low)
	return (read_inputs(9) >> offset & 1) ? 0 : 0x80;
}

READ8_MEMBER(fidel68k_state::eag_input2_r)
{
	// d7: multiplexed inputs highest bit
	return (read_inputs(9) & 0x100) ? 0x80 : 0;
}

WRITE8_MEMBER(fidel68k_state::eag_leds_w)
{
	// a1-a3,d0: led data
	m_led_data = (m_led_data & ~(1 << offset)) | ((data & 1) << offset);
	eag_prepare_display();
}

WRITE8_MEMBER(fidel68k_state::eag_7seg_w)
{
	// a1-a3,d0(d8): digit segment data
	m_7seg_data = (m_7seg_data & ~(1 << offset)) | ((data & 1) << offset);
	eag_prepare_display();
}

WRITE8_MEMBER(fidel68k_state::eag_mux_w)
{
	// d0-d3: 74145 A-D
	// 74145 0-8: input mux, digit/led select
	// 74145 9: speaker out
	UINT16 sel = 1 << (data & 0xf);
	m_speaker->level_w(sel >> 9 & 1);
	m_inp_mux = sel & 0x1ff;
	eag_prepare_display();
}



/******************************************************************************
    Address Maps
******************************************************************************/

// EAG

static ADDRESS_MAP_START( eag_map, AS_PROGRAM, 16, fidel68k_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x104000, 0x107fff) AM_RAM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM // DRAM slots
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_READWRITE8(eag_input1_r, eag_leds_w, 0x00ff)
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_WRITE8(eag_7seg_w, 0xff00) AM_READNOP
	AM_RANGE(0x400000, 0x400001) AM_WRITE8(eag_mux_w, 0x00ff)
	AM_RANGE(0x400002, 0x400007) AM_WRITENOP // ?
	AM_RANGE(0x604000, 0x607fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x700002, 0x700003) AM_READ8(eag_input2_r, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( eagv7_map, AS_PROGRAM, 32, fidel68k_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_READWRITE8(eag_input1_r, eag_leds_w, 0x00ff00ff)
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_WRITE8(eag_7seg_w, 0xff00ff00) AM_READNOP
	AM_RANGE(0x400000, 0x400003) AM_WRITE8(eag_mux_w, 0x00ff0000)
	AM_RANGE(0x400004, 0x400007) AM_WRITENOP // ?
	AM_RANGE(0x604000, 0x607fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x800000, 0x807fff) AM_RAM
	AM_RANGE(0x700000, 0x700003) AM_READ8(eag_input2_r, 0x000000ff)
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( eag )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b3")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c4")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a4")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e5")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b5")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a5")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h6")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e6")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d6")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a6")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e7")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d7")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c7")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a7")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square h8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square g8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square f8")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square e8")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square d8")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square c8")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square b8")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Square a8")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB/King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV/Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("TM/Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("ST/Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("TB/Knight")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("LV/Pawn")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( eag, fidel68k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(eag_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(fidel68k_state, irq2_line_hold, 600) // complete guess

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_eag)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( eagv7, eag )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68020, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP(eagv7_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(fidel68k_state, irq2_line_hold, 600) // complete guess
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( feagv2 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("6114_e5.u18", 0x00000, 0x10000, CRC(f9c7bada) SHA1(60e545f829121b9a4f1100d9e85ac83797715e80) ) // 27c512
	ROM_LOAD16_BYTE("6114_o5.u19", 0x00001, 0x10000, CRC(04f97b22) SHA1(8b2845dd115498f7b385e8948eca6a5893c223d1) ) // 27c512
ROM_END


ROM_START( feagv7 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("eag-v7b", 0x00000, 0x10000, CRC(f2f68b63) SHA1(621e5073e9c5083ac9a9b467f3ef8aa29beac5ac) )
	ROM_LOAD16_BYTE("eag-v7a", 0x00001, 0x10000, CRC(506b688f) SHA1(0a091c35d0f01166b57f964b111cde51c5720d58) )
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1989, feagv2,  0,      0,      eag,     eag,     driver_device, 0, "Fidelity Electronics", "Elite Avant Garde (model 6114-2/3/4)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
COMP( 1990, feagv7,  0,      0,      eagv7,   eag,     driver_device, 0, "Fidelity Electronics", "Elite Avant Garde (model 6117-7)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
