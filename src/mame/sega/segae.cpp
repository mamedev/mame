// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Sega System E

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
Slap Shooter            1986
Transformer/Astro Flash 1986
Riddle of Pythagoras    1986
Megumi Rescue           1987
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

Megumi Rescue also includes a 834-6193 daughter card for spinner controls

ROMs:
-----

Game                     IC2         IC3         IC4         IC5         IC7
---------------------------------------------------------------------------------
Megumi Rescue            V10.30 IC-2 V10.30 IC-3 V10.30 IC-4 V10.30 IC-5 V10.30 IC-7
Hang-On Jr.              EPR-7261    EPR-7260    EPR-7259    EPR-7258    EPR-7257B
Transformer              EPR-7350    EPR-7606    EPR-7348    EPR-7347    EPR-7605
           /Astro Flash  EPR-7350    EPR-7349    EPR-7348    EPR-7347    EPR-7723
Slap Shooter             EPR-7355    EPR-7354    EPR-7353    EPR-7352    EPR-7751
Riddle of Pythagoras     EPR-10422   EPR-10423   EPR-10424   EPR-10425   EPR-10426
Opa Opa (unencrypted)    EPR-11019   EPR-11020   EPR-11021   EPR-11022   EPR-11023A
Opa Opa (encrypted)      EPR-11220   EPR-11221   EPR-11222   EPR-11223   EPR-11224
Fantasy Zone 2           EPR-11412   EPR-11413   EPR-11414   EPR-11415   EPR-11416
Tetris                   -           -           EPR-12211   EPR-12212   EPR-12213

A System E PCB can run all of the games simply by swapping the EPROMs plus CPU.
Well, in theory anyway. To run the non-encrypted games, just swap EPROMs and they will work.


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
 as a two player game, there is preliminary code for 2 player support which
 never gets executed, see code around 0x0E95. There's also quite a bit of
 pointless code here and there. Some Interesting Memory Locations

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
#include "segaipt.h"

#include "cpu/z80/mc8123.h"
#include "cpu/z80/z80.h"
#include "machine/adc0804.h"
#include "machine/i8255.h"
#include "machine/rescap.h"
#include "machine/segacrp2_device.h"
#include "machine/upd4701.h"
#include "video/315_5124.h"
#include "speaker.h"


namespace {

class systeme_state : public driver_device
{
public:
	systeme_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp1(*this, "vdp1"),
		m_vdp2(*this, "vdp2"),
		m_ppi(*this, "ppi"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu_region(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_bank0d(*this, "bank0d"),
		m_bank1d(*this, "bank1d"),
		m_analog_ports(*this, "IN%u", 2U),
		m_lamp(*this, "lamp0")
	{ }

	void systemex_315_5177(machine_config &config);
	void systemex(machine_config &config);
	void ridleofp(machine_config &config);
	void hangonjr(machine_config &config);
	void systeme(machine_config &config);
	void systemeb(machine_config &config);

	void init_opaopa();
	void init_fantzn2();

private:
	void bank_write(uint8_t data);
	void coin_counters_write(uint8_t data);

	uint8_t hangonjr_analog_read();
	void hangonjr_analog_select(uint8_t data);


	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void banked_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void systeme_map(address_map &map) ATTR_COLD;
	void vdp1_map(address_map &map) ATTR_COLD;
	void vdp2_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	// Devices
	required_device<cpu_device>          m_maincpu;
	required_device<sega315_5124_device> m_vdp1;
	required_device<sega315_5124_device> m_vdp2;
	required_device<i8255_device>        m_ppi;

	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_memory_region m_maincpu_region;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank0d;
	optional_memory_bank m_bank1d;
	optional_ioport_array<2> m_analog_ports;
	output_finder<> m_lamp;
	std::unique_ptr<uint8_t[]> m_banked_decrypted_opcodes;

	// Analog input related
	uint8_t m_port_select;

	// Video RAM
	uint8_t m_vram[2][0x4000 * 2];
};


/****************************************************************************************
 Memory Maps
****************************************************************************************/

/* we have to fill in the ROM addresses for systeme due to the encrypted games */
void systeme_state::systeme_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();                                                     /* Fixed ROM */
	map(0x8000, 0xbfff).bankr("bank1").bankw("vram_write");          /* Banked ROM */
	map(0xc000, 0xffff).ram().share("mainram");
}

void systeme_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share("decrypted_opcodes");
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xffff).ram().share("mainram");
}

