// license:BSD-3-Clause
// copyright-holders:James Wallace
// thanks-to: 'Re-Animator'
/****************************************************************************************

    bfm_sc2.c

    Bellfruit scorpion2/3 driver, (under heavy construction !!!)

*****************************************************************************************

     04-2011: J Wallace: Fixed watchdog to match actual circuit, also fixed lamping code.
  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware setup.
  18-01-2006: Cleaned up for MAME inclusion
  19-08-2005: Re-Animator

Standard scorpion2 memorymap
The hardware in Scorpion 2 is effectively a Scorpion 1 board with better, non-compatible
microcontrollers, incorporating many of the old expansions on board.

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k) battery backed up
-----------+---+-----------------+-----------------------------------------
2000-20FF  | W | D D D D D D D D | Reel 1 + 2 stepper latch
-----------+---+-----------------+-----------------------------------------
2000       | R | D D D D D D D D | vfd status
-----------+---+-----------------+-----------------------------------------
2100-21FF  | W | D D D D D D D D | Reel 3 + 4 stepper latch
-----------+---+-----------------+-----------------------------------------
2200-22FF  | W | D D D D D D D D | Reel 5 + 6 stepper latch
-----------+---+-----------------+-----------------------------------------
2300-231F  | W | D D D D D D D D | output mux
-----------+---+-----------------+-----------------------------------------
2300-230B  | R | D D D D D D D D | input mux
-----------+---+-----------------+-----------------------------------------
2320       |R/W| D D D D D D D D | dimas0 ?
-----------+---+-----------------+-----------------------------------------
2321       |R/W| D D D D D D D D | dimas1 ?
-----------+---+-----------------+-----------------------------------------
2322       |R/W| D D D D D D D D | dimas2 ?
-----------+---+-----------------+-----------------------------------------
2323       |R/W| D D D D D D D D | dimas3 ?
-----------+---+-----------------+-----------------------------------------
2324       |R/W| D D D D D D D D | expansion latch
-----------+---+-----------------+-----------------------------------------
2325       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2326       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2327       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2328       |R/W| D D D D D D D D | muxena
-----------+---+-----------------+-----------------------------------------
2329       | W | D D D D D D D D | Timer IRQ enable
-----------+---+-----------------+-----------------------------------------
232A       |R/W| D D D D D D D D | blkdiv ?
-----------+---+-----------------+-----------------------------------------
232B       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
232C       |R/W| D D D D D D D D | dimena ?
-----------+---+-----------------+-----------------------------------------
232D       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
232E       | R | D D D D D D D D | chip status b0 = IRQ status
-----------+---+-----------------+-----------------------------------------
232F       | W | D D D D D D D D | coin inhibits
-----------+---+-----------------+-----------------------------------------
2330       | W | D D D D D D D D | payout slide latch
-----------+---+-----------------+-----------------------------------------
2331       | W | D D D D D D D D | payout triac latch
-----------+---+-----------------+-----------------------------------------
2332       |R/W| D D D D D D D D | Watchdog timer
-----------+---+-----------------+-----------------------------------------
2333       | W | D D D D D D D D | electro mechanical meters
-----------+---+-----------------+-----------------------------------------
2334       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2335       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2336       |?/W| D D D D D D D D | dimcnt ?
-----------+---+-----------------+-----------------------------------------
2337       | W | D D D D D D D D | volume override
-----------+---+-----------------+-----------------------------------------
2338       | W | D D D D D D D D | payout chip select
-----------+---+-----------------+-----------------------------------------
2339       | W | D D D D D D D D | clkden ?
-----------+---+-----------------+-----------------------------------------
2400       |R/W| D D D D D D D D | uart1 (MC6850 compatible) control/status
-----------+---+-----------------+-----------------------------------------
2500       |R/W| D D D D D D D D | uart1 (MC6850 compatible) data
-----------+---+-----------------+-----------------------------------------
2600       |R/W| D D D D D D D D | uart2 (MC6850 compatible) control/status
-----------+---+-----------------+-----------------------------------------
2700       |R/W| D D D D D D D D | uart2 (MC6850 compatible) data
-----------+---+-----------------+-----------------------------------------
2800       |R/W| D D D D D D D D | vfd1
-----------+---+-----------------+-----------------------------------------
2900       |R/W| D D D D D D D D | reset vfd1 + vfd2
-----------+---+-----------------+-----------------------------------------
2D00       |R/W| D D D D D D D D | ym2413 control
-----------+---+-----------------+-----------------------------------------
2D01       |R/W| D D D D D D D D | ym2413 data
-----------+---+-----------------+-----------------------------------------
2E00       |R/W| D D D D D D D D | ROM page latch
-----------+---+-----------------+-----------------------------------------
2F00       |R/W| D D D D D D D D | vfd2
-----------+---+-----------------+-----------------------------------------
3FFE       | R | D D D D D D D D | direct input1
-----------+---+-----------------+-----------------------------------------
3FFF       | R | D D D D D D D D | direct input2
-----------+---+-----------------+-----------------------------------------
2A00       | W | D D D D D D D D | NEC uPD7759 data
-----------+---+-----------------+-----------------------------------------
2B00       | W | D D D D D D D D | NEC uPD7759 reset
-----------+---+-----------------+-----------------------------------------
4000-5FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-----------------------------------------
6000-7FFF  | R | D D D D D D D D | Paged ROM (8k)
           |   |                 |   page 0 : rom area 0x0000 - 0x1FFF
           |   |                 |   page 1 : rom area 0x2000 - 0x3FFF
           |   |                 |   page 2 : rom area 0x4000 - 0x5FFF
           |   |                 |   page 3 : rom area 0x6000 - 0x7FFF
-----------+---+-----------------+-----------------------------------------
8000-FFFF  | R | D D D D D D D D | ROM (32k)
-----------+---+-----------------+-----------------------------------------

Adder hardware:
    Games supported:
        * Quintoon (2 sets Dutch, 2 sets UK)
        * Pokio (1 set)
        * Paradice (1 set)
        * Pyramid (1 set)
        * Slots (1 set Dutch, 2 sets Belgian)
        * Golden Crown (1 Set)

    Known issues:
        * Need to find the 'missing' game numbers
        * Fix RS232 protocol
***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"

#include "machine/nvram.h"

#include "video/bfm_adr2.h"

#include "sound/2413intf.h"
#include "sound/upd7759.h"

/* fruit machines only */
#include "video/awpvid.h"
#include "machine/steppers.h" // stepper motor

#include "machine/bfm_bd1.h"  // vfd
#include "machine/meters.h"

#include "sc2_vid.lh"
#include "gldncrwn.lh"
#include "paradice.lh"
#include "pokio.lh"
#include "pyramid.lh"
#include "quintoon.lh"
#include "sltblgpo.lh"
#include "sltblgtk.lh"
#include "slots.lh"

/* fruit machines only */
#include "video/bfm_dm01.h"
#include "sc2_vfd.lh"
#include "sc2_dmd.lh"
#include "drwho.lh"
#include "machine/bfm_comn.h"


class bfm_sc2_state : public driver_device
{
public:
	bfm_sc2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_reel0(*this, "reel0"),
			m_reel1(*this, "reel1"),
			m_reel2(*this, "reel2"),
			m_reel3(*this, "reel3"),
			m_reel4(*this, "reel4"),
			m_reel5(*this, "reel5"),
			m_upd7759(*this, "upd"),
			m_vfd0(*this, "vfd0"),
			m_vfd1(*this, "vfd1"),
			m_dm01(*this, "dm01"),
			m_meters(*this, "meters") { }

	required_device<cpu_device> m_maincpu;
	optional_device<stepper_device> m_reel0;
	optional_device<stepper_device> m_reel1;
	optional_device<stepper_device> m_reel2;
	optional_device<stepper_device> m_reel3;
	optional_device<stepper_device> m_reel4;
	optional_device<stepper_device> m_reel5;
	required_device<upd7759_device> m_upd7759;
	optional_device<bfm_bd1_t> m_vfd0;
	optional_device<bfm_bd1_t> m_vfd1;
	optional_device<bfmdm01_device> m_dm01;
	optional_device<meters_device> m_meters; // scorpion2_vid doesn't use this (scorpion2_vidm does)

	int m_sc2gui_update_mmtr; //not used?
	UINT8 *m_nvram;
	UINT8 m_key[8];
	UINT8 m_e2ram[1024];
	int m_mmtr_latch;
	int m_irq_status;
	int m_optic_pattern;
	DECLARE_WRITE_LINE_MEMBER(reel0_optic_cb) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	DECLARE_WRITE_LINE_MEMBER(reel1_optic_cb) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	DECLARE_WRITE_LINE_MEMBER(reel2_optic_cb) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	DECLARE_WRITE_LINE_MEMBER(reel3_optic_cb) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }
	DECLARE_WRITE_LINE_MEMBER(reel4_optic_cb) { if (state) m_optic_pattern |= 0x10; else m_optic_pattern &= ~0x10; }
	DECLARE_WRITE_LINE_MEMBER(reel5_optic_cb) { if (state) m_optic_pattern |= 0x20; else m_optic_pattern &= ~0x20; }
	int m_uart1_data;
	int m_uart2_data;
	int m_data_to_uart1;
	int m_data_to_uart2;
	int m_is_timer_enabled;
	int m_coin_inhibits;
	int m_irq_timer_stat;
	int m_expansion_latch;
	int m_global_volume;
	int m_volume_override;
	int m_sc2_show_door;
	int m_sc2_door_state;
	int m_reels;
	int m_reel12_latch;
	int m_reel34_latch;
	int m_reel56_latch;
	int m_pay_latch;
	int m_slide_states[6];
	int m_slide_pay_sensor[6];
	int m_has_hopper;
	int m_triac_select;
	int m_hopper_running;
	int m_hopper_coin_sense;
	int m_timercnt;
	UINT8 m_sc2_Inputs[64];
	UINT8 m_input_override[64];
	int m_e2reg;
	int m_e2state;
	int m_e2cnt;
	int m_e2data;
	int m_e2address;
	int m_e2rw;
	int m_e2data_pin;
	int m_e2dummywrite;
	int m_e2data_to_read;
	UINT8 m_codec_data[256];
	void e2ram_init(nvram_device &nvram, void *data, size_t size);
	DECLARE_WRITE_LINE_MEMBER(bfmdm01_busy);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(reel12_vid_w);
	DECLARE_WRITE8_MEMBER(reel12_w);
	DECLARE_WRITE8_MEMBER(reel34_w);
	DECLARE_WRITE8_MEMBER(reel56_w);
	DECLARE_WRITE8_MEMBER(mmtr_w);
	DECLARE_WRITE8_MEMBER(mux_output_w);
	DECLARE_READ8_MEMBER(mux_input_r);
	DECLARE_WRITE8_MEMBER(unlock_w);
	DECLARE_WRITE8_MEMBER(dimas_w);
	DECLARE_WRITE8_MEMBER(dimcnt_w);
	DECLARE_WRITE8_MEMBER(unknown_w);
	DECLARE_WRITE8_MEMBER(volume_override_w);
	DECLARE_READ8_MEMBER(vfd_status_hop_r);
	DECLARE_WRITE8_MEMBER(expansion_latch_w);
	DECLARE_READ8_MEMBER(expansion_latch_r);
	DECLARE_WRITE8_MEMBER(muxena_w);
	DECLARE_WRITE8_MEMBER(timerirq_w);
	DECLARE_READ8_MEMBER(timerirqclr_r);
	DECLARE_READ8_MEMBER(irqstatus_r);
	DECLARE_WRITE8_MEMBER(coininhib_w);
	DECLARE_READ8_MEMBER(coin_input_r);
	DECLARE_WRITE8_MEMBER(payout_latch_w);
	DECLARE_WRITE8_MEMBER(payout_triac_w);
	DECLARE_WRITE8_MEMBER(payout_select_w);
	DECLARE_WRITE8_MEMBER(vfd_reset_w);
	DECLARE_READ8_MEMBER(uart1stat_r);
	DECLARE_READ8_MEMBER(uart1data_r);
	DECLARE_WRITE8_MEMBER(uart1ctrl_w);
	DECLARE_WRITE8_MEMBER(uart1data_w);
	DECLARE_READ8_MEMBER(uart2stat_r);
	DECLARE_READ8_MEMBER(uart2data_r);
	DECLARE_WRITE8_MEMBER(uart2ctrl_w);
	DECLARE_WRITE8_MEMBER(uart2data_w);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_READ8_MEMBER(vfd_status_r);
	DECLARE_WRITE8_MEMBER(vfd1_bd1_w);
	DECLARE_WRITE8_MEMBER(vfd1_dmd_w);
	DECLARE_WRITE8_MEMBER(dmd_reset_w);
	DECLARE_WRITE8_MEMBER(vfd2_data_w);
	DECLARE_WRITE8_MEMBER(e2ram_w);
	DECLARE_READ8_MEMBER(direct_input_r);
	int recdata(int changed, int data);
	DECLARE_WRITE8_MEMBER(nec_reset_w);
	DECLARE_WRITE8_MEMBER(nec_latch_w);
	DECLARE_DRIVER_INIT(sltsbelg);
	DECLARE_DRIVER_INIT(pyramid);
	DECLARE_DRIVER_INIT(gldncrwn);
	DECLARE_DRIVER_INIT(bbrkfst);
	DECLARE_DRIVER_INIT(ofah);
	DECLARE_DRIVER_INIT(quintoon);
	DECLARE_DRIVER_INIT(drwhon);
	DECLARE_DRIVER_INIT(adder_dutch);
	DECLARE_DRIVER_INIT(bfmcgslm);
	DECLARE_DRIVER_INIT(luvjub);
	DECLARE_DRIVER_INIT(prom);
	DECLARE_DRIVER_INIT(cpeno1);
	DECLARE_DRIVER_INIT(focus);
	DECLARE_DRIVER_INIT(drwho);
	DECLARE_DRIVER_INIT(drwho_common);
	DECLARE_MACHINE_START(bfm_sc2);
	DECLARE_MACHINE_RESET(init);
	DECLARE_MACHINE_RESET(awp_init);
	DECLARE_MACHINE_START(sc2dmd);
	DECLARE_MACHINE_RESET(dm01_init);
	INTERRUPT_GEN_MEMBER(timer_irq);
	void on_scorpion2_reset();
	void Scorpion2_SetSwitchState(int strobe, int data, int state);
	int Scorpion2_GetSwitchState(int strobe, int data);
	void e2ram_reset();
	int recAck(int changed, int data);
	int read_e2ram();
	int sc2_find_project_string( );
	void sc2_common_init(int decrypt);
	void adder2_common_init();
	void sc2awp_common_init(int reels, int decrypt);
	void sc2awpdmd_common_init(int reels, int decrypt);
	void save_state();
};


#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

// log serial communication between mainboard (scorpion2) and videoboard (adder2)
#define LOG_SERIAL(x) do { if (VERBOSE) logerror x; } while (0)
#define UART_LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define MASTER_CLOCK        (XTAL_8MHz)

/*      INPUTS layout

     b7 b6 b5 b4 b3 b2 b1 b0

     82 81 80 04 03 02 01 00  0
     92 91 90 14 13 12 11 10  1
     A2 A1 A0 24 23 22 21 20  2
     B2 B1 B0 34 33 32 31 30  3
     -- 84 83 44 43 42 41 40  4
     -- 94 93 54 53 52 51 50  5
     -- A4 A3 64 63 62 61 60  6
     -- B4 B3 74 73 72 71 70  7

     B7 B6 B5 B4 B3 B2 B1 B0
      0  1  1  0  0  0

*/

///////////////////////////////////////////////////////////////////////////
// called if board is reset ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void bfm_sc2_state::on_scorpion2_reset()
{
	m_mmtr_latch        = 0;
	m_irq_status        = 0;
	m_is_timer_enabled  = 1;
	m_coin_inhibits     = 0;
	m_irq_timer_stat    = 0;
	m_expansion_latch   = 0;
	m_global_volume     = 0;
	m_volume_override   = 0;
	m_triac_select      = 0;
	m_pay_latch         = 0;

	m_reel12_latch      = 0;
	m_reel34_latch      = 0;
	m_reel56_latch      = 0;

	m_hopper_running    = 0;  // for video games
	m_hopper_coin_sense = 0;

	m_slide_states[0] = 0;
	m_slide_states[1] = 0;
	m_slide_states[2] = 0;
	m_slide_states[3] = 0;
	m_slide_states[4] = 0;
	m_slide_states[5] = 0;

	e2ram_reset();

	machine().device("ymsnd")->reset();

	// make sure no inputs are overidden ////////////////////////////////////
	memset(m_input_override, 0, sizeof(m_input_override));

	// init rom bank ////////////////////////////////////////////////////////

	{
		UINT8 *rom = memregion("maincpu")->base();

		membank("bank1")->configure_entries(0, 4, &rom[0x00000], 0x02000);

		membank("bank1")->set_entry(3);
	}
}

///////////////////////////////////////////////////////////////////////////

