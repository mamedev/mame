// license:BSD-3-Clause
// copyright-holders:Victor Trucco,Steve Ellenoff,Phil Stroffolino,Tatsuyuki Satoh,Tomasz Slanina,Nicola Salmoria,Vas Crabb
/***************************************************************************

Ping Pong King  (c) Taito 1985
Gladiator       (c) Taito 1986 - developed by Allumer

Credits:
- Victor Trucco: original emulation and MAME driver
- Steve Ellenoff: YM2203 Sound, ADPCM Sound, dip switch fixes, high score save,
          input port patches, panning fix, sprite banking,
          Golden Castle Rom Set Support
- Phil Stroffolino: palette, sprites, misc video driver fixes
- Tatsuyuki Satoh: YM2203 sound improvements, NEC 8741 simulation, ADPCM with MC6809
- Tomasz Slanina:  preliminary Ping Pong King driver
- Nicola Salmoria: clean up
- Vas Crabb:       MCU hookup

special thanks to:
- Camilty for precious hardware information and screenshots
- Jason Richmond for hardware information and misc. notes
- Joe Rounceville for schematics
- JunoMan for measuring and tracing signals
- and everyone else who's offered support along the way!


***************************************************************************

PING PONG KING
TAITO 1985
M6100094A

-------------
P0-003


X Q0_10 Q0_9 Q0_8 Q0_7 Q0_6 6116 Q0_5 Q0_4 Q0_3 Q1_2 Q1_1
                                                 Q1
                                   Z80B
                                       8251
 2009 2009 2009
               AQ-001
          Q2
              2116
              2116
                                 AQ-003
-------------
P1-004


 QO_15 TMM2009 Q0_14 Q0_13 Q0_12     2128 2128


                                                           SW1
                                                    AQ-004

                                              SW2          SW3
   6809 X Q0_19 Q0_18
                           Z80  Q0_17 Q0_16 2009 AQ-003 YM2203


***************************************************************************

Gladiator Memory Map
--------------------

Main CPU (Z80)
--------------
The address decoding is done by two PROMs, Q3 @ 2B and Q4 @ 5S.

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx ROM 1F    program ROM
010xxxxxxxxxxxxx R   xxxxxxxx ROM 1D    program ROM
011xxxxxxxxxxxxx R   xxxxxxxx ROM 1A    program ROM (paged)
10xxxxxxxxxxxxxx R   xxxxxxxx ROM 1C    program ROM (paged)
110000xxxxxxxxxx R/W xxxxxxxx RAM 4S    sprite RAM [1]
110001xxxxxxxxxx R/W xxxxxxxx RAM 4U    sprite RAM
110010xxxxxxxxxx R/W xxxxxxxx RAM 4T    sprite RAM
110011000-------   W xxxxxxxx VCSA2     fg relative scroll Y?
110011001-------   W ------xx           fg tile bank select
110011001-------   W -----x--           bg scroll X msb
110011001-------   W ----x---           fg scroll X msb
110011001-------   W ---x----           bg tile bank select
110011001-------   W --x-----           blank screen?
11001101--------   W xxxxxxxx VCSA1     fg scroll X
11001110--------   W xxxxxxxx           fg+bg scroll Y?
11001111--------   W xxxxxxxx VCSA3     bg scroll X
11010xxxxxxxxxxx R/W xxxxxxxx CCS       palette RAM
11011xxxxxxxxxxx R/W xxxxxxxx VCS1      bg tilemap RAM
11100xxxxxxxxxxx R/W xxxxxxxx VCS2      bg tilemap RAM
11101xxxxxxxxxxx R/W xxxxxxxx VCS3      fg tilemap RAM
11110xxxxxxxxxxx R/W xxxxxxxx RAM 1H    work RAM (battery backed)
11111-----------              n.c.

[1] only the first 256 bytes of each RAM actually contain sprite data (two buffers
    of 128 bytes).

I/O ports:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
--------000--000   W -------x OBJACS ?  select active sprite buffer
--------000--001   W -------x OBJCGBK   sprite bank select
--------000--010   W -------x PR.BK     ROM bank select
--------000--011   W -------x NMIFG     NMI enable/acknowledge (NMI not used by Gladiator)
--------000--100   W -------x SRST      reset sound CPU
--------000--101   W -------x CBK0      unknown
--------000--110   W -------x LOBJ      unknown (related to sprites)
--------000--111   W -------x REVERS    flip screen
--------001-----              n.c.
--------010-----              n.c.
--------011-----              n.c.
--------100----x R/W xxxxxxxx CIOMCS    8741 #0 (communication with 2nd Z80, DIPSW1)
--------101----- R/W          ARST      watchdog reset
--------110----x R/W xxxxxxxx SI/.CS    8251 (serial communication for debug purposes)
--------111-----              n.c.


Sound CPU (Z80)
---------------

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx ROM 6F    program ROM
01xxxxxxxxxxxxxx R   xxxxxxxx ROM 6E    program ROM
10----xxxxxxxxxx R/W xxxxxxxx RAM 6D    work RAM
11--------------              n.c.

I/O ports:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
--------000----x R/W xxxxxxxx OPNCS     YM2203 (portA: SSRST, unknown; portB: DIPSW3)
--------001----x R/W xxxxxxxx CIOSCS    8741 #1 (communication with main Z80, DIPSW2)
--------010----- R/W -------- INTCS     irq enable/acknowledge
--------011----x R/W xxxxxxxx AX1       8741 #2 (digital inputs, coins)
--------100----x R/W xxxxxxxx AX2       8741 #3 (digital inputs)
--------101--xxx   W -------x FCS       control filters in sound output section
--------110-----              n.c.
--------111-----   W xxxxxxxx ADCS      command to 6809


Third CPU (HD6809)
----------------

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000------------              n.c.
0001------------   W ----xxxx           MSM5205 data
0001------------   W ---x----           MSM5205 clock
0001------------   W --x-----           MSM5205 reset
0001------------   W -x------ ADBR      ROM bank select
0010------------ R   xxxxxxxx           command from Z80
0011------------              n.c.
01xxxxxxxxxxxxxx R   xxxxxxxx ROM 5P    program ROM (banked)
10xxxxxxxxxxxxxx R   xxxxxxxx ROM 5N    program ROM (banked)
11xxxxxxxxxxxxxx R   xxxxxxxx ROM 5M    program ROM (banked)


***************************************************************************

Notes:
------
- The fg tilemap is a 1bpp layer which selects the second palette bank when
  active, so it could be used for some cool effects. Gladiator just sets the
  whole palette to white so we can just treat it as a monochromatic layer.

TODO:
-----
- Tilemap Y scroll is not implemented because the game doesn't use it so I
  can't verify it's right.
- gladiatr_irq_patch_w, which triggers irq on the second CPU, is a kludge. It
  shouldn't work that way, that address should actually reset the second CPU
  (but the main CPU never asserts the line). The schematics are too fuzzy to
  understand what should trigger irq on the second CPU. Just using a vblank
  interrupt doesn't work, probably because the CPU expects interrupts to only
  begin happening when the main CPU has finished the self test.
- YM2203 mixing problems (loss of bass notes)
- YM2203 some sound effects just don't sound correct
- Audio Filter Switch not hooked up (might solve YM2203 mixing issue)
- Ports 60,61,80,81 not fully understood yet...
- Three gladiatr 8741 dumps come from an unprotected bootleg, we need dumps
  from original boards.
- 2 of the bootleg gladiatr 8741 dumps are suspected to be bad, see rom patch
  at driver init.

***************************************************************************/

