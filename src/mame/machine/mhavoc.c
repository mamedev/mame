/***************************************************************************

    Atari Major Havoc hardware

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/avgdvg.h"
#include "sound/5220intf.h"
#include "cpu/m6502/m6502.h"
#include "mhavoc.h"

UINT8 *mhavoc_zram0, *mhavoc_zram1;

static UINT8 alpha_data;
static UINT8 alpha_rcvd;
static UINT8 alpha_xmtd;

static UINT8 gamma_data;
static UINT8 gamma_rcvd;
static UINT8 gamma_xmtd;

static UINT8 player_1;

static UINT8 alpha_irq_clock;
static UINT8 alpha_irq_clock_enable;
static UINT8 gamma_irq_clock;

static UINT8 has_gamma_cpu;



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static TIMER_CALLBACK( cpu_irq_clock )
{
	/* clock the LS161 driving the alpha CPU IRQ */
	if (alpha_irq_clock_enable)
	{
		alpha_irq_clock++;
		if ((alpha_irq_clock & 0x0c) == 0x0c)
		{
			cpunum_set_input_line(machine, 0, 0, ASSERT_LINE);
			alpha_irq_clock_enable = 0;
		}
	}

	/* clock the LS161 driving the gamma CPU IRQ */
	if (has_gamma_cpu)
	{
		gamma_irq_clock++;
		cpunum_set_input_line(machine, 1, 0, (gamma_irq_clock & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}


WRITE8_HANDLER( mhavoc_alpha_irq_ack_w )
{
	/* clear the line and reset the clock */
	cpunum_set_input_line(Machine, 0, 0, CLEAR_LINE);
	alpha_irq_clock = 0;
	alpha_irq_clock_enable = 1;
}


WRITE8_HANDLER( mhavoc_gamma_irq_ack_w )
{
	/* clear the line and reset the clock */
	cpunum_set_input_line(Machine, 1, 0, CLEAR_LINE);
	gamma_irq_clock = 0;
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET( mhavoc )
{
	has_gamma_cpu = (cpu_gettotalcpu() > 1);

	memory_configure_bank(1, 0, 1, mhavoc_zram0, 0);
	memory_configure_bank(1, 1, 1, mhavoc_zram1, 0);
	memory_configure_bank(2, 0, 4,  memory_region(REGION_CPU1) + 0x10000, 0x2000);

	/* reset RAM/ROM banks to 0 */
	mhavoc_ram_banksel_w(machine, 0, 0);
	mhavoc_rom_banksel_w(machine, 0, 0);

	/* reset alpha comm status */
	alpha_data = 0;
	alpha_rcvd = 0;
	alpha_xmtd = 0;

	/* reset gamma comm status */
	gamma_data = 0;
	gamma_rcvd = 0;
	gamma_xmtd = 0;

	/* reset player 1 flag */
	player_1 = 0;

	/* reset IRQ clock states */
	alpha_irq_clock = 0;
	alpha_irq_clock_enable = 1;
	gamma_irq_clock = 0;

	/* set a timer going for the CPU interrupt generators */
	timer_pulse(ATTOTIME_IN_HZ(MHAVOC_CLOCK_5K), NULL, 0, cpu_irq_clock);

	state_save_register_item("misc", 0, alpha_data);
	state_save_register_item("misc", 0, alpha_rcvd);
	state_save_register_item("misc", 0, alpha_xmtd);
	state_save_register_item("misc", 0, gamma_data);
	state_save_register_item("misc", 0, gamma_rcvd);
	state_save_register_item("misc", 0, gamma_xmtd);
	state_save_register_item("misc", 0, player_1);
	state_save_register_item("misc", 0, alpha_irq_clock);
	state_save_register_item("misc", 0, alpha_irq_clock_enable);
	state_save_register_item("misc", 0, gamma_irq_clock);
}



/*************************************
 *
 *  Alpha -> gamma communications
 *
 *************************************/

static TIMER_CALLBACK( delayed_gamma_w )
{
	/* mark the data received */
	gamma_rcvd = 0;
	alpha_xmtd = 1;
	alpha_data = param;

	/* signal with an NMI pulse */
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);

	/* the sound CPU needs to reply in 250microseconds (according to Neil Bradley) */
	timer_set(ATTOTIME_IN_USEC(250), NULL, 0, 0);
}


WRITE8_HANDLER( mhavoc_gamma_w )
{
	logerror("  writing to gamma processor: %02x (%d %d)\n", data, gamma_rcvd, alpha_xmtd);
	timer_call_after_resynch(NULL, data, delayed_gamma_w);
}


READ8_HANDLER( mhavoc_alpha_r )
{
	logerror("\t\t\t\t\treading from alpha processor: %02x (%d %d)\n", alpha_data, gamma_rcvd, alpha_xmtd);
	gamma_rcvd = 1;
	alpha_xmtd = 0;
	return alpha_data;
}



/*************************************
 *
 *  Gamma -> alpha communications
 *
 *************************************/

WRITE8_HANDLER( mhavoc_alpha_w )
{
	logerror("\t\t\t\t\twriting to alpha processor: %02x %d %d\n", data, alpha_rcvd, gamma_xmtd);
	alpha_rcvd = 0;
	gamma_xmtd = 1;
	gamma_data = data;
}


READ8_HANDLER( mhavoc_gamma_r )
{
	logerror("  reading from gamma processor: %02x (%d %d)\n", gamma_data, alpha_rcvd, gamma_xmtd);
	alpha_rcvd = 1;
	gamma_xmtd = 0;
	return gamma_data;
}



/*************************************
 *
 *  RAM/ROM banking
 *
 *************************************/

WRITE8_HANDLER( mhavoc_ram_banksel_w )
{
	memory_set_bank(1, data & 1);
}


WRITE8_HANDLER( mhavoc_rom_banksel_w )
{
	memory_set_bank(2, data & 3);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

READ8_HANDLER( mhavoc_port_0_r )
{
	UINT8 res;

	/* Bits 7-6 = selected based on Player 1 */
	/* Bits 5-4 = common */
	if (player_1)
		res = (input_port_read_indexed(machine, 0) & 0x30) | (input_port_read_indexed(machine, 5) & 0xc0);
	else
		res = input_port_read_indexed(machine, 0) & 0xf0;

	/* Bit 3 = Gamma rcvd flag */
	if (gamma_rcvd)
		res |= 0x08;

	/* Bit 2 = Gamma xmtd flag */
	if (gamma_xmtd)
		res |= 0x04;

	/* Bit 1 = 2.4kHz (divide 2.5MHz by 1024) */
	if (!(activecpu_gettotalcycles() & 0x400))
		res |= 0x02;

	/* Bit 0 = Vector generator halt flag */
	if (avgdvg_done())
		res |= 0x01;

	return res;
}


READ8_HANDLER( alphaone_port_0_r )
{
	/* Bits 7-2 = common */
	UINT8 res = input_port_read_indexed(machine, 0) & 0xfc;

	/* Bit 1 = 2.4kHz (divide 2.5MHz by 1024) */
	if (!(activecpu_gettotalcycles() & 0x400))
		res |= 0x02;

	/* Bit 0 = Vector generator halt flag */
	if (avgdvg_done())
		res |= 0x01;

	return res;
}


READ8_HANDLER( mhavoc_port_1_r )
{
	/* Bits 7-2 = input switches */
	UINT8 res = input_port_read_indexed(machine, 1) & 0xfc;

	/* Bit 1 = Alpha rcvd flag */
	if (has_gamma_cpu && alpha_rcvd)
		res |= 0x02;

	/* Bit 0 = Alpha xmtd flag */
	if (has_gamma_cpu && alpha_xmtd)
		res |= 0x01;

	return res;
}

READ8_HANDLER( mhavoc_port_1_sp_r )
{
	/* Bits 7-3 = input switches */
	UINT8 res = input_port_read_indexed(machine, 1) & 0xf8;

	/* Bit 2 = TMS5220 ready flag */
	if (!tms5220_ready_r())
			res |= 0x04;

	/* Bit 1 = Alpha rcvd flag */
	if (has_gamma_cpu && alpha_rcvd)
		res |= 0x02;

	/* Bit 0 = Alpha xmtd flag */
	if (has_gamma_cpu && alpha_xmtd)
		res |= 0x01;

	return res;
}




/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE8_HANDLER( mhavoc_out_0_w )
{
	/* Bit 7 = Invert Y -- unemulated */
	/* Bit 6 = Invert X -- unemulated */

	/* Bit 5 = Player 1 */
	player_1 = (data >> 5) & 1;

	/* Bit 3 = Gamma reset */
	cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x08))
	{
		logerror("\t\t\t\t*** resetting gamma processor. ***\n");
		alpha_rcvd = 0;
		alpha_xmtd = 0;
		gamma_rcvd = 0;
		gamma_xmtd = 0;
	}

	/* Bit 0 = Roller light (Blinks on fatal errors) */
	set_led_status(0, data & 0x01);
}


WRITE8_HANDLER( alphaone_out_0_w )
{
	/* Bit 5 = P2 lamp */
	set_led_status(0, ~data & 0x20);

	/* Bit 4 = P1 lamp */
	set_led_status(1, ~data & 0x10);

	/* Bit 1 = right coin counter */
	coin_counter_w(1, data & 0x02);

	/* Bit 0 = left coin counter */
	coin_counter_w(0, data & 0x01);

logerror("alphaone_out_0_w(%02X)\n", data);
}


WRITE8_HANDLER( mhavoc_out_1_w )
{
	/* Bit 1 = left coin counter */
	coin_counter_w(0, data & 0x02);

	/* Bit 0 = right coin counter */
	coin_counter_w(1, data & 0x01);
}
