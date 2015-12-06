// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/**************************************************************************************************

  MEGA DOUBLE POKER (BLITZ SYSTEM INC.)
  -------------------------------------

  Driver by Roberto Fresca.
  Based on Golden Poker Double Up driver.


  Games running on this hardware:

  * Mega Double Poker (conversion kit, set 1). 1990, Blitz System Inc.
  * Mega Double Poker (conversion kit, set 2). 1990, Blitz System Inc.


***************************************************************************************************

  Mega Double Poker is distributed as standalone PCB, or as upgrade kit for
  modified Golden Poker boards. The game has an undumped MC68705P5 MCU, so
  the emulation should be impossible till this device can be decapped and
  properly dumped.


***************************************************************************************************

  Hardware Notes (Mega Double Poker, kit):

  - CPU:            1x R6502AP.
  - MCU:            1x MC68705P5S.
  - Video:          1x MC6845.
  - RAM:            4x uPD2114LC or similar
  - I/O             2x 6821 PIAs.
  - prg ROMs:       2x 2732 (32KB) or similar.
  - gfx ROMs:       3x 2732 (32KB) or similar.
  - sound:          (discrete).
  - battery backup: 2x S8423


  PCB Layout: Main board.
   _______________________________________________________________________________
  |   _________                                                                   |
  |  |         |               -- DIP SW x8 --                                    |
  |  | Battery |   _________   _______________   _________  _________   ________  |
  |  |   055   |  | 74LS32  | |1|2|3|4|5|6|7|8| | HCF4011 || HCF4096 | | LM339N | |
  |  |_________|  |_________| |_|_|_|_|_|_|_|_| |_________||_________| |________| |
  |       _________     _________   _________   _________                         |
  |      | 74LS138 |   | S-8423  | | 74LS08N | | 74LS(XX)|                        |
  |      |_________|   |_________| |_________| |_________|                        |
  |  _______________    _________   ____________________                      ____|
  | |               |  | S-8423  | |                    |                    |
  | |     2732      |  |_________| |       6502P        |                    |
  | |_______________|   _________  |____________________|                    |
  |  _______________   |  7432   |  ____________________                     |____
  | |               |  |_________| |                    |                     ____|
  | |     2732      |   _________  |       6821P        |                     ____|
  | |_______________|  | 74LS157 | |____________________|                     ____|
  |  _______________   |_________|  ____________________                      ____|
  | |               |   _________  |                    |                     ____|
  | |     2732      |  | 74LS157 | |       6821P        |                     ____|
  | |_______________|  |_________| |____________________|                     ____|
  |  _______________    _________   ____________________                      ____|
  | |               |  | 74LS157 | |                    |                     ____|
  | |     2732      |  |_________| |       6845SP       |                     ____|
  | |_______________|   _________  |____________________|                     ____|
  |                    | 2114-LC |                                            ____| 28x2
  |                    |_________|                                            ____| connector
  |       _________     _________                                             ____|
  |      | 74LS245 |   | 2114-LC |                                            ____|
  |      |_________|   |_________|                                            ____|
  |       _________     _________               _________                     ____|
  |      | 74LS245 |   | 2114-LC |             | 74LS174 |                    ____|
  |      |_________|   |_________|             |_________|                    ____|
  |  ________________   _________   _________   _________                     ____|
  | |                | | 2114-LC | | 74LS08H | | TI (XX) | <-- socketed.      ____|
  | |      2716      | |_________| |_________| |_________|       PROM         ____|
  | |________________|              _________   _________                     ____|
  |  ________________              | 74LS04P | | 74LS174 |                    ____|
  | |                |             |_________| |_________|                    ____|
  | |      2716      |              _________   _________                     ____|
  | |________________|             | 74166P  | | 74LS86C |                    ____|
  |  ________________              |_________| |_________|                    ____|
  | |                |              _________    _______                     |
  | |      2716      |             | 74166P  |  | 555TC |                    |
  | |________________|             |_________|  |_______|                    |
  |  ________________                                                        |____
  | |                |                                                        ____|
  | |      2716      |              _________   _________      ________       ____| 5x2
  | |________________|             | 74166P  | |  7407N  |    | LM380N |      ____| connector
  |                                |_________| |_________|    |________|      ____|
  |  ________  ______               _________   _________      ___            ____|
  | | 74LS04 || osc. |             | 74LS193 | |  7407N  |    /   \          |
  | |________||10 MHz|             |_________| |_________|   | POT |         |
  |           |______|                                        \___/          |
  |__________________________________________________________________________|


  PCB Layout: Daughterboard.
   ________________________________________________________
  |                                ::::::::::::::::::::    |
  |   __________                     40-pin connector      |
  |  | GD74LS04 | U8                                       |
  |  |__________|                                          |
  |   ____________                ______________________   |
  |  |KS74HCTLS08N| U9           |                      |  |
  |  |____________|              |     R6502AP (U6)     |  |
  |   ____________               |______________________|  |
  |  |KS74HCTLS32N| U10                                    |
  |  |____________|                                        |
  |   _____________                                        |
  |  |KS74HCTLS74AN| U11                            _______|
  |  |_____________|         _______________       |
  |   _____________         |  Unknown RAM  |      |
  |  |KS74HCTLS139N| U12    |     (U5)      |      |
  |  |_____________|        |_______________|      |
  |                          _________________     |
  |                         |   27C256 (U2)   |    |
  |                         |     MEGA-2      |    |
  |                         |_________________|    |
  |                          _________________     |
  |                         |   27C256 (U3)   |    |
  |     _____________       |     MEGA-3      |    |
  |    |KS74HCTLS374N|      |_________________|    |
  |    |_____________|       _________________     |
  |      U13                |Empty Socket (U4)|    |
  |                         |  'SPARE EPROM'  |    |
  |   _______               |_________________|    |
  |  |       |                                     |
  |  |MC68705|   ____________                      |
  |  |  P5S  |  |KS74HCTLS86N| U14                 |
  |  |       |  |____________|                     |
  |  |MEGA-1 |   ____________     _____________    |
  |  |       |  |KS74HCTLS86N|   |KS74HCTLS245N|   |
  |  | (U1)  |  |____________|   |_____________|   |
  |  |_______|    U15              U16  __         |
  |                                    /--\ BLITZ  |
  | Model B0-BL-01B                    \__/ SYSTEM |
  |________________________________________________|


  Connections... (pins in parenthesis)
  ------------------------------------

  The following diagrams are still incomplete and could have errors.
  At simple sight, mega-2.u2 & mega-3.u3 ROMs are sharing the same
  addressing space, but /CE line for both devices are connected to
  the same places... Need to be traced from the scratch.

  Also CPU lines A14 & A15 should be traced to know the addressing
  system accuratelly.


                                                     CPU  R6502AP (U6)
                                                   .--------\ /--------.
  MCU (01-05-07), MEGA-2 (14-20), MEGA-3 (14-20) --|01 VSS   '  /RES 40|--
                                        MCU (02) --|02 RDY    PH2(O) 39|--
                                        MCU (04) --|03 PH1(O)    /SO 38|--
                                                 --|04 /IRQ   PH0(I) 37|--
                                                 --|05 (NC)     (NC) 36|--
  MCU (03-06), MEGA-2 (01-27-28), MEGA-3 (01-28) --|06 /NMI     (NC) 35|--
                                                 --|07 SYNC      R/W 34|--
  MCU (03-06), MEGA-2 (01-27-28), MEGA-3 (01-28) --|08 VCC        D0 33|-- MEGA-2 (11)
                        MEGA-2 (10), MEGA-3 (10) --|09 A0         D1 32|-- MEGA-2 (12)
                        MEGA-2 (09), MEGA-3 (09) --|10 A1         D2 31|--
                        MEGA-2 (08), MEGA-3 (08) --|11 A2         D3 30|-- MEGA-2 (15)
                        MEGA-2 (07), MEGA-3 (07) --|12 A3         D4 29|-- MEGA-2 (16)
                        MEGA-2 (06), MEGA-3 (06) --|13 A4         D5 28|-- MEGA-2 (17)
                        MEGA-2 (05), MEGA-3 (05) --|14 A5         D6 27|-- MEGA-2 (18)
                        MEGA-2 (04), MEGA-3 (04) --|15 A6         D7 26|-- MEGA-2 (19)
                        MEGA-2 (03), MEGA-3 (03) --|16 A7        A15 25|--
                        MEGA-2 (25), MEGA-3 (25) --|17 A8        A14 24|--
                        MEGA-2 (24), MEGA-3 (24) --|18 A9        A13 23|-- MEGA-2 (26), MEGA-3 (26)
                        MEGA-2 (21), MEGA-3 (21) --|19 A10       A12 22|-- MEGA-2 (02), MEGA-3 (02)
                        MEGA-2 (23), MEGA-3 (23) --|20 A11       VSS 21|-- MCU (01-05-07), MEGA-2 (14-20), MEGA-3 (14-20)
                                                   '-------------------'


                                      MCU MC68705P5S (U1)
                                     .--------\ /--------.
  MEGA-2 (12, 20), MEGA-3 (12, 20) --|01 VSS   '  /RES 28|--
                          CPU (02) --|02 /INT      PA7 27|--
  MEGA-2 (01, 28), MEGA-3 (01, 28) --|03 VCC       PA6 26|--
                          CPU (03) --|04 EXTAL     PA5 25|--
  MEGA-2 (14, 20), MEGA-3 (14, 20) --|05 XTAL      PA4 24|--
  MEGA-2 (27, 28), MEGA-3 (27, 28) --|06 VPP       PA3 23|--
  MEGA-2 (14, 20), MEGA-3 (14, 20) --|07 TIMER     PA2 22|--
                                   --|08 PC0       PA1 21|--
                                   --|09 PC1       PA0 20|--
                                   --|10 PC2       PB7 19|--
                                   --|11 PC3       PB6 18|--
                                   --|12 PB0       PB5 17|--
                                   --|13 PB1       PB4 16|--
                                   --|14 PB2       PB3 15|--
                                     '-------------------'


                         MEGA-2 27C256 (U2)                               MEGA-3 27C256 (U3)
                        .-------\ /-------.                              .-------\ /-------.
                      --|01 VPP  '  VCC 28|--                          --|01 VPP  '  VCC 28|--
                      --|02 A12     A14 27|--                          --|02 A12     A14 27|--
                      --|03 A7      A13 26|--                          --|03 A7      A13 26|--
                      --|04 A6       A8 25|--                          --|04 A6       A8 25|--
                      --|05 A5       A9 24|--                          --|05 A5       A9 24|--
                      --|06 A4      A11 23|--                          --|06 A4      A11 23|--
                      --|07 A3      /OE 22|--                          --|07 A3      /OE 22|--
                      --|08 A2      A10 21|--  .-- MCU (01-05-07)      --|08 A2      A10 21|--  .-- MCU (01-05-07)
                      --|09 A1      /CE 20|----+-- MEGA-3 (14-20)      --|09 A1      /CE 20|----+-- MEGA-2 (14-20)
                      --|10 A0       D7 19|--  '-- CPU (01-21)         --|10 A0       D7 19|--  '-- CPU (01-21)
                      --|11 D0       D6 18|--                          --|11 D0       D6 18|--
                      --|12 D1       D5 17|--                          --|12 D1       D5 17|--
  MCU (01-05-07) --.  --|13 D2       D4 16|--      MCU (01-05-07) --.  --|13 D2       D4 16|--
  MEGA-3 (14-20) --+----|14 GND      D3 15|--      MEGA-2 (14-20) --+----|14 GND      D3 15|--
     CPU (01-21) --'    '-----------------'           CPU (01-21) --'    '-----------------'