#include "emu.h"
#include "gladiatr.h"

#include "machine/74259.h"
#include "machine/clock.h"
#include "machine/nvram.h"

#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "screen.h"
#include "speaker.h"

#include <algorithm>


void gladiatr_state::machine_reset()
{
	// 6809 bank memory set
	m_adpcmbank->set_entry(0);
	m_audiocpu->reset();
}

/* YM2203 port B handler (output) */
void gladiatr_state::gladiator_int_control_w(u8 data)
{
	/* bit 7   : SSRST = sound reset ? */
	/* bit 6-1 : N.C.                  */
	/* bit 0   : ??                    */
	m_ccpu->set_input_line(INPUT_LINE_RESET, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
	m_cctl->set_input_line(INPUT_LINE_RESET, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
}

/* YM2203 IRQ */
void gladiatr_state_base::ym_irq(int state)
{
	/* NMI IRQ is not used by gladiator sound program */
	m_subcpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

/*Sound Functions*/
void gladiatr_state::gladiator_adpcm_w(u8 data)
{
	// bit 6 = bank offset
	m_adpcmbank->set_entry(BIT(data, 6));

	m_msm->data_w(data);            // bit 0..3
	m_msm->reset_w(BIT(data, 5));   // bit 5
	m_msm->vclk_w(BIT(data, 4));    // bit 4
}

void ppking_state::ppking_adpcm_w(u8 data)
{
	// bit 6 = bank offset
	//m_adpcmbank->set_entry(BIT(data, 6));

	m_msm->data_w(data);            // bit 0..3
	m_msm->reset_w(BIT(data, 5));   // bit 5
	m_msm->vclk_w(BIT(data, 4));    // bit 4
}

void ppking_state::cpu2_irq_ack_w(u8 data)
{
	m_subcpu->set_input_line(0, CLEAR_LINE);
}

void gladiatr_state_base::adpcm_command_w(u8 data)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

u8 gladiatr_state_base::adpcm_command_r()
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_soundlatch->read();
}

void gladiatr_state_base::flipscreen_w(int state)
{
	flip_screen_set(state);
}

#if 1
/* !!!!! patch to IRQ timing for 2nd CPU !!!!! */
void gladiatr_state::gladiatr_irq_patch_w(u8 data)
{
	m_subcpu->set_input_line(0, HOLD_LINE);
}
#endif


void gladiatr_state::tclk_w(int state)
{
	m_tclk_val = state != 0;
}

u8 gladiatr_state::cctl_p1_r()
{
	return m_cctl_p1 & m_in2->read();
}

u8 gladiatr_state::cctl_p2_r()
{
	return m_cctl_p2;
}

u8 gladiatr_state::ucpu_p2_r()
{
	return bitswap<8>(m_dsw1->read(), 0,1,2,3,4,5,6,7);
}

void gladiatr_state::ccpu_p2_w(u8 data)
{
	// almost certainly active low (bootleg MCU never uses these outputs, which makes them always high)
	// coin counters and lockout pass through 4049 inverting buffer at 12L
	machine().bookkeeping().coin_counter_w(0, !BIT(data, 6));
	machine().bookkeeping().coin_counter_w(1, !BIT(data, 7));
}

int gladiatr_state::tclk_r()
{
	// fed to t0 on comms MCUs
	return m_tclk_val ? 1 : 0;
}

int gladiatr_state::ucpu_t1_r()
{
	// connected to p1 on other MCU
	return BIT(m_csnd_p1, 1);
}

u8 gladiatr_state::ucpu_p1_r()
{
	// p10 connected to corresponding line on other MCU
	// p11 connected to t1 on other MCU
	// other lines floating
	return m_csnd_p1 |= 0xfe;
}

void gladiatr_state::ucpu_p1_w(u8 data)
{
	m_ucpu_p1 = data;
}

int gladiatr_state::csnd_t1_r()
{
	// connected to p1 on other MCU
	return BIT(m_ucpu_p1, 1);
}

u8 gladiatr_state::csnd_p1_r()
{
	// p10 connected to corresponding line on other MCU
	// p11 connected to t1 on other MCU
	// other lines floating
	return m_ucpu_p1 |= 0xfe;
}

void gladiatr_state::csnd_p1_w(u8 data)
{
	m_csnd_p1 = data;
}

u8 gladiatr_state::csnd_p2_r()
{
	return bitswap<8>(m_dsw2->read(), 2,3,4,5,6,7,1,0);
}


INPUT_CHANGED_MEMBER(gladiatr_state::p1_s1)
{
	// P11 gets the value of 1P-S2 at the moment 1P-S1 was pressed
	if (oldval && !newval)
		m_cctl_p1 = (m_cctl_p1 & 0xfd) | (BIT(m_cctl_p1, 0) << 1);
}

INPUT_CHANGED_MEMBER(gladiatr_state::p1_s2)
{
	// P10 is high when 1P-S2 is pressed
	m_cctl_p1 = (m_cctl_p1 & 0xfe) | (newval ? 0x00 : 0x01);
}

INPUT_CHANGED_MEMBER(gladiatr_state::p2_s1)
{
	// P21 gets the value of 2P-S2 at the moment 2P-S1 was pressed
	if (oldval && !newval)
		m_cctl_p2 = (m_cctl_p2 & 0xfd) | (BIT(m_cctl_p2, 0) << 1);
}

INPUT_CHANGED_MEMBER(gladiatr_state::p2_s2)
{
	// P20 is high when 2P-S2 is pressed
	m_cctl_p2 = (m_cctl_p2 & 0xfe) | (newval ? 0x00 : 0x01);
}



u8 ppking_state::ppking_f1_r()
{
	return 0xff;
}

/**********************************
 *
 * Ping Pong King MCU simulation
 *
 * 0x8ad =
 * 0x8f6 = IO check (must return 0x40)
 *
 **********************************/

//#include "debugger.h"

inline bool ppking_state::mcu_parity_check()
{
	u8 res = 0;

	for(int i=0;i<8;i++)
	{
		if(m_mcu[0].rxd & (1 << i))
			res++;
	}

	if(res % 2)
		return false;

	return true;
}

inline void ppking_state::mcu_input_check()
{
	if(m_mcu[0].txd == 0x41)
	{
		m_mcu[0].state = 1;
		//m_mcu[0].packet_type = 0;
	}
	else if(m_mcu[0].txd == 0x42)
	{
		m_mcu[0].state = 2;
		//m_mcu[0].packet_type = 0;
		//machine().debug_break();
	}
	else if(m_mcu[0].txd == 0x44)
	{
		m_mcu[0].state = 3;
	}
}

u8 ppking_state::ppking_qx0_r(offs_t offset)
{
	// status
	if(offset == 1)
		return 1;

	if(m_mcu[0].rst)
	{
		switch(m_mcu[0].state)
		{
			case 1:
			{
				m_mcu[0].packet_type^=1;

				if(m_mcu[0].packet_type & 1)
				{
					m_mcu[0].rxd = ((ioport("DSW3")->read()) & 0x9f) | 0x20;
				}
				else
				{
					m_mcu[0].rxd = ((ioport("SYSTEM")->read()) & 0x9f);
				}

				break;
			}

			case 2:
			{
				m_mcu[0].packet_type^=1;
				if(m_mcu[0].packet_type & 1)
				{
					// Host wants this from time to time, otherwise huge input lag happens periodically (protection?)
					m_mcu[0].rxd = 0x17;
				}
				else
				{
					m_mcu[0].rxd = ((ioport("P1")->read()) & 0x3f);
					m_mcu[0].rxd |= ((ioport("SYSTEM")->read()) & 0x80);
				}

				break;
			}

			case 3:
			{
				m_mcu[0].packet_type^=1;
				if(m_mcu[0].packet_type & 1)
				{
					// same as above for player 2
					m_mcu[0].rxd = 0x17;
				}
				else
				{
					m_mcu[0].rxd = ((ioport("P2")->read()) & 0x3f);
					m_mcu[0].rxd |= ((ioport("SYSTEM")->read()) & 0x80);
				}

				break;
			}
		}

		if(mcu_parity_check() == false)
			m_mcu[0].rxd |= 0x40;
	}

	//printf("%04x rst %d\n",m_maincpu->pc(),m_mcu[0].rst);

	return m_mcu[0].rxd;
}

void ppking_state::ppking_qx0_w(offs_t offset, u8 data)
{
	if(offset == 1)
	{
		switch(data)
		{
			case 0:
				m_mcu[0].rxd = 0x40;
				m_mcu[0].rst = 0;
				m_mcu[0].state = 0;
				break;
			case 1:
				/*
				status codes:
				0x06 sub NG IOX2
				0x05 sub NG IOX1
				0x04 sub NG CIOS
				0x03 sub NG OPN
				0x02 sub NG ROM
				0x01 sub NG RAM
				0x00 ok
				*/
				m_mcu[0].rxd = 0x40;
				m_mcu[0].rst = 0;
				break;
			case 2:
				m_mcu[0].rxd = ((ioport("DSW2")->read() & 0x1f) << 2);
				m_mcu[0].rst = 0;
				break;
			case 3:
				mcu_input_check();
				m_mcu[0].rst = 1;
				break;

			default:
				printf("%02x %02x\n",offset,data);
				break;
		}
	}
	else
	{
		m_mcu[0].txd = data;

		m_mcu[1].rst = 0;
		m_soundlatch2->write(data & 0xff);

		mcu_input_check();

	}
}

void ppking_state::ppking_qx1_w(offs_t offset, u8 data)
{
	if(offset == 1)
	{
		if(data == 0xf0)
			m_mcu[1].rst = 1;
	}
}

void ppking_state::ppking_qx3_w(u8 data)
{
}


u8 ppking_state::ppking_qx1_r(offs_t offset)
{
	// status
	if(offset == 1)
		return 1;

	if(m_mcu[1].rst == 1)
		return 0x40;

	return m_soundlatch2->read();
}

u8 ppking_state::ppking_qx3_r(offs_t offset)
{
	if(offset == 1)
		return 1;

	return 0;
}

// serial communication with another board (COMU in service mode)
// NMI is used to acquire data from the other board,
// either sent via 1->0 poll of the 0xc003 port or by reading 0xc0c0 (former more likely)
u8 ppking_state::ppking_qxcomu_r(offs_t offset)
{
	if(offset == 1)
		return 1;

	return 0;
}

void ppking_state::ppking_qxcomu_w(u8 data)
{
	// ...
}

void ppking_state::machine_reset()
{
	// yes, it expects to read DSW1 without sending commands first ...
	m_mcu[0].rxd = (ioport("DSW1")->read() & 0x1f) << 2;
	m_mcu[0].rst = 0;
	m_mcu[0].state = 0;
}

void ppking_state::ppking_cpu1_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcbff).ram().share(m_spriteram);
	map(0xcc00, 0xcfff).w(FUNC(ppking_state::ppking_video_registers_w));
	map(0xd000, 0xd3ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xd400, 0xd7ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xd800, 0xdfff).ram().w(FUNC(ppking_state::videoram_w)).share(m_videoram);
	map(0xe000, 0xe7ff).ram().w(FUNC(ppking_state::colorram_w)).share(m_colorram);
	map(0xe800, 0xefff).ram().w(FUNC(ppking_state::textram_w)).share(m_textram);
	map(0xf000, 0xf7ff).ram().share("nvram"); /* battery backed RAM */
}


