// license:BSD-3-Clause
// copyright-holders:David Graves, Brian Troha
// thanks-to:Richard Bush
/***************************************************************************

Asuka & Asuka  (+ Taito/Visco games on similar hardware)
=============

David Graves, Brian Troha

Made out of:    Rastan driver by Jarek Burczynski
                MAME Taito F2 driver
                Raine source - very special thanks to
                  Richard Bush and the Raine Team.
                two different drivers for Bonze Adventure that were
                  written at the same time by Yochizo and Frotz

    Bonze Adventure (c) 1988 Taito Corporation
    Asuka & Asuka   (c) 1988 Taito Corporation
    Maze of Flott   (c) 1989 Taito Corporation
    Galmedes        (c) 1992 Visco Corporation
    Earth Joker     (c) 1993 Visco Corporation
    Kokontouzai Eto Monogatari (c) 1994 Visco Corporation

Main CPU: MC68000 uses irq 5 (4 in bonze, 4&5 in cadash).
Sound   : Z80 & YM2151 + MSM5205 (YM2610 in bonze)
Chips   : TC0100SCN + TC0002OBJ + TC0110PCR (+ C-Chip in bonze)
(Bryan McPhail:  My Bonze uses TC0100SCN + PC0900J (OBJ) + TC0110PCR + TC0140SYT (SND))

Memory map for Asuka & Asuka
----------------------------

The other games seem identical but Eto is slightly different.

0x000000 - 0x0fffff : ROM (not all used for each game)
0x100000 - 0x103fff : 16k of RAM
0x200000 - 0x20000f : palette generator
0x400000 - 0x40000f : input ports and dipswitches
0x3a0000 - 0x3a0003 : sprite control
0x3e0000 - 0x3e0003 : communication with sound CPU
0xc00000 - 0xc2000f : TC0100SCN (see video/tc0100scn.cpp)
0xd00000 - 0xd007ff : sprite RAM


Cadashu Info (Malcor)
---------------------

Main PCB (JAMMA) K1100528A
Main processor  - 68000 12MHz
                - HD64180RP8 8MHz (8 bit processor, dual channel DMAC,
                             memory mapped I/O, used for multigame link)
Misc custom ICs including three PQFPs, one PGA, and one SIP


From "garmedes.txt"
-------------------

The following cord is written, on PCB:  K1100388A   J1100169A   M6100708A
There are the parts that were written as B68 on this PCB.
The original title of the game called B68 is unknown.
This PCB is the same as the one that is used with EARTH-JOKER.
<B68 is the verified Taito ROM id# for Asuka & Asuka - B.Troha>

***************************************************************************
Bonze Adventure, Taito, 1988
Hardware info by Guru
Last Update: 25th August 2021


PCB Layout
----------

MAIN BOARD J1100144A K1100330A
Sticker: K1100330B JIGOKU MEGURI
|-------------------------------------------------------------------|
|MB3735    YM2610                         Z0840004PSC     B41-03.IC1|
|VOL       B41-04.IC48                    B41_13.IC20               |
|                          |-------|      CXK5864                   |
|   4556  TL074  YM3016F   |TAITO  |          |-------|             |
|Y                         |TC0140SYT   16MHz | TAITO |             |
|              TMM2063     |-------|          |TC0100SCN   CXK58256 |
|      TL074   TMM2063                        |-------|             |
|                         TC0110PCR                        CXK58256 |
|       TC0070RGB                                 26.868MHz         |
|J                2018       |---------|                  B41-02.IC6|
|A                2018       | TAITO   |                            |
|M                2018       | PC0900J |                            |
|M       PC050CM  2018       |         |                            |
|A                           |---------|                            |
|            |-------------|                                        |
|            | B41_05.IC43 |                                        |
|            |-------------|          12MHz          B41-01.IC15    |
|  48CR-1(x6)                                                       |
|       |------------------|          B41_12.IC25    B41_10.IC15    |
|       |       68000      |                                  MB3771|
|       |------------------|          B41_11-1.IC26  B41_09-1.IC17  |
|                 B41-08.IC45  B41-06.IC34                          |
|      SWB  SWA   B41-07.IC46            TMM2064        TMM2064     |
|-------------------------------------------------------------------|
Notes:
        MB3735 - Fujitsu MB3735 20W BTL mono power amplifier
        MB3771 - Fujitsu MB3771 power supply monitor IC. This is used to provide the power-on reset.
       SWA/SWB - 8-position DIP switch
          4556 - 4556 dual operational amplifier
         TL074 - Texas Instruments TL074 JFET Low-Noise Quad OP Amp
           VOL - 5k volume pot
      CXK58256 - Sony CXK58256 32k x8-bit static RAM
       CXK5864 - Sony CXK5864 8k x8-bit static RAM
       TMM2064 - Toshiba TMM2064 8k x8-bit static RAM
          2018 - Toshiba TMM2018 2k x8-bit static RAM or equivalent Motorola MCM2016 or HM3-65728 or Sony CXK5816
         68000 - Motorola MC68000P12 CPU. Clock input 8.000MHz [16/2]
   Z0840004PSC - Zilog Z0840006PSC Z80 CPU. Clock input 4.000MHz [16/4]
        YM2610 - Yamaha YM2610 sound chip. Clock input 8.000MHz [16/2]
      YM3016-F - Yamaha Y3016-F 2-Channel Serial & Binary Input Floating D/A Converter (SOIC16)
                 Clock input 2.66666MHz (16/2/3, source = pin 64 of YM2610)
     TC0070RGB - 5-bit RGB Video Mixer/RGB DAC (Ceramic Flat Pack SIL25)
       PC050CM - Taito PC050CM custom SIL28 ceramic module for coins, coin lockout and coin counters
     TC0140SYT - Taito custom TC0140SYT sound communication IC
     TC0110PCR - Taito custom palette generator IC
     TC0100SCN - Taito custom tilemap generator IC
       PC0900J - Taito custom sprite generator IC
        48CR-1 - Custom resistor array used for inputs
             Y - 3 pin connector for 2nd speaker output (mono only)
         HSync - 15.55385kHz
         VSync - 60.054Hz
   B41-01.IC15 - MN234000 mask ROM (main program)
    B41-02.IC6 - MN234000 mask ROM (sprites)
    B41-03.IC1 - MN234000 mask ROM (tiles)
   B41-04.IC48 - MN234000 mask ROM (ADPCM samples)
   B41_05.IC43 - Taito TC0030CMD hybrid custom IC containing uPD78C11 MCU with 4k x8-bit mask ROM, uPD27C64 8k x8-bit EPROM, uPD4464 8k x8-bit static RAM & logic glue.
                 Clock input 12.000MHz. This chip is known as a 'C-Chip'. Only the EPROM differs between different games.
   B41-06.IC34 - MMI PAL16L8 marked 'B41-06'
   B41-07.IC46 - MMI PAL16L8 marked 'B41-07'
   B41-08.IC45 - MMI PAL20L8 marked 'B41-08'
 B41_09-1.IC17 - Toshiba TMM27512 64k x8-bit EPROM (main program)
   B41_10.IC15 - Toshiba TMM27512 64k x8-bit EPROM (main program)
 B41_11-1.IC26 - Toshiba TMM27512 64k x8-bit EPROM (main program)
   B41_12.IC25 - Toshiba TMM27512 64k x8-bit EPROM (main program)
   B41_13.IC20 - Toshiba TMM27512 64k x8-bit EPROM (sound program)

***************************************************************************

Use of TC0100SCN
----------------

Asuka & Asuka: $e6a init code clearing TC0100SCN areas is erroneous.
It only clears 1/8 of the BG layers; then it clears too much of the
rowscroll areas [0xc000, 0xc400] causing overrun into next 64K block.

Asuka is one of the early Taito games using the TC0100SCN. (Ninja
Warriors was probably the first.) They didn't bother using its FG (text)
layer facility, instead placing text in the BG / sprite layers.

Maze of Flott [(c) one year later] and most other games with the
TC0100SCN do use the FG layer for text (Driftout is an exception).


Stephh's notes (based on the game M68000 code and some tests) :

1) 'bonzeadv', 'jigkmgri' and 'bonzeadvu'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'bonzeadv' : region = 0x0002
      * 'jigkmgri' : region = 0x0000
      * 'bonzeadvu': region = 0x0001
  - These 3 games are 100% the same, only region differs !
  - Coinage relies on the region (code at 0x02d344) :
      * 0x0000 (Japan) and 0x0001 (US) use TAITO_COINAGE_JAPAN_OLD_LOC()
      * 0x0002 (World) uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen only if region = 0x0000
  - Texts and game name rely on the region :
      * 0x0000 : most texts in Japanese - game name is "Jigoku Meguri"
      * other : all texts in English - game name is "Bonze Adventure"
  - Bonus lives aren't awarded correctly due to bogus code at 0x00961e :

      00961E: 302D 0B7E                  move.w  ($b7e,A5), D0
      009622: 0240 0018                  andi.w  #$18, D0
      009626: E648                       lsr.w   #3, D0

    Here is what the correct code should be :

      00961E: 302D 0B7E                  move.w  ($b7e,A5), D0
      009622: 0240 0030                  andi.w  #$30, D0
      009626: E848                       lsr.w   #4, D0

  - DSWB bit 7 was previously used to allow map viewing (C-Chip test ?),
    but it is now unused due to "bra" instruction at 0x007572


2) 'bonzeadvo'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'bonzeadvo' : region = 0x0002
  - The only difference is that the following code is missing :

      00D218: 08AD 0004 15DE             bclr    #$4, ($15de,A5)

    So the "crouch" bit wasn't always reset, which may cause you
    to consume all your magic powers in less than 4 frames !
    See bonzeadv0107u1ora full report on MAME Testers site
  - Same other notes as for 'bonzeadv'


3) 'asuka*'

  - No region
  - BOTH sets use TAITO_COINAGE_JAPAN_OLD_LOC() for coinage,
    so I wonder if the World version isn't a US version
  - Additional notice screen in 'asukaj'


4) 'mofflott'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'mofflott' : region = 0x0001
  - Coinage relies on the region (code at 0x0145ec) :
      * 0x0001 (Japan) and 0x0002 (US ?) use TAITO_COINAGE_JAPAN_OLD_LOC()
      * 0x0003 (World) uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen only if region = 0x0001


5) 'cadash*'

  - Region stored at 0x07fffe.w
  - Sets :
      * 'cadash'   : region = 0x0003
      * 'cadashj'  : region = 0x0001
      * 'cadashu'  : region = 0x0002
      * 'cadashfr' : region = 0x0003
      * 'cadashit' : region = 0x0003
  - These 5 games are 100% the same, only region differs !
    However each version requires its specific texts
  - Coinage relies on the region (code at 0x0013d6) :
      * 0x0001 (Japan) uses TAITO_COINAGE_JAPAN_OLD_LOC()
      * 0x0002 (US) uses TAITO_COINAGE_US_LOC()
      * 0x0003 (World) uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen only if region = 0x0001 or region = 0x0002
  - FBI logo only if region = 0x0002
  - I can't tell about the Italian and Japanese versions,
    but translation in the French version is really poor !


6) 'galmedes'

  - No region (not a Taito game anyway)
  - Coinage relies on "Coin Mode" Dip Switch (code at 0x0801c0) :
      * "Mode A" uses TAITO_COINAGE_JAPAN_OLD_LOC()
      * "Mode B" uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen


7) 'earthjkr'

  - No region (not a Taito game anyway)
  - Game uses TAITO_COINAGE_JAPAN_OLD_LOC()
  - Notice screen only if "Copyright" Dip Switch set to "Visco"


8) 'eto'

  - No region (not a Taito game anyway)
  - Game uses TAITO_COINAGE_JAPAN_OLD_LOC()
  - No notice screen


TODO
----

Mofflot: $14c46 sub inits sound system: in a pause loop during this
it reads a dummy address.

Earthjkr: Wrong screen size? Left edge of green blueprints in
attract looks like it's incorrectly off screen.

Cadash: Hooks for twin arcade machine setup: will involve emulating an extra
microcontroller, the 07 ROM might be the program for it. Cadash background
colors don't reinitialize properly with save states.

Galmedes: Test mode has select1/2 stuck at on.

Eto: $76d0 might be a protection check? It reads to and writes from
the program ROM. Doesn't seem to cause problems though.

DIP locations verified for:
    - bonzeadv (manual)
    - cadash (manual)
    - asuka (manual)
    - mofflott (manual)
    - galmedes (manual)

***************************************************************************/

