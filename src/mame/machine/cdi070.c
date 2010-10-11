/******************************************************************************


    CD-i-specific SCC68070 SoC peripheral emulation
    -------------------

    MESS implementation by Harmony


*******************************************************************************

STATUS:

- Skeleton.  Just enough for the CD-i to run.

TODO:

- Proper handling of the 68070's internal devices (UART, DMA, Timers, etc.)

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/cdi070.h"
#include "includes/cdi.h"

#define ENABLE_QUIZARD_HACK (1)

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine *machine, int n_level, const char *s_fmt, ...)
{
    if( VERBOSE_LEVEL >= n_level )
    {
        va_list v;
        char buf[ 32768 ];
        va_start( v, s_fmt );
        vsprintf( buf, s_fmt, v );
        va_end( v );
        logerror( "%08x: %s", cpu_get_pc(machine->device("maincpu")), buf );
    }
}
#else
#define verboselog(x,y,z,...)
#endif

static void scc68070_set_timer_callback(scc68070_regs_t *scc68070, int channel)
{
    UINT32 compare = 0;
    attotime period;
    switch(channel)
    {
        case 0:
            compare = 0x10000 - scc68070->timers.timer0;
            period = attotime_mul(ATTOTIME_IN_HZ(CLOCK_A/192), compare);
            timer_adjust_oneshot(scc68070->timers.timer0_timer, period, 0);
            break;
        default:
            fatalerror( "Unsupported timer channel to scc68070_set_timer_callback!" );
    }
}

static bool hack_active = false;
static UINT16 hack_value = 0;
static UINT32 hack_base = 0;
static UINT8 hack_ack = 0;
static UINT32 hack2_addr = 0;

void scc68070_set_secondary_hack(UINT32 address)
{
	hack2_addr = address;
}

void scc68070_set_hack_ack(UINT8 ack)
{
	hack_ack = ack;
}

void scc68070_set_hack_value(UINT16 value)
{
	hack_value = value;
}

void scc68070_set_hack_base(UINT32 base)
{
	hack_base = base;
}

static void quizard_patch(running_machine *machine)
{
    address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	if(hack_active)
	{
		// Patch out:
		// 00264C42: C0EE 86BE                  mulu.w  (-$7942,A6), D0
		// 00264C46: 2200                       move.l  D0, D1
		// 00264C48: 41EE 86C0                  lea     (-$7940,A6), A0
		// 00264C4C: 2008                       move.l  A0, D0
		// 00264C4E: 6100 FE6A                  bsr     $264aba
		// 00264C52: 3400                       move.w  D0, D2
		// 00264C54: 3017                       move.w  (A7), D0
		// 00264C56: C0EE 86BE                  mulu.w  (-$7942,A6), D0
		// 00264C5A: 2200                       move.l  D0, D1
		// 00264C5C: 41EE 86C0                  lea     (-$7940,A6), A0
		// 00264C60: 2008                       move.l  A0, D0
		// 00264C62: 6100 FE56                  bsr     $264aba
		// 00264C66: B142                       eor.w   D0, D2
		// 00264C68: 7000                       moveq   #$0, D0
		// 00264C6A: 3002                       move.w  D2, D0
		space->write_word(hack_base + 0x00, 0);
		space->write_word(hack_base + 0x02, 0);
		space->write_word(hack_base + 0x04, 0);
		space->write_word(hack_base + 0x06, 0);
		space->write_word(hack_base + 0x08, 0);
		space->write_word(hack_base + 0x0a, 0);
		space->write_word(hack_base + 0x0c, 0);
		space->write_word(hack_base + 0x0e, 0);
		space->write_word(hack_base + 0x10, 0);
		space->write_word(hack_base + 0x12, 0);
		space->write_word(hack_base + 0x14, 0);
		space->write_word(hack_base + 0x16, 0);
		space->write_word(hack_base + 0x18, 0);
		space->write_word(hack_base + 0x1a, 0);
		space->write_word(hack_base + 0x1c, 0);
		space->write_word(hack_base + 0x1e, 0);
		space->write_word(hack_base + 0x20, 0);
		space->write_word(hack_base + 0x22, 0);
		space->write_word(hack_base + 0x24, 0x203C);
		space->write_word(hack_base + 0x26, 0x0000);
		space->write_word(hack_base + 0x28, hack_value);

		if(hack2_addr)
		{
			//space->write_byte(hack2_addr, 1);
		}
	}
}

TIMER_CALLBACK( scc68070_timer0_callback )
{
    cdi_state *state = machine->driver_data<cdi_state>();
    scc68070_regs_t *scc68070 = &state->scc68070_regs;

    scc68070->timers.timer0 = scc68070->timers.reload_register;
    scc68070->timers.timer_status_register |= TSR_OV0;
    if(scc68070->picr1 & 7)
    {
        UINT8 interrupt = scc68070->picr1 & 7;
        scc68070->timers.timer_status_register |= TSR_OV0;
        if(interrupt)
        {
        	cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
        	cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
		}
    }

#if ENABLE_QUIZARD_HACK
	quizard_patch(machine);
#endif

    scc68070_set_timer_callback(&state->scc68070_regs, 0);
}

static void scc68070_uart_rx_check(running_machine *machine, scc68070_regs_t *scc68070)
{
	if((scc68070->uart.command_register & 3) == 1 &&
		scc68070->uart.receive_pointer > -1)
	{
		scc68070->uart.status_register |= USR_RXRDY;
		UINT32 div = 0x10000 >> ((scc68070->uart.clock_select >> 4) & 7);
		timer_adjust_oneshot(scc68070->uart.rx_timer, ATTOTIME_IN_HZ((49152000 / div) / 8), 0);
	}
	else
	{
		scc68070->uart.status_register &= ~USR_RXRDY;
		timer_adjust_oneshot(scc68070->uart.rx_timer, attotime_never, 0);
	}
}

static void scc68070_uart_tx_check(running_machine *machine, scc68070_regs_t *scc68070)
{
	if(((scc68070->uart.command_register >> 2) & 3) == 1 &&
		scc68070->uart.transmit_pointer > -1 &&
		attotime_compare(timer_timeleft(scc68070->uart.tx_timer), attotime_never) == 0)
	{
		UINT32 div = 0x10000 >> (scc68070->uart.clock_select & 7);
		timer_adjust_oneshot(scc68070->uart.tx_timer, ATTOTIME_IN_HZ((49152000 / div) / 8), 0);
	}
	else
	{
		timer_adjust_oneshot(scc68070->uart.tx_timer, attotime_never, 0);
	}
}

void scc68070_uart_rx(running_machine *machine, scc68070_regs_t *scc68070, UINT8 data)
{
	scc68070->uart.receive_pointer++;
	scc68070->uart.receive_buffer[scc68070->uart.receive_pointer] = data;
	scc68070->uart.status_register |= USR_RXRDY;
	scc68070_uart_rx_check(machine, scc68070);
}

void scc68070_uart_tx(running_machine *machine, scc68070_regs_t *scc68070, UINT8 data)
{
	scc68070->uart.transmit_pointer++;
	scc68070->uart.transmit_buffer[scc68070->uart.transmit_pointer] = data;
	scc68070->uart.status_register |= USR_TXRDY;
	scc68070_uart_tx_check(machine, scc68070);
}

TIMER_CALLBACK( scc68070_rx_callback )
{
	bool clear_int = false;
	cdi_state *state = machine->driver_data<cdi_state>();
	scc68070_regs_t *scc68070 = &state->scc68070_regs;

	if((scc68070->uart.command_register & 3) == 1)
	{
		scc68070->uart.receive_holding_register = scc68070->uart.receive_buffer[0];
		if(scc68070->uart.receive_pointer > -1)
		{
			verboselog(machine, 2, "scc68070_rx_callback: Receiving %02x\n", scc68070->uart.receive_holding_register);
			for(int index = 0; index < scc68070->uart.receive_pointer; index++)
			{
				scc68070->uart.receive_buffer[index] = scc68070->uart.receive_buffer[index + 1];
			}
			scc68070->uart.receive_pointer--;

			UINT8 interrupt = (scc68070->picr2 >> 4) & 7;
			if(interrupt)
			{
				cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
				cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
			}
			else
			{
				clear_int = true;
			}

			scc68070->uart.status_register |= USR_RXRDY;
			UINT32 div = 0x10000 >> ((scc68070->uart.clock_select >> 4) & 7);
			timer_adjust_oneshot(scc68070->uart.rx_timer, ATTOTIME_IN_HZ((49152000 / div) / 8), 0);
		}
		else
		{
			clear_int = true;
			scc68070->uart.status_register &= ~USR_RXRDY;
			timer_adjust_oneshot(scc68070->uart.rx_timer, attotime_never, 0);
		}
	}
	else
	{
		clear_int = true;
		scc68070->uart.status_register &= ~USR_RXRDY;
		timer_adjust_oneshot(scc68070->uart.rx_timer, attotime_never, 0);
	}

	if(clear_int)
	{
		UINT8 interrupt = scc68070->picr2 & 7;
		cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), CLEAR_LINE);
	}
}

static void scc68070_tx_empty(running_machine *machine, scc68070_regs_t *scc68070)
{
	UINT8 interrupt = scc68070->picr2 & 7;
	scc68070->uart.status_register |= USR_TXRDY;
	if(interrupt)
	{
		cpu_set_input_line_vector(machine->device("maincpu"), M68K_IRQ_1 + (interrupt - 1), 56 + interrupt);
		cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), ASSERT_LINE);
	}
}

TIMER_CALLBACK( scc68070_tx_callback )
{
	cdi_state *state = machine->driver_data<cdi_state>();
	scc68070_regs_t *scc68070 = &state->scc68070_regs;

	if(((scc68070->uart.command_register >> 2) & 3) == 1)
	{
		if(scc68070->uart.transmit_pointer > -1)
		{
			scc68070->uart.transmit_holding_register = scc68070->uart.transmit_buffer[0];
			verboselog(machine, 2, "scc68070_tx_callback: Transmitting %02x\n", scc68070->uart.transmit_holding_register);
			scc68070->uart.transmit_pointer--;
			for(int index = 0; index < scc68070->uart.transmit_pointer; index++)
			{
				scc68070->uart.transmit_buffer[index] = scc68070->uart.transmit_buffer[index+1];
			}

			UINT32 div = 0x10000 >> (scc68070->uart.clock_select & 7);
			timer_adjust_oneshot(scc68070->uart.tx_timer, ATTOTIME_IN_HZ((49152000 / div) / 8), 0);
		}
		else
		{
			timer_adjust_oneshot(scc68070->uart.tx_timer, attotime_never, 0);
			scc68070_tx_empty(machine, scc68070);
		}
	}
	else
	{
		timer_adjust_oneshot(scc68070->uart.tx_timer, attotime_never, 0);
		UINT8 interrupt = (scc68070->picr2 >> 4) & 7;
		if(interrupt)
		{
			cputag_set_input_line(machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), CLEAR_LINE);
		}
	}
}

READ16_HANDLER( scc68070_periphs_r )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    scc68070_regs_t *scc68070 = &state->scc68070_regs;

    switch(offset)
    {
        // Interupts: 80001001
        case 0x1000/2: // LIR priority level
            return scc68070->lir;

        // I2C interface: 80002001 to 80002009
        case 0x2000/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Data Register: %04x & %04x\n", scc68070->i2c.data_register, mem_mask);
            }
            return scc68070->i2c.data_register;
        case 0x2002/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Address Register: %04x & %04x\n", scc68070->i2c.address_register, mem_mask);
            }
            return scc68070->i2c.address_register;
        case 0x2004/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Status Register: %04x & %04x\n", scc68070->i2c.status_register, mem_mask);
            }
            return scc68070->i2c.status_register;
        case 0x2006/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Control Register: %04x & %04x\n", scc68070->i2c.control_register, mem_mask);
            }
            return scc68070->i2c.control_register;
        case 0x2008/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Clock Control Register: %04x & %04x\n", scc68070->i2c.clock_control_register, mem_mask);
            }
            return scc68070->i2c.clock_control_register;

        // UART interface: 80002011 to 8000201b
        case 0x2010/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Mode Register: %04x & %04x\n", scc68070->uart.mode_register, mem_mask);
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
            return scc68070->uart.mode_register | 0x20;
        case 0x2012/2:
        	scc68070->uart.status_register |= (1 << 1);
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Status Register: %04x & %04x\n", scc68070->uart.status_register, mem_mask);
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
            return scc68070->uart.status_register;
        case 0x2014/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Clock Select: %04x & %04x\n", scc68070->uart.clock_select, mem_mask);
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
            return scc68070->uart.clock_select | 0x08;
        case 0x2016/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Command Register: %02x & %04x\n", scc68070->uart.command_register, mem_mask);
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
            return scc68070->uart.command_register | 0x80;
        case 0x2018/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Transmit Holding Register: %02x & %04x\n", scc68070->uart.transmit_holding_register, mem_mask);
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
            return scc68070->uart.transmit_holding_register;
        case 0x201a/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: UART Receive Holding Register: %02x & %04x\n", scc68070->uart.receive_holding_register, mem_mask);
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
			}
			return scc68070->uart.receive_holding_register;

        // Timers: 80002020 to 80002029
        case 0x2020/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: Timer Control Register: %02x & %04x\n", scc68070->timers.timer_control_register, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 12, "scc68070_periphs_r: Timer Status Register: %02x & %04x\n", scc68070->timers.timer_status_register, mem_mask);
            }
            return (scc68070->timers.timer_status_register << 8) | scc68070->timers.timer_control_register;
        case 0x2022/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer Reload Register: %04x & %04x\n", scc68070->timers.reload_register, mem_mask);
            return scc68070->timers.reload_register;
        case 0x2024/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer 0: %04x & %04x\n", scc68070->timers.timer0, mem_mask);
            return scc68070->timers.timer0;
        case 0x2026/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer 1: %04x & %04x\n", scc68070->timers.timer1, mem_mask);
            return scc68070->timers.timer1;
        case 0x2028/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: Timer 2: %04x & %04x\n", scc68070->timers.timer2, mem_mask);
            return scc68070->timers.timer2;

        // PICR1: 80002045
        case 0x2044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: Peripheral Interrupt Control Register 1: %02x & %04x\n", scc68070->picr1, mem_mask);
            }
            return scc68070->picr1;

        // PICR2: 80002047
        case 0x2046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: Peripheral Interrupt Control Register 2: %02x & %04x\n", scc68070->picr2, mem_mask);
            }
            return scc68070->picr2 & 0x77;

        // DMA controller: 80004000 to 8000406d
        case 0x4000/2:
        case 0x4040/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Error Register: %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].channel_error, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Status Register: %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].channel_status, mem_mask);
            }
            return (scc68070->dma.channel[(offset - 0x2000) / 32].channel_status << 8) | scc68070->dma.channel[(offset - 0x2000) / 32].channel_error;
        case 0x4004/2:
        case 0x4044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Operation Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].operation_control, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Device Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].device_control, mem_mask);
            }
            return (scc68070->dma.channel[(offset - 0x2000) / 32].device_control << 8) | scc68070->dma.channel[(offset - 0x2000) / 32].operation_control;
        case 0x4006/2:
        case 0x4046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Channel Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].channel_control, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Sequence Control Register: %02x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].sequence_control, mem_mask);
            }
            return (scc68070->dma.channel[(offset - 0x2000) / 32].sequence_control << 8) | scc68070->dma.channel[(offset - 0x2000) / 32].channel_control;
        case 0x400a/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Memory Transfer Counter: %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].transfer_counter, mem_mask);
            return scc68070->dma.channel[(offset - 0x2000) / 32].transfer_counter;
        case 0x400c/2:
        case 0x404c/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Memory Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, (scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter >> 16), mem_mask);
            return (scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter >> 16);
        case 0x400e/2:
        case 0x404e/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Memory Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter, mem_mask);
            return scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter;
        case 0x4014/2:
        case 0x4054/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Device Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, (scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter >> 16), mem_mask);
            return (scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter >> 16);
        case 0x4016/2:
        case 0x4056/2:
            verboselog(space->machine, 2, "scc68070_periphs_r: DMA(%d) Device Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter, mem_mask);
            return scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter;

        // MMU: 80008000 to 8000807f
        case 0x8000/2:  // Status / Control register
            if(ACCESSING_BITS_0_7)
            {   // Control
                verboselog(space->machine, 2, "scc68070_periphs_r: MMU Control: %02x & %04x\n", scc68070->mmu.control, mem_mask);
                return scc68070->mmu.control;
            }   // Status
            else
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: MMU Status: %02x & %04x\n", scc68070->mmu.status, mem_mask);
                return scc68070->mmu.status;
            }
            break;
        case 0x8040/2:
        case 0x8048/2:
        case 0x8050/2:
        case 0x8058/2:
        case 0x8060/2:
        case 0x8068/2:
        case 0x8070/2:
        case 0x8078/2:  // Attributes (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d attributes: %04x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].attr, mem_mask);
            return scc68070->mmu.desc[(offset - 0x4020) / 4].attr;
        case 0x8042/2:
        case 0x804a/2:
        case 0x8052/2:
        case 0x805a/2:
        case 0x8062/2:
        case 0x806a/2:
        case 0x8072/2:
        case 0x807a/2:  // Segment Length (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d length: %04x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].length, mem_mask);
            return scc68070->mmu.desc[(offset - 0x4020) / 4].length;
        case 0x8044/2:
        case 0x804c/2:
        case 0x8054/2:
        case 0x805c/2:
        case 0x8064/2:
        case 0x806c/2:
        case 0x8074/2:
        case 0x807c/2:  // Segment Number (SD0-7, A0=1 only)
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d segment: %02x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].segment, mem_mask);
                return scc68070->mmu.desc[(offset - 0x4020) / 4].segment;
            }
            break;
        case 0x8046/2:
        case 0x804e/2:
        case 0x8056/2:
        case 0x805e/2:
        case 0x8066/2:
        case 0x806e/2:
        case 0x8076/2:
        case 0x807e/2:  // Base Address (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_r: MMU descriptor %d base: %04x & %04x\n", (offset - 0x4020) / 4, scc68070->mmu.desc[(offset - 0x4020) / 4].base, mem_mask);
            return scc68070->mmu.desc[(offset - 0x4020) / 4].base;
        default:
            verboselog(space->machine, 0, "scc68070_periphs_r: Unknown address: %04x & %04x\n", offset * 2, mem_mask);
            break;
    }

    return 0;
}

WRITE16_HANDLER( scc68070_periphs_w )
{
    cdi_state *state = space->machine->driver_data<cdi_state>();
    scc68070_regs_t *scc68070 = &state->scc68070_regs;

    switch(offset)
    {
        // Interupts: 80001001
        case 0x1000/2: // LIR priority level
            verboselog(space->machine, 2, "scc68070_periphs_w: LIR: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->lir);
            break;

        // I2C interface: 80002001 to 80002009
        case 0x2000/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Data Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.data_register = data & 0x00ff;
            }
            break;
        case 0x2002/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Address Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.address_register = data & 0x00ff;
            }
            break;
        case 0x2004/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Status Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.status_register = data & 0x00ff;
            }
            break;
        case 0x2006/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Control Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.control_register = data & 0x00ff;
            }
            break;
        case 0x2008/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: I2C Clock Control Register: %04x & %04x\n", data, mem_mask);
                scc68070->i2c.clock_control_register = data & 0x00ff;
            }
            break;

        // UART interface: 80002011 to 8000201b
        case 0x2010/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Mode Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.mode_register = data & 0x00ff;
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
            break;
        case 0x2012/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Status Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.status_register = data & 0x00ff;
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
            break;
        case 0x2014/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Clock Select: %04x & %04x\n", data, mem_mask);
                scc68070->uart.clock_select = data & 0x00ff;
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
            break;
        case 0x2016/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Command Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.command_register = data & 0x00ff;
                if(((scc68070->uart.command_register >> 2) & 3) == 1 &&
                	scc68070->uart.transmit_pointer == -1)
                {
					scc68070_tx_empty(space->machine, scc68070);
				}
				scc68070_uart_rx_check(space->machine, scc68070);
				scc68070_uart_tx_check(space->machine, scc68070);
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
            break;
        case 0x2018/2:
            if(ACCESSING_BITS_0_7)
            {
				static int count = 0;

                verboselog(space->machine, 2, "scc68070_periphs_w: UART Transmit Holding Register: %04x & %04x: %c\n", data, mem_mask, (data >= 0x20 && data < 0x7f) ? (data & 0x00ff) : ' ');
				scc68070_uart_tx(space->machine, scc68070, data & 0x00ff);
                scc68070->uart.transmit_holding_register = data & 0x00ff;

				printf("%02x ", scc68070->uart.transmit_holding_register);

                if((data & 0x00ff) == hack_ack)
                {
					count++;
					if(count == 2)
					{
						static unsigned char check_array[9] = { 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

						for(int index = 0; index < 9; index++)
						{
							scc68070_uart_rx(space->machine, scc68070, check_array[index]);
							if(index > 0)
							{
								check_array[index]++;
							}
						}
						count = 0;

						hack_active = true;
						quizard_patch(space->machine);
					}
				}
				else
				{
					count = 0;
				}
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
            break;
        case 0x201a/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: UART Receive Holding Register: %04x & %04x\n", data, mem_mask);
                scc68070->uart.receive_holding_register = data & 0x00ff;
            }
            else
            {
            	verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
			}
            break;

        // Timers: 80002020 to 80002029
        case 0x2020/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: Timer Control Register: %04x & %04x\n", data, mem_mask);
                scc68070->timers.timer_control_register = data & 0x00ff;
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 12, "scc68070_periphs_w: Timer Status Register: %04x & %04x\n", data, mem_mask);
                scc68070->timers.timer_status_register &= ~(data >> 8);
                if(!scc68070->timers.timer_status_register)
                {
                    UINT8 interrupt = scc68070->picr1 & 7;
                    cputag_set_input_line(space->machine, "maincpu", M68K_IRQ_1 + (interrupt - 1), CLEAR_LINE);
                }
            }
            break;
        case 0x2022/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer Reload Register: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.reload_register);
            break;
        case 0x2024/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer 0: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.timer0);
            scc68070_set_timer_callback(&state->scc68070_regs, 0);
            break;
        case 0x2026/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer 1: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.timer1);
            break;
        case 0x2028/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: Timer 2: %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&scc68070->timers.timer2);
            break;

        // PICR1: 80002045
        case 0x2044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: Peripheral Interrupt Control Register 1: %04x & %04x\n", data, mem_mask);
                scc68070->picr1 = data & 0x00ff;
            }
            break;

        // PICR2: 80002047
        case 0x2046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: Peripheral Interrupt Control Register 2: %04x & %04x\n", data, mem_mask);
                scc68070->picr2 = data & 0x00ff;
            }
            break;

        // DMA controller: 80004000 to 8000406d
        case 0x4000/2:
        case 0x4040/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Error (invalid): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Status: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].channel_status &= ~(data & 0xb0);
            }
            break;
        case 0x4004/2:
        case 0x4044/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Operation Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].operation_control = data & 0x00ff;
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Device Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].device_control = data >> 8;
            }
            break;
        case 0x4006/2:
        case 0x4046/2:
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Channel Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].channel_control = data & 0x007f;
                if(data & CCR_SO)
                {
                    scc68070->dma.channel[(offset - 0x2000) / 32].channel_status |= CSR_COC;
                }
            }
            if(ACCESSING_BITS_8_15)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Sequence Control Register: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
                scc68070->dma.channel[(offset - 0x2000) / 32].sequence_control = data >> 8;
            }
            break;
        case 0x400a/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Memory Transfer Counter: %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            COMBINE_DATA(&scc68070->dma.channel[(offset - 0x2000) / 32].transfer_counter);
            break;
        case 0x400c/2:
        case 0x404c/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Memory Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter &= ~(mem_mask << 16);
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter |= data << 16;
            break;
        case 0x400e/2:
        case 0x404e/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Memory Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter &= ~mem_mask;
            scc68070->dma.channel[(offset - 0x2000) / 32].memory_address_counter |= data;
            break;
        case 0x4014/2:
        case 0x4054/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Device Address Counter (High Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter &= ~(mem_mask << 16);
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter |= data << 16;
            break;
        case 0x4016/2:
        case 0x4056/2:
            verboselog(space->machine, 2, "scc68070_periphs_w: DMA(%d) Device Address Counter (Low Word): %04x & %04x\n", (offset - 0x2000) / 32, data, mem_mask);
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter &= ~mem_mask;
            scc68070->dma.channel[(offset - 0x2000) / 32].device_address_counter |= data;
            break;

        // MMU: 80008000 to 8000807f
        case 0x8000/2:  // Status / Control register
            if(ACCESSING_BITS_0_7)
            {   // Control
                verboselog(space->machine, 2, "scc68070_periphs_w: MMU Control: %04x & %04x\n", data, mem_mask);
                scc68070->mmu.control = data & 0x00ff;
            }   // Status
            else
            {
                verboselog(space->machine, 0, "scc68070_periphs_w: MMU Status (invalid): %04x & %04x\n", data, mem_mask);
            }
            break;
        case 0x8040/2:
        case 0x8048/2:
        case 0x8050/2:
        case 0x8058/2:
        case 0x8060/2:
        case 0x8068/2:
        case 0x8070/2:
        case 0x8078/2:  // Attributes (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d attributes: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
            COMBINE_DATA(&scc68070->mmu.desc[(offset - 0x4020) / 4].attr);
            break;
        case 0x8042/2:
        case 0x804a/2:
        case 0x8052/2:
        case 0x805a/2:
        case 0x8062/2:
        case 0x806a/2:
        case 0x8072/2:
        case 0x807a/2:  // Segment Length (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d length: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
            COMBINE_DATA(&scc68070->mmu.desc[(offset - 0x4020) / 4].length);
            break;
        case 0x8044/2:
        case 0x804c/2:
        case 0x8054/2:
        case 0x805c/2:
        case 0x8064/2:
        case 0x806c/2:
        case 0x8074/2:
        case 0x807c/2:  // Segment Number (SD0-7, A0=1 only)
            if(ACCESSING_BITS_0_7)
            {
                verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d segment: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
                scc68070->mmu.desc[(offset - 0x4020) / 4].segment = data & 0x00ff;
            }
            break;
        case 0x8046/2:
        case 0x804e/2:
        case 0x8056/2:
        case 0x805e/2:
        case 0x8066/2:
        case 0x806e/2:
        case 0x8076/2:
        case 0x807e/2:  // Base Address (SD0-7)
            verboselog(space->machine, 2, "scc68070_periphs_w: MMU descriptor %d base: %04x & %04x\n", (offset - 0x4020) / 4, data, mem_mask);
            COMBINE_DATA(&scc68070->mmu.desc[(offset - 0x4020) / 4].base);
            break;
        default:
            verboselog(space->machine, 0, "scc68070_periphs_w: Unknown address: %04x = %04x & %04x\n", offset * 2, data, mem_mask);
            break;
    }
}

void scc68070_init(running_machine *machine, scc68070_regs_t *scc68070)
{
    int index = 0;
    scc68070->lir = 0;

    scc68070->picr1 = 0;
    scc68070->picr2 = 0;

    scc68070->i2c.data_register = 0;
    scc68070->i2c.address_register = 0;
    scc68070->i2c.status_register = 0;
    scc68070->i2c.control_register = 0;
    scc68070->i2c.clock_control_register = 0;

    scc68070->uart.mode_register = 0;
    scc68070->uart.status_register = 0;
    scc68070->uart.clock_select = 0;
    scc68070->uart.command_register = 0;
    scc68070->uart.transmit_holding_register = 0;
    scc68070->uart.receive_holding_register = 0;
	scc68070->uart.receive_pointer = -1;
	scc68070->uart.transmit_pointer = -1;

    scc68070->timers.timer_status_register = 0;
    scc68070->timers.timer_control_register = 0;
    scc68070->timers.reload_register = 0;
    scc68070->timers.timer0 = 0;
    scc68070->timers.timer1 = 0;
    scc68070->timers.timer2 = 0;

    for(index = 0; index < 2; index++)
    {
        scc68070->dma.channel[index].channel_status = 0;
        scc68070->dma.channel[index].channel_error = 0;
        scc68070->dma.channel[index].device_control = 0;
        scc68070->dma.channel[index].operation_control = 0;
        scc68070->dma.channel[index].sequence_control = 0;
        scc68070->dma.channel[index].channel_control = 0;
        scc68070->dma.channel[index].transfer_counter = 0;
        scc68070->dma.channel[index].memory_address_counter = 0;
        scc68070->dma.channel[index].device_address_counter = 0;
    }

    scc68070->mmu.status = 0;
    scc68070->mmu.control = 0;
    for(index = 0; index < 8; index++)
    {
        scc68070->mmu.desc[index].attr = 0;
        scc68070->mmu.desc[index].length = 0;
        scc68070->mmu.desc[index].segment = 0;
        scc68070->mmu.desc[index].base = 0;
    }
}

void scc68070_register_globals(running_machine *machine, scc68070_regs_t *scc68070)
{
    state_save_register_global(machine, scc68070->lir);

    state_save_register_global(machine, scc68070->picr1);
    state_save_register_global(machine, scc68070->picr2);

    state_save_register_global(machine, scc68070->i2c.data_register);
    state_save_register_global(machine, scc68070->i2c.address_register);
    state_save_register_global(machine, scc68070->i2c.status_register);
    state_save_register_global(machine, scc68070->i2c.control_register);
    state_save_register_global(machine, scc68070->i2c.clock_control_register);

    state_save_register_global(machine, scc68070->uart.mode_register);
    state_save_register_global(machine, scc68070->uart.status_register);
    state_save_register_global(machine, scc68070->uart.clock_select);
    state_save_register_global(machine, scc68070->uart.command_register);
    state_save_register_global(machine, scc68070->uart.transmit_holding_register);
    state_save_register_global(machine, scc68070->uart.receive_holding_register);

    state_save_register_global(machine, scc68070->timers.timer_status_register);
    state_save_register_global(machine, scc68070->timers.timer_control_register);
    state_save_register_global(machine, scc68070->timers.reload_register);
    state_save_register_global(machine, scc68070->timers.timer0);
    state_save_register_global(machine, scc68070->timers.timer1);
    state_save_register_global(machine, scc68070->timers.timer2);

    state_save_register_global(machine, scc68070->dma.channel[0].channel_status);
    state_save_register_global(machine, scc68070->dma.channel[0].channel_error);
    state_save_register_global(machine, scc68070->dma.channel[0].device_control);
    state_save_register_global(machine, scc68070->dma.channel[0].operation_control);
    state_save_register_global(machine, scc68070->dma.channel[0].sequence_control);
    state_save_register_global(machine, scc68070->dma.channel[0].channel_control);
    state_save_register_global(machine, scc68070->dma.channel[0].transfer_counter);
    state_save_register_global(machine, scc68070->dma.channel[0].memory_address_counter);
    state_save_register_global(machine, scc68070->dma.channel[0].device_address_counter);
    state_save_register_global(machine, scc68070->dma.channel[1].channel_status);
    state_save_register_global(machine, scc68070->dma.channel[1].channel_error);
    state_save_register_global(machine, scc68070->dma.channel[1].device_control);
    state_save_register_global(machine, scc68070->dma.channel[1].operation_control);
    state_save_register_global(machine, scc68070->dma.channel[1].sequence_control);
    state_save_register_global(machine, scc68070->dma.channel[1].channel_control);
    state_save_register_global(machine, scc68070->dma.channel[1].transfer_counter);
    state_save_register_global(machine, scc68070->dma.channel[1].memory_address_counter);
    state_save_register_global(machine, scc68070->dma.channel[1].device_address_counter);

    state_save_register_global(machine, scc68070->mmu.status);
    state_save_register_global(machine, scc68070->mmu.control);
    state_save_register_global(machine, scc68070->mmu.desc[0].attr);
    state_save_register_global(machine, scc68070->mmu.desc[0].length);
    state_save_register_global(machine, scc68070->mmu.desc[0].segment);
    state_save_register_global(machine, scc68070->mmu.desc[0].base);
    state_save_register_global(machine, scc68070->mmu.desc[1].attr);
    state_save_register_global(machine, scc68070->mmu.desc[1].length);
    state_save_register_global(machine, scc68070->mmu.desc[1].segment);
    state_save_register_global(machine, scc68070->mmu.desc[1].base);
    state_save_register_global(machine, scc68070->mmu.desc[2].attr);
    state_save_register_global(machine, scc68070->mmu.desc[2].length);
    state_save_register_global(machine, scc68070->mmu.desc[2].segment);
    state_save_register_global(machine, scc68070->mmu.desc[2].base);
    state_save_register_global(machine, scc68070->mmu.desc[3].attr);
    state_save_register_global(machine, scc68070->mmu.desc[3].length);
    state_save_register_global(machine, scc68070->mmu.desc[3].segment);
    state_save_register_global(machine, scc68070->mmu.desc[3].base);
    state_save_register_global(machine, scc68070->mmu.desc[4].attr);
    state_save_register_global(machine, scc68070->mmu.desc[4].length);
    state_save_register_global(machine, scc68070->mmu.desc[4].segment);
    state_save_register_global(machine, scc68070->mmu.desc[4].base);
    state_save_register_global(machine, scc68070->mmu.desc[5].attr);
    state_save_register_global(machine, scc68070->mmu.desc[5].length);
    state_save_register_global(machine, scc68070->mmu.desc[5].segment);
    state_save_register_global(machine, scc68070->mmu.desc[5].base);
    state_save_register_global(machine, scc68070->mmu.desc[6].attr);
    state_save_register_global(machine, scc68070->mmu.desc[6].length);
    state_save_register_global(machine, scc68070->mmu.desc[6].segment);
    state_save_register_global(machine, scc68070->mmu.desc[6].base);
    state_save_register_global(machine, scc68070->mmu.desc[7].attr);
    state_save_register_global(machine, scc68070->mmu.desc[7].length);
    state_save_register_global(machine, scc68070->mmu.desc[7].segment);
    state_save_register_global(machine, scc68070->mmu.desc[7].base);

	scc68070->timers.timer0_timer = timer_alloc(machine, scc68070_timer0_callback, 0);
	timer_adjust_oneshot(scc68070->timers.timer0_timer, attotime_never, 0);

	scc68070->uart.rx_timer = timer_alloc(machine, scc68070_rx_callback, 0);
	timer_adjust_oneshot(scc68070->uart.rx_timer, attotime_never, 0);

	scc68070->uart.tx_timer = timer_alloc(machine, scc68070_tx_callback, 0);
	timer_adjust_oneshot(scc68070->uart.tx_timer, attotime_never, 0);
}

#if ENABLE_UART_PRINTING
READ16_HANDLER( uart_loopback_enable )
{
    return 0x1234;
}
#endif
