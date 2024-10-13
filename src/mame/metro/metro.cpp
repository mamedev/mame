// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                              -= Metro Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  MC68000 Or H8/3007

Video Chips  :  Imagetek I4100 052  Or
                Imagetek I4220 071  Or
                Imagetek I4300 095

Sound CPU    :  NEC78C10 [Optional]

Sound Chips  :  OKIM6295 + YM2413  or
                 YMF278B + YRW801-M

Other        :  Memory Blitter

------------------------------------------------------------------------------------------------
Year + Game                       PCB           Video  Sub CPU  Sound         Issues / Notes
------------------------------------------------------------------------------------------------
92  Last Fortress - Toride        VG420         I4100  uPD7810  YM2413+M6295
92  Last Fortress - Toride (Ger)  VG460-(A)     I4100  uPD7810  YM2413+M6295
92  Pang Pom's                    VG420         I4100  uPD7810  YM2413+M6295
92  Sky Alert                     VG420         I4100  uPD7810  YM2413+M6295
92  The Karate Tournament         VG460-(A)     I4100  uPD7810  YM2413+M6295
92  The Karate Tournament (Japan) VG460-(A)     I4100  uPD7810  YM2413+M6295
93? Lady Killer / Moeyo Gonta!!   VG460-B       I4100  uPD7810  YM2413+M6295
93  Poitto!                       MTR5260-A     I4100  uPD7810  YM2413+M6295

94  Dharma Doujou                 MTR5260-A     I4220  uPD7810  YM2413+M6295
94  Dharma Doujou (Korea)         MTR527        I4220  uPD7810  YM2413+M6295
94  Toride II Adauchi Gaiden      MTR5260-A     I4220  uPD7810  YM2413+M6295
94  Toride II Adauchi Gaiden(Kr)  MTR5260-A     I4220  uPD7810  YM2413+M6295
94  Gun Master                    MTR5260-A     I4220  uPD7810  YM2151+M6295
95  Daitoride                     MTR5260-A     I4220  uPD7810  YM2151+M6295
95  Pururun                       MTR5260-A     I4220  uPD7810  YM2151+M6295
95  Puzzli                        MTR5260-A     I4220  uPD7810  YM2151+M6295
96  Sankokushi                    MTR5260-A     I4220  uPD7810  YM2413+M6295

95  Mouse Shooter GoGo            -             I4220   -       YMF278B       GFX ROM data lines swapped
96  Bal Cube                      -             I4220   -       YMF278B       GFX ROM data lines swapped
96  Daitoride (YMF278B)           -             I4220   -       YMF278B       GFX ROM data lines swapped
96  Bang Bang Ball                -             I4220   -       YMF278B       GFX ROM data lines swapped
99  Battle Bubble v2.00           LM2D-Y        I4220   -       YMF278B       GFX ROM data lines swapped

94  Blazing Tornado               HUM-002-A-(B) I4220  Z80      YM2610        Konami 053936 PSAC2
96  Grand Striker 2               HUM-003(A)    I4220  Z80      YM2610        Konami 053936 PSAC2

95  Varia Metal                   ES-9309B-B    I4220   -       ES8712+M6295+M6585

95  Mahjong Doukyuusei            VG330-B       I4300   -       YM2413+M6295
95  Mahjong Doukyuusei Special    VG340-A       I4300   -       YM2413+M6295
96  Mouja                         VG410-B       I4300   -       YM2413+M6295
97  Mahjong Gakuensai             VG340-A       I4300   -       YM2413+M6295
98  Mahjong Gakuensai 2           VG340-A       I4300   -       YM2413+M6295

00  Puzzlet                       VG2200-(B)    I4300  Z86E02   YM2413+M6295  H8/3007 CPU
01  Metabee Shot                  VG2200-(B)    I4300  Z86E02   not populated H8/3007 CPU
------------------------------------------------------------------------------------------------

Mouse Shooter GoGo, Bal Cube, Bang Bang Ball & Daitoride (YMF278B) PCBs have
no PCB number but all look identical to each other.

To Do:

-   For video related issues @see devices/video/imagetek_i4100.cpp
-   Most games in service mode, seem to require that you press start1&2 *exactly at once*
    in order to advance to the next screen (e.g. holding 1 then pressing 2 doesn't work).
-   Interrupt timing needs figuring out properly, having it incorrect
    causes scrolling glitches in some games.  Test cases Mouse Go Go
    title screen, GunMaster title screen.  Changing it can cause
    excessive slowdown in said games however.
-   karatour, ladykill, 3kokushi: understand what the irq source 5 is really tied to.
    All these games also have a vblank delay check outside irq routine,
    cfr. PC=1322 in karatour;
-   vmetal: ES8712 actually controls a M6585 and an unknown logic selector chip.
-   split these games into different files, check PCB markings.
-   Coin lockout;

Notes:

-   To enter service mode in ladykill, 3kokushi: toggle the dip switch and reset
    keeping start 2 pressed.
-   Sprite zoom in Mouja at the end of a match looks wrong, but it's been verified
    to be the same on the original board
-   vmetal: has Sega and Taito logos in the roms ?!

driver modified by Hau
***************************************************************************/

#include "emu.h"
#include "metro.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/h8/h83006.h"
#include "cpu/z8/z8.h"
#include "machine/watchdog.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"
#include "speaker.h"

#include <algorithm>

/***************************************************************************


                                Interrupts


***************************************************************************/

// This is for games that supply an *IRQ Vector* on the data bus together with an IRQ level for each possible IRQ source
void metro_state::ipl_w(u8 data)
{
	for (int i = 1; i < 8; i++)
		m_maincpu->set_input_line(i, (i == data) ? ASSERT_LINE : CLEAR_LINE);
}


void metro_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).r(m_vdp3, FUNC(imagetek_i4300_device::irq_vector_r)).umask16(0x00ff);
}


TIMER_CALLBACK_MEMBER(mouja_state::mouja_irq)
{
	if (m_vdp) m_vdp->set_irq(0);
	if (m_vdp2) m_vdp2->set_irq(0);
	if (m_vdp3) m_vdp3->set_irq(0);
}

void metro_state::vblank_irq(int state)
{
	if (state)
	{
		if (m_vdp) m_vdp->screen_eof(state);
		if (m_vdp2) m_vdp2->screen_eof(state);
		if (m_vdp3) m_vdp3->screen_eof(state);
	}
}

INTERRUPT_GEN_MEMBER(metro_state::periodic_interrupt)
{
	if (m_vdp) m_vdp->set_irq(4);
	if (m_vdp2) m_vdp2->set_irq(4);
	if (m_vdp3) m_vdp3->set_irq(4);
}

TIMER_DEVICE_CALLBACK_MEMBER(metro_state::bangball_scanline)
{
	int const scanline = param;

	// vblank irq
	if(scanline == 224)
	{
		m_vdp2->set_irq(4); // ???
		m_vdp2->screen_eof(ASSERT_LINE);
	}
	else if(scanline < 224 && (m_vdp2->irq_enable() & 2) == 0)
	{
		// pretty likely hblank irq (pressing a button when clearing a stage)
		m_vdp2->set_irq(1);
	}
}

// lev 2-7 (lev 1 seems sound related)
void metro_state::karatour_vblank_irq(int state)
{
//  logerror("%d %d %lld\n", state, m_screen->vpos(), m_screen->frame_number());

	if (state)
	{
		if (m_vdp) m_vdp->screen_eof(state);
		if (m_vdp2) m_vdp2->screen_eof(state);
		if (m_vdp3) m_vdp3->screen_eof(state);

		if (m_ext_irq_enable)
		{
			if (m_vdp) m_vdp->set_irq(5);
			if (m_vdp2) m_vdp2->set_irq(5);
			if (m_vdp3) m_vdp3->set_irq(5);
		}
	}
	else
	{
		if (m_vdp) m_vdp->clear_irq(5);
		if (m_vdp2) m_vdp2->clear_irq(5);
		if (m_vdp3) m_vdp3->clear_irq(5);
	}
}

void metro_state::ext_irq5_enable_w(int state)
{
	m_ext_irq_enable = state;
}

void mouja_state::irq_timer_ctrl_w(u16 data)
{
	double const freq = 58.0 + (0xff - (data & 0xff)) / 2.2; // 0xff=58Hz, 0x80=116Hz?

	m_mouja_irq_timer->adjust(attotime::zero, 0, attotime::from_hz(freq));
}

void metro_state::puzzlet_vblank_irq(int state)
{
	if (state)
	{
		m_vdp2->screen_eof(state);
		if (m_ext_irq_enable)
			m_vdp2->set_irq(5);
	}
	else
		m_vdp2->clear_irq(5);
}

/***************************************************************************


                            Sound Communication


***************************************************************************/

int metro_upd7810_state::rxd_r()
{
	u8 const data = m_sound_data;

	// TODO: shift on SCK falling edges
	if (!machine().side_effects_disabled())
		m_sound_data >>= 1;

	return data & 1;
}

void metro_upd7810_state::sound_data_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(metro_upd7810_state::sound_data_sync), this), data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); // seen rxd_r
}

TIMER_CALLBACK_MEMBER(metro_upd7810_state::sound_data_sync)
{
	m_sound_data = param;
	m_busy_sndcpu = true;
}


u8 metro_upd7810_state::soundstatus_r()
{
	return (m_busy_sndcpu ? 0x00 : 0x01);
}

int metro_upd7810_state::custom_soundstatus_r()
{
	return (m_busy_sndcpu ? 1 : 0);
}

void metro_upd7810_state::soundstatus_w(u8 data)
{
	m_soundstatus = data & 0x01;
}

template<int Mask>
void metro_upd7810_state::upd7810_rombank_w(u8 data)
{
	m_audiobank->set_entry((data >> 4) & Mask);
}


u8 metro_upd7810_state::upd7810_porta_r()
{
	return m_porta;
}

void metro_upd7810_state::upd7810_porta_w(u8 data)
{
	m_porta = data;
}

void metro_upd7810_state::upd7810_portb_w(u8 data)
{
	/* port B layout:
	   7 !clock latch for message to main CPU
	   6
	   5 !clock YM2413 I/O
	   4 !clock MSM6295 I/O
	   3 !enable read from 6295
	   2 !enable write to YM2413/6295
	   1 select YM2413 register or data port
	   0
	*/


	if (BIT(m_portb, 7) && !BIT(data, 7))   // clock 1->0
	{
		m_busy_sndcpu = false;
		m_portb = data;
		return;
	}

	if (BIT(m_portb, 5) && !BIT(data, 5))   // clock 1->0
	{
		if (!BIT(data, 2))
		{
			downcast<ym2413_device *>(m_ymsnd.target())->write(BIT(data, 1), m_porta);
		}
		m_portb = data;
		return;
	}

	if (BIT(m_portb, 2) && !BIT(data, 2))   // clock 1->0
	{
		// write
		if (!BIT(data, 4))
			m_oki->write(m_porta);
	}

	if (BIT(m_portb, 3) && !BIT(data, 3))   // clock 1->0
	{
		// read
		if (!BIT(data, 4))
			m_porta = m_oki->read();
	}

	m_portb = data;
}


void metro_upd7810_state::daitorid_portb_w(u8 data)
{
	/* port B layout:
	   7 !clock latch for message to main CPU
	   6 !clock YM2151 I/O
	   5
	   4 !clock MSM6295 I/O
	   3 !enable read from YM2151/6295
	   2 !enable write to YM2151/6295
	   1 select YM2151 register or data port
	   0
	*/

	if (BIT(m_portb, 7) && !BIT(data, 7))   // clock 1->0
	{
		m_busy_sndcpu = false;
		m_portb = data;
		return;
	}

	if (BIT(m_portb, 6) && !BIT(data, 6))   // clock 1->0
	{
		if (!BIT(data, 2))
		{
			// write
			downcast<ym2151_device *>(m_ymsnd.target())->write(BIT(data, 1), m_porta);
		}

		if (!BIT(data, 3))
		{
			// read
			m_porta = downcast<ym2151_device *>(m_ymsnd.target())->read(BIT(data, 1));
		}

		m_portb = data;
		return;
	}

	if (BIT(m_portb, 2) && !BIT(data, 2))   // clock 1->0
	{
		// write
		if (!BIT(data, 4))
			m_oki->write(m_porta);
	}

	if (BIT(m_portb, 3) && !BIT(data, 3))   // clock 1->0
	{
		// read
		if (!BIT(data, 4))
			m_porta = m_oki->read();
	}

	m_portb = data;
}


/***************************************************************************


                                Coin Lockout


***************************************************************************/

void metro_state::coin_lockout_1word_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	if (data & ~3)  logerror("CPU #0 PC %06X : unknown bits of coin lockout written: %04X\n", m_maincpu->pc(), data);
}

// value written doesn't matter, also each counted coin gets reported after one full second.
// TODO: maybe the counter also controls lockout?
void metro_state::coin_lockout_4words_w(offs_t offset, u16 data)
{
	machine().bookkeeping().coin_counter_w((offset >> 1) & 1, offset & 1);
//  machine().bookkeeping().coin_lockout_w((offset >> 1) & 1, offset & 1);

	if (data & ~1)  logerror("CPU #0 PC %06X : unknown bits of coin lockout written: %04X\n", m_maincpu->pc(), data);
}

/***************************************************************************


                                Memory Maps


***************************************************************************/

/*
 Lines starting with an empty comment in the following MemoryReadAddress
 arrays are there for debug (e.g. the game does not read from those ranges
 AFAIK)
*/


void metro_upd7810_state::upd7810_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();         // External ROM
	map(0x4000, 0x7fff).bankr(m_audiobank);    // External ROM (Banked)
	map(0x8000, 0x87ff).ram();         // External RAM
}

/*****************/


void metro_state::ymf278_map(address_map &map)
{
	map(0x000000, 0x27ffff).rom();
}


/***************************************************************************
                                    Bal Cube
***************************************************************************/

u16 metro_state::balcube_dsw_r(offs_t offset)
{
	u16 const dsw0 = m_io_dsw[0]->read();
	u16 const in2 = m_io_in[2]->read();

	u16 result = 0xffff;
	for (unsigned b = 0; 16 > b; ++b)
	{
		if (!BIT(offset, b))
			result &= (BIT(dsw0, b) << 6) | (BIT(in2, b) << 7);
	}
	return result;
}


void metro_state::balcube_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                 // ROM
	map(0x300001, 0x300001).r("ymf", FUNC(ymf278b_device::read));                  // Sound
	map(0x300000, 0x30000b).w("ymf", FUNC(ymf278b_device::write)).umask16(0x00ff); // Sound
	map(0x400000, 0x41ffff).r(FUNC(metro_state::balcube_dsw_r));                   // DSW x 3
	map(0x500000, 0x500001).portr("IN0");                                          // Inputs
	map(0x500002, 0x500003).portr("IN1");                                          //
	map(0x500006, 0x500007).nopr();                                                //
	map(0x500002, 0x500009).w(FUNC(metro_state::coin_lockout_4words_w));           // Coin Lockout
	map(0x600000, 0x67ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                                // RAM (mirrored)
}


/***************************************************************************
                            Daitoride (YMF278B version)
***************************************************************************/


void metro_state::daitoa_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                 // ROM
	map(0x100000, 0x17ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x200000, 0x200001).portr("IN0");                                          // Inputs
	map(0x200002, 0x200003).portr("IN1");                                          //
	map(0x200006, 0x200007).nopr();                                                //
	map(0x200002, 0x200009).w(FUNC(metro_state::coin_lockout_4words_w));           // Coin Lockout
	map(0x300000, 0x31ffff).r(FUNC(metro_state::balcube_dsw_r));                   // DSW x 3
	map(0x400001, 0x400001).r("ymf", FUNC(ymf278b_device::read));                  // Sound
	map(0x400000, 0x40000b).w("ymf", FUNC(ymf278b_device::write)).umask16(0x00ff); // Sound
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                                // RAM (mirrored)
}


/***************************************************************************
                                Bang Bang Ball
***************************************************************************/

void metro_state::bangball_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                 // ROM
	map(0xb00001, 0xb00001).r("ymf", FUNC(ymf278b_device::read));                  // Sound
	map(0xb00000, 0xb0000b).w("ymf", FUNC(ymf278b_device::write)).umask16(0x00ff); // Sound
	map(0xc00000, 0xc1ffff).r(FUNC(metro_state::balcube_dsw_r));                   // DSW x 3
	map(0xd00000, 0xd00001).portr("IN0");                                          // Inputs
	map(0xd00002, 0xd00003).portr("IN1");                                          //
	map(0xd00006, 0xd00007).nopr();                                                //
	map(0xd00002, 0xd00009).w(FUNC(metro_state::coin_lockout_4words_w));           // Coin Lockout
	map(0xe00000, 0xe7ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                                // RAM (mirrored)
}


/***************************************************************************
                                Battle Bubble
***************************************************************************/

void metro_state::batlbubl_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                 // ROM
	map(0x100000, 0x17ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x200000, 0x200001).portr("IN1");                                          // Inputs
	map(0x200002, 0x200003).portr("DSW0");                                         //
	map(0x200004, 0x200005).portr("IN0");                                          //
	map(0x200006, 0x200007).portr("IN2");                                          //
	map(0x200002, 0x200009).w(FUNC(metro_state::coin_lockout_4words_w));           // Coin Lockout
	map(0x300000, 0x31ffff).r(FUNC(metro_state::balcube_dsw_r));                   // read but ignored?
	map(0x400001, 0x400001).r("ymf", FUNC(ymf278b_device::read));                  // Sound
	map(0x400000, 0x40000b).w("ymf", FUNC(ymf278b_device::write)).umask16(0x00ff); //
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                                // RAM (mirrored)
}


/***************************************************************************
                             Mouse Shooter GoGo
***************************************************************************/

void metro_state::msgogo_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x100000, 0x17ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x200000, 0x200001).portr("COINS");                              // Inputs
	map(0x200002, 0x200003).portr("JOYS");                               //
	map(0x200006, 0x200007).nopr();                                         //
	map(0x200002, 0x200009).w(FUNC(metro_state::coin_lockout_4words_w));              // Coin Lockout
	map(0x300000, 0x31ffff).r(FUNC(metro_state::balcube_dsw_r));                             // 3 x DSW
	map(0x400001, 0x400001).r("ymf", FUNC(ymf278b_device::read));   // Sound
	map(0x400000, 0x40000b).w("ymf", FUNC(ymf278b_device::write)).umask16(0x00ff); //
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}

/***************************************************************************
                       Daitoride and Puzzli (revision B)
***************************************************************************/

void metro_upd7810_state::daitorid_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400000, 0x47ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x4788a9, 0x4788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0x800000, 0x80ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
	map(0xc00000, 0xc00001).portr("IN0");
	map(0xc00001, 0xc00001).w(FUNC(metro_upd7810_state::soundstatus_w));  // To Sound CPU
	map(0xc00002, 0xc00003).portr("IN1");                                //
	map(0xc00004, 0xc00005).portr("DSW0");                               //
	map(0xc00006, 0xc00007).portr("IN2");                                //
	map(0xc00002, 0xc00009).w(FUNC(metro_upd7810_state::coin_lockout_4words_w));              // Coin Lockout
}


/***************************************************************************
                                Dharma Doujou
***************************************************************************/

void metro_upd7810_state::dharma_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400000, 0x40ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
	map(0x800000, 0x87ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x8788a9, 0x8788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0xc00000, 0xc00001).portr("IN0");
	map(0xc00001, 0xc00001).w(FUNC(metro_upd7810_state::soundstatus_w));  // To Sound CPU
	map(0xc00002, 0xc00003).portr("IN1");                                //
	map(0xc00004, 0xc00005).portr("DSW0");                               //
	map(0xc00006, 0xc00007).portr("IN2");                                //
	map(0xc00002, 0xc00009).w(FUNC(metro_upd7810_state::coin_lockout_4words_w));              // Coin Lockout
}


/***************************************************************************
                                Karate Tournament
***************************************************************************/

void metro_upd7810_state::karatour_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x400001, 0x400001).rw(FUNC(metro_upd7810_state::soundstatus_r), FUNC(metro_upd7810_state::soundstatus_w)); // From Sound CPU
	map(0x400002, 0x400003).portr("IN0");                                // Inputs
	map(0x400003, 0x400003).w(FUNC(metro_upd7810_state::coin_lockout_1word_w));               // Coin Lockout
	map(0x400004, 0x400005).portr("IN1");                                //
	map(0x400006, 0x400007).portr("DSW0");                               //
	map(0x40000a, 0x40000b).portr("DSW1");                               //
	map(0x40000c, 0x40000d).portr("IN2");                                //
	map(0x800000, 0x87ffff).m(m_vdp, FUNC(imagetek_i4100_device::map));
	map(0x8788a9, 0x8788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                                Sankokushi
***************************************************************************/

// same limited tilemap access as karatour

void metro_upd7810_state::kokushi_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x700000, 0x70ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
	map(0x800000, 0x87ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x8788a9, 0x8788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0xc00000, 0xc00001).portr("IN0");
	map(0xc00001, 0xc00001).w(FUNC(metro_upd7810_state::soundstatus_w));  // To Sound CPU
	map(0xc00002, 0xc00003).portr("IN1");                                // Inputs
	map(0xc00004, 0xc00005).portr("DSW0");                               //
	map(0xc00002, 0xc00009).w(FUNC(metro_upd7810_state::coin_lockout_4words_w));              // Coin Lockout
}


/***************************************************************************
                                Last Fortress
***************************************************************************/

void metro_upd7810_state::lastfort_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400000, 0x40ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
	map(0x800000, 0x87ffff).m(m_vdp, FUNC(imagetek_i4100_device::map));
	map(0x8788a9, 0x8788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0xc00001, 0xc00001).rw(FUNC(metro_upd7810_state::soundstatus_r), FUNC(metro_upd7810_state::soundstatus_w)); // From / To Sound CPU
	map(0xc00003, 0xc00003).w(FUNC(metro_upd7810_state::coin_lockout_1word_w));               // Coin Lockout
	map(0xc00004, 0xc00005).portr("IN0");                                // Inputs
	map(0xc00006, 0xc00007).portr("IN1");                                //
	map(0xc00008, 0xc00009).portr("IN2");                                //
	map(0xc0000a, 0xc0000b).portr("DSW0");                               //
	map(0xc0000c, 0xc0000d).portr("DSW1");                               //
	map(0xc0000e, 0xc0000f).portr("IN3");                                //
}

// the German version is halfway between lastfort and ladykill (karatour) memory maps

// todo: clean up input reads etc.
void metro_upd7810_state::lastforg_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400001, 0x400001).rw(FUNC(metro_upd7810_state::soundstatus_r), FUNC(metro_upd7810_state::soundstatus_w)); // From / To Sound CPU
	map(0x400002, 0x400003).portr("IN0");                                // Inputs
	map(0x400003, 0x400003).w(FUNC(metro_upd7810_state::coin_lockout_1word_w));               // Coin Lockout
	map(0x400004, 0x400005).portr("IN1");                                //
	map(0x400006, 0x400007).portr("DSW0");                               //
	map(0x40000a, 0x40000b).portr("DSW1");                               //
	map(0x40000c, 0x40000d).portr("IN2");                                //
	map(0x880000, 0x8fffff).m(m_vdp, FUNC(imagetek_i4100_device::map));
	map(0x8f88a9, 0x8f88a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0xc00000, 0xc0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                                Mahjong Gakuensai
***************************************************************************/

void gakusai_state::oki_bank_set()
{
	u8 const bank = (m_oki_bank_lo & 7) + (m_oki_bank_hi & 1) * 8;
	m_oki->set_rom_bank(bank);
}

void gakusai_state::oki_bank_hi_w(u8 data)
{
	m_oki_bank_hi = data;
	oki_bank_set();
}

void gakusai_state::oki_bank_lo_w(u8 data)
{
	m_oki_bank_lo = data;
	oki_bank_set();
}


u16 gakusai_state::input_r()
{
	u16 const input_sel = *m_input_sel;
	u16 result = 0xffff;
	// Bit 0 ??
	if (!BIT(input_sel, 1)) result &= m_io_key[0]->read();
	if (!BIT(input_sel, 2)) result &= m_io_key[1]->read();
	if (!BIT(input_sel, 3)) result &= m_io_key[2]->read();
	if (!BIT(input_sel, 4)) result &= m_io_key[3]->read();
	if (!BIT(input_sel, 5)) result &= m_io_key[4]->read();
	return result;
}

u8 gakusai_state::gakusai_eeprom_r()
{
	return m_eeprom->do_read() & 1;
}

void gakusai_state::gakusai_eeprom_w(u8 data)
{
	// latch the bit
	m_eeprom->di_write(BIT(data, 0));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE );

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE );
}

