// license:BSD-3-Clause
// copyright-holders: Roberto Fresca
/********************************************************************

  Lucky Ball 96.
  6-player LEDs Roulette.

  Copyright 1991/96 by Sielcon Games.
  Industria Argentina.

  Driver by Roberto Fresca.


  4 sets dumped:
  Version: 3.50 - 616
  Version: 3.50 - 623
  Version: 3.50 - 626
  Version: 3.50 - 627

  Each set has:
  1x 64K program ROM (unknown CPU)
  1x 512K sound ROM (unsigned 8-bit PCM @ 11025 KHz)


*********************************************************************

  PCB specs:

  Sikscreened: 'PCB Ver 1.5'
  Sikscreened: 'SIELCON GAMES'
  Sikscreened: 'Copyright 1996'
  Sikscreened: 'MADE IN ARGENTINA'

  1x Unknown sanded PLCC68 IC (identified as Zilog Z180).
  1x unknown sanded DIL24 IC labeled PLD-01.
  1x unknown sanded DIL28 IC labeled PLD-02 (identified as ST6265 MCU).
  1x unknown sanded DIL20 IC labeled PLD-03 @U21.
  1x unknown sanded DIL64 IC @U12 (identified as Yamaha 9938/58 VDP).

  1x PPI 8255 (@U10)
  3x 4099 (8-Bit Addressable Latch) @U1-U3-U5.
  3x 4512 (8-Channel Data Selector) @U2-U4-U7.
  
  1x INMOS B IMS1630K-70M (8K x 8 SRAM, DIL28)... NOTE: Replaced by UT6264CPC-70LL in another board.
  4x sanded IC's identified as 4464 (64K x 4 DRAM) @U16-U17-U18-U19, near the Yamaha 9938 for video RAM.

  1x 27C512   (program ROM).
  1x M27C4001 (GFX + sound samples).
  1x 24C04 serial EEPROM (missing).

  1x unknown DAC (DIL16) @U13.
  1x LM324 (Single Supply Quad Operational Amplifier)

  1x Maxim MAX691CPE (battery supervisor). 

  1x 21.47727 MHz. crystal, near the Yamaha 9938/58 VDP.
  1x 12.288 MHz. crystal, near the Zilog Z180.
  1x 8.000 MHz. sanded crystal near the ST6265 MCU (measured).

  1x 8 DIP Switches bank.
  1x pushbutton silkscreened 'PAGE'.
  1x pushbutton silkscreened 'RESET'.

  1x 3.6 V. battery.

  1x 2x10 blind connector silkscreened 'EXPANSION PORT'.
  1x 2x17 blind connector silkscreened 'EXPANSION SLOT'.
  1x 6-pin connector silkscreened 'NET'.
  1x RS232 connector silkscreened 'RS-232'.
  1x 2x28 JAMMA type connector.


*********************************************************************

  Edge Connector:


           Solder side |  | Components
  ---------------------|--|-----------------
                 +5VCC |01| GND
                 +5VCC |02| -5VCC
             Jugador 2 |03| Jugador 1
             Jugador 4 |04| Jugador 3
             Jugador 6 |06| Jugador 5
                       |07|
                       |08| Izquierda
               Derecha |09| Abajo
               Entrada |10| Salida
               Apuesta |11| Arriba
                       |12|
                       |13|
                       |14|
                       |15|
                       |16|
                       |17|
                       |18| +5VCC
     Contador Entradas |19|
                       |20| Contador Salidas
          Habilitacion |21|
                       |22| Programacion
                       |23|
                 Verde |24| Sincronismo
                  Azul |25| Rojo
                +12VCC |26| Parlante
                 +5VCC |27| GND
                 +5VCC |28| GND


  DIP Switches:

  .---------------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches                          |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +---------------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +---------------------------------------+-----+-----------------------------------------+
  | Acceso a programación        No       | OFF |                                         |
  | (settings access)            Si (yes) | ON  |                                         |
  +---------------------------------------+-----+-----+-----------------------------------+
  | Seteo del Pozo               No       |     | OFF |                                   |
  | (Jackpot settings)           Si (yes) |     | ON  |                                   |
  +---------------------------------------+-----+-----+-----+-----------------------------+
  | Reset de Audit. Parciales    No       |           | OFF |                             |
  | (Reset short term meters)    Si (yes) |           | ON  |                             |
  +---------------------------------------+-----------+-----+-----+-----------------------+
  | Prog. de Pozo acumulado      No       |                 | OFF |                       |
  | (Jackpot program)            Si (yes) |                 | ON  |                       |
  +---------------------------------------+-----------------+-----+-----+-----------------+
  | Prog. de multiplicadores     No       |                       | OFF |                 |
  | (Multipliers program)        Si (yes) |                       | ON  |                 |
  +---------------------------------------+-----------------------+-----+-----+-----+-----+
  | Not Used                              |                             | OFF | OFF |     |
  |                                       |                             | ON  | ON  |     |
  +---------------------------------------+-----------------------------+-----+-----+-----+
  | Reset total                  No       |                                         | OFF |
  | (Full Reset)                 Si (yes) |                                         | ON  |
  '---------------------------------------+-----------------------------------------+-----'


*********************************************************************

  Program ROM
  Data Lines Scrambling.     

  BUS     ROM

  D0  ->  D1
  D1  ->  D0
  D2  ->  D3
  D3  ->  D2
  D4  ->  D5
  D5  ->  D4
  D6  ->  D7
  D7  ->  D6

*********************************************************************

  Notes:

  - Programming mode is driven through POS#5 controls (joystick + bet button)

  - The roulette numbers lamps multiplexion & animation seems to be drive from
    the DIL28 IC (labeled PLD-02).

  - Identified the PLCC68 CPU as Zilog Z180.
  - Identified the DIL28 IC as ST6265 MCU.
  - Identified the DIL64 IC as Yamaha 9938/58 VDP.

*********************************************************************

  Media files (27c4001)

  00000-085ff    GFX
  08600-0ffff    blank
  10000-7ffff    samples

  Samples are 8-bit unsigned PCM.

*********************************************************************

  I/O:
  
  1x PPI 8255.
  3x 4099 (8-Bit Addressable Latch).
  3x 4512 (8-Channel Data Selector).

      PPI 8255
  .------v------.
  |         PA0 |------> DAC (D7)
  |         PA1 |------> DAC (D6)
  |         PA2 |------> DAC (D5)
  |         PA3 |------> DAC (D4)
  |         PA4 |------> DAC (D3)
  |         PA5 |------> DAC (D2)
  |         PA6 |------> DAC (D1)
  |         PA7 |------> DAC (D0)
  |             |
  |         PB0 |------> I/O 4099 (A0) & 4512 (A)
  |         PB1 |------> I/O 4099 (A1) & 4512 (B)
  |         PB2 |------> I/O 4099 (A2) & 4512 (C)
  |         PB3 |------> I/O 4099 (WD 4099 #0, Players) 
  |         PB4 |------> I/O 4099 (WD 4099 #1, Counters)
  |         PB5 |------> I/O 4099 (WD 4099 #2, unknown)
  |         PB6 |------> I/O 4099 (D)
  |         PB7 |------> PB7
  |             |
  |         PC0 |------> EEPROM (DI)
  |         PC1 |------> EEPROM (CS)
  |         PC2 |------> EEPROM (SK)
  |         PC3 |------\ tied together
  |         PC4 |------/ to PC3.
  |         PC5 |------> I/O 4512 #0 (SO, inputs #0)
  |         PC6 |------> I/O 4512 #1 (SO, inputs #1)
  |         PC7 |------> I/O 4512 #2 (SO, DIP switches)
  '-------------'


*********************************************************************

  Dev notes:

  Currently the program is resetting due to a couple of routines that
  compare writes/reads and jump to offset 0000.
  
  002E5: F3          di
  002E6: ED 5E       im    2
  002E8: 31 F5 FE    ld    sp,$FEF5
  002EB: CD C0 02    call  $02C0
  002EE: 3E 44       ld    a,$44
  002F0: D3 C3       out   ($C3),a
  002F2: D3 C3       out   ($C3),a
  002F4: 3E 5A       ld    a,$5A      ; loads $5A (bitpattern)
  002F6: D3 C0       out   ($C0),a    ; out to DAC (through PPI, port A)
  002F8: DB C0       in    a,($C0)    ; loads from DAC???
  002FA: FE 5A       cp    $5A        ; compare for the same value like it was RAM
  002FC: 28 04       jr    z,$00302   ; match?... jumps over the reset.
  002FE: FB          ei               ; otherwise...
  002FF: C3 00 00    jp    $0000      : RESET.
  00302: 3E A5       ld    a,$A5      ; loads $A5 (inverted bitpattern)
  00304: D3 C0       out   ($C0),a    ; out to DAC (through PPI, port A)
  00306: DB C0       in    a,($C0)    ; loads from DAC???
  00308: FE A5       cp    $A5        ; compare for the same value like it was RAM
  0030A: 28 04       jr    z,$00310   ; match?... jumps over the reset.
  0030C: FB          ei               ; otherwise...
  0030D: C3 00 00    jp    $0000      : RESET.
  00310: AF          xor   a
  00311: D3 C0       out   ($C0),a

*********************************************************************/