void bfm_sc2_state::Scorpion2_SetSwitchState(int strobe, int data, int state)
{
	if ( strobe < 11 && data < 8 )
	{
		if ( strobe < 8 )
		{
			m_input_override[strobe] |= (1<<data);

			if ( state ) m_sc2_Inputs[strobe] |=  (1<<data);
			else         m_sc2_Inputs[strobe] &= ~(1<<data);
		}
		else
		{
			if ( data > 2 )
			{
				m_input_override[strobe-8+4] |= (1<<(data+2));

				if ( state ) m_sc2_Inputs[strobe-8+4] |=  (1<<(data+2));
				else         m_sc2_Inputs[strobe-8+4] &= ~(1<<(data+2));
			}
			else
			{
				m_input_override[strobe-8] |= (1<<(data+5));

				if ( state ) m_sc2_Inputs[strobe-8] |=  (1 << (data+5));
				else         m_sc2_Inputs[strobe-8] &= ~(1 << (data+5));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

int bfm_sc2_state::Scorpion2_GetSwitchState(int strobe, int data)
{
	int state = 0;

	if ( strobe < 11 && data < 8 )
	{
		if ( strobe < 8 )
		{
			state = (m_sc2_Inputs[strobe] & (1<<data) ) ? 1 : 0;
		}
		else
		{
			if ( data > 2 )
			{
				state = (m_sc2_Inputs[strobe-8+4] & (1<<(data+2)) ) ? 1 : 0;
			}
			else
			{
				state = (m_sc2_Inputs[strobe-8] & (1 << (data+5)) ) ? 1 : 0;
			}
		}
	}
	return state;
}

///////////////////////////////////////////////////////////////////////////

void bfm_sc2_state::e2ram_init(nvram_device &nvram, void *data, size_t size)
{
	static const UINT8 init_e2ram[] = { 1, 4, 10, 20, 0, 1, 1, 4, 10, 20 };
	memset(data,0x00,size);
	memcpy(data,init_e2ram,sizeof(init_e2ram));
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x03);
}

///////////////////////////////////////////////////////////////////////////

INTERRUPT_GEN_MEMBER(bfm_sc2_state::timer_irq)
{
	m_timercnt++;

	if ( m_is_timer_enabled )
	{
		m_irq_timer_stat = 0x01;
		m_irq_status     = 0x02;

		device.execute().set_input_line(M6809_IRQ_LINE, HOLD_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::reel12_vid_w)// in a video cabinet this is used to drive a hopper
{
	m_reel12_latch = data;

	if ( m_has_hopper )
	{
		int oldhop = m_hopper_running;

		if ( data & 0x01 )
		{ // hopper power
			if ( data & 0x02 )
			{
				m_hopper_running    = 1;
			}
			else
			{
				m_hopper_running    = 0;
			}
		}
		else
		{
			//m_hopper_coin_sense = 0;
			m_hopper_running    = 0;
		}

		if ( oldhop != m_hopper_running )
		{
			m_hopper_coin_sense = 0;
			oldhop = m_hopper_running;
		}
	}
}


/* Reels 1 and 2 */
WRITE8_MEMBER(bfm_sc2_state::reel12_w)
{
	m_reel12_latch = data;

	m_reel0->update( data    &0x0f);
	m_reel1->update((data>>4)&0x0f);

	awp_draw_reel(machine(),"reel1", m_reel0);
	awp_draw_reel(machine(),"reel2", m_reel1);
}

WRITE8_MEMBER(bfm_sc2_state::reel34_w)
{
	m_reel34_latch = data;

	m_reel2->update( data    &0x0f);
	m_reel3->update((data>>4)&0x0f);

	awp_draw_reel(machine(),"reel3", m_reel2);
	awp_draw_reel(machine(),"reel4", m_reel3);
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::reel56_w)
{
	m_reel56_latch = data;

	m_reel4->update( data    &0x0f);
	m_reel5->update((data>>4)&0x0f);

	awp_draw_reel(machine(),"reel5", m_reel4);
	awp_draw_reel(machine(),"reel6", m_reel5);
}



///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::mmtr_w)
{
	int i;
	int  changed = m_mmtr_latch ^ data;

	m_mmtr_latch = data;

	if (m_meters != nullptr)
	{
		for (i = 0; i<8; i++)
		{
			if ( changed & (1 << i) )
			{
				m_meters->update(i, data & (1 << i) );
			}
		}
	}
	
	if ( data & 0x1F ) m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE );
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::mux_output_w)
{
	int i;
	int off = offset<<3;

	for (i=0; i<8; i++)
		output().set_lamp_value(off+i, ((data & (1 << i)) != 0));

}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::mux_input_r)
{
	int result = 0xFF,t1,t2;
	static const char *const port[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7", "STROBE8", "STROBE9", "STROBE10", "STROBE11" };

	if (offset < 8)
	{
		int idx = (offset & 4) ? 4 : 8;
		t1 = m_input_override[offset];  // strobe 0-7 data 0-4
		t2 = m_input_override[offset+idx];  // strobe 8-B data 0-4

		t1 = (m_sc2_Inputs[offset]   & t1) | ( ( ioport(port[offset])->read()   & ~t1) & 0x1F);
		if (idx == 8)
			t2 = (m_sc2_Inputs[offset+8] & t2) | ( ( ioport(port[offset+8])->read() & ~t2) << 5);
		else
			t2 =  (m_sc2_Inputs[offset+4] & t2) | ( ( ( ioport(port[offset+4])->read() & ~t2) << 2) & 0x60);

		m_sc2_Inputs[offset]   = (m_sc2_Inputs[offset]   & ~0x1F) | t1;
		m_sc2_Inputs[offset+idx] = (m_sc2_Inputs[offset+idx] & ~0x60) | t2;
		result = t1 | t2;
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::unlock_w)
{
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::dimas_w)
{
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::dimcnt_w)
{
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::unknown_w)
{
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::volume_override_w)
{
	int old = m_volume_override;

	m_volume_override = data?1:0;

	if ( old != m_volume_override )
	{
		ym2413_device *ym = machine().device<ym2413_device>("ymsnd");
		float percent = m_volume_override? 1.0f : (32-m_global_volume)/32.0f;

		ym->set_output_gain(0, percent);
		ym->set_output_gain(1, percent);
		m_upd7759->set_output_gain(0, percent);
	}
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::nec_reset_w)
{
	m_upd7759->start_w(0);
	m_upd7759->reset_w(data);
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::nec_latch_w)
{
	int bank = 0;

	if ( data & 0x80 )         bank |= 0x01;
	if ( m_expansion_latch & 2 ) bank |= 0x02;

	m_upd7759->set_bank_base(bank*0x20000);

	m_upd7759->port_w(space, 0, data&0x3F);    // setup sample
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::vfd_status_hop_r)// on video games, hopper inputs are connected to this
{
	// b7 = NEC busy
	// b6 = alpha busy (also matrix board)
	// b5 - b0 = reel optics

	int result = 0;

	if ( m_has_hopper )
	{
		result |= 0x04; // hopper high level
		result |= 0x08; // hopper low  level

		result |= 0x01|0x02;

		if ( m_hopper_running )
		{
			result &= ~0x01;                                  // set motor running input

			if ( m_timercnt & 0x04 ) m_hopper_coin_sense ^= 1;    // toggle coin seen

			if ( m_hopper_coin_sense ) result &= ~0x02;       // update coin seen input
		}
	}

	if ( !m_upd7759->busy_r() ) result |= 0x80;           // update sound busy input

	return result;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::expansion_latch_w)
{
	int changed = m_expansion_latch^data;

	m_expansion_latch = data;

	// bit0,  1 = lamp mux disabled, 0 = lamp mux enabled
	// bit1,  ? used in Del's millions
	// bit2,  digital volume pot meter, clock line
	// bit3,  digital volume pot meter, direction line
	// bit4,  ?
	// bit5,  ?
	// bit6,  ? used in Del's millions
	// bit7   ?

	if ( changed & 0x04)
	{ // digital volume clock line changed
		if ( !(data & 0x04) )
		{ // changed from high to low,
			if ( !(data & 0x08) )
			{
				if ( m_global_volume < 31 ) m_global_volume++; //0-31 expressed as 1-32
			}
			else
			{
				if ( m_global_volume > 0  ) m_global_volume--;
			}

			{
				ym2413_device *ym = machine().device<ym2413_device>("ymsnd");
				float percent = m_volume_override ? 1.0f : (32-m_global_volume)/32.0f;

				ym->set_output_gain(0, percent);
				ym->set_output_gain(1, percent);
				m_upd7759->set_output_gain(0, percent);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::expansion_latch_r)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::muxena_w)
{
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::timerirq_w)
{
	m_is_timer_enabled = data & 1;
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::timerirqclr_r)
{
	m_irq_timer_stat = 0;
	m_irq_status     = 0;

	return 0;
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::irqstatus_r)
{
	int result = m_irq_status | m_irq_timer_stat | 0x80;    // 0x80 = ~MUXERROR

	m_irq_timer_stat = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::coininhib_w)
{
	int changed = m_coin_inhibits^data,i,p;

	m_coin_inhibits = data;

	p = 0x01;
	i = 0;

	while ( i < 8 && changed )
	{
		if ( changed & p )
		{ // this inhibit line has changed
			machine().bookkeeping().coin_lockout_w(i, (~data & p) ); // update lockouts
			changed &= ~p;
		}

		p <<= 1;
		i++;
	}
}

///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::coin_input_r)
{
	return ioport("COINS")->read();
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::payout_latch_w)
{
	m_pay_latch = data;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::payout_triac_w)
{
	if ( m_triac_select == 0x57 )
	{
		int slide = 0;

		switch ( m_pay_latch )
		{
			case 0x01: slide = 1;
				break;

			case 0x02: slide = 2;
				break;

			case 0x04: slide = 3;
				break;

			case 0x08: slide = 4;
				break;

			case 0x10: slide = 5;
				break;

			case 0x20: slide = 6;
				break;
		}

		if ( slide )
		{
			if ( data == 0x4D )
			{
				if ( !m_slide_states[slide] )
				{
					if ( m_slide_pay_sensor[slide] )
					{
						int strobe = m_slide_pay_sensor[slide]>>4, data = m_slide_pay_sensor[slide]&0x0F;

						Scorpion2_SetSwitchState(strobe, data, 0);
					}
					m_slide_states[slide] = 1;
				}
			}
			else
			{
				if ( m_slide_states[slide] )
				{
					if ( m_slide_pay_sensor[slide] )
					{
						int strobe = m_slide_pay_sensor[slide]>>4, data = m_slide_pay_sensor[slide]&0x0F;

						Scorpion2_SetSwitchState(strobe, data, 1);
					}
					m_slide_states[slide] = 0;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::payout_select_w)
{
	m_triac_select = data;
}



///////////////////////////////////////////////////////////////////////////
//TODO: Change this!
WRITE8_MEMBER(bfm_sc2_state::vfd2_data_w)
{
	m_vfd1->write_char(data);
}

///////////////////////////////////////////////////////////////////////////
// serial port ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::uart1stat_r)
{
	int status = 0x06;

	if ( m_data_to_uart1  ) status |= 0x01;
	if ( !m_data_to_uart2 ) status |= 0x02;

	return status;
}
///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::uart1data_r)
{
	return m_uart1_data;
}

//////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::uart1ctrl_w)
{
	UART_LOG(("uart1ctrl:%x\n", data));
}
///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::uart1data_w)
{
	m_data_to_uart2 = 1;
	m_uart1_data    = data;
	UART_LOG(("uart1:%x\n", data));
}
///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::uart2stat_r)
{
	int status = 0x06;

	if ( m_data_to_uart2  ) status |= 0x01;
	if ( !m_data_to_uart1 ) status |= 0x02;

	return status;
}
///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::uart2data_r)
{
	return m_uart2_data;
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::uart2ctrl_w)
{
	UART_LOG(("uart2ctrl:%x\n", data));
}

///////////////////////////////////////////////////////////////////////////

WRITE8_MEMBER(bfm_sc2_state::uart2data_w)
{
	m_data_to_uart1 = 1;
	m_uart2_data    = data;
	UART_LOG(("uart2:%x\n", data));
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

READ8_MEMBER(bfm_sc2_state::key_r)
{
	int result = m_key[ offset ];

	if ( offset == 7 )
	{
		result = (result & 0xFE) | read_e2ram();
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
/*

The X24C08 is a CMOS 8,192 bit serial EEPROM,
internally organized 1024 x 8. The X24C08 features a
serial interface and software protocol allowing operation
on a simple two wire bus.

*/




#define SCL 0x01    //SCL pin (clock)
#define SDA 0x02    //SDA pin (data)


void bfm_sc2_state::e2ram_reset()
{
	m_e2reg   = 0;
	m_e2state = 0;
	m_e2address = 0;
	m_e2rw    = 0;
	m_e2data_pin = 0;
	m_e2data  = (SDA|SCL);
	m_e2dummywrite = 0;
	m_e2data_to_read = 0;
}

int bfm_sc2_state::recdata(int changed, int data)
{
	int res = 1;

	if ( m_e2cnt < 8 )
	{
		res = 0;

		if ( (changed & SCL) && (data & SCL) )
		{ // clocked in new data
			int pattern = 1 << (7-m_e2cnt);

			if ( data & SDA ) m_e2data |=  pattern;
			else              m_e2data &= ~pattern;

			m_e2data_pin = m_e2data_to_read & 0x80 ? 1 : 0;

			m_e2data_to_read <<= 1;

			LOG(("e2d pin= %d\n", m_e2data_pin));

			m_e2cnt++;
			if ( m_e2cnt >= 8 )
			{
				res++;
			}
		}
	}

	return res;
}

int bfm_sc2_state::recAck(int changed, int data)
{
	int result = 0;

	if ( (changed & SCL) && (data & SCL) )
	{
		if ( data & SDA )
		{
			result = 1;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////


/* VFD Status */
READ8_MEMBER(bfm_sc2_state::vfd_status_r)
{
	/* b7 = NEC busy */
	/* b6 = alpha busy (also matrix board) */
	/* b5 - b0 = reel optics */

	int result = m_optic_pattern;

	if ( !m_upd7759->busy_r() ) result |= 0x80;

	if (machine().device("matrix"))
		if ( m_dm01->busy() ) result |= 0x40;

	return result;
}

WRITE8_MEMBER(bfm_sc2_state::vfd1_bd1_w)
{
	m_vfd0->write_char(data);
}

WRITE8_MEMBER(bfm_sc2_state::vfd_reset_w)
{
	m_vfd0->reset();
	m_vfd1->reset();
}

WRITE8_MEMBER(bfm_sc2_state::vfd1_dmd_w)
{
	m_dm01->writedata(data);
}

//
WRITE8_MEMBER(bfm_sc2_state::e2ram_w)
{
	int changed, ack;

	data ^= (SDA|SCL);  // invert signals

	changed  = (m_e2reg^data) & 0x03;

	m_e2reg = data;

	if ( changed )
	{
		while ( 1 )
		{
			if ( (  (changed & SDA) && !(data & SDA))   &&  // 1->0 on SDA  AND
				( !(changed & SCL) && (data & SCL) )    // SCL=1 and not changed
				)
			{   // X24C08 Start condition (1->0 on SDA while SCL=1)
				m_e2dummywrite = ( m_e2state == 5 );

				LOG(("e2ram:   c:%d d:%d Start condition dummywrite=%d\n", (data & SCL)?1:0, (data&SDA)?1:0, m_e2dummywrite ));

				m_e2state = 1; // ready for commands
				m_e2cnt   = 0;
				m_e2data  = 0;
				break;
			}

			if ( (  (changed & SDA) && (data & SDA))    &&  // 0->1 on SDA  AND
				( !(changed & SCL) && (data & SCL) )     // SCL=1 and not changed
				)
			{   // X24C08 Stop condition (0->1 on SDA while SCL=1)
				LOG(("e2ram:   c:%d d:%d Stop condition\n", (data & SCL)?1:0, (data&SDA)?1:0 ));
				m_e2state = 0;
				m_e2data  = 0;
				break;
			}

			switch ( m_e2state )
			{
				case 1: // Receiving address + R/W bit

					if ( recdata(changed, data) )
					{
						m_e2address = (m_e2address & 0x00FF) | ((m_e2data>>1) & 0x03) << 8;
						m_e2cnt   = 0;
						m_e2rw    = m_e2data & 1;

						LOG(("e2ram: Slave address received !!  device id=%01X device adr=%01d high order adr %0X RW=%d) %02X\n",
							m_e2data>>4, (m_e2data & 0x08)?1:0, (m_e2data>>1) & 0x03, m_e2rw , m_e2data ));

						m_e2state = 2;
					}
					break;

				case 2: // Receive Acknowledge

					ack = recAck(changed,data);
					if ( ack )
					{
						m_e2data_pin = 0;

						if ( ack < 0 )
						{
							LOG(("ACK = 0\n"));
							m_e2state = 0;
						}
						else
						{
							LOG(("ACK = 1\n"));
							if ( m_e2dummywrite )
							{
								m_e2dummywrite = 0;

								m_e2data_to_read = m_e2ram[m_e2address];

								if ( m_e2rw & 1 ) m_e2state = 7; // read data
								else          m_e2state = 0; //?not sure
							}
							else
							{
								if ( m_e2rw & 1 ) m_e2state = 7; // reading
								else            m_e2state = 3; // writing
							}
							switch ( m_e2state )
							{
								case 7:
									LOG(("read address %04X\n",m_e2address));
									m_e2data_to_read = m_e2ram[m_e2address];
									break;
								case 3:
									LOG(("write, awaiting address\n"));
									break;
								default:
									LOG(("?unknow action %04X\n",m_e2address));
									break;
							}
						}
						m_e2data = 0;
					}
					break;

				case 3: // writing data, receiving address

					if ( recdata(changed, data) )
					{
						m_e2data_pin = 0;
						m_e2address = (m_e2address & 0xFF00) | m_e2data;

						LOG(("write address = %04X waiting for ACK\n", m_e2address));
						m_e2state = 4;
						m_e2cnt   = 0;
						m_e2data  = 0;
					}
					break;

				case 4: // wait ack, for write address

					ack = recAck(changed,data);
					if ( ack )
					{
						m_e2data_pin = 0;   // pin=0, no error !!

						if ( ack < 0 )
						{
							m_e2state = 0;
							LOG(("ACK = 0, cancel write\n" ));
						}
						else
						{
							m_e2state = 5;
							LOG(("ACK = 1, awaiting data to write\n" ));
						}
					}
					break;

				case 5: // receive data to write
					if ( recdata(changed, data) )
					{
						LOG(("write data = %02X received, awaiting ACK\n", m_e2data));
						m_e2cnt   = 0;
						m_e2state = 6;  // wait ack
					}
					break;

				case 6: // Receive Acknowlede after writing

					ack = recAck(changed,data);
					if ( ack )
					{
						if ( ack < 0 )
						{
							m_e2state = 0;
							LOG(("ACK=0, write canceled\n"));
						}
						else
						{
							LOG(("ACK=1, writing %02X to %04X\n", m_e2data, m_e2address));

							m_e2ram[m_e2address] = m_e2data;

							m_e2address = (m_e2address & ~0x000F) | ((m_e2address+1)&0x0F);

							m_e2state = 5; // write next address
						}
					}
					break;

				case 7: // receive address from read

					if ( recdata(changed, data) )
					{
						//m_e2data_pin = 0;

						LOG(("address read, data = %02X waiting for ACK\n", m_e2data ));

						m_e2state = 8;
					}
					break;

				case 8:

					if ( recAck(changed, data) )
					{
						m_e2state = 7;

						m_e2address = (m_e2address & ~0x0F) | ((m_e2address+1)&0x0F); // lower 4 bits wrap around

						m_e2data_to_read = m_e2ram[m_e2address];

						LOG(("ready for next address %04X\n", m_e2address));

						m_e2cnt   = 0;
						m_e2data  = 0;
					}
					break;

				case 0:

					LOG(("e2ram: ? c:%d d:%d\n", (data & SCL)?1:0, (data&SDA)?1:0 ));
					break;
			}
			break;
		}
	}
}

int bfm_sc2_state::read_e2ram()
{
	LOG(("e2ram: r %d (%02X) \n", m_e2data_pin, m_e2data_to_read ));

	return m_e2data_pin;
}



// machine init (called only once) ////////////////////////////////////////

MACHINE_RESET_MEMBER(bfm_sc2_state,init)
{
	// reset the board //////////////////////////////////////////////////////

	on_scorpion2_reset();
	m_vfd0->reset();
	m_vfd1->reset();

}


READ8_MEMBER(bfm_sc2_state::direct_input_r)
{
	return 0;
}

void bfm_sc2_state::save_state()
{
	/* TODO: Split between the different machine types */

	save_item(NAME(m_key));
	save_item(NAME(m_mmtr_latch));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_optic_pattern));
	save_item(NAME(m_uart1_data));
	save_item(NAME(m_uart2_data));
	save_item(NAME(m_data_to_uart1));
	save_item(NAME(m_data_to_uart2));
	save_item(NAME(m_is_timer_enabled));
	save_item(NAME(m_coin_inhibits));
	save_item(NAME(m_irq_timer_stat));
	save_item(NAME(m_expansion_latch));
	save_item(NAME(m_global_volume));
	save_item(NAME(m_volume_override));
	save_item(NAME(m_reel12_latch));
	save_item(NAME(m_reel34_latch));
	save_item(NAME(m_reel56_latch));
	save_item(NAME(m_pay_latch));
	save_item(NAME(m_slide_states));
	save_item(NAME(m_slide_pay_sensor));
	save_item(NAME(m_triac_select));
	save_item(NAME(m_hopper_running));
	save_item(NAME(m_hopper_coin_sense));
	save_item(NAME(m_timercnt));
	save_item(NAME(m_sc2_Inputs));
	save_item(NAME(m_input_override));
	save_item(NAME(m_e2reg));
	save_item(NAME(m_e2state));
	save_item(NAME(m_e2cnt));
	save_item(NAME(m_e2data));
	save_item(NAME(m_e2address));
	save_item(NAME(m_e2rw));
	save_item(NAME(m_e2data_pin));
	save_item(NAME(m_e2dummywrite));
	save_item(NAME(m_e2data_to_read));
	save_item(NAME(m_codec_data));
}


static ADDRESS_MAP_START( sc2_basemap, AS_PROGRAM, 8, bfm_sc2_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") //8k

	AM_RANGE(0x2300, 0x230B) AM_READ(mux_input_r)
	AM_RANGE(0x2300, 0x231F) AM_WRITE(mux_output_w)
	AM_RANGE(0x2320, 0x2323) AM_WRITE(dimas_w)              /* ?unknown dim related */

	AM_RANGE(0x2324, 0x2324) AM_READWRITE(expansion_latch_r, expansion_latch_w)
	AM_RANGE(0x2325, 0x2327) AM_WRITE(unknown_w)
	AM_RANGE(0x2328, 0x2328) AM_WRITE(muxena_w)
	AM_RANGE(0x2329, 0x2329) AM_READWRITE(timerirqclr_r, timerirq_w)
	AM_RANGE(0x232A, 0x232D) AM_WRITE(unknown_w)
	AM_RANGE(0x232E, 0x232E) AM_READ(irqstatus_r)

	AM_RANGE(0x232F, 0x232F) AM_WRITE(coininhib_w)
	AM_RANGE(0x2330, 0x2330) AM_WRITE(payout_latch_w)
	AM_RANGE(0x2331, 0x2331) AM_WRITE(payout_triac_w)
	AM_RANGE(0x2332, 0x2332) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x2333, 0x2333) AM_WRITE(mmtr_w)
	AM_RANGE(0x2334, 0x2335) AM_WRITE(unknown_w)
	AM_RANGE(0x2336, 0x2336) AM_WRITE(dimcnt_w)
	AM_RANGE(0x2337, 0x2337) AM_WRITE(volume_override_w)
	AM_RANGE(0x2338, 0x2338) AM_WRITE(payout_select_w)
	AM_RANGE(0x2339, 0x2339) AM_WRITE(unknown_w)
	AM_RANGE(0x2400, 0x2400) AM_READWRITE(uart1stat_r, uart1ctrl_w) /* mc6850 compatible uart */
	AM_RANGE(0x2500, 0x2500) AM_READWRITE(uart1data_r, uart1data_w)
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(uart2stat_r, uart2ctrl_w) /* mc6850 compatible uart */
	AM_RANGE(0x2700, 0x2700) AM_READWRITE(uart2data_r, uart2data_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(vfd1_bd1_w)                   /* vfd1 data */
	AM_RANGE(0x2900, 0x2900) AM_WRITE(vfd_reset_w)                  /* vfd1+vfd2 reset line */
	AM_RANGE(0x2A00, 0x2AFF) AM_WRITE(nec_latch_w)
	AM_RANGE(0x2B00, 0x2BFF) AM_WRITE(nec_reset_w)
	AM_RANGE(0x2C00, 0x2C00) AM_WRITE(unlock_w)                     /* custom chip unlock */
	AM_RANGE(0x2D00, 0x2D01) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE(0x2E00, 0x2E00) AM_WRITE(bankswitch_w)                 /* write bank (rom page select for 0x6000 - 0x7fff ) */
	//AM_RANGE(0x2F00, 0x2F00) AM_WRITE(vfd2_data_w)                /* vfd2 data (not usually connected!)*/

	AM_RANGE(0x3FFE, 0x3FFE) AM_READ(direct_input_r )
	AM_RANGE(0x3FFF, 0x3FFF) AM_READ(coin_input_r)
	AM_RANGE(0x4000, 0x5FFF) AM_ROM
	AM_RANGE(0x4000, 0xFFFF) AM_WRITE(unknown_w)            // contains unknown I/O registers
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xFFFF) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( memmap_no_vid, AS_PROGRAM, 8, bfm_sc2_state )
	AM_IMPORT_FROM( sc2_basemap )
	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_r)
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITE(reel34_w)
	AM_RANGE(0x2200, 0x22FF) AM_WRITE(reel56_w)
ADDRESS_MAP_END

// memory map for scorpion2 board video addon /////////////////////////////

static ADDRESS_MAP_START( memmap_vid, AS_PROGRAM, 8, bfm_sc2_state )
	AM_IMPORT_FROM( sc2_basemap )

	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_hop_r)      // vfd status register
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_vid_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITENOP
	AM_RANGE(0x2200, 0x22FF) AM_WRITENOP

	AM_RANGE(0x3C00, 0x3C07) AM_READ(key_r   )
	AM_RANGE(0x3C80, 0x3C80) AM_WRITE(e2ram_w )

	AM_RANGE(0x3E00, 0x3E00) AM_DEVREADWRITE("adder2", bfm_adder2_device, vid_uart_ctrl_r, vid_uart_ctrl_w)     // video uart control reg
	AM_RANGE(0x3E01, 0x3E01) AM_DEVREADWRITE("adder2", bfm_adder2_device, vid_uart_rx_r,   vid_uart_tx_w)       // video uart data  reg
ADDRESS_MAP_END

// input ports for pyramid ////////////////////////////////////////

static INPUT_PORTS_START( pyramid )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.50")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER)     PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "Coin 1 Lockout")PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin 2 Lockout")PORT_DIPLOCATION("DIL:!03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin 3 Lockout")PORT_DIPLOCATION("DIL:!04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin 4 Lockout")PORT_DIPLOCATION("DIL:!05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPNAME( 0x02, 0x00, "Attract mode language" ) PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, "Dutch"       )
	PORT_DIPNAME( 0x0C, 0x00, "Skill Level" ) PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x04, "Medium-Low" )
	PORT_DIPSETTING(    0x08, "Medium-High")
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!11" )

	PORT_START("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x04, "Attract mode" ) PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Stake" ) PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "4 credits per game"  )
	PORT_DIPSETTING(    0x08, "1 credit  per round" )
	PORT_DIPSETTING(    0x10, "2 credit  per round" )
	PORT_DIPSETTING(    0x18, "4 credits per round" )
INPUT_PORTS_END

// input ports for golden crown ///////////////////////////////////

static INPUT_PORTS_START( gldncrwn )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME( "Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME( "Reel 1" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME( "Reel 2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME( "Reel 3" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME( "Reel 4" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME( "Reel 5" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME( "Reel 6" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Hall Of Fame" ) PORT_CODE( KEYCODE_J )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Attract mode language" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, "Dutch")
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )
	PORT_DIPNAME( 0x02, 0x00, "Max number of spins" )PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, "99")
	PORT_DIPSETTING(    0x02, "50")
	PORT_DIPNAME( 0x0C, 0x00, "Skill Level" )PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ))
	PORT_DIPSETTING(    0x04, "Medium-Low"  )
	PORT_DIPSETTING(    0x08, "Medium-High" )
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, "Base Pricing on:" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, "Full Game")
	PORT_DIPSETTING(    0x10, "Individual Rounds")

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Credits required:" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, "4 credits per game")PORT_CONDITION("STROBE10",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x01, "2 credits per game")PORT_CONDITION("STROBE10",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 credit  per round")PORT_CONDITION("STROBE10",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x01, "4 credits per round")PORT_CONDITION("STROBE10",0x10,EQUALS,0x10)
	PORT_DIPNAME( 0x02, 0x00, "Attract Mode" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Time bar" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Time bar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "1 (fast)" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4 (slow)" )
INPUT_PORTS_END

// input ports for dutch quintoon /////////////////////////////////

static INPUT_PORTS_START( qntoond )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.50")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)    PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hand 1" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hand 2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hand 3" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hand 4" )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hand 5" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x1e, 0x1c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIL:!02,!03,!04,!05")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, "C1=2.5 C2=1.25 C3=0.5 C4=2C_0.25C" )
	PORT_DIPSETTING(    0x04, "C1=10 C2=5 C3=2 C4=0.5" )
	PORT_DIPSETTING(    0x06, "C1=1.5/3.25/5 C2=0.75/1.5/3.25 C3=0.25/0.5/1 C4=3C_0.25C" )
	PORT_DIPSETTING(    0x08, "C1=20 C2=10 C3=4 C4=1" )
	PORT_DIPSETTING(    0x0a, "C1=2 C2=1 C3=0.25/0.75/1/1.5/2 C4=3C_0.25C 5C_0.5C" )
	PORT_DIPSETTING(    0x0c, "C1=5 C2=2.5 C3=1 C4=0.25" )
	PORT_DIPSETTING(    0x0e, "C1=1.25 C2=0.5/1.25/1.75 C3=0.25 C4=0.25" )
	//PORT_DIPSETTING(    0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x12, "C1=3 C2=1.5 C3=0.5 C4=0.25" )
	PORT_DIPSETTING(    0x14, "C1=12 C2=6 C3=2 C4=0.5" )
	PORT_DIPSETTING(    0x16, "C1=2 C2=1 C3=0.25 C4=3C_0.25C" )
	PORT_DIPSETTING(    0x18, "C1=24 C2=12 C3=4 C4=1" )
	PORT_DIPSETTING(    0x1a, "C1=2.25/4.75 C2=1/2.25/3.5/4.75/6 C3=0.25/0.75/1/1.5/2 C4=3C_0.25C 5C_0.5C" )
	PORT_DIPSETTING(    0x1c, "C1=6 C2=3 C3=1 C4=0.25" )
	PORT_DIPSETTING(    0x1e, "C1=1.5 C2=0.75 C3=0.25 C4=4C_0.25C" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin 1 Lockout")PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin 2 Lockout")PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin 3 Lockout")PORT_DIPLOCATION("DIL:!08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin 4 Lockout")PORT_DIPLOCATION("DIL:!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Time bar" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Clear credits on reset" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x04, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract mode language" )PORT_DIPLOCATION("DIL:!15")
	PORT_DIPSETTING(    0x00, DEF_STR( English  ) )
	PORT_DIPSETTING(    0x08, "Dutch"    )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!16" )
INPUT_PORTS_END

// input ports for UK quintoon ////////////////////////////////////////////

static INPUT_PORTS_START( quintoon )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hand 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hand 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hand 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hand 4")

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hand 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("?1") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("?2") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("?3") PORT_CODE(KEYCODE_O)

	PORT_MODIFY("STROBE5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SPECIAL) //Payout opto

	PORT_MODIFY("STROBE9")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!02" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!03" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!04" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!05" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin Lockout")PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) //Will activate coin lockout when Credit >= 1 Play
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Stake per Game / Jackpot" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, "20p / 6 Pounds" )
	PORT_DIPSETTING(    0x10, "50p / 20 Pounds" )

	PORT_MODIFY("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x1C, 0x00, "Target percentage" )PORT_DIPLOCATION("DIL:!14,!15,!16")
	PORT_DIPSETTING(    0x1C, "50%")
	PORT_DIPSETTING(    0x0C, "55%")
	PORT_DIPSETTING(    0x08, "60%")
	PORT_DIPSETTING(    0x18, "65%")
	PORT_DIPSETTING(    0x10, "70%")
	PORT_DIPSETTING(    0x00, "75%")
	PORT_DIPSETTING(    0x04, "80%")
	PORT_DIPSETTING(    0x14, "85%")
INPUT_PORTS_END

// input ports for slotsnl  ///////////////////////////////////////////////

static INPUT_PORTS_START( slotsnl )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Slot 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Slot 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_NAME("Slot 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_NAME("Slot 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )   PORT_NAME("Enter") PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x04, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x18, "4" )
INPUT_PORTS_END

// input ports for sltblgtk  //////////////////////////////////////////////

static INPUT_PORTS_START( sltblgtk )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20 BFr")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50 BFr")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Slot 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Slot 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Slot 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Slot 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("Enter") PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SPECIAL ) //Tube 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SPECIAL ) //Tube 2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "CashMeters in refill menu" )PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Token Lockout" )PORT_DIPLOCATION("DIL:!03")
	PORT_DIPSETTING(    0x00, DEF_STR( No  ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "20 Bfr Lockout" )PORT_DIPLOCATION("DIL:!04")
	PORT_DIPSETTING(    0x00, DEF_STR( No  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "50 Bfr Lockout" )PORT_DIPLOCATION("DIL:!05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPNAME( 0x0E, 0x00, "Payout Percentage" )PORT_DIPLOCATION("DIL:!07,!08,!10")
	PORT_DIPSETTING(    0x00, "60%")
	PORT_DIPSETTING(    0x08, "65%")
	PORT_DIPSETTING(    0x04, "70%")
	PORT_DIPSETTING(    0x0C, "75%")
	PORT_DIPSETTING(    0x02, "80%")
	PORT_DIPSETTING(    0x0A, "84%")
	PORT_DIPSETTING(    0x06, "88%")
	PORT_DIPSETTING(    0x0E, "90%")
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Timebar" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Clear credits" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On   ) )
	PORT_DIPNAME( 0x08, 0x00, "Show hints" )PORT_DIPLOCATION("DIL:!15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Pay win to credits" )PORT_DIPLOCATION("DIL:!16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

// input ports for sltblgpo  //////////////////////////////////////////////

static INPUT_PORTS_START( sltblgpo )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Bfr 20")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Bfr 50")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hand 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hand 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_NAME("Hand 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_NAME("Hand 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_NAME("Stake")  PORT_CODE( KEYCODE_O )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Collect") PORT_CODE(KEYCODE_C)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "Hopper Limit" )PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!03" )
	PORT_DIPNAME( 0x18, 0x00, "Attendant payout" )PORT_DIPLOCATION("DIL:!04,!05")
	PORT_DIPSETTING(    0x00, "1000 Bfr" )
	PORT_DIPSETTING(    0x08, "1250 Bfr" )
	PORT_DIPSETTING(    0x10, "1500 Bfr" )
	PORT_DIPSETTING(    0x18, "1750 Bfr" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Bfr 20 Lockout" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Clear credits on reset?" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Attract Mode" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Target Percentage" )PORT_DIPLOCATION("DIL:!14,!15,!16")
	PORT_DIPSETTING(    0x14, "80%")
	PORT_DIPSETTING(    0x04, "82%")
	PORT_DIPSETTING(    0x1C, "84%")
	PORT_DIPSETTING(    0x0C, "86%")
	PORT_DIPSETTING(    0x10, "90%")
	PORT_DIPSETTING(    0x00, "92%")
	PORT_DIPSETTING(    0x18, "94%")
	PORT_DIPSETTING(    0x08, "96%")
INPUT_PORTS_END

// input ports for paradice ///////////////////////////////////////////////

static INPUT_PORTS_START( paradice )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME( "1 Player Start (Left)" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME( "2 Player Start (Right)" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME( "A" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME( "B" )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME( "C" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME( "Enter" ) PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Joker" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Language ) )PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, "Dutch"    )
	PORT_DIPNAME( 0x0C, 0x00, "Payout level" )PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x08, "Medium-Low"  )
	PORT_DIPSETTING(    0x04, "Medium-High" )
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Difficulty ) )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x03, 0x00, "Winlines to go" )PORT_DIPLOCATION("DIL:!12,!13")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x03, "9" )
	PORT_DIPNAME( 0x04, 0x04, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
INPUT_PORTS_END

// input ports for pokio //////////////////////////////////////////////////

static INPUT_PORTS_START( pokio )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Hand 1 Left" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME( "Hand 2 Left" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME( "Hand 3 Left" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME( "1 Player Start (Left)" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME( "Enter" ) PORT_CODE( KEYCODE_SPACE )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME( "2 Player Start (Right)" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 )PORT_NAME( "Hand 3 Right" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 )PORT_NAME( "Hand 2 Right" )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )PORT_NAME( "Hand 1 Right" )

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x10, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On   ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Time bar" ) PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x04, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
INPUT_PORTS_END

///////////////////////////////////////////////////////////////////////////
// machine config fragments for different meters numbers //////////////////
///////////////////////////////////////////////////////////////////////////

MACHINE_CONFIG_FRAGMENT( _3meters )
	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(3)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( _5meters )
	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(5)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( _8meters )
	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(8)
MACHINE_CONFIG_END

///////////////////////////////////////////////////////////////////////////
// machine driver for scorpion2 board + adder2 expansion //////////////////
///////////////////////////////////////////////////////////////////////////

MACHINE_START_MEMBER(bfm_sc2_state,bfm_sc2)
{
	nvram_device *e2ram = subdevice<nvram_device>("e2ram");
	if (e2ram != nullptr)
		e2ram->set_base(m_e2ram, sizeof(m_e2ram));

	save_state();
}

static MACHINE_CONFIG_START( scorpion2_vid, bfm_sc2_state )
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 ) // 6809 CPU at 2 Mhz
	MCFG_CPU_PROGRAM_MAP(memmap_vid)                    // setup scorpion2 board memorymap
	MCFG_CPU_PERIODIC_INT_DRIVER(bfm_sc2_state, timer_irq,  1000)               // generate 1000 IRQ's per second
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))
	MCFG_QUANTUM_TIME(attotime::from_hz(960))                                   // needed for serial communication !!

	MCFG_BFMBD1_ADD("vfd0",0)
	MCFG_BFMBD1_ADD("vfd1",1)

	MCFG_MACHINE_START_OVERRIDE(bfm_sc2_state,bfm_sc2)
	MCFG_MACHINE_RESET_OVERRIDE(bfm_sc2_state, init )                           // main scorpion2 board initialisation

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("e2ram", bfm_sc2_state, e2ram_init)
	MCFG_DEFAULT_LAYOUT(layout_sc2_vid)

	MCFG_BFM_ADDER2_ADD("adder2")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

/* machine driver for scorpion2_vid board with meters (i.e. quintoon uk). Are we really sure the other games don't?*/
static MACHINE_CONFIG_DERIVED( scorpion2_vidm, scorpion2_vid )
	MCFG_FRAGMENT_ADD(_8meters)
MACHINE_CONFIG_END



int bfm_sc2_state::sc2_find_project_string( )
{
	// search for the project string to find the title (usually just at ff00)
	char title_string[4][32] = { "PROJECT NUMBER", "PROJECT PR", "PROJECT ", "CASH ON THE NILE 2" };
	UINT8 *src = memregion( "maincpu" )->base();
	int size = memregion( "maincpu" )->bytes();

	for (auto & elem : title_string)
	{
		int strlength = strlen(elem);

		for (int i=0;i<size-strlength;i++)
		{
			int j;
			int found = 1;
			for (j=0;j<strlength;j+=1)
			{
				UINT8 rom = src[(i+j)];
				UINT8 chr = elem[j];

				if (rom != chr)
				{
					found = 0;
					break;
				}
			}

			if (found!=0)
			{
				int end=0;
				int count = 0;
				int blankcount = 0;
				printf("ID String @ %08x\n", i);

				while (!end)
				{
					UINT8 rom;
					int addr;

					addr = (i+count);

					if (addr<size)
					{
						rom = src[addr];

						if ((rom>=0x20) && (rom<0x7f))
						{
							printf("%c", rom);
							blankcount = 0;
						}
						else
						{
							blankcount++;
							if (blankcount<10) printf(" ");
						}

						count++;
					}
					else
						end = 1;

					if (count>=0x100)
						end = 1;
				}
				printf("\n");

				return 1;
			}
		}
	}

	return 0;
}


void bfm_sc2_state::sc2_common_init(int decrypt)
{
	if (decrypt) bfm_decode_mainrom(machine(), "maincpu", m_codec_data);         // decode main rom

	memset(m_sc2_Inputs, 0, sizeof(m_sc2_Inputs));  // clear all inputs
}

void bfm_sc2_state::adder2_common_init()
{
	if (memregion("proms") != nullptr)
	{
		UINT8 *pal;
		pal = memregion("proms")->base();
		memcpy(m_key, pal, 8);
	}
}

// UK quintoon initialisation ////////////////////////////////////////////////

DRIVER_INIT_MEMBER(bfm_sc2_state,quintoon)
{
	sc2_common_init( 1);

	m_has_hopper = 0;

	Scorpion2_SetSwitchState(3,0,1);  // tube1 level switch
	Scorpion2_SetSwitchState(3,1,1);  // tube2 level switch
	Scorpion2_SetSwitchState(3,2,1);  // tube3 level switch

	Scorpion2_SetSwitchState(5,2,1);
	Scorpion2_SetSwitchState(6,4,1);

	m_sc2_show_door   = 1;
	m_sc2_door_state  = 0x41;
}

// dutch pyramid intialisation //////////////////////////////////////////////

DRIVER_INIT_MEMBER(bfm_sc2_state,pyramid)
{
	sc2_common_init(1);
	adder2_common_init();

	m_has_hopper = 1;

	Scorpion2_SetSwitchState(3,0,1);  // tube1 level switch
	Scorpion2_SetSwitchState(3,1,1);  // tube2 level switch
	Scorpion2_SetSwitchState(3,2,1);  // tube3 level switch

	m_sc2_show_door   = 1;
	m_sc2_door_state  = 0x41;
}
// belgian slots initialisation /////////////////////////////////////////////

DRIVER_INIT_MEMBER(bfm_sc2_state,sltsbelg)
{
	sc2_common_init(1);
	adder2_common_init();

	m_has_hopper = 1;

	m_sc2_show_door   = 1;
	m_sc2_door_state  = 0x41;
}

// other dutch adder games ////////////////////////////////////////////////

DRIVER_INIT_MEMBER(bfm_sc2_state,adder_dutch)
{
	sc2_common_init(1);
	adder2_common_init();

	m_has_hopper = 0;

	Scorpion2_SetSwitchState(3,0,1);  // tube1 level switch
	Scorpion2_SetSwitchState(3,1,1);  // tube2 level switch
	Scorpion2_SetSwitchState(3,2,1);  // tube3 level switch

	m_sc2_show_door   = 1;
	m_sc2_door_state  = 0x41;
}

// golden crown //////////////////////////////////////////////////////////

DRIVER_INIT_MEMBER(bfm_sc2_state,gldncrwn)
{
	sc2_common_init(1);
	adder2_common_init();

	m_has_hopper = 0;

	Scorpion2_SetSwitchState(3,0,1);  // tube1 level switch
	Scorpion2_SetSwitchState(3,1,1);  // tube2 level switch
	Scorpion2_SetSwitchState(3,2,1);  // tube3 level switch

	m_sc2_show_door   = 0;
	m_sc2_door_state  = 0x41;
}

// ROM definition UK Quintoon ////////////////////////////////////////////

ROM_START( quintoon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750206.p1", 0x00000, 0x10000,  CRC(05f4bfad) SHA1(22751573f3a51a9fd2d2a75a7d1b20d78112e0bb))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",        0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",    0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",        0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition UK Quintoon (older) ////////////////////////////////////

ROM_START( quintono )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750203.bin",    0x00000, 0x10000,  CRC(037ef2d0) SHA1(6958624e29629a7639a80e8929b833a8b0201833))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",        0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",    0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",        0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition UK Quintoon (data) /////////////////////////////////////

ROM_START( quintond )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95751206.bin",    0x00000, 0x10000,  CRC(63def707) SHA1(d016df74f4f83cd72b16f9ccbe78cc382bf056c8))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",        0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",    0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",        0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition Dutch Quintoon ///////////////////////////////////////////

ROM_START( qntoond )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750243.bin", 0x00000, 0x10000, CRC(36a8dcd1) SHA1(ab21301312fbb6609f850e1cf6bcda5a2b7f66f5))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770024.vid", 0x00000, 0x20000, CRC(5bc7ac55) SHA1(b54e9684f750b73c357d41b88ca8c527258e2a10))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770025.chr", 0x00000, 0x20000, CRC(f59748ea) SHA1(f0f7f914fdf72db8eb60717b95e7d027c0081339))
ROM_END

// ROM definition Dutch Quintoon alternate set /////////////////////////////

ROM_START( qntoondo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750136.bin", 0x00000, 0x10000, CRC(839ea01d) SHA1(d7f77dbaea4e87c3d782408eb50d10f44b6df5e2))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770024.vid", 0x00000, 0x20000, CRC(5bc7ac55) SHA1(b54e9684f750b73c357d41b88ca8c527258e2a10))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770025.chr", 0x00000, 0x20000, CRC(f59748ea) SHA1(f0f7f914fdf72db8eb60717b95e7d027c0081339))
ROM_END

// ROM definition dutch golden crown //////////////////////////////////////

ROM_START( gldncrwn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95752011.bin", 0x00000, 0x10000, CRC(54f7cca0) SHA1(835727d88113700a38060f880b4dfba2ded41487))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770117.vid", 0x00000, 0x20000, CRC(598ba7cb) SHA1(ab518d7df24b0b453ec3fcddfc4db63e0391fde7))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001039.snd", 0x00000, 0x20000, CRC(6af26157) SHA1(9b3a85f5dd760c4430e38e2844928b74aadc7e75))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770118.ch1", 0x00000, 0x20000, CRC(9c9ac946) SHA1(9a571e7d00f6654242aface032c2fb186ef44aba))
	ROM_LOAD("95770119.ch2", 0x20000, 0x20000, CRC(9e0fdb2e) SHA1(05e8257285b0009df4fcc73e93490876358a8be8))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("gcrpal.bin", 0, 8 , CRC(4edd5a1d) SHA1(d6fe38377d5f2291d33ee8ed808548871e63c4d7))
ROM_END

// ROM definition Dutch Paradice //////////////////////////////////////////

ROM_START( paradice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750615.bin", 0x00000, 0x10000, CRC(f51192e5) SHA1(a1290e32bba698006e83fd8d6075202586232929))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770084.vid", 0x00000, 0x20000, CRC(8f27bd34) SHA1(fccf7283b5c952b74258ee6e5138c1ca89384e24))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001037.snd", 0x00000, 0x20000, CRC(82f74276) SHA1(c51c3caeb7bf514ec7a1b452c8effc4c79186062))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770085.ch1", 0x00000, 0x20000, CRC(4d1fb82f) SHA1(054f683d1d7c884911bd2d0f85aab4c59ddf9930))
	ROM_LOAD("95770086.ch2", 0x20000, 0x20000, CRC(7b566e11) SHA1(f34c82ad75a0f88204ac4ae83a00801215c46ca9))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD( "pdcepal.bin", 0, 8 , CRC(64020c97) SHA1(9371841e2df950c1f2e5b5a4b52621beb6f60945))
ROM_END

// ROM definition Dutch Pokio /////////////////////////////////////////////

ROM_START( pokio )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750278.bin", 0x00000, 0x10000, CRC(5124b24d) SHA1(9bc63891a8e9283c2baa64c264a5d6d1625d44b2))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770044.vid", 0x00000, 0x20000, CRC(46d7a6d8) SHA1(01f58e735621661b57c61491b3769ae99e92476a))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(98aaff76) SHA1(4a59cf83daf018d93f1ff7805e06309d2f3d7252))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770045.chr", 0x00000, 0x20000, CRC(dd30da90) SHA1(b4f5a229d88613c0c7d43adf3f325c619abe38a3))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("pokiopal.bin", 0, 8 , CRC(53535184) SHA1(c5c98085e39ca3671dca72c21a8466d7d70cd341))
ROM_END

// ROM definition pyramid prototype  //////////////////////////////////////

ROM_START( pyramid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750898.bin", 0x00000, 0x10000,  CRC(3b0df16c) SHA1(9af599fe604f86c72986aa1610d74837852e023f))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770108.vid", 0x00000, 0x20000,  CRC(216ff683) SHA1(227764771600ce88c5f36bed9878e6bb9988ae8f))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001038.snd", 0x00000, 0x20000, CRC(f885c42e) SHA1(4d79fc5ae4c58247740d78d81302bfbb43331c43))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770106.ch1", 0x00000, 0x20000, CRC(a83c27ae) SHA1(f61ca3cdf19a933bae18c1b32a5fb0a2204dde78))
	ROM_LOAD("95770107.ch2", 0x20000, 0x20000, CRC(52e59f64) SHA1(ea4828c2cfb72cd77c92c60560b4d5ee424f7dca))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("pyrmdpal.bin", 0, 8 , CRC(1c7c37bb) SHA1(fe0276603fee8f58e4318f91645260368212b78b))
ROM_END

// ROM definition Dutch slots /////////////////////////////////////////////

ROM_START( slotsnl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750368.bin", 0x00000, 0x10000, CRC(3a43048c) SHA1(13728e05b334cba90ea9cc51ea00c4384baa8614))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("video.vid",    0x00000, 0x20000, CRC(cc760208) SHA1(cc01b1e31335b26f2d0f3470d8624476b153655f))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001029.snd", 0x00000, 0x20000, CRC(7749c724) SHA1(a87cce0c99e392f501bba44b3936a7059d682c9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("charset.chr",  0x00000, 0x20000,  CRC(ef4300b6) SHA1(a1f765f38c2f146651fc685ea6195af72465f559))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD( "slotspal.bin", 0, 8 , CRC(ee5421f0) SHA1(21bdcbf11dda8b1a93c49ae1c706954bba53c917))
ROM_END

// ROM definition Belgian Slots (Token pay per round) Payslide ////////////

ROM_START( sltblgtk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750943.bin", 0x00000, 0x10000, CRC(c9fb8153) SHA1(7c1d0660c15f05b1e0784d8322c62981fe8dc4c9))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("adder121.bin", 0x00000, 0x20000, CRC(cedbbf28) SHA1(559ae341b55462feea771127394a54fc65266818))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound029.bin", 0x00000, 0x20000, CRC(7749c724) SHA1(a87cce0c99e392f501bba44b3936a7059d682c9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("chr122.bin",   0x00000, 0x20000, CRC(a1e3bdf4) SHA1(f0cabe08dee028e2014cbf0fc3fe0806cdfa60c6))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbtpal.bin", 0, 8 , CRC(20e13635) SHA1(5aa7e7cac8c00ebc193d63d0c6795904f42c70fa))
ROM_END

// ROM definition Belgian Slots (Cash Payout) /////////////////////////////

ROM_START( sltblgp1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95752008.bin", 0x00000, 0x10000, CRC(3167d3b9) SHA1(a28563f65d55c4d47f3e7fdb41e050d8a733b9bd))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("adder142.bin", 0x00000, 0x20000, CRC(a6f6356b) SHA1(b3d3063155ee3ea888273081f844279b6e33f7d9))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound033.bin", 0x00000, 0x20000, CRC(bb1dfa55) SHA1(442454fccfe03e6f4c3353551cb7459e184a099d))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("chr143.bin",   0x00000, 0x20000, CRC(a40e91e2) SHA1(87dc76963ea961fcfbe4f3e25df9162348d39d79))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbcpal.bin", 0, 8 , CRC(c63bcab6) SHA1(238841165d5b3241b0bcc5c1792e9c0be1fc0177))
ROM_END

// ROM definition Belgian Slots (Cash Payout) /////////////////////////////

ROM_START( sltblgpo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95770938.bin", 0x00000, 0x10000, CRC(7e802634) SHA1(fecf86e632546649d5e647c42a248b39fc2cf982))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770120.chr", 0x00000, 0x20000, CRC(ad505138) SHA1(67ccd8dc30e76283247ab5a62b22337ebaff74cd))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound033.bin", 0x00000, 0x20000, CRC(bb1dfa55) SHA1(442454fccfe03e6f4c3353551cb7459e184a099d))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770110.add", 0x00000, 0x20000, CRC(64b03284) SHA1(4b1c17b75e449c9762bb949d7cde0694a3aaabeb))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbcpal.bin", 0, 8 , CRC(c63bcab6) SHA1(238841165d5b3241b0bcc5c1792e9c0be1fc0177))
ROM_END


/**************************************************************************

    Mechanical Scorpion 2 Games
        AGEMAME driver

***************************************************************************

  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware
              setup.
  18-01-2006: Cleaned up for MAME inclusion
  19-08-2005: Re-Animator

***************************************************************************/



///////////////////////////////////////////////////////////////////////////






#ifdef UNUSED_FUNCTION
/* Scorpion 3 expansion */
READ8_MEMBER(bfm_sc2_state::sc3_expansion_r)
{
	int result = 0;

	switch ( offset )
	{
		case 0: result = 0;
		break;
		case 1: result = input_port_read_indexed(machine,0);  /* coin input */
	}

	return result;
}


WRITE8_MEMBER(bfm_sc2_state::sc3_expansion_w)
{
	switch ( offset )
	{
		case 0:
		break;
		case 1:
		break;
	}
}
#endif

WRITE_LINE_MEMBER(bfm_sc2_state::bfmdm01_busy)
{
	Scorpion2_SetSwitchState(4,4, state?0:1);
}

/* machine init (called only once) */
MACHINE_RESET_MEMBER(bfm_sc2_state,awp_init)
{
	on_scorpion2_reset();
	m_vfd0->reset();
	m_vfd1->reset();
}


MACHINE_RESET_MEMBER(bfm_sc2_state,dm01_init)
{
	on_scorpion2_reset();
}

static INPUT_PORTS_START( bbrkfst )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Take Big Breakfast")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Take Feature")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED  )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)  PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)  PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )     PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( drwho )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )  PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for 1 Pound*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for 20p*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for Token Front*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for Token Rear*/
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")/*Certain combinations give different percentages*/
	PORT_DIPSETTING(    0x00, "No key") /*Some day, I'll work all these values out.*/
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for 1 Pound*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for 20p*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for Token Front*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for Token Rear*/
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cpeno1 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Stop/Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract Hi/Lo reel" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Acceptor type" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin play" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, "Multi coin" )
	PORT_DIPSETTING(    0x01, "Single coin" )
	PORT_DIPNAME( 0x02, 0x00, "CashPot Freq" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High) )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Jam Alarm" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Percentage setting" ) PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x0C, "72%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPSETTING(    0x10, "81%" )
	PORT_DIPSETTING(    0x18, "85%" )
INPUT_PORTS_END

static INPUT_PORTS_START( luvjub )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Take win")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME( "Yes!" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME( "No!" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9)   PORT_NAME("Answer the phone")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bfmcgslm )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER)     PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( scorpion3 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Stop/Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract Hi/Lo reel" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Acceptor type" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin play" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, "Multi coin" )
	PORT_DIPSETTING(    0x01, "Single coin" )
	PORT_DIPNAME( 0x02, 0x00, "CashPot Freq" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High) )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Jam Alarm" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Percentage setting" ) PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x0C, "72%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPSETTING(    0x10, "81%" )
	PORT_DIPSETTING(    0x18, "85%" )

INPUT_PORTS_END

WRITE8_MEMBER(bfm_sc2_state::dmd_reset_w)
{
//TODO: Reset callback for DMD
}

MACHINE_START_MEMBER(bfm_sc2_state,sc2dmd)
{
	MACHINE_START_CALL_MEMBER(bfm_sc2);
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_write_handler(0x2800, 0x2800, 0, 0, write8_delegate(FUNC(bfm_sc2_state::vfd1_dmd_w),this));
	space.install_write_handler(0x2900, 0x2900, 0, 0, write8_delegate(FUNC(bfm_sc2_state::dmd_reset_w),this));
}

/* machine driver for scorpion2 board */

static MACHINE_CONFIG_START( scorpion2, bfm_sc2_state )
	MCFG_MACHINE_RESET_OVERRIDE(bfm_sc2_state,awp_init)
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP(memmap_no_vid)
	MCFG_CPU_PERIODIC_INT_DRIVER(bfm_sc2_state, timer_irq,  1000)
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))

	MCFG_BFMBD1_ADD("vfd0",0)
	MCFG_BFMBD1_ADD("vfd1",1)

	MCFG_MACHINE_START_OVERRIDE(bfm_sc2_state,bfm_sc2)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ymsnd",YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("e2ram", bfm_sc2_state, e2ram_init)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_sc2_vfd)

	MCFG_STARPOINT_48STEP_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel0_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel1_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel2_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel3_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel4_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel5_optic_cb))
	
	MCFG_FRAGMENT_ADD(_8meters)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( scorpion2_3m, scorpion2 )
	MCFG_DEVICE_REMOVE("meters")
	MCFG_FRAGMENT_ADD(_3meters)
MACHINE_CONFIG_END