void ppking_state::ppking_cpu3_map(address_map &map)
{
	map(0x1000, 0x1fff).w(FUNC(ppking_state::ppking_adpcm_w));
	map(0x2000, 0x2fff).r(FUNC(ppking_state::adpcm_command_r));
	map(0x8000, 0xffff).rom().nopw();
}

void ppking_state::ppking_cpu1_io(address_map &map)
{
//  map.global_mask(0xff);
	map(0xc000, 0xc007).w("mainlatch", FUNC(ls259_device::write_d0));
//  map(0xc004, 0xc004).noprw(); //.w(FUNC(ppking_state::ppking_irq_patch_w));
	map(0xc09e, 0xc09f).r(FUNC(ppking_state::ppking_qx0_r)).w(FUNC(ppking_state::ppking_qx0_w));
	map(0xc0bf, 0xc0bf).noprw(); // watchdog
	map(0xc0c0, 0xc0c1).r(FUNC(ppking_state::ppking_qxcomu_r)).w(FUNC(ppking_state::ppking_qxcomu_w));
}

void ppking_state::ppking_cpu2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x20, 0x21).r(FUNC(ppking_state::ppking_qx1_r)).w(FUNC(ppking_state::ppking_qx1_w));
	map(0x40, 0x40).w(FUNC(ppking_state::cpu2_irq_ack_w));
	map(0x80, 0x81).rw(FUNC(ppking_state::ppking_qx3_r), FUNC(ppking_state::ppking_qx3_w));
	map(0xe0, 0xe0).w(FUNC(ppking_state::adpcm_command_w));
}




