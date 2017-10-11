// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

    Fidelity Electronics 68000 based board driver

    NOTE: MAME doesn't include a generalized implementation for boardpieces yet,
    greatly affecting user playability of emulated electronic board games.
    As workaround for the chess games, use an external chess GUI on the side,
    such as Arena(in editmode).

    TODO:
    - fex68km3 addressmap RAM mirror? PCB has 2*32K RAM, emulated has 16K+64K
    - how does dual-CPU work?
    - USART is not emulated
    - V9(68030 @ 32MHz) is faster than V10(68040 @ 25MHz) but it should be the other
      way around, culprit is unemulated cache?
    - V11 CPU should be M68EC060, not yet emulated. Now using M68EC040 in its place
      at twice the frequency due to lack of superscalar.
    - V11 beeper is too high pitched, obviously related to wrong CPU type too

******************************************************************************

Excel 68000 (model 6094)
------------------------
16KB RAM(2*SRM2264C-10 @ U8/U9), 64KB ROM(2*AT27C256-15DC @ U6/U7)
HD68HC000P12 CPU, 12MHz XTAL
PCB label 510-1129A01
PCB has edge connector for module, but no external slot

There's room for 2 SIMMs at U22 and U23. Unpopulated in Excel 68000, used for
128KB hashtable RAM in Mach II. Mach III has wire mods to U8/U9(2*32KB).

I/O is via TTL, overall very similar to EAG.


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

V6-V11 are on model 6117. Older 1986 model 6081/6088/6089 uses a 6502 CPU.

Hardware info:
--------------
- MC68HC000P12F 16MHz CPU, 16MHz XTAL
- MB1422A DRAM Controller, 25MHz XTAL near, 4 DRAM slots
  (V2: slot 2 & 3 64KB, V3: slot 2 & 3 256KB)
- 2*27C512 64KB EPROM, 2*KM6264AL-10 8KB SRAM, 2*AT28C64X 8KB EEPROM
- OKI M82C51A-2 USART, 4.9152MHz XTAL
- other special: magnet sensors, external module slot, serial port

IRQ source is a 4,9152MHz quartz crystal (Y3) connected to a 74HC4060 (U8,
ripple counter/divider). From Q13 output (counter=8192) we obtain the IRQ signal
applied to IPL1 of 68000 (pin 24) 4,9152 MHz / 8192 = 600 Hz.

The module slot pinout is different from SCC series. The data on those appears
to be compatible with EAG though and will load fine with an adapter.

The USART allows for a serial connection between the chess computer and another
device, for example a PC. Fidelity released a DOS tool called EAGLINK which
featured PC printer support, complete I/O control, detailed information while
the program is 'thinking', etc.

Memory map: (of what is known)
-----------
000000-01FFFF: 128KB ROM
104000-107FFF: 16KB SRAM
200000-2FFFFF: hashtable DRAM (max. 1MB)
300000-30000F W hi d0: NE591: 7seg data
300000-30000F W lo d0: NE591: LED data
300000-30000F R lo d7: 74259: keypad rows 0-7
400000-400001 W lo d0-d3: 74145/7442: led/keypad mux, buzzer out
400000-4????? R hi: external module slot
700002-700003 R lo d7: 74251: keypad row 8
604000-607FFF: 16KB EEPROM


******************************************************************************

Elite Avant Garde (EAG, model 6117)
-----------------------------------

There are 6 versions of model 6114(V6 to V11). From a programmer's point of view,
the hardware is very similar to model 6114.

V6: 68020, 512KB hashtable RAM
V7: 68020, 1MB h.RAM
V8: 2*68020, 512KB+128KB h.RAM
V9: 68030, 1MB h.RAM
V10: 68040, 1MB h.RAM
V11: 68060, 2MB h.RAM, high speed

V7 Hardware info:
-----------------
- MC68020RC25E CPU, 25MHz XTAL - this PCB was overclocked, original was 20MHz so let's use that
- 4*AS7C164-20PC 8KB SRAM, 2*KM684000ALG-7L 512KB CMOS SRAM
- 2*27C512? 64KB EPROM, 2*HM6264LP-15 8KB SRAM, 2*AT28C64B 8KB EEPROM, 2*GAL16V8C
- same as 6114: M82C51A, NE555, SN74HC4060, module slot, chessboard, ..