#define CPU_CLOCK       XTAL_12_288MHz
#define MCU_CLOCK       XTAL_8MHz
#define VID_CLOCK       XTAL_21_4772MHz

#define VDP_MEM         0x20000  // 4x 4464 (64K x 4 DRAM)


#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "video/v9938.h"
#include "screen.h"


class luckybal_state : public driver_device
{
public:
	luckybal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_v9938(*this, "v9938")
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(luckybal);

	required_device<v9938_device> m_v9938;
	required_device<cpu_device> m_maincpu;
};


/**************************************
*             Memory Map              *
**************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, luckybal_state )
	AM_RANGE(0x0000, 0x57ff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM  // 6264 SRAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io, AS_IO, 8, luckybal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc0, 0xc3) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE("v9938", v9938_device, read, write)
ADDRESS_MAP_END
/*
[:maincpu] ':maincpu' (00006): unmapped io memory write to 0010 = 00 & FF
[:maincpu] ':maincpu' (00009): unmapped io memory read from 0018 & FF
[:maincpu] ':maincpu' (002C9): unmapped io memory write to 003F = 00 & FF
[:maincpu] ':maincpu' (002CE): unmapped io memory write to 0033 = A0 & FF
[:maincpu] ':maincpu' (002D2): unmapped io memory write to 0034 = 00 & FF
[:maincpu] ':maincpu' (049C3): unmapped io memory write to 0032 = 00 & FF
[:maincpu] ':maincpu' (0498F): unmapped io memory write to 000C = 1B & FF
[:maincpu] ':maincpu' (04993): unmapped io memory write to 000D = 00 & FF
[:maincpu] ':maincpu' (04998): unmapped io memory write to 000E = 1B & FF
[:maincpu] ':maincpu' (0499C): unmapped io memory write to 000F = 00 & FF
[:maincpu] ':maincpu' (049A1): unmapped io memory write to 0014 = 00 & FF
[:maincpu] ':maincpu' (049A6): unmapped io memory write to 0015 = 18 & FF
[:maincpu] ':maincpu' (049AA): unmapped io memory write to 0016 = 00 & FF
[:maincpu] ':maincpu' (049AF): unmapped io memory write to 0017 = 18 & FF
[:maincpu] ':maincpu' (049B2): unmapped io memory read from 0010 & FF
[:maincpu] ':maincpu' (049B9): unmapped io memory write to 0010 = 04 & FF
[:maincpu] ':maincpu' (04A65): unmapped io memory write to 000A = 04 & FF
[:maincpu] ':maincpu' (049C7): unmapped io memory write to 0002 = 00 & FF
[:maincpu] ':maincpu' (049D1): unmapped io memory write to 0000 = 64 & FF
[:maincpu] ':maincpu' (002F2): unmapped io memory write to 00C3 = 44 & FF
[:maincpu] ':maincpu' (002F4): unmapped io memory write to 00C3 = 44 & FF
[:maincpu] ':maincpu' (002F8): unmapped io memory write to 00C0 = 5A & FF
[:maincpu] ':maincpu' (002FA): unmapped io memory read from 00C0 & FF
*/
/*
;*********** PPI 8255 *******
PORTCN    EQU  C0H    ;80H C
P_AUDIO	  EQU  C0H    ;Port A ---> DAC
PORTIN    EQU  C2H    ;Port C (High nibble) Inputs. (4=EE, 5=PLATE, 6=KEY, 7=DIP)
PORT_EE	  EQU  C2H    ;Port C (Low nibble) EEPROM.  (0=DI, 1=CS, 2=SK)
PORTIO    EQU  C1H    ;Port B I/O 4099 (0=A0, 1=A1, 2=A2, 3=WJ, 4=WC, 5=WP, 6=D, 7=LED)
PPI_CTRL  EQU  C3H    ;Mode.

;---VDP
PORT0     EQU  E0H
PORT1     EQU  E1H
PORT2     EQU  E2H
PORT3     EQU  E3H

;******* 74LS273 MEMORY MAPPED **********
M_MAP     EQU  90H    ; [A]= Bank to select (BIT6=MEM, BIT7=EN_NMI)

*/