void gakusai_state::gakusai_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x200000, 0x27ffff).m(m_vdp3, FUNC(imagetek_i4300_device::v3_map));
	map(0x278836, 0x278837).nopr().w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x278880, 0x278881).r(FUNC(gakusai_state::input_r));                           // Inputs
	map(0x278882, 0x278883).portr("IN0");                                //
	map(0x278888, 0x278889).writeonly().share(m_input_sel);                 // Inputs
	map(0x400000, 0x400001).nopw();                                        // ? 5
	map(0x500001, 0x500001).w(FUNC(gakusai_state::oki_bank_lo_w));           // Sound
	map(0x600000, 0x600003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x700001, 0x700001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // Sound
	map(0xc00001, 0xc00001).rw(FUNC(gakusai_state::gakusai_eeprom_r), FUNC(gakusai_state::gakusai_eeprom_w));      // EEPROM
	map(0xd00001, 0xd00001).w(FUNC(gakusai_state::oki_bank_hi_w));
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                                Mahjong Gakuensai 2
***************************************************************************/

void gakusai_state::gakusai2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x600000, 0x67ffff).m(m_vdp3, FUNC(imagetek_i4300_device::v3_map));
	map(0x678836, 0x678837).nopr().w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x678880, 0x678881).r(FUNC(gakusai_state::input_r));                           // Inputs
	map(0x678882, 0x678883).portr("IN0");                                //
	map(0x678888, 0x678889).writeonly().share(m_input_sel);                 // Inputs
	map(0x800000, 0x800001).nopw();                                        // ? 5
	map(0x900001, 0x900001).w(FUNC(gakusai_state::oki_bank_lo_w));           // Sound bank
	map(0xa00001, 0xa00001).w(FUNC(gakusai_state::oki_bank_hi_w));           //
	map(0xb00001, 0xb00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // Sound
	map(0xc00000, 0xc00003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0xe00001, 0xe00001).rw(FUNC(gakusai_state::gakusai_eeprom_r), FUNC(gakusai_state::gakusai_eeprom_w));      // EEPROM
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                        Mahjong Doukyuusei Special
***************************************************************************/

u8 gakusai_state::dokyusp_eeprom_r()
{
	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(CLEAR_LINE);
	m_eeprom->clk_write(ASSERT_LINE);

	return m_eeprom->do_read() & 1;
}

void gakusai_state::dokyusp_eeprom_bit_w(u8 data)
{
	// latch the bit
	m_eeprom->di_write(BIT(data, 0));

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(CLEAR_LINE);
	m_eeprom->clk_write(ASSERT_LINE);
}

void gakusai_state::dokyusp_eeprom_reset_w(u8 data)
{
	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
}

void gakusai_state::dokyusp_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x200000, 0x27ffff).m(m_vdp3, FUNC(imagetek_i4300_device::v3_map));
	map(0x278836, 0x278837).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x278880, 0x278881).r(FUNC(gakusai_state::input_r));                           // Inputs
	map(0x278882, 0x278883).portr("IN0");                                //
	map(0x278888, 0x278889).writeonly().share(m_input_sel);                 //
	map(0x400000, 0x400001).nopw();                                        // ? 5
	map(0x500001, 0x500001).w(FUNC(gakusai_state::oki_bank_lo_w));           // Sound
	map(0x600000, 0x600003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x700001, 0x700001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // Sound
	map(0xc00001, 0xc00001).w(FUNC(gakusai_state::dokyusp_eeprom_reset_w));                      // EEPROM
	map(0xd00001, 0xd00001).rw(FUNC(gakusai_state::dokyusp_eeprom_r), FUNC(gakusai_state::dokyusp_eeprom_bit_w));  // EEPROM
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                            Mahjong Doukyuusei
***************************************************************************/

void gakusai_state::dokyusei_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400000, 0x47ffff).m(m_vdp3, FUNC(imagetek_i4300_device::v3_map));
	map(0x478836, 0x478837).nopw();                                        // ? watchdog ?
	map(0x478880, 0x478881).r(FUNC(gakusai_state::input_r));                           // Inputs
	map(0x478882, 0x478883).portr("IN0");                                //
	map(0x478884, 0x478885).portr("DSW0");                               // 2 x DSW
	map(0x478886, 0x478887).portr("DSW1");                               //
	map(0x478888, 0x478889).writeonly().share(m_input_sel);                 // Inputs
	map(0x800001, 0x800001).w(FUNC(gakusai_state::oki_bank_hi_w));           // Samples Bank?
	map(0x900000, 0x900001).nopw();                                        // ? 4
	map(0xa00001, 0xa00001).w(FUNC(gakusai_state::oki_bank_lo_w));           // Samples Bank
	map(0xc00000, 0xc00003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);     //
	map(0xd00001, 0xd00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // Sound
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                                Pang Pom's
***************************************************************************/

void metro_upd7810_state::pangpoms_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400000, 0x47ffff).m(m_vdp, FUNC(imagetek_i4100_device::map));
	map(0x4788a9, 0x4788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0x800001, 0x800001).rw(FUNC(metro_upd7810_state::soundstatus_r), FUNC(metro_upd7810_state::soundstatus_w));  // From / To Sound CPU
	map(0x800002, 0x800003).nopr();
	map(0x800003, 0x800003).w(FUNC(metro_upd7810_state::coin_lockout_1word_w));    // Coin Lockout
	map(0x800004, 0x800005).portr("IN0");                                // Inputs
	map(0x800006, 0x800007).portr("IN1");                                //
	map(0x800008, 0x800009).portr("IN2");                                //
	map(0x80000a, 0x80000b).portr("DSW0");                               //
	map(0x80000c, 0x80000d).portr("DSW1");                               //
	map(0x80000e, 0x80000f).portr("IN3");                                //
	map(0xc00000, 0xc0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                                Poitto!
***************************************************************************/

void metro_upd7810_state::poitto_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400000, 0x40ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
	map(0x800000, 0x800001).portr("IN0");
	map(0x800001, 0x800001).w(FUNC(metro_upd7810_state::soundstatus_w));  // To Sound CPU
	map(0x800002, 0x800003).portr("IN1");                                //
	map(0x800004, 0x800005).portr("DSW0");                               //
	map(0x800006, 0x800007).portr("IN2");                                //
	map(0x800002, 0x800009).w(FUNC(metro_upd7810_state::coin_lockout_4words_w));              // Coin Lockout
	map(0xc00000, 0xc7ffff).m(m_vdp, FUNC(imagetek_i4100_device::map));
	map(0xc788a9, 0xc788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
}


/***************************************************************************
                                Sky Alert
***************************************************************************/

void metro_upd7810_state::skyalert_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                             // ROM
	map(0x400001, 0x400001).rw(FUNC(metro_upd7810_state::soundstatus_r), FUNC(metro_upd7810_state::soundstatus_w));  // From / To Sound CPU
	map(0x400002, 0x400003).nopr();
	map(0x400003, 0x400003).w(FUNC(metro_upd7810_state::coin_lockout_1word_w));    // Coin Lockout
	map(0x400004, 0x400005).portr("IN0");                                // Inputs
	map(0x400006, 0x400007).portr("IN1");                                //
	map(0x400008, 0x400009).portr("IN2");                                //
	map(0x40000a, 0x40000b).portr("DSW0");                               //
	map(0x40000c, 0x40000d).portr("DSW1");                               //
	map(0x40000e, 0x40000f).portr("IN3");                                //
	map(0x800000, 0x87ffff).m(m_vdp, FUNC(imagetek_i4100_device::map));
	map(0x8788a9, 0x8788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
	map(0xc00000, 0xc0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                        Pururun and Puzzli (revision A)
***************************************************************************/

void metro_upd7810_state::pururun_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x400000, 0x400001).portr("IN0");
	map(0x400001, 0x400001).w(FUNC(metro_upd7810_state::soundstatus_w));  // To Sound CPU
	map(0x400002, 0x400003).portr("IN1");                                //
	map(0x400004, 0x400005).portr("DSW0");                               //
	map(0x400006, 0x400007).portr("IN2");                                //
	map(0x400002, 0x400009).w(FUNC(metro_upd7810_state::coin_lockout_4words_w));              // Coin Lockout
	map(0x800000, 0x80ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
	map(0xc00000, 0xc7ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0xc788a9, 0xc788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
}


/***************************************************************************
                            Toride II Adauchi Gaiden
***************************************************************************/

void metro_upd7810_state::toride2g_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x400000, 0x40ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
	map(0x800000, 0x800001).portr("IN0");  // Watchdog?
	map(0x800001, 0x800001).w(FUNC(metro_upd7810_state::soundstatus_w));  // To Sound CPU
	map(0x800002, 0x800003).portr("IN1");                                //
	map(0x800004, 0x800005).portr("DSW0");                               //
	map(0x800006, 0x800007).portr("IN2");                                //
	map(0x800002, 0x800009).w(FUNC(metro_upd7810_state::coin_lockout_4words_w));              // Coin Lockout
	map(0xc00000, 0xc7ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0xc788a9, 0xc788a9).w(FUNC(metro_upd7810_state::sound_data_w));                       // To Sound CPU
}


/***************************************************************************
                            Blazing Tornado
***************************************************************************/

void blzntrnd_state::audiobank_w(u8 data)
{
	m_audiobank->set_entry(data & 0x07);
}

void blzntrnd_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xe000, 0xffff).ram();
}

void blzntrnd_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(blzntrnd_state::audiobank_w));
	map(0x40, 0x40).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw();
	map(0x80, 0x83).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}

void blzntrnd_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                             // ROM
	map(0x200000, 0x27ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));

	map(0x400000, 0x43ffff).ram().w(FUNC(blzntrnd_state::k053936_w)).share("k053936_ram");  // 053936
	map(0x500000, 0x500fff).w(m_k053936, FUNC(k053936_device::linectrl_w));      // 053936 line control
	map(0x600000, 0x60001f).w(m_k053936, FUNC(k053936_device::ctrl_w));          // 053936 control

	map(0xe00000, 0xe00001).portr("DSW0").nopw();                   // Inputs
	map(0xe00002, 0xe00003).portr("DSW1");                               //
	map(0xe00002, 0xe00003).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0xff00).cswidth(16); // To Sound CPU
	map(0xe00004, 0xe00005).portr("IN0");                                //
	map(0xe00006, 0xe00007).portr("IN1");                                //
	map(0xe00008, 0xe00009).portr("IN2");                                //
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************
                                    Mouja
***************************************************************************/

void mouja_state::sound_rombank_w(u8 data)
{
	m_okibank->set_entry((data >> 3) & 0x07);
}

void mouja_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                             // ROM
	map(0x400000, 0x47ffff).m(m_vdp3, FUNC(imagetek_i4300_device::v3_map));
	map(0x478834, 0x478835).w(FUNC(mouja_state::irq_timer_ctrl_w));                   // IRQ set timer count
	map(0x478836, 0x478837).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x478880, 0x478881).portr("IN0");                                // Inputs
	map(0x478882, 0x478883).portr("IN1");                                //
	map(0x478884, 0x478885).portr("DSW0");                               //
	map(0x478886, 0x478887).portr("IN2");                                //
	map(0x800001, 0x800001).w(FUNC(mouja_state::sound_rombank_w));
	map(0xc00000, 0xc00003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}

void mouja_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}


/***************************************************************************
                                Puzzlet
***************************************************************************/

void metro_state::puzzlet_irq_enable_w(u8 data)
{
	m_vdp2->irq_enable_w(data ^ 0xff);
}


// H8/3007 CPU
void metro_state::puzzlet_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x430000, 0x433fff).ram();
	map(0x470000, 0x47dfff).ram();

	map(0x500000, 0x500000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x580000, 0x580003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0xff00);

	// TODO: !!! i4300 !!!
	map(0x700000, 0x77ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x7788a5, 0x7788a5).w(FUNC(metro_state::puzzlet_irq_enable_w));                   // IRQ Enable

	map(0x7f2000, 0x7f3fff).ram();

	map(0x7f8880, 0x7f8881).portr("IN1");
	map(0x7f8884, 0x7f8885).portr("DSW0");
	map(0x7f8886, 0x7f8887).portr("DSW0");

	map(0x7f88a3, 0x7f88a3).r(m_vdp2, FUNC(imagetek_i4220_device::irq_cause_r));
}


void metro_state::puzzlet_portb_w(u8 data)
{
//  popmessage("PORTB %02x", data);
}

/***************************************************************************
                                Varia Metal
***************************************************************************/

void vmetal_state::vmetal_control_w(u8 data)
{
	/* Lower nibble is the coin control bits shown in
	   service mode, but in game mode they're different */
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));  // 2nd coin schute activates coin 0 counter in game mode??
//  machine().bookkeeping().coin_lockout_w(0, BIT(data, 0));  // always on in game mode??
	machine().bookkeeping().coin_lockout_w(1, BIT(data, 1));  // never activated in game mode??

	m_essnd_gate = BIT(data, 6);
	if (!m_essnd_gate)
		m_maincpu->set_input_line(3, CLEAR_LINE);

	if (m_essnd_bank != (data & 0x10) >> 4)
	{
		m_essnd_bank = (data & 0x10) >> 4;
		m_essnd->set_rom_bank(m_essnd_bank);
		logerror("Bankswitching ES8712 ROM %02x\n", m_essnd_bank);
	}

	if (data & 0xa0)
		logerror("%s: Writing unknown bits %04x to $200000\n",machine().describe_context(),data);
}

void vmetal_state::es8712_reset_w(u8 data)
{
	m_essnd->reset();
}

void vmetal_state::es8712_irq(int state)
{
	if (m_essnd_gate)
		m_maincpu->set_input_line(3, state);
}

void vmetal_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                             // ROM
	map(0x100000, 0x17ffff).m(m_vdp2, FUNC(imagetek_i4220_device::v2_map));
	map(0x200000, 0x200001).portr("P1_P2");
	map(0x200001, 0x200001).w(FUNC(vmetal_state::vmetal_control_w));
	map(0x200002, 0x200003).portr("SYSTEM");
	map(0x300000, 0x31ffff).r(FUNC(vmetal_state::balcube_dsw_r));                             // DSW x 3
	map(0x400001, 0x400001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x400003, 0x400003).w(m_oki, FUNC(okim6295_device::write));
	map(0x500000, 0x500000).w(FUNC(vmetal_state::es8712_reset_w));
	map(0x500000, 0x50000d).w(m_essnd, FUNC(es8712_device::write)).umask16(0x00ff);
	map(0xf00000, 0xf0ffff).ram().mirror(0x0f0000);                         // RAM (mirrored)
}


/***************************************************************************


                                Input Ports


***************************************************************************/


#define JOY_LSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_)

#define JOY_MSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_)

#define COINS \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW,  IPT_SERVICE1 ) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW,  IPT_TILT     ) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW,  IPT_START1   ) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW,  IPT_START2   ) \
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN  ) \
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

#define COINS_SOUND \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW,  IPT_SERVICE1 ) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW,  IPT_TILT     ) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW,  IPT_START1   ) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW,  IPT_START2   ) \
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN  ) \
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(metro_upd7810_state, custom_soundstatus_r)   /* From Sound CPU */


#define COINAGE_SERVICE_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3") \
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) ) \
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":4,5,6") \
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) ) \
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION(#DIPBANK":7") \
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) ) \
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, #DIPBANK":8" )


#define COINAGE_FLIP_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":4,5,6") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION(#DIPBANK":7") \
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


/***************************************************************************
                                    Bal Cube
***************************************************************************/

static INPUT_PORTS_START( balcube )
	PORT_START("IN0")   // $500000
	COINS

	PORT_START("IN1")   // $500002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Switch matrix in the 0x400000-0x41ffff range
	COINAGE_SERVICE_LOC(SW1)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "2 Players Game" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0400, "2 Credits" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN2")   // Switch matrix in the 0x400000-0x41ffff range
	PORT_BIT(  0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused
INPUT_PORTS_END


/***************************************************************************
                                Bang Bang Ball
***************************************************************************/

static INPUT_PORTS_START( bangball )
	PORT_START("IN0")   // $d00000
	COINS

	PORT_START("IN1")   // $d00002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Switch matrix in the 0xc00000-0xc1ffff range
	COINAGE_SERVICE_LOC(SW1)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0c00, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )

	PORT_START("IN2")   // Switch matrix in the 0xc00000-0xc1ffff range
	PORT_BIT(  0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // used for debug
INPUT_PORTS_END

/***************************************************************************
                                Battle Bubble
***************************************************************************/

static INPUT_PORTS_START( batlbubl )
	PORT_START("IN1")
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Switch matrix in the 0x300000-0x31ffff range
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x000c, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN0")   // $200004
	COINS

	PORT_START("IN2")   // Switch matrix in the 0x300000-0x31ffff range
	// DSW3 is used for debug (it's not soldered on the PCB)
	PORT_DIPNAME( 0x0001, 0x0001, "0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Debug Mode?" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                             Mouse Shooter GoGo
***************************************************************************/

static INPUT_PORTS_START( msgogo )
	PORT_START("COINS") // $200000
	COINS

	PORT_START("JOYS")  // $200002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Switch matrix in the 0x300000-0x31ffff range
	COINAGE_SERVICE_LOC(SW1)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )  // 0
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )  // 1
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )  // 2
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  // 3
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Allow P2 to Join Game" )     PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )

	PORT_START("IN2")   // Switch matrix in the 0x300000-0x31ffff range
	// DSW3 is used for debug (it's not soldered on the PCB)
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Debug: Offset" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Debug: Menu" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                            Blazing Tornado
***************************************************************************/

static INPUT_PORTS_START( blzntrnd )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "Beginner" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0001, "Expert" )
	PORT_DIPSETTING(      0x0000, "Master" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00c0, 0x0000, "Control Panel" )         PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(      0x0000, "4 Players" )
//  PORT_DIPSETTING(      0x0040, "4 Players" )
	PORT_DIPSETTING(      0x0080, "1P & 2P Tag only" )
	PORT_DIPSETTING(      0x00c0, "1P & 2P vs only" )
	PORT_DIPNAME( 0x0300, 0x0300, "Half Continue" )         PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING(      0x0000, "6C to start, 3C to continue" )
	PORT_DIPSETTING(      0x0100, "4C to start, 2C to continue" )
	PORT_DIPSETTING(      0x0200, "2C to start, 1C to continue" )
	PORT_DIPSETTING(      0x0300, "Disabled" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW4:3" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW4:4" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW4:5" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW4:6" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW4:7" ) // Not read in Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW4:8" ) // Not read in Service Mode

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, "CP Single" )         PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "2:00" )
	PORT_DIPSETTING(      0x0200, "2:30" )
	PORT_DIPSETTING(      0x0100, "3:00" )
	PORT_DIPSETTING(      0x0000, "3:30" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "CP Tag" )            PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "2:00" )
	PORT_DIPSETTING(      0x0800, "2:30" )
	PORT_DIPSETTING(      0x0400, "3:00" )
	PORT_DIPSETTING(      0x0000, "3:30" )
	PORT_DIPNAME( 0x3000, 0x3000, "Vs Single" )         PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "2:30" )
	PORT_DIPSETTING(      0x2000, "3:00" )
	PORT_DIPSETTING(      0x1000, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )
	PORT_DIPNAME( 0xc000, 0xc000, "Vs Tag" )            PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0xc000, "2:30" )
	PORT_DIPSETTING(      0x8000, "3:00" )
	PORT_DIPSETTING(      0x4000, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )

	PORT_START("IN0")
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("IN1")
	JOY_LSB(3, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	JOY_MSB(4, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END


/***************************************************************************
                            Grand Striker 2
***************************************************************************/

static INPUT_PORTS_START( gstrik2 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0003, 0x0003, "Player Vs Com" )         PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0003, "1:00" )
	PORT_DIPSETTING(      0x0002, "1:30" )
	PORT_DIPSETTING(      0x0001, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:30" )
	PORT_DIPNAME( 0x000c, 0x000c, "1P Vs 2P" )          PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x000c, "0:45" )
	PORT_DIPSETTING(      0x0008, "1:00" )
	PORT_DIPSETTING(      0x0004, "1:30" )
	PORT_DIPSETTING(      0x0000, "2:00" )
	PORT_DIPNAME( 0x0030, 0x0030, "Extra Time" )            PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(      0x0030, "0:30" )
	PORT_DIPSETTING(      0x0020, "0:45" )
	PORT_DIPSETTING(      0x0010, "1:00" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7") // Does not in Service Mode
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Time Period" )           PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, "Sudden Death" )
	PORT_DIPSETTING(      0x0000, "Full" )
	PORT_DIPNAME( 0x0700, 0x0400, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0700, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5") // Does not in Service Mode
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6") // Does not in Service Mode
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW4:8" )      // Does not in Service Mode

	PORT_START("DSW1")
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(      0x001c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0008, "4 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0016, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000d, "3 Coins/5 Credits" )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(      0x0000, "4 Coins/8 Credits" )
	PORT_DIPSETTING(      0x0009, "3 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0012, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, "3 Coins/7 Credits" )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0001, "3 Coins/8 Credits" )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x00e0, "Same as Coin A" )
	PORT_DIPNAME( 0x0300, 0x0300, "Credits to Start" )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Credits to Continue" )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Continue" )          PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6") // Does not in Service Mode
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Playmode" )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "1 Credit for 1 Player" )
	PORT_DIPSETTING(      0x0000, "1 Credit for 2 Players" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN0")
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, UNUSED)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, UNUSED)

	PORT_START("IN1")
	// Not Used

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Daitoride
***************************************************************************/

/* If only ONE of the "Coinage" is set to "Free Play", it is in fact "5C_1C".

   IN2 bits 12 and 13 are in fact "merged" :

     12  13    effect
     Off Off   Continue, Retry level
     On  Off   Continue, Ask player for retry
     Off On    No continue
     On  On    Continue, Retry level

*/
static INPUT_PORTS_START( daitoa )
	PORT_START("IN0") // $c00000
	COINS

	PORT_START("IN1") // $c00002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("DSW0") // $c00004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, "Timer Speed" )               PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "Slower" )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPNAME( 0x0800, 0x0800, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "1/1" )
	PORT_DIPSETTING(      0x0800, "2/3" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "Retry Level" )
	PORT_DIPSETTING(      0x2000, "Ask Player" )
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, "Retry Level" )   // Dulicate setting
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") // $c00006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( daitorid )
	PORT_INCLUDE( daitoa )

	PORT_MODIFY("IN0") // $c00000
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(metro_upd7810_state, custom_soundstatus_r)   // From Sound CPU
INPUT_PORTS_END


/***************************************************************************
                                Dharma Doujou
***************************************************************************/

/* Difficulty refers to how difficult the stack is solve in the given time.
   The manual calls it "Placement Difficulty" or block placement in the
   stack when you start the level. */

static INPUT_PORTS_START( dharma )
	PORT_START("IN0") //$c00000
	COINS_SOUND

	PORT_START("IN1") //$c00002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only

	PORT_START("DSW0") //$c00004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2") // Check code at 0x00da0a and see notes
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )     //   Table offset : 0x00e718
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )   //   Table offset : 0x00e770
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )     //   Table offset : 0x00e6c0
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  //   Table offset : 0x00e668
	PORT_DIPNAME( 0x0c00, 0x0c00, "Timer" )         PORT_DIPLOCATION("SW2:3,4") // Timer (crab) speed
	PORT_DIPSETTING(      0x0800, "Slow" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x1000, 0x1000, "2 Players Game" )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze (Cheat)")     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2") // $c00006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Gun Master
***************************************************************************/

static INPUT_PORTS_START( gunmast )
	PORT_START("IN0") //$400000
	COINS_SOUND

	PORT_START("IN1") //$400002
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, UNKNOWN)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, UNKNOWN)

	PORT_START("DSW0") //$400004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Allow P2 to Join Game" )     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" ) // Listed as "Unused"

	PORT_START("IN2")   // IN3 - $400006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************
                                Karate Tournament
***************************************************************************/

static INPUT_PORTS_START( karatour )
	PORT_START("IN0") // $400002
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("IN1") //$400004
	COINS

	PORT_START("DSW0") // $400006
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Time" )              PORT_DIPLOCATION("SW2:5") // Listed as "Timer"
	PORT_DIPSETTING(      0x0010, "60" ) // Listed as "Normal"
	PORT_DIPSETTING(      0x0000, "40" ) // Listed as "Short"
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:6") // Listed as "Unused"
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW1") // $40000a
	COINAGE_FLIP_LOC(SW1)
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN2") // $40000c
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
INPUT_PORTS_END


/***************************************************************************
                                Lady Killer
***************************************************************************/

static INPUT_PORTS_START( ladykill )
	PORT_START("IN0")   /*$400002*/
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("IN1")   /*$400004*/
	COINS

	PORT_START("DSW0")  // $400006
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Nudity" )                    PORT_DIPLOCATION("SW2:5") // Manual calls this "Sexy Version"
	PORT_DIPSETTING(      0x0010, "Partial" )
	PORT_DIPSETTING(      0x0000, "Full" )
	PORT_DIPNAME( 0x0020, 0x0020, "Service Mode / Free Play" )  PORT_DIPLOCATION("SW2:6")   // Keep Start2 pressed during boot - Manual states "Don't Change"
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW1") /*$40000a*/
	COINAGE_FLIP_LOC(SW1)
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )        // Manual states "Don't Change"

	PORT_START("IN2") /*$40000c*/
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START( moegonta )
	PORT_INCLUDE( ladykill )

	PORT_MODIFY("DSW0") // $400006
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )        // Same as 'ladykill' but NO "Nudity" Dip Switch
INPUT_PORTS_END


/***************************************************************************
                                Last Fortress
***************************************************************************/

/* The code which tests IN4 bit 7 is the SAME as the one for 'lastfero'.
   So WHY can't the game display cards instead of mahjong tiles ?
   Is it due to different GFX ROMS or to an emulation bug ?
*/

static INPUT_PORTS_START( lastfort )
	PORT_START("IN0")   /*$c00004*/
	COINS

	PORT_START("IN1")   /*$c00006*/
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only*/

	PORT_START("IN2")   /*$c00008*/
	JOY_LSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      /*BUTTON2 and BUTTON3 in "test mode" only*/

	PORT_START("DSW0")  /*$c0000a*/
	COINAGE_SERVICE_LOC(SW1)

	PORT_START("DSW1")  // $c0000c
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Timer speed
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )     //   Slow
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )   //   Normal
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )     //   Fast
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  //   Fastest
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Retry Level On Continue" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, "Ask Player" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0010, "2 Players Game" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
//  PORT_DIPNAME( 0x0080, 0x0080, "Tiles" )             PORT_DIPLOCATION("SW2:8")
//  PORT_DIPSETTING(      0x0080, "Mahjong" )
//  PORT_DIPSETTING(      0x0000, "Cards" )             // Not working - See notes
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )

	PORT_START("IN3")   // $c0000e
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Last Fortress (Erotic)
***************************************************************************/

