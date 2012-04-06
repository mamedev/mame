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

void micro3d_duart_irq_handler(device_t *device, UINT8 vector)
{
	cputag_set_input_line_and_vector(device->machine(), "maincpu", 3, HOLD_LINE, vector);
};

void micro3d_duart_tx(device_t *device, int channel, UINT8 data)
{
	micro3d_state *state = device->machine().driver_data<micro3d_state>();

	if (channel == 0)
	{
#if HOST_MONITOR_DISPLAY
		mame_debug_printf("%c", data);
#endif
	}
	else
	{
		state->m_m68681_tx0 = data;
		cputag_set_input_line(device->machine(), "audiocpu", MCS51_RX_LINE, ASSERT_LINE);
		// TODO: next line should be behind a timer callback which lasts one audiocpu clock cycle
		cputag_set_input_line(device->machine(), "audiocpu", MCS51_RX_LINE, CLEAR_LINE);
	}
};

static int data_to_i8031(device_t *device)
{
	micro3d_state *state = device->machine().driver_data<micro3d_state>();
	return state->m_m68681_tx0;
}

static void data_from_i8031(device_t *device, int data)
{
	micro3d_state *state = device->machine().driver_data<micro3d_state>();
	duart68681_rx_data(state->m_duart68681, 1, data);
}

/*
 * 0: Monitor port P4
 * 1: 5V
 * 2: /AM29000 present
 * 3: /TMS34010 present
 * 4: -
 * 5: -
 */
UINT8 micro3d_duart_input_r(device_t *device)
{
	return 0x2;
}