#include "emu.h"

#include "taitosnd.h"
#include "taitoipt.h"
#include "taitocchip.h"
#include "taitoio.h"
#include "pc090oj.h"
#include "tc0100scn.h"
#include "tc0110pcr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z180/z180.h"
#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/msm5205.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"

#include "screen.h"
#include "speaker.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_audiobank(*this, "audiobank")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_pc090oj(*this, "pc090oj")
		, m_tc0100scn(*this, "tc0100scn")
	{ }

	void init_earthjkr();

	void eto(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	// memory pointers
	required_memory_bank m_audiobank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<pc090oj_device> m_pc090oj;
	required_device<tc0100scn_device> m_tc0100scn;

	void coin_control_w(u8 data);
	void sound_bankswitch_w(u8 data);
	void variable_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl);
	void fixed_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void z80_map(address_map &map) ATTR_COLD;

private:
	void eto_map(address_map &map) ATTR_COLD;
};


class msm_state : public base_state
{
public:
	msm_state(const machine_config &mconfig, device_type type, const char *tag)
		: base_state(mconfig, type, tag)
		, m_sound_data(*this, "msm")
		, m_msm(*this, "msm")
		, m_adpcm_select(*this, "adpcm_select")
	{ }

	void mofflott(machine_config &config);
	void asuka(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_region_ptr<u8> m_sound_data;

	// misc
	u16         m_adpcm_pos = 0;
	bool        m_adpcm_ff = false;

	// devices
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_select;

	void msm5205_address_w(u8 data);
	void msm5205_start_w(u8 data);
	void msm5205_stop_w(u8 data);
	void msm5205_vck(int state);

	void asuka_map(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;
};


class bonzeadv_state : public base_state
{
public:
	bonzeadv_state(const machine_config &mconfig, device_type type, const char *tag)
		: base_state(mconfig, type, tag)
		, m_cchip(*this, "cchip")
		, m_cchip_irq_clear(*this, "cchip_irq_clear")
	{ }

	void bonzeadv(machine_config &config);

private:
	// devices
	required_device<taito_cchip_device> m_cchip;
	required_device<timer_device> m_cchip_irq_clear;

	void counters_w(u8 data);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cchip_irq_clear_cb);

	void main_map(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;
};


class cadash_state : public base_state
{
public:
	 cadash_state(const machine_config &mconfig, device_type type, const char *tag)
		: base_state(mconfig, type, tag)
		, m_shared_ram(*this, "sharedram")
	{ }

	void cadash(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u8> m_shared_ram;

	emu_timer *m_int5_timer = nullptr;
	TIMER_CALLBACK_MEMBER(interrupt5);

	u16 share_r(offs_t offset);
	void share_w(offs_t offset, u16 data);
	INTERRUPT_GEN_MEMBER(interrupt);

	void main_map(address_map &map) ATTR_COLD;
	void sub_io(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


/**************************************************************
                 SPRITE READ AND WRITE HANDLERS
**************************************************************/

void base_state::variable_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)
{
	sprite_colbank = (sprite_ctrl & 0x3c) << 2;
	pri_mask = (sprite_ctrl & 1) ? 0xfc : 0xf0; // variable sprite/tile priority
}

void base_state::fixed_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)
{
	sprite_colbank = (sprite_ctrl & 0x3c) << 2;
	pri_mask = 0xf0; // sprites over top bg layer
}


/**************************************************************
                        SCREEN REFRESH
**************************************************************/

uint32_t base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	// Ensure screen blanked even when bottom layer not drawn due to disable bit
	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	m_pc090oj->draw_sprites(screen, bitmap, cliprect);
	return 0;
}


/***********************************************************
                INTERRUPTS
***********************************************************/

TIMER_CALLBACK_MEMBER(cadash_state::interrupt5)
{
	m_maincpu->set_input_line(5, HOLD_LINE);
}


INTERRUPT_GEN_MEMBER(cadash_state::interrupt)
{
	m_int5_timer->adjust(m_maincpu->cycles_to_attotime(500));
	m_maincpu->set_input_line(4, HOLD_LINE);  // interrupt vector 4
}


/************************************************
            SOUND
************************************************/

void base_state::sound_bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 0x03);
}


void msm_state::msm5205_vck(int state)
{
	if (!state)
		return;

	m_adpcm_ff = !m_adpcm_ff;
	m_adpcm_select->select_w(m_adpcm_ff);

	if (m_adpcm_ff)
	{
		m_adpcm_select->ba_w(m_sound_data[m_adpcm_pos]);
		m_adpcm_pos = (m_adpcm_pos + 1) & 0xffff;
	}
}

void msm_state::msm5205_address_w(u8 data)
{
	m_adpcm_pos = (m_adpcm_pos & 0x00ff) | (data << 8);
}

void msm_state::msm5205_start_w(u8 data)
{
	m_msm->reset_w(0);
	m_adpcm_ff = false;
}

void msm_state::msm5205_stop_w(u8 data)
{
	m_msm->reset_w(1);
	m_adpcm_pos &= 0xff00;
}

u16 cadash_state::share_r(offs_t offset)
{
	return m_shared_ram[offset];
}

void cadash_state::share_w(offs_t offset, u16 data)
{
	m_shared_ram[offset] = data & 0xff;
}


void base_state::coin_control_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void bonzeadv_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x0fffff).rom();
	map(0x10c000, 0x10ffff).ram();
	map(0x200000, 0x200007).rw("tc0110pcr", FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));
	map(0x390000, 0x390001).portr("DSWA");
	map(0x3a0000, 0x3a0001).w(m_pc090oj, FUNC(pc090oj_device::sprite_ctrl_w));
	map(0x3b0000, 0x3b0001).portr("DSWB");
	map(0x3c0000, 0x3c0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x3d0000, 0x3d0001).nopr();
	map(0x3e0001, 0x3e0001).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x3e0003, 0x3e0003).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x8007ff).rw(m_cchip, FUNC(taito_cchip_device::mem68_r), FUNC(taito_cchip_device::mem68_w)).umask16(0x00ff);
	map(0x800800, 0x800fff).rw(m_cchip, FUNC(taito_cchip_device::asic_r), FUNC(taito_cchip_device::asic68_w)).umask16(0x00ff);
	map(0xc00000, 0xc0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    // tilemaps
	map(0xc20000, 0xc2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xd00000, 0xd03fff).rw(m_pc090oj, FUNC(pc090oj_device::word_r), FUNC(pc090oj_device::word_w));  // sprite RAM
}

void msm_state::asuka_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x1076f0, 0x1076f1).nopr(); // Mofflott init does dummy reads here
	map(0x200000, 0x20000f).rw("tc0110pcr", FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));
	map(0x3a0000, 0x3a0003).w(m_pc090oj, FUNC(pc090oj_device::sprite_ctrl_w));
	map(0x3e0000, 0x3e0001).nopr();
	map(0x3e0001, 0x3e0001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x3e0003, 0x3e0003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0x400000, 0x40000f).rw("tc0220ioc", FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0xc00000, 0xc0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    // tilemaps
	map(0xc10000, 0xc103ff).nopw();    // error in Asuka init code
	map(0xc20000, 0xc2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xd00000, 0xd03fff).rw(m_pc090oj, FUNC(pc090oj_device::word_r), FUNC(pc090oj_device::word_w));  // sprite RAM
}

void cadash_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x080003).w(m_pc090oj, FUNC(pc090oj_device::sprite_ctrl_w));
	map(0x0c0000, 0x0c0001).nopr();
	map(0x0c0001, 0x0c0001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x0c0003, 0x0c0003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0x100000, 0x107fff).ram();
	map(0x800000, 0x800fff).rw(FUNC(cadash_state::share_r), FUNC(cadash_state::share_w));    // network RAM
	map(0x900000, 0x90000f).rw("tc0220ioc", FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0xa00000, 0xa0000f).rw("tc0110pcr", FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_4bpg_word_w));
	map(0xb00000, 0xb03fff).rw(m_pc090oj, FUNC(pc090oj_device::word_r), FUNC(pc090oj_device::word_w));  // sprite RAM
	map(0xc00000, 0xc0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    // tilemaps
	map(0xc20000, 0xc2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}

void base_state::eto_map(address_map &map)
{ // N.B. tc100scn mirror overlaps spriteram
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10000f).rw("tc0110pcr", FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));
	map(0x200000, 0x203fff).ram();
	map(0x300000, 0x30000f).rw("tc0220ioc", FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x400000, 0x40000f).r("tc0220ioc", FUNC(tc0220ioc_device::read)).umask16(0x00ff);   // service mode mirror
	map(0x4a0000, 0x4a0003).w(m_pc090oj, FUNC(pc090oj_device::sprite_ctrl_w));
	map(0x4e0000, 0x4e0001).nopr();
	map(0x4e0001, 0x4e0001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x4e0003, 0x4e0003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xc00000, 0xc0ffff).w(m_tc0100scn, FUNC(tc0100scn_device::ram_w));
	map(0xc00000, 0xc03fff).rw(m_pc090oj, FUNC(pc090oj_device::word_r), FUNC(pc090oj_device::word_w));  // sprite RAM
	map(0xd00000, 0xd0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    // tilemaps
	map(0xd20000, 0xd2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}


/***************************************************************************/

void bonzeadv_state::z80_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).w("tc0140syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("tc0140syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).nopw(); // pan
	map(0xe600, 0xe600).nopw();
	map(0xee00, 0xee00).nopw();
	map(0xf000, 0xf000).nopw();
	map(0xf200, 0xf200).w(FUNC(bonzeadv_state::sound_bankswitch_w));
}

// no MSM5205
void base_state::z80_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_audiobank);
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
}

void msm_state::z80_map(address_map &map)
{
	base_state::z80_map(map);
//  map(0x9002, 0x9100).nopr();
	map(0xb000, 0xb000).w(FUNC(msm_state::msm5205_address_w));
	map(0xc000, 0xc000).w(FUNC(msm_state::msm5205_start_w));
	map(0xd000, 0xd000).w(FUNC(msm_state::msm5205_stop_w));
}

