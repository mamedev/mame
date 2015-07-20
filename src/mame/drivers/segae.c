// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega System E */

/*


 Sega System 'E' is a piece of hardware used for a couple of Arcade Games
 produced by Sega in the mid 80's. It's roughly based on their Sega Master System
 home console unit, using the same '315-5124' VDP (actually in this case 2 of
 them)

 An interesting feature of the system is that the CPU is contained on the ROM
 board, the MAIN System E board contains the Graphics processor, this opens the
 possibility for using processors other than the Standard Z80 to run the main
 game code on; several games have an encrypted Z80 module instead. However, the
 system as a whole is very Z80-centric; using a completely non-Z80 processor
 would require the addition of glue logic to the rom board to make the cpu
 'look' like a Z80 to the rest of the system.

 Also interesting is each VDP has double the Video RAM found on the SMS console
 this is banked through Port Writes, the System also allows for the Video RAM
 to be written directly, bypassing the usual procedure of writing to it via the
 '315-5124' data port, it can not however be read directly, the same area used
 for writing is used to access banked ROMs when reading

 Pretty much everything on this hardware is done through port accesses, the
 main memory map consists of simply ROM, BANKED ROM / BANKED RAM, RAM

********************************************************************************

    ROMs + CPU Board (32kb ROMs)

    IC 07 (Good)    IC 05 (Good)
    IC 04 (Good)    IC 03 (Good)
    IC 02 (Good)

    (834-5803) MAIN Board (8kb RAMs)

    IC 49 (Good)    IC 55 (Good)    System RAM (0xc000 - 0xffff)
    IC 03 (Good)    IC 07 (Good)    Front Layer VRAM (Bank 1)   Port F7 -0------
    IC 04 (Good)    IC 08 (Good)    Front Layer VRAM (Bank 2)   Port F7 -1------
    IC 01 (Good)    IC 05 (Good)    Back Layer VRAM (Bank 1)    Port F7 0-------
    IC 02 (Good)    IC 06 (Good)    Back Layer VRAM (Bank 2)    Port F7 1-------
    (or at least this is how it appears from HangOnJr's RAMs Test)

    2x (315-5124)'s here too, these are the VDP chips

    PORTS (to be completed)

    0xba - 0xbb r/w     Back Layer VDP
    0xbe - 0xbf r/w     Front Layer VDP

    0xf7 w/o            Banking Controls

    0xe0 r/o            Inputs (Coins, Start Btns)
    0xe1 r/o            Controls (Transformer)
    0xe2 r/o            Controls P2 (Opa Opa)

    0xf2 - 0xf3 r/o     Dipswitches

    0xf8 r/o            Analog Input (Hang On Jr, Riddle of Pythagoras)

    0x7e r/o            V Counter (vertical beam pos in scanlines)
    0x7f r/o            H Counter (horizontal beam pos in 'pixel clock cycles')

********************************************************************************

Sega System E Hardware Overview
Sega, 1985-1988

This PCB is essentially a Sega Master System home console unit, but using
two '315-5124' VDPs and extra RAM.
The CPU is located on a plug-in board that also holds all of the EPROMs.

The games that run on this hardware include....
Hang-On Jr.             1985
Transformer/Astro Flash 1986
Riddle of Pythagoras    1986
Opa Opa                 1987
Fantasy Zone 2          1988
Tetris                  1988

PCB Layout
----------
834-5803 (C)Sega 1985
|-----------------------------------------------------|
|         D4168      D4168      D4168       D4168     |
|                                                     |
|         D4168      D4168      D4168       D4168     |
|                                                     |
|                                                     |
|    SW1                                              |
|CN1                                                  |
|    SW2                                              |
|                                                     |
|   LED            |---|             |---|            |
|                  | 3 |             | 3 |            |
|                  | 1 |             | 1 |            |
|                  | 5 |             | 5 |            |
|                  | | |             | | |         CN3|
|                  | 5 |             | 5 |     8255   |
|CN4               | 1 |             | 1 |            |
|                  | 2 |             | 2 |            |
|                  | 4 |             | 4 |            |
|                  |---|             |---|            |
|               |--ROM-BOARD-(mounted above here)--|  |
|               |                                  |  |
|               |CN2                     XTAL1     |  |
|               |         D4168                    |  |
|  VOL          |         D4168                    |  |
| LA4460        |----------------------------------|  |
|-----------------------------------------------------|
Notes:
      XTAL1              - 10.7386Mhz
      315-5124 VDP clock - 10.7386MHz
      SN76496 clock      - 3.579533MHz [10.7386/3]
      D4168              - 8k x8 SRAM
      VSync              - 60Hz
      HSync              - 15.58kHz
      CN1                - Connector used for standard controls
      CN2                - connector for CPU/ROM PCB
      CN3                - Connector used for special controls (via a small plug-in interface PCB)
      CN4                - Connector for power

ROM Daughterboard
-----------------
834-6592-01
|--------------------------------------------|
|                                            |
|    |---|                                   |
|C   |   |                           IC6     |
|N   |Z80|                                   |
|2   |   |                                   |
|    |   |   IC2   IC3   IC4   IC5        IC7|
|    |---|                                   |
|     IC1             PAD1 PAD2     PAD3 PAD4|
|--------------------------------------------|
Notes:
       IC1: Z80 clock - 5.3693MHz [10.7386/2]
            On some games this is replaced with a NEC MC-8123 Encrypted CPU Module.
            The clock speed is the same. The MC-8123 contains a Z80 core, plus a RAM chip
            and battery. When the battery dies, the program can no longer be decrypted
            and the PCB does not boot up at all. The battery can not be changed because the
            MC-8123 is sealed, so there is no way to access it.

 IC2 - IC5: 27C256 EPROM (DIP28)

       IC6: 74LS139

       IC7: 27C256 EPROM (DIP28)

    PAD1-4: These are jumper pads used to configure the ROM board for use with the
            Z80 or with the MC8123 CPU.
            PAD1 - Ties Z80 pin 24 (WAIT) to pin1 of the EPROMs at IC2, 3, 4 & 5
            PAD2 - Ties CN2 pin B21 to pin1 of the EPROMs at IC2, 3, 4 & 5
            PAD3 - Ties CN2 pin B21 to pin 2 of the 74LS139 @ IC6
            PAD4 - Ties Z80 pin 24 (WAIT) to pin 2 of the 74LS139 @ IC6

            The pads are configured like this..... (U=Upper, L=Lower)

                                                 |----|      |----|
                                                 |IC6 |      |IC7 |
                                                 |  12|------|22  |
                                                 |    |      |    |
                       IC2   IC3    IC4   IC5    |   1|------|27  |
                       PIN1  PIN1   PIN1  PIN1   |   2|--|   |    |
                        O-----O--+---O------O    |----|  |   |----|
                                 |                       |
                                 |         |----|        |
                              O--+----O    |    O    |---O
            CN2    Z80      PAD1U   PAD2U  |  PAD3U  | PAD4U
            B21    PIN24    PAD1L   PAD2L  |  PAD3L  | PAD4L
             O       O--4.7k--O       O----|    O----|   O
             |                |       |                  |
             |                |-------|------------------|
             |                        |
             |------------------------|

            When using a regular Z80B (and thus, unencrypted code):
            PAD1 - Open
            PAD2 - Open
            PAD3 - Shorted
            PAD4 - Open

            When using an encrypted CPU module (MC-8123):
            PAD1 - Open
            PAD2 - Shorted
            PAD3 - Open
            PAD4 - Open
            Additionally, a wire must be tied from CN2 pin B22 to the side
            of PAD3 nearest IC6 (i.e. PAD3U).

ROMs:
-----

Game                     IC2         IC3         IC4         IC5         IC7
---------------------------------------------------------------------------------
Hang-On Jr.              EPR-?       EPR-?       EPR-?       EPR-?       EPR-?     Hello, Sega Part Numbers....!?
Transformer              EPR-7350    EPR-?       EPR-7348    EPR-7347    EPR-?     Ditto
           /Astro Flash  EPR-7350    EPR-7349    EPR-7348    EPR-7347    EPR-7723
Riddle of Pythagoras     EPR-10422   EPR-10423   EPR-10424   EPR-10425   EPR-10426
Opa Opa                  EPR-11220   EPR-11221   EPR-11222   EPR-11223   EPR-11224
Fantasy Zone 2           EPR-11412   EPR-11413   EPR-11414   EPR-11415   EPR-11416
Tetris                   -           -           EPR-12211   EPR-12212   EPR-12213

A System E PCB can run all of the games simply by swapping the EPROMs plus CPU.
Well, in theory anyway. To run the not-encrypted games, just swap EPROMs and they will work.

To run the encrypted games, use a double sized EPROM in IC7 (i.e. a 27C512)
and program the decrypted opcodes to the lower half and the decrypted data to the upper half,
then connect the highest address pin of the EPROM (A15 pin 1) to the M1 pin on the Z80.
This method has been tested and does not actually work. An update on this may follow....


System E PCB Pinout
-------------------

CN1
---

+12V             1A  1B  Coin switch 1
Coin switch 2    2A  2B  Test switch
Service switch   3A  3B
                 4A  4B  1P start
2P start         5A  5B  1P up
1P down          6A  6B  1P left
1P right         7A  7B  1P button 1
1P button 2      8A  8B
                 9A  9B  2P up
2P down          10A 10B 2P left
2P RIGHT         11A 11B 2P button 1
2P button 2      12A 12B
                 13A 13B Video RED
                 14A 14B Video GREEN
                 15A 15B Video BLUE
                 16A 16B Video SYNC
                 17A 17B
                 18A 18B
Speaker [+]      19A 19B
Speaker [-]      20A 20B
Coin counter GND 21A 21B
GND              22A 22B Coin counter 1
                 23A 23B Coin counter 2
                 24A 24B
                 25A 25B
CN4
---

+5V  1A 1B +5V
+5V  2A 2B +5V
     3A 3B
GND  4A 4B GND
GND  5A 5B GND
+12V 6A 6B +12V
+12V 7A 7B +12V
GND  8A 8B GND


 Game Notes:
 Riddle of Pythagoras is interesting, it looks like Sega might have planned it
 as a two player game, there is prelimiary code for 2 player support which
 never gets executed, see code around 0x0E95.  Theres also quite a bit of
 pointless code here and there.  Some Interesting Memory Locations

 C000 : level - value (00-0x32)
 C001 : level - display (00-0x50, BCD coded)
 C003 : credits (00-0x99, BCD coded)
 C005 : DSWA put here (coinage, left and right nibbles for left and right slot
        - freeplay when 0x0f or 0xf0)
 C006 : DSWB put here
  bits 0 and 1 : lives ("02", "03", "04", "98")
  bit 3 : difficulty
  bits 5 and 6 : bonus lives ("50K, 100K, 200K, 500K", "100K, 200K, 500K", "100K,
                               200K, 500K, 99999999", "none")
 C009 : lives (for player 1)
 C00A : lives (for player 2)
 C00B : bonus lives counter

 E20B-E20E : score (00000000-0x99999999, BCD coded)
 E215-E218 : hi-score (00000000-0x99999999, BCD coded)

 E543 : bit 0 : ON = player 1 one still has lives
        bit 1 : ON = player 2 one still has lives
        bit 2 : ON = player 1 is the current player - OFF = player 2 is the
         current player

 E572 : table with L. slot infos (5 bytes wide)
 E577 : table with R. slot infos (5 bytes wide)


*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "machine/mc8123.h"
#include "machine/segacrp2.h"
#include "video/315_5124.h"
#include "includes/segaipt.h"


class systeme_state : public driver_device
{
protected:
	virtual void machine_start();

public:
	systeme_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp1(*this, "vdp1"),
		m_vdp2(*this, "vdp2"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu_region(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_bank0d(*this, "bank0d"),
		m_bank1d(*this, "bank1d") { }

	DECLARE_WRITE8_MEMBER( bank_write );
	DECLARE_WRITE_LINE_MEMBER( int_callback );

	DECLARE_READ8_MEMBER( ridleofp_port_f8_read );
	DECLARE_WRITE8_MEMBER( ridleofp_port_fa_write );
	DECLARE_READ8_MEMBER( hangonjr_port_f8_read );
	DECLARE_WRITE8_MEMBER( hangonjr_port_fa_write );

	DECLARE_DRIVER_INIT( hangonjr );
	DECLARE_DRIVER_INIT( astrofl );
	DECLARE_DRIVER_INIT( ridleofp );
	DECLARE_DRIVER_INIT( opaopa );
	DECLARE_DRIVER_INIT( fantzn2 );

	// Devices
	required_device<cpu_device>          m_maincpu;
	required_device<sega315_5124_device> m_vdp1;
	required_device<sega315_5124_device> m_vdp2;

	optional_shared_ptr<UINT8> m_decrypted_opcodes;
	required_memory_region m_maincpu_region;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank0d;
	optional_memory_bank m_bank1d;

	// Analog input related
	UINT8 m_port_select;
	UINT16 m_last1;
	UINT16 m_last2;
	UINT16 m_diff1;
	UINT16 m_diff2;

	// Video RAM
	UINT8 m_vram[2][0x4000 * 2];

	UINT32 screen_update_systeme(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/****************************************************************************************
 Memory Maps

 most of the memory map / IO maps are filled in at run time - this is due to the SMS
 code that this is based on being designed that way due to weird features of the MD.

****************************************************************************************/