/*
 * 5: /I8051 reset
 * 7: Status LED
*/
void micro3d_duart_output_w(device_t *device, UINT8 data)
{
	cputag_set_input_line(device->machine(), "audiocpu", INPUT_LINE_RESET, data & 0x20 ? CLEAR_LINE : ASSERT_LINE);
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


WRITE16_MEMBER(micro3d_state::micro3d_ti_uart_w)
{

	switch (offset)
	{
		case 0x0:
		{
			m_ti_uart[TX] = data;
#if VGB_MONITOR_DISPLAY
			mame_debug_printf("%c",data);
#endif
			m_ti_uart[STATUS] |= 1;
			break;
		}
		case 0x1:
		{
			if (m_ti_uart_mode_cycle == 0)
			{
				m_ti_uart[MODE1] = data;
				m_ti_uart_mode_cycle = 1;
			}
			else
			{
				m_ti_uart[MODE2] = data;
				m_ti_uart_mode_cycle = 0;
			}
			break;
		}
		case 0x2:
		{
			if (m_ti_uart_sync_cycle == 0)
			{
				m_ti_uart[SYN1] = data;
				m_ti_uart_mode_cycle = 1;
			}
			else if (m_ti_uart_sync_cycle == 1)
			{
				m_ti_uart[SYN2] = data;
				m_ti_uart_mode_cycle = 2;
			}
			else
			{
				m_ti_uart[DLE] = data;
				m_ti_uart_mode_cycle = 0;
			}
			break;
		}
		case 0x3:
		{
			m_ti_uart[COMMAND] = data;
			m_ti_uart_mode_cycle = 0;
			m_ti_uart_sync_cycle = 0;
			break;
		}
	}
}

READ16_MEMBER(micro3d_state::micro3d_ti_uart_r)
{

	switch (offset)
	{
		case 0x0:
		{
			m_ti_uart[STATUS] ^= 2;
			return m_ti_uart[RX];
		}
		case 0x1:
		{
			if (m_ti_uart_mode_cycle == 0)
			{
				m_ti_uart_mode_cycle = 1;
				return m_ti_uart[MODE1];
			}
			else
			{
				m_ti_uart_mode_cycle = 0;
				return m_ti_uart[MODE2];
			}
		}
		case 0x2:
		{
			return m_ti_uart[STATUS];
		}
		case 0x3:
		{
			m_ti_uart_mode_cycle = m_ti_uart_sync_cycle = 0;
			return m_ti_uart[COMMAND];
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

WRITE32_MEMBER(micro3d_state::micro3d_scc_w)
{
#if DRMATH_MONITOR_DISPLAY
	if (offset == 1)
		mame_printf_debug("%c", data);
#endif
}

READ32_MEMBER(micro3d_state::micro3d_scc_r)
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

READ16_MEMBER(micro3d_state::micro3d_tms_host_r)
{
	return tms34010_host_r(machine().device("vgb"), offset);
}

WRITE16_MEMBER(micro3d_state::micro3d_tms_host_w)
{
	tms34010_host_w(machine().device("vgb"), offset, data);
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
	micro3d_state *state = machine.driver_data<micro3d_state>();

	cputag_set_input_line(machine, "drmath", AM29000_INTR0, ASSERT_LINE);
	state->m_mac_stat = 0;
}


WRITE32_MEMBER(micro3d_state::micro3d_mac1_w)
{

	m_vtx_addr = (data & 0x3ffff);
	m_sram_w_addr = (data >> 18) & 0xfff;
}

READ32_MEMBER(micro3d_state::micro3d_mac2_r)
{

	return (m_mac_inst << 1) | m_mac_stat;
}

WRITE32_MEMBER(micro3d_state::micro3d_mac2_w)
{

	UINT32 cnt = data & 0xff;
	UINT32 inst = (data >> 8) & 0x1f;
	UINT32 mac_cycles = 1;

	UINT32 mrab11;
	UINT32 vtx_addr;
	UINT32 sram_r_addr;
	UINT32 sram_w_addr;
	UINT32 *mac_sram;

	m_mac_stat = BIT(data, 13);
	m_mac_inst = inst & 0x7;
	m_mrab11 = (data >> 18) & (1 << 11);
	m_sram_r_addr = (data >> 18) & 0xfff;

	mrab11 = m_mrab11;
	vtx_addr = m_vtx_addr;
	sram_r_addr = m_sram_r_addr;
	sram_w_addr = m_sram_w_addr;
	mac_sram = m_mac_sram;

	if (data & (1 << 14))
		cputag_set_input_line(machine(), "drmath", AM29000_INTR0, CLEAR_LINE);

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
			const UINT16 *rom = (UINT16*)machine().region("vertex")->base();

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
			const UINT16 *rom = (UINT16*)machine().region("vertex")->base();

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
			const UINT16 *rom = (UINT16*)machine().region("vertex")->base();

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
	if (m_mac_stat)
		machine().scheduler().timer_set(attotime::from_hz(MAC_CLK) * mac_cycles, FUNC(mac_done_callback));

	m_mrab11 = mrab11;
	m_vtx_addr = vtx_addr;
	m_sram_r_addr = sram_r_addr;
	m_sram_w_addr = sram_w_addr;
}


/*************************************
 *
 *  Analog controls
 *
 *************************************/

READ16_MEMBER(micro3d_state::micro3d_encoder_h_r)
{
	UINT16 x_encoder = input_port_read_safe(machine(), "JOYSTICK_X", 0);
	UINT16 y_encoder = input_port_read_safe(machine(), "JOYSTICK_Y", 0);

	return (y_encoder & 0xf00) | ((x_encoder & 0xf00) >> 8);
}

READ16_MEMBER(micro3d_state::micro3d_encoder_l_r)
{
	UINT16 x_encoder = input_port_read_safe(machine(), "JOYSTICK_X", 0);
	UINT16 y_encoder = input_port_read_safe(machine(), "JOYSTICK_Y", 0);

	return ((y_encoder & 0xff) << 8) | (x_encoder & 0xff);
}

static TIMER_CALLBACK( adc_done_callback )
{
	micro3d_state *state = machine.driver_data<micro3d_state>();

	switch (param)
	{
		case 0: state->m_adc_val = input_port_read_safe(machine, "THROTTLE", 0);
				break;
		case 1: state->m_adc_val = (UINT8)((255.0/100.0) * input_port_read(machine, "VOLUME") + 0.5);
				break;
		case 2: break;
		case 3: break;
	}

//  mc68901_int_gen(machine, GPIP3);
}

READ16_MEMBER(micro3d_state::micro3d_adc_r)
{

	return m_adc_val;
}

WRITE16_MEMBER(micro3d_state::micro3d_adc_w)
{
	/* Only handle single-ended mode */
	if (data < 4 || data > 7)
	{
		logerror("ADC0844 unhandled MUX mode: %x\n", data);
		return;
	}

	machine().scheduler().timer_set(attotime::from_usec(40), FUNC(adc_done_callback), data & ~4);
}

CUSTOM_INPUT( botssa_hwchk_r )
{
	micro3d_state *state = field.machine().driver_data<micro3d_state>();

	return state->m_botssa_latch;
}

READ16_MEMBER(micro3d_state::botssa_140000_r)
{

	m_botssa_latch = 0;
	return 0xffff;
}

READ16_MEMBER(micro3d_state::botssa_180000_r)
{

	m_botssa_latch = 1;
	return 0xffff;
}

/*************************************
 *
 *  CPU control
 *
 *************************************/

WRITE16_MEMBER(micro3d_state::micro3d_reset_w)
{
	data >>= 8;
	cputag_set_input_line(machine(), "drmath", INPUT_LINE_RESET, data & 1 ? CLEAR_LINE : ASSERT_LINE);
	cputag_set_input_line(machine(), "vgb", INPUT_LINE_RESET, data & 2 ? CLEAR_LINE : ASSERT_LINE);
	/* TODO: Joystick reset? */
}

WRITE16_MEMBER(micro3d_state::host_drmath_int_w)
{
	cputag_set_input_line(machine(), "drmath", AM29000_INTR2, ASSERT_LINE);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));
}


/*************************************
 *
 *
 *
 *************************************/

WRITE32_MEMBER(micro3d_state::micro3d_shared_w)
{

	m_shared_ram[offset * 2 + 1] = data & 0xffff;
	m_shared_ram[offset * 2 + 0] = data >> 16;
}

READ32_MEMBER(micro3d_state::micro3d_shared_r)
{

	return (m_shared_ram[offset * 2] << 16) | m_shared_ram[offset * 2 + 1];
}

WRITE32_MEMBER(micro3d_state::drmath_int_w)
{
	cputag_set_input_line(machine(), "maincpu", 5, HOLD_LINE);
}

WRITE32_MEMBER(micro3d_state::drmath_intr2_ack)
{
	cputag_set_input_line(machine(), "drmath", AM29000_INTR2, CLEAR_LINE);
}


/*************************************
 *
 *  Driver initialisation
 *
 *************************************/

DRIVER_INIT( micro3d )
{
	micro3d_state *state = machine.driver_data<micro3d_state>();
	address_space *space = machine.device("drmath")->memory().space(AS_DATA);

	i8051_set_serial_tx_callback(machine.device("audiocpu"), data_from_i8031);
	i8051_set_serial_rx_callback(machine.device("audiocpu"), data_to_i8031);

	state->m_duart68681 = machine.device("duart68681");

	/* The Am29000 program seems to rely on RAM from 0x00470000 onwards being
    non-zero on a reset, otherwise the 3D object data doesn't get uploaded! */
	space->write_dword(0x00470000, 0xa5a5a5a5);

	/* TODO? BOTSS crashes when starting the final stage because the 68000
    overwrites memory in use by the Am29000. Slowing down the 68000 slightly
    avoids this */
	machine.device("maincpu")->set_clock_scale(0.945f);
}

DRIVER_INIT( botssa )
{
	micro3d_state *state = machine.driver_data<micro3d_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* Required to pass the hardware version check */
	space->install_read_handler(0x140000, 0x140001, read16_delegate(FUNC(micro3d_state::botssa_140000_r),state));
	space->install_read_handler(0x180000, 0x180001, read16_delegate(FUNC(micro3d_state::botssa_180000_r),state));

	DRIVER_INIT_CALL(micro3d);
}

MACHINE_RESET( micro3d )
{
	micro3d_state *state = machine.driver_data<micro3d_state>();

	state->m_ti_uart[STATUS] = 1;

	cputag_set_input_line(machine, "vgb", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "drmath", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
}
