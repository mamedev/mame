/***************************************************************************

    Microprose Games machine hardware

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/am29000/am29000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/68681.h"
#include "includes/micro3d.h"


/*************************************
 *
 *  Defines
 *
 *************************************/

#define MAC_CLK				XTAL_10MHz
#define VTXROM_FMT(x)		(((x) << 14) | ((x) & (1 << 15) ? 0xc0000000 : 0))


/*************************************
 *
 *  68681 DUART
 *
 *************************************/

void micro3d_duart_irq_handler(running_device *device, UINT8 vector)
{
	cputag_set_input_line_and_vector(device->machine, "maincpu", 3, HOLD_LINE, vector);
};

void micro3d_duart_tx(running_device *device, int channel, UINT8 data)
{
	micro3d_state *state = device->machine->driver_data<micro3d_state>();

	if (channel == 0)
	{
#if HOST_MONITOR_DISPLAY
		mame_debug_printf("%c", data);
#endif
	}
	else
	{
		state->m68681_tx0 = data;
		cputag_set_input_line(device->machine, "audiocpu", MCS51_RX_LINE, HOLD_LINE);
	}
};

static int data_to_i8031(running_device *device)
{
	micro3d_state *state = device->machine->driver_data<micro3d_state>();
	return state->m68681_tx0;
}

static void data_from_i8031(running_device *device, int data)
{
	micro3d_state *state = device->machine->driver_data<micro3d_state>();
	duart68681_rx_data(state->duart68681, 1, data);
}

/*
 * 0: Monitor port P4
 * 1: 5V
 * 2: /AM29000 present
 * 3: /TMS34010 present
 * 4: -
 * 5: -
 */
UINT8 micro3d_duart_input_r(running_device *device)
{
	return 0x2;
}

/*
 * 5: /I8051 reset
 * 7: Status LED
*/
void micro3d_duart_output_w(running_device *device, UINT8 data)
{
	cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_RESET, data & 0x20 ? CLEAR_LINE : ASSERT_LINE);
}


/*************************************
 *
 *  68901
 *
 *************************************/

enum
{
	TMRB = 0,
	TXERR,
	TBE,
	RXERR,
	RBF,
	TMRA,
	GPIP6,
	GPIP7,
	GPIP0,
	GPIP1,
	GPIP2,
	GPIP3,
	TMRD,
	TMRC,
	GPIP4,
	GPIP5
};


static void micro3d_mc68901_int_gen(running_machine *machine, int source)
{
	micro3d_state *state = machine->driver_data<micro3d_state>();
	UINT8 *ien = source & 8 ? &state->mc68901.ierb : &state->mc68901.iera;
	UINT8 *ipend = source & 8 ? &state->mc68901.iprb : &state->mc68901.ipra;
	UINT8 *imask = source & 8 ? &state->mc68901.imrb : &state->mc68901.imra;
	int bit = source & 7;

	if (*ien & (1 << bit))
	{
		*ipend |= (1 << bit);
	}

	if (*imask & (1 << bit))
	{
		cputag_set_input_line(machine, "maincpu", 4, HOLD_LINE);
	}
}

static TIMER_CALLBACK( mfp_timer_a_cb )
{
	micro3d_mc68901_int_gen(machine, TMRA);
}

READ16_HANDLER( micro3d_mc68901_r )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();
	UINT16 val = 0;

	switch (offset)
	{
		case 0:
			val = 0xff;
			break;
		default:
			val = state->mc68901.regs[offset] << 8;
	}

	return val;
}

WRITE16_HANDLER( micro3d_mc68901_w )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	data >>= 8;
	state->mc68901.regs[offset] = data;

	switch (offset)
	{
		case 0x0f:
		{
			int mode = state->mc68901.tacr & 0xf;

			state->mc68901.tadr = data;

			/* Timer stopped */
			if (mode == 0)
			{
				timer_enable(state->mc68901.timer_a, 0);
			}
			else if (mode < 8)
			{
				int divisor = 1;
				attotime period;

				switch (mode)
				{
					case 1: divisor = 4; break;
					case 2: divisor = 10; break;
					case 3: divisor = 16; break;
					case 4: divisor = 50; break;
					case 5: divisor = 64; break;
					case 6: divisor = 100; break;
					case 7: divisor = 200; break;
				}

				period = attotime_mul(ATTOTIME_IN_HZ(4000000 / divisor), data);

				timer_adjust_periodic(state->mc68901.timer_a, period, 0, period);
			}
			else
			{
				fatalerror("MC68901: Unsupported Timer A mode! (%x)", state->mc68901.tadr);
			}
			break;
		}
		default:
			state->mc68901.regs[offset] = data;
	}
}