// Same as 'lastfort' but WORKING "Tiles" Dip Switch
static INPUT_PORTS_START( lastfero )
	PORT_INCLUDE( lastfort )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Timer speed
	PORT_DIPSETTING(      0x0000, DEF_STR( Easiest ) )  //   Slowest
	PORT_DIPSETTING(      0x0001, DEF_STR( Easy ) )     //   Slow
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )   //   Normal
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )     //   Fast
	PORT_DIPNAME( 0x0080, 0x0080, "Tiles" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Mahjong" )
	PORT_DIPSETTING(      0x0000, "Cards" )
INPUT_PORTS_END


/***************************************************************************
                            Mahjong Doukyuusei
***************************************************************************/

static INPUT_PORTS_START( mj_panel )
	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dokyusei )
	PORT_INCLUDE( mj_panel )

	PORT_START("DSW0")  // $478884.w
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Game Sound" )            PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Auto TSUMO after REACH" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )

	PORT_START("DSW1")  // $478886.w
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "CPU wears clothes on RON" )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, "CPU clothes on continue play" )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Return to default" )
	PORT_DIPSETTING(      0x0000, "Keep current status" )
	PORT_SERVICE_DIPLOC(  0x0800, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x1000, 0x0000, "Self Test" )         PORT_DIPLOCATION("SW2:5") //!
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown 2-6" )           PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown 2-7" )           PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 2-8" )           PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                        Mahjong Gakuensai 1 & 2
***************************************************************************/

// Same as dokyusei, without the DSWs (these games have an eeprom)

static INPUT_PORTS_START( gakusai )

	PORT_INCLUDE( mj_panel )

INPUT_PORTS_END


/***************************************************************************
                                    Mouja
***************************************************************************/

static INPUT_PORTS_START( mouja )
	PORT_START("IN0") //$478880
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN1") //$478882
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0080, IP_ACTIVE_LOW)

	PORT_START("DSW0") //$478884
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Free_Play ) )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0000, "Winning Rounds (Player VS Computer)" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1/1" )
	PORT_DIPSETTING(      0x0000, "2/3" )
	PORT_DIPNAME( 0x1000, 0x1000, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "1/1" )
	PORT_DIPSETTING(      0x0000, "2/3" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$478886
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Pang Pom's
***************************************************************************/

static INPUT_PORTS_START( pangpoms )
	PORT_START("IN0") //$800004
	COINS

	PORT_START("IN1") //$800006
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("IN2") //$800008
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("DSW0") //$80000a
	COINAGE_SERVICE_LOC(SW1)

	PORT_START("DSW1") //$80000c
	PORT_DIPNAME( 0x0003, 0x0003, "Time Speed" )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "Slowest" )           // 60 (1 game sec. lasts x/60 real sec.)
	PORT_DIPSETTING(      0x0001, "Slow"    )           // 90
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )      // 120
	PORT_DIPSETTING(      0x0002, "Fast"    )           // 150
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0020, "400k and 800k" )
	PORT_DIPSETTING(      0x0030, "400k" )
	PORT_DIPSETTING(      0x0010, "800k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("IN3") //$80000e
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Poitto!
***************************************************************************/

static INPUT_PORTS_START( poitto )
	PORT_START("IN0") //$800000
	COINS_SOUND

	PORT_START("IN1") //$800002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only

	PORT_START("DSW0") //$800004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$800006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Puzzlet
***************************************************************************/

static INPUT_PORTS_START( puzzlet )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")       // IN1 - 7f8880.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)   // Next
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)   // Next
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // Rotate CW
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // Rotate CW
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // Push
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   // Push
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")       // IN2 - port 7
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")      // IN3 - dsw?
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x0002, 0x0002 ) // possibly Demo_Sounds? Verify when sound works.
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x0008, 0x0008 )
	PORT_DIPNAME( 0x0010, 0x0010, "Nudity" )
	PORT_DIPSETTING(      0x0010, "Topless" )
	PORT_DIPSETTING(      0x0000, "Full" )
	PORT_DIPUNKNOWN( 0x0020, 0x0020 )
	PORT_DIPUNKNOWN( 0x0040, 0x0040 ) // both 0x0040 and 0x0080 switch from 14 to 16 pieces to complete the puzzle. What's the difference between them?
	PORT_DIPUNKNOWN( 0x0080, 0x0080 )

	PORT_DIPNAME( 0xff00, 0xff00, DEF_STR( Coinage ) ) // all other settings redundant
	PORT_DIPSETTING(      0xef00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(      0xe700, "9 Coins/2 Credits" )
	PORT_DIPSETTING(      0xeb00, "9 Coins/3 Credits" )
	PORT_DIPSETTING(      0xe300, "9 Coins/4 Credits" )
	PORT_DIPSETTING(      0xed00, "9 Coins/5 Credits" )
	PORT_DIPSETTING(      0xe500, "9 Coins/6 Credits" )
	PORT_DIPSETTING(      0xe900, "9 Coins/7 Credits" )
	PORT_DIPSETTING(      0xe100, "9 Coins/8 Credits" )
	PORT_DIPSETTING(      0xee00, "9 Coins/9 Credits" )
	PORT_DIPSETTING(      0x1f00, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x1700, "8 Coins/2 Credits" )
	PORT_DIPSETTING(      0x1b00, "8 Coins/3 Credits" )
	PORT_DIPSETTING(      0x1300, "8 Coins/4 Credits" )
	PORT_DIPSETTING(      0x1d00, "8 Coins/5 Credits" )
	PORT_DIPSETTING(      0x1500, "8 Coins/6 Credits" )
	PORT_DIPSETTING(      0x1900, "8 Coins/7 Credits" )
	PORT_DIPSETTING(      0x1100, "8 Coins/8 Credits" )
	PORT_DIPSETTING(      0x1e00, "8 Coins/9 Credits" )
	PORT_DIPSETTING(      0x9f00, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0x9700, "7 Coins/2 Credits" )
	PORT_DIPSETTING(      0x9b00, "7 Coins/3 Credits" )
	PORT_DIPSETTING(      0x9300, "7 Coins/4 Credits" )
	PORT_DIPSETTING(      0x9d00, "7 Coins/5 Credits" )
	PORT_DIPSETTING(      0x9500, "7 Coins/6 Credits" )
	PORT_DIPSETTING(      0x9900, "7 Coins/7 Credits" )
	PORT_DIPSETTING(      0x9100, "7 Coins/8 Credits" )
	PORT_DIPSETTING(      0x9e00, "7 Coins/9 Credits" )
	PORT_DIPSETTING(      0x5f00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x5700, "6 Coins/2 Credits" )
	PORT_DIPSETTING(      0x5b00, "6 Coins/3 Credits" )
	PORT_DIPSETTING(      0x5300, "6 Coins/4 Credits" )
	PORT_DIPSETTING(      0x5d00, "6 Coins/5 Credits" )
	PORT_DIPSETTING(      0x5500, "6 Coins/6 Credits" )
	PORT_DIPSETTING(      0x5900, "6 Coins/7 Credits" )
	PORT_DIPSETTING(      0x5100, "6 Coins/8 Credits" )
	PORT_DIPSETTING(      0x5e00, "6 Coins/9 Credits" )
	PORT_DIPSETTING(      0xdf00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xd700, "5 Coins/2 Credits" )
	PORT_DIPSETTING(      0xdb00, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0xd300, "5 Coins/4 Credits" )
	PORT_DIPSETTING(      0xdd00, "5 Coins/5 Credits" )
	PORT_DIPSETTING(      0xd500, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0xd900, "5 Coins/7 Credits" )
	PORT_DIPSETTING(      0xd100, "5 Coins/8 Credits" )
	PORT_DIPSETTING(      0xde00, "5 Coins/9 Credits" )
	PORT_DIPSETTING(      0x3f00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x3700, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x3b00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3300, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x3d00, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x3500, "4 Coins/6 Credits" )
	PORT_DIPSETTING(      0x3900, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(      0x3100, "4 Coins/8 Credits" )
	PORT_DIPSETTING(      0x3e00, "4 Coins/9 Credits" )
	PORT_DIPSETTING(      0xbf00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xb700, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xbb00, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0xb300, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0xbd00, "3 Coins/5 Credits" )
	PORT_DIPSETTING(      0xb500, "3 Coins/6 Credits" )
	PORT_DIPSETTING(      0xb900, "3 Coins/7 Credits" )
	PORT_DIPSETTING(      0xb100, "3 Coins/8 Credits" )
	PORT_DIPSETTING(      0xbe00, "3 Coins/9 Credits" )
	PORT_DIPSETTING(      0x7f00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7700, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x7b00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x7300, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x7d00, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x7500, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(      0x7900, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(      0x7100, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(      0x7e00, "2 Coins/9 Credits" )
	PORT_DIPSETTING(      0xff00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xf700, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xfb00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xf300, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xfd00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xf500, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0xf900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0xf100, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(      0xfe00, DEF_STR( 1C_9C ) )
INPUT_PORTS_END


/***************************************************************************
                                Puzzli
***************************************************************************/

static INPUT_PORTS_START( puzzli )
	PORT_START("IN0") //$c00000
	COINS_SOUND

	PORT_START("IN1") //$c00002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("DSW0") //$c00004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
//  PORT_DIPSETTING(      0x0100, DEF_STR( Normal ) )           // Duplicated setting
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Join In" )               PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, "2 Players Game" )            PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0800, "2 Credits" )
	PORT_DIPNAME( 0x1000, 0x1000, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, "1/1" )
	PORT_DIPSETTING(      0x1000, "2/3" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$c00006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Sankokushi
***************************************************************************/

static INPUT_PORTS_START( sankokushi )
	PORT_START("IN0") //$c00000
	COINS_SOUND

	PORT_START("IN1") //$c00002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("DSW0") //$c00004
	COINAGE_FLIP_LOC(SW1)
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Timer speed
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )         //   Slow
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )       //   Normal
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )         //   Fast
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      //   Fastest
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Service Mode / Free Play" )  PORT_DIPLOCATION("SW2:6")   // Keep Start2 pressed during boot
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Helps" )                     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
INPUT_PORTS_END


/***************************************************************************
                                Pururun
***************************************************************************/

static INPUT_PORTS_START( pururun )
	PORT_START("IN0") //$400000
	COINS_SOUND

	PORT_START("IN1") //$400002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("DSW0") //$400004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Distance to goal
	PORT_DIPSETTING(      0x0200, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Join In" )           PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, "2 Players Game" )        PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0800, "2 Credits" )
	PORT_DIPNAME( 0x1000, 0x1000, "Bombs" )             PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2")   // IN3 - $400006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Sky Alert
***************************************************************************/

/* The game shows wrong values on screen for the "Bonus Life" Dip Switch !
   The wrong values are text which is stored at 0x02671a, and to determine
   which text to display, the routine at 0x0022f2 is called.
   The REAL "Bonus Life" table is stored at 0x0097f6, and to determine what
   are the values, the routine at 0x00974e is called.

   Here is the correspondance between real and fake values :

        Real         Fake
     100K, 400K   100K, 400K
     200K, 400K    50K, 300K
     200K         150K, 500K
       "none"       "none"

*/
static INPUT_PORTS_START( skyalert )
	PORT_START("IN0") //$400004
	COINS

	PORT_START("IN1") //$400006
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("IN2") //$400008
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("DSW0") //$40000a
	COINAGE_SERVICE_LOC(SW1)

	PORT_START("DSW1") //$40000c
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:5,6") // See notes
	PORT_DIPSETTING(      0x0030, "100K, every 400K" )
	PORT_DIPSETTING(      0x0020, "200K, every 400K" )
	PORT_DIPSETTING(      0x0010, "200K" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("IN3") //$40000e
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Toride II Adauchi Gaiden
***************************************************************************/

/* I don't really know HOW to describe the effect of IN2 bit 10.
   All I can tell is that is that it affects the levels which are
   proposed, but there is no evidence that one "table" is harder
   than another. */
static INPUT_PORTS_START( toride2g )
	PORT_START("IN0") //$800000
	COINS_SOUND

	PORT_START("IN1") //$800002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only

	PORT_START("DSW0") //$800004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, "Timer Speed" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "Slower" )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x0400, 0x0400, "Tile Arrangement" )      PORT_DIPLOCATION("SW2:3") // As listed by the manual
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0800, 0x0000, "Retry Level On Continue" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "Ask Player" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000, 0x1000, "2 Players Game" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$800006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // BIT 6 !?
INPUT_PORTS_END

/***************************************************************************
                                Varia Metal
***************************************************************************/

// verified from M68000 code
static INPUT_PORTS_START( vmetal )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_TILT )             // 'Tilt' only in "test mode" - no effect ingame
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )         // same coinage as COIN1 and COIN2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 )         // 'Test' only in "test mode" - no effect ingame
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	// DSW1, stored at 0xff0085.b (cpl'ed)
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C )  )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C )  )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C )  )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C )  )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_3C )  )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_4C )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C )  )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C )  )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:4" ) // 0x01 (OFF) or 0x02 (ON) written to 0xff0112.b but NEVER read back - old credits for 2 players game ?
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:5")  // 0x07c1 written to 0x1788ac.w (screen control ?) at first (code at 0x0001b8)
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )          // 0x07c1 written to 0xff0114.w (then 0x1788ac.w) during initialisation (code at 0x000436)
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )           // 0x07c0 written to 0xff0114.w (then 0x1788ac.w) during initialisation (code at 0x000436)
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN2")
	// DSW2, stored at 0xff0084.b (cpl'ed)
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "1"  )
	PORT_DIPSETTING(      0x0004, "2"  )
	PORT_DIPSETTING(      0x000c, "3"  )
	PORT_DIPSETTING(      0x0000, "4"  )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5")   // code at 0x0004a4
	PORT_DIPSETTING(      0x0010, "Every 30000" )
	PORT_DIPSETTING(      0x0000, "Every 60000" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END



/***************************************************************************


                            Graphics Layouts


***************************************************************************/


static const gfx_layout layout_053936_16 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8),STEP8(8*8*8*1,8) },
	{ STEP8(0,8*8),STEP8(8*8*8*2,8*8) },
	8*8*8*4
};

static GFXDECODE_START( gfx_blzntrnd )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x8_raw,    0x0, 0x10 ) // [0] 053936 Tiles
GFXDECODE_END

static GFXDECODE_START( gfx_gstrik2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_053936_16, 0x0, 0x10 ) // [0] 053936 Tiles
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void metro_upd7810_state::machine_start()
{
	metro_state::machine_start();

	if (m_audiobank.found())
	{
		m_audiobank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
		m_audiobank->set_entry(0);
	}

	m_porta = 0x00;
	m_portb = 0x00;
	m_busy_sndcpu = false;

	save_item(NAME(m_sound_data));
	save_item(NAME(m_soundstatus));
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_busy_sndcpu));
}

void vmetal_state::machine_start()
{
	metro_state::machine_start();

	m_essnd_gate = false;
	save_item(NAME(m_essnd_bank));
	save_item(NAME(m_essnd_gate));
}

void gakusai_state::machine_start()
{
	metro_state::machine_start();

	save_item(NAME(m_oki_bank_lo));
	save_item(NAME(m_oki_bank_hi));
}

void mouja_state::machine_start()
{
	metro_state::machine_start();

	m_mouja_irq_timer = timer_alloc(FUNC(mouja_state::mouja_irq), this);
	m_okibank->configure_entries(0, 8, memregion("oki")->base(), 0x20000);
}

void blzntrnd_state::machine_start()
{
	metro_state::machine_start();

	m_audiobank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
	save_item(NAME(m_ext_irq_enable));
}

void metro_state::i4100_config(machine_config &config)
{
	I4100(config, m_vdp, 24_MHz_XTAL);
	m_vdp->set_vblank_irq_level(0);
	m_vdp->set_blit_irq_level(2);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58.2328); // VSync 58.2328Hz, HSync 15.32kHz
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1500));
	m_screen->set_size(392, 263);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("vdp", FUNC(imagetek_i4100_device::screen_update));
}

void metro_state::i4220_config(machine_config &config)
{
	I4220(config, m_vdp2, 26.666_MHz_XTAL);
	m_vdp2->set_vblank_irq_level(0);
	m_vdp2->set_blit_irq_level(2);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58.2328); // VSync 58.2328Hz, HSync 15.32kHz
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1500));
	m_screen->set_size(392, 263);
	m_screen->set_visarea(0, 320-1, 0, 224-1);
	m_screen->set_screen_update("vdp2", FUNC(imagetek_i4100_device::screen_update));
}

void metro_state::i4300_config(machine_config &config)
{
	I4300(config, m_vdp3, 26.666_MHz_XTAL);
	m_vdp3->set_vblank_irq_level(1);
	m_vdp3->set_blit_irq_level(2);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58.2328); // VSync 58.2328Hz, HSync 15.32kHz
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1500));
	m_screen->set_size(392, 263);
	m_screen->set_visarea(0, 320-1, 0, 224-1);
	m_screen->set_screen_update("vdp3", FUNC(imagetek_i4100_device::screen_update));
}

// TODO: these comes from the CRTC inside the i4100
void metro_state::i4100_config_360x224(machine_config &config)
{
	i4100_config(config);

//  m_screen->set_size(392, 263);
	m_screen->set_visarea(0, 360-1, 0, 224-1);
}

void metro_state::i4220_config_320x240(machine_config &config)
{
	i4220_config(config);

//  m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
}

void metro_state::i4220_config_304x224(machine_config &config)
{
	i4220_config(config);

	m_screen->set_visarea(0, 304-1, 0, 224-1);
}

void metro_state::i4300_config_384x224(machine_config &config)
{
	i4300_config(config);
	m_vdp3->set_clock(32_MHz_XTAL);

//  m_screen->set_size(384, 240);
	m_screen->set_visarea(0, 384-1, 0, 224-1);
}

void metro_state::i4300_config_320x240(machine_config &config)
{
	i4300_config(config);

//  m_screen->set_size(384, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
}


void metro_state::msgogo(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_state::msgogo_map);
	m_maincpu->set_periodic_int(FUNC(metro_state::periodic_interrupt), attotime::from_hz(60)); // ?

	// video hardware
	i4220_config(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_1);
	m_vdp2->set_tmap_xoffsets(2,2,2);

	m_screen->screen_vblank().set(FUNC(metro_state::vblank_irq)); // timing is off, shaking sprites in intro

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ymf278b_device &ymf(YMF278B(config, "ymf", 33.8688_MHz_XTAL));
	ymf.set_addrmap(0, &metro_state::ymf278_map);
	ymf.irq_handler().set_inputline("maincpu", M68K_IRQ_2);
	ymf.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void metro_state::balcube(machine_config &config)
{
	msgogo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_state::balcube_map);
	m_maincpu->set_periodic_int(FUNC(metro_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	m_screen->screen_vblank().set(FUNC(metro_state::vblank_irq));
}

void metro_state::daitoa(machine_config &config)
{
	msgogo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_state::daitoa_map);
	m_maincpu->set_periodic_int(FUNC(metro_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	m_screen->screen_vblank().set(FUNC(metro_state::vblank_irq));
}

void metro_state::bangball(machine_config &config)
{
	msgogo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_state::bangball_map);
	m_maincpu->remove_periodic_int();
	TIMER(config, "scantimer").configure_scanline(FUNC(metro_state::bangball_scanline), "screen", 0, 1);

	// doesn't like 58.2 Hz
	m_screen->set_refresh_hz(60);
	m_screen->screen_vblank().set_nop();
}

void metro_state::batlbubl(machine_config &config)
{
	msgogo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_state::batlbubl_map);
	m_maincpu->remove_periodic_int();
	TIMER(config, "scantimer").configure_scanline(FUNC(metro_state::bangball_scanline), "screen", 0, 1);

	// doesn't like 58.2 Hz
	m_screen->set_refresh_hz(60);
	m_screen->screen_vblank().set_nop();
}

void metro_upd7810_state::metro_upd7810_sound(machine_config &config)
{
	upd78c10_device &upd(UPD78C10(config, m_audiocpu, 24_MHz_XTAL/2));
	upd.rxd_func().set(FUNC(metro_upd7810_state::rxd_r));
	upd.set_addrmap(AS_PROGRAM, &metro_upd7810_state::upd7810_map);
	upd.pa_in_cb().set(FUNC(metro_upd7810_state::upd7810_porta_r));
	upd.pa_out_cb().set(FUNC(metro_upd7810_state::upd7810_porta_w));
	upd.pb_out_cb().set(FUNC(metro_upd7810_state::upd7810_portb_w));
	upd.pc_out_cb().set(FUNC(metro_upd7810_state::upd7810_rombank_w<0x03>));
}

void metro_upd7810_state::daitorid_upd7810_sound(machine_config &config)
{
	upd78c10_device &upd(UPD78C10(config, m_audiocpu, 12_MHz_XTAL));
	upd.rxd_func().set(FUNC(metro_upd7810_state::rxd_r));
	upd.set_addrmap(AS_PROGRAM, &metro_upd7810_state::upd7810_map);
	upd.pa_in_cb().set(FUNC(metro_upd7810_state::upd7810_porta_r));
	upd.pa_out_cb().set(FUNC(metro_upd7810_state::upd7810_porta_w));
	upd.pb_out_cb().set(FUNC(metro_upd7810_state::daitorid_portb_w));
	upd.pc_out_cb().set(FUNC(metro_upd7810_state::upd7810_rombank_w<0x07>));
}

void metro_upd7810_state::daitorid(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::daitorid_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	daitorid_upd7810_sound(config);

	// video hardware
	i4220_config(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);
	m_vdp2->set_tmap_xoffsets(2,2,2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set_inputline(m_audiocpu, UPD7810_INTF2);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.80);

	OKIM6295(config, m_oki, 1200000, okim6295_device::PIN7_HIGH); // sample rate =  M6295 clock / 132
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);
}

void metro_upd7810_state::puzzli(machine_config &config)
{
	daitorid(config);

	m_maincpu->remove_periodic_int();
	TIMER(config, "scantimer").configure_scanline(FUNC(metro_upd7810_state::bangball_scanline), "screen", 0, 1);

	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);
	m_screen->screen_vblank().set_nop();
}

void metro_upd7810_state::puzzlia(machine_config &config)
{
	puzzli(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::pururun_map);
}

void metro_upd7810_state::dharma(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::dharma_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	// video hardware
	i4220_config(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_HIGH); // sample rate =  M6295 clock / 132
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void metro_upd7810_state::karatour(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::karatour_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	// video hardware
	i4100_config(config);
	m_vdp->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);
	m_vdp->ext_ctrl_0_cb().set(FUNC(metro_upd7810_state::ext_irq5_enable_w));
	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::karatour_vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_HIGH); // was /128.. so pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void metro_upd7810_state::sankokushi(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::kokushi_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	// video hardware
	i4220_config_320x240(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);
	m_vdp2->ext_ctrl_0_cb().set(FUNC(metro_upd7810_state::ext_irq5_enable_w));

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::karatour_vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_HIGH); // was /128.. so pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}


void metro_upd7810_state::lastfort(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::lastfort_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	// video hardware
	i4100_config_360x224(config);
	m_vdp->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_LOW); // sample rate =  M6295 clock / 165
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void metro_upd7810_state::lastforg(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::lastforg_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	i4100_config_360x224(config);
	m_vdp->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);
	m_vdp->ext_ctrl_0_cb().set(FUNC(metro_upd7810_state::ext_irq5_enable_w));
	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::karatour_vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_HIGH); // was /128.. so pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void gakusai_state::dokyusei(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &gakusai_state::dokyusei_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &gakusai_state::cpu_space_map);

	// video hardware
	i4300_config(config);
	m_vdp3->irq_cb().set(FUNC(gakusai_state::ipl_w));
	m_vdp3->set_blit_irq_level(3);

	m_screen->screen_vblank().set(FUNC(gakusai_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void gakusai_state::dokyusp(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gakusai_state::dokyusp_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &gakusai_state::cpu_space_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	i4300_config_384x224(config);
	m_vdp3->irq_cb().set(FUNC(gakusai_state::ipl_w));
	m_vdp3->set_blit_irq_level(3);

	m_screen->screen_vblank().set(FUNC(gakusai_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 2112000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void gakusai_state::gakusai(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 26.666_MHz_XTAL/2); // OSCs are 26.6660MHz & 3.579545MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &gakusai_state::gakusai_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &gakusai_state::cpu_space_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	i4300_config_320x240(config);
	m_vdp3->irq_cb().set(FUNC(gakusai_state::ipl_w));
	m_vdp3->set_blit_irq_level(3);

	m_screen->screen_vblank().set(FUNC(gakusai_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 2112000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 2.00);
}

void gakusai_state::gakusai2(machine_config &config)
{
	gakusai(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gakusai_state::gakusai2_map);
}

void metro_upd7810_state::pangpoms(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::pangpoms_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	// video hardware
	i4100_config_360x224(config);
	m_vdp->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_HIGH); // was /128.. so pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void metro_upd7810_state::poitto(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::poitto_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	// video hardware
	i4100_config_360x224(config);
	m_vdp->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_HIGH); // was /128.. so pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void metro_upd7810_state::pururun(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2); // Not confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::pururun_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	daitorid_upd7810_sound(config);

	// video hardware
	i4220_config(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));  // Confirmed match to reference video
	ymsnd.irq_handler().set_inputline(m_audiocpu, UPD7810_INTF2);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.80);

	OKIM6295(config, m_oki, 3.579545_MHz_XTAL/3, okim6295_device::PIN7_HIGH); // sample rate =  M6295 clock / 132
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.40);
}


void metro_upd7810_state::skyalert(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::skyalert_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	i4100_config_360x224(config);
	m_vdp->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_LOW); // sample rate =  M6295 clock / 165
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}