/* machine driver for scorpion3 board */
static MACHINE_CONFIG_DERIVED( scorpion3, scorpion2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(memmap_no_vid)
	
	MCFG_DEVICE_REMOVE("meters")
	MCFG_FRAGMENT_ADD(_5meters)
MACHINE_CONFIG_END


/* machine driver for scorpion2 board + matrix board */
static MACHINE_CONFIG_START( scorpion2_dm01, bfm_sc2_state )
	MCFG_MACHINE_RESET_OVERRIDE(bfm_sc2_state,dm01_init)
	MCFG_QUANTUM_TIME(attotime::from_hz(960))                                   // needed for serial communication !!
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP(memmap_no_vid)
	MCFG_CPU_PERIODIC_INT_DRIVER(bfm_sc2_state, timer_irq,  1000)
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_MONOSTABLE(120000,100e-9))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd",YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_MACHINE_START_OVERRIDE(bfm_sc2_state,sc2dmd)
	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("e2ram", bfm_sc2_state, e2ram_init)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_sc2_dmd)
	MCFG_DEVICE_ADD("dm01", BF_DM01, 0)
	MCFG_BF_DM01_BUSY_CB(WRITELINE(bfm_sc2_state, bfmdm01_busy))
	MCFG_CPU_ADD("matrix", M6809, 2000000 )             /* matrix board 6809 CPU at 2 Mhz ?? I don't know the exact freq.*/
	MCFG_CPU_PROGRAM_MAP(bfm_dm01_memmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(bfm_sc2_state, nmi_line_assert, 1500 )          /* generate 1500 NMI's per second ?? what is the exact freq?? */

	MCFG_STARPOINT_48STEP_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel0_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel1_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel2_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel3_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel4_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(bfm_sc2_state, reel5_optic_cb))
	
	MCFG_FRAGMENT_ADD( _8meters)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( scorpion2_dm01_3m, scorpion2_dm01 )
	MCFG_DEVICE_REMOVE("meters")
	MCFG_FRAGMENT_ADD( _3meters)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( scorpion2_dm01_5m, scorpion2_dm01 )
	MCFG_DEVICE_REMOVE("meters")
	MCFG_FRAGMENT_ADD( _5meters)
MACHINE_CONFIG_END

void bfm_sc2_state::sc2awp_common_init(int reels, int decrypt)
{
	sc2_common_init(decrypt);
	/* setup n default 96 half step reels */

	m_reels=reels;
}

void bfm_sc2_state::sc2awpdmd_common_init(int reels, int decrypt)
{
	sc2_common_init(decrypt);
	/* setup n default 96 half step reels */

	m_reels=reels;
}



DRIVER_INIT_MEMBER(bfm_sc2_state,bbrkfst)
{
	sc2awp_common_init(5, 1);

	m_has_hopper = 0;

	Scorpion2_SetSwitchState(4,0, 1);   /* GBP1 Low Level Switch */
	Scorpion2_SetSwitchState(4,1, 1);   /* 20p Low Level Switch */
	Scorpion2_SetSwitchState(4,2, 1);   /* Token Front Low Level Switch */
	Scorpion2_SetSwitchState(4,3, 1);   /* Token Rear  Low Level Switch */
	Scorpion2_SetSwitchState(4,4, 1);
	Scorpion2_SetSwitchState(6,0, 0);
	Scorpion2_SetSwitchState(6,1, 1);
	Scorpion2_SetSwitchState(6,2, 0);
	Scorpion2_SetSwitchState(6,3, 1);

	sc2_find_project_string();
}

DRIVER_INIT_MEMBER(bfm_sc2_state,drwho_common)
{
	m_has_hopper = 0;

	Scorpion2_SetSwitchState(4,0, 0);   /* GBP1 Low Level Switch */
	Scorpion2_SetSwitchState(4,1, 0);   /* 20p Low Level Switch */
	Scorpion2_SetSwitchState(4,2, 0);   /* Token Front Low Level Switch */
	Scorpion2_SetSwitchState(4,3, 0);   /* Token Rear  Low Level Switch */
	Scorpion2_SetSwitchState(7,0, 0);   /* GBP1 High Level Switch */
	Scorpion2_SetSwitchState(7,1, 0);   /* 20P High Level Switch */
	Scorpion2_SetSwitchState(7,2, 0);   /* Token Front High Level Switch */
	Scorpion2_SetSwitchState(7,3, 0);   /* Token Rear High Level Switch */

	sc2_find_project_string();
}

DRIVER_INIT_MEMBER(bfm_sc2_state,drwho)
{
	sc2awp_common_init(6, 1);
	DRIVER_INIT_CALL(drwho_common);
}

DRIVER_INIT_MEMBER(bfm_sc2_state,drwhon)
{
	sc2awp_common_init(4, 0);
	DRIVER_INIT_CALL(drwho_common);
}


DRIVER_INIT_MEMBER(bfm_sc2_state,focus)
{
	sc2awp_common_init(6, 1);
	sc2_find_project_string();
}

DRIVER_INIT_MEMBER(bfm_sc2_state,cpeno1)
{
	sc2awpdmd_common_init(6, 1);

	Scorpion2_SetSwitchState(3,3,1);  /*  5p play */
	Scorpion2_SetSwitchState(3,4,1);  /* 20p play */

	Scorpion2_SetSwitchState(4,0,1);  /* pay tube low (1 pound front) */
	Scorpion2_SetSwitchState(4,1,1);  /* pay tube low (20p) */
	Scorpion2_SetSwitchState(4,2,1);  /* pay tube low (?1 right) */
	Scorpion2_SetSwitchState(4,3,1);  /* pay tube low (?1 left) */

	Scorpion2_SetSwitchState(5,0,1);  /* pay sensor (GBP1 front) */
	Scorpion2_SetSwitchState(5,1,1);  /* pay sensor (20 p) */
	Scorpion2_SetSwitchState(5,2,1);  /* pay sensor (1 right) */
	Scorpion2_SetSwitchState(5,3,1);  /* pay sensor (?1 left) */
	Scorpion2_SetSwitchState(5,4,1);  /* payout unit present */

	m_slide_pay_sensor[0] = 0x50;
	m_slide_pay_sensor[1] = 0x51;
	m_slide_pay_sensor[2] = 0x52;
	m_slide_pay_sensor[3] = 0x53;
	m_slide_pay_sensor[4] = 0;
	m_slide_pay_sensor[5] = 0;

	Scorpion2_SetSwitchState(6,0,1);  /* ? percentage key */
	Scorpion2_SetSwitchState(6,1,1);
	Scorpion2_SetSwitchState(6,2,1);
	Scorpion2_SetSwitchState(6,3,1);
	Scorpion2_SetSwitchState(6,4,1);

	Scorpion2_SetSwitchState(7,0,0);  /* GBP1 High Level Switch  */
	Scorpion2_SetSwitchState(7,1,0);  /* 20P High Level Switch */
	Scorpion2_SetSwitchState(7,2,0);  /* Token Front High Level Switch */
	Scorpion2_SetSwitchState(7,3,0);  /* Token Rear High Level Switch */

	m_sc2_show_door   = 1;
	m_sc2_door_state  = 0x31;

	m_has_hopper = 0;
	sc2_find_project_string();
}

DRIVER_INIT_MEMBER(bfm_sc2_state,ofah)
{
	sc2awpdmd_common_init(4, 1);

	Scorpion2_SetSwitchState(4,0,1);  /* pay tube low (1 pound front) */
	Scorpion2_SetSwitchState(4,1,1);  /* pay tube low (20p) */
	Scorpion2_SetSwitchState(4,2,1);  /* pay tube low (?1 right) */
	Scorpion2_SetSwitchState(4,3,1);  /* pay tube low (?1 left) */

	Scorpion2_SetSwitchState(6,0,0);  /* ? percentage key */
	Scorpion2_SetSwitchState(6,1,1);
	Scorpion2_SetSwitchState(6,2,0);
	Scorpion2_SetSwitchState(6,3,1);

	sc2_find_project_string();
}

DRIVER_INIT_MEMBER(bfm_sc2_state,prom)
{
	sc2awpdmd_common_init(6, 1);

	Scorpion2_SetSwitchState(4,0,1);  /* pay tube low (1 pound front) */
	Scorpion2_SetSwitchState(4,1,1);  /* pay tube low (20p) */
	Scorpion2_SetSwitchState(4,2,1);  /* pay tube low (?1 right) */
	Scorpion2_SetSwitchState(4,3,1);  /* pay tube low (?1 left) */

	Scorpion2_SetSwitchState(6,0,0);  /* ? percentage key */
	Scorpion2_SetSwitchState(6,1,1);
	Scorpion2_SetSwitchState(6,2,0);
	Scorpion2_SetSwitchState(6,3,1);

	sc2_find_project_string();
}

DRIVER_INIT_MEMBER(bfm_sc2_state,bfmcgslm)
{
	sc2awp_common_init(6, 1);
	m_has_hopper = 0;
	sc2_find_project_string();
}

DRIVER_INIT_MEMBER(bfm_sc2_state,luvjub)
{
	sc2awpdmd_common_init(6, 1);

	m_has_hopper = 0;

	Scorpion2_SetSwitchState(3,0,1);
	Scorpion2_SetSwitchState(3,1,1);

	Scorpion2_SetSwitchState(4,0,1);
	Scorpion2_SetSwitchState(4,1,1);
	Scorpion2_SetSwitchState(4,2,1);
	Scorpion2_SetSwitchState(4,3,1);

	Scorpion2_SetSwitchState(6,0,1);
	Scorpion2_SetSwitchState(6,1,1);
	Scorpion2_SetSwitchState(6,2,1);
	Scorpion2_SetSwitchState(6,3,0);

	Scorpion2_SetSwitchState(7,0,0);
	Scorpion2_SetSwitchState(7,1,0);
	Scorpion2_SetSwitchState(7,2,0);
	Scorpion2_SetSwitchState(7,3,0);
	sc2_find_project_string();
}


//these differ by only two bytes, and with no obvious labelling, this has been a bit of a guess
#define sc2_gslam_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "grandslamsnd.bin", 0x0000, 0x080000, CRC(e4af3787) SHA1(9aa40f7c4c4db3618b553505b02663c1d5f297c3) )
#define sc2_gslam_sound_alt \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "gslamsndb.bin", 0x0000, 0x080000, CRC(c9dfb6f5) SHA1(6e529c210b26e7ce164cebbff8ec314c6fa8f7bf) )

#define sc2_catms_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "catandmousesnd.bin", 0x0000, 0x080000, CRC(00d3b224) SHA1(5ae35a7bfa65e8343564e6f6a219bc674710fadc) )
#define sc2_gsclb_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "95004024.bin", 0x0000, 0x080000, CRC(e1a0323f) SHA1(a015d99c882962651869d8ec71a6c17a1cba687f) )
#define sc2_cpg_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "pharaohsgoldsnd.bin", 0x0000, 0x080000, CRC(7d67d53e) SHA1(159e0e9af1cfd6adc141daaa0f75d38af55218c3) )
#define sc2_suprz_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "surprisesurprizesnd.bin", 0x0000, 0x01fedb, CRC(c0981343) SHA1(71278c3446cf204a31415dd2ed8f1de7f7a16645) )
#define sc2_motd_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "modsndf.bin", 0x0000, 0x080000, CRC(088471f5) SHA1(49fb22daf04450186e9a83aee3312bb85ccf6842) )
#define sc2_easy_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "easy-money_snd.bin", 0x0000, 0x080000, CRC(56d224c5) SHA1(43b81a1a9a7d30ef7bfb2bbc61e3106faa927778) )
#define sc2_luvv_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD("snd.bin",      0x00000, 0x80000, CRC(19efac32) SHA1(26f901fc11f052a4d3cff67f8f61dcdd04f3dc22))
#define sc2_ofool_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "onlyfools_snd.bin", 0x0000, 0x080000, CRC(c073bb0c) SHA1(54b3df8c8d814af1fbb662834739a32a693fc7ee) )
#define sc2_ofool_matrix\
	ROM_REGION( 0x20000, "matrix", 0 )\
	ROM_LOAD( "onlyfoolsnhorsesdotmatrix.bin", 0x0000, 0x010000, CRC(521611f7) SHA1(08cdc9f7434657151d90fcfd26ce4668477c2998) )
#define sc2_town_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "attsnd.bin", 0x0000, 0x040000, CRC(9b5327c8) SHA1(b9e5aeb3e9a6ece796e9164e425829d97c5f3a82) )
#define sc2_cpe_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD("cpe1_snd.bin",  0x00000, 0x80000, CRC(ca8a56bb) SHA1(36434dae4369f004fa5b4dd00eb6b1a965be60f9))
#define sc2_cpe_sound_alt1 \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "pen1c_snd.bin", 0x0000, 0x080000, CRC(57f3d152) SHA1(f5ccd11042d54396352df149e85c4aa271342d49) )

#define sc2_cpe_sound_alt2 \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "95004012.p1", 0x0000, 0x080000, CRC(30d1f22a) SHA1(73cb2d12b090841a12a2ed21653248f41d02e125) )

#define sc2_cops_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "copssnd.bin", 0x0000, 0x040000, CRC(4bebbc37) SHA1(10eb8542a9de35efc0f75b532c94e1b3e0d21e47) )
#define sc2_copcl_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "club-cops-and-robbers-sound.bin", 0x0000, 0x040000, CRC(b5ba009d) SHA1(806b1d739fbf00b7e55ed0b8056440e47bfba87a) )
//missing a sound rom - is it the same as the non-deluxe version?
#define sc2_copdc_sound\
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
//can't tell any difference between these audibly, could one have the 'ruder' samples dummied out in the code?
//For now, I'm putting the first ROM with Bellfruit sets, and the second with Mazooma ones
#define sc2_dels_sound\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "dmsnd.bin", 0x0000, 0x080000, CRC(0a68550b) SHA1(82a4a8d2a754a59da553b3568df870107e33f978) )
#define sc2_dels_sound_alt\
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "delssnd.bin", 0x0000, 0x080000, CRC(cb298f06) SHA1(fdc857101ad15d58aeb7ffc4a489c3de9373fc80) )
#define sc2_wembl_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "wembley_sound.bin", 0x0000, 0x080000, CRC(5ce2fc50) SHA1(26533428582058f0cd618e3657f967bc64e551fc) )
#define sc2_prem_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "premclubsnd.bin", 0x0000, 0x080000, CRC(b20c74f1) SHA1(b43a79f8f59387ef777fffd07a39b7333811d464) )
#define sc2_downt_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "dtownsnd.dat", 0x0000, 0x080000, CRC(a41b109b) SHA1(22470d731741521321d004fc56ff8217e506ef69) )
#define sc2_goldr_sound\
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "gold_reserve_snd", 0x0000, 0x080000, CRC(e8e7ab7b) SHA1(ce43e8ffccc0421548c6683a72267b7e5f805db4) )
#define sc2_hifly_sound\
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "hiflyersound.bin", 0x0000, 0x080000, CRC(acdef7dc) SHA1(c2cc219ca8f4a3e3cdcb1147ad49cd69adb3751b) )
#define sc2_inst_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "instantjackpotssnd.bin", 0x0000, 0x080000, CRC(ba922860) SHA1(7d84c7fa72b1fb567faccf8464e0fd859c76838d) )
#define sc2_mam_sound\
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "mamsnd.bin", 0x0000, 0x080000, CRC(32537b18) SHA1(c26697162edde97ec999ed0459656edb85a01a50) )
//This was also in the non-club, so may be an alt set
#define sc2_mamcl_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "cmamsnd.bin", 0x0000, 0x080000, CRC(9a80977a) SHA1(0a6dc9465efa9e3d12894daf88a2746e74409349))
#define sc2_showt_sound\
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "stspec", 0x0000, 0x080000, CRC(01e4a017) SHA1(f2f0cadf2334edf35db98af0dcb6d827c991f3f2) )
#define sc2_sstar_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "superstarsnd.bin", 0x0000, 0x080000, CRC(9a2609b5) SHA1(d29a5029e39cd44739682954f034f2d1f2e1cebf) )
//missing
#define sc2_wwcl_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "club-wild-west_sound.bin", 0x0000, 0x080000, NO_DUMP )

//is this upd?
#define sc2_dick_sound \
	ROM_REGION( 0x100000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "global-spotted-dick_snd.bin", 0x0000, 0x100000, CRC(f2c66aab) SHA1(6fe94a193779c91711588365591cf42d197cb7b9) )

//is this upd?
#define sc2_pick_sound\
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "pickofthebunchsnd1.bin", 0x000000, 0x100000, CRC(f717b9c7) SHA1(06c90cc9779d475100926e986c742f0acffa0dc3) )\
	ROM_LOAD( "pickofthebunchsnd2.bin", 0x100000, 0x100000, CRC(eaac3e67) SHA1(3aaed6514eeeb41c26f365789d8736908785b1c2) )
//Is this upd?
#define sc2_rock_sound \
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "hbiyr_snd.bin", 0x0000, 0x100000, CRC(96cc0d54) SHA1(612f8c7f353bb847c1a28e2b76b64916d5b2d36a) )
//this is a guess
#define sc2_gcclb_sound\
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "gold_cas.snd", 0x0000, 0x080000, CRC(d93d39fb) SHA1(ce0c0c1430a6136ce39ffae018b009e629cbad61) )\
	ROM_REGION( 0x80000, "altupd", 0 )/* looks bad */ \
	ROM_LOAD( "95004065.p1", 0x0000, 0x080000, CRC(2670726b) SHA1(0f8045c68131191fceea5728e14c901d159bfb57) )
#define sc2_gcclb_matrix \
	ROM_REGION( 0x20000, "matrix", 0 ) \
	ROM_LOAD( "95000589.p1", 0x0000, 0x010000, CRC(36400074) SHA1(611b48650e59b52f661be2730afaef2e5772607c) )

// The below file also matches superstarsnd.bin
#define sc2_cb7_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )\
	ROM_LOAD( "casinobar7_bfm_snd1.bin", 0x0000, 0x080000, CRC(9a2609b5) SHA1(d29a5029e39cd44739682954f034f2d1f2e1cebf) )
// The below file also matches football-club_mtx_ass.bin
#define sc2_foot_matrix \
	ROM_REGION( 0x20000, "matrix", 0 )\
	ROM_LOAD( "95000590.p1", 0x0000, 0x010000, CRC(6b78de57) SHA1(84638836cdbfa6e4b3b76cd38e238d12bb312c53) )