void gladiatr_state::gladiatr_cpu1_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xcbff).ram().share(m_spriteram);
	map(0xcc00, 0xcfff).w(FUNC(gladiatr_state::gladiatr_video_registers_w));
	map(0xd000, 0xd3ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xd400, 0xd7ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xd800, 0xdfff).ram().w(FUNC(gladiatr_state::videoram_w)).share(m_videoram);
	map(0xe000, 0xe7ff).ram().w(FUNC(gladiatr_state::colorram_w)).share(m_colorram);
	map(0xe800, 0xefff).ram().w(FUNC(gladiatr_state::textram_w)).share(m_textram);
	map(0xf000, 0xf7ff).ram().share("nvram"); /* battery backed RAM */
}

void gladiatr_state_base::cpu2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram();
}

void gladiatr_state::gladiatr_cpu3_map(address_map &map)
{
	map(0x1000, 0x1fff).w(FUNC(gladiatr_state::gladiator_adpcm_w));
	map(0x2000, 0x2fff).r(FUNC(gladiatr_state::adpcm_command_r));
	map(0x4000, 0xffff).bankr(m_adpcmbank).nopw();
}


void gladiatr_state::gladiatr_cpu1_io(address_map &map)
{
	map(0xc000, 0xc007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xc004, 0xc004).w(FUNC(gladiatr_state::gladiatr_irq_patch_w)); /* !!! patch to 2nd CPU IRQ !!! */
	map(0xc09e, 0xc09f).rw(m_ucpu, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0xc0bf, 0xc0bf).noprw(); // watchdog_reset_w doesn't work
}

void gladiatr_state::gladiatr_cpu2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x20, 0x21).rw(m_csnd, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0x40, 0x40).noprw(); // .w(FUNC(gladiatr_state::sub_irq_ack_w));
	map(0x60, 0x61).rw(m_cctl, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0x80, 0x81).rw(m_ccpu, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0xa0, 0xa7).w("filtlatch", FUNC(ls259_device::write_d0));
	map(0xe0, 0xe0).w(FUNC(gladiatr_state::adpcm_command_w));
}


static INPUT_PORTS_START( ppking )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:5" )
	PORT_DIPNAME( 0x08, 0x08, "VS Mode (link)" ) PORT_DIPLOCATION("SW1:4") // unemulated
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW2")
	// TODO: coinage not working (controlled by MCU)
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 5C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	// Round -> Normal/Round Free in manual, allows player to continue playing even if he loses
	PORT_DIPNAME( 0x02, 0x00, "Win Round even when losing (Cheat)" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	// Happens at reset
	PORT_DIPNAME( 0x04, 0x00, "Backup RAM Clear" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW3:4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(   0x80, IP_ACTIVE_HIGH, "SW3:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( gladiatr )
	PORT_START("DSW1")      /* (8741-0 parallel port)*/
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, "After 4 Stages" )            PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Continues ) )
	PORT_DIPSETTING(    0x04, "Ends" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:4")   /*NOTE: Actual manual has these settings reversed(typo?)! */
	PORT_DIPSETTING(    0x08, "Only at 100000" )
	PORT_DIPSETTING(    0x00, "Every 100000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")      /* (8741-1 parallel port) - Dips 6 Unused */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )            /* Listed as "Unused" */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")      /* (YM2203 port B) - Dips 5,6,7 Unused */
	PORT_DIPNAME( 0x01, 0x01, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Memory Backup" )             PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Clear" )
	PORT_DIPNAME( 0x0c, 0x0c, "Starting Stage" )            PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )            /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(   0x80, IP_ACTIVE_LOW, "SW3:8" )

	PORT_START("IN0")   // ccpu p1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )                                   PORT_CHANGED_MEMBER(DEVICE_SELF, gladiatr_state, p1_s1, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )                                   PORT_CHANGED_MEMBER(DEVICE_SELF, gladiatr_state, p1_s2, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")   // ccpu p2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY   PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY   PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY   PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY   PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )                    PORT_COCKTAIL  PORT_CHANGED_MEMBER(DEVICE_SELF, gladiatr_state, p2_s1, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )                    PORT_COCKTAIL  PORT_CHANGED_MEMBER(DEVICE_SELF, gladiatr_state, p2_s2, 0)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )                                     // coin counter outputs

	PORT_START("IN2")   // cctl p1
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )                                     // other stuff mixed here
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )                    PORT_COCKTAIL

	PORT_START("COINS") // ccpu test, cctl test
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*******************************************************************/

static const gfx_layout tilelayout  =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout  =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( gfx_ppking )
	GFXDECODE_ENTRY( "tx_tiles", 0, gfx_8x8x1,  0, 1 )
	GFXDECODE_ENTRY( "bg_tiles", 0, tilelayout, 0, 32 )
	GFXDECODE_ENTRY( "sprites",  0, spritelayout, 0x100, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_gladiatr )
	GFXDECODE_ENTRY( "tx_tiles", 0, gfx_8x8x1,    0x200, 1 )
	GFXDECODE_ENTRY( "bg_tiles", 0, tilelayout,   0x000, 32 )
	GFXDECODE_ENTRY( "sprites",  0, spritelayout, 0x100, 32 )
GFXDECODE_END



void ppking_state::ppking(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12_MHz_XTAL/2);  /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &ppking_state::ppking_cpu1_map);
	m_maincpu->set_addrmap(AS_IO, &ppking_state::ppking_cpu1_io);
	m_maincpu->set_vblank_int("screen", FUNC(ppking_state::irq0_line_hold));

	Z80(config, m_subcpu, 12_MHz_XTAL/4);   /* verified on pcb */
	m_subcpu->set_addrmap(AS_PROGRAM, &ppking_state::cpu2_map);
	m_subcpu->set_addrmap(AS_IO, &ppking_state::ppking_cpu2_io);
	m_subcpu->set_periodic_int(FUNC(ppking_state::irq0_line_assert), attotime::from_hz(60));

	MC6809(config, m_audiocpu, 12_MHz_XTAL/4);  /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &ppking_state::ppking_cpu3_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 5L on main board
	mainlatch.q_out_cb<0>().set(FUNC(ppking_state::spritebuffer_w));