void metro_upd7810_state::toride2g(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &metro_upd7810_state::toride2g_map);
	m_maincpu->set_periodic_int(FUNC(metro_upd7810_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	metro_upd7810_sound(config);

	// video hardware
	i4220_config(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	m_screen->screen_vblank().set(FUNC(metro_upd7810_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL/20, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.10);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.90);
}


void mouja_state::mouja(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mouja_state::main_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &mouja_state::cpu_space_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	i4300_config(config);
	m_vdp3->irq_cb().set(FUNC(mouja_state::ipl_w));

	m_screen->screen_vblank().set(FUNC(mouja_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, 16_MHz_XTAL/1024*132, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->set_addrmap(0, &mouja_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2413_device &ymsnd(YM2413(config, m_ymsnd, 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.00);
}


void vmetal_state::vmetal(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vmetal_state::main_map);
	m_maincpu->set_periodic_int(FUNC(vmetal_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	// video hardware
	i4220_config_304x224(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_1);

	m_screen->screen_vblank().set(FUNC(vmetal_state::vblank_irq));

	m_vdp2->set_tmap_xoffsets(0,0,0);
	m_vdp2->set_tmap_yoffsets(0,0,0);
	// TODO: very fussy on screen geometry changes
	// CRTC is set as 320x224, bottom 16 pixels are actually aligned properly when flip screen is on.
	// Easiest alignment test is during story lore in attract, specifically at bomb explosion screen
	// (latter being a sprite needs to be 1:1 aligned with underlying background layer)
	m_vdp2->set_tmap_flip_xoffsets(88,88,88);
	m_vdp2->set_tmap_flip_yoffsets(39,39,39);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50);

	ES8712(config, m_essnd, 0);
	m_essnd->reset_handler().set(FUNC(vmetal_state::es8712_irq));
	m_essnd->msm_write_handler().set("msm", FUNC(msm6585_device::data_w));
	m_essnd->set_msm_tag("msm");

	msm6585_device &msm(MSM6585(config, "msm", 640_kHz_XTAL)); // Not verified, value from docs
	msm.vck_legacy_callback().set("essnd", FUNC(es8712_device::msm_int));
	msm.set_prescaler_selector(msm6585_device::S40); // Not verified, value from docs
	msm.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void blzntrnd_state::blzntrnd(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &blzntrnd_state::main_map);
	m_maincpu->set_periodic_int(FUNC(blzntrnd_state::periodic_interrupt), attotime::from_hz(8*60)); // ?

	Z80(config, m_audiocpu, 16_MHz_XTAL/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &blzntrnd_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &blzntrnd_state::sound_io_map);

	// video hardware
	I4220(config, m_vdp2, 26.666_MHz_XTAL);
	m_vdp2->irq_cb().set_inputline(m_maincpu, M68K_IRQ_1);
	m_vdp2->set_vblank_irq_level(0);
	m_vdp2->set_blit_irq_level(3);
	m_vdp2->set_spriteram_buffered(true); // sprites are 1 frame delayed
	m_vdp2->ext_ctrl_0_cb().set(FUNC(blzntrnd_state::ext_irq5_enable_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(58.2328); // VSync 58.2328Hz, HSync 15.32kHz
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1500));
	m_screen->set_size(392, 263);
	m_screen->set_visarea(0, 304-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(blzntrnd_state::screen_update));
	m_screen->screen_vblank().set(FUNC(blzntrnd_state::karatour_vblank_irq));

	MCFG_VIDEO_START_OVERRIDE(blzntrnd_state,blzntrnd)

	GFXDECODE(config, m_gfxdecode, "vdp2:palette", gfx_blzntrnd);

	K053936(config, m_k053936, 0);
	m_k053936->set_offsets(-77, -21);

	// sound hardware
	// HUM-002 PCB Configuration : Stereo output with second speaker connector
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2610_device &ymsnd(YM2610(config, m_ymsnd, 16_MHz_XTAL/2));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void blzntrnd_state::gstrik2(machine_config &config)
{
	blzntrnd(config);
	m_gfxdecode->set_info(gfx_gstrik2);

	MCFG_VIDEO_START_OVERRIDE(blzntrnd_state,gstrik2)

	m_k053936->set_offsets(-77, -19);

	m_vdp2->set_tmap_xoffsets(0,8,0);

	// HUM-003 PCB Configuration : Mono output only
	config.device_remove("lspeaker");
	config.device_remove("rspeaker");
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config.replace(), m_ymsnd, 16_MHz_XTAL/2));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.5);
	ymsnd.add_route(1, "mono", 0.5);
	ymsnd.add_route(2, "mono", 0.5);
}


void metro_state::puzzlet(machine_config &config)
{
	// basic machine hardware
	auto &maincpu(H83007(config, m_maincpu, 20_MHz_XTAL)); // H8/3007 - Hitachi HD6413007F20 CPU. Clock 20MHz
	maincpu.set_addrmap(AS_PROGRAM, &metro_state::puzzlet_map);
	maincpu.read_port7().set_ioport("IN2");
	maincpu.read_portb().set_ioport("DSW0");
	maincpu.write_portb().set(FUNC(metro_state::puzzlet_portb_w));

	// Coins/service
	z8_device &coinmcu(Z86E02(config, "coinmcu", 20_MHz_XTAL/5)); // clock divider guessed
	coinmcu.p0_in_cb().set_ioport("COIN");
	coinmcu.p2_in_cb().set_ioport("START");
	coinmcu.p2_out_cb().set(maincpu, FUNC(h83007_device::sci_rx_w<1>)).bit(6);
	maincpu.write_sci_tx<1>().set_inputline("coinmcu", INPUT_LINE_IRQ2).invert();
	maincpu.write_sci_clk<1>().set_inputline("coinmcu", INPUT_LINE_IRQ0).invert();

	// video hardware
	// TODO: looks like game is running in i4220 compatibility mode, $778000 seems to be an id for the chip?
	i4220_config(config);
	m_vdp2->irq_cb().set_inputline(m_maincpu, 0);
	m_vdp2->set_vblank_irq_level(1);
	m_vdp2->set_blit_irq_level(3);
	m_vdp2->ext_ctrl_0_cb().set(FUNC(metro_state::ext_irq5_enable_w));

	m_screen->screen_vblank().set(FUNC(metro_state::puzzlet_vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, 20_MHz_XTAL/5, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.50);
	YM2413(config, m_ymsnd, 20_MHz_XTAL/5).add_route(0, "mono", 0.90);
}


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

Bal Cube
Metro 1996

+--------------------------------------------+
|               BAL-CUBE_07      BAL-CUBE_01 |
|                 YRW801-M       BAL-CUBE_02 |
|                                BAL-CUBE_03 |
|J      33.869MHz YMF278B        BAL-CUBE_04 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|     BAL-CUBE_06 BAL-CUBE_05 I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199 61C64  |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T
Video: Imagetek I4220

SW3 - Not Populated
***************************************************************************/

ROM_START( balcube )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "bal-cube_06.6", 0x000000, 0x040000, CRC(c400f84d) SHA1(416eb82ec1201d24d9d964191a5a1792c9445923) ) // Silkscreened 6 and U18
	ROM_LOAD16_BYTE( "bal-cube_05.5", 0x000001, 0x040000, CRC(15313e3f) SHA1(10a8702016f223194dc91875b4736253fd47dbb8) ) // Silkscreened 5 and U19

	ROM_REGION( 0x400000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "bal-cube_02.2", 0x000000, 0x080000, CRC(492ca8f0) SHA1(478336a462a2bfc288cf91262314f5767f8c707d) ) // Silkscreened 2 and U30
	ROM_LOAD64_WORD( "bal-cube_04.4", 0x000002, 0x080000, CRC(d1acda2c) SHA1(f58015302af6c864523d48bdf8f8a4383b69fa9d) ) // Silkscreened 4 and U29
	ROM_LOAD64_WORD( "bal-cube_01.1", 0x000004, 0x080000, CRC(0ea3d161) SHA1(63ae430a19e777ce82b41ab02baef3bb224c7557) ) // Silkscreened 1 and U28
	ROM_LOAD64_WORD( "bal-cube_03.3", 0x000006, 0x080000, CRC(eef1d3b4) SHA1(be535963c00390e34a2305586397a16325f3c3c0) ) // Silkscreened 3 and U27

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",      0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) // Silkscreened U52        // Yamaha YRW801 2MB ROM with samples for the OPL4.
	ROM_LOAD( "bal-cube_07.7", 0x200000, 0x080000, CRC(f769287d) SHA1(dd0f781b4a1a1fd6bf0a50048b4996f3cf41e155) ) // Silkscreened 7 and U49  // PCM 16 Bit (Signed)
ROM_END


/***************************************************************************

Bang Bang Ball
(c)1996 Banpresto/Kunihiko Tashiro/Goodhouse

+--------------------------------------------+
|                 rom#007         BP963A_U28 |
|                 YRW801-M        BP963A_U30 |
|                                 BP963A_U27 |
|J      33.369MHz YMF278B         BP963A_U29 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|         rom#006 rom#005     I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199 61C64  |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T D9522
Video: Imagetek I4220 071 9403EK701

SW3 - Not Populated

ROMs:
B-BALL/J rom #005.u19 - Main programs (27c020)
B-BALL/J rom #006.u18 /

B-BALL/J rom #007.u49 - Sound samples (27c040)
yrw801-m.u52          - Yamaha wave data ROM (44pin SOP 16M mask (LH537019))

BP963A U27 - Graphics (MASK, read as 27c800)
BP963A U28 |
BP963A U29 |
BP963A U30 /

**********************************

Battle Bubble
(c)1999 Limenko

  Listed on Limenko's Web site as kit LM2DY00

PCB -
 REV: LM2D-Y
 SEL: 00-200-004

Same basic components as those listed for Bang Bang Ball, except
PCB uses a Xlinix XC9536 istead of the Altera EMP7032LC44 PLD.

Did Limenko license this or bootleg it?  The board doesn't look like a
bootleg and has all original parts on it..

Limenko's web site states:

 1998  6 Developed LM2D-Y00-LM
      10 Contract the technology and products in cooperation with Metro Ltd.
 1999 11 Begin to sell Battle Bubble internally
      12 Received an overseas order for Battle Bubble

***************************************************************************/

ROM_START( bangball )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "b-ball_j_rom@006.u18", 0x000000, 0x040000, CRC(0e4124bc) SHA1(f5cd762df4e822ab5c8dba6f276b3366895235d1) ) // Silkscreened 6 and U18
	ROM_LOAD16_BYTE( "b-ball_j_rom@005.u19", 0x000001, 0x040000, CRC(3fa08587) SHA1(8fdafdde5e77d077b5cd8f94f97b5430fe062936) ) // Silkscreened 5 and U19

	ROM_REGION( 0x400000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "bp963a_u30.u30", 0x000000, 0x100000, CRC(b0ca8e39) SHA1(f2eb1d07cd10050c234f0b418146c742b496f196) ) // Silkscreened 2 and U30
	ROM_LOAD64_WORD( "bp963a_u29.u29", 0x000002, 0x100000, CRC(d934468f) SHA1(b93353bf2302b68a297d71fc9d91dc55c1cccce4) ) // Silkscreened 4 and U29
	ROM_LOAD64_WORD( "bp963a_u28.u28", 0x000004, 0x100000, CRC(96d03c6a) SHA1(6257585721291e5a5ce311c2873c9e1e1dac2fc6) ) // Silkscreened 1 and U28
	ROM_LOAD64_WORD( "bp963a_u27.u27", 0x000006, 0x100000, CRC(5e3c7732) SHA1(e8c442a8038921ae3de48ce52923d25cb97e36ea) ) // Silkscreened 3 and U27

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",             0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) // Silkscreened U52
	ROM_LOAD( "b-ball_j_rom@007.u49", 0x200000, 0x080000, CRC(04cc91a9) SHA1(e5cf6055a0803f4ad44919090cd147702e805d88) ) // Silkscreened 7 and U49
ROM_END

ROM_START( batlbubl )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "lm-01.u11", 0x000000, 0x080000, CRC(1d562807) SHA1(3e5dbe6f4b04aa9e01b7b8938d0b46d4862054bf) )
	ROM_LOAD16_WORD_SWAP( "lm-02.u12", 0x080000, 0x080000, CRC(852e4750) SHA1(d8b703ba65d0f267eba07f160b13dbe0f5ac40c2) )

	ROM_REGION( 0x800000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "lm-07.u30", 0x000000, 0x200000, CRC(03d9dfd8) SHA1(33c96f1b0fa28c6e46b2d2c0a62dfe0306139e09) )
	ROM_LOAD64_WORD( "lm-06.u29", 0x000002, 0x200000, CRC(5efb905b) SHA1(c3f5d781941225c17d37473e2e0ed84875cebace) )
	ROM_LOAD64_WORD( "lm-05.u28", 0x000004, 0x200000, CRC(e53ba59f) SHA1(d82749c04d776fbf9e5cc44a23d2bfafe073fafa) )
	ROM_LOAD64_WORD( "lm-04.u27", 0x000006, 0x200000, CRC(2e687cfb) SHA1(4766ddc882c3e330e948b64e4e44a08846bf2046) )

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "lm-08.u40", 0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) // PCB labeled YRM801, Sticker says lm-08
	ROM_LOAD( "lm-03.u42", 0x200000, 0x080000, CRC(04cc91a9) SHA1(e5cf6055a0803f4ad44919090cd147702e805d88) )
ROM_END

/***************************************************************************

Blazing Tornado
(c)1994 Human

CPU:    68000-16
Sound:  Z80-8
    YMF286K (YM2610 compatible)
OSC:    16.0000MHz
    26.666MHz
Chips:  Imagetek I4220 071
    Konami 053936 (PSAC2)

***************************************************************************/

ROM_START( blzntrnd )
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000
	ROM_LOAD16_BYTE( "1k.bin", 0x000000, 0x80000, CRC(b007893b) SHA1(609363449c0218b8a38de72d37c66e6f3bb4f8cd) )
	ROM_LOAD16_BYTE( "2k.bin", 0x000001, 0x80000, CRC(ec173252) SHA1(652d70055d2799442beede1ae68e54551931068f) )
	ROM_LOAD16_BYTE( "3k.bin", 0x100000, 0x80000, CRC(1e230ba2) SHA1(ca96c82d57a6b5bacc1bfd2f7965503c2a6e162f) )
	ROM_LOAD16_BYTE( "4k.bin", 0x100001, 0x80000, CRC(e98ca99e) SHA1(9346fc0d419add23eaceb5843c505f3ffa69e495) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "rom5.bin", 0x00000, 0x20000, CRC(7e90b774) SHA1(abd0eda9eababa1f7ab17a2f60534dcebda33c9c) )

	ROM_REGION( 0x1800000, "vdp2", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "rom142.bin", 0x0000000, 0x200000, CRC(a7200598) SHA1(f8168a94abc380308901303a69cbd15097019797) )
	ROM_LOAD64_WORD( "rom186.bin", 0x0000002, 0x200000, CRC(6ee28ea7) SHA1(b33bcbf16423999135d96a62bf25c6ff23031f2a) )
	ROM_LOAD64_WORD( "rom131.bin", 0x0000004, 0x200000, CRC(c77e75d3) SHA1(8ad716d4e37d6efe478a8e49feb4e68283310890) )
	ROM_LOAD64_WORD( "rom175.bin", 0x0000006, 0x200000, CRC(04a84f9b) SHA1(83aabbc1c7ab06b351168153335f3c2f91fba0e9) )
	ROM_LOAD64_WORD( "rom242.bin", 0x0800000, 0x200000, CRC(1182463f) SHA1(6fa2a0b3186a3542b43926e3f37714b78a890542) )
	ROM_LOAD64_WORD( "rom286.bin", 0x0800002, 0x200000, CRC(384424fc) SHA1(f89d43756bd38515a223fe4ffbed3a44c673ae28) )
	ROM_LOAD64_WORD( "rom231.bin", 0x0800004, 0x200000, CRC(f0812362) SHA1(9f8be51f60f7baf72f9de8352e4e13d730f85903) )
	ROM_LOAD64_WORD( "rom275.bin", 0x0800006, 0x200000, CRC(184cb129) SHA1(8ffb3cdc7e0d227b6f0a7962bc6d853c6b84c8d2) )
	ROM_LOAD64_WORD( "rom342.bin", 0x1000000, 0x200000, CRC(e527fee5) SHA1(e5de1e134d95aa7a48695183189924061482e3a3) )
	ROM_LOAD64_WORD( "rom386.bin", 0x1000002, 0x200000, CRC(d10b1401) SHA1(0eb75a283000a8b19a14177461b6f335c9d9dec2) )
	ROM_LOAD64_WORD( "rom331.bin", 0x1000004, 0x200000, CRC(4d909c28) SHA1(fb9bb824e518f67713799ed2c0159a7bd70f35c4) )
	ROM_LOAD64_WORD( "rom375.bin", 0x1000006, 0x200000, CRC(6eb4f97c) SHA1(c7f006230cbf10e706b0362eeed34655a3aef1a5) )

	ROM_REGION( 0x200000, "gfx2", 0 )   // 053936 gfx data
	ROM_LOAD( "rom9.bin", 0x000000, 0x200000, CRC(37ca3570) SHA1(3374c586bf84583fa33f2793c4e8f2f61a0cab1c) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )   // Samples
	ROM_LOAD( "rom8.bin", 0x000000, 0x080000, CRC(565a4086) SHA1(bd5780acfa5affa8705acbfccb0af16bac8ed298) )

	ROM_REGION( 0x400000, "ymsnd:adpcma", 0 )  // Samples
	ROM_LOAD( "rom6.bin", 0x000000, 0x200000, CRC(8b8819fc) SHA1(5fd9d2b5088cb676c11d32cac7ba8c5c18e31b64) )
	ROM_LOAD( "rom7.bin", 0x200000, 0x200000, CRC(0089a52b) SHA1(d643ac122d62557de27f06ba1413ef757a45a927) )
ROM_END

/*

Grand Striker 2
Human Entertainment, 1996

PCB Layout
----------

HUM-003-(A)
|-----------------------------------------------------------------------|
|           YM3016 ROM8.22  ROM342.88  ROM386.87  ROM331.86  ROM375.85  |
|                                                                       |
| 6264  YM2610         ROM142.80  ROM186.79  ROM131.78  ROM175.77       |
|                                                                       |
|                  ROM7.27  ROM442.92  ROM486.91  ROM431.90  ROM475.89  |
|                                                                       |
|          PAL         ROM242.84  ROM286.83  ROM231.82  ROM275.81       |
|  SPRG.30                                                              |
|  PAL     Z80     ROM6.23                                              |
|                                                                       |
|J                                                                      |
|A                                                                      |
|M                                               |--------|             |
|M                   PRG2  PRG3                  |IMAGETEK|   6264      |
|A                                               |I4220   |             |
|                    PRG0  PRG1                  |--------|             |
|     16MHz  68000   62256  62256    26.666MHz                          |
|                                                                       |
|     DSW1                                                              |
|     DSW2   EPM7032         |------|  62256  62256                     |
|     DSW3            6116   |053936|  62256  62256                     |
|     DSW4            6116   |PSAC2 |                   PAL             |
|                            |------|                          ROM9.60  |
|-----------------------------------------------------------------------|

Notes:
       68000 clock: 16.000MHz
         Z80 clock: 8.000MHz
      YM2610 clock: 8.000MHz
             VSync: 58Hz
             HSync: 15.11kHz

TODO:
    HUM-002-A-(B) PCB set is also exists, but not dumped. it's blazing tornado conversion?
*/


// The MASK roms weren't dumped from this set, but it's safe to assume they're the same in this case
ROM_START( gstrik2 )
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000
	ROM_LOAD16_BYTE( "hum_003_g2f.rom1.u107", 0x000000, 0x80000, CRC(2712d9ca) SHA1(efa967de931728534a663fa1529e92003afbb3e9) )
	ROM_LOAD16_BYTE( "hum_003_g2f.rom2.u108", 0x000001, 0x80000, CRC(86785c64) SHA1(ef172d6e859a68eb80f7c127b61883d50eefb0fe) )
	ROM_LOAD16_BYTE( "prg2.109", 0x100000, 0x80000, CRC(ead86919) SHA1(eb9b68dff4e08d90ac90043c7f3021914caa007d) )
	ROM_LOAD16_BYTE( "prg3.110", 0x100001, 0x80000, CRC(e0b026e3) SHA1(05f75c0432efda3dec0372199382e310bb268fba) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "sprg.30", 0x00000, 0x20000, CRC(aeef6045) SHA1(61b8c89ca495d3aac79e53413a85dd203db816f3) )

	ROM_REGION( 0x1000000, "vdp2", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "chr0.80", 0x0000000, 0x200000, CRC(f63a52a9) SHA1(1ad52bb3a051eaffe8fb6ba49d4fc1d0b6144156) )
	ROM_LOAD64_WORD( "chr1.79", 0x0000002, 0x200000, CRC(4110c184) SHA1(90ccb3d50eff7a655336cfa9c072f7213589e64c) )
	ROM_LOAD64_WORD( "chr2.78", 0x0000004, 0x200000, CRC(ddb4b9ee) SHA1(0e2c151c3690b9c3d298dda8842e283660d37386) )
	ROM_LOAD64_WORD( "chr3.77", 0x0000006, 0x200000, CRC(5ab367db) SHA1(adf8749451f4583f8e9e00ab61f3408d804a7265) )
	ROM_LOAD64_WORD( "chr4.84", 0x0800000, 0x200000, CRC(77d7ef99) SHA1(8f5cf72f5919fe9363e7549e0bb1b3ee633cec3b) )
	ROM_LOAD64_WORD( "chr5.83", 0x0800002, 0x200000, CRC(a4d49e95) SHA1(9789bacba7876100e0f0293f54c81def545ed068) )
	ROM_LOAD64_WORD( "chr6.82", 0x0800004, 0x200000, CRC(32eb33b0) SHA1(2ea06484ca326b44a35ee470343147a9d91d5626) )
	ROM_LOAD64_WORD( "chr7.81", 0x0800006, 0x200000, CRC(2d30a21e) SHA1(749e86b7935ef71556eaee4caf6f954634e9bcbf) )
	// not populated
//  ROM_LOAD64_WORD( "chr8.88", 0x1000000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr9.87", 0x1000002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr10.86", 0x1000004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr11.85", 0x1000006, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr12.92", 0x1800000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr13.91", 0x1800002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr14.90", 0x1800004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr15.89", 0x1800006, 0x200000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx2", 0 )   // 053936 gfx data
	ROM_LOAD( "psacrom.60", 0x000000, 0x200000,  CRC(73f1f279) SHA1(1135b2b1eb4c52249bc12ee178340bbb202a94c8) )

	ROM_REGION( 0x200000, "ymsnd:adpcmb", 0 )   // Samples
	ROM_LOAD( "sndpcm-b.22", 0x000000, 0x200000, CRC(a5d844d2) SHA1(18d644545f0844e66aa53775b67b0a29c7b7c31b) )

	ROM_REGION( 0x400000, "ymsnd:adpcma", 0 )  // Samples
	ROM_LOAD( "sndpcm-a.23", 0x000000, 0x200000, CRC(e6d32373) SHA1(8a79d4ea8b27d785fffd80e38d5ae73b7cea7304) )
	// ROM7.27 not populated?
ROM_END

ROM_START( gstrik2j )
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000
	ROM_LOAD16_BYTE( "prg0.107", 0x000000, 0x80000, CRC(e60a8c19) SHA1(19be6cfcb60ede6fd4eb2e14914b174107c4b52d) )
	ROM_LOAD16_BYTE( "prg1.108", 0x000001, 0x80000, CRC(853f6f7c) SHA1(8fb9d7cd0390f620560a1669bb13f2033eed7c81) )
	ROM_LOAD16_BYTE( "prg2.109", 0x100000, 0x80000, CRC(ead86919) SHA1(eb9b68dff4e08d90ac90043c7f3021914caa007d) )
	ROM_LOAD16_BYTE( "prg3.110", 0x100001, 0x80000, CRC(e0b026e3) SHA1(05f75c0432efda3dec0372199382e310bb268fba) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // Z80
	ROM_LOAD( "sprg.30", 0x00000, 0x20000, CRC(aeef6045) SHA1(61b8c89ca495d3aac79e53413a85dd203db816f3) )

	ROM_REGION( 0x1000000, "vdp2", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "chr0.80", 0x0000000, 0x200000, CRC(f63a52a9) SHA1(1ad52bb3a051eaffe8fb6ba49d4fc1d0b6144156) )
	ROM_LOAD64_WORD( "chr1.79", 0x0000002, 0x200000, CRC(4110c184) SHA1(90ccb3d50eff7a655336cfa9c072f7213589e64c) )
	ROM_LOAD64_WORD( "chr2.78", 0x0000004, 0x200000, CRC(ddb4b9ee) SHA1(0e2c151c3690b9c3d298dda8842e283660d37386) )
	ROM_LOAD64_WORD( "chr3.77", 0x0000006, 0x200000, CRC(5ab367db) SHA1(adf8749451f4583f8e9e00ab61f3408d804a7265) )
	ROM_LOAD64_WORD( "chr4.84", 0x0800000, 0x200000, CRC(77d7ef99) SHA1(8f5cf72f5919fe9363e7549e0bb1b3ee633cec3b) )
	ROM_LOAD64_WORD( "chr5.83", 0x0800002, 0x200000, CRC(a4d49e95) SHA1(9789bacba7876100e0f0293f54c81def545ed068) )
	ROM_LOAD64_WORD( "chr6.82", 0x0800004, 0x200000, CRC(32eb33b0) SHA1(2ea06484ca326b44a35ee470343147a9d91d5626) )
	ROM_LOAD64_WORD( "chr7.81", 0x0800006, 0x200000, CRC(2d30a21e) SHA1(749e86b7935ef71556eaee4caf6f954634e9bcbf) )
	// not populated
//  ROM_LOAD64_WORD( "chr8.88", 0x1000000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr9.87", 0x1000002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr10.86", 0x1000004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr11.85", 0x1000006, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr12.92", 0x1800000, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr13.91", 0x1800002, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr14.90", 0x1800004, 0x200000, NO_DUMP )
//  ROM_LOAD64_WORD( "chr15.89", 0x1800006, 0x200000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx2", 0 )   // 053936 gfx data
	ROM_LOAD( "psacrom.60", 0x000000, 0x200000,  CRC(73f1f279) SHA1(1135b2b1eb4c52249bc12ee178340bbb202a94c8) )

	ROM_REGION( 0x200000, "ymsnd:adpcmb", 0 )   // Samples
	ROM_LOAD( "sndpcm-b.22", 0x000000, 0x200000, CRC(a5d844d2) SHA1(18d644545f0844e66aa53775b67b0a29c7b7c31b) )

	ROM_REGION( 0x400000, "ymsnd:adpcma", 0 )  // Samples
	ROM_LOAD( "sndpcm-a.23", 0x000000, 0x200000, CRC(e6d32373) SHA1(8a79d4ea8b27d785fffd80e38d5ae73b7cea7304) )
	// ROM7.27 not populated?
ROM_END


/***************************************************************************

Daitoride
Metro 1995

MTR5260-A

                      3.5759MHz  12MHz  6116
   26.666MHz        YM2151          DT7  DT8
                            M6295
     7C199                             78C10
     7C199       Imagetek I4220
     61C64

                  68000-16             DT1
                  32MHz    52258       DT2
   SW1                     52258       DT3
   SW2            DT6  DT5             DT4

********************************************************

Daitoride (YMF278B version)
Metro 1996

+--------------------------------------------+
|                 DT_JA-7            DT_JA-1 |
|                 YRW801-M           DT_JA-2 |
|                                    DT_JA-3 |
|J      33.369MHz YMF278B            DT_JA-4 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|         DT_JA-6 DT_JA-5     I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199 61C64  |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T D9519
Video: Imagetek I4220 071 9338EK709

SW3 - Not Populated
***************************************************************************/

ROM_START( daitorid )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "dt-ja-5.19e", 0x000000, 0x020000, CRC(441efd77) SHA1(18b255f42ba7a180535f0897aaeebe5d2a33df46) )
	ROM_LOAD16_BYTE( "dt-ja-6.19c", 0x000001, 0x020000, CRC(494f9cc3) SHA1(b88af581fee9e2d94a12a5c1fed0797614bb738e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "dt-ja-8.3h", 0x000000, 0x020000, CRC(0351ad5b) SHA1(942c1cbb52bf2933aea4209335c1bc4cdd1cc3dd) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "dt-ja-2.14h", 0x000000, 0x080000, CRC(56881062) SHA1(150a8f043e61b28c22d0f898aea61853d1accddc) )
	ROM_LOAD64_WORD( "dt-ja-4.18h", 0x000002, 0x080000, CRC(85522e3b) SHA1(2c6e7c8ad01d39843669ef1afe7a0843ea6c107c) )
	ROM_LOAD64_WORD( "dt-ja-1.12h", 0x000004, 0x080000, CRC(2a220bf2) SHA1(553dea2ab42d845b2e91930219fe8df026748642) )
	ROM_LOAD64_WORD( "dt-ja-3.16h", 0x000006, 0x080000, CRC(fd1f58e0) SHA1(b4bbe94127ae59d4c899d09862703c374c8f4746) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "dt-ja-7.3f", 0x000000, 0x040000, CRC(0d888cde) SHA1(fa871fc34f8b8ff0eebe47f338733e4f9fe65b76) )
ROM_END

ROM_START( daitorida )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "dt_ja-6.6", 0x000000, 0x040000, CRC(c753954e) SHA1(f895c776ec6e2da063d3fbf9630f4812ba7bc455) ) // Silkscreened 6 and U18
	ROM_LOAD16_BYTE( "dt_ja-5.5", 0x000001, 0x040000, CRC(c4340290) SHA1(6748572a8733d88a1dd03604628e3d0e90171cf0) ) // Silkscreened 5 and U19

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "dt_ja-2.2", 0x000000, 0x080000, CRC(6a262249) SHA1(93b58825a454403d568e7d9a3b4d998322d0baef) ) // Silkscreened 2 and U30
	ROM_LOAD64_WORD( "dt_ja-4.4", 0x000002, 0x080000, CRC(cdcef57a) SHA1(4b386f5ebde1ab6866bbbe528e43b813eba99237) ) // Silkscreened 4 and U29
	ROM_LOAD64_WORD( "dt_ja-1.1", 0x000004, 0x080000, CRC(a6ccb1d2) SHA1(87570b8d82af0529c054b3038b3d3e9aa550ce6a) ) // Silkscreened 1 and U28
	ROM_LOAD64_WORD( "dt_ja-3.3", 0x000006, 0x080000, CRC(32353e04) SHA1(16ac82de9e6e43eabef3adab2d3a006bb50100fb) ) // Silkscreened 3 and U27

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",  0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) // Silkscreened U52    // Yamaha YRW801 2MB ROM with samples for the OPL4.
	ROM_LOAD( "dt_ja-7.7", 0x200000, 0x080000, CRC(7a2d3222) SHA1(1a16bf483a5a086ad48029dd23dd16ad47c3740e) ) // Silkscreened 7 and U49  // PCM 16 Bit (Signed)