/*
Cadash communication CPU is a z180.
[0x8000]: at pc=31, z180 checks a byte ... if it's equal to 0x4d ("M") then the board is in master mode, otherwise it's in slave mode.
Right now, the z180 is too fast, so it never checks it properly ... maybe I'm missing a z180 halt line that's lying to somewhere on m68k side.
[0x8002]: puts T in master mode, R in slave mode ... looks a rather obvious flag that says the current Tx / Rx state
[0x8080-0x80ff]: slave data
[0x8100-0x817f]: master data

Internal I/O Asynchronous SCI regs are then checked ... we can't emulate this at the current time, needs two MAME instances.

m68k M communicates with z180 M through shared ram, then the z180 M communicates with z180 S through these ASCI regs ... finally, the z180 S
communicates with m68k S with its own shared ram. In short:

m68k M -> z180 M <-> z180 S <- m68k S
*/

void cadash_state::sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share(m_shared_ram);
}

void cadash_state::sub_io(address_map &map)
{
	map(0x00, 0x3f).ram(); // z180 internal I/O regs
}

/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

#define CADASH_PLAYERS_INPUT( player ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	INPUT_GENERIC_JOY_HIGH_NIBBLE(player, IP_ACTIVE_LOW, PORT_8WAY, RIGHT, LEFT, DOWN, UP)


// different players and system inputs than 'asuka'
static INPUT_PORTS_START( bonzeadv )
	// 0x390000 -> 0x10cb7c ($b7c,A5)
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	// 0x3b0000 -> 0x10cb7e ($b7e,A5)
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SWB:3,4") // see notes
	PORT_DIPSETTING(    0x08, "40k 100k" )                  // 300k 1000k 1500k 2000k 2500k 3000k 3500k 5000k
	PORT_DIPSETTING(    0x0c, "50k 150k" )                  // 500k 1000k 2000k 3000k 4000k 5000k 6000k 7000k
	PORT_DIPSETTING(    0x04, "60k 200k" )                  // 500k 1000k 2000k 3000k 4000k 5000k 6000k 7000k
	PORT_DIPSETTING(    0x00, "80k 250k" )                  // 500k 1000k 2000k 3000k 4000k 5000k 6000k 7000k
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )            // see notes

	PORT_START("800007")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("800009")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("80000B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("80000D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( jigkmgri )
	PORT_INCLUDE(bonzeadv)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( asuka )
	// 0x400000 -> 0x103618
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)

	// 0x400002 -> 0x10361c
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, "Bonus Points" )              PORT_DIPLOCATION("SWB:3,4") // for each plane shot after each end of level boss
	PORT_DIPSETTING(    0x0c, "500" )
	PORT_DIPSETTING(    0x08, "1500" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x00, "2500" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0xc0, "Up To Level 2" )
	PORT_DIPSETTING(    0x80, "Up To Level 3" )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( mofflott )
	PORT_INCLUDE(asuka)

	// 0x400000 -> 0x100a92.b
	PORT_MODIFY("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)

	// 0x400002 -> 0x100a93.b
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "20k And Every 50k" )
	PORT_DIPSETTING(    0x08, "50k And Every 100k" )
	PORT_DIPSETTING(    0x04, "100k Only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Number Of Keys" )            PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "B 14" )                      // Hard
	PORT_DIPSETTING(    0x80, "A 16" )                      // Easy
INPUT_PORTS_END

// different players and system inputs than 'asuka'
static INPUT_PORTS_START( cadash )
	// 0x900000 -> 0x10317a ($317a,A5)
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	// 0x900002 -> 0x10317c ($317c,A5)
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, "Starting Time" )         PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "5:00" )
	PORT_DIPSETTING(    0x04, "6:00" )
	PORT_DIPSETTING(    0x0c, "7:00" )
	PORT_DIPSETTING(    0x08, "8:00" )
	// Round cleared   Added time
	//       1            8:00
	//       2           10:00
	//       3            8:00
	//       4            7:00
	//       5            9:00
	PORT_DIPNAME( 0x30, 0x30, "Added Time (after round clear)" ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "Default - 2:00" )
	PORT_DIPSETTING(    0x10, "Default - 1:00" )
	PORT_DIPSETTING(    0x30, "Default" )
	PORT_DIPSETTING(    0x20, "Default + 1:00" )
	PORT_DIPNAME( 0xc0, 0xc0, "Communication Mode" )    PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, "Stand alone" )
	PORT_DIPSETTING(    0x80, "Master" )
	PORT_DIPSETTING(    0x00, "Slave" )
//  PORT_DIPSETTING(    0x40, "Stand alone" )

	PORT_START("IN0")
	CADASH_PLAYERS_INPUT( 1 )

	PORT_START("IN1")
	CADASH_PLAYERS_INPUT( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cadashj )
	PORT_INCLUDE(cadash)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( cadashu )
	PORT_INCLUDE(cadash)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( galmedes )
	PORT_INCLUDE(asuka)

	// 0x400000 -> 0x100982
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)

	// 0x400002 -> 0x100984
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "Every 100k" )
	PORT_DIPSETTING(    0x0c, "100k And Every 200k" )
	PORT_DIPSETTING(    0x04, "150k And Every 200k" )
	PORT_DIPSETTING(    0x00, "Every 200k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )        // Listed as "Unused"
	PORT_DIPNAME( 0x80, 0x80, "Coin Mode" )             PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, "Mode A (Japan)" )        // Mode A is TAITO_COINAGE_JAPAN_OLD
	PORT_DIPSETTING(    0x00, "Mode B (World)" )        // Mode B is TAITO_COINAGE_WORLD
INPUT_PORTS_END

static INPUT_PORTS_START( earthjkr )
	PORT_INCLUDE(asuka)
	// DSWA: 0x400000 -> 0x100932

	// 0x400002 -> 0x1009842
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "100k and 300k" )
	PORT_DIPSETTING(    0x08, "100k only" )
	PORT_DIPSETTING(    0x04, "200k only" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Copyright" )             PORT_DIPLOCATION("SWB:7") // code at 0x00b982 and 0x00dbce
	PORT_DIPSETTING(    0x40, "Visco" )                          // Japan notice screen ON
	PORT_DIPSETTING(    0x00, "Visco (distributed by Romstar)" ) // Japan notice screen OFF
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( earthjkrp )
	PORT_INCLUDE(asuka)

	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( eto )
	PORT_INCLUDE(asuka)
	// DSWA: 0x300000 -> 0x200914

	// 0x300002 -> 0x200916
	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWB:6" )    // value stored at 0x20090a but not read back
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

void base_state::machine_start()
{
	// configure the banks
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x04000);
}

void msm_state::machine_start()
{
	base_state::machine_start();

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_ff));
}

void msm_state::machine_reset()
{
	m_adpcm_pos = 0;
	m_adpcm_ff = false;
}

void cadash_state::machine_start()
{
	base_state::machine_start();

	m_int5_timer = timer_alloc(FUNC(cadash_state::interrupt5), this);
}

void base_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_pc090oj->eof_callback();
	}
}

INTERRUPT_GEN_MEMBER(bonzeadv_state::interrupt)
{
	m_maincpu->set_input_line(4, HOLD_LINE);
	m_cchip->ext_interrupt(ASSERT_LINE);
	m_cchip_irq_clear->adjust(attotime::zero);
}

TIMER_DEVICE_CALLBACK_MEMBER(bonzeadv_state::cchip_irq_clear_cb)
{
	m_cchip->ext_interrupt(CLEAR_LINE);
}

void bonzeadv_state::counters_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(1, data & 0x80);
	machine().bookkeeping().coin_lockout_w(0, data & 0x40);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
	machine().bookkeeping().coin_counter_w(0, data & 0x10);
}

void bonzeadv_state::bonzeadv(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000) / 2);    // checked on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &bonzeadv_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(bonzeadv_state::interrupt));

	Z80(config, m_audiocpu, XTAL(16'000'000) / 4);    // sound CPU, also required for test mode
	m_audiocpu->set_addrmap(AS_PROGRAM, &bonzeadv_state::z80_map);

	TAITO_CCHIP(config, m_cchip, 12_MHz_XTAL); // 12MHz OSC near C-Chip
	m_cchip->in_pa_callback().set_ioport("800007");
	m_cchip->in_pb_callback().set_ioport("800009");
	m_cchip->in_pc_callback().set_ioport("80000B");
	m_cchip->in_ad_callback().set_ioport("80000D");
	m_cchip->out_pb_callback().set(FUNC(bonzeadv_state::counters_w));

	TIMER(config, m_cchip_irq_clear).configure_generic(FUNC(bonzeadv_state::cchip_irq_clear_cb));

	config.set_maximum_quantum(attotime::from_hz(600));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 3*8, 31*8-1);
	screen.set_screen_update(FUNC(bonzeadv_state::screen_update));
	screen.screen_vblank().set(FUNC(bonzeadv_state::screen_vblank));
	screen.set_palette("tc0110pcr");

	PC090OJ(config, m_pc090oj, 0);
	m_pc090oj->set_offsets(0, 8);
	m_pc090oj->set_palette("tc0110pcr");
	m_pc090oj->set_colpri_callback(FUNC(bonzeadv_state::fixed_colpri_cb));

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette("tc0110pcr");

	TC0110PCR(config, "tc0110pcr", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000)/2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