//  mainlatch.q_out_cb<1>().set(FUNC(ppking_state::spritebank_w));
//  mainlatch.q_out_cb<2>().set_membank(m_mainbank);
//  mainlatch.q_out_cb<3>().set(FUNC(ppking_state::nmi_mask_w));
//  mainlatch.q_out_cb<4>().set("sub", INPUT_LINE_RESET); // shadowed by aforementioned hack
//  Q6 used
	mainlatch.q_out_cb<7>().set(FUNC(ppking_state::flipscreen_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(60);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_raw(12_MHz_XTAL/2,384,0,256,264,16,240); // assume same as Arkanoid
	screen.set_screen_update(FUNC(ppking_state::screen_update_ppking));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ppking);
	PALETTE(config, m_palette).set_format(palette_device::xBGRBBBBGGGGRRRR_bit0, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12_MHz_XTAL/8)); /* verified on pcb */
	ymsnd.irq_handler().set(FUNC(ppking_state::ym_irq));
	ymsnd.port_a_read_callback().set(FUNC(ppking_state::ppking_f1_r));
	ymsnd.port_b_read_callback().set_ioport("DSW3");
	ymsnd.add_route(0, "mono", 0.60);
	ymsnd.add_route(1, "mono", 0.60);
	ymsnd.add_route(2, "mono", 0.60);
	ymsnd.add_route(3, "mono", 0.50);

	MSM5205(config, m_msm, 455_kHz_XTAL); /* verified on pcb */
	m_msm->set_prescaler_selector(msm5205_device::SEX_4B);  /* vclk input mode */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.60);
}

void gladiatr_state::gladiatr(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12_MHz_XTAL/2);  /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &gladiatr_state::gladiatr_cpu1_map);
	m_maincpu->set_addrmap(AS_IO, &gladiatr_state::gladiatr_cpu1_io);
	m_maincpu->set_vblank_int("screen", FUNC(gladiatr_state::irq0_line_hold));

	Z80(config, m_subcpu, 12_MHz_XTAL/4);   /* verified on pcb */
	m_subcpu->set_addrmap(AS_PROGRAM, &gladiatr_state::cpu2_map);
	m_subcpu->set_addrmap(AS_IO, &gladiatr_state::gladiatr_cpu2_io);

	MC6809(config, m_audiocpu, 12_MHz_XTAL/4);  /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &gladiatr_state::gladiatr_cpu3_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // NEC uPD449 CMOS SRAM

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 5L on main board
	mainlatch.q_out_cb<0>().set(FUNC(gladiatr_state::spritebuffer_w));
	mainlatch.q_out_cb<1>().set(FUNC(gladiatr_state::spritebank_w));
	mainlatch.q_out_cb<2>().set_membank(m_mainbank);
	mainlatch.q_out_cb<4>().set_inputline("sub", INPUT_LINE_RESET); // shadowed by aforementioned hack
	mainlatch.q_out_cb<7>().set(FUNC(gladiatr_state::flipscreen_w));

	I8741A(config, m_cctl, 12_MHz_XTAL/2); /* verified on pcb */
	m_cctl->t0_in_cb().set_ioport("COINS").bit(3);
	m_cctl->t1_in_cb().set_ioport("COINS").bit(2);
	m_cctl->p1_in_cb().set(FUNC(gladiatr_state::cctl_p1_r));
	m_cctl->p2_in_cb().set(FUNC(gladiatr_state::cctl_p2_r));

	I8741A(config, m_ccpu, 12_MHz_XTAL/2); /* verified on pcb */
	m_ccpu->p1_in_cb().set_ioport("IN0");
	m_ccpu->p2_in_cb().set_ioport("IN1");
	m_ccpu->p2_out_cb().set(FUNC(gladiatr_state::ccpu_p2_w));
	m_ccpu->t0_in_cb().set_ioport("COINS").bit(1);
	m_ccpu->t1_in_cb().set_ioport("COINS").bit(0);

	I8741A(config, m_ucpu, 12_MHz_XTAL/2); /* verified on pcb */
	m_ucpu->p1_in_cb().set(FUNC(gladiatr_state::ucpu_p1_r));
	m_ucpu->p1_out_cb().set(FUNC(gladiatr_state::ucpu_p1_w));
	m_ucpu->p2_in_cb().set(FUNC(gladiatr_state::ucpu_p2_r));
	m_ucpu->t0_in_cb().set(FUNC(gladiatr_state::tclk_r));
	m_ucpu->t1_in_cb().set(FUNC(gladiatr_state::ucpu_t1_r));

	I8742(config, m_csnd, 12_MHz_XTAL/2); /* verified on pcb */
	m_csnd->p1_in_cb().set(FUNC(gladiatr_state::csnd_p1_r));
	m_csnd->p1_out_cb().set(FUNC(gladiatr_state::csnd_p1_w));
	m_csnd->p2_in_cb().set(FUNC(gladiatr_state::csnd_p2_r));
	m_csnd->t0_in_cb().set(FUNC(gladiatr_state::tclk_r));
	m_csnd->t1_in_cb().set(FUNC(gladiatr_state::csnd_t1_r));

	/* lazy way to make polled serial between MCUs work */
	config.set_perfect_quantum(m_ucpu);

	CLOCK(config, "tclk", 12_MHz_XTAL/8/128/2) /* verified on pcb */
		.signal_handler().set(FUNC(gladiatr_state::tclk_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gladiatr_state::screen_update_gladiatr));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gladiatr);
	PALETTE(config, m_palette).set_format(palette_device::xBGRBBBBGGGGRRRR_bit0, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12_MHz_XTAL/8)); /* verified on pcb */
	ymsnd.irq_handler().set(FUNC(gladiatr_state::ym_irq));
	ymsnd.port_b_read_callback().set_ioport("DSW3");
	ymsnd.port_a_write_callback().set(FUNC(gladiatr_state::gladiator_int_control_w));
	ymsnd.add_route(0, "mono", 0.60);
	ymsnd.add_route(1, "mono", 0.60);
	ymsnd.add_route(2, "mono", 0.60);
	ymsnd.add_route(3, "mono", 0.50);

	MSM5205(config, m_msm, 455_kHz_XTAL);   /* verified on pcb */
	m_msm->set_prescaler_selector(msm5205_device::SEX_4B);  /* vclk input mode */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.60);

	LS259(config, "filtlatch", 0); // 9R - filters on sound output
}