V7 Memory map:
--------------
000000-01FFFF: 128KB ROM
104000-107FFF: 16KB SRAM (unused?)
200000-2FFFFF: hashtable SRAM
300000-30000x: see model 6114
400000-40000x: see model 6114
700000-70000x: see model 6114
604000-607FFF: 16KB EEPROM
800000-807FFF: 32KB SRAM

V10 Hardware info:
------------------
- 68040 CPU, 25MHz
- other: assume same or very similar to V11(see below)

The ROM dump came from the V11(see below). Built-in factory test proves
that this program is a V10. Hold TB button immediately after power-on and
press it for a sequence of tests:
1) all LEDs on
2) F40C: V10 program version
3) 38b9: V10 ROM checksum
4) xxxx: external module ROM checksum (0000 if no module present)
5) xxxx: user settings (stored in EEPROM)
6) xxxx: "
7) 1024: hashtable RAM size
8) return to game

V11 Hardware info:
------------------
- MC68EC060RC75 CPU, 36MHz XTAL(36MHz bus, 72MHz CPU), CPU cooler required
- 4*CXK5863AP-20 8KB SRAM, 4*K6X4008C1F-DF55 512KB CMOS SRAM
- 4*M27C256B 32KB EPROM, 2*AT28C64 8KB EEPROM, 5*GAL16V8D
- NEC D71051C USART, assume 8MHz, on quick glance it's same as the OKI USART
- same as 6114: NE555, SN74HC4060, module slot, chessboard, ..

This is a custom overclocked V10, manufactured by Wilfried Bucke. PCB is marked:
"CHESS HW DESIGN COPYRIGHT 22-10-2002: REVA03 510.1136A01/510.1144B01 COMPONENT SIDE"
There are two versions of this, one with a 66MHz CPU, one with a 72MHz CPU.
Maybe other differences too?

V1x Memory map:
---------------
000000-01FFFF: 128KB ROM
200000-3FFFFF: hashtable SRAM (less on V10?)
B0000x-xxxxxx: see V7, -800000

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_ex_68k.lh" // clickable
#include "fidel_eag_68k.lh" // clickable


class fidel68k_state : public fidelbase_state
{
public:
	fidel68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: fidelbase_state(mconfig, type, tag),
		m_ram(*this, "ram")
	{ }

	optional_device<ram_device> m_ram;

	TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE); }

	// Excel 68000
	DECLARE_WRITE8_MEMBER(fex68k_mux_w);

	// EAG(6114/6117)
	DECLARE_DRIVER_INIT(eag);
	void eag_prepare_display();
	DECLARE_READ8_MEMBER(eag_input1_r);
	DECLARE_WRITE8_MEMBER(eag_leds_w);
	DECLARE_WRITE8_MEMBER(eag_7seg_w);
	DECLARE_WRITE8_MEMBER(eag_mux_w);
	DECLARE_READ8_MEMBER(eag_input2_r);
};



// Devices, I/O

/******************************************************************************
    Excel 68000
******************************************************************************/

WRITE8_MEMBER(fidel68k_state::fex68k_mux_w)
{
	// a1-a3,d0: 74259
	u8 mask = 1 << offset;
	m_led_select = (m_led_select & ~mask) | ((data & 1) ? mask : 0);

	// 74259 Q0-Q3: 74145 A-D (Q4-Q7 N/C)
	eag_mux_w(space, offset, m_led_select & 0xf);
}



/******************************************************************************
    EAG
******************************************************************************/

// TTL/generic

void fidel68k_state::eag_prepare_display()
{
	// Excel 68000: 4*7seg leds, 8*8 chessboard leds
	// EAG: 8*7seg leds(2 panels), (8+1)*8 chessboard leds
	u8 seg_data = BITSWAP8(m_7seg_data,0,1,3,2,7,5,6,4);
	set_display_segmask(0x1ff, 0x7f);
	display_matrix(16, 9, m_led_data << 8 | seg_data, m_inp_mux);
}

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
	u16 sel = 1 << (data & 0xf);
	m_dac->write(BIT(sel, 9));
	m_inp_mux = sel & 0x1ff;
	eag_prepare_display();
}



/******************************************************************************
    Address Maps
******************************************************************************/

// Excel 68000