void msm_state::asuka(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000)/2);   // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &msm_state::asuka_map);
	m_maincpu->set_vblank_int("screen", FUNC(msm_state::irq5_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000)/4); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &msm_state::z80_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	tc0220ioc_device &tc0220ioc(TC0220IOC(config, "tc0220ioc", 0));
	tc0220ioc.read_0_callback().set_ioport("DSWA");
	tc0220ioc.read_1_callback().set_ioport("DSWB");
	tc0220ioc.read_2_callback().set_ioport("IN0");
	tc0220ioc.read_3_callback().set_ioport("IN1");
	tc0220ioc.write_4_callback().set(FUNC(msm_state::coin_control_w));
	tc0220ioc.read_7_callback().set_ioport("IN2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(msm_state::screen_update));
	screen.screen_vblank().set(FUNC(msm_state::screen_vblank));
	screen.set_palette("tc0110pcr");

	PC090OJ(config, m_pc090oj, 0);
	m_pc090oj->set_offsets(0, 8);
	m_pc090oj->set_usebuffer(true);
	m_pc090oj->set_palette("tc0110pcr");
	m_pc090oj->set_colpri_callback(FUNC(msm_state::variable_colpri_cb));

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette("tc0110pcr");

	TC0110PCR(config, "tc0110pcr", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 16_MHz_XTAL/4)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_write_handler().set_membank(m_audiobank).mask(0x03);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);

	MSM5205(config, m_msm, XTAL(384'000)); // verified on PCB
	m_msm->vck_legacy_callback().set(FUNC(msm_state::msm5205_vck));  // VCK function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);      // 8 kHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.5);

	LS157(config, m_adpcm_select, 0);
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

void cadash_state::cadash(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(32'000'000)/2);   // 68000p12 running at 16Mhz, verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &cadash_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(cadash_state::interrupt));

	Z80(config, m_audiocpu, XTAL(8'000'000)/2);  // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &cadash_state::z80_map);

	z180_device &subcpu(HD64180RP(config, "subcpu", XTAL(8'000'000)));   // 8MHz HD64180RP8 Z180
	subcpu.set_addrmap(AS_PROGRAM, &cadash_state::sub_map);
	subcpu.set_addrmap(AS_IO, &cadash_state::sub_io);

	config.set_maximum_quantum(attotime::from_hz(600));

	tc0220ioc_device &tc0220ioc(TC0220IOC(config, "tc0220ioc", 0));
	tc0220ioc.read_0_callback().set_ioport("DSWA");
	tc0220ioc.read_1_callback().set_ioport("DSWB");
	tc0220ioc.read_2_callback().set_ioport("IN0");
	tc0220ioc.read_3_callback().set_ioport("IN1");
	tc0220ioc.write_4_callback().set(FUNC(cadash_state::coin_control_w));
	tc0220ioc.read_7_callback().set_ioport("IN2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(cadash_state::screen_update));
	screen.screen_vblank().set(FUNC(cadash_state::screen_vblank));
	screen.set_palette("tc0110pcr");

	PC090OJ(config, m_pc090oj, 0);
	m_pc090oj->set_offsets(0, 8);
	m_pc090oj->set_usebuffer(true);
	m_pc090oj->set_palette("tc0110pcr");
	m_pc090oj->set_colpri_callback(FUNC(cadash_state::fixed_colpri_cb));

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_offsets(1, 0);
	m_tc0100scn->set_palette("tc0110pcr");

	TC0110PCR(config, "tc0110pcr", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 8_MHz_XTAL/2)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_write_handler().set_membank(m_audiobank).mask(0x03);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

void msm_state::mofflott(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 8000000);    // 8 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &msm_state::asuka_map);
	m_maincpu->set_vblank_int("screen", FUNC(msm_state::irq5_line_hold));

	Z80(config, m_audiocpu, 4000000);  // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &msm_state::z80_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	tc0220ioc_device &tc0220ioc(TC0220IOC(config, "tc0220ioc", 0));
	tc0220ioc.read_0_callback().set_ioport("DSWA");
	tc0220ioc.read_1_callback().set_ioport("DSWB");
	tc0220ioc.read_2_callback().set_ioport("IN0");
	tc0220ioc.read_3_callback().set_ioport("IN1");
	tc0220ioc.write_4_callback().set(FUNC(msm_state::coin_control_w));
	tc0220ioc.read_7_callback().set_ioport("IN2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(msm_state::screen_update));
	screen.screen_vblank().set(FUNC(msm_state::screen_vblank));
	screen.set_palette("tc0110pcr");

	PC090OJ(config, m_pc090oj, 0);
	m_pc090oj->set_offsets(0, 8);
	m_pc090oj->set_palette("tc0110pcr");
	m_pc090oj->set_colpri_callback(FUNC(msm_state::variable_colpri_cb));

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_offsets(1, 0);
	m_tc0100scn->set_palette("tc0110pcr");

	TC0110PCR(config, "tc0110pcr", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 4000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_write_handler().set_membank(m_audiobank).mask(0x03);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(msm_state::msm5205_vck));  // VCK function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);      // 8 kHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.5);

	LS157(config, m_adpcm_select, 0);
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

void base_state::eto(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 8000000);    // 8 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &base_state::eto_map);
	m_maincpu->set_vblank_int("screen", FUNC(base_state::irq5_line_hold));

	Z80(config, m_audiocpu, 4000000);  // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &base_state::z80_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	tc0220ioc_device &tc0220ioc(TC0220IOC(config, "tc0220ioc", 0));
	tc0220ioc.read_0_callback().set_ioport("DSWA");
	tc0220ioc.read_1_callback().set_ioport("DSWB");
	tc0220ioc.read_2_callback().set_ioport("IN0");
	tc0220ioc.read_3_callback().set_ioport("IN1");
	tc0220ioc.write_4_callback().set(FUNC(base_state::coin_control_w));
	tc0220ioc.read_7_callback().set_ioport("IN2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(base_state::screen_update));
	screen.screen_vblank().set(FUNC(base_state::screen_vblank));
	screen.set_palette("tc0110pcr");

	PC090OJ(config, m_pc090oj, 0);
	m_pc090oj->set_offsets(0, 8);
	m_pc090oj->set_palette("tc0110pcr");
	m_pc090oj->set_colpri_callback(FUNC(base_state::variable_colpri_cb));

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_offsets(1, 0);
	m_tc0100scn->set_palette("tc0110pcr");

	TC0110PCR(config, "tc0110pcr", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 4000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_write_handler().set_membank(m_audiobank).mask(0x03);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( bonzeadv )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 68000 code
	ROM_LOAD16_BYTE( "b41-09-1.17", 0x00000, 0x10000, CRC(af821fbc) SHA1(55bc13742033a31c92d6268d6b8344062ca78633) )
	ROM_LOAD16_BYTE( "b41-11-1.26", 0x00001, 0x10000, CRC(823fff00) SHA1(b8b8cafbe860136c202d8d9f3ed5a54e2f4df363) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-15.25",   0x20001, 0x10000, CRC(aed7a0d0) SHA1(99ffc0b0e88b81231756610bf48df5365e12603b) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "cchip_b41-05.43", 0x0000, 0x2000, CRC(75c52553) SHA1(87bbaefab90e7d43f63556fbae3e937baf9d397b) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b41-13.20", 0x00000, 0x10000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) ) // banked

	// no ADPCM-A samples

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( bonzeadvo )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 68000 code
	ROM_LOAD16_BYTE( "b41-09.17",   0x00000, 0x10000, CRC(06818710) SHA1(b8045f4e15246231a5645d22bb965953f7fb47a3) )
	ROM_LOAD16_BYTE( "b41-11.26",   0x00001, 0x10000, CRC(33c4c2f4) SHA1(3f1e76932d8f7e06e976b968a711177d25254bef) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-15.25",   0x20001, 0x10000, CRC(aed7a0d0) SHA1(99ffc0b0e88b81231756610bf48df5365e12603b) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "cchip_b41-05.43", 0x0000, 0x2000, CRC(75c52553) SHA1(87bbaefab90e7d43f63556fbae3e937baf9d397b) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b41-13.20", 0x00000, 0x10000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) ) // banked

	// no ADPCM-A samples

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( bonzeadvu )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 68000 code
	ROM_LOAD16_BYTE( "b41-09-1.17", 0x00000, 0x10000, CRC(af821fbc) SHA1(55bc13742033a31c92d6268d6b8344062ca78633) )
	ROM_LOAD16_BYTE( "b41-11-1.26", 0x00001, 0x10000, CRC(823fff00) SHA1(b8b8cafbe860136c202d8d9f3ed5a54e2f4df363) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-14.25",   0x20001, 0x10000, CRC(37def16a) SHA1(b0a3b7206db55e29454672fffadf4e2a64eed873) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "cchip_b41-05.43", 0x0000, 0x2000, CRC(75c52553) SHA1(87bbaefab90e7d43f63556fbae3e937baf9d397b) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b41-13.20", 0x00000, 0x10000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) ) // banked

	// no ADPCM-A samples

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( jigkmgri )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 68000 code
	ROM_LOAD16_BYTE( "b41-09-1.17", 0x00000, 0x10000, CRC(af821fbc) SHA1(55bc13742033a31c92d6268d6b8344062ca78633) )
	ROM_LOAD16_BYTE( "b41-11-1.26", 0x00001, 0x10000, CRC(823fff00) SHA1(b8b8cafbe860136c202d8d9f3ed5a54e2f4df363) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-12.25",   0x20001, 0x10000, CRC(40d9c1fc) SHA1(6f03d263e10559988aaa2be00d9bbf55f2fb864e) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "cchip_b41-05.43", 0x0000, 0x2000, CRC(75c52553) SHA1(87bbaefab90e7d43f63556fbae3e937baf9d397b) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) // Tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b41-13.20", 0x00000, 0x10000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) ) // banked

	// no ADPCM-A samples

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( jigkmgria )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 68000 code
	ROM_LOAD16_BYTE( "b41-09.17",   0x00000, 0x10000, CRC(06818710) SHA1(b8045f4e15246231a5645d22bb965953f7fb47a3) )
	ROM_LOAD16_BYTE( "b41-11.26",   0x00001, 0x10000, CRC(33c4c2f4) SHA1(3f1e76932d8f7e06e976b968a711177d25254bef) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-12.25",   0x20001, 0x10000, CRC(40d9c1fc) SHA1(6f03d263e10559988aaa2be00d9bbf55f2fb864e) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "cchip_b41-05.43", 0x0000, 0x2000, CRC(75c52553) SHA1(87bbaefab90e7d43f63556fbae3e937baf9d397b) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) // Tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b41-13.20", 0x00000, 0x10000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) ) // banked

	// no ADPCM-A samples

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( bonzeadvp ) // Labels consists of hand written checksum values of the ROMs
	ROM_REGION( 0x100000, "maincpu", 0 )     // 68000 code
	ROM_LOAD16_BYTE( "0l.ic17", 0x00000, 0x10000, CRC(9e046e6f) SHA1(a05ed46930bcfa8f59fda6f1d370b841ad261258) )
	ROM_LOAD16_BYTE( "0h.ic26", 0x00001, 0x10000, CRC(3e2b2628) SHA1(66ee0e5d2c38c467edc3f22b83b73643764ae8f0) )
	ROM_LOAD16_BYTE( "1h.ic16", 0x20000, 0x10000, CRC(52f31b98) SHA1(8a20a79350073438522361d3f598afa42f0f62ed) )
	ROM_LOAD16_BYTE( "1l.ic25", 0x20001, 0x10000, CRC(c7e79b98) SHA1(1d92861c6337362cdd9d31a2da944d8eb3171170) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_BYTE( "fd65.ic20",  0x80001, 0x20000, CRC(c32f3bd5) SHA1(d9db14ec26cac5504a61058e87da4e404647ca94) ) // these 4 == b41-01.15 but split
	ROM_LOAD16_BYTE( "49eb.ic26",  0x80000, 0x20000, CRC(c747650b) SHA1(ef03931d233ec1f9e61d45d02abb23e69edd8c15) ) // ^
	ROM_LOAD16_BYTE( "a418.ic23",  0xc0001, 0x20000, CRC(51b02be6) SHA1(20e3423aea359f3ca92dd24f4f87351d93c279b6) ) // ^
	ROM_LOAD16_BYTE( "0e7e.ic28",  0xc0000, 0x20000, CRC(dc1f9fd0) SHA1(88addfa2c3bb854efd88a3556d23a7607b6ec848) ) // ^

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )
	ROM_LOAD( "cchip_b41-05.43", 0x0000, 0x2000, CRC(75c52553) SHA1(87bbaefab90e7d43f63556fbae3e937baf9d397b) ) // is the C-Chip the same as the final?

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_BYTE( "abbe.ic9",   0x00001, 0x20000, CRC(50e6581c) SHA1(230724d65c9b1ea5d72117dca077464dd599ad68) ) // first 2 == first half b41-03.1 but split
	ROM_LOAD16_BYTE( "0ac8.ic15",  0x00000, 0x20000, CRC(29002fc4) SHA1(5ddbefc0d173865802362990e99a3b542c096412) ) // ^
	ROM_LOAD16_BYTE( "5ebf.ic5",   0x40001, 0x20000, CRC(dac6f11f) SHA1(8c79d05ca539ebfbec35c7426c207937745c1949) ) // these 2 have differences
	ROM_LOAD16_BYTE( "77c8.ic12",  0x40000, 0x20000, CRC(d8aaae12) SHA1(240dda7d7e74ffc6a084c39ca19903fd35ad0157) ) // ^

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "9369.ic19",  0x00001, 0x20000, CRC(a9dd7f90) SHA1(c3acf2dcd9325b9a74967d4b9cfff59bdb4045c6) ) // these 4 == b41-02.7 but split
	ROM_LOAD16_BYTE( "e3ed.ic25",  0x00000, 0x20000, CRC(7cc66ee2) SHA1(145d3bd0e3ef765874fc679e709391d516e74ef0) ) // ^
	ROM_LOAD16_BYTE( "03eb.ic16",  0x40001, 0x20000, CRC(39f32715) SHA1(5c555fde1ae0bb1e796e0122157bc694392122f3) ) // ^
	ROM_LOAD16_BYTE( "b8e1.ic22",  0x40000, 0x20000, CRC(15b836cf) SHA1(0f7e5cb6a57c336125909e28af664fe7387947d4) ) // ^

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b41-13.20", 0x00000, 0x10000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) ) // missing from dump // banked

	// no ADPCM-A samples

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "6089.ic17",  0x00000, 0x20000, CRC(b092783c) SHA1(e13f765e2884b6194926bf982595de18376ffef9) ) // these 4 == b41-04.48 but split
	ROM_LOAD( "2e1f.ic14",  0x20000, 0x20000, CRC(df1f87c0) SHA1(ad3df38c22f1bb7bdc449922bd3c2a5c78aa87f8) ) // ^
	ROM_LOAD( "f66e.ic11",  0x40000, 0x20000, CRC(c6df1b3e) SHA1(84d6ad3e3af565060aa4324c6e3e91e4dc5089b6) ) // ^
	ROM_LOAD( "49d7.ic7",   0x60000, 0x20000, CRC(5584c02c) SHA1(00402df66debb257c97a609a37de0f8eeeb6e9f0) ) // ^