ROM_START( sc2brkfs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ar_var_a.bin",  0x00000, 0x10000, CRC(5f016daa) SHA1(25ee10138bddf453588e3c458268533a88a51217) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big-breakfast_dat_ar_var_a.bin", 0x0000, 0x010000, CRC(ade2834f) SHA1(54914fbc8416b2d08c13c56088b1665e267e6777) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfsm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigbreakfastcasino", 0x0000, 0x010000, CRC(db45b17b) SHA1(927513f6fe326b216b0f13f34bbbc9970ab4f0cc) )

	ROM_REGION( 0x80000, "upd", 0 ) // might not be right for this version
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfsm1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400020", 0x0000, 0x010000, CRC(7a18f268) SHA1(ad352d613333072c62c38a493cf3183d387b7562) )

	ROM_REGION( 0x80000, "upd", 0 ) // might not be right for this version
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfsm2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400022", 0x0000, 0x010000, CRC(66482cbb) SHA1(933d8ec98d5bc3026d547b657093e07f96fbdafa) )

	ROM_REGION( 0x80000, "upd", 0 ) // might not be right for this version
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ss_var_a.bin",  0x00000, 0x10000, CRC(08d1fa7d) SHA1(a3dba79eef32835f0b46dbd7b376b797324df904) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big-breakfast_dat_ss_var_a.bin", 0x0000, 0x010000, CRC(57aff227) SHA1(5d4c6190194719b3fa5c02d30e7c6b59978c93c3) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_var_jp-8_a.bin", 0x00000, 0x10000, CRC(2671af1b) SHA1(0a34dd2953a99be9fb2a128f9d1f7ddc0fc8242a) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_8pnd20p_a.bin",  0x00000, 0x10000, CRC(054c38ad) SHA1(f4ab55f977848e3d2a933bba1ab619ffa3e14db6) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big-breakfast_dat_ac_var_8pnd_a.bin", 0x0000, 0x010000, CRC(d97dbf7a) SHA1(d46270ff69cbc636744fc902d38cc282613cfdd2) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END


ROM_START( sc2brkfs4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_var_10pnd-20p_a.bin",    0x00000, 0x10000, CRC(d879feaa) SHA1(2656fbe018fe40194c2b77d289b77fabbc9e537c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big-breakfast_dat_ac_var_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(a5967b05) SHA1(f0d4bc804181781a391fa052251c4bbf7d8f5e50) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END


ROM_START( sc2brkfs5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_10pnd-20p_a.bin",    0x00000, 0x10000, CRC(55d7321c) SHA1(0b4a6b66aa64fbb3238539a2167f761d0910b814) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2brkfs5p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big-breakfast_dat_ac_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(cc54617f) SHA1(078e56b948d68ebcfaf986dd0f15be64607d0e4f) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END


ROM_START( sc2brkfs6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_ac_8pnd-20p_ass.bin", 0x0000, 0x010000, CRC(86baaf46) SHA1(acb9c5cad4c35621219380a997ae67accaea4206) ) // wrong name, it's big breakfast

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( sc2drwho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750288.bin",    0x00000, 0x10000, CRC(fe95b5a5) SHA1(876a812f69903fd99f896b35eeaf132c215b0035) ) // dr-who-time-lord_std_ss_20p_ass.bin


	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750661.p1", 0x00000, 0x10000, CRC(4b5b50eb) SHA1(fe2b820c214b3e967348b99ccff30a4bfe0251dc) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_dat_ac_ass.bin", 0x00000, 0x10000, CRC(5a467a44) SHA1(d5a3dcdf50e07e36187350072b5d82d620f8f1d8) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwhop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_dat_ss_20p_ass.bin", 0x00000, 0x10000, CRC(8ce06af9) SHA1(adb58507b2b6aae59857384748d59485f1739eaf) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_std_ac_ass.bin", 0x00000, 0x10000, CRC(053313cc) SHA1(2a52b7edae0ce676255eb347bba17a2e48c1707a) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_std_var_20p_ass.bin",    0x00000, 0x10000, CRC(35f4e6ab) SHA1(5e5e35889adb7d3384aae663c667b0251d39aeee) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_20p_uk94_ass.bin",  0x00000, 0x10000, CRC(e65717c2) SHA1(9b8db0bcac9fd996de29527440d6af3592102120) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho7p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ac_10pnd-20p-25p_ass.bin",  0x00000, 0x10000, CRC(9a27ac6d) SHA1(d1b0e85d41198c5d2cd1b492e53359a5dc1ac474) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho5p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ac_8pnd-20p_ass.bin",   0x00000, 0x10000, CRC(b6629b5e) SHA1(d20085b4ab9a0786063eb063f7d1df2a6814f40c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho6p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ar_10p_ass.bin",    0x00000, 0x10000, CRC(04653c3b) SHA1(0c23f939103772fac628342074de820ec6b472ce) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ar_20p_uk94_ass.bin",   0x00000, 0x10000, CRC(40aaa98f) SHA1(80705e24e419558d8a7b1f886bfc2b3ce5465446) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_var_no-jp-spin_ass.bin",    0x00000, 0x10000, CRC(bf087547) SHA1(f4b7289a76e814af5fb3affc360a9ac659c09bbe) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_20p_uk94_ass.bin",  0x00000, 0x10000, CRC(278f559e) SHA1(d4396df02a5e24b3684c26fcaa57c8e499789332) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ac_8pnd-20p_ass.bin",   0x00000, 0x10000, CRC(0b2850c8) SHA1(5fac64f35a6b6158d8c15f41e82574768b1c3617) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ar_10p_ass.bin",    0x00000, 0x10000, CRC(f716a21d) SHA1(340df4cdea3309bfebeba7c419057f1bf5ed5024) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwho3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ar_20p_uk94_ass.bin",   0x00000, 0x10000, CRC(8dd0f908) SHA1(2eca748874cc061f9a8145b081d2c097a40e1e47) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwhodx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("tmld5pa", 0x00000, 0x10000, CRC(b9ddfd0d) SHA1(915afd83eab330a0e70635c35f031f2041b9f5ad) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwhomzp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98401002.bin", 0x0000, 0x010000, CRC(e7c23331) SHA1(f6823fa206d28f53a13ef44c9e4cf37d6b8aa758) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwhomz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400002.bin", 0x0000, 0x010000, CRC(40cc7d8b) SHA1(05f98e29bb92b3581691ee6df8ff5ae73e351d40) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( sc2drwhodx1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tlrdx56c", 0x0000, 0x010000, CRC(80da4ba0) SHA1(0c725da5eead9371d895ca9650fbbec8aa1509b2) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END


/* not encrypted, bootleg? */
ROM_START( sc2drwhou )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("drwho.bin",   0x00000, 0x10000, CRC(9e53a1f7) SHA1(60c6aa226c96678a6e487fbf0f32554fd85ebd66) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END



ROM_START( sc2focus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("focus.bin",    0x00000, 0x10000, CRC(ddd1a21e) SHA1(cbb467b03642d6de37f6dc204b902f2d7e92230e))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("focsound.bin", 0x00000, 0x20000, CRC(fce86700) SHA1(546680dd85234608c1b7e850bad3165400fd981c))
ROM_END

ROM_START( sc2gslam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-slam_std_ac_ass.bin", 0x0000, 0x010000, CRC(b28dcd9c) SHA1(f20ef0f0a1b5cc287cf93a175fede98dde3fecf4) )

	sc2_gslam_sound
ROM_END

ROM_START( sc2gslamp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-slam_dat_acss.bin", 0x0000, 0x010000, CRC(82ff3cb9) SHA1(87794063421724201c8a3e67cd6e454b0f578c3e) )

	sc2_gslam_sound
ROM_END

ROM_START( sc2gslam1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750843.bin", 0x00000, 0x10000, CRC(e159ddf6) SHA1(c897564a956becbd9d4c155df33b239e899156c0))

	sc2_gslam_sound_alt
ROM_END

ROM_START( sc2gslam1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-slam_dat_ac_var_rot_ass.bin", 0x0000, 0x010000, CRC(d505db66) SHA1(6e40186a699a81138674e332acbd0d7d3939b9f6) )

	sc2_gslam_sound_alt
ROM_END


ROM_START( sc2cshcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club_cashino_std_ac_f65_rot_ass.bin", 0x0000, 0x010000, CRC(23aa2c72) SHA1(155df9b501cf5ae9eb3afca48c4100617793ac09) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cashsnd", 0x0000, 0x080000, CRC(807d37a6) SHA1(bd5f7c39a64a562e96a850a2cc82bfe3f74f1e54) )
ROM_END

ROM_START( sc2cshclp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club_cashino_dat_ac_f65_rot_ass.bin", 0x0000, 0x010000, CRC(c2552162) SHA1(2c373b60588d870acd34d88025f6bb14687694fb) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cashsnd", 0x0000, 0x080000, CRC(807d37a6) SHA1(bd5f7c39a64a562e96a850a2cc82bfe3f74f1e54) )
ROM_END

ROM_START( sc2cshcl1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club_cashino_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(0e9fad24) SHA1(d14569f106ba29f9cb7769234f5531382e28bd69) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cashsnd", 0x0000, 0x080000, CRC(807d37a6) SHA1(bd5f7c39a64a562e96a850a2cc82bfe3f74f1e54) )
ROM_END

ROM_START( sc2cshcl1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club_cashino_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(b529604e) SHA1(87f8dca7e570472697de2cbe7565a038503a6251) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cashsnd", 0x0000, 0x080000, CRC(807d37a6) SHA1(bd5f7c39a64a562e96a850a2cc82bfe3f74f1e54) )
ROM_END

	//There are two distinct builds here, one clearly marked up as mark 2. For sanity's sake, though they share sound, I'm assigning them as separate entities

ROM_START( sc2catms )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat and mouse p1.bin", 0x0000, 0x010000, CRC(b33b2a75) SHA1(ac57b4d33ac1218e39b8bbd669c40bdbb3839ccf) )

	sc2_catms_sound
ROM_END

ROM_START( sc2catms1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse_std_ac_10pnd-25p_ass.bin", 0x0000, 0x010000, CRC(4c538143) SHA1(4045599cfe57f442ac58aa1f0ed3a03ce63e2e4c) )

	sc2_catms_sound
ROM_END

ROM_START( sc2catms1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse_dat_ac_10pnd-25p_ass.bin", 0x0000, 0x010000, CRC(d9811472) SHA1(dffab64155ed2c5193c24a660af7ad7c3c7bc093) )

	sc2_catms_sound
ROM_END

ROM_START( sc2catms2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(05396936) SHA1(61d976c22ba82bbff12fdcfb6b9320efebc9ad37) )

	sc2_catms_sound
ROM_END

ROM_START( sc2catms2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(40ba729e) SHA1(d7b4fe209588d77921d6c37d1739805aed80f103) )

	sc2_catms_sound
ROM_END

ROM_START( sc2catms3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm20std", 0x0000, 0x010000, CRC(74ca0fd5) SHA1(2345bf3810820a12c613013fedad936ab9134b22) )

	sc2_catms_sound
ROM_END


ROM_START( sc2ctms2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cnm20mk2", 0x0000, 0x010000, CRC(0604a78a) SHA1(c75b90f93b1d36928ad46643cfce03dda2b20408) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms21 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-mk2_std_ar_ac_8pnd-20p_uk94_ass.bin", 0x0000, 0x010000, CRC(c5fccfb0) SHA1(c427b42da60cd14516991a08a08f68421fa9ff88) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms21p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-mk2_dat_ar_ac_8pnd-20p_uk94_ass.bin", 0x0000, 0x010000, CRC(87b5fc94) SHA1(3e2b4aba0847fe1958710bff394ea98e02276b43) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms22 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-mk2_std_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(95beca0c) SHA1(6e2b175139c616cf80f020588b073f325a0c2684) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms22p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-mk2_dat_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(22e2d319) SHA1(ca3f335f9f52cd152e420bd6c2e15fc1fac4eb29) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms23 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-mk2_std_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(d8e72750) SHA1(b0431cbb311c88b4701bae3bbfdf1d45a070181c) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms23p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-mk2_dat_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(6806cfad) SHA1(8eb427688bc19e9b1508de1afa584bcba7e8d421) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms24p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-mk2_dat_ar_10p_ass.bin", 0x0000, 0x010000, CRC(c332595b) SHA1(3ea62b98129913b2ff576c42cfa7fe4d15a34b8e) )

	sc2_catms_sound
ROM_END

ROM_START( sc2ctms25 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat and mouse ver puss7.2.bin", 0x0000, 0x010000, CRC(6968bf9c) SHA1(c44faf2e5b391bee43021ad8544fb8d502f90433) )

	sc2_catms_sound
ROM_END

ROM_START( sc2eggs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eggs-on-legs_std_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(3fdad116) SHA1(d5fc405af8b14d8b85acb10aaa3c8a219753c864) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "eggsonlegssnd.bin", 0x0000, 0x080000, CRC(24fef504) SHA1(75a05e0cf064f736dd9164c24ccef77a46aaee94) )
ROM_END

ROM_START( sc2eggsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eggs-on-legs_dat_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(cdde5a4d) SHA1(b61e61193db4921217a7c285fd8fe2780d1f8091) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "eggsonlegssnd.bin", 0x0000, 0x080000, CRC(24fef504) SHA1(75a05e0cf064f736dd9164c24ccef77a46aaee94) )
ROM_END

ROM_START( sc2eggs1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750746.p1", 0x0000, 0x010000, CRC(a4b13487) SHA1(7ef2953ca11526bbae57b1aebb7a90de59c2d379) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "eggsonlegssnd.bin", 0x0000, 0x080000, CRC(24fef504) SHA1(75a05e0cf064f736dd9164c24ccef77a46aaee94) )
ROM_END

ROM_START( sc2eggs1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eggs-on-legs_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(718915f2) SHA1(717b57c0e81a48db005516135fdd4d82f7cfda28) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "eggsonlegssnd.bin", 0x0000, 0x080000, CRC(24fef504) SHA1(75a05e0cf064f736dd9164c24ccef77a46aaee94) )
ROM_END

ROM_START( sc2gsclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_std_ac_p65_ass.bin", 0x0000, 0x010000, CRC(9a390095) SHA1(ee4b08956de0b018b9ceaf16a6410463053c1f3d) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclbp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_dat_ac_p65_ass.bin", 0x0000, 0x010000, CRC(61adb76f) SHA1(a7fcc6504d5eeae664b9aaca190bbf43bd989c93) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_std_ac_250pnd-24p_p65_ass.bin", 0x0000, 0x010000, CRC(142d828a) SHA1(2fe40e9d641be1cf89cfe9fe5cd4b29dd9ea01e7) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_dat_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(5d59e87e) SHA1(91684551db11d95768c364515cf5cd337b3f482b) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(01ae9d52) SHA1(3b85a7ebc346d4eb6a16b2b9a03aa12220020aff) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(85cf033f) SHA1(ca7e506437e1ff229f2d79bedb13ae0fe5dd2696) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_std_fe_ac_ass.bin", 0x0000, 0x010000, CRC(6e479cc4) SHA1(99c15b0d1584ab7b460f273de825eb17681c5d0a) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_dat_fe_ac_ass.bin", 0x0000, 0x010000, CRC(b5a03c26) SHA1(ef1bc28905a8a9db71299f5c30a15c5576766346) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_std_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(d2819fc3) SHA1(23c7cbf9e04913f5cb62ef6accdd5b470eed3cd4) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_dat_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(7e003d2a) SHA1(f8a6f6810b1733f46e470e89fa821cd51fbe1c5e) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gameshow.bin", 0x0000, 0x010000, CRC(babeb912) SHA1(41bc1cf82bef84f840998af1278c55ea1727a163) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750844.p1", 0x0000, 0x010000, CRC(36efa743) SHA1(0f5392f55e42d7ac17e179c966997f41859f925a) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb6p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-game-show_dat_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(299b89f3) SHA1(eb78378410ca2380ec564e8268a51309dc8044ce) )

	sc2_gsclb_sound
ROM_END

ROM_START( sc2gsclb7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gsho1_3", 0x0000, 0x010000, CRC(783ee8cb) SHA1(b509f167fddc71e313ffbff0a3e1ce7d387c424e) )

	sc2_gsclb_sound
ROM_END


ROM_START( sc2cpg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-pharaohs-gold_std_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(f83a68dc) SHA1(1a7aa08835d03116199034378ae0c617520a5ac6) )

	sc2_cpg_sound
ROM_END

ROM_START( sc2cpgp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-pharaohs-gold_dat_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(2de3b252) SHA1(02c3bfabd5c732e37e71278be5aad0b6b44d28c6) )

	sc2_cpg_sound
ROM_END

ROM_START( sc2cpg1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-pharaohs-gold_std_fe_ac_p65_rot_ass.bin", 0x0000, 0x010000, CRC(e97c5bb4) SHA1(4df5f50bbfe453fbc351855dc6f6a24296563498) )

	sc2_cpg_sound
ROM_END

ROM_START( sc2cpg1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-pharaohs-gold_dat_fe_ac_p65_rot_ass.bin", 0x0000, 0x010000, CRC(4ccba14d) SHA1(a0529a732a1a8c5c9a3d9830072ff1003c80b7d2) )

	sc2_cpg_sound
ROM_END

ROM_START( sc2cpg2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-pharaohs-gold_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(9376c3c4) SHA1(9e67c982dfb838cde538d0893ea36eafe8bda2d3) )

	sc2_cpg_sound
ROM_END

ROM_START( sc2cpg2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-pharaohs-gold_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(bb790c4b) SHA1(d1126b9848047f15a65119e6446caced2c982287) )

	sc2_cpg_sound
ROM_END


ROM_START( sc2suprz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprise-surprize_std_ga_20p_ass.bin", 0x0000, 0x010000, CRC(7e52c975) SHA1(a610f7170fda13f64e805e3d99b5f57c61206cfe) )

	sc2_suprz_sound
ROM_END

ROM_START( sc2suprzp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprise-surprize_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(8ee54a57) SHA1(471a06d9840ecbf850c8896f8bf45264c0b8390f) )

	sc2_suprz_sound
ROM_END

ROM_START( sc2suprz1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprise-surprize_std_var_ass.bin", 0x0000, 0x010000, CRC(5ef85273) SHA1(2ca9e3245c97fbed97a781e135fbb79df5b1bf18) )

	sc2_suprz_sound
ROM_END

ROM_START( sc2suprz1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprise-surprize_dat_var_ass.bin", 0x0000, 0x010000, CRC(37ab423e) SHA1(6b2ab927eb851b8f77eb474a1c5b68c335a17b2f) )

	sc2_suprz_sound
ROM_END

ROM_START( sc2suprz2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprise-surprize_std_ac_6pnd-20p_ass.bin", 0x0000, 0x010000, CRC(297959d7) SHA1(9bc8bc3d1be1f282573a3ad6994f06ee7bb64dfd) )

	sc2_suprz_sound
ROM_END

ROM_START( sc2suprz2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprise-surprize_dat_ac_6pnd-20p_ass.bin", 0x0000, 0x010000, CRC(7e0b263e) SHA1(bcbd82a87e7db65db22e55d9111b0f819a62150a) )

	sc2_suprz_sound
ROM_END

ROM_START( sc2suprz3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "surprise-surprize-6pound.bin", 0x0000, 0x010000, CRC(d00de4ab) SHA1(cdee9c2c27ab6bad8b0c633ce396fbe2987dbb61) )

	sc2_suprz_sound
ROM_END


ROM_START( sc2motd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_ac_10pnd_uk94_ass.bin", 0x0000, 0x010000, CRC(f75d128d) SHA1(7da2fb6bc7265848c20cfc137de846439af83b90) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motdp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_ac_10pnd_uk94_ass.bin", 0x0000, 0x010000, CRC(632325d8) SHA1(92c68b51b4e594bec5d9af43a697a4dd912ed864) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(10b7a217) SHA1(615bf8e6d1b79c96efd91335a9c6f5db0df95891) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(948b3ede) SHA1(f1c7b4e9fb83ba848d4d8a3ab02a1a5e3b630054) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(b059fe93) SHA1(33d15c464f3f80f4600d961ddade0b6a661747ba) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(412a30ed) SHA1(c7118954c086fb1243e441ed7728d801667e98ba) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(96687a5a) SHA1(dafd7b0af3e26d609b5927c431f4adf2f424322a) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_ar_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(f5adb4aa) SHA1(85afff3251e13808f140d6e58f1c9e2e23ce9d8c) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_ar_20p_ass.bin", 0x0000, 0x010000, CRC(27f942a3) SHA1(928d3c2eef6b202c0d71b0843f64aba15aab4f42) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_ar_20p_ass.bin", 0x0000, 0x010000, CRC(ab1c44b9) SHA1(ce34570fabcb2c6ceab48ef7c4367ccafa95ef1a) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_ss_20p_ass.bin", 0x0000, 0x010000, CRC(ce926573) SHA1(dff243d0eb12d4c13c8334099c5958e897cb8bd5) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd5p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_ss_20p_ass.bin", 0x0000, 0x010000, CRC(19dafe2d) SHA1(8a7bc4bfb7acd5386fdcadf91c2ba4f5615fa3c9) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_8pnd-20p_ass.bin", 0x0000, 0x010000, CRC(8042a61d) SHA1(3e0e75918d6df2d4ed537ee532d1a7fa0bb359b7) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd6p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_8pnd-20p_ass.bin", 0x0000, 0x010000, CRC(da77960d) SHA1(e6fc97994612d9280b60df6600c26aa7919381d2) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_std_20p_ass.bin", 0x0000, 0x010000, CRC(441931ef) SHA1(9c8c79470dda2a6589d04e4eb8d00d8a984bd1ed) )

	sc2_motd_sound
ROM_END




ROM_START( sc2motd8p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_20p_uk94_ass.bin", 0x0000, 0x010000, CRC(fa9216fa) SHA1(3d5d164419f022488e60e738958d3f66f4206e87) )

	sc2_motd_sound
ROM_END

ROM_START( sc2motd9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "motd6ac", 0x0000, 0x010000, CRC(d8e7811c) SHA1(ac67683984465aaf8a96322e71ab7b7bffe92361) )

	sc2_motd_sound
ROM_END




ROM_START( sc2easy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "easy-money_std_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(e9f581ca) SHA1(aee8a1af609921a0b33db7b460e4a58517bf9276) )

	sc2_easy_sound
ROM_END

ROM_START( sc2easyp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "easy-money_dat_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(e5633ac3) SHA1(d868d782e7d5f6c62ab8958150857336b7acff97) )

	sc2_easy_sound
ROM_END

ROM_START( sc2easy1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "easy-money_std_wi_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(38434925) SHA1(17148ba440c8fd139f7889a211a914ed679a195f) )

	sc2_easy_sound
ROM_END

ROM_START( sc2easy1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "easy-money_dat_wi_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(f841d5cf) SHA1(05afdfa483271635b530652385e2e566920e533d) )

	sc2_easy_sound
ROM_END

ROM_START( sc2easy2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "easy-money_std_wi_ac_var_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(05622afc) SHA1(169a492870a70aeb17078b2b27c36f5b82274b3f) )

	sc2_easy_sound
ROM_END

ROM_START( sc2easy2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "easy-money_dat_wi_ac_var_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(97f62e2d) SHA1(0884ddd0b25e78dd402983158e8c623ff4326cbd) )

	sc2_easy_sound
ROM_END

ROM_START( sc2majes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "majestic.p1", 0x0000, 0x010000, CRC(37289a5f) SHA1(a9d86ed16fc2ff2b83b60e48a1704b4e189c3ac7) )

ROM_END

ROM_START( sc2majesp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match-of-the-day_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(77710913) SHA1(709fff877ee863021e958bcecbd5cd58a977ea09) ) // wrong name, it's majestic bells

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "majesticsnd.bin", 0x0000, 0x080000, CRC(3ee3fee3) SHA1(6a5e72e8a808d870a84a0e3523eebfadfab6d5df) )
ROM_END


ROM_START( sc2luvv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("95750808.bin", 0x00000, 0x10000, CRC(e6668fc7) SHA1(71dd412114c6386cba72e2b29ea07f2d99d14065))

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("95000584.p1",  0x00000, 0x10000, CRC(cfdd7bb2) SHA1(90086aaff743a7b2385488af1e8a126029113028))//mtx_ass.bin

	sc2_luvv_sound
ROM_END

ROM_START( sc2luvv1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_std_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(e4440803) SHA1(be9b49cbe2cfcaa0e640365e190da9c3fcf82bea) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END



ROM_START( sc2luvv1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_dat_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(9dee74fc) SHA1(d29756d743b781ab9ce7baf990f4a2cc0e9d7972) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END

ROM_START( sc2luvv2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(d40a59d0) SHA1(7173fc6d349868b9194c4ad581762d299dfb1c69) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END

ROM_START( sc2luvv2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(886a3a8e) SHA1(4c986e0c7278bd058ce2df2d755cbc8e4f31b3fa) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END


ROM_START( sc2luvv4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_std_ac_4pnd-5p_ass.bin", 0x0000, 0x010000, CRC(065ee9bb) SHA1(5d46f0e1b5d48dc94b9843998dedf6d3dfc83e3c) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END

ROM_START( sc2luvvp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_dat_ac_10pnd-25p_ass.bin", 0x0000, 0x010000, CRC(355210a0) SHA1(c03e1109ee1a419fc4ebdcf861d5220303a9c587) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END

ROM_START( sc2luvv6p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(8c0a6180) SHA1(1c1ee2b5081ee901b5929405a78d3e7a7989916a) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END

ROM_START( sc2luvv4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luvvley-jubbley_dat_ac_4pnd-5p_ass.bin", 0x0000, 0x010000, CRC(4b3155b8) SHA1(aaba2e3d54a2b099b63ee4f5d3560d8eb562c4f1) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000575.p1", 0x0000, 0x010000, CRC(e4e06767) SHA1(bee2385c2a9c7ca39ff6a599f827ddba4324b903) )//luvvley-jubbley_mat_ass.bin

	sc2_luvv_sound
ROM_END

ROM_START( sc2ptytm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750806.p1", 0x0000, 0x010000, CRC(4e98c6c6) SHA1(7f4ec51f384b5203229da28f39c3127cd40cf67d) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000585.p1", 0x0000, 0x010000, CRC(0672a9f4) SHA1(9e8e01aaa081ffb68aa494fe9dbae0620da0f6b9) )//party-time_mtx_ass.bin

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "partysnd1.bin", 0x0000, 0x020000, CRC(b5a5cc9e) SHA1(c9b132ad0d1ce9ff6b56ebde89d5006a5cf7dff6) )
ROM_END

ROM_START( sc2ptytm1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "partytime.bin", 0x0000, 0x010000, CRC(20ef430c) SHA1(b5d35704da425e7ca84500071f34b4d65d87b9fa) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "partydot.bin", 0x0000, 0x010000, CRC(8a09b858) SHA1(bc932bebc7718da2b97e5f6ef06eb739748353f4) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "partysnd1.bin", 0x0000, 0x020000, CRC(b5a5cc9e) SHA1(c9b132ad0d1ce9ff6b56ebde89d5006a5cf7dff6) )
ROM_END

ROM_START( sc2ptytmp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "party-time_dat_ac_4pnd-10p_ass.bin", 0x0000, 0x010000, CRC(a33a6d08) SHA1(cf93f42971978b00a15e17d4da6bb6e16e8f1fab) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "partydot.bin", 0x0000, 0x010000, CRC(8a09b858) SHA1(bc932bebc7718da2b97e5f6ef06eb739748353f4) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "partysnd1.bin", 0x0000, 0x020000, CRC(b5a5cc9e) SHA1(c9b132ad0d1ce9ff6b56ebde89d5006a5cf7dff6) )
ROM_END


ROM_START( sc2ofool )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "onlyfoolsnhorses_std.bin", 0x0000, 0x010000, CRC(03cc611a) SHA1(e37d6b87017a52f8de339bbd69b2ccbff9872fae) )

	sc2_ofool_matrix
	sc2_ofool_sound
ROM_END

ROM_START( sc2ofool1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "game 147s only fools.bin", 0x0000, 0x010000, CRC(6cb6cef1) SHA1(bfa40f517b1455e4d563be5964605be63e950e87) )

	sc2_ofool_matrix
	sc2_ofool_sound
ROM_END

ROM_START( sc2ofool2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fools & horses 10m 6.bin", 0x0000, 0x010000, CRC(5fe48a02) SHA1(fd5b07a58567e0c5eb75bf1526a853b3a60ddfa9) )

	sc2_ofool_matrix
	sc2_ofool_sound
ROM_END

ROM_START( sc2ofool3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fools.bin", 0x0000, 0x010000, CRC(eaa0757a) SHA1(b6bec8f4f443d6c22c18e16ec0d65839fe30b61c) )

	sc2_ofool_matrix
	sc2_ofool_sound
ROM_END


ROM_START( sc2ofool4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fools scor ii 10p.bin", 0x0000, 0x010000, CRC(1d6245b7) SHA1(f73b4741cf07d96ec79d907b88d07cd20c748dd3) )

	sc2_ofool_matrix
	sc2_ofool_sound
ROM_END




ROM_START( sc2town )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round-the-town_std_ac_10pnd-20p-25p_ass.bin", 0x0000, 0x010000, CRC(8394c0e9) SHA1(b9b45e0c855a5f7270259543337fb441694b61e2) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "round-the-town_mtx.bin", 0x0000, 0x010000, CRC(aa6aac1d) SHA1(57ed376f602dd70495b3bd356bea5113fa8e861e) )

	sc2_town_sound
ROM_END

ROM_START( sc2townp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round-the-town_dat_ac_10pnd-20p-25p_ass.bin", 0x0000, 0x010000, CRC(8291ad4e) SHA1(cd304052123dfe6d8504a6f5e92413c569bcaf8e) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "round-the-town_mtx.bin", 0x0000, 0x010000, CRC(aa6aac1d) SHA1(57ed376f602dd70495b3bd356bea5113fa8e861e) )

	sc2_town_sound
ROM_END

ROM_START( sc2town1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round-the-town_std_ar_var_ass.bin", 0x0000, 0x010000, CRC(e5be3a13) SHA1(8a31c67641bce3c2160bb1c651535902374349b4) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END

ROM_START( sc2town1a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rtwn8arc.bin", 0x0000, 0x010000, CRC(b054b38e) SHA1(98aa68a4fb6db4a53a63a4976954277c082ee8bf) )
	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END


ROM_START( sc2town1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round-the-town_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(3d811bb4) SHA1(134e1c65f4f8377eca6d7ccfded5d4600d2949bf) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END

ROM_START( sc2town2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750069.p1", 0x0000, 0x010000, CRC(6bc0c2ff) SHA1(9a2bac50978f2b7d2072e0febe4bf4a935bf287d) )//round-the-town_std_ac_20p_20po_ass.bin

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END

ROM_START( sc2town3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round-the-town_std_var_ass.bin", 0x0000, 0x010000, CRC(1909994f) SHA1(47268e1119c808096ddff872e28444ed67bc5dbf) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END

ROM_START( sc2town3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round-the-town_dat_var_ass.bin", 0x0000, 0x010000, CRC(85110517) SHA1(30eba3987cc60ccbaecbc4c700bb2f1ba088d12f) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END

ROM_START( sc2town4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "atown20p", 0x0000, 0x010000, CRC(4f7ec25e) SHA1(52af065633942a9e4c195f3294b81ae57bf0c414) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END

ROM_START( sc2town5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rtt8ac", 0x0000, 0x010000, CRC(e495e5ea) SHA1(4fb6a43cee1c79ce05b71b35b195f2d35913c40c) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000581.p1", 0x0000, 0x010000, CRC(1a3b2fb1) SHA1(3d51c6e16558c1ac8ad852a461cd89aef9bc91e4) )//round-the-town_mtx_ass.bin

	sc2_town_sound
ROM_END


//Multiple matrix ROMS in the set, so bear in mind if something looks wrong, the other ones may be right. Tried to match set to label.
//Similarly, multiple sound ROMs mean there's been an attempt to organise them into logical sets.
ROM_START( sc2cpe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ce1std25p.bin", 0x00000, 0x10000, CRC(2fad9a49) SHA1(5ffb53031eef8778363836143c4e8d2a65361d51))

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("cpe1_mtx.bin",  0x00000, 0x10000, CRC(5fd1fd7c) SHA1(7645f8c011be77ac48f4eb2c75c92cc4245fdad4))

	sc2_cpe_sound
ROM_END

ROM_START( sc2cpep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_dat_ac_25p_ass.bin", 0x0000, 0x010000, CRC(00bedbdf) SHA1(97b3e23fed6692ae88e6a6110008124422478355) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("cpe1_mtx.bin",  0x00000, 0x10000, CRC(5fd1fd7c) SHA1(7645f8c011be77ac48f4eb2c75c92cc4245fdad4))

	sc2_cpe_sound
ROM_END

ROM_START( sc2cpe1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_std_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(2d56a73b) SHA1(31195fa16c1c95d49716448b80f1d0aa973f29d5) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000572.p1", 0x0000, 0x010000, CRC(551ef8ca) SHA1(825f4c3ff56cb2da20ffe1b2ec33f1692f6806b2) )

	ROM_REGION( 0x20000, "altmatrix", 0 )
	ROM_LOAD( "95000572.hex", 0x0000, 0x01be8c, CRC(e57e66b5) SHA1(f3e44cdb697e6e666bd0008824e802a2cf997aa5) )//club-public-enemy-no1_mtx_25pss.hex

	sc2_cpe_sound_alt1
ROM_END

ROM_START( sc2cpe1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_dat_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(131375cd) SHA1(4899e8dd4acec9563fa40109bb9b839c5d7209a8) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000572.p1", 0x0000, 0x010000, CRC(551ef8ca) SHA1(825f4c3ff56cb2da20ffe1b2ec33f1692f6806b2) )

	sc2_cpe_sound_alt1
ROM_END

ROM_START( sc2cpe2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_std_fe_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(0a36fd07) SHA1(6338858eb0dd6ba43bfea66afde0d6d1d5097aee) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000572.p1", 0x0000, 0x010000, CRC(551ef8ca) SHA1(825f4c3ff56cb2da20ffe1b2ec33f1692f6806b2) )

	sc2_cpe_sound_alt1
ROM_END

ROM_START( sc2cpe2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_dat_fe_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(5a79358b) SHA1(bf728108aad6937be0a5d79fa604f7ac3b191b42) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000572.p1", 0x0000, 0x010000, CRC(551ef8ca) SHA1(825f4c3ff56cb2da20ffe1b2ec33f1692f6806b2) )

	sc2_cpe_sound_alt1
ROM_END

ROM_START( sc2cpe3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_std_ac_200pnd_ass.bin", 0x0000, 0x010000, CRC(5704e52d) SHA1(dfae48734794cea2e9a952d808dedb96fd5204b3) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "matrix.bin", 0x0000, 0x010000, CRC(64014f73) SHA1(67d44db91944738fcadc38bfd0d2b7c0536adb9a) ) // seems to be from a cops+robbers instead, will say 'wrong display prom' during attract cycle

	sc2_cpe_sound_alt2
ROM_END

ROM_START( sc2cpe3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_dat_ac_200pnd_ass.bin", 0x0000, 0x010000, CRC(fec925a3) SHA1(5ce3b6f1236f511ae8975c7ecd1549e8d427a245) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "matrix.bin", 0x0000, 0x010000, CRC(64014f73) SHA1(67d44db91944738fcadc38bfd0d2b7c0536adb9a) ) // see above comment

	sc2_cpe_sound_alt2
ROM_END

ROM_START( sc2cpe4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750273.p1", 0x0000, 0x010000, CRC(950da13c) SHA1(2c544e06112969f7914a5b4fd15e6b0dfedf6b0b) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "matrix.bin", 0x0000, 0x010000, CRC(64014f73) SHA1(67d44db91944738fcadc38bfd0d2b7c0536adb9a) ) // see above comment

	sc2_cpe_sound_alt2
ROM_END

ROM_START( sc2cpe4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-public-enemy-no1_dat_fe_ac_200pnd_p65_rot_ass.bin", 0x0000, 0x010000, CRC(8d5ff953) SHA1(bdf6b5e014c46f6abac792a5913e98cb897b2a73) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "matrix.bin", 0x0000, 0x010000, CRC(64014f73) SHA1(67d44db91944738fcadc38bfd0d2b7c0536adb9a) ) // see above comment

	sc2_cpe_sound_alt2
ROM_END



//Multiple matrix roms again, best guesses based on labelling but may need to swap them about
//It'll probably becom clear when the casino hardware is working
ROM_START( sc2cops )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops-and-robbers_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(2a74bf68) SHA1(e6d0cf5c26815184d74bc2b1769d13321ce5e33a) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000578.p1", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )//official part number for cops-and-robbers-mtx-ass.bin, cops & robbers 10 p2 (27512

	sc2_cops_sound
ROM_END



ROM_START( sc2copsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops-and-robbers_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(6f544505) SHA1(177a8d4038759dc0e52c14b463aaa6afce81d338) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000578.p1", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )//official part number for cops-and-robbers-mtx-ass.bin, cops & robbers 10 p2 (27512

	sc2_cops_sound
ROM_END



ROM_START( sc2cops1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops-and-robbers_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(2e3d0614) SHA1(b8be9a1d0be643d0dde7f6d89c067af1e85018bf) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000578.p1", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )//official part number for cops-and-robbers-mtx-ass.bin, cops & robbers 10 p2 (27512

	sc2_cops_sound
ROM_END

ROM_START( sc2cops2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops1020", 0x0000, 0x010000, CRC(3219a07f) SHA1(1f775189b50eeb55c584dd1054c9119d02b2f738) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "copdot10", 0x0000, 0x010000, CRC(30c41ddd) SHA1(9aa66c30aa0fcbd3fb79a6d0d45d777a116f951c) )

	sc2_cops_sound
ROM_END

ROM_START( sc2cops3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops-and-robbers_std_ss_var_ass.bin", 0x0000, 0x010000, CRC(664216d2) SHA1(e222147d71f251554207627b7e5e9de5f10cfff8) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "copsdot8", 0x0000, 0x010000, CRC(0eff2127) SHA1(e9788999ac6006faf0eb4e9d8ef1fd52f092be5a) )

	sc2_cops_sound
ROM_END

ROM_START( sc2cops3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops-and-robbers_dat_ss_var_ass.bin", 0x0000, 0x010000, CRC(f14af5f8) SHA1(8bb4d9fc78f1f2c274c4b21c7f4e67c3856f0019) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "copsdot8", 0x0000, 0x010000, CRC(0eff2127) SHA1(e9788999ac6006faf0eb4e9d8ef1fd52f092be5a) )

	sc2_cops_sound
ROM_END

ROM_START( sc2cops4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops8ac", 0x0000, 0x010000, CRC(c2ef20ff) SHA1(3841fcaacb739ee90ddc064d42d3275dc6a64016) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "copsdot8", 0x0000, 0x010000, CRC(0eff2127) SHA1(e9788999ac6006faf0eb4e9d8ef1fd52f092be5a) )

	sc2_cops_sound
ROM_END

//Does this even need a matrix?
ROM_START( sc2cops5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops & robbers 6 25p (27512)", 0x0000, 0x010000, CRC(0ad3fedf) SHA1(25775a80272c72234be9f528cc8f13cf9e1adbf7) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "copsdot8", 0x0000, 0x010000, CRC(0eff2127) SHA1(e9788999ac6006faf0eb4e9d8ef1fd52f092be5a) )

	sc2_cops_sound
ROM_END

ROM_START( sc2copsc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-cops-and-robbers_std_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(549457c2) SHA1(271c7077fd3ee5de67c914faf095b5295dfb6207) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000578.p1", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )//official part number for cops-and-robbers-mtx-ass.bin, cops & robbers 10 p2 (27512

	sc2_cops_sound
ROM_END

ROM_START( sc2copscp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-cops-and-robbers_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(fadde12b) SHA1(9b041c932558a0132c853514ca3f325f6f97bc65) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000578.p1", 0x0000, 0x010000, CRC(bdd56a09) SHA1(92d0416578c55075a127f1c2af8d6de5216dd189) )//official part number for cops-and-robbers-mtx-ass.bin, cops & robbers 10 p2 (27512

	sc2_cops_sound
ROM_END

ROM_START( sc2copsc1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-cops-and-robbers_std_ms_to_8pnd_ass.bin", 0x0000, 0x010000, CRC(600a91fd) SHA1(b04bce98df824d2c217c70bd8a49349f93043360) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "copsdot8", 0x0000, 0x010000, CRC(0eff2127) SHA1(e9788999ac6006faf0eb4e9d8ef1fd52f092be5a) )

	sc2_cops_sound
ROM_END

ROM_START( sc2copsc1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-cops-and-robbers_dat_ms_to_8pnd_ass.bin", 0x0000, 0x010000, CRC(361ad99f) SHA1(444f2aeef404b087d49e2283bb36bde5e4e673ee) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "copsdot8", 0x0000, 0x010000, CRC(0eff2127) SHA1(e9788999ac6006faf0eb4e9d8ef1fd52f092be5a) )

	sc2_cops_sound
ROM_END

ROM_START( sc2copsc1pa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-cops-and-robbers_dat_ms_to_8pnd_ass.bin", 0x0000, 0x010000, CRC(361ad99f) SHA1(444f2aeef404b087d49e2283bb36bde5e4e673ee) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "casino-cops-n-robbers.rom", 0x0000, 0x010000, CRC(54a5168f) SHA1(dfc2bf940ced5a53255238cd9e7d0503e3227691) )

	sc2_cops_sound
ROM_END


//Some of these are labelled as for different cabinets, which probably means different reel motors or configurations
ROM_START( sc2copcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_250pnd-25p_phx_ass.bin", 0x0000, 0x010000, CRC(668def2e) SHA1(802ca565a20d0fce2f5e4340c646429af6aadff6) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copclp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_250pnd-25p_phx_ass.bin", 0x0000, 0x010000, CRC(f6e9a013) SHA1(02b6c203c3facdd7015ba1119bcb70bf34b4ec00) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(078651b5) SHA1(2acc45e5d66625753e5869f6f3ac1379d0c9dfcd) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_250pnd-20p_rot_ass.bin", 0x0000, 0x010000, CRC(05635f8b) SHA1(d3cf98e3858189db725621d4ba07728a585d7a3b) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(229c65c1) SHA1(8052c4b8702275235545807e7b075571fc97d4f3) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_fe_ac_p67_ass.bin", 0x0000, 0x010000, CRC(4906d170) SHA1(c304a2986560d675b2e776965fdf444e4d56f104) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_fe_ac_p67_ass.bin", 0x0000, 0x010000, CRC(327db998) SHA1(aa8583cedd52a3cd06be6423a32e48273ec6218a) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_var_p65_ass.bin", 0x0000, 0x010000, CRC(23d80392) SHA1(d7f5bab4fc8f42c1a38e26b54bc519e0f03d20bc) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_fe_ac_p63_ass.bin", 0x0000, 0x010000, CRC(fc7f9b85) SHA1(d9f940bca29919d097fa7d128869725e01d6dbc3) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_200pnd-20p_ass.bin", 0x0000, 0x010000, CRC(214cda40) SHA1(fc585f211256495bfaaa6cb6c4d9c8a110ab5051) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl6p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_200pnd-20p_ass.bin", 0x0000, 0x010000, CRC(b4071611) SHA1(2596ccee2b94bb56aa629ee892bd357b706005b0) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_var_200pnd_ijf_ass.bin", 0x0000, 0x010000, CRC(db5a287e) SHA1(5615480767348061b7f08a709a16aa0b9cf0658e) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_ffp_ass.bin", 0x0000, 0x010000, CRC(347255bf) SHA1(7f96277579e68bdf1e21788cc5e35941d98df87f) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl8p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_ac_ffp_ass.bin", 0x0000, 0x010000, CRC(ec92b62d) SHA1(f10bc8fa55cd59127f179a35a61c1a57597856b6) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_fe_ac_ass.bin", 0x0000, 0x010000, CRC(c7461e95) SHA1(f4088056e848742d3795f5b067476b56071f99bd) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl9p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_fe_ac_ass.bin", 0x0000, 0x010000, CRC(ead8cbe5) SHA1(5594eb9a736e0f15a6f0f097a8cbbd8352e46fc4) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_std_ac_npr_ass.bin", 0x0000, 0x010000, CRC(b9c0bcb4) SHA1(c1a398bd58097411b80d36030760e7820dc346f4) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl11 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops200", 0x0000, 0x010000, CRC(05d29adc) SHA1(06a986356c1b48ad5ee92c9a7f6fb2531e1806af) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl11p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cops-and-robbers_dat_fr_ac_p63_ass.bin", 0x0000, 0x010000, CRC(93965bfc) SHA1(52af75234f56a77f082132d9532d3ffcaef5d271) )
	sc2_copcl_sound
ROM_END

ROM_START( sc2copcl12 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "premier-club-manager_dat_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(6446176c) SHA1(17cccc00d443ffde11943ebda112ef1e79134455) ) // filename is wrong, this is a club cops n robbers
	sc2_copcl_sound
ROM_END

ROM_START( sc2copdc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd-25p_p67_ass.bin", 0x0000, 0x010000, CRC(fd19db9a) SHA1(441d80b8463ffd5f8783b3cb80d8321f64e8fcc5) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	ROM_REGION( 0x20000, "altmatrix", 0 )//HEX equivalent of above?
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25pss.hex", 0x0000, 0x01cfbf, CRC(b2abbab4) SHA1(40e202e1678f637f7c0097b4f8f4884de439935e) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdcp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd-25p_p67_ass.bin", 0x0000, 0x010000, CRC(734c5e16) SHA1(e6a6a31ef5156e207dd77c40f5b29b10ef4f9def) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(8f5396a6) SHA1(c7cd83bdeca3a852a8203330ca14574608b9a9e9) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd-25p_p65_ass.bin", 0x0000, 0x010000, CRC(f2433167) SHA1(88c90c047f67361e1974ea29a887f11c79c78b55) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(4e7da1cb) SHA1(1c61f47f30a9d27f558548c23ddf6de2e5366344) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(c5f6c4f6) SHA1(69be1c6f134406a5457cf4bd7ed78dc4524bac6d) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_250pnd_ass.bin", 0x0000, 0x010000, CRC(10a9d7d3) SHA1(7d147ce9c2c98f10694ee99e14286be3f74bbdf4) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_250pnd_ass.bin", 0x0000, 0x010000, CRC(6b899a10) SHA1(58b7e2e9eda0d3715de8a4af31b49e059942b6f2) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

//Cannot be sure the matrix rom matches these, but we have no alternative...
ROM_START( sc2copdc4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_20p_p63_ass.bin", 0x0000, 0x010000, CRC(cb2c995c) SHA1(2a618eb611637e048dc054de0d8f6466f5071617) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_20p_p63_ass.bin", 0x0000, 0x010000, CRC(5c97d505) SHA1(6ade77a6dcf1cc57afe879502534f855f6bd4cc8) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_std_ac_var_200pnd_ass.bin", 0x0000, 0x010000, CRC(23d239fa) SHA1(44dae2cd2be573df71b60ba3918cc2d728cde4b4) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc5p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_dat_ac_var_200pnd_ass.bin", 0x0000, 0x010000, CRC(a914cb23) SHA1(cd3332506229184cf0c3db37c43d2fa4cd2e54d9) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END

ROM_START( sc2copdc6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubcopsnrobbersdeluxe.bin", 0x0000, 0x010000, CRC(055e0f2c) SHA1(8aa7386031fd381deb7d79ce3217bab0d01671f0) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "club-deluxe-cops-and-robbers_mtx_250pnd-25p.bin", 0x0000, 0x010000, CRC(e1e4c10d) SHA1(5c508fe8ed96191eb1fa7156a09441f2f840544f) )

	sc2_copdc_sound
ROM_END


ROM_START( sc2dels )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_std_ac_10pnd-20p-25p_a.bin", 0x0000, 0x010000, CRC(b1e8d4ef) SHA1(189184aa6f9ff2204e35d0f7ae40493bcb0751bd) )
	sc2_dels_sound
ROM_END

ROM_START( sc2delsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_dat_ac_10pnd-20p-25p_a.bin", 0x0000, 0x010000, CRC(c81f200f) SHA1(8a9ee842e17a63276a0850adc52159dc46a239c0) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_std_wi_ac_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(dd44aecb) SHA1(1e8ced54323580f43facf683c1f489f1ea281e16) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_dat_wi_ac_10pnd-20p_a.bin", 0x0000, 0x010000, CRC(fdb33c9b) SHA1(2506fe8e7e1e49f90652309996813ac5967442a0) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_std_ac_8pnd-20p_a.bin", 0x0000, 0x010000, CRC(9194fb69) SHA1(30d2c5a8a16c96c081f442a66172f8b9fb1d602d) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_dat_ac_8pnd-20p_a.bin", 0x0000, 0x010000, CRC(92c0e403) SHA1(5410365137ab8debb10358f24cdd0b0b74755677) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_std_ac_8pnd_a.bin", 0x0000, 0x010000, CRC(58f87c90) SHA1(a6dcdf1edc7620226d89c907a5910c4a4b2d4190) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_dat_ac_8pnd_a.bin", 0x0000, 0x010000, CRC(23eca216) SHA1(f427d92929e51d6f0148d212e13067ddc15e2307) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(f4a5803d) SHA1(c9b6f71847a4dd87ea34b51935618df5a735150d) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_dat_ms_20p_a.bin", 0x0000, 0x010000, CRC(57ade491) SHA1(3aed99d92c391f99fa8ff7d61370d59245156121) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "del's-millions_std_ss_20p_a.bin", 0x0000, 0x010000, CRC(755b8546) SHA1(67d2bb5556c03acf71e0b50c8cf54ac92acbce69) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "delm20p", 0x0000, 0x010000, CRC(9d8acc21) SHA1(04d9cb4d01ddfb4e33774b313446dcd763f869fa) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dem20arc", 0x0000, 0x010000, CRC(9ae6291d) SHA1(966416d234e2ec708984595dedbfbe554ff1c867) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dels10", 0x0000, 0x010000, CRC(8bf1b9f5) SHA1(eb9c36579d56f83d72952fab9911a991aeec0579) )
	sc2_dels_sound
ROM_END

ROM_START( sc2dels9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95751541.p1", 0x0000, 0x010000, CRC(495b7cec) SHA1(779a80371580b9154f0915e7c438dbf965dd1a02) )
	sc2_dels_sound
ROM_END

ROM_START( sc2delsd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "delsdlx6", 0x0000, 0x010000, CRC(64acb285) SHA1(7a011b915809712fd69902258f1e6c9b42f163eb) )
	sc2_dels_sound
ROM_END

// sets below are mazooma
//Protocol status is guessed from part number for now until we're certain everything works with Mazooma games.
ROM_START( sc2delsm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400005", 0x0000, 0x010000, CRC(bd9153cf) SHA1(695a897077b2136ba4d0699cad616df5ceadf824) )
	sc2_dels_sound_alt
ROM_END

ROM_START( sc2delsmp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400006", 0x0000, 0x010000, CRC(2dc3355c) SHA1(6db6ddc93e05516b75d0dd27d5ab190d183a2bd1) )
	sc2_dels_sound_alt
ROM_END

ROM_START( sc2delsm1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400007", 0x0000, 0x010000, CRC(f29b0110) SHA1(b2a56e68a2bb4f4cc5b0f32933bf9e9acb0582d2) )
	sc2_dels_sound_alt
ROM_END

ROM_START( sc2delsm1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400008", 0x0000, 0x010000, CRC(38a0159b) SHA1(2f25ae4d858f68750a627d298556a7ce461480e5) )
	sc2_dels_sound_alt
ROM_END

ROM_START( sc2delsm2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98401005", 0x0000, 0x010000, CRC(d91beaa2) SHA1(b018d335e8551efe4cc09381324d7ae3d77b2907) )
	sc2_dels_sound_alt
ROM_END

ROM_START( sc2delsm2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98401006", 0x0000, 0x010000, CRC(262d57f9) SHA1(157bfa2d9de8da9f7791295b1e476bf2329f55cd) )
	sc2_dels_sound_alt
ROM_END

ROM_START( sc2delsm3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98401007", 0x0000, 0x010000, CRC(013c5e7c) SHA1(f3e960b44faecc7d19c6e058b62a30e45c3cfeae) )
	sc2_dels_sound_alt
ROM_END

ROM_START( sc2delsm3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98401008", 0x0000, 0x010000, CRC(665b3af4) SHA1(a7d51976caa8c373ac772e1315a33f0f042974a6) )
	sc2_dels_sound_alt
ROM_END


ROM_START( sc2wembl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_std_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(7b8e7a47) SHA1(3026850a18ef9cb44584550e28f62165bfa690e9) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wemblp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_dat_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(6ab89e2f) SHA1(6b2faa587153f453e9fdf043c6ca5a90d8c6b66d) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_std_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(065f2f8b) SHA1(81471db8de879b7d5b8741beefa5214f2c48ef84) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_dat_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(45c3df4c) SHA1(48ef0e46a94a815e1e429f402cc8fd13bde4d738) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(ae2330f0) SHA1(d309284f0f0333f6e065f30d7ac9416b2fc4ee1f) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(bf15d498) SHA1(f94d21d1202107db7955829340ada445d59f74ff) )
	sc2_wembl_sound
ROM_END


ROM_START( sc2wembl4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_dat_ss_10p_ass.bin", 0x0000, 0x010000, CRC(630b5306) SHA1(aa23645cc7f1c86e88a62420a837ab64c5090d09) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl5a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_std_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(17cd6162) SHA1(80129b26db4617281bb6e5aa1f573cf222660303) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl5ap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_dat_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(55b1764a) SHA1(1b1e5b89eda0d07662af003d1259e0da725abbc9) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl6ap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_dat_ar_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(550f82ec) SHA1(80b1d0839f600b01f2a60de0e191add0faaad089) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl7a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rtw816rm", 0x0000, 0x010000, CRC(337264ae) SHA1(5e3e67bd20416331df6e35c6a384d5b88b70aa17) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl7ap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "road-to-wembley_dat_ac_8pnd_16rm_ass.bin", 0x0000, 0x010000, CRC(512fafcb) SHA1(fe90c7fc58bd3dc0bc84e060c6b7a37dd855733b) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750499.p1", 0x0000, 0x010000, CRC(a2b11ca6) SHA1(cc1931504f8da98119f771499db616898d92e0d9) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750500.p1", 0x0000, 0x010000, CRC(bfe45926) SHA1(6a2814735e0894bb5152cba8f90d98cfa98c250b) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wembl10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95750501.p1", 0x0000, 0x010000, CRC(cab3da07) SHA1(8ef7ed8427cbb213f218328666da3ebd92aca5a5) )
	sc2_wembl_sound
ROM_END

ROM_START( sc2wemblm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400014", 0x0000, 0x010000, CRC(e4f3e02d) SHA1(ce2b961e6142ecfb1532daaa53746d785e2342eb) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "wembley_sound.bin", 0x0000, 0x080000, CRC(5ce2fc50) SHA1(26533428582058f0cd618e3657f967bc64e551fc) )
ROM_END

//There are two matrix ROMs here, presumably for different payouts or stakes, I've made my best guess as to which matches which but bear in mind
//the other one exists if there are any issues when these games start running


ROM_START( sc2prem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "premier-club-manager_std_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(404716ed) SHA1(57916fb70621c96eccb0e5bbee821ca2133aaa5f) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000571.p1", 0x0000, 0x010000, CRC(4b4bdb8b) SHA1(de9b52da600629e680fd96f0d82a9f76fbc84bdf) )//premier-club-manager_mtx_250pnd-25p_ass.bin
	sc2_prem_sound
ROM_END

ROM_START( sc2prem1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "premier-club-manager_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(68e5474e) SHA1(927d41f73e287c71546823ffe829f1e046f3cca6) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000570.p1", 0x0000, 0x010000, CRC(7ac2a278) SHA1(f95a7451d1514be19d747707a32bf7280dcfb8b6) )//premier-club-manager_mtx_ass.bin
	sc2_prem_sound
ROM_END

ROM_START( sc2prem1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "premier-club-manager_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(d1880c7a) SHA1(d1f7891fc8d4570e02c0bfc23e1ed0b159e280c1) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000570.p1", 0x0000, 0x010000, CRC(7ac2a278) SHA1(f95a7451d1514be19d747707a32bf7280dcfb8b6) )//premier-club-manager_mtx_ass.bin
	sc2_prem_sound
ROM_END

ROM_START( sc2prem2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "premclub.bin", 0x0000, 0x010000, CRC(5231ab3e) SHA1(a9e16a5bbeaa0612212d3ef0e78fbc7628cfc0fa) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD( "95000570.p1", 0x0000, 0x010000, CRC(7ac2a278) SHA1(f95a7451d1514be19d747707a32bf7280dcfb8b6) )//premier-club-manager_mtx_ass.bin
	sc2_prem_sound
ROM_END


ROM_START( sc2downt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_ar_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(bffe2f17) SHA1(c9daeec2b715d318649c8883b4437fdd997d0dc8) )
	sc2_downt_sound
ROM_END


ROM_START( sc2downtp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(3390da28) SHA1(80abda7a0d6913b701fb030b525db794d130df5b) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_ar_20p_ass.bin", 0x0000, 0x010000, CRC(a162c04a) SHA1(516f754b2e9cc33d43bac37f1f0697c1a886027e) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_ar_20p_ass.bin", 0x0000, 0x010000, CRC(a84c92c7) SHA1(99519d3e6166ab80236f1c16be82f7b2648f0aff) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(ef4c489f) SHA1(3b4e0c811edcb4f1f9c133ce92b7d965e167e51c) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_ar_20p_15rm_ass.bin", 0x0000, 0x010000, CRC(39fc9af0) SHA1(3b3a2a2ada79fa822332c066d50d81e64860292b) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_ac_10pnd-20p_15rm_ass.bin", 0x0000, 0x010000, CRC(7ef9d60d) SHA1(54000f31eac051efd2fd3fe485076f845ef3da30) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_ac_10pnd-20p_15rm_ass.bin", 0x0000, 0x010000, CRC(b082210f) SHA1(cd8d18fc2dcaf6fc02bc05d4c9e4a76f2199ad8d) )
	sc2_downt_sound
ROM_END


ROM_START( sc2downt4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_wi_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(bb448916) SHA1(ed62858cb78c9f08a55679cfdb19a3fa951d1aed) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_wi_ac_10pnd_15rm_ass.bin", 0x0000, 0x010000, CRC(29a1a709) SHA1(6b2de1e7902ba5b678aebf04b0f8c3bceed8f637) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtown8c.bin", 0x0000, 0x010000, CRC(6b93171c) SHA1(90e01e827b473bb6ffb567a350d9d8de9119cf8d) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtwn20v", 0x0000, 0x010000, CRC(5e6f05e4) SHA1(78ba0636aca6d6f5d8aee0f27c337975c5680e98) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtwnac", 0x0000, 0x010000, CRC(f553e337) SHA1(1881912807e4d245b8f2455ca8ca6d0c158ac5a8) )
	sc2_downt_sound
ROM_END


//All these ROMs are near identical to their similarly named counterparts, but are designed to handle the characteristics of a different motor
//(Starpoint 16RM vs Starpoint 15RM)

ROM_START( sc2downt3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_ac_10pnd-20p_16rm_ass.bin", 0x0000, 0x010000, CRC(932e49d9) SHA1(05ae4751f55eefe9884444745bcf3f2ecb69e332) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt3ap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_ac_10pnd-20p_16rm_ass.bin", 0x0000, 0x010000, CRC(d6d95ff4) SHA1(55d2b97a0609e305d28c92f439eb3b834d29aff5) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_wi_ac_10pnd_16rm_ass.bin", 0x0000, 0x010000, CRC(96ddfacd) SHA1(9085cdafc7b3ddf5ef77251a9ff4d4b4beff4ff1) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt4ap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_wi_ac_10pnd_16rm_ass.bin", 0x0000, 0x010000, CRC(6f6f8c71) SHA1(5fba18cc092a04b3b737bb17a03d5e37a33da985) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt8a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_std_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(593f59a5) SHA1(578173ec26980072a00bb46370c2c1113916c279) )
	sc2_downt_sound
ROM_END

ROM_START( sc2downt8ap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "down-town_dat_ss_20p_16rm_ass.bin", 0x0000, 0x010000, CRC(39a1cd5d) SHA1(bce1e1bfe4e9e3bc62bdf8a57b0b2db2b3accd4f) )
	sc2_downt_sound
ROM_END

ROM_START( sc2goldr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gr1_3.bin", 0x0000, 0x010000, CRC(caed7c10) SHA1(3ea4b786d7574a3274131554885a372283eb1cf4) )
	sc2_goldr_sound
ROM_END

ROM_START( sc2goldrp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gr1_3d.bin", 0x0000, 0x010000, CRC(e5ad5d10) SHA1(8a2bf68b923848421b90af8a1c42f5cef1a02121) )
	sc2_goldr_sound
ROM_END

ROM_START( sc2goldr1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gold_reserve_game", 0x0000, 0x010000, CRC(581726a3) SHA1(7e122a9d48f49648feeeb3fe430013402a5dc8d7) )
	sc2_goldr_sound
ROM_END

ROM_START( sc2hifly )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hf4_1.bin", 0x0000, 0x010000, CRC(ee58ed3b) SHA1(4372ca48854b5a4b2c9ac24b17afce899a88da15) )
	sc2_hifly_sound
ROM_END

ROM_START( sc2hifly2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hf3_1.bin", 0x0000, 0x010000, CRC(0ec80578) SHA1(8bbe5aaefe7c5ab77e27daad3fe43d7bbe600a54) )
	sc2_hifly_sound
ROM_END

ROM_START( sc2hifly3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hf2_1.bin", 0x0000, 0x010000, CRC(6c1350eb) SHA1(062e4533c28c8129aae787805bdf99a2837f93f5) )
	sc2_hifly_sound
ROM_END

ROM_START( sc2hifly4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hiflyergame.bin", 0x0000, 0x010000, CRC(b3627b55) SHA1(105ff7da69eb2ca722ee251a4a6af49c46ab1bc8) )
	sc2_hifly_sound
ROM_END

ROM_START( sc2inst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_std_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(81a235e9) SHA1(3ed26da7511b2b2324d74f8395215157c41850ce) )
	sc2_inst_sound
ROM_END

ROM_START( sc2instp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(01034a5b) SHA1(c4f7b05d5c15c309d0c13f4bef72429e54e4fd5e) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_std_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(641928a0) SHA1(8d68af148838987a4ebfd7927b8eda5cfa4bbb53) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_dat_wi_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(6d289dfa) SHA1(a1245373ad5a99e2794751dd8e4d3ea28dcb0a53) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_std_var_to_htpa_ass.bin", 0x0000, 0x010000, CRC(1566696f) SHA1(c8cda3f1d15bcb8ba67fab8cb4b972c02106eceb) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_dat_var_to_htpa_ass.bin", 0x0000, 0x010000, CRC(0dcd87a1) SHA1(4d53a346665bf22e467cc0e0859ee44c177b7661) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_std_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(f21c8970) SHA1(67ecb5202cc4a8f2568df6c0a4ed36f4c85b8bb4) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_dat_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(5ded0d95) SHA1(6f1f57e6883f4b0421ca4d49f7593a937918f9e4) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_std_var_ass.bin", 0x0000, 0x010000, CRC(ca8ab34a) SHA1(ecf5ccf0f95a8d149326d24ac468660dde073a16) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instant-jackpot_dat_var_ass.bin", 0x0000, 0x010000, CRC(26f50252) SHA1(587ca9490e04247c8b93c4c931caadf0b5aea4b3) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instantjackpotgame21.bin", 0x0000, 0x010000, CRC(478a4ee9) SHA1(bb33c63d3db961dc14a02f9ab69908757b8ccd87) )
	sc2_inst_sound
ROM_END

ROM_START( sc2inst6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "instantjackpotgame.bin", 0x0000, 0x010000, CRC(183d53bf) SHA1(4ceca64324a95580270b66d60e678996c79db965) )
	sc2_inst_sound
ROM_END

ROM_START( sc2mam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_std_ac_10pnd-20p-25p_ass.bin", 0x0000, 0x010000, CRC(33fce86f) SHA1(1fa06c834397f97e3723091eb331adab91e3d720) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mamp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_dat_ac_10pnd-25p-20p_ass.bin", 0x0000, 0x010000, CRC(b721a965) SHA1(23c8f3e98b7a2d7aa11593bff2caea26c893a98a) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_std_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(d2dceb05) SHA1(f4dd4f0ce3aa97caba0356a19fe78e3c3455af54) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_dat_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(a63f1ae3) SHA1(37920ade2a162f6663a8384ff3cf55e1de71d3d6) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_std_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(9150bd44) SHA1(0ef8884337c188c696a15cf2bc5a821bdc64d8ae) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_dat_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(722420ea) SHA1(7c3a8a7218770645f5644a68c65b8e2104857367) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_std_ar_var_ass.bin", 0x0000, 0x010000, CRC(06759280) SHA1(168743d4d116850c3c23db3cd0149c7f5f8b4da3) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(f9307781) SHA1(56bef9b7d4db0d4569a855dba49d931125f038a4) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_std_ss_var_ass.bin", 0x0000, 0x010000, CRC(4de6346a) SHA1(ae30a5adfad59dd282ca3c2e16e18cbd17d956e9) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "make-a-million_dat_ss_var_ass.bin", 0x0000, 0x010000, CRC(be526b6b) SHA1(e3e6eb91480015edc3ef46158a277c90d1bf5662) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mam3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mam8arc.bin", 0x0000, 0x010000, CRC(91ee99ca) SHA1(8e7e26e0ab518e55784b91b5d8c9780eb1f72525) )
	sc2_mam_sound
ROM_END

ROM_START( sc2mamcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-make-a-million_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(f7b67981) SHA1(ccddb63cd24969fb74a3e4c51c8ab7453b3e99a1) )
	sc2_mamcl_sound
ROM_END

ROM_START( sc2mamclp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-make-a-million_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(4a6a6e05) SHA1(684bb86de514e66409cc04255d4212569ad5f2e6) )
	sc2_mamcl_sound
ROM_END

ROM_START( sc2mamcl1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-make-a-million_std_ac_var_p65_ass.bin", 0x0000, 0x010000, CRC(296b5724) SHA1(437d789313960db9e4da147353da81d3e162e563) )
	sc2_mamcl_sound
ROM_END

ROM_START( sc2mamcl1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-make-a-million_dat_ac_var_p65_ass.bin", 0x0000, 0x010000, CRC(ee687364) SHA1(a414c71659a81fc464bc167c05e9426a37d33f82) )
	sc2_mamcl_sound
ROM_END

ROM_START( sc2mamcl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-make-a-million_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(c0685075) SHA1(4906d1e81e7d9b43e6c147ebc72081634dd7cd45) )
	sc2_mamcl_sound
ROM_END

ROM_START( sc2mamcl2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-make-a-million_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(484ea479) SHA1(c1542dcd664508e4ebea3b66b9961680b7f4d711) )
	sc2_mamcl_sound
ROM_END

ROM_START( sc2mamcl3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mmilclub", 0x0000, 0x010000, CRC(c3c6856a) SHA1(6163bfcf4271bef2517bdf16b526a882574c0bf1) )
	sc2_mamcl_sound
ROM_END

ROM_START( sc2scc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc4_4t.bin", 0x0000, 0x010000, CRC(99235ed7) SHA1(f2d851ce1abe6c1dc4ab1ce3aea067c6434ef6ee) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "scsnd", 0x0000, 0x040000, CRC(5f201e1a) SHA1(cc67bcd3a59681b7eb535c966a1e100a17ca1acc) )
ROM_END

ROM_START( sc2showt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_std_ac_8-10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(44176459) SHA1(e7321fb659be162507f095e3b586706837892c2d) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showtp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_dat_ac_8-10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(98111157) SHA1(ddc0e194d330348ce133467324155787f98bf8fd) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_std_wi_ac_8-10.pnd_ass.bin", 0x0000, 0x010000, CRC(d4867696) SHA1(7d8d9eed052ab6a84c52136bb604b91987f6120e) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_dat_wi_ac_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(9233e7e2) SHA1(fafc9fe99fb3b04e494302e2e1c566e611c1cd54) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_std_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(c6760a9b) SHA1(bf85edd0a0d10da04b1a3608fa2f2f3c5d4ed7ec) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_dat_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(99bddd9c) SHA1(256b11ffc0415c21ad20d7192cf5bb67dca38a54) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_std_ar_var_ass.bin", 0x0000, 0x010000, CRC(b2a8470c) SHA1(43eecd76e6a028595ee91a7be92490bda9d8eef0) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_dat_ar_var_ass.bin", 0x0000, 0x010000, CRC(f70d696e) SHA1(5ddaa1323586dd7de87ee18f666c632a149b8c6c) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_std_ss_var_ass.bin", 0x0000, 0x010000, CRC(a42d951d) SHA1(e6c0491e69195043f0f228b80ded6c84116b8ddc) )
	sc2_showt_sound
ROM_END

ROM_START( sc2showt4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "showtime-spectacular_dat_ss_var_ass.bin", 0x0000, 0x010000, CRC(180984c3) SHA1(789cf4e7d99ad25d21ea02ec4de39f30fb6e7474) )
	sc2_showt_sound
ROM_END

ROM_START( sc2sstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_std_ac_tri3_ass.bin", 0x0000, 0x010000, CRC(7a6c9f8d) SHA1(2a721823a95b2c324dd8500b32a04e8492e49f67) )
	sc2_sstar_sound
ROM_END

ROM_START( sc2sstarp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_dat_ac_tri3_ass.bin", 0x0000, 0x010000, CRC(caeaf463) SHA1(c07569da462de24f477a974f7d18368ea7b6b461) )
	sc2_sstar_sound
ROM_END

ROM_START( sc2sstar1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_std_ac_var_4-8-10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(1e294299) SHA1(c961be1289bc77e34535d913ff19c75b1edeaba7) )
	sc2_sstar_sound
ROM_END

ROM_START( sc2sstar1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_dat_ac_var_4-8-10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(f65ed8c9) SHA1(c0322c63d02d11425518fdacb98d30e7e49e498b) )
	sc2_sstar_sound
ROM_END

ROM_START( sc2sstar2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_std_ac_8pnd-20p_tri2_ass.bin", 0x0000, 0x010000, CRC(441b76ff) SHA1(46b1ac77798cee4dfdd703af768c83b5c246f135) )
	sc2_sstar_sound
ROM_END

ROM_START( sc2sstar2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_dat_ac_8pnd-20p_tri2_ass.bin", 0x0000, 0x010000, CRC(c1134d01) SHA1(d36ea1c58261353c86da562825ccadcdc2ddb9e8) )
	sc2_sstar_sound
ROM_END

ROM_START( sc2sstar3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_std_wi_ac_10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(adca7b5a) SHA1(4c889a0cda94c2698a4102a53d04594f7f931ee5) )
	sc2_sstar_sound
ROM_END

ROM_START( sc2sstar3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superstar_dat_wi_ac_10pnd_tri2_rot_ass.bin", 0x0000, 0x010000, CRC(71ef63d6) SHA1(c0b1cbca8c801002a3eb7fd11474107c6bc6a1d1) )
	sc2_sstar_sound
ROM_END



ROM_START( sc2pe1g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pe95750415proma.bin", 0x0000, 0x010000, CRC(e518f28e) SHA1(0f693814409b9aa69d736dc97f26d2a79afd06c5) ) // not scrambled?

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "pe95004031sound.bin", 0x0000, 0x080000, CRC(82f895b0) SHA1(888e172b24cb95c2723d9aa5cf1153a3af2ff2c7) )

	ROM_REGION( 0x80000, "other", ROMREGION_ERASE00 )
	ROM_LOAD( "pal.bin", 0x0000, 0x000010, CRC(d33fb7d2) SHA1(6de1a205808bccb9bc86f630c0eda261041a3b00) )
ROM_END





ROM_START( sc2wwcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-wild-west_std_ac_var_250pnd_ass.bin", 0x0000, 0x010000, CRC(a4c33524) SHA1(34d46b912488f630ddec301bde5ee1d87661b2a4) )
	sc2_wwcl_sound
ROM_END

ROM_START( sc2wwclp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-wild-west_dat_ac_var_250pnd_ass.bin", 0x0000, 0x010000, CRC(deca21f2) SHA1(a79ef84271742f98e4557cba7b6b976f4d5b220f) )
	sc2_wwcl_sound
ROM_END

ROM_START( sc2wwcl1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-wild-west_std_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(74b2592a) SHA1(f83a1fb5db69403a6b2922d2e3654fb753e0079c) )
	sc2_wwcl_sound
ROM_END

ROM_START( sc2wwcl1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-wild-west_dat_ac_var_ffp_ass.bin", 0x0000, 0x010000, CRC(2361e6c7) SHA1(5277d8d784a358441b86f4b9e3999511c74b7b09) )
	sc2_wwcl_sound
ROM_END


ROM_START( sc2dick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spot3-1n.p1", 0x0000, 0x010000, CRC(794cec5b) SHA1(91ba4fcc459194fcf89f27e9c687cbdb8a10bb78) )
	sc2_dick_sound
ROM_END

ROM_START( sc2dickp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spot3-1p.p1", 0x0000, 0x010000, CRC(fa027939) SHA1(7fc6d26d179d976add3ca18c5df71dd9df7af1f2) )
	sc2_dick_sound
ROM_END

ROM_START( sc2dick1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spotteddick2_2.bin", 0x0000, 0x010000, CRC(497ef3b2) SHA1(f5021e35397081c62e817b86ff9e8a49d78748a5) )
	sc2_dick_sound
ROM_END

ROM_START( sc2dick2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp151-4n.p1", 0x0000, 0x010000, CRC(ee18a5a1) SHA1(17c2984fb305a571df83c663c9e42164f2322938) )
	sc2_dick_sound
ROM_END

ROM_START( sc2dick2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp151-4p.p1", 0x0000, 0x010000, CRC(94d96a28) SHA1(307e1cb5fe3c6050eb039dcd97e6ac88494707b3) )
	sc2_dick_sound
ROM_END

ROM_START( sc2dick2e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spotteddickeuro.bin", 0x0000, 0x010000, CRC(c3b68821) SHA1(d86e098c3f0aec4f8068942934134e394075473d) )
	sc2_dick_sound
ROM_END

ROM_START( sc2dick2eu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "global-spotted-dick_euro.bin", 0x0000, 0x010000, CRC(695a3ec4) SHA1(f9f2f47f74479ef444997e2deef1c5f4677368ca) ) // this one isn't scrambled
	sc2_dick_sound
ROM_END

ROM_START( sc2pick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pick2-3n.p1", 0x0000, 0x010000, CRC(b89c1dde) SHA1(8e1ece392dbb8e88daece79c5bea832149d8f442) )
	sc2_pick_sound
ROM_END

ROM_START( sc2pickp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pick2-3p.p1", 0x0000, 0x010000, CRC(53ced0cb) SHA1(113a5e9414a3fcf0dacb6024748681f2b8e8bb55) )
	sc2_pick_sound
ROM_END

ROM_START( sc2pickc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dpic1-9n.p1", 0x0000, 0x010000, CRC(89b24a0b) SHA1(f56a79258497bc787b50d37ddf75b5d4920848e8) )
	sc2_pick_sound
ROM_END

ROM_START( sc2pickcp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dpic1-9p.p1", 0x0000, 0x010000, CRC(1c0adb51) SHA1(aeca44490c8b0517eddd69fcdc36cf2cafb4d844) )
	sc2_pick_sound
ROM_END



ROM_START( sc2rock )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hb151-6n.p1", 0x0000, 0x010000, CRC(982de54a) SHA1(20e65e163f0455d683eb47ac37bc1e3355548c9a) )
	sc2_rock_sound
ROM_END

ROM_START( sc2rockp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hb151-6p.p1", 0x0000, 0x010000, CRC(c9063e3c) SHA1(e47765ff56abb8d25c559cc5ebbe679ca40c498b) )
	sc2_rock_sound
ROM_END

ROM_START( sc2rock1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock1-4n.p1", 0x0000, 0x010000, CRC(e3888e8b) SHA1(7e394cbc219259a5eed9ccb283fff5f4b257e87f) )
	sc2_rock_sound
ROM_END

ROM_START( sc2rock1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rock1-4p.p1", 0x0000, 0x010000, CRC(a4b61df4) SHA1(ffbfab5fc976edc68bb599625387295df793f449) )
	sc2_rock_sound
ROM_END

ROM_START( sc2rocke )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hbiyr_euro.bin", 0x0000, 0x010000, CRC(bc4f8ffe) SHA1(de51fda4fe1c57945133a25c2ad8fba48064a23c) )
	sc2_rock_sound
ROM_END

ROM_START( sc2call )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "your2-7n.p1", 0x0000, 0x010000, CRC(9d3b4987) SHA1(131808aa90627b0aa830c6b49b12e15af96665a5) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END

ROM_START( sc2callp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "your2-7p.p1", 0x0000, 0x010000, CRC(03af9c27) SHA1(03dcdb3d20903a116d85e4e0cfafc5495f0e9d60) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END

ROM_START( sc2callc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dyor1-6n.p1", 0x0000, 0x010000, CRC(5e516bd1) SHA1(52a108e3d7aa9fdffb25e09922fa84c0155f18f5) )


	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END

ROM_START( sc2callcp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dyor1-6p.p1", 0x0000, 0x010000, CRC(843edbd2) SHA1(67496753f3687800413418d65dcfc764695b4997) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) // not upd?
	/* missing? */
ROM_END

ROM_START( sc2prom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "alongtheprom.bin", 0x0000, 0x010000, CRC(0f212ba9) SHA1(34dfe67f8cbdf1cba806dcc7a3e872a8b59747d3) )

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("alongthepromdot.bin",  0x00000, 0x10000, CRC(b5a96f4d) SHA1(716dda738e8437b13cb72a6b071e0898abceb647))

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "alongthepromsnd.bin", 0x0000, 0x040000, CRC(380f56af) SHA1(9125c09e6585e6f4a2de9ea8715371662245aa9a) )
ROM_END

ROM_START( sc2payr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "98400023", 0x0000, 0x010000, CRC(9478e97a) SHA1(c269f2a8e7eb6d76bf51563c6588d21bd71c1acf) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	/* missing */
ROM_END

/* this might be for the concept title ec_bar7 */
#define sc2_bar7_sound \
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) \
	ROM_LOAD( "b7snd1.bin", 0x0000, 0x00ff28, CRC(27efbf06) SHA1(735ffb552aacebe46405828b87de947b99edc4ea) )


ROM_START( sc2bar7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b7code.bin", 0x0000, 0x010000, CRC(bf8dbb1f) SHA1(fb07fbd1cc48bd0a6712ac9b71dcb8202720f86b) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bar73v1", 0x0000, 0x010000, CRC(2cc8dad8) SHA1(c3cce5e9ae032a6797828c8d42948ad749a03777) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bar72v5", 0x0000, 0x010000, CRC(6a9ce006) SHA1(c2b1efcdc576ea49243852f8a65c89fbca0ba7a4) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1v9.bin", 0x0000, 0x010000, CRC(f2aacd4d) SHA1(c6386de65cacbfb877ead00ee48d7cf9d43e61b0) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1v9p.bin", 0x0000, 0x010000, CRC(d7d38831) SHA1(5053b7e586b95d5e0853a506bb3df9203672469c) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2v0.bin", 0x0000, 0x010000, CRC(c5226f5d) SHA1(157b744b56a04f507798e857001b2e8255f2a3d9) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7f )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2v2.bin", 0x0000, 0x010000, CRC(0e7c9399) SHA1(9892313a4c8c7e8cca0f580ac6a2ad62fdf1ad1b) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2v3.bin", 0x0000, 0x010000, CRC(11fed7c4) SHA1(9164a81933fae960ba06d2b5aa5c47125db80fb7) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2v4.bin", 0x0000, 0x010000, CRC(26ddef97) SHA1(ab42a3b328c78257e4a207be0ab4e643c5c07b23) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7i )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2v6.bin", 0x0000, 0x010000, CRC(5842d19c) SHA1(a764a899745cf5a81f7c62ff8339c0847a7f8d50) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bar7", 0x008000, 0x008000, CRC(ce0429bc) SHA1(d9cda09589a6e7c72c4d777de2964abe6b4e18c3) )
	sc2_bar7_sound
ROM_END

ROM_START( sc2bar7k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bar71v7", 0x0000, 0x010000, CRC(c3e01545) SHA1(4a4c06226587acb0875e6d19985916469b2eaa23) )
	sc2_bar7_sound
ROM_END


#define sc2_bbar7_sound \
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 ) \
	ROM_LOAD( "bb7snd", 0x0000, 0x080000, CRC(044c4ad5) SHA1(3d5e2e268bc2a4bac8df60e7d29b883f3d2fe61d) ) //Seems bad (loads of 00)


ROM_START( sc2bbar7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bb71v11.lit", 0x0000, 0x010000, CRC(4ba2cbbc) SHA1(6767d5935e12586a6bbd213e999940e3990af007) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bb7cz10", 0x0000, 0x010000, CRC(672f262f) SHA1(8d4ebf6df585ec34a6142175ab114367029b2cd8) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7b ) // might be an electrocoin hw set
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "bb7v1", 0x0000, 0x020000, CRC(dee2e740) SHA1(e5bd24cb0722d2aec3ac2799f66cf5c8dd7ddd74) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big16a.bin", 0x0000, 0x010000, CRC(4fd95f69) SHA1(424c074efaccb2ad2bf4c97fdd37d9fa01c0a411) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big16b.bin", 0x0000, 0x010000, CRC(4604a0ff) SHA1(55d95ce2be1ef01fdeae7d727682989744da863d) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big16c.bin", 0x0000, 0x010000, CRC(f93eab19) SHA1(488c722d55c354923dc302558f28b58b1e71a64e) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7f )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigb713a.bin", 0x0000, 0x010000, CRC(39792e6c) SHA1(5288cdd5d03314b07fa02d1c14c2d37068ba947e) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigb713b.bin", 0x0000, 0x010000, CRC(7471adcd) SHA1(99369d9063c1bbe10ca7994b7d7936bbefc3c9ee) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigb713c.bin", 0x0000, 0x010000, CRC(a4185331) SHA1(b501e7046ac4a7ea91b7e3b1ee56e57a3321d988) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7i )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigb714a.bin", 0x0000, 0x010000, CRC(dbe28212) SHA1(eae79d4b671c5e9ac02ff71acdc45159a3ddc6a2) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigb714b.bin", 0x0000, 0x010000, CRC(f59500b2) SHA1(90eb80249d1c1798922c0e39053b6839027cd20d) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigb714c.bin", 0x0000, 0x010000, CRC(7d0fe1ab) SHA1(bcbdef94dc984560cede1249cc21803141539717) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7l )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigb71v11.bin", 0x0000, 0x010000, CRC(7151e450) SHA1(4348c2cc3de96e28326325b4ae81b9cd20cda2cb) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7m )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigx.bin", 0x0000, 0x008000, CRC(d6b6996b) SHA1(5226fc89e892ce0b3884bea0d220e3835dbb6c17) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7n )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigx003.bin", 0x0000, 0x008000, CRC(638391f3) SHA1(8b34282c1d96d929f6e193486ddb6f348330d08c) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7o )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigx007.bin", 0x0000, 0x008000, CRC(ac618c9d) SHA1(27813c09493f3a8d8fbf4a976ce1f5573c65a24d) )
	sc2_bbar7_sound
ROM_END

ROM_START( sc2bbar7p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "podbig7.bin", 0x0000, 0x010000, CRC(44c76818) SHA1(eb467c8bb1a9347c7537ef0c6b664620e0d5f015) )
	sc2_bbar7_sound
ROM_END



ROM_START( sc2flutr )
	//This is weird, it looks like the sc2 board is some sort of master controller for linked machines (serial connection)?.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "code.bin", 0x0000, 0x010000, CRC(3e5d54d6) SHA1(a0ad4a4c723e0d03683c7f53fd0932b46f49cb41) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "102.bin", 0x0000, 0x010000, CRC(c65261f5) SHA1(605799fc75b11255e6f17e168ad6c717e0d8d911) )
	ROM_LOAD( "103.bin", 0x0000, 0x010000, CRC(1923fbb3) SHA1(783a707580771842511d0aefa24694f1762f296e) )
	ROM_LOAD( "105.bin", 0x0000, 0x010000, CRC(b7069d2e) SHA1(62c6accb383a85f395ba33d50290044fffeb5d1d) )
	ROM_LOAD( "106.bin", 0x0000, 0x010000, CRC(8ce41a8f) SHA1(5d1fbd0ec16f19a10645315fb3adbb117ed30a4d) )
	ROM_LOAD( "107.bin", 0x0000, 0x010000, CRC(07862655) SHA1(d3d7cd7b8ecb3d5b821bc813c414ed99daa72b5b) )
	ROM_LOAD( "108.bin", 0x0000, 0x010000, CRC(5082e079) SHA1(e78489cd9e8763426de16b49af298fc9b6aaf6cc) )
	ROM_LOAD( "flutter.bin", 0x0000, 0x018008, CRC(281a9c91) SHA1(9ada7698aaafc0c60985a028ed6aab680eb355fb) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "flutsnd.bin", 0x0000, 0x0105cb, CRC(947cddfa) SHA1(7ae5a3cae065e35519a13007767568471aacca1a) )
ROM_END

ROM_START( sc2smnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "smn1.2", 0x8000, 0x008000, CRC(e2d2fdd9) SHA1(0e2f44fa64dfa342752e53e9d514ca64e70b3046) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "super_multi-nudge_game_(27512)", 0x0000, 0x010000, CRC(6a0de579) SHA1(308fec509371b93cb6ab957c83f2e041db449dfe) ) // both halves identical, but doesn't work, start vector is 4000?
	ROM_LOAD( "chezb10.bin", 0x0000, 0x010000, CRC(f00b6b95) SHA1(e2c3c7127bc9f9c77bd5b1f36aef47ffa05143a9) )
	ROM_LOAD( "chezb10.s", 0x0000, 0x010000, CRC(78e526a0) SHA1(2e7c90efa5c8d04214b5065aba446f9782c8298c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "super_multi-nudge_sound_(4meg)", 0x0000, 0x080000, CRC(efd87dab) SHA1(8b4b5de351ce3b1cefa4d0dc01072a942db072dc) )
ROM_END

ROM_START( sc2sghst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ghostgrn.bin", 0x0000, 0x010000, CRC(56d0141d) SHA1(0dd1b71892d60361626e073da12ca8f2ec2e610b) )

	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "gstgrncx", 0x0000, 0x010000, CRC(b0d4ff82) SHA1(afbec53b4be1d39bdb2227b58bea77d024cf3e2a) )
	ROM_LOAD( "lg1v0.bin", 0x0000, 0x010000, CRC(20950ebb) SHA1(f301dc147f0997d781cfecca35ef97bcb4627106) )
	ROM_LOAD( "sg1v0.bin", 0x0000, 0x010000, CRC(79ed8e14) SHA1(29ec1556d98bc536936046d6ca0b572f8d7674a4) )
	ROM_LOAD( "sg1v1.bin", 0x0000, 0x010000, CRC(d836be41) SHA1(d294180fcd2110d5d350bc578c91b67745996d51) )
	ROM_LOAD( "sg1v2.bin", 0x0000, 0x010000, CRC(d2cc95a0) SHA1(02394e08d108c215e2bff8556a61002acdb8a453) )
	ROM_LOAD( "sg1v4.bin", 0x0000, 0x010000, CRC(033cd636) SHA1(8be8f828a9b966b00f395a9cf33ec5f6b469cddb) )
	ROM_LOAD( "sg2v0.bin", 0x0000, 0x010000, CRC(7247d3a1) SHA1(1c349ed86ea335d5db78045d770ba550f8c365d0) )
	ROM_LOAD( "sg2v1.bin", 0x0000, 0x010000, CRC(ebfd636b) SHA1(9c25f2a368556ceff218e006b0917850ff80a53d) )
	ROM_LOAD( "sg2v2.bin", 0x0000, 0x010000, CRC(894856bc) SHA1(f193f538d80a6b0c5eb2b21a67b7a96db7127c8f) )
	ROM_LOAD( "sg2v2sam.bin", 0x0000, 0x039ac3, CRC(394bcb10) SHA1(dc48e22ead641945373f27e480680db37979c64b) )
	ROM_LOAD( "sg2v5.bin", 0x0000, 0x010000, CRC(6f8954c8) SHA1(e720b2cb49068d3788a2aef90ed464090cb757e1) )
	ROM_LOAD( "sg2v6.bin", 0x0000, 0x010000, CRC(52f79b3a) SHA1(13cfcd60d853283ef6bb722bb08756da88c4bfe8) )
	ROM_LOAD( "sg2v7.bin", 0x0000, 0x010000, CRC(1774d598) SHA1(f80ea78c0337d396fd6b4807fb59e1a54e929ea6) )
	ROM_LOAD( "sg2v7b.bin", 0x0000, 0x010000, CRC(9c14b804) SHA1(c3831a96640be9ab89f8e05a36c1ac967d50bd69) )
	ROM_LOAD( "sg2v7c.bin", 0x0000, 0x010000, CRC(07fcf016) SHA1(6f12018336c71afb98206a6c2e9276d6a21272ec) )
	ROM_LOAD( "sg2v8.bin", 0x0000, 0x010000, CRC(6362c3b6) SHA1(cbb7c56f64fc960e05f06632608b4e55f9e6385d) )
	ROM_LOAD( "sg2v9.bin", 0x0000, 0x010000, CRC(829ff8dd) SHA1(32aa1577aa61b3d7fc79e8890906a90225490542) )
	ROM_LOAD( "sghost.bin", 0x0000, 0x010000, CRC(a48a0c03) SHA1(0c647efaf0b9917bd9a7e07e010d3157f160e040) )
	ROM_LOAD( "sghost_gamesman_oneonly.bin", 0x0000, 0x010000, CRC(8fee4957) SHA1(95256c5bd511ffa11df25d3791c0ad8eeef9d9b6) )
	ROM_LOAD( "superghost2v9.bin", 0x0000, 0x010000, CRC(4ff0c3c2) SHA1(e8cefbcec11dab118299e04ef757cf7c2c485927) )
	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "sgsnd2", 0x0000, 0x080000, CRC(8ecf978a) SHA1(dd7cd6beb43dab011d661d9c507b20e507ad289b) )
	ROM_REGION( 0x800000, "altupd", 0 )
	ROM_LOAD( "sgstsnd", 0x0000, 0x03af13, CRC(2c6b2237) SHA1(7da432ccea45ce30bba72a0b565d53b33257f877) )
	ROM_LOAD( "ghostsnd.dat", 0x0000, 0x0d9ce8, CRC(56f4377f) SHA1(ddf296d2d705def19870b24019ecfdb42bc45342) )
ROM_END

ROM_START( sc2scshx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scx0v1e.bin", 0x0000, 0x010000, CRC(f0e52b5e) SHA1(431a56bbcb2c519ba5af9f58a3588ce5c2deb1f8) )
	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "scx 1p bin.dat", 0x0000, 0x010000, CRC(4a4d70d2) SHA1(9f14f6ba0a3c6c5c1a78c68b0bba49fb948142e5) )
	ROM_LOAD( "scx1.1b", 0x0000, 0x010000, CRC(7856b2bb) SHA1(cfa89418113bbbf9400c45b06a6e86fd15b57d52) )
	ROM_LOAD( "scx1v0.bin", 0x0000, 0x010000, CRC(8ea1be86) SHA1(42bd63e94e3876f21643813de64f16e701c1429f) )
	ROM_LOAD( "scx1v1.bin", 0x0000, 0x010000, CRC(4cb99292) SHA1(956b951a51d1dfae361f9e554eb918730c8013fc) )
	ROM_LOAD( "scx1v1a.bin", 0x0000, 0x010000, CRC(f01c5926) SHA1(5f499306f60111a423a74cdb624da07550ce48f5) )
	ROM_LOAD( "scx1v1a~.bin", 0x0000, 0x010000, CRC(90ce3521) SHA1(8cb7dbc78ac02e6772aaa3341b904767dd1c1301) )
	ROM_LOAD( "scx1v2.bin", 0x0000, 0x010000, CRC(054603f1) SHA1(9fca7772812bdfed1d67d916da520cbfd2bf82a8) )
	ROM_LOAD( "scx1v3.bin", 0x0000, 0x010000, CRC(711a0f93) SHA1(5b3efda6a01663655ec614feab9e1d0c857e823e) )
	ROM_LOAD( "scx1v6hi.bin", 0x0000, 0x010000, CRC(cae3fd0b) SHA1(1fe2ab0037c5a0be58378e95f72dc2782325fb71) )
	ROM_LOAD( "scx1v6lo.bin", 0x0000, 0x010000, CRC(ca5fdbca) SHA1(60079aeb4904e42a4a45feb7f31cf6c71b611845) )
//  ROM_LOAD( "scx1v7hi.bin", 0x0000, 0x010000, CRC(b8ae7542) SHA1(22230e9a67c0f8408d6ba7adafd581cd3d62c5ad) ) // in sc2scshxcas
//  ROM_LOAD( "scx1v7lo.bin", 0x0000, 0x010000, CRC(1ed97ef6) SHA1(1aaf911369dc814ee2edf5d59baa2961bfc73168) ) // in sc2scshxcas
	ROM_LOAD( "scx1v8hi.bin", 0x0000, 0x010000, CRC(06e35b38) SHA1(0a48489aee24066526da2cf56775f805d9603995) )
	ROM_LOAD( "scx1v8lo.bin", 0x0000, 0x010000, CRC(82bc1820) SHA1(301775e0e32e44d5cbe43c0cb83d94cf2aab9a50) )
	ROM_LOAD( "scxsp10.bin", 0x0000, 0x010000, CRC(e006d449) SHA1(73acc9c729e73d3a262d1a21fe89e00047eabdb2) )
	ROM_LOAD( "scxv2hi.pg", 0x0000, 0x010000, CRC(ee5219bd) SHA1(d193289ab9d2348292f122a7dfd4121c37b1635a) )
	ROM_LOAD( "scxv2lo.pg", 0x0000, 0x010000, CRC(48aea8e3) SHA1(601c22fda44171e292a284c0e6cb202cb8a14e24) )
	ROM_LOAD( "supercashx1v8.bin", 0x0000, 0x010000, CRC(3123327f) SHA1(b2edc4cbbe2fb1c451dc22dd8a7cf40d7012a3f3) )
/*
    QF18144*
    QP44*
    QV0*
    F0*
    X0*
    J0 0*
    N DEVICE XC9536-15-PC44 */
	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "supercx.jed", 0x0000, 0x0008e0, CRC(d80bc698) SHA1(2cfda3f945250253097b8a87924f14946c294894) )
ROM_END

ROM_START( sc2scshxgman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scxgm10.bin", 0x0000, 0x010000, CRC(f8c5bac8) SHA1(7858b2c8442b80b69598244870620d45042b7abb) )
	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "scxgm10a.bin", 0x0000, 0x010000, CRC(deab7e4e) SHA1(472a55b0ba289b0f4e538bb4c8b826dede3a40bb) )
//scxgm10b.bin identical
	ROM_LOAD( "scxhiv1.gmn", 0x0000, 0x010000, CRC(c43c2f43) SHA1(8bd8b2a71f19d6fd1f96d6032d1b60bb75dcaeb8) )
	ROM_LOAD( "scxhiv2.gmn", 0x0000, 0x010000, CRC(83a1ecc9) SHA1(b0176b25c97739442f3743136833d0e5fe51c03f) )
	ROM_LOAD( "scxlov1.gm", 0x0000, 0x010000, CRC(e305ff5a) SHA1(0bbc1cfaf7c7aaf324c65fd22148437e2bd4ca1e) )
	ROM_LOAD( "scxlov2.gm", 0x0000, 0x010000, CRC(62d384bd) SHA1(b7a0821fb37e3fb290e620888411d81525ab1635) )
	ROM_LOAD( "scxcoastv2.bin", 0x0000, 0x010000, CRC(53a0708a) SHA1(01fc2f5cd7f126989da6df7b295c2dae41b1a622) )
	ROM_LOAD( "scxcstv1", 0x0000, 0x010000, CRC(d8b62a7e) SHA1(c099f1d75b02c1535b81473a7ded6f58ab439430) )
	ROM_LOAD( "scxcstv2", 0x0000, 0x010000, CRC(ea0a9f41) SHA1(35799bacb2a1f3862169881f0f6dc10417d57fc4) )
	ROM_LOAD( "scxgm10c.bin", 0x0000, 0x010000, CRC(4a4d70d2) SHA1(9f14f6ba0a3c6c5c1a78c68b0bba49fb948142e5) )
	ROM_LOAD( "scxgm14n", 0x0000, 0x010000, CRC(604ec82a) SHA1(01876c1d97be5d4c32641b01314909254a7b5b26) )
	ROM_LOAD( "scxgm14o", 0x0000, 0x010000, CRC(52f27d15) SHA1(72a87d09f57b88f18ca185aace5026db870a40ff) )
	ROM_LOAD( "scxgm1jg", 0x0000, 0x010000, CRC(4995b83b) SHA1(aeb2d19dab1dab906f3418b5047bcebe0b395c90) )
	ROM_LOAD( "scxgm2.0", 0x0000, 0x020000, CRC(216cb51b) SHA1(0814115cb0d8f1042b3b9c9802079be0adc0e106) )
	ROM_LOAD( "scxgmbb.bin", 0x0000, 0x010000, CRC(1786f17d) SHA1(91b6f1badc09d28d81cfa08d8713ababae59dfab) )
	ROM_LOAD( "scxgmbt.bin", 0x0000, 0x010000, CRC(253a4442) SHA1(d362261a9e537e61be52efb13e825942934fa2ac) )
	ROM_LOAD( "scxgmv2.grn", 0x0000, 0x010000, CRC(b682bc15) SHA1(45b8aeedb63b8e0aa9ebf5b3b74e44cb07aedff9) )
	ROM_LOAD( "scxgmv2b", 0x0000, 0x010000, CRC(b682bc15) SHA1(45b8aeedb63b8e0aa9ebf5b3b74e44cb07aedff9) )
	ROM_LOAD( "scxgv1gr", 0x0000, 0x010000, CRC(a7f159ec) SHA1(6aedd61233d3e29e074b2c44679a7ac7ab999949) )
	ROM_LOAD( "scxgv1hi", 0x0000, 0x010000, CRC(04730062) SHA1(ea84b52556b03abe2ed2676cb14ef3a4d7dfdc64) )
	ROM_LOAD( "scxgv1lo", 0x0000, 0x010000, CRC(c5db0a69) SHA1(7a500bd4f68ce3bc56fd3d370f1144c485089023) )
	ROM_LOAD( "scxgv2gr", 0x0000, 0x010000, CRC(2e37f306) SHA1(ba0a8dc107abc9ab093c2d6f81ec3f11e5460598) )
	ROM_LOAD( "scxgv2hi", 0x0000, 0x010000, CRC(8db5aa88) SHA1(626db3b1eddb50137e8f05535137db9dff466806) )
	ROM_LOAD( "scxgv2lo", 0x0000, 0x010000, CRC(4c1da083) SHA1(75684018ed2988688bb3be7990dc0050d28bd4ef) )
ROM_END

ROM_START( sc2scshxstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scxsp10.bin", 0x0000, 0x010000, CRC(2fe512ad) SHA1(d409f27a62405dc45f487f9351e4d158e4d35440) ) // sldh
	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "scxspv1grn.bin", 0x0000, 0x010000, CRC(67f69bd4) SHA1(ee2dd0cd98c98a4727df8c7c721de9ac49b583ff) )
	ROM_LOAD( "scxspv2grn.bin", 0x0000, 0x010000, CRC(2fe512ad) SHA1(d409f27a62405dc45f487f9351e4d158e4d35440) )
	ROM_LOAD( "scxv1hi.str", 0x0000, 0x010000, CRC(5c7781d7) SHA1(0bbd48d6b506a31fe7d48122589f434a4473c225) )
	ROM_LOAD( "scxv1lo.str", 0x0000, 0x010000, CRC(2dc93bce) SHA1(63ce1eecf454f83f51107ec7c1d8ac04408c7414) )
	ROM_LOAD( "scxv2hi.str", 0x0000, 0x010000, CRC(e7e921ea) SHA1(fa150e78981bd91f5b8d148a1a32836ee4dde926) )
	ROM_LOAD( "scxv2lo.str", 0x0000, 0x010000, CRC(96579bf3) SHA1(02abf8c84119a3ac828f91c236ce8573cf6cd646) )
	ROM_LOAD( "scxhiv1.stp", 0x0000, 0x010000, CRC(f087f88c) SHA1(a303e1d8249eb2a83e122f5b355dc084ce46b172) )
	ROM_LOAD( "scxhiv2.stp", 0x0000, 0x010000, CRC(adf9a0bf) SHA1(58c0e64175ceb222e285fae29337f2a5437364e4) )
	ROM_LOAD( "scxlov1.stp", 0x0000, 0x010000, CRC(3b53fcaa) SHA1(bb6d9b70063dbbeb7562225a07610b424d1ebdd4) )
	ROM_LOAD( "scxlov2.stp", 0x0000, 0x010000, CRC(42086397) SHA1(254bc42c9f2cc55bbeecbe2fb06234aaeda7967d) )
ROM_END

ROM_START( sc2scshxcas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scx1v7h", 0x0000, 0x010000, CRC(b8ae7542) SHA1(22230e9a67c0f8408d6ba7adafd581cd3d62c5ad) )
	ROM_REGION( 0x200000, "altrevs", ROMREGION_ERASE00 )
	ROM_LOAD( "scx1v7l", 0x0000, 0x010000, CRC(1ed97ef6) SHA1(1aaf911369dc814ee2edf5d59baa2961bfc73168) )// Second board?
ROM_END

ROM_START( sc2cgc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95751968.p1", 0x0000, 0x010000, CRC(e9eef2be) SHA1(61015e0c90fd516da56243a7eef3d5d2412d880f) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cnile )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cash-on-the-nile_std_ac_var_150pnd_rot_ass.bin", 0x0000, 0x010000, CRC(4a5b4b9f) SHA1(aaeaa42cf42d91002c61e4c0df49d7ef97e00b2a) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cnile1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash-nile 150 047s.bin", 0x0000, 0x010000, CRC(2d8e9037) SHA1(b3d93488d662260cfaaf624baec68dbe92f71640) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cnile2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cash-on-the-nile_std_ac_var_200pnd_rot_ass.bin", 0x0000, 0x010000, CRC(41cbb60d) SHA1(4fede32a8d0957a46732f6851d4af7fd959d9fb5) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cnile2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cash-on-the-nile_dat_ac_var_200pnd_rot_ass.bin", 0x0000, 0x010000, CRC(3bfac54c) SHA1(ecfd7607676c1620ee37718578675437911cf147) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cnilep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-cash-on-the-nile_dat_ac_var_250pnd_rot_ass.bin", 0x0000, 0x010000, CRC(42d0a11d) SHA1(b38fa1360f0b8d465bb0e0759f73e0b98a545ad3) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_std_wit_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(cc2ef9dd) SHA1(9e85e319fbe74f31de1fddc4f15dd0ce49691d2c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casrp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_dat_wit_ac_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(6436974c) SHA1(419d4f58f518582f0fe334323d0d9fa68f9458a6) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_std_ac_var_tri3_ass.bin", 0x0000, 0x010000, CRC(5f47c57b) SHA1(5ce7baab279ee28c337a4ee72038b6d6cee1da9c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_dat_ac_var_tri3_ass.bin", 0x0000, 0x010000, CRC(bc805e51) SHA1(c6b0e2fc1011688ca9c374bb5cca5788e6dea005) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_std_wit_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(11663ae8) SHA1(f8e0fb8b23c192f48df4e5d9fc94f8c625d4771c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_dat_wit_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(8a6eab70) SHA1(ef2bb7f7fd534dfee1322b9fd151e24642cb28bf) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_std_ss_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(6a585c28) SHA1(8ac91085efd8382544868b8b0b45fddede38b5ec) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_dat_ss_ac_var_8pnd_ass.bin", 0x0000, 0x010000, CRC(341e69a8) SHA1(3b719a437e11ca71a9acccc76cd5f2b05325e203) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_std_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(a25c051a) SHA1(6b7e954c53e3f1f90d24f88c7fd09606a1cd8630) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2casr4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-royale_dat_ac_var_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(8c59c768) SHA1(fec9cfbd9a8c262d053ae84c09535a7d8331bfa2) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END




ROM_START( sc2cmbt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-and-bonzo-too_std_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(6d1612e7) SHA1(641104b4ebc99ec3b20a081fccbde70084cc329a) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cmbtp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cat-and-mouse-and-bonzo-too_dat_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(14770ed7) SHA1(6e7a0f596063c28cad0ecc13241e53e4a5b025f9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END




ROM_START( sc2dbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "double-diamond_std_ga_20p_ass.bin", 0x0000, 0x010000, CRC(eded5c38) SHA1(31a687de56f95f0ab730fed2b618e492fbc0c749) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2dblp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "double-diamond_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(7e117a69) SHA1(d73ec1cfe3d2b9d9e1f18a3979d76b13b5d89988) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2dbl1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "double-diamond_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(90a52fc4) SHA1(afb6078fc884e08afb4f6a9ac2a8abcb36fae2bd) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2dbl1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "double-diamond_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(d45b4a5c) SHA1(ccf33b36c01155e78492e861ae4a328b4086ade9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc2flaca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashcash_std_ac_var_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(2cb1802a) SHA1(ae7bc9374f5882ba4142d67ad83335d4d2accf0c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2flacap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashcash_dat_ac_var_10pnd_tri3_ass.bin", 0x0000, 0x010000, CRC(0d6d8eb2) SHA1(c47e61d08afbeb542132b2f8e157417a008e9387) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2flaca1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashcash_std_ac_var_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(e7e0fe1f) SHA1(a6c1f6565d785aa36daecb55a5c33042a84117e6) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2flaca1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashcash_dat_ac_var_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(89d6df7f) SHA1(cc8f6ca2233d77cbdcb0735cb31de9cdd9a66408) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2flaca2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashcash_std_wi_ac_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(5f5b879b) SHA1(1b0f0cf54112615ea6b2ecdebc4076d132531a2c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2flaca2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashcash_dat_wi_ac_10pnd_tri2_ass.bin", 0x0000, 0x010000, CRC(b781ecf9) SHA1(1d62a315f8292144d8129c1be9a83a5e717c6ed9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2foot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "football-club_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(b4d46ee2) SHA1(3cbe603c2703570eb49682ca9dbb6ad9ede020e6) )

	sc2_foot_matrix
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2footp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "football-club_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(fabaf221) SHA1(ddefc6f46339f83b6cfbacbe1ff6cf065d0157aa) )

	sc2_foot_matrix

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2foot1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "football-club_std_ac_var_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(bf35ad75) SHA1(c5e8906138184449b90eea2e280e6f75e6768776) )

	sc2_foot_matrix

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2foot1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "football-club_dat_ac_var_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(7f6acf47) SHA1(b6e8254d4af1e5a85166e4eca1dc2b1ea2eed292) )

	sc2_foot_matrix

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2foot2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "football-club_std_var_ass.bin", 0x0000, 0x010000, CRC(cae35c7a) SHA1(2beda0150cd2d413269c350e34102c0e1d3ed007) )

	sc2_foot_matrix

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2foot2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "football-club_dat_var_ass.bin", 0x0000, 0x010000, CRC(ac088604) SHA1(d1db45aa19b645aad56bbf84e551dc1cca22f92d) )

	sc2_foot_matrix

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END




ROM_START( sc2gcclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-golden-casino_std_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(5f7fc343) SHA1(264c5bba36c820440c2ed97c04d4dd3592e111da) )
	sc2_gcclb_matrix
	sc2_gcclb_sound
ROM_END

ROM_START( sc2gcclbp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-golden-casino_dat_ac_250pnd-20p_ass.bin", 0x0000, 0x010000, CRC(b4dee6d2) SHA1(856672fb4767f66e976619392fc8e659fbca3c2e) )
	sc2_gcclb_matrix
	sc2_gcclb_sound
ROM_END

ROM_START( sc2gcclb1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-golden-casino_std_ac_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(bf7b9ff1) SHA1(890a6b96592e9d2e890bea95e711b890c1cda7ad) )
	sc2_gcclb_matrix
	sc2_gcclb_sound
ROM_END

ROM_START( sc2gcclb1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-golden-casino_dat_ac_100pnd-5p_ass.bin", 0x0000, 0x010000, CRC(1f5c2a2b) SHA1(facaab47716ae3c4a10839523f3249074ae8abb1) )
	sc2_gcclb_matrix
	sc2_gcclb_sound
ROM_END

ROM_START( sc2gcclb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-golden-casino_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(2de27b80) SHA1(57f1c40ceeb6ab82e9bac547aa00d8c1c1c07dab) )
	sc2_gcclb_matrix
	sc2_gcclb_sound
ROM_END

ROM_START( sc2gcclb2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-golden-casino_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(2ad9647e) SHA1(d423c060996417f3f7f1b61e911b6e523ad08e7a) )
	sc2_gcclb_matrix
	sc2_gcclb_sound
ROM_END




ROM_START( sc2groul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "golden-roulette_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(d865188a) SHA1(c4318984b6abdb5671fe7c323608e4af84d1ae6e) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2groulp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "golden-roulette_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(c388fa79) SHA1(4ce7d183130fd2aae2c4ffeff652e2602208c3ff) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



ROM_START( sc2gtr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gtr.bin", 0x0000, 0x010000, CRC(b6cd277c) SHA1(4951bb6b4cc1bf655d3b63b7af4f1a6a297a201c) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gtr_snd.bin", 0x0000, 0x080000, CRC(90eaa8b6) SHA1(9c15787d73889013717f01c6b11780b7f9314b05) )
ROM_END

ROM_START( sc2heypr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hey-presto_std_ac_4pnd-10p_ass.bin", 0x0000, 0x010000, CRC(7f3803fa) SHA1(56a12bb96fe7cce07734842f6c5581648154154e) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2heyprp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hey-presto_dat_ac_4pnd-10p_ass.bin", 0x0000, 0x010000, CRC(cb8780ad) SHA1(a0a3cd2c9c3caf6607b55d2d14f6e3d581540808) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


ROM_START( sc2hypr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyperactive_std_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(042b848c) SHA1(ceec2cb26ae9b969c5da3cc0be25455b1f89d09f) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2hyprp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyperactive_dat_ac_var_10pnd_ass.bin", 0x0000, 0x010000, CRC(e6956fec) SHA1(ea8e25e16a451a1f52f30567571090f635379f4c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2hypr1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyperactive_std_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(2d01bc08) SHA1(c2186fb639735d4e1d46ceaeae6eee63c7a740b7) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2hypr1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyperactive_dat_wi_ac_10pnd-20p_ass.bin", 0x0000, 0x010000, CRC(6d11a9eb) SHA1(d68564a96984c5dde536add4507bc8bae75e19ea) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2kcclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-king-cash_std_ac_var_ass.bin", 0x0000, 0x010000, CRC(60c1eccd) SHA1(5b9f5c8c7cc501b557eadcf7e520967c58b8ce1a) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2kcclbp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-king-cash_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(c83be316) SHA1(9e87152977fdabb71ee6d8be1d382b978d856c83) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2kcclb1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-king-cash_std_ac_ds_rot_ass.bin", 0x0000, 0x010000, CRC(cf13d7e4) SHA1(6b3bfc8e7e4877e7ab7e5d3adbd89a6bcc2ebde9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2kcclb1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-king-cash_dat_ac_ds_rot_ass.bin", 0x0000, 0x010000, CRC(3fb9f61f) SHA1(176e517d049b4e588a2fe425041d701ff8e3e7b8) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2maina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "game115s.rom", 0x0000, 0x010000, CRC(6f3b16d2) SHA1(b5c7796a4a87dc5ffa6243863ac3f9bc777228ca) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd2008.rom", 0x0000, 0x040000, CRC(9b2b5b33) SHA1(3ec9200529eba5bc4ef4a9a289d58312f29628a5) )
ROM_END


ROM_START( sc2olgld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympic-gold_std_var_ac_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(baa98b60) SHA1(2b73eb21d6b612fabf855edf9f6c46897714729b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2olgldp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympic-gold_dat_var_ac_8-10pnd_ass.bin", 0x0000, 0x010000, CRC(1348a519) SHA1(e7a2434235347433522c55e4d4f89fbb97759765) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2olgld1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympic-gold_std_wi_var_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(8a8b8429) SHA1(ba886878d4ef428653032d04e21a9031fdea68e0) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2olgld1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "olympic-gold_dat_wi_var_ac_10pnd_ass.bin", 0x0000, 0x010000, CRC(bb50e0a2) SHA1(b208053e114f7fb411f16f02aab3061f6075b42c) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2relgm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reel-gems_std_ms_20p_ass.bin", 0x0000, 0x010000, CRC(ebbae111) SHA1(6372e19b0dd030aac517344449ce47e8f6f74b29) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2relgmp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reel-gems_dat_ms_20p_ass.bin", 0x0000, 0x010000, CRC(5abde2bc) SHA1(74a745938934533b1b33c99828b79fa9d1e86a91) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2relgm1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reel-gems_dat_ga_20p_ass.bin", 0x0000, 0x010000, CRC(86e81781) SHA1(7b59efa627f70b2c3598c5abd276a7c2737b0751) )

	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2topwk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "top-wack_std_wi_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(248080cf) SHA1(067077af93dd6a41bd6d84d9ace9ac4cea36f01b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2topwkp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "top-wack_dat_wi_ac_10pnd_tri1_ass.bin", 0x0000, 0x010000, CRC(56fd3003) SHA1(37ef5c9a750f9bdc609fc78ea5131424eb74c79d) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END


/* was in an SC4 set, is it meant to link with the SC4 units? */
ROM_START( sc2cb7p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95751960.p1", 0x0000, 0x010000, CRC(9f944d0c) SHA1(feb8fe4ce0a8f5c4a034aafec0f5aae29a834e8d) )
	sc2_cb7_sound
ROM_END


ROM_START( sc2cb7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bar7.bin", 0x0000, 0x010000, CRC(c5b426e8) SHA1(a60aed70f2a4cf4356fae61c1031124fd5987d86) )

	sc2_cb7_sound
ROM_END



ROM_START( sc2cb71 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casinobar7_bfm_allcash.bin", 0x0000, 0x010000, CRC(2d459734) SHA1(293cf250b7b71b55325b18a10be7dead1cddb565) )

	sc2_cb7_sound
ROM_END

ROM_START( sc2cb72 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-bar-7_std_10pnd_ass.bin", 0x0000, 0x010000, CRC(3d0ae920) SHA1(4c6575d979f686e928842afc3ee9b344e45e3a31) )

	sc2_cb7_sound
ROM_END

ROM_START( sc2cb72p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casino-bar-7_dat_10pnd_ass.bin", 0x0000, 0x010000, CRC(6960f4f8) SHA1(7274276d1d4032ed7fe660ac0f87eea1e9c6e4e4) )

	sc2_cb7_sound
ROM_END

ROM_START( sc2cgcas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-casino_std_ac_p65_ass.bin", 0x0000, 0x010000, CRC(6ca2cccb) SHA1(762e0809e70d4dd2161a2ffcc30d191720e8ad9a) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cgcasp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-casino_dat_ac_var_ass.bin", 0x0000, 0x010000, CRC(15c3b640) SHA1(94a4e105b9fbd4b12ec246a0f1a6751acf25eac2) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cgcas1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-casino_std_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(edfc3d74) SHA1(192a893b5a9b188de094d0f45881788306523e0b) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cgcas1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club-grand-casino_dat_ac_250pnd-25p_ass.bin", 0x0000, 0x010000, CRC(77cf0f11) SHA1(88da3f2e18f621033a8d32428b1422d5e3873ab5) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_std_ac_var_a.bin", 0x0000, 0x010000, CRC(88dd09b9) SHA1(36b4f3504794b638a31e45d1f155360166f77ab2) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_std_ac_var_10pnd_tri3_a.bin", 0x0000, 0x010000, CRC(3d808af5) SHA1(db29c03a33dce6342fec4da3664590ab072dd6d9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega1p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_dat_ac_var_10pnd_tri3.bin", 0x0000, 0x010000, CRC(ab3e503c) SHA1(2c26865eab6cf128d8f3ff09077daa3c4d2aee30) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_std_to_var_8pnd_a.bin", 0x0000, 0x010000, CRC(c8e98a0e) SHA1(1436f3a464b2f298b161e5328f0540cf23441803) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_dat_to_8pnd_a.bin", 0x0000, 0x010000, CRC(cabec1cd) SHA1(acbe41e0d5fa77f11df8d119ad09aeccd421f603) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_std_w_i_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(521b918d) SHA1(4d9b94d561d89aa1dd8746a33eb27d89b53b6ba9) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega3p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_dat_wi_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(99ee9eef) SHA1(c4b325a39e898f069ac3471af8ea955c62c488a5) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END

ROM_START( sc2cvega4p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_vegas_dat_ac_var_10pnd_a.bin", 0x0000, 0x010000, CRC(e880c6b6) SHA1(387f7e3659e42ac488db9a4768c2035f7c870c44) )
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
ROM_END



/* Video Based (Adder 2) */

#define GAME_FLAGS MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL

GAMEL( 1993, quintoon, 0,         scorpion2_vidm, quintoon, bfm_sc2_state,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-750-206)",          MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND,layout_quintoon ) //Current samples need verification
GAMEL( 1993, quintond, quintoon,  scorpion2_vidm, quintoon, bfm_sc2_state,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-751-206, Datapak)",MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND|MACHINE_NOT_WORKING,layout_quintoon ) //Current samples need verification
GAMEL( 1993, quintono, quintoon,  scorpion2_vidm, quintoon, bfm_sc2_state,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-750-203)",          MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND,layout_quintoon ) //Current samples need verification

GAMEL( 1993, qntoond,  0,         scorpion2_vid, qntoond, bfm_sc2_state,   adder_dutch,0,       "BFM/ELAM", "Quintoon (Dutch, Game Card 95-750-243)",       MACHINE_SUPPORTS_SAVE,layout_quintoon )
GAMEL( 1993, qntoondo, qntoond,   scorpion2_vid, qntoond, bfm_sc2_state,   adder_dutch,0,       "BFM/ELAM", "Quintoon (Dutch, Game Card 95-750-136)",       MACHINE_SUPPORTS_SAVE,layout_quintoon )

GAMEL( 1994, pokio,    0,         scorpion2_vid, pokio, bfm_sc2_state,     adder_dutch,0,       "BFM/ELAM", "Pokio (Dutch, Game Card 95-750-278)",          MACHINE_SUPPORTS_SAVE,layout_pokio )

GAMEL( 1995, paradice, 0,         scorpion2_vid, paradice, bfm_sc2_state,  adder_dutch,0,       "BFM/ELAM", "Paradice (Dutch, Game Card 95-750-615)",       MACHINE_SUPPORTS_SAVE,layout_paradice )

GAMEL( 1996, pyramid,  0,         scorpion2_vid, pyramid, bfm_sc2_state,   pyramid, 0,          "BFM/ELAM", "Pyramid (Dutch, Game Card 95-750-898)",       MACHINE_SUPPORTS_SAVE,layout_pyramid )

GAMEL( 1995, slotsnl,  0,         scorpion2_vid, slotsnl, bfm_sc2_state,   adder_dutch,0,       "BFM/ELAM", "Slots (Dutch, Game Card 95-750-368)",          MACHINE_SUPPORTS_SAVE,layout_slots )

GAMEL( 1996, sltblgtk, 0,         scorpion2_vid, sltblgtk, bfm_sc2_state,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Token, Game Card 95-750-943)",  MACHINE_SUPPORTS_SAVE,layout_sltblgtk )

GAMEL( 1996, sltblgpo, 0,         scorpion2_vid, sltblgpo, bfm_sc2_state,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Cash, Game Card 95-750-938)",   MACHINE_SUPPORTS_SAVE,layout_sltblgpo )
GAMEL( 1996, sltblgp1, sltblgpo,  scorpion2_vid, sltblgpo, bfm_sc2_state,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Cash, Game Card 95-752-008)",   MACHINE_SUPPORTS_SAVE,layout_sltblgpo )

GAMEL( 1997, gldncrwn, 0,         scorpion2_vid, gldncrwn, bfm_sc2_state,  gldncrwn,   0,       "BFM/ELAM", "Golden Crown (Dutch, Game Card 95-752-011)",   MACHINE_SUPPORTS_SAVE,layout_gldncrwn )

/* Non-Video */




/********************************************************************************************************************************************************************************************************************
 Dr.Who The Timelord
  (also Dr.Who The Timelord Deluxe)
  the Mazooma release doesn't boot, gives error 99
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6305  DR WHO TIMELORD - 28-SEP-1994 11:14:58
GAMEL( 1994, sc2drwho   , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 1, UK, Single Site) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-288
GAMEL( 1994, sc2drwhou  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwhon     , 0,         "BFM",      "Dr.Who The Timelord (set 1, UK, Single Site) (Scorpion 2/3) (not encrypted)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) //  GAME No 95-750-288 (unencrypted bootleg?)
GAMEL( 1994, sc2drwhop  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 1, UK, Single Site Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-288
// PROJECT NUMBER 6305  DR WHO TIMELORD IRISH ALL CASH   - 28-SEP-1994 11:20:17
GAMEL( 1994, sc2drwho1  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 2, UK, Arcade) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-290
GAMEL( 1994, sc2drwho1p , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 2, UK, Arcade, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-290
// PROJECT NUMBER 6305  DR WHO TIMELORD NO JP SPIN - 17-NOV-1994 09:34:50
GAMEL( 1994, sc2drwho2  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 3, UK, no Jackpot spin) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-309
GAMEL( 1994, sc2drwho2p , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 3, UK, no Jackpot spin, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-309
// PROJECT NUMBER 6305  DR WHO TIMELORD ARCADE - 24-OCT-1995 16:12:44
GAMEL( 1994, sc2drwho3  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 4, UK, Arcade) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-536
GAMEL( 1994, sc2drwho3p , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 4, UK, Arcade, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-536
// PROJECT NUMBER 6305  DR WHO TIMELORD 4/8 - 24-OCT-1995 16:14:30
GAMEL( 1994, sc2drwho4  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 5, UK) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-535
GAMEL( 1994, sc2drwho4p , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 5, UK, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-535
// PROJECT NUMBER 6305  DR WHO TIMELORD IRISH ALL CASH 4/8 - 25-OCT-1995 09:50:12
GAMEL( 1994, sc2drwho5  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 6, UK, Arcade, 8GBP Jackpot) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-531
GAMEL( 1994, sc2drwho5p , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 6, UK, Arcade, 8GBP Jackpot, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-531
// PROJECT NUMBER 6305  TIMELORD ARCADE 10P PLAY 4/8 - 25-OCT-1995 09:53:06
GAMEL( 1994, sc2drwho6  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 7, UK, Arcade) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-533
GAMEL( 1994, sc2drwho6p , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 7, UK, Arcade, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-533
// PROJECT NUMBER 6305  DR WHO TIMELORD 10 POUNDS - 28-MAR-1996 13:21:58
GAMEL( 1994, sc2drwho7  , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 8, UK, Arcade, 10GBP Jackpot) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-661
GAMEL( 1994, sc2drwho7p , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord (set 8, UK, Arcade, 10GBP Jackpot, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-751-661
// PROJECT NUMBER 6419  DR WHO TIMELORD DELUXE - 8-MAR-1995 15:37:53
GAMEL( 1994, sc2drwhodx , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord Deluxe (set 1) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-370
// PROJECT NUMBER 6419  TIMELORD DELUXE MULTI-SITE ALL CASH - 4-DEC-1995 10:48:34
GAMEL( 1994, sc2drwhodx1, sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Dr.Who The Timelord Deluxe (set 2) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL,layout_drwho) // GAME No 95-750-572
// PROJECT NUMBER 6999  TIMELORD AT PLAYMAKER 5P/10p  500P - 15-SEP-1997 10:02:47
GAMEL( 1994, sc2drwhomz , sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",      "Dr.Who The Timelord (Mazooma) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL | MACHINE_NOT_WORKING,layout_drwho) // GAME No TLVMAZ12_N, error 99
// PROJECT NUMBER TLP12  TIMELORD AT PLAYMAKER 5P/10p  500P - 15-SEP-1997 10:03:49
GAMEL( 1994, sc2drwhomzp, sc2drwho  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",      "Dr.Who The Timelord (Mazooma, Protocol) (Scorpion 2/3)", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL | MACHINE_NOT_WORKING,layout_drwho) // GAME No TLVMAZ12_P,  error 99

/********************************************************************************************************************************************************************************************************************
 The Big Breakfast
  project number jumps between 640X and 6514, why?
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 640X  BIG BREAKFAST VAR STAKE/JACKPOT - 16-OCT-1995 14:57:47
GAME( 1994, sc2brkfs1   , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 1 UK, Single Site) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-523
GAME( 1994, sc2brkfs1p  , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 1 UK, Single Site, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-523
// PROJECT NUMBER 6514  BIG BREAKFAST ARCADE VAR STAKE/JACKPOT  - 16-OCT-1995 14:59:52
GAME( 1994, sc2brkfs    , 0         ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 2) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-524
GAME( 1994, sc2brkfsp   , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 2, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-524
// PROJECT NUMBER 640X  BIG BREAKFAST VAR STAKE #6 CASH JACKPOT - 20-OCT-1995 11:59:24
GAME( 1994, sc2brkfs3   , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 3) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-517
GAME( 1994, sc2brkfs6   , sc2brkfs  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Big Breakfast (set 3, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-517
// PROJECT NUMBER 6514  BIG BREAKFAST VAR STAKE #8 CASH JACKPOT - 12-JAN-1996 12:52:36
GAME( 1994, sc2brkfs2   , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 4 UK, Arcade, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-604
GAME( 1994, sc2brkfs3p  , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 4 UK, Arcade, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-604
// PROJECT NUMBER 6514  BIG BREAKFAST VAR STAKE !10 CASH JACKPOT - 16-MAY-1996 11:52:49
GAME( 1994, sc2brkfs4   , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 5 UK, Arcade, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-709
GAME( 1994, sc2brkfs4p  , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 5 UK, Arcade, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-709
// PROJECT NUMBER 6514  BIG BREAKFAST WHITBREAD #10 ALL CASH 20P PLAY - 16-MAY-1996 12:16:31
GAME( 1994, sc2brkfs5   , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 6 UK, Arcade, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-710
GAME( 1994, sc2brkfs5p  , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast (set 6 UK, Arcade, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-710


/********************************************************************************************************************************************************************************************************************
 The Big Breakfast Casino
  alpha seems different to others, unless it should be a DMD?
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6621  BIG BREAKFAST CASINO GALA TOKEN - 19-AUG-1997 09:21:25
GAME( 1994, sc2brkfsm   , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "The Big Breakfast Casino (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-986
// PROJECT NUMBER 7013  BIG BREAKFAST CASINO AT MAZOOMA 5P - 23-JAN-1998 11:29:13
GAME( 1994, sc2brkfsm1  , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM/Mazooma",      "The Big Breakfast Casino (Mazooma, set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No BBCVM51
// PROJECT NUMBER 7013  BIG BREAKFAST CASINO AT MAZOOMA 5P - 14-MAY-1998 10:05:23
GAME( 1994, sc2brkfsm2  , sc2brkfs  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM/Mazooma",      "The Big Breakfast Casino (Mazooma, set 2) (Scorpion 2/3)", GAME_FLAGS) // GAME No BBCVM52

/********************************************************************************************************************************************************************************************************************
 Match Of The Day
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6400  MATCH OF THE DAY IRISH ALL CASH - 15-MAY-1995 14:47:45
GAME( 199?, sc2motd9    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 1, Irish) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-413
// PROJECT NUMBER 6560  MATCH OF THE DAY STANDARD S+P #4/#8 - 6-OCT-1995 10:59:44
GAME( 199?, sc2motd5    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 2, UK, Single Site) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-515
GAME( 199?, sc2motd5p   , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 2, UK, Single Site, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-515
// PROJECT NUMBER 6560  MATCH OF THE DAY ARCADE S+P #4/#8 - 6-OCT-1995 11:37:00
GAME( 199?, sc2motd4    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 3, UK, Arcade) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-513
GAME( 199?, sc2motd4p   , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 3, UK, Arcade, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-513
// PROJECT NUMBER 6560  MATCH OF THE DAY STANDARD S+P #4/#8 IRISH ALL CASH - 6-OCT-1995 12:20:16
GAME( 199?, sc2motd6    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 4, Irish, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-514
GAME( 199?, sc2motd6p   , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 4, Irish, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-514
// PROJECT NUMBER 6587 (6311)  MATCH OF THE DAY STANDARD S+P #4/#8 - 14-NOV-1995 13:40:49
GAME( 199?, sc2motd7    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 5, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-559
GAME( 199?, sc2motd8p   , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 5, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-559
// PROJECT NUMBER 6587 (6311)  MATCH OF THE DAY ARCADE S+P #4/#8 - 14-NOV-1995 13:42:06
GAME( 199?, sc2motd3    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 6, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-560
GAME( 199?, sc2motd3p   , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 6, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-560
// PROJECT NUMBER 6770  MATCH OF THE DAY STANDARD #10 ALL CASH - 11-APR-1996 15:05:22
GAME( 199?, sc2motd1    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 7, UK, 10GBP Jackpot, 1st Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-671
GAME( 199?, sc2motd1p   , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 7, UK, 10GBP Jackpot, 1st Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-671
// PROJECT NUMBER 6770  MATCH OF THE DAY WHITBREAD #10 ALL CASH - 11-APR-1996 15:08:33
GAME( 199?, sc2motd2    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 8, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-672
GAME( 199?, sc2motd2p   , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 8, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-672
// PROJECT NUMBER 6798  MATCH OF THE DAY STANDARD #10 ALL CASH - 14-AUG-1996 11:54:58
GAME( 199?, sc2motd     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 9, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-852
GAME( 199?, sc2motdp    , sc2motd   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day (Bellfruit) (set 9, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-852

/********************************************************************************************************************************************************************************************************************
 Match Of The Day - Road To Wembley
  earliest version has project number 6401 and MOTD in the header (all versions show the title as Match of the Day - Road To Wembley)
  there are also versions with #6555 (oct 1995) #6781 (in 1996) and 7005 (the Mazooma rebuild)
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6401  MOTD ROAD TO WEMBLEY 16RM  GAME No 95-750-335 -  9-JAN-1995 12:26:53
GAME( 199?, sc2wembl7a  , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 1, UK, 8GBP Jackpot, 16RM motor) (Scorpion 2/3)", GAME_FLAGS)
// PROJECT NUMBER 6555  ROAD TO WEMBLEY 16RM #8 - 2-OCT-1995 14:53:30
GAME( 199?, sc2wembl5a  , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 2, UK, 16RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-498
GAME( 199?, sc2wembl5ap , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 2, UK, 16RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-498
// PROJECT NUMBER 6555  ROAD TO WEMBLEY 16RM 10P VERSION #8 - 2-OCT-1995 15:09:05
GAME( 199?, sc2wembl8   , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-499
GAME( 199?, sc2wembl4p  , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 3, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-499
// PROJECT NUMBER 6555  ROAD TO WEMBLEY ARCADE 16RM #8 - 2-OCT-1995 15:12:53
GAME( 199?, sc2wembl9   , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 4, Arcade, 16RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-500
GAME( 199?, sc2wembl6ap , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 4, Arcade, 16RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-500
// PROJECT NUMBER 6555  ROAD TO WEMBLEY IRISH ALL CASH #8 16RM - 2-OCT-1995 15:14:41
GAME( 199?, sc2wembl10  , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 5, Irish, 8GBP Jackpot, 16RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-501
GAME( 199?, sc2wembl7ap , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 5, Irish, 8GBP Jackpot, 16RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-501
// PROJECT NUMBER 6555  ROAD TO WEMBLEY 15RM #8 - 18-OCT-1995 11:46:51
GAME( 199?, sc2wembl1   , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 6, UK, 15RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-525
GAME( 199?, sc2wembl1p  , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 6, UK, 15RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-525
// PROJECT NUMBER 6781  ROAD TO WEMBLEY #10 ALL CASH 16RM - 15-MAR-1996 12:52:04
GAME( 199?, sc2wembl2   , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 7, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-653
GAME( 199?, sc2wembl2p  , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 7, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-653
// PROJECT NUMBER 6781  ROAD TO WEMBLEY #10 ALL CASH 15RM - 28-MAR-1996 14:01:40
GAME( 199?, sc2wembl    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 8, UK, 10GBP Jackpot, 15RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-659
GAME( 199?, sc2wemblp   , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Match Of The Day - Road To Wembley (Bellfruit) (set 8, UK, 10GBP Jackpot, 15RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-659
// PROJECT NUMBER 7005  ROAD TO WEMBLEY !5 ALL CASH 16RM  GAME No RWVMAZ12_N - 22-OCT-1997 20:03:46
GAME( 199?, sc2wemblm   , sc2wembl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",      "Match Of The Day - Road To Wembley (Bellfruit/Mazooma) (Scorpion 2/3)", GAME_FLAGS) // error 99


/********************************************************************************************************************************************************************************************************************
 The Game Show
  project numbers 6431, and 6575 (apr 1996+)
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6431  THE GAME SHOW - 28-FEB-1995 11:51:15
GAME( 199?, sc2gsclb5   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-362
// PROJECT NUMBER PR6431  THE GAME SHOW  - 9-JUN-1995 12:16:14
GAME( 199?, sc2gsclb7   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-424
// PROJECT NUMBER PR6431  THE GAME SHOW - 14-DEC-1995 16:10:08
GAME( 199?, sc2gsclb2   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 3, UK, Arcade) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-587
GAME( 199?, sc2gsclb2p  , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 3, UK, Arcade, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-587
// PROJECT NUMBER PR6431  THE GAME SHOW FIXED 65% - 23-JAN-1996 11:56:38
GAME( 199?, sc2gsclb    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 4, UK, Arcade, p65) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-588
GAME( 199?, sc2gsclbp   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 4, UK, Arcade, p65, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-588
// PROJECT NUMBER PR6431  THE GAME SHOW SEALINK VERSION - 23-JAN-1996 11:58:58
GAME( 199?, sc2gsclb3   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 5, UK, Arcade) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-589
GAME( 199?, sc2gsclb3p  , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 5, UK, Arcade, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-589
// PROJECT NUMBER PR6431  THE GAME SHOW FAST FILL CASHPOT - 23-JAN-1996 12:17:04
GAME( 199?, sc2gsclb4   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 6, UK, Arcade) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-590
GAME( 199?, sc2gsclb4p  , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 6, UK, Arcade, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-590
// PROJECT NUMBER PR6575  THE GAME SHOW FIXED 65% 25P PLAY - 11-APR-1996 14:46:58
GAME( 199?, sc2gsclb1   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 7, UK, Arcade, 250GBP Jackpot, p65) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-667
GAME( 199?, sc2gsclb1p  , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 7, UK, Arcade, 250GBP Jackpot, p65, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-667
// PROJECT NUMBER PR6575  THE GAME SHOW 25P PLAY - 20-AUG-1996 10:03:25
GAME( 199?, sc2gsclb6   , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 8, UK) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-844
GAME( 199?, sc2gsclb6p  , sc2gsclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "The Game Show Club (Bellfruit) (set 8, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-844


/********************************************************************************************************************************************************************************************************************
 Cops 'n' Robbers Club
********************************************************************************************************************************************************************************************************************/

// PROJECT PR6231  CLUB COPS AND ROBBERS - 20-JUL-1993 15:15:32
GAME( 199?, sc2copcl11  , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-118
// PROJECT PR6231  CLUB COPS AND ROBBERS 150 POUND JACKPOT - 16-NOV-1993 12:34:10
GAME( 199?, sc2copcl1   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-154
GAME( 199?, sc2copcl1p  , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 2, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-154
// PROJECT NUMBER PR6231  CLUB COPS AND ROBBERS FIXED 65% - 16-NOV-1993 12:35:38
GAME( 199?, sc2copcl4   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-156
// PROJECT PR6231  CLUB COPS AND ROBBERS - 16-NOV-1993 12:39:31
GAME( 199?, sc2copcl6   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 4, UK) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-153
GAME( 199?, sc2copcl6p  , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 4, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-153
// PROJECT PR6231  CLUB COPS AND ROBBERS GENEROUS 5P - 20-JAN-1994 11:13:45
GAME( 199?, sc2copcl7   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 5, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-190
// PROJECT PR6231  CLUB COPS AND ROBBERS - 4-AUG-1994 16:23:21
GAME( 199?, sc2copcl10  , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 6, UK) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-750-268
// PROJECT PR6231  CLUB COPS AND ROBBERS SEALINK VERSION - 22-MAY-1995 11:47:58
GAME( 199?, sc2copcl9   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 7, UK) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-409
GAME( 199?, sc2copcl9p  , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 7, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-409
// PROJECT NUMBER 6231  CLUB COPS AND ROBBERS GENEROUS 5P NPO - 2-FEB-1996 12:39:22
GAME( 199?, sc2copcl8   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 8, UK) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-628
GAME( 199?, sc2copcl8p  , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 8, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-628
// PROJECT NUMBER PR6231  CLUB COPS AND ROBBERS 25P/#250 - 2-SEP-1996 17:17:50
GAME( 199?, sc2copcl    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 9, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-859
GAME( 199?, sc2copclp   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 9, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-859
// PROJECT NUMBER PR6231  CLUB COPS AND ROBBERS 20P/#250 - 2-SEP-1996 17:26:44
GAME( 199?, sc2copcl2   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 10, UK) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-858
GAME( 199?, sc2copcl12  , sc2copcl  ,  scorpion2_3m     , drwho     , bfm_sc2_state, prom       , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 10, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-858
// PROJECT PR6231  CLUB COPS AND ROBBERS NPO 63% SEALINK VERSION - 5-JAN-1998 11:53:49
GAME( 199?, sc2copcl5   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 11, UK) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-752-015
GAME( 199?, sc2copcl11p , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 11, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-753-015
// PROJECT PR6231  CLUB COPS AND ROBBERS NPO 67% SEALINK VERSION - 5-JAN-1998 11:56:01
GAME( 199?, sc2copcl3   , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 12, UK) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-752-014
GAME( 199?, sc2copcl3p  , sc2copcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cops 'n' Robbers Club (Bellfruit) (set 12, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-753-014


/********************************************************************************************************************************************************************************************************************
 Super Bar 7 Casino
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6625  SUPER BAR SEVEN CASINO ALL CASH - 22-AUG-1996 16:22:26
GAME( 199?, sc2cb71     , sc2cb7    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Super Bar 7 Casino (Bellfruit) (set 1, UK, All Cash) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-853
// PROJECT NUMBER 6625  SUPER BAR SEVEN CASINO ALL CASH - 7-OCT-1996 11:08:33
GAME( 199?, sc2cb72     , sc2cb7    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Super Bar 7 Casino (Bellfruit) (set 2, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-872
GAME( 199?, sc2cb72p    , sc2cb7    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Super Bar 7 Casino (Bellfruit) (set 2, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-872
// PROJECT NUMBER 6625  SUPER BAR SEVEN CASINO NEW STATS - 8-MAY-1997 11:17:30
GAME( 199?, sc2cb7      , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Super Bar 7 Casino (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-960
GAME( 199?, sc2cb7p     , sc2cb7    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Super Bar 7 Casino (Bellfruit) (set 3, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-960


/********************************************************************************************************************************************************************************************************************
 Del's Millions
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6304  DELS MILLIONS - 6-APR-1994 09:57:37
GAME( 199?, sc2dels6    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-221
// PROJECT NUMBER 6304  DELS MILLIONS ARCADE - 4-AUG-1994 10:28:31
GAME( 199?, sc2dels7    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-262
// PROJECT NUMBER 6304  DELS MILLIONS ALL CASH - 2-SEP-1994 11:16:44
GAME( 199?, sc2dels3    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 3, UK, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-284
GAME( 199?, sc2dels3p   , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 3, UK, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-284
// PROJECT NUMBER 6304  DELS MILLIONS ARCADE DELUXE - 2-MAR-1995 09:13:57
GAME( 199?, sc2delsd    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 4, Deluxe) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-365
// PROJECT NUMBER 6566  DELS MILLIONS S&P #8 - 31-OCT-1995 15:17:16
GAME( 199?, sc2dels5    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 5, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-541
GAME( 199?, sc2dels9    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 5, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-541
// PROJECT NUMBER 6566  DELS MILLIONS S&P #8 ARCADE - 31-OCT-1995 16:09:31
GAME( 199?, sc2dels4    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 6, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-542
GAME( 199?, sc2dels4p   , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 6, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-542
// PROJECT NUMBER 6566  DELS MILLIONS S&P #8 ALL CASH - 11-APR-1996 14:29:24
GAME( 199?, sc2dels2    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 7, UK, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-670
GAME( 199?, sc2dels2p   , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 7, UK, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-670
// PROJECT NUMBER 6566  DELS MILLIONS !10 ALL CASH - 11-APR-1996 14:32:53
GAME( 199?, sc2dels8    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 8, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-669
// PROJECT NUMBER 6566  DELS MILLIONS !10 ALL CASH - 10-JUL-1996 08:00:19
GAME( 199?, sc2dels     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 9, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-821
GAME( 199?, sc2delsp    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 9, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-821
// PROJECT NUMBER 6566  DELS MILLIONS !10 WHITBREAD   - 10-JUL-1996 08:02:38
GAME( 199?, sc2dels1    , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 10, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-820
GAME( 199?, sc2dels1p   , sc2dels   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Del's Millions (Bellfruit) (set 10, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-820

// the Mazooma versioning is different, N/P should be protocol / non-protocol, but the builds are minutes apart, so 11/12/13/14 can't really be version numbers
// PROJECT NUMBER P7003  DELS MILLIONS  !5 ALL CASH
GAME( 199?, sc2delsm    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ11_N) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ11_N - 15-SEP-1997 11:52:00
GAME( 199?, sc2delsm2   , sc2delsm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ11_P) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ11_P - 15-SEP-1997 11:53:06
GAME( 199?, sc2delsmp   , sc2delsm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ12_N) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ12_N - 15-SEP-1997 11:54:08
GAME( 199?, sc2delsm2p  , sc2delsm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ12_P) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ12_P - 15-SEP-1997 11:55:11
GAME( 199?, sc2delsm1   , sc2delsm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ13_N) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ13_N - 15-SEP-1997 11:56:15
GAME( 199?, sc2delsm3   , sc2delsm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ13_P) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ13_P - 15-SEP-1997 11:57:17
GAME( 199?, sc2delsm1p  , sc2delsm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ14_N) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ14_N - 15-SEP-1997 11:58:21
GAME( 199?, sc2delsm3p  , sc2delsm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",  "Del's Millions (Bellfruit/Mazooma) (DMVMAZ14_P) (Scorpion 2/3)", GAME_FLAGS) // GAME No DMVMAZ14_P - 15-SEP-1997 11:59:25


/********************************************************************************************************************************************************************************************************************
 Down Town
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6402  DOWN TOWN 16RM - 12-JAN-1995 09:31:41
GAME( 199?, sc2downt6   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 1, UK, 16RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-337
// PROJECT NUMBER 6402  DOWN TOWN IRISH ALL CASH 16RM - 7-FEB-1995 16:45:43
GAME( 199?, sc2downt7   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 2, Irish, 16RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-354
// PROJECT NUMBER 6558  DOWN TOWN 16RM #8 - 17-OCT-1995 17:46:53
GAME( 199?, sc2downt8a  , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 3, UK, 16RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-519
GAME( 199?, sc2downt8ap , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 3, UK, 16RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-519
// PROJECT NUMBER 6558  DOWN TOWN ARCADE 16RM #8 - 17-OCT-1995 17:59:15
GAME( 199?, sc2downt1   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 4, UK, 16RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-520
GAME( 199?, sc2downt1p  , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 4, UK, 16RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-520
// PROJECT NUMBER 6558  DOWN TOWN ALL CASH #8 - 10-NOV-1995 09:56:29
GAME( 199?, sc2downt5   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 5, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-510
// PROJECT NUMBER 6558  DOWN TOWN #8 - 2-FEB-1996 10:36:24
GAME( 199?, sc2downt2   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 6, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-624
GAME( 199?, sc2downtp   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 6, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-624
// PROJECT NUMBER 6558  DOWN TOWN ARCADE #8 - 2-FEB-1996 10:38:03
GAME( 199?, sc2downt    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 7, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-625
GAME( 199?, sc2downt2p  , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 7, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-625
// PROJECT NUMBER 6558  DOWN TOWN ALL CASH !10 - 29-MAY-1996 12:37:51
GAME( 199?, sc2downt3a  , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 8, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-733
GAME( 199?, sc2downt3ap , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 8, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-733
// PROJECT NUMBER 6558  DOWN TOWN WHITBREAD !10 - 29-MAY-1996 12:39:25
GAME( 199?, sc2downt4a  , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 9, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-734
GAME( 199?, sc2downt4ap , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 9, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-734
// PROJECT NUMBER 6782  DOWN TOWN WHITBREAD !10 15RM - 18-JUN-1996 12:01:0
GAME( 199?, sc2downt4   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 10, UK, 15RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-780
GAME( 199?, sc2downt4p  , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 10, UK, 15RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-751-780
// PROJECT NUMBER 6782  DOWN TOWN ALL CASH !10 15RM - 18-JUN-1996 14:56:33
GAME( 199?, sc2downt3   , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 11, UK, 15RM motor) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-782
GAME( 199?, sc2downt3p  , sc2downt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Down Town (Bellfruit) (set 11, UK, 15RM motor, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-782


/********************************************************************************************************************************************************************************************************************
 Club Grand Slam
  (there are 2 sound roms, they differ by 2 bytes, one is probably bad)
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6591  CLUB GRAND SLAM - 7-AUG-1996 12:32:05
GAME( 1996, sc2gslam1   , sc2gslam  ,  scorpion2        , bfmcgslm  , bfm_sc2_state, bfmcgslm   , 0,         "BFM",      "Club Grand Slam (UK, set 1) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-750-843
GAME( 1996, sc2gslam1p  , sc2gslam  ,  scorpion2        , bfmcgslm  , bfm_sc2_state, bfmcgslm   , 0,         "BFM",      "Club Grand Slam (UK, set 1, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-843
// PROJECT NUMBER PR6591  CLUB GRAND SLAM - 29-OCT-2001 14:48:47
GAME( 1996, sc2gslam    , 0         ,  scorpion2        , bfmcgslm  , bfm_sc2_state, bfmcgslm   , 0,         "BFM",      "Club Grand Slam (UK, set 2) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-752-056
GAME( 1996, sc2gslamp   , sc2gslam  ,  scorpion2        , bfmcgslm  , bfm_sc2_state, bfmcgslm   , 0,         "BFM",      "Club Grand Slam (UK, set 2, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-753-056

/********************************************************************************************************************************************************************************************************************
 Make A Million
  check sc2mam3 / sc2mam3a, they claim to be the same version
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6553  MAKE A MILLION #4/#8 - 18-SEP-1995 13:54:26
GAME( 199?, sc2mam4     , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-483
GAME( 199?, sc2mam4p    , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-483
// PROJECT NUMBER 6553  MAKE A MILLION ARCADE #4/#8 - 18-SEP-1995 14:51:08
GAME( 199?, sc2mam3     , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-484
GAME( 199?, sc2mam3a    , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 2, UK, alt) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-484
GAME( 199?, sc2mam3p    , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-484
// PROJECT NUMBER 6553  MAKE A MILLION #8 ALL CASH - 19-SEP-1995 12:11:36
GAME( 199?, sc2mam2     , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 3, UK, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-485
GAME( 199?, sc2mam2p    , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 3, UK, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-485
// PROJECT NUMBER 6780  MAKE A MILLION #10 10/25P - 24-MAY-1996 16:15:11
GAME( 199?, sc2mam      , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 4, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-731
GAME( 199?, sc2mamp     , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 4, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-731
// PROJECT NUMBER 6780  MAKE A MILLION #10 10/25P - 24-MAY-1996 16:18:45
GAME( 199?, sc2mam1     , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 5, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-732
GAME( 199?, sc2mam1p    , sc2mam    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million (Bellfruit) (set 5, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-732


/********************************************************************************************************************************************************************************************************************
 Make A Million Club
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6546  CLUB MAKE A MILLION CASHPOT/JACKPOT -  7-AUG-1996 12:35:19
GAME( 199?, sc2mamcl3   , sc2mamcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-849
// PROJECT NUMBER 6546  CLUB MAKE A MILLION CASHPOT/JACKPOT -  8-JAN-1997 16:59:29
GAME( 199?, sc2mamcl2   , sc2mamcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million Club (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-916
GAME( 199?, sc2mamcl2p  , sc2mamcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million Club (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-916
// PROJECT NUMBER 6546  CLUB MAKE A MILLION CPOT/JPOT 20P #250 - 15-JAN-1997 15:03:44
GAME( 199?, sc2mamcl    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million Club (Bellfruit) (set 3, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-914
GAME( 199?, sc2mamclp   , sc2mamcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million Club (Bellfruit) (set 3, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-914
// PROJECT NUMBER 6546  CLUB MAKE A MILLION CASHPOT/JACKPOT 65% - 31-JAN-1997 11:32:37
GAME( 199?, sc2mamcl1   , sc2mamcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million Club (Bellfruit) (set 4, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-915
GAME( 199?, sc2mamcl1p  , sc2mamcl  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Make A Million Club (Bellfruit) (set 4, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-915


/********************************************************************************************************************************************************************************************************************
 Instant Jackpot
  the set with V2 in the header is the earliest one, is there an earlier V1, or was it on earlier hw (SC1?)
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6517  INSTANT JACKPOTS V2 - 22-AUG-1995 14:36:22
GAME( 199?, sc2inst5    , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-463
// PROJECT NUMBER 6517  INSTANT JACKPOTS VAR STAKE/JACKPOT - 19-SEP-1995 10:15:02
GAME( 199?, sc2inst6    , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-473
// PROJECT NUMBER 6517  INSTANT JACKPOTS VAR STAKE/JACKPOT - 16-OCT-1995 15:03:51
GAME( 199?, sc2inst4    , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-521
GAME( 199?, sc2inst4p   , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 3, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-521
// PROJECT NUMBER 6517  INSTANT JACKPOTS VAR STAKE/JPOT ARCADE - 16-OCT-1995 15:31:35
GAME( 199?, sc2inst2    , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 4, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-522
GAME( 199?, sc2inst2p   , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 4, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-522
// PROJECT NUMBER 6517  INSTANT JACKPOTS #8 ALL CASH   - 12-JAN-1996 10:15:03
GAME( 199?, sc2inst3    , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 5, UK, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-605
GAME( 199?, sc2inst3p   , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 5, UK, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-605
// PROJECT NUMBER 6517  INSTANT JACKPOTS DE-REG - 15-MAY-1996 15:29:07
GAME( 199?, sc2inst     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 6, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-715
GAME( 199?, sc2instp    , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 6, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-715
// PROJECT NUMBER 6517  INSTANT JACKPOTS WHITBREAD #10 ALL CASH 20P PLAY - 15-MAY-1996 15:30:35
GAME( 199?, sc2inst1    , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 7, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-716
GAME( 199?, sc2inst1p   , sc2inst   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Instant Jackpot (Bellfruit) (set 7, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-716

/********************************************************************************************************************************************************************************************************************
 Cash On The Nile
  the 9 APR set lacks a properly headered 'PROJECT' string (even the one for the earlier sets is badly formed compared to other games)
********************************************************************************************************************************************************************************************************************/

// PROJECT 6060 CASH ON THE NILE  CASH ON THE NILE 12RM #150 - 12-JAN-1993 12:27:05
GAME( 199?, sc2cnile1   , sc2cnile  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cash On The Nile Club (Bellfruit) (set 1 UK, 150GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-047
// PROJECT 6060 CASH ON THE NILE  CASH ON THE NILE 12RM #150 - 10-FEB-1993 14:42:56
GAME( 199?, sc2cnile    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cash On The Nile Club (Bellfruit) (set 2 UK, 150GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-049
GAME( 199?, sc2cnilep   , sc2cnile  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cash On The Nile Club (Bellfruit) (set 2 UK, 150GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-049
// CASH ON THE NILE 200 POUND JACKPOT  CASH ON THE NILE 12RM #200 J/P - 9-APR-1993 10:33:22
GAME( 199?, sc2cnile2   , sc2cnile  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cash On The Nile Club (Bellfruit) (set 3 UK, 200GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-071
GAME( 199?, sc2cnile2p  , sc2cnile  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cash On The Nile Club (Bellfruit) (set 3 UK, 200GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-071


/********************************************************************************************************************************************************************************************************************
 Football Club
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6707  FOOTBALL CLUB CASHPOT/JACKPOT - 16-DEC-1996 15:24:47
GAME( 199?, sc2foot2    , sc2foot   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Football Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-905
GAME( 199?, sc2foot2p   , sc2foot   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Football Club (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-751-905
// PROJECT NUMBER PR6707  FOOTBALL CLUB CASHPOT/JACKPOT 20P !250 - 20-DEC-1996 12:35:37
GAME( 199?, sc2foot     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Football Club (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-909
GAME( 199?, sc2footp    , sc2foot   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Football Club (Bellfruit) (set 2, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-909
// PROJECT NUMBER PR6707  FOOTBALL CLUB CASHPOT/JACKPOT 5p #100 - 31-JAN-1997 12:24:35
GAME( 199?, sc2foot1    , sc2foot   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Football Club (Bellfruit) (set 3, UK, 100GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-908
GAME( 199?, sc2foot1p   , sc2foot   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Football Club (Bellfruit) (set 3, UK, 100GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-908

/********************************************************************************************************************************************************************************************************************
 Super Star
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6609  SUPER STAR - 15-JUL-1996 13:33:17
GAME( 199?, sc2sstar1   , sc2sstar  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 1, UK, 2nd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-822
GAME( 199?, sc2sstar1p  , sc2sstar  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 1, UK, 2nd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-822
// PROJECT NUMBER 6609  SUPER STAR TRIDENT 3 - 17-JUL-1996 15:00:38
GAME( 199?, sc2sstar    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 2, UK, 3rd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-824
GAME( 199?, sc2sstarp   , sc2sstar  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 2, UK, 3rd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-824
// PROJECT NUMBER 6609  SUPER STAR WHITBREAD - 19-JUL-1996 12:30:57
GAME( 199?, sc2sstar3   , sc2sstar  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 3, UK, 2nd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-826
GAME( 199?, sc2sstar3p  , sc2sstar  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 3, UK, 2nd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-826
// PROJECT NUMBER 6609  SUPER STAR IRISH -  7-AUG-1996 12:39:04
GAME( 199?, sc2sstar2   , sc2sstar  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 4, UK, 2nd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-829
GAME( 199?, sc2sstar2p  , sc2sstar  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Super Star (Bellfruit) (set 4, UK, 2nd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-829

/********************************************************************************************************************************************************************************************************************
 Club Grand Casino
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6547  GRAND CASINO - 5-FEB-1997 09:41:41
GAME( 199?, sc2cgcas    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Club Grand Casino (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-922
GAME( 199?, sc2cgcasp   , sc2cgcas  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Club Grand Casino (Bellfruit) (set 1, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-922
// PROJECT NUMBER PR6547  GRAND CASINO UK - 5-FEB-1997 09:47:11
GAME( 199?, sc2cgcas1   , sc2cgcas  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Club Grand Casino (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-750-923
GAME( 199?, sc2cgcas1p  , sc2cgcas  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Club Grand Casino (Bellfruit) (set 2, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-923

/********************************************************************************************************************************************************************************************************************
 Casino Royale
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6603  CASINO ROYALE (T2 - 5/10/20P) -  7-MAY-1996 17:02:50
GAME( 199?, sc2casr3    , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 1, UK, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-704
GAME( 199?, sc2casr3p   , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 1, UK, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-704
// PROJECT NUMBER 6603  CASINO ROYALE (T2) DE-REG -  5-JUN-1996 15:03:27
GAME( 199?, sc2casr4    , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-743
GAME( 199?, sc2casr4p   , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-743
// PROJECT NUMBER 6603  CASINO ROYALE (T2) WHITBREAD -  5-JUN-1996 15:06:18
GAME( 199?, sc2casr2    , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-744
GAME( 199?, sc2casr2p   , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 3, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-744
// PROJECT NUMBER 6690  CASINO ROYALE (T3) DE-REG - 14-JUN-1996 08:18:20
GAME( 199?, sc2casr1    , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 4, UK, 3rd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-773
GAME( 199?, sc2casr1p   , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 4, UK, 3rd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-773
// PROJECT NUMBER 6690  CASINO ROYALE (T3) DE-REG WHITBREAD - 14-JUN-1996 08:20:40
GAME( 199?, sc2casr     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 5, UK, 10GBP Jackpot, 3rd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-774
GAME( 199?, sc2casrp    , sc2casr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Casino Royale (Bellfruit) (set 5, UK, 10GBP Jackpot, 3rd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-774

/********************************************************************************************************************************************************************************************************************
 Cash Vegas
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6714  CASH VEGAS - 11-NOV-1996 14:12:19
GAME( 199?, sc2cvega    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-890
GAME( 199?, sc2cvega4p  , sc2cvega  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-890
// PROJECT NUMBER 6714  CASH VEGAS WHITBREAD - 11-NOV-1996 14:14:44
GAME( 199?, sc2cvega3   , sc2cvega  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 2, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-891
GAME( 199?, sc2cvega3p  , sc2cvega  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 2, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-891
// PROJECT NUMBER 6714 TOKEN  CASH VEGAS T2 TOKEN - 19-NOV-1996 15:39:02
GAME( 199?, sc2cvega2   , sc2cvega  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 3, UK, 8GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-893
GAME( 199?, sc2cvega2p  , sc2cvega  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 3, UK, 8GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-893
// PROJECT NUMBER 6714  CASH VEGAS HOPPER - 25-NOV-1996 12:09:44
GAME( 199?, sc2cvega1   , sc2cvega  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 4, UK, 10GBP Jackpot, 3rd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-896
GAME( 199?, sc2cvega1p  , sc2cvega  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Cash Vegas (Bellfruit) (set 4, UK, 10GBP Jackpot, 3rd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-896

/********************************************************************************************************************************************************************************************************************
 Surprise Surprize
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6572  SURPRISE SURPRIZE GALA S+P 95 - 4-JAN-1996 10:03:38
GAME( 199?, sc2suprz    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Surprise Surprize (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-594
GAME( 199?, sc2suprzp   , sc2suprz  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Surprise Surprize (Bellfruit) (set 1, UK, Protocol)(Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-594
// PROJECT NUMBER 6572  SURPRISE SURPRIZE SINGLESITE S+P 95 - 4-JAN-1996 10:05:52
GAME( 199?, sc2suprz1   , sc2suprz  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Surprise Surprize (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-593
GAME( 199?, sc2suprz1p  , sc2suprz  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Surprise Surprize (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-593
// PROJECT NUMBER 6139  SURPRISE SURPRIZE SCORPION 2 BINGO #3/#6 - 18-JUN-1993 11:34:01    o
GAME( 199?, sc2suprz3   , sc2suprz  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Surprise Surprize (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-096
// PROJECT NUMBER 6139  SURPRISE SURPRIZE SCORPION 2 #6 ALL CASH 20P - 1-JUL-1996 10:52:24
GAME( 199?, sc2suprz2   , sc2suprz  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Surprise Surprize (Bellfruit) (set 4, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-807
GAME( 199?, sc2suprz2p  , sc2suprz  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Surprise Surprize (Bellfruit) (set 4, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-807

/********************************************************************************************************************************************************************************************************************
 Pharaoh's Gold Club
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6635 PHARAOHS GOLD  PHARAOHS GOLD  250 POUND JACKPOT -  6-AUG-1996 16:55:46
GAME( 199?, sc2cpg2     , sc2cpg    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Pharaoh's Gold Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-840
GAME( 199?, sc2cpg2p    , sc2cpg    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Pharaoh's Gold Club (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-840
// PROJECT NUMBER PR6635 PHARAOHS GOLD  PHARAOHS GOLD 20PP 250 POUND JACKPOT - 30-AUG-1996 08:03:38
GAME( 199?, sc2cpg      , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Pharaoh's Gold Club (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-860
GAME( 199?, sc2cpgp     , sc2cpg    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Pharaoh's Gold Club (Bellfruit) (set 2, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-860
// PROJECT NUMBER PR6635 PHARAOHS GOLD  PHARAOHS GOLD  250 POUND JACKPOT 65% - 19-SEP-1996 15:49:24
GAME( 199?, sc2cpg1     , sc2cpg    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Pharaoh's Gold Club (Bellfruit) (set 3, UK, p65) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-867
GAME( 199?, sc2cpg1p    , sc2cpg    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Pharaoh's Gold Club (Bellfruit) (set 3, UK, p65, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-867

/********************************************************************************************************************************************************************************************************************
 Showtime Spectacular
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6550  SHOWTIME SPECTACULAR S+P 5/10/20P #4/#8 - 14-SEP-1995 15:46:26
GAME( 199?, sc2showt4   , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-478
GAME( 199?, sc2showt4p  , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-478
// PROJECT NUMBER 6550  SHOWTIME SPECTACULAR ARCADE S+P #4/#8 - 14-SEP-1995 15:48:13
GAME( 199?, sc2showt3   , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-479
GAME( 199?, sc2showt3p  , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-479
// PROJECT NUMBER 6550  SHOWTIME SPECTACULAR S+P 5/10/20P IRISH ALL CASH #8 - 14-SEP-1995 16:26:44
GAME( 199?, sc2showt2   , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-480
GAME( 199?, sc2showt2p  , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 3, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-480
// PROJECT NUMBER 6779  SHOWTIME SPECTACULAR S+P 20/25P #10 - 22-MAY-1996 10:30:47
GAME( 199?, sc2showt    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 4, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-729
GAME( 199?, sc2showtp   , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 4, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-729
// PROJECT NUMBER 6779  SHOWTIME SPECTACULAR S+P WHITBREAD 20P #10 - 22-MAY-1996 10:32:59
GAME( 199?, sc2showt1   , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 5, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-730
GAME( 199?, sc2showt1p  , sc2showt  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Showtime Spectacular (Bellfruit) (set 5, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-730

/********************************************************************************************************************************************************************************************************************
 Cat & Mouse
  Project Number
  #6306 is the original version
  #6426 is 'Deluxe'
  #6564 is used for the newer versions aka 'Mark 2'?
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6306  CAT+MOUSE - 4-MAY-1994 17:16:31
GAME( 199?, sc2catms3   , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-237
// PROJECT NUMBER 6306  CAT+MOUSE - 25-AUG-1994 10:08:22
GAME( 199?, sc2ctms2    , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-279
// PROJECT NUMBER 6306  CAT+MOUSE ARCADE - 25-AUG-1994 10:09:28
GAME( 199?, sc2ctms25   , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 3) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-280
// PROJECT NUMBER 6426  CAT+MOUSE DELUXE ARCADE - 15-JUN-1995 15:56:14
GAME( 199?, sc2catms    , 0         ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 4, Deluxe) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-428
// PROJECT NUMBER 6564  CAT+MOUSE #8 - 26-OCT-1995 08:49:39
GAME( 199?, sc2ctms23   , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 5) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-537
GAME( 199?, sc2ctms23p  , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 5, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-537
// PROJECT NUMBER 6564  CAT+MOUSE ARCADE #8 - 26-OCT-1995 08:51:39
GAME( 199?, sc2ctms22   , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 6) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-538
GAME( 199?, sc2ctms22p  , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 6, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-538
// PROJECT NUMBER 6564  CAT+MOUSE #8 ALL CASH  - 26-OCT-1995 10:53:48
GAME( 199?, sc2ctms21   , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 7) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-539
GAME( 199?, sc2ctms21p  , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 7, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-539
// PROJECT NUMBER 6564  CAT+MOUSE ARCADE 10P PLAY #8 - 27-FEB-1996 11:26:40
GAME( 199?, sc2ctms24p  , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 8, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-540
// PROJECT NUMBER 6564  CAT+MOUSE #10 ALL CASH - 26-MAR-1996 08:34:02
GAME( 199?, sc2catms2   , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 9) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-660
GAME( 199?, sc2catms2p  , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 9, Protocol) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-751-660
// PROJECT NUMBER 6564  CAT+MOUSE #10 ALL CASH WHITBREAD - 18-APR-1996 10:51:30
GAME( 199?, sc2catms1   , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 10) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-673
GAME( 199?, sc2catms1p  , sc2catms  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cat & Mouse (Bellfruit) (set 10, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-673


/********************************************************************************************************************************************************************************************************************
 Cat & Mouse & Bonzo Too
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6772  C+M+B TOO #10 DE-REG - 3-JUL-1996 12:51:26
GAME( 199?, sc2cmbt     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cat & Mouse & Bonzo Too (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-809
GAME( 199?, sc2cmbtp    , sc2cmbt   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Cat & Mouse & Bonzo Too (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-809

/********************************************************************************************************************************************************************************************************************
 Easy Money
  the May 2 1996 releases use project code 6608, while the Jun 14 1996 is 6613
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6608  EASY MONEY STANDARD SINGLE SITE #8/#10 ALL CASH -  2-MAY-1996 11:02:39
GAME( 199?, sc2easy     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Easy Money (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-685
GAME( 199?, sc2easyp    , sc2easy   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Easy Money (Bellfruit) (set 1, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-685
// PROJECT NUMBER 6608  EASY MONEY WHITBREAD #10 ALL CASH 20P PLAY -  2-MAY-1996 11:04:21
GAME( 199?, sc2easy2    , sc2easy   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Easy Money (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-686
GAME( 199?, sc2easy2p   , sc2easy   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Easy Money (Bellfruit) (set 2, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-686
// PROJECT NUMBER 6613  EASY MONEY WHITBREAD #10 ALL CASH 20P PLAY - 14-JUN-1996 11:44:11
GAME( 199?, sc2easy1    , sc2easy   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Easy Money (Bellfruit) (set 3) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-776
GAME( 199?, sc2easy1p   , sc2easy   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Easy Money (Bellfruit) (set 3, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-776

/********************************************************************************************************************************************************************************************************************
 Flash Cash
  the Oct 10 1996 releases use project code 6713, while the Oct 23 1996 is 6723
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6713  FLASH CASH TRIDENT2 #8/#10 ALL CASH 5P/10P/20P/25P PLAY - 10-OCT-1996 10:45:44
GAME( 199?, sc2flaca1   , sc2flaca  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Flash Cash (Bellfruit) (set 1, UK, 10GBP Jackpot, 2nd Triennial) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-750-873
GAME( 199?, sc2flaca1p  , sc2flaca  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Flash Cash (Bellfruit) (set 1, UK, 10GBP Jackpot, 2nd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-873
// PROJECT NUMBER 6713  FLASH CASH TRIDENT2 WHITBREAD #10 ALL CASH 20P PLAY - 10-OCT-1996 11:14:42
GAME( 199?, sc2flaca2   , sc2flaca  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Flash Cash (Bellfruit) (set 2, UK, 10GBP Jackpot, 2nd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-874
GAME( 199?, sc2flaca2p  , sc2flaca  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Flash Cash (Bellfruit) (set 2, UK, 10GBP Jackpot, 2nd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-874
// PROJECT NUMBER 6723  FLASH CASH TRIDENT3 #8/#10 ALL CASH 5P/10P/20P/25P PLAY - 23-OCT-1996 12:13:03
GAME( 199?, sc2flaca    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Flash Cash (Bellfruit) (set 3, UK, 10GBP Jackpot, 3rd Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-877
GAME( 199?, sc2flacap   , sc2flaca  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Flash Cash (Bellfruit) (set 3, UK, 10GBP Jackpot, 3rd Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-877

/********************************************************************************************************************************************************************************************************************
 Cashino Club
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6232  CASHINO - 22-JUL-1994 12:10:28
GAME( 199?, sc2cshcl1   , sc2cshcl  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cashino Club (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-254
GAME( 199?, sc2cshcl1p  , sc2cshcl  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cashino Club (Bellfruit) (set 1, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-254
// PROJECT NUMBER 6232  CASHINO FIXED 65% - 22-JUL-1994 12:19:27
GAME( 199?, sc2cshcl    , 0         ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cashino Club (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-255
GAME( 199?, sc2cshclp   , sc2cshcl  ,  scorpion2        , bbrkfst   , bfm_sc2_state, bbrkfst    , 0,         "BFM",      "Cashino Club (Bellfruit) (set 2, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-255

/********************************************************************************************************************************************************************************************************************
 Eggs On Legs Tour
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6604  EGGS ON LEGS !10 ALL CASH - 11-JUN-1996 08:54:37
GAME( 199?, sc2eggs1    , sc2eggs   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Eggs On Legs Tour (Bellfruit) (set 1, UK, Arcade, 10GBP Jackpot?) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-746
GAME( 199?, sc2eggs1p   , sc2eggs   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Eggs On Legs Tour (Bellfruit) (set 1, UK, Arcade, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-746
// PROJECT NUMBER 6604  EGGS ON LEGS !10 WHITBREAD - 11-JUN-1996 08:59:45
GAME( 199?, sc2eggs     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Eggs On Legs Tour (Bellfruit) (set 2, UK, Arcade, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-747
GAME( 199?, sc2eggsp    , sc2eggs   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Eggs On Legs Tour (Bellfruit) (set 2, UK, Arcade, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-747

/********************************************************************************************************************************************************************************************************************
 Wild West Club
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6433  WILD  WEST FAST POT FILL - 11-DEC-1995 17:27:48
GAME( 199?, sc2wwcl1    , sc2wwcl   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Wild West Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-582
GAME( 199?, sc2wwcl1p   , sc2wwcl   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Wild West Club (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-582
// PROJECT NUMBER PR6433  WILD  WEST -  7-AUG-1996 16:44:24
GAME( 199?, sc2wwcl     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Wild West Club (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-845
GAME( 199?, sc2wwclp    , sc2wwcl   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Wild West Club (Bellfruit) (set 2, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-845

/********************************************************************************************************************************************************************************************************************
 Double Diamond
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6570  DOUBLE DIAMOND STAKES AND PRIZES #8 - 3-JAN-1996 12:21:14
GAME( 199?, sc2dbl1     , sc2dbl    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Double Diamond (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-599
GAME( 199?, sc2dbl1p    , sc2dbl    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Double Diamond (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-599
// PROJECT NUMBER 6322  DOUBLE DIAMOND GALA 82% STAKES AND PRIZES - 3-JAN-1996 12:22:52
GAME( 199?, sc2dbl      , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Double Diamond (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-600
GAME( 199?, sc2dblp     , sc2dbl    ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Double Diamond (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-600

/********************************************************************************************************************************************************************************************************************
 Hyperactive
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6784  HYPERACTIVE !10 ALL CASH - 18-JUN-1996 12:10:31
GAME( 199?, sc2hypr     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Hyperactive (Bellfruit) (set 1, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-748
GAME( 199?, sc2hyprp    , sc2hypr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Hyperactive (Bellfruit) (set 1, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-748
// PROJECT NUMBER 6784  HYPERACTIVE WHITBREAD !10 ALL CASH - 18-JUN-1996 12:12:26
GAME( 199?, sc2hypr1    , sc2hypr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Hyperactive (Bellfruit) (set 2, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-749
GAME( 199?, sc2hypr1p   , sc2hypr   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Hyperactive (Bellfruit) (set 2, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-749

/********************************************************************************************************************************************************************************************************************
 King Cash Club
  the dual stake version is PR6184, the regular is PR6034
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6184  KING CASH 200 POUND JACKPOT DUAL STAKE - 26-NOV-1993 11:38:06
GAME( 199?, sc2kcclb1   , sc2kcclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "King Cash Club (Bellfruit) (set 2, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-168
GAME( 199?, sc2kcclb1p  , sc2kcclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "King Cash Club (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-168
// PROJECT NUMBER PR6034  KING CASH 200 POUND JACKPOT - 26-NOV-1993 11:40:08
GAME( 199?, sc2kcclb    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "King Cash Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-166
GAME( 199?, sc2kcclbp   , sc2kcclb  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "King Cash Club (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-166

/********************************************************************************************************************************************************************************************************************
 Olympic Gold
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6606  OLYMPIC GOLD !10 ALL CASH - 30-APR-1996 15:42:35
GAME( 199?, sc2olgld    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Olympic Gold (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-689
GAME( 199?, sc2olgldp   , sc2olgld  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Olympic Gold (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-689
// PROJECT NUMBER 6606  OLYMPIC GOLD !10 WHITBREAD - 30-APR-1996 15:44:58
GAME( 199?, sc2olgld1   , sc2olgld  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Olympic Gold (Bellfruit) (set 2, UK, 10GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-690
GAME( 199?, sc2olgld1p  , sc2olgld  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Olympic Gold (Bellfruit) (set 2, UK, 10GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-690

/********************************************************************************************************************************************************************************************************************
 Reel Gems
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6321  REEL GEMS STANDARD STAKES AND PRIZES #4/#8 - 29-JAN-1996 11:58:16
GAME( 199?, sc2relgm    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Reel Gems (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-612
GAME( 199?, sc2relgmp   , sc2relgm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Reel Gems (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-612
// PROJECT NUMBER 6321  REEL GEMS GALA 82% FIXED S+P #4/#8 - 29-JAN-1996 11:59:54
GAME( 199?, sc2relgm1p  , sc2relgm  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Reel Gems (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-613

/********************************************************************************************************************************************************************************************************************
 Top Wack
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6610   TOP WHACK 10PD 20P WHITBREAD T2 - 19-JUN-1996 16:02:20
GAME( 199?, sc2topwk    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Top Wack (Bellfruit) (set 1, UK, 10GBP Jackpot, 1st Triennial) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-791
GAME( 199?, sc2topwkp   , sc2topwk  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Top Wack (Bellfruit) (set 1, UK, 10GBP Jackpot, 1st Triennial, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-791

/********************************************************************************************************************************************************************************************************************
 Golden Roulette
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6421  GOLDEN ROULETTE S+P VARIABLE STAKE/PAYOUT -  4-OCT-1995 10:46:40
GAME( 199?, sc2groul    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Golden Roulette (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-507
GAME( 199?, sc2groulp   , sc2groul  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Golden Roulette (Bellfruit) (set 2, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-507

/********************************************************************************************************************************************************************************************************************
 Hey Presto
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6138  HEY PRESTO % VARIABLE - ALL CASH - SCORPION 2 -  9-JUL-1996 17:03:26
GAME( 199?, sc2heypr    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Hey Presto (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-812
GAME( 199?, sc2heyprp   , sc2heypr  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Hey Presto (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-812

/********************************************************************************************************************************************************************************************************************
 Majestic Bells
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6324  MAJESTIC BELLS S+P - 16-NOV-1995 15:37:58
GAME( 199?, sc2majes    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Majestic Bells (Bellfruit) (set 1) (set 1)", GAME_FLAGS) // GAME No 95-750-563
GAME( 199?, sc2majesp   , sc2majes  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Majestic Bells (Bellfruit) (set 1, Protocol) (set 2)", GAME_FLAGS) // GAME No 95-751-563

/********************************************************************************************************************************************************************************************************************
 Pay Roll Casino
  we only have a Mazooma set, was there a regular BFM one?
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 7015  PAYROLL CASINO AT MAZOOMA 5P - 14-MAY-1998 10:20:02
GAME( 199?, sc2payr     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM/Mazooma",   "Pay Roll Casino (Bellfruit/Mazooma) (Scorpion 2/3)", GAME_FLAGS) // GAME No PRCVM52

/********************************************************************************************************************************************************************************************************************
 Carrot Gold Club
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6833  250 CARROT GOLD - 22-MAY-1997 09:04:15
GAME( 199?, sc2cgc      , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",      "Carrot Gold Club (Bellfruit) (Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-968

/********************************************************************************************************************************************************************************************************************
 The Great Train Robbery
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6205  THE GREAT TRAIN ROBBERY 15RM - 20-AUG-1993 11:53:20
GAME( 199?, sc2gtr      , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "The Great Train Robbery (Bellfruit) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-137

/********************************************************************************************************************************************************************************************************************
 Main Attraction
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6203  MAIN ATTRACTION - 12-JUL-1993 17:45:15
GAME( 199?, sc2maina    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "BFM",   "Main Attraction (Bellfruit) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-115

/********************************************************************************************************************************************************************************************************************
 Focus (Dutch)
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6346  FOCUS   - 23-JAN-1995 15:31:32
GAME( 1995, sc2focus    , 0         ,  scorpion3        , scorpion3 , bfm_sc2_state, focus      , 0,         "BFM/ELAM", "Focus (Dutch, Game Card 95-750-347) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-750-347

/********************************************************************************************************************************************************************************************************************
 Public Enemy No.1 (German)
  this one is a bit strange (not encrypted, gives 'PROM ERROR 2'), is it really sc2? BFMemulator layout dat says it is
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6489  PUBLIC ENEMY No1 - 22-MAY-1995 09:24:05
GAME( 199?, sc2pe1g     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwhon , 0,         "BFM",      "Public Enemy No.1 (Bellfruit) [German] (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-415


/********************************************************************************************************************************************************************************************************************
 Winning Streak
********************************************************************************************************************************************************************************************************************/

// taken from the sc1 set, might be wrong here
#define sc2_winst_sound \
	ROM_REGION( 0x80000, "upd", 0 )\
	ROM_LOAD( "winningstreaksnd.bin", 0x0000, 0x080000, CRC(ba30cb97) SHA1(e7f5ca36ca993ad14b3a348868e73d7ba02be7c5) )
ROM_START( sc2winstb )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_std_ac_var_8-10pnd_ass.bin",       0x00000, 0x10000, CRC(f2d16bd5) SHA1(bd6a9da9da24459b14917386c64ecbc46c8adfda) ) sc2_winst_sound ROM_END
ROM_START( sc2winstbp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_dat_ac_var_8-10pnd_ass.bin",       0x00000, 0x10000, CRC(351560f4) SHA1(b33c6bdeadeabbe5a4231b8bd5b134f9ea402133) ) sc2_winst_sound ROM_END
ROM_START( sc2winst )   ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_std_ar_var_8pnd_ass.bin",          0x00000, 0x10000, CRC(d7a10aeb) SHA1(7346c83df7fd3de57a1b6f0ce498daabacb11491) ) sc2_winst_sound ROM_END
ROM_START( sc2winstp )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_dat_ar_var_8pnd_ass.bin",          0x00000, 0x10000, CRC(a83633ef) SHA1(66caadd3127a424249fe78918ff99be833b81fad) ) sc2_winst_sound ROM_END
ROM_START( sc2winstd )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_std_ss_var_ass.bin",               0x00000, 0x10000, CRC(c88f9a6e) SHA1(19a2b708f90a53a8dcfe69d2f6c683362867daba) ) sc2_winst_sound ROM_END
ROM_START( sc2winstdp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_dat_ss_var_ass.bin",               0x00000, 0x10000, CRC(311550dd) SHA1(17dc789cba542e7c3c137a7e6a2a2d8869c84a7a) ) sc2_winst_sound ROM_END
ROM_START( sc2winste )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_std_wi_ac_10pnd-20p_ass.bin",      0x00000, 0x10000, CRC(ecbb7707) SHA1(ea064149c515e39b17e851bcd39092ea3ae999a0) ) sc2_winst_sound ROM_END
ROM_START( sc2winstep ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_dat_wi_ac_10pnd-20p_ass.bin",      0x00000, 0x10000, CRC(ae418733) SHA1(f63c63232056929760742fcf7f8beda387f5c597) ) sc2_winst_sound ROM_END
ROM_START( sc2winstf )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_std_wi_ac_10pnd-20p_tri3_ass.bin", 0x00000, 0x10000, CRC(eb9ee9ae) SHA1(3150aec95039aa65a9126a0326e4dd10829347b2) ) sc2_winst_sound ROM_END
ROM_START( sc2winstfp ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_dat_wi_ac_10pnd-20p_tri3_ass.bin", 0x00000, 0x10000, CRC(39ac4021) SHA1(bd5f4d8800a794fdca8abee15acc3ea8d30c538a) ) sc2_winst_sound ROM_END
ROM_START( sc2winstg )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "winning-streak_std_ac_tri3_ass.bin",              0x00000, 0x10000, CRC(b3e2b2d6) SHA1(0008e9d329327b4aecae5d861303c486942ef694) ) sc2_winst_sound ROM_END

// PROJECT NUMBER PR6605  WINNING STREAK T2,ARCADE,#8,5/10/20 - 8-MAY-1996 10:31:16
GAME( 198?, sc2winst        , 0         , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 1) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-702
GAME( 198?, sc2winstp       , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 1, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-702
// PROJECT NUMBER PR6605  WINNING STREAK T2,8PD TOK,5/10/20 - 8-MAY-1996 10:36:21
GAME( 198?, sc2winstd       , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 2) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-700
GAME( 198?, sc2winstdp      , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 2, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-700
// PROJECT NUMBER PR6605  WINNING STREAK T2,#8/#10 CASH 5/10/20/25P - 13-JUN-1996 12:27:29
GAME( 198?, sc2winstb       , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 3) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-766
GAME( 198?, sc2winstbp      , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 3, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-766
// PROJECT NUMBER PR6605  WINNING STREAK T2,WHITBREAD,#10 CASH,20p - 13-JUN-1996 12:30:20
GAME( 198?, sc2winste       , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 4) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-767
GAME( 198?, sc2winstep      , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 4, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-767
// PROJECT NUMBER PR6691  WINNING STREAK T3 HOPPERS #8/10 5-25P - 13-JUN-1996 16:28:02
GAME( 198?, sc2winstg       , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 5) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-777
// PROJECT NUMBER PR6691  WINNING STREAK T3,WHITBREAD,#10 CASH,20p - 13-JUN-1996 16:31:36
GAME( 198?, sc2winstf       , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 6) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-778
GAME( 198?, sc2winstfp      , sc2winst  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Winning Streak (Bellfruit) (set 6, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-778


/********************************************************************************************************************************************************************************************************************
 Cash Explosion
********************************************************************************************************************************************************************************************************************/

#define sc2_cexpl_sound \
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
ROM_START( sc2cexpl )  ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "cash_explosion_dat_ac_8_10pnd_20p_a.bin",  0x0000, 0x010000, CRC(1d155799) SHA1(4e76328a4d093d1f9c64c633c3558db2dce4e219) ) sc2_cexpl_sound ROM_END
ROM_START( sc2cexpla ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "cash_explosion_dat_ac_var_8pnd_a.bin",     0x0000, 0x010000, CRC(4aa53121) SHA1(cf0510e224de62b837915d39c2fe3559cfe8c85f) ) sc2_cexpl_sound ROM_END
ROM_START( sc2cexplb ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "cash_explosion_dat_wi_ac_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(889eb206) SHA1(91b23a2cc475e68470d01976b88b9ea7aa0afed9) ) sc2_cexpl_sound ROM_END
ROM_START( sc2cexplc ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "cash_explosion_std_ac_8_10pnd_20p_a.bin",  0x0000, 0x010000, CRC(de6bbee2) SHA1(3c321fa442b25a27c3f14b7ac94255f020056663) ) sc2_cexpl_sound ROM_END
ROM_START( sc2cexpld ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "cash_explosion_std_ac_var_8pnd_a.bin",     0x0000, 0x010000, CRC(e8a21401) SHA1(479edd734ca949e344fb7e17ed7af7c8c9604efc) ) sc2_cexpl_sound ROM_END
ROM_START( sc2cexple ) ROM_REGION( 0x10000, "maincpu", 0 ) ROM_LOAD( "cash_explosion_std_wi_ac_10pnd_20p_a.bin", 0x0000, 0x010000, CRC(2901a315) SHA1(c9733488894ccead7a69b161f2afacdb3f892b89) ) sc2_cexpl_sound ROM_END

// PROJECT NUMBER 6518  CASH EXPLOSION 5/10/20P #8 IRISH ALL CASH - 12-JAN-1996 10:12:16
GAME( 198?, sc2cexpld       , sc2cexpl  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 1) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-606
GAME( 198?, sc2cexpla       , sc2cexpl  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 1, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-606
// PROJECT NUMBER 6776  CASH EXPLOSION DE-REG - 21-MAY-1996 12:38:53
GAME( 198?, sc2cexplc       , sc2cexpl  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 2) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-723
GAME( 198?, sc2cexpl        , 0         , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 2, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-723
// PROJECT NUMBER 6776  CASH EXPLOSION DE-REG WHITBREAD - 21-MAY-1996 12:40:20
GAME( 198?, sc2cexple       , sc2cexpl  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 3) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-750-728
GAME( 198?, sc2cexplb       , sc2cexpl  , scorpion2         , drwho , bfm_sc2_state, drwho          , 0,       "BFM",      "Cash Explosion (Bellfruit) (set 3, Protocol) (Scorpion 2)", GAME_FLAGS ) // GAME No 95-751-728

/********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************

 Games with Dot Matrix Displays
  (DMD<->set rom pairings still need to be checked for all)


*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
********************************************************************************************************************************************************************************************************************/


/********************************************************************************************************************************************************************************************************************
 Luvvly Jubbly
  are there earlier versions with game code 6224?
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6569 (6224)  LUVVLY JUBBLY GALA S+P 95 - 30-JAN-1996 11:12:00
GAME( 1996, sc2luvv6p   , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 1, UK, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-621
// PROJECT NUMBER 6569 (6224)  LUVVLY JUBBLY MULTISITE S+P 95 - 30-JAN-1996 11:14:05
GAME( 1996, sc2luvv2    , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 2, UK, Multisite) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-622
GAME( 1996, sc2luvv2p   , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 2, UK, Multisite, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-622
// PROJECT NUMBER 6569 (6224)  LUVVLY JUBBLY MULTISITE #10/25P - 2-JUL-1996 16:32:17
GAME( 1996, sc2luvv     , 0         ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 3, UK, Multisite 10GBP/25p) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-808
GAME( 1996, sc2luvvp    , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 3, UK, Multisite 10GBP/25p, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-808
// PROJECT NUMBER 6569 (6224)  LUVVLY JUBBLY MULTISITE #4/5P PLAY - 4-JUL-1996 10:56:24
GAME( 1996, sc2luvv4    , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 4, UK, Multisite 4GBP/5p) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-810
GAME( 1996, sc2luvv4p   , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 4, UK, Multisite 4GBP/5p, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-810
// PROJECT NUMBER 6569 (6224)  LUVVLY JUBBLY MULTISITE #10/20P - 12-SEP-1996 14:07:57
GAME( 1996, sc2luvv1    , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 3, UK, Multisite 10GBP/20p) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-866
GAME( 1996, sc2luvv1p   , sc2luvv   ,  scorpion2_dm01   , luvjub    , bfm_sc2_state, luvjub , 0,         "BFM",      "Luvvly Jubbly (set 3, UK, Multisite 10GBP/20p, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-866


/********************************************************************************************************************************************************************************************************************
 Club Public Enemy No.1
  game code changes from 6331 to 6574 for the 1996 release?
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6331  PUBLIC ENEMY NO.1 - 26-JUL-1994 09:24:19
GAME( 1996, sc2cpe3     , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 1, UK) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-257
GAME( 1996, sc2cpe3p    , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 1, UK, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-257
// PROJECT NUMBER PR6331  PUBLIC ENEMY NO.1 FIXED 65% - 10-AUG-1994 11:26:30
GAME( 1996, sc2cpe4     , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 2, UK) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-273
GAME( 1996, sc2cpe4p    , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 2, UK, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-273
// PROJECT NUMBER PR6574  PUBLIC ENEMY NO.1 S+P 25P/#250 STENA SEALINK - 3-JAN-1996 12:17:33
GAME( 1996, sc2cpe2     , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 3, UK) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-597
GAME( 1996, sc2cpe2p    , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 3, UK, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-597
// PROJECT NUMBER PR6574  PUBLIC ENEMY NO.1 S+P 25P/#250 FIXED 65% - 3-JAN-1996 12:19:01
GAME( 1996, sc2cpe1     , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 4, UK) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-598
GAME( 1996, sc2cpe1p    , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 4, UK, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-598
// PROJECT NUMBER PR6574  PUBLIC ENEMY NO.1 S+P 25P/#250 - 20-AUG-1996 10:05:21
GAME( 1996, sc2cpe      , 0         ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 5, UK) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)           // GAME No 95-750-846
GAME( 1996, sc2cpep     , sc2cpe    ,  scorpion2_dm01_5m   , cpeno1    , bfm_sc2_state, cpeno1 , 0,         "BFM",      "Club Public Enemy No.1 (set 5, UK, Protocol) (Scorpion 2/3)", MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL) // GAME No 95-751-846

/********************************************************************************************************************************************************************************************************************
 Cops 'n' Robbers
  game code changes from 6012 to 6589 for the 1995 release?
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6012  COPS & ROBBERS #3/#6 - 29-DEC-1992 21:26:28
GAME( 199?, sc2cops5    , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-044
// PROJECT NUMBER 6012  COPS AND ROBBERS S+P 10P - 7-JUL-1993 10:17:18
GAME( 199?, sc2cops2    , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-110
// PROJECT NUMBER 6589 (6012)  COPS & ROBBERS (SINGLE SITE 5P/10P/20P) - 4-DEC-1995 10:52:08
GAME( 199?, sc2cops3    , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 3) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-577
GAME( 199?, sc2cops3p   , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 3, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-577
// PROJECT NUMBER 6589 (6012)  COPS & ROBBERS (ARCADE 5P/10P/20P) - 4-DEC-1995 10:53:58
GAME( 199?, sc2copsp    , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 4, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-578
// PROJECT NUMBER 6589 (6012)  COPS & ROBBERS (IRISH ALL CASH 5P/10P/20P) - 7-MAR-1996 15:07:40
GAME( 199?, sc2cops4    , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 5) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-645
// PROJECT NUMBER 6589 (6012)  COPS & ROBBERS (#10 ALL CASH 20P/25P) - 15-MAR-1996 11:52:02
GAME( 199?, sc2cops     , 0         ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 6) (Scorpion 2/3)", GAME_FLAGS)             // GAME No 95-750-652
GAME( 199?, sc2cops1p   , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers (Bellfruit) (set 6, Protocol) (Scorpion 2/3)", GAME_FLAGS)  // GAME No 95-751-652


/********************************************************************************************************************************************************************************************************************
  Casino Cops 'n' Robbers
   note, sc2copsc1p & sc2copsc1pa are the same set with a different matrix rom
         I don't think the matrix roms being used a correct, everything except sc2copsc1pa boots up with 'Nudge Now'
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6622  BINGO COPS N ROBBERS #8/#10 ALL CASH - 9-JUL-1996 17:08:15
GAME( 199?, sc2copsc    , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Casino Cops 'n' Robbers (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS)            // GAME No 95-750-814
GAME( 199?, sc2copscp   , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Casino Cops 'n' Robbers (Bellfruit) (set 1, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-814
// PROJECT NUMBER 6622  BINGO COPS N ROBBERS SWITCHABLE BINGO/ARCADE - 9-JUL-1996 17:12:33
GAME( 199?, sc2copsc1   , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Casino Cops 'n' Robbers (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS)                            // GAME No 95-750-816
GAME( 199?, sc2copsc1p  , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Casino Cops 'n' Robbers (Bellfruit) (set 2, Protocol) (Scorpion 2/3)", GAME_FLAGS)                  // GAME No 95-751-816
GAME( 199?, sc2copsc1pa , sc2cops   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Casino Cops 'n' Robbers (Bellfruit) (set 2, Protocol) (Scorpion 2/3) (alt matrix rom)", GAME_FLAGS) // GAME No 95-751-816


/********************************************************************************************************************************************************************************************************************
  Cops 'n' Robbers Club Deluxe
   game code changes from 6332 to 6588 between the 2 17-NOV-1995 versions?
********************************************************************************************************************************************************************************************************************/

// PROJECT PR6332  CLUB COPS AND ROBBERS DELUXE - 17-NOV-1995 12:36:04
GAME( 199?, sc2copdc5   , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 1, UK, 200GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-561
GAME( 199?, sc2copdc5p  , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 1, UK, 200GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-561
// PROJECT PR6588  CLUB COPS AND ROBBERS DELUXE 25P/#250 - 17-NOV-1995 13:18:57
GAME( 199?, sc2copdc6   , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-562
// PROJECT PR6588  CLUB COPS AND ROBBERS DELUXE 25P/#250 65% - 30-NOV-1995 16:14:25
GAME( 199?, sc2copdc1   , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 3, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-574
GAME( 199?, sc2copdc1p  , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 3, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-574
// PROJECT NUMBER 6588  CLUB COPS AND ROBBERS DELUXE 20P/#250 - 25-MAR-1996 13:57:23
GAME( 199?, sc2copdc2   , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 4, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-658
GAME( 199?, sc2copdc2p  , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 4, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-658
// PROJECT PR6588  CLUB COPS AND ROBBERS DELUXE 25P/#250 - 13-AUG-1996 14:01:25
GAME( 199?, sc2copdc3   , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 5, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-842
GAME( 199?, sc2copdc3p  , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 5, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-842
// PROJECT PR6588  CLUB COPS AND ROBBERS DELUXE 25P/#250 63% - 25-SEP-1997 08:30:05
GAME( 199?, sc2copdc4   , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 6, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-993
GAME( 199?, sc2copdc4p  , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 6, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-993
// PROJECT PR6588  CLUB COPS AND ROBBERS DELUXE 25P/#250 67% - 25-SEP-1997 08:33:14
GAME( 199?, sc2copdc    , 0         ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 7, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-992
GAME( 199?, sc2copdcp   , sc2copdc  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Cops 'n' Robbers Club Deluxe (Bellfruit) (set 7, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-992


/********************************************************************************************************************************************************************************************************************
  Round The Town
   game code changes from 6201 to 6620 between the 1995 release?
   check sc2town1 / sc2town1a they claim to be the same version, sc2town1a was incorrectly in a road to wembley set
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6201  ROUND THE TOWN - 4-MAR-1993 11:05:07
GAME( 199?, sc2town4    , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-052
// PROJECT NUMBER 6201  ROUND THE TOWN IRISH ALL CASH - 1-APR-1993 14:44:50
GAME( 199?, sc2town2    , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-069
// PROJECT NUMBER 6620 (6201)  ROUND THE TOWN S&P - 15-DEC-1995 14:50:50
GAME( 199?, sc2town3    , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 3) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-591
GAME( 199?, sc2town3p   , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 3, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-591
// PROJECT NUMBER 6620 (6201)  ROUND THE TOWN (ARCADE/HIGH TOKEN) - 18-DEC-1995 15:59:22
GAME( 199?, sc2town1    , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 4) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-592
GAME( 199?, sc2town1a   , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 4, alt) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-750-592
GAME( 199?, sc2town1p   , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 4, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-592
// PROJECT NUMBER 6620 (6201)  ROUND THE TOWN S&P IRISH AC - 5-MAR-1996 12:05:06
GAME( 199?, sc2town5    , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 5) (Scorpion 2/3)", GAME_FLAGS) //  GAME No 95-750-642
// PROJECT NUMBER 6620 (6201)  ROUND THE TOWN #10 AC - 15-MAR-1996 12:07:18
GAME( 199?, sc2town     , 0         ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 6) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-654
GAME( 199?, sc2townp    , sc2town   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Round The Town (Bellfruit) (set 6, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-654

// PROJECT NUMBER 6620 (6201)  ROUND THE TOWN (ARCADE/HIGH TOKEN) - 18-DEC-1995 15:59:22

/********************************************************************************************************************************************************************************************************************
  Only Fools & Horses
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6207  ONLY FOOLS AND HORSES - 14-OCT-1993 15:28:16
GAME( 199?, sc2ofool1   , sc2ofool  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Only Fools & Horses (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-147
// PROJECT NUMBER 6207  ONLY FOOLS AND HORSES ALL CASH - 29-OCT-1993 13:00:02
GAME( 199?, sc2ofool3   , sc2ofool  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Only Fools & Horses (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-151
// PROJECT NUMBER 6207  ONLY FOOLS AND HORSES - 18-NOV-1993 14:32:21
GAME( 199?, sc2ofool    , 0         ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Only Fools & Horses (Bellfruit) (set 3) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-159
// PROJECT NUMBER 6207  ONLY FOOLS AND HORSES ALL CASH - 18-NOV-1993 16:12:34
GAME( 199?, sc2ofool2   , sc2ofool  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Only Fools & Horses (Bellfruit) (set 4) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-162
// PROJECT NUMBER 6207  ONLY FOOLS AND HORSES ARCADE 10P PLAY - 14-DEC-1993 14:51:34
GAME( 199?, sc2ofool4   , sc2ofool  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Only Fools & Horses (Bellfruit) (set 5) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-177


/********************************************************************************************************************************************************************************************************************
  Party Time
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6221  PARTY TIME BINGO SCORPION 2 - 10-JUN-1993 14:26:26
GAME( 199?, sc2ptytm1   , sc2ptytm  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Party Time (Bellfruit) (set 1) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-092
// PROJECT NUMBER 6221  PARTY TIME BINGO SCORPION 2 #4 ALL CASH 10P PLAY - 1-JUL-1996 12:02:22
GAME( 199?, sc2ptytm    , 0         ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Party Time (Bellfruit) (set 2) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-806
GAME( 199?, sc2ptytmp   , sc2ptytm  ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, ofah       , 0,         "BFM",      "Party Time (Bellfruit) (set 2, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-806


/********************************************************************************************************************************************************************************************************************
  Along The Prom
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6172  ALONG THE PROM  SINGLE SITE - 30-MAR-1993 12:03:27
GAME( 199?, sc2prom     , 0         ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, prom       , 0,         "BFM",      "Along The Prom (Bellfruit) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-062


/********************************************************************************************************************************************************************************************************************
  Premier Club Manager
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER PR6432  PREMIER CLUB MANAGER - 26-JAN-1996 11:52:43
GAME( 199?, sc2prem2    , sc2prem   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, prom       , 0,         "BFM",      "Premier Club Manager (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-616
// PROJECT NUMBER PR6432  PREMIER CLUB MANAGER 25P !250 - 13-AUG-1996 14:05:05
GAME( 199?, sc2prem     , 0         ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, prom       , 0,         "BFM",      "Premier Club Manager (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-848
// PROJECT NUMBER PR6432  PREMIER CLUB MANAGER - 20-AUG-1996 10:06:44
GAME( 199?, sc2prem1    , sc2prem   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, prom       , 0,         "BFM",      "Premier Club Manager (Bellfruit) (set 3, UK) (Scorpion 2/3)", GAME_FLAGS)           // GAME No 95-750-847
GAME( 199?, sc2prem1p   , sc2prem   ,  scorpion2_dm01_3m   , drwho     , bfm_sc2_state, prom       , 0,         "BFM",      "Premier Club Manager (Bellfruit) (set 3, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-847

/********************************************************************************************************************************************************************************************************************
  Golden Casino Club
   also there is a (bad?) dump of a sound rom too, maybe it can be repaired, it differs from the one we're using
********************************************************************************************************************************************************************************************************************/

// PROJECT NUMBER 6632  GOLDEN CASINO VAR STAKE/JACKPOT - 12-FEB-1997 15:54:10
GAME( 199?, sc2gcclb2   , sc2gcclb  ,  scorpion2_dm01_3m       , drwho     , bfm_sc2_state, prom       , 0,         "BFM",   "Golden Casino Club (Bellfruit) (set 1, UK) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-927
GAME( 199?, sc2gcclb2p  , sc2gcclb  ,  scorpion2_dm01_3m       , drwho     , bfm_sc2_state, prom       , 0,         "BFM",   "Golden Casino Club (Bellfruit) (set 1, UK, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-927
// PROJECT NUMBER 6632  GOLDEN CASINO 20P STAKE/#250 JACKPOT - 12-FEB-1997 15:56:05
GAME( 199?, sc2gcclb    , 0         ,  scorpion2_dm01_3m       , drwho     , bfm_sc2_state, prom       , 0,         "BFM",   "Golden Casino Club (Bellfruit) (set 2, UK, 250GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-928
GAME( 199?, sc2gcclbp   , sc2gcclb  ,  scorpion2_dm01_3m       , drwho     , bfm_sc2_state, prom       , 0,         "BFM",   "Golden Casino Club (Bellfruit) (set 2, UK, 250GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-928
// PROJECT NUMBER 6632  GOLDEN CASINO 5P STAKE/#100 JACKPOT - 12-FEB-1997 15:57:23
GAME( 199?, sc2gcclb1   , sc2gcclb  ,  scorpion2_dm01_3m       , drwho     , bfm_sc2_state, prom       , 0,         "BFM",   "Golden Casino Club (Bellfruit) (set 3, UK, 100GBP Jackpot) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-750-929
GAME( 199?, sc2gcclb1p  , sc2gcclb  ,  scorpion2_dm01_3m       , drwho     , bfm_sc2_state, prom       , 0,         "BFM",   "Golden Casino Club (Bellfruit) (set 3, UK, 100GBP Jackpot, Protocol) (Scorpion 2/3)", GAME_FLAGS) // GAME No 95-751-929


/********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************

 3rd party games below
  anything that seems to be on SC2 hardware, but lacks the BFM header
  some might run on modified hardware, eg. different sound chips, different reels, different alpha parts


*********************************************************************************************************************************************************************************************************************
*********************************************************************************************************************************************************************************************************************
********************************************************************************************************************************************************************************************************************/


// these need inverted service door, and seem to have some issues with the reels jumping between 2 values?
GAME( 199?, sc2goldr    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "Gold Reserve (Mdm) (v1.3) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2goldrp   , sc2goldr  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "Gold Reserve (Mdm) (v1.3 Protocol) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2goldr1   , sc2goldr  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "Gold Reserve (Mdm) (set 2) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2hifly    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "High Flyer (Mdm) (v4.1) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2hifly2   , sc2hifly  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "High Flyer (Mdm) (v3.1) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2hifly3   , sc2hifly  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "High Flyer (Mdm) (v2.1) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2hifly4   , sc2hifly  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "High Flyer (Mdm) (v?.?) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2scc      , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Mdm",      "Safe Cracker Club (Mdm) (v4.4) (Scorpion 2/3)", GAME_FLAGS) // also marked as 'GLOBAL'?

// custom Global sound system?
GAME( 199?, sc2dick     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Spotted Dick (Global) (v3.1) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dickp    , sc2dick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Spotted Dick (Global) (v3.1 Protocol) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dick1    , sc2dick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Spotted Dick (Global) (v2.2) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dick2    , sc2dick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Spotted Dick (Global) (v1.5) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dick2p   , sc2dick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Spotted Dick (Global) (v1.5 Protocol ) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dick2e   , sc2dick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Spotted Dick (Global) (v?.? Euro) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2dick2eu  , sc2dick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Spotted Dick (Global) (v?.? Euro unencrypted) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2pick     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Pick Of The Bunch (Global) (v2.3) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2pickp    , sc2pick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Pick Of The Bunch (Global) (v2.3 Protocol) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2pickc    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Pick Of The Bunch (Club?) (Global) (v1.9) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2pickcp   , sc2pick   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "Pick Of The Bunch (Club?) (Global) (v1.9 Protocol) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2rock     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "How Big's Your Rock? (Global) (v1.5) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2rockp    , sc2rock   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "How Big's Your Rock? (Global) (v1.5 Protocol) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2rock1    , sc2rock   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "How Big's Your Rock? (Global) (v1.4) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2rock1p   , sc2rock   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "How Big's Your Rock? (Global) (v1.4 Protocol) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2rocke    , sc2rock   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "How Big's Your Rock? (Global) (v?.? Euro) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2call     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "It's Your Call (Global) (v2.7) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2callp    , sc2call   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "It's Your Call (Global) (v2.7 Protocol) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2callc    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "It's Your Call (Club?) (Global) (v1.6) (Scorpion 2/3)", GAME_FLAGS)
GAME( 199?, sc2callcp   , sc2callc  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Global",   "It's Your Call (Club?) (Global) (v1.6 Protocol) (Scorpion 2/3)", GAME_FLAGS)

GAME( 199?, sc2bar7     , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 1)", GAME_FLAGS)
GAME( 199?, sc2bar7a    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 2)", GAME_FLAGS)
GAME( 199?, sc2bar7b    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 3)", GAME_FLAGS)
GAME( 199?, sc2bar7c    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 4)", GAME_FLAGS)
GAME( 199?, sc2bar7d    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 5)", GAME_FLAGS)
GAME( 199?, sc2bar7e    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 6)", GAME_FLAGS)
GAME( 199?, sc2bar7f    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 7)", GAME_FLAGS)
GAME( 199?, sc2bar7g    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 8)", GAME_FLAGS)
GAME( 199?, sc2bar7h    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 9)", GAME_FLAGS)
GAME( 199?, sc2bar7i    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 10)", GAME_FLAGS)
GAME( 199?, sc2bar7j    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 11)", GAME_FLAGS)
GAME( 199?, sc2bar7k    , sc2bar7   ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Bar 7 (Concept) (set 12)", GAME_FLAGS)

GAME( 199?, sc2bbar7    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 1)", GAME_FLAGS)
GAME( 199?, sc2bbar7a   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 2)", GAME_FLAGS)
GAME( 199?, sc2bbar7b   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 3)", GAME_FLAGS)
GAME( 199?, sc2bbar7c   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 4)", GAME_FLAGS)
GAME( 199?, sc2bbar7d   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 5)", GAME_FLAGS)
GAME( 199?, sc2bbar7e   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 6)", GAME_FLAGS)
GAME( 199?, sc2bbar7f   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 7)", GAME_FLAGS)
GAME( 199?, sc2bbar7g   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 8)", GAME_FLAGS)
GAME( 199?, sc2bbar7h   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 9)", GAME_FLAGS)
GAME( 199?, sc2bbar7i   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 10)", GAME_FLAGS)
GAME( 199?, sc2bbar7j   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 11)", GAME_FLAGS)
GAME( 199?, sc2bbar7k   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 12)", GAME_FLAGS)
GAME( 199?, sc2bbar7l   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 13)", GAME_FLAGS)
GAME( 199?, sc2bbar7m   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 14)", GAME_FLAGS)
GAME( 199?, sc2bbar7n   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 15)", GAME_FLAGS)
GAME( 199?, sc2bbar7o   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 16)", GAME_FLAGS)
GAME( 199?, sc2bbar7p   , sc2bbar7  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Big Bar 7 (Concept) (set 17)", GAME_FLAGS)

GAME( 199?, sc2flutr    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Flutter (Concept)", GAME_FLAGS) // not a game, but a link unit?
GAME( 199?, sc2smnud    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Super Multi Nudger (Concept)", GAME_FLAGS)

//Seems to be plain Scorpion 2 - keeps tripping watchdog?
GAME( 199?, sc2scshx    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Super Cash X (Concept)", GAME_FLAGS)
GAME( 199?, sc2sghst    , 0         ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Super Ghost (Concept)", GAME_FLAGS)
GAME( 199?, sc2scshxgman, sc2scshx  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Super Cash X (Concept) (Gamesman Hardware)", GAME_FLAGS)
GAME( 199?, sc2scshxstar, sc2scshx  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Super Cash X (Concept) (Starpoint Hardware)", GAME_FLAGS)
GAME( 199?, sc2scshxcas,  sc2scshx  ,  scorpion2        , drwho     , bfm_sc2_state, drwho      , 0,         "Concept",   "Super Casino Cash X (Concept)", GAME_FLAGS)