static ADDRESS_MAP_START( fex68k_map, AS_PROGRAM, 16, fidel68k_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x000000, 0x00000f) AM_MIRROR(0x00fff0) AM_WRITE8(eag_leds_w, 0x00ff)
	AM_RANGE(0x000000, 0x00000f) AM_MIRROR(0x00fff0) AM_WRITE8(eag_7seg_w, 0xff00)
	AM_RANGE(0x044000, 0x047fff) AM_RAM
	AM_RANGE(0x100000, 0x10000f) AM_MIRROR(0x03fff0) AM_READ8(eag_input1_r, 0x00ff)
	AM_RANGE(0x140000, 0x14000f) AM_MIRROR(0x03fff0) AM_WRITE8(fex68k_mux_w, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fex68km2_map, AS_PROGRAM, 16, fidel68k_state )
	AM_RANGE(0x200000, 0x21ffff) AM_RAM
	AM_IMPORT_FROM( fex68k_map )
ADDRESS_MAP_END

static ADDRESS_MAP_START( fex68km3_map, AS_PROGRAM, 16, fidel68k_state )
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_IMPORT_FROM( fex68k_map )
ADDRESS_MAP_END


// EAG

DRIVER_INIT_MEMBER(fidel68k_state, eag)
{
	// eag_map: DRAM slots at $200000-$2fffff - V1/V2: 128K, V3: 512K, V4: 1M
	m_maincpu->space(AS_PROGRAM).install_ram(0x200000, 0x200000 + m_ram->size() - 1, m_ram->pointer());
}