/*************************************
 *
 *  SCN2651 (TMS34010)
 *
 *************************************/

enum
{
	RX, TX, STATUS, SYN1, SYN2, DLE, MODE1, MODE2, COMMAND
};


WRITE16_HANDLER( micro3d_ti_uart_w )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	switch (offset)
	{
		case 0x0:
		{
			state->ti_uart[TX] = data;
#if VGB_MONITOR_DISPLAY
			mame_debug_printf("%c",data);
#endif
			state->ti_uart[STATUS] |= 1;
			break;
		}
		case 0x1:
		{
			if (state->ti_uart_mode_cycle == 0)
			{
				state->ti_uart[MODE1] = data;
				state->ti_uart_mode_cycle = 1;
			}
			else
			{
				state->ti_uart[MODE2] = data;
				state->ti_uart_mode_cycle = 0;
			}
			break;
		}
		case 0x2:
		{
			if (state->ti_uart_sync_cycle == 0)
			{
				state->ti_uart[SYN1] = data;
				state->ti_uart_mode_cycle = 1;
			}
			else if (state->ti_uart_sync_cycle == 1)
			{
				state->ti_uart[SYN2] = data;
				state->ti_uart_mode_cycle = 2;
			}
			else
			{
				state->ti_uart[DLE] = data;
				state->ti_uart_mode_cycle = 0;
			}
			break;
		}
		case 0x3:
		{
			state->ti_uart[COMMAND] = data;
			state->ti_uart_mode_cycle = 0;
			state->ti_uart_sync_cycle = 0;
			break;
		}
	}
}

READ16_HANDLER( micro3d_ti_uart_r )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	switch (offset)
	{
		case 0x0:
		{
			state->ti_uart[STATUS] ^= 2;
			return state->ti_uart[RX];
		}
		case 0x1:
		{
			if (state->ti_uart_mode_cycle == 0)
			{
				state->ti_uart_mode_cycle = 1;
				return state->ti_uart[MODE1];
			}
			else
			{
				state->ti_uart_mode_cycle = 0;
				return state->ti_uart[MODE2];
			}
		}
		case 0x2:
		{
			return state->ti_uart[STATUS];
		}
		case 0x3:
		{
			state->ti_uart_mode_cycle = state->ti_uart_sync_cycle = 0;
			return state->ti_uart[COMMAND];
		}
		default:
		{
			logerror("Unknown TI UART access.\n");
			return 0;
		}
	}
}


/*************************************
 *
 *  Z8530 SCC (Am29000)
 *
 *************************************/

WRITE32_HANDLER( micro3d_scc_w )
{
#if DRMATH_MONITOR_DISPLAY
	if (offset == 1)
		mame_printf_debug("%c", data);
#endif
}

READ32_HANDLER( micro3d_scc_r )
{
	if (offset == 1)
		return 0xd;
	else
		return 5;
}


/*************************************
 *
 *  Host<->TMS34010 interface
 *
 *************************************/

READ16_HANDLER( micro3d_tms_host_r )
{
	return tms34010_host_r(space->machine->device("vgb"), offset);
}

WRITE16_HANDLER( micro3d_tms_host_w )
{
	tms34010_host_w(space->machine->device("vgb"), offset, data);
}


/*************************************
 *
 *  Math unit
 *
 *************************************/

INLINE INT64 dot_product(micro3d_vtx *v1, micro3d_vtx *v2)
{
	INT64 result = ((INT64)v1->x * (INT64)v2->x) +
					((INT64)v1->y * (INT64)v2->y) +
					((INT64)v1->z * (INT64)v2->z);
	return result;
}

INLINE INT64 normalised_multiply(INT32 a, INT32 b)
{
	INT64 result;

	result = (INT64)a * (INT64)b;
	return result >> 14;
}