ROM_END


/***************************************************************************

Dharma Doujou
Metro 1994

MTR5260-A
|-----------------------------------------------|
|TA7222            3.579545MHz                  |
|            YM3012                      6116   |
|C3403  C3403      YM2413  M6295                |
|       26.666MHz             DD_JA-7  DD_JA-8  |
|            7C199                              |
|J           7C199          |--------|  D78C10  |
|A           7C199          |IMAGETEK|          |
|M                          |I4220   |          |
|M               MM1035     |        |          |
|A         |------------|   |--------|          |
|          |  68000-12  |              DD_JB-1  |
|          |------------|                       |
|                                      DD_JB-2  |
|              24MHz                            |
|       MACH110                        DD_JB-3  |
|                  6264                         |
|DSW1              6264                DD_JB-4  |
|DSW2  DD_JC-6   DD_JC-5                        |
|-----------------------------------------------|
Notes:
      68000 clock     - 12.000MHz [24/2]
      D78C10 clock    - 12.000MHz [24/2]
      YM2413 clock    - 3.579545MHz
      Oki M6295 clock - 1.200MHz [24/20], sample rate = 1200000 / 132
      VSync - 60Hz
      HSync - 15.55kHz


Korean version & international version of Dharma run on Metro hardware PCB Number - METRO CORP. MTR527


***************************************************************************/

ROM_START( dharma )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "dd__wea5.u39", 0x000000, 0x020000, CRC(960319d7) SHA1(f76783fcbb5e5a027889620c783f053d372346a8) )
	ROM_LOAD16_BYTE( "dd__wea6.u42", 0x000001, 0x020000, CRC(386eb6b3) SHA1(e353ea70bae521c4cc362cf2f5ce643c98c61681) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "dd__wa-8.u9", 0x000000, 0x020000, CRC(af7ebc4c) SHA1(6abf0036346da10be56932f9674f8c250a3ea592) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF) // == dd_ja-8

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "dd__wa-2.u4",  0x000000, 0x080000, CRC(2c67a5c8) SHA1(777d5f64446004bbb6dafee610ad9a1ff262349d) )
	ROM_LOAD64_WORD( "dd__wa-4.u5",  0x000002, 0x080000, CRC(36ca7848) SHA1(278788727193ae65ed012d230a4e5966c07afe9e) )
	ROM_LOAD64_WORD( "dd__wa-1.u10", 0x000004, 0x080000, CRC(d8034574) SHA1(a9bf29ae980033dfaae43b6ab46f850744020d92) )
	ROM_LOAD64_WORD( "dd__wa-3.u11", 0x000006, 0x080000, CRC(fe320fa3) SHA1(80532cc38bd21608e4cff1254d993e0df72eaccf) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "dd__wa-7.u3", 0x000000, 0x040000, CRC(7ce817eb) SHA1(9dfb79021a552877fbc26049cca853c0b93735b5) ) // == dd_ja-7
ROM_END

ROM_START( dharmag )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "dd__wga5.u39", 0x000000, 0x020000, CRC(b08664f7) SHA1(d4df2af4c8c0a736d5454d74cd2ce1d770feb8e3) )
	ROM_LOAD16_BYTE( "dd__wga6.u42", 0x000001, 0x020000, CRC(4ae89edc) SHA1(a02dba09359fa99f946c8afad89625c63c7ed14e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "dd__wa-8.u9", 0x000000, 0x020000, CRC(af7ebc4c) SHA1(6abf0036346da10be56932f9674f8c250a3ea592) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF) // == dd_ja-8

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "dd__wa-2.u4",  0x000000, 0x080000, CRC(2c67a5c8) SHA1(777d5f64446004bbb6dafee610ad9a1ff262349d) )
	ROM_LOAD64_WORD( "dd__wa-4.u5",  0x000002, 0x080000, CRC(36ca7848) SHA1(278788727193ae65ed012d230a4e5966c07afe9e) )
	ROM_LOAD64_WORD( "dd__wa-1.u10", 0x000004, 0x080000, CRC(d8034574) SHA1(a9bf29ae980033dfaae43b6ab46f850744020d92) )
	ROM_LOAD64_WORD( "dd__wa-3.u11", 0x000006, 0x080000, CRC(fe320fa3) SHA1(80532cc38bd21608e4cff1254d993e0df72eaccf) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "dd__wa-7.u3", 0x000000, 0x040000, CRC(7ce817eb) SHA1(9dfb79021a552877fbc26049cca853c0b93735b5) ) // == dd_ja-7
ROM_END

ROM_START( dharmaj )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "dd_jc-5", 0x000000, 0x020000, CRC(b5d44426) SHA1(d68aaf6b9976ccf5cb665d7ec0afa44e2453094d) )
	ROM_LOAD16_BYTE( "dd_jc-6", 0x000001, 0x020000, CRC(bc5a202e) SHA1(c2b6d2e44e3605e0525bde4030c5162badad4d4b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "dd_ja-8", 0x000000, 0x020000, CRC(af7ebc4c) SHA1(6abf0036346da10be56932f9674f8c250a3ea592) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "dd_jb-2", 0x000000, 0x080000, CRC(2c07c29b) SHA1(26244145139df1ffe2b6ec25a32e5009da6a5aba) )
	ROM_LOAD64_WORD( "dd_jb-4", 0x000002, 0x080000, CRC(fe15538e) SHA1(a52ac04656783611ec5d5af01b18e22254decc0c) )
	ROM_LOAD64_WORD( "dd_jb-1", 0x000004, 0x080000, CRC(e6ca9bf6) SHA1(0379250303eb6895a4dda080da8bf031d055ce8e) )
	ROM_LOAD64_WORD( "dd_jb-3", 0x000006, 0x080000, CRC(6ecbe193) SHA1(33b799699d5d17705df36591cdc40032278388d1) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "dd_ja-7", 0x000000, 0x040000, CRC(7ce817eb) SHA1(9dfb79021a552877fbc26049cca853c0b93735b5) )
ROM_END

ROM_START( dharmak )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x020000, CRC(7dec1f77) SHA1(86cda990392e738f1bacec9d7a232d27887c1135) )
	ROM_LOAD16_BYTE( "6.bin", 0x000001, 0x020000, CRC(a194edbe) SHA1(676a4c0d4ee842a1b9d1c86ecd89417ebd6b5927) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "8.bin", 0x000000, 0x020000, CRC(d0e0a8e2) SHA1(99a3142589a1763ba162ed5b1b6c44961a5aaabc) )   // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter) // note, these are bitswapped, see init
	ROM_LOAD64_WORD( "2.bin", 0x000000, 0x080000, CRC(3cc0bb6c) SHA1(aaa063fa748e0f6fe3c07f2dfb510c1b69ea92af) )
	ROM_LOAD64_WORD( "4.bin", 0x000002, 0x080000, CRC(2cdcdf91) SHA1(44da8eac822a89e9c07bfd28720ec0b566d19b44) )
	ROM_LOAD64_WORD( "1.bin", 0x000004, 0x080000, CRC(312ee2ec) SHA1(73ea401e4615eb9ad5f42be9c75ca4550c3a4668) )
	ROM_LOAD64_WORD( "3.bin", 0x000006, 0x080000, CRC(b81aede8) SHA1(fe11e1523a9bcd59397b5866bc03c9d24049a5f5) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "7.bin", 0x000000, 0x040000, CRC(8af698d7) SHA1(9f8b2ecc07c19f38088cd4be05a498ae4f5af6f5) )
ROM_END



/*

Gun Master
Metro Corp. 1994

PCB Layout
----------

MTR5260-A
|-----------------------------------------------|
|TA7222            3.579545MHz                  |
|            YM3012                      6116   |
|C3403  C3403      YM2151  M6295                |
|       26.666MHz              GMJA-7   GMJA-8  |
|            6264                               |
|J           6264           |--------|  D78C10  |
|A           6264           |IMAGETEK|          |
|M                          |I4220   |          |
|M               MM1035     |        |          |
|A         |------------|   |--------|          |
|          |    68000   |               GMJA-1  |
|          |------------|                       |
|                                       GMJA-2  |
|              24MHz                            |
|       MACH110                         GMJA-3  |
|                  6264                         |
|DSW1              6264                 GMJA-4  |
|DSW2    GMJA-6    GMJA-5                       |
|-----------------------------------------------|
Notes:
      68000 clock     - 12.000MHz [24/2]
      D78C10 clock    - 12.000MHz [24/2]
      YM2151 clock    - 3.579545MHz
      Oki M6295 clock - 1.200MHz [24/20], sample rate = 1200000 / 132
      VSync - 60Hz
      HSync - 15.55kHz

RAM - CY7C199 (x2), 6164 (x2), LH5168 (x2), 6116 (x1)
ROMs 5+6 = Main Prg
ROMs 7+8 = Sound Data
ROMs 1-4 = GFX Data

*/

ROM_START( gunmast )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "gmja-5.20e", 0x000000, 0x040000, CRC(7334b2a3) SHA1(23f0a00b7539329f23eb564bc2823383997f83a9) )
	ROM_LOAD16_BYTE( "gmja-6.20c", 0x000001, 0x040000, CRC(c38d185e) SHA1(fdbc16a6ffc791778cb7ac2dafd15f4eb72c4cf9) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "gmja-8.3i", 0x000000, 0x020000, CRC(ab4bcc56) SHA1(9ef91e14d0974f30c874a12370ddd04ee8ab6d5d) )   // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "gmja-2.14i", 0x000000, 0x080000, CRC(bc9acd54) SHA1(e6154cc5e8e33b38f56a0055dd0a51aa6adc4f9c) )
	ROM_LOAD64_WORD( "gmja-4.18i", 0x000002, 0x080000, CRC(f2d72d90) SHA1(575a01999e4608d1503904ba22310413b680b2b9) )
	ROM_LOAD64_WORD( "gmja-1.12i", 0x000004, 0x080000, CRC(336d0a90) SHA1(39ff59ba13e21f2a8488e5dc2d44cf2c50f7c4fb) )
	ROM_LOAD64_WORD( "gmja-3.16i", 0x000006, 0x080000, CRC(a6651297) SHA1(cdfb8a176cced552a9e72d39980c7fb005edf4f9) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "gmja-7.3g", 0x000000, 0x040000, CRC(3a342312) SHA1(5c31bc9ec5159e1a0c9a931c7b702a31d3a1af10) )
ROM_END



/***************************************************************************

Karate Tournament
Mitchell 1992

Note: This identical PCB with ROM and PAL swap is used by Moeyo Gonta!! (Lady Killer)

VG460-(A)
|----------------------------------------------|
|TA7222       YM2413  KT008  D78C10      KT001 |
|VOLUME UPC3403 3.579545MHz M6295         6116 |
|       UPC3403 *YM2151                        |
|*UPC3403       *YM3012              24MHz     |
|                                              |
|M54532          62256 20MHz             MM1035|
|                62256         460A24  460A21  |
|J  HE-2         |--------|    460A23  460A22  |
|A  HE-2  6264   |IMAGETEK|                    |
|M  HE-2         |I4100   |                    |
|M               |        |  6264 KT002  |---| |
|A               |--------|  6264 KT003  | 6 | |
|                                        | 8 | |
|                                        | 0 | |
|                                        | 0 | |
|SW1                                     | 0 | |
|SW2          361A06    361A04           |---| |
|             361A07    361A05                 |
|----------------------------------------------|
Notes:
           * - Not populated
       68000 - Clock input 12.000MHz [24/2]
      D78C10 - NEC D78C10 8-bit CPU with A/D Converter. Clock input 12.000MHz [24/2]
       M6295 - OKI M6295 4-Channel Mixing ADPCM Voice Synthesis LSI. Clock input 1.200MHz [24/20]. Pin 7 HIGH
      YM2413 - Yamaha YM2413 FM Operator TYPE-LL sound IC. Clock input 3.579545MHz
       I4100 - ImageTek Inc. I4100 052 9227KK702 graphics chip
       KT001 - 27C010 EPROM at location 1I
       KT002 - 27C2001 EPROM at location 8G
       KT003 - 27C2001 EPROM at location 10G
       KT008 - 27C2001 EPROM at location 1D
      361A0* - 42 pin 1M x8-bit (8Mbit) mask ROM
460A24/23/22 - AMI PAL18CV8
      460A21 - AMI PAL22CV10
       SW1/2 - 8-position DIP switch
        HE-2 - Resistor array
     UPC3403 - NEC uPC3403C Quad Operational Amplifier
        6116 - 2k x8-bit SRAM
       62256 - 32k x8-bit SRAM
        6264 - 8k x8-bit SRAM
      M54532 - Mitsubishi M54532P Quad 1.5A Darlington Transistor Array with Clamp Diode
      MM1035 - Mitsumi System Reset IC with Built-in Watchdog Timer (==Fujitsu MB3773)
      TA7222 - Toshiba TA7222 5.8w Audio Power Amplifier
       HSync - 14.9505kHz
       VSync - 57.1556Hz

***************************************************************************/

ROM_START( karatour )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "2.2fab.8g",  0x000000, 0x040000, CRC(199a28d4) SHA1(ae880b5d5a1703c54e0ef27015039c7bb05eb185) )  // Hand-written label "(2) 2FAB"
	ROM_LOAD16_BYTE( "3.0560.10g", 0x000001, 0x040000, CRC(b054e683) SHA1(51e28a99f87684f3e56c7a168523f94717903d79) )  // Hand-written label "(3) 0560"

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "kt001.1i", 0x000000, 0x020000, CRC(1dd2008c) SHA1(488b6f5d15bdbc069ee2cd6d7a0980a228d2f790) )    // 11xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "361a04.15f", 0x000000, 0x100000, CRC(f6bf20a5) SHA1(cb4cb249eb1c106fe7ef0ace735c0cc3106f1ab7) )
	ROM_LOAD64_WORD( "361a07.17d", 0x000002, 0x100000, CRC(794cc1c0) SHA1(ecfdec5874a95846c0fb7966fdd1da625d85531f) )
	ROM_LOAD64_WORD( "361a05.17f", 0x000004, 0x100000, CRC(ea9c11fc) SHA1(176c4419cfe13ff019654a93cd7b0befa238bbc3) )
	ROM_LOAD64_WORD( "361a06.15d", 0x000006, 0x100000, CRC(7e15f058) SHA1(267f0a5acb874d4fff3556ffa405e24724174667) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "8.4a06.1d", 0x000000, 0x040000, CRC(8d208179) SHA1(54a27ef155828435bc5eba60790a8584274c8b4a) )  // Hand-written label "(8) 4A06"
ROM_END

ROM_START( karatourj )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "kt002.8g",  0x000000, 0x040000, CRC(316a97ec) SHA1(4b099d2fa91822c9c85d647aab3d6779fc400250) )
	ROM_LOAD16_BYTE( "kt003.10g", 0x000001, 0x040000, CRC(abe1b991) SHA1(9b6327169d66717dd9dd74816bc33eb208c3763c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "kt001.1i", 0x000000, 0x020000, CRC(1dd2008c) SHA1(488b6f5d15bdbc069ee2cd6d7a0980a228d2f790) )    // 11xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "361a04.15f", 0x000000, 0x100000, CRC(f6bf20a5) SHA1(cb4cb249eb1c106fe7ef0ace735c0cc3106f1ab7) )
	ROM_LOAD64_WORD( "361a07.17d", 0x000002, 0x100000, CRC(794cc1c0) SHA1(ecfdec5874a95846c0fb7966fdd1da625d85531f) )
	ROM_LOAD64_WORD( "361a05.17f", 0x000004, 0x100000, CRC(ea9c11fc) SHA1(176c4419cfe13ff019654a93cd7b0befa238bbc3) )
	ROM_LOAD64_WORD( "361a06.15d", 0x000006, 0x100000, CRC(7e15f058) SHA1(267f0a5acb874d4fff3556ffa405e24724174667) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "kt008.1d", 0x000000, 0x040000, CRC(47cf9fa1) SHA1(88923ace550154c58c066f859cadfa7864c5344c) )
ROM_END

/***************************************************************************

Moeyo Gonta!! (Lady Killer)
(c)1993 Yanyaka
VG460-(B)

CPU  : TMP68HC000P-16
Sound: D78C10ACW YM2413 M6295
OSC  : 3.579545MHz(XTAL1) 20.0000MHz(XTAL2) 24.0000MHz(XTAL3)

ROMs:
e1.1i - Sound program (27c010)

j2.8g  - Main programs (27c020)
j3.10g /

ladyj-4.15f - Graphics (mask, read as 27c800)
ladyj-5.17f |
ladyj-6.15d |
ladyj-7.17d /

e8j.1d - Samples (27c020)

Others:
Imagetek I4100 052 9330EK712

***************************************************************************/

ROM_START( ladykill )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "e2.8g",  0x000000, 0x040000, CRC(211a4865) SHA1(4315c0a708383d357d8dd89a1820fe6cf7652adb) )
	ROM_LOAD16_BYTE( "e3.10g", 0x000001, 0x040000, CRC(581a55ea) SHA1(41bfcaae84e583bf185948ab53ec39c05180a7a4) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "e1.1i", 0x000000, 0x020000, CRC(a4d95cfb) SHA1(2fd8a5cbb0dc289bd5294519dbd5369bfb4c2d4d) )   // 11xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "ladyj-4.15f", 0x000000, 0x100000, CRC(65e5906c) SHA1(cc3918c2094ca819ec4043055564e1dbff4a4750) )
	ROM_LOAD64_WORD( "ladyj-7.17d", 0x000002, 0x100000, CRC(56bd64a5) SHA1(911272078b0fd375111f5d1463945c2075c19e40) )
	ROM_LOAD64_WORD( "ladyj-5.17f", 0x000004, 0x100000, CRC(a81ffaa3) SHA1(5c161b0ef33f1bab077e9a2eb2d3432825729e83) )
	ROM_LOAD64_WORD( "ladyj-6.15d", 0x000006, 0x100000, CRC(3a34913a) SHA1(a55624ede7c368e61555ca7b9cd9e6948265b784) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "e8.1d", 0x000000, 0x040000, CRC(da88244d) SHA1(90c0cc275b69afffd9a0126985fd3fe16d44dced) )
ROM_END

/* an 'Electronic Devices' manufactured board has been seen with the following roms. The data is 100% identical to the above set
   but due to lazy manufacturing larger ROMs were used and the first half filled with 0xff

    ROM_LOAD16_BYTE( "ladyki_3.h9",   0x000000, 0x080000, CRC(c658f954) SHA1(d50043457e67a94feff1328fe9bf522aa3c124b6) ) // == e2.8g
    ROM_LOAD16_BYTE( "ladyki_2.h10",  0x000001, 0x080000, CRC(bf58e4db) SHA1(9d7f74dc348b0ccb3bcf1b618d6092292b6945b8) ) // == e3.10g
    ROM_LOAD( "ladyki_1.d1", 0x000000, 0x080000, CRC(3dca957c) SHA1(4b815b7cb124a38c639a4b425ed6e8b1f0946451) ) // == e8.1d
*/

ROM_START( moegonta )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "j2.8g",  0x000000, 0x040000, CRC(aa18d130) SHA1(6e0fd3b95d8589665b418bcae4fe64b288289c78) )
	ROM_LOAD16_BYTE( "j3.10g", 0x000001, 0x040000, CRC(b555e6ab) SHA1(adfc6eafec612c8770b9f832a0a2574c53c3d047) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "e1.1i", 0x000000, 0x020000, CRC(a4d95cfb) SHA1(2fd8a5cbb0dc289bd5294519dbd5369bfb4c2d4d) )   // 11xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "ladyj-4.15f", 0x000000, 0x100000, CRC(65e5906c) SHA1(cc3918c2094ca819ec4043055564e1dbff4a4750) )
	ROM_LOAD64_WORD( "ladyj-7.17d", 0x000002, 0x100000, CRC(56bd64a5) SHA1(911272078b0fd375111f5d1463945c2075c19e40) )
	ROM_LOAD64_WORD( "ladyj-5.17f", 0x000004, 0x100000, CRC(a81ffaa3) SHA1(5c161b0ef33f1bab077e9a2eb2d3432825729e83) )
	ROM_LOAD64_WORD( "ladyj-6.15d", 0x000006, 0x100000, CRC(3a34913a) SHA1(a55624ede7c368e61555ca7b9cd9e6948265b784) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "e8j.1d", 0x000000, 0x040000, CRC(f66c2a80) SHA1(d95ddc8fe4144a6ad4a92385ff962d0b9391d53b) )
ROM_END


/***************************************************************************

Last Fortress - Toride
Metro 1992
VG420

                                     TR_JB12 5216
                     SW2 SW1           NEC78C10   3.579MHz

                                                          6269
                                                          TR_JB11
  55328 55328 55328       24MHz

                           4064   4064   TR_   TR_          68000-12
       Imagetek                          JC10  JC09
       I4100

    TR_  TR_  TR_  TR_  TR_  TR_  TR_  TR_
    JC08 JC07 JC06 JC05 JC04 JC03 JC02 JC01

CPU     :MC68000P12
Sound   :Yamaha YM2413, OKI M6295
OSC     :24.0000MHz, 3.579545MHz
other   :D78C10ACW, Imagetek Inc I4100 052

Clock measurements by the Guru:
Master clock: 24.00MHz
 D7810 clock: 12.00MHz (24 / 2)
 M6295 clock: 1.200MHz (24 / 20), sample rate =  M6295 clock /165
YM2413 clock: 3.579545MHz

Vsync: 58Hz
HSync: 15.16kHz

***************************************************************************/