ROM_END

// this prototype seems earlier: Test mode doesn't work properly and main CPU ROMs have strings about 'Rainbow' and 'Jump' instead of 'Fire' and 'Warp'. Remnants from Rainbow Island? Did Taito use it as a start for coding Bonze Adventure?
ROM_START( bonzeadvp2 ) // only main CPU first 4 ROMs and audio CPU ROM differ from bonzeadvp. Rest is the same but split in smaller chips. All labels handwritten.
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )     // on BV903001B MAIN PCB, 68000 code
	ROM_LOAD16_BYTE( "prg 0h.ic17",        0x00000, 0x10000, CRC(ce530615) SHA1(23263f5cb5aa28bb8dfae90351c93056f6560fe6) )
	ROM_LOAD16_BYTE( "prg 0l.ic26",        0x00001, 0x10000, CRC(048a0dcb) SHA1(2b74e8fd1aa9119b528660dc82d5ea3d779f3e7d) )
	ROM_LOAD16_BYTE( "prg 1h.ic16",        0x20000, 0x10000, CRC(e5d63e9b) SHA1(e9cb0052811098c0f1f97575fd297cf5fde5d480) )
	ROM_LOAD16_BYTE( "prg 1l europe.ic25", 0x20001, 0x10000, CRC(d04b8e2b) SHA1(0f9f44a76e90c08745481117d2610eb02ea25d99) )
	// 0x040000 - 0x7ffff is intentionally empty
	// these ROMs below are on a ROM P.C.BOARD BT203007B
	ROM_LOAD16_BYTE( "map 0h ad2e.ic7", 0x80000, 0x10000, CRC(ca894028) SHA1(8d650763d7451f3c4e0c84fecee73a6e2b7ce653) )
	ROM_LOAD16_BYTE( "map 0l 676f.ic2", 0x80001, 0x10000, CRC(956bc558) SHA1(65ef807ce203e542c38da689fd519f97192e1062) )
	ROM_LOAD16_BYTE( "map 1h 9cbd.ic8", 0xa0000, 0x10000, CRC(08a5320f) SHA1(8ac6b55fb55c5117bb8f980ea7cfa664025d7aa4) )
	ROM_LOAD16_BYTE( "map 1l 95f6.ic3", 0xa0001, 0x10000, CRC(f65988c0) SHA1(e05764217aa7a8b92a3d03227538d67bb4a994c6) )
	ROM_LOAD16_BYTE( "map 2h 0e7e.ic9", 0xc0000, 0x10000, CRC(4513dcf7) SHA1(637dc4bc28b054f27e323febc8ac47d9863e5c13) )
	ROM_LOAD16_BYTE( "map 2l a418.ic4", 0xc0001, 0x10000, CRC(106475e3) SHA1(5a737e933f39a7820300adc8db7f531fd4449b96) )
	// ic10 and ic5 not populated

	ROM_REGION( 0x2000, "cchip:cchip_eprom", 0 )  // on BV903001B MAIN PCB, not dumped
	ROM_LOAD( "generic 10-9 f3eb.ic43", 0x0000, 0x2000, BAD_DUMP CRC(75c52553) SHA1(87bbaefab90e7d43f63556fbae3e937baf9d397b) ) // actually 汎用 on label (translated as 'generic').
	// Is the C-Chip the same as the final? Probably not as SUM doesn't match

	ROM_REGION( 0x80000, "tc0100scn", ROMREGION_ERASEFF ) // on a second ROM P.C.BOARD BT203007B
	ROM_LOAD16_BYTE( "scn 0h 8711.ic7", 0x00000, 0x10000, CRC(17466da0) SHA1(1de9cc4bcbfa50d6c40d1acd4ac0a47c705e2f09) )
	ROM_LOAD16_BYTE( "scn 0l 7fe0.ic2", 0x00001, 0x10000, CRC(c6413f92) SHA1(ac8b1cfc3ba140a68192df1de6750998f5e357a0) )
	ROM_LOAD16_BYTE( "scn 1h 83b7.ic8", 0x20000, 0x10000, CRC(35fde3c7) SHA1(b6caddff84e607b5ca2dd021151def80ff051f41) )
	ROM_LOAD16_BYTE( "scn 1l 2bde.ic3", 0x20001, 0x10000, CRC(ca459623) SHA1(b071e672df6872fa777d1baae663ec85f92ec728) )
	ROM_LOAD16_BYTE( "scn 2h f411.ic9", 0x40000, 0x10000, CRC(229debcf) SHA1(ea99d0ea0169b72109b830cd335a43eb122c0e34) )
	ROM_LOAD16_BYTE( "scn 2l db08.ic4", 0x40001, 0x10000, CRC(2431e8db) SHA1(1773f0dfe59c4cb9b114a784f345b7fc05bb8934) )
	// ic10 and ic5 not populated

	ROM_REGION( 0x80000, "pc090oj", 0 ) // on a third ROM P.C.BOARD BT203007B
	ROM_LOAD16_BYTE( "obj 0h 6ce2.ic7",  0x00000, 0x10000, CRC(d5bff0fd) SHA1(c4998a788c48371d0f83ebec8987b584c5e94d1a) )
	ROM_LOAD16_BYTE( "obj 0l c67e.ic2",  0x00001, 0x10000, CRC(2f9615a8) SHA1(d89f122ce4af193f56560f5183993e242b22551c) ) // actual SUM of the dump is c672, but given data matches the other existing dumps the one on the label is probably a typo
	ROM_LOAD16_BYTE( "obj 1h 7708.ic8",  0x20000, 0x10000, CRC(81357279) SHA1(241954743238815a18598d76d96a8a312e087351) ) // actual SUM of the dump is 770b, but given data matches the other existing dumps the one on the label is probably a typo
	ROM_LOAD16_BYTE( "obj 1l ccf7.ic3",  0x20001, 0x10000, CRC(c7a654d9) SHA1(58b7e5e2e05f7980ad199083b31b5c406b99298e) )
	ROM_LOAD16_BYTE( "obj 2h 818b.ic9",  0x40000, 0x10000, CRC(d614732b) SHA1(0bfa5dd2876a3480d4f5f3e2baeb94e9601a691a) )
	ROM_LOAD16_BYTE( "obj 2l 6096.ic4",  0x40001, 0x10000, CRC(6dd67af8) SHA1(7d9d836d0aee2d3d636f28af84bf5f04fc54db74) )
	ROM_LOAD16_BYTE( "obj 3h 3756.ic10", 0x60000, 0x10000, CRC(981e66a9) SHA1(98db800e99a2508fbe05a8bf12617abd18721f4b) )
	ROM_LOAD16_BYTE( "obj 3l a355.ic5",  0x60001, 0x10000, CRC(173ddd11) SHA1(c28fd4f728066f7926752f9af1115bc90ce838fc) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on BV903001B MAIN PCB
	ROM_LOAD( "sound main 3-9.ic20", 0x00000, 0x10000, CRC(2b4fc69a) SHA1(030ba4eb81c59174527edcda65f2ed94bd768140) ) // actual label is 3/9 instead of 3-9, possibly a date?

	// no ADPCM-A samples

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // on KE7X0003B 4M 8BIT ROM BOARD, // actual label is 3/8 instead of 3-8, possibly a date?
	ROM_LOAD( "sound 0h 3-8.ic2",  0x00000, 0x10000, CRC(2996e756) SHA1(b6fa6f20bdecb1e6ed7166083c9581f8f97f9836) )
	ROM_LOAD( "sound 1h 3-8.ic3",  0x10000, 0x10000, CRC(780368ac) SHA1(bc5592fbefa310b1c496b4a9f60e29cb1efa9070) )
	ROM_LOAD( "sound 2h 3-8.ic4",  0x20000, 0x10000, CRC(8f3b9fa5) SHA1(c4d536af730336b70813653e3b216f7ca9518909) )
	ROM_LOAD( "sound 3h 3-8.ic5",  0x30000, 0x10000, CRC(1a8be621) SHA1(410a4f93e95f1824643ee918a06fb8c3ac93765d) )
	ROM_LOAD( "sound 0l 3-8.ic7",  0x40000, 0x10000, CRC(3711abfa) SHA1(35237b13b201f2883bc969ebbb51f7e112f0eb35) )
	ROM_LOAD( "sound 1l 3-8.ic8",  0x50000, 0x10000, CRC(f24a3d1a) SHA1(c2d8a8e4835429b7830dad5a8d3cdf1afe21b7d6) )
	ROM_LOAD( "sound 2l 3-8.ic9",  0x60000, 0x10000, CRC(5987900c) SHA1(1109e48d196b3ed5788326df235f1ad950492d17) )
	ROM_LOAD( "sound 3l 3-8.ic10", 0x70000, 0x10000, CRC(e8a6a9e6) SHA1(435a599d188f6c4650bd3dd72d8080541824148d) )
ROM_END

ROM_START( asuka ) // Taito PCB: ASKA&ASKA - K1100388A / J1100169A
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "b68-13.ic23", 0x00000, 0x20000, CRC(855efb3e) SHA1(644e02e207adeaec7839c824688d88ab8d046418) )
	ROM_LOAD16_BYTE( "b68-12.ic8",  0x00001, 0x20000, CRC(271eeee9) SHA1(c08e347be4aae929c0ab95ff7618edaa1a7d6da9) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "b68-03.ic30", 0x80000, 0x80000, CRC(d3a59b10) SHA1(35a2ff18b64e73ac5e17484354c0cc58bc2cd7fc) )    // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b68-01.ic3", 0x00000, 0x80000, CRC(89f32c94) SHA1(74fbb699e05e2336509cb5ac06ed94335ff870d5) )   // SCR tiles (8 x 8)

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b68-02.ic6", 0x00000, 0x80000, CRC(f5018cd3) SHA1(860ce140ae369556d03d5d78987b87c0d6070df5) ) // Sprites (16 x 16)
	ROM_LOAD16_BYTE     ( "b68-07.ic5", 0x80001, 0x10000, CRC(c113acc8) SHA1(613c61a78df73dcb0b9c9018ae829e865baac772) )
	ROM_LOAD16_BYTE     ( "b68-06.ic4", 0x80000, 0x10000, CRC(f517e64d) SHA1(8be491bfe0f7eed58521de9d31da677acf635c23) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b68-11.ic27", 0x00000, 0x10000, CRC(c378b508) SHA1(1b145fe736b924f298e02532cf9f26cc18b42ca7) ) // banked

	ROM_REGION( 0x10000, "msm", 0 )
	ROM_LOAD( "b68-10.ic24", 0x00000, 0x10000, CRC(387aaf40) SHA1(47c583564ef1d49ece15f97221b2e073e8fb0544) )

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( asukaj )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "b68-09-1.ic23", 0x00000, 0x20000, CRC(d61be555) SHA1(c1c711df1f39ae6ec9ee964c93fa4d7ef46cd271) )
	ROM_LOAD16_BYTE( "b68-08-1.ic8",  0x00001, 0x20000, CRC(e916f17b) SHA1(f5b662ac1533c76beac501a053528a595f36258a) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "b68-03.ic30", 0x80000, 0x80000, CRC(d3a59b10) SHA1(35a2ff18b64e73ac5e17484354c0cc58bc2cd7fc) )    // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b68-01.ic3", 0x00000, 0x80000, CRC(89f32c94) SHA1(74fbb699e05e2336509cb5ac06ed94335ff870d5) )   // SCR tiles (8 x 8)

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b68-02.ic6", 0x00000, 0x80000, CRC(f5018cd3) SHA1(860ce140ae369556d03d5d78987b87c0d6070df5) ) // Sprites (16 x 16)
	ROM_LOAD16_BYTE     ( "b68-07.ic5", 0x80001, 0x10000, CRC(c113acc8) SHA1(613c61a78df73dcb0b9c9018ae829e865baac772) )
	ROM_LOAD16_BYTE     ( "b68-06.ic4", 0x80000, 0x10000, CRC(f517e64d) SHA1(8be491bfe0f7eed58521de9d31da677acf635c23) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b68-11.ic27", 0x00000, 0x10000, CRC(c378b508) SHA1(1b145fe736b924f298e02532cf9f26cc18b42ca7) ) // banked

	ROM_REGION( 0x10000, "msm", 0 )
	ROM_LOAD( "b68-10.ic24", 0x00000, 0x10000, CRC(387aaf40) SHA1(47c583564ef1d49ece15f97221b2e073e8fb0544) )

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( asukaja )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "b68-09.ic23", 0x00000, 0x20000, CRC(1eaa1bbb) SHA1(01ca6a5f3c47dab49654b84601119714eb329cc5) )
	ROM_LOAD16_BYTE( "b68-08.ic8",  0x00001, 0x20000, CRC(8cc96e60) SHA1(dc94f3fd48c0407ec72e8330bc688e9e16d39213) )
	// 0x040000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "b68-03.ic30", 0x80000, 0x80000, CRC(d3a59b10) SHA1(35a2ff18b64e73ac5e17484354c0cc58bc2cd7fc) )    // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b68-01.ic3", 0x00000, 0x80000, CRC(89f32c94) SHA1(74fbb699e05e2336509cb5ac06ed94335ff870d5) )   // SCR tiles (8 x 8)

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "b68-02.ic6", 0x00000, 0x80000, CRC(f5018cd3) SHA1(860ce140ae369556d03d5d78987b87c0d6070df5) ) // Sprites (16 x 16)
	ROM_LOAD16_BYTE     ( "b68-07.ic5", 0x80001, 0x10000, CRC(c113acc8) SHA1(613c61a78df73dcb0b9c9018ae829e865baac772) )
	ROM_LOAD16_BYTE     ( "b68-06.ic4", 0x80000, 0x10000, CRC(f517e64d) SHA1(8be491bfe0f7eed58521de9d31da677acf635c23) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b68-11.ic27", 0x00000, 0x10000, CRC(c378b508) SHA1(1b145fe736b924f298e02532cf9f26cc18b42ca7) ) // banked

	ROM_REGION( 0x10000, "msm", 0 )
	ROM_LOAD( "b68-10.ic24", 0x00000, 0x10000, CRC(387aaf40) SHA1(47c583564ef1d49ece15f97221b2e073e8fb0544) )

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( mofflott )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "c17-09.bin",  0x00000, 0x20000, CRC(05ee110f) SHA1(8cedd911d3fdcca1e409260d12dd03a2fb35ef86) )
	ROM_LOAD16_BYTE( "c17-08.bin",  0x00001, 0x20000, CRC(d0aacffd) SHA1(2c5ec4020aad2c1cd3a004dc70a12e0d77eb6aa7) )
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "c17-03.bin",  0x80000, 0x80000, CRC(27047fc3) SHA1(1f88a7a42a94bac0e164a69896ae168ab821fbb3) )    // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c17-01.bin",  0x00000, 0x80000, CRC(e9466d42) SHA1(93d533a9a992e3ff537e914577ede41729235826) )   // SCR tiles (8 x 8)

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c17-02.bin", 0x00000, 0x80000, CRC(8860a8db) SHA1(372adea8835a9524ece30ab71181ef9d05b120e9) ) // Sprites (16 x 16)
	ROM_LOAD16_BYTE     ( "c17-05.bin", 0x80001, 0x10000, CRC(57ac4741) SHA1(3188ff0866324c68fba8e9745a0cb186784cb53d) )
	ROM_LOAD16_BYTE     ( "c17-04.bin", 0x80000, 0x10000, CRC(f4250410) SHA1(1f5f6baca4aa695ce2ae5c65adcb460da872a239) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c17-07.bin", 0x00000, 0x10000, CRC(cdb7bc2c) SHA1(5113055c954a39918436db75cc06b53c29c60728) ) // banked

	ROM_REGION( 0x10000, "msm", 0 )
	ROM_LOAD( "c17-06.bin", 0x00000, 0x10000, CRC(5c332125) SHA1(408f42df18b38347c8a4e177a9484162a66877e1) )
ROM_END

ROM_START( cadash )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_14.ic11",  0x00000, 0x20000, CRC(5daf13fb) SHA1(c2be42b2cdc90b6463ce87211cf711c951b17fab) )
	ROM_LOAD16_BYTE( "c21_16.ic15",  0x00001, 0x20000, CRC(cbaa2e75) SHA1(c41ea71f2b0e72bf993dfcfd30f1994cae9f52a0) )
	ROM_LOAD16_BYTE( "c21_13.ic10",  0x40000, 0x20000, CRC(6b9e0ee9) SHA1(06314b9c0be19314e6b6ecb5274a63eb36b642f5) )
	ROM_LOAD16_BYTE( "c21_17.ic14",  0x40001, 0x20000, CRC(bf9a578a) SHA1(42bde46081db6be2f61eaf171438ecc9264d18be) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashp )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "euro main h.ic11",  0x00000, 0x20000, CRC(9dae00ca) SHA1(e80a069d1afbc624fa3e9cbe9c18bcd0364b3889) )
	ROM_LOAD16_BYTE( "euro main l.ic15",  0x00001, 0x20000, CRC(ba66b6a5) SHA1(26040c847209c2fd25805eefb99c280b12564a17) )
	ROM_LOAD16_BYTE( "euro data h.bin",   0x40000, 0x20000, CRC(bcce9d44) SHA1(e20a79e1e1c3367f92d05a2313cbeee122c1d3c5) )
	ROM_LOAD16_BYTE( "euro data l.bin",   0x40001, 0x20000, CRC(21f5b591) SHA1(6ff70f79bca705407ab9a4825466826bc2dbab32) )

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "com.ic57",   0x00000, 0x08000, CRC(bae1a92f) SHA1(dbe10a02a294dfa7d6052a692c3a49aad85d6ffd) )

	// all other ROMs are under some kind of epoxy, assuming to be the same..
	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashj )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_04-2.ic11",  0x00000, 0x20000, CRC(7a9c1828) SHA1(491eea29efc47159ad904e734a980c444bfbd8aa) )
	ROM_LOAD16_BYTE( "c21_06-2.ic15",  0x00001, 0x20000, CRC(c9d6440a) SHA1(2555af4c4043811a53e9f069d97571672237c18e) )
	ROM_LOAD16_BYTE( "c21_03-2.ic10",  0x40000, 0x20000, CRC(30afc320) SHA1(d4c1d1ef30be633244c6b71b24491d6eb3562cef) )
	ROM_LOAD16_BYTE( "c21_05-2.ic14",  0x40001, 0x20000, CRC(2bc93209) SHA1(3352659ea9364ca9462343f03e26dd10087d6834) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", ROMREGION_ERASE00 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashj1 )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_04-1.ic11",  0x00000, 0x20000, CRC(cc22ebe5) SHA1(170787e7ab2055af593f3f2596cab44feb53b060) )
	ROM_LOAD16_BYTE( "c21_06-1.ic15",  0x00001, 0x20000, CRC(26e03304) SHA1(c8b271e455dde312c8871dc8dd4d3f0f063fa894) )
	ROM_LOAD16_BYTE( "c21_03-1.ic10",  0x40000, 0x20000, CRC(c54888ed) SHA1(8a58da25eb8986a1c6496290e82344840badef0a) )
	ROM_LOAD16_BYTE( "c21_05-1.ic14",  0x40001, 0x20000, CRC(834018d2) SHA1(0b1a29316f90a98478b47d7fa3f05c68e5ddd9b3) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", ROMREGION_ERASE00 )  // HD64180RP8 code (link), the board this set was from did not have the link section populated
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashjo )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_04.ic11",  0x00000, 0x20000, CRC(be7d3f12) SHA1(16e445317d053a19fc430625743f4afa54ce1d8e) )
	ROM_LOAD16_BYTE( "c21_06.ic15",  0x00001, 0x20000, CRC(1db3fe02) SHA1(3abb341596eed8f991ed2002d2e7b71fa2dd099d) )
	ROM_LOAD16_BYTE( "c21_03.ic10",  0x40000, 0x20000, CRC(7e31c5a3) SHA1(a0abc5862d594800934a4792de4ec655f60c1f23) )
	ROM_LOAD16_BYTE( "c21_05.ic14",  0x40001, 0x20000, CRC(a4f4901d) SHA1(a3e8d9ad033e6fb1c8383669e6e59f2f79386e32) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", ROMREGION_ERASE00 )  // HD64180RP8 code (link), the board this set was from did not have the link section populated
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashu )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_14-2.ic11",  0x00000, 0x20000, CRC(f823d418) SHA1(5b4a0b42fb5a2e1ba1e25465762cdc24c41b33f8) )
	ROM_LOAD16_BYTE( "c21_16-2.ic15",  0x00001, 0x20000, CRC(90165577) SHA1(b8e163cf60933aaaa53873fbc866d8d1750240ab) )
	ROM_LOAD16_BYTE( "c21_13-2.ic10",  0x40000, 0x20000, CRC(92dcc3ae) SHA1(7d11c6d8b54468f0c56b4f58adc176e4d46a62eb) )
	ROM_LOAD16_BYTE( "c21_15-2.ic14",  0x40001, 0x20000, CRC(f915d26a) SHA1(cdc7e6a35077ebff937350aee1eee332352e9383) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashu1 )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_14-x.ic11",  0x00000, 0x20000, CRC(64f22e5e) SHA1(3a68a4460173e7a90611c2ade8aa313700f98cf9) ) // These program ROMs are most likely Rev 1
	ROM_LOAD16_BYTE( "c21_16-x.ic15",  0x00001, 0x20000, CRC(77f5d79f) SHA1(eddfa55fd1e3038f98e3b49ec4e146b0edb92b8f) ) // Need to verify proper label and revision
	ROM_LOAD16_BYTE( "c21_13-x.ic10",  0x40000, 0x20000, CRC(488fd6d6) SHA1(fda175bbc4f41c821922fb7310b14f6c74575174) )
	ROM_LOAD16_BYTE( "c21_15-x.ic14",  0x40001, 0x20000, CRC(3a44a8b4) SHA1(19a9775872824d0f3596a1cea379f7b325a7e878) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashi )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_27-1.ic11",  0x00000, 0x20000, CRC(d1d9e613) SHA1(296c188daec962bdb4e78e20f1cc4c7d1f4dda09) )
	ROM_LOAD16_BYTE( "c21_29-1.ic15",  0x00001, 0x20000, CRC(142256ef) SHA1(9ffc64d7c900bfa0300de9e6d18c7458f4c76ed7) )
	ROM_LOAD16_BYTE( "c21_26-1.ic10",  0x40000, 0x20000, CRC(c9cf6e30) SHA1(872c871cd60e0aa7149660277f67f90748d82743) )
	ROM_LOAD16_BYTE( "c21_28-1.ic14",  0x40001, 0x20000, CRC(641fc9dd) SHA1(1497e39f6b250de39ef2785aaca7e68a803612fa) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashf )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_19.ic11",  0x00000, 0x20000, CRC(4d70543b) SHA1(4fc8d4a9f978232a484af3d91bf8eea2afc839a7) )
	ROM_LOAD16_BYTE( "c21_21.ic15",  0x00001, 0x20000, CRC(0e5b9950) SHA1(872919bab057fc9e5baffe5dfe35b1b8c1ed0105) )
	ROM_LOAD16_BYTE( "c21_18.ic10",  0x40000, 0x20000, CRC(8a19e59b) SHA1(b42a0c8273ca6f202a5dc6e33965423da3b074d8) )
	ROM_LOAD16_BYTE( "c21_20.ic14",  0x40001, 0x20000, CRC(b96acfd9) SHA1(d05b55fd5bbf8fd0e5a7272d1951f27a4900371f) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashg )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_23-1.ic11",  0x00000, 0x20000, CRC(30ddbabe) SHA1(f48ea6fe36c4d9fe291232fd7adddb8f3547270f) )
	ROM_LOAD16_BYTE( "c21_25-1.ic15",  0x00001, 0x20000, CRC(24e10611) SHA1(6f406267777dd693a3869ccb34fe3f2f8dea857d) )
	ROM_LOAD16_BYTE( "c21_22-1.ic10",  0x40000, 0x20000, CRC(daf58b2d) SHA1(7a64df848f46f27bb6f9757ce0cc81311c2f172f) )
	ROM_LOAD16_BYTE( "c21_24-1.ic14",  0x40001, 0x20000, CRC(2359b93e) SHA1(9a5ce34dd8667a987ab8b6e6246f0ad032af868f) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashgo )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "c21_23.ic11",  0x00000, 0x20000, CRC(fad37785) SHA1(f61bccb29d354a57cebaa0c773f212bffbba19d5) )
	ROM_LOAD16_BYTE( "c21_25.ic15",  0x00001, 0x20000, CRC(594dda9f) SHA1(ab9fcd44fb316b64cbb8a5265563dcade417895d) )
	ROM_LOAD16_BYTE( "c21_22.ic10",  0x40000, 0x20000, CRC(7610a9b4) SHA1(25c858f25efdbd4c25cbd1cc64fc68c9aba762ea) )
	ROM_LOAD16_BYTE( "c21_24.ic14",  0x40001, 0x20000, CRC(551d947e) SHA1(237397dfe1e4dcd76acc37536304dd526d2d6f41) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashs ) // no labels on the program ROMs
	ROM_REGION( 0x80000, "maincpu", 0 )     // 512k for 68000 code
	ROM_LOAD16_BYTE( "ic11",  0x00000, 0x20000, CRC(6c11743e) SHA1(847266a04090b34e20985d65f4d1f7e7776efa02) )
	ROM_LOAD16_BYTE( "ic15",  0x00001, 0x20000, CRC(73224356) SHA1(28f8fa58bf62d8f9aa94115b84568f20810f5342) )
	ROM_LOAD16_BYTE( "ic10",  0x40000, 0x20000, CRC(57d659d9) SHA1(6bf0c7d514a65bd1a0d51fe1c6bb208419d016e6) )
	ROM_LOAD16_BYTE( "ic14",  0x40001, 0x20000, CRC(53c1b195) SHA1(5985304fa65a3f33a26fbd5dcccb153de6860841) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c21-08.38",   0x00000, 0x10000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) ) // banked

	ROM_REGION( 0x08000, "subcpu", 0 )  // HD64180RP8 code (link)
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( galmedes ) // Taito PCB: K1100388A / J1100169A
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "gm-prg1.ic23", 0x00000, 0x20000, CRC(32a70753) SHA1(3bd094b7ae600dbc87ba74e8b2d6b86a68346f4f) )
	ROM_LOAD16_BYTE( "gm-prg0.ic8",  0x00001, 0x20000, CRC(fae546a4) SHA1(484cad5287daa495b347f6b5b065f3b3d02d8f0e) )
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "gm-30.ic30",   0x80000, 0x80000, CRC(4da2a407) SHA1(7bd0eb629dd7022a16e328612c786c544267f7bc) )   // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "gm-scn.ic3", 0x00000, 0x80000, CRC(3bab0581) SHA1(56b79a4ffd9f4880a63450b7d1b79f029de75e20) )    // SCR tiles (8 x 8)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "gm-obj.ic6", 0x00000, 0x80000, CRC(7a4a1315) SHA1(e2010ee4222415fd55ba3102003be4151d29e39b) )    // Sprites (16 x 16)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gm-snd.ic27", 0x00000, 0x10000, CRC(d6f56c21) SHA1(ff9743448ac8ce57a2f8c33a26145e7b92cbe3c3) ) // banked

	ROM_REGION( 0x10000, "msm", ROMREGION_ERASEFF )
	// Empty socket on Galmedes - but sound chips present

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( earthjkr ) // Taito PCB: K1100388A / J1100169A
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "ej_3b.ic23",  0x00000, 0x20000, BAD_DUMP CRC(bdd86fc2) SHA1(96578860ed03718f8a68847b367eac6c81b79ca2) )
	ROM_LOAD16_BYTE( "ej_3a.ic8",   0x00001, 0x20000, CRC(9c8050c6) SHA1(076c882f75787e8120de66ff0dcd2cb820513c45) )
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "ej_30e.ic30", 0x80000, 0x80000, CRC(49d1f77f) SHA1(f6c9b2fc88b77cc9baa5be48da5c3eb72310e471) ) // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_chr-0.ic3", 0x00000, 0x80000, CRC(ac675297) SHA1(2a34e1eae3a4be84dbf709053f5e8a781b1073fc) )    // SCR tiles (8 x 8) - mask ROM

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_obj-0.ic6", 0x00000, 0x80000, CRC(5f21ac47) SHA1(45c94ffb53ee9b822b0676f6fb151fed4ce6d967) ) // Sprites (16 x 16) - mask ROM
	ROM_LOAD16_BYTE     ( "ej_1.ic5",     0x80001, 0x10000, CRC(cb4891db) SHA1(af1112608cdd897ef6028ef617f5ca69d7964861) )
	ROM_LOAD16_BYTE     ( "ej_0.ic4",     0x80000, 0x10000, CRC(b612086f) SHA1(625748fcb698ec57b7b3ce46019cf85de99aaaa1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ej_2.ic27", 0x00000, 0x10000, CRC(42ba2566) SHA1(c437388684b565c7504d6bad6accd73aa000faca) ) // banked

	ROM_REGION( 0x10000, "msm", ROMREGION_ERASEFF )
	// Empty socket on U.N. Defense Force: Earth Joker - but sound chips present

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( earthjkra )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	// Blank ROM labels, might be for the Korean market, although region handling is unchanged. Very close to parent set, but some clearly additional intentional changes that can't be attributed to the bitrot in the parent
	ROM_LOAD16_BYTE( "ejok_ic23",  0x00000, 0x20000, CRC(cbd29731) SHA1(4cbbdc9352cb203b6b5ec37c1b11c09d827960fc) ) // ejok_ic23 vs ej_3b.ic23 99.945831% similar (71 changed bytes)
	ROM_LOAD16_BYTE( "ejok_ic8",   0x00001, 0x20000, CRC(cfd4953c) SHA1(6aa91ebca4444070841c1f8307430bc787656df3) ) // ejok_ic8  vs ej_3a.ic8  99.945831% similar (71 changed bytes)
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "ejok_ic30", 0x80000, 0x80000, CRC(49d1f77f) SHA1(f6c9b2fc88b77cc9baa5be48da5c3eb72310e471) ) // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_chr-0.ic3", 0x00000, 0x80000, CRC(ac675297) SHA1(2a34e1eae3a4be84dbf709053f5e8a781b1073fc) )    // SCR tiles (8 x 8) - mask ROM

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_obj-0.ic6", 0x00000, 0x80000, CRC(5f21ac47) SHA1(45c94ffb53ee9b822b0676f6fb151fed4ce6d967) ) // Sprites (16 x 16) - mask ROM
	ROM_LOAD16_BYTE     ( "ejok_ic5",     0x80001, 0x10000, CRC(cb4891db) SHA1(af1112608cdd897ef6028ef617f5ca69d7964861) )
	ROM_LOAD16_BYTE     ( "ejok_ic4",     0x80000, 0x10000, CRC(b612086f) SHA1(625748fcb698ec57b7b3ce46019cf85de99aaaa1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ejok_ic28", 0x00000, 0x10000, CRC(42ba2566) SHA1(c437388684b565c7504d6bad6accd73aa000faca) ) // banked

	ROM_REGION( 0x10000, "msm", ROMREGION_ERASEFF )
	// Empty socket on U.N. Defense Force: Earth Joker - but sound chips present

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( earthjkrb ) // Taito PCB: K1100726A / J1100169B
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	// Very close to earthjkra set. Labels are numbered in the same way as earthjkrp, but 3 and 4 ones are swapped (Maybe a typo in earthjkrp ?). In this case 4 is placed at ic24 position and 3 is placed at ic8 position
	ROM_LOAD16_BYTE( "4.ic23", 0x00000, 0x20000, CRC(250f09f8) SHA1(124f65a499414b4ec06cf6c370850cdc962dd2ee) ) // 4.ic23 vs ejok_ic23 99.967957% similar (42 changed bytes)
	ROM_LOAD16_BYTE( "3.ic8",  0x00001, 0x20000, CRC(88fc1c5d) SHA1(83d4177603c5671ece906810f01284a477388bf7) ) // 3.ic8  vs ejok_ic8  99.967957% similar (42 changed bytes)
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "ic30e.ic30", 0x80000, 0x80000, CRC(49d1f77f) SHA1(f6c9b2fc88b77cc9baa5be48da5c3eb72310e471) ) // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_chr-0.ic3", 0x00000, 0x80000, CRC(ac675297) SHA1(2a34e1eae3a4be84dbf709053f5e8a781b1073fc) )    // SCR tiles (8 x 8) - mask ROM

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_obj-0.ic6", 0x00000, 0x80000, CRC(5f21ac47) SHA1(45c94ffb53ee9b822b0676f6fb151fed4ce6d967) ) // Sprites (16 x 16) - mask ROM
	ROM_LOAD16_BYTE     ( "1.ic5",        0x80001, 0x10000, CRC(cb4891db) SHA1(af1112608cdd897ef6028ef617f5ca69d7964861) )
	ROM_LOAD16_BYTE     ( "0.ic4",        0x80000, 0x10000, CRC(b612086f) SHA1(625748fcb698ec57b7b3ce46019cf85de99aaaa1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2.ic27", 0x00000, 0x10000, CRC(42ba2566) SHA1(c437388684b565c7504d6bad6accd73aa000faca) ) // banked

	ROM_REGION( 0x10000, "msm", ROMREGION_ERASEFF )
	// Empty socket on U.N. Defense Force: Earth Joker - but sound chips present

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

// Known to exist (not dumped) a Japanese version with ROMs 3 & 4 also stamped "A" same as above or different version??
// Also known to exist (not dumped) a US version of Earth Joker, title screen shows "DISTRIBUTED BY ROMSTAR, INC."  ROMs were numbered
// from 0 through 4 and the fix ROM at IC30 is labeled 1 even though IC5 is also labeled as 1 similar to the below set:
// (ROMSTAR license is set by a dipswitch, is set mentioned above really undumped?)

ROM_START( earthjkrp ) // was production PCB complete with mask ROM, could just be an early revision, not proto
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "3.ic23", 0x00001, 0x20000, CRC(26c33225) SHA1(b039c47d0776c90813ab52c867e95989cab2c567) )
	ROM_LOAD16_BYTE( "4.ic8",  0x00000, 0x20000, CRC(e9b1ef0c) SHA1(5e104146d37922a8c7e93696c2c156223653025b) )
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "5.ic30", 0x80000, 0x80000, CRC(bf760b2d) SHA1(4aff36623e5a31ab86c77461fa93e40e77f08edd) ) // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_chr-0.ic3", 0x00000, 0x80000, CRC(ac675297) SHA1(2a34e1eae3a4be84dbf709053f5e8a781b1073fc) )    // SCR tiles (8 x 8) - mask ROM

	ROM_REGION( 0xa0000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "ej_obj-0.ic6", 0x00000, 0x80000, CRC(5f21ac47) SHA1(45c94ffb53ee9b822b0676f6fb151fed4ce6d967) ) // Sprites (16 x 16) - mask ROM
	ROM_LOAD16_BYTE     ( "1.ic5",        0x80001, 0x10000, CRC(cb4891db) SHA1(af1112608cdd897ef6028ef617f5ca69d7964861) )
	ROM_LOAD16_BYTE     ( "0.ic4",        0x80000, 0x10000, CRC(b612086f) SHA1(625748fcb698ec57b7b3ce46019cf85de99aaaa1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2.ic27", 0x00000, 0x10000, CRC(42ba2566) SHA1(c437388684b565c7504d6bad6accd73aa000faca) ) // banked

	ROM_REGION( 0x10000, "msm", ROMREGION_ERASEFF )
	// Empty socket on U.N. Defense Force: Earth Joker - but sound chips present

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.ic32", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.ic43", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( eto )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "eto-1.ic23",  0x00000, 0x20000, CRC(44286597) SHA1(ac37e5edbf9d187f60232adc5e9ebed45b3d2fe2) )
	ROM_LOAD16_BYTE( "eto-0.ic8",   0x00001, 0x20000, CRC(57b79370) SHA1(25f83eada982ef654260fe92016d42a90005a05c) )
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "eto-2.ic30",  0x80000, 0x80000, CRC(12f46fb5) SHA1(04db8b6ccd0051668bd2930275efa0265c0cfd2b) )    // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "eto-4.ic3", 0x00000, 0x80000, CRC(a8768939) SHA1(a2cbbd3e10ed48ba32a680b2e40ea03900cf33fa) )   // Sprites (16 x 16)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "eto-3.ic6", 0x00000, 0x80000, CRC(dd247397) SHA1(53a7bf877fd7e5f3daf295a698f4012447b6f113) )   // SCR tiles (8 x 8)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eto-5.ic27", 0x00000, 0x10000, CRC(b3689da0) SHA1(812d2e0a794403df9f0a5035784f14cd070ea080) ) // banked
ROM_END

// Possible prototype or test version
// Smaller program (ends at 0x01eee4, while eto ends at 0x020952)
// Smaller data (ends at 0x0a74aa, while eto ends at 0x0fdd94)
ROM_START( etoa )
	ROM_REGION( 0x100000, "maincpu", 0 )     // 1024k for 68000 code
	ROM_LOAD16_BYTE( "pe.ic23",  0x00000, 0x20000, CRC(36a6a742) SHA1(32d49842cb46c8acfc44fbbf8da54e25541c2a13) )
	ROM_LOAD16_BYTE( "po.ic8",   0x00001, 0x20000, CRC(bc86f328) SHA1(ff746e7f17e62c09af2e1011583ee1aedce782d4) )
	// 0x40000 - 0x7ffff is intentionally empty
	ROM_LOAD16_WORD( "pd.ic30",  0x80000, 0x80000, CRC(39e6a0f3) SHA1(f75d4313db5f292c5fd8f86f4e6871ee244d30d3) )    // Fix ROM

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "sc.ic3", 0x00000, 0x80000, CRC(a8768939) SHA1(a2cbbd3e10ed48ba32a680b2e40ea03900cf33fa) )   // Sprites (16 x 16)

	ROM_REGION( 0x80000, "pc090oj", 0 )
	ROM_LOAD16_WORD_SWAP( "ob.ic6", 0x00000, 0x80000, CRC(dd247397) SHA1(53a7bf877fd7e5f3daf295a698f4012447b6f113) )   // SCR tiles (8 x 8)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sd.ic27", 0x00000, 0x10000, CRC(b3689da0) SHA1(812d2e0a794403df9f0a5035784f14cd070ea080) ) // banked
ROM_END


void base_state::init_earthjkr()
{
	u16 *rom = (u16 *)memregion("maincpu")->base();
	// 357c -> 317c, I think this is bitrot, see ROM loading for which ROM needs redumping, causes rowscroll to be broken on final stage (writes to ROM area instead)
	// code is correct in the 'prototype?' set
	rom[0x7aaa/2] = 0x317c;
}

} // Anonymous namespace