/**************************************
*            Input Ports              *
**************************************/

static INPUT_PORTS_START( luckybal )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1_01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1_02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1_04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1_08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1_10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1_20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1_40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1_80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/**************************************
*           Machine Driver            *
**************************************/

static MACHINE_CONFIG_START( luckybal )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	
	/* video hardware */
	MCFG_V9938_ADD("v9938", "screen", VDP_MEM, VID_CLOCK)
	MCFG_V99X8_INTERRUPT_CALLBACK(INPUTLINE("maincpu", 0))
	MCFG_V99X8_SCREEN_ADD_NTSC("screen", "v9938", VID_CLOCK)

MACHINE_CONFIG_END


/**************************************
*              ROM Load               *
**************************************/

ROM_START( luckybal )  // luckyball96 v350-627
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lb97_627_m27c512.u15", 0x00000, 0x10000, CRC(c1bcffef) SHA1(da5db0ab0555cd98ff8e0206c1ee4ebd3d7447ef) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD( "lb97_627_m27c4001.u20", 0x00000, 0x80000, CRC(dbc45c4a) SHA1(720c6861fa2bfa9c9dad69d687f12bd1e0a71afb) )
ROM_END


ROM_START( luckybala )  // luckyball96 v350-626
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lb97_626_m27c512.u15", 0x00000, 0x10000, CRC(d25588c1) SHA1(fc24b1e869d726d2e2ab8cd38ab6304fdca6dfa9) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD( "lb96_sounds_001_m27c4001.u20", 0x00000, 0x80000, CRC(dbc45c4a) SHA1(720c6861fa2bfa9c9dad69d687f12bd1e0a71afb) )
ROM_END