***************************************************************************************************

  ------------------------------------------------
  ***  Memory Map (from Golden Poker hardware) ***
  ------------------------------------------------

  $0000 - $00FF   RAM     ; Zero Page (pointers and registers)

  $0100 - $01FF   RAM     ; 6502 Stack Pointer.

  $0800 - $0801   MC6845  ; MC6845 use $0800 for register addressing and $0801 for register values.

  $0844 - $0847   PIA0    ; Muxed inputs and lamps.
  $0848 - $084B   PIA1    ; Sound writes and muxed inputs selector.

  $1000 - $13FF   Video RAM.
  $1800 - $1BFF   Color RAM.

  $4000 - $7FFF   ROM

  $8000 - $FFFF           ; Mirrored from $0000 - $7FFF due to lack of A15 line connection.


****************************************************************************************************

  Driver Updates:
  --------------

  [2010-07-30]

  * Initial release.
  * Preliminary memory map.
  * Hooked both PIAs, but need more analysis to confirm the offsets.
  * Accurate graphics and color decode.
  * Added main PCB and daughterboard layouts.
  * Added partial docs and diagrams about the CPU/MCU/ROMs addressing.
  * Added debug and technical notes.

***************************************************************************************************/

#define MASTER_CLOCK    XTAL_10MHz
#define CPU_CLOCK       (MASTER_CLOCK/16)