/* we have to fill in the ROM addresses for systeme due to the encrypted games */
static ADDRESS_MAP_START( systeme_map, AS_PROGRAM, 8, systeme_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                                                     /* Fixed ROM */
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank1") AM_WRITE_BANK("vram_write")          /* Banked ROM */
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, systeme_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( banked_decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, systeme_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("bank0d")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1d")
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( io_map, AS_IO, 8, systeme_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE(0x7b, 0x7b) AM_DEVWRITE("sn1", segapsg_device, write )
	AM_RANGE(0x7e, 0x7f) AM_DEVWRITE("sn2", segapsg_device, write )
	AM_RANGE(0x7e, 0x7e) AM_DEVREAD( "vdp1", sega315_5124_device, vcount_read )
	AM_RANGE(0xba, 0xba) AM_DEVREADWRITE( "vdp1", sega315_5124_device, vram_read, vram_write )
	AM_RANGE(0xbb, 0xbb) AM_DEVREADWRITE( "vdp1", sega315_5124_device, register_read, register_write )
	AM_RANGE(0xbe, 0xbe) AM_DEVREADWRITE( "vdp2", sega315_5124_device, vram_read, vram_write )
	AM_RANGE(0xbf, 0xbf) AM_DEVREADWRITE( "vdp2", sega315_5124_device, register_read, register_write )
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT( "e0" )
	AM_RANGE(0xe1, 0xe1) AM_READ_PORT( "e1" )
	AM_RANGE(0xe2, 0xe2) AM_READ_PORT( "e2" )
	AM_RANGE(0xf2, 0xf2) AM_READ_PORT( "f2" )
	AM_RANGE(0xf3, 0xf3) AM_READ_PORT( "f3" )
	AM_RANGE(0xf7, 0xf7) AM_WRITE( bank_write )
ADDRESS_MAP_END


static ADDRESS_MAP_START( vdp1_map, AS_0, 8, systeme_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_RAMBANK("vdp1_bank")
ADDRESS_MAP_END


static ADDRESS_MAP_START( vdp2_map, AS_0, 8, systeme_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_RAMBANK("vdp2_bank")
ADDRESS_MAP_END


WRITE8_MEMBER( systeme_state::bank_write )
{
	membank("vdp1_bank")->set_entry((data >> 7) & 1);
	membank("vdp2_bank")->set_entry((data >> 6) & 1);
	membank("vram_write")->set_entry(data >> 5);
	m_bank1->set_entry(data & 0x0f);
	if(m_bank1d)
		m_bank1d->set_entry(data & 0x0f);
}


void systeme_state::machine_start()
{
	membank("vdp1_bank")->configure_entries(0, 2, m_vram[0], 0x4000);
	membank("vdp2_bank")->configure_entries(0, 2, m_vram[1], 0x4000);
	m_bank1->configure_entries(0, 16, m_maincpu_region->base() + 0x10000, 0x4000);

	for (int i = 7; i >= 0; i--)
	{
		int which_vdp, offset;
		if (i & 1)
		{
			which_vdp = 0;
			offset = (i & 4) ? 0 : 0x4000;
		}
		else
		{
			which_vdp = 1;
			offset = (i & 2) ? 0 : 0x4000;
		}
		membank("vram_write")->configure_entry(i, &m_vram[which_vdp][offset]);
	}

	save_item(NAME(m_port_select));
	save_item(NAME(m_last1));
	save_item(NAME(m_last2));
	save_item(NAME(m_diff1));
	save_item(NAME(m_diff2));
	save_item(NAME(m_vram));
}


/*- Hang On Jr. Specific -*/
READ8_MEMBER( systeme_state::hangonjr_port_f8_read )
{
	UINT8 temp;

	temp = 0;

	if (m_port_select == 0x08)  /* 0000 1000 */ /* Angle */
		temp = ioport("IN2")->read();

	if (m_port_select == 0x09)  /* 0000 1001 */ /* Accel */
		temp = ioport("IN3")->read();

	return temp;
}

WRITE8_MEMBER( systeme_state::hangonjr_port_fa_write)
{
	/* Seems to write the same pattern again and again bits ---- xx-x used */
	m_port_select = data;
}

/*- Riddle of Pythagoras Specific -*/

READ8_MEMBER( systeme_state::ridleofp_port_f8_read )
{
	switch (m_port_select)
	{
		default:
		case 0: return m_diff1 & 0xff;
		case 1: return m_diff1 >> 8;
		case 2: return m_diff2 & 0xff;
		case 3: return m_diff2 >> 8;
	}
}

WRITE8_MEMBER( systeme_state::ridleofp_port_fa_write )
{
	/* 0x10 is written before reading the dial (hold counters?) */
	/* 0x03 is written after reading the dial (reset counters?) */

	m_port_select = (data & 0x0c) >> 2;

	if (data & 1)
	{
		int curr = ioport("IN2")->read();
		m_diff1 = ((curr - m_last1) & 0x0fff) | (curr & 0xf000);
		m_last1 = curr;
	}
	if (data & 2)
	{
		int curr = ioport("IN3")->read() & 0x0fff;
		m_diff2 = ((curr - m_last2) & 0x0fff) | (curr & 0xf000);
		m_last2 = curr;
	}
}


/*******************************************************************************
 Input Ports
********************************************************************************
 mostly unknown for the time being
*******************************************************************************/

	/* The Coinage is similar to Sega System 1 and C2, but
	it seems that Free Play is not used in all games
	(in fact, the only playable game that use it is
	Riddle of Pythagoras) */

static INPUT_PORTS_START( segae_joy1_generic )
	PORT_START("f2")    /* Read from Port 0xf2 */
	SEGA_COINAGE_NO_FREE_LOC(SW1)

	PORT_START("f3")    /* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("e0")    /* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 ) // spare

	PORT_START("e1")    /* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")    /* Read from Port 0xe2 */
	//PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( segae_joy2_generic )
	PORT_START("f2")    /* Read from Port 0xf2 */
	SEGA_COINAGE_NO_FREE_LOC(SW1)

	PORT_START("f3")    /* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("e0")    /* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("e1")    /* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")    /* Read from Port 0xe2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( segae_hangonjr_generic )
	PORT_START("f2")    /* Read from Port 0xf2 */
	SEGA_COINAGE_NO_FREE_LOC(SW1)

	PORT_START("f3")    /* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("e0")    /* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e1")    /* Read from Port 0xe1 */
	//PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")    /* Read from Port 0xe2 */
	//PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("IN2")   /* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("IN3")  /* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)
INPUT_PORTS_END

static INPUT_PORTS_START( segae_ridleofp_generic )
	PORT_START("f2")    /* Read from Port 0xf2 */
	SEGA_COINAGE_EASY_FREE_LOC(SW1)

	PORT_START("f3")    /* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("e0")    /* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED ) // Would Be IPT_START2 but the code doesn't use it

	PORT_START("e1")    /* Read from Port 0xe1 */
	//PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")    /* Read from Port 0xe2 */
	//PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	//PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("IN2")   /* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) /* is this used in the game? */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN3")   /* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125) PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( transfrm ) /* Used By Transformer */
	PORT_INCLUDE( segae_joy1_generic )

	PORT_MODIFY("f3")   /* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x00, "1 Player Only" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "10k, 30k, 50k and 70k" )
	PORT_DIPSETTING(    0x30, "20k, 60k, 100k and 140k"  )
	PORT_DIPSETTING(    0x10, "30k, 80k, 130k and 180k" )
	PORT_DIPSETTING(    0x00, "50k, 150k and 250k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( fantzn2 ) /* Used By Fantasy Zone 2 */
	PORT_INCLUDE( segae_joy1_generic )

	PORT_MODIFY("f3")   /* Read from Port 0xf3 */
	//"SW2:1" unused
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "90" )    /* 210 seconds */
	PORT_DIPSETTING(    0x30, "80" )    /* 180 seconds */
	PORT_DIPSETTING(    0x10, "70" )    /* 150 seconds */
	PORT_DIPSETTING(    0x00, "60" )    /* 120 seconds */
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( opaopa ) /* Used By Opa Opa */
	PORT_INCLUDE( segae_joy2_generic )

	PORT_MODIFY("f3")   /* Read from Port 0xf3 */
	//"SW2:1" unused
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6") // Bonus life egg appearance
	PORT_DIPSETTING(    0x20, "25k, 45k and 70k" )
	PORT_DIPSETTING(    0x30, "40k, 60k and 90k" )
	PORT_DIPSETTING(    0x10, "50k and 90k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tetrisse ) /* Used By Tetris */
	PORT_INCLUDE( segae_joy1_generic )

	PORT_MODIFY("f3")   /* Read from Port 0xf3 */
	//"SW2:1" unused
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:3" unused
	//"SW2:4" unused
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END


static INPUT_PORTS_START( hangonjr ) /* Used By Hang On Jr */
	PORT_INCLUDE( segae_hangonjr_generic )

	PORT_MODIFY("f3")   /* Read from Port 0xf3 */
	//"SW2:1" unused //Japanese manual says "DIP SW 2:1 / Sounds After Game Over / Off=No / On=Yes", but no effect
	PORT_DIPNAME( 0x06, 0x06, "Enemies" ) PORT_DIPLOCATION("SW2:2,3") // Enemies appearance frequency
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) ) // Japanese manual says "Normal"
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )  //  "Medium" = "Normal" * 130%
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )    //    "Hard" = "Normal" * 160%
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) ) // "Hardest" = "Normal" * 190%
	PORT_DIPNAME( 0x18, 0x18, "Time Adj." ) PORT_DIPLOCATION("SW2:4,5") // time limit per stage //   Stage  1  2  3  4  5  6  7  8  9 10
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )                         // Japanese manual says // Normal  65 57 55 55 55 65 55 56 55 55
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )                                               // Medium  65 54 56 55 55 65 54 55 55 55
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )                                                 // Hard    60 56 56 56 55 60 56 56 55 55
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )                                              // Hardest 60 54 56 55 55 60 54 55 55 55
	//"SW2:6" unused // Japanese manual says "DIP SW 2:6,7,8 NOT USED"
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END