static TIMER_CALLBACK( mac_done_callback )
{
	micro3d_state *state = machine->driver_data<micro3d_state>();

	cputag_set_input_line(machine, "drmath", AM29000_INTR0, ASSERT_LINE);
	state->mac_stat = 0;
}


WRITE32_HANDLER( micro3d_mac1_w )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	state->vtx_addr = (data & 0x3ffff);
	state->sram_w_addr = (data >> 18) & 0xfff;
}

READ32_HANDLER( micro3d_mac2_r )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	return (state->mac_inst << 1) | state->mac_stat;
}

WRITE32_HANDLER( micro3d_mac2_w )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	UINT32 cnt = data & 0xff;
	UINT32 inst = (data >> 8) & 0x1f;
	UINT32 mac_cycles = 1;

	UINT32 mrab11;
	UINT32 vtx_addr;
	UINT32 sram_r_addr;
	UINT32 sram_w_addr;
	UINT32 *mac_sram;

	state->mac_stat = BIT(data, 13);
	state->mac_inst = inst & 0x7;
	state->mrab11 = (data >> 18) & (1 << 11);
	state->sram_r_addr = (data >> 18) & 0xfff;

	mrab11 = state->mrab11;
	vtx_addr = state->vtx_addr;
	sram_r_addr = state->sram_r_addr;
	sram_w_addr = state->sram_w_addr;
	mac_sram = state->mac_sram;

	if (data & (1 << 14))
		cputag_set_input_line(space->machine, "drmath", AM29000_INTR0, CLEAR_LINE);

	switch (inst)
	{
		case 0x00: break;
		case 0x04: break;

		case 0x0b: cnt += 0x100;
		case 0x0a: cnt += 0x100;
		case 0x09: cnt += 0x100;
		case 0x08:
		{
			int i;
			const UINT16 *rom = (UINT16*)memory_region(space->machine, "vertex");

			for (i = 0; i <= cnt; ++i)
			{
				INT64 acc;
				micro3d_vtx v1;

				v1.x = VTXROM_FMT(rom[vtx_addr]);	vtx_addr++;
				v1.y = VTXROM_FMT(rom[vtx_addr]);	vtx_addr++;
				v1.z = VTXROM_FMT(rom[vtx_addr]);	vtx_addr++;

				acc  = normalised_multiply(mac_sram[mrab11 + 0x7f0], v1.x);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f1], v1.y);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f2], v1.z);
				acc += mac_sram[mrab11 + 0x7f3];
				mac_sram[sram_w_addr++] = acc;

				acc  = normalised_multiply(mac_sram[mrab11 + 0x7f4], v1.x);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f5], v1.y);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f6], v1.z);
				acc += mac_sram[mrab11 + 0x7f7];
				mac_sram[sram_w_addr++] = acc;

				acc  = normalised_multiply(mac_sram[mrab11 + 0x7f8], v1.x);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f9], v1.y);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7fa], v1.z);
				acc += mac_sram[mrab11 + 0x7fb];
				mac_sram[sram_w_addr++] = acc;

				mac_cycles = 16 * cnt;
			}

			break;
		}
		case 0x0e: cnt += 0x100;
		case 0x0d: cnt += 0x100;
		case 0x0c:
		{
			int i;
			const UINT16 *rom = (UINT16*)memory_region(space->machine, "vertex");

			for (i = 0; i <= cnt; ++i)
			{
				INT64 acc;
				micro3d_vtx v1;

				v1.x = VTXROM_FMT(rom[vtx_addr]);	vtx_addr++;
				v1.y = VTXROM_FMT(rom[vtx_addr]);	vtx_addr++;
				v1.z = VTXROM_FMT(rom[vtx_addr]);	vtx_addr++;

				acc  = normalised_multiply(mac_sram[mrab11 + 0x7f0], v1.x);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f1], v1.y);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f2], v1.z);
				mac_sram[sram_w_addr++] = acc;

				acc  = normalised_multiply(mac_sram[mrab11 + 0x7f4], v1.x);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f5], v1.y);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f6], v1.z);
				mac_sram[sram_w_addr++] = acc;

				acc  = normalised_multiply(mac_sram[mrab11 + 0x7f8], v1.x);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7f9], v1.y);
				acc += normalised_multiply(mac_sram[mrab11 + 0x7fa], v1.z);
				mac_sram[sram_w_addr++] = acc;

				mac_cycles = 12 * cnt;
			}
			break;
		}
		case 0x0f:
		{
			int i;
			const UINT16 *rom = (UINT16*)memory_region(space->machine, "vertex");

			for (i = 0; i <= cnt; ++i, vtx_addr += 4)
			{
				mac_sram[sram_w_addr++] = VTXROM_FMT(rom[vtx_addr + 0]);
				mac_sram[sram_w_addr++] = VTXROM_FMT(rom[vtx_addr + 1]);
				mac_sram[sram_w_addr++] = VTXROM_FMT(rom[vtx_addr + 2]);
				mac_sram[sram_w_addr++] = VTXROM_FMT(rom[vtx_addr + 3]);
			}

			mac_cycles = 8 * cnt;
			break;
		}
		/* Dot product of SRAM vectors with single SRAM vector */
		case 0x11: cnt += 0x100;
		case 0x10:
		{
			int i;
			micro3d_vtx v2;

			v2.x = mac_sram[mrab11 + 0x7fc];
			v2.y = mac_sram[mrab11 + 0x7fd];
			v2.z = mac_sram[mrab11 + 0x7fe];

			for (i = 0; i <= cnt; ++i)
			{
				micro3d_vtx v1;
				INT64 dp;

				v1.x = mac_sram[sram_r_addr++];
				v1.y = mac_sram[sram_r_addr++];
				v1.z = mac_sram[sram_r_addr++];

				dp = dot_product(&v1, &v2);
				mac_sram[sram_w_addr++] = dp >> 32;
				mac_sram[sram_w_addr++] = dp & 0xffffffff;
				mac_sram[sram_w_addr++] = 0;
			}

			mac_cycles = 10 * cnt;
			break;
		}
		/* Dot product of SRAM vectors with SRAM vectors */
		case 0x16: cnt += 0x100;
		case 0x15: cnt += 0x100;
		case 0x14:
		{
			int i;

			for (i = 0; i <= cnt; ++i)
			{
				micro3d_vtx v1;
				micro3d_vtx v2;
				INT64 dp;

				v1.x = mac_sram[sram_r_addr++];
				v1.y = mac_sram[sram_r_addr++];
				v1.z = mac_sram[sram_r_addr++];

				v2.x = mac_sram[vtx_addr++];
				v2.y = mac_sram[vtx_addr++];
				v2.z = mac_sram[vtx_addr++];

				dp = dot_product(&v1, &v2);
				mac_sram[sram_w_addr++] = dp >> 32;
				mac_sram[sram_w_addr++] = dp & 0xffffffff;
				mac_sram[sram_w_addr++] = 0;
			}

			mac_cycles = 10 * cnt;
			break;
		}
		default:
			logerror("Unknown MAC instruction : %x\n", inst);
			break;
	}

	/* TODO: Calculate a better estimate for timing */
	if (state->mac_stat)
		timer_set(space->machine, attotime_mul(ATTOTIME_IN_HZ(MAC_CLK), mac_cycles), NULL, 0, mac_done_callback);

	state->mrab11 = mrab11;
	state->vtx_addr = vtx_addr;
	state->sram_r_addr = sram_r_addr;
	state->sram_w_addr = sram_w_addr;
}