#include "emu.h"
#include "cpu/m6502/m6502.h"
//#include "cpu/m6805/m6805.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/discrete.h"


class blitz_state : public driver_device
{
public:
	blitz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	int m_mux_data;
	DECLARE_WRITE8_MEMBER(megadpkr_videoram_w);
	DECLARE_WRITE8_MEMBER(megadpkr_colorram_w);
	DECLARE_READ8_MEMBER(megadpkr_mux_port_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(lamps_a_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(blitz);
	UINT32 screen_update_megadpkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*********************************************
*               Video Hardware               *
*********************************************/

WRITE8_MEMBER(blitz_state::megadpkr_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(blitz_state::megadpkr_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(blitz_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    ---- --x-   tiles bank.
    ---- ---x   tiles extended address (MSB).
    xx-- ----   unused.
*/
	int attr = m_colorram[tile_index];
	int code = ((attr & 1) << 8) | m_videoram[tile_index];
	int bank = (attr & 0x02) >> 1;  /* bit 1 switch the gfx banks */
	int color = (attr & 0x3c) >> 2; /* bits 2-3-4-5 for color */

	SET_TILE_INFO_MEMBER(bank, code, color, 0);
}


void blitz_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(blitz_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 blitz_state::screen_update_megadpkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


PALETTE_INIT_MEMBER(blitz_state, blitz)
{
	const UINT8 *color_prom = memregion("proms")->base();
/*
    This hardware has a feature called BLUE KILLER.
    Using the original intensity line, the PCB has a bridge
    that allow (as default) turn the background black.

    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   blue killer.
    xxxx ----   unused.
*/
	int i;

	/* 0000KBGR */

	if (color_prom == nullptr) return;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b, bk;

		/* blue killer (from schematics) */
		bit3 = (color_prom[i] >> 3) & 0x01;
		bk = bit3;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		r = (bit0 * 0xff);

		/* green component */
		bit1 = (color_prom[i] >> 1) & 0x01;
		g = (bit1 * 0xff);

		/* blue component */
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = bk * (bit2 * 0xff);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*******************************************
*               R/W Handlers               *
*******************************************/


/* Inputs (buttons) are multiplexed.
   There are 4 sets of 5 bits each and are connected to PIA0, portA.
   The selector bits are located in PIA1, portB (bits 4-7).
*/
READ8_MEMBER(blitz_state::megadpkr_mux_port_r)
{
	switch( m_mux_data & 0xf0 )     /* bits 4-7 */
	{
		case 0x10: return ioport("IN0-0")->read();
		case 0x20: return ioport("IN0-1")->read();
		case 0x40: return ioport("IN0-2")->read();
		case 0x80: return ioport("IN0-3")->read();
	}
	return 0xff;
}


WRITE8_MEMBER(blitz_state::mux_w)
{
	m_mux_data = data ^ 0xff;   /* inverted */
}


/***** Lamps & Counters wiring *****/

WRITE8_MEMBER(blitz_state::lamps_a_w)
{
//  output_set_lamp_value(0, 1 - ((data) & 1));         /* Lamp 0 */
//  output_set_lamp_value(1, 1 - ((data >> 1) & 1));    /* Lamp 1 */
//  output_set_lamp_value(2, 1 - ((data >> 2) & 1));    /* Lamp 2 */
//  output_set_lamp_value(3, 1 - ((data >> 3) & 1));    /* Lamp 3 */
//  output_set_lamp_value(4, 1 - ((data >> 4) & 1));    /* Lamp 4 */

//  popmessage("written : %02X", data);
//  coin_counter_w(machine(), 0, data & 0x40);    /* counter1 */
//  coin_counter_w(machine(), 1, data & 0x80);    /* counter2 */
//  coin_counter_w(machine(), 2, data & 0x20);    /* counter3 */
}


WRITE8_MEMBER(blitz_state::sound_w)
{
	/* 555 voltage controlled */
	logerror("Sound Data: %2x\n",data & 0x0f);
}


/*********************************************
*           Memory Map Information           *
*********************************************/

static ADDRESS_MAP_START( megadpkr_map, AS_PROGRAM, 8, blitz_state )
//  ADDRESS_MAP_GLOBAL_MASK(0x7fff) // seems that hardware is playing with A14 & A15 CPU lines...

	AM_RANGE(0x0000, 0x07ff) AM_RAM //AM_SHARE("nvram")   /* battery backed RAM */
//  AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_device, address_w)
//  AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x0844, 0x0847) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x0848, 0x084b) AM_DEVREADWRITE("pia1", pia6821_device, read, write)

/*  There is another set of PIAs controlled by the code.
    Maybe they are just mirrors...

    AM_RANGE(0x10f4, 0x10f7) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
    AM_RANGE(0x10f8, 0x10fb) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
*/
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(megadpkr_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(megadpkr_colorram_w) AM_SHARE("colorram")

	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*

  0844-0847 / 0848-084b --> PIAS?
  10f4-10f7 / 10f8-10fb --> PIAS?

  Also 47f4-47f7 & 47f8-47fb are treated as PIA offsets... (code at $6eaf and $6f74)


  $80ff --> register, comm ack, or bankswitching?


  FE99: 78            sei            ; set IRQ
  FE9A: 4E 2F 02      lsr  $022F
  FE9D: D8            cld
  FE9E: A2 FF         ldx  #$FF
  FEA0: 9A            txs
  FEA1: A9 00         lda  #$00
  FEA3: 8D 25 02      sta  $0225
  FEA6: A9 F7         lda  #$F7      ; load 0xf7
  FEA8: 9D 00 80      sta  $8000,x   ; store at $80ff ($00ff if A15 is disconnected)
  FEAB: 20 4E EE      jsr  $EE4E     ; delay for sync
  FEAE: AD 98 46      lda  $4698     ; load from $4698
  FEB1: C9 A2         cmp  #$A2      ; compare with 0xa2
  FEB3: D0 F1         bne  $FEA6     ; if not ---> again to $fea6
  FEB5: A9 F4         lda  #$F4      ; load 0xf4
  FEB7: 9D 00 80      sta  $8000,x   ; store at $80ff ($00ff if A15 is disconnected)
  FEBA: 20 4E EE      jsr  $EE4E     ; delay for sync
  FEBD: AD 98 46      lda  $4698     ; load from $4698
  FEC0: C9 45         cmp  #$45      ; compare with 0x45
  FEC2: D0 E2         bne  $FEA6     ; if not ---> again to $fea6
  FEC4: 20 4E EE      jsr  $EE4E     ; delay for sync
  FEC7: 20 4E EE      jsr  $EE4E     ; delay for sync
  FECA: A9 00         lda  #$00
  FECC: 85 C1         sta  $C1
  FECE: 85 C2         sta  $C2
  FED0: 85 E0         sta  $E0
  FED2: A9 10         lda  #$10      ; load 0x10
  FED4: 85 1C         sta  $1C
  FED6: 9D 00 80      sta  $8000,x   ; store at $80ff ($00ff if A15 is disconnected)
  FED9: 20 4E EE      jsr  $EE4E     ; delay for sync
  FEDC: A9 00         lda  #$00      ; load 0x00
  FEDE: 9D 00 80      sta  $8000,x   ; store at $80ff ($00ff if A15 is disconnected)
  FEE1: 20 4E EE      jsr  $EE4E     ; delay for sync
  FEE4: 20 4E EE      jsr  $EE4E     ; delay for sync
  FEE7: 4C 3F 62      jmp  $623F     ; transfer the control to $623f (no valid code there. RTS in the alt program)


  Some routines are writing to $47xx that should be ROM space.

  Also some pieces of code are copying code to the first 0x80 bytes of the stack and execute from there.
  The code is so obfuscated. The copied code is using undocumented opcodes as LAX.

  EEDA: A2 00         ldx  #$00
  EEDC: BD 22 EF      lda  $EF22,x
  EEDF: 9D 00 01      sta  $0100,x
  EEE2: E8            inx
  EEE3: E0 80         cpx  #$80
  EEE5: D0 F5         bne  $EEDC
  EEE7: 78            sei
  EEE8: 20 00 01      jsr  $0100
  EEEB: 58            cli

  Here the copied subroutine:

  EF22: 38            sec
  EF23: B0 08         bcs  $EF2D
  EF25: C5 3A         cmp  $3A
  EF27: A3 5C         lax  ($5C,x)  ------> Indexed LAX???
  EF29: C5 3A         cmp  $3A
  EF2B: A3 5C         lax  ($5C,x)  ------> Indexed LAX???
  EF2D: AD F7 47      lda  $47F7
  EF30: A2 00         ldx  #$00
  EF32: BD 03 01      lda  $0103,x
  EF35: A0 08         ldy  #$08
  EF37: 8D F7 47      sta  $47F7
  EF3A: 6A            ror  a
  EF3B: 88            dey
  EF3C: D0 F9         bne  $EF37
  EF3E: E8            inx
  EF3F: E0 08         cpx  #$08
  EF41: D0 EF         bne  $EF32
  EF43: A2 00         ldx  #$00
  EF45: A0 08         ldy  #$08
  EF47: AD F7 47      lda  $47F7
  EF4A: 4A            lsr  a
  EF4B: 76 B0         ror  $B0,x
  EF4D: 88            dey
  EF4E: D0 F7         bne  $EF47
  EF50: E8            inx
  EF51: E0 08         cpx  #$08
  EF53: D0 F0         bne  $EF45
  EF55: A9 00         lda  #$00
  EF57: 85 B0         sta  $B0
  EF59: 60            rts

*/


/*
static ADDRESS_MAP_START( mcu_map, AS_PROGRAM, 8, blitz_state )
    ADDRESS_MAP_GLOBAL_MASK(0x7ff)
    AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END
*/


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( megadpkr )
	/* Multiplexed - 4x5bits */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-8") PORT_CODE(KEYCODE_L)

	PORT_START("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,6),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( megadpkr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 16 )
GFXDECODE_END


/**********************************************************
*                 Discrete Sound Routines                 *
***********************************************************

    Discrete sound circuitry.
    -------------------------

           +12V                            .--------+---------------.  +5V
            |                              |        |               |   |
         .--+                              |        Z             +-------+
         |  |                             8|        Z 1K          | PC617 |
         Z  Z                         +----+----+   Z             +-------+
    330K Z  Z 47K                     |   VCC   |3  |   20K   1uF  |     |
         Z  Z             1.7uF       |        Q|---|--ZZZZZ--||---+     Z   1uF         6|\
         |  |            .-||-- GND   |         |7  |              |  1K Z<--||--+--------| \
         |  |   30K      |           4|      DIS|---+              Z     Z       |   LM380|  \  220uF
  PA0 ---|--|--ZZZZZ--.  |      .-----|R        |   |         2.2K Z     |       Z        |  8>--||----> Audio Out.
         |  |         |  |      |     |   555   |   Z              Z     |   10K Z     7+-|  /
         |  |   15K   |  |      |    5|         |   Z 10K          |     |       Z     3+-| /-.1   .---> Audio Out.
  PA1 ---+--|--ZZZZZ--+--+------|-----|CV       |   Z              |     |       |     2+-|/  |    |
            |         |         |    2|         |6  |              |     |       |      |     |    |
            |  7.5K   |         |  .--|TR    THR|---+---||---------+-----+-------+------+-||--'    |
  PA2 ------+--ZZZZZ--'  |\     |  |  |   GND   |   |                                   |          |
                         | \    |  |  +----+----+   |  .1uF                             | 10uF     |
                        9|  \   |  |      1|        |                                   |          |
  PA3 -------------------|  8>--'  |      -+-       |                                   +----------'
                    4069 |  /      |      GND       |                                   |
                         | /       |                |                                  -+-
                         |/        '----------------'                                  GND
*/


/*********************************************
*              Machine Drivers               *
*********************************************/

static MACHINE_CONFIG_START( megadpkr, blitz_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(megadpkr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blitz_state,  nmi_line_pulse)

//  MCFG_CPU_ADD("mcu", M68705, CPU_CLOCK) /* unknown */
//  MCFG_CPU_PROGRAM_MAP(mcu_map)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(blitz_state, megadpkr_mux_port_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(blitz_state, lamps_a_w))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("SW1"))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(blitz_state, sound_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(blitz_state, mux_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((32)*8, (32)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(blitz_state, screen_update_megadpkr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CPU_CLOCK)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", megadpkr)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(blitz_state, blitz)
MACHINE_CONFIG_END


/*********************************************
*                  Rom Load                  *
*********************************************/

/******************************************

  MEGA DOUBLE POKER
  BLITZ SYSTEM INC.

  Conversion kit for Golden Poker boards.

******************************************/

ROM_START( megadpkr )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program ROM */
	ROM_LOAD( "mega-2.u2",  0x8000, 0x8000, CRC(2b133b92) SHA1(97bc21c42897cfd13c0247e239aebb18f73cde91) )

	/* sharing the same space, but not totally understood... banked through MCU? */
	ROM_REGION( 0x10000, "cpubank", 0 )
	ROM_LOAD( "mega-3.u3",  0x8000, 0x8000, CRC(ff0a46c6) SHA1(df053c323c0e2dd0e41e22286d38e889bfda3aa5) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the undumped 68705 microcontroller */
	ROM_LOAD( "mega-1.u1",  0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, nullptr ) /* filling the R-G bitplanes */
	ROM_LOAD( "car1.5a",    0x2000, 0x1000, CRC(29e244d2) SHA1(c309a5ee6922bf2752d218c134edb3ef5f808afa) )    /* text chars / cards deck gfx, bitplane3 */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "car3.2a",    0x0000, 0x1000, CRC(819c06c4) SHA1(45b874554fb487173acf12daa4ff99e49e335362) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "car2.4a",    0x1000, 0x1000, CRC(41eec680) SHA1(3723f66e1def3908f2e6ba2989def229d9846b02) )    /* cards deck gfx, bitplane2 */
	ROM_COPY( "gfx1",   0x2800, 0x2000, 0x0800 )    /* cards deck gfx, bitplane3. found in the 2nd quarter of the chars rom */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "m3-7611-5.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Manufacturer : Blitz system
  Game name :    Mega Double Poker
  Platform  :    Bonanza golden poker interface

  BoardID
  BO-BL-01

  Protection:    U11  MC68705P5S  microcontroller with window

  Main CPU:
  U6  UM6502
  U5  MK48T02B-15   time/clock backup RAM

  U2.bin  27C256 ROM
  U3.bin  27C256 ROM

  Graphics IC
  car1_5a.bin  27C32 ROM
  car2_4a.bin  27C32 ROM
  car3_2a.bin  27C32 ROM

  note : MC68705P5S is protected
*/

ROM_START( megadpkrb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program ROM */
	ROM_LOAD( "u2.bin", 0x8000, 0x8000, CRC(0efdf472) SHA1(4b1ae10427c2ae8d7cbbe525a6b30973372d4420) )

	/* sharing the same space, but not totally understood... banked through MCU? */
	ROM_REGION( 0x10000, "cpubank", 0 )
	ROM_LOAD( "u3.bin", 0x8000, 0x8000, CRC(c973e345) SHA1(aae9da8cbaf0cf07086e5acacf9052e49fbdd896) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the undumped 68705 microcontroller */
	ROM_LOAD( "u11.bin",  0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, nullptr ) /* filling the R-G bitplanes */
	ROM_LOAD( "car1_5a.bin",    0x2000, 0x1000, CRC(29e244d2) SHA1(c309a5ee6922bf2752d218c134edb3ef5f808afa) )    /* text chars / cards deck gfx, bitplane3 */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "car3_2a.bin",    0x0000, 0x1000, CRC(819c06c4) SHA1(45b874554fb487173acf12daa4ff99e49e335362) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "car2_4a.bin",    0x1000, 0x1000, CRC(41eec680) SHA1(3723f66e1def3908f2e6ba2989def229d9846b02) )    /* cards deck gfx, bitplane2 */
	ROM_COPY( "gfx1",   0x2800, 0x2000, 0x0800 )    /* cards deck gfx, bitplane3. found in the 2nd quarter of the chars rom */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "m3-7611-5.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/*********************************************
*                Game Drivers                *
*********************************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     STATE          INIT  ROT     COMPANY              FULLNAME                                    FLAGS */
GAME( 1990, megadpkr,  0,        megadpkr, megadpkr, driver_device, 0,    ROT0,  "Blitz System Inc.", "Mega Double Poker (conversion kit, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1990, megadpkrb, megadpkr, megadpkr, megadpkr, driver_device, 0,    ROT0,  "Blitz System Inc.", "Mega Double Poker (conversion kit, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