ROM_START( luckybalb )  // luckyball96 v350-623
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lb96_625_m27c512.u15", 0x00000, 0x10000, CRC(2017edf7) SHA1(0208423b116aeb139c5db193b567bc79fd2a21ac) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD( "lb96_625_m27c4001.u20", 0x00000, 0x80000, CRC(dbc45c4a) SHA1(720c6861fa2bfa9c9dad69d687f12bd1e0a71afb) )
ROM_END


ROM_START( luckybalc )  // luckyball96 v350-616
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_a18e_m27c512.u15", 0x00000, 0x10000, CRC(9bdf0243) SHA1(e353a86c4b020784d084c4fa12feb6ccd8ebd77b) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD( "nosticker_af7c_m27c4001.u20", 0x00000, 0x80000, CRC(de796370) SHA1(6edee4de330a65ad6b2dbe59c7ac0ddecb3ed0f1) )
ROM_END


/************************************
*            Driver Init            *
************************************/

DRIVER_INIT_MEMBER(luckybal_state, luckybal)
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0;
	int i;

	for (i = start; i < size; i++)
	{
		rom[i] = bitswap<8>(rom[i], 6, 7, 4, 5, 2, 3, 0, 1);
	}
}


/**************************************
*           Game Driver(s)            *
**************************************/

/*    YEAR  NAME        PARENT    MACHINE   INPUT     STATE           INIT      ROT    COMPANY          FULLNAME                         FLAGS  */
GAME( 1996, luckybal,   0,        luckybal, luckybal, luckybal_state, luckybal, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 627)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1996, luckybala,  luckybal, luckybal, luckybal, luckybal_state, luckybal, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 626)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1996, luckybalb,  luckybal, luckybal, luckybal, luckybal_state, luckybal, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 623)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1996, luckybalc,  luckybal, luckybal, luckybal, luckybal_state, luckybal, ROT0, "Sielcon Games", "Lucky Ball 96 (Ver 3.50 - 616)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