void systeme_state::banked_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank0d");
	map(0x8000, 0xbfff).bankr("bank1d");
	map(0xc000, 0xffff).ram().share("mainram");
}


void systeme_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	/* TODO : PSG connection correct? */
	map(0x7b, 0x7b).w(m_vdp1, FUNC(sega315_5124_device::psg_w));
	map(0x7e, 0x7f).w(m_vdp2, FUNC(sega315_5124_device::psg_w));
	map(0x7e, 0x7e).r(m_vdp1, FUNC(sega315_5124_device::vcount_read));
	map(0xba, 0xba).rw(m_vdp1, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0xbb, 0xbb).rw(m_vdp1, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xbe, 0xbe).rw(m_vdp2, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0xbf, 0xbf).rw(m_vdp2, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xe0, 0xe0).portr("e0");
	map(0xe1, 0xe1).portr("e1");
	map(0xe2, 0xe2).portr("e2");
	map(0xf2, 0xf2).portr("f2");
	map(0xf3, 0xf3).portr("f3");
	map(0xf7, 0xf7).w(FUNC(systeme_state::bank_write));
	map(0xf8, 0xfb).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}


void systeme_state::vdp1_map(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("vdp1_bank");
}


void systeme_state::vdp2_map(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("vdp2_bank");
}


void systeme_state::bank_write(uint8_t data)
{
	membank("vdp1_bank")->set_entry((data >> 7) & 1);
	membank("vdp2_bank")->set_entry((data >> 6) & 1);
	membank("vram_write")->set_entry(data >> 5);
	m_bank1->set_entry(data & 0x0f);
	if (m_bank1d.found())
		m_bank1d->set_entry(data & 0x0f);
}

void systeme_state::coin_counters_write(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1)); // only one counter used in most games?
	m_lamp = BIT(data, 2); // used only by hangonjr?
}


void systeme_state::machine_start()
{
	m_lamp.resolve();
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
	save_item(NAME(m_vram));
}


/*- Hang On Jr. Specific -*/
uint8_t systeme_state::hangonjr_analog_read()
{
	return m_analog_ports[m_port_select & 0x01]->read();
}