static ADDRESS_MAP_START( eag_map, AS_PROGRAM, 16, fidel68k_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x104000, 0x107fff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_READWRITE8(eag_input1_r, eag_leds_w, 0x00ff)
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_WRITE8(eag_7seg_w, 0xff00) AM_READNOP
	AM_RANGE(0x400000, 0x407fff) AM_READ8(cartridge_r, 0xff00)
	AM_RANGE(0x400000, 0x400001) AM_WRITE8(eag_mux_w, 0x00ff)
	AM_RANGE(0x400002, 0x400007) AM_WRITENOP // ?
	AM_RANGE(0x604000, 0x607fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x700002, 0x700003) AM_READ8(eag_input2_r, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( eagv7_map, AS_PROGRAM, 32, fidel68k_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x104000, 0x107fff) AM_RAM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_READWRITE8(eag_input1_r, eag_leds_w, 0x00ff00ff)
	AM_RANGE(0x300000, 0x30000f) AM_MIRROR(0x000010) AM_WRITE8(eag_7seg_w, 0xff00ff00) AM_READNOP
	AM_RANGE(0x400000, 0x407fff) AM_READ8(cartridge_r, 0xff00ff00)
	AM_RANGE(0x400000, 0x400003) AM_WRITE8(eag_mux_w, 0x00ff0000)
	AM_RANGE(0x400004, 0x400007) AM_WRITENOP // ?
	AM_RANGE(0x604000, 0x607fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x700000, 0x700003) AM_READ8(eag_input2_r, 0x000000ff)
	AM_RANGE(0x800000, 0x807fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( eagv11_map, AS_PROGRAM, 32, fidel68k_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
	AM_RANGE(0x00200000, 0x003fffff) AM_RAM
	AM_RANGE(0x00b00000, 0x00b0000f) AM_MIRROR(0x00000010) AM_READWRITE8(eag_input1_r, eag_leds_w, 0x00ff00ff)
	AM_RANGE(0x00b00000, 0x00b0000f) AM_MIRROR(0x00000010) AM_WRITE8(eag_7seg_w, 0xff00ff00) AM_READNOP
	AM_RANGE(0x00c00000, 0x00c07fff) AM_READ8(cartridge_r, 0xff00ff00)
	AM_RANGE(0x00c00000, 0x00c00003) AM_WRITE8(eag_mux_w, 0x00ff0000)
	AM_RANGE(0x00c00004, 0x00c00007) AM_WRITENOP // ?
	AM_RANGE(0x00e04000, 0x00e07fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x00f00000, 0x00f00003) AM_READ8(eag_input2_r, 0x000000ff)
	AM_RANGE(0x01000000, 0x0101ffff) AM_RAM
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( fex68k )
	PORT_INCLUDE( fidel_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Hint / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Take Back / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Options / Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Verify / King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END


static INPUT_PORTS_START( eag )
	PORT_INCLUDE( fidel_cb_magnets )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("LV / Pawn")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( fex68k )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz) // HD68HC000P12
	MCFG_CPU_PROGRAM_MAP(fex68k_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_on", fidel68k_state, irq_on, attotime::from_hz(618)) // theoretical frequency from 556 timer (22nF, 91K + 20K POT @ 14.8K, 0.1K), measurement was 580Hz
	MCFG_TIMER_START_DELAY(attotime::from_hz(618) - attotime::from_nsec(1525)) // active for 1.525us
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_off", fidel68k_state, irq_off, attotime::from_hz(618))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelbase_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_ex_68k)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC_1BIT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fex68km2, fex68k )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(fex68km2_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fex68km3, fex68k )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(XTAL_16MHz) // factory overclock
	MCFG_CPU_PROGRAM_MAP(fex68km3_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( eag )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(eag_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_on", fidel68k_state, irq_on, attotime::from_hz(XTAL_4_9152MHz/0x2000)) // 600Hz
	MCFG_TIMER_START_DELAY(attotime::from_hz(XTAL_4_9152MHz/0x2000) - attotime::from_nsec(8250)) // active for 8.25us
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_off", fidel68k_state, irq_off, attotime::from_hz(XTAL_4_9152MHz/0x2000))

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelbase_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_eag_68k)

	MCFG_RAM_ADD("ram")
	MCFG_RAM_DEFAULT_SIZE("1M")
	MCFG_RAM_EXTRA_OPTIONS("128K, 512K, 1M")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC_1BIT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "fidel_scc")
	MCFG_GENERIC_EXTENSIONS("bin,dat")
	MCFG_GENERIC_LOAD(fidelbase_state, scc_cartridge)
	MCFG_SOFTWARE_LIST_ADD("cart_list", "fidel_scc")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( eagv7, eag )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68020, XTAL_20MHz)
	MCFG_CPU_PROGRAM_MAP(eagv7_map)

	MCFG_RAM_REMOVE("ram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( eagv9, eagv7 )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68030, XTAL_32MHz)
	MCFG_CPU_PROGRAM_MAP(eagv7_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( eagv10, eagv7 )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68040, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP(eagv11_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( eagv11, eagv7 )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68EC040, XTAL_36MHz*2*2) // wrong! should be M68EC060 @ 72MHz
	MCFG_CPU_PROGRAM_MAP(eagv11_map)

	MCFG_CPU_PERIODIC_INT_DRIVER(fidel68k_state, irq2_line_hold, 600)
	MCFG_DEVICE_REMOVE("irq_on") // 8.25us is too long
	MCFG_DEVICE_REMOVE("irq_off")
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fex68k ) // model 6094, PCB label 510.1120B01
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("e3_yellow.u6", 0x00000, 0x08000, CRC(a8a27714) SHA1(bc42a561eb39dd389c7831f1a25ad260510085d8) ) // AT27C256-15
	ROM_LOAD16_BYTE("o4_red.u7",    0x00001, 0x08000, CRC(560a14b7) SHA1(11f2375255bfa229314697f103e891ba1cf0c715) ) // "
ROM_END

ROM_START( fex68km2 ) // model 6097, PCB label 510.1120B01
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("fex68km2.u6", 0x00000, 0x08000, CRC(2e65e7ad) SHA1(4f3aec12041c9014d5d700909bac66bae1f9eadf) )
	ROM_LOAD16_BYTE("fex68km2.u7", 0x00001, 0x08000, CRC(4c20334a) SHA1(2e575b88c41505cc89599d2fc13e1e84fe474469) )
ROM_END

ROM_START( fex68km3 ) // model 6098, PCB label 510.1120B01
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("me_white.u6",  0x00000, 0x08000, CRC(4b14cd9f) SHA1(4d41196900a71bf0699dae50f4726acc0ed3dced) ) // 27c256
	ROM_LOAD16_BYTE("mo_yellow.u7", 0x00001, 0x08000, CRC(b96b0b5f) SHA1(281145be802efb38ed764aecb26b511dcd71cb87) ) // "
ROM_END


ROM_START( feagv2 ) // from a V2 board
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("6114_e5_yellow.u22", 0x00000, 0x10000, CRC(f9c7bada) SHA1(60e545f829121b9a4f1100d9e85ac83797715e80) ) // 27c512
	ROM_LOAD16_BYTE("6114_o5_green.u19",  0x00001, 0x10000, CRC(04f97b22) SHA1(8b2845dd115498f7b385e8948eca6a5893c223d1) ) // "
ROM_END

ROM_START( feagv2a ) // from a V3 board
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("elite_1.6_e.u22", 0x00000, 0x10000, CRC(c8b89ccc) SHA1(d62e0a72f54b793ab8853468a81255b62f874658) )
	ROM_LOAD16_BYTE("elite_1.6_o.u19", 0x00001, 0x10000, CRC(904c7061) SHA1(742110576cf673321440bc81a4dae4c949b49e38) )
ROM_END

ROM_START( feagv7 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("eag-v7b", 0x00000, 0x10000, CRC(f2f68b63) SHA1(621e5073e9c5083ac9a9b467f3ef8aa29beac5ac) )
	ROM_LOAD16_BYTE("eag-v7a", 0x00001, 0x10000, CRC(506b688f) SHA1(0a091c35d0f01166b57f964b111cde51c5720d58) )
ROM_END

ROM_START( feagv9 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("eag-v9b", 0x00000, 0x10000, CRC(60523199) SHA1(a308eb6b782732af1ab2fd0ed8b046de7a8dd24b) )
	ROM_LOAD16_BYTE("eag-v9a", 0x00001, 0x10000, CRC(255c63c0) SHA1(8aa0397bdb3731002f5b066cd04ec62531267e22) )
ROM_END

ROM_START( feagv10 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD32_BYTE("16", 0x00000, 0x08000, CRC(8375d61f) SHA1(e042f6f01480c59ee09a458cf34f135664479824) ) // 27c256
	ROM_LOAD32_BYTE("17", 0x00001, 0x08000, CRC(bfd14916) SHA1(115af6dfd29ddd8ad6d2ce390f8ecc4d60de6fce) ) // "
	ROM_LOAD32_BYTE("18", 0x00002, 0x08000, CRC(9341dcaf) SHA1(686bd4799e89ffaf11a813d4cf5a2aedd4c2d97a) ) // "
	ROM_LOAD32_BYTE("19", 0x00003, 0x08000, CRC(a70c5468) SHA1(7f6b4f46577d5cfdaa84d387c7ce35d941e5bbc7) ) // "
ROM_END

ROM_START( feagv11 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD32_BYTE("16", 0x00000, 0x08000, CRC(8375d61f) SHA1(e042f6f01480c59ee09a458cf34f135664479824) ) // 27c256
	ROM_LOAD32_BYTE("17", 0x00001, 0x08000, CRC(bfd14916) SHA1(115af6dfd29ddd8ad6d2ce390f8ecc4d60de6fce) ) // "
	ROM_LOAD32_BYTE("18", 0x00002, 0x08000, CRC(9341dcaf) SHA1(686bd4799e89ffaf11a813d4cf5a2aedd4c2d97a) ) // "
	ROM_LOAD32_BYTE("19", 0x00003, 0x08000, CRC(a70c5468) SHA1(7f6b4f46577d5cfdaa84d387c7ce35d941e5bbc7) ) // "
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT  CMP MACHINE   INPUT   STATE           INIT  COMPANY, FULLNAME, FLAGS
CONS( 1987, fex68k,   0,       0, fex68k,   fex68k, fidel68k_state, 0,    "Fidelity Electronics", "Excel 68000", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1988, fex68km2, fex68k,  0, fex68km2, fex68k, fidel68k_state, 0,    "Fidelity Electronics", "Excel 68000 Mach II (rev. C+)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1988, fex68km3, fex68k,  0, fex68km3, fex68k, fidel68k_state, 0,    "Fidelity Electronics", "Excel 68000 Mach III", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1989, feagv2,   0,       0, eag,      eag,    fidel68k_state, eag,  "Fidelity Electronics", "Elite Avant Garde (model 6114-2/3/4, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1989, feagv2a,  feagv2,  0, eag,      eag,    fidel68k_state, eag,  "Fidelity Electronics", "Elite Avant Garde (model 6114-2/3/4, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, feagv7,   feagv2,  0, eagv7,    eag,    fidel68k_state, 0,    "Fidelity Electronics", "Elite Avant Garde (model 6117-7)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, feagv9,   feagv2,  0, eagv9,    eag,    fidel68k_state, 0,    "Fidelity Electronics", "Elite Avant Garde (model 6117-9)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, feagv10,  feagv2,  0, eagv10,   eag,    fidel68k_state, 0,    "Fidelity Electronics", "Elite Avant Garde (model 6117-10)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
CONS( 2002, feagv11,  feagv2,  0, eagv11,   eag,    fidel68k_state, 0,    "hack (Wilfried Bucke)", "Elite Avant Garde (model 6117-11)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