static INPUT_PORTS_START( ridleofp ) /* Used By Riddle Of Pythagoras */
	PORT_INCLUDE( segae_ridleofp_generic )

	PORT_MODIFY("f3")   /* Read from Port 0xf3 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "100 (Cheat)")
	//"SW2:3" unused
	PORT_DIPNAME( 0x08, 0x08, "Ball Speed" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )  // "Difficult" on datasheet, not "Hard"
	//"SW2:5" unused
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6,7") /* Values came from an original dipsheet */
	PORT_DIPSETTING(    0x60, "50K 100K 200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x40, "100K 200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x20, "200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	//"SW2:8" unused
INPUT_PORTS_END



ROM_START( hangonjr )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "rom5.ic7",   0x00000, 0x08000, CRC(d63925a7) SHA1(699f222d9712fa42651c753fe75d7b60e016d3ad) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "rom4.ic5",   0x10000, 0x08000, CRC(ee3caab3) SHA1(f583cf92c579d1ca235e8b300e256ba58a04dc90) )
	ROM_LOAD( "rom3.ic4",   0x18000, 0x08000, CRC(d2ba9bc9) SHA1(85cf2a801883bf69f78134fc4d5075134f47dc03) )
	ROM_LOAD( "rom2.ic3",   0x20000, 0x08000, CRC(e14da070) SHA1(f8781f65be5246a23c1f492905409775bbf82ea8) )
	ROM_LOAD( "rom1.ic2",   0x28000, 0x08000, CRC(3810cbf5) SHA1(c8d5032522c0c903ab3d138f62406a66e14a5c69) )