void gladiatr_state::greatgur(machine_config &config)
{
	gladiatr(config);

	I8741A(config.replace(), m_csnd, 12_MHz_XTAL/2); /* verified on pcb */
	m_csnd->p1_in_cb().set(FUNC(gladiatr_state::csnd_p1_r));
	m_csnd->p1_out_cb().set(FUNC(gladiatr_state::csnd_p1_w));
	m_csnd->p2_in_cb().set(FUNC(gladiatr_state::csnd_p2_r));
	m_csnd->t0_in_cb().set(FUNC(gladiatr_state::tclk_r));
	m_csnd->t1_in_cb().set(FUNC(gladiatr_state::csnd_t1_r));
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ppking )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "q1_1.a1",        0x00000, 0x2000, CRC(b74b2718) SHA1(29833439211076873324ccfa5897eb1e6aa9d134) )
	ROM_LOAD( "q1_2.b1",        0x02000, 0x2000, CRC(1b1e4cd4) SHA1(34c6cf5e0775c0c834dda34a3a2a4685465daa8e) )
	ROM_LOAD( "q0_3.c1",        0x04000, 0x2000, CRC(6a7acf8e) SHA1(06d37e813605f507ea1c720764fc554e58defdf8) )
	ROM_LOAD( "q0_4.d1",        0x06000, 0x2000, CRC(b83eb6d5) SHA1(f112d3c0d701977dcc5c312ad74d78b44882201b) )
	ROM_LOAD( "q0_5.e1",        0x08000, 0x4000, CRC(4d2007e2) SHA1(973ef0e6ff6065b753402489a3d10a9b68164969) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "q0_17.6f",       0x0000, 0x2000, CRC(f7fe0d24) SHA1(6dcb23aa7fc08fc892a8b3843ccb982997c20571) )
	ROM_LOAD( "q0_16.6e",       0x4000, 0x2000, CRC(b1e32588) SHA1(13c74479238a34a08e249f9120b42a52d80f8274) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "q0_19.5n",       0x0a000, 0x2000, CRC(4bcf896d) SHA1(f587a66fcc63e989742ce2d5f4cf2bb464987038) )
	ROM_RELOAD(                 0x08000, 0x2000 )
	ROM_LOAD( "q0_18.5m",       0x0e000, 0x2000, CRC(89ba64f8) SHA1(fa01316ea744b4277ee64d5f14cb6d7e3a949f2b) )
	ROM_RELOAD(                 0x0c000, 0x2000 )

	ROM_REGION( 0x02000, "tx_tiles", 0 )
	ROM_LOAD( "q0_15.1r",       0x00000, 0x2000, CRC(fbd33219) SHA1(78b9bb327ededaa818d26c41c5e8fd1c041ef142) )

	ROM_REGION( 0x8000, "bg_tiles", 0 )
	ROM_LOAD( "q0_12.1j",       0x00000, 0x2000, CRC(b1a44482) SHA1(84cc40976aa9b015a9f970a878bbde753651b3ba) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "q0_13.1k",       0x04000, 0x2000, CRC(468f35e6) SHA1(8e28481910663fe525cefd4ad406468b7736900e) ) /* planes 1,2 */
	ROM_LOAD( "q0_14.1m",       0x06000, 0x2000, CRC(eed04a7f) SHA1(d139920889653c33ded38a85510789380dd0aa9e) ) /* planes 1,2 */

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "q0_6.k1",        0x00000, 0x2000, CRC(bb3d666c) SHA1(a689c7a1e75b916d69f396db7c4688ac355c2aff) ) /* plane 3 */
	ROM_LOAD( "q0_7.l1",        0x02000, 0x2000, CRC(16a2550e) SHA1(adb54b70a6db660b5f29ad66da02afd8e99884bb) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "q0_8.m1",        0x08000, 0x2000, CRC(41235b22) SHA1(4d9702efe0ea320dab7c0d889f4d03f196b32661) ) /* planes 1,2 */
	ROM_LOAD( "q0_9.p1",        0x0a000, 0x2000, CRC(74cc94b2) SHA1(2cb981ecb2487dfa5c0974e036106fc06c2c1880) ) /* planes 1,2 */
	ROM_LOAD( "q0_10.r1",       0x0c000, 0x2000, CRC(af438cc6) SHA1(cf79c8d3f2a0c489a756b341f8df747f6654764b) ) /* planes 1,2 */
	/* empty! */

	ROM_REGION( 0x040, "proms", 0 )
	ROM_LOAD( "q1",             0x0000, 0x0020, CRC(cca9ae7b) SHA1(e18416fbe2a5b09db749cc9a14fa89186ed8f4ba) )
	ROM_LOAD( "q2",             0x0020, 0x0020, CRC(da952b5e) SHA1(0863ad8fdcf69435a7455cd345d3d0af0b0c0a0a) )
ROM_END


ROM_START( gladiatr )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "qb0_5",          0x00000, 0x4000, CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) )
	ROM_LOAD( "qb0_4",          0x04000, 0x2000, CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) )
	ROM_LOAD( "qb0_1",          0x10000, 0x2000, CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) )
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "qc0_3",          0x12000, 0x4000, CRC(8d182326) SHA1(f0af3757c2cf9e1e8035272567adee6efc733319) )
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "tx_tiles", 0 )
	ROM_LOAD( "qc0_15",         0x00000, 0x2000, CRC(a7efa340) SHA1(f87e061b8e4d8cd0834fab301779a8493549419b) ) /* (monochrome) */

	ROM_REGION( 0x20000, "bg_tiles", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "sprites", 0 )    /* sprites */
	ROM_LOAD( "qc1_6",          0x00000, 0x4000, CRC(651e6e44) SHA1(78ce576e6c29e43d590c42f0d4926cff82fd0268) ) /* plane 3 */
	ROM_LOAD( "qc2_7",          0x04000, 0x8000, CRC(c992c4f7) SHA1(3263973474af07c8b93c4ec97924568848cb7201) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "qc1_9",          0x1c000, 0x4000, CRC(01043e03) SHA1(6a6dddc0a036873135dceaa989e757bdd2455ae7) ) /* planes 1,2 */
	ROM_LOAD( "qc1_10",         0x20000, 0x8000, CRC(364cdb58) SHA1(4d8548f9dfa9d105dd277c61cf3d56583a5ebbcb) ) /* planes 1,2 */
	ROM_LOAD( "qc2_11",         0x28000, 0x8000, CRC(c9fecfff) SHA1(7c13ace4293fbfab7fe924b7b24c498d8cefc7ac) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 )   /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )

	ROM_REGION( 0x0400, "cctl", 0 ) /* I/O MCU */
	ROM_LOAD( "aq_002.9b",      0x00000, 0x0400, CRC(b30d225f) SHA1(f383286530975c440589c276aa8c46fdfe5292b6) BAD_DUMP )

	ROM_REGION( 0x0400, "ccpu", 0 ) /* I/O MCU */
	ROM_LOAD( "aq_003.xx",      0x00000, 0x0400, CRC(1d02cd5f) SHA1(f7242039788c66a1d91b01852d7d447330b847c4) BAD_DUMP )

	ROM_REGION( 0x0400, "ucpu", 0 ) /* comms MCU */
	ROM_LOAD( "aq_006.3a",      0x00000, 0x0400, CRC(3c5ca4c6) SHA1(0d8c2e1c2142ada11e30cfb9a48663386fee9cb8) BAD_DUMP )

	ROM_REGION( 0x0800, "csnd", 0 ) /* comms MCU */
	ROM_LOAD( "aq_007.6c",      0x00000, 0x0800, CRC(f19af04d) SHA1(61105cb905128e5d10b2e97d6201034584eb1ada) )