/*************************************
 *
 *  Analog controls
 *
 *************************************/

READ16_HANDLER( micro3d_encoder_h_r )
{
	UINT16 x_encoder = input_port_read_safe(space->machine, "JOYSTICK_X", 0);
	UINT16 y_encoder = input_port_read_safe(space->machine, "JOYSTICK_Y", 0);

	return (y_encoder & 0xf00) | ((x_encoder & 0xf00) >> 8);
}

READ16_HANDLER( micro3d_encoder_l_r )
{
	UINT16 x_encoder = input_port_read_safe(space->machine, "JOYSTICK_X", 0);
	UINT16 y_encoder = input_port_read_safe(space->machine, "JOYSTICK_Y", 0);

	return ((y_encoder & 0xff) << 8) | (x_encoder & 0xff);
}

static TIMER_CALLBACK( adc_done_callback )
{
	micro3d_state *state = machine->driver_data<micro3d_state>();

	switch (param)
	{
		case 0: state->adc_val = input_port_read_safe(machine, "THROTTLE", 0);
				break;
		case 1: state->adc_val = (UINT8)((255.0/100.0) * input_port_read(machine, "VOLUME") + 0.5);
				break;
		case 2: break;
		case 3: break;
	}

//  mc68901_int_gen(machine, GPIP3);
}