GAME( 1988, bonzeadv,  0,        bonzeadv, bonzeadv,  bonzeadv_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Bonze Adventure (World, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bonzeadvo, bonzeadv, bonzeadv, bonzeadv,  bonzeadv_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Bonze Adventure (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bonzeadvu, bonzeadv, bonzeadv, jigkmgri,  bonzeadv_state, empty_init,    ROT0,   "Taito America Corporation", "Bonze Adventure (US, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, jigkmgri,  bonzeadv, bonzeadv, jigkmgri,  bonzeadv_state, empty_init,    ROT0,   "Taito Corporation",         "Jigoku Meguri (Japan, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, jigkmgria, bonzeadv, bonzeadv, jigkmgri,  bonzeadv_state, empty_init,    ROT0,   "Taito Corporation",         "Jigoku Meguri (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bonzeadvp, bonzeadv, bonzeadv, jigkmgri,  bonzeadv_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Bonze Adventure (World, prototype, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bonzeadvp2,bonzeadv, bonzeadv, jigkmgri,  bonzeadv_state, empty_init,    ROT0,   "Taito Corporation Japan",   "Bonze Adventure (World, prototype, older)", MACHINE_SUPPORTS_SAVE )

GAME( 1988, asuka,     0,        asuka,    asuka,     msm_state,      empty_init,    ROT270, "Taito Corporation",         "Asuka & Asuka (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, asukaj,    asuka,    asuka,    asuka,     msm_state,      empty_init,    ROT270, "Taito Corporation",         "Asuka & Asuka (Japan, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, asukaja,   asuka,    asuka,    asuka,     msm_state,      empty_init,    ROT270, "Taito Corporation",         "Asuka & Asuka (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, mofflott,  0,        mofflott, mofflott,  msm_state,      empty_init,    ROT270, "Taito Corporation",         "Maze of Flott (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, cadash,    0,        cadash,   cadash,    cadash_state,   empty_init,    ROT0,   "Taito Corporation Japan",   "Cadash (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashj,   cadash,   cadash,   cadashj,   cadash_state,   empty_init,    ROT0,   "Taito Corporation",         "Cadash (Japan, rev 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashj1,  cadash,   cadash,   cadashj,   cadash_state,   empty_init,    ROT0,   "Taito Corporation",         "Cadash (Japan, rev 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashjo,  cadash,   cadash,   cadashj,   cadash_state,   empty_init,    ROT0,   "Taito Corporation",         "Cadash (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashu,   cadash,   cadash,   cadashu,   cadash_state,   empty_init,    ROT0,   "Taito America Corporation", "Cadash (US, rev 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashu1,  cadash,   cadash,   cadashu,   cadash_state,   empty_init,    ROT0,   "Taito America Corporation", "Cadash (US, rev 1?)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashi,   cadash,   cadash,   cadash,    cadash_state,   empty_init,    ROT0,   "Taito Corporation Japan",   "Cadash (Italy)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashf,   cadash,   cadash,   cadash,    cadash_state,   empty_init,    ROT0,   "Taito Corporation Japan",   "Cadash (France)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashg,   cadash,   cadash,   cadash,    cadash_state,   empty_init,    ROT0,   "Taito Corporation Japan",   "Cadash (Germany, rev 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashgo,  cadash,   cadash,   cadash,    cadash_state,   empty_init,    ROT0,   "Taito Corporation Japan",   "Cadash (Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1989, cadashp,   cadash,   cadash,   cadashj,   cadash_state,   empty_init,    ROT0,   "Taito Corporation Japan",   "Cadash (World, prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN)
GAME( 1989, cadashs,   cadash,   cadash,   cadash,    cadash_state,   empty_init,    ROT0,   "Taito Corporation Japan",   "Cadash (Spain, rev 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )

GAME( 1992, galmedes,  0,        asuka,    galmedes,  msm_state,      empty_init,    ROT270, "Visco",                     "Galmedes (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, earthjkr,  0,        asuka,    earthjkr,  msm_state,      init_earthjkr, ROT270, "Visco",                     "U.N. Defense Force: Earth Joker (US / Japan, set 1)", MACHINE_SUPPORTS_SAVE ) // sets 1 + 2 + 3 have ROMSTAR (US?) license and no region disclaimer if you change the dipswitch
GAME( 1993, earthjkra, earthjkr, asuka,    earthjkr,  msm_state,      empty_init,    ROT270, "Visco",                     "U.N. Defense Force: Earth Joker (US / Japan, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, earthjkrb, earthjkr, asuka,    earthjkr,  msm_state,      empty_init,    ROT270, "Visco",                     "U.N. Defense Force: Earth Joker (US / Japan, set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, earthjkrp, earthjkr, asuka,    earthjkrp, msm_state,      empty_init,    ROT270, "Visco",                     "U.N. Defense Force: Earth Joker (Japan, prototype?)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, eto,       0,        eto,      eto,       base_state,     empty_init,    ROT0,   "Visco",                     "Kokontouzai Eto Monogatari (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, etoa,      eto,      eto,      eto,       base_state,     empty_init,    ROT0,   "Visco",                     "Kokontouzai Eto Monogatari (Japan, prototype?)", MACHINE_SUPPORTS_SAVE )