ROM_END

ROM_START( ogonsiro )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "qb0_5",          0x00000, 0x4000, CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) )
	ROM_LOAD( "qb0_4",          0x04000, 0x2000, CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) )
	ROM_LOAD( "qb0_1",          0x10000, 0x2000, CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) )
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "qb0_3",          0x12000, 0x4000, CRC(d6a342e7) SHA1(96274ae3bda4679108a25fcc514b625552abda30) )
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "tx_tiles", 0 )
	ROM_LOAD( "qb0_15",         0x00000, 0x2000, CRC(5e1332b8) SHA1(fab6e2c7ea9bc94c1245bf759b4004a70c57d666) ) /* (monochrome) */

	ROM_REGION( 0x20000, "bg_tiles", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "sprites", 0 )    /* sprites */
	ROM_LOAD( "qb0_6",          0x00000, 0x4000, CRC(1a2bc769) SHA1(498861f4d0cffeaff90609c8000c921a114756b6) ) /* plane 3 */
	ROM_LOAD( "qb0_7",          0x04000, 0x8000, CRC(4b677bd9) SHA1(3314ef58ff5307faf0ecd8f99950d43d571c91a6) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "qb0_9",          0x1c000, 0x4000, CRC(38f5152d) SHA1(fbb7b13a625999807d180a3212e6e12870629438) ) /* planes 1,2 */
	ROM_LOAD( "qb0_10",         0x20000, 0x8000, CRC(87ab6cc4) SHA1(50bc1108ff5609c0e7dad615e92e16eb72b7bc03) ) /* planes 1,2 */
	ROM_LOAD( "qb0_11",         0x28000, 0x8000, CRC(25eaa4ff) SHA1(3547fc600a617ba7fe5240a7830edb90230b6c51) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 ) /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )

	ROM_REGION( 0x0400, "cctl", 0 ) /* I/O MCU */
	ROM_LOAD( "aq_002.9b",      0x00000, 0x0400, CRC(b30d225f) SHA1(f383286530975c440589c276aa8c46fdfe5292b6) BAD_DUMP )

	ROM_REGION( 0x0400, "ccpu", 0 ) /* I/O MCU */
	ROM_LOAD( "aq_003.xx",      0x00000, 0x0400, CRC(1d02cd5f) SHA1(f7242039788c66a1d91b01852d7d447330b847c4) BAD_DUMP )

	ROM_REGION( 0x0400, "ucpu", 0 ) /* comms MCU */
	ROM_LOAD( "aq_006.3a",      0x00000, 0x0400, CRC(3c5ca4c6) SHA1(0d8c2e1c2142ada11e30cfb9a48663386fee9cb8) BAD_DUMP )

	ROM_REGION( 0x0800, "csnd", 0 ) /* comms MCU */
	ROM_LOAD( "aq_007.6c",      0x00000, 0x0800, CRC(f19af04d) SHA1(61105cb905128e5d10b2e97d6201034584eb1ada) )
ROM_END

ROM_START( greatgur )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "qb0_5",          0x00000, 0x4000, CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) )
	ROM_LOAD( "qb0_4",          0x04000, 0x2000, CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) )
	ROM_LOAD( "qb0_1",          0x10000, 0x2000, CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) )
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "qb0_3",          0x12000, 0x4000, CRC(d6a342e7) SHA1(96274ae3bda4679108a25fcc514b625552abda30) )
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "tx_tiles", 0 )
	ROM_LOAD( "qb0_15",         0x00000, 0x2000, CRC(5e1332b8) SHA1(fab6e2c7ea9bc94c1245bf759b4004a70c57d666) ) /* (monochrome) */

	ROM_REGION( 0x20000, "bg_tiles", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "sprites", 0 )    /* sprites */
	ROM_LOAD( "qc0_06.bin",     0x00000, 0x4000, CRC(96b20201) SHA1(212270d3ba72974f22e96744c752860cc5ffba5b) ) /* plane 3 */
	ROM_LOAD( "qc0_07.bin",     0x04000, 0x8000, CRC(9e89fa8f) SHA1(b133ae2ac62f43a7a51fa0d1a023a4f95fef2996) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "qc0_09.bin",     0x1c000, 0x4000, CRC(204cd385) SHA1(e7a8720feeac8ced581d72190345daed5750379f) ) /* planes 1,2 */
	ROM_LOAD( "qc1_10",         0x20000, 0x8000, CRC(364cdb58) SHA1(4d8548f9dfa9d105dd277c61cf3d56583a5ebbcb) ) /* planes 1,2 */
	ROM_LOAD( "qc1_11.bin",     0x28000, 0x8000, CRC(b2aabbf5) SHA1(9eb4d80f38a30f6e45231a9bfd1aff7a124c6ee9) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 ) /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )

	ROM_REGION( 0x0400, "cctl", 0 ) /* I/O MCU */
	ROM_LOAD( "gladcctl.1",     0x00000, 0x0400, CRC(b30d225f) SHA1(f383286530975c440589c276aa8c46fdfe5292b6) BAD_DUMP )

	ROM_REGION( 0x0400, "ccpu", 0 ) /* I/O MCU */
	ROM_LOAD( "gladccpu.2",     0x00000, 0x0400, CRC(1d02cd5f) SHA1(f7242039788c66a1d91b01852d7d447330b847c4) BAD_DUMP )

	ROM_REGION( 0x0400, "ucpu", 0 ) /* comms MCU */
	ROM_LOAD( "gladucpu.17",    0x00000, 0x0400, CRC(3c5ca4c6) SHA1(0d8c2e1c2142ada11e30cfb9a48663386fee9cb8) )

	ROM_REGION( 0x0400, "csnd", 0 ) /* comms MCU */
	ROM_LOAD( "gladcsnd.18",    0x00000, 0x0400, CRC(3c5ca4c6) SHA1(0d8c2e1c2142ada11e30cfb9a48663386fee9cb8) )
ROM_END