ROM_END

ROM_START( ridleofp )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr10426.bin",   0x00000, 0x08000, CRC(4404c7e7) SHA1(555f44786976a009d96a6395c9173929ad6138a7) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr10425.bin",   0x10000, 0x08000, CRC(35964109) SHA1(a7bc64a87b23139b0edb9c3512f47dcf73feb854) )
	ROM_LOAD( "epr10424.bin",   0x18000, 0x08000, CRC(fcda1dfa) SHA1(b8497b04de28fc0d6b7cb0206ad50948cff07840) )
	ROM_LOAD( "epr10423.bin",   0x20000, 0x08000, CRC(0b87244f) SHA1(c88041614735a9b6cba1edde0a11ed413e115361) )
	ROM_LOAD( "epr10422.bin",   0x28000, 0x08000, CRC(14781e56) SHA1(f15d9d89e1ebff36c3867cfc8f0bdf7f6b3c96bc) )
ROM_END

ROM_START( transfrm )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "ic7.top",    0x00000, 0x08000, CRC(ccf1d123) SHA1(5ade9b00e2a36d034fafdf1902d47a9a00e96fc4) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",   0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",   0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "ic3.top",        0x20000, 0x08000, CRC(9d485df6) SHA1(b25f04803c8f7188021f3039aa13aac80d480823) )
	ROM_LOAD( "epr-7350.ic2",   0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END

ROM_START( astrofl )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-7723.ic7",   0x00000, 0x08000, CRC(66061137) SHA1(cb6a2c7864f9f87bbedfd4b1448ad6c2de65d6ca) ) /* encrypted */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",   0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",   0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "epr-7349.ic3",   0x20000, 0x08000, CRC(f8c352d5) SHA1(e59565ab6928c67706c6f82f6ea9a64cdfc65a21) )
	ROM_LOAD( "epr-7350.ic2",   0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END


ROM_START( tetrisse )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr12213.7", 0x00000, 0x08000, CRC(ef3c7a38) SHA1(cbb91aef330ab1a37d3e21ecf1d008143d0dd7ec) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr12212.5", 0x10000, 0x08000, CRC(28b550bf) SHA1(445922a62e8a7360335c754ad70dabbe0208207b) )
	ROM_LOAD( "epr12211.4", 0x18000, 0x08000, CRC(5aa114e9) SHA1(f9fc7fe4d0444a264185e74d2abc8475f0976534) )
	/* ic3 unpopulated */
	/* ic2 unpopulated */
ROM_END


ROM_START( fantzn2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-11416.ic7",  0x00000, 0x08000, CRC(76db7b7b) SHA1(d60e2961fc893dcb4445aed5f67515cbd25b610f) )    /* encrypted */

	ROM_LOAD( "epr-11415.ic5",  0x10000, 0x10000, CRC(57b45681) SHA1(1ae6d0d58352e246a4ec4e1ce02b0417257d5d20) )
	ROM_LOAD( "epr-11413.ic3",  0x20000, 0x10000, CRC(a231dc85) SHA1(45b94fdbde28c02e88546178ef3e8f9f3a96ab86) )
	ROM_LOAD( "epr-11414.ic4",  0x30000, 0x10000, CRC(6f7a9f5f) SHA1(b53aa2eded781c80466a79b7d81383b9a875d0be) )
	ROM_LOAD( "epr-11412.ic2",  0x40000, 0x10000, CRC(b14db5af) SHA1(04c7fb659385438b3d8f9fb66800eb7b6373bda9) )

	ROM_REGION( 0x2000, "key", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0057.key",  0x0000, 0x2000, CRC(ee43d0f0) SHA1(72cb75a4d8352fe372db12046a59ea044360d5c3) )
ROM_END

ROM_START( opaopa )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr11224.ic7",   0x00000, 0x08000, CRC(024b1244) SHA1(59a522ac3d98982cc4ddb1c81f9584d3da453649) ) /* encrypted */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr11223.ic5",   0x10000, 0x08000, CRC(6bc41d6e) SHA1(8997a4ac2a9704f1400d0ec16b259ee496a7efef) ) /* encrypted */
	ROM_LOAD( "epr11222.ic4",   0x18000, 0x08000, CRC(395c1d0a) SHA1(1594bad13e78c5fad4db644cd85a6bac1eaddbad) ) /* encrypted */
	ROM_LOAD( "epr11221.ic3",   0x20000, 0x08000, CRC(4ca132a2) SHA1(cb4e4c01b6ab070eef37c0603190caafe6236ccd) ) /* encrypted */
	ROM_LOAD( "epr11220.ic2",   0x28000, 0x08000, CRC(a165e2ef) SHA1(498ff4c5d3a2658567393378c56be6ed86ac0384) ) /* encrypted */

	ROM_REGION( 0x2000, "key", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0042.key",  0x0000, 0x2000, CRC(d6312538) SHA1(494ac7f080775c21dc7d369e6ea78f3299e6975a) )
ROM_END


WRITE_LINE_MEMBER( systeme_state::int_callback )
{
	m_maincpu->set_input_line(0, state);
}


UINT32 systeme_state::screen_update_systeme(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap_rgb32 &vdp1_bitmap = m_vdp1->get_bitmap();
	bitmap_rgb32 &vdp2_bitmap = m_vdp2->get_bitmap();
	bitmap_ind8 &vdp2_y1 = m_vdp2->get_y1_bitmap();

	for( int y = cliprect.min_y; y <= cliprect.max_y; y++ )
	{
		UINT32 *dest_ptr = &bitmap.pix32(y);
		UINT32 *vdp1_ptr = &vdp1_bitmap.pix32(y);
		UINT32 *vdp2_ptr = &vdp2_bitmap.pix32(y);
		UINT8 *y1_ptr = &vdp2_y1.pix8(y);

		for ( int x = cliprect.min_x; x <= cliprect.max_x; x++ )
		{
			dest_ptr[x] = ( y1_ptr[x] ) ? vdp2_ptr[x] : vdp1_ptr[x];
			//dest_ptr[x] = y1_ptr[x] ? 0x00FF00 : 0xFF0000;
		}
	}

	return 0;
}

static MACHINE_CONFIG_START( systeme, systeme_state )
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10_738635MHz/2) /* Z80B @ 5.3693Mhz */
	MCFG_CPU_PROGRAM_MAP(systeme_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_738635MHz/2, \
		SEGA315_5124_WIDTH , SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_192_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_192_TBORDER_HEIGHT + 192)
	MCFG_SCREEN_UPDATE_DRIVER(systeme_state, screen_update_systeme)

	MCFG_DEVICE_ADD("vdp1", SEGA315_5124, 0)
	MCFG_SEGA315_5124_IS_PAL(false)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vdp1_map)

	MCFG_DEVICE_ADD("vdp2", SEGA315_5124, 0)
	MCFG_SEGA315_5124_IS_PAL(false)
	MCFG_SEGA315_5124_INT_CB(WRITELINE(systeme_state, int_callback))
	MCFG_DEVICE_ADDRESS_MAP(AS_0, vdp2_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SEGAPSG, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SEGAPSG, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( systemex, systeme )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( systemeb, systeme )
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(banked_decrypted_opcodes_map)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(systeme_state, hangonjr)
{
	m_maincpu->space(AS_IO).install_read_handler(0xf8, 0xf8, read8_delegate(FUNC(systeme_state::hangonjr_port_f8_read), this));
	m_maincpu->space(AS_IO).install_write_handler(0xfa, 0xfa, write8_delegate(FUNC(systeme_state::hangonjr_port_fa_write), this));
}


DRIVER_INIT_MEMBER(systeme_state, astrofl)
{
	// 315-5177
	static const UINT8 xor_table[128] =
	{
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,

		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,0x41,0x45,0x00,0x50,0x54,0x11,0x45,0x40,
		0x04,0x54,0x51,0x15,0x40,0x44,0x01,0x51,0x55,0x10,0x44,0x41,
		0x05,0x55,0x50,0x14,
	};

	static const int swap_table[128] =
	{
		0,0,0,0,
		1,1,1,1,1,
		2,2,2,2,2,
		3,3,3,3,
		4,4,4,4,4,
		5,5,5,5,5,
		6,6,6,6,6,
		7,7,7,7,7,
		8,8,8,8,
		9,9,9,9,9,
		10,10,10,10,10,
		11,11,11,11,11,
		12,12,12,12,12,
		13,13,

		8,8,8,8,
		9,9,9,9,9,
		10,10,10,10,10,
		11,11,11,11,
		12,12,12,12,12,
		13,13,13,13,13,
		14,14,14,14,14,
		15,15,15,15,15,
		16,16,16,16,
		17,17,17,17,17,
		18,18,18,18,18,
		19,19,19,19,19,
		20,20,20,20,20,
		21,21,
	};

	sega_decode_2(m_maincpu_region->base(), m_decrypted_opcodes, xor_table, swap_table);
}


DRIVER_INIT_MEMBER(systeme_state, ridleofp)
{
	m_maincpu->space(AS_IO).install_read_handler(0xf8, 0xf8, read8_delegate(FUNC(systeme_state::ridleofp_port_f8_read), this));
	m_maincpu->space(AS_IO).install_write_handler(0xfa, 0xfa, write8_delegate(FUNC(systeme_state::ridleofp_port_fa_write), this));
}


DRIVER_INIT_MEMBER(systeme_state, opaopa)
{
	UINT8 *banked_decrypted_opcodes = auto_alloc_array(machine(), UINT8, m_maincpu_region->bytes());
	mc8123_decode(m_maincpu_region->base(), banked_decrypted_opcodes, memregion("key")->base(), m_maincpu_region->bytes());

	m_bank0d->set_base(banked_decrypted_opcodes);
	m_bank1d->configure_entries(0, 16, banked_decrypted_opcodes + 0x10000, 0x4000);
}


DRIVER_INIT_MEMBER(systeme_state, fantzn2)
{
	mc8123_decode(m_maincpu_region->base(), m_decrypted_opcodes, memregion("key")->base(), 0x8000);
}


//    YEAR, NAME,     PARENT,   MACHINE,  INPUT,    INIT,                    MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1985, hangonjr, 0,        systeme,  hangonjr, systeme_state, hangonjr, ROT0,   "Sega", "Hang-On Jr.", GAME_SUPPORTS_SAVE )
GAME( 1986, transfrm, 0,        systeme,  transfrm, driver_device, 0,        ROT0,   "Sega", "Transformer", GAME_SUPPORTS_SAVE )
GAME( 1986, astrofl,  transfrm, systemex, transfrm, systeme_state, astrofl,  ROT0,   "Sega", "Astro Flash (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1986, ridleofp, 0,        systeme,  ridleofp, systeme_state, ridleofp, ROT90,  "Sega / Nasco", "Riddle of Pythagoras (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1987, opaopa,   0,        systemeb, opaopa,   systeme_state, opaopa,   ROT0,   "Sega", "Opa Opa (MC-8123, 317-0042)", GAME_SUPPORTS_SAVE )
GAME( 1988, fantzn2,  0,        systemex, fantzn2,  systeme_state, fantzn2,  ROT0,   "Sega", "Fantasy Zone II - The Tears of Opa-Opa (MC-8123, 317-0057)", GAME_SUPPORTS_SAVE )
GAME( 1988, tetrisse, 0,        systeme,  tetrisse, driver_device, 0,        ROT0,   "Sega", "Tetris (Japan, System E)", GAME_SUPPORTS_SAVE )