ROM_START( lastfort ) // Japanese version on PCB VG420
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tr_jc09", 0x000000, 0x020000, CRC(8b98a49a) SHA1(15adca78d54973820d04f8b308dc58d0784eb900) )
	ROM_LOAD16_BYTE( "tr_jc10", 0x000001, 0x020000, CRC(8d04da04) SHA1(5c7e65a39929e94d1fa99aeb5fed7030b110451f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr_jb12", 0x000000, 0x020000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x100000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "tr_jc02", 0x000000, 0x020000, CRC(db3c5b79) SHA1(337f4c547a6267f317415cbc78cdac41574b1024) )
	ROM_LOAD64_BYTE( "tr_jc04", 0x000001, 0x020000, CRC(f8ab2f9b) SHA1(bfbbd5ec2bc039b8eaef92467c2e7fd3b425b477) )
	ROM_LOAD64_BYTE( "tr_jc06", 0x000002, 0x020000, CRC(47a7f397) SHA1(1d2b11b95ce81ca66713457283464d6d85753e4b) )
	ROM_LOAD64_BYTE( "tr_jc08", 0x000003, 0x020000, CRC(d7ba5e26) SHA1(294fd9b68eebd28ca64627f0d6e64b325cab18a0) )
	ROM_LOAD64_BYTE( "tr_jc01", 0x000004, 0x020000, CRC(3e3dab03) SHA1(e3c6eb73467f0ed207657084e51ee87d85152c3f) )
	ROM_LOAD64_BYTE( "tr_jc03", 0x000005, 0x020000, CRC(87ac046f) SHA1(6555a55642383990bc7a8282ab5ea8fc0ba6cd14) )
	ROM_LOAD64_BYTE( "tr_jc05", 0x000006, 0x020000, CRC(3fbbe49c) SHA1(642631e69d78898403013884cf0fb711ea000541) )
	ROM_LOAD64_BYTE( "tr_jc07", 0x000007, 0x020000, CRC(05e1456b) SHA1(51cd3ad2aa9c0adc7b9d63a337b247b4b65701ca) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END

ROM_START( lastfortk )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "7f-9",  0x000000, 0x020000, CRC(d2894c1f) SHA1(4f4ab6d8ce69999cd7c4a9ddabec8d1e8fefc6fc) )
	ROM_LOAD16_BYTE( "8f-10", 0x000001, 0x020000, CRC(9696ea39) SHA1(27af0c6399cd7be40aa8a1c1b58e0db8408aff11) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr_jb12", 0x000000, 0x020000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x200000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "7i-2",  0x000000, 0x040000, CRC(d1fe8d7b) SHA1(88b1973ebb47b91a49f6b4f722c9cc33e5330694) )
	ROM_LOAD64_BYTE( "10i-4", 0x000001, 0x040000, CRC(058126d4) SHA1(985177556c8545e6a65a41083246b31509de7130) )
	ROM_LOAD64_BYTE( "13i-6", 0x000002, 0x040000, CRC(39a9dea2) SHA1(9f8067cff15be93771d42b3776ee7ca1b7c61798) )
	ROM_LOAD64_BYTE( "16i-8", 0x000003, 0x040000, CRC(4c050baa) SHA1(3e0b2029d7c0b6cd32b22f147663cd22975ce8c3) )
	ROM_LOAD64_BYTE( "5i-1",  0x000004, 0x040000, CRC(0d503f05) SHA1(0b1ce22630bb2326930f0f3b5710c6c191730c45) )
	ROM_LOAD64_BYTE( "8i-3",  0x000005, 0x040000, CRC(b6d4f753) SHA1(2864ad5fe4186e4e15bb7d5dafa6a9b8c803d7d0) )
	ROM_LOAD64_BYTE( "12i-5", 0x000006, 0x040000, CRC(ce69c805) SHA1(88debdbd8e73da54c1c25a1a60f27a05dac3f104) )
	ROM_LOAD64_BYTE( "14i-7", 0x000007, 0x040000, CRC(0cb38317) SHA1(6e18096f6616aa0d9c4f3a2394561ed3f636731e) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END

ROM_START( lastfortj ) // Japanese version on PCB VG460-(A)
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tr_mja2.8g",  0x000000, 0x020000, CRC(4059a8c8) SHA1(05f271fb86a01359b1737bbfdf3c0a83364dd7d3) )
	ROM_LOAD16_BYTE( "tr_mja3.10g", 0x000001, 0x020000, CRC(8fc6ddcd) SHA1(626070094fbbd982e1c8d699f171a1c500db1620) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr_ma01.1i",  0x000000, 0x020000,  CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // Same as parent set, but different label

	ROM_REGION( 0x200000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "tr_ma04.15f", 0x000000, 0x080000, CRC(5feafc6f) SHA1(eb50905eb0d25eb342e08d591907f79b5eadff43) )
	ROM_LOAD64_WORD( "tr_ma07.17d", 0x000002, 0x080000, CRC(7519d569) SHA1(c88932a19a48d45a19b777113a4719b18f42a297) )
	ROM_LOAD64_WORD( "tr_ma05.17f", 0x000004, 0x080000, CRC(5d917ba5) SHA1(34fc72924fa2877c1038d7f61b22f7667af01e9f) )
	ROM_LOAD64_WORD( "tr_ma06.15d", 0x000006, 0x080000, CRC(d366c04e) SHA1(e0a67688043cb45916860d32ff1076d9257e6ad9) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr_ma08.1d", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) ) // Same as parent set, but different label
ROM_END

ROM_START( lastfortg ) // German version on PCB VG460-(A)
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tr_ma02.8g",  0x000000, 0x020000, CRC(e6f40918) SHA1(c8c9369103530b2214c779c8a643ba9349b3eac5) )
	ROM_LOAD16_BYTE( "tr_ma03.10g", 0x000001, 0x020000, CRC(b00fb126) SHA1(7dd4b7a2d1c5401fde2275ef76fac1ccc586a0bd) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr_ma01.1i",  0x000000, 0x020000,  CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // Same as parent set, but different label

	ROM_REGION( 0x200000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "tr_ma04.15f", 0x000000, 0x080000, CRC(5feafc6f) SHA1(eb50905eb0d25eb342e08d591907f79b5eadff43) )
	ROM_LOAD64_WORD( "tr_ma07.17d", 0x000002, 0x080000, CRC(7519d569) SHA1(c88932a19a48d45a19b777113a4719b18f42a297) )
	ROM_LOAD64_WORD( "tr_ma05.17f", 0x000004, 0x080000, CRC(5d917ba5) SHA1(34fc72924fa2877c1038d7f61b22f7667af01e9f) )
	ROM_LOAD64_WORD( "tr_ma06.15d", 0x000006, 0x080000, CRC(d366c04e) SHA1(e0a67688043cb45916860d32ff1076d9257e6ad9) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr_ma08.1d", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) ) // Same as parent set, but different label
ROM_END

/***************************************************************************

Last Fortress - Toride (Erotic)
Metro Corporation.

Board number VG420

CPU: MC68000P12
SND: OKI M6295+ YM2413 + NEC D78C10ACW + NEC D4016 (ram?)
DSW: see manual (scanned in sub-directory Manual)
OSC: 24.000 MHz, 3.579545MHz

***************************************************************************/

ROM_START( lastforte )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tr_hc09", 0x000000, 0x020000, CRC(32f43390) SHA1(b5bad9d80f2155f277265fe487a59f0f4ec6575d) )
	ROM_LOAD16_BYTE( "tr_hc10", 0x000001, 0x020000, CRC(9536369c) SHA1(39291e92c107be35d130ff29533b42581efc308b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr_jb12", 0x000000, 0x020000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x100000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "tr_ha02", 0x000000, 0x020000, CRC(11cfbc84) SHA1(fb7005be7678564713b5480569f2cdab6c36f029) )
	ROM_LOAD64_BYTE( "tr_ha04", 0x000001, 0x020000, CRC(32bf9c26) SHA1(9d16eca8810d1823726dc9c047504bd24f2a55f7) )
	ROM_LOAD64_BYTE( "tr_ha06", 0x000002, 0x020000, CRC(16937977) SHA1(768bb6b1c9b90b2eedc9dbb19c8e9fa8f4265f17) )
	ROM_LOAD64_BYTE( "tr_ha08", 0x000003, 0x020000, CRC(6dd96a9b) SHA1(fe8214d57dc83157eff53f2d83bd3a4e2da91555) )
	ROM_LOAD64_BYTE( "tr_ha01", 0x000004, 0x020000, CRC(aceb44b3) SHA1(9a236eddbc916c206bfa694b576d971d788e8eb1) )
	ROM_LOAD64_BYTE( "tr_ha03", 0x000005, 0x020000, CRC(f18f1248) SHA1(30e39d904368c61a46719a0f21a6acb7fa55593f) )
	ROM_LOAD64_BYTE( "tr_ha05", 0x000006, 0x020000, CRC(79f769dd) SHA1(7a9ff8e961ae09fdf36a0a751befc141f47c9fd8) )
	ROM_LOAD64_BYTE( "tr_ha07", 0x000007, 0x020000, CRC(b6feacb2) SHA1(85df28d5ff6601753a435e31bcaf45702c7489ea) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END

ROM_START( lastfortea )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tr_ha09", 0x000000, 0x020000, CRC(61fe8fb2) SHA1(d3f33bbc5326f89407fe1f4e389af7510ce134a0) )
	ROM_LOAD16_BYTE( "tr_ha10", 0x000001, 0x020000, CRC(14a9fba2) SHA1(984247397f204b9e1bdf69e68299b2e061fba5b1) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr_jb12", 0x000000, 0x020000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x100000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "tr_ha02", 0x000000, 0x020000, CRC(11cfbc84) SHA1(fb7005be7678564713b5480569f2cdab6c36f029) )
	ROM_LOAD64_BYTE( "tr_ha04", 0x000001, 0x020000, CRC(32bf9c26) SHA1(9d16eca8810d1823726dc9c047504bd24f2a55f7) )
	ROM_LOAD64_BYTE( "tr_ha06", 0x000002, 0x020000, CRC(16937977) SHA1(768bb6b1c9b90b2eedc9dbb19c8e9fa8f4265f17) )
	ROM_LOAD64_BYTE( "tr_ha08", 0x000003, 0x020000, CRC(6dd96a9b) SHA1(fe8214d57dc83157eff53f2d83bd3a4e2da91555) )
	ROM_LOAD64_BYTE( "tr_ha01", 0x000004, 0x020000, CRC(aceb44b3) SHA1(9a236eddbc916c206bfa694b576d971d788e8eb1) )
	ROM_LOAD64_BYTE( "tr_ha03", 0x000005, 0x020000, CRC(f18f1248) SHA1(30e39d904368c61a46719a0f21a6acb7fa55593f) )
	ROM_LOAD64_BYTE( "tr_ha05", 0x000006, 0x020000, CRC(79f769dd) SHA1(7a9ff8e961ae09fdf36a0a751befc141f47c9fd8) )
	ROM_LOAD64_BYTE( "tr_ha07", 0x000007, 0x020000, CRC(b6feacb2) SHA1(85df28d5ff6601753a435e31bcaf45702c7489ea) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END


/***************************************************************************

Mahjong Doukyuusei (JPN Ver.)

(c)1995 make software/elf/media trading corp.

Board: VG330-B

CPU   : 68000 16MHz
Sound : YM2413, M6295
OSC   : 16.0000MHz 3.579545MHz 26.666MHz
Custom: Imagetek Inc I4300

***************************************************************************/

ROM_START( dokyusei )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "330_a06.bin", 0x000000, 0x020000, CRC(36157c2e) SHA1(f855175143caf476dcbee5a8aaec802a8fdb64fa) )
	ROM_LOAD16_BYTE( "330_a05.bin", 0x000001, 0x020000, CRC(177f50d2) SHA1(2298411152553041b907d9243aaa7983ca21c946) )

	ROM_REGION( 0x800000, "vdp3", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "2.bin", 0x000000, 0x200000, CRC(075bface) SHA1(7f0e47ebdc37a1fc09b072cb8e0f38258a702a3d) )
	ROM_LOAD64_WORD( "4.bin", 0x000002, 0x200000, CRC(bc631438) SHA1(da3ef24d94e69197e3c69e4fd2b716162c275278) )
	ROM_LOAD64_WORD( "1.bin", 0x000004, 0x200000, CRC(4566c29b) SHA1(3216e21d898855cbb0ad328e6d45f3726d95b099) )
	ROM_LOAD64_WORD( "3.bin", 0x000006, 0x200000, CRC(5f6d7969) SHA1(bcb48c5808f268ca35a28f162d4e9da9df65b843) )

	ROM_REGION( 0x100000, "oki", 0 )    // Samples
	ROM_LOAD( "7.bin", 0x000000, 0x100000, CRC(c572aee1) SHA1(2a3baf962617577f8ac3f9e58fb4e5a0dae4f0e8) )   // 4 x 0x40000
ROM_END


/***************************************************************************

Mahjong Doukyuusei Special
(c)1995 Make Software / Elf / Media Trading

Board:  VG340-A

CPU:    68000-16
Sound:  M6295
        YM2413
OSC:    32.0000MHz
        3.579545MHz
EEPROM: 93C46
Custom: Imagetek Inc I4300 095

***************************************************************************/

ROM_START( dokyusp )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x020000, CRC(01729b7f) SHA1(42a60f034ee5d5c2a42856b97d0d4c499b24627b) )
	ROM_LOAD16_BYTE( "5.bin", 0x000001, 0x020000, CRC(57770776) SHA1(15093886f2fe49443e8d7541903714de0a14aa0b) )

	ROM_REGION( 0x1000000, "vdp3", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "2l.bin", 0x0000000, 0x400000, CRC(4bed184d) SHA1(12bdb00030d19c2c9fb2120ed6b267a7982c213a) )
	ROM_LOAD64_WORD( "4l.bin", 0x0000002, 0x400000, CRC(2ee468e6) SHA1(ced58fdd8b5c99ce3f09cece2e05d7fcf4c7f786) )
	ROM_LOAD64_WORD( "1l.bin", 0x0000004, 0x400000, CRC(510ace14) SHA1(f5f1f46f4d8d150dd9e17083f32e9b45938c1dad) )
	ROM_LOAD64_WORD( "3l.bin", 0x0000006, 0x400000, CRC(82ea562e) SHA1(42839de9f346ccd0736bdbd3eead61ad66fcb666) )

	ROM_REGION( 0x200000, "oki", 0 )    // Samples
	ROM_LOAD( "7.bin", 0x000000, 0x200000, CRC(763985e1) SHA1(395d925b79922de5060a3f59de99fbcc9bd40fad) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-dokyusp.bin", 0x0000, 0x0080, CRC(cf159485) SHA1(f8e9c89e1b7c8bcd77ae5f55e334f79285f602a8) )
ROM_END


/***************************************************************************

Mahjong Gakuensai (JPN Ver.)
(c)1997 Make Software

Board:  VG340-A

CPU:    68000-16
Sound:  M6295
        YM2413
OSC:    26.6660MHz
        3.5795MHz

Custom: I4300 095

***************************************************************************/

ROM_START( gakusai )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "6.u38", 0x000000, 0x040000, CRC(6f8ab082) SHA1(18caf49a0c65f831d375f089f27b8570b094f029) )
	ROM_LOAD16_BYTE( "5.u39", 0x000001, 0x040000, CRC(010176c4) SHA1(48fcea18c02c1426a699a636f44b21cf7625e8a0) )

	ROM_REGION( 0x2000000, "vdp3", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "2l.u26", 0x0000000, 0x400000, CRC(45dfb5c7) SHA1(04338d695bd6973fd7d7286a8da563250ae4f71b) )
	ROM_LOAD64_WORD( "4l.u27", 0x0000002, 0x400000, CRC(7ab64f49) SHA1(e4d9a7bf97635b41fe632b3542eee1f609db080a) )
	ROM_LOAD64_WORD( "1l.u24", 0x0000004, 0x400000, CRC(75093421) SHA1(cfe549e24abfedd740ead30cab235df494e9f45d) )
	ROM_LOAD64_WORD( "3l.u25", 0x0000006, 0x400000, CRC(4dcfcd98) SHA1(bfb882d99c854e68e86f4e8f8aa7d02dcf5e9cfc) )
	ROM_LOAD64_WORD( "2u.u21", 0x1000000, 0x400000, CRC(8d4f912b) SHA1(1fcf1dd50fd678cc908ab47bcccaa4ed7b2b6938) )
	ROM_LOAD64_WORD( "4u.u20", 0x1000002, 0x400000, CRC(1f83e98a) SHA1(10b2d3ceb4bda6a2ecf795b865c948563c2fb84d) )
	ROM_LOAD64_WORD( "1u.u23", 0x1000004, 0x400000, CRC(28b386d9) SHA1(d1e151fa112c86d2cb97b7a5439a1e549359055d) )
	ROM_LOAD64_WORD( "3u.u22", 0x1000006, 0x400000, CRC(87f3c5e6) SHA1(097c0a53b040399d928f17fe3e9f42755b1d72f3) )

	ROM_REGION( 0x400000, "oki", 0 )    // Samples
	ROM_LOAD( "7.u11", 0x000000, 0x400000, CRC(34575a14) SHA1(53d458513f208f07844e1727d5889e85dcd4f0ed) )
ROM_END

ROM_START( gakusaia )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "6.u38", 0x000000, 0x040000, CRC(16155d87) SHA1(8fc44f9a9c03b211edc204deba1aa4ac9d455eeb) )
	ROM_LOAD16_BYTE( "5.u39", 0x000001, 0x040000, CRC(2564acb8) SHA1(fe526594c7fa2eba05eeb3c58081244c8966588f) )

	ROM_REGION( 0x2000000, "vdp3", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "2l.u26", 0x0000000, 0x400000, CRC(45dfb5c7) SHA1(04338d695bd6973fd7d7286a8da563250ae4f71b) )
	ROM_LOAD64_WORD( "4l.u27", 0x0000002, 0x400000, CRC(7ab64f49) SHA1(e4d9a7bf97635b41fe632b3542eee1f609db080a) )
	ROM_LOAD64_WORD( "1l.u24", 0x0000004, 0x400000, CRC(75093421) SHA1(cfe549e24abfedd740ead30cab235df494e9f45d) )
	ROM_LOAD64_WORD( "3l.u25", 0x0000006, 0x400000, CRC(4dcfcd98) SHA1(bfb882d99c854e68e86f4e8f8aa7d02dcf5e9cfc) )
	ROM_LOAD64_WORD( "2u.u21", 0x1000000, 0x400000, CRC(8d4f912b) SHA1(1fcf1dd50fd678cc908ab47bcccaa4ed7b2b6938) )
	ROM_LOAD64_WORD( "4u.u20", 0x1000002, 0x400000, CRC(1f83e98a) SHA1(10b2d3ceb4bda6a2ecf795b865c948563c2fb84d) )
	ROM_LOAD64_WORD( "1u.u23", 0x1000004, 0x400000, CRC(28b386d9) SHA1(d1e151fa112c86d2cb97b7a5439a1e549359055d) )
	ROM_LOAD64_WORD( "3u.u22", 0x1000006, 0x400000, CRC(87f3c5e6) SHA1(097c0a53b040399d928f17fe3e9f42755b1d72f3) )

	ROM_REGION( 0x400000, "oki", 0 )    // Samples
	ROM_LOAD( "7.u11", 0x000000, 0x400000, CRC(34575a14) SHA1(53d458513f208f07844e1727d5889e85dcd4f0ed) )
ROM_END


/***************************************************************************

Mahjong Gakuensai 2 (JPN Ver.)
(c)1998 Make Software

Board:  VG340-A

CPU:    68000-16
Sound:  M6295
        YM2413
OSC:    26.6660MHz
        3.579545MHz

Custom: I4300 095

***************************************************************************/

ROM_START( gakusai2 )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "mg2a06.bin", 0x000000, 0x020000, CRC(8b006dd4) SHA1(893ec0e7c367d79bc99e65ab8abd0d290f2ede58) )
	ROM_LOAD16_BYTE( "mg2a05.bin", 0x000001, 0x020000, CRC(7702b9ac) SHA1(09d0c11fa2c9ed9cde365cb1ff215d55e39b7734) )

	ROM_REGION( 0x2000000, "vdp3", 0 )  // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "mg22l.bin", 0x0000000, 0x400000, CRC(28366708) SHA1(56fccee126916cc301678a205dfe629efefb79db) )
	ROM_LOAD64_WORD( "mg24l.bin", 0x0000002, 0x400000, CRC(9e003bb0) SHA1(aa73cc0e79732fd6826c89671b179cb3189571e0) )
	ROM_LOAD64_WORD( "mg21l.bin", 0x0000004, 0x400000, CRC(3827098d) SHA1(dda9fb6c56c4408802d54c5975fb9470ca2e1d34) )
	ROM_LOAD64_WORD( "mg23l.bin", 0x0000006, 0x400000, CRC(a6f96961) SHA1(dd2578da5d091991580a2c7a979ba8dbfa0cceb3) )
	ROM_LOAD64_WORD( "mg22u.bin", 0x1000000, 0x400000, CRC(53ffa68a) SHA1(3d8d69c2063c78bd79cdbd7457bca1af9700bf3c) )
	ROM_LOAD64_WORD( "mg24u.bin", 0x1000002, 0x400000, CRC(c218e9ab) SHA1(3b6ee4cc828198b284ac9020e2da911efc90725a) )
	ROM_LOAD64_WORD( "mg21u.bin", 0x1000004, 0x400000, CRC(385495e5) SHA1(5181e279fef23780d07ab5a124618e4d0e5cb821) )
	ROM_LOAD64_WORD( "mg23u.bin", 0x1000006, 0x400000, CRC(d8315923) SHA1(6bb5cad317f7efa6a384f6c257c5faeb789a8eed) )

	ROM_REGION( 0x400000, "oki", 0 )    // Samples
	ROM_LOAD( "mg2-7.bin", 0x000000, 0x400000, CRC(2f1c041e) SHA1(a72720b3d7f816e23452775f2fd4223cf2d02985) )
ROM_END


/***************************************************************************

Mouja (JPN Ver.)
(c)1996 Etona / (c)1995 FPS/FWS

VG410-B
+------------------------+
|       SW2 SW1 68000-16 |
|       SW4 SW3   10   9 |
|             62256 62256|
|J YM2413 ALTERA         |
|A  3.579545MHz          |
|M   M6295 16MHz LH53882B|
|M LH538711              |
|A 26.666MHz     LH53882C|
|        I4300           |
|  61S256        LH53882D|
|  61S256 61C64          |
|                LH53882E|
+------------------------+

CPU     :TMP68H000P-16
Sound   :Yamaha YM2413, OKI M6295
OSC     :16000.00KHz, 3.579545MHz, 26.666MHz
other   :Imagetek Inc I4300 095, ALTERA EPM7032LC44-15T

* SW3 & SW4 are unpopulated

9, 10 Program roms, 27C020

LH53882B - LH53882E are MASK roms
LH53711 is a MASK rom

***************************************************************************/

ROM_START( mouja )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "10.u38", 0x000000, 0x040000, CRC(f9742b69) SHA1(f8c6508b227403a82413ceeb0651922759d7e0f4) ) // Silkscreened U38 and 10
	ROM_LOAD16_BYTE( "9.u39",  0x000001, 0x040000, CRC(dc77216f) SHA1(3b73d29f4e8e385f45f2abfb38eaffc2d8406948) ) // Silkscreened U39 and 9

	ROM_REGION( 0x400000, "vdp3", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "lh53882c.u6", 0x000000, 0x100000, CRC(c4dd3194) SHA1(c9c88a8d2046224957b35de14763aa4bdf0d407f) ) // Silkscreened U6 and 42
	ROM_LOAD64_WORD( "lh53882e.u5", 0x000002, 0x100000, CRC(09530f9d) SHA1(03f2ec5ea694266808d245abe7f688de0ef6d853) ) // Silkscreened U5 and 86
	ROM_LOAD64_WORD( "lh53882b.u4", 0x000004, 0x100000, CRC(5dd7a7b2) SHA1(b0347e8951b29356a7d945b906d93c40b9abc19c) ) // Silkscreened U4 and 31
	ROM_LOAD64_WORD( "lh53882d.u1", 0x000006, 0x100000, CRC(430c3925) SHA1(41e5bd02a665eee87ef8f4ae9f4bee374c25e00b) ) // Silkscreened U1 and 75

	ROM_REGION( 0x100000, "oki", 0 )  // Samples
	ROM_LOAD( "lh538711.u53",     0x000000, 0x100000, CRC(fe3df432) SHA1(4fb7ad997ca6e91468d7516e5c4a94cde6e07104) ) // Silkscreened U53 and 11
ROM_END


/***************************************************************************

Mouse Shooter GoGO
Metro 1995

+--------------------------------------------+
|                 MS_WA-7            MS_JA-1 |
|                 YRW801-M           MS_WA-2 |
|                                    MS_WA-3 |
|J      33.369MHz YMF278B            MS_WA-4 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|         MS_WA-6 MS_WA-5     I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199  61C64 |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T D9443
Video: Imagetek I4220 071 9430WK440

SW3 - Not Populated

ms_ja-1.1    tms27c240 <-- Is there an undumped MS_WA1 World rom??
ms_wa-2.2    tms27c240
ms_wa-3.3    tms27c240
ms_wa-4.4    tms27c240
ms_wa-5.5    tms27c020
ms_wa-6.6    tms27c020
ms_wa-7.7    hn27c4001g

***************************************************************************/

ROM_START( msgogo )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ms_wa-6.6", 0x000000, 0x040000, CRC(986acac8) SHA1(97c24f5b730aa811951db4c7e9c894c0701c58fd) ) // Silkscreened 6 and U18
	ROM_LOAD16_BYTE( "ms_wa-5.5", 0x000001, 0x040000, CRC(746d9f99) SHA1(6e3e34dfb67fecc93213fe040465eccd88575822) ) // Silkscreened 5 and U19

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "ms_wa-2.2", 0x000000, 0x080000, CRC(0d36c2b9) SHA1(3fd6631ad657c73e7e6bfdff9d9caf5ab044bdeb) ) // Silkscreened 2 and U30
	ROM_LOAD64_WORD( "ms_wa-4.4", 0x000002, 0x080000, CRC(fd387126) SHA1(a2f82a66b098a97d8f245e3c2f96c31c63642fec) ) // Silkscreened 4 and U29
	ROM_LOAD64_WORD( "ms_ja-1.1", 0x000004, 0x080000, CRC(8ec4e81d) SHA1(46947ad2941af154f91e47acee281302a12e3aa5) ) // Silkscreened 1 and U28
	ROM_LOAD64_WORD( "ms_wa-3.3", 0x000006, 0x080000, CRC(06cb6807) SHA1(d7303b4047983117cd33e057b1f4b98ed3f7dd32) ) // Silkscreened 3 and U27

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",  0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) // Silkscreened U52
	ROM_LOAD( "ms_wa-7.7", 0x200000, 0x080000, CRC(e19941cb) SHA1(93777c9cd22ddd33d9584b6edad33b95c1e28bde) ) // Silkscreened 7 and U49
ROM_END


/***************************************************************************

Pang Pom's (c) 1992 Metro

Pcb code:  VG420 (Same as Toride)

Cpus:  M68000, Z80
Clocks: 24 MHz, 3.579 MHz
Sound: M6295, YM2413, _unused_ slot for a YM2151

Custom graphics chip - Imagetek I4100 052 9227KK701 (same as Karate Tournament)

***************************************************************************/

ROM_START( pangpoms )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ppoms09.9.f7",  0x000000, 0x020000, CRC(0c292dbc) SHA1(8b09de2a560e804e0dea514c95b317c2e2b6501d) )
	ROM_LOAD16_BYTE( "ppoms10.10.f8", 0x000001, 0x020000, CRC(0bc18853) SHA1(68d50ad50caad34e72d32e7b9fea1d85af74b879) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pj_a12.12.a7", 0x000000, 0x020000, CRC(a749357b) SHA1(1555f565c301c5be7c49fc44a004b5c0cb3777c6) )

	ROM_REGION( 0x100000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "pj_e_02.i7",  0x000000, 0x020000, CRC(88f902f7) SHA1(12ea58d7c000b629ccdceec3dedc2747a63b84be) )
	ROM_LOAD64_BYTE( "ppoms04.bin", 0x000001, 0x020000, CRC(9190c2a0) SHA1(a7399cc2dea5a963e7c930e426915e8eb3552213) )
	ROM_LOAD64_BYTE( "ppoms06.bin", 0x000002, 0x020000, CRC(ed15c93d) SHA1(95072e7d1def0d8e97946a612b90ce078c64aed2) )
	ROM_LOAD64_BYTE( "pj_e_08.i16", 0x000003, 0x020000, CRC(9a3408b9) SHA1(924b184d3a47bbe8aa5d41761ea5e94ba7e4f2e9) )
	ROM_LOAD64_BYTE( "pj_e_01.i6",  0x000004, 0x020000, CRC(11ac3810) SHA1(6ada82a73d4383f99f5be67369b810a692d27ef9) )
	ROM_LOAD64_BYTE( "ppoms03.bin", 0x000005, 0x020000, CRC(e595529e) SHA1(91b4bd1f029ce09d7689815099b38916fe0d2686) )
	ROM_LOAD64_BYTE( "ppoms05.bin", 0x000006, 0x020000, CRC(02226214) SHA1(82302e7f1e7269c45e11dfba45ec7bbf522b47f1) )
	ROM_LOAD64_BYTE( "pj_e_07.i14", 0x000007, 0x020000, CRC(48471c87) SHA1(025fa79993788a0091c4edb83423725abd3a47a2) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "pj_a11.11.e1", 0x000000, 0x020000, CRC(e89bd565) SHA1(6c7c1ad67ba708dbbe9654c1d290af290207d2be) )
ROM_END

ROM_START( pangpomsm )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pa_c_09.9.f7",  0x000000, 0x020000, CRC(e01a7a08) SHA1(1890b290dfb1521ab73b2392409aaf44b99d63bb) )
	ROM_LOAD16_BYTE( "pa_c_10.10.f8", 0x000001, 0x020000, CRC(5e509cee) SHA1(821cfbf5f65cc3091eb8008310266f9f2c838072) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pj_a12.12.a7", 0x000000, 0x020000, CRC(a749357b) SHA1(1555f565c301c5be7c49fc44a004b5c0cb3777c6) )

	ROM_REGION( 0x100000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "pj_e_02.i7",  0x000000, 0x020000, CRC(88f902f7) SHA1(12ea58d7c000b629ccdceec3dedc2747a63b84be) )
	ROM_LOAD64_BYTE( "pj_e_04.i10", 0x000001, 0x020000, CRC(54bf2f10) SHA1(2f0f18984e336f226457295d375a73bcf86cef31) )
	ROM_LOAD64_BYTE( "pj_e_06.i13", 0x000002, 0x020000, CRC(c8b6347d) SHA1(7090e44dc7032432795b6fb6bc166bf4de159685) )
	ROM_LOAD64_BYTE( "pj_e_08.i16", 0x000003, 0x020000, CRC(9a3408b9) SHA1(924b184d3a47bbe8aa5d41761ea5e94ba7e4f2e9) )
	ROM_LOAD64_BYTE( "pj_e_01.i6",  0x000004, 0x020000, CRC(11ac3810) SHA1(6ada82a73d4383f99f5be67369b810a692d27ef9) )
	ROM_LOAD64_BYTE( "pj_e_03.i9",  0x000005, 0x020000, CRC(d126e774) SHA1(f782d1e1277956f088dc91dec8f338f85b9af13a) )
	ROM_LOAD64_BYTE( "pj_e_05.i12", 0x000006, 0x020000, CRC(79c0ec1e) SHA1(b15582e89d859dda4f82908c62e9e07cb45229b9) )
	ROM_LOAD64_BYTE( "pj_e_07.i14", 0x000007, 0x020000, CRC(48471c87) SHA1(025fa79993788a0091c4edb83423725abd3a47a2) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "pj_a11.11.e1", 0x000000, 0x020000, CRC(e89bd565) SHA1(6c7c1ad67ba708dbbe9654c1d290af290207d2be) )
ROM_END

ROM_START( pangpomsn )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pn_e_09.9.f7",  0x000000, 0x020000, CRC(2cc925aa) SHA1(27a09b4b990a867c624207474cb8c55f7d72ce88) )
	ROM_LOAD16_BYTE( "pn_e_10.10.f8", 0x000001, 0x020000, CRC(6d7ad1d2) SHA1(4b6f83f90631fa3eac4d6a3d3ab44760be821f54) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pj_a12.12.a7", 0x000000, 0x020000, CRC(a749357b) SHA1(1555f565c301c5be7c49fc44a004b5c0cb3777c6) )

	ROM_REGION( 0x100000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "pj_e_02.i7",  0x000000, 0x020000, CRC(88f902f7) SHA1(12ea58d7c000b629ccdceec3dedc2747a63b84be) )
	ROM_LOAD64_BYTE( "pj_e_04.i10", 0x000001, 0x020000, CRC(54bf2f10) SHA1(2f0f18984e336f226457295d375a73bcf86cef31) )
	ROM_LOAD64_BYTE( "pj_e_06.i13", 0x000002, 0x020000, CRC(c8b6347d) SHA1(7090e44dc7032432795b6fb6bc166bf4de159685) )
	ROM_LOAD64_BYTE( "pj_e_08.i16", 0x000003, 0x020000, CRC(9a3408b9) SHA1(924b184d3a47bbe8aa5d41761ea5e94ba7e4f2e9) )
	ROM_LOAD64_BYTE( "pj_e_01.i6",  0x000004, 0x020000, CRC(11ac3810) SHA1(6ada82a73d4383f99f5be67369b810a692d27ef9) )
	ROM_LOAD64_BYTE( "pj_e_03.i9",  0x000005, 0x020000, CRC(d126e774) SHA1(f782d1e1277956f088dc91dec8f338f85b9af13a) )
	ROM_LOAD64_BYTE( "pj_e_05.i12", 0x000006, 0x020000, CRC(79c0ec1e) SHA1(b15582e89d859dda4f82908c62e9e07cb45229b9) )
	ROM_LOAD64_BYTE( "pj_e_07.i14", 0x000007, 0x020000, CRC(48471c87) SHA1(025fa79993788a0091c4edb83423725abd3a47a2) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "pj_a11.11.e1", 0x000000, 0x020000, CRC(e89bd565) SHA1(6c7c1ad67ba708dbbe9654c1d290af290207d2be) )
ROM_END


/***************************************************************************

Poitto! (c)1993 Metro corp
MTR5260-A

CPU  : TMP68HC000P-16
Sound: D78C10ACW M6295 YM2413
OSC  : 24.0000MHz  (OSC1)
                   (OSC2)
                   (OSC3)
       3.579545MHz (OSC4)
                   (OSC5)

ROMs:
pt-1.13i - Graphics (23c4000)
pt-2.15i |
pt-3.17i |
pt-4.19i /

pt-jd05.20e - Main programs (27c010)
pt-jd06.20c /

pt-jc07.3g - Sound data (27c020)
pt-jc08.3i - Sound program (27c010)

Others:
Imagetek I4100 052 9309EK701 (208pin PQFP)
AMD MACH110-20 (CPLD)

***************************************************************************/

ROM_START( poitto )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pt-jd05.20e", 0x000000, 0x020000, CRC(6b1be034) SHA1(270c94f6017c5ce77f562bfe17273c79d4455053) )
	ROM_LOAD16_BYTE( "pt-jd06.20c", 0x000001, 0x020000, CRC(3092d9d4) SHA1(4ff95355fdf94eaa55c0ad46e6ce3b505e3ef790) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pt-jc08.3i", 0x000000, 0x020000, CRC(f32d386a) SHA1(655c561aec1112d88c1b94725e932059e5d1d5a8) )  // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "pt-2.15i", 0x000000, 0x080000, CRC(05d15d01) SHA1(24405908fb8207228cd3419657e0be49e413f152) )
	ROM_LOAD64_WORD( "pt-4.19i", 0x000002, 0x080000, CRC(8a39edb5) SHA1(1d860e0a1b975a93907d5bb0704e3bad383bbda7) )
	ROM_LOAD64_WORD( "pt-1.13i", 0x000004, 0x080000, CRC(ea6e2289) SHA1(2c939b32d2bf155bb5c8bd979dadcf4f75e178b0) )
	ROM_LOAD64_WORD( "pt-3.17i", 0x000006, 0x080000, CRC(522917c1) SHA1(cc2f5b574d31b0b93fe52c690f450b20b233dcad) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "pt-jc07.3g", 0x000000, 0x040000, CRC(5ae28b8d) SHA1(5e5f80ebbc4e3726ac8dbbfbefb9217f2e3e3563) )
ROM_END

ROM_START( poittoc )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pt-jc05.20e", 0x000000, 0x020000, CRC(96681051) SHA1(5717e50e6cf66694aa5b1a1d763f449adde18e3f) )
	ROM_LOAD16_BYTE( "pt-jc06.20c", 0x000001, 0x020000, NO_DUMP ) // faulty chip

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pt-jc08.3i", 0x000000, 0x020000, CRC(f32d386a) SHA1(655c561aec1112d88c1b94725e932059e5d1d5a8) )  // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "pt-2.15i", 0x000000, 0x080000, CRC(05d15d01) SHA1(24405908fb8207228cd3419657e0be49e413f152) )
	ROM_LOAD64_WORD( "pt-4.19i", 0x000002, 0x080000, CRC(8a39edb5) SHA1(1d860e0a1b975a93907d5bb0704e3bad383bbda7) )
	ROM_LOAD64_WORD( "pt-1.13i", 0x000004, 0x080000, CRC(ea6e2289) SHA1(2c939b32d2bf155bb5c8bd979dadcf4f75e178b0) )
	ROM_LOAD64_WORD( "pt-3.17i", 0x000006, 0x080000, CRC(522917c1) SHA1(cc2f5b574d31b0b93fe52c690f450b20b233dcad) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "pt-jc07.3g", 0x000000, 0x040000, CRC(5ae28b8d) SHA1(5e5f80ebbc4e3726ac8dbbfbefb9217f2e3e3563) )
ROM_END

/***************************************************************************

Puzzlet
2000 Yunizu Corporation

Very small PCB using Metro-like hardware with
Imagetek GFX chip and H8/3007 CPU

PCB Layout
----------
VG2200-(B)
|--------------------------------------------|
|TA7222     YM2413B  SOUND4   PRG1           |
|  VOL  JRC3403              DSW1(8)         |
|             M6295          DSW2(8) CG2 CG3 |
|          20MHz                             |
|  MM1035             |---------|            |
|      H8/3007        |IMAGETEK |    CY7C199 |
|TD62307     HM6216255|I4300    |            |
|TD62064              |         | CY7C199    |
|                     |         |            |
|  Z86E02             |---------| CY7C199    |
|        JAMMA              26.666MHz        |
|--------------------------------------------|
Notes:
      H8/3007   - Hitachi HD6413007F20 CPU. Clock 20MHz
      M6295     - Clock 4MHz [20/5]. Pin7 LOW
      I4300     - Imagetek I4300 Graphics Generator IC
      VSync     - 58Hz
      HSync     - 15.26kHz
      Z86E02    - DIP18 surface scratched, decapping reveals Zilog Z8 MCU
      HM6216255 - Hitachi 4M high speed SRAM (256-kword x16-bit)
      CY7C199   - 32k x8 SRAM
      YM2413B   - Clock 4MHz [20/5]
      MM1035    - System Reset IC with Watchdog Timer
      TD62307   - 7 Channel Low Saturation Driver
      TD62064   - 4 Channel High Current Darlington Driver
      TA7222    - 40V 4.5A 12.5W 5.8W Audio Power Amplifier IC

      All ROMs 27C160

***************************************************************************/

ROM_START( puzzlet )
	ROM_REGION( 0x200000, "maincpu", 0 )    // H8/3007 Code
	ROM_LOAD16_WORD_SWAP( "prg1_ver2.u9", 0x000000, 0x200000, CRC(592760da) SHA1(08f7493d2e50831438f53bbf0ae211ec40057da7) )

	ROM_REGION( 0x200, "coinmcu", 0 )    // Zilog Z8 family 8-bit MCU
	ROM_LOAD( "z86e02.mcu", 0x000, 0x200, CRC(399fa417) SHA1(f6c57020ea394c858742759050bf4f4b2f1e1fc5) )

	ROM_REGION( 0x400000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD32_WORD( "cg2.u2", 0x000000, 0x200000, CRC(7720f2d8) SHA1(8e0ccd1e8efe00df909327aefdb1e23e50487524) )
	ROM_LOAD32_WORD( "cg3.u1", 0x000002, 0x200000, CRC(77d39d12) SHA1(4bb339e479f0425931cff4eef3a6bc6ad1fac1f5) )

	ROM_REGION( 0x200000, "oki", 0 )    // Samples
	ROM_LOAD( "sound4.u23", 0x000000, 0x200000, CRC(9a611369) SHA1(97b9188354292b120a1bd0f01b4d884461bfa298) )
ROM_END

ROM_START( metabee ) // handwritten labels, unpopulated sound chips and ROM. Still the dumper says it has sound.
	ROM_REGION( 0x200000, "maincpu", 0 )    // H8/3007 Code
	ROM_LOAD16_WORD_SWAP( "medabee2way.u9", 0x000000, 0x200000, CRC(aba51e0f) SHA1(99f18d772a73c499b1b33222b9bae8c1e1d4114b) ) // ST-M27C160 handwritten "メダビー2WAY"

	ROM_REGION( 0x200, "coinmcu", 0 )    // Zilog Z8 family 8-bit MCU
	ROM_LOAD( "z86e02.mcu", 0x000, 0x200, NO_DUMP )

	ROM_REGION( 0x800000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD32_WORD( "medabeechara1.u2", 0x000000, 0x400000, CRC(d56918bd) SHA1(bcb3f4624a6f92e5c48273638bdb3f71608ac5b4) ) // ST-M27C322 handwritten "メダビーキャラ1"
	ROM_LOAD32_WORD( "medabeechara2.u1", 0x000002, 0x400000, CRC(81a3c0cb) SHA1(970978f07bb9e9dddd13b3946fb7230c2b205769) ) // ST-M27C322 handwritten "メダビーキャラ2"

	ROM_REGION( 0x200000, "oki", ROMREGION_ERASEFF )    // Samples
	// not populated on the dumped PCB
ROM_END

/***************************************************************************

Puzzli
Metro/Banpresto 1995

MTR5260-A                3.5759MHz  12MHz
               YM2151                         6116
   26.666MHz           M6295    PZ_JB7  PZ_JB8
                                     78C10
      7C199         Imagetek
      7C199           I4220
      61C64

                                          PZ_JB1
           68000-16                       PZ_JB2
               32MHz   6164               PZ_JB3
                       6164               PZ_JB4
    SW1     PZ_JB6 PZ_JB5
    SW2

***************************************************************************/

ROM_START( puzzli )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pz_jb5.20e", 0x000000, 0x020000, CRC(33bbbd28) SHA1(41a98cfbdd60a638e4aa08f15f1730a2436106f9) )
	ROM_LOAD16_BYTE( "pz_jb6.20c", 0x000001, 0x020000, CRC(e0bdea18) SHA1(9941a2cd88d7a3c1a640f837d9f34c39ba643ee5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pz_jb8.3i", 0x000000, 0x020000, CRC(c652da32) SHA1(907eba5103373ca6204f9d62c426ccdeef0a3791) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "pz_jb2.14i", 0x000000, 0x080000, CRC(0c0997d4) SHA1(922d8553ef505f65238e5cc77b45861a80022d75) )
	ROM_LOAD64_WORD( "pz_jb4.18i", 0x000002, 0x080000, CRC(576bc5c2) SHA1(08c10e0a3356ee1f79b78eff92395d8b18e43485) )
	ROM_LOAD64_WORD( "pz_jb1.12i", 0x000004, 0x080000, CRC(29f01eb3) SHA1(1a56f0b8efb599ae4f3cd0a4f0b6a6152ea6b117) )
	ROM_LOAD64_WORD( "pz_jb3.16i", 0x000006, 0x080000, CRC(6753e282) SHA1(49d092543db34f2cb54697897790df12ca3eda74) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "pz_jb7.3g", 0x000000, 0x040000, CRC(b3aab610) SHA1(9bcf1f98e19a7e26b22e152313dfbd43c882f008) )
ROM_END

ROM_START( puzzlia )
	ROM_REGION( 0x80000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pz-ja-5.20e", 0x000000, 0x020000, CRC(4e162574) SHA1(808de335064d6483cfaf8548ba0b1b8769828ca8) )
	ROM_LOAD16_BYTE( "pz-ja-6.20c", 0x000001, 0x020000, CRC(19210626) SHA1(d0d20dbe65cbe255f66526db6fc75d8c37d2a842) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pz-ja-8.3i", 0x000000, 0x020000, CRC(fd492a57) SHA1(43699a4607d586827b771180fcd6581988173bed) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "pz-ja-2.14i", 0x000000, 0x080000, CRC(0c0997d4) SHA1(922d8553ef505f65238e5cc77b45861a80022d75) )
	ROM_LOAD64_WORD( "pz-ja-4.18i", 0x000002, 0x080000, CRC(576bc5c2) SHA1(08c10e0a3356ee1f79b78eff92395d8b18e43485) )
	ROM_LOAD64_WORD( "pz-ja-1.12i", 0x000004, 0x080000, CRC(29f01eb3) SHA1(1a56f0b8efb599ae4f3cd0a4f0b6a6152ea6b117) )
	ROM_LOAD64_WORD( "pz-ja-3.16i", 0x000006, 0x080000, CRC(6753e282) SHA1(49d092543db34f2cb54697897790df12ca3eda74) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "pz-ja-7.3g", 0x000000, 0x040000, CRC(de285717) SHA1(040f999a640337716baa1c09ab0740f2d2ca09d2) )
ROM_END

/***************************************************************************

Sankokushi (JPN Ver.)
(c)1996 Mitchell

Board:  MTR5260-A

sound: YM2413 + M6295

***************************************************************************/

ROM_START( 3kokushi )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "5.20e", 0x000000, 0x040000, CRC(6104ea35) SHA1(efb4a9a98577894fac720028f18cb9877a00239a) )
	ROM_LOAD16_BYTE( "6.20c", 0x000001, 0x040000, CRC(aac25540) SHA1(811de761bb1b3cc47d811b00f4b5c960c8f061d0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "8.3i", 0x000000, 0x020000, CRC(f56cca45) SHA1(4739b83b0b3a4235fac10def3d26b0bd190eb12a) )    // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "2.14i", 0x000000, 0x080000, CRC(291f8149) SHA1(82f460517543ef544c21a81e51987fb2f5c6273d) )
	ROM_LOAD64_WORD( "4.18i", 0x000002, 0x080000, CRC(9317c359) SHA1(9756757fb5d2b298a2b1917a131f391ef0e31fb9) )
	ROM_LOAD64_WORD( "1.12i", 0x000004, 0x080000, CRC(d5495759) SHA1(9cbcb48915ec44a8026d88d96ab391e118e89df5) )
	ROM_LOAD64_WORD( "3.16i", 0x000006, 0x080000, CRC(3d76bdf3) SHA1(f621fcc8e6bde58077216b534c2e876ea9311e15) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "7.3g", 0x000000, 0x040000, CRC(78fe9d44) SHA1(365a2d51daa24741957fa619bbbbf96e8f370701) )
ROM_END


/***************************************************************************

Pururun (c)1995 Metro/Banpresto
MTR5260-A

CPU  : TMP68HC000P-16
Sound: D78C10ACW M6295 YM2151 Y3012
OSC  : 24.000MHz   (OSC1)
                   (OSC2)
       26.6660MHz  (OSC3)
                   (OSC4)
       3.579545MHz (OSC5)

ROMs:
pu9-19-1.12i - Graphics (27c4096)
pu9-19-2.14i |
pu9-19-3.16i |
pu9-19-4.18i /

pu9-19-5.20e - Main programs (27c010)
pu9-19-6.20c /

pu9-19-7.3g - Sound data (27c020)
pu9-19-8.3i - Sound program (27c010)

Others:
Imagetek I4220 071 9338EK707 (208pin PQFP)
AMD MACH110-20 (CPLD)

***************************************************************************/

ROM_START( pururun ) // These labels follow standard production format, IE: Game code, region/revision - PU JA
	ROM_REGION( 0x80000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pu_ja-5.20e", 0x000000, 0x020000, CRC(c15ae3db) SHA1(7056ef5d96b7bdd30e85c2e7f0482cf04e2300c7) )
	ROM_LOAD16_BYTE( "pu_ja-6.20c", 0x000001, 0x020000, CRC(2e21328a) SHA1(6aad7fb728042eef2edce73acd73cd9a29cb1082) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pu_ja-8.3i", 0x000000, 0x020000, CRC(edc3830b) SHA1(13ee759d10711218465f6d7155e9c443a82b323c) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "pu_ja-2.14i", 0x000000, 0x080000, CRC(93a9dbed) SHA1(76d11a14e655d3d6de95847dd69f425467d1a1d9) )
	ROM_LOAD64_WORD( "pu_ja-4.18i", 0x000002, 0x080000, CRC(47d82187) SHA1(a588f053781773d54a2333bf70965ff357da0253) )
	ROM_LOAD64_WORD( "pu_ja-1.12i", 0x000004, 0x080000, CRC(436096c6) SHA1(35575defa25f0e3a7db3fb7a0e22e003e05d9c67) )
	ROM_LOAD64_WORD( "pu_ja-3.16i", 0x000006, 0x080000, CRC(80619e1a) SHA1(625d3fc829e3b7f24de9b0b932b2cef2cd8509ef) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "pu_ja-7.3g", 0x000000, 0x040000, CRC(51ae4926) SHA1(1a69a00e960bda399aaf051b3dcc9e0a108c8047) )
ROM_END

ROM_START( pururuna ) // dev or proto version?, PU9 & 19 don't follow normal Game code, region/revision format
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "pu9-19-5.20e", 0x000000, 0x020000, CRC(5a466a1b) SHA1(032eeaf66ce1b601385a8e76d2efd9ea6fd34680) )
	ROM_LOAD16_BYTE( "pu9-19-6.20c", 0x000001, 0x020000, CRC(d155a53c) SHA1(6916a1bad82c624b8757f5124416dac50a8dd7f5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "pu9-19-8.3i", 0x000000, 0x020000, CRC(edc3830b) SHA1(13ee759d10711218465f6d7155e9c443a82b323c) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "pu9-19-2.14i", 0x000000, 0x080000, CRC(21550b26) SHA1(cb2a2f672cdca84def2fac8d325b7a80a1e9bfc0) )
	ROM_LOAD64_WORD( "pu9-19-4.18i", 0x000002, 0x080000, CRC(3f3e216d) SHA1(9881e07d5ee237b7134e2ddcf9a9887a1d7f3b4c) )
	ROM_LOAD64_WORD( "pu9-19-1.12i", 0x000004, 0x080000, CRC(7e83a75f) SHA1(9f516bbfc4ca8a8e857ebf7a19c37d7f026695a6) )
	ROM_LOAD64_WORD( "pu9-19-3.16i", 0x000006, 0x080000, CRC(d15485c5) SHA1(d37670b0d696f4ee9da7b8199da114fb4e45cd20) )

	ROM_REGION( 0x040000, "oki", 0 )    // Samples
	ROM_LOAD( "pu9-19-7.3g", 0x000000, 0x040000, CRC(51ae4926) SHA1(1a69a00e960bda399aaf051b3dcc9e0a108c8047) )
ROM_END


/***************************************************************************

Sky Alert (JPN Ver.)
(c)1992 Metro
VG420
                                     SA
                     SW2 SW1         B12   4016
                                           NEC78C10  3.579MHz

                                                          6269
                                                          SA
                                                          A11
  55328 55328 55328       24MHz

                           4064   4064   SA   SA          68000-12
       Imagetek                          C10  C09
       I4100


    SA  SA  SA  SA  SA  SA  SA  SA
    A08 A07 A06 A05 A04 A03 A02 A01

CPU     :MC68000P12
Sound   :Yamaha YM2413, OKI M6295
OSC     :24.0000MHz, 3.579545MHz
other   :D78C10ACW, Imagetek Inc I4100 052

Master clock: 24.00MHz
 D7810 clock: 12.00MHz (24 / 2)
 M6295 clock: 1.200MHz (24 / 20), sample rate =  M6295 clock /165
YM2413 clock: 3.579545MHz


***************************************************************************/

ROM_START( skyalert )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "sa_c_09.bin", 0x000000, 0x020000, CRC(6f14d9ae) SHA1(37e134af3d8461280dab971bc3ee9112f25de335) )
	ROM_LOAD16_BYTE( "sa_c_10.bin", 0x000001, 0x020000, CRC(f10bb216) SHA1(d904030fbb838d906ca69a77cffe286e903b273d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "sa_b_12.bin", 0x000000, 0x020000, CRC(f358175d) SHA1(781d0f846217aa71e3c6d73c1d63bd87d1fa6b48) ) // (c)1992 Imagetek (1xxxxxxxxxxxxxxxx = 0xFF)

	ROM_REGION( 0x200000, "vdp", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_BYTE( "sa_a_02.bin", 0x000000, 0x040000, CRC(f4f81d41) SHA1(85e587b4fda71fa5b944b0ac158d36c00e290f5f) )
	ROM_LOAD64_BYTE( "sa_a_04.bin", 0x000001, 0x040000, CRC(7d071e7e) SHA1(24b9b0cb7e9f719259b0444ee896bdc1ad79a28d) )
	ROM_LOAD64_BYTE( "sa_a_06.bin", 0x000002, 0x040000, CRC(77e4d5e1) SHA1(420e5aaf187e297b371830ebd5787675cff6177b) )
	ROM_LOAD64_BYTE( "sa_a_08.bin", 0x000003, 0x040000, CRC(f2a5a093) SHA1(66d482cc3f45ff7bf1363cf3c88e2dabc902a299) )
	ROM_LOAD64_BYTE( "sa_a_01.bin", 0x000004, 0x040000, CRC(41ec6491) SHA1(c0bd66409bc6ea969f4c45cc006fde891ba8b4d7) )
	ROM_LOAD64_BYTE( "sa_a_03.bin", 0x000005, 0x040000, CRC(e0dff10d) SHA1(3aa18b05f06b4b0a88ba4df86dfc0ca650c2684e) )
	ROM_LOAD64_BYTE( "sa_a_05.bin", 0x000006, 0x040000, CRC(62169d31) SHA1(294887b6ce0d56e053e7f7583b8a160afeef4ce5) )
	ROM_LOAD64_BYTE( "sa_a_07.bin", 0x000007, 0x040000, CRC(a6f5966f) SHA1(00319b96dacc4dcfd70935e1626da0ae6aa63e5a) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "sa_a_11.bin", 0x000000, 0x020000, CRC(04842a60) SHA1(ade016c85867dee7ac27efe3910b01f5f8e730a0) )
ROM_END


/***************************************************************************

Toride II Adauchi Gaiden (c)1994 Metro corp
MTR5260-A

CPU  : TMP68HC000P-16
Sound: D78C10ACW M6295 YM2413
OSC  : 24.0000MHz  (OSC1)
                   (OSC2)
       26.6660MHz  (OSC3)
       3.579545MHz (OSC4)
                   (OSC5)

ROMs:
tr2aja-1.12i - Graphics (27c4096)
tr2aja-2.14i |
tr2aja-3.16i |
tr2aja-4.18i /

tr2aja-5.20e - Main programs (27c020)
tr2aja-6.20c /

tr2aja-7.3g - Sound data (27c010)
tr2aja-8.3i - Sound program (27c010)

Others:
Imagetek I4220 071 9338EK700 (208pin PQFP)
AMD MACH110-20 (CPLD)

***************************************************************************/

ROM_START( toride2g )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tr2aja-5.20e", 0x000000, 0x040000, CRC(b96a52f6) SHA1(353b5599d50d96b96bdd6352c046ad669cf8da44) )
	ROM_LOAD16_BYTE( "tr2aja-6.20c", 0x000001, 0x040000, CRC(2918b6b4) SHA1(86ebb884759dc9a8a701784d19845467aa1ce11b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr2aja-8.3i", 0x000000, 0x020000, CRC(fdd29146) SHA1(8e996e1afd33f16d35ebf5a40829feb3e92f781f) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "tr2aja-2.14i", 0x000000, 0x080000, CRC(5c73f629) SHA1(b38b7ee213bcc0dd5e4c339a8f9f2fdd81ede6ad) )
	ROM_LOAD64_WORD( "tr2aja-4.18i", 0x000002, 0x080000, CRC(67ebaf1b) SHA1(a0c5f253cc33620251fb58ef6f1647453d778462) )
	ROM_LOAD64_WORD( "tr2aja-1.12i", 0x000004, 0x080000, CRC(96245a5c) SHA1(524990c88a08648de6f330652fc5c02a27e1325c) )
	ROM_LOAD64_WORD( "tr2aja-3.16i", 0x000006, 0x080000, CRC(49013f5d) SHA1(8f29bd2606b30260e9b21886f2b257f7ae8fb2bf) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr2aja-7.3g", 0x000000, 0x020000, CRC(630c6193) SHA1(ddb63724e0b0f7264cb02904e49b24b87beb35a9) )
ROM_END

ROM_START( toride2gg )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "trii_ge_5.20e", 0x000000, 0x040000, CRC(5e0815a8) SHA1(574c1bf1149b7e98222876b402b20d824f207c79) )
	ROM_LOAD16_BYTE( "trii_ge_6.20c", 0x000001, 0x040000, CRC(55eba67d) SHA1(c12a11a98d49baf3643404a594d2b87b434acb01) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr2_jb-8.3i", 0x000000, 0x020000, CRC(0168f46f) SHA1(01bf4cc425d72936897c3c572f6c0b1366fe4041) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "trii_gb_2.14i", 0x000000, 0x080000, CRC(5949e65f) SHA1(f51ff9590904e691b9ec91b22d3c52bf579deaff) )
	ROM_LOAD64_WORD( "trii_gb_4.18i", 0x000002, 0x080000, CRC(adc84c7b) SHA1(fe0f2b6e3c586c427701e43fdd4827c8b183b42a) )
	ROM_LOAD64_WORD( "trii_gb_1.12i", 0x000004, 0x080000, CRC(bcf30944) SHA1(c36fbffa6062a2443a47d8faf83baa903529ee97) )
	ROM_LOAD64_WORD( "trii_gb_3.16i", 0x000006, 0x080000, CRC(138e68d0) SHA1(5a9655f31e2f2e2f16a5bdc334efa78b2cfc37d2) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr2_ja_7.3g", 0x000000, 0x020000, CRC(6ee32315) SHA1(ef4d59576929deab0aa459a67be21d97c2803dea) )
ROM_END

/***************************************************************************

Toride II Bok Su Oi Jeon Adauchi Gaiden
(C)1994 Metro (Yu Jin?)

MTR5260-A
|----------------------------------------------|
| TA7222       YM2413  3.579545MHz  *12MHz     |
|VOLUME  *UPC3403 *YM3012                 6116 |
|UPC3403             *YM2151       ROM7   ROM8 |
|       26.824MHz          M6295               |
|                    |--------|          78C10 |
|         62256      |IMAGETEK|                |
|         62256      |I4220   |                |
|J                   |        |                |
|A        6264       |        |          ROM1  |
|M                   |--------|                |
|M                                             |
|A           MB3771                      ROM2  |
|          |---------|                         |
|          |68000-12 |                         |
|          |---------|                         |
|                     6264               ROM3  |
|                                              |
|     MACH110  24MHz  6264                     |
|SW1                                     ROM4  |
|                                              |
|SW2       ROM6       ROM5                     |
|----------------------------------------------|
* = Not populated on Toride II
MB3771 == MM1035 used on other MTR5260-A PCBs

info by Guru

***************************************************************************/

ROM_START( toride2gk )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "5", 0x00000, 0x40000, CRC(7e3f943a) SHA1(d9f36ee85ad8ae562433e0173562ededf6c6f3e4) )
	ROM_LOAD16_BYTE( "6", 0x00001, 0x40000, CRC(92726910) SHA1(529644fb8e4ea8df0dde617afd3e274821513ab4) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "8", 0x00000, 0x20000, CRC(fdd29146) SHA1(8e996e1afd33f16d35ebf5a40829feb3e92f781f) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "2", 0x00000, 0x80000, CRC(5e7fb9db) SHA1(37094ea750be8605bd2130d0d5ce5f9c43b0cc77) )
	ROM_LOAD64_WORD( "4", 0x00002, 0x80000, CRC(558c03e7) SHA1(f7fa5aa9eacd8953d998d9b05d5f03e65056bd78) )
	ROM_LOAD64_WORD( "1", 0x00004, 0x80000, CRC(5e819ccd) SHA1(b1d4e800bac0f55286317d2a39c2b245d87a3e50) )
	ROM_LOAD64_WORD( "3", 0x00006, 0x80000, CRC(24029583) SHA1(6e03db0a9835a8cf5c565d10794e8b01c919a679) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "7", 0x00000, 0x20000, CRC(630c6193) SHA1(ddb63724e0b0f7264cb02904e49b24b87beb35a9) )
ROM_END

ROM_START( toride2j )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tr2_jk-5.20e", 0x000000, 0x040000, CRC(f2668578) SHA1(1dd18a5597efb25c937697b50fb1262f50580a63) )
	ROM_LOAD16_BYTE( "tr2_jk-6.20c", 0x000001, 0x040000, CRC(4c87629f) SHA1(5fde8580bedb783491ee87ecfe4b1c22d0c9f716) )

	ROM_REGION( 0x20000, "audiocpu", 0 )       // NEC78C10 Code
	ROM_LOAD( "tr2_jb-8.3i", 0x000000, 0x020000, CRC(0168f46f) SHA1(01bf4cc425d72936897c3c572f6c0b1366fe4041) )

	ROM_REGION( 0x200000, "vdp2", 0 )   // Gfx + Data (Addressable by CPU & Blitter)
	ROM_LOAD64_WORD( "tr2_jb-2.14i", 0x000000, 0x080000, CRC(b31754dc) SHA1(be2423bafbf07c93c3d222e907190b44616014f0) )
	ROM_LOAD64_WORD( "tr2_jb-4.18i", 0x000002, 0x080000, CRC(a855c3fa) SHA1(eca3e235256df7e6ae66ecbe43bc0edb974af503) )
	ROM_LOAD64_WORD( "tr2_jb-1.12i", 0x000004, 0x080000, CRC(856f40b7) SHA1(99aca5472b991cd08e9c2128ffdd40675a3b968d) )
	ROM_LOAD64_WORD( "tr2_jb-3.16i", 0x000006, 0x080000, CRC(78ba205f) SHA1(1069a362e60747aaa284c0d9bb7718013df347f3) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "tr2_ja_7.3g", 0x000000, 0x020000, CRC(6ee32315) SHA1(ef4d59576929deab0aa459a67be21d97c2803dea) )
ROM_END

/***************************************************************************

Varia Metal
Excellent System Ltd, 1995

PCB Layout
----------
This game runs on Metro hardware.

ES-9309B-B
|--------------------------------------------|
| TA7222   8.U9     DSW1(8)  DSW2(8)         |
|VOL    M6295  1.000MHz                      |
|                   |------------|           |
|          7.U12    |   68000    |           |
|  uPC3403          |------------|           |
|J 640kHz  ES-8712                           |
|A M6585           EPM7032    6B.U18  5B.U19 |
|M       MM1035                              |
|M        26.666MHz  16MHz    62256   62256  |
|A                                           |
|                 |--------|          1.U29  |
|         62256   |Imagetek|                 |
|         62256   |I4220   |          2.U31  |
|                 |        |                 |
|                 |--------|          3.U28  |
|                                            |
|                  6264               4.U30  |
|--------------------------------------------|
Notes:
      68000   - clock 16.000MHz
      ES-8712 - Excellent System ES-8712 Sound Controller for M6585 (SDIP48)
      M6295   - clock 1.000MHz. Sample rate = 1000000/132
      M6585   - Oki M6585 ADPCM Voice Synthesizer IC (DIP18). Clock 640kHz.
                Sample rate = 16kHz (selection - pin 1 LOW, pin 2 HIGH = 16kHz)
                This is a version-up to the previous M5205 with some additional
                capabilities and improvements.
      MM1035  - Mitsumi Monolithic IC MM1035 System Reset and Watchdog Timer (DIP8)
      uPC3403 - NEC uPC3403 High Performance Quad Operational Amplifier (DIP14)
      62256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      TA7222  - Toshiba TA7222 5.8 Watt Audio Power Amplifier (SIP10)
      EPM7032 - Altera EPM7032LC44-15T High Performance EEPROM-based Programmable Logic Device (PLCC44)
      Custom  - Imagetek I4220 Graphics Controller (QFP208)
      VSync   - 58.2328Hz
      HSync   - 15.32kHz
      ROMs    -
                6B & 5B are 27C040 EPROM (DIP32)
                8 is 4M MaskROM (DIP32)
                All other ROMs are 16M MaskROM (DIP42)

***************************************************************************/

ROM_START( vmetal )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "6b.u18", 0x00000, 0x80000, CRC(4eb939d5) SHA1(741ab05043fc3bd886162d878630e45da9359718) )
	ROM_LOAD16_BYTE( "5b.u19", 0x00001, 0x80000, CRC(4933ac6c) SHA1(1a3303e32fcb08854d4d6e13f36ca99d92aed4cc) )

	ROM_REGION( 0x800000, "vdp2", 0 )
	ROM_LOAD64_WORD( "2.u31", 0x000000, 0x200000, CRC(b36f8d60) SHA1(1676859d0fee4eb9897ce1601a2c9fd9a6dc4a43) )
	ROM_LOAD64_WORD( "4.u30", 0x000002, 0x200000, CRC(5a25a49c) SHA1(c30781202ec882e1ec6adfb560b0a1075b3cce55) )
	ROM_LOAD64_WORD( "1.u29", 0x000004, 0x200000, CRC(b470c168) SHA1(c30462dc134da1e71a94b36ef96ecd65c325b07e) )
	ROM_LOAD64_WORD( "3.u28", 0x000006, 0x200000, CRC(00fca765) SHA1(ca9010bd7f59367e483868018db9a9abf871386e) )

	ROM_REGION( 0x080000, "oki", 0 ) // OKI6295 Samples
	// Second half is junk
	ROM_LOAD( "8.u9", 0x00000, 0x80000, CRC(c14c001c) SHA1(bad96b5cd40d1c34ef8b702262168ecab8192fb6) )

	ROM_REGION( 0x200000, "essnd", 0 ) // Samples
	ROM_LOAD( "7.u12", 0x00000, 0x200000, CRC(a88c52f1) SHA1(d74a5a11f84ba6b1042b33a2c156a1071b6fbfe1) )
ROM_END

ROM_START( vmetaln )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "vm6.bin", 0x00000, 0x80000, CRC(cb292ab1) SHA1(41fdfe67e6cb848542fd5aa0dfde3b1936bb3a28) )
	ROM_LOAD16_BYTE( "vm5.bin", 0x00001, 0x80000, CRC(43ef844e) SHA1(c673f34fcc9e406282c9008795b52d01a240099a) )

	ROM_REGION( 0x800000, "vdp2", 0 )
	ROM_LOAD64_WORD( "2.u31", 0x000000, 0x200000, CRC(b36f8d60) SHA1(1676859d0fee4eb9897ce1601a2c9fd9a6dc4a43) )
	ROM_LOAD64_WORD( "4.u30", 0x000002, 0x200000, CRC(5a25a49c) SHA1(c30781202ec882e1ec6adfb560b0a1075b3cce55) )
	ROM_LOAD64_WORD( "1.u29", 0x000004, 0x200000, CRC(b470c168) SHA1(c30462dc134da1e71a94b36ef96ecd65c325b07e) )
	ROM_LOAD64_WORD( "3.u28", 0x000006, 0x200000, CRC(00fca765) SHA1(ca9010bd7f59367e483868018db9a9abf871386e) )

	ROM_REGION( 0x080000, "oki", 0 ) // OKI6295 Samples
	// Second half is junk
	ROM_LOAD( "8.u9", 0x00000, 0x80000, CRC(c14c001c) SHA1(bad96b5cd40d1c34ef8b702262168ecab8192fb6) )

	ROM_REGION( 0x200000, "essnd", 0 ) // Samples
	ROM_LOAD( "7.u12", 0x00000, 0x200000, CRC(a88c52f1) SHA1(d74a5a11f84ba6b1042b33a2c156a1071b6fbfe1) )
ROM_END


/***************************************************************************


                                Driver Inits


***************************************************************************/

void metro_state::init_karatour()
{
	save_item(NAME(m_ext_irq_enable));
}


// Unscramble the GFX ROMs
void metro_state::init_balcube()
{
	u8 *ROM       = memregion("vdp2")->base();
	const unsigned len = memregion("vdp2")->bytes();

	for (unsigned i = 0; i < len; i+=2)
	{
		ROM[i]  = bitswap<8>(ROM[i],0,1,2,3,4,5,6,7);
	}
}


void metro_upd7810_state::init_dharmak()
{
	u8 *src = memregion("vdp2")->base();
	for (int i = 0; i < 0x200000; i += 4)
	{
		u8 dat = src[i + 1];
		dat = bitswap<8>(dat, 7,3,2,4, 5,6,1,0);
		src[i + 1] = dat;

		dat = src[i + 3];
		dat = bitswap<8>(dat, 7,2,5,4, 3,6,1,0);
		src[i + 3] = dat;
	}
}

/***************************************************************************


                                Game Drivers


***************************************************************************/

// VG420 / VG460
GAME( 1992, karatour,  0,        karatour,  karatour,   metro_upd7810_state, init_karatour, ROT0,   "Mitchell",                                        "The Karate Tournament", MACHINE_SUPPORTS_SAVE )
GAME( 1992, karatourj, karatour, karatour,  karatour,   metro_upd7810_state, init_karatour, ROT0,   "Mitchell",                                        "Chatan Yara Kuushanku - The Karate Tournament (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, pangpoms,  0,        pangpoms,  pangpoms,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Pang Pom's", MACHINE_SUPPORTS_SAVE )
GAME( 1992, pangpomsm, pangpoms, pangpoms,  pangpoms,   metro_upd7810_state, empty_init,    ROT0,   "Metro (Mitchell license)",                        "Pang Pom's (Mitchell)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, pangpomsn, pangpoms, pangpoms,  pangpoms,   metro_upd7810_state, empty_init,    ROT0,   "Nova",                                            "Pang Pom's (Nova)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, skyalert,  0,        skyalert,  skyalert,   metro_upd7810_state, empty_init,    ROT270, "Metro",                                           "Sky Alert", MACHINE_SUPPORTS_SAVE )
GAME( 1993, ladykill,  0,        karatour,  ladykill,   metro_upd7810_state, init_karatour, ROT90,  "Yanyaka (Mitchell license)",                      "Lady Killer", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, moegonta,  ladykill, karatour,  moegonta,   metro_upd7810_state, init_karatour, ROT90,  "Yanyaka",                                         "Moeyo Gonta!! (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfort,  0,        lastfort,  lastfort,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Last Fortress - Toride (Japan, VG420 PCB)", MACHINE_SUPPORTS_SAVE ) // VG420 PCB
GAME( 1994, lastforte, lastfort, lastfort,  lastfero,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Last Fortress - Toride (China, Rev C)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfortea,lastfort, lastfort,  lastfero,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Last Fortress - Toride (China, Rev A)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfortk, lastfort, lastfort,  lastfero,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Last Fortress - Toride (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfortj, lastfort, lastforg,  ladykill,   metro_upd7810_state, init_karatour, ROT0,   "Metro",                                           "Last Fortress - Toride (Japan, VG460 PCB)", MACHINE_SUPPORTS_SAVE ) // VG460-(A) PCB
GAME( 1994, lastfortg, lastfort, lastforg,  ladykill,   metro_upd7810_state, init_karatour, ROT0,   "Metro",                                           "Last Fortress - Toride (Germany)", MACHINE_SUPPORTS_SAVE ) // VG460-(A) PCB

// MTR5260 / MTR527
GAME( 1993, poitto,    0,        poitto,    poitto,     metro_upd7810_state, empty_init,    ROT0,   "Metro / Able Corp.",                              "Poitto! (revision D)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, poittoc,   poitto,   poitto,    poitto,     metro_upd7810_state, empty_init,    ROT0,   "Metro / Able Corp.",                              "Poitto! (revision C)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // missing 1 program ROM
GAME( 1994, dharma,    0,        dharma,    dharma,     metro_upd7810_state, init_dharmak,  ROT0,   "Metro",                                           "Dharma Doujou", MACHINE_SUPPORTS_SAVE )
GAME( 1994, dharmag,   dharma,   dharma,    dharma,     metro_upd7810_state, init_dharmak,  ROT0,   "Metro",                                           "Dharma Doujou (Germany)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, dharmaj,   dharma,   dharma,    dharma,     metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Dharma Doujou (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, dharmak,   dharma,   dharma,    dharma,     metro_upd7810_state, init_dharmak,  ROT0,   "Metro",                                           "Dharma Doujou (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2g,  0,        toride2g,  toride2g,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Toride II Adauchi Gaiden", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2gg, toride2g, toride2g,  toride2g,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Toride II Adauchi Gaiden (German)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2gk, toride2g, toride2g,  toride2g,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Toride II Bok Su Oi Jeon Adauchi Gaiden (Korea)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2j,  toride2g, toride2g,  toride2g,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Toride II (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, gunmast,   0,        pururun,   gunmast,    metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Gun Master", MACHINE_SUPPORTS_SAVE )
GAME( 1995, daitorid,  0,        daitorid,  daitorid,   metro_upd7810_state, empty_init,    ROT0,   "Metro",                                           "Daitoride", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, pururun,   0,        pururun,   pururun,    metro_upd7810_state, empty_init,    ROT0,   "Metro / Banpresto",                               "Pururun (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, pururuna,  pururun,  pururun,   pururun,    metro_upd7810_state, empty_init,    ROT0,   "Metro / Banpresto",                               "Pururun (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, puzzli,    0,        puzzli,    puzzli,     metro_upd7810_state, empty_init,    ROT0,   "Metro / Banpresto",                               "Puzzli (revision B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, puzzlia,   puzzli,   puzzlia,   puzzli,     metro_upd7810_state, empty_init,    ROT0,   "Metro / Banpresto",                               "Puzzli (revision A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, 3kokushi,  0,        sankokushi,sankokushi, metro_upd7810_state, init_karatour, ROT0,   "Mitchell",                                        "Sankokushi (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// ? with additional gfx data scramble (probably MTR5260 based)
GAME( 1995, msgogo,    0,        msgogo,    msgogo,     metro_state,         init_balcube,  ROT0,   "Metro",                                           "Mouse Shooter GoGo", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, daitorida, daitorid, daitoa,    daitoa,     metro_state,         init_balcube,  ROT0,   "Metro",                                           "Daitoride (YMF278B version)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, balcube,   0,        balcube,   balcube,    metro_state,         init_balcube,  ROT0,   "Metro",                                           "Bal Cube", MACHINE_SUPPORTS_SAVE )
GAME( 1996, bangball,  0,        bangball,  bangball,   metro_state,         init_balcube,  ROT0,   "Banpresto / Kunihiko Tashiro+Goodhouse",          "Bang Bang Ball (v1.05)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, batlbubl,  bangball, batlbubl,  batlbubl,   metro_state,         init_balcube,  ROT0,   "Banpresto (Limenko license?)",                    "Battle Bubble (v2.00)", MACHINE_SUPPORTS_SAVE ) // or bootleg?

// VG330 / VG340 / VG410
GAME( 1995, dokyusei,  0,        dokyusei,  dokyusei,   gakusai_state,       empty_init,    ROT0,   "Make Software / Elf / Media Trading",             "Mahjong Doukyuusei", MACHINE_SUPPORTS_SAVE )
GAME( 1995, dokyusp,   0,        dokyusp,   gakusai,    gakusai_state,       empty_init,    ROT0,   "Make Software / Elf / Media Trading",             "Mahjong Doukyuusei Special", MACHINE_SUPPORTS_SAVE )
GAME( 1996, mouja,     0,        mouja,     mouja,      mouja_state,         empty_init,    ROT0,   "Etona",                                           "Mouja (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, gakusai,   0,        gakusai,   gakusai,    gakusai_state,       empty_init,    ROT0,   "MakeSoft",                                        "Mahjong Gakuensai (Japan, set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1997, gakusaia,  gakusai,  gakusai,   gakusai,    gakusai_state,       empty_init,    ROT0,   "MakeSoft",                                        "Mahjong Gakuensai (Japan, set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1998, gakusai2,  0,        gakusai2,  gakusai,    gakusai_state,       empty_init,    ROT0,   "MakeSoft",                                        "Mahjong Gakuensai 2 (Japan)", MACHINE_SUPPORTS_SAVE )

// HUM-002 / HUM-003
GAME( 1994, blzntrnd,  0,        blzntrnd,  blzntrnd,   blzntrnd_state,      empty_init,    ROT0,   "Human Amusement",                                 "Blazing Tornado", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1996, gstrik2,   0,        gstrik2,   gstrik2,    blzntrnd_state,      empty_init,    ROT0,   "Human Amusement",                                 "Grand Striker 2 (Europe and Oceania)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1996, gstrik2j,  gstrik2,  gstrik2,   gstrik2,    blzntrnd_state,      empty_init,    ROT0,   "Human Amusement",                                 "Grand Striker 2 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // priority between rounds

// ES-9309B-B
GAME( 1995, vmetal,    0,        vmetal,    vmetal,     vmetal_state,        empty_init,    ROT90,  "Excellent System",                                "Varia Metal", MACHINE_SUPPORTS_SAVE )
GAME( 1995, vmetaln,   vmetal,   vmetal,    vmetal,     vmetal_state,        empty_init,    ROT90,  "Excellent System (New Ways Trading Co. license)", "Varia Metal (New Ways Trading Co.)", MACHINE_SUPPORTS_SAVE )

// VG2200
GAME( 2000, puzzlet,   0,        puzzlet,   puzzlet,    metro_state,         init_karatour, ROT0,   "Unies Corporation",                               "Puzzlet (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, metabee,   0,        puzzlet,   puzzlet,    metro_state,         init_karatour, ROT0,   "Natsume / Banpresto",                             "Metabee Shot", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // Hopper problem
