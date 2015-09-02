// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese
/*
    Sega Model 2: i960KB + (5x TGP) or (2x SHARC) or (2x TGPx4)
    System 24 tilemaps
    Custom Sega/Lockheed-Martin rasterization hardware
    (68000 + YM3834 + 2x MultiPCM) or (68000 + SCSP)

    Hardware and protection reverse-engineering and general assistance by ElSemi.
    MAME driver by R. Belmont, Olivier Galibert, ElSemi and Angelo Salese.

    TODO (updated as for April 2014):
    - all Model 2B games: FIFO comms looks way wrong, and 3d is mostly missing/incomplete. Games also tends to stalls at some point, culprit might be when i960 tries
      to use a burst type opcode read;
    - Inputs needs device-ification and clean-ups;
    - Sound comms actually passes thru a 8251-compatible device, hook it up;
    - daytona: runs at half speed in gameplay;
    - desert: several 3d bugs, presumably down to FIFO;
    - dynamcop: stalls at stage select screen;
    - fvipers: enables timers, but then irq register is empty, hence it crashes with an "interrupt halt" at POST (regression);
    - lastbrnx: uses external DMA port 0 for uploading SHARC program, hook-up might not be 100% right;
    - lastbrnx: uses a shitload of unsupported SHARC opcodes (compute_fmul_avg, shift operation 0x11, ALU operation 0x89 (compute_favg));
    - lastbrnx: eventually crashes in attract mode, geo_parse_nn_s() is the culprit apparently;
    - manxtt: missing 3d;
    - motoraid: stalls after course select;
    - pltkidsa: after few secs of gameplay, background 3d disappears and everything reports a collision against the player;
    - skytargt: MAME hardlocks after disclaimer screen;
    - srallyc: opponent cars flickers like wild;
    - vcop: lightgun input is offsetted;
    - vcop: sound dies at enter initial screen (i.e. after played the game once);
    - vcop: tilemap priority bug at stage select screen;
    - vf2: stalls after disclaimer screen;
    - vstriker: countdown in team select goes way too fast ...
    - vstriker: ... meanwhile gameplay is way too slow!?
    - zeroguna: stalls after some seconds of gameplay;

    OK (controls may be wrong/missing/incomplete)
    --
    daytona/daytonat/daytonam
    desert
    vcop
    vf2
    vcop2
    zerogun
    gunblade
    indy500
    bel
    hotd
    topskatr
    von
    fvipers
    schamp
    stcc
    srallyc
    skytargt
    dynamcop
    dynabb97
    lastbrnj/lastbrnx
    skisuprg

    almost OK
    ---------
    overrev: sound CPU crashes.
    vstriker: shows some attract mode, then hangs
    manxtt: no escape from "active motion slider" tutorial (needs analog inputs), bypass it by entering then exiting service mode
    manxtt: crashes after the title screen, the TGP is the cause
    pltkids/pltkidsa: crashes after some time of gameplay.
    rchase2: fails drive bd i/o check

    TODO
    ----
    Controls are pretty basic right now
    Some games (sgt24h, indy500) hangs at random places, presumably due to a regression with the SHARC fifo comms
    Sound doesn't work properly in all games
    System 24 tilemaps need more advanced linescroll support (see fvipers, daytona)
    2C needs DSP still

======================================================================================================================================

    Sega Model 2 Feedback Driver Board
    ----------------------------------


    PCB Layout
    ----------

    SJ25-0207-01
    838-10646 (Daytona)
    838-11661 (Sega Rally)
    |---------------------------------------------|
    |             7-SEG-LED 7-SEG-LED             |
    |                                             |
    |   315-5296      315-5296                    |
    |                 DSW(8)                      |
    |    M6253                      MB3759        |
    |                                             |
    |           GAL.IC23  ROM.IC12                |
    |                                             |
    |     Z80                                     |
    |8MHz   MB3771 MB3771  8464                   |
    |---------------------------------------------|
    Notes:
          Z80      - clock 4.000MHz [8/2]
          8464     - 8k x8 SRAM
          ROM.IC12 - EPR-16488A for Daytona
                     EPR-17891  for Sega Rally
          GAL      - Lattice GAL16V8B stamped 315-5625 common to both Daytona and Sega Rally
          DSW(8)   - 8-Position dip switch, all OFF
          M6253    - Oki M6253
          315-5296 - Sega Custom QFP100
          plus several transistors, resistors, a couple of relays and 8 connectors.


*/

#include "emu.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "video/segaic24.h"
#include "cpu/i960/i960.h"
#include "cpu/m68000/m68000.h"
#include "cpu/sharc/sharc.h"
#include "cpu/mb86233/mb86233.h"
#include "cpu/mb86235/mb86235.h"
#include "cpu/z80/z80.h"
#include "sound/2612intf.h"
#include "includes/model2.h"

enum {
	DSP_TYPE_TGP    = 1,
	DSP_TYPE_SHARC  = 2,
	DSP_TYPE_TGPX4  = 3
};


#define COPRO_FIFOIN_SIZE   32000
bool model2_state::copro_fifoin_pop(device_t *device, UINT32 *result,UINT32 offset, UINT32 mem_mask)
{
	UINT32 r;

	if (m_copro_fifoin_num == 0)
	{
		if (m_dsp_type == DSP_TYPE_TGP)
			return false;

		fatalerror("Copro FIFOIN underflow (at %08X)\n", device->safe_pc());
	}

	r = m_copro_fifoin_data[m_copro_fifoin_rpos++];

	if (m_copro_fifoin_rpos == COPRO_FIFOIN_SIZE)
	{
		m_copro_fifoin_rpos = 0;
	}

	m_copro_fifoin_num--;
	if (m_dsp_type == DSP_TYPE_SHARC)
	{
		if (m_copro_fifoin_num == 0)
		{
			dynamic_cast<adsp21062_device *>(device)->set_flag_input(0, ASSERT_LINE);
		}
		else
		{
			dynamic_cast<adsp21062_device *>(device)->set_flag_input(0, CLEAR_LINE);
		}
	}

	*result = r;

	return true;
}

READ_LINE_MEMBER(model2_state::copro_tgp_fifoin_pop_ok)
{
	if (m_copro_fifoin_num == 0)
	{
		return CLEAR_LINE;
	}

	return ASSERT_LINE;
}


READ32_MEMBER(model2_state::copro_tgp_fifoin_pop)
{
	UINT32 r = m_copro_fifoin_data[m_copro_fifoin_rpos++];


	if (m_copro_fifoin_rpos == COPRO_FIFOIN_SIZE)
	{
		m_copro_fifoin_rpos = 0;
	}

	m_copro_fifoin_num--;

	return r;
}


void model2_state::copro_fifoin_push(device_t *device, UINT32 data, UINT32 offset, UINT32 mem_mask)
{
	if (m_copro_fifoin_num == COPRO_FIFOIN_SIZE)
	{
		fatalerror("Copro FIFOIN overflow (at %08X)\n", device->safe_pc());
	}

	//printf("COPRO FIFOIN at %08X, %08X, %f\n", device->safe_pc(), data, *(float*)&data);

	m_copro_fifoin_data[m_copro_fifoin_wpos++] = data;
	if (m_copro_fifoin_wpos == COPRO_FIFOIN_SIZE)
	{
		m_copro_fifoin_wpos = 0;
	}

	m_copro_fifoin_num++;

	// clear FIFO empty flag on SHARC
	if (m_dsp_type == DSP_TYPE_SHARC)
	{
		dynamic_cast<adsp21062_device *>(device)->set_flag_input(0, CLEAR_LINE);
	}
}


#define COPRO_FIFOOUT_SIZE  32000
UINT32 model2_state::copro_fifoout_pop(address_space &space,UINT32 offset, UINT32 mem_mask)
{
	UINT32 r;

	if (m_copro_fifoout_num == 0)
	{
		/* Reading from empty FIFO causes the i960 to enter wait state */
		downcast<i960_cpu_device &>(space.device()).i960_stall();

		/* spin the main cpu and let the TGP catch up */
		space.device().execute().spin_until_time(attotime::from_usec(100));

		return 0;
	}

	r = m_copro_fifoout_data[m_copro_fifoout_rpos++];

	if (m_copro_fifoout_rpos == COPRO_FIFOOUT_SIZE)
	{
		m_copro_fifoout_rpos = 0;
	}

	m_copro_fifoout_num--;

//  logerror("COPRO FIFOOUT POP %08X, %f, %d\n", r, *(float*)&r,m_copro_fifoout_num);

	// set SHARC flag 1: 0 if space available, 1 if FIFO full
	if (m_dsp_type == DSP_TYPE_SHARC)
	{
		if (m_copro_fifoout_num == COPRO_FIFOOUT_SIZE)
		{
			space.machine().device<adsp21062_device>("dsp")->set_flag_input(1, ASSERT_LINE);
		}
		else
		{
			space.machine().device<adsp21062_device>("dsp")->set_flag_input(1, CLEAR_LINE);
		}
	}

	return r;
}

void model2_state::copro_fifoout_push(device_t *device, UINT32 data,UINT32 offset,UINT32 mem_mask)
{
	if (m_copro_fifoout_num == COPRO_FIFOOUT_SIZE)
	{
		fatalerror("Copro FIFOOUT overflow (at %08X)\n", device->safe_pc());
	}

//  logerror("COPRO FIFOOUT PUSH %08X, %f, %d\n", data, *(float*)&data,m_copro_fifoout_num);

	m_copro_fifoout_data[m_copro_fifoout_wpos++] = data;
	if (m_copro_fifoout_wpos == COPRO_FIFOOUT_SIZE)
	{
		m_copro_fifoout_wpos = 0;
	}

	m_copro_fifoout_num++;

	// set SHARC flag 1: 0 if space available, 1 if FIFO full
	if (m_dsp_type == DSP_TYPE_SHARC)
	{
		if (m_copro_fifoout_num == COPRO_FIFOOUT_SIZE)
		{
			dynamic_cast<adsp21062_device *>(device)->set_flag_input(1, ASSERT_LINE);

			//device->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
		}
		else
		{
			dynamic_cast<adsp21062_device *>(device)->set_flag_input(1, CLEAR_LINE);

			//device->execute().set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);
		}
	}
}

WRITE32_MEMBER(model2_state::copro_tgp_fifoout_push)
{
	if (m_copro_fifoout_num == COPRO_FIFOOUT_SIZE)
	{
		fatalerror("Copro FIFOOUT overflow (at %08X)\n", m_tgp->pc());
	}

//  logerror("COPRO FIFOOUT PUSH %08X, %f, %d\n", data, *(float*)&data,m_copro_fifoout_num);

	m_copro_fifoout_data[m_copro_fifoout_wpos++] = data;
	if (m_copro_fifoout_wpos == COPRO_FIFOOUT_SIZE)
	{
		m_copro_fifoout_wpos = 0;
	}

	m_copro_fifoout_num++;
}

/* Timers - these count down at 25 MHz and pull IRQ2 when they hit 0 */
READ32_MEMBER(model2_state::timers_r)
{
	m_maincpu->i960_noburst();

	// if timer is running, calculate current value
	if (m_timerrun[offset])
	{
		// get elapsed time, convert to units of 25 MHz
		UINT32 cur = (m_timers[offset]->time_elapsed() * 25000000).as_double();

		// subtract units from starting value
		m_timervals[offset] = m_timerorig[offset] - cur;
	}

	return m_timervals[offset];
}

WRITE32_MEMBER(model2_state::timers_w)
{
	attotime period;

	m_maincpu->i960_noburst();
	COMBINE_DATA(&m_timervals[offset]);

	m_timerorig[offset] = m_timervals[offset];
	period = attotime::from_hz(25000000) * m_timerorig[offset];
	m_timers[offset]->adjust(period);
	m_timerrun[offset] = 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(model2_state::model2_timer_cb)
{
	int tnum = (int)(FPTR)ptr;
	int bit = tnum + 2;

	if(m_timerrun[tnum] == 0)
		return;

	m_timers[tnum]->reset();

	m_intreq |= (1<<bit);
	if(m_intena & 1<<bit)
		m_maincpu->set_input_line(I960_IRQ2, ASSERT_LINE);
	//printf("%08x %08x (%08x)\n",m_intreq,m_intena,1<<bit);
	model2_check_irq_state();

	m_timervals[tnum] = 0xfffff;
	m_timerrun[tnum] = 0;
}

MACHINE_START_MEMBER(model2_state,model2)
{
	m_copro_fifoin_data = auto_alloc_array_clear(machine(), UINT32, COPRO_FIFOIN_SIZE);
	m_copro_fifoout_data = auto_alloc_array_clear(machine(), UINT32, COPRO_FIFOOUT_SIZE);
}

MACHINE_RESET_MEMBER(model2_state,model2_common)
{
	int i;

	m_intreq = 0;
	m_intena = 0;
	m_coproctl = 0;
	m_coprocnt = 0;
	m_geoctl = 0;
	m_geocnt = 0;
	m_ctrlmode = 0;
	m_analog_channel = 0;
	m_soundack = 0;

	m_timervals[0] = 0xfffff;
	m_timervals[1] = 0xfffff;
	m_timervals[2] = 0xfffff;
	m_timervals[3] = 0xfffff;

	m_timerrun[0] = m_timerrun[1] = m_timerrun[2] = m_timerrun[3] = 0;

	m_timers[0] = machine().device<timer_device>("timer0");
	m_timers[1] = machine().device<timer_device>("timer1");
	m_timers[2] = machine().device<timer_device>("timer2");
	m_timers[3] = machine().device<timer_device>("timer3");
	for (i=0; i<4; i++)
		m_timers[i]->reset();
}

MACHINE_RESET_MEMBER(model2_state,model2o)
{
	MACHINE_RESET_CALL_MEMBER(model2_common);

	// hold TGP in halt until we have code
	m_tgp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_dsp_type = DSP_TYPE_TGP;
}

MACHINE_RESET_MEMBER(model2_state,model2_scsp)
{
	membank("bank4")->set_base(memregion("scsp")->base() + 0x200000);
	membank("bank5")->set_base(memregion("scsp")->base() + 0x600000);

	// copy the 68k vector table into RAM
	memcpy(m_soundram, memregion("audiocpu")->base() + 0x80000, 16);
	m_audiocpu->reset();
	m_scsp->set_ram_base(m_soundram);
}

MACHINE_RESET_MEMBER(model2_state,model2)
{
	MACHINE_RESET_CALL_MEMBER(model2_common);
	MACHINE_RESET_CALL_MEMBER(model2_scsp);

	// hold TGP in halt until we have code
	m_tgp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_dsp_type = DSP_TYPE_TGP;
}

MACHINE_RESET_MEMBER(model2_state,model2b)
{
	MACHINE_RESET_CALL_MEMBER(model2_common);
	MACHINE_RESET_CALL_MEMBER(model2_scsp);

	m_dsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	// set FIFOIN empty flag on SHARC
	m_dsp->set_input_line(SHARC_INPUT_FLAG0, ASSERT_LINE);
	// clear FIFOOUT buffer full flag on SHARC
	m_dsp->set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);

	m_dsp_type = DSP_TYPE_SHARC;
}

MACHINE_RESET_MEMBER(model2_state,model2c)
{
	MACHINE_RESET_CALL_MEMBER(model2_common);
	MACHINE_RESET_CALL_MEMBER(model2_scsp);

	m_tgpx4->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_dsp_type = DSP_TYPE_TGPX4;
}

static void chcolor(palette_device &palette, pen_t color, UINT16 data)
{
	palette.set_pen_color(color, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
}

WRITE16_MEMBER(model2_state::model2_palette_w)
{
	COMBINE_DATA(&m_palram[offset]);
	chcolor(m_palette, offset, m_palram[offset]);
}

READ16_MEMBER(model2_state::model2_palette_r)
{
	return m_palram[offset];
}

WRITE32_MEMBER(model2_state::ctrl0_w)
{
	if(ACCESSING_BITS_0_7)
	{
		m_ctrlmode = data & 0x01;
		m_eeprom->di_write((data & 0x20) >> 5);
		m_eeprom->clk_write((data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->cs_write((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE32_MEMBER(model2_state::analog_2b_w)
{
	m_analog_channel = (data >> 16) & 0x07;
}


READ32_MEMBER(model2_state::fifoctl_r)
{
	UINT32 r = 0;

	if (m_copro_fifoout_num == 0)
	{
		r |= 1;
	}

	// #### 1 if fifo empty, zerogun needs | 0x04 set
	return r | 0x04;
}

READ32_MEMBER(model2_state::videoctl_r)
{
	return ((m_screen->frame_number() & 1) << 2) | (m_videocontrol & 3);
}

WRITE32_MEMBER(model2_state::videoctl_w)
{
	COMBINE_DATA(&m_videocontrol);
}

CUSTOM_INPUT_MEMBER(model2_state::_1c00000_r)
{
	UINT32 ret = ioport("IN0")->read();

	if(m_ctrlmode == 0)
	{
		return ret;
	}
	else
	{
		ret &= ~0x0030;
		return ret | 0x00d0 | (m_eeprom->do_read() << 5);
	}
}

CUSTOM_INPUT_MEMBER(model2_state::_1c0001c_r)
{
	UINT32 iptval = 0x00ff;
	if(m_analog_channel < 4)
	{
		static const char *const ports[] = { "ANA0", "ANA1", "ANA2", "ANA3" };
		iptval = ioport(ports[m_analog_channel])->read_safe(0);
		++m_analog_channel;
	}
	return iptval;
}

/*  PORT_DIPSETTING(    0x00, "0" ) // 0: neutral
    PORT_DIPSETTING(    0x10, "1" ) // 2nd gear
    PORT_DIPSETTING(    0x20, "2" ) // 1st gear
    PORT_DIPSETTING(    0x30, "3" )
    PORT_DIPSETTING(    0x40, "4" )
    PORT_DIPSETTING(    0x50, "5" ) // 4th gear
    PORT_DIPSETTING(    0x60, "6" ) // 3rd gear
    PORT_DIPSETTING(    0x70, "7" )*/

/* Used specifically by Sega Rally, others might be different */
CUSTOM_INPUT_MEMBER(model2_state::srallyc_gearbox_r)
{
	UINT8 res = ioport("GEARS")->read_safe(0);
	int i;
	const UINT8 gearvalue[5] = { 0, 2, 1, 6, 5 };

	for(i=0;i<5;i++)
	{
		if(res & 1<<i)
		{
			m_gearsel = i;
			return gearvalue[i];
		}
	}

	return gearvalue[m_gearsel];
}

/*
    Rail Chase 2 "Drive I/O BD" documentation

    Controlled by a CPU with undumped program code.
    Aux board 837-11694, Z80 (4Mhz) with program rom EPR-17895 (undumped)

    commands 0x2* are for device status bits (all of them active low)

    command 0x27 (4 port valve rear cylinder)
    ---- --xx Cylinder Position (00 - neutral, 01 - up, 10 - down, 11 - error)

    command 0x29
    ---- -x-- Compressor Motor
    ---- --x- Unloader Valve
    ---- ---x Compression Valve

    command 0x2a (4 port valve left cylinder)
    ---- -x-- Rev Valve
    ---- --x- Down Valve
    ---- ---x Up Valve

    command 0x2b (4 port valve right cylinder)
    ---- -x-- Rev Valve
    ---- --x- Down Valve
    ---- ---x Up Valve

    command 0x2e
    ---- --xx Compression SW (00 - error, 01 - low, 10 - high, 11 - error)

    command 0x2f
    ---- x--- Emergency SW
    ---- ---x Safety Sensor

    These are all used on network check, probably some specific data port R/Ws

    command 0x3b
    command 0xe0
    command 0xd0
    command 0xb0
    command 0x70
    command 0x0e
    command 0x0d
    command 0x0b
    command 0x07

    Every other write of this controls devices behaviour:

    command 0x4f (left up valve off)
    command 0x5b (left down valve off)
    command 0x5d (compression valve on)
    command 0x5e (left rev valve on)
    command 0x5f (left Cylinder reset)

    command 0x6f (right up valve off)
    command 0x7b (right down valve off)
    command 0x7d (compression valve on)
    command 0x7e (right rev valve on)
    command 0x7f (right Cylinder reset)

    command 0x84 (reset up/down valves of rear cylinder)
    command 0x85 (rear up valve on)
    command 0x86 (rear down valve on)

    command 0x8b (compression valve on)
    command 0x8d (left rev valve is on)
    command 0x8e (right rev valve is on)
    command 0x8f (reset 4 port valve left / right cylinders and compression valve)

*/


CUSTOM_INPUT_MEMBER(model2_state::rchase2_devices_r)
{
	return 0xffff;
}

WRITE32_MEMBER(model2_state::rchase2_devices_w)
{
	/*
	0x00040000 start 1 lamp
	0x00080000 start 2 lamp
	*/

	if(mem_mask == 0x0000ffff)
		m_cmd_data = data;
}


WRITE32_MEMBER(model2_state::srallyc_devices_w)
{
	/*
	0x00040000 start 1 lamp
	0x00200000 vr lamp
	0x00800000 leader lamp
	*/

	if(mem_mask == 0x000000ff || mem_mask == 0x0000ffff)
	{
		m_driveio_comm_data = data & 0xff;
		m_drivecpu->set_input_line(0, HOLD_LINE);
	}
}

/*****************************************************************************/
/* COPRO */

READ32_MEMBER(model2_state::copro_prg_r)
{
	return 0xffffffff;
}


WRITE32_MEMBER(model2_state::copro_prg_w)
{
	if (m_coproctl & 0x80000000)
	{
		logerror("copro_prg_w: %08X:   %08X\n", m_coprocnt, data);
		m_tgpx4_program[m_coprocnt] = data;
		m_coprocnt++;
	}
	else
	{
		//osd_printf_debug("COPRO: push %08X\n", data);
	}
}

READ32_MEMBER(model2_state::copro_ctl1_r)
{
	return m_coproctl;
}

WRITE32_MEMBER(model2_state::copro_ctl1_w)
{
	// did hi bit change?
	if ((data ^ m_coproctl) == 0x80000000)
	{
		if (data & 0x80000000)
		{
			logerror("Start copro upload\n");
			m_coprocnt = 0;
		}
		else
		{
			logerror("Boot copro, %d dwords\n", m_coprocnt);
			switch(m_dsp_type)
			{
				case DSP_TYPE_TGP:
					m_tgp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
					break;
				case DSP_TYPE_SHARC:
					m_dsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
					break;
				case DSP_TYPE_TGPX4:
					m_tgpx4->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
					break;
			}
		}
	}

	COMBINE_DATA(&m_coproctl);
}

WRITE32_MEMBER(model2_state::copro_function_port_w)
{
	UINT32 d = data & 0x800fffff;
	UINT32 a = (offset >> 2) & 0xff;
	d |= a << 23;

	//logerror("copro_function_port_w: %08X, %08X, %08X\n", data, offset, mem_mask);
	if (m_dsp_type == DSP_TYPE_SHARC)
		copro_fifoin_push(machine().device("dsp"), d,offset,mem_mask);
	else
		copro_fifoin_push(machine().device("tgp"), d,offset,mem_mask);
}

READ32_MEMBER(model2_state::copro_fifo_r)
{
	//logerror("copro_fifo_r: %08X, %08X\n", offset, mem_mask);
	return copro_fifoout_pop(space,offset,mem_mask);
}

WRITE32_MEMBER(model2_state::copro_fifo_w)
{
	if (m_coproctl & 0x80000000)
	{
		if (m_dsp_type == DSP_TYPE_SHARC)
		{
			machine().device<adsp21062_device>("dsp")->external_dma_write(m_coprocnt, data & 0xffff);
		}
		else if (m_dsp_type == DSP_TYPE_TGP)
		{
			m_tgp_program[m_coprocnt] = data;
		}

		m_coprocnt++;
	}
	else
	{
//      if(m_coprocnt == 0)
//          return;

		//osd_printf_debug("copro_fifo_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
		if (m_dsp_type == DSP_TYPE_SHARC)
			copro_fifoin_push(machine().device("dsp"), data,offset,mem_mask);
		else
			copro_fifoin_push(machine().device("tgp"), data,offset,mem_mask);
	}
}

WRITE32_MEMBER(model2_state::copro_sharc_iop_w)
{
	/* FIXME: clean this mess */
	if ((strcmp(machine().system().name, "schamp" ) == 0) ||
		(strcmp(machine().system().name, "sfight" ) == 0) ||
		(strcmp(machine().system().name, "fvipers" ) == 0) ||
		(strcmp(machine().system().name, "vstriker" ) == 0) ||
		(strcmp(machine().system().name, "vstrikero" ) == 0) ||
		(strcmp(machine().system().name, "gunblade" ) == 0) ||
		(strcmp(machine().system().name, "von" ) == 0) ||
		(strcmp(machine().system().name, "vonj" ) == 0) ||
		(strcmp(machine().system().name, "rchase2" ) == 0))
	{
		machine().device<adsp21062_device>("dsp")->external_iop_write(offset, data);
	}
	else
	{
		if(offset == 0x10/4)
		{
			machine().device<adsp21062_device>("dsp")->external_iop_write(offset, data);
			return;
		}

		if ((m_iop_write_num & 1) == 0)
		{
			m_iop_data = data & 0xffff;
		}
		else
		{
			m_iop_data |= (data & 0xffff) << 16;
			machine().device<adsp21062_device>("dsp")->external_iop_write(offset, m_iop_data);
		}
		m_iop_write_num++;
	}
}





/*****************************************************************************/
/* GEO */


WRITE32_MEMBER(model2_state::geo_ctl1_w)
{
	// did hi bit change?
	if ((data ^ m_geoctl) == 0x80000000)
	{
		if (data & 0x80000000)
		{
			logerror("Start geo upload\n");
			m_geocnt = 0;
		}
		else
		{
			logerror("Boot geo, %d dwords\n", m_geocnt);
		}
	}

	m_geoctl = data;
}


#ifdef UNUSED_FUNCTION
WRITE32_MEMBER(model2_state::geo_sharc_ctl1_w)
{
	// did hi bit change?
	if ((data ^ m_geoctl) == 0x80000000)
	{
		if (data & 0x80000000)
		{
			logerror("Start geo upload\n");
			m_geocnt = 0;
		}
		else
		{
			logerror("Boot geo, %d dwords\n", m_geocnt);
			m_dsp2->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			//space.device().execute().spin_until_time(attotime::from_usec(1000));       // Give the SHARC enough time to boot itself
		}
	}

	m_geoctl = data;
}

READ32_MEMBER(model2_state::geo_sharc_fifo_r)
{
	if ((strcmp(machine().system().name, "manxtt" ) == 0) || (strcmp(machine().system().name, "srallyc" ) == 0))
	{
		return 8;
	}
	else
	{
		//logerror("copro_fifo_r: %08X, %08X\n", offset, mem_mask);
		return 0;
	}
}

WRITE32_MEMBER(model2_state::geo_sharc_fifo_w)
{
	if (m_geoctl & 0x80000000)
	{
		machine().device<adsp21062_device>("dsp2")->external_dma_write(m_geocnt, data & 0xffff);

		m_geocnt++;
	}
	else
	{
		//osd_printf_debug("copro_fifo_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
	}
}

WRITE32_MEMBER(model2_state::geo_sharc_iop_w)
{
	if ((strcmp(machine().system().name, "schamp" ) == 0))
	{
		machine().device<adsp21062_device>("dsp2")->external_iop_write(offset, data);
	}
	else
	{
		if ((m_geo_iop_write_num & 1) == 0)
		{
			m_geo_iop_data = data & 0xffff;
		}
		else
		{
			m_geo_iop_data |= (data & 0xffff) << 16;
			machine().device<adsp21062_device>("dsp2")->external_iop_write(offset, m_geo_iop_data);
		}
		m_geo_iop_write_num++;
	}
}
#endif


void model2_state::push_geo_data(UINT32 data)
{
	//osd_printf_debug("push_geo_data: %08X: %08X\n", 0x900000+m_geo_write_start_address, data);
	m_bufferram[m_geo_write_start_address/4] = data;
	m_geo_write_start_address += 4;
}

READ32_MEMBER(model2_state::geo_prg_r)
{
	popmessage("Read from Geometry FIFO at %08x, contact MAMEdev",offset*4);
	return 0xffffffff;
}

WRITE32_MEMBER(model2_state::geo_prg_w)
{
	if (m_geoctl & 0x80000000)
	{
		//logerror("geo_prg_w: %08X:   %08X\n", m_geocnt, data);
		m_geocnt++;
	}
	else
	{
		//osd_printf_debug("GEO: %08X: push %08X\n", m_geo_write_start_address, data);
		push_geo_data(data);
	}
}

READ32_MEMBER(model2_state::geo_r)
{
	int address = offset * 4;
	if (address == 0x2008)
	{
		return m_geo_write_start_address;
	}
	else if (address == 0x3008)
	{
		return m_geo_read_start_address;
	}

//  fatalerror("geo_r: %08X, %08X\n", address, mem_mask);
	osd_printf_debug("geo_r: PC:%08x - %08X\n", space.device().safe_pc(), address);

	return 0;
}

WRITE32_MEMBER(model2_state::geo_w)
{
	int address = offset * 4;

	if (address < 0x1000)
	{
		/*if (data & 0x80000000)
		{
		    int i;
		    UINT32 a;
		    osd_printf_debug("GEO: jump to %08X\n", (data & 0xfffff));
		    a = (data & 0xfffff) / 4;
		    for (i=0; i < 4; i++)
		    {
		        osd_printf_debug("   %08X: %08X %08X %08X %08X\n", 0x900000+(a*4)+(i*16),
		            m_bufferram[a+(i*4)+0], m_bufferram[a+(i*4)+1], m_bufferram[a+(i*4)+2], m_bufferram[a+(i*4)+3]);
		    }
		}
		else
		{
		    int function = (address >> 4) & 0x3f;
		    switch (address & 0xf)
		    {
		        case 0x0:
		        {
		            osd_printf_debug("GEO: function %02X (%08X, %08X)\n", function, address, data);
		            break;
		        }

		        case 0x4:   osd_printf_debug("GEO: function %02X, command length %d\n", function, data & 0x3f); break;
		        case 0x8:   osd_printf_debug("GEO: function %02X, data length %d\n", function, data & 0x7f); break;
		    }
		}*/

		if (data & 0x80000000)
		{
			UINT32 r = 0;
			r |= data & 0x800fffff;
			r |= ((address >> 4) & 0x3f) << 23;
			push_geo_data(r);
		}
		else
		{
			if ((address & 0xf) == 0)
			{
				UINT32 r = 0;
				r |= data & 0x000fffff;
				r |= ((address >> 4) & 0x3f) << 23;
				if((address >> 4) & 0xc0)
				{
					UINT8 function = (address >> 4) & 0x3f;
					if(function == 1)
					{
						r |= ((address>>10)&3)<<29; // Eye Mode, used by Sega Rally on car select
						//popmessage("Eye mode %02x? Contact MAMEdev",function);
					}
				}
				push_geo_data(r);
			}
		}
	}
	else if (address == 0x1008)
	{
		//osd_printf_debug("GEO: Write Start Address: %08X\n", data);
		m_geo_write_start_address = data & 0xfffff;
	}
	else if (address == 0x3008)
	{
		//osd_printf_debug("GEO: Read Start Address: %08X\n", data);
		m_geo_read_start_address = data & 0xfffff;
	}
	else
	{
		fatalerror("geo_w: %08X = %08X\n", address, data);
	}
}

/*****************************************************************************/


READ32_MEMBER(model2_state::hotd_lightgun_r)
{
	static const char *const ports[] = { "P1_Y", "P1_X", "P2_Y", "P2_X" };
	UINT16 res = 0xffff;

	if(m_lightgun_mux < 8)
		res = (ioport(ports[m_lightgun_mux >> 1])->read_safe(0) >> ((m_lightgun_mux & 1)*8)) & 0xff;
	else
	{
		UINT16 p1x,p1y,p2x,p2y;

		res = 0xfffc;

		p1x = ioport("P1_X")->read_safe(0);
		p1y = ioport("P1_Y")->read_safe(0);
		p2x = ioport("P2_X")->read_safe(0);
		p2y = ioport("P2_Y")->read_safe(0);

		/* TODO: might be better, supposedly user has to calibrate guns in order to make these settings to work ... */
		if(p1x <= 0x28 || p1x >= 0x3e0 || p1y <= 0x40 || p1y >= 0x3c0)
			res |= 1;

		if(p2x <= 0x28 || p2x >= 0x3e0 || p2y <= 0x40 || p2y >= 0x3c0)
			res |= 2;
	}

	/* upper bits are presumably related to lightgun latches */
	return 0x000c0000 | res;
}

WRITE32_MEMBER(model2_state::hotd_lightgun_w)
{
	m_lightgun_mux = data;
}


#ifdef UNUSED_FUNCTION
READ32_MEMBER(model2_state::sonic_unk_r)
{
	return 0x001a0000;
}
#endif

READ32_MEMBER(model2_state::daytona_unk_r)
{
	return 0x00400000;
}

#if 0
READ32_MEMBER(model2_state::desert_unk_r)
{
	static UINT8 test;

	test ^= 8;
	// vcop needs bit 3 clear (infinite loop otherwise)
	// desert needs other bits set (not sure which specifically)
	// daytona needs the MSW to return ff
	return 0x00f700ff | (test << 16);
}
#endif

READ32_MEMBER(model2_state::model2_irq_r)
{
	m_maincpu->i960_noburst();

	if (offset)
	{
		return m_intena;
	}

	return m_intreq;
}

void model2_state::model2_check_irq_state()
{
	return;

	/* TODO: vf2 and fvipers hangs with an irq halt on POST, disabled for now */
	const int irq_type[12]= {I960_IRQ0,I960_IRQ1,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ3,I960_IRQ3};

	for(int i=0;i<12;i++)
	{
		if (m_intena & (1<<i) && m_intreq & (1<<i))
		{
			m_maincpu->set_input_line(irq_type[i], ASSERT_LINE);
			return;
		}
	}
}

void model2_state::model2_check_irqack_state(UINT32 data)
{
	const int irq_type[12]= {I960_IRQ0,I960_IRQ1,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ2,I960_IRQ3,I960_IRQ3};

	for(int i=0;i<12;i++)
	{
		if(data & 1<<i)
			m_maincpu->set_input_line(irq_type[i], CLEAR_LINE);
	}
}

WRITE32_MEMBER(model2_state::model2_irq_w)
{
	m_maincpu->i960_noburst();

	if (offset)
	{
		COMBINE_DATA(&m_intena);
		model2_check_irq_state();
		return;
	}

	m_intreq &= data;

	model2_check_irqack_state(data ^ 0xffffffff);
}

/* TODO: rewrite this part. It's a 8251-compatible chip */
READ32_MEMBER(model2_state::model2_serial_r)
{
	if ((offset == 0) && (mem_mask == 0xffff0000))
	{
		return 0x00070000;  // TxRdy RxRdy (zeroguna also needs bit 4 set)
	}

	return 0xffffffff;
}


WRITE32_MEMBER(model2_state::model2o_serial_w)
{
	if(mem_mask == 0xffff0000)
	{
		if (!space.debugger_access())
		{
			//m_soundack++;
		}
	}
	if (mem_mask == 0x0000ffff)
	{
		if (!m_m1audio->ready_r(space, 0))
		{
			space.device().execute().spin_until_time(attotime::from_usec(40));
		}

		m_m1audio->write_fifo(data & 0xff);

		// give the 68k time to notice
		space.device().execute().spin_until_time(attotime::from_usec(40));
	}
}

WRITE32_MEMBER(model2_state::model2_serial_w)
{
	if (ACCESSING_BITS_0_7 && (offset == 0))
	{
		if (m_dsbz80 != NULL)
		{
			m_dsbz80->latch_w(space, 0, data&0xff);
		}

		// for Manx TT DX
		if (m_m1audio != NULL)
		{
			if (!m_m1audio->ready_r(space, 0))
			{
				space.device().execute().spin_until_time(attotime::from_usec(40));
			}

			m_m1audio->write_fifo(data & 0xff);
		}

		m_scsp->midi_in(space, 0, data&0xff, 0);

		// give the 68k time to notice
		space.device().execute().spin_until_time(attotime::from_usec(40));
	}
}

/* Protection handling */


READ32_MEMBER(model2_state::model2_5881prot_r)
{
	UINT32 retval = 0;

	if (offset == 0x0000/4)
	{
		// status: bit 0 = 1 for busy, 0 for ready
		retval = 0;   // we're always ready
	}
	else if (offset == 0x000e/4)
	{
		if (first_read == 1)
		{
			// the RAM based schemes expect a dummy value before the start of the stream
			// to match the previous simulation (dynamite cop) I use 0x0000 here
			first_read = 0;
			retval = 0;
		}
		else
		{
			UINT8* base;
			retval = m_cryptdevice->do_decrypt(base);
			retval = ((retval & 0xff00) >> 8) | ((retval & 0x00ff) << 8);
			retval <<= 16;
		}
	}
	else logerror("Unhandled Protection READ @ %x mask %x (PC=%x)\n", offset, mem_mask, space.device().safe_pc());

	logerror("model2_5881prot_r %08x: %08x (%08x)\n", offset*4, retval, mem_mask);

	return retval;
}

WRITE32_MEMBER(model2_state::model2_5881prot_w)
{
	logerror("model2_5881prot_w %08x: %08x (%08x)\n", offset*4, data, mem_mask);


	if (offset == 0x0008/4)
	{
		// Zero Gunner uses this, it's encrypted data in prot.RAM consists of several small chunks, selected using low address
		// so far this is only known game with 315-5881 which uses not 0 offset in prot.RAM
		if (mem_mask == 0x0000ffff)
			m_cryptdevice->set_addr_low(data&0xffff);
		else if (mem_mask == 0xffff0000)
		{
			m_cryptdevice->set_addr_high(0);
			if (data != 0)
				printf("model2_5881prot_w not zero high address %08x (%08x)\n", data, mem_mask);
		}
		first_read = 1;
	}
	else if (offset == 0x000c/4)
	{
		printf("subkey %08x (%08x)\n", data, mem_mask);
		m_cryptdevice->set_subkey(data&0xffff);
	}
	else printf("Unhandled Protection WRITE %x @ %x mask %x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());

}



/* Daytona "To The MAXX" PIC protection simulation */


READ32_MEMBER(model2_state::maxx_r)
{
	UINT32 *ROM = (UINT32 *)memregion("maincpu")->base();

	if (offset <= 0x1f/4)
	{
		// special
		if (mem_mask == 0xffff0000)
		{
			// 16-bit protection reads
			m_maxxstate++;
			m_maxxstate &= 0xf;
			if (!m_maxxstate)
			{
				return 0x00070000;
			}
			else
			{
				if (m_maxxstate & 0x2)
				{
					return 0;
				}
				else
				{
					return 0x00040000;
				}
			}
		}
		else if (mem_mask == 0xffffffff)
		{
			// 32-bit read
			if (offset == 0x22/4)
			{
				return 0x00ff0000;
			}
		}
	}

	return ROM[offset + (0x040000/4)];
}

/* Network board emulation */



READ32_MEMBER(model2_state::network_r)
{
	if ((mem_mask == 0xffffffff) || (mem_mask == 0x0000ffff) || (mem_mask == 0xffff0000))
	{
		return 0xffffffff;
	}

	if (offset < 0x4000/4)
	{
		return m_netram[offset];
	}

	if (mem_mask == 0x00ff0000)
	{
		return m_sysres<<16;
	}
	else if (mem_mask == 0x000000ff)
	{
		return m_zflagi;
	}

	return 0xffffffff;
}

WRITE32_MEMBER(model2_state::network_w)
{
	if ((mem_mask == 0xffffffff) || (mem_mask == 0x0000ffff) || (mem_mask == 0xffff0000))
	{
		COMBINE_DATA(&m_netram[offset+0x4000/4]);
		return;
	}

	if (offset < 0x4000/4)
	{
		COMBINE_DATA(&m_netram[offset]);
		return;
	}

	if (mem_mask == 0x00ff0000)
	{
		m_sysres = data>>16;
	}
	else if (mem_mask == 0x000000ff)
	{
		m_zflagi = data;
		m_zflag = 0;
		if (data & 0x01) m_zflag |= 0x80;
		if (data & 0x80) m_zflag |= 0x01;
	}
}

#ifdef UNUSED_FUNCTION
WRITE32_MEMBER(model2_state::copro_w)
{
	int address = offset * 4;

	if (address < 0x400)
	{
		int function = (address & 0xfff) >> 4;
		switch (address & 0xf)
		{
			case 0x0:   osd_printf_debug("COPRO: function %02X, command %d\n", function, (data >> 23) & 0x3f); break;
			case 0x4:   osd_printf_debug("COPRO: function %02X, command length %d\n", function, data & 0x3f); break;
			case 0x8:   osd_printf_debug("COPRO: function %02X, data length %d\n", function, data & 0x7f); break;
		}
	}

	//osd_printf_debug("COPRO: %08X = %08X\n", offset, data);
}
#endif

WRITE32_MEMBER(model2_state::mode_w)
{
	osd_printf_debug("Mode = %08X\n", data);
}

WRITE32_MEMBER(model2_state::model2o_tex_w0)
{
	if ( (offset & 1) == 0 )
	{
		m_textureram0[offset>>1] &= 0xffff0000;
		m_textureram0[offset>>1] |= data & 0xffff;
	}
	else
	{
		m_textureram0[offset>>1] &= 0x0000ffff;
		m_textureram0[offset>>1] |= (data & 0xffff) << 16;
	}
}

WRITE32_MEMBER(model2_state::model2o_tex_w1)
{
	if ( (offset & 1) == 0 )
	{
		m_textureram1[offset>>1] &= 0xffff0000;
		m_textureram1[offset>>1] |= data & 0xffff;
	}
	else
	{
		m_textureram1[offset>>1] &= 0x0000ffff;
		m_textureram1[offset>>1] |= (data & 0xffff) << 16;
	}
}

WRITE32_MEMBER(model2_state::model2o_luma_w)
{
	if ( (offset & 1) == 0 )
	{
		m_lumaram[offset>>1] &= 0xffff0000;
		m_lumaram[offset>>1] |= data & 0xffff;
	}
	else
	{
		m_lumaram[offset>>1] &= 0x0000ffff;
		m_lumaram[offset>>1] |= (data & 0xffff) << 16;
	}
}

WRITE32_MEMBER(model2_state::model2_3d_zclip_w)
{
	model2_3d_set_zclip( machine(), data & 0xFF );
}

/* Top Skater reads here and discards the result */
READ8_MEMBER(model2_state::tgpid_r)
{
	unsigned char ID[]={0,'T','A','H',0,'A','K','O',0,'Z','A','K',0,'M','T','K'};

	return ID[offset];
}

/* Sky Target, TODO */
READ32_MEMBER(model2_state::polygon_count_r)
{
	return 0;
}

/* common map for all Model 2 versions */
static ADDRESS_MAP_START( model2_base_mem, AS_PROGRAM, 32, model2_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_WRITENOP

	AM_RANGE(0x00500000, 0x005fffff) AM_RAM AM_SHARE("workram")

	AM_RANGE(0x00800000, 0x00803fff) AM_READWRITE(geo_r, geo_w)
	//AM_RANGE(0x00800010, 0x00800013) AM_WRITENOP
	//AM_RANGE(0x008000b0, 0x008000b3) AM_WRITENOP
	//AM_RANGE(0x00804004, 0x0080400f) AM_WRITENOP  // quiet psikyo games

	//AM_RANGE(0x00880000, 0x00883fff) AM_WRITE(copro_w)

	AM_RANGE(0x00900000, 0x0097ffff) AM_RAM AM_SHARE("bufferram")

	AM_RANGE(0x00980004, 0x00980007) AM_READ(fifoctl_r)
	AM_RANGE(0x0098000c, 0x0098000f) AM_READWRITE(videoctl_r,videoctl_w)
	AM_RANGE(0x00980030, 0x0098003f) AM_READ8(tgpid_r,0xffffffff)

	AM_RANGE(0x00e00000, 0x00e00037) AM_RAM // CPU control (wait-states)
	AM_RANGE(0x00e80000, 0x00e80007) AM_READWRITE(model2_irq_r, model2_irq_w)

	AM_RANGE(0x00f00000, 0x00f0000f) AM_READWRITE(timers_r, timers_w)

	AM_RANGE(0x01000000, 0x0100ffff) AM_DEVREADWRITE("tile", segas24_tile, tile32_r, tile32_w) AM_MIRROR(0x110000)
	AM_RANGE(0x01020000, 0x01020003) AM_WRITENOP AM_MIRROR(0x100000)        // Unknown, always 0
	AM_RANGE(0x01040000, 0x01040003) AM_WRITENOP AM_MIRROR(0x100000)        // Horizontal synchronization register
	AM_RANGE(0x01060000, 0x01060003) AM_WRITENOP AM_MIRROR(0x100000)        // Vertical synchronization register
	AM_RANGE(0x01070000, 0x01070003) AM_WRITENOP AM_MIRROR(0x100000)        // Video synchronization switch
	AM_RANGE(0x01080000, 0x010fffff) AM_DEVREADWRITE("tile", segas24_tile, char32_r, char32_w) AM_MIRROR(0x100000)

	AM_RANGE(0x01800000, 0x01803fff) AM_READWRITE16(model2_palette_r,model2_palette_w,0xffffffff)
	AM_RANGE(0x01810000, 0x0181bfff) AM_RAM AM_SHARE("colorxlat")
	AM_RANGE(0x0181c000, 0x0181c003) AM_WRITE(model2_3d_zclip_w)
	AM_RANGE(0x01a10000, 0x01a1ffff) AM_READWRITE(network_r, network_w)
	AM_RANGE(0x01d00000, 0x01d03fff) AM_RAM AM_SHARE("backup1") // Backup sram
	AM_RANGE(0x02000000, 0x03ffffff) AM_ROM AM_REGION("user1", 0)

	// "extra" data
	AM_RANGE(0x06000000, 0x06ffffff) AM_ROM AM_REGION("user1", 0x1000000)

	AM_RANGE(0x10000000, 0x101fffff) AM_WRITE(mode_w)
	AM_RANGE(0x10400000, 0x105fffff) AM_READ(polygon_count_r)

	AM_RANGE(0x11600000, 0x1167ffff) AM_RAM AM_SHARE("share1") // framebuffer (last bronx)
	AM_RANGE(0x11680000, 0x116fffff) AM_RAM AM_SHARE("share1") // FB mirror
ADDRESS_MAP_END

READ8_MEMBER(model2_state::virtuacop_lightgun_r)
{
	static const char *const ports[] = { "P1_Y", "P1_X", "P2_Y", "P2_X" };
	UINT8 res;

	res = (ioport(ports[offset >> 1])->read_safe(0) >> ((offset & 1)*8)) & 0xff;

	return res;
}

/* handles offscreen gun trigger detection here */
READ8_MEMBER(model2_state::virtuacop_lightgun_offscreen_r)
{
	UINT16 special_res = 0xfffc;
	UINT16 p1x,p1y,p2x,p2y;

	p1x = ioport("P1_X")->read_safe(0);
	p1y = ioport("P1_Y")->read_safe(0);
	p2x = ioport("P2_X")->read_safe(0);
	p2y = ioport("P2_Y")->read_safe(0);

	/* TODO: might be better, supposedly user has to calibrate guns in order to make these settings to work ... */
	if(p1x <= 0x28 || p1x >= 0x3e0 || p1y <= 0x40 || p1y >= 0x3c0)
		special_res |= 1;

	if(p2x <= 0x28 || p2x >= 0x3e0 || p2y <= 0x40 || p2y >= 0x3c0)
		special_res |= 2;

	return (special_res >> ((offset & 1)*8)) & 0xff;
}

/* Apparently original Model 2 doesn't have fifo control? */
READ32_MEMBER(model2_state::model2o_fifoctrl_r)
{
	return 0xffffffff;
}

/* original Model 2 overrides */
static ADDRESS_MAP_START( model2o_mem, AS_PROGRAM, 32, model2_state )
	AM_RANGE(0x00200000, 0x0021ffff) AM_RAM
	AM_RANGE(0x00220000, 0x0023ffff) AM_ROM AM_REGION("maincpu", 0x20000)

	AM_RANGE(0x00804000, 0x00807fff) AM_READWRITE(geo_prg_r, geo_prg_w)
	AM_RANGE(0x00880000, 0x00883fff) AM_WRITE(copro_function_port_w)
	AM_RANGE(0x00884000, 0x00887fff) AM_READWRITE(copro_fifo_r, copro_fifo_w)

	AM_RANGE(0x00980000, 0x00980003) AM_READWRITE(copro_ctl1_r,copro_ctl1_w)
	AM_RANGE(0x00980004, 0x00980007) AM_READ(model2o_fifoctrl_r)
	AM_RANGE(0x00980008, 0x0098000b) AM_WRITE(geo_ctl1_w)
	AM_RANGE(0x009c0000, 0x009cffff) AM_READWRITE(model2_serial_r, model2o_serial_w)

	AM_RANGE(0x12000000, 0x121fffff) AM_RAM_WRITE(model2o_tex_w0) AM_MIRROR(0x200000) AM_SHARE("textureram0")   // texture RAM 0
	AM_RANGE(0x12400000, 0x125fffff) AM_RAM_WRITE(model2o_tex_w1) AM_MIRROR(0x200000) AM_SHARE("textureram1")   // texture RAM 1
	AM_RANGE(0x12800000, 0x1281ffff) AM_RAM_WRITE(model2o_luma_w) AM_SHARE("lumaram") // polygon "luma" RAM

	AM_RANGE(0x01c00000, 0x01c00003) AM_READ_PORT("1c00000")
	AM_RANGE(0x01c00004, 0x01c00007) AM_READ_PORT("1c00004")
	AM_RANGE(0x01c00010, 0x01c00013) AM_READ_PORT("1c00010")
	AM_RANGE(0x01c00014, 0x01c00017) AM_READ_PORT("1c00014")
	AM_RANGE(0x01c0001c, 0x01c0001f) AM_READ_PORT("1c0001c")
	AM_RANGE(0x01c00040, 0x01c00043) AM_READ(daytona_unk_r )
	AM_RANGE(0x01c00100, 0x01c0010f) AM_READ8(virtuacop_lightgun_r,0x00ff00ff)
	AM_RANGE(0x01c00110, 0x01c00113) AM_READ8(virtuacop_lightgun_offscreen_r,0x00ff00ff)
	AM_RANGE(0x01c00200, 0x01c002ff) AM_RAM AM_SHARE("backup2")
	AM_RANGE(0x01c80000, 0x01c80003) AM_READWRITE(model2_serial_r, model2o_serial_w )

	AM_IMPORT_FROM(model2_base_mem)
ADDRESS_MAP_END

/* TODO: read by Sonic the Fighters (bit 1), unknown purpose */
READ32_MEMBER(model2_state::copro_status_r)
{
	if(m_coprocnt == 0)
		return -1;

	return 0;
}

/* 2A-CRX overrides */
static ADDRESS_MAP_START( model2a_crx_mem, AS_PROGRAM, 32, model2_state )
	AM_RANGE(0x00200000, 0x0023ffff) AM_RAM

	AM_RANGE(0x00804000, 0x00807fff) AM_READWRITE(geo_prg_r, geo_prg_w)
	AM_RANGE(0x00880000, 0x00883fff) AM_WRITE(copro_function_port_w)
	AM_RANGE(0x00884000, 0x00887fff) AM_READWRITE(copro_fifo_r, copro_fifo_w)

	AM_RANGE(0x00980000, 0x00980003) AM_READWRITE(copro_ctl1_r,copro_ctl1_w)
	AM_RANGE(0x00980008, 0x0098000b) AM_WRITE(geo_ctl1_w)
	AM_RANGE(0x00980014, 0x00980017) AM_READ(copro_status_r)
	AM_RANGE(0x009c0000, 0x009cffff) AM_READWRITE(model2_serial_r, model2_serial_w )

	AM_RANGE(0x12000000, 0x121fffff) AM_RAM_WRITE(model2o_tex_w0) AM_MIRROR(0x200000) AM_SHARE("textureram0")   // texture RAM 0
	AM_RANGE(0x12400000, 0x125fffff) AM_RAM_WRITE(model2o_tex_w1) AM_MIRROR(0x200000) AM_SHARE("textureram1")   // texture RAM 1
	AM_RANGE(0x12800000, 0x1281ffff) AM_RAM_WRITE(model2o_luma_w) AM_SHARE("lumaram") // polygon "luma" RAM

	AM_RANGE(0x01c00000, 0x01c00003) AM_READ_PORT("1c00000") AM_WRITE(ctrl0_w )
	AM_RANGE(0x01c00004, 0x01c00007) AM_READ_PORT("1c00004")
	AM_RANGE(0x01c00008, 0x01c0000f) AM_READNOP AM_WRITENOP
	AM_RANGE(0x01c0000c, 0x01c0000f) AM_READ_PORT("1c0000c")
	AM_RANGE(0x01c00010, 0x01c00013) AM_READ_PORT("1c00010") AM_WRITENOP
	AM_RANGE(0x01c00014, 0x01c00017) AM_READ_PORT("1c00014") AM_WRITE(hotd_lightgun_w)
	AM_RANGE(0x01c00018, 0x01c0001b) AM_READ(hotd_lightgun_r)
	AM_RANGE(0x01c0001c, 0x01c0001f) AM_READ_PORT("1c0001c") AM_WRITE(analog_2b_w )
	AM_RANGE(0x01c00040, 0x01c00043) AM_WRITENOP
	AM_RANGE(0x01c80000, 0x01c80003) AM_READWRITE(model2_serial_r, model2_serial_w )

	AM_IMPORT_FROM(model2_base_mem)
ADDRESS_MAP_END

/* 2B-CRX overrides */
static ADDRESS_MAP_START( model2b_crx_mem, AS_PROGRAM, 32, model2_state )
	AM_RANGE(0x00200000, 0x0023ffff) AM_RAM

	AM_RANGE(0x00804000, 0x00807fff) AM_READWRITE(geo_prg_r, geo_prg_w)
	//AM_RANGE(0x00804000, 0x00807fff) AM_READWRITE(geo_sharc_fifo_r, geo_sharc_fifo_w)
	//AM_RANGE(0x00840000, 0x00840fff) AM_WRITE(geo_sharc_iop_w)

	AM_RANGE(0x00880000, 0x00883fff) AM_WRITE(copro_function_port_w)
	AM_RANGE(0x00884000, 0x00887fff) AM_READWRITE(copro_fifo_r, copro_fifo_w)
	AM_RANGE(0x008c0000, 0x008c0fff) AM_WRITE(copro_sharc_iop_w)

	AM_RANGE(0x00980000, 0x00980003) AM_READWRITE(copro_ctl1_r,copro_ctl1_w)
	AM_RANGE(0x00980008, 0x0098000b) AM_WRITE(geo_ctl1_w)
	AM_RANGE(0x00980014, 0x00980017) AM_READ(copro_status_r)
	//AM_RANGE(0x00980008, 0x0098000b) AM_WRITE(geo_sharc_ctl1_w )

	AM_RANGE(0x009c0000, 0x009cffff) AM_READWRITE(model2_serial_r, model2_serial_w )

	AM_RANGE(0x11000000, 0x110fffff) AM_RAM AM_SHARE("textureram0") // texture RAM 0 (2b/2c)
	AM_RANGE(0x11100000, 0x111fffff) AM_RAM AM_SHARE("textureram0") // texture RAM 0 (2b/2c)
	AM_RANGE(0x11200000, 0x112fffff) AM_RAM AM_SHARE("textureram1") // texture RAM 1 (2b/2c)
	AM_RANGE(0x11300000, 0x113fffff) AM_RAM AM_SHARE("textureram1") // texture RAM 1 (2b/2c)
	AM_RANGE(0x11400000, 0x1140ffff) AM_RAM AM_SHARE("lumaram")     // polygon "luma" RAM (2b/2c)
	AM_RANGE(0x12800000, 0x1281ffff) AM_RAM AM_SHARE("lumaram") // polygon "luma" RAM


	AM_RANGE(0x01c00000, 0x01c00003) AM_READ_PORT("1c00000") AM_WRITE(ctrl0_w )
	AM_RANGE(0x01c00004, 0x01c00007) AM_READ_PORT("1c00004")
	AM_RANGE(0x01c00008, 0x01c0000b) AM_READNOP AM_WRITENOP
	AM_RANGE(0x01c0000c, 0x01c0000f) AM_READNOP
	AM_RANGE(0x01c00010, 0x01c00013) AM_READ_PORT("1c00010")
	AM_RANGE(0x01c00014, 0x01c00017) AM_READ_PORT("1c00014") AM_WRITE(hotd_lightgun_w)
	AM_RANGE(0x01c00018, 0x01c0001b) AM_READ(hotd_lightgun_r)
	AM_RANGE(0x01c0001c, 0x01c0001f) AM_READ_PORT("1c0001c") AM_WRITE(analog_2b_w )
	AM_RANGE(0x01c00040, 0x01c00043) AM_WRITENOP
	AM_RANGE(0x01c80000, 0x01c80003) AM_READWRITE(model2_serial_r, model2_serial_w )

	AM_IMPORT_FROM(model2_base_mem)
ADDRESS_MAP_END

/* 2C-CRX overrides */
static ADDRESS_MAP_START( model2c_crx_mem, AS_PROGRAM, 32, model2_state )
	AM_RANGE(0x00200000, 0x0023ffff) AM_RAM

	AM_RANGE(0x00804000, 0x00807fff) AM_READWRITE(geo_prg_r, geo_prg_w)
	AM_RANGE(0x00884000, 0x00887fff) AM_READWRITE(copro_prg_r, copro_prg_w)

	AM_RANGE(0x00980000, 0x00980003) AM_READWRITE(copro_ctl1_r,copro_ctl1_w)
	AM_RANGE(0x00980008, 0x0098000b) AM_WRITE(geo_ctl1_w )
	AM_RANGE(0x00980014, 0x00980017) AM_READ(copro_status_r)
	AM_RANGE(0x009c0000, 0x009cffff) AM_READWRITE(model2_serial_r, model2_serial_w )

	AM_RANGE(0x11000000, 0x111fffff) AM_RAM AM_SHARE("textureram0") // texture RAM 0 (2b/2c)
	AM_RANGE(0x11200000, 0x113fffff) AM_RAM AM_SHARE("textureram1") // texture RAM 1 (2b/2c)
	AM_RANGE(0x11400000, 0x1140ffff) AM_RAM AM_SHARE("lumaram")     // polygon "luma" RAM (2b/2c)

	AM_RANGE(0x01c00000, 0x01c00003) AM_READ_PORT("1c00000") AM_WRITE(ctrl0_w )
	AM_RANGE(0x01c00004, 0x01c00007) AM_READ_PORT("1c00004")
	AM_RANGE(0x01c00010, 0x01c00013) AM_READ_PORT("1c00010")
	AM_RANGE(0x01c00014, 0x01c00017) AM_READ_PORT("1c00014") AM_WRITE(hotd_lightgun_w)
	AM_RANGE(0x01c00018, 0x01c0001b) AM_READ(hotd_lightgun_r)
	AM_RANGE(0x01c0001c, 0x01c0001f) AM_READ_PORT("1c0001c") AM_WRITE(analog_2b_w )
	AM_RANGE(0x01c80000, 0x01c80003) AM_READWRITE(model2_serial_r, model2_serial_w )

	AM_IMPORT_FROM(model2_base_mem)
ADDRESS_MAP_END

/* Input definitions */

#define MODEL2_PLAYER_INPUTS(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_)

static INPUT_PORTS_START( model2 )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c00000_r, NULL)

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")

	PORT_START("1c0000c")
	PORT_BIT( 0xfffffff7, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0x0000001a, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be high
	PORT_BIT( 0x0000ffe5, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c0001c_r, NULL)

	PORT_START("IN0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1P Push Switch") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2P Push Switch") PORT_CODE(KEYCODE_8)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	MODEL2_PLAYER_INPUTS(1, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	MODEL2_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( desert )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "STEER")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "ACCEL")

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "BRAKE")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0xfff7ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) // VR 1 (Blue)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // VR 2 (Green)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) // VR 3 (Red)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // shift
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)  // machine gun
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)  // cannon
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	MODEL2_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("BRAKE")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( vcop )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0xfff7ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Trigger")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Trigger")
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED ) // <- one bit here enables "debug mode"

	PORT_START("P1_X")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(1)

	PORT_START("P1_Y")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(1)

	PORT_START("P2_X")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(2)

	PORT_START("P2_Y")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( daytona )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "STEER")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "ACCEL")

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "BRAKE")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0xfff7ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")


	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) // VR 1 (Red)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // VR 2 (Blue)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) // VR 3 (Yellow)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) // VR 4 (Green)
	PORT_BIT(0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // shift 3
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // shift 4
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)

	PORT_START("IN2")
	MODEL2_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("BRAKE")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( manxtt )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c00000_r, NULL)

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN3")

	PORT_START("1c0000c")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN4")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0x0000001a, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be high
	PORT_BIT( 0x0000ffe5, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c0001c_r, NULL)

	PORT_START("IN0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / VR")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shift up in Manx TT, Punch in Motoraid
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) // shift down in Manx TT, Kick in Motoraid
	PORT_BIT(0xffcf, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANA0")  // throttle
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ANA1")  // brake
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ANA2")  // bank
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( srallyc )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c00000_r, NULL)

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN3")

	PORT_START("1c0000c")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN4")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0x0000001a, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be high
	PORT_BIT( 0x0000ffe5, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c0001c_r, NULL)

	PORT_START("IN0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("VR") // VR
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x0090, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT(0x0070, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,srallyc_gearbox_r, NULL)
	PORT_BIT(0x008f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GEARS") // fake to handle gear bits
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("GEAR N")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("GEAR 1")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("GEAR 2")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("GEAR 3")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("GEAR 4")

	PORT_START("IN3")
	//PORT_BIT(0x00ff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_REVERSE PORT_NAME("Handle (Drive)")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("ANA0")  // steer
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ANA1")  // accel
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ANA2")  // brake
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( vcop2 )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c00000_r, NULL)

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN3")

	PORT_START("1c0000c")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN4")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0x0000001a, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be high
	PORT_BIT( 0x0000ffe5, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c0001c_r, NULL)

	PORT_START("IN0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Trigger")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Trigger")
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_X")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(1)

	PORT_START("P1_Y")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(1)

	PORT_START("P2_X")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(2)

	PORT_START("P2_Y")
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0, 0x3ff ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER(2)

INPUT_PORTS_END

static INPUT_PORTS_START( bel )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c00000_r, NULL)

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0x0000001a, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be high
	PORT_BIT( 0x0000ffe5, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c0001c_r, NULL)

	PORT_START("IN0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1P Push Switch") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2P Push Switch") PORT_CODE(KEYCODE_8)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	MODEL2_PLAYER_INPUTS(1, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	MODEL2_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( rchase2 )
	PORT_INCLUDE( model2 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // 1p shot
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // 2p shot
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,rchase2_devices_r, NULL)

	/* FIXME: don't know yet if min max values are really correct, we'll see ... */
	PORT_START("ANA0")
	PORT_BIT( 0x00ff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0x80,0x7f) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("ANA1")
	PORT_BIT( 0x00ff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0x80,0x7f) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("ANA2")
	PORT_BIT( 0x00ff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("ANA3")
	PORT_BIT( 0x00ff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( skytargt )
	PORT_START("1c00000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c00000_r, NULL)

	PORT_START("1c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")

	PORT_START("1c0000c")
	PORT_BIT( 0xfffffff7, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("1c00010")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("1c00014")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("1c0001c")
	PORT_BIT( 0x0000001a, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be high
	PORT_BIT( 0x0000ffe5, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, model2_state,_1c0001c_r, NULL)

	PORT_START("IN0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Machine Gun")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Missile")
	PORT_BIT(0x00c0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	MODEL2_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(model2_state::model2_interrupt)
{
	int scanline = param;

	if(scanline == 384)
	{
		m_intreq |= (1<<0);
		if(m_intena & 1<<0)
			m_maincpu->set_input_line(I960_IRQ0, ASSERT_LINE);
		model2_check_irq_state();
	}
	else if(scanline == 0)
	{
		/* From sound to main CPU (TODO: what enables this?) */
		m_intreq |= (1<<10);
		if(m_intena & 1<<10)
			m_maincpu->set_input_line(I960_IRQ3, ASSERT_LINE);
		model2_check_irq_state();
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(model2_state::model2c_interrupt)
{
	int scanline = param;

	if(scanline == 384)
	{
		m_intreq |= (1<<0);
		if(m_intena & 1<<0)
			m_maincpu->set_input_line(I960_IRQ0, ASSERT_LINE);
		model2_check_irq_state();
	}
	else if(scanline == 0) // 384
	{
		m_intreq |= (1<<10);
		if(m_intena & 1<<10)
			m_maincpu->set_input_line(I960_IRQ3, ASSERT_LINE);
		model2_check_irq_state();
	}
	else if(scanline == 256)
	{
		/* TODO: irq source? Scroll allocation in dynamcopc? */
		m_intreq |= (1<<2);
		if(m_intena & 1<<2)
			m_maincpu->set_input_line(I960_IRQ2, ASSERT_LINE);
		model2_check_irq_state();
	}


}

/* Model 2 sound board emulation */

WRITE16_MEMBER(model2_state::model2snd_ctrl)
{
	// handle sample banking
	if (memregion("scsp")->bytes() > 0x800000)
	{
		UINT8 *snd = memregion("scsp")->base();
		if (data & 0x20)
		{
			membank("bank4")->set_base(snd + 0x200000);
			membank("bank5")->set_base(snd + 0x600000);
		}
		else
		{
			membank("bank4")->set_base(snd + 0x800000);
			membank("bank5")->set_base(snd + 0xa00000);
		}
	}
}

static ADDRESS_MAP_START( model2_snd, AS_PROGRAM, 16, model2_state )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_REGION("audiocpu", 0) AM_SHARE("soundram")
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE("scsp", scsp_device, read, write)
	AM_RANGE(0x400000, 0x400001) AM_WRITE(model2snd_ctrl)
	AM_RANGE(0x600000, 0x67ffff) AM_ROM AM_REGION("audiocpu", 0x80000)
	AM_RANGE(0x800000, 0x9fffff) AM_ROM AM_REGION("scsp", 0)
	AM_RANGE(0xa00000, 0xdfffff) AM_ROMBANK("bank4")
	AM_RANGE(0xe00000, 0xffffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END


WRITE8_MEMBER(model2_state::scsp_irq)
{
	m_audiocpu->set_input_line(offset, data);
}


/*****************************************************************************/
// SHARC memory maps

READ32_MEMBER(model2_state::copro_sharc_input_fifo_r)
{
	UINT32 result = 0;
	bool type;
	//osd_printf_debug("SHARC FIFOIN pop at %08X\n", space.device().safe_pc());

	type = copro_fifoin_pop(machine().device("dsp"), &result,offset,mem_mask);
	if(type == false)
		printf("Read from empty FIFO?\n");
	return result;
}

WRITE32_MEMBER(model2_state::copro_sharc_output_fifo_w)
{
	//osd_printf_debug("SHARC FIFOOUT push %08X\n", data);
	copro_fifoout_push(machine().device("dsp"), data,offset,mem_mask);
}

READ32_MEMBER(model2_state::copro_sharc_buffer_r)
{
	return m_bufferram[offset & 0x7fff];
}

WRITE32_MEMBER(model2_state::copro_sharc_buffer_w)
{
	//osd_printf_debug("sharc_buffer_w: %08X at %08X, %08X, %f\n", offset, space.device().safe_pc(), data, *(float*)&data);
	m_bufferram[offset & 0x7fff] = data;
}

static ADDRESS_MAP_START( copro_sharc_map, AS_DATA, 32, model2_state )
	AM_RANGE(0x0400000, 0x0bfffff) AM_READ(copro_sharc_input_fifo_r)
	AM_RANGE(0x0c00000, 0x13fffff) AM_WRITE(copro_sharc_output_fifo_w)
	AM_RANGE(0x1400000, 0x1bfffff) AM_READWRITE(copro_sharc_buffer_r, copro_sharc_buffer_w)
	AM_RANGE(0x1c00000, 0x1dfffff) AM_ROM AM_REGION("user5", 0)
ADDRESS_MAP_END

#if 0
static ADDRESS_MAP_START( geo_sharc_map, AS_DATA, 32, model2_state )
ADDRESS_MAP_END
#endif

/*****************************************************************************/
/* TGP memory maps */

READ32_MEMBER(model2_state::copro_tgp_buffer_r)
{
	return m_bufferram[offset & 0x7fff];
}

WRITE32_MEMBER(model2_state::copro_tgp_buffer_w)
{
	m_bufferram[offset&0x7fff] = data;
}

static ADDRESS_MAP_START( copro_tgp_map, AS_PROGRAM, 32, model2_state )
	AM_RANGE(0x00000000, 0x00007fff) AM_RAM AM_SHARE("tgp_program")
	AM_RANGE(0x00400000, 0x00407fff) AM_READWRITE(copro_tgp_buffer_r, copro_tgp_buffer_w)
	AM_RANGE(0x00800000, 0x009fffff) AM_ROM AM_REGION("tgp", 0)
	AM_RANGE(0xff800000, 0xff9fffff) AM_ROM AM_REGION("tgp", 0)
ADDRESS_MAP_END

/*****************************************************************************/

#define VIDEO_CLOCK         XTAL_32MHz

/* original Model 2 */
static MACHINE_CONFIG_START( model2o, model2_state )
	MCFG_CPU_ADD("maincpu", I960, 25000000)
	MCFG_CPU_PROGRAM_MAP(model2o_mem)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", model2_state, model2_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("tgp", MB86233, 16000000)
	MCFG_CPU_PROGRAM_MAP(copro_tgp_map)
	MCFG_MB86233_FIFO_READ_CB(READ32(model2_state,copro_tgp_fifoin_pop))
	MCFG_MB86233_FIFO_READ_OK_CB(READLINE(model2_state,copro_tgp_fifoin_pop_ok))
	MCFG_MB86233_FIFO_WRITE_CB(WRITE32(model2_state,copro_tgp_fifoout_push))
	MCFG_MB86233_TABLE_REGION("user5")

	MCFG_MACHINE_START_OVERRIDE(model2_state,model2)
	MCFG_MACHINE_RESET_OVERRIDE(model2_state,model2o)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_NVRAM_ADD_1FILL("backup1")
	MCFG_NVRAM_ADD_1FILL("backup2")

	MCFG_TIMER_DRIVER_ADD("timer0", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)0)
	MCFG_TIMER_DRIVER_ADD("timer1", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)1)
	MCFG_TIMER_DRIVER_ADD("timer2", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)2)
	MCFG_TIMER_DRIVER_ADD("timer3", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_S24TILE_DEVICE_ADD("tile", 0x3fff)
	MCFG_S24TILE_DEVICE_GFXDECODE("gfxdecode")
	MCFG_S24TILE_DEVICE_PALETTE("palette")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 656, 0/*+69*/, 496/*+69*/, 424, 0/*+25*/, 384/*+25*/) // TODO: from System 24, might not be accurate for Model 2
	MCFG_SCREEN_UPDATE_DRIVER(model2_state, screen_update_model2)

	MCFG_PALETTE_ADD("palette", 8192)

	MCFG_VIDEO_START_OVERRIDE(model2_state,model2)

	MCFG_SEGAM1AUDIO_ADD("m1audio")
MACHINE_CONFIG_END

/* 2A-CRX */
static MACHINE_CONFIG_START( model2a, model2_state )
	MCFG_CPU_ADD("maincpu", I960, 25000000)
	MCFG_CPU_PROGRAM_MAP(model2a_crx_mem)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", model2_state, model2_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(model2_snd)

	MCFG_CPU_ADD("tgp", MB86233, 16000000)
	MCFG_CPU_PROGRAM_MAP(copro_tgp_map)
	MCFG_MB86233_FIFO_READ_CB(READ32(model2_state,copro_tgp_fifoin_pop))
	MCFG_MB86233_FIFO_READ_OK_CB(READLINE(model2_state,copro_tgp_fifoin_pop_ok))
	MCFG_MB86233_FIFO_WRITE_CB(WRITE32(model2_state,copro_tgp_fifoout_push))
	MCFG_MB86233_TABLE_REGION("user5")

	MCFG_MACHINE_START_OVERRIDE(model2_state,model2)
	MCFG_MACHINE_RESET_OVERRIDE(model2_state,model2)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_NVRAM_ADD_1FILL("backup1")

	MCFG_TIMER_DRIVER_ADD("timer0", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)0)
	MCFG_TIMER_DRIVER_ADD("timer1", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)1)
	MCFG_TIMER_DRIVER_ADD("timer2", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)2)
	MCFG_TIMER_DRIVER_ADD("timer3", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_S24TILE_DEVICE_ADD("tile", 0x3fff)
	MCFG_S24TILE_DEVICE_GFXDECODE("gfxdecode")
	MCFG_S24TILE_DEVICE_PALETTE("palette")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK )
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 656, 0/*+69*/, 496/*+69*/, 424, 0/*+25*/, 384/*+25*/) // TODO: from System 24, might not be accurate for Model 2
	MCFG_SCREEN_UPDATE_DRIVER(model2_state, screen_update_model2)

	MCFG_PALETTE_ADD("palette", 8192)

	MCFG_VIDEO_START_OVERRIDE(model2_state,model2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("scsp", SCSP, 0)
	MCFG_SCSP_IRQ_CB(WRITE8(model2_state,scsp_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 2.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( manxttdx, model2a ) /* Includes a Model 1 Sound board for additional sounds - Deluxe version only */
	MCFG_SEGAM1AUDIO_ADD("m1audio")
MACHINE_CONFIG_END

UINT16 model2_state::crypt_read_callback(UINT32 addr)
{
	UINT16 dat= m_maincpu->space().read_word((0x1d80000+2*addr));
	return ((dat&0xff00)>>8)|((dat&0x00ff)<<8);
}

static MACHINE_CONFIG_DERIVED( model2a_5881, model2a )
	MCFG_DEVICE_ADD("315_5881", SEGA315_5881_CRYPT, 0)
	MCFG_SET_READ_CALLBACK(model2_state, crypt_read_callback)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( model2a_0229, model2a )
	MCFG_DEVICE_ADD("317_0229", SEGA315_5838_COMP, 0)
//  MCFG_SET_5838_READ_CALLBACK(model2_state, crypt_read_callback)
MACHINE_CONFIG_END

READ8_MEMBER(model2_state::driveio_port_r)
{
	return m_driveio_comm_data;
}

WRITE8_MEMBER(model2_state::driveio_port_w)
{
//  TODO: hook up to the main CPU
//  popmessage("%02x",data);
}

READ8_MEMBER(model2_state::driveio_port_str_r)
{
	static const char sega_str[4] = { 'S', 'E', 'G', 'A' };

	return sega_str[offset];
}

static ADDRESS_MAP_START( drive_map, AS_PROGRAM, 8, model2_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( drive_io_map, AS_IO, 8, model2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP //watchdog
	AM_RANGE(0x23, 0x23) AM_WRITE(driveio_port_w)
	AM_RANGE(0x26, 0x27) AM_READ(driveio_port_r)
	AM_RANGE(0x28, 0x2b) AM_READ(driveio_port_str_r)
	AM_RANGE(0x40, 0x4f) AM_WRITENOP //Oki M6253
	AM_RANGE(0x80, 0x83) AM_NOP //r/w it during irq
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED( srallyc, model2a )

	MCFG_CPU_ADD("drivecpu", Z80, 16000000/4) //???
	MCFG_CPU_PROGRAM_MAP(drive_map)
	MCFG_CPU_IO_MAP(drive_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", model2_state,  irq0_line_hold)
MACHINE_CONFIG_END

/* 2B-CRX */
static MACHINE_CONFIG_START( model2b, model2_state )
	MCFG_CPU_ADD("maincpu", I960, 25000000)
	MCFG_CPU_PROGRAM_MAP(model2b_crx_mem)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", model2_state, model2_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(model2_snd)

	MCFG_CPU_ADD("dsp", ADSP21062, 40000000)
	MCFG_SHARC_BOOT_MODE(BOOT_MODE_HOST)
	MCFG_CPU_DATA_MAP(copro_sharc_map)

	//MCFG_CPU_ADD("dsp2", ADSP21062, 40000000)
	//MCFG_SHARC_BOOT_MODE(BOOT_MODE_HOST)
	//MCFG_CPU_DATA_MAP(geo_sharc_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(18000))

	MCFG_MACHINE_START_OVERRIDE(model2_state,model2)
	MCFG_MACHINE_RESET_OVERRIDE(model2_state,model2b)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_NVRAM_ADD_1FILL("backup1")

	MCFG_TIMER_DRIVER_ADD("timer0", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)0)
	MCFG_TIMER_DRIVER_ADD("timer1", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)1)
	MCFG_TIMER_DRIVER_ADD("timer2", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)2)
	MCFG_TIMER_DRIVER_ADD("timer3", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_S24TILE_DEVICE_ADD("tile", 0x3fff)
	MCFG_S24TILE_DEVICE_GFXDECODE("gfxdecode")
	MCFG_S24TILE_DEVICE_PALETTE("palette")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK )
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 656, 0/*+69*/, 496/*+69*/, 424, 0/*+25*/, 384/*+25*/) // TODO: from System 24, might not be accurate for Model 2
	MCFG_SCREEN_UPDATE_DRIVER(model2_state, screen_update_model2)

	MCFG_PALETTE_ADD("palette", 8192)

	MCFG_VIDEO_START_OVERRIDE(model2_state,model2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("scsp", SCSP, 0)
	MCFG_SCSP_IRQ_CB(WRITE8(model2_state,scsp_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 2.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( model2b_5881, model2b )
	MCFG_DEVICE_ADD("315_5881", SEGA315_5881_CRYPT, 0)
	MCFG_SET_READ_CALLBACK(model2_state, crypt_read_callback)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( model2b_0229, model2b )
	MCFG_DEVICE_ADD("317_0229", SEGA315_5838_COMP, 0)
//  MCFG_SET_5838_READ_CALLBACK(model2_state, crypt_read_callback)
MACHINE_CONFIG_END


static ADDRESS_MAP_START( copro_tgpx4_map, AS_PROGRAM, 32, model2_state )
	AM_RANGE(0x00000000, 0x00007fff) AM_RAM AM_SHARE("tgpx4_program")
ADDRESS_MAP_END

/* 2C-CRX */
static MACHINE_CONFIG_START( model2c, model2_state )
	MCFG_CPU_ADD("maincpu", I960, 25000000)
	MCFG_CPU_PROGRAM_MAP(model2c_crx_mem)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", model2_state, model2c_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("tgpx4", MB86235, 40000000)
	MCFG_CPU_PROGRAM_MAP(copro_tgpx4_map)

	MCFG_CPU_ADD("audiocpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(model2_snd)

	MCFG_MACHINE_START_OVERRIDE(model2_state,model2)
	MCFG_MACHINE_RESET_OVERRIDE(model2_state,model2c)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_NVRAM_ADD_1FILL("backup1")

	MCFG_TIMER_DRIVER_ADD("timer0", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)0)
	MCFG_TIMER_DRIVER_ADD("timer1", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)1)
	MCFG_TIMER_DRIVER_ADD("timer2", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)2)
	MCFG_TIMER_DRIVER_ADD("timer3", model2_state, model2_timer_cb)
	MCFG_TIMER_PTR((FPTR)3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_S24TILE_DEVICE_ADD("tile", 0x3fff)
	MCFG_S24TILE_DEVICE_GFXDECODE("gfxdecode")
	MCFG_S24TILE_DEVICE_PALETTE("palette")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK )
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 656, 0/*+69*/, 496/*+69*/, 424, 0/*+25*/, 384/*+25*/) // TODO: from System 24, might not be accurate for Model 2
	MCFG_SCREEN_UPDATE_DRIVER(model2_state, screen_update_model2)

	MCFG_PALETTE_ADD("palette", 8192)

	MCFG_VIDEO_START_OVERRIDE(model2_state,model2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("scsp", SCSP, 0)
	MCFG_SCSP_IRQ_CB(WRITE8(model2_state, scsp_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 2.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( stcc, model2c )
	MCFG_DSBZ80_ADD(DSBZ80_TAG)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( model2c_5881, model2c )
	MCFG_DEVICE_ADD("315_5881", SEGA315_5881_CRYPT, 0)
	MCFG_SET_READ_CALLBACK(model2_state, crypt_read_callback)
MACHINE_CONFIG_END


/* ROM definitions */

/*
(info from 2a)

The smt ROMs are located on the CPU board and are labelled....
OPR-14742A \
OPR-14743A /  Linked to 315-5674
OPR-14744    \
OPR-14745    /  Linked to 315-5679B
OPR-14746    \
OPR-14747    /  Linked to 315-5679B

*/

#define MODEL2_CPU_BOARD \
	ROM_REGION( 0xc0000, "user5", 0 ) \
	ROM_LOAD32_WORD("opr-14742a.45",  0x000000,  0x20000, CRC(90c6b117) SHA1(f46429fffcee17d056f56d5fe035a33f1fd6c27e) ) \
	ROM_LOAD32_WORD("opr-14743a.46",  0x000002,  0x20000, CRC(ae7f446b) SHA1(5b9f1fc47caf21e061e930c0d72804e4ec8c7bca) ) \
	ROM_LOAD("opr-14744.58",   0x040000,  0x20000, CRC(730ea9e0) SHA1(651f1db4089a400d073b19ada299b4b08b08f372) ) \
	ROM_LOAD("opr-14745.59",   0x060000,  0x20000, CRC(4c934d96) SHA1(e3349ece0e47f684d61ad11bfea4a90602287350) ) \
	ROM_LOAD("opr-14746.62",   0x080000,  0x20000, CRC(2a266cbd) SHA1(34e047a93459406c22acf4c25089d1a4955f94ca) ) \
	ROM_LOAD("opr-14747.63",   0x0a0000,  0x20000, CRC(a4ad5e19) SHA1(7d7ec300eeb9a8de1590011e37108688c092f329) )
/*
These are smt ROMs found on Sega Model 2A Video board
They are linked to a QFP208 IC labelled 315-5645
*/

#define MODEL2A_VID_BOARD \
	ROM_REGION( 0x180000, "user6", 0 ) \
	ROM_LOAD("mpr-16310.15",   0x000000,  0x80000, CRC(c078a780) SHA1(0ad5b49774172743e2708b7ca4c061acfe10957a) ) \
	ROM_LOAD("mpr-16311.16",   0x080000,  0x80000, CRC(452a492b) SHA1(88c2f6c2dbfd0c1b39a7bf15c74455fb68c7274e) ) \
	ROM_LOAD("mpr-16312.14",   0x100000,  0x80000, CRC(a25fef5b) SHA1(c6a37856b97f5bc4996cb6b66209f47af392cc38) )

/* Is there an undumped Zero Gunner with program roms EPR-20292 & EPR-20293? Numbering would suggest so, Japan Model2C or Model2A US? */
ROM_START( zeroguna ) /* Zero Gunner (Export), Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20437.12", 0x000000, 0x080000, CRC(fad30cc0) SHA1(5c6222e07594b4be59b5095f7cc0a164d5895306) )
	ROM_LOAD32_WORD("epr-20438.13", 0x000002, 0x080000, CRC(ca364408) SHA1(4672ebdd7d9ccab5e107fda9d322b70583246c7a) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20296.11", 0x000000, 0x400000, CRC(072d8a5e) SHA1(7f69c90dd3c3e6e522d1065b3c4b09434cb4e634) )
	ROM_LOAD32_WORD("mpr-20297.12", 0x000002, 0x400000, CRC(ba6a825b) SHA1(670a86c3a1a78550c760cc66c0a6181928fb9054) )
	ROM_LOAD32_WORD("mpr-20294.9",  0x800000, 0x400000, CRC(a0bd1474) SHA1(c0c032adac69bd545e3aab481878b08f3c3edab8) )
	ROM_LOAD32_WORD("mpr-20295.10", 0x800002, 0x400000, CRC(c548cced) SHA1(d34f2fc9b4481c75a6824aa4bdd3f1884188d35b) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20298.17", 0x000000, 0x400000, CRC(8ab782fc) SHA1(595f6fc2e9c58ce9763d51798ceead8d470f0a33) )
	ROM_LOAD32_WORD("mpr-20299.21", 0x000002, 0x400000, CRC(90e20cdb) SHA1(730d58286fb7e91aa4128dc208b0f60eb3becc78) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20301.27", 0x000000, 0x200000, CRC(52010fb2) SHA1(8dce67c6f9e48d749c64b11d4569df413dc40e07) )
	ROM_LOAD32_WORD("mpr-20300.25", 0x000002, 0x200000, CRC(6f042792) SHA1(75db68e57ec3fbc7af377342eef81f26fae4e1c4) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20302.31",  0x080000,  0x80000, CRC(44ff50d2) SHA1(6ffec81042fd5708e8a5df47b63f9809f93bf0f8) )

	ROM_REGION( 0x400000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20303.32", 0x000000, 0x200000, CRC(c040973f) SHA1(57a496c5dcc1a3931b6e41bf8d41e45d6dac0c31) )
	ROM_LOAD("mpr-20304.33", 0x200000, 0x200000, CRC(6decfe83) SHA1(d73adafceff2f1776c93e53bd5677d67f1c2c08f) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD

	//             1997     317-5038-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "042c0d13" )
ROM_END

ROM_START( zerogunaj ) /* Zero Gunner (Japan), Model 2A - Sega game ID# 833-11341, Sega ROM board ID# 834-11342 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20288.12", 0x000000, 0x080000, CRC(162305d5) SHA1(c0d67fbb8f89daacd32bbc1ad0d55a73b60016d8) )
	ROM_LOAD32_WORD("epr-20289.13", 0x000002, 0x080000, CRC(b5acb940) SHA1(e4c66c6bc9d5433b76ea12cf625fc359439144bb) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20296.11", 0x000000, 0x400000, CRC(072d8a5e) SHA1(7f69c90dd3c3e6e522d1065b3c4b09434cb4e634) )
	ROM_LOAD32_WORD("mpr-20297.12", 0x000002, 0x400000, CRC(ba6a825b) SHA1(670a86c3a1a78550c760cc66c0a6181928fb9054) )
	ROM_LOAD32_WORD("mpr-20294.9",  0x800000, 0x400000, CRC(a0bd1474) SHA1(c0c032adac69bd545e3aab481878b08f3c3edab8) )
	ROM_LOAD32_WORD("mpr-20295.10", 0x800002, 0x400000, CRC(c548cced) SHA1(d34f2fc9b4481c75a6824aa4bdd3f1884188d35b) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20298.17", 0x000000, 0x400000, CRC(8ab782fc) SHA1(595f6fc2e9c58ce9763d51798ceead8d470f0a33) )
	ROM_LOAD32_WORD("mpr-20299.21", 0x000002, 0x400000, CRC(90e20cdb) SHA1(730d58286fb7e91aa4128dc208b0f60eb3becc78) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20301.27", 0x000000, 0x200000, CRC(52010fb2) SHA1(8dce67c6f9e48d749c64b11d4569df413dc40e07) )
	ROM_LOAD32_WORD("mpr-20300.25", 0x000002, 0x200000, CRC(6f042792) SHA1(75db68e57ec3fbc7af377342eef81f26fae4e1c4) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20302.31",  0x080000,  0x80000, CRC(44ff50d2) SHA1(6ffec81042fd5708e8a5df47b63f9809f93bf0f8) )

	ROM_REGION( 0x400000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20303.32", 0x000000, 0x200000, CRC(c040973f) SHA1(57a496c5dcc1a3931b6e41bf8d41e45d6dac0c31) )
	ROM_LOAD("mpr-20304.33", 0x200000, 0x200000, CRC(6decfe83) SHA1(d73adafceff2f1776c93e53bd5677d67f1c2c08f) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD

	//             1997     317-5038-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "042c0d13" )
ROM_END

ROM_START( zerogun ) /* Zero Gunner (Export), Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20439.15", 0x000000, 0x080000, CRC(10125381) SHA1(1e178e6bd2b1312cd6290f1be4b386f520465836) )
	ROM_LOAD32_WORD("epr-20440.16", 0x000002, 0x080000, CRC(ce872747) SHA1(82bf138a42c659b675b14e41d526b1628fb46ae3) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20296.11", 0x000000, 0x400000, CRC(072d8a5e) SHA1(7f69c90dd3c3e6e522d1065b3c4b09434cb4e634) )
	ROM_LOAD32_WORD("mpr-20297.12", 0x000002, 0x400000, CRC(ba6a825b) SHA1(670a86c3a1a78550c760cc66c0a6181928fb9054) )
	ROM_LOAD32_WORD("mpr-20294.9",  0x800000, 0x400000, CRC(a0bd1474) SHA1(c0c032adac69bd545e3aab481878b08f3c3edab8) )
	ROM_LOAD32_WORD("mpr-20295.10", 0x800002, 0x400000, CRC(c548cced) SHA1(d34f2fc9b4481c75a6824aa4bdd3f1884188d35b) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20298.17", 0x000000, 0x400000, CRC(8ab782fc) SHA1(595f6fc2e9c58ce9763d51798ceead8d470f0a33) )
	ROM_LOAD32_WORD("mpr-20299.21", 0x000002, 0x400000, CRC(90e20cdb) SHA1(730d58286fb7e91aa4128dc208b0f60eb3becc78) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20301.27", 0x000000, 0x200000, CRC(52010fb2) SHA1(8dce67c6f9e48d749c64b11d4569df413dc40e07) )
	ROM_LOAD32_WORD("mpr-20300.25", 0x000002, 0x200000, CRC(6f042792) SHA1(75db68e57ec3fbc7af377342eef81f26fae4e1c4) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20302.31",  0x080000,  0x80000, CRC(44ff50d2) SHA1(6ffec81042fd5708e8a5df47b63f9809f93bf0f8) )

	ROM_REGION( 0x400000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20303.32", 0x000000, 0x200000, CRC(c040973f) SHA1(57a496c5dcc1a3931b6e41bf8d41e45d6dac0c31) )
	ROM_LOAD("mpr-20304.33", 0x200000, 0x200000, CRC(6decfe83) SHA1(d73adafceff2f1776c93e53bd5677d67f1c2c08f) )

	//             1997     317-5038-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "042c0d13" )
ROM_END

ROM_START( zerogunj ) /* Zero Gunner (Japan), Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20290.15", 0x000000, 0x080000, CRC(9ce3ad21) SHA1(812ab45cc9e2920e74e58937d1826774f3f54183) )
	ROM_LOAD32_WORD("epr-20291.16", 0x000002, 0x080000, CRC(7267a03d) SHA1(a7216914ee7535fa1856cb19bc05c89948a93c89) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20296.11", 0x000000, 0x400000, CRC(072d8a5e) SHA1(7f69c90dd3c3e6e522d1065b3c4b09434cb4e634) )
	ROM_LOAD32_WORD("mpr-20297.12", 0x000002, 0x400000, CRC(ba6a825b) SHA1(670a86c3a1a78550c760cc66c0a6181928fb9054) )
	ROM_LOAD32_WORD("mpr-20294.9",  0x800000, 0x400000, CRC(a0bd1474) SHA1(c0c032adac69bd545e3aab481878b08f3c3edab8) )
	ROM_LOAD32_WORD("mpr-20295.10", 0x800002, 0x400000, CRC(c548cced) SHA1(d34f2fc9b4481c75a6824aa4bdd3f1884188d35b) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20298.17", 0x000000, 0x400000, CRC(8ab782fc) SHA1(595f6fc2e9c58ce9763d51798ceead8d470f0a33) )
	ROM_LOAD32_WORD("mpr-20299.21", 0x000002, 0x400000, CRC(90e20cdb) SHA1(730d58286fb7e91aa4128dc208b0f60eb3becc78) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20301.27", 0x000000, 0x200000, CRC(52010fb2) SHA1(8dce67c6f9e48d749c64b11d4569df413dc40e07) )
	ROM_LOAD32_WORD("mpr-20300.25", 0x000002, 0x200000, CRC(6f042792) SHA1(75db68e57ec3fbc7af377342eef81f26fae4e1c4) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20302.31",  0x080000,  0x80000, CRC(44ff50d2) SHA1(6ffec81042fd5708e8a5df47b63f9809f93bf0f8) )

	ROM_REGION( 0x400000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20303.32", 0x000000, 0x200000, CRC(c040973f) SHA1(57a496c5dcc1a3931b6e41bf8d41e45d6dac0c31) )
	ROM_LOAD("mpr-20304.33", 0x200000, 0x200000, CRC(6decfe83) SHA1(d73adafceff2f1776c93e53bd5677d67f1c2c08f) )

	//             1997     317-5038-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "042c0d13" )
ROM_END

ROM_START( gunblade ) /* Gunblade NY Revision A, Model 2B, Sega game ID# 833-12562 GUN BLADE, Sega ROM board ID# 834-12563 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18988a.15", 0x000000, 0x080000, CRC(f63f1ad2) SHA1(fcfb0a4691cd7d66168c421e4e1694ecaea56ab2) )
	ROM_LOAD32_WORD("epr-18989a.16", 0x000002, 0x080000, CRC(c1c84d65) SHA1(92bffbf1250c53499c37a53f9e2a054fc7bf256f) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18974.11",  0x000000, 0x400000, CRC(e29ecaff) SHA1(dcdfe9f59158cec2f02b213ee13f5e40cdb92e55) )
	ROM_LOAD32_WORD("mpr-18975.12",  0x000002, 0x400000, CRC(d8187582) SHA1(34a0b32eeed1a9f41bca8b9261851881b2ba79f2) )
	ROM_LOAD32_WORD("mpr-18976.9",   0x800000, 0x400000, CRC(c95c15eb) SHA1(892063e91b2ed20e0600d4b188da1e9f45a19692) )
	ROM_LOAD32_WORD("mpr-18977.10",  0x800002, 0x400000, CRC(db8f5b6f) SHA1(c11d2c9e1e215aa7b2ebb777639c8cd651901f52) )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("mpr-18986.29",  0x000000, 0x400000, CRC(04820f7b) SHA1(5eb6682399b358d77658d82e612b02b724e3f3e1) )
	ROM_LOAD32_WORD("mpr-18987.30",  0x000002, 0x400000, CRC(2419367f) SHA1(0a04a1049d2da486dc9dbb97b383bd24259b78c8) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18980.17",  0x000000, 0x400000, CRC(22345534) SHA1(7b8bdcfe88953ce1b2d75af2ce4712ab6507e2cf) )
	ROM_LOAD32_WORD("mpr-18981.21",  0x000002, 0x400000, CRC(2544a33d) SHA1(a76193f70adb6abeba02328b290af5cca47d4e25) )
	ROM_LOAD32_WORD("mpr-18982.18",  0x800000, 0x400000, CRC(d0a92b2a) SHA1(95404baed88cc95b75ff9b9084d09622961d3e57) )
	ROM_LOAD32_WORD("mpr-18983.22",  0x800002, 0x400000, CRC(1b4af982) SHA1(550f8248699b9267da7d2e64002be56972381714) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18985.27", 0x000000, 0x400000, CRC(ad6166e3) SHA1(2c487fb743730cacf92dbea952b1efada0f073df) )
	ROM_LOAD32_WORD("mpr-18984.25", 0x000002, 0x400000, CRC(756f6f37) SHA1(095964de773f515d64d65dbc8f8ef9bae97e5ba9) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18990.31", 0x080000,  0x80000, CRC(02b1b0d1) SHA1(759b4683dc7149e04f41ddac7bd395e8d07ea858) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18978.32", 0x000000, 0x400000, CRC(0f78b3e3) SHA1(6c2cd6236cb001bb8d487a9b1e9907519dc43daa) )
	ROM_LOAD("mpr-18979.34", 0x400000, 0x400000, CRC(f13ea36f) SHA1(a8165116b5e07e031ff960201dd8c9a441544961) )
ROM_END

ROM_START( vf2 ) /* Virtua Fighter 2 Version 2.1, Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-18385.12",    0x000000, 0x020000, CRC(78ed2d41) SHA1(471c19389ceeec6138107dd81863320bd4825327) )
	ROM_LOAD32_WORD( "epr-18386.13",    0x000002, 0x020000, CRC(3418f428) SHA1(0f51e389e13efc172a26471331a60c459ad43c38) )
	ROM_LOAD32_WORD( "epr-18387.14",    0x040000, 0x020000, CRC(124a8453) SHA1(26fb787451824fc6060724e37fe0ba6bb66796cb) )
	ROM_LOAD32_WORD( "epr-18388.15",    0x040002, 0x020000, CRC(8d347980) SHA1(da79e51ad501b9560c4ed7cf1ec768daad93efe0) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17560.10", 0x000000, 0x200000, CRC(d1389864) SHA1(88e9a8b6b0f58c96957015179e7ff10f837040e6) )
	ROM_LOAD32_WORD( "mpr-17561.11", 0x000002, 0x200000, CRC(b98d0101) SHA1(e154877380b9250d8119dd4c14ba306c7b337dcd) )
	ROM_LOAD32_WORD( "mpr-17558.8",  0x400000, 0x200000, CRC(4b15f5a6) SHA1(9a34724958fef9b49eae39c6ea136e0cf532154b) )
	ROM_LOAD32_WORD( "mpr-17559.9",  0x400002, 0x200000, CRC(d3264de6) SHA1(2f094ff0b95bf1cd5c283414634ea9597204d374) )
	ROM_LOAD32_WORD( "mpr-17566.6",  0x800000, 0x200000, CRC(fb41ef98) SHA1(ad4d1ba5e5b39b2d87105ae80750284867aa4ed3) )
	ROM_LOAD32_WORD( "mpr-17567.7",  0x800002, 0x200000, CRC(c3396922) SHA1(7e0700ded530e4eb58e9a68cdb92791284c91431) )
	ROM_LOAD32_WORD( "mpr-17564.4",  0xc00000, 0x200000, CRC(d8062489) SHA1(57666b6937f79bb65c43ed02b04a454882d01e61) )
	ROM_LOAD32_WORD( "mpr-17565.5",  0xc00002, 0x200000, CRC(0517c6e9) SHA1(d9ba93998286713758385033119416714674c8d8) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17554.16", 0x000000, 0x200000, CRC(27896d82) SHA1(c0624e58de2e427465daaa10dbb02ea2a1fd0f1b) )
	ROM_LOAD32_WORD( "mpr-17548.20", 0x000002, 0x200000, CRC(c95facc2) SHA1(09d19abe5d75a335df7510df8abb2d4425159cdf) )
	ROM_LOAD32_WORD( "mpr-17555.17", 0x400000, 0x200000, CRC(4df2810b) SHA1(720c4628d7783f0323b5723b441e13741556241e) )
	ROM_LOAD32_WORD( "mpr-17549.21", 0x400002, 0x200000, CRC(e0bce0e6) SHA1(0570604dc2007288795a3125ffd480bc4b3b0802) )
	ROM_LOAD32_WORD( "mpr-17556.18", 0x800000, 0x200000, CRC(41a47616) SHA1(55b909d2bc2079d0dfed5036c78c9e09bce09843) )
	ROM_LOAD32_WORD( "mpr-17550.22", 0x800002, 0x200000, CRC(c36ff3f5) SHA1(f14fdf275905a90a0d4cc534d90b0302f26676d8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17553.25", 0x000000, 0x200000, CRC(5da1c5d3) SHA1(c627b25a1f61a9fe9182e2199f70f6e485503c7b) )
	ROM_LOAD32_WORD( "mpr-17552.24", 0x000002, 0x200000, CRC(e91e7427) SHA1(0ac1111f2ecb4f924b5119eaaac8fa7bc87ab9d1) )
	ROM_LOAD32_WORD( "mpr-17547.27", 0x400000, 0x200000, CRC(be940431) SHA1(5c1196a6454a4fead79a930979f2e69639ec2bb9) )
	ROM_LOAD32_WORD( "mpr-17546.26", 0x400002, 0x200000, CRC(042a194b) SHA1(c6d8524dc0a879394f1234b7bb04836081bb3830) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-17574.30", 0x080000, 0x080000, CRC(4d4c3a55) SHA1(b6c0c3f0473bd7fc3ef4f5146110dfcc899a5af9) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-17573.31", 0x000000, 0x200000, CRC(e43557fe) SHA1(4c61a135819862df02347c118dc4d88a0adac273) )
	ROM_LOAD( "mpr-17572.32", 0x200000, 0x200000, CRC(4febecc8) SHA1(9683ea9bedfc5cd7b4a28e9a68792c0dc549d911) )
	ROM_LOAD( "mpr-17571.36", 0x400000, 0x200000, CRC(51caa584) SHA1(cbbde1c55eddbeeefd283bb5afd79a670a282e3a) )
	ROM_LOAD( "mpr-17570.37", 0x600000, 0x200000, CRC(bccd324b) SHA1(4c7ebdea08b2dedf621f121785ed1c40ebae4236) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( vf2b ) /* Virtua Fighter 2 Revision B, Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17568b.12", 0x000000, 0x020000, CRC(5d966bbf) SHA1(01d46313148ce509fa5641fb07a3f840c00886ac) )
	ROM_LOAD32_WORD( "epr-17569b.13", 0x000002, 0x020000, CRC(0b8c1ccc) SHA1(ba2e0ac8b31955fed237ba9a5eda9fa14d1db11f) )
	ROM_LOAD32_WORD( "epr-17562b.14", 0x040000, 0x020000, CRC(b778d4eb) SHA1(a7162d9c39d601ac92310c8cf2ae388647a5295a) )
	ROM_LOAD32_WORD( "epr-17563b.15", 0x040002, 0x020000, CRC(a05c15f6) SHA1(b9b1f3c68c53a86dfa3cbc85fcb9150546c13f23) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17560.10", 0x000000, 0x200000, CRC(d1389864) SHA1(88e9a8b6b0f58c96957015179e7ff10f837040e6) )
	ROM_LOAD32_WORD( "mpr-17561.11", 0x000002, 0x200000, CRC(b98d0101) SHA1(e154877380b9250d8119dd4c14ba306c7b337dcd) )
	ROM_LOAD32_WORD( "mpr-17558.8",  0x400000, 0x200000, CRC(4b15f5a6) SHA1(9a34724958fef9b49eae39c6ea136e0cf532154b) )
	ROM_LOAD32_WORD( "mpr-17559.9",  0x400002, 0x200000, CRC(d3264de6) SHA1(2f094ff0b95bf1cd5c283414634ea9597204d374) )
	ROM_LOAD32_WORD( "mpr-17566.6",  0x800000, 0x200000, CRC(fb41ef98) SHA1(ad4d1ba5e5b39b2d87105ae80750284867aa4ed3) )
	ROM_LOAD32_WORD( "mpr-17567.7",  0x800002, 0x200000, CRC(c3396922) SHA1(7e0700ded530e4eb58e9a68cdb92791284c91431) )
	ROM_LOAD32_WORD( "mpr-17564.4",  0xc00000, 0x200000, CRC(d8062489) SHA1(57666b6937f79bb65c43ed02b04a454882d01e61) )
	ROM_LOAD32_WORD( "mpr-17565.5",  0xc00002, 0x200000, CRC(0517c6e9) SHA1(d9ba93998286713758385033119416714674c8d8) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17554.16", 0x000000, 0x200000, CRC(27896d82) SHA1(c0624e58de2e427465daaa10dbb02ea2a1fd0f1b) )
	ROM_LOAD32_WORD( "mpr-17548.20", 0x000002, 0x200000, CRC(c95facc2) SHA1(09d19abe5d75a335df7510df8abb2d4425159cdf) )
	ROM_LOAD32_WORD( "mpr-17555.17", 0x400000, 0x200000, CRC(4df2810b) SHA1(720c4628d7783f0323b5723b441e13741556241e) )
	ROM_LOAD32_WORD( "mpr-17549.21", 0x400002, 0x200000, CRC(e0bce0e6) SHA1(0570604dc2007288795a3125ffd480bc4b3b0802) )
	ROM_LOAD32_WORD( "mpr-17556.18", 0x800000, 0x200000, CRC(41a47616) SHA1(55b909d2bc2079d0dfed5036c78c9e09bce09843) )
	ROM_LOAD32_WORD( "mpr-17550.22", 0x800002, 0x200000, CRC(c36ff3f5) SHA1(f14fdf275905a90a0d4cc534d90b0302f26676d8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17553.25", 0x000000, 0x200000, CRC(5da1c5d3) SHA1(c627b25a1f61a9fe9182e2199f70f6e485503c7b) )
	ROM_LOAD32_WORD( "mpr-17552.24", 0x000002, 0x200000, CRC(e91e7427) SHA1(0ac1111f2ecb4f924b5119eaaac8fa7bc87ab9d1) )
	ROM_LOAD32_WORD( "mpr-17547.27", 0x400000, 0x200000, CRC(be940431) SHA1(5c1196a6454a4fead79a930979f2e69639ec2bb9) )
	ROM_LOAD32_WORD( "mpr-17546.26", 0x400002, 0x200000, CRC(042a194b) SHA1(c6d8524dc0a879394f1234b7bb04836081bb3830) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-17574.30", 0x080000, 0x080000, CRC(4d4c3a55) SHA1(b6c0c3f0473bd7fc3ef4f5146110dfcc899a5af9) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-17573.31", 0x000000, 0x200000, CRC(e43557fe) SHA1(4c61a135819862df02347c118dc4d88a0adac273) )
	ROM_LOAD( "mpr-17572.32", 0x200000, 0x200000, CRC(4febecc8) SHA1(9683ea9bedfc5cd7b4a28e9a68792c0dc549d911) )
	ROM_LOAD( "mpr-17571.36", 0x400000, 0x200000, CRC(51caa584) SHA1(cbbde1c55eddbeeefd283bb5afd79a670a282e3a) )
	ROM_LOAD( "mpr-17570.37", 0x600000, 0x200000, CRC(bccd324b) SHA1(4c7ebdea08b2dedf621f121785ed1c40ebae4236) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( vf2a ) /* Virtua Fighter 2 Revision A, Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17568a.12", 0x000000, 0x020000, CRC(5b10f232) SHA1(04df1eb9cf094d8dc5118b95028b544b47d5d328) )
	ROM_LOAD32_WORD( "epr-17569a.13", 0x000002, 0x020000, CRC(17c208e0) SHA1(260c762d7853fb1d6f894d4dd954d82dfbc92d2d) )
	ROM_LOAD32_WORD( "epr-17562a.14", 0x040000, 0x020000, CRC(db68a01a) SHA1(1e9d3f09821596d3560bf54f6323ba295ee430d8) )
	ROM_LOAD32_WORD( "epr-17563a.15", 0x040002, 0x020000, CRC(4696439d) SHA1(846fa1435a1a5c9f7f9690e1c810ca89008d1626) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17560.10", 0x000000, 0x200000, CRC(d1389864) SHA1(88e9a8b6b0f58c96957015179e7ff10f837040e6) )
	ROM_LOAD32_WORD( "mpr-17561.11", 0x000002, 0x200000, CRC(b98d0101) SHA1(e154877380b9250d8119dd4c14ba306c7b337dcd) )
	ROM_LOAD32_WORD( "mpr-17558.8",  0x400000, 0x200000, CRC(4b15f5a6) SHA1(9a34724958fef9b49eae39c6ea136e0cf532154b) )
	ROM_LOAD32_WORD( "mpr-17559.9",  0x400002, 0x200000, CRC(d3264de6) SHA1(2f094ff0b95bf1cd5c283414634ea9597204d374) )
	ROM_LOAD32_WORD( "mpr-17566.6",  0x800000, 0x200000, CRC(fb41ef98) SHA1(ad4d1ba5e5b39b2d87105ae80750284867aa4ed3) )
	ROM_LOAD32_WORD( "mpr-17567.7",  0x800002, 0x200000, CRC(c3396922) SHA1(7e0700ded530e4eb58e9a68cdb92791284c91431) )
	ROM_LOAD32_WORD( "mpr-17564.4",  0xc00000, 0x200000, CRC(d8062489) SHA1(57666b6937f79bb65c43ed02b04a454882d01e61) )
	ROM_LOAD32_WORD( "mpr-17565.5",  0xc00002, 0x200000, CRC(0517c6e9) SHA1(d9ba93998286713758385033119416714674c8d8) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17554.16", 0x000000, 0x200000, CRC(27896d82) SHA1(c0624e58de2e427465daaa10dbb02ea2a1fd0f1b) )
	ROM_LOAD32_WORD( "mpr-17548.20", 0x000002, 0x200000, CRC(c95facc2) SHA1(09d19abe5d75a335df7510df8abb2d4425159cdf) )
	ROM_LOAD32_WORD( "mpr-17555.17", 0x400000, 0x200000, CRC(4df2810b) SHA1(720c4628d7783f0323b5723b441e13741556241e) )
	ROM_LOAD32_WORD( "mpr-17549.21", 0x400002, 0x200000, CRC(e0bce0e6) SHA1(0570604dc2007288795a3125ffd480bc4b3b0802) )
	ROM_LOAD32_WORD( "mpr-17556.18", 0x800000, 0x200000, CRC(41a47616) SHA1(55b909d2bc2079d0dfed5036c78c9e09bce09843) )
	ROM_LOAD32_WORD( "mpr-17550.22", 0x800002, 0x200000, CRC(c36ff3f5) SHA1(f14fdf275905a90a0d4cc534d90b0302f26676d8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17553.25", 0x000000, 0x200000, CRC(5da1c5d3) SHA1(c627b25a1f61a9fe9182e2199f70f6e485503c7b) )
	ROM_LOAD32_WORD( "mpr-17552.24", 0x000002, 0x200000, CRC(e91e7427) SHA1(0ac1111f2ecb4f924b5119eaaac8fa7bc87ab9d1) )
	ROM_LOAD32_WORD( "mpr-17547.27", 0x400000, 0x200000, CRC(be940431) SHA1(5c1196a6454a4fead79a930979f2e69639ec2bb9) )
	ROM_LOAD32_WORD( "mpr-17546.26", 0x400002, 0x200000, CRC(042a194b) SHA1(c6d8524dc0a879394f1234b7bb04836081bb3830) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-17574.30", 0x080000, 0x080000, CRC(4d4c3a55) SHA1(b6c0c3f0473bd7fc3ef4f5146110dfcc899a5af9) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-17573.31", 0x000000, 0x200000, CRC(e43557fe) SHA1(4c61a135819862df02347c118dc4d88a0adac273) )
	ROM_LOAD( "mpr-17572.32", 0x200000, 0x200000, CRC(4febecc8) SHA1(9683ea9bedfc5cd7b4a28e9a68792c0dc549d911) )
	ROM_LOAD( "mpr-17571.36", 0x400000, 0x200000, CRC(51caa584) SHA1(cbbde1c55eddbeeefd283bb5afd79a670a282e3a) )
	ROM_LOAD( "mpr-17570.37", 0x600000, 0x200000, CRC(bccd324b) SHA1(4c7ebdea08b2dedf621f121785ed1c40ebae4236) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( vf2o ) /* Virtua Fighter 2, Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17568.12", 0x000000, 0x020000, CRC(cf5d53d1) SHA1(4ed907bbfc1a47e51c9cc11f55645752574adaef) )
	ROM_LOAD32_WORD( "epr-17569.13", 0x000002, 0x020000, CRC(0fb32808) SHA1(95efb3eeaf95fb5f79ddae4ef20e2211b07f8d30) )
	ROM_LOAD32_WORD( "epr-17562.14", 0x040000, 0x020000, CRC(b893bcef) SHA1(2f862a7099aa757ee1f2ad8245eb4f8f4fdfb7bc) )
	ROM_LOAD32_WORD( "epr-17563.15", 0x040002, 0x020000, CRC(3b55f5a8) SHA1(b1ca3d4d3568c1652dcd8e546ffff23a4a21a699) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17560.10", 0x000000, 0x200000, CRC(d1389864) SHA1(88e9a8b6b0f58c96957015179e7ff10f837040e6) )
	ROM_LOAD32_WORD( "mpr-17561.11", 0x000002, 0x200000, CRC(b98d0101) SHA1(e154877380b9250d8119dd4c14ba306c7b337dcd) )
	ROM_LOAD32_WORD( "mpr-17558.8",  0x400000, 0x200000, CRC(4b15f5a6) SHA1(9a34724958fef9b49eae39c6ea136e0cf532154b) )
	ROM_LOAD32_WORD( "mpr-17559.9",  0x400002, 0x200000, CRC(d3264de6) SHA1(2f094ff0b95bf1cd5c283414634ea9597204d374) )
	ROM_LOAD32_WORD( "mpr-17566.6",  0x800000, 0x200000, CRC(fb41ef98) SHA1(ad4d1ba5e5b39b2d87105ae80750284867aa4ed3) )
	ROM_LOAD32_WORD( "mpr-17567.7",  0x800002, 0x200000, CRC(c3396922) SHA1(7e0700ded530e4eb58e9a68cdb92791284c91431) )
	ROM_LOAD32_WORD( "mpr-17564.4",  0xc00000, 0x200000, CRC(d8062489) SHA1(57666b6937f79bb65c43ed02b04a454882d01e61) )
	ROM_LOAD32_WORD( "mpr-17565.5",  0xc00002, 0x200000, CRC(0517c6e9) SHA1(d9ba93998286713758385033119416714674c8d8) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17554.16", 0x000000, 0x200000, CRC(27896d82) SHA1(c0624e58de2e427465daaa10dbb02ea2a1fd0f1b) )
	ROM_LOAD32_WORD( "mpr-17548.20", 0x000002, 0x200000, CRC(c95facc2) SHA1(09d19abe5d75a335df7510df8abb2d4425159cdf) )
	ROM_LOAD32_WORD( "mpr-17555.17", 0x400000, 0x200000, CRC(4df2810b) SHA1(720c4628d7783f0323b5723b441e13741556241e) )
	ROM_LOAD32_WORD( "mpr-17549.21", 0x400002, 0x200000, CRC(e0bce0e6) SHA1(0570604dc2007288795a3125ffd480bc4b3b0802) )
	ROM_LOAD32_WORD( "mpr-17556.18", 0x800000, 0x200000, CRC(41a47616) SHA1(55b909d2bc2079d0dfed5036c78c9e09bce09843) )
	ROM_LOAD32_WORD( "mpr-17550.22", 0x800002, 0x200000, CRC(c36ff3f5) SHA1(f14fdf275905a90a0d4cc534d90b0302f26676d8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17553.25", 0x000000, 0x200000, CRC(5da1c5d3) SHA1(c627b25a1f61a9fe9182e2199f70f6e485503c7b) )
	ROM_LOAD32_WORD( "mpr-17552.24", 0x000002, 0x200000, CRC(e91e7427) SHA1(0ac1111f2ecb4f924b5119eaaac8fa7bc87ab9d1) )
	ROM_LOAD32_WORD( "mpr-17547.27", 0x400000, 0x200000, CRC(be940431) SHA1(5c1196a6454a4fead79a930979f2e69639ec2bb9) )
	ROM_LOAD32_WORD( "mpr-17546.26", 0x400002, 0x200000, CRC(042a194b) SHA1(c6d8524dc0a879394f1234b7bb04836081bb3830) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-17574.30", 0x080000, 0x080000, CRC(4d4c3a55) SHA1(b6c0c3f0473bd7fc3ef4f5146110dfcc899a5af9) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-17573.31", 0x000000, 0x200000, CRC(e43557fe) SHA1(4c61a135819862df02347c118dc4d88a0adac273) )
	ROM_LOAD( "mpr-17572.32", 0x200000, 0x200000, CRC(4febecc8) SHA1(9683ea9bedfc5cd7b4a28e9a68792c0dc549d911) )
	ROM_LOAD( "mpr-17571.36", 0x400000, 0x200000, CRC(51caa584) SHA1(cbbde1c55eddbeeefd283bb5afd79a670a282e3a) )
	ROM_LOAD( "mpr-17570.37", 0x600000, 0x200000, CRC(bccd324b) SHA1(4c7ebdea08b2dedf621f121785ed1c40ebae4236) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( srallyc ) /* Sega Rally Championship Revision C, Model 2A, Sega game ID# 833-11649 RALLY TWIN, Sega ROM board ID# 834-11618 RALLY TWIN */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17888c.12",  0x000000, 0x080000, CRC(3d6808aa) SHA1(33abf9cdcee9583dc600c94e1e29ce260e8c5d32) )
	ROM_LOAD32_WORD( "epr-17889c.13",  0x000002, 0x080000, CRC(f43c7802) SHA1(4b1efb3d5644fed1753da1750bf5c300d3a15d2c) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17746.10", 0x000000, 0x200000, CRC(8fe311f4) SHA1(f4ada8e5c906fc384bed1b96f09cdf313f89e825) )
	ROM_LOAD32_WORD( "mpr-17747.11", 0x000002, 0x200000, CRC(543593fd) SHA1(5ba63a77e9fc70569af21d50b3171bc8ff4522b8) )
	ROM_LOAD32_WORD( "mpr-17744.8",  0x400000, 0x200000, CRC(71fed098) SHA1(1d187cad375121a45348d640edd3cc7dce658d28) )
	ROM_LOAD32_WORD( "mpr-17745.9",  0x400002, 0x200000, CRC(8ecca705) SHA1(ed2b3298aad6f4e52dc672a0168183e457564b43) )
	ROM_LOAD32_WORD( "mpr-17884.6",  0x800000, 0x200000, CRC(4cfc95e1) SHA1(81d927b8c4f9d0c4c5e29d676b30f30f83751fdc) )
	ROM_LOAD32_WORD( "mpr-17885.7",  0x800002, 0x200000, CRC(a08d2467) SHA1(9449ac8f8f9ce8d8e536b05a91e46841fed7f2d0) )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD( "mpr-17754.28", 0x000000, 0x200000, CRC(81a84f67) SHA1(c0a9b690523a529e4015e9af10dc3fb2a1726f08) )
	ROM_LOAD32_WORD( "mpr-17755.29", 0x000002, 0x200000, CRC(2a6e7da4) SHA1(e60803ae951489fe47d66731d15c32249ca547b4) )

	ROM_REGION( 0x010000, "drivecpu", 0 ) // Drive I/O program
	ROM_LOAD( "epr-17891.ic12", 0x000000, 0x010000, CRC(9a33b437) SHA1(3e8f210aa5159e78f640126cb5ce7f05f22560f2) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17748.16", 0x000000, 0x200000, CRC(3148a2b2) SHA1(283cc49bfb6c6381a7ead9273fd097dca5b981b6) )
	ROM_LOAD32_WORD( "mpr-17750.20", 0x000002, 0x200000, CRC(232aec29) SHA1(4d470e71df61298282c356814e2d151fda323fb6) )
	ROM_LOAD32_WORD( "mpr-17749.17", 0x400000, 0x200000, CRC(0838d184) SHA1(704175c8b29e4c989afcb7be42e7e0e096740eaf) )
	ROM_LOAD32_WORD( "mpr-17751.21", 0x400002, 0x200000, CRC(ed87ac62) SHA1(601542149d33ca52a47536b4b0af47bf1fd87eb2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17753.25", 0x000000, 0x200000, CRC(6db0eb36) SHA1(dd5fd3c9592360d3e95623ac2491e6faabe9dbcb) )
	ROM_LOAD32_WORD( "mpr-17752.24", 0x000002, 0x200000, CRC(d6aa86ce) SHA1(1d342f87d1af1e5438d1ae818b1b14268e765897) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-17890a.30", 0x080000, 0x040000, CRC(5bac3fa1) SHA1(3635333d36463b6fab25560ed918e05138f964dc) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-17756.31", 0x000000, 0x200000, CRC(7725f111) SHA1(1f1ee3f19a6bcf57bc5a1c7dd64ee83f8b81f084) )
	ROM_LOAD( "mpr-17757.32", 0x200000, 0x200000, CRC(1616e649) SHA1(1d3a0e441d150ada0535a9d50e2f69dd4b99c584) )
	ROM_LOAD( "mpr-17886.36", 0x400000, 0x200000, CRC(54a72923) SHA1(103c4838b27378c834c08d29d6fb6ba95e7f9d03) )
	ROM_LOAD( "mpr-17887.37", 0x600000, 0x200000, CRC(38c31fdd) SHA1(a85f05160b060d9d4a431aaa73cfc03f24214fb9) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( srallycb ) /* Sega Rally Championship Revision B, Model 2A, Sega game ID# 833-11649 RALLY TWIN, Sega ROM board ID# 834-11618 RALLY TWIN */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17888b.12",  0x000000, 0x080000, CRC(95bce0b9) SHA1(9b293b430db14cfab35466d2f9a1e3f7e2df3143) )
	ROM_LOAD32_WORD( "epr-17889b.13",  0x000002, 0x080000, CRC(395c425e) SHA1(9868d2b79255120abfdb7f9c0930a607aeef5363) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17746.10", 0x000000, 0x200000, CRC(8fe311f4) SHA1(f4ada8e5c906fc384bed1b96f09cdf313f89e825) )
	ROM_LOAD32_WORD( "mpr-17747.11", 0x000002, 0x200000, CRC(543593fd) SHA1(5ba63a77e9fc70569af21d50b3171bc8ff4522b8) )
	ROM_LOAD32_WORD( "mpr-17744.8",  0x400000, 0x200000, CRC(71fed098) SHA1(1d187cad375121a45348d640edd3cc7dce658d28) )
	ROM_LOAD32_WORD( "mpr-17745.9",  0x400002, 0x200000, CRC(8ecca705) SHA1(ed2b3298aad6f4e52dc672a0168183e457564b43) )
	ROM_LOAD32_WORD( "mpr-17884.6",  0x800000, 0x200000, CRC(4cfc95e1) SHA1(81d927b8c4f9d0c4c5e29d676b30f30f83751fdc) )
	ROM_LOAD32_WORD( "mpr-17885.7",  0x800002, 0x200000, CRC(a08d2467) SHA1(9449ac8f8f9ce8d8e536b05a91e46841fed7f2d0) )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD( "mpr-17754.28", 0x000000, 0x200000, CRC(81a84f67) SHA1(c0a9b690523a529e4015e9af10dc3fb2a1726f08) )
	ROM_LOAD32_WORD( "mpr-17755.29", 0x000002, 0x200000, CRC(2a6e7da4) SHA1(e60803ae951489fe47d66731d15c32249ca547b4) )

	ROM_REGION( 0x010000, "drivecpu", 0 ) // Drive I/O program
	ROM_LOAD( "epr-17891.ic12", 0x000000, 0x010000, CRC(9a33b437) SHA1(3e8f210aa5159e78f640126cb5ce7f05f22560f2) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17748.16", 0x000000, 0x200000, CRC(3148a2b2) SHA1(283cc49bfb6c6381a7ead9273fd097dca5b981b6) )
	ROM_LOAD32_WORD( "mpr-17750.20", 0x000002, 0x200000, CRC(232aec29) SHA1(4d470e71df61298282c356814e2d151fda323fb6) )
	ROM_LOAD32_WORD( "mpr-17749.17", 0x400000, 0x200000, CRC(0838d184) SHA1(704175c8b29e4c989afcb7be42e7e0e096740eaf) )
	ROM_LOAD32_WORD( "mpr-17751.21", 0x400002, 0x200000, CRC(ed87ac62) SHA1(601542149d33ca52a47536b4b0af47bf1fd87eb2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17753.25", 0x000000, 0x200000, CRC(6db0eb36) SHA1(dd5fd3c9592360d3e95623ac2491e6faabe9dbcb) )
	ROM_LOAD32_WORD( "mpr-17752.24", 0x000002, 0x200000, CRC(d6aa86ce) SHA1(1d342f87d1af1e5438d1ae818b1b14268e765897) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-17890a.30", 0x080000, 0x040000, CRC(5bac3fa1) SHA1(3635333d36463b6fab25560ed918e05138f964dc) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-17756.31", 0x000000, 0x200000, CRC(7725f111) SHA1(1f1ee3f19a6bcf57bc5a1c7dd64ee83f8b81f084) )
	ROM_LOAD( "mpr-17757.32", 0x200000, 0x200000, CRC(1616e649) SHA1(1d3a0e441d150ada0535a9d50e2f69dd4b99c584) )
	ROM_LOAD( "mpr-17886.36", 0x400000, 0x200000, CRC(54a72923) SHA1(103c4838b27378c834c08d29d6fb6ba95e7f9d03) )
	ROM_LOAD( "mpr-17887.37", 0x600000, 0x200000, CRC(38c31fdd) SHA1(a85f05160b060d9d4a431aaa73cfc03f24214fb9) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( srallyca ) /* Sega Rally Championship DX Revision A, Model 2A - Single player cabinet - NO LINK option!, Sega ROM board ID# 834-11254 RALLY 50 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17760a.12",  0x000000, 0x020000, CRC(2c1b996b) SHA1(28c1196aac1c242e61069ee809c9e8229c061950) ) /* AMD 27C1024 EPROM */
	ROM_LOAD32_WORD( "epr-17761a.13",  0x000002, 0x020000, CRC(50813f66) SHA1(f27ffb314e06fa18d863fdf172dafe56122cd606) ) /* AMD 27C1024 EPROM */

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17746.10", 0x000000, 0x200000, CRC(8fe311f4) SHA1(f4ada8e5c906fc384bed1b96f09cdf313f89e825) )
	ROM_LOAD32_WORD( "mpr-17747.11", 0x000002, 0x200000, CRC(543593fd) SHA1(5ba63a77e9fc70569af21d50b3171bc8ff4522b8) )
	ROM_LOAD32_WORD( "mpr-17744.8",  0x400000, 0x200000, CRC(71fed098) SHA1(1d187cad375121a45348d640edd3cc7dce658d28) )
	ROM_LOAD32_WORD( "mpr-17745.9",  0x400002, 0x200000, CRC(8ecca705) SHA1(ed2b3298aad6f4e52dc672a0168183e457564b43) )
	ROM_LOAD32_WORD( "mpr-17764a.6", 0x800000, 0x200000, CRC(dcb91e31) SHA1(2725268e97b9f4c14d56c040af38bc82f5020e3e) )
	ROM_LOAD32_WORD( "mpr-17765a.7", 0x800002, 0x200000, CRC(b657dc48) SHA1(ae0f1bc6e2479fa51ca36f8be3a1785981c4dfe9) )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD( "mpr-17754.28", 0x000000, 0x200000, CRC(81a84f67) SHA1(c0a9b690523a529e4015e9af10dc3fb2a1726f08) )
	ROM_LOAD32_WORD( "mpr-17755.29", 0x000002, 0x200000, CRC(2a6e7da4) SHA1(e60803ae951489fe47d66731d15c32249ca547b4) )

	ROM_REGION( 0x010000, "drivecpu", 0 ) // Drive I/O program
	ROM_LOAD( "epr-17762.ic12", 0x000000, 0x010000, NO_DUMP ) /* Need to verify actual EPR-xxxx number, might be EPR-17759 */
	ROM_LOAD( "epr-17891.ic12", 0x000000, 0x010000, CRC(9a33b437) SHA1(3e8f210aa5159e78f640126cb5ce7f05f22560f2) ) /* REMOVE when EPR-17762 is dumped & added */

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17748.16", 0x000000, 0x200000, CRC(3148a2b2) SHA1(283cc49bfb6c6381a7ead9273fd097dca5b981b6) )
	ROM_LOAD32_WORD( "mpr-17750.20", 0x000002, 0x200000, CRC(232aec29) SHA1(4d470e71df61298282c356814e2d151fda323fb6) )
	ROM_LOAD32_WORD( "mpr-17749.17", 0x400000, 0x200000, CRC(0838d184) SHA1(704175c8b29e4c989afcb7be42e7e0e096740eaf) )
	ROM_LOAD32_WORD( "mpr-17751.21", 0x400002, 0x200000, CRC(ed87ac62) SHA1(601542149d33ca52a47536b4b0af47bf1fd87eb2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17753.25", 0x000000, 0x200000, CRC(6db0eb36) SHA1(dd5fd3c9592360d3e95623ac2491e6faabe9dbcb) )
	ROM_LOAD32_WORD( "mpr-17752.24", 0x000002, 0x200000, CRC(d6aa86ce) SHA1(1d342f87d1af1e5438d1ae818b1b14268e765897) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-17763.30", 0x080000, 0x040000, NO_DUMP ) /* Number verified via Sega Rally Champ DX manual */
	ROM_LOAD16_WORD_SWAP( "epr-17890a.30", 0x080000, 0x040000, CRC(5bac3fa1) SHA1(3635333d36463b6fab25560ed918e05138f964dc) ) /* REMOVE when EPR-17763 & EPR-17758 is dumped & added */

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-17756.31", 0x000000, 0x200000, CRC(7725f111) SHA1(1f1ee3f19a6bcf57bc5a1c7dd64ee83f8b81f084) )
	ROM_LOAD( "mpr-17757.32", 0x200000, 0x200000, CRC(1616e649) SHA1(1d3a0e441d150ada0535a9d50e2f69dd4b99c584) )
	ROM_LOAD( "mpr-17758.36", 0x400000, 0x200000, NO_DUMP ) /* Number verified via Sega Rally Champ DX manual */
	/* The DX version doesn't have any sound rom at IC37 */
	ROM_LOAD( "mpr-17886.36", 0x400000, 0x200000, CRC(54a72923) SHA1(103c4838b27378c834c08d29d6fb6ba95e7f9d03) ) /* REMOVE when EPR-17758 & EPR-17763 is dumped & added */
	ROM_LOAD( "mpr-17887.37", 0x600000, 0x200000, CRC(38c31fdd) SHA1(a85f05160b060d9d4a431aaa73cfc03f24214fb9) ) /* REMOVE when EPR-17758 & EPR-17763 is dumped & added */

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

/*

Manx TT

837-10848-01-91  Model2 A-CRX CPU BD
837-10849-02     Model2 A-CRX VIDEO BD
837-12396        COMM BD MANX TT

837-12279        SOUND BD MANX T.T (for DX only)

Rom boards:
 834-12467  ROM BD MANX T.T TWIN
 834-12277  ROM BD MANX T.T DX

Known missing roms:

Manx TT DX
EPR-18742 - Sound CPU rom (on Sound BD)
MPR-18743 - Sound Samples (on Sound BD)
EPR-18744.12 - Program rom
EPR-18745.13 - Program rom
EPR-18784.14 - Program rom*
EPR-18785.15 - Program rom*
EPR-18746.30 - Sound CPU rom*
EPR-18767.5  - Data*
EPR-18768.4  - Data*

EPR-18763.31 & alt sound CPU code EPR-18924a.30 are dumped

* Note: The manual scan was low-res and these numbers might be incorrect as they were VERY hard to read!

*/

ROM_START( manxtt ) /* Manx TT Superbike Twin Revision D, Model 2A - Can be set to Twin or Deluxe - Found in DX setup */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-18822d.12", 0x000000, 0x020000, CRC(4f435990) SHA1(0fcf64598384012caea27394280de89a9348a47d) )
	ROM_LOAD32_WORD( "epr-18823d.13", 0x000002, 0x020000, CRC(b8eddb5c) SHA1(7e3b97e3370e68d92922e8999246064196610270) )
	ROM_LOAD32_WORD( "epr-18824d.14", 0x040000, 0x020000, CRC(aca9f61f) SHA1(629db70371ea9986ef75557044b5e98329712418) )
	ROM_LOAD32_WORD( "epr-18825d.15", 0x040002, 0x020000, CRC(5a1d7799) SHA1(bb5e8a5a3b766b5dc4285ecba330094caf8a71e6) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-18751.10", 0x000000, 0x200000, CRC(773ad43d) SHA1(4d1601dc08a08b724e33e7cd90a4f22e18cfed9c) )
	ROM_LOAD32_WORD( "mpr-18752.11", 0x000002, 0x200000, CRC(4da3719e) SHA1(24007e4ae3ba1a06321328d14e2bd6002fa1936e) )
	ROM_LOAD32_WORD( "mpr-18749.8", 0x400000, 0x200000, CRC(c3fe0eea) SHA1(ada21405a136935ac4da1a3535c25fccf903f2d1) )
	ROM_LOAD32_WORD( "mpr-18750.9", 0x400002, 0x200000, CRC(40b55494) SHA1(d98ae5518c5d31b155b1a7c4f7d9d67f44d7beae) )
	ROM_LOAD32_WORD( "mpr-18747.6", 0x800000, 0x200000, CRC(a65ec1e8) SHA1(92636bdff0ae4cdb43dfc2986fad2d1b59469323) )
	ROM_LOAD32_WORD( "mpr-18748.7", 0x800002, 0x200000, CRC(375e3748) SHA1(6c2e903dd073b130bcabb347631b876dc868b494) )
	ROM_LOAD32_WORD( "epr-18862.4", 0xc00000, 0x080000, CRC(9adc3a30) SHA1(029db946338f8e0eccace8590082cc96bdf13e31) )
	ROM_LOAD32_WORD( "epr-18863.5", 0xc00002, 0x080000, CRC(603742e9) SHA1(f78a5f7e582d313880c734158bb0fa68b256a58a) )
	ROM_COPY( "user1", 0xc00000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0xc00000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0xc00000, 0xf00000, 0x100000 )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-18753.16", 0x000000, 0x200000, CRC(33ddaa0d) SHA1(26f643d6b9cecf08bd249290a670a0edea1b5be4) )
	ROM_LOAD32_WORD( "mpr-18756.20", 0x000002, 0x200000, CRC(28713617) SHA1(fc2a6258387a1bc3fae2109b2dae6dd2a1984ab5) )
	ROM_LOAD32_WORD( "mpr-18754.17", 0x400000, 0x200000, CRC(09aabde5) SHA1(e50646efb2ca59792833ce91398c4efa861ad6d1) )
	ROM_LOAD32_WORD( "mpr-18757.21", 0x400002, 0x200000, CRC(25fc92e9) SHA1(226c4c7289b3b6009c1ffea4a171e3fb4e31a67c) )
	ROM_LOAD32_WORD( "mpr-18755.18", 0x800000, 0x200000, CRC(bf094d9e) SHA1(2cd7130b226a28098191a6caf6fd761bb0bfac7b) )
	ROM_LOAD32_WORD( "mpr-18758.22", 0x800002, 0x200000, CRC(1b5473d0) SHA1(658e33503f6990f4d9a954c63efad5f53d15f3a4) )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD( "mpr-18761.28", 0x000000, 0x200000, CRC(4e39ec05) SHA1(50696cd320f1a6492e0c193713acbce085d959cd) )
	ROM_LOAD32_WORD( "mpr-18762.29", 0x000002, 0x200000, CRC(4ab165d8) SHA1(7ff42a4c7236fec76f94f2d0c5537e503bcc98e5) )


	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-18760.25", 0x000000, 0x200000, CRC(4e3a4a89) SHA1(bba6cd2a15b3f963388a3a87880da86b10f6e0a2) )
	ROM_LOAD32_WORD( "mpr-18759.24", 0x000002, 0x200000, CRC(278d8742) SHA1(5f285fc8cfe88c00ba2bbe1b509b49abd38e00ec) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-18924a.30", 0x080000, 0x040000, CRC(ad6f40ec) SHA1(27aa0477dc325162766d459ffe95b61ee65dd28f) ) /* Sound program for DX set */

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-18763.31", 0x000000, 0x200000, CRC(1bcb2283) SHA1(a4a8a2f8f0901bfb57778351210ccfc421cacbd4) ) /* Sound sample for DX set */
	ROM_LOAD( "mpr-18764.32", 0x200000, 0x200000, CRC(0dc6a860) SHA1(cb2ada0f8a592940de11ee781ad4beb5095c3b37) )
	ROM_LOAD( "mpr-18765.36", 0x400000, 0x200000, CRC(ca4a803c) SHA1(70b59da8f2532a02e980caba5bb86ec13a4d7ab5) )
	ROM_LOAD( "mpr-18766.37", 0x600000, 0x200000, CRC(e41892ea) SHA1(9ef5e26db4abf0ed36df63fc246b568e1c5d6cfa) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-18742.7", 0x000000, 0x020000, CRC(1b78da74) SHA1(939b0f2413ae3c11fac11b49ab8b0de2c5e35e61) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-18743.32", 0x000000, 0x200000, CRC(17e84e15) SHA1(8437cddc4c4d729e886a5ab076885a54bb7a30d0) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, ROMREGION_ERASE00 ) // Samples

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD16_WORD_SWAP( "epr-18643a.7", 0x000000, 0x020000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( manxttc ) /* Manx TT Superbike Twin Revision C, Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-18822c.12",  0x000000, 0x020000, CRC(c7b3e45a) SHA1(d3a6910bf6efc138e0e40332219b90dea7d6ea56) )
	ROM_LOAD32_WORD( "epr-18823c.13",  0x000002, 0x020000, CRC(6b0c1dfb) SHA1(6da5c071e3ce842a99f928f473d4ccf7165785ac) )
	ROM_LOAD32_WORD( "epr-18824c.14",  0x040000, 0x020000, CRC(352bb817) SHA1(389cbf951ba606acb9ab7bff5cda85d9166e64ff) )
	ROM_LOAD32_WORD( "epr-18825c.15",  0x040002, 0x020000, CRC(f88b036c) SHA1(f6196e8da5e6579fe3fa5c24ab9538964c98e267) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-18751.10",  0x000000, 0x200000, CRC(773ad43d) SHA1(4d1601dc08a08b724e33e7cd90a4f22e18cfed9c) )
	ROM_LOAD32_WORD( "mpr-18752.11",  0x000002, 0x200000, CRC(4da3719e) SHA1(24007e4ae3ba1a06321328d14e2bd6002fa1936e) )
	ROM_LOAD32_WORD( "mpr-18749.8",   0x400000, 0x200000, CRC(c3fe0eea) SHA1(ada21405a136935ac4da1a3535c25fccf903f2d1) )
	ROM_LOAD32_WORD( "mpr-18750.9",   0x400002, 0x200000, CRC(40b55494) SHA1(d98ae5518c5d31b155b1a7c4f7d9d67f44d7beae) )
	ROM_LOAD32_WORD( "mpr-18747.6",   0x800000, 0x200000, CRC(a65ec1e8) SHA1(92636bdff0ae4cdb43dfc2986fad2d1b59469323) )
	ROM_LOAD32_WORD( "mpr-18748.7",   0x800002, 0x200000, CRC(375e3748) SHA1(6c2e903dd073b130bcabb347631b876dc868b494) )
	ROM_LOAD32_WORD( "epr-18862.4",   0xc00000, 0x080000, CRC(9adc3a30) SHA1(029db946338f8e0eccace8590082cc96bdf13e31) )
	ROM_LOAD32_WORD( "epr-18863.5",   0xc00002, 0x080000, CRC(603742e9) SHA1(f78a5f7e582d313880c734158bb0fa68b256a58a) )
	ROM_COPY( "user1", 0xc00000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0xc00000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0xc00000, 0xf00000, 0x100000 )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-18753.16", 0x000000, 0x200000, CRC(33ddaa0d) SHA1(26f643d6b9cecf08bd249290a670a0edea1b5be4) )
	ROM_LOAD32_WORD( "mpr-18756.20", 0x000002, 0x200000, CRC(28713617) SHA1(fc2a6258387a1bc3fae2109b2dae6dd2a1984ab5) )
	ROM_LOAD32_WORD( "mpr-18754.17", 0x400000, 0x200000, CRC(09aabde5) SHA1(e50646efb2ca59792833ce91398c4efa861ad6d1) )
	ROM_LOAD32_WORD( "mpr-18757.21", 0x400002, 0x200000, CRC(25fc92e9) SHA1(226c4c7289b3b6009c1ffea4a171e3fb4e31a67c) )
	ROM_LOAD32_WORD( "mpr-18755.18", 0x800000, 0x200000, CRC(bf094d9e) SHA1(2cd7130b226a28098191a6caf6fd761bb0bfac7b) )
	ROM_LOAD32_WORD( "mpr-18758.22", 0x800002, 0x200000, CRC(1b5473d0) SHA1(658e33503f6990f4d9a954c63efad5f53d15f3a4) )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD( "mpr-18761.28", 0x000000, 0x200000, CRC(4e39ec05) SHA1(50696cd320f1a6492e0c193713acbce085d959cd) )
	ROM_LOAD32_WORD( "mpr-18762.29", 0x000002, 0x200000, CRC(4ab165d8) SHA1(7ff42a4c7236fec76f94f2d0c5537e503bcc98e5) )

	ROM_REGION( 0x800000, "tgpextra", ROMREGION_ERASEFF) // Coprocessor Data ROM

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-18760.25", 0x000000, 0x200000, CRC(4e3a4a89) SHA1(bba6cd2a15b3f963388a3a87880da86b10f6e0a2) )
	ROM_LOAD32_WORD( "mpr-18759.24", 0x000002, 0x200000, CRC(278d8742) SHA1(5f285fc8cfe88c00ba2bbe1b509b49abd38e00ec) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-18826.30",  0x080000, 0x040000, CRC(ed9fe4c1) SHA1(c3dd8a1324a4dc9b012bd9bf21d1f48578870f72) ) /* Sound program for Twin set */
//  ROM_LOAD16_WORD_SWAP( "epr-18924a.30", 0x080000, 0x040000, CRC(ad6f40ec) SHA1(27aa0477dc325162766d459ffe95b61ee65dd28f) ) /* Sound program for ?? set */

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-18827.31", 0x000000, 0x200000, CRC(58d78ca1) SHA1(95275ed8315c044bfde2f23c10416f22627b34df) ) /* Sound sample for Twin set */
//  ROM_LOAD( "mpr-18763.31", 0x000000, 0x200000, CRC(1bcb2283) SHA1(a4a8a2f8f0901bfb57778351210ccfc421cacbd4) ) /* Sound sample for DX set */
	ROM_LOAD( "mpr-18764.32", 0x200000, 0x200000, CRC(0dc6a860) SHA1(cb2ada0f8a592940de11ee781ad4beb5095c3b37) )
	ROM_LOAD( "mpr-18765.36", 0x400000, 0x200000, CRC(ca4a803c) SHA1(70b59da8f2532a02e980caba5bb86ec13a4d7ab5) )
	ROM_LOAD( "mpr-18766.37", 0x600000, 0x200000, CRC(e41892ea) SHA1(9ef5e26db4abf0ed36df63fc246b568e1c5d6cfa) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD16_WORD_SWAP( "epr-18643.7",  0x000000, 0x020000, CRC(7166fca7) SHA1(f5d02906b64bb2fd1af8e3772c1b01a4e006c060) )
//  ROM_LOAD16_WORD_SWAP( "epr-18643a.7", 0x000000, 0x020000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) ) /* COMM boards found with either revision */

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( motoraid ) /* Motoraid, Model 2A, Sega game ID# 833-13232 MOTOR RAID TWIN, Sega ROM board ID# 834-13233  */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-20007.12",  0x000000, 0x080000, CRC(f040c108) SHA1(a6a0fa8fb9d62d0cc2ac84ea3ad457953952d980) )
	ROM_LOAD32_WORD( "epr-20008.13",  0x000002, 0x080000, CRC(78976e1a) SHA1(fd15e8c81b3b2f3bdf3bb8d9414b9b8a6f1f000f) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-20019.10",   0x0000000, 0x400000, CRC(49053727) SHA1(0543d19d1d60b1d12b4409c1491782f2232da685) )
	ROM_LOAD32_WORD( "mpr-20020.11",   0x0000002, 0x400000, CRC(cc5ddb15) SHA1(19e15e0e9ec1bb5d1b789876778fbb487cfea1ba) )
	ROM_LOAD32_WORD( "mpr-20017.8",    0x0800000, 0x400000, CRC(4e206acd) SHA1(b48b5bd3a2f68c62d16516a037fbd45f49283d23) )
	ROM_LOAD32_WORD( "mpr-20018.9",    0x0800002, 0x400000, CRC(e7ed0e85) SHA1(78a0c72095a664c4b6e529beea46a31ae0a99e5a) )
	ROM_LOAD32_WORD( "mpr-20015.6",    0x1000000, 0x400000, CRC(23427339) SHA1(3e37cfcb4dcc8976805934faf8805cd83acde66e) )
	ROM_LOAD32_WORD( "mpr-20016.7",    0x1000002, 0x400000, CRC(c99a83f4) SHA1(b057d61478f7dc7a32ad233473f1a63498b3779e) )
	ROM_LOAD32_WORD( "epr-20013.4",    0x1800000, 0x080000, CRC(a4478f52) SHA1(28f430319b34e715ca57ce4e01be23a786eab4bc) )
	ROM_LOAD32_WORD( "epr-20014.5",    0x1800002, 0x080000, CRC(1aa541be) SHA1(c4cc61a42e89aaae075ad1b6e8df2907c5710d3e) )
	ROM_COPY( "user1", 0x1800000, 0x1900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x1800000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-20023.16", 0x0000000, 0x400000, CRC(016be8d6) SHA1(804f69cd342e25cf1bed48e778981d67c4d1c9c7) )
	ROM_LOAD32_WORD( "mpr-20026.20", 0x0000002, 0x400000, CRC(20044a30) SHA1(46be0cc2b8a4a3f530d081d11c6099d814977270) )
	ROM_LOAD32_WORD( "mpr-20024.17", 0x0800000, 0x400000, CRC(62fd2d5b) SHA1(6a386a666ae57da5e47364da7b97da9c913710ef) )
	ROM_LOAD32_WORD( "mpr-20027.21", 0x0800002, 0x400000, CRC(b2504ea6) SHA1(17c23c64b1080ab6a8eb282cabcd7d7612193045) )
	ROM_LOAD32_WORD( "mpr-20025.18", 0x1000000, 0x400000, CRC(d4ecd0be) SHA1(9df0d1db32b818dad28f9eeab3bc19c56d27ec6d) )
	ROM_LOAD32_WORD( "mpr-20028.22", 0x1000002, 0x400000, CRC(3147e0e1) SHA1(9aa0e13c8dc5073a603279a538cc7662531dfd19) )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD( "epr-20011.28", 0x000000, 0x100000, CRC(794c026c) SHA1(85abd667491fd019ee18ba256fd580356f4e1fe9) )
	ROM_LOAD32_WORD( "epr-20012.29", 0x000002, 0x100000, CRC(f53db4e3) SHA1(4474610eed52248e5e36be438eff5d39f076b134) )

	ROM_REGION( 0x800000, "tgpextra", ROMREGION_ERASEFF) // Coprocessor Data ROM

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-20022.25", 0x000000, 0x400000, CRC(9e47b3c2) SHA1(c73279e837f56c0417c07ba3c642af28fe9a24fa) )
	ROM_LOAD32_WORD( "mpr-20021.24", 0x000002, 0x400000, CRC(3cbf36cb) SHA1(059cea17f9d6f5960d9fd869c36ffb6fcf230c1a) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-20029.30", 0x080000, 0x080000, CRC(927d31b9) SHA1(e7a18ccf5a0b9ebf18ae1d5518973fa3b4eb4653) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-20030.31", 0x000000, 0x200000, CRC(b70ab686) SHA1(006911ce6332091d17808855c60a72fe928df778) )
	ROM_LOAD( "mpr-20031.32", 0x200000, 0x200000, CRC(84da70e4) SHA1(77962afcac82589cc7bc852329335676ae3e23cf) )
	ROM_LOAD( "mpr-20032.36", 0x400000, 0x200000, CRC(15516d35) SHA1(bced0d30f9b6ab579a11ac069cbb9d6d91352246) )
	ROM_LOAD( "mpr-20033.37", 0x600000, 0x200000, CRC(8c8ed187) SHA1(a9e8e2d38b23716df2e211748c52b6b666f4c111) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( skytargt ) /* Sky Target, Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-18406.12", 0x000000, 0x080000, CRC(fde9c00a) SHA1(01cd519daaf6138d9df4940bf8bb5923a1f163df) )
	ROM_LOAD32_WORD( "epr-18407.13", 0x000002, 0x080000, CRC(35f8b529) SHA1(faf6dcf8f345c1e7968823f2dba60afcd88f37c2) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-18415.10",   0x0000000, 0x400000, CRC(d7a1bbd7) SHA1(3061cc68755ca36255f325135aa44659afc3498c) )
	ROM_LOAD32_WORD( "mpr-18416.11",   0x0000002, 0x400000, CRC(b77c9243) SHA1(6ffeef418364df9e08398c7564142cbf5750beb2) )
	ROM_LOAD32_WORD( "mpr-18417.8",    0x0800000, 0x400000, CRC(a0d03f63) SHA1(88b97a76f0a85a3977915808eee4d64b69734e88) )
	ROM_LOAD32_WORD( "mpr-18418.9",    0x0800002, 0x400000, CRC(c7a6f97f) SHA1(cf7c6887519e53d7fa321a2ad888b1673e16565b) )
	ROM_LOAD32_WORD( "epr-18404.6",    0x1000000, 0x080000, CRC(f1407ec4) SHA1(d6805faea657ea0f998fb2470d7d24aa78a02bd4) )
	ROM_LOAD32_WORD( "epr-18405.7",    0x1000002, 0x080000, CRC(00b40f9e) SHA1(21b6b390d8635349ba76899acea176954a24985e) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)
	ROM_LOAD32_WORD( "mpr-18420.28", 0x000000, 0x200000, CRC(92b87817) SHA1(b6949b745d0bedeecd6d0240f8911cb345c16d8d) )
	ROM_LOAD32_WORD( "mpr-18419.29", 0x000002, 0x200000, CRC(74542d87) SHA1(37230e96dd526fb47fcbde5778e5466d8955a969) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-18413.16", 0x000000, 0x400000, CRC(1c4d416c) SHA1(2bd6eae4ab5751d485be105a06776fccd3c48d21) )
	ROM_LOAD32_WORD( "mpr-18409.20", 0x000002, 0x400000, CRC(666037ef) SHA1(6f622a82fd5ffd7a4692b5bf51b76755053a674b) )
	ROM_LOAD32_WORD( "mpr-18414.17", 0x800000, 0x400000, CRC(858885ba) SHA1(1729f6ff689a462a3d6e303ebc2dac323145a67c) )
	ROM_LOAD32_WORD( "mpr-18410.21", 0x800002, 0x400000, CRC(b821a695) SHA1(139cbba0ceffa83c0f9925258944ec8a414b3040) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-18411.24", 0x000002, 0x400000, CRC(9c2dc40c) SHA1(842a647a70ef29a8c775e88c0bcbc63782496bba) )
	ROM_LOAD32_WORD( "mpr-18412.25", 0x000000, 0x400000, CRC(4db52f8b) SHA1(66796f6c20e680a87e8939a70692680b1dd0b324) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-18408.30", 0x080000, 0x080000, CRC(6deb9657) SHA1(30e1894432a0765c64b93dd5ca7ca17ef58ac6c0) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-18424.31",   0x000000, 0x200000, CRC(590a4338) SHA1(826f167d7a4f5d30466b2f75f0123187c29c2d69) )
	ROM_LOAD( "mpr-18423.32",   0x200000, 0x200000, CRC(c356d765) SHA1(ae69c9d4e333579d826178d2863156dc784aedef) )
	ROM_LOAD( "mpr-18422.36",   0x400000, 0x200000, CRC(b4f3cea6) SHA1(49669be09e10dfae7fddce0fc4e415466cb29566) )
	ROM_LOAD( "mpr-18421.37",   0x600000, 0x200000, CRC(00522390) SHA1(5dbbf2ba008adad36929fcecb7c2c1e5ffd12618) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( vcop2 ) /* Virtua Cop 2, Model 2A, Sega Game ID# 833-12266, ROM board ID# 834-12267 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-18524.12", 0x000000, 0x080000, CRC(1858988b) SHA1(2979f8470cc31e6c5c32c6fec1a87dbd29b52309) )
	ROM_LOAD32_WORD( "epr-18525.13", 0x000002, 0x080000, CRC(0c13df3f) SHA1(6b4188f04aad80b89f1826e8ca47cff763980410) )
	ROM_LOAD32_WORD( "epr-18518.14", 0x100000, 0x080000, CRC(7842951b) SHA1(bed4ec9a5e59807d17e5e602bdaf3c68fcba08b6) )
	ROM_LOAD32_WORD( "epr-18519.15", 0x100002, 0x080000, CRC(31a30edc) SHA1(caf3c2676508a2ed032d3657ac640a257f04bdd4) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-18516.10",  0x000000, 0x200000, CRC(a3928ff0) SHA1(5a9695fb5eda394a1111a05ee5fb9cce29970e91) )
	ROM_LOAD32_WORD( "mpr-18517.11",  0x000002, 0x200000, CRC(4bd73da4) SHA1(a4434bce019729e2148a95e3a6dea38de7f789c1) )
	ROM_LOAD32_WORD( "mpr-18514.8",   0x400000, 0x200000, CRC(791283c5) SHA1(006fb22eefdd9205ede9a74fe53cbffe8c8fd45b) )
	ROM_LOAD32_WORD( "mpr-18515.9",   0x400002, 0x200000, CRC(6ba1ffec) SHA1(70f493aa4eb93edce8dd5b7b532d1f50f81069ce) )
	ROM_LOAD32_WORD( "mpr-18522.6",   0x800000, 0x200000, CRC(61d18536) SHA1(cc467cb26a8fccc48837d000fe9e1c41b0c0f4f9) )
	ROM_LOAD32_WORD( "mpr-18523.7",   0x800002, 0x200000, CRC(61d08dc4) SHA1(40d8231d184582c0fc01ad874371aaec7dfcc337) )
	ROM_LOAD32_WORD( "epr-18520.4",   0xc00000, 0x080000, CRC(1d4ec5e8) SHA1(44c4b5560d150909342e4182496f136c8c5e2edb) )
	ROM_LOAD32_WORD( "epr-18521.5",   0xc00002, 0x080000, CRC(b8b3781c) SHA1(11956fe912c34d6a86a6b91d55987f6bead73473) )
	ROM_COPY( "user1", 0xc00000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0xc00000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0xc00000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-18513.16", 0x000000, 0x200000, CRC(777a3633) SHA1(edc2798c4d88975ce67b54fc0db008e7d24db6ef) )
	ROM_LOAD32_WORD( "mpr-18510.20", 0x000002, 0x200000, CRC(e83de997) SHA1(8a8597aa31609663869e584cc5fad6e4b84f7dbe) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	/* TODO: Wrong labels? */
	ROM_LOAD32_WORD( "mpr-18511.24", 0x000002, 0x200000, CRC(cae77a4f) SHA1(f21474486f0dc4092cbad4566deea8a952862ab7) )
	ROM_LOAD32_WORD( "mpr-18512.25", 0x000000, 0x200000, CRC(d9bc7e71) SHA1(774eba886083b0dad9a47519c5801e44346312cf) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-18530.30", 0x080000, 0x080000, CRC(ac9c8357) SHA1(ad297c7fecaa9b877f0dd31e859983816947e437) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-18529.31", 0x000000, 0x200000, CRC(f76715b1) SHA1(258418c1cb37338a694e48f3b48fadfae5f40239) )
	ROM_LOAD( "mpr-18528.32", 0x200000, 0x200000, CRC(287a2f9a) SHA1(78ba93ab90322152efc37f7130073b0dc516ef5d) )
	ROM_LOAD( "mpr-18527.36", 0x400000, 0x200000, CRC(e6a49314) SHA1(26563f425f2f0906ae9278fe5de02955653d49fe) )
	ROM_LOAD( "mpr-18526.37", 0x600000, 0x200000, CRC(6516d9b5) SHA1(8f13cb02c76f7b7cd11f3c3772ff13302d55e9c3) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

/* Dynamite Cop program rom EPR numbers via "DYNAMITE COP ROM CVT MANUAL" 421-9897-01:

      USA            Export          Korea           Japan*
  ------------------------------------------------------------
Model2a:
  epr-20926.12    epr-20930.12    epr-20971.12    epr-20922.12
  epr-20927.13    epr-20931.13    epr-20972.13    epr-20923.13
  epr-20928.14    epr-20932.14    epr-20973.14    epr-20924.14
  epr-20929.15    epr-20933.15    epr-20974.15    epr-20925.15
Model2b:
  epr-20938.13    epr-20942.13    epr-20975.13    epr-20934.13
  epr-20939.14    epr-20943.14    epr-20976.14    epr-20935.14
  epr-20940.15    epr-20944.15    epr-20977.15    epr-20936.15
  epr-20941.16    epr-20945.16    epr-20978.16    epr-20937.16
Model2c:
  epr-20950.13    epr-20954.13    epr-20979.13    epr-20946.13
  epr-20951.14    epr-20955.14    epr-20980.14    epr-20947.14
  epr-20952.15    epr-20956.15    epr-20981.15    epr-20948.15
  epr-20953.16    epr-20957.16    epr-20982.16    epr-20949.16

* The numbers for the Japan sets were not listed, but are shown for comparision

In Dynamite Deka 2 manual 420-6406-01 it states there are C-CRX versions of the
USA, Export and Korea versions as well as the Japan version.

*/

ROM_START( dynamcop ) /* Dynamite Cop (Export), Model 2A, Sega Game ID# 833-11341, ROM board ID# 834-11342 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20930.12", 0x000000, 0x080000, CRC(b8fc8ff7) SHA1(53b0f9dc8494effa077170ddced2d95f43a5f134) )
	ROM_LOAD32_WORD("epr-20931.13", 0x000002, 0x080000, CRC(89d13f88) SHA1(5e266b5e153a0d9a57360cfd1af81e3a58a2fb7d) )
	ROM_LOAD32_WORD("epr-20932.14", 0x100000, 0x080000, CRC(618a68bf) SHA1(3022283dded4d08d790d034b6d543c0397b5bf5a) )
	ROM_LOAD32_WORD("epr-20933.15", 0x100002, 0x080000, CRC(13abe49c) SHA1(a741a0205c1b3664ab4d09d6d991a768269a79ea) )

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20797.10", 0x0000000, 0x400000, CRC(87bab1e4) SHA1(af2b2d82364621a1528d6ed59fbfbf15dc93ee72) )
	ROM_LOAD32_WORD("mpr-20798.11", 0x0000002, 0x400000, CRC(40dd752b) SHA1(8c2e210ac7c7b133ba9befc79a07c4ca6b4e3f18) )
	ROM_LOAD32_WORD("mpr-20795.8",  0x0800000, 0x400000, CRC(0ef85e12) SHA1(97c657edd98cde6f0780a04a7711e5b370087a60) )
	ROM_LOAD32_WORD("mpr-20796.9",  0x0800002, 0x400000, CRC(870139cb) SHA1(24fda2cd458cf7a3db485564c02ac61d30cbdf5e) )
	ROM_LOAD32_WORD("mpr-20793.6",  0x1000000, 0x400000, CRC(42ea08f8) SHA1(e70b55709067628ea0bf3f5190a300100b61eed1) )
	ROM_LOAD32_WORD("mpr-20794.7",  0x1000002, 0x400000, CRC(8e5cd1db) SHA1(d90e86d38bda12f2d0f99e23a42928f05bde3ea8) )
	ROM_LOAD32_WORD("mpr-20791.4",  0x1800000, 0x400000, CRC(4883d0df) SHA1(b98af63e81f6c1b2766d7e96acbd1821bba000d4) )
	ROM_LOAD32_WORD("mpr-20792.5",  0x1800002, 0x400000, CRC(47becfa2) SHA1(a333885872a64b322f3cb464a70352d73654b1b3) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20799.16", 0x0000000, 0x400000, CRC(424571bf) SHA1(18a4e8d0e968fff3b645b59a0023b0ef38d51924) )
	ROM_LOAD32_WORD("mpr-20803.20", 0x0000002, 0x400000, CRC(61a8ad52) SHA1(0215b5de6d10f0852ac0ca4e10475e10243e39c7) )
	ROM_LOAD32_WORD("mpr-20800.17", 0x0800000, 0x400000, CRC(3c2ee808) SHA1(dc0c470c6b410ab991ef0e09ce1cc0f63c8a204d) )
	ROM_LOAD32_WORD("mpr-20804.21", 0x0800002, 0x400000, CRC(03b35cb8) SHA1(7bd2ae89f9cc7c0570dbaffe5f54aea2dfa1b39e) )
	ROM_LOAD32_WORD("mpr-20801.18", 0x1000000, 0x400000, CRC(c6914173) SHA1(d0861366c4123c833a325df5345f951386a94d1a) )
	ROM_LOAD32_WORD("mpr-20805.22", 0x1000002, 0x400000, CRC(f6605ede) SHA1(7c95bfe2e95bae3d59c3c9efe1f40b5bc292ad44) )
	ROM_LOAD32_WORD("mpr-20802.19", 0x1800000, 0x400000, CRC(d11b5267) SHA1(b90909849fbe0f62d5ec7c38608c84e7fa845ebf) )
	ROM_LOAD32_WORD("mpr-20806.23", 0x1800002, 0x400000, CRC(0c942073) SHA1(5f32a56857e2213b110c32deea184dba882e34b8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20809.25", 0x0000000, 0x400000, CRC(3b7b4622) SHA1(c6f1a1fd2684f352d3846b7f859b0405fa2d667a) )
	ROM_LOAD32_WORD("mpr-20807.24", 0x0000002, 0x400000, CRC(1241e0f2) SHA1(3f7fa1d7d3d398bc8d5295bc1df6fe11405d20d9) )
	ROM_LOAD32_WORD("mpr-20810.27", 0x0800000, 0x400000, CRC(838a10a7) SHA1(a658f1864829058b1d419e7c001e47cd0ab06a20) )
	ROM_LOAD32_WORD("mpr-20808.26", 0x0800002, 0x400000, CRC(706bd495) SHA1(f857b303afda6301b19d97dfe5c313126261716e) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20811.30", 0x080000,  0x80000, CRC(a154b83e) SHA1(2640c6b6966f4a888329e583b6b713bd0e779b6b) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20812.31", 0x000000, 0x200000, CRC(695b6088) SHA1(09682d18144e60d740a9d7a3e19db6f76fa581f1) )
	ROM_LOAD("mpr-20813.32", 0x200000, 0x200000, CRC(1908679c) SHA1(32913385f09da2e43af0c4a4612b955527bfe759) )
	ROM_LOAD("mpr-20814.36", 0x400000, 0x200000, CRC(e8ebc74c) SHA1(731ce721bb9e148f3a9f7fbe569522567a681c4e) )
	ROM_LOAD("mpr-20815.37", 0x600000, 0x200000, CRC(1b5aaae4) SHA1(32b4bf6c096fdccdd5d8f1ddb6c27d3389a52234) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD

	//             1998     317-0236-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "2c2a4a93" )
ROM_END

ROM_START( dyndeka2 ) /* Dynamite Deka 2 (Japan), Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20922.12",  0x000000, 0x080000, CRC(0a8b5604) SHA1(4076998fc600c1df3bb5ef48d42681c01e651495) )
	ROM_LOAD32_WORD("epr-20923.13",  0x000002, 0x080000, CRC(83be73d4) SHA1(1404a9c79cd2bae13f60e5e008307417324c3666) )
	ROM_LOAD32_WORD("epr-20924.14",  0x100000, 0x080000, CRC(618a68bf) SHA1(3022283dded4d08d790d034b6d543c0397b5bf5a) ) /* same as epr-20932.14 listed above */
	ROM_LOAD32_WORD("epr-20925.15",  0x100002, 0x080000, CRC(13abe49c) SHA1(a741a0205c1b3664ab4d09d6d991a768269a79ea) ) /* same as epr-20933.15 listed above */

	ROM_REGION32_LE( 0x2400000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20797.10", 0x0000000, 0x400000, CRC(87bab1e4) SHA1(af2b2d82364621a1528d6ed59fbfbf15dc93ee72) )
	ROM_LOAD32_WORD("mpr-20798.11", 0x0000002, 0x400000, CRC(40dd752b) SHA1(8c2e210ac7c7b133ba9befc79a07c4ca6b4e3f18) )
	ROM_LOAD32_WORD("mpr-20795.8",  0x0800000, 0x400000, CRC(0ef85e12) SHA1(97c657edd98cde6f0780a04a7711e5b370087a60) )
	ROM_LOAD32_WORD("mpr-20796.9",  0x0800002, 0x400000, CRC(870139cb) SHA1(24fda2cd458cf7a3db485564c02ac61d30cbdf5e) )
	ROM_LOAD32_WORD("mpr-20793.6",  0x1000000, 0x400000, CRC(42ea08f8) SHA1(e70b55709067628ea0bf3f5190a300100b61eed1) )
	ROM_LOAD32_WORD("mpr-20794.7",  0x1000002, 0x400000, CRC(8e5cd1db) SHA1(d90e86d38bda12f2d0f99e23a42928f05bde3ea8) )
	ROM_LOAD32_WORD("mpr-20791.4",  0x1800000, 0x400000, CRC(4883d0df) SHA1(b98af63e81f6c1b2766d7e96acbd1821bba000d4) )
	ROM_LOAD32_WORD("mpr-20792.5",  0x1800002, 0x400000, CRC(47becfa2) SHA1(a333885872a64b322f3cb464a70352d73654b1b3) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20799.16", 0x0000000, 0x400000, CRC(424571bf) SHA1(18a4e8d0e968fff3b645b59a0023b0ef38d51924) )
	ROM_LOAD32_WORD("mpr-20803.20", 0x0000002, 0x400000, CRC(61a8ad52) SHA1(0215b5de6d10f0852ac0ca4e10475e10243e39c7) )
	ROM_LOAD32_WORD("mpr-20800.17", 0x0800000, 0x400000, CRC(3c2ee808) SHA1(dc0c470c6b410ab991ef0e09ce1cc0f63c8a204d) )
	ROM_LOAD32_WORD("mpr-20804.21", 0x0800002, 0x400000, CRC(03b35cb8) SHA1(7bd2ae89f9cc7c0570dbaffe5f54aea2dfa1b39e) )
	ROM_LOAD32_WORD("mpr-20801.18", 0x1000000, 0x400000, CRC(c6914173) SHA1(d0861366c4123c833a325df5345f951386a94d1a) )
	ROM_LOAD32_WORD("mpr-20805.22", 0x1000002, 0x400000, CRC(f6605ede) SHA1(7c95bfe2e95bae3d59c3c9efe1f40b5bc292ad44) )
	ROM_LOAD32_WORD("mpr-20802.19", 0x1800000, 0x400000, CRC(d11b5267) SHA1(b90909849fbe0f62d5ec7c38608c84e7fa845ebf) )
	ROM_LOAD32_WORD("mpr-20806.23", 0x1800002, 0x400000, CRC(0c942073) SHA1(5f32a56857e2213b110c32deea184dba882e34b8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20809.25", 0x0000000, 0x400000, CRC(3b7b4622) SHA1(c6f1a1fd2684f352d3846b7f859b0405fa2d667a) )
	ROM_LOAD32_WORD("mpr-20807.24", 0x0000002, 0x400000, CRC(1241e0f2) SHA1(3f7fa1d7d3d398bc8d5295bc1df6fe11405d20d9) )
	ROM_LOAD32_WORD("mpr-20810.27", 0x0800000, 0x400000, CRC(838a10a7) SHA1(a658f1864829058b1d419e7c001e47cd0ab06a20) )
	ROM_LOAD32_WORD("mpr-20808.26", 0x0800002, 0x400000, CRC(706bd495) SHA1(f857b303afda6301b19d97dfe5c313126261716e) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20811.30", 0x080000,  0x80000, CRC(a154b83e) SHA1(2640c6b6966f4a888329e583b6b713bd0e779b6b) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20812.31", 0x000000, 0x200000, CRC(695b6088) SHA1(09682d18144e60d740a9d7a3e19db6f76fa581f1) )
	ROM_LOAD("mpr-20813.32", 0x200000, 0x200000, CRC(1908679c) SHA1(32913385f09da2e43af0c4a4612b955527bfe759) )
	ROM_LOAD("mpr-20814.36", 0x400000, 0x200000, CRC(e8ebc74c) SHA1(731ce721bb9e148f3a9f7fbe569522567a681c4e) )
	ROM_LOAD("mpr-20815.37", 0x600000, 0x200000, CRC(1b5aaae4) SHA1(32b4bf6c096fdccdd5d8f1ddb6c27d3389a52234) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD

	//             1998     317-0236-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "2c2a4a93" )
ROM_END

ROM_START( dynamcopb ) /* Dynamite Cop (Export), Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20944.15", 0x000000, 0x080000, CRC(29b142f2) SHA1(b81d1ee7203b2f5fb6db4ff4185f4071e99aaedf) )
	ROM_LOAD32_WORD("epr-20945.16", 0x000002, 0x080000, CRC(c495912e) SHA1(1a45296a5554923cb52b38586e40ceda2517f1bf) )
	ROM_LOAD32_WORD("epr-20942.13", 0x100000, 0x080000, CRC(618a68bf) SHA1(3022283dded4d08d790d034b6d543c0397b5bf5a) ) /* same as epr-20932.14 listed above */
	ROM_LOAD32_WORD("epr-20943.14", 0x100002, 0x080000, CRC(13abe49c) SHA1(a741a0205c1b3664ab4d09d6d991a768269a79ea) ) /* same as epr-20933.15 listed above */

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20797.10", 0x0000000, 0x400000, CRC(87bab1e4) SHA1(af2b2d82364621a1528d6ed59fbfbf15dc93ee72) )
	ROM_LOAD32_WORD("mpr-20798.11", 0x0000002, 0x400000, CRC(40dd752b) SHA1(8c2e210ac7c7b133ba9befc79a07c4ca6b4e3f18) )
	ROM_LOAD32_WORD("mpr-20795.8",  0x0800000, 0x400000, CRC(0ef85e12) SHA1(97c657edd98cde6f0780a04a7711e5b370087a60) )
	ROM_LOAD32_WORD("mpr-20796.9",  0x0800002, 0x400000, CRC(870139cb) SHA1(24fda2cd458cf7a3db485564c02ac61d30cbdf5e) )
	ROM_LOAD32_WORD("mpr-20793.6",  0x1000000, 0x400000, CRC(42ea08f8) SHA1(e70b55709067628ea0bf3f5190a300100b61eed1) )
	ROM_LOAD32_WORD("mpr-20794.7",  0x1000002, 0x400000, CRC(8e5cd1db) SHA1(d90e86d38bda12f2d0f99e23a42928f05bde3ea8) )
	ROM_LOAD32_WORD("mpr-20791.4",  0x1800000, 0x400000, CRC(4883d0df) SHA1(b98af63e81f6c1b2766d7e96acbd1821bba000d4) )
	ROM_LOAD32_WORD("mpr-20792.5",  0x1800002, 0x400000, CRC(47becfa2) SHA1(a333885872a64b322f3cb464a70352d73654b1b3) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20799.16", 0x0000000, 0x400000, CRC(424571bf) SHA1(18a4e8d0e968fff3b645b59a0023b0ef38d51924) )
	ROM_LOAD32_WORD("mpr-20803.20", 0x0000002, 0x400000, CRC(61a8ad52) SHA1(0215b5de6d10f0852ac0ca4e10475e10243e39c7) )
	ROM_LOAD32_WORD("mpr-20800.17", 0x0800000, 0x400000, CRC(3c2ee808) SHA1(dc0c470c6b410ab991ef0e09ce1cc0f63c8a204d) )
	ROM_LOAD32_WORD("mpr-20804.21", 0x0800002, 0x400000, CRC(03b35cb8) SHA1(7bd2ae89f9cc7c0570dbaffe5f54aea2dfa1b39e) )
	ROM_LOAD32_WORD("mpr-20801.18", 0x1000000, 0x400000, CRC(c6914173) SHA1(d0861366c4123c833a325df5345f951386a94d1a) )
	ROM_LOAD32_WORD("mpr-20805.22", 0x1000002, 0x400000, CRC(f6605ede) SHA1(7c95bfe2e95bae3d59c3c9efe1f40b5bc292ad44) )
	ROM_LOAD32_WORD("mpr-20802.19", 0x1800000, 0x400000, CRC(d11b5267) SHA1(b90909849fbe0f62d5ec7c38608c84e7fa845ebf) )
	ROM_LOAD32_WORD("mpr-20806.23", 0x1800002, 0x400000, CRC(0c942073) SHA1(5f32a56857e2213b110c32deea184dba882e34b8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20809.25", 0x0000000, 0x400000, CRC(3b7b4622) SHA1(c6f1a1fd2684f352d3846b7f859b0405fa2d667a) )
	ROM_LOAD32_WORD("mpr-20807.24", 0x0000002, 0x400000, CRC(1241e0f2) SHA1(3f7fa1d7d3d398bc8d5295bc1df6fe11405d20d9) )
	ROM_LOAD32_WORD("mpr-20810.27", 0x0800000, 0x400000, CRC(838a10a7) SHA1(a658f1864829058b1d419e7c001e47cd0ab06a20) )
	ROM_LOAD32_WORD("mpr-20808.26", 0x0800002, 0x400000, CRC(706bd495) SHA1(f857b303afda6301b19d97dfe5c313126261716e) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20811.30", 0x080000,  0x80000, CRC(a154b83e) SHA1(2640c6b6966f4a888329e583b6b713bd0e779b6b) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20812.31", 0x000000, 0x200000, CRC(695b6088) SHA1(09682d18144e60d740a9d7a3e19db6f76fa581f1) )
	ROM_LOAD("mpr-20813.32", 0x200000, 0x200000, CRC(1908679c) SHA1(32913385f09da2e43af0c4a4612b955527bfe759) )
	ROM_LOAD("mpr-20814.36", 0x400000, 0x200000, CRC(e8ebc74c) SHA1(731ce721bb9e148f3a9f7fbe569522567a681c4e) )
	ROM_LOAD("mpr-20815.37", 0x600000, 0x200000, CRC(1b5aaae4) SHA1(32b4bf6c096fdccdd5d8f1ddb6c27d3389a52234) )

	//             1998     317-0236-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "2c2a4a93" )
ROM_END

ROM_START( dyndeka2b ) /* Dynamite Deka 2 (Japan), Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20936.15", 0x000000, 0x080000, CRC(23ef98f2) SHA1(0a106125ed4a2569b54924130ca2ffa05acf2322) )
	ROM_LOAD32_WORD("epr-20937.16", 0x000002, 0x080000, CRC(25a14e00) SHA1(ebdd21f269fd8a0798306e349d2985eead7e989f) )
	ROM_LOAD32_WORD("epr-20934.13", 0x100000, 0x080000, CRC(618a68bf) SHA1(3022283dded4d08d790d034b6d543c0397b5bf5a) ) /* same as epr-20932.14 listed above */
	ROM_LOAD32_WORD("epr-20935.14", 0x100002, 0x080000, CRC(13abe49c) SHA1(a741a0205c1b3664ab4d09d6d991a768269a79ea) ) /* same as epr-20933.15 listed above */

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20797.10", 0x0000000, 0x400000, CRC(87bab1e4) SHA1(af2b2d82364621a1528d6ed59fbfbf15dc93ee72) )
	ROM_LOAD32_WORD("mpr-20798.11", 0x0000002, 0x400000, CRC(40dd752b) SHA1(8c2e210ac7c7b133ba9befc79a07c4ca6b4e3f18) )
	ROM_LOAD32_WORD("mpr-20795.8",  0x0800000, 0x400000, CRC(0ef85e12) SHA1(97c657edd98cde6f0780a04a7711e5b370087a60) )
	ROM_LOAD32_WORD("mpr-20796.9",  0x0800002, 0x400000, CRC(870139cb) SHA1(24fda2cd458cf7a3db485564c02ac61d30cbdf5e) )
	ROM_LOAD32_WORD("mpr-20793.6",  0x1000000, 0x400000, CRC(42ea08f8) SHA1(e70b55709067628ea0bf3f5190a300100b61eed1) )
	ROM_LOAD32_WORD("mpr-20794.7",  0x1000002, 0x400000, CRC(8e5cd1db) SHA1(d90e86d38bda12f2d0f99e23a42928f05bde3ea8) )
	ROM_LOAD32_WORD("mpr-20791.4",  0x1800000, 0x400000, CRC(4883d0df) SHA1(b98af63e81f6c1b2766d7e96acbd1821bba000d4) )
	ROM_LOAD32_WORD("mpr-20792.5",  0x1800002, 0x400000, CRC(47becfa2) SHA1(a333885872a64b322f3cb464a70352d73654b1b3) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20799.16", 0x0000000, 0x400000, CRC(424571bf) SHA1(18a4e8d0e968fff3b645b59a0023b0ef38d51924) )
	ROM_LOAD32_WORD("mpr-20803.20", 0x0000002, 0x400000, CRC(61a8ad52) SHA1(0215b5de6d10f0852ac0ca4e10475e10243e39c7) )
	ROM_LOAD32_WORD("mpr-20800.17", 0x0800000, 0x400000, CRC(3c2ee808) SHA1(dc0c470c6b410ab991ef0e09ce1cc0f63c8a204d) )
	ROM_LOAD32_WORD("mpr-20804.21", 0x0800002, 0x400000, CRC(03b35cb8) SHA1(7bd2ae89f9cc7c0570dbaffe5f54aea2dfa1b39e) )
	ROM_LOAD32_WORD("mpr-20801.18", 0x1000000, 0x400000, CRC(c6914173) SHA1(d0861366c4123c833a325df5345f951386a94d1a) )
	ROM_LOAD32_WORD("mpr-20805.22", 0x1000002, 0x400000, CRC(f6605ede) SHA1(7c95bfe2e95bae3d59c3c9efe1f40b5bc292ad44) )
	ROM_LOAD32_WORD("mpr-20802.19", 0x1800000, 0x400000, CRC(d11b5267) SHA1(b90909849fbe0f62d5ec7c38608c84e7fa845ebf) )
	ROM_LOAD32_WORD("mpr-20806.23", 0x1800002, 0x400000, CRC(0c942073) SHA1(5f32a56857e2213b110c32deea184dba882e34b8) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20809.25", 0x0000000, 0x400000, CRC(3b7b4622) SHA1(c6f1a1fd2684f352d3846b7f859b0405fa2d667a) )
	ROM_LOAD32_WORD("mpr-20807.24", 0x0000002, 0x400000, CRC(1241e0f2) SHA1(3f7fa1d7d3d398bc8d5295bc1df6fe11405d20d9) )
	ROM_LOAD32_WORD("mpr-20810.27", 0x0800000, 0x400000, CRC(838a10a7) SHA1(a658f1864829058b1d419e7c001e47cd0ab06a20) )
	ROM_LOAD32_WORD("mpr-20808.26", 0x0800002, 0x400000, CRC(706bd495) SHA1(f857b303afda6301b19d97dfe5c313126261716e) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20811.30", 0x080000,  0x80000, CRC(a154b83e) SHA1(2640c6b6966f4a888329e583b6b713bd0e779b6b) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20812.31", 0x000000, 0x200000, CRC(695b6088) SHA1(09682d18144e60d740a9d7a3e19db6f76fa581f1) )
	ROM_LOAD("mpr-20813.32", 0x200000, 0x200000, CRC(1908679c) SHA1(32913385f09da2e43af0c4a4612b955527bfe759) )
	ROM_LOAD("mpr-20814.36", 0x400000, 0x200000, CRC(e8ebc74c) SHA1(731ce721bb9e148f3a9f7fbe569522567a681c4e) )
	ROM_LOAD("mpr-20815.37", 0x600000, 0x200000, CRC(1b5aaae4) SHA1(32b4bf6c096fdccdd5d8f1ddb6c27d3389a52234) )

	//             1998     317-0236-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "2c2a4a93" )
ROM_END

ROM_START( dynamcopc ) /* Dynamite Cop (USA), Model 2C */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20952.15", 0x000000, 0x080000, CRC(ec8bc896) SHA1(85deb1dc1348730a0c9b6ce3679582e7894ff2ed) )
	ROM_LOAD32_WORD("epr-20953.16", 0x000002, 0x080000, CRC(a8276ffd) SHA1(9bea99c043775c00742c20e2f917d211dca09cc5) )
	ROM_LOAD32_WORD("epr-20950.13", 0x100000, 0x080000, CRC(618a68bf) SHA1(3022283dded4d08d790d034b6d543c0397b5bf5a) ) /* same as epr-20932.14 listed above */
	ROM_LOAD32_WORD("epr-20951.14", 0x100002, 0x080000, CRC(13abe49c) SHA1(a741a0205c1b3664ab4d09d6d991a768269a79ea) ) /* same as epr-20933.15 listed above */

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20797.10", 0x0000000, 0x400000, CRC(87bab1e4) SHA1(af2b2d82364621a1528d6ed59fbfbf15dc93ee72) ) /* Located at position 11 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20798.11", 0x0000002, 0x400000, CRC(40dd752b) SHA1(8c2e210ac7c7b133ba9befc79a07c4ca6b4e3f18) ) /* Located at position 12 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20795.8",  0x0800000, 0x400000, CRC(0ef85e12) SHA1(97c657edd98cde6f0780a04a7711e5b370087a60) ) /* Located at position  9 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20796.9",  0x0800002, 0x400000, CRC(870139cb) SHA1(24fda2cd458cf7a3db485564c02ac61d30cbdf5e) ) /* Located at position 10 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20793.6",  0x1000000, 0x400000, CRC(42ea08f8) SHA1(e70b55709067628ea0bf3f5190a300100b61eed1) ) /* Located at position  7 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20794.7",  0x1000002, 0x400000, CRC(8e5cd1db) SHA1(d90e86d38bda12f2d0f99e23a42928f05bde3ea8) ) /* Located at position  8 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20791.4",  0x1800000, 0x400000, CRC(4883d0df) SHA1(b98af63e81f6c1b2766d7e96acbd1821bba000d4) ) /* Located at position  5 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20792.5",  0x1800002, 0x400000, CRC(47becfa2) SHA1(a333885872a64b322f3cb464a70352d73654b1b3) ) /* Located at position  6 on 2C-CRX rom board */

	ROM_REGION( 0x800000, "cpu2", ROMREGION_ERASE00 ) // TGPx4 program (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20799.16", 0x0000000, 0x400000, CRC(424571bf) SHA1(18a4e8d0e968fff3b645b59a0023b0ef38d51924) ) /* Located at position 17 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20803.20", 0x0000002, 0x400000, CRC(61a8ad52) SHA1(0215b5de6d10f0852ac0ca4e10475e10243e39c7) ) /* Located at position 21 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20800.17", 0x0800000, 0x400000, CRC(3c2ee808) SHA1(dc0c470c6b410ab991ef0e09ce1cc0f63c8a204d) ) /* Located at position 18 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20804.21", 0x0800002, 0x400000, CRC(03b35cb8) SHA1(7bd2ae89f9cc7c0570dbaffe5f54aea2dfa1b39e) ) /* Located at position 22 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20801.18", 0x1000000, 0x400000, CRC(c6914173) SHA1(d0861366c4123c833a325df5345f951386a94d1a) ) /* Located at position 19 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20805.22", 0x1000002, 0x400000, CRC(f6605ede) SHA1(7c95bfe2e95bae3d59c3c9efe1f40b5bc292ad44) ) /* Located at position 23 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20802.19", 0x1800000, 0x400000, CRC(d11b5267) SHA1(b90909849fbe0f62d5ec7c38608c84e7fa845ebf) ) /* Located at position 20 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20806.23", 0x1800002, 0x400000, CRC(0c942073) SHA1(5f32a56857e2213b110c32deea184dba882e34b8) ) /* Located at position 24 on 2C-CRX rom board */

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20809.25", 0x0000000, 0x400000, CRC(3b7b4622) SHA1(c6f1a1fd2684f352d3846b7f859b0405fa2d667a) ) /* Located at position 27 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20807.24", 0x0000002, 0x400000, CRC(1241e0f2) SHA1(3f7fa1d7d3d398bc8d5295bc1df6fe11405d20d9) ) /* Located at position 25 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20810.27", 0x0800000, 0x400000, CRC(838a10a7) SHA1(a658f1864829058b1d419e7c001e47cd0ab06a20) ) /* Located at position 28 on 2C-CRX rom board */
	ROM_LOAD32_WORD("mpr-20808.26", 0x0800002, 0x400000, CRC(706bd495) SHA1(f857b303afda6301b19d97dfe5c313126261716e) ) /* Located at position 26 on 2C-CRX rom board */

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-20811.30", 0x080000,  0x80000, CRC(a154b83e) SHA1(2640c6b6966f4a888329e583b6b713bd0e779b6b) ) /* Located at position 31 on 2C-CRX rom board */

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20812.31", 0x000000, 0x200000, CRC(695b6088) SHA1(09682d18144e60d740a9d7a3e19db6f76fa581f1) ) /* Located at position 32 on 2C-CRX rom board */
	ROM_LOAD("mpr-20813.32", 0x200000, 0x200000, CRC(1908679c) SHA1(32913385f09da2e43af0c4a4612b955527bfe759) ) /* Located at position 33 on 2C-CRX rom board */
	ROM_LOAD("mpr-20814.36", 0x400000, 0x200000, CRC(e8ebc74c) SHA1(731ce721bb9e148f3a9f7fbe569522567a681c4e) ) /* Located at position 34 on 2C-CRX rom board */
	ROM_LOAD("mpr-20815.37", 0x600000, 0x200000, CRC(1b5aaae4) SHA1(32b4bf6c096fdccdd5d8f1ddb6c27d3389a52234) ) /* Located at position 35 on 2C-CRX rom board */

	//             1998     317-0236-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "2c2a4a93" )
ROM_END

ROM_START( schamp ) /* Sonic Championship, Model 2B - Sega ROM board ID# 834-12786 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19141.15", 0x000000, 0x080000, CRC(b942ef21) SHA1(2372412d49349894c99d545313c12413c2d1ec86) ) /* Default country is USA, game title is "Sonic Championship" when region */
	ROM_LOAD32_WORD("epr-19142.16", 0x000002, 0x080000, CRC(2d54bd76) SHA1(9456fb9a847e01548fc30d36ef161325788653d5) ) /*  is USA or Export; "Sonic the Fighters" when set to Japan */

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19007.11",    0x0000000, 0x400000, CRC(8b8ff751) SHA1(5343a9a2502052e3587424c984bd48caa7564849) )
	ROM_LOAD32_WORD("mpr-19008.12",    0x0000002, 0x400000, CRC(a94654f5) SHA1(39ad2e9431543ea6cbc0307bc39933cf64956a74) )
	ROM_LOAD32_WORD("mpr-19005.9",     0x0800000, 0x400000, CRC(98cd1127) SHA1(300c9cdef199f31255bacb95399e9c75be73f817) )
	ROM_LOAD32_WORD("mpr-19006.10",    0x0800002, 0x400000, CRC(e79f0a26) SHA1(37a4ff13cfccfda587ca59a9ef08b5914d2c28d4) )
	ROM_LOAD32_WORD("epr-19143.7",     0x1000000, 0x080000, CRC(f97176fd) SHA1(8c9d871d4639563f8298a7f93032e07a8f863faa) )
	ROM_LOAD32_WORD("epr-19144.8",     0x1000002, 0x080000, CRC(d040202a) SHA1(950ad9174196e776881439545983f91655922a49) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("mpr-19015.29", 0x000000, 0x200000, CRC(c74d99e3) SHA1(9914be9925b86af6af670745b5eba3a9e4f24af9) )
	ROM_LOAD32_WORD("mpr-19016.30", 0x000002, 0x200000, CRC(746ae931) SHA1(a6f0f589ad174a34493ee24dc0cb509ead3aed70) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19009.17", 0x000000, 0x400000, CRC(fd410350) SHA1(5af3a90c87ec8a90a8fc58ae469ef23ec6e6213c) )
	ROM_LOAD32_WORD("mpr-19012.21", 0x000002, 0x400000, CRC(9bb7b5b6) SHA1(8e13a0bb34e187a340b38d76ab15ff6fe4bae764) )
	ROM_LOAD32_WORD("mpr-19010.18", 0x800000, 0x400000, CRC(6fd94187) SHA1(e3318ef0eb0168998e139e527339c7c667c17fb1) )
	ROM_LOAD32_WORD("mpr-19013.22", 0x800002, 0x400000, CRC(9e232fe5) SHA1(a6c4b2b3bf8efc6f6263f73d6f4cacf9785010c1) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19019.27", 0x000000, 0x400000, CRC(59121896) SHA1(c29bedb41b14d63c6067ae12ad009deaafca2aa4) )
	ROM_LOAD32_WORD("mpr-19017.25", 0x000002, 0x400000, CRC(7b298379) SHA1(52fad61412040c90c7dd300c0fd7aa5b8d5af441) )
	ROM_LOAD32_WORD("mpr-19020.28", 0x800000, 0x400000, CRC(9540dba0) SHA1(7b9a75caa8c5b12ba54c6f4f746d80b165ee97ab) )
	ROM_LOAD32_WORD("mpr-19018.26", 0x800002, 0x400000, CRC(3b7e7a12) SHA1(9c707a7c2cffc5eff19f9919ddfae7300842fd19) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19021.31", 0x080000,  0x80000, CRC(0b9f7583) SHA1(21290389cd8bd9e52ed438152cc6cb5793f809d3) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19022.32", 0x000000, 0x200000, CRC(4381869b) SHA1(43a21609b49926a227558d4938088526acf1fe42) )
	ROM_LOAD("mpr-19023.33", 0x200000, 0x200000, CRC(07c67f88) SHA1(696dc85e066fb27c7618e52e0acd0d00451e4589) )
	ROM_LOAD("mpr-19024.34", 0x400000, 0x200000, CRC(15ff76d3) SHA1(b431bd85c973aa0a4d6032ac98fb057139f142a2) )
	ROM_LOAD("mpr-19025.35", 0x600000, 0x200000, CRC(6ad8fb70) SHA1(b666d31f9be26eb0cdcb71041a3c3c08d5aa41e1) )
ROM_END

ROM_START( sfight ) /* Sonic The Fighters, Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19001.15", 0x000000, 0x080000, CRC(9b088511) SHA1(20718d985d14f4d2b1b8e982bfbebddd73cdb972) ) /* Default country is Japan, the game title is "Sonic the Fighters" */
	ROM_LOAD32_WORD("epr-19002.16", 0x000002, 0x080000, CRC(46f510da) SHA1(edcbf61122db568ccaa4c3106f507087c1740c9b) ) /*  in all regions */

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19007.11",    0x0000000, 0x400000, CRC(8b8ff751) SHA1(5343a9a2502052e3587424c984bd48caa7564849) )
	ROM_LOAD32_WORD("mpr-19008.12",    0x0000002, 0x400000, CRC(a94654f5) SHA1(39ad2e9431543ea6cbc0307bc39933cf64956a74) )
	ROM_LOAD32_WORD("mpr-19005.9",     0x0800000, 0x400000, CRC(98cd1127) SHA1(300c9cdef199f31255bacb95399e9c75be73f817) )
	ROM_LOAD32_WORD("mpr-19006.10",    0x0800002, 0x400000, CRC(e79f0a26) SHA1(37a4ff13cfccfda587ca59a9ef08b5914d2c28d4) )
	ROM_LOAD32_WORD("epr-19003.7",     0x1000000, 0x080000, CRC(63bae5c5) SHA1(cbd55b7b7376ac2f67befaf4c43eef3727ba7b7f) )
	ROM_LOAD32_WORD("epr-19004.8",     0x1000002, 0x080000, CRC(c10c9f39) SHA1(cf806501dbfa48d16cb7ed5f39a6146f734ba455) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("mpr-19015.29", 0x000000, 0x200000, CRC(c74d99e3) SHA1(9914be9925b86af6af670745b5eba3a9e4f24af9) )
	ROM_LOAD32_WORD("mpr-19016.30", 0x000002, 0x200000, CRC(746ae931) SHA1(a6f0f589ad174a34493ee24dc0cb509ead3aed70) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19009.17", 0x000000, 0x400000, CRC(fd410350) SHA1(5af3a90c87ec8a90a8fc58ae469ef23ec6e6213c) )
	ROM_LOAD32_WORD("mpr-19012.21", 0x000002, 0x400000, CRC(9bb7b5b6) SHA1(8e13a0bb34e187a340b38d76ab15ff6fe4bae764) )
	ROM_LOAD32_WORD("mpr-19010.18", 0x800000, 0x400000, CRC(6fd94187) SHA1(e3318ef0eb0168998e139e527339c7c667c17fb1) )
	ROM_LOAD32_WORD("mpr-19013.22", 0x800002, 0x400000, CRC(9e232fe5) SHA1(a6c4b2b3bf8efc6f6263f73d6f4cacf9785010c1) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19019.27", 0x000000, 0x400000, CRC(59121896) SHA1(c29bedb41b14d63c6067ae12ad009deaafca2aa4) )
	ROM_LOAD32_WORD("mpr-19017.25", 0x000002, 0x400000, CRC(7b298379) SHA1(52fad61412040c90c7dd300c0fd7aa5b8d5af441) )
	ROM_LOAD32_WORD("mpr-19020.28", 0x800000, 0x400000, CRC(9540dba0) SHA1(7b9a75caa8c5b12ba54c6f4f746d80b165ee97ab) )
	ROM_LOAD32_WORD("mpr-19018.26", 0x800002, 0x400000, CRC(3b7e7a12) SHA1(9c707a7c2cffc5eff19f9919ddfae7300842fd19) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19021.31", 0x080000,  0x80000, CRC(0b9f7583) SHA1(21290389cd8bd9e52ed438152cc6cb5793f809d3) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19022.32", 0x000000, 0x200000, CRC(4381869b) SHA1(43a21609b49926a227558d4938088526acf1fe42) )
	ROM_LOAD("mpr-19023.33", 0x200000, 0x200000, CRC(07c67f88) SHA1(696dc85e066fb27c7618e52e0acd0d00451e4589) )
	ROM_LOAD("mpr-19024.34", 0x400000, 0x200000, CRC(15ff76d3) SHA1(b431bd85c973aa0a4d6032ac98fb057139f142a2) )
	ROM_LOAD("mpr-19025.35", 0x600000, 0x200000, CRC(6ad8fb70) SHA1(b666d31f9be26eb0cdcb71041a3c3c08d5aa41e1) )
ROM_END

ROM_START( stcc ) /* Sega Touring Car Championship, Model 2C - Defaults to Japan, Twin & Default View set to Bird's */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19570.15", 0x000000, 0x080000, CRC(9d6a1965) SHA1(6f1e75d86a37b8579f968b2fb51d32a1a860697f) ) /* Higher rom numbers indicate a newer version */
	ROM_LOAD32_WORD("epr-19571.16", 0x000002, 0x080000, CRC(97254d16) SHA1(f9154cd9e954b16f7f45019b4758b1f971190437) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19257.11",  0x000000, 0x400000, CRC(ac28ee24) SHA1(31d360dc435336942f70365d0491a2ccfc24c4c0) )
	ROM_LOAD32_WORD("mpr-19258.12",  0x000002, 0x400000, CRC(f5ba7d78) SHA1(9c8304a1f856d1ded869ed2b86de52129510f019) )
	ROM_LOAD32_WORD("epr-19270.9",   0x800000, 0x080000, CRC(7bd1d04e) SHA1(0490f3abc97af16e05f0dc9623e8fc635b1d4262) )
	ROM_LOAD32_WORD("epr-19271.10",  0x800002, 0x080000, CRC(d2d74f85) SHA1(49e7a1e6478122b4f0e679d7b336fb34044b503b) )
	ROM_COPY("user1", 0x800000, 0x900000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xa00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xb00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xc00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xd00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xe00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xf00000, 0x100000)

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-19255.29", 0x000000, 0x200000, CRC(d78bf030) SHA1(e6b3d8422613d22db50cf6c251f9a21356d96653) )
	ROM_LOAD32_WORD("mpr-19256.30", 0x000002, 0x200000, CRC(cb2b2d9e) SHA1(86b2b8bb6074352f72eb81e616093a1ba6f5163f) )

	ROM_REGION( 0x900000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19251.17", 0x000000, 0x400000, CRC(e06ff7ba) SHA1(6d472d03cd3caeb66be929c74ae63c32d305a3db) )
	ROM_LOAD32_WORD("mpr-19252.21", 0x000002, 0x400000, CRC(68509993) SHA1(654d5cdf44e7e1e788b26593f418ce76a5c1165a) )
	ROM_LOAD32_WORD("epr-19266.18", 0x800000, 0x080000, CRC(41464ee2) SHA1(afbbc0328bd36c34c69f0f54404dfd6a64036417) )
	ROM_LOAD32_WORD("epr-19267.22", 0x800002, 0x080000, CRC(780f994d) SHA1(f134482ed0fcfc7b3eea39947da47081301a111a) )

	ROM_REGION( 0x900000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19254.27", 0x000000, 0x200000, CRC(1ec49c02) SHA1(a9bdbab7b4b265c9118cf27fd45ca94f4516d5c6) )
	ROM_RELOAD     (                0x400000, 0x200000 )
	ROM_LOAD32_WORD("mpr-19253.25", 0x000002, 0x200000, CRC(41ba79fb) SHA1(f4d8a4f8278eec6d528bd947b91ebeb5223559d5) )
	ROM_RELOAD     (                0x400002, 0x200000 )
	ROM_LOAD32_WORD("epr-19269.28", 0x800000, 0x080000, CRC(01881121) SHA1(fe711709e70b3743b2a0318b823d859f233d3ff8) )
	ROM_LOAD32_WORD("epr-19268.26", 0x800002, 0x080000, CRC(bc4e081c) SHA1(b89d39ed19a146d1e94e52682f67d2cd23d8df7f) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19274.31", 0x080000,  0x20000, CRC(2dcc08ae) SHA1(bad26e2c994f2d4db5d9be0e34cf21a8bf5aa7e9) )

	ROM_REGION( 0x20000, "mpegcpu", 0) // Z80 DSB program
	ROM_LOAD("epr-19275.2s", 0x000000,  0x20000, CRC(ee809d3f) SHA1(347080858fbfe9955002f382603a1b86a52d26d5) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD16_WORD_SWAP("epr-18643a.7", 0x000000,  0x20000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD16_WORD_SWAP("mpr-19259.32", 0x000000, 0x400000, CRC(4d55dbfc) SHA1(6e57e6e6e785b0f14bb5e221a44d518dbde7ad65) )
	ROM_LOAD16_WORD_SWAP("mpr-19261.34", 0x400000, 0x400000, CRC(b88878ff) SHA1(4bebcfba68b0cc2fa0bcacfaaf2d2e8af3625c5d) )

	ROM_REGION( 0x800000, "mpeg", 0 ) // MPEG audio data
	ROM_LOAD("mpr-19262.57s", 0x000000, 0x200000, CRC(922aed7a) SHA1(8d6872bdd46eaf2076c10d18c10af8ccbd3b10e8) )
	ROM_LOAD("mpr-19263.58s", 0x200000, 0x200000, CRC(a256f4cd) SHA1(a17b49050f1ecf1970477b12201cc3b58b31d89c) )
	ROM_LOAD("mpr-19264.59s", 0x400000, 0x200000, CRC(b6c51d0f) SHA1(9e0969a1e49ec1462f69cd0f0f9ce630d66174ce) )
	ROM_LOAD("mpr-19265.60s", 0x600000, 0x200000, CRC(7d98700a) SHA1(bedd37314ecab424b5b27030e1e7dc1b596303f3) )

	ROM_REGION( 0x10000, "drive", 0 ) // drive board CPU (code is Z80 compatible)
	ROM_LOAD( "epr-18261.ic9", 0x000000, 0x010000, CRC(0c7fac58) SHA1(68c1724c41401e28a5123022981c8919fd22656e) )
ROM_END

ROM_START( stccb ) /* Sega Touring Car Championship Revision B, Model 2C - Defaults to Japan, Twin & Default View set to Driver's - Sega Game ID# 833-12779, Sega ROM board ID# 834-12780 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19272b.15", 0x000000, 0x080000, CRC(efdfb625) SHA1(5026e28b9d8267492bd0d9746d64526540a001da) )
	ROM_LOAD32_WORD("epr-19273b.16", 0x000002, 0x080000, CRC(61a357d9) SHA1(3f22f13a3baa46f93cb40e8af9534afaa57ead9c) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19257.11",  0x000000, 0x400000, CRC(ac28ee24) SHA1(31d360dc435336942f70365d0491a2ccfc24c4c0) )
	ROM_LOAD32_WORD("mpr-19258.12",  0x000002, 0x400000, CRC(f5ba7d78) SHA1(9c8304a1f856d1ded869ed2b86de52129510f019) )
	ROM_LOAD32_WORD("epr-19270.9",   0x800000, 0x080000, CRC(7bd1d04e) SHA1(0490f3abc97af16e05f0dc9623e8fc635b1d4262) )
	ROM_LOAD32_WORD("epr-19271.10",  0x800002, 0x080000, CRC(d2d74f85) SHA1(49e7a1e6478122b4f0e679d7b336fb34044b503b) )
	ROM_COPY("user1", 0x800000, 0x900000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xa00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xb00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xc00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xd00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xe00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xf00000, 0x100000)

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-19255.29", 0x000000, 0x200000, CRC(d78bf030) SHA1(e6b3d8422613d22db50cf6c251f9a21356d96653) )
	ROM_LOAD32_WORD("mpr-19256.30", 0x000002, 0x200000, CRC(cb2b2d9e) SHA1(86b2b8bb6074352f72eb81e616093a1ba6f5163f) )

	ROM_REGION( 0x900000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19251.17", 0x000000, 0x400000, CRC(e06ff7ba) SHA1(6d472d03cd3caeb66be929c74ae63c32d305a3db) )
	ROM_LOAD32_WORD("mpr-19252.21", 0x000002, 0x400000, CRC(68509993) SHA1(654d5cdf44e7e1e788b26593f418ce76a5c1165a) )
	ROM_LOAD32_WORD("epr-19266.18", 0x800000, 0x080000, CRC(41464ee2) SHA1(afbbc0328bd36c34c69f0f54404dfd6a64036417) )
	ROM_LOAD32_WORD("epr-19267.22", 0x800002, 0x080000, CRC(780f994d) SHA1(f134482ed0fcfc7b3eea39947da47081301a111a) )

	ROM_REGION( 0x900000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19254.27", 0x000000, 0x200000, CRC(1ec49c02) SHA1(a9bdbab7b4b265c9118cf27fd45ca94f4516d5c6) )
	ROM_RELOAD     (                0x400000, 0x200000 )
	ROM_LOAD32_WORD("mpr-19253.25", 0x000002, 0x200000, CRC(41ba79fb) SHA1(f4d8a4f8278eec6d528bd947b91ebeb5223559d5) )
	ROM_RELOAD     (                0x400002, 0x200000 )
	ROM_LOAD32_WORD("epr-19269.28", 0x800000, 0x080000, CRC(01881121) SHA1(fe711709e70b3743b2a0318b823d859f233d3ff8) )
	ROM_LOAD32_WORD("epr-19268.26", 0x800002, 0x080000, CRC(bc4e081c) SHA1(b89d39ed19a146d1e94e52682f67d2cd23d8df7f) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19274.31", 0x080000,  0x20000, CRC(2dcc08ae) SHA1(bad26e2c994f2d4db5d9be0e34cf21a8bf5aa7e9) )

	ROM_REGION( 0x20000, "mpegcpu", 0) // Z80 DSB program
	ROM_LOAD("epr-19275.2s", 0x000000,  0x20000, CRC(ee809d3f) SHA1(347080858fbfe9955002f382603a1b86a52d26d5) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD16_WORD_SWAP("epr-18643a.7", 0x000000,  0x20000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD16_WORD_SWAP("mpr-19259.32", 0x000000, 0x400000, CRC(4d55dbfc) SHA1(6e57e6e6e785b0f14bb5e221a44d518dbde7ad65) )
	ROM_LOAD16_WORD_SWAP("mpr-19261.34", 0x400000, 0x400000, CRC(b88878ff) SHA1(4bebcfba68b0cc2fa0bcacfaaf2d2e8af3625c5d) )

	ROM_REGION( 0x800000, "mpeg", 0 ) // MPEG audio data
	ROM_LOAD("mpr-19262.57s", 0x000000, 0x200000, CRC(922aed7a) SHA1(8d6872bdd46eaf2076c10d18c10af8ccbd3b10e8) )
	ROM_LOAD("mpr-19263.58s", 0x200000, 0x200000, CRC(a256f4cd) SHA1(a17b49050f1ecf1970477b12201cc3b58b31d89c) )
	ROM_LOAD("mpr-19264.59s", 0x400000, 0x200000, CRC(b6c51d0f) SHA1(9e0969a1e49ec1462f69cd0f0f9ce630d66174ce) )
	ROM_LOAD("mpr-19265.60s", 0x600000, 0x200000, CRC(7d98700a) SHA1(bedd37314ecab424b5b27030e1e7dc1b596303f3) )

	ROM_REGION( 0x10000, "drive", 0 ) // drive board CPU (code is Z80 compatible)
	ROM_LOAD( "epr-18261.ic9", 0x000000, 0x010000, CRC(0c7fac58) SHA1(68c1724c41401e28a5123022981c8919fd22656e) )
ROM_END

ROM_START( stcca ) /* Sega Touring Car Championship Revision A, Model 2C - Defaults to Japan, Twin & no "Default View" option - Sega Game ID# 833-12779, Sega ROM board ID# 834-12780 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19272a.15", 0x000000, 0x080000, CRC(20cedd05) SHA1(e465967c784de18caaaac77e164796e9779f576a) )
	ROM_LOAD32_WORD("epr-19273a.16", 0x000002, 0x080000, CRC(1b0ab4d6) SHA1(142bcd53fa6632fcc866bbda817aa83470111ef1) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19257.11",  0x000000, 0x400000, CRC(ac28ee24) SHA1(31d360dc435336942f70365d0491a2ccfc24c4c0) )
	ROM_LOAD32_WORD("mpr-19258.12",  0x000002, 0x400000, CRC(f5ba7d78) SHA1(9c8304a1f856d1ded869ed2b86de52129510f019) )
	ROM_LOAD32_WORD("epr-19270.9",   0x800000, 0x080000, CRC(7bd1d04e) SHA1(0490f3abc97af16e05f0dc9623e8fc635b1d4262) )
	ROM_LOAD32_WORD("epr-19271.10",  0x800002, 0x080000, CRC(d2d74f85) SHA1(49e7a1e6478122b4f0e679d7b336fb34044b503b) )
	ROM_COPY("user1", 0x800000, 0x900000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xa00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xb00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xc00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xd00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xe00000, 0x100000)
	ROM_COPY("user1", 0x800000, 0xf00000, 0x100000)

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-19255.29", 0x000000, 0x200000, CRC(d78bf030) SHA1(e6b3d8422613d22db50cf6c251f9a21356d96653) )
	ROM_LOAD32_WORD("mpr-19256.30", 0x000002, 0x200000, CRC(cb2b2d9e) SHA1(86b2b8bb6074352f72eb81e616093a1ba6f5163f) )

	ROM_REGION( 0x900000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19251.17", 0x000000, 0x400000, CRC(e06ff7ba) SHA1(6d472d03cd3caeb66be929c74ae63c32d305a3db) )
	ROM_LOAD32_WORD("mpr-19252.21", 0x000002, 0x400000, CRC(68509993) SHA1(654d5cdf44e7e1e788b26593f418ce76a5c1165a) )
	ROM_LOAD32_WORD("epr-19266.18", 0x800000, 0x080000, CRC(41464ee2) SHA1(afbbc0328bd36c34c69f0f54404dfd6a64036417) )
	ROM_LOAD32_WORD("epr-19267.22", 0x800002, 0x080000, CRC(780f994d) SHA1(f134482ed0fcfc7b3eea39947da47081301a111a) )

	ROM_REGION( 0x900000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19254.27", 0x000000, 0x200000, CRC(1ec49c02) SHA1(a9bdbab7b4b265c9118cf27fd45ca94f4516d5c6) )
	ROM_RELOAD     (                0x400000, 0x200000 )
	ROM_LOAD32_WORD("mpr-19253.25", 0x000002, 0x200000, CRC(41ba79fb) SHA1(f4d8a4f8278eec6d528bd947b91ebeb5223559d5) )
	ROM_RELOAD     (                0x400002, 0x200000 )
	ROM_LOAD32_WORD("epr-19269.28", 0x800000, 0x080000, CRC(01881121) SHA1(fe711709e70b3743b2a0318b823d859f233d3ff8) )
	ROM_LOAD32_WORD("epr-19268.26", 0x800002, 0x080000, CRC(bc4e081c) SHA1(b89d39ed19a146d1e94e52682f67d2cd23d8df7f) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19274.31", 0x080000,  0x20000, CRC(2dcc08ae) SHA1(bad26e2c994f2d4db5d9be0e34cf21a8bf5aa7e9) )

	ROM_REGION( 0x20000, "mpegcpu", 0) // Z80 DSB program
	ROM_LOAD("epr-19275.2s", 0x000000,  0x20000, CRC(ee809d3f) SHA1(347080858fbfe9955002f382603a1b86a52d26d5) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD16_WORD_SWAP("epr-18643a.7", 0x000000,  0x20000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD16_WORD_SWAP("mpr-19259.32", 0x000000, 0x400000, CRC(4d55dbfc) SHA1(6e57e6e6e785b0f14bb5e221a44d518dbde7ad65) )
	ROM_LOAD16_WORD_SWAP("mpr-19261.34", 0x400000, 0x400000, CRC(b88878ff) SHA1(4bebcfba68b0cc2fa0bcacfaaf2d2e8af3625c5d) )

	ROM_REGION( 0x800000, "mpeg", 0 ) // MPEG audio data
	ROM_LOAD("mpr-19262.57s", 0x000000, 0x200000, CRC(922aed7a) SHA1(8d6872bdd46eaf2076c10d18c10af8ccbd3b10e8) )
	ROM_LOAD("mpr-19263.58s", 0x200000, 0x200000, CRC(a256f4cd) SHA1(a17b49050f1ecf1970477b12201cc3b58b31d89c) )
	ROM_LOAD("mpr-19264.59s", 0x400000, 0x200000, CRC(b6c51d0f) SHA1(9e0969a1e49ec1462f69cd0f0f9ce630d66174ce) )
	ROM_LOAD("mpr-19265.60s", 0x600000, 0x200000, CRC(7d98700a) SHA1(bedd37314ecab424b5b27030e1e7dc1b596303f3) )

	ROM_REGION( 0x10000, "drive", 0 ) // drive board CPU (code is Z80 compatible)
	ROM_LOAD( "epr-18261.ic9", 0x000000, 0x010000, CRC(0c7fac58) SHA1(68c1724c41401e28a5123022981c8919fd22656e) )
ROM_END

ROM_START( skisuprg ) /* Sega Ski Super G, Model 2C, Sega Game ID# 833-12861, ROM board ID# 834-12862 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-19489.15", 0x000000, 0x080000, CRC(1df948a7) SHA1(a38faeb97c65b379ad05f7311b55217118c8d2be) )
	ROM_LOAD32_WORD( "epr-19490.16", 0x000002, 0x080000, CRC(e6fc24d3) SHA1(1ac9172cf0b4d6a3488483ffa490a4ca5d410927) )
	ROM_LOAD32_WORD( "epr-19551.13", 0x100000, 0x080000, CRC(3ee8f0d5) SHA1(23f45858559776a70b3b57f4cb2840f44e6a6531) )
	ROM_LOAD32_WORD( "epr-19552.14", 0x100002, 0x080000, CRC(baa2e49a) SHA1(b234f3b65e8fabfb6ec7ca62dd9a1d2935b2e95a) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-19494.11", 0x000000, 0x400000, CRC(f19cdb5c) SHA1(bdbb7d9e91a7742ff5a908b6244adbed291e5e7f) )
	ROM_LOAD32_WORD( "mpr-19495.12", 0x000002, 0x400000, CRC(d42e5ef2) SHA1(21ca5e7e543595a4691aacdbcdd2af21d464d939) )
	ROM_LOAD32_WORD( "mpr-19492.9",  0x800000, 0x400000, CRC(4805318f) SHA1(dbd1359817933313c6d74d3a1450682e8ce5857a) )
	ROM_LOAD32_WORD( "mpr-19493.10", 0x800002, 0x400000, CRC(39daa909) SHA1(e29e50c7fc39bd4945f993ceaa100358054efc5a) )

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD( "mpr-19502.29", 0x000000, 0x400000, CRC(2212d8d6) SHA1(3b8a4da2dc00a1eac41b48cbdc322ea1c31b8b29) )
	ROM_LOAD32_WORD( "mpr-19503.30", 0x000002, 0x400000, CRC(3c9cfc73) SHA1(2213485a00cef0bcef11b67f00027c4159c5e2f5) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-19496.17", 0x000000, 0x400000, CRC(0e9aef4e) SHA1(d4b511b90c0a6e27d6097cb25ff005f68d5fa83c) )
	ROM_LOAD32_WORD( "mpr-19497.21", 0x000002, 0x400000, CRC(5397efe9) SHA1(4b20bab36462f9506fa2601c2545051ca49de7f5) )
	ROM_LOAD32_WORD( "mpr-19498.18", 0x800000, 0x400000, CRC(32e5ae60) SHA1(b8a1cc117875c3919a78eedb60a06926288d9b95) )
	ROM_LOAD32_WORD( "mpr-19499.22", 0x800002, 0x400000, CRC(2b9f5b48) SHA1(40f3f2844244c3f1c8792aa262872243ad20fd69) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-19501.27", 0x000000, 0x400000, CRC(66d7b02e) SHA1(cede0dc5c8d9fbfa8de01fe864b3cc101abf67d7) )
	ROM_LOAD32_WORD( "mpr-19500.25", 0x000002, 0x400000, CRC(905f5798) SHA1(31f104e3022b5bc7ed7c667eb801a57949a06c93) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-19491.31", 0x080000, 0x080000, CRC(1c9b15fd) SHA1(045244a4eebc45f149aecf47f090cede1813477b) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-19504.32", 0x000000, 0x400000, CRC(9419ec08) SHA1(d05d9ceb7fd09fa8991c0df4d1c57eb621460e30) )
	ROM_LOAD( "mpr-19505.34", 0x400000, 0x400000, CRC(eba7f41d) SHA1(f6e521bedf298808a768f6fdcb0b60b320a66d04) )
ROM_END

/* Sega Water Ski - There should be a version with program roms EPR-19965 & EPR-19966 (currently undumped) */
ROM_START( segawski ) /* Sega Water Ski Revision A, Model 2C, Sega Game ID# 833-13204, ROM board ID# 834-13205 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19963a.15", 0x000000, 0x080000, CRC(89c9cb0d) SHA1(7f1f600222447effb28cf2d56193ea9f45fd0646) )
	ROM_LOAD32_WORD("epr-19964a.16", 0x000002, 0x080000, CRC(c382cefe) SHA1(c0ccee4eb19d9626dee0f77f08060f1d9708b39d) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19980.11", 0x0000000, 0x400000, CRC(709804f2) SHA1(6633eeb2bc0ddd5eb5994cb7cc39ed4ab2094a7e) )
	ROM_LOAD32_WORD("mpr-19981.12", 0x0000002, 0x400000, CRC(a1e8ec09) SHA1(53abaf71e85f874d28a79c8775b1f0ac919fbe22) )
	ROM_LOAD32_WORD("mpr-19982.9",  0x0800000, 0x400000, CRC(41c398bb) SHA1(e6652111a494165c93d5e9dc7d1c3df46f1a8961) )
	ROM_LOAD32_WORD("mpr-19983.10", 0x0800002, 0x400000, CRC(e210dea2) SHA1(a32010648046738f64cbc7e1a074b9443bbde865) )
	ROM_LOAD32_WORD("mpr-19984.7",  0x1000000, 0x400000, CRC(14b967d1) SHA1(8e0fd13f5838218224ce162a55558b75dcda4cbf) )
	ROM_LOAD32_WORD("mpr-19985.8",  0x1000002, 0x400000, CRC(57827677) SHA1(0d764ec9e136e4d027aefb10e9f025aa2e081573) )
	ROM_LOAD32_WORD("epr-19961.5",  0x1800000, 0x080000, CRC(cc34ecaf) SHA1(8e540a429826d40acef5a34a0b6b1b89e059961b) )
	ROM_LOAD32_WORD("epr-19962.6",  0x1800002, 0x080000, CRC(e8a30e5e) SHA1(2d7290186aaca28f3aaf6656b090e1cbe509aa48) )

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-19986.29", 0x0000000, 0x400000, CRC(4b8e26f8) SHA1(859e3788c75599295a8b57ed7852f2cbb6a2a738) )
	ROM_LOAD32_WORD("mpr-19987.30", 0x0000002, 0x400000, CRC(8d5b9d38) SHA1(35f41c474af3754152aecefe81e912120823e0ff) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19972.17", 0x0000000, 0x400000, CRC(493c0f8f) SHA1(c68f662d53fbcec3d2dbd9ebdfd37cbf1ff46408) )
	ROM_LOAD32_WORD("mpr-19973.21", 0x0000002, 0x400000, CRC(01b6f8e1) SHA1(be1f1017ff5f43218de2d99090e4e7db64f7a483) )
	ROM_LOAD32_WORD("mpr-19974.18", 0x0800000, 0x400000, CRC(2bd947d3) SHA1(7f16e668db5a4673cd909bdfa1b9dc665827b392) )
	ROM_LOAD32_WORD("mpr-19975.22", 0x0800002, 0x400000, CRC(07e6b699) SHA1(35bbea4cd42ab39f976cd2ec964c32d5d3982c4a) )
	ROM_LOAD32_WORD("mpr-19976.19", 0x1000000, 0x400000, CRC(f9496566) SHA1(e6dee764301c0ed34e5134a5232898bb42563c20) )
	ROM_LOAD32_WORD("mpr-19977.23", 0x1000002, 0x400000, CRC(b3dbf54b) SHA1(d61448394b7b2036e27bdbf7a062d63f076db9da) )
	ROM_LOAD32_WORD("mpr-19978.20", 0x1800000, 0x400000, CRC(c80f4ed7) SHA1(e7ea6dfb57ab1fe924a80c244b8acfe4aad2b76d) )
	ROM_LOAD32_WORD("mpr-19979.24", 0x1800002, 0x400000, CRC(1c0db4d2) SHA1(65a81f4503c6b9c26650befd183db821545c53e2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19969.27", 0x0000000, 0x400000, CRC(c0c33d39) SHA1(f61a6266556d5687cd03031065baa2dd73112666) )
	ROM_LOAD32_WORD("mpr-19968.25", 0x0000002, 0x400000, CRC(b8d2f04f) SHA1(f0f4d2dd06cdf745ed07fe428eaa3e3ad030bff6) )
	ROM_LOAD32_WORD("mpr-19971.28", 0x0800000, 0x400000, CRC(c8708096) SHA1(c27e0a90dc1183b0cf7f32e324afa6c126f61d37) )
	ROM_LOAD32_WORD("mpr-19970.26", 0x0800002, 0x400000, CRC(c59d8d36) SHA1(24232390f0cac5ffbb17a0093a602363c686fbf8) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19967.31", 0x080000, 0x080000, CRC(c6b8ef3f) SHA1(9f86d6e365a5535d354ff6b0614f3a19c0790d0f) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19988.32", 0x000000, 0x400000, CRC(303732fb) SHA1(63efbd9f67b38fddeeed25de660514867e03486a) )
	ROM_LOAD("mpr-19989.34", 0x400000, 0x400000, CRC(8074a4b3) SHA1(98dc1d122ffb9b5c52994dea2b5d8c4f004a5f8e) )
ROM_END

ROM_START( hotd ) /* House of the Dead, Model 2C, Sega Game ID# 610-0396-13054 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19696.15", 0x000000, 0x080000, CRC(03da5623) SHA1(be0bd34a9216375c7204445f084f6c74c4d3b0c8) )
	ROM_LOAD32_WORD("epr-19697.16", 0x000002, 0x080000, CRC(a9722d87) SHA1(0b14f9a81272f79a5b294bc024711042c5fb2637) )
	ROM_LOAD32_WORD("epr-19694.13", 0x100000, 0x080000, CRC(e85ca1a3) SHA1(3d688be98f78fe40c2af1e91df6decd500400ae9) )
	ROM_LOAD32_WORD("epr-19695.14", 0x100002, 0x080000, CRC(cd52b461) SHA1(bc96ab2a4ba7f30c0b89814acc8931c8bf800a82) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19704.11",    0x0000000, 0x400000, CRC(aa80dbb0) SHA1(24e63f4392847f288971469cd10448536eb435d4) )
	ROM_LOAD32_WORD("mpr-19705.12",    0x0000002, 0x400000, CRC(f906843b) SHA1(bee4f43b3ad15d93a2f9f07b873c9cf5d228e2f9) )
	ROM_LOAD32_WORD("mpr-19702.9",     0x0800000, 0x400000, CRC(fc8aa3b7) SHA1(b64afb17d9c97277d8c4f20811f14f65a61cbb56) )
	ROM_LOAD32_WORD("mpr-19703.10",    0x0800002, 0x400000, CRC(208d993d) SHA1(e5c45ea5621f99661a87ffe88e24764d2bbcb51e) )
	ROM_LOAD32_WORD("mpr-19700.7",     0x1000000, 0x400000, CRC(0558cfd3) SHA1(94440839d3325176c2d03f39a78949d0ef040bba) )
	ROM_LOAD32_WORD("mpr-19701.8",     0x1000002, 0x400000, CRC(224a8929) SHA1(933770546d46abca400e7f524eff2ae89241e56d) )
	ROM_LOAD32_WORD("epr-19698.5",     0x1800000, 0x080000, CRC(e7a7b6ea) SHA1(77cb53f8730fdb55080b70910ab8c750d79acb02) )
	ROM_LOAD32_WORD("epr-19699.6",     0x1800002, 0x080000, CRC(8160b3d9) SHA1(9dab483c60624dddba8085e94a4325739592ec17) )
	ROM_COPY( "user1", 0x1800000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1f00000, 0x100000 )


	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("epr-19707.29",  0x000000, 0x080000, CRC(384fd133) SHA1(6d060378d0f801b04d12e7ee874f2fa0572992d9) )
	ROM_LOAD32_WORD("epr-19706.30",  0x000002, 0x080000, CRC(1277531c) SHA1(08d3e733ba9989fcd32290634171c73f26ab6e2b) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19715.17", 0x0000000, 0x400000, CRC(3ff7dda7) SHA1(0a61b091bb0bc659f0cbca8ad401d0925a1dc2ea) )
	ROM_LOAD32_WORD("mpr-19711.21", 0x0000002, 0x400000, CRC(080d13f1) SHA1(4167428a2a903aea2c14631ccf924afb81338b89) )
	ROM_LOAD32_WORD("mpr-19714.18", 0x0800000, 0x400000, CRC(3e55ab49) SHA1(70b4c1627db80e6734112c02265495e2b4a53278) )
	ROM_LOAD32_WORD("mpr-19710.22", 0x0800002, 0x400000, CRC(80df1036) SHA1(3cc59bb4910aa5382e95762f63325c06b763bd23) )
	ROM_LOAD32_WORD("mpr-19713.19", 0x1000000, 0x400000, CRC(4d092cd3) SHA1(b6d0be283c25235249186751c7f025a7c38d2f36) )
	ROM_LOAD32_WORD("mpr-19709.23", 0x1000002, 0x400000, CRC(d08937bf) SHA1(c92571e35960f27dc8b0b059f12167026d0666d1) )
	ROM_LOAD32_WORD("mpr-19712.20", 0x1800000, 0x400000, CRC(41577943) SHA1(25a0d921c8662043c5860dc7a226d4895ff9fff6) )
	ROM_LOAD32_WORD("mpr-19708.24", 0x1800002, 0x400000, CRC(5cb790f2) SHA1(d3cae450186bc62fd746b14d6a05cb397efcfe40) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19718.27", 0x0000000, 0x400000, CRC(a9de5924) SHA1(3ebac2aeb1467939337c9a5c87ad9c293560dae2) )
	ROM_LOAD32_WORD("mpr-19716.25", 0x0000002, 0x400000, CRC(45c7dcce) SHA1(f602cabd879c69afee544848feafb9fb9f5d51e2) )
	ROM_LOAD32_WORD("mpr-19719.28", 0x0800000, 0x400000, CRC(838f8343) SHA1(fe6622b5917f9a99c097fd60d9446ac6b481fa75) )
	ROM_LOAD32_WORD("mpr-19717.26", 0x0800002, 0x400000, CRC(393e440b) SHA1(927ac9cad22f87b339cc86043678470ff139ce1f) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19720.31", 0x080000,  0x80000, CRC(b367d21d) SHA1(1edaed489a3518ddad85728e416319f940ea02bb) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19721.32", 0x000000, 0x400000, CRC(f5d8fa9a) SHA1(6836973a687c59dd80f8e6c30d33155e306be199) )
	ROM_LOAD("mpr-19722.34", 0x400000, 0x400000, CRC(a56fa539) SHA1(405a892bc368ba862ba71bda7525b421d6973c0e) )
ROM_END

ROM_START( lastbrnx ) /* Last Bronx Revision A (Export), Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19061a.15", 0x000000, 0x080000, CRC(c0aebab2) SHA1(fa63081b0aa6f02c3d197485865ee38e9c78b43d) )
	ROM_LOAD32_WORD("epr-19062a.16", 0x000002, 0x080000, CRC(cdf597e8) SHA1(a85ca36a537ba21d11ef3cfdf914c2c93ac5e68f) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19050.11",  0x000000, 0x400000, CRC(e6af2b61) SHA1(abdf7aa4c594f0916d4335c70fdd67dc6b1f4630) )
	ROM_LOAD32_WORD("mpr-19051.12",  0x000002, 0x400000, CRC(14b88961) SHA1(bec22f657c6d939c095b99ca9c6eb44b9683fd72) )
	ROM_LOAD32_WORD("mpr-19048.9",   0x800000, 0x400000, CRC(02180215) SHA1(cc5f8e61fee07aa4fc5bfe2d011088ee523c77c2) )
	ROM_LOAD32_WORD("mpr-19049.10",  0x800002, 0x400000, CRC(db7eecd6) SHA1(5955885ad2bfd69d7a2c4e1d1df907aca41fbdd0) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19052.17",  0x000000, 0x400000, CRC(d7f27216) SHA1(b393af96522306dc2e055aea1e837979f41940d4) )
	ROM_LOAD32_WORD("mpr-19053.21",  0x000002, 0x400000, CRC(1f328465) SHA1(950a92209b7c24f66db62c31627a1f1d52721f1e) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19055.27",  0x000000, 0x200000, CRC(85a57d49) SHA1(99c49fe135dc46fa861337b5bac654ae8478778a) )
	ROM_LOAD32_WORD("mpr-19054.25",  0x000002, 0x200000, CRC(05366277) SHA1(f618e2b9b26a1f7eccebfc8f8e17ef8ad9029be8) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("mpr-19056.31", 0x080000,  0x80000, CRC(22a22918) SHA1(baa039cd86650b6cd81f295916c4d256e60cb29c) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19057.32", 0x0000000, 0x400000, CRC(64809438) SHA1(aa008f83e1eff0daafe01944248ebae6054cee9f) )
	ROM_LOAD("mpr-19058.34", 0x0400000, 0x400000, CRC(e237c11c) SHA1(7c89cba757bd58747ed0d633b2fe7ef559fcd15e) )
ROM_END

ROM_START( lastbrnxu ) /* Last Bronx Revision A (USA), Model 2B - Sega ROM board ID# 834-12360 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19059a.15", 0x000000, 0x080000, CRC(25478257) SHA1(c6b7a5788617faff6cf612a824b29a9474db87f3) )
	ROM_LOAD32_WORD("epr-19060a.16", 0x000002, 0x080000, CRC(c48906b2) SHA1(a0904c97234f218caf489dc55e33082e453791a0) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19050.11",  0x000000, 0x400000, CRC(e6af2b61) SHA1(abdf7aa4c594f0916d4335c70fdd67dc6b1f4630) )
	ROM_LOAD32_WORD("mpr-19051.12",  0x000002, 0x400000, CRC(14b88961) SHA1(bec22f657c6d939c095b99ca9c6eb44b9683fd72) )
	ROM_LOAD32_WORD("mpr-19048.9",   0x800000, 0x400000, CRC(02180215) SHA1(cc5f8e61fee07aa4fc5bfe2d011088ee523c77c2) )
	ROM_LOAD32_WORD("mpr-19049.10",  0x800002, 0x400000, CRC(db7eecd6) SHA1(5955885ad2bfd69d7a2c4e1d1df907aca41fbdd0) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19052.17",  0x000000, 0x400000, CRC(d7f27216) SHA1(b393af96522306dc2e055aea1e837979f41940d4) )
	ROM_LOAD32_WORD("mpr-19053.21",  0x000002, 0x400000, CRC(1f328465) SHA1(950a92209b7c24f66db62c31627a1f1d52721f1e) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19055.27",  0x000000, 0x200000, CRC(85a57d49) SHA1(99c49fe135dc46fa861337b5bac654ae8478778a) )
	ROM_LOAD32_WORD("mpr-19054.25",  0x000002, 0x200000, CRC(05366277) SHA1(f618e2b9b26a1f7eccebfc8f8e17ef8ad9029be8) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("mpr-19056.31", 0x080000,  0x80000, CRC(22a22918) SHA1(baa039cd86650b6cd81f295916c4d256e60cb29c) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19057.32", 0x0000000, 0x400000, CRC(64809438) SHA1(aa008f83e1eff0daafe01944248ebae6054cee9f) )
	ROM_LOAD("mpr-19058.34", 0x0400000, 0x400000, CRC(e237c11c) SHA1(7c89cba757bd58747ed0d633b2fe7ef559fcd15e) )
ROM_END

ROM_START( lastbrnxj ) /* Last Bronx Revision A (Japan), Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19046a.15", 0x000000, 0x080000, CRC(75be7b7a) SHA1(e57320ac3abac54b7b5278596979746ed1856188) )
	ROM_LOAD32_WORD("epr-19047a.16", 0x000002, 0x080000, CRC(1f5541e2) SHA1(87214f285a7bf67fbd824f2190cb9b2daf408193) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19050.11",  0x000000, 0x400000, CRC(e6af2b61) SHA1(abdf7aa4c594f0916d4335c70fdd67dc6b1f4630) )
	ROM_LOAD32_WORD("mpr-19051.12",  0x000002, 0x400000, CRC(14b88961) SHA1(bec22f657c6d939c095b99ca9c6eb44b9683fd72) )
	ROM_LOAD32_WORD("mpr-19048.9",   0x800000, 0x400000, CRC(02180215) SHA1(cc5f8e61fee07aa4fc5bfe2d011088ee523c77c2) )
	ROM_LOAD32_WORD("mpr-19049.10",  0x800002, 0x400000, CRC(db7eecd6) SHA1(5955885ad2bfd69d7a2c4e1d1df907aca41fbdd0) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19052.17",  0x000000, 0x400000, CRC(d7f27216) SHA1(b393af96522306dc2e055aea1e837979f41940d4) )
	ROM_LOAD32_WORD("mpr-19053.21",  0x000002, 0x400000, CRC(1f328465) SHA1(950a92209b7c24f66db62c31627a1f1d52721f1e) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19055.27",  0x000000, 0x200000, CRC(85a57d49) SHA1(99c49fe135dc46fa861337b5bac654ae8478778a) )
	ROM_LOAD32_WORD("mpr-19054.25",  0x000002, 0x200000, CRC(05366277) SHA1(f618e2b9b26a1f7eccebfc8f8e17ef8ad9029be8) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("mpr-19056.31", 0x080000,  0x80000, CRC(22a22918) SHA1(baa039cd86650b6cd81f295916c4d256e60cb29c) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19057.32", 0x0000000, 0x400000, CRC(64809438) SHA1(aa008f83e1eff0daafe01944248ebae6054cee9f) )
	ROM_LOAD("mpr-19058.34", 0x0400000, 0x400000, CRC(e237c11c) SHA1(7c89cba757bd58747ed0d633b2fe7ef559fcd15e) )
ROM_END

ROM_START( pltkidsa ) /* Pilot Kids, Model 2A */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-21281.pr0", 0x000000, 0x080000, CRC(293ead5d) SHA1(5a6295e543d7e68387de0ca4d88e930a0d8ed25c) )
	ROM_LOAD32_WORD("epr-21282.pr1", 0x000002, 0x080000, CRC(ed0e7b9e) SHA1(15f3fab6ac2dd40f32bda55503378ab14f998707) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-21262.da0", 0x000000, 0x400000, CRC(aa71353e) SHA1(6eb5e8284734f01beec1dbbee049b6b7672e2504) )
	ROM_LOAD32_WORD("mpr-21263.da1", 0x000002, 0x400000, CRC(d55d4509) SHA1(641db6ec3e9266f8265a4b541bcd8c2f7d164cc3) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-21264.tp0", 0x0000000, 0x400000, CRC(6b35204d) SHA1(3a07701b140eb3088fad29c8b2d9c1e1e7ef9471) )
	ROM_LOAD32_WORD("mpr-21268.tp1", 0x0000002, 0x400000, CRC(16ce2147) SHA1(39cba6b4f1130a3da7e2d226c948425eec34090e) )
	ROM_LOAD32_WORD("mpr-21265.tp2", 0x0800000, 0x400000, CRC(f061e639) SHA1(a89b7a84192fcc1e9e0fe9adf7446f7b275d5a03) )
	ROM_LOAD32_WORD("mpr-21269.tp3", 0x0800002, 0x400000, CRC(8c06255e) SHA1(9a8c302528e590be1b56ed301da30abf21f0be2e) )
	ROM_LOAD32_WORD("mpr-21266.tp4", 0x1000000, 0x400000, CRC(f9c32021) SHA1(b21f8bf281bf2cfcdc7e5eb798cd633e905ab8b8) )
	ROM_LOAD32_WORD("mpr-21270.tp5", 0x1000002, 0x400000, CRC(b61f81c3) SHA1(7733f44e791974070df139958eb97e0585ee50f8) )
	ROM_LOAD32_WORD("mpr-21267.tp6", 0x1800000, 0x400000, CRC(c42cc938) SHA1(6153f52add63295122e1215dd07d648d030a7306) )
	ROM_LOAD32_WORD("mpr-21271.tp7", 0x1800002, 0x400000, CRC(a5325c75) SHA1(d52836760475c7d9fbb4e5b8147ac416ffd1fcd9) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-21274.tx1", 0x0000000, 0x400000, CRC(f045e3d1) SHA1(548909d2da22ed98594e0ab6ecffebec4fca2f93) )
	ROM_LOAD32_WORD("mpr-21272.tx0", 0x0000002, 0x400000, CRC(dd605c21) SHA1(8363a082a666ceeb84df84929ff3fbaff49af821) )
	ROM_LOAD32_WORD("mpr-21275.tx3", 0x0800000, 0x400000, CRC(c4870b7c) SHA1(feb8a34acb620a36ed5aea92d22622a76d7e1b29) )
	ROM_LOAD32_WORD("mpr-21273.tx2", 0x0800002, 0x400000, CRC(722ec8a2) SHA1(1a1dc92488cde6284a96acce80e47a9cceccde76) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-21276.sd0", 0x080000, 0x080000, CRC(8f415bc3) SHA1(4e8e1ccbe025deca42fcf2582f3da46fa34780b7) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-21277.sd1", 0x0000000, 0x200000, CRC(bfba0ff6) SHA1(11081b3eabc33a42ecfc0b2b535ce16510496144) )
	ROM_LOAD("mpr-21278.sd2", 0x0200000, 0x200000, CRC(27e18e08) SHA1(254c0ad4d6bd572ff0efc3ea80489e73716a31a7) )
	ROM_LOAD("mpr-21279.sd3", 0x0400000, 0x200000, CRC(3a8dcf68) SHA1(312496b45b699051c8b4dd0e5d94e73fe5f3ad8d) )
	ROM_LOAD("mpr-21280.sd4", 0x0600000, 0x200000, CRC(aa548124) SHA1(a94adfe16b5c3236746451c181ccd3e1c27432f4) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD

	//             1998     317-5044-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "042e2dc1" )
ROM_END

ROM_START( pltkids ) /* Pilot Kids Revision A, Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-21285a.15",  0x000000, 0x080000, CRC(bdde5b41) SHA1(14c3f5031f85c6756c00bc67765a967ebaf7eb7f) )
	ROM_LOAD32_WORD("epr-21286a.16",  0x000002, 0x080000, CRC(c8092e0e) SHA1(01030621efa9c97eb43f4a5e3e029ec99a2363c5) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-21262.da0", 0x000000, 0x400000, CRC(aa71353e) SHA1(6eb5e8284734f01beec1dbbee049b6b7672e2504) )
	ROM_LOAD32_WORD("mpr-21263.da1", 0x000002, 0x400000, CRC(d55d4509) SHA1(641db6ec3e9266f8265a4b541bcd8c2f7d164cc3) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-21264.tp0", 0x0000000, 0x400000, CRC(6b35204d) SHA1(3a07701b140eb3088fad29c8b2d9c1e1e7ef9471) )
	ROM_LOAD32_WORD("mpr-21268.tp1", 0x0000002, 0x400000, CRC(16ce2147) SHA1(39cba6b4f1130a3da7e2d226c948425eec34090e) )
	ROM_LOAD32_WORD("mpr-21265.tp2", 0x0800000, 0x400000, CRC(f061e639) SHA1(a89b7a84192fcc1e9e0fe9adf7446f7b275d5a03) )
	ROM_LOAD32_WORD("mpr-21269.tp3", 0x0800002, 0x400000, CRC(8c06255e) SHA1(9a8c302528e590be1b56ed301da30abf21f0be2e) )
	ROM_LOAD32_WORD("mpr-21266.tp4", 0x1000000, 0x400000, CRC(f9c32021) SHA1(b21f8bf281bf2cfcdc7e5eb798cd633e905ab8b8) )
	ROM_LOAD32_WORD("mpr-21270.tp5", 0x1000002, 0x400000, CRC(b61f81c3) SHA1(7733f44e791974070df139958eb97e0585ee50f8) )
	ROM_LOAD32_WORD("mpr-21267.tp6", 0x1800000, 0x400000, CRC(c42cc938) SHA1(6153f52add63295122e1215dd07d648d030a7306) )
	ROM_LOAD32_WORD("mpr-21271.tp7", 0x1800002, 0x400000, CRC(a5325c75) SHA1(d52836760475c7d9fbb4e5b8147ac416ffd1fcd9) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-21274.tx1", 0x0000000, 0x400000, CRC(f045e3d1) SHA1(548909d2da22ed98594e0ab6ecffebec4fca2f93) )
	ROM_LOAD32_WORD("mpr-21272.tx0", 0x0000002, 0x400000, CRC(dd605c21) SHA1(8363a082a666ceeb84df84929ff3fbaff49af821) )
	ROM_LOAD32_WORD("mpr-21275.tx3", 0x0800000, 0x400000, CRC(c4870b7c) SHA1(feb8a34acb620a36ed5aea92d22622a76d7e1b29) )
	ROM_LOAD32_WORD("mpr-21273.tx2", 0x0800002, 0x400000, CRC(722ec8a2) SHA1(1a1dc92488cde6284a96acce80e47a9cceccde76) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-21276.sd0", 0x080000, 0x080000, CRC(8f415bc3) SHA1(4e8e1ccbe025deca42fcf2582f3da46fa34780b7) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-21277.sd1", 0x0000000, 0x200000, CRC(bfba0ff6) SHA1(11081b3eabc33a42ecfc0b2b535ce16510496144) )
	ROM_LOAD("mpr-21278.sd2", 0x0200000, 0x200000, CRC(27e18e08) SHA1(254c0ad4d6bd572ff0efc3ea80489e73716a31a7) )
	ROM_LOAD("mpr-21279.sd3", 0x0400000, 0x200000, CRC(3a8dcf68) SHA1(312496b45b699051c8b4dd0e5d94e73fe5f3ad8d) )
	ROM_LOAD("mpr-21280.sd4", 0x0600000, 0x200000, CRC(aa548124) SHA1(a94adfe16b5c3236746451c181ccd3e1c27432f4) )

	//             1998     317-5044-COM   Model 2
	ROM_PARAMETER( ":315_5881:key", "042e2dc1" )
ROM_END

ROM_START( indy500 ) /* Defaults to Twin (Stand Alone) Cab version.  2 credits to start - Can be set to Deluxe setting in service mode, Sega Game ID# 833-12361, ROM board ID# 834-12362 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18598a.15", 0x000000, 0x080000, CRC(3cdcac0f) SHA1(2f616e363f4d246fece309e81325e5e3c4e9d9f8) ) /* Higher rom numbers indicate a newer version */
	ROM_LOAD32_WORD("epr-18599a.16", 0x000002, 0x080000, CRC(32bde9a2) SHA1(0982952ab3c5b035f37beb9304ac950c0e78aea8) ) /* Different attract mode... what else??? */

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18237.11",    0x0000000, 0x400000, CRC(37e4255a) SHA1(3ee69a5b9364048dfab242773d97f3af430845b7) )
	ROM_LOAD32_WORD("mpr-18238.12",    0x0000002, 0x400000, CRC(bf837bac) SHA1(6624417b65f15f20427bc42c27283f10342c76b5) )
	ROM_LOAD32_WORD("mpr-18239.9",     0x0800000, 0x400000, CRC(9a2db86e) SHA1(0b81f6037657af7d96ed5e9bfef407d87cbcc294) )
	ROM_LOAD32_WORD("mpr-18240.10",    0x0800002, 0x400000, CRC(ab46a35f) SHA1(67da857db7155a858a1fa575b6c50f4be3c9ab7c) )
	ROM_LOAD32_WORD("epr-18596.7",     0x1000000, 0x080000, CRC(8be1a5cd) SHA1(56ed21234c6494d95b4efda6c3374199c5ac65db) )
	ROM_LOAD32_WORD("epr-18597.8",     0x1000002, 0x080000, CRC(44824e38) SHA1(f0fb0d73c1e72ce77c3931d436c54c034d2107a8) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("epr-18249.29", 0x000000, 0x080000, CRC(a399f023) SHA1(8b453313c16d935701ed7dbf71c1607c40aede63) )
	ROM_LOAD32_WORD("epr-18250.30", 0x000002, 0x080000, CRC(7479ad52) SHA1(d453e25709cd5970cd21bdc8b4785bc8eb5a50d7) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18233.17", 0x000000, 0x400000, CRC(48a024d3) SHA1(501c6ab969713187025331942f922cb0e8efa69a) )
	ROM_LOAD32_WORD("mpr-18234.21", 0x000002, 0x400000, CRC(1178bfc8) SHA1(4a9982fdce08f9d375371763dd5287e8485c24b1) )
	ROM_LOAD32_WORD("mpr-18235.18", 0x800000, 0x400000, CRC(e7d70d59) SHA1(6081739c15a634d5cc7680a4fc7decead93540ed) )
	ROM_LOAD32_WORD("mpr-18236.22", 0x800002, 0x400000, CRC(6ca29e0e) SHA1(5de8b569d2a91047836f4a251c21db82fd7841c9) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18232.27", 0x000000, 0x400000, CRC(f962347d) SHA1(79f07ee6b821724294ca9e7a079cb33249102508) )
	ROM_LOAD32_WORD("mpr-18231.25", 0x000002, 0x400000, CRC(673d5338) SHA1(ce592857496ccc0a51efb377cf7cccc000b4296b) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18600.31", 0x080000,  0x40000, CRC(269ee4a0) SHA1(8ebabfcd27d634ff0bead0a1f138efc8708575e7) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18241.32", 0x0000000, 0x200000, CRC(3a380ae1) SHA1(114113325e9e5262af8750c05089f24818943cde) )
	ROM_LOAD("mpr-18242.33", 0x0200000, 0x200000, CRC(1cc3deae) SHA1(5c9cb8ce43a909b25b4e734c6a4ffd786f4dde31) )
	ROM_LOAD("mpr-18243.34", 0x0400000, 0x200000, CRC(a00a0053) SHA1(9c24fbcd0318c7e195dd153d6ba05e8c1e052968) )
	ROM_LOAD("mpr-18244.35", 0x0600000, 0x200000, CRC(bfa75beb) SHA1(fec89260d887e90ee9c2803e2eaf937cf9bfa10b) )
ROM_END

ROM_START( indy500d ) /* Defaults to Deluxe (Stand Alone) Cab version. 3 credits to start - Can be set to Twin setting in service mode, Sega Game ID# 833-11992, ROM board ID# 834-11993 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18251a.15", 0x000000, 0x080000, CRC(fdabb40b) SHA1(e60a4814b54b76c7c0a4d9cf2b093c577c2f6ecf) )
	ROM_LOAD32_WORD("epr-18252a.16", 0x000002, 0x080000, CRC(4935832a) SHA1(8fc9244fd0eaf93d016f4494604e5a70bf1f7303) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18237.11",    0x0000000, 0x400000, CRC(37e4255a) SHA1(3ee69a5b9364048dfab242773d97f3af430845b7) )
	ROM_LOAD32_WORD("mpr-18238.12",    0x0000002, 0x400000, CRC(bf837bac) SHA1(6624417b65f15f20427bc42c27283f10342c76b5) )
	ROM_LOAD32_WORD("mpr-18239.9",     0x0800000, 0x400000, CRC(9a2db86e) SHA1(0b81f6037657af7d96ed5e9bfef407d87cbcc294) )
	ROM_LOAD32_WORD("mpr-18240.10",    0x0800002, 0x400000, CRC(ab46a35f) SHA1(67da857db7155a858a1fa575b6c50f4be3c9ab7c) )
	ROM_LOAD32_WORD("epr-18245.7",     0x1000000, 0x080000, CRC(854b1037) SHA1(6bbbae53e2f56ab1007f37fdd5eb66dda4828c28) )
	ROM_LOAD32_WORD("epr-18246.8",     0x1000002, 0x080000, CRC(1a68acdc) SHA1(425ca92d75054a17313732a9d11bbb31bea17a38) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("epr-18249.29", 0x000000, 0x080000, CRC(a399f023) SHA1(8b453313c16d935701ed7dbf71c1607c40aede63) )
	ROM_LOAD32_WORD("epr-18250.30", 0x000002, 0x080000, CRC(7479ad52) SHA1(d453e25709cd5970cd21bdc8b4785bc8eb5a50d7) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18233.17", 0x000000, 0x400000, CRC(48a024d3) SHA1(501c6ab969713187025331942f922cb0e8efa69a) )
	ROM_LOAD32_WORD("mpr-18234.21", 0x000002, 0x400000, CRC(1178bfc8) SHA1(4a9982fdce08f9d375371763dd5287e8485c24b1) )
	ROM_LOAD32_WORD("mpr-18235.18", 0x800000, 0x400000, CRC(e7d70d59) SHA1(6081739c15a634d5cc7680a4fc7decead93540ed) )
	ROM_LOAD32_WORD("mpr-18236.22", 0x800002, 0x400000, CRC(6ca29e0e) SHA1(5de8b569d2a91047836f4a251c21db82fd7841c9) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18232.27", 0x000000, 0x400000, CRC(f962347d) SHA1(79f07ee6b821724294ca9e7a079cb33249102508) )
	ROM_LOAD32_WORD("mpr-18231.25", 0x000002, 0x400000, CRC(673d5338) SHA1(ce592857496ccc0a51efb377cf7cccc000b4296b) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18253.31", 0x080000,  0x40000, CRC(2934e034) SHA1(4a3037b69c4835ef16a20c5573de32a862f0b13e) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18241.32", 0x0000000, 0x200000, CRC(3a380ae1) SHA1(114113325e9e5262af8750c05089f24818943cde) )
	ROM_LOAD("mpr-18242.33", 0x0200000, 0x200000, CRC(1cc3deae) SHA1(5c9cb8ce43a909b25b4e734c6a4ffd786f4dde31) )
	ROM_LOAD("mpr-18243.34", 0x0400000, 0x200000, CRC(a00a0053) SHA1(9c24fbcd0318c7e195dd153d6ba05e8c1e052968) )
	ROM_LOAD("mpr-18244.35", 0x0600000, 0x200000, CRC(bfa75beb) SHA1(fec89260d887e90ee9c2803e2eaf937cf9bfa10b) )
ROM_END

ROM_START( indy500to ) /* Defaults to Twin (Stand Alone) Cab version. 2 credits to start - Can be set to Deluxe setting in service mode, Sega Game ID# 833-11994, ROM board ID# 834-11995 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18254a.15", 0x000000, 0x080000, CRC(ad0f1fc5) SHA1(0bff35fc1d892aaffbf1a3965bf3109c54839f4b) )
	ROM_LOAD32_WORD("epr-18255a.16", 0x000002, 0x080000, CRC(784daab8) SHA1(299e87f8ec7bdefa6f94f4ab65e29e91f290611e) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18237.11",    0x0000000, 0x400000, CRC(37e4255a) SHA1(3ee69a5b9364048dfab242773d97f3af430845b7) )
	ROM_LOAD32_WORD("mpr-18238.12",    0x0000002, 0x400000, CRC(bf837bac) SHA1(6624417b65f15f20427bc42c27283f10342c76b5) )
	ROM_LOAD32_WORD("mpr-18239.9",     0x0800000, 0x400000, CRC(9a2db86e) SHA1(0b81f6037657af7d96ed5e9bfef407d87cbcc294) )
	ROM_LOAD32_WORD("mpr-18240.10",    0x0800002, 0x400000, CRC(ab46a35f) SHA1(67da857db7155a858a1fa575b6c50f4be3c9ab7c) )
	ROM_LOAD32_WORD("epr-18389.7",     0x1000000, 0x080000, CRC(d22ea019) SHA1(ef10bb0ffcb1bbcf4672bb5f705a27679a793764) )
	ROM_LOAD32_WORD("epr-18390.8",     0x1000002, 0x080000, CRC(38e796e5) SHA1(b23cfe45c363d616a65decd57aeb8ae61d5370e9) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("epr-18249.29", 0x000000, 0x080000, CRC(a399f023) SHA1(8b453313c16d935701ed7dbf71c1607c40aede63) )
	ROM_LOAD32_WORD("epr-18250.30", 0x000002, 0x080000, CRC(7479ad52) SHA1(d453e25709cd5970cd21bdc8b4785bc8eb5a50d7) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18233.17", 0x000000, 0x400000, CRC(48a024d3) SHA1(501c6ab969713187025331942f922cb0e8efa69a) )
	ROM_LOAD32_WORD("mpr-18234.21", 0x000002, 0x400000, CRC(1178bfc8) SHA1(4a9982fdce08f9d375371763dd5287e8485c24b1) )
	ROM_LOAD32_WORD("mpr-18235.18", 0x800000, 0x400000, CRC(e7d70d59) SHA1(6081739c15a634d5cc7680a4fc7decead93540ed) )
	ROM_LOAD32_WORD("mpr-18236.22", 0x800002, 0x400000, CRC(6ca29e0e) SHA1(5de8b569d2a91047836f4a251c21db82fd7841c9) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18232.27", 0x000000, 0x400000, CRC(f962347d) SHA1(79f07ee6b821724294ca9e7a079cb33249102508) )
	ROM_LOAD32_WORD("mpr-18231.25", 0x000002, 0x400000, CRC(673d5338) SHA1(ce592857496ccc0a51efb377cf7cccc000b4296b) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18391.31", 0x080000,  0x40000, CRC(79579b72) SHA1(36fed8a9eeb34968b2852ea8fc9198427f0d27c6) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18241.32", 0x0000000, 0x200000, CRC(3a380ae1) SHA1(114113325e9e5262af8750c05089f24818943cde) )
	ROM_LOAD("mpr-18242.33", 0x0200000, 0x200000, CRC(1cc3deae) SHA1(5c9cb8ce43a909b25b4e734c6a4ffd786f4dde31) )
	ROM_LOAD("mpr-18243.34", 0x0400000, 0x200000, CRC(a00a0053) SHA1(9c24fbcd0318c7e195dd153d6ba05e8c1e052968) )
	ROM_LOAD("mpr-18244.35", 0x0600000, 0x200000, CRC(bfa75beb) SHA1(fec89260d887e90ee9c2803e2eaf937cf9bfa10b) )
ROM_END

ROM_START( waverunr ) /* Wave Runner Revision A (Japan), Model 2C, Sega Game ID# 833-12838, ROM board ID# 834-12839 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19282a.15", 0x000000, 0x080000, CRC(5df58604) SHA1(a136bb80746f37450be51f98ca60791b4022035d) )
	ROM_LOAD32_WORD("epr-19283a.16", 0x000002, 0x080000, CRC(bca188e1) SHA1(428f156f60e61ef314b7b50474abddf6d4dc2aca) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19291.11",    0x0000000, 0x400000, CRC(54431d67) SHA1(25e25d9ecf3b3b1c8f5e017243cc5e02f7a13015) )
	ROM_LOAD32_WORD("mpr-19292.12",    0x0000002, 0x400000, CRC(9152d979) SHA1(0e86e21e1c88263c548e03bce48ed4ce75643596) )
	ROM_LOAD32_WORD("mpr-19293.9",     0x0800000, 0x400000, CRC(b168bea9) SHA1(0497e886ccd5e5ef0cd8670200bf4cf64d9bfc2b) )
	ROM_LOAD32_WORD("mpr-19294.10",    0x0800002, 0x400000, CRC(c731e659) SHA1(a898b03d66973a49deb9799102ab1faf4384c376) )
	ROM_LOAD32_WORD("epr-19278.7",     0x1000000, 0x080000, CRC(29ed421d) SHA1(c91eb2d68acd6ded394e0bd9f504cbb8f421c3ed) )
	ROM_LOAD32_WORD("epr-19279.8",     0x1000002, 0x080000, CRC(6ae9f899) SHA1(da46379cabe5f151160b2558e255e9b64eaca22e) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("epr-19280.29", 0x000000, 0x080000, CRC(c6b59fb9) SHA1(909663f440d19a34591d1f9707972c313e34f909) )
	ROM_LOAD32_WORD("epr-19281.30", 0x000002, 0x080000, CRC(5a6110e7) SHA1(39ba8a35fdcfdd6c88b44ab392ca0e958da44767) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19287.17", 0x0000000, 0x400000, CRC(203b9e45) SHA1(8e677a106ceb9355632fadbdb837dc4a23c83684) )
	ROM_LOAD32_WORD("mpr-19288.21", 0x0000002, 0x400000, CRC(4a488e4d) SHA1(1e680680354d873c515c955256b0e4e72451326a))
	ROM_LOAD32_WORD("mpr-19289.18", 0x0800000, 0x400000, CRC(35361cb3) SHA1(f36033765c93274a1bcdd311fb2026aa2a8a0e42) )
	ROM_LOAD32_WORD("mpr-19290.22", 0x0800002, 0x400000, CRC(67300826) SHA1(6e6bf7c709202221e03a06d9e53147d67ab4404f) )
	ROM_LOAD32_WORD("epr-19304.19", 0x1000000, 0x080000, CRC(1a7d9521) SHA1(2fdb9344441c625c3841dfc62e424ddaf16416d5) )
	ROM_LOAD32_WORD("epr-19305.23", 0x1000002, 0x080000, CRC(00412412) SHA1(93db2c6d672c5c4bf1623edad8d4237c65c4f4e1) )
	ROM_COPY( "user2", 0x1000000, 0x1100000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1700000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1800000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1900000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1a00000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1b00000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1c00000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1d00000, 0x100000 )
	ROM_COPY( "user2", 0x1000000, 0x1e00000, 0x100000 )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19286.27", 0x000000, 0x400000, CRC(3ccc33cb) SHA1(1fe53306e370da724df5caace720107798fb24b9) )
	ROM_LOAD32_WORD("mpr-19285.25", 0x000002, 0x400000, CRC(a070fbd0) SHA1(2b5441b9d7d613b0a90dfea243e9de44980d219d) )
	ROM_LOAD32_WORD("epr-19303.28", 0x800000, 0x080000, CRC(fcffc8a0) SHA1(f4b776028f581329effa583022d8d65e889b6b0a) )
	ROM_LOAD32_WORD("epr-19302.26", 0x800002, 0x080000, CRC(bd00933a) SHA1(572b49a4d0189d0513c27753e3563909d1977f03) )
	ROM_COPY( "user3", 0x800000, 0x900000, 0x100000 )
	ROM_COPY( "user3", 0x800000, 0xa00000, 0x100000 )
	ROM_COPY( "user3", 0x800000, 0xb00000, 0x100000 )
	ROM_COPY( "user3", 0x800000, 0xc00000, 0x100000 )
	ROM_COPY( "user3", 0x800000, 0xd00000, 0x100000 )
	ROM_COPY( "user3", 0x800000, 0xe00000, 0x100000 )
	ROM_COPY( "user3", 0x800000, 0xf00000, 0x100000 )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19284.31", 0x080000,  0x40000, CRC(efe5f0f3) SHA1(5e36fc7cca92e2eab7d65434cb39597505a2f8cf) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19295.32", 0x0000000, 0x400000, CRC(b14eeb09) SHA1(2a6d1b14ea3c031cad9905e4b9b6973755689ee1) )
	ROM_LOAD("mpr-19296.34", 0x0400000, 0x400000, CRC(b4b9faff) SHA1(3a258e0f7c642d043cbab5f94dfe69fac8561e93) )
ROM_END

ROM_START( rchase2 ) /* Rail Chase 2 Revision A, Model 2B. Sega game ID# 833-11809 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18045a.15", 0x000000, 0x080000, CRC(bfca0314) SHA1(9eb0f2cdab8c10fda9edc0ddc439263af3903cdc) )
	ROM_LOAD32_WORD("epr-18046a.16", 0x000002, 0x080000, CRC(0b8d3074) SHA1(fee8436399fb97ad5b8357b81e69bd5c27af1dde) )
	ROM_LOAD32_WORD("epr-18074a.13", 0x100000, 0x080000, CRC(ca4b58df) SHA1(d41cb8efd9fd65eea9e7aefadebfd0a27ef145fb) )
	ROM_LOAD32_WORD("epr-18075a.14", 0x100002, 0x080000, CRC(b82672e4) SHA1(519fdb5a978b6e82989b9841c6b59819f0d417cb) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18037.11",    0x0000000, 0x200000, CRC(dea8f896) SHA1(8eb45e46bd14a2ffbdaac47d381a1ea9b9a03ca2) )
	ROM_LOAD32_WORD("mpr-18038.12",    0x0000002, 0x200000, CRC(441f7709) SHA1(cbfa687839b6cad6a5ace45b44b95c45e4cfab0d) )
	ROM_LOAD32_WORD("mpr-18039.9",     0x0800000, 0x200000, CRC(b98c6f06) SHA1(dd1ff9c682778de1c6c09e7a5cbc95a8149488c4) )
	ROM_LOAD32_WORD("mpr-18040.10",    0x0800002, 0x200000, CRC(0d872667) SHA1(33e56486ec6b953341552b6bc21dc66f6f8aaf74) )
	ROM_LOAD32_WORD("mpr-18041.7",     0x1000000, 0x200000, CRC(e511ab0a) SHA1(c6ea14b3bdefdc59603bd2fc152ac0421fae4d6f) )
	ROM_LOAD32_WORD("mpr-18042.8",     0x1000002, 0x200000, CRC(e9a04159) SHA1(0204ba86af2707bc9e277cac68dd9ef759189c23) )
	ROM_LOAD32_WORD("mpr-18043.5",     0x1800000, 0x200000, CRC(ff84dfd6) SHA1(82833bf4cb1f367aea5fec6cffb7023cbbd3c8cb) )
	ROM_LOAD32_WORD("mpr-18044.6",     0x1800002, 0x200000, CRC(ab9b406d) SHA1(62e95ceea6f71eedbebae59e188aac03e6129e62) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASEFF ) // Coprocessor Data ROM
	/* empty?? */

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18031.17", 0x0000000, 0x200000, CRC(25d0deae) SHA1(2d0339dd7eeb2625f78e2fbe4ebdc976967175a4) )
	ROM_LOAD32_WORD("mpr-18032.21", 0x0000002, 0x200000, CRC(dbae35c2) SHA1(9510104975192a0ef1750251636daff7f089feb9) )
	ROM_LOAD32_WORD("mpr-18033.18", 0x0800000, 0x200000, CRC(1e75946c) SHA1(7dee991f0c43de9bfe17ae44767f65f12e83c811) )
	ROM_LOAD32_WORD("mpr-18034.22", 0x0800002, 0x200000, CRC(215235ad) SHA1(48227544209412fca3035e85a00d33ea654dc7b5) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18035.27", 0x000000, 0x200000, CRC(4423f66e) SHA1(c1f8dda4781dea00bd97dbf9ecfbb626dadd2c35) )
	ROM_LOAD32_WORD("mpr-18036.25", 0x000002, 0x200000, CRC(69221cf5) SHA1(e39644a08aa631dbdcfc7c0dc356e73f6a4412a9) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18047.31", 0x080000,  0x80000, CRC(4c31d459) SHA1(424d5e5a7787d0d4c68aa919ba7d575babfd1ce0) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18029.32", 0x0000000, 0x200000, CRC(f6804150) SHA1(ef40c11008c75d04159772ad30f02cdb8c5464f3) )
	ROM_LOAD("mpr-18030.34", 0x0400000, 0x200000, CRC(1167615d) SHA1(bae0060aec3c15f08342f11df665c05c5703523d) )

	/* Z80 code located on the I/O board type 837-11694. Z80 @ 4Mhz with 8-way DSW & unknown SONY CXD10950 QFP64 chip */
	ROM_REGION( 0x8000, "iocpu", 0 )
	ROM_LOAD("epr-17895.ic8", 0x0000, 0x8000, CRC(8fd7003d) SHA1(b8b16e20e3ed07326330ba335ea1e701cc0bec17) )
ROM_END


/*
Behind Enemy Lines
Sega, 1998

This game runs on Sega Model2 C-CRX hardware

PCB No: 837-12469-01
CPU   : intel i960
SOUND : MC68EC000FN12
OSC   : 32.000MHz (x3), 50.000MHz, 20.000MHz, 45.158MHz
DIPSW : 8 position (x1)
RAM   : M5M44170CJ (x2), HM514270CJ7 (x1), HM538254BJ-7 (x4), N341256SJ-15 (x10)
        UM62256EM-70LL (x4), TC18128CFWL-80V (x12), TC55V328AJ-15 (x3)
        CY7C185-25VC (x7), CY7C188-25VC (x2), dt71256 (x4), BR6265BF-10SL (x2)
        65256BLFP-10T (x2), LH521002AK-20 (x16)

CUSTOM: SEGA 315-5687   (128 QFP)
        FUJITSU MB86235 (x2, 208 QFP)
        SEGA 315-5673   (240 QFP)
        SEGA 315-5798   (304 QFP)
        SEGA 315-5799   (368 QFP)
        SEGA 315-5725   (x2, 144 QFP)
        SEGA 315-5292A  (160 QFP)
        SEGA 315-5648   (64 QFP)
        SEGA 315-5672   (196 QFP)
        SEGA 315-5649   (100 QFP)
PAL   : 315-5879
OTHER : D71051GU-10
        LATTICE PLSI 2032 80LJ D702S08 (x2)
        TDA1386T

Note: All epr* ROMs are 27C1024
      All mpr* ROMs are 16M MASK

[JUMPERS]
JP1     2-3
JP2     1-2
JP3     2-3
JP4     1-2
JP5     1-2
JP6     1-2
JP7     1-2
JP8     2-3
JP9     2-3
JP10    1-2
JP11    1-2
JP12    1-2
JP13    1-2
JP14    1-2
JP15    2-3
JP16    1-2
JP17    1-2
JP18    1-2
JP19    1-2
JP20    2-3
JP21    2-3
JP22    2-3
JP23    2-3
JP24    2-3
*/
ROM_START( bel ) /* Behind Enemy Lines, Model 2C */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-20225.15",   0x000000, 0x020000, CRC(4abc6b59) SHA1(cc6da75aafcbbc86720435182a66e8de065c8e99) )
	ROM_LOAD32_WORD("epr-20226.16",   0x000002, 0x020000, CRC(43e05b3a) SHA1(204b3cc6bbfdc92b4871c45fe4abff4ab4a66317) )
	ROM_LOAD32_WORD("epr-20223.13",   0x040000, 0x020000, CRC(61b1be98) SHA1(03c308c58a72bf3b78f41d5a9c0adaa7aad631c2) )
	ROM_LOAD32_WORD("epr-20224.14",   0x040002, 0x020000, CRC(eb2d7dbf) SHA1(f3b126e2fcef1cf673b239696ed8018241b1170e) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-20233.11",   0x000000, 0x200000, CRC(3e079a3b) SHA1(a3f15cd68a514cf80f0a40dbbb08e8b0489a0e4b) )
	ROM_LOAD32_WORD("mpr-20234.12",   0x000002, 0x200000, CRC(58bde826) SHA1(386d0d07738f579cb23e4168aceb26f56bcca1c1) )
	ROM_LOAD32_WORD("mpr-20231.9",    0x400000, 0x200000, CRC(b3393e93) SHA1(aa52ae307aa37faaaf86c326642af1946c5f4056) )
	ROM_LOAD32_WORD("mpr-20232.10",   0x400002, 0x200000, CRC(da4a2e11) SHA1(f9138813f6d1ca2126f5de10d8d69dcbb533aa0e) )
	ROM_LOAD32_WORD("mpr-20229.7",    0x800000, 0x200000, CRC(cdec7bf4) SHA1(510b6d41f1d32a9929379ba76037db137164cd43) )
	ROM_LOAD32_WORD("mpr-20230.8",    0x800002, 0x200000, CRC(a166fa87) SHA1(d4f6d4fba7f43b21f0bf9d948ec93b372425bf7c) )
	ROM_LOAD32_WORD("mpr-20227.5",    0xc00000, 0x200000, CRC(1277686e) SHA1(fff27006659458300001425261b944e690f1d494) )
	ROM_LOAD32_WORD("mpr-20228.6",    0xc00002, 0x200000, CRC(49cb5568) SHA1(ee3273302830f3499c7d4e548b629c51e0369e8a) )

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-20236.29",   0x000000, 0x200000, CRC(8de9a3c2) SHA1(e7fde1fd509531e1002ff813163067dc0d134536) )
	ROM_LOAD32_WORD("mpr-20235.30",   0x000002, 0x200000, CRC(78fa11ef) SHA1(a60deabb662e9c09f5d6342dc1a1c6045744d93f) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-20244.17",  0x0000000, 0x200000, CRC(9d2a8660) SHA1(59302e7119c9ff779ce0c871713fe3688c29cccb) )
	ROM_LOAD32_WORD("mpr-20240.21",  0x0000002, 0x200000, CRC(51615908) SHA1(c70252b0b6f17aa0cd9b5264d4166df8ab7d1784) )
	ROM_LOAD32_WORD("mpr-20243.18",  0x0400000, 0x200000, CRC(48671f7c) SHA1(b0bdc7f42450c8d9cebbcf43cf858f7399e378e4) )
	ROM_LOAD32_WORD("mpr-20239.22",  0x0400002, 0x200000, CRC(6cd8d8a5) SHA1(1c634fbbcbafb1c3825117682901a3264599b246) )
	ROM_LOAD32_WORD("mpr-20242.19",  0x0800000, 0x200000, CRC(e7f86ac7) SHA1(7b7724127b27834eaaa228050ceb779d8a027882) )
	ROM_LOAD32_WORD("mpr-20238.23",  0x0800002, 0x200000, CRC(0a480c7c) SHA1(239d2c9c49cb8ddc0d6aa956a497b494217f38d7) )
	ROM_LOAD32_WORD("mpr-20241.20",  0x0c00000, 0x200000, CRC(51974b98) SHA1(7d6ab9c0ccec77676222611bf200d2e067e20520) )
	ROM_LOAD32_WORD("mpr-20237.24",  0x0c00002, 0x200000, CRC(89b5d8b6) SHA1(6e0a0323d6a804f1f1e4404694cc1ea7dfbf2d95) )

	ROM_REGION( 0xc00000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-20247.27",   0x000000, 0x200000, CRC(00b0417d) SHA1(5e9d38509c1e5273079a342a64ca2c956cd47e6d) )
	ROM_LOAD32_WORD("mpr-20245.25",   0x000002, 0x200000, CRC(36490a08) SHA1(a462e094c9a9ec4743e4bf2c4ce23357257a2a54) )
	ROM_LOAD32_WORD("mpr-20248.28",   0x800000, 0x200000, CRC(0ace6bef) SHA1(a231aeb7b984f5b927144f0eec4ef2282429494f) )
	ROM_LOAD32_WORD("mpr-20246.26",   0x800002, 0x200000, CRC(250d6ca1) SHA1(cd1d4bc0fcf89e47884b87863a09bb263bce72cc) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("mpr-20249.31",  0x080000, 0x020000, CRC(dc24f13d) SHA1(66ab8e843319d07663ef13f3d2299c6c7414071f) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-20250.32",  0x0000000, 0x200000, CRC(91b735d3) SHA1(b0e7e493fb20ebf30c17378199e49d529ffb3f20) )
	ROM_LOAD("mpr-20251.33",  0x0200000, 0x200000, CRC(703a947b) SHA1(95b8d3dc29e87e6537b288d8e946728e0b345dd0) )
	ROM_LOAD("mpr-20252.34",  0x0400000, 0x200000, CRC(8f48f375) SHA1(9e511e89e99c77f06a5fba033ca8f9b98bd86f91) )
	ROM_LOAD("mpr-20253.35",  0x0600000, 0x200000, CRC(ca6aa17c) SHA1(f6df2483ca75573449ba36638dbbed4be7843a44) )
ROM_END

ROM_START( overrev ) /* Over Rev Revision A, Model 2C */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-20124a.15", 0x000000, 0x080000, CRC(74beb8d7) SHA1(c65c641138ecd7312c4930702d1498b8a346175a) )
	ROM_LOAD32_WORD( "epr-20125a.16", 0x000002, 0x080000, CRC(def64456) SHA1(cedb64d2d99a73301ef45c2f5f860a9b87faf6a7) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-19996.11",  0x000000, 0x400000, CRC(21928a00) SHA1(6b439fd2b113b64df9378ef8180a17aa6fa975c5) )
	ROM_LOAD32_WORD( "mpr-19997.12",  0x000002, 0x400000, CRC(2a169cab) SHA1(dbf9af938afd0599d345c42c1df242e575c14de9) )
	ROM_LOAD32_WORD( "mpr-19994.9",   0x800000, 0x400000, CRC(e691fbd5) SHA1(b99c2f3f2a682966d792917dfcb8ed8e53bc0b7a) )
	ROM_LOAD32_WORD( "mpr-19995.10",  0x800002, 0x400000, CRC(82a7828e) SHA1(4336a12a07a67f94091b4a9b491bab02c375dd15) )

	ROM_REGION( 0x800000, "cpu2", ROMREGION_ERASE00 ) // TGPx4 program (COPRO sockets)

	ROM_REGION( 0x800000, "user2", 0 ) // Models (TGP sockets)
	ROM_LOAD32_WORD( "mpr-19998.17",  0x000000, 0x200000, CRC(6a834574) SHA1(8be19bf42dbb157d6acde62a2018ef4c0d41aab4) )
	ROM_LOAD32_WORD( "mpr-19999.21",  0x000002, 0x200000, CRC(ff590a2d) SHA1(ad29e4270b4a2f82189fbab83358eb1200f43777) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures (TEXTURE sockets)
	ROM_LOAD32_WORD( "mpr-20001.27",  0x000000, 0x200000, CRC(6ca236aa) SHA1(b3cb89fadb42afed13be4f229d7158dee487978a) )
	ROM_LOAD32_WORD( "mpr-20000.25",  0x000002, 0x200000, CRC(894d8ded) SHA1(9bf7c754a29eef47fa49b5567980601895127306) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-20002.31", 0x080000, 0x080000, CRC(7efb069e) SHA1(30b1bbaf348d6a6b9ee2fdf82a0749baa025e0bf) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-20003.32", 0x000000, 0x400000, CRC(149ac22b) SHA1(c890bbaebbbb07b62bcb8a3a8edded9fa0ec9a1e) )
	ROM_LOAD( "mpr-20004.34", 0x400000, 0x400000, CRC(0b9c5410) SHA1(e5bb30702fc853ccc03316be07a334269d3ebb4a) )
ROM_END

ROM_START( overrevb ) /* Over Rev Revision B, Model 2B, rom board stickered as 836-13275 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-19992b.15", 0x000000, 0x080000, CRC(6d3e78d5) SHA1(40d18ee284ea2e038f7e3d04db56e793ab3e3dd5) ) /* sum16 492A printed on label */
	ROM_LOAD32_WORD( "epr-19993b.16", 0x000002, 0x080000, CRC(765dc9ce) SHA1(a718c32ca27ec1fb5ed2d7d3797ea7e906510a04) ) /* sum16 B955 printed on label */

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-19996.11",  0x000000, 0x400000, CRC(21928a00) SHA1(6b439fd2b113b64df9378ef8180a17aa6fa975c5) )
	ROM_LOAD32_WORD( "mpr-19997.12",  0x000002, 0x400000, CRC(2a169cab) SHA1(dbf9af938afd0599d345c42c1df242e575c14de9) )
	ROM_LOAD32_WORD( "mpr-19994.9",   0x800000, 0x400000, CRC(e691fbd5) SHA1(b99c2f3f2a682966d792917dfcb8ed8e53bc0b7a) )
	ROM_LOAD32_WORD( "mpr-19995.10",  0x800002, 0x400000, CRC(82a7828e) SHA1(4336a12a07a67f94091b4a9b491bab02c375dd15) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-19998.17",  0x000000, 0x200000, CRC(6a834574) SHA1(8be19bf42dbb157d6acde62a2018ef4c0d41aab4) )
	ROM_LOAD32_WORD( "mpr-19999.21",  0x000002, 0x200000, CRC(ff590a2d) SHA1(ad29e4270b4a2f82189fbab83358eb1200f43777) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-20001.27",  0x000000, 0x200000, CRC(6ca236aa) SHA1(b3cb89fadb42afed13be4f229d7158dee487978a) )
	ROM_LOAD32_WORD( "mpr-20000.25",  0x000002, 0x200000, CRC(894d8ded) SHA1(9bf7c754a29eef47fa49b5567980601895127306) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP( "epr-20002.31", 0x080000, 0x080000, CRC(7efb069e) SHA1(30b1bbaf348d6a6b9ee2fdf82a0749baa025e0bf) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD( "mpr-20003.32", 0x000000, 0x400000, CRC(149ac22b) SHA1(c890bbaebbbb07b62bcb8a3a8edded9fa0ec9a1e) )
	ROM_LOAD( "mpr-20004.34", 0x400000, 0x400000, CRC(0b9c5410) SHA1(e5bb30702fc853ccc03316be07a334269d3ebb4a) )
ROM_END

ROM_START( topskatr ) /* Top Skater Revision A (Export), Model 2C, Sega Game ID# 833-13080-02, ROM board ID# 834-13081-02 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19755a.15", 0x000000, 0x080000, CRC(b80633b9) SHA1(5396da414beeb918e6f38f25a43dd76345a0c8ed) )
	ROM_LOAD32_WORD("epr-19756a.16", 0x000002, 0x080000, CRC(472046a2) SHA1(06d0f609257ba476e6bd3b956e0850e7167429ce) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19735.11",  0x000000, 0x400000, CRC(8e509266) SHA1(49afc91467f08befaf34e743cbe823de3e3c9d85) )
	ROM_LOAD32_WORD("mpr-19736.12",  0x000002, 0x400000, CRC(094e0a0d) SHA1(de2c739f71e51166263446b9f6a566866ab8bee8) )
	ROM_LOAD32_WORD("mpr-19737.9",   0x800000, 0x400000, CRC(281a7dde) SHA1(71d5ba434328a81969bfdc71ac1160c5ff3ae9d3) )
	ROM_LOAD32_WORD("mpr-19738.10",  0x800002, 0x400000, CRC(f688327e) SHA1(68c9db242ef7e8f98979e968a09e4b093bc5d470) )

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-19743.29",  0x000000, 0x200000, CRC(d41a41bf) SHA1(a5f6b24e6526d0d2ef9c526c273c018d1e0fed59) )
	ROM_LOAD32_WORD("mpr-19744.30",  0x000002, 0x200000, CRC(84f203bf) SHA1(4952b764e6bf6cd735018738c5eff08781ee2315) )

	ROM_REGION( 0x400000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19741.17",  0x000000, 0x200000, CRC(111a6e29) SHA1(8664059f157626e4bbdcf8357e3d30b37d3c25b8) )
	ROM_LOAD32_WORD("mpr-19742.21",  0x000002, 0x200000, CRC(28510aff) SHA1(3e68aec090f36a60b3b70bc90f09e2f9ce088718) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19740.27",  0x000000, 0x400000, CRC(b20f508b) SHA1(c90fa3b42d87291ea459ccc137f3a2f3eb7efec0) )
	ROM_LOAD32_WORD("mpr-19739.25",  0x000002, 0x400000, CRC(8120cfd8) SHA1(a82744bff5dcdfae296c7c3e8c3fbfda26324e85) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("mpr-19759.31", 0x080000,  0x80000, CRC(573530f2) SHA1(7b205085965d6694f8e75e29c4028f7cb6f631ab) )

	ROM_REGION( 0x20000, "cpu3", 0) // DSB program
	ROM_LOAD16_WORD_SWAP("mpr-19760.2s", 0x000000,  0x20000, CRC(2e41ca15) SHA1(a302209bfe0f1491dff2da64b32cfaa13c3d3304) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD16_WORD_SWAP("mpr-19745.32", 0x000000, 0x400000, CRC(7082a0af) SHA1(415f9d0793a697cb1719bbd96370f4a741866527) )
	ROM_LOAD16_WORD_SWAP("mpr-19746.34", 0x400000, 0x400000, CRC(657b5977) SHA1(ca76f211d68b6b55678a4d7949bfd2ddef1b1710) )

	ROM_REGION( 0x1000000, "mpeg", 0 ) // MPEG audio data
	ROM_LOAD("mpr-19747.18s", 0x000000, 0x400000, CRC(6e895aaa) SHA1(4c67c1e1d58a3034bbd711252a78689db9f235bb) )
	ROM_LOAD("mpr-19748.20s", 0x400000, 0x400000, CRC(fcd74de3) SHA1(fd4da4cf40c4342c6263cf22eee5968292a4d2c0) )
	ROM_LOAD("mpr-19749.22s", 0x800000, 0x400000, CRC(842ca1eb) SHA1(6ee6b2eb2ea400bdb9c0a9b4a126b4b86886e813) )
	ROM_LOAD("mpr-19750.24s", 0xc00000, 0x400000, CRC(cd95d0bf) SHA1(40e2a2980c89049c339fefd48bf7aac79962cd2e) )
ROM_END

ROM_START( topskatru ) /* Top Skater Revision A (USA), Model 2C, Sega Game ID# 833-13080-01, ROM board ID# 834-13081-01 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-19753a.15", 0x000000, 0x080000, CRC(3b3028de) SHA1(717ebf0ccd87128a24776e618cf15f07aaf48537) )
	ROM_LOAD32_WORD( "epr-19754a.16", 0x000002, 0x080000, CRC(17535b98) SHA1(a2329d09821900ec4f867caf1a93759085bd0a62) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19735.11",  0x000000, 0x400000, CRC(8e509266) SHA1(49afc91467f08befaf34e743cbe823de3e3c9d85) )
	ROM_LOAD32_WORD("mpr-19736.12",  0x000002, 0x400000, CRC(094e0a0d) SHA1(de2c739f71e51166263446b9f6a566866ab8bee8) )
	ROM_LOAD32_WORD("mpr-19737.9",   0x800000, 0x400000, CRC(281a7dde) SHA1(71d5ba434328a81969bfdc71ac1160c5ff3ae9d3) )
	ROM_LOAD32_WORD("mpr-19738.10",  0x800002, 0x400000, CRC(f688327e) SHA1(68c9db242ef7e8f98979e968a09e4b093bc5d470) )

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-19743.29",  0x000000, 0x200000, CRC(d41a41bf) SHA1(a5f6b24e6526d0d2ef9c526c273c018d1e0fed59) )
	ROM_LOAD32_WORD("mpr-19744.30",  0x000002, 0x200000, CRC(84f203bf) SHA1(4952b764e6bf6cd735018738c5eff08781ee2315) )

	ROM_REGION( 0x400000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19741.17",  0x000000, 0x200000, CRC(111a6e29) SHA1(8664059f157626e4bbdcf8357e3d30b37d3c25b8) )
	ROM_LOAD32_WORD("mpr-19742.21",  0x000002, 0x200000, CRC(28510aff) SHA1(3e68aec090f36a60b3b70bc90f09e2f9ce088718) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19740.27",  0x000000, 0x400000, CRC(b20f508b) SHA1(c90fa3b42d87291ea459ccc137f3a2f3eb7efec0) )
	ROM_LOAD32_WORD("mpr-19739.25",  0x000002, 0x400000, CRC(8120cfd8) SHA1(a82744bff5dcdfae296c7c3e8c3fbfda26324e85) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("mpr-19759.31", 0x080000,  0x80000, CRC(573530f2) SHA1(7b205085965d6694f8e75e29c4028f7cb6f631ab) )

	ROM_REGION( 0x20000, "cpu3", 0) // DSB program
	ROM_LOAD16_WORD_SWAP("mpr-19760.2s", 0x000000,  0x20000, CRC(2e41ca15) SHA1(a302209bfe0f1491dff2da64b32cfaa13c3d3304) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD16_WORD_SWAP("mpr-19745.32", 0x000000, 0x400000, CRC(7082a0af) SHA1(415f9d0793a697cb1719bbd96370f4a741866527) )
	ROM_LOAD16_WORD_SWAP("mpr-19746.34", 0x400000, 0x400000, CRC(657b5977) SHA1(ca76f211d68b6b55678a4d7949bfd2ddef1b1710) )

	ROM_REGION( 0x1000000, "mpeg", 0 ) // MPEG audio data
	ROM_LOAD("mpr-19747.18s", 0x000000, 0x400000, CRC(6e895aaa) SHA1(4c67c1e1d58a3034bbd711252a78689db9f235bb) )
	ROM_LOAD("mpr-19748.20s", 0x400000, 0x400000, CRC(fcd74de3) SHA1(fd4da4cf40c4342c6263cf22eee5968292a4d2c0) )
	ROM_LOAD("mpr-19749.22s", 0x800000, 0x400000, CRC(842ca1eb) SHA1(6ee6b2eb2ea400bdb9c0a9b4a126b4b86886e813) )
	ROM_LOAD("mpr-19750.24s", 0xc00000, 0x400000, CRC(cd95d0bf) SHA1(40e2a2980c89049c339fefd48bf7aac79962cd2e) )
ROM_END

ROM_START( topskatrj ) /* Top Skater (Japan), Model 2C, Sega Game ID# 833-13080-03, ROM board ID# 834-13081-03 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-19751.15", 0x000000, 0x080000, CRC(d615a15f) SHA1(ca998de446c4c423db186696f3478f3daa4f8373) )
	ROM_LOAD32_WORD( "epr-19752.16", 0x000002, 0x080000, CRC(42f0ba8b) SHA1(f72f25cbd380918b919c11a7d2051948c8c484db) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19735.11",  0x000000, 0x400000, CRC(8e509266) SHA1(49afc91467f08befaf34e743cbe823de3e3c9d85) )
	ROM_LOAD32_WORD("mpr-19736.12",  0x000002, 0x400000, CRC(094e0a0d) SHA1(de2c739f71e51166263446b9f6a566866ab8bee8) )
	ROM_LOAD32_WORD("mpr-19737.9",   0x800000, 0x400000, CRC(281a7dde) SHA1(71d5ba434328a81969bfdc71ac1160c5ff3ae9d3) )
	ROM_LOAD32_WORD("mpr-19738.10",  0x800002, 0x400000, CRC(f688327e) SHA1(68c9db242ef7e8f98979e968a09e4b093bc5d470) )

	ROM_REGION( 0x800000, "cpu2", 0 ) // TGPx4 program
	ROM_LOAD32_WORD("mpr-19743.29",  0x000000, 0x200000, CRC(d41a41bf) SHA1(a5f6b24e6526d0d2ef9c526c273c018d1e0fed59) )
	ROM_LOAD32_WORD("mpr-19744.30",  0x000002, 0x200000, CRC(84f203bf) SHA1(4952b764e6bf6cd735018738c5eff08781ee2315) )

	ROM_REGION( 0x400000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19741.17",  0x000000, 0x200000, CRC(111a6e29) SHA1(8664059f157626e4bbdcf8357e3d30b37d3c25b8) )
	ROM_LOAD32_WORD("mpr-19742.21",  0x000002, 0x200000, CRC(28510aff) SHA1(3e68aec090f36a60b3b70bc90f09e2f9ce088718) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19740.27",  0x000000, 0x400000, CRC(b20f508b) SHA1(c90fa3b42d87291ea459ccc137f3a2f3eb7efec0) )
	ROM_LOAD32_WORD("mpr-19739.25",  0x000002, 0x400000, CRC(8120cfd8) SHA1(a82744bff5dcdfae296c7c3e8c3fbfda26324e85) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("mpr-19759.31", 0x080000,  0x80000, CRC(573530f2) SHA1(7b205085965d6694f8e75e29c4028f7cb6f631ab) )

	ROM_REGION( 0x20000, "cpu3", 0) // DSB program
	ROM_LOAD16_WORD_SWAP("mpr-19760.2s", 0x000000,  0x20000, CRC(2e41ca15) SHA1(a302209bfe0f1491dff2da64b32cfaa13c3d3304) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD16_WORD_SWAP("mpr-19745.32", 0x000000, 0x400000, CRC(7082a0af) SHA1(415f9d0793a697cb1719bbd96370f4a741866527) )
	ROM_LOAD16_WORD_SWAP("mpr-19746.34", 0x400000, 0x400000, CRC(657b5977) SHA1(ca76f211d68b6b55678a4d7949bfd2ddef1b1710) )

	ROM_REGION( 0x1000000, "mpeg", 0 ) // MPEG audio data
	ROM_LOAD("mpr-19747.18s", 0x000000, 0x400000, CRC(6e895aaa) SHA1(4c67c1e1d58a3034bbd711252a78689db9f235bb) )
	ROM_LOAD("mpr-19748.20s", 0x400000, 0x400000, CRC(fcd74de3) SHA1(fd4da4cf40c4342c6263cf22eee5968292a4d2c0) )
	ROM_LOAD("mpr-19749.22s", 0x800000, 0x400000, CRC(842ca1eb) SHA1(6ee6b2eb2ea400bdb9c0a9b4a126b4b86886e813) )
	ROM_LOAD("mpr-19750.24s", 0xc00000, 0x400000, CRC(cd95d0bf) SHA1(40e2a2980c89049c339fefd48bf7aac79962cd2e) )
ROM_END

ROM_START( doaa ) /* Dead or Alive Revision A, Model 2A, Sega Game ID# 833-11341, ROM board ID# 834-11342, 837-12880 security board */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19310a.12", 0x000000, 0x080000, CRC(06486f7a) SHA1(b3e14103570e5f45aed16e1c158e469bc85002ae) )
	ROM_LOAD32_WORD("epr-19311a.13", 0x000002, 0x080000, CRC(1be62912) SHA1(dcc2df8e28e1a107867f74248e6ffcac83afe7c0) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19318.11",  0x0000000, 0x400000, CRC(ab431bfe) SHA1(45b5ccf67c91014daf6bf3c4bd8ec372b246e404) )
	ROM_LOAD32_WORD("mpr-19319.12",  0x0000002, 0x400000, CRC(c5cb694d) SHA1(448b45d30cc7a71395a49a2c5789989fd7b7b4e7) )
	ROM_LOAD32_WORD("mpr-19316.9",   0x0800000, 0x400000, CRC(2d2d1b1a) SHA1(77ce5d8aa98bdbc97ae08a452f584b30d8885cfc) )
	ROM_LOAD32_WORD("mpr-19317.10",  0x0800002, 0x400000, CRC(96b17bcf) SHA1(3aa9d2f8afad74b5626ce2cf2d7a86aef8cac80b) )
	ROM_LOAD32_WORD("mpr-19314.7",   0x1000000, 0x400000, CRC(a8d963fb) SHA1(6a1680d6380321279b0d701e4b47d4ae712f3b72) )
	ROM_LOAD32_WORD("mpr-19315.8",   0x1000002, 0x400000, CRC(90ae5682) SHA1(ec56df14f0847daf9bd0435f785a8946c94d2988) )
	ROM_LOAD32_WORD("mpr-19312.5",   0x1800000, 0x200000, CRC(1dcedb10) SHA1(a60fb9e7c0731004d0f0ff28c4cde272b21dd658) )
	ROM_RELOAD     (                 0x1c00000, 0x200000 )
	ROM_LOAD32_WORD("mpr-19313.6",   0x1800002, 0x200000, CRC(8c63055e) SHA1(9f375b3f4a8884163ffcf364989499f2cd21e18b) )
	ROM_RELOAD     (                 0x1c00002, 0x200000 )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP data (COPRO sockets)

	ROM_REGION( 0x1800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19322.17",  0x0000000, 0x400000, CRC(d0e6ecf0) SHA1(1b87f6337b4286fd738856da899462e7baa92601) )
	ROM_LOAD32_WORD("mpr-19325.21",  0x0000002, 0x400000, CRC(7cbe432d) SHA1(8b31e292160b88df9c77b36096914d09ab8b6086) )
	ROM_LOAD32_WORD("mpr-19323.18",  0x0800000, 0x400000, CRC(453d3f4a) SHA1(8c0530824bb8ecb007021ee6e93412597bb0ecd6) )
	ROM_LOAD32_WORD("mpr-19326.22",  0x0800002, 0x400000, CRC(b976da02) SHA1(a154eb128604aac9e35438d8811971133eab94a1) )
	ROM_LOAD32_WORD("mpr-19324.19",  0x1000000, 0x400000, CRC(d972201f) SHA1(1857ffc58697997ee22436586c398eb0c3daba6c) )
	ROM_LOAD32_WORD("mpr-19327.23",  0x1000002, 0x400000, CRC(6a75634c) SHA1(8ed74c7afd95fc7a4df0f01a47479b6f44e3073c) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19321.27", 0x000000, 0x400000, CRC(9c49e845) SHA1(344839640d9814263fa5ed00c2043cd6f18d5cb2) )
	ROM_LOAD32_WORD("mpr-19320.25", 0x000002, 0x400000, CRC(190c017f) SHA1(4c3250b9abe39fc5c8fd0fcdb5fb7ea131434516) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19328.30", 0x080000,  0x80000, CRC(400bdbfb) SHA1(54db969fa54cf3c502d77aa6a6aaeef5d7db9f04) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19329.31", 0x000000, 0x200000, CRC(8fd2708a) SHA1(7a341b15afa489aa95af70cb34ac3934b1a7d887) )
	ROM_LOAD("mpr-19330.32", 0x200000, 0x200000, CRC(0c69787d) SHA1(dc5870cd93da2babe5fc9c03b252fc6ea6e45721) )
	ROM_LOAD("mpr-19331.33", 0x400000, 0x200000, CRC(c18ea0b8) SHA1(0f42458829ae85fffcedd42cd9f728a7a3d75f1c) )
	ROM_LOAD("mpr-19332.34", 0x600000, 0x200000, CRC(2877f96f) SHA1(00e5677da30527b862e238f10762a5cbfbabde2b) )

	MODEL2_CPU_BOARD
	MODEL2A_VID_BOARD
ROM_END

ROM_START( doa ) /* Dead or Alive Revision B, Model 2B, 837-12880 security board */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19379b.15", 0x000000, 0x080000, CRC(8a10a944) SHA1(c675a344f74d0118907fb5292495883c0c30c719) )
	ROM_LOAD32_WORD("epr-19380b.16", 0x000002, 0x080000, CRC(766c1ec8) SHA1(49250886f66db9fd37d88bc22c8f22046f74f043) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19318.11",  0x0000000, 0x400000, CRC(ab431bfe) SHA1(45b5ccf67c91014daf6bf3c4bd8ec372b246e404) )
	ROM_LOAD32_WORD("mpr-19319.12",  0x0000002, 0x400000, CRC(c5cb694d) SHA1(448b45d30cc7a71395a49a2c5789989fd7b7b4e7) )
	ROM_LOAD32_WORD("mpr-19316.9",   0x0800000, 0x400000, CRC(2d2d1b1a) SHA1(77ce5d8aa98bdbc97ae08a452f584b30d8885cfc) )
	ROM_LOAD32_WORD("mpr-19317.10",  0x0800002, 0x400000, CRC(96b17bcf) SHA1(3aa9d2f8afad74b5626ce2cf2d7a86aef8cac80b) )
	ROM_LOAD32_WORD("mpr-19314.7",   0x1000000, 0x400000, CRC(a8d963fb) SHA1(6a1680d6380321279b0d701e4b47d4ae712f3b72) )
	ROM_LOAD32_WORD("mpr-19315.8",   0x1000002, 0x400000, CRC(90ae5682) SHA1(ec56df14f0847daf9bd0435f785a8946c94d2988) )
	ROM_LOAD32_WORD("mpr-19312.5",   0x1800000, 0x200000, CRC(1dcedb10) SHA1(a60fb9e7c0731004d0f0ff28c4cde272b21dd658) )
	ROM_RELOAD     (                 0x1c00000, 0x200000 )
	ROM_LOAD32_WORD("mpr-19313.6",   0x1800002, 0x200000, CRC(8c63055e) SHA1(9f375b3f4a8884163ffcf364989499f2cd21e18b) )
	ROM_RELOAD     (                 0x1c00002, 0x200000 )

	ROM_REGION( 0x1800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19322.17",  0x0000000, 0x400000, CRC(d0e6ecf0) SHA1(1b87f6337b4286fd738856da899462e7baa92601) )
	ROM_LOAD32_WORD("mpr-19325.21",  0x0000002, 0x400000, CRC(7cbe432d) SHA1(8b31e292160b88df9c77b36096914d09ab8b6086) )
	ROM_LOAD32_WORD("mpr-19323.18",  0x0800000, 0x400000, CRC(453d3f4a) SHA1(8c0530824bb8ecb007021ee6e93412597bb0ecd6) )
	ROM_LOAD32_WORD("mpr-19326.22",  0x0800002, 0x400000, CRC(b976da02) SHA1(a154eb128604aac9e35438d8811971133eab94a1) )
	ROM_LOAD32_WORD("mpr-19324.19",  0x1000000, 0x400000, CRC(d972201f) SHA1(1857ffc58697997ee22436586c398eb0c3daba6c) )
	ROM_LOAD32_WORD("mpr-19327.23",  0x1000002, 0x400000, CRC(6a75634c) SHA1(8ed74c7afd95fc7a4df0f01a47479b6f44e3073c) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19321.27", 0x000000, 0x400000, CRC(9c49e845) SHA1(344839640d9814263fa5ed00c2043cd6f18d5cb2) )
	ROM_LOAD32_WORD("mpr-19320.25", 0x000002, 0x400000, CRC(190c017f) SHA1(4c3250b9abe39fc5c8fd0fcdb5fb7ea131434516) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19328.30", 0x080000,  0x80000, CRC(400bdbfb) SHA1(54db969fa54cf3c502d77aa6a6aaeef5d7db9f04) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19329.31", 0x000000, 0x200000, CRC(8fd2708a) SHA1(7a341b15afa489aa95af70cb34ac3934b1a7d887) )
	ROM_LOAD("mpr-19330.32", 0x200000, 0x200000, CRC(0c69787d) SHA1(dc5870cd93da2babe5fc9c03b252fc6ea6e45721) )
	ROM_LOAD("mpr-19331.33", 0x400000, 0x200000, CRC(c18ea0b8) SHA1(0f42458829ae85fffcedd42cd9f728a7a3d75f1c) )
	ROM_LOAD("mpr-19332.34", 0x600000, 0x200000, CRC(2877f96f) SHA1(00e5677da30527b862e238f10762a5cbfbabde2b) )
ROM_END

ROM_START( sgt24h ) /* Super GT 24h, Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19155.15", 0x000000, 0x080000, CRC(593952fd) SHA1(1fc4afc6e3910cc8adb0688542e61a9efb442e56) )
	ROM_LOAD32_WORD("epr-19156.16", 0x000002, 0x080000, CRC(a91fc4ee) SHA1(a37611da0295f7d7e5d2411c3f9b73140d311f74) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19146.11", 0x000000, 0x400000, CRC(d66b5b0f) SHA1(c2a5b83c9041d8f46dfac4a3ff8cfdefb96d02b3) )
	ROM_LOAD32_WORD("mpr-19147.12", 0x000002, 0x400000, CRC(d5558f48) SHA1(c9f40328d6974b7767fa6ba719d0d2b7a173c210) )
	ROM_LOAD32_WORD("mpr-19148.9",  0x800000, 0x400000, CRC(a14c86db) SHA1(66cd8672c00e4e2572de7c5648de595674ffa8f8) )
	ROM_LOAD32_WORD("mpr-19149.10", 0x800002, 0x400000, CRC(94ef5849) SHA1(3e1748dc5e61c93eedbf0ca6b1946a30be722403) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19150.17", 0x000000, 0x400000, CRC(e0ad870e) SHA1(3429d9f9434d75ddb5fa05d4b493828adfe826a4) )
	ROM_LOAD32_WORD("mpr-19151.21", 0x000002, 0x400000, CRC(e2a1b125) SHA1(cc5c2d9ab8a01f52e66969464f53ae3cefca6a09) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19153.27", 0x000000, 0x200000, CRC(136adfd0) SHA1(70ce4e609c8b003ff04518044c18d29089e6a353) )
	ROM_LOAD32_WORD("mpr-19152.25", 0x000002, 0x200000, CRC(363769a2) SHA1(51b2f11a01fb72e151025771f8a8496993e605c2) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD16_WORD_SWAP("epr-18643a.7", 0x000000,  0x20000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19157.31", 0x080000,  0x80000, CRC(8ffea0cf) SHA1(439e784081329db2fe03419681150f3216f4ccff) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19154.32", 0x000000, 0x400000, CRC(7cd9e679) SHA1(b9812c4f3042f95febc96bcdd46e3b0724ad4b4f) )
ROM_END

ROM_START( von ) /* Virtual On Cyber Troopers Revision B (US), Model 2B, Sega Game ID# 833-12345-01, ROM board ID# 834-12346-01 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18828b.15", 0x000000, 0x080000, CRC(6499cc59) SHA1(8289be295f021acbf0c903513ba97ae7de50dedb) )
	ROM_LOAD32_WORD("epr-18829b.16", 0x000002, 0x080000, CRC(0053b10f) SHA1(b89cc814b02b4ab5e37c75ee1a9cf57b88b63053) )
	ROM_LOAD32_WORD("epr-18666.13",  0x100000, 0x080000, CRC(66edb432) SHA1(b67131b0158a58138380734dd5b9394b70010026) )
	ROM_LOAD32_WORD("epr-18667.14",  0x100002, 0x080000, CRC(b593d31f) SHA1(1e9f23f4052ab1b0275307cc80e51352f13bc319) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18648.11",  0x000000, 0x400000, CRC(2edbe489) SHA1(ded2e4b295be08970d13c387818c570c3afe8109) )
	ROM_LOAD32_WORD("mpr-18649.12",  0x000002, 0x400000, CRC(e68c5aa6) SHA1(cdee1ba9247eda4282442d0522f8de7d7c86e1e6) )
	ROM_LOAD32_WORD("mpr-18650.9",   0x800000, 0x400000, CRC(89a855b9) SHA1(5096db1da1f7e175000e89fca2a1dd3fd53030ea) )
	ROM_LOAD32_WORD("mpr-18651.10",  0x800002, 0x400000, CRC(f4c23107) SHA1(f65984614111b12dd414db80751efe64fcf5ef16) )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("mpr-18662.29",  0x000000, 0x200000, CRC(a33d3335) SHA1(991bbe9dcbef8bfa96682e9d142623fc9b7c0879) )
	ROM_LOAD32_WORD("mpr-18663.30",  0x000002, 0x200000, CRC(ea74a641) SHA1(a684e13c0afe2ef3f3108ae9b73389121368fc4e) )

	ROM_REGION( 0x2000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18654.17",  0x000000, 0x400000, CRC(6a0caf29) SHA1(9f009f44e62ae0f9dec7a34a163bc186d1c4cbbd) )
	ROM_LOAD32_WORD("mpr-18655.21",  0x000002, 0x400000, CRC(a4293e78) SHA1(af512c994bedbdaf3a5eeed607e771dcd87810fc) )
	ROM_LOAD32_WORD("mpr-18656.18",  0x800000, 0x400000, CRC(b4f51e76) SHA1(eb71ada331576f2a7219d238ea07a61bcbf6381a) )
	ROM_LOAD32_WORD("mpr-18657.22",  0x800002, 0x400000, CRC(a9be4674) SHA1(a918c2a3de78a08104480097edfb9d6aeaeda873) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18660.27",  0x000000, 0x200000, CRC(e53663e5) SHA1(0a4908be654bad4f00d7d58f0e42f631996911c9) )
	ROM_LOAD32_WORD("mpr-18658.25",  0x000002, 0x200000, CRC(3d0fcd01) SHA1(c8626c879bfcf7abd095cac5dc03a04ae8629423) )
	ROM_LOAD32_WORD("mpr-18661.28",  0x800000, 0x200000, CRC(52b50410) SHA1(64ea7b2f86745954e0b8a15d71203444705240a2) )
	ROM_LOAD32_WORD("mpr-18659.26",  0x800002, 0x200000, CRC(27aa8ae2) SHA1(e9b756e5b4b1c19e52e47af03c773fee544be420) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD16_WORD_SWAP("epr-18643a.7", 0x000000,  0x20000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18670.31", 0x080000,  0x80000, CRC(3e715f76) SHA1(4fd997e379a8cdb94ec3b1986b3ab443fc6fa12a) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18652.32", 0x000000, 0x400000, CRC(037eee53) SHA1(e592f9e97abe0a7bc9009d8327b93da9bc43749c))
	ROM_LOAD("mpr-18653.34", 0x400000, 0x400000, CRC(9ec3e7bf) SHA1(197bc8adc823e93128c1cebf69361a7c7297f808))
ROM_END

ROM_START( vonj ) /* Virtual On Cyber Troopers Revision B (Japan), Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18664b.15", 0x000000, 0x080000, CRC(27d0172c) SHA1(f3bcae9898c7d656eccb4d2546c9bb93daaefbb7) )
	ROM_LOAD32_WORD("epr-18665b.16", 0x000002, 0x080000, CRC(2f0142ee) SHA1(73f2a19a519ced8e0a1ab5cf69a4bf9d9841e288) )
	ROM_LOAD32_WORD("epr-18666.13",  0x100000, 0x080000, CRC(66edb432) SHA1(b67131b0158a58138380734dd5b9394b70010026) )
	ROM_LOAD32_WORD("epr-18667.14",  0x100002, 0x080000, CRC(b593d31f) SHA1(1e9f23f4052ab1b0275307cc80e51352f13bc319) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18648.11", 0x000000, 0x400000, CRC(2edbe489) SHA1(ded2e4b295be08970d13c387818c570c3afe8109) )
	ROM_LOAD32_WORD("mpr-18649.12", 0x000002, 0x400000, CRC(e68c5aa6) SHA1(cdee1ba9247eda4282442d0522f8de7d7c86e1e6) )
	ROM_LOAD32_WORD("mpr-18650.9",  0x800000, 0x400000, CRC(89a855b9) SHA1(5096db1da1f7e175000e89fca2a1dd3fd53030ea) )
	ROM_LOAD32_WORD("mpr-18651.10", 0x800002, 0x400000, CRC(f4c23107) SHA1(f65984614111b12dd414db80751efe64fcf5ef16) )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("mpr-18662.29", 0x000000, 0x200000, CRC(a33d3335) SHA1(991bbe9dcbef8bfa96682e9d142623fc9b7c0879) )
	ROM_LOAD32_WORD("mpr-18663.30", 0x000002, 0x200000, CRC(ea74a641) SHA1(a684e13c0afe2ef3f3108ae9b73389121368fc4e) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18654.17", 0x000000, 0x400000, CRC(6a0caf29) SHA1(9f009f44e62ae0f9dec7a34a163bc186d1c4cbbd) )
	ROM_LOAD32_WORD("mpr-18655.21", 0x000002, 0x400000, CRC(a4293e78) SHA1(af512c994bedbdaf3a5eeed607e771dcd87810fc) )
	ROM_LOAD32_WORD("mpr-18656.18", 0x800000, 0x400000, CRC(b4f51e76) SHA1(eb71ada331576f2a7219d238ea07a61bcbf6381a) )
	ROM_LOAD32_WORD("mpr-18657.22", 0x800002, 0x400000, CRC(a9be4674) SHA1(a918c2a3de78a08104480097edfb9d6aeaeda873) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18660.27", 0x000000, 0x200000, CRC(e53663e5) SHA1(0a4908be654bad4f00d7d58f0e42f631996911c9) )
	ROM_LOAD32_WORD("mpr-18658.25", 0x000002, 0x200000, CRC(3d0fcd01) SHA1(c8626c879bfcf7abd095cac5dc03a04ae8629423) )
	ROM_LOAD32_WORD("mpr-18661.28", 0x800000, 0x200000, CRC(52b50410) SHA1(64ea7b2f86745954e0b8a15d71203444705240a2) )
	ROM_LOAD32_WORD("mpr-18659.26", 0x800002, 0x200000, CRC(27aa8ae2) SHA1(e9b756e5b4b1c19e52e47af03c773fee544be420) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD16_WORD_SWAP("epr-18643a.7", 0x000000,  0x20000, CRC(b5e048ec) SHA1(8182e05a2ffebd590a936c1359c81e60caa79c2a) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18670.31", 0x080000,  0x80000, CRC(3e715f76) SHA1(4fd997e379a8cdb94ec3b1986b3ab443fc6fa12a) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18652.32", 0x000000, 0x400000, CRC(037eee53) SHA1(e592f9e97abe0a7bc9009d8327b93da9bc43749c) )
	ROM_LOAD("mpr-18653.34", 0x400000, 0x400000, CRC(9ec3e7bf) SHA1(197bc8adc823e93128c1cebf69361a7c7297f808) )
ROM_END

ROM_START( vstriker ) /* Virtua Striker Revision A, Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18068a.15",  0x000000, 0x020000, CRC(afc69b54) SHA1(2127bde1de3cd6663c31cf2126847815234e09a4) )
	ROM_LOAD32_WORD("epr-18069a.16",  0x000002, 0x020000, CRC(0243250c) SHA1(3cbeac09d503a19c5950cf70e3b329f791acfa13) )
	ROM_LOAD32_WORD("epr-18066a.13",  0x040000, 0x020000, CRC(e658b33a) SHA1(33266e6372e73f670688f58e51081ec5a7deec11) )
	ROM_LOAD32_WORD("epr-18067a.14",  0x040002, 0x020000, CRC(49e94047) SHA1(56c8d1a365985886dffeddf24d692ce6b377760a) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18055.11", 0x000000, 0x200000, CRC(5aba9fc0) SHA1(40d45af7e58fa48b6afa85071c2bd1d4b5b5ffa5) )
	ROM_LOAD32_WORD("mpr-18056.12", 0x000002, 0x200000, CRC(017f0c55) SHA1(744e5a02abd82fbeb875c5cd30c5543570140cff) )
	ROM_LOAD32_WORD("mpr-18053.9",  0x400000, 0x200000, CRC(46c770c8) SHA1(000e9edfed49cc3dcc136f80e044dcd2b42378ce) )
	ROM_LOAD32_WORD("mpr-18054.10", 0x400002, 0x200000, CRC(437af66e) SHA1(c5afa62100a93e160aa96b327a260cc7fee51fdc) )
	ROM_LOAD32_WORD("epr-18070a.7", 0x800000, 0x080000, CRC(1961e2fc) SHA1(12ead9b782e092346b7cd5a7343b302f546fe066) )
	ROM_LOAD32_WORD("epr-18071a.8", 0x800002, 0x080000, CRC(b2492dca) SHA1(3b35522ab8e1fdfa327245fef797e3d7c0cceb85) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18057.17", 0x000000, 0x200000, CRC(890d8806) SHA1(fe73e4ea310e13b172e49d39c7eafba8f9052e67) )
	ROM_LOAD32_WORD("mpr-18059.21", 0x000002, 0x200000, CRC(c5cdf534) SHA1(fd127d33bc5a78b81aaa7d5886beca2192a62867) )
	ROM_LOAD32_WORD("mpr-18058.18", 0x400000, 0x200000, CRC(d4cbdf7c) SHA1(fe783c5bc94c2581fd990f0f0a705bdc5c05a386) )
	ROM_LOAD32_WORD("mpr-18060.22", 0x400002, 0x200000, CRC(93d5c95f) SHA1(bca83f024d85c97ca59fae8d9097fc510ec0fc7f) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18062.27", 0x000000, 0x200000, CRC(126e7de3) SHA1(0810364934dee8d5035cef623d01dfbacc64bf2b) )
	ROM_LOAD32_WORD("mpr-18061.25", 0x000002, 0x200000, CRC(c37f1c67) SHA1(c917046c2d98af17c59ceb0ea4f89d215cc0ead8) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18072.31", 0x080000,  0x20000, CRC(73eabb58) SHA1(4f6d70d6e0d7b469c5f2527efb08f208f4aa017e) )

	ROM_REGION( 0x600000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18063.32", 0x000000, 0x200000, CRC(b74d7c8a) SHA1(da0bc8b3822b01087b6f9de0446cab1eb6617e8e) )
	ROM_LOAD("mpr-18064.33", 0x200000, 0x200000, CRC(783b9910) SHA1(108b23bb57e3133c555083aa4f9bc573ac6e3152) )
	ROM_LOAD("mpr-18065.34", 0x400000, 0x200000, CRC(046b55fe) SHA1(2db7eabf4318881a67b10dba24f6f0cd68940ace) )
ROM_END

ROM_START( vstrikero ) /* Virtua Striker, Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18068.15", 0x000000, 0x020000, CRC(74a47795) SHA1(3ba34bd467e11e768eda95ff345f5993fb9d6bca) )
	ROM_LOAD32_WORD("epr-18069.16", 0x000002, 0x020000, CRC(f6c3fcbf) SHA1(84bf16fc2a441cb724f4bc635a4c4209c240cfbf) )
	ROM_LOAD32_WORD("epr-18066.13", 0x040000, 0x020000, CRC(e774229e) SHA1(0ff20aa3e030df869767bb9614565acc9f3fe3b1) )
	ROM_LOAD32_WORD("epr-18067.14", 0x040002, 0x020000, CRC(7dfd950c) SHA1(d5eff8aff37fb0ef3c7f9d8bfca8460213b0f0a7) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18055.11", 0x000000, 0x200000, CRC(5aba9fc0) SHA1(40d45af7e58fa48b6afa85071c2bd1d4b5b5ffa5) )
	ROM_LOAD32_WORD("mpr-18056.12", 0x000002, 0x200000, CRC(017f0c55) SHA1(744e5a02abd82fbeb875c5cd30c5543570140cff) )
	ROM_LOAD32_WORD("mpr-18053.9",  0x400000, 0x200000, CRC(46c770c8) SHA1(000e9edfed49cc3dcc136f80e044dcd2b42378ce) )
	ROM_LOAD32_WORD("mpr-18054.10", 0x400002, 0x200000, CRC(437af66e) SHA1(c5afa62100a93e160aa96b327a260cc7fee51fdc) )
	ROM_LOAD32_WORD("epr-18070.7",  0x800000, 0x080000, CRC(f52e4db5) SHA1(731452284c45329701258ee9fb8b7df6514fbba1) )
	ROM_LOAD32_WORD("epr-18071.8",  0x800002, 0x080000, CRC(1be63a7d) SHA1(c678f1f42de86cc968c3f823994d36c74b2e55fd) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18057.17", 0x000000, 0x200000, CRC(890d8806) SHA1(fe73e4ea310e13b172e49d39c7eafba8f9052e67) )
	ROM_LOAD32_WORD("mpr-18059.21", 0x000002, 0x200000, CRC(c5cdf534) SHA1(fd127d33bc5a78b81aaa7d5886beca2192a62867) )
	ROM_LOAD32_WORD("mpr-18058.18", 0x400000, 0x200000, CRC(d4cbdf7c) SHA1(fe783c5bc94c2581fd990f0f0a705bdc5c05a386) )
	ROM_LOAD32_WORD("mpr-18060.22", 0x400002, 0x200000, CRC(93d5c95f) SHA1(bca83f024d85c97ca59fae8d9097fc510ec0fc7f) )

	ROM_REGION( 0x400000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18062.27", 0x000000, 0x200000, CRC(126e7de3) SHA1(0810364934dee8d5035cef623d01dfbacc64bf2b) )
	ROM_LOAD32_WORD("mpr-18061.25", 0x000002, 0x200000, CRC(c37f1c67) SHA1(c917046c2d98af17c59ceb0ea4f89d215cc0ead8) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18072.31", 0x080000,  0x20000, CRC(73eabb58) SHA1(4f6d70d6e0d7b469c5f2527efb08f208f4aa017e) )

	ROM_REGION( 0x600000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18063.32", 0x000000, 0x200000, CRC(b74d7c8a) SHA1(da0bc8b3822b01087b6f9de0446cab1eb6617e8e) )
	ROM_LOAD("mpr-18064.33", 0x200000, 0x200000, CRC(783b9910) SHA1(108b23bb57e3133c555083aa4f9bc573ac6e3152) )
	ROM_LOAD("mpr-18065.34", 0x400000, 0x200000, CRC(046b55fe) SHA1(2db7eabf4318881a67b10dba24f6f0cd68940ace) )
ROM_END

ROM_START( dynabb ) /* Dynamite Baseball, Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19170.15", 0x000000, 0x080000, CRC(e00eb49e) SHA1(20975d892cf1c9f50605238d6ab41d79ece39f69) )
	ROM_LOAD32_WORD("epr-19171.16", 0x000002, 0x080000, CRC(9878d67d) SHA1(d3350546b7e0e6fe8bb2f9d1a91475655f931b8b) )
	ROM_LOAD32_WORD("epr-19168.13", 0x100000, 0x080000, CRC(041da66b) SHA1(4a58153baf5f0b34e054bf23e519edcf364a9336) )
	ROM_LOAD32_WORD("epr-19169.14", 0x100002, 0x080000, CRC(91a5acef) SHA1(2520a3e4ff15e4d583861ba656570abca5f7c611) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19178.11", 0x0000000, 0x400000, CRC(0d621e21) SHA1(31adc229258a5d468ff80d789c59bd8a6777f900) )
	ROM_LOAD32_WORD("mpr-19179.12", 0x0000002, 0x400000, CRC(337a4ec2) SHA1(77d7d186344715237895ac1ed0ab219fcc340a7e) )
	ROM_LOAD32_WORD("mpr-19176.9",  0x0800000, 0x400000, CRC(2c4e90f5) SHA1(8d5ed0b26e79dd6476282bc69cb27b42381635f2) )
	ROM_LOAD32_WORD("mpr-19177.10", 0x0800002, 0x400000, CRC(b0f1e512) SHA1(81e4124ac7766c7ea6bac7e7f4db110783394ae3) )
	ROM_LOAD32_WORD("mpr-19174.7",  0x1000000, 0x400000, CRC(057e5200) SHA1(dd07eb438d91a8132789154a633fb6ec4e2ef0d1) )
	ROM_LOAD32_WORD("mpr-19175.8",  0x1000002, 0x400000, CRC(85254156) SHA1(aae9531980d1b394d86e285c00c7384601875470) )
	ROM_LOAD32_WORD("mpr-19172.5",  0x1800000, 0x400000, CRC(9214aaaf) SHA1(769ad943ca90f0f3cc81f00e7a8cca95c660d266) )
	ROM_LOAD32_WORD("mpr-19173.6",  0x1800002, 0x400000, CRC(31adbeed) SHA1(3984be892f0dce21c8d423dda055ef7e57df4d4e) )

	ROM_REGION( 0x800000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19180.17", 0x000000, 0x400000, CRC(d2e311a5) SHA1(83fb31c6ad7c32f1a7bcf870edb2719653c3db97) )
	ROM_LOAD32_WORD("mpr-19181.21", 0x000002, 0x400000, BAD_DUMP CRC(09a86c33) SHA1(30601c5b00fa3c9db815f60a0de16576e34b8c42) ) // BAD_DUMP: 2nd half empty

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19183.27", 0x000000, 0x400000, CRC(5e29074b) SHA1(f4dfa396653aeb649ec170c9584ea1a74377929a) )
	ROM_LOAD32_WORD("mpr-19182.25", 0x000002, 0x400000, CRC(c899923d) SHA1(15cc86c885329227d3c19e9837363eaf6c38829b) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19184.31", 0x080000,  0x80000, CRC(c013a163) SHA1(c564df8295e3c19082ead0eb22478dc651e0b430) )

	ROM_REGION( 0x600000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19185.32", 0x000000, 0x200000, CRC(5175b7d8) SHA1(bed43db286703e95cc8025013b2d129598faab3c) )
	ROM_LOAD("mpr-19186.33", 0x200000, 0x200000, CRC(f23440b5) SHA1(9bb862d61ed079cb3eb0bd7a37b19c6134859b99) )
	ROM_LOAD("mpr-19187.34", 0x400000, 0x200000, CRC(20918769) SHA1(90951bd61654d39537c54325b6e157a019edcda8) )
ROM_END

ROM_START( dynabb97 ) /* Dynamite Baseball 97 Revision A, Model 2B. Sega game ID# 833-12803 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-19833a.15", 0x000000, 0x080000, CRC(d99ed1b2) SHA1(b04613d564c04c35feafccad56ed85810d894185) )
	ROM_LOAD32_WORD("epr-19834a.16", 0x000002, 0x080000, CRC(24192bb1) SHA1(c535ab4b38ffd42f03eed6a5a1706e867eaccd67) )
	ROM_LOAD32_WORD("epr-19831a.13", 0x100000, 0x080000, CRC(0527ea40) SHA1(8e80e2627aafe395d8ced4a97ba50cd9a781fb45) )
	ROM_LOAD32_WORD("epr-19832a.14", 0x100002, 0x080000, CRC(2f380a40) SHA1(d770dfd70aa14dcc716aa47e6cbf26f32649f294) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-19841.11", 0x0000000, 0x400000, CRC(989309af) SHA1(d527f46865d00a91d5b38a93dc38baf62f372cb1) )
	ROM_LOAD32_WORD("mpr-19842.12", 0x0000002, 0x400000, CRC(eec54070) SHA1(29ed4a005b52f6e16492998183ec4e5f7475022b) )
	ROM_LOAD32_WORD("mpr-19839.9",  0x0800000, 0x400000, CRC(d5a74cf4) SHA1(ddea9cfc0a14461448acae2eed2092829ef3b418) )
	ROM_LOAD32_WORD("mpr-19840.10", 0x0800002, 0x400000, CRC(45704e95) SHA1(2a325ee39f9d719399040ed2a41123bcf0c6f385) )
	ROM_LOAD32_WORD("mpr-19837.7",  0x1000000, 0x400000, CRC(c02187d9) SHA1(1da108a2ec00e3fc472b1a819655aff8c679051d) )
	ROM_LOAD32_WORD("mpr-19838.8",  0x1000002, 0x400000, CRC(546b61cd) SHA1(0cc0edd0a9c288143168d63a7d48d0fbfa64d8bf) )
	ROM_LOAD32_WORD("mpr-19835.5",  0x1800000, 0x400000, CRC(a3b0a37c) SHA1(dcde1946008ab86c7fca212ec57c1cc468f30c58) )
	ROM_LOAD32_WORD("mpr-19836.6",  0x1800002, 0x400000, CRC(d70a32aa) SHA1(fd56bb284eb66e6c078b386a0db1c2b10dc1dd4a) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-19843.17", 0x000000, 0x400000, CRC(019bc583) SHA1(8889a9438d8f3ea50058372ad03ebd4653f23313) )
	ROM_LOAD32_WORD("mpr-19845.21", 0x000002, 0x400000, CRC(2d23e73a) SHA1(63e5859518172f88a5ba98b69309d4162c233cf0) )
	ROM_LOAD32_WORD("mpr-19844.18", 0x800000, 0x400000, CRC(150198d6) SHA1(3ea5c3e41eb95e715860619f771bc580c91b095f) )
	ROM_LOAD32_WORD("mpr-19846.22", 0x800002, 0x400000, CRC(fe53cd17) SHA1(58eab07976972917c345a8d3a50ff1e96e5fa798) )

	ROM_REGION( 0x800000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-19848.27", 0x000000, 0x400000, CRC(4c0526b7) SHA1(e8db7125be8a052e41a00c69cc08ca0d75b3b96f) )
	ROM_LOAD32_WORD("mpr-19847.25", 0x000002, 0x400000, CRC(fe55edbd) SHA1(b0b6135b23349d7d6ae007002d8df83748cab7b1) )

	ROM_REGION( 0x800000, "user5", ROMREGION_ERASE00 ) // Coprocessor Data ROM

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-19849.31", 0x080000,  0x80000, CRC(b0d5bff0) SHA1(1fb824adaf3ed330a8039be726a87eb85c00abd7) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-19850.32", 0x000000, 0x200000, CRC(e1fd27bf) SHA1(a7189ad398138a91f96b192cb7c112c0301dcda4) )
	ROM_LOAD("mpr-19851.33", 0x200000, 0x200000, CRC(dc644077) SHA1(8765bdb1d471dbeea065a97ae131f2d8f78aa13d) )
	ROM_LOAD("mpr-19852.34", 0x400000, 0x200000, CRC(cfda4efd) SHA1(14d55f127da6673c538c2ef9be34a4e02ca449f3) )
	ROM_LOAD("mpr-19853.35", 0x600000, 0x200000, CRC(cfc64857) SHA1(cf51fafb3d45bf799b9ccb407bee862e15c95981) )
ROM_END

ROM_START( fvipers ) /* Fighting Vipers Revision D, Model 2B */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-18606d.15", 0x000000, 0x020000, CRC(7334de7d) SHA1(d10355198a3f62b503701f44dc49bfe018c787d1) )
	ROM_LOAD32_WORD("epr-18607d.16", 0x000002, 0x020000, CRC(700d2ade) SHA1(656e25a6389f04f7fb9099f0b41fb03fa645a2f0) )
	ROM_LOAD32_WORD("epr-18604d.13", 0x040000, 0x020000, CRC(704fdfcf) SHA1(52b6ae90231d40a3ece133debaeb210fc36c6fcb) )
	ROM_LOAD32_WORD("epr-18605d.14", 0x040002, 0x020000, CRC(7dddf81f) SHA1(3e0da0eaf1f98dbbd4ca5f78c04052b347b234b2) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-18614.11",    0x0000000, 0x400000, CRC(0ebc899f) SHA1(49c80b11b207cba4ec10fbb7cc140f3a5b039e82) )
	ROM_LOAD32_WORD("mpr-18615.12",    0x0000002, 0x400000, CRC(018abdb7) SHA1(59e5b6378404e10ace4f3675428d61d3ae9d1963) )
	ROM_LOAD32_WORD("mpr-18612.9",     0x0800000, 0x400000, CRC(1f174cd1) SHA1(89b56dd2f350edd093dc06f4cc258652c26b1d45) )
	ROM_LOAD32_WORD("mpr-18613.10",    0x0800002, 0x400000, CRC(f057cdf2) SHA1(e16d5de2a00670aba4fbe0dc88ccf317de9842be) )
	ROM_LOAD32_WORD("epr-18610d.7",    0x1000000, 0x080000, CRC(a1871703) SHA1(8d7b362a8fd9d63f5cea2f3fab97e5fe3fa30d87) )
	ROM_LOAD32_WORD("epr-18611d.8",    0x1000002, 0x080000, CRC(39a75fee) SHA1(c962805f03e2503dd1671ba3e906c6e306a92e48) )
	ROM_COPY( "user1", 0x1000000, 0x1100000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x1000000, 0x1200000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1300000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1400000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1500000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1600000, 0x100000 )
	ROM_COPY( "user1", 0x1000000, 0x1700000, 0x100000 )
	ROM_LOAD32_WORD("epr-18608d.5",    0x1800000, 0x080000, CRC(5bc11881) SHA1(97ce5faf9719cb02dd3a15d47245cc4634f08fcb) )
	ROM_LOAD32_WORD("epr-18609d.6",    0x1800002, 0x080000, CRC(cd426035) SHA1(94c85a656c86bc4880db6bff2ef795ec30f62f39) )
	ROM_COPY( "user1", 0x1800000, 0x1900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x1800000, 0x1a00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1b00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1c00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1d00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1e00000, 0x100000 )
	ROM_COPY( "user1", 0x1800000, 0x1f00000, 0x100000 )

	ROM_REGION( 0x800000, "user5", 0 ) // Coprocessor Data ROM
	ROM_LOAD32_WORD("mpr-18622.29", 0x000000, 0x200000, CRC(c74d99e3) SHA1(9914be9925b86af6af670745b5eba3a9e4f24af9) )
	ROM_LOAD32_WORD("mpr-18623.30", 0x000002, 0x200000, CRC(746ae931) SHA1(a6f0f589ad174a34493ee24dc0cb509ead3aed70) )

	ROM_REGION( 0xc00000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-18616.17", 0x000000, 0x200000, CRC(15a239be) SHA1(1a33c48f99eed20da4b622219d21ec5995acc9aa) )
	ROM_LOAD32_WORD("mpr-18619.21", 0x000002, 0x200000, CRC(9d5e8e2b) SHA1(f79ae0a7b966ddb0948b464d233845d4f362a2e7) )
	ROM_LOAD32_WORD("mpr-18617.18", 0x400000, 0x200000, CRC(a62cab7d) SHA1(f20a545148f2a1d6f4f1c897f1ed82ad17429dce) )
	ROM_LOAD32_WORD("mpr-18620.22", 0x400002, 0x200000, CRC(4d432afd) SHA1(30a1ef1e309a163b2d8756810fc33debf069141c) )
	ROM_LOAD32_WORD("mpr-18618.19", 0x800000, 0x200000, CRC(adab589f) SHA1(67818ec4185da17f1549fb3a125cade267a46a48) )
	ROM_LOAD32_WORD("mpr-18621.23", 0x800002, 0x200000, CRC(f5eeaa95) SHA1(38d7019afcef6dbe292354d717fd49da511cbc2b) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-18626.27", 0x000000, 0x200000, CRC(9df0a961) SHA1(d8fb4bbbdc00303330047be380a79da7838d4fd5) )
	ROM_LOAD32_WORD("mpr-18624.25", 0x000002, 0x200000, CRC(1d74433e) SHA1(5b6d2d17609ae741546d99d40f575bb24d62b5d3) )
	ROM_LOAD32_WORD("mpr-18627.28", 0x800000, 0x200000, CRC(946175a0) SHA1(8b6e5e1342f98c9c6f2f7d61e843275d244f331a) )
	ROM_LOAD32_WORD("mpr-18625.26", 0x800002, 0x200000, CRC(182fd572) SHA1(b09a682eff7e835ff8c33aaece12f3727a91dd5e) )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Sound program
	ROM_LOAD16_WORD_SWAP("epr-18628.31", 0x080000,  0x80000, CRC(aa7dd79f) SHA1(d8bd1485273652d7c2a303bbdcdf607d3b530283) )

	ROM_REGION( 0x800000, "scsp", 0 ) // Samples
	ROM_LOAD("mpr-18629.32", 0x000000, 0x200000, CRC(5d0006cc) SHA1(f6d2552ffc5473836aafb06735b62f65ef8f5ef5) )
	ROM_LOAD("mpr-18630.33", 0x200000, 0x200000, CRC(9d405615) SHA1(7e7ffbb4ec080a0815c6ca49b9d8efe1f676203b) )
	ROM_LOAD("mpr-18631.34", 0x400000, 0x200000, CRC(9dae5b45) SHA1(055ac989eafb81749326520d0be264f7a984c627) )
	ROM_LOAD("mpr-18632.35", 0x600000, 0x200000, CRC(39da6805) SHA1(9e9523b7c2bc50f869d062f80955da1281951299) )
ROM_END

ROM_START( daytona ) /* Daytona USA (Japan, Revision A), Original Model 2 w/Model 1 sound board */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-16722a.12", 0x000000, 0x020000, CRC(48b94318) SHA1(a476a9a3531beef760c88c9634ed4a7d270e8ee7) )
	ROM_LOAD32_WORD("epr-16723a.13", 0x000002, 0x020000, CRC(8af8b32d) SHA1(2039ec1f8da524176fcf85473c10a8b6e49e139a) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16528.10",   0x000000, 0x200000, CRC(9ce591f6) SHA1(e22fc8a70b533f7a6191f5952c581fb8f9627906) )
	ROM_LOAD32_WORD("mpr-16529.11",   0x000002, 0x200000, CRC(f7095eaf) SHA1(da3c922f950dd730ea348eae12aa1cb69cee9a58) )
	ROM_LOAD32_WORD("mpr-16808.8",    0x400000, 0x200000, CRC(44f1f5a0) SHA1(343866a6e2187a8ebc17f6727080f9f2f9ac9200) )
	ROM_LOAD32_WORD("mpr-16809.9",    0x400002, 0x200000, CRC(37a2dd12) SHA1(8192d8698d6bd52ee11cc28917aff5840c447627) )
	ROM_LOAD32_WORD("epr-16724a.6",   0x800000, 0x080000, CRC(469f10fd) SHA1(7fad3b8d03960e5e1f7a6cb36509238977e00fcc) )
	ROM_LOAD32_WORD("epr-16725a.7",   0x800002, 0x080000, CRC(ba0df8db) SHA1(d0c5581c56500b5266cab8e8151db24fcbdea0d7) )
	ROM_COPY( "user1", 0x800000, 0x900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x800000, 0xa00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xb00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xc00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("mpr-16537.ic28", 0x000000, 0x200000, CRC(36b7c35a) SHA1(b32fd1d3fc8983fb5f2a7b236b665a8c9b52769f) )
	ROM_LOAD32_WORD("mpr-16536.ic29", 0x000002, 0x200000, CRC(6d6afed9) SHA1(2018468d7d849854b3d0cfbcd217317e2fc93555) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16523.ic16", 0x000000, 0x200000, CRC(2f484d42) SHA1(0b83a3fc92b7d913a14cfb01d688c63555c17c41) )
	ROM_LOAD32_WORD("mpr-16518.ic20", 0x000002, 0x200000, CRC(df683bf7) SHA1(16afe5029591f3536b5b75d9cf50a34d0ea72c3d) )
	ROM_LOAD32_WORD("mpr-16524.ic17", 0x400000, 0x200000, CRC(34658bd7) SHA1(71b47626ffe5b26d1140afe1b830a9a2be86c88f) )
	ROM_LOAD32_WORD("mpr-16519.ic21", 0x400002, 0x200000, CRC(facd1c81) SHA1(dac8c281a5e9a6c4b60197e6676f3727264ee420) )
	ROM_LOAD32_WORD("mpr-16525.ic18", 0x800000, 0x200000, CRC(fb517521) SHA1(33f5f37ea2e09fc73eed5388b46fdf1fa9e285e6) )
	ROM_LOAD32_WORD("mpr-16520.ic22", 0x800002, 0x200000, CRC(d66bd9bd) SHA1(660171674484375a27595630e5e2d2ad76a06d1a) )
	ROM_LOAD32_WORD("mpr-16772.ic19", 0xc00000, 0x200000, CRC(770ed912) SHA1(1789f35dd403f73f8be18495a0fe4ad1e6841417) )
	ROM_LOAD32_WORD("mpr-16771.ic23", 0xc00002, 0x200000, CRC(a2205124) SHA1(257a3675e4ef6adbf61285a5daa5954223c28cb2) )

	ROM_REGION16_LE( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16522.25", 0x000000, 0x200000, CRC(55d39a57) SHA1(abf7b0fc0f111f90da42463d600db9fa32e95efe) )
	ROM_LOAD32_WORD("mpr-16521.24", 0x000002, 0x200000, CRC(af1934fb) SHA1(a6a21a23cd34d0de6d3e6a5c3c2687f905d0dc2a) )
	ROM_LOAD32_WORD("mpr-16770.27", 0x800000, 0x200000, CRC(f9fa7bfb) SHA1(8aa933b74d4e05dc49987238705e50b00e5dae73) )
	ROM_LOAD32_WORD("mpr-16769.26", 0x800002, 0x200000, CRC(e57429e9) SHA1(8c712ab09e61ef510741a55f29b3c4e497471372) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16720.7", 0x000000, 0x020000, CRC(8e73cffd) SHA1(9933ccc0757e8c86e0adb938d1c89210b26841ea) )
	ROM_LOAD16_WORD_SWAP("epr-16721.8", 0x020000, 0x020000, CRC(1bb3b7b7) SHA1(ee2fd1480e535fc37e9932e6fe4e31344559fc87) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16491.32", 0x000000, 0x200000, CRC(89920903) SHA1(06d1d55470ae99f8de0f8c88c694f34c4eb13668) )
	ROM_LOAD("mpr-16492.33", 0x200000, 0x200000, CRC(459e701b) SHA1(2054f69cecad677eb00c6a3051f5b5d90885e19b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16493.4", 0x000000, 0x200000, CRC(9990db15) SHA1(ea9a8b45a07dccaae62be7cf095532ce7596a70c) )
	ROM_LOAD("mpr-16494.5", 0x200000, 0x200000, CRC(600e1d6c) SHA1(d4e246fc57a16ff562bbcbccf6a739b706f58696) )

	MODEL2_CPU_BOARD /* Model 2 CPU board extra roms */

	ROM_REGION( 0x10000, "user7", 0 ) // Unknown ROM
	ROM_LOAD("epr-14869c.25", 0x000000, 0x010000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371) )
ROM_END

ROM_START( daytonase ) /* Daytona USA (Japan, Revision A), Original Model 2 w/Model 1 sound board */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-17369a.12", 0x000000, 0x020000, CRC(3bc6ca62) SHA1(16e9fd25670ce4eda378df402066e3d9652210b1) )
	ROM_LOAD32_WORD("epr-17370a.13", 0x000002, 0x020000, CRC(5d1c74e4) SHA1(26eff5a07f6906e1ad20cd264ce6e25a9068ea2b) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16528.10",   0x000000, 0x200000, CRC(9ce591f6) SHA1(e22fc8a70b533f7a6191f5952c581fb8f9627906) )
	ROM_LOAD32_WORD("mpr-16529.11",   0x000002, 0x200000, CRC(f7095eaf) SHA1(da3c922f950dd730ea348eae12aa1cb69cee9a58) )
	ROM_LOAD32_WORD("mpr-16808.8",    0x400000, 0x200000, CRC(44f1f5a0) SHA1(343866a6e2187a8ebc17f6727080f9f2f9ac9200) )
	ROM_LOAD32_WORD("mpr-16809.9",    0x400002, 0x200000, CRC(37a2dd12) SHA1(8192d8698d6bd52ee11cc28917aff5840c447627) )
	ROM_LOAD32_WORD("epr-17371.6",    0x800000, 0x080000, CRC(7478f0d2) SHA1(412d4db62436746da8d0d55ccf2016d14c05153c) )
	ROM_LOAD32_WORD("epr-17372.7",    0x800002, 0x080000, CRC(308a06a9) SHA1(0c7502c2fe5a64db7e6020457b9f8e47f2c9af0e) )
	ROM_COPY( "user1", 0x800000, 0x900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x800000, 0xa00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xb00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xc00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("mpr-16537.ic28", 0x000000, 0x200000, CRC(36b7c35a) SHA1(b32fd1d3fc8983fb5f2a7b236b665a8c9b52769f) )
	ROM_LOAD32_WORD("mpr-16536.ic29", 0x000002, 0x200000, CRC(6d6afed9) SHA1(2018468d7d849854b3d0cfbcd217317e2fc93555) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16523.ic16", 0x000000, 0x200000, CRC(2f484d42) SHA1(0b83a3fc92b7d913a14cfb01d688c63555c17c41) )
	ROM_LOAD32_WORD("mpr-16518.ic20", 0x000002, 0x200000, CRC(df683bf7) SHA1(16afe5029591f3536b5b75d9cf50a34d0ea72c3d) )
	ROM_LOAD32_WORD("mpr-16524.ic17", 0x400000, 0x200000, CRC(34658bd7) SHA1(71b47626ffe5b26d1140afe1b830a9a2be86c88f) )
	ROM_LOAD32_WORD("mpr-16519.ic21", 0x400002, 0x200000, CRC(facd1c81) SHA1(dac8c281a5e9a6c4b60197e6676f3727264ee420) )
	ROM_LOAD32_WORD("mpr-16525.ic18", 0x800000, 0x200000, CRC(fb517521) SHA1(33f5f37ea2e09fc73eed5388b46fdf1fa9e285e6) )
	ROM_LOAD32_WORD("mpr-16520.ic22", 0x800002, 0x200000, CRC(d66bd9bd) SHA1(660171674484375a27595630e5e2d2ad76a06d1a) )
	ROM_LOAD32_WORD("mpr-16772.ic19", 0xc00000, 0x200000, CRC(770ed912) SHA1(1789f35dd403f73f8be18495a0fe4ad1e6841417) )
	ROM_LOAD32_WORD("mpr-16771.ic23", 0xc00002, 0x200000, CRC(a2205124) SHA1(257a3675e4ef6adbf61285a5daa5954223c28cb2) )

	ROM_REGION16_LE( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16522.25", 0x000000, 0x200000, CRC(55d39a57) SHA1(abf7b0fc0f111f90da42463d600db9fa32e95efe) )
	ROM_LOAD32_WORD("mpr-16521.24", 0x000002, 0x200000, CRC(af1934fb) SHA1(a6a21a23cd34d0de6d3e6a5c3c2687f905d0dc2a) )
	ROM_LOAD32_WORD("mpr-16770.27", 0x800000, 0x200000, CRC(f9fa7bfb) SHA1(8aa933b74d4e05dc49987238705e50b00e5dae73) )
	ROM_LOAD32_WORD("mpr-16769.26", 0x800002, 0x200000, CRC(e57429e9) SHA1(8c712ab09e61ef510741a55f29b3c4e497471372) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16720.7", 0x000000, 0x020000, CRC(8e73cffd) SHA1(9933ccc0757e8c86e0adb938d1c89210b26841ea) )
	ROM_LOAD16_WORD_SWAP("epr-16721.8", 0x020000, 0x020000, CRC(1bb3b7b7) SHA1(ee2fd1480e535fc37e9932e6fe4e31344559fc87) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16491.32", 0x000000, 0x200000, CRC(89920903) SHA1(06d1d55470ae99f8de0f8c88c694f34c4eb13668) )
	ROM_LOAD("mpr-16492.33", 0x200000, 0x200000, CRC(459e701b) SHA1(2054f69cecad677eb00c6a3051f5b5d90885e19b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16493.4", 0x000000, 0x200000, CRC(9990db15) SHA1(ea9a8b45a07dccaae62be7cf095532ce7596a70c) )
	ROM_LOAD("mpr-16494.5", 0x200000, 0x200000, CRC(600e1d6c) SHA1(d4e246fc57a16ff562bbcbccf6a739b706f58696) )

	MODEL2_CPU_BOARD /* Model 2 CPU board extra roms */

	ROM_REGION( 0x10000, "user7", 0 ) // Unknown ROM
	ROM_LOAD("epr-14869c.25", 0x000000, 0x010000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371) )
ROM_END

ROM_START( daytona93 ) /* Daytona USA Deluxe '93 version (There is said to be a Deluxe '94 edition) */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-16530a.12", 0x000000, 0x020000, CRC(39e962b5) SHA1(b98a1faabb4f1eff707a94c32224c7820f259874) )
	ROM_LOAD32_WORD("epr-16531a.13", 0x000002, 0x020000, CRC(693126eb) SHA1(779734ba536db67e14760d52e8d8d7db07816481) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16528.10",   0x000000, 0x200000, CRC(9ce591f6) SHA1(e22fc8a70b533f7a6191f5952c581fb8f9627906) )
	ROM_LOAD32_WORD("mpr-16529.11",   0x000002, 0x200000, CRC(f7095eaf) SHA1(da3c922f950dd730ea348eae12aa1cb69cee9a58) )
	ROM_LOAD32_WORD("mpr-16526.8",    0x400000, 0x200000, CRC(5273b8b5) SHA1(f505910394d41a9ffecfdea7b45ef25b21469b7a) )
	ROM_LOAD32_WORD("mpr-16527.9",    0x400002, 0x200000, CRC(fc4cb0ef) SHA1(1bf3aec88ef9fb40bde054f5f0b884bf715cbcc8) )
	ROM_LOAD32_WORD("epr-16534a.6",   0x800000, 0x100000, CRC(1bb0d72d) SHA1(814004e3426b5638e9c8b226594f4f2a9138ffed) )
	ROM_LOAD32_WORD("epr-16535a.7",   0x800002, 0x100000, CRC(459a8bfb) SHA1(607bc0f6c478c3d83ce81f34b7f69997361f906f) )
	ROM_COPY( "user1", 0x900000, 0xa00000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x900000, 0xb00000, 0x100000 )
	ROM_COPY( "user1", 0x900000, 0xc00000, 0x100000 )
	ROM_COPY( "user1", 0x900000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0x900000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0x900000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("mpr-16537.ic28", 0x000000, 0x200000, CRC(36b7c35a) SHA1(b32fd1d3fc8983fb5f2a7b236b665a8c9b52769f) )
	ROM_LOAD32_WORD("mpr-16536.ic29", 0x000002, 0x200000, CRC(6d6afed9) SHA1(2018468d7d849854b3d0cfbcd217317e2fc93555) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16523.ic16", 0x000000, 0x200000, CRC(2f484d42) SHA1(0b83a3fc92b7d913a14cfb01d688c63555c17c41) )
	ROM_LOAD32_WORD("mpr-16518.ic20", 0x000002, 0x200000, CRC(df683bf7) SHA1(16afe5029591f3536b5b75d9cf50a34d0ea72c3d) )
	ROM_LOAD32_WORD("mpr-16524.ic17", 0x400000, 0x200000, CRC(34658bd7) SHA1(71b47626ffe5b26d1140afe1b830a9a2be86c88f) )
	ROM_LOAD32_WORD("mpr-16519.ic21", 0x400002, 0x200000, CRC(facd1c81) SHA1(dac8c281a5e9a6c4b60197e6676f3727264ee420) )
	ROM_LOAD32_WORD("mpr-16525.ic18", 0x800000, 0x200000, CRC(fb517521) SHA1(33f5f37ea2e09fc73eed5388b46fdf1fa9e285e6) )
	ROM_LOAD32_WORD("mpr-16520.ic22", 0x800002, 0x200000, CRC(d66bd9bd) SHA1(660171674484375a27595630e5e2d2ad76a06d1a) )
	ROM_LOAD32_WORD("epr-16646.ic19", 0xc00000, 0x080000, CRC(7ba9fd6b) SHA1(6bcae009e8264bf038fe5d4bc436ec4fc1674831) )
	ROM_LOAD32_WORD("epr-16645.ic23", 0xc00002, 0x080000, CRC(78fe0b8a) SHA1(488fd6c0246752b7d9c25d7ba6cdc5b3911d1836) )

	ROM_REGION16_LE( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16522.25", 0x000000, 0x200000, CRC(55d39a57) SHA1(abf7b0fc0f111f90da42463d600db9fa32e95efe) )
	ROM_LOAD32_WORD("mpr-16521.24", 0x000002, 0x200000, CRC(af1934fb) SHA1(a6a21a23cd34d0de6d3e6a5c3c2687f905d0dc2a) )
	ROM_LOAD32_WORD("mpr-16517.27", 0x800000, 0x200000, CRC(4705d3dd) SHA1(99be9c5d9d99f7016199ffa8404fc471d09e360d) )
	ROM_LOAD32_WORD("mpr-16516.26", 0x800002, 0x200000, CRC(a260d45d) SHA1(a2ed7c586dfcb7980190b7057fa5366239035fe8) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16720.7", 0x000000, 0x020000, CRC(8e73cffd) SHA1(9933ccc0757e8c86e0adb938d1c89210b26841ea) )
	ROM_LOAD16_WORD_SWAP("epr-16721.8", 0x020000, 0x020000, CRC(1bb3b7b7) SHA1(ee2fd1480e535fc37e9932e6fe4e31344559fc87) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16491.32", 0x000000, 0x200000, CRC(89920903) SHA1(06d1d55470ae99f8de0f8c88c694f34c4eb13668) )
	ROM_LOAD("mpr-16492.33", 0x200000, 0x200000, CRC(459e701b) SHA1(2054f69cecad677eb00c6a3051f5b5d90885e19b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16493.4", 0x000000, 0x200000, CRC(9990db15) SHA1(ea9a8b45a07dccaae62be7cf095532ce7596a70c) )
	ROM_LOAD("mpr-16494.5", 0x200000, 0x200000, CRC(600e1d6c) SHA1(d4e246fc57a16ff562bbcbccf6a739b706f58696) )

	MODEL2_CPU_BOARD /* Model 2 CPU board extra roms */

	ROM_REGION( 0x10000, "user7", 0 ) // Unknown ROM
	ROM_LOAD("epr-14869c.25", 0x000000, 0x010000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371) )
ROM_END

ROM_START( daytonas ) /* Daytona USA (With Saturn Adverts) */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-17965.ic12", 0x000000, 0x020000, CRC(f022b3da) SHA1(3c337d12f4e12141b412a7289df46f44c66964b2) )
	ROM_LOAD32_WORD("epr-17966.ic13", 0x000002, 0x020000, CRC(f9e4ece5) SHA1(2df03455a00ae7066c30bace5c2b81581529e6f4) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16528.10",   0x000000, 0x200000, CRC(9ce591f6) SHA1(e22fc8a70b533f7a6191f5952c581fb8f9627906) )
	ROM_LOAD32_WORD("mpr-16529.11",   0x000002, 0x200000, CRC(f7095eaf) SHA1(da3c922f950dd730ea348eae12aa1cb69cee9a58) )
	ROM_LOAD32_WORD("mpr-16808.8",    0x400000, 0x200000, CRC(44f1f5a0) SHA1(343866a6e2187a8ebc17f6727080f9f2f9ac9200) )
	ROM_LOAD32_WORD("mpr-16809.9",    0x400002, 0x200000, CRC(37a2dd12) SHA1(8192d8698d6bd52ee11cc28917aff5840c447627) )
	ROM_LOAD32_WORD("epr-17967.ic6",  0x800000, 0x080000, CRC(a94d8690) SHA1(a716646be6be0b87a550cb88b40e62a5c203ffdf) )
	ROM_LOAD32_WORD("epr-17968.ic7",  0x800002, 0x080000, CRC(9d5a92c6) SHA1(9eb43314f3dc6acbbe0aa991d7a5fa44afe9cdd0) )
	ROM_COPY( "user1", 0x800000, 0x900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x800000, 0xa00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xb00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xc00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("mpr-16537.ic28", 0x000000, 0x200000, CRC(36b7c35a) SHA1(b32fd1d3fc8983fb5f2a7b236b665a8c9b52769f) )
	ROM_LOAD32_WORD("mpr-16536.ic29", 0x000002, 0x200000, CRC(6d6afed9) SHA1(2018468d7d849854b3d0cfbcd217317e2fc93555) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16523.ic16", 0x000000, 0x200000, CRC(2f484d42) SHA1(0b83a3fc92b7d913a14cfb01d688c63555c17c41) )
	ROM_LOAD32_WORD("mpr-16518.ic20", 0x000002, 0x200000, CRC(df683bf7) SHA1(16afe5029591f3536b5b75d9cf50a34d0ea72c3d) )
	ROM_LOAD32_WORD("mpr-16524.ic17", 0x400000, 0x200000, CRC(34658bd7) SHA1(71b47626ffe5b26d1140afe1b830a9a2be86c88f) )
	ROM_LOAD32_WORD("mpr-16519.ic21", 0x400002, 0x200000, CRC(facd1c81) SHA1(dac8c281a5e9a6c4b60197e6676f3727264ee420) )
	ROM_LOAD32_WORD("mpr-16525.ic18", 0x800000, 0x200000, CRC(fb517521) SHA1(33f5f37ea2e09fc73eed5388b46fdf1fa9e285e6) )
	ROM_LOAD32_WORD("mpr-16520.ic22", 0x800002, 0x200000, CRC(d66bd9bd) SHA1(660171674484375a27595630e5e2d2ad76a06d1a) )
	ROM_LOAD32_WORD("mpr-16772.ic19", 0xc00000, 0x200000, CRC(770ed912) SHA1(1789f35dd403f73f8be18495a0fe4ad1e6841417) )
	ROM_LOAD32_WORD("mpr-16771.ic23", 0xc00002, 0x200000, CRC(a2205124) SHA1(257a3675e4ef6adbf61285a5daa5954223c28cb2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16522.25", 0x000000, 0x200000, CRC(55d39a57) SHA1(abf7b0fc0f111f90da42463d600db9fa32e95efe) )
	ROM_LOAD32_WORD("mpr-16521.24", 0x000002, 0x200000, CRC(af1934fb) SHA1(a6a21a23cd34d0de6d3e6a5c3c2687f905d0dc2a) )
	ROM_LOAD32_WORD("mpr-16770.27", 0x800000, 0x200000, CRC(f9fa7bfb) SHA1(8aa933b74d4e05dc49987238705e50b00e5dae73) )
	ROM_LOAD32_WORD("mpr-16769.26", 0x800002, 0x200000, CRC(e57429e9) SHA1(8c712ab09e61ef510741a55f29b3c4e497471372) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16720.7", 0x000000, 0x020000, CRC(8e73cffd) SHA1(9933ccc0757e8c86e0adb938d1c89210b26841ea) )
	ROM_LOAD16_WORD_SWAP("epr-16721.8", 0x020000, 0x020000, CRC(1bb3b7b7) SHA1(ee2fd1480e535fc37e9932e6fe4e31344559fc87) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16491.32", 0x000000, 0x200000, CRC(89920903) SHA1(06d1d55470ae99f8de0f8c88c694f34c4eb13668) )
	ROM_LOAD("mpr-16492.33", 0x200000, 0x200000, CRC(459e701b) SHA1(2054f69cecad677eb00c6a3051f5b5d90885e19b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16493.4", 0x000000, 0x200000, CRC(9990db15) SHA1(ea9a8b45a07dccaae62be7cf095532ce7596a70c) )
	ROM_LOAD("mpr-16494.5", 0x200000, 0x200000, CRC(600e1d6c) SHA1(d4e246fc57a16ff562bbcbccf6a739b706f58696) )

	MODEL2_CPU_BOARD /* Model 2 CPU board extra roms */

	ROM_REGION( 0x10000, "user7", 0 ) // Unknown ROM
	ROM_LOAD("epr-14869c.25", 0x000000, 0x010000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371) )
ROM_END

ROM_START( daytonat )/* Daytona USA (Japan, Turbo hack) */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
//  ROM_LOAD32_WORD( "turbo1.12", 0x000000, 0x080000, CRC(0b3d5d4e) SHA1(1660959cb383e22f0d6204547c30cf5fe9272b03) ) /* 4x overdump?, 0x20000 bytes repeat 4 times */
//  ROM_LOAD32_WORD( "turbo2.13", 0x000002, 0x080000, CRC(f7d4e866) SHA1(c8c43904257f718665f9f7a89838eba14bde9465) ) /* 4x overdump?, 0x20000 bytes repeat 4 times */
	ROM_LOAD32_WORD( "turbo1.12", 0x000000, 0x020000, CRC(4b41a341) SHA1(daa75f38a11eb16b04550edf53e11f0eaf55cd3e) )
	ROM_LOAD32_WORD( "turbo2.13", 0x000002, 0x020000, CRC(6ca580fa) SHA1(102ad6bf5fed4c9c407a9e82d85cff9f15db31c8) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16528.10",   0x000000, 0x200000, CRC(9ce591f6) SHA1(e22fc8a70b533f7a6191f5952c581fb8f9627906) )
	ROM_LOAD32_WORD("mpr-16529.11",   0x000002, 0x200000, CRC(f7095eaf) SHA1(da3c922f950dd730ea348eae12aa1cb69cee9a58) )
	ROM_LOAD32_WORD("mpr-16808.8",    0x400000, 0x200000, CRC(44f1f5a0) SHA1(343866a6e2187a8ebc17f6727080f9f2f9ac9200) )
	ROM_LOAD32_WORD("mpr-16809.9",    0x400002, 0x200000, CRC(37a2dd12) SHA1(8192d8698d6bd52ee11cc28917aff5840c447627) )
	ROM_LOAD32_WORD("epr-16724a.6",   0x800000, 0x080000, CRC(469f10fd) SHA1(7fad3b8d03960e5e1f7a6cb36509238977e00fcc) )
	ROM_LOAD32_WORD("epr-16725a.7",   0x800002, 0x080000, CRC(ba0df8db) SHA1(d0c5581c56500b5266cab8e8151db24fcbdea0d7) )
	ROM_COPY( "user1", 0x800000, 0x900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x800000, 0xa00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xb00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xc00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("mpr-16537.ic28", 0x000000, 0x200000, CRC(36b7c35a) SHA1(b32fd1d3fc8983fb5f2a7b236b665a8c9b52769f) )
	ROM_LOAD32_WORD("mpr-16536.ic29", 0x000002, 0x200000, CRC(6d6afed9) SHA1(2018468d7d849854b3d0cfbcd217317e2fc93555) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16523.ic16", 0x000000, 0x200000, CRC(2f484d42) SHA1(0b83a3fc92b7d913a14cfb01d688c63555c17c41) )
	ROM_LOAD32_WORD("mpr-16518.ic20", 0x000002, 0x200000, CRC(df683bf7) SHA1(16afe5029591f3536b5b75d9cf50a34d0ea72c3d) )
	ROM_LOAD32_WORD("mpr-16524.ic17", 0x400000, 0x200000, CRC(34658bd7) SHA1(71b47626ffe5b26d1140afe1b830a9a2be86c88f) )
	ROM_LOAD32_WORD("mpr-16519.ic21", 0x400002, 0x200000, CRC(facd1c81) SHA1(dac8c281a5e9a6c4b60197e6676f3727264ee420) )
	ROM_LOAD32_WORD("mpr-16525.ic18", 0x800000, 0x200000, CRC(fb517521) SHA1(33f5f37ea2e09fc73eed5388b46fdf1fa9e285e6) )
	ROM_LOAD32_WORD("mpr-16520.ic22", 0x800002, 0x200000, CRC(d66bd9bd) SHA1(660171674484375a27595630e5e2d2ad76a06d1a) )
	ROM_LOAD32_WORD("mpr-16772.ic19", 0xc00000, 0x200000, CRC(770ed912) SHA1(1789f35dd403f73f8be18495a0fe4ad1e6841417) )
	ROM_LOAD32_WORD("mpr-16771.ic23", 0xc00002, 0x200000, CRC(a2205124) SHA1(257a3675e4ef6adbf61285a5daa5954223c28cb2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16522.25", 0x000000, 0x200000, CRC(55d39a57) SHA1(abf7b0fc0f111f90da42463d600db9fa32e95efe) )
	ROM_LOAD32_WORD("mpr-16521.24", 0x000002, 0x200000, CRC(af1934fb) SHA1(a6a21a23cd34d0de6d3e6a5c3c2687f905d0dc2a) )
	ROM_LOAD32_WORD("mpr-16770.27", 0x800000, 0x200000, CRC(f9fa7bfb) SHA1(8aa933b74d4e05dc49987238705e50b00e5dae73) )
	ROM_LOAD32_WORD("mpr-16769.26", 0x800002, 0x200000, CRC(e57429e9) SHA1(8c712ab09e61ef510741a55f29b3c4e497471372) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16720.7", 0x000000, 0x020000, CRC(8e73cffd) SHA1(9933ccc0757e8c86e0adb938d1c89210b26841ea) )
	ROM_LOAD16_WORD_SWAP("epr-16721.8", 0x020000, 0x020000, CRC(1bb3b7b7) SHA1(ee2fd1480e535fc37e9932e6fe4e31344559fc87) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16491.32", 0x000000, 0x200000, CRC(89920903) SHA1(06d1d55470ae99f8de0f8c88c694f34c4eb13668) )
	ROM_LOAD("mpr-16492.33", 0x200000, 0x200000, CRC(459e701b) SHA1(2054f69cecad677eb00c6a3051f5b5d90885e19b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16493.4", 0x000000, 0x200000, CRC(9990db15) SHA1(ea9a8b45a07dccaae62be7cf095532ce7596a70c) )
	ROM_LOAD("mpr-16494.5", 0x200000, 0x200000, CRC(600e1d6c) SHA1(d4e246fc57a16ff562bbcbccf6a739b706f58696) )

	MODEL2_CPU_BOARD /* Model 2 CPU board extra roms */

	ROM_REGION( 0x10000, "user7", 0 ) // Unknown ROM
	ROM_LOAD("epr-14869c.25", 0x000000, 0x010000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371) )
ROM_END

ROM_START( daytonata )/* Daytona USA (Japan, Turbo hack) */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "dayturbo.12", 0x000000, 0x020000, CRC(aec6857a) SHA1(e29261de4344c99d82c9e494467605593cc776d8) )
	ROM_LOAD32_WORD( "dayturbo.13", 0x000002, 0x020000, CRC(cb657edc) SHA1(90b8f673a4ef88e7c1f6012b80823d3e756f9743) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16528.10",   0x000000, 0x200000, CRC(9ce591f6) SHA1(e22fc8a70b533f7a6191f5952c581fb8f9627906) )
	ROM_LOAD32_WORD("mpr-16529.11",   0x000002, 0x200000, CRC(f7095eaf) SHA1(da3c922f950dd730ea348eae12aa1cb69cee9a58) )
	ROM_LOAD32_WORD("mpr-16808.8",    0x400000, 0x200000, CRC(44f1f5a0) SHA1(343866a6e2187a8ebc17f6727080f9f2f9ac9200) )
	ROM_LOAD32_WORD("mpr-16809.9",    0x400002, 0x200000, CRC(37a2dd12) SHA1(8192d8698d6bd52ee11cc28917aff5840c447627) )
	ROM_LOAD32_WORD("epr-16724a.6",   0x800000, 0x080000, CRC(469f10fd) SHA1(7fad3b8d03960e5e1f7a6cb36509238977e00fcc) )
	ROM_LOAD32_WORD("epr-16725a.7",   0x800002, 0x080000, CRC(ba0df8db) SHA1(d0c5581c56500b5266cab8e8151db24fcbdea0d7) )
	ROM_COPY( "user1", 0x800000, 0x900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x800000, 0xa00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xb00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xc00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("mpr-16537.ic28", 0x000000, 0x200000, CRC(36b7c35a) SHA1(b32fd1d3fc8983fb5f2a7b236b665a8c9b52769f) )
	ROM_LOAD32_WORD("mpr-16536.ic29", 0x000002, 0x200000, CRC(6d6afed9) SHA1(2018468d7d849854b3d0cfbcd217317e2fc93555) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16523.ic16", 0x000000, 0x200000, CRC(2f484d42) SHA1(0b83a3fc92b7d913a14cfb01d688c63555c17c41) )
	ROM_LOAD32_WORD("mpr-16518.ic20", 0x000002, 0x200000, CRC(df683bf7) SHA1(16afe5029591f3536b5b75d9cf50a34d0ea72c3d) )
	ROM_LOAD32_WORD("mpr-16524.ic17", 0x400000, 0x200000, CRC(34658bd7) SHA1(71b47626ffe5b26d1140afe1b830a9a2be86c88f) )
	ROM_LOAD32_WORD("mpr-16519.ic21", 0x400002, 0x200000, CRC(facd1c81) SHA1(dac8c281a5e9a6c4b60197e6676f3727264ee420) )
	ROM_LOAD32_WORD("mpr-16525.ic18", 0x800000, 0x200000, CRC(fb517521) SHA1(33f5f37ea2e09fc73eed5388b46fdf1fa9e285e6) )
	ROM_LOAD32_WORD("mpr-16520.ic22", 0x800002, 0x200000, CRC(d66bd9bd) SHA1(660171674484375a27595630e5e2d2ad76a06d1a) )
	ROM_LOAD32_WORD("mpr-16772.ic19", 0xc00000, 0x200000, CRC(770ed912) SHA1(1789f35dd403f73f8be18495a0fe4ad1e6841417) )
	ROM_LOAD32_WORD("mpr-16771.ic23", 0xc00002, 0x200000, CRC(a2205124) SHA1(257a3675e4ef6adbf61285a5daa5954223c28cb2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16522.25", 0x000000, 0x200000, CRC(55d39a57) SHA1(abf7b0fc0f111f90da42463d600db9fa32e95efe) )
	ROM_LOAD32_WORD("mpr-16521.24", 0x000002, 0x200000, CRC(af1934fb) SHA1(a6a21a23cd34d0de6d3e6a5c3c2687f905d0dc2a) )
	ROM_LOAD32_WORD("mpr-16770.27", 0x800000, 0x200000, CRC(f9fa7bfb) SHA1(8aa933b74d4e05dc49987238705e50b00e5dae73) )
	ROM_LOAD32_WORD("mpr-16769.26", 0x800002, 0x200000, CRC(e57429e9) SHA1(8c712ab09e61ef510741a55f29b3c4e497471372) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16720.7", 0x000000, 0x020000, CRC(8e73cffd) SHA1(9933ccc0757e8c86e0adb938d1c89210b26841ea) )
	ROM_LOAD16_WORD_SWAP("epr-16721.8", 0x020000, 0x020000, CRC(1bb3b7b7) SHA1(ee2fd1480e535fc37e9932e6fe4e31344559fc87) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16491.32", 0x000000, 0x200000, CRC(89920903) SHA1(06d1d55470ae99f8de0f8c88c694f34c4eb13668) )
	ROM_LOAD("mpr-16492.33", 0x200000, 0x200000, CRC(459e701b) SHA1(2054f69cecad677eb00c6a3051f5b5d90885e19b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16493.4", 0x000000, 0x200000, CRC(9990db15) SHA1(ea9a8b45a07dccaae62be7cf095532ce7596a70c) )
	ROM_LOAD("mpr-16494.5", 0x200000, 0x200000, CRC(600e1d6c) SHA1(d4e246fc57a16ff562bbcbccf6a739b706f58696) )

	MODEL2_CPU_BOARD /* Model 2 CPU board extra roms */

	ROM_REGION( 0x10000, "user7", 0 ) // Unknown ROM
	ROM_LOAD("epr-14869c.25", 0x000000, 0x010000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371) )
ROM_END

/*
Daytona "To The MAXX" upgrade.
Unofficial Sega hack for Model 2 Daytona machines

Kits contains 4 IC's
3 of them are standard 27C1024 EPROMS
1 of them is a PIC 16F84 mounted to a small board the size of an EPROM
with a 40 pin socket mounted on it, which plugs into position IC15
*/

ROM_START( daytonam ) /* Daytona USA (Japan, To The MAXX) */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "maxx.12", 0x000000, 0x020000, CRC(604ef2d9) SHA1(b1d5f0d41bea2e74fb9346da35a5041f4464265e) )
	ROM_LOAD32_WORD( "maxx.13", 0x000002, 0x020000, CRC(7d319970) SHA1(5bc150a77f20a29f54acdf5043fb1e8e55f6b08b) )
	ROM_LOAD32_WORD( "maxx.14", 0x040000, 0x020000, CRC(2debfce0) SHA1(b0f578ae68d49a3eebaf9b453a1ad774c8620476) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16528.10",   0x000000, 0x200000, CRC(9ce591f6) SHA1(e22fc8a70b533f7a6191f5952c581fb8f9627906) )
	ROM_LOAD32_WORD("mpr-16529.11",   0x000002, 0x200000, CRC(f7095eaf) SHA1(da3c922f950dd730ea348eae12aa1cb69cee9a58) )
	ROM_LOAD32_WORD("mpr-16808.8",    0x400000, 0x200000, CRC(44f1f5a0) SHA1(343866a6e2187a8ebc17f6727080f9f2f9ac9200) )
	ROM_LOAD32_WORD("mpr-16809.9",    0x400002, 0x200000, CRC(37a2dd12) SHA1(8192d8698d6bd52ee11cc28917aff5840c447627) )
	ROM_LOAD32_WORD("epr-16724a.6",   0x800000, 0x080000, CRC(469f10fd) SHA1(7fad3b8d03960e5e1f7a6cb36509238977e00fcc) )
	ROM_LOAD32_WORD("epr-16725a.7",   0x800002, 0x080000, CRC(ba0df8db) SHA1(d0c5581c56500b5266cab8e8151db24fcbdea0d7) )
	ROM_COPY( "user1", 0x800000, 0x900000, 0x100000 ) // rgn,srcoffset,offset,length.
	ROM_COPY( "user1", 0x800000, 0xa00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xb00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xc00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xd00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xe00000, 0x100000 )
	ROM_COPY( "user1", 0x800000, 0xf00000, 0x100000 )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("mpr-16537.ic28", 0x000000, 0x200000, CRC(36b7c35a) SHA1(b32fd1d3fc8983fb5f2a7b236b665a8c9b52769f) )
	ROM_LOAD32_WORD("mpr-16536.ic29", 0x000002, 0x200000, CRC(6d6afed9) SHA1(2018468d7d849854b3d0cfbcd217317e2fc93555) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16523.ic16", 0x000000, 0x200000, CRC(2f484d42) SHA1(0b83a3fc92b7d913a14cfb01d688c63555c17c41) )
	ROM_LOAD32_WORD("mpr-16518.ic20", 0x000002, 0x200000, CRC(df683bf7) SHA1(16afe5029591f3536b5b75d9cf50a34d0ea72c3d) )
	ROM_LOAD32_WORD("mpr-16524.ic17", 0x400000, 0x200000, CRC(34658bd7) SHA1(71b47626ffe5b26d1140afe1b830a9a2be86c88f) )
	ROM_LOAD32_WORD("mpr-16519.ic21", 0x400002, 0x200000, CRC(facd1c81) SHA1(dac8c281a5e9a6c4b60197e6676f3727264ee420) )
	ROM_LOAD32_WORD("mpr-16525.ic18", 0x800000, 0x200000, CRC(fb517521) SHA1(33f5f37ea2e09fc73eed5388b46fdf1fa9e285e6) )
	ROM_LOAD32_WORD("mpr-16520.ic22", 0x800002, 0x200000, CRC(d66bd9bd) SHA1(660171674484375a27595630e5e2d2ad76a06d1a) )
	ROM_LOAD32_WORD("mpr-16772.ic19", 0xc00000, 0x200000, CRC(770ed912) SHA1(1789f35dd403f73f8be18495a0fe4ad1e6841417) )
	ROM_LOAD32_WORD("mpr-16771.ic23", 0xc00002, 0x200000, CRC(a2205124) SHA1(257a3675e4ef6adbf61285a5daa5954223c28cb2) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16522.25", 0x000000, 0x200000, CRC(55d39a57) SHA1(abf7b0fc0f111f90da42463d600db9fa32e95efe) )
	ROM_LOAD32_WORD("mpr-16521.24", 0x000002, 0x200000, CRC(af1934fb) SHA1(a6a21a23cd34d0de6d3e6a5c3c2687f905d0dc2a) )
	ROM_LOAD32_WORD("mpr-16770.27", 0x800000, 0x200000, CRC(f9fa7bfb) SHA1(8aa933b74d4e05dc49987238705e50b00e5dae73) )
	ROM_LOAD32_WORD("mpr-16769.26", 0x800002, 0x200000, CRC(e57429e9) SHA1(8c712ab09e61ef510741a55f29b3c4e497471372) )

	ROM_REGION( 0x20000, "cpu3", 0) // Communication program
	ROM_LOAD( "epr-16726.bin", 0x000000, 0x020000, CRC(c179b8c7) SHA1(86d3e65c77fb53b1d380b629348f4ab5b3d39228) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16720.7", 0x000000, 0x020000, CRC(8e73cffd) SHA1(9933ccc0757e8c86e0adb938d1c89210b26841ea) )
	ROM_LOAD16_WORD_SWAP("epr-16721.8", 0x020000, 0x020000, CRC(1bb3b7b7) SHA1(ee2fd1480e535fc37e9932e6fe4e31344559fc87) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16491.32", 0x000000, 0x200000, CRC(89920903) SHA1(06d1d55470ae99f8de0f8c88c694f34c4eb13668) )
	ROM_LOAD("mpr-16492.33", 0x200000, 0x200000, CRC(459e701b) SHA1(2054f69cecad677eb00c6a3051f5b5d90885e19b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16493.4", 0x000000, 0x200000, CRC(9990db15) SHA1(ea9a8b45a07dccaae62be7cf095532ce7596a70c) )
	ROM_LOAD("mpr-16494.5", 0x200000, 0x200000, CRC(600e1d6c) SHA1(d4e246fc57a16ff562bbcbccf6a739b706f58696) )

	MODEL2_CPU_BOARD /* Model 2 CPU board extra roms */

	ROM_REGION( 0x10000, "user7", 0 ) // Unknown ROM
	ROM_LOAD("epr-14869c.25", 0x000000, 0x010000, CRC(24b68e64) SHA1(c19d044d4c2fe551474492aa51922587394dd371) )
ROM_END

ROM_START( vcop ) /* Virtua Cop Revision B, Model 2 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17166b.12", 0x000000, 0x020000, CRC(a5647c59) SHA1(0a9e0be447d3591e82efd40ef4acbfe7ae211579) )
	ROM_LOAD32_WORD( "epr-17167b.13", 0x000002, 0x020000, CRC(f5dde26a) SHA1(95db029bc4206a44ea216afbcd1c19689f79115a) )
	ROM_LOAD32_WORD( "epr-17160a.14", 0x040000, 0x020000, CRC(267f3242) SHA1(40ec09cda984bb80969bfae2278432153137c213) )
	ROM_LOAD32_WORD( "epr-17161a.15", 0x040002, 0x020000, CRC(f7126876) SHA1(b0ceb1206edaa507ec15723497fcd447a511f423) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17164.10", 0x000000, 0x200000, CRC(ac5fc501) SHA1(e60deec1e79d207d37d3f4ddd83a1b2125c411ac) )
	ROM_LOAD32_WORD( "mpr-17165.11", 0x000002, 0x200000, CRC(82296d00) SHA1(23327137b36c98dfb9175ea9d36478e7385dfac2) )
	ROM_LOAD32_WORD( "mpr-17162.8",  0x400000, 0x200000, CRC(60ddd41e) SHA1(0894c9bcdedeb09f921419a309858e242cb8db3a) )
	ROM_LOAD32_WORD( "mpr-17163.9",  0x400002, 0x200000, CRC(8c1f9dc8) SHA1(cf99a5bb4f343d59c8d6f5716287b6e16bef6412) )
	ROM_LOAD32_WORD( "epr-17168a.6", 0x800000, 0x080000, CRC(59091a37) SHA1(14591c7015aaf126755be584aa94c04e6de222fa) )
	ROM_LOAD32_WORD( "epr-17169a.7", 0x800002, 0x080000, CRC(0495808d) SHA1(5b86a9a68c2b52f942aa8d858ee7a491f546a921) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP program

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17159.16", 0x000000, 0x200000, CRC(e218727d) SHA1(1458d01d49936a0b8d497b62ff9ea940ca753b37) )
	ROM_LOAD32_WORD( "mpr-17156.20", 0x000002, 0x200000, CRC(c4f4aabf) SHA1(8814cd329609cc8a188fedd770230bb9a5d00361) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17158.25", 0x000000, 0x200000, CRC(1108d1ec) SHA1(e95d4166bd4b26c5f21b85821b410f53045f4309) )
	ROM_LOAD32_WORD( "mpr-17157.24", 0x000002, 0x200000, CRC(cf31e33d) SHA1(0cb62d4f28b5ad8a7e4c82b0ca8aea3037b05455) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD32_WORD( "epr-17181.6", 0x000000, 0x010000, CRC(1add2b82) SHA1(81892251d466f630a96af25bde652c20e47d7ede) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP( "epr-17170.7", 0x000000, 0x020000, CRC(06a38ae2) SHA1(a2c3d14d9266449ebfc6d976a956e0a8a602cfb0) )
	ROM_LOAD16_WORD_SWAP( "epr-17171.8", 0x020000, 0x020000, CRC(b5e436f8) SHA1(1da3cb52d64f52d03a8de9954afffbc6e1549a5b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD( "mpr-17172.32", 0x000000, 0x100000, CRC(ab22cac3) SHA1(0e872158faeb8c0404b10cdf0a3fa36f89a5093e) )
	ROM_LOAD( "mpr-17173.33", 0x200000, 0x100000, CRC(3cb4005c) SHA1(a56f436ea6dfe0968b73ae7bc92bb2f4c612460d) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD( "mpr-17174.4", 0x000000, 0x200000, CRC(a50369cc) SHA1(69807157baf6e3679adc95633c82b0236db01247) )
	ROM_LOAD( "mpr-17175.5", 0x200000, 0x200000, CRC(9136d43c) SHA1(741f80a8ff8165ffe171dc568e0da4ad0bde4809) )

	MODEL2_CPU_BOARD
ROM_END

ROM_START( vcopa ) /* Virtua Cop Revision A, Model 2 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD( "epr-17166a.12", 0x000000, 0x020000, CRC(702566e6) SHA1(478eec1e1d51a2ff63e8fd591528f0ca70df9310) )
	ROM_LOAD32_WORD( "epr-17167a.13", 0x000002, 0x020000, CRC(9b8e05a8) SHA1(5e95f3f901d7f87f8c9cbeb3a65cd1b74e9cc09b) )
	ROM_LOAD32_WORD( "epr-17160a.14", 0x040000, 0x020000, CRC(267f3242) SHA1(40ec09cda984bb80969bfae2278432153137c213) )
	ROM_LOAD32_WORD( "epr-17161a.15", 0x040002, 0x020000, CRC(f7126876) SHA1(b0ceb1206edaa507ec15723497fcd447a511f423) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD( "mpr-17164.10", 0x000000, 0x200000, CRC(ac5fc501) SHA1(e60deec1e79d207d37d3f4ddd83a1b2125c411ac) )
	ROM_LOAD32_WORD( "mpr-17165.11", 0x000002, 0x200000, CRC(82296d00) SHA1(23327137b36c98dfb9175ea9d36478e7385dfac2) )
	ROM_LOAD32_WORD( "mpr-17162.8",  0x400000, 0x200000, CRC(60ddd41e) SHA1(0894c9bcdedeb09f921419a309858e242cb8db3a) )
	ROM_LOAD32_WORD( "mpr-17163.9",  0x400002, 0x200000, CRC(8c1f9dc8) SHA1(cf99a5bb4f343d59c8d6f5716287b6e16bef6412) )
	ROM_LOAD32_WORD( "epr-17168a.6", 0x800000, 0x080000, CRC(59091a37) SHA1(14591c7015aaf126755be584aa94c04e6de222fa) )
	ROM_LOAD32_WORD( "epr-17169a.7", 0x800002, 0x080000, CRC(0495808d) SHA1(5b86a9a68c2b52f942aa8d858ee7a491f546a921) )

	ROM_REGION( 0x800000, "tgp", ROMREGION_ERASE00 ) // TGP program

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD( "mpr-17159.16", 0x000000, 0x200000, CRC(e218727d) SHA1(1458d01d49936a0b8d497b62ff9ea940ca753b37) )
	ROM_LOAD32_WORD( "mpr-17156.20", 0x000002, 0x200000, CRC(c4f4aabf) SHA1(8814cd329609cc8a188fedd770230bb9a5d00361) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD( "mpr-17158.25", 0x000000, 0x200000, CRC(1108d1ec) SHA1(e95d4166bd4b26c5f21b85821b410f53045f4309) )
	ROM_LOAD32_WORD( "mpr-17157.24", 0x000002, 0x200000, CRC(cf31e33d) SHA1(0cb62d4f28b5ad8a7e4c82b0ca8aea3037b05455) )

	ROM_REGION( 0x20000, "cpu4", 0) // Communication program
	ROM_LOAD32_WORD( "epr-17181.6", 0x000000, 0x010000, CRC(1add2b82) SHA1(81892251d466f630a96af25bde652c20e47d7ede) )

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP( "epr-17170.7", 0x000000, 0x020000, CRC(06a38ae2) SHA1(a2c3d14d9266449ebfc6d976a956e0a8a602cfb0) )
	ROM_LOAD16_WORD_SWAP( "epr-17171.8", 0x020000, 0x020000, CRC(b5e436f8) SHA1(1da3cb52d64f52d03a8de9954afffbc6e1549a5b) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD( "mpr-17172.32", 0x000000, 0x100000, CRC(ab22cac3) SHA1(0e872158faeb8c0404b10cdf0a3fa36f89a5093e) )
	ROM_LOAD( "mpr-17173.33", 0x200000, 0x100000, CRC(3cb4005c) SHA1(a56f436ea6dfe0968b73ae7bc92bb2f4c612460d) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD( "mpr-17174.4", 0x000000, 0x200000, CRC(a50369cc) SHA1(69807157baf6e3679adc95633c82b0236db01247) )
	ROM_LOAD( "mpr-17175.5", 0x200000, 0x200000, CRC(9136d43c) SHA1(741f80a8ff8165ffe171dc568e0da4ad0bde4809) )

	MODEL2_CPU_BOARD
ROM_END

ROM_START( desert ) /* Desert Tank, Model 2 */
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("epr-16976.12", 0x000000, 0x020000, CRC(d036dff0) SHA1(f3e5f22ef1f3ff9c9a1ff7352cdad3e2c2977a51) )
	ROM_LOAD32_WORD("epr-16977.13", 0x000002, 0x020000, CRC(e91194bd) SHA1(cec8eb8d4b52c387d5750ee5a0c6e6ce7c0fe80d) )
	ROM_LOAD32_WORD("epr-16970.14", 0x040000, 0x020000, CRC(4ea12d1f) SHA1(75133b03a450518bae27d62f0a1c37451c8c49a0) )
	ROM_LOAD32_WORD("epr-16971.15", 0x040002, 0x020000, CRC(d630b220) SHA1(ca7bd1e01e396b8b6a0925e767cc714729e0fd42) )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Data
	ROM_LOAD32_WORD("mpr-16974.10", 0x000000, 0x200000, CRC(2ab491c5) SHA1(79deb3877d0ffc8ee75c01d3bf0a6dd71cc2b552) )
	ROM_LOAD32_WORD("mpr-16975.11", 0x000002, 0x200000, CRC(e24fe7d3) SHA1(f8ab28c95d421978b1517adeacf09e7ee203d8f6) )
	ROM_LOAD32_WORD("mpr-16972.8",  0x400000, 0x200000, CRC(23e53748) SHA1(9c8a1d8aec8f9e5504e5aac0390dfb3770ab8616) )
	ROM_LOAD32_WORD("mpr-16973.9",  0x400002, 0x200000, CRC(77d6f509) SHA1(c83bce7f7b0a15bd14b99e829640b7dd9948e671) )
	ROM_LOAD32_WORD("epr-16978.6",  0x800000, 0x080000, CRC(38b3e574) SHA1(a1133df608b0fbb9c53bbeb29138650c87845d2c) )
	ROM_LOAD32_WORD("epr-16979.7",  0x800002, 0x080000, CRC(c314eb8b) SHA1(0c851dedd5c42b026195faed7d028924698a8b27) )

	ROM_REGION( 0x800000, "tgp", 0 ) // TGP program? (COPRO socket)
	ROM_LOAD32_WORD("epr-16981.28", 0x000000, 0x080000, CRC(ae847571) SHA1(32d0f9e685667ae9fddacea0b9f4ad6fb3a6fdad) )
	ROM_LOAD32_WORD("epr-16980.29", 0x000002, 0x080000, CRC(5239b864) SHA1(e889556e0f1ea80de52afff563b0923f87cef7ab) )

	ROM_REGION( 0x1000000, "user2", 0 ) // Models
	ROM_LOAD32_WORD("mpr-16968.16", 0x000000, 0x200000, CRC(4a16f465) SHA1(411214ed65ce966040d4299b50bfaa40f7f5f266) )
	ROM_LOAD32_WORD("mpr-16964.21", 0x000002, 0x200000, CRC(d4a769b6) SHA1(845c34f95a49e06e3996b0c67aa73b4886fa8996) )
	ROM_LOAD32_WORD("mpr-16969.17", 0x400000, 0x200000, CRC(887380ac) SHA1(03a9f601764d06cb0b2daaadf4f8433f327abd4a) )
	ROM_LOAD32_WORD("mpr-16965.20", 0x400002, 0x200000, CRC(9ba7645f) SHA1(c04f369961f908bac16fad8e32b863202390c205) )

	ROM_REGION( 0x1000000, "user3", 0 ) // Textures
	ROM_LOAD32_WORD("mpr-16967.25", 0x000000, 0x200000, CRC(b8b84c9d) SHA1(00ef320988609e98c8af383b68d845e3be8d0a03) )
	ROM_LOAD32_WORD("mpr-16966.24", 0x000002, 0x200000, CRC(7484efe9) SHA1(33e72139ad6c2990428e3fa041dbcdf39aca1c7a) )

	ROM_REGION( 0x20000, "cpu4", ROMREGION_ERASE00 ) // Communication program

	ROM_REGION( 0xc0000, M1AUDIO_CPU_REGION, ROMREGION_BE|ROMREGION_16BIT )  /* 68K code */
	ROM_LOAD16_WORD_SWAP("epr-16985.7", 0x000000, 0x20000, CRC(8c4d9056) SHA1(785752d761c648d1177c5f0cfa3e9fa44135d6dc) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM1_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16986.32", 0x000000, 0x200000, CRC(559612f9) SHA1(33bcaddfc7d8fe899707e663299e8f04e9004d51) )

	ROM_REGION( 0x400000, M1AUDIO_MPCM2_REGION, 0 ) // Samples
	ROM_LOAD("mpr-16988.4", 0x000000, 0x200000, CRC(bc705875) SHA1(5351c6bd2d75df57ff92960e7f90493d95d9dfb9) )
	ROM_LOAD("mpr-16989.5", 0x200000, 0x200000, CRC(1b616b31) SHA1(35bd2bfd08514ba6f235cda2605c171cd51fd78e) )

	MODEL2_CPU_BOARD
ROM_END

DRIVER_INIT_MEMBER(model2_state,genprot)
{
	//std::string key = parameter(":315_5881:key");
	m_maincpu->space(AS_PROGRAM).install_ram(0x01d80000, 0x01d8ffff);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x01d90000, 0x01d9ffff, read32_delegate(FUNC(model2_state::model2_5881prot_r), this), write32_delegate(FUNC(model2_state::model2_5881prot_w), this));
}

DRIVER_INIT_MEMBER(model2_state,pltkids)
{
	DRIVER_INIT_CALL(genprot);

	// fix bug in program: it destroys the interrupt table and never fixes it
	UINT32 *ROM = (UINT32 *)memregion("maincpu")->base();
	ROM[0x730/4] = 0x08000004;
}

DRIVER_INIT_MEMBER(model2_state,zerogun)
{
	DRIVER_INIT_CALL(genprot);

	// fix bug in program: it destroys the interrupt table and never fixes it
	UINT32 *ROM = (UINT32 *)memregion("maincpu")->base();
	ROM[0x700/4] = 0x08000004;
}

DRIVER_INIT_MEMBER(model2_state,daytonam)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x240000, 0x24ffff, read32_delegate(FUNC(model2_state::maxx_r),this));
}

/* very crude support for let the game set itself into stand-alone mode */

READ32_MEMBER(model2_state::jaleco_network_r)
{
	if(offset == 0x4000/4)
	{
		if(m_netram[offset] == 0x00000000)
			m_jnet_time_out = 0;

		if((m_netram[offset] & 0xffff) == 0x0001)
			m_jnet_time_out++;

		if(m_jnet_time_out > 0x80)
			m_netram[offset]|= 0x00800000;

		return m_netram[offset];
	}

	return m_netram[offset];
}

WRITE32_MEMBER(model2_state::jaleco_network_w)
{
	COMBINE_DATA(&m_netram[offset]);
}

DRIVER_INIT_MEMBER(model2_state,sgt24h)
{
//  DRIVER_INIT_CALL(genprot);

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x01a10000, 0x01a1ffff, read32_delegate(FUNC(model2_state::jaleco_network_r),this), write32_delegate(FUNC(model2_state::jaleco_network_w),this));

	UINT32 *ROM = (UINT32 *)memregion("maincpu")->base();
	ROM[0x56578/4] = 0x08000004;
	//ROM[0x5b3e8/4] = 0x08000004;
}

DRIVER_INIT_MEMBER(model2_state,overrev)
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x01a10000, 0x01a1ffff, read32_delegate(FUNC(model2_state::jaleco_network_r),this), write32_delegate(FUNC(model2_state::jaleco_network_w),this));
}


DRIVER_INIT_MEMBER(model2_state,doa)
{
	m_0229crypt->install_doa_protection();

	UINT32 *ROM = (UINT32 *)memregion("maincpu")->base();
	ROM[0x630/4] = 0x08000004;
	ROM[0x808/4] = 0x08000004;
}

DRIVER_INIT_MEMBER(model2_state,rchase2)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x01c00008, 0x01c0000b, write32_delegate(FUNC(model2_state::rchase2_devices_w),this));
}

DRIVER_INIT_MEMBER(model2_state,srallyc)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x01c00008, 0x01c0000b, write32_delegate(FUNC(model2_state::srallyc_devices_w),this));
}


// Model 2 (TGPs, Model 1 sound board)
GAME( 1993, daytona,         0, model2o, daytona, driver_device, 0,        ROT0, "Sega",   "Daytona USA (Japan, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, daytonase, daytona, model2o, daytona, driver_device, 0,        ROT0, "Sega",   "Daytona USA Special Edition (Japan, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, daytona93, daytona, model2o, daytona, driver_device, 0,        ROT0, "Sega",   "Daytona USA Deluxe '93", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, daytonas,  daytona, model2o, daytona, driver_device, 0,        ROT0, "Sega",   "Daytona USA (With Saturn Adverts)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, daytonat,  daytona, model2o, daytona, driver_device, 0,        ROT0, "Sega",   "Daytona USA (Japan, Turbo hack, set 1)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, daytonata, daytona, model2o, daytona, driver_device, 0,        ROT0, "Sega",   "Daytona USA (Japan, Turbo hack, set 2)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, daytonam,  daytona, model2o, daytona, model2_state,  daytonam, ROT0, "Sega",   "Daytona USA (Japan, To The MAXX)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, desert,          0, model2o, desert,  driver_device, 0,        ROT0, "Sega / Martin Marietta", "Desert Tank", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, vcop,            0, model2o, vcop,    driver_device, 0,        ROT0, "Sega",   "Virtua Cop (Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, vcopa,        vcop, model2o, vcop,    driver_device, 0,        ROT0, "Sega",   "Virtua Cop (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )

// Model 2A-CRX (TGPs, SCSP sound board)
GAME( 1995, manxtt,          0, manxttdx,     manxtt,   driver_device, 0,       ROT0, "Sega",   "Manx TT Superbike - DX (Revision D)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, manxttc,         0, model2a,      manxtt,   driver_device, 0,       ROT0, "Sega",   "Manx TT Superbike - Twin (Revision C)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, srallyc,         0, srallyc,      srallyc,  model2_state,  srallyc, ROT0, "Sega",   "Sega Rally Championship - TWIN (Revision C)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, srallycb,  srallyc, srallyc,      srallyc,  model2_state,  srallyc, ROT0, "Sega",   "Sega Rally Championship - TWIN (Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, srallyca,  srallyc, srallyc,      srallyc,  model2_state,  srallyc, ROT0, "Sega",   "Sega Rally Championship - DX (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, vf2,             0, model2a,      model2,   driver_device, 0,       ROT0, "Sega",   "Virtua Fighter 2 (Version 2.1)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, vf2b,          vf2, model2a,      model2,   driver_device, 0,       ROT0, "Sega",   "Virtua Fighter 2 (Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, vf2a,          vf2, model2a,      model2,   driver_device, 0,       ROT0, "Sega",   "Virtua Fighter 2 (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, vf2o,          vf2, model2a,      model2,   driver_device, 0,       ROT0, "Sega",   "Virtua Fighter 2", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, vcop2,           0, model2a,      vcop2,    driver_device, 0,       ROT0, "Sega",   "Virtua Cop 2", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, skytargt,        0, model2a,      skytargt, driver_device, 0,       ROT0, "Sega",   "Sky Target", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, doaa,          doa, model2a_0229, model2,   model2_state,  doa,     ROT0, "Sega",   "Dead or Alive (Model 2A, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, zeroguna,  zerogun, model2a_5881, model2,   model2_state,  zerogun, ROT0, "Psikyo", "Zero Gunner (Export, Model 2A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, zerogunaj, zerogun, model2a_5881, model2,   model2_state,  zerogun, ROT0, "Psikyo", "Zero Gunner (Japan, Model 2A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, motoraid,        0, model2a,      manxtt,   driver_device, 0,       ROT0, "Sega",   "Motor Raid - Twin", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, dynamcop,        0, model2a_5881, model2,   model2_state,  genprot, ROT0, "Sega",   "Dynamite Cop (Export, Model 2A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, dyndeka2, dynamcop, model2a_5881, model2,   model2_state,  genprot, ROT0, "Sega",   "Dynamite Deka 2 (Japan, Model 2A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, pltkidsa,  pltkids, model2a_5881, model2,   model2_state,  pltkids, ROT0, "Psikyo", "Pilot Kids (Model 2A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )

// Model 2B-CRX (SHARC, SCSP sound board)
GAME( 1994, vstriker,        0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Virtua Striker (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, vstrikero,vstriker, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Virtua Striker", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, fvipers,         0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Fighting Vipers (Revision D)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, gunblade,        0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Gunblade NY (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, indy500,         0, model2b,      srallyc, driver_device, 0,       ROT0, "Sega",   "INDY 500 Twin (Revision A, Newer)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, indy500d,  indy500, model2b,      srallyc, driver_device, 0,       ROT0, "Sega",   "INDY 500 Deluxe (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, indy500to, indy500, model2b,      srallyc, driver_device, 0,       ROT0, "Sega",   "INDY 500 Twin (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, schamp,          0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Sonic Championship (USA)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, sfight,     schamp, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Sonic the Fighters (Japan)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, lastbrnx,        0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Last Bronx (Export, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, lastbrnxu,lastbrnx, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Last Bronx (USA, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, lastbrnxj,lastbrnx, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Last Bronx (Japan, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, doa,             0, model2b_0229, model2,  model2_state,  doa,     ROT0, "Sega",   "Dead or Alive (Model 2B, Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, sgt24h,          0, model2b,      srallyc, model2_state,  sgt24h,  ROT0, "Jaleco", "Super GT 24h", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, von,             0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Cyber Troopers Virtual-On (USA, Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, vonj,          von, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Cyber Troopers Virtual-On (Japan, Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, dynabb,          0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Dynamite Baseball", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, dynabb97,        0, model2b,      model2,  driver_device, 0,       ROT0, "Sega",   "Dynamite Baseball 97 (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, overrevb,  overrev, model2b,      srallyc, model2_state,  overrev, ROT0, "Jaleco", "Over Rev (Model 2B, Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, zerogun,         0, model2b_5881, model2,  model2_state,  zerogun, ROT0, "Psikyo", "Zero Gunner (Export, Model 2B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, zerogunj,  zerogun, model2b_5881, model2,  model2_state,  zerogun, ROT0, "Psikyo", "Zero Gunner (Japan, Model 2B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, dynamcopb,dynamcop, model2b_5881, model2,  model2_state,  genprot, ROT0, "Sega",   "Dynamite Cop (Export, Model 2B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, dyndeka2b,dynamcop, model2b_5881, model2,  model2_state,  genprot, ROT0, "Sega",   "Dynamite Deka 2 (Japan, Model 2B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, pltkids,         0, model2b_5881, model2,  model2_state,  pltkids, ROT0, "Psikyo", "Pilot Kids (Model 2B, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, rchase2,         0, model2b,      rchase2, model2_state,  rchase2, ROT0, "Sega",   "Rail Chase 2 (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )

// Model 2C-CRX (TGPx4, SCSP sound board)
GAME( 1996, skisuprg,        0, model2c,      model2,  driver_device, 0,       ROT0, "Sega",   "Sega Ski Super G", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS|MACHINE_UNEMULATED_PROTECTION )
GAME( 1996, stcc,            0,    stcc,      model2,  driver_device, 0,       ROT0, "Sega",   "Sega Touring Car Championship", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, stccb,        stcc,    stcc,      model2,  driver_device, 0,       ROT0, "Sega",   "Sega Touring Car Championship (Revision B)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, stcca,        stcc,    stcc,      model2,  driver_device, 0,       ROT0, "Sega",   "Sega Touring Car Championship (Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, waverunr,        0, model2c,      model2,  driver_device, 0,       ROT0, "Sega",   "Wave Runner (Japan, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, hotd,            0, model2c,      model2,  driver_device, 0,       ROT0, "Sega",   "House of the Dead", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, overrev,         0, model2c,      srallyc, model2_state,  overrev, ROT0, "Jaleco", "Over Rev (Model 2C, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, segawski,        0, model2c,      model2,  driver_device, 0,       ROT0, "Sega",   "Sega Water Ski (Japan, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, topskatr,        0, model2c,      model2,  driver_device, 0,       ROT0, "Sega",   "Top Skater (Export, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, topskatru,topskatr, model2c,      model2,  driver_device, 0,       ROT0, "Sega",   "Top Skater (USA, Revision A)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, topskatrj,topskatr, model2c,      model2,  driver_device, 0,       ROT0, "Sega",   "Top Skater (Japan)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, bel,             0, model2c,      bel,     driver_device, 0,       ROT0, "Sega / EPL Productions", "Behind Enemy Lines", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, dynamcopc,dynamcop, model2c_5881, model2,  model2_state,  genprot, ROT0, "Sega",   "Dynamite Cop (USA, Model 2C)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