void systeme_state::hangonjr_analog_select(uint8_t data)
{
	m_port_select = data;
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

	PORT_START("IN2")   /* Angle - Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("IN3")  /* Accel - Read from Port 0xf8 */
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

	PORT_START("PAD1")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125)

	PORT_START("PAD2")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125) PORT_COCKTAIL

	PORT_START("BUTTONS")
	PORT_BIT( 0x1, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", upd4701_device, middle_w) // is this used in the game?
	PORT_BIT( 0x2, IP_ACTIVE_LOW,  IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", upd4701_device, right_w)
	PORT_BIT( 0x4, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", upd4701_device, left_w)
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
	PORT_INCLUDE( segae_joy2_generic )

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

static INPUT_PORTS_START( slapshtr )
	PORT_INCLUDE( segae_joy1_generic )

	PORT_MODIFY("f3")   /* Read from Port 0xf3 */
	// todo
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

static INPUT_PORTS_START( megrescu )
	PORT_INCLUDE( segae_ridleofp_generic )

	PORT_MODIFY("e0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_MODIFY("f3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "Cheat" ) // unlimited?
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(   0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x10, DEF_STR( Cocktail ) )
INPUT_PORTS_END

uint32_t systeme_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap_rgb32 const &vdp1_bitmap = m_vdp1->get_bitmap();
	bitmap_rgb32 const &vdp2_bitmap = m_vdp2->get_bitmap();
	bitmap_ind8 const &vdp2_y1 = m_vdp2->get_y1_bitmap();

	for( int y = cliprect.min_y; y <= cliprect.max_y; y++ )
	{
		uint32_t *const dest_ptr = &bitmap.pix(y);
		uint32_t const *const vdp1_ptr = &vdp1_bitmap.pix(y);
		uint32_t const *const vdp2_ptr = &vdp2_bitmap.pix(y);
		uint8_t const *const y1_ptr = &vdp2_y1.pix(y);

		for ( int x = cliprect.min_x; x <= cliprect.max_x; x++ )
		{
			dest_ptr[x] = ( y1_ptr[x] ) ? vdp2_ptr[x] : vdp1_ptr[x];
			//dest_ptr[x] = y1_ptr[x] ? 0x00FF00 : 0xFF0000;
		}
	}

	return 0;
}

void systeme_state::systeme(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(10'738'635)/2); /* Z80B @ 5.3693Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &systeme_state::systeme_map);
	m_maincpu->set_addrmap(AS_IO, &systeme_state::io_map);

	I8255(config, m_ppi);
	m_ppi->out_pb_callback().set(FUNC(systeme_state::coin_counters_write));
	m_ppi->tri_pb_callback().set_constant(0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(10'738'635)/2,
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT + 192);
	screen.set_screen_update(FUNC(systeme_state::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SEGA315_5124(config, m_vdp1, XTAL(10'738'635));
	m_vdp1->set_is_pal(false);
	m_vdp1->set_addrmap(0, &systeme_state::vdp1_map);
	m_vdp1->add_route(ALL_OUTPUTS, "mono", 0.50);

	SEGA315_5124(config, m_vdp2, XTAL(10'738'635));
	m_vdp2->set_is_pal(false);
	m_vdp2->n_int().set_inputline(m_maincpu, 0);
	m_vdp2->set_addrmap(0, &systeme_state::vdp2_map);
	m_vdp2->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void systeme_state::hangonjr(machine_config &config)
{
	systeme(config);
	m_ppi->in_pa_callback().set("adc", FUNC(adc0804_device::read));
	m_ppi->in_pc_callback().set("adc", FUNC(adc0804_device::intr_r)).lshift(4);
	m_ppi->out_pc_callback().set(FUNC(systeme_state::hangonjr_analog_select)); // CD4051 selector input
	m_ppi->out_pc_callback().append("adc", FUNC(adc0804_device::rd_w)).bit(2);
	m_ppi->out_pc_callback().append("adc", FUNC(adc0804_device::wr_w)).bit(3);

	adc0804_device &adc(ADC0804(config, "adc", RES_K(10), CAP_P(82))); // R1=10K/C11=82pF circuit on 834-5805 card
	adc.vin_callback().set(FUNC(systeme_state::hangonjr_analog_read));
	adc.set_rd_mode(adc0804_device::RD_BITBANGED);
}

void systeme_state::ridleofp(machine_config &config)
{
	systeme(config);
	upd4701_device &upd4701(UPD4701A(config, "upd4701")); // on 834-6193 I/O sub board
	upd4701.set_portx_tag("PAD1");
	upd4701.set_porty_tag("PAD2");

	i8255_device &ppi(*subdevice<i8255_device>("ppi"));
	ppi.in_pa_callback().set("upd4701", FUNC(upd4701_device::d_r));
	ppi.out_pc_callback().set("upd4701", FUNC(upd4701_device::cs_w)).bit(4);
	ppi.out_pc_callback().append("upd4701", FUNC(upd4701_device::xy_w)).bit(3);
	ppi.out_pc_callback().append("upd4701", FUNC(upd4701_device::ul_w)).bit(2);
	ppi.out_pc_callback().append("upd4701", FUNC(upd4701_device::resetx_w)).bit(1); // or possibly bit 0
	ppi.out_pc_callback().append("upd4701", FUNC(upd4701_device::resety_w)).bit(0); // or possibly bit 1
}

void systeme_state::systemex(machine_config &config)
{
	systeme(config);
	mc8123_device &maincpu(MC8123(config.replace(), m_maincpu, XTAL(10'738'635)/2)); /* Z80B @ 5.3693Mhz */
	maincpu.set_addrmap(AS_PROGRAM, &systeme_state::systeme_map);
	maincpu.set_addrmap(AS_IO, &systeme_state::io_map);
	maincpu.set_addrmap(AS_OPCODES, &systeme_state::decrypted_opcodes_map);
}

void systeme_state::systemex_315_5177(machine_config &config)
{
	systeme(config);
	sega_315_5177_device &maincpu(SEGA_315_5177(config.replace(), m_maincpu, XTAL(10'738'635)/2)); /* Z80B @ 5.3693Mhz */
	maincpu.set_addrmap(AS_PROGRAM, &systeme_state::systeme_map);
	maincpu.set_addrmap(AS_IO, &systeme_state::io_map);
	maincpu.set_addrmap(AS_OPCODES, &systeme_state::decrypted_opcodes_map);
	maincpu.set_decrypted_tag(m_decrypted_opcodes);
}

void systeme_state::systemeb(machine_config &config)
{
	systeme(config);
	mc8123_device &maincpu(MC8123(config.replace(), m_maincpu, XTAL(10'738'635)/2)); /* Z80B @ 5.3693Mhz */
	maincpu.set_addrmap(AS_PROGRAM, &systeme_state::systeme_map);
	maincpu.set_addrmap(AS_IO, &systeme_state::io_map);
	maincpu.set_addrmap(AS_OPCODES, &systeme_state::banked_decrypted_opcodes_map);
}


void systeme_state::init_opaopa()
{
	m_banked_decrypted_opcodes = std::make_unique<uint8_t[]>(m_maincpu_region->bytes());
	downcast<mc8123_device &>(*m_maincpu).decode(m_maincpu_region->base(), m_banked_decrypted_opcodes.get(), m_maincpu_region->bytes());

	m_bank0d->set_base(m_banked_decrypted_opcodes.get());
	m_bank1d->configure_entries(0, 16, &m_banked_decrypted_opcodes[0x10000], 0x4000);
}


void systeme_state::init_fantzn2()
{
	downcast<mc8123_device &>(*m_maincpu).decode(m_maincpu_region->base(), m_decrypted_opcodes, 0x8000);
}


//*************************************************************************************************************************
//  Fantasy Zone II - The Tears of Opa-Opa (MC-8123, 317-0057), Sega System E
//
ROM_START( fantzn2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-11416.ic7",  0x00000, 0x08000, CRC(76db7b7b) SHA1(d60e2961fc893dcb4445aed5f67515cbd25b610f) ) // encrypted

	ROM_LOAD( "epr-11415.ic5",  0x10000, 0x10000, CRC(57b45681) SHA1(1ae6d0d58352e246a4ec4e1ce02b0417257d5d20) )
	ROM_LOAD( "epr-11413.ic3",  0x20000, 0x10000, CRC(a231dc85) SHA1(45b94fdbde28c02e88546178ef3e8f9f3a96ab86) )
	ROM_LOAD( "epr-11414.ic4",  0x30000, 0x10000, CRC(6f7a9f5f) SHA1(b53aa2eded781c80466a79b7d81383b9a875d0be) )
	ROM_LOAD( "epr-11412.ic2",  0x40000, 0x10000, CRC(b14db5af) SHA1(04c7fb659385438b3d8f9fb66800eb7b6373bda9) )

	ROM_REGION( 0x2000, "maincpu:key", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0057.key",  0x0000, 0x2000, CRC(ee43d0f0) SHA1(72cb75a4d8352fe372db12046a59ea044360d5c3) )
ROM_END

//*************************************************************************************************************************
//  Hang-On Jr., Sega System E
//   Game ID# 833-5911 HANG ON JR. REV.
//   ROM BD # 834-5910 REV.B
//
// Analog control board:  834-5805 (required for game to boot)
// ICs on this board are LS244 (IC1), ADC0804 (IC2), LS367 (IC3) and CD4051 (IC4).
//
ROM_START( hangonjr )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr-7257b.ic7",   0x00000, 0x08000, CRC(d63925a7) SHA1(699f222d9712fa42651c753fe75d7b60e016d3ad) ) // fixed code

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7258.ic5",   0x10000, 0x08000, CRC(ee3caab3) SHA1(f583cf92c579d1ca235e8b300e256ba58a04dc90) )
	ROM_LOAD( "epr-7259.ic4",   0x18000, 0x08000, CRC(d2ba9bc9) SHA1(85cf2a801883bf69f78134fc4d5075134f47dc03) )
	ROM_LOAD( "epr-7260.ic3",   0x20000, 0x08000, CRC(e14da070) SHA1(f8781f65be5246a23c1f492905409775bbf82ea8) )
	ROM_LOAD( "epr-7261.ic2",   0x28000, 0x08000, CRC(3810cbf5) SHA1(c8d5032522c0c903ab3d138f62406a66e14a5c69) )
ROM_END

//*************************************************************************************************************************
//  Opa Opa (MC-8123, 317-0042), Sega System E
//   Game ID# 833-6407-01 OPA OPA
//
ROM_START( opaopa )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-11054.ic7",  0x00000, 0x08000, CRC(024b1244) SHA1(59a522ac3d98982cc4ddb1c81f9584d3da453649) ) // encrypted

	// The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff
	ROM_LOAD( "epr-11053.ic5",  0x10000, 0x08000, CRC(6bc41d6e) SHA1(8997a4ac2a9704f1400d0ec16b259ee496a7efef) ) // encrypted
	ROM_LOAD( "epr-11052.ic4",  0x18000, 0x08000, CRC(395c1d0a) SHA1(1594bad13e78c5fad4db644cd85a6bac1eaddbad) ) // encrypted
	ROM_LOAD( "epr-11051.ic3",  0x20000, 0x08000, CRC(4ca132a2) SHA1(cb4e4c01b6ab070eef37c0603190caafe6236ccd) ) // encrypted
	ROM_LOAD( "epr-11050.ic2",  0x28000, 0x08000, CRC(a165e2ef) SHA1(498ff4c5d3a2658567393378c56be6ed86ac0384) ) // encrypted

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-0042.key",  0x0000, 0x2000, CRC(d6312538) SHA1(494ac7f080775c21dc7d369e6ea78f3299e6975a) )
ROM_END

//*************************************************************************************************************************
//  Opa Opa, Sega System E
//   Game ID# 833-6407 OPA OPA
//
ROM_START( opaopan )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-11023a.ic7",  0x00000, 0x08000, CRC(101c5c6a) SHA1(5862c6b8d9e1fc8dc9cd26d87f36fde5ce9484ac) ) // fixed code

	// The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff
	ROM_LOAD( "epr-11022.ic5",  0x10000, 0x08000, CRC(15203a42) SHA1(41cfb9a884ed313d4dc3a36696a63a87e49b3b34) )
	ROM_LOAD( "epr-11021.ic4",  0x18000, 0x08000, CRC(b4e83340) SHA1(57955b2b1e5c55b50ed6b53f1b52787442fe716b) )
	ROM_LOAD( "epr-11020.ic3",  0x20000, 0x08000, CRC(c51aad27) SHA1(b6828d7f7283d00964bde7c93f67f4b7f3b9dd87) )
	ROM_LOAD( "epr-11019.ic2",  0x28000, 0x08000, CRC(bd0a6248) SHA1(6b313809dffdb50ee1dc4d83e0567811dc2f1a67) )
ROM_END

//*************************************************************************************************************************
//  Riddle of Pythagoras (Japan), Sega System E
//   Game ID# 833-6200 ピタゴラス ノ ナゾ
//   I/O board 834-6193 © SEGA 1986
//
ROM_START( ridleofp )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr-10426.bin",   0x00000, 0x08000, CRC(4404c7e7) SHA1(555f44786976a009d96a6395c9173929ad6138a7) ) // fixed code

	// The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff
	ROM_LOAD( "epr-10425.bin",   0x10000, 0x08000, CRC(35964109) SHA1(a7bc64a87b23139b0edb9c3512f47dcf73feb854) )
	ROM_LOAD( "epr-10424.bin",   0x18000, 0x08000, CRC(fcda1dfa) SHA1(b8497b04de28fc0d6b7cb0206ad50948cff07840) )
	ROM_LOAD( "epr-10423.bin",   0x20000, 0x08000, CRC(0b87244f) SHA1(c88041614735a9b6cba1edde0a11ed413e115361) )
	ROM_LOAD( "epr-10422.bin",   0x28000, 0x08000, CRC(14781e56) SHA1(f15d9d89e1ebff36c3867cfc8f0bdf7f6b3c96bc) )
ROM_END

//*************************************************************************************************************************
//  Slap Shooter, Sega System E
//   ROM BD # 834-5930
//
ROM_START( slapshtr )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr-7351.ic7",   0x00000, 0x08000, CRC(894adb04) SHA1(e38d296aa56c531985cde75423ae03f0d9cc8f5d) ) // fixed code

	ROM_LOAD( "epr-7352.ic5",   0x10000, 0x08000, CRC(61c938b6) SHA1(bdce617050371c7b2880967c7c7356f34b91911d) )
	ROM_LOAD( "epr-7353.ic4",   0x18000, 0x08000, CRC(8ee2951a) SHA1(562fef28e6358cbbb4889cc7cd592ca659c238fb) )
	ROM_LOAD( "epr-7354.ic3",   0x20000, 0x08000, CRC(41482aa0) SHA1(280d5a1c5685c692a924c62ed928fc25fec2352a) )
	ROM_LOAD( "epr-7355.ic2",   0x28000, 0x08000, CRC(c67e1aef) SHA1(121bc40d652449c7b1b5f66a837baeaffb7af0b9) )
ROM_END

//*************************************************************************************************************************
//  Tetris (Japan), Sega System E
//
ROM_START( tetrisse )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr-12213.7", 0x00000, 0x08000, CRC(ef3c7a38) SHA1(cbb91aef330ab1a37d3e21ecf1d008143d0dd7ec) ) // Fixed Code

	// The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff
	ROM_LOAD( "epr-12212.5", 0x10000, 0x08000, CRC(28b550bf) SHA1(445922a62e8a7360335c754ad70dabbe0208207b) )
	ROM_LOAD( "epr-12211.4", 0x18000, 0x08000, CRC(5aa114e9) SHA1(f9fc7fe4d0444a264185e74d2abc8475f0976534) )
	// ic3 unpopulated
	// ic2 unpopulated
ROM_END

//*************************************************************************************************************************
//  Transformers, Sega System E
//   Game ID# 833-5927-01
//   ROM BD # 834-5929-01
//
ROM_START( transfrm )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr-7605.ic7",   0x00000, 0x08000, CRC(ccf1d123) SHA1(5ade9b00e2a36d034fafdf1902d47a9a00e96fc4) ) // fixed code

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",   0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",   0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "epr-7606.ic3",   0x20000, 0x08000, CRC(9d485df6) SHA1(b25f04803c8f7188021f3039aa13aac80d480823) )
	ROM_LOAD( "epr-7350.ic2",   0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END

//*************************************************************************************************************************
//  Astro Flash (Japan), Sega System E
//
ROM_START( astrofl )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-7723.ic7",   0x00000, 0x08000, CRC(66061137) SHA1(cb6a2c7864f9f87bbedfd4b1448ad6c2de65d6ca) ) // encrypted

	// The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff
	ROM_LOAD( "epr-7347.ic5",   0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",   0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "epr-7349.ic3",   0x20000, 0x08000, CRC(f8c352d5) SHA1(e59565ab6928c67706c6f82f6ea9a64cdfc65a21) )
	ROM_LOAD( "epr-7350.ic2",   0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END

//*************************************************************************************************************************
//  Megumi Rescue
//   Game ID# 833-6200
//
//   ROMs have no SEGA EPR codes but are all marked
//
//        Megumi Rescue
//        Version 10.30
//        Final Version
//            IC-x
//       (c)1987SEGA/EXA
//
//   (where -x is the IC position on the PCB)
//
ROM_START( megrescu )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "megumi_rescue_version_10.30_final_version_ic-7.ic7",   0x00000, 0x08000, CRC(490d0059) SHA1(de4e23eb862ef3c29b2fbdceba14360eb6e2a8ef) ) // fixed code

	ROM_LOAD( "megumi_rescue_version_10.30_final_version_ic-5.ic5",   0x10000, 0x08000, CRC(278caba8) SHA1(809e504f6c680f742f0a5968d6bb16c2f67f851c) )
	ROM_LOAD( "megumi_rescue_version_10.30_final_version_ic-4.ic4",   0x18000, 0x08000, CRC(bda242d1) SHA1(3704da98fe91d9e7f4380ea5e1f897b6b7049466) )
	ROM_LOAD( "megumi_rescue_version_10.30_final_version_ic-3.ic3",   0x20000, 0x08000, CRC(56e36f85) SHA1(84aa78bc628bce64b1b990a8c9fcca25e5940bd3) )
	ROM_LOAD( "megumi_rescue_version_10.30_final_version_ic-2.ic2",   0x28000, 0x08000, CRC(5b74c767) SHA1(dbc82a4e046f01130c72bbd7a81190d7f0ca209c) )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT    MACHINE            INPUT     STATE          INIT           MONITOR COMPANY         FULLNAME,FLAGS
GAME( 1985, hangonjr, 0,        hangonjr,          hangonjr, systeme_state, empty_init,    ROT0,   "Sega",         "Hang-On Jr. (Rev. B)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, slapshtr, 0,        systeme,           slapshtr, systeme_state, empty_init,    ROT0,   "Sega",         "Slap Shooter", MACHINE_SUPPORTS_SAVE) // 1986 date from flyer
GAME( 1986, transfrm, 0,        systeme,           transfrm, systeme_state, empty_init,    ROT0,   "Sega",         "Transformer", MACHINE_SUPPORTS_SAVE )
GAME( 1986, astrofl,  transfrm, systemex_315_5177, transfrm, systeme_state, empty_init,    ROT0,   "Sega",         "Astro Flash (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ridleofp, 0,        ridleofp,          ridleofp, systeme_state, empty_init,    ROT90,  "Sega / Nasco", "Riddle of Pythagoras (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, opaopa,   0,        systemeb,          opaopa,   systeme_state, init_opaopa,   ROT0,   "Sega",         "Opa Opa (MC-8123, 317-0042)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, opaopan,  opaopa,   systeme,           opaopa,   systeme_state, empty_init,    ROT0,   "Sega",         "Opa Opa (Rev A, unprotected)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, fantzn2,  0,        systemex,          fantzn2,  systeme_state, init_fantzn2,  ROT0,   "Sega",         "Fantasy Zone II - The Tears of Opa-Opa (MC-8123, 317-0057)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, tetrisse, 0,        systeme,           tetrisse, systeme_state, empty_init,    ROT0,   "Sega",         "Tetris (Japan, System E)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, megrescu, 0,        ridleofp,          megrescu, systeme_state, empty_init,    ROT90,  "Sega / Exa",   "Megumi Rescue", MACHINE_SUPPORTS_SAVE )