ROM_START( gcastle )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "gc0_5",          0x00000, 0x4000, CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) BAD_DUMP ) // not dumped, placeholder taken from ogonsiro
	ROM_LOAD( "gc0_4",          0x04000, 0x2000, CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) BAD_DUMP ) // "
	ROM_LOAD( "gc0_1",          0x10000, 0x2000, CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) BAD_DUMP ) // "
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "gc0_3",          0x12000, 0x4000, CRC(d6a342e7) SHA1(96274ae3bda4679108a25fcc514b625552abda30) BAD_DUMP ) // "
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "tx_tiles", 0 )
	ROM_LOAD( "qb0_15",         0x00000, 0x2000, CRC(5e1332b8) SHA1(fab6e2c7ea9bc94c1245bf759b4004a70c57d666) ) /* (monochrome) */

	ROM_REGION( 0x20000, "bg_tiles", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "sprites", 0 )    /* sprites */
	ROM_LOAD( "gc1_6",          0x00000, 0x4000, CRC(94f49be2) SHA1(adc9f38469d32eee5906b37289245df062b134b4) ) /* plane 3 */
	ROM_LOAD( "gc2_7",          0x04000, 0x8000, CRC(bb2cb454) SHA1(3cac1716a5c90953117deadcc3eba02000cda7c0) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "gc1_9",          0x1c000, 0x4000, CRC(69b977fd) SHA1(9d05200e2b57577f7f353853cbbaa59bfd7a2804) ) /* planes 1,2 */
	ROM_LOAD( "qb0_10",         0x20000, 0x8000, CRC(87ab6cc4) SHA1(50bc1108ff5609c0e7dad615e92e16eb72b7bc03) ) /* planes 1,2 */
	ROM_LOAD( "gc2_11",         0x28000, 0x8000, CRC(5c512365) SHA1(f6b283ed7ec6f530b9c0f2e1e29c1a766af20a1e) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 ) /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )

	ROM_REGION( 0x0400, "cctl", 0 ) /* I/O MCU */
	ROM_LOAD( "aq_002.9b",      0x00000, 0x0400, CRC(b30d225f) SHA1(f383286530975c440589c276aa8c46fdfe5292b6) BAD_DUMP )

	ROM_REGION( 0x0400, "ccpu", 0 ) /* I/O MCU */
	ROM_LOAD( "aq_003.xx",      0x00000, 0x0400, CRC(1d02cd5f) SHA1(f7242039788c66a1d91b01852d7d447330b847c4) BAD_DUMP )

	ROM_REGION( 0x0400, "ucpu", 0 ) /* comms MCU */
	ROM_LOAD( "aq_006.3a",      0x00000, 0x0400, CRC(3c5ca4c6) SHA1(0d8c2e1c2142ada11e30cfb9a48663386fee9cb8) BAD_DUMP )

	ROM_REGION( 0x0800, "csnd", 0 ) /* comms MCU */
	ROM_LOAD( "aq_007.6c",      0x00000, 0x0800, CRC(f19af04d) SHA1(61105cb905128e5d10b2e97d6201034584eb1ada) )
ROM_END


void gladiatr_state::init_gladiatr()
{
	// unpack 3bpp graphics
	u8 *rom = memregion("bg_tiles")->base();
	for (int j = 3; j >= 0; j--)
	{
		for (int i = 0; i < 0x2000; i++)
		{
			rom[i+(2*j+1)*0x2000] = rom[i+j*0x2000] >> 4;
			rom[i+2*j*0x2000] = rom[i+j*0x2000];
		}
	}
	// sort data
	std::swap_ranges(rom + 0x14000, rom + 0x18000, rom + 0x18000);

	// unpack 3bpp graphics
	rom = memregion("sprites")->base();
	for (int j = 5; j >= 0; j--)
	{
		for (int i = 0; i < 0x2000; i++)
		{
			rom[i+(2*j+1)*0x2000] = rom[i+j*0x2000] >> 4;
			rom[i+2*j*0x2000] = rom[i+j*0x2000];
		}
	}
	// sort data
	std::swap_ranges(rom + 0x1a000, rom + 0x1c000, rom + 0x1c000);
	std::swap_ranges(rom + 0x22000, rom + 0x24000, rom + 0x28000);
	std::swap_ranges(rom + 0x26000, rom + 0x28000, rom + 0x2c000);
	std::swap_ranges(rom + 0x24000, rom + 0x28000, rom + 0x28000);

	// 2 of the bootleg MCU ROM dumps are suspected to be bad, this patch changes
	// the 1st byte from 0x26 to 0x22. The init routine makes more sense with it
	// and becomes identical to the bootleg ucpu/csnd init routine. Without it,
	// the game starts with 1 credit inserted.
	*memregion("cctl")->base() = 0x22;
	*memregion("ccpu")->base() = 0x22;

	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x6000);
	m_adpcmbank->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0xc000);

	// make sure bank is valid in cpu-reset
	m_adpcmbank->set_entry(0);

	m_tclk_val = false;
	m_cctl_p1 = 0xff;
	m_cctl_p2 = 0xff;
	m_ucpu_p1 = 0xff;
	m_csnd_p1 = 0xff;

	save_item(NAME(m_tclk_val));
	save_item(NAME(m_cctl_p1));
	save_item(NAME(m_cctl_p2));
	save_item(NAME(m_ucpu_p1));
	save_item(NAME(m_csnd_p1));
}


void ppking_state::init_ppking()
{
	// unpack 3bpp graphics
	u8 *rom = memregion("bg_tiles")->base();
	for (int i = 0; i < 0x2000; i++)
	{
		rom[i+0x2000] = rom[i] >> 4;
	}

	// unpack 3bpp graphics
	rom = memregion("sprites")->base();
	for (int j = 1; j >= 0; j--)
	{
		for (int i = 0; i < 0x2000; i++)
		{
			rom[i+(2*j+1)*0x2000] = rom[i+j*0x2000] >> 4;
			rom[i+2*j*0x2000] = rom[i+j*0x2000];
		}
	}

	// patch audio CPU crash + ROM checksums
	rom = memregion("sub")->base();
	rom[0x1b9] = 0x00;
	rom[0x1ba] = 0x00;
	rom[0x1bb] = 0x00;
	rom[0x839] = 0x00;
	rom[0x83a] = 0x00;
	rom[0x83b] = 0x00;
	rom[0x845] = 0x00;
	rom[0x846] = 0x00;
	rom[0x847] = 0x00;
}


//    year  name      parent    machine   input     class           init           rot    company                                fullname                                         flags
GAME( 1985, ppking,   0,        ppking,   ppking,   ppking_state,   init_ppking,   ROT90, "Taito America Corporation",           "Ping-Pong King",                                MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL | MACHINE_NODEVICE_LAN )
GAME( 1986, gladiatr, 0,        gladiatr, gladiatr, gladiatr_state, init_gladiatr, ROT0,  "Allumer / Taito America Corporation", "Gladiator (US)",                                MACHINE_SUPPORTS_SAVE )
GAME( 1986, ogonsiro, gladiatr, gladiatr, gladiatr, gladiatr_state, init_gladiatr, ROT0,  "Allumer / Taito Corporation",         "Ougon no Shiro (Japan)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1986, greatgur, gladiatr, greatgur, gladiatr, gladiatr_state, init_gladiatr, ROT0,  "Allumer / Taito Corporation",         "Great Gurianos (bootleg?)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1986, gcastle,  gladiatr, gladiatr, gladiatr, gladiatr_state, init_gladiatr, ROT0,  "Allumer / Taito Corporation",         "Golden Castle (prototype?)",                    MACHINE_SUPPORTS_SAVE ) // incomplete dump