READ16_HANDLER( micro3d_adc_r )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	return state->adc_val;
}

WRITE16_HANDLER( micro3d_adc_w )
{
	/* Only handle single-ended mode */
	if (data < 4 || data > 7)
	{
		logerror("ADC0844 unhandled MUX mode: %x\n", data);
		return;
	}

	timer_set(space->machine, ATTOTIME_IN_USEC(40), NULL, data & ~4, adc_done_callback);
}

CUSTOM_INPUT( botssa_hwchk_r )
{
	micro3d_state *state = field->port->machine->driver_data<micro3d_state>();

	return state->botssa_latch;
}

READ16_HANDLER( botssa_140000_r )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	state->botssa_latch = 0;
	return 0xffff;
}

READ16_HANDLER( botssa_180000_r )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	state->botssa_latch = 1;
	return 0xffff;
}

/*************************************
 *
 *  CPU control
 *
 *************************************/

WRITE16_HANDLER( micro3d_reset_w )
{
	data >>= 8;
	cputag_set_input_line(space->machine, "drmath", INPUT_LINE_RESET, data & 1 ? CLEAR_LINE : ASSERT_LINE);
	cputag_set_input_line(space->machine, "vgb", INPUT_LINE_RESET, data & 2 ? CLEAR_LINE : ASSERT_LINE);
	/* TODO: Joystick reset? */
}

WRITE16_HANDLER( host_drmath_int_w )
{
	cputag_set_input_line(space->machine, "drmath", AM29000_INTR2, ASSERT_LINE);
	cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(10));
}


/*************************************
 *
 *
 *
 *************************************/

WRITE32_HANDLER( micro3d_shared_w )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	state->shared_ram[offset * 2 + 1] = data & 0xffff;
	state->shared_ram[offset * 2 + 0] = data >> 16;
}

READ32_HANDLER( micro3d_shared_r )
{
	micro3d_state *state = space->machine->driver_data<micro3d_state>();

	return (state->shared_ram[offset * 2] << 16) | state->shared_ram[offset * 2 + 1];
}

WRITE32_HANDLER( drmath_int_w )
{
	cputag_set_input_line(space->machine, "maincpu", 5, HOLD_LINE);
}

WRITE32_HANDLER( drmath_intr2_ack )
{
	cputag_set_input_line(space->machine, "drmath", AM29000_INTR2, CLEAR_LINE);
}


/*************************************
 *
 *  Driver initialisation
 *
 *************************************/

DRIVER_INIT( micro3d )
{
	micro3d_state *state = machine->driver_data<micro3d_state>();
	const address_space *space = cputag_get_address_space(machine, "drmath", ADDRESS_SPACE_DATA);

	i8051_set_serial_tx_callback(machine->device("audiocpu"), data_from_i8031);
	i8051_set_serial_rx_callback(machine->device("audiocpu"), data_to_i8031);

	state->duart68681 = machine->device("duart68681");

	/* The Am29000 program seems to rely on RAM from 0x00470000 onwards being
    non-zero on a reset, otherwise the 3D object data doesn't get uploaded! */
	memory_write_dword(space, 0x00470000, 0xa5a5a5a5);

	state->mc68901.timer_a = timer_alloc(machine, mfp_timer_a_cb, NULL);

	/* TODO? BOTSS crashes when starting the final stage because the 68000
    overwrites memory in use by the Am29000. Slowing down the 68000 slightly
    avoids this */
	machine->device("maincpu")->set_clock_scale(0.945f);
}

DRIVER_INIT( botssa )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	/* Required to pass the hardware version check */
	memory_install_read16_handler(space, 0x140000, 0x140001, 0, 0, botssa_140000_r );
	memory_install_read16_handler(space, 0x180000, 0x180001, 0, 0, botssa_180000_r );

	DRIVER_INIT_CALL(micro3d);
}

MACHINE_RESET( micro3d )
{
	micro3d_state *state = machine->driver_data<micro3d_state>();

	state->ti_uart[STATUS] = 1;

	cputag_set_input_line(machine, "vgb", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "drmath", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
}
