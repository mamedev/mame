/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/


#define LOG_COMM			(0)
#define LOG_32031_IOPORTS	(0)


#include "driver.h"
#include "cpu/tms32031/tms32031.h"
#include "sound/dmadac.h"
#include "cage.h"



/*************************************
 *
 *  Constants/macros
 *
 *************************************/

#define DAC_BUFFER_CHANNELS		4
#define STACK_SOUND_BUFSIZE		(1024)



/*************************************
 *
 *  Statics
 *
 *************************************/

static const device_config *cage_cpu;
static attotime cage_cpu_h1_clock_period;

static UINT8 cpu_to_cage_ready;
static UINT8 cage_to_cpu_ready;

static void (*cage_irqhandler)(running_machine *, int);

static attotime serial_period_per_word;

static UINT8 dma_enabled;
static UINT8 dma_timer_enabled;
static const device_config *dma_timer;

static UINT8 cage_timer_enabled[2];
static const device_config *timer[2];

static UINT32 *tms32031_io_regs;

static UINT16 cage_from_main;
static UINT16 cage_control;

static UINT32 *speedup_ram;

static const device_config *dmadac[DAC_BUFFER_CHANNELS];



/*************************************
 *
 *  I/O port definitions
 *
 *************************************/

#define DMA_GLOBAL_CTL			0x00
#define DMA_SOURCE_ADDR			0x04
#define DMA_DEST_ADDR			0x06
#define DMA_TRANSFER_COUNT		0x08

#define TIMER0_GLOBAL_CTL		0x20
#define TIMER0_COUNTER			0x24
#define TIMER0_PERIOD			0x28

#define TIMER1_GLOBAL_CTL		0x30
#define TIMER1_COUNTER			0x34
#define TIMER1_PERIOD			0x38

#define SPORT_GLOBAL_CTL		0x40
#define SPORT_TX_CTL			0x42
#define SPORT_RX_CTL			0x43
#define SPORT_TIMER_CTL			0x44
#define SPORT_TIMER_COUNTER		0x45
#define SPORT_TIMER_PERIOD		0x46
#define SPORT_DATA_TX			0x48
#define SPORT_DATA_RX			0x4c

static const char *const register_names[] =
{
	"TMS32031-DMA global control", NULL, NULL, NULL,
	"TMS32031-DMA source address", NULL, "TMS32031-DMA destination address", NULL,
	"TMS32031-DMA transfer counter", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,

	"TMS32031-Timer 0 global control", NULL, NULL, NULL,
	"TMS32031-Timer 0 counter", NULL, NULL, NULL,
	"TMS32031-Timer 0 period", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,

	"TMS32031-Timer 1 global control", NULL, NULL, NULL,
	"TMS32031-Timer 1 counter", NULL, NULL, NULL,
	"TMS32031-Timer 1 period", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,

	"TMS32031-Serial port global control", NULL, "TMS32031-Serial port TX control", "TMS32031-Serial port RX control",
	"TMS32031-Serial port timer control", "TMS32031-Serial port timer counter", "TMS32031-Serial port timer period", NULL,
	"TMS32031-Serial port data TX", NULL, NULL, NULL,
	"TMS32031-Serial port data RX", NULL, NULL, NULL,

	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL,
	"TMS32031-Primary bus control", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL
};



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( dma_timer_callback );
static TIMER_DEVICE_CALLBACK( cage_timer_callback );
static void update_timer(int which);
static WRITE32_HANDLER( speedup_w );



/*************************************
 *
 *  Initialization
 *
 *************************************/

void cage_init(running_machine *machine, offs_t speedup)
{
	attotime cage_cpu_clock_period;
	int chan;

	cage_irqhandler = NULL;

	memory_set_bankptr(machine, "bank10", memory_region(machine, "cageboot"));
	memory_set_bankptr(machine, "bank11", memory_region(machine, "cage"));

	cage_cpu = cputag_get_cpu(machine, "cage");
	cage_cpu_clock_period = ATTOTIME_IN_HZ(cpu_get_clock(cage_cpu));
	cage_cpu_h1_clock_period = attotime_mul(cage_cpu_clock_period, 2);

	dma_timer = devtag_get_device(machine, "cage_dma_timer");
	timer[0] = devtag_get_device(machine, "cage_timer0");
	timer[1] = devtag_get_device(machine, "cage_timer1");

	if (speedup)
		speedup_ram = memory_install_write32_handler(cpu_get_address_space(cage_cpu, ADDRESS_SPACE_PROGRAM), speedup, speedup, 0, 0, speedup_w);

	for (chan = 0; chan < DAC_BUFFER_CHANNELS; chan++)
	{
		char buffer[10];
		sprintf(buffer, "dac%d", chan + 1);
		dmadac[chan] = devtag_get_device(machine, buffer);
	}

	state_save_register_global(machine, cpu_to_cage_ready);
	state_save_register_global(machine, cage_to_cpu_ready);
	state_save_register_global(machine, serial_period_per_word.seconds);
	state_save_register_global(machine, serial_period_per_word.attoseconds);
	state_save_register_global(machine, dma_enabled);
	state_save_register_global(machine, dma_timer_enabled);
	state_save_register_global_array(machine, cage_timer_enabled);
	state_save_register_global(machine, cage_from_main);
	state_save_register_global(machine, cage_control);
}


void cage_set_irq_handler(void (*irqhandler)(running_machine *, int))
{
	cage_irqhandler = irqhandler;
}


void cage_reset_w(int state)
{
	if (state)
		cage_control_w(cage_cpu->machine, 0);
	cpu_set_input_line(cage_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  DMA timers
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( dma_timer_callback )
{
	/* if we weren't enabled, don't do anything, just shut ourself off */
	if (!dma_enabled)
	{
		if (dma_timer_enabled)
		{
			timer_device_adjust_oneshot(timer, attotime_never, 0);
			dma_timer_enabled = 0;
		}
		return;
	}

	/* set the final count to 0 and the source address to the final address */
	tms32031_io_regs[DMA_TRANSFER_COUNT] = 0;
	tms32031_io_regs[DMA_SOURCE_ADDR] = param;

	/* set the interrupt */
	cpu_set_input_line(cage_cpu, TMS32031_DINT, ASSERT_LINE);
	dma_enabled = 0;
}


static void update_dma_state(const address_space *space)
{
	/* determine the new enabled state */
	int enabled = ((tms32031_io_regs[DMA_GLOBAL_CTL] & 3) == 3) && (tms32031_io_regs[DMA_TRANSFER_COUNT] != 0);

	/* see if we turned on */
	if (enabled && !dma_enabled)
	{
		INT16 sound_data[STACK_SOUND_BUFSIZE];
		UINT32 addr, inc;
		int i;

		/* make sure our assumptions are correct */
		if (tms32031_io_regs[DMA_DEST_ADDR] != 0x808048)
			logerror("CAGE DMA: unexpected dest address %08X!\n", tms32031_io_regs[DMA_DEST_ADDR]);
		if ((tms32031_io_regs[DMA_GLOBAL_CTL] & 0xfef) != 0xe03)
			logerror("CAGE DMA: unexpected transfer params %08X!\n", tms32031_io_regs[DMA_GLOBAL_CTL]);

		/* do the DMA up front */
		addr = tms32031_io_regs[DMA_SOURCE_ADDR];
		inc = (tms32031_io_regs[DMA_GLOBAL_CTL] >> 4) & 1;
		for (i = 0; i < tms32031_io_regs[DMA_TRANSFER_COUNT]; i++)
		{
			sound_data[i % STACK_SOUND_BUFSIZE] = memory_read_dword(space, addr * 4);
			addr += inc;
			if (i % STACK_SOUND_BUFSIZE == STACK_SOUND_BUFSIZE - 1)
				dmadac_transfer(&dmadac[0], DAC_BUFFER_CHANNELS, 1, DAC_BUFFER_CHANNELS, STACK_SOUND_BUFSIZE / DAC_BUFFER_CHANNELS, sound_data);
		}
		if (tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE != 0)
			dmadac_transfer(&dmadac[0], DAC_BUFFER_CHANNELS, 1, DAC_BUFFER_CHANNELS, (tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE) / DAC_BUFFER_CHANNELS, sound_data);

		/* compute the time of the interrupt and set the timer */
		if (!dma_timer_enabled)
		{
			attotime period = attotime_mul(serial_period_per_word, tms32031_io_regs[DMA_TRANSFER_COUNT]);
			timer_device_adjust_periodic(dma_timer, period, addr, period);
			dma_timer_enabled = 1;
		}
	}

	/* see if we turned off */
	else if (!enabled && dma_enabled)
	{
		timer_device_adjust_oneshot(dma_timer, attotime_never, 0);
		dma_timer_enabled = 0;
	}

	/* set the new state */
	dma_enabled = enabled;
}



/*************************************
 *
 *  Internal timers
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( cage_timer_callback )
{
	int which = param;

	/* set the interrupt */
	cpu_set_input_line(cage_cpu, TMS32031_TINT0 + which, ASSERT_LINE);
	cage_timer_enabled[which] = 0;
	update_timer(which);
}


static void update_timer(int which)
{
	/* determine the new enabled state */
	int base = 0x10 * which;
	int enabled = ((tms32031_io_regs[base + TIMER0_GLOBAL_CTL] & 0xc0) == 0xc0);

	/* see if we turned on */
	if (enabled && !cage_timer_enabled[which])
	{
		attotime period = attotime_mul(cage_cpu_h1_clock_period, 2 * tms32031_io_regs[base + TIMER0_PERIOD]);

		/* make sure our assumptions are correct */
		if (tms32031_io_regs[base + TIMER0_GLOBAL_CTL] != 0x2c1)
			logerror("CAGE TIMER%d: unexpected timer config %08X!\n", which, tms32031_io_regs[base + TIMER0_GLOBAL_CTL]);

		timer_device_adjust_oneshot(timer[which], period, which);
	}

	/* see if we turned off */
	else if (!enabled && cage_timer_enabled[which])
	{
		timer_device_adjust_oneshot(timer[which], attotime_never, which);
	}

	/* set the new state */
	cage_timer_enabled[which] = enabled;
}



/*************************************
 *
 *  Serial port I/O
 *
 *************************************/

static void update_serial(running_machine *machine)
{
	attotime serial_clock_period, bit_clock_period;
	UINT32 freq;

	/* we start out at half the H1 frequency (or 2x the H1 period) */
	serial_clock_period = attotime_mul(cage_cpu_h1_clock_period, 2);

	/* if we're in clock mode, muliply by another factor of 2 */
	if (tms32031_io_regs[SPORT_GLOBAL_CTL] & 4)
		serial_clock_period = attotime_mul(serial_clock_period, 2);

	/* now multiply by the timer period */
	bit_clock_period = attotime_mul(serial_clock_period, (tms32031_io_regs[SPORT_TIMER_PERIOD] & 0xffff));

	/* and times the number of bits per sample */
	serial_period_per_word = attotime_mul(bit_clock_period, 8 * (((tms32031_io_regs[SPORT_GLOBAL_CTL] >> 18) & 3) + 1));

	/* compute the step value to stretch this to the sample_rate */
	freq = ATTOSECONDS_TO_HZ(serial_period_per_word.attoseconds) / DAC_BUFFER_CHANNELS;
	if (freq > 0 && freq < 100000)
	{
		dmadac_set_frequency(&dmadac[0], DAC_BUFFER_CHANNELS, freq);
		dmadac_enable(&dmadac[0], DAC_BUFFER_CHANNELS, 1);
	}
}



/*************************************
 *
 *  Master read/write
 *
 *************************************/

static READ32_HANDLER( tms32031_io_r )
{
	UINT16 result = tms32031_io_regs[offset];

	switch (offset)
	{
		case DMA_GLOBAL_CTL:
			result = (result & ~0xc) | (dma_enabled ? 0xc : 0x0);
			break;
	}

	if (LOG_32031_IOPORTS)
		logerror("CAGE:%06X:%s read -> %08X\n", cpu_get_pc(space->cpu), register_names[offset & 0x7f], result);
	return result;
}


static WRITE32_HANDLER( tms32031_io_w )
{
	COMBINE_DATA(&tms32031_io_regs[offset]);

	if (LOG_32031_IOPORTS)
		logerror("CAGE:%06X:%s write = %08X\n", cpu_get_pc(space->cpu), register_names[offset & 0x7f], tms32031_io_regs[offset]);

	switch (offset)
	{
		case DMA_GLOBAL_CTL:
		case DMA_SOURCE_ADDR:
		case DMA_DEST_ADDR:
		case DMA_TRANSFER_COUNT:
			update_dma_state(space);
			break;

		case TIMER0_GLOBAL_CTL:
		case TIMER0_COUNTER:
		case TIMER0_PERIOD:
			update_timer(0);
			break;

		case TIMER1_GLOBAL_CTL:
		case TIMER1_COUNTER:
		case TIMER1_PERIOD:
			update_timer(1);
			break;

		case SPORT_TX_CTL:
		case SPORT_RX_CTL:
		case SPORT_TIMER_COUNTER:
		case SPORT_DATA_RX:
			break;

		case SPORT_DATA_TX:
#if (DAC_BUFFER_CHANNELS == 4)
			if ((int)ATTOSECONDS_TO_HZ(serial_period_per_word.attoseconds) == 22050*4 && (tms32031_io_regs[SPORT_RX_CTL] & 0xff) == 0x62)
				tms32031_io_regs[SPORT_RX_CTL] ^= 0x800;
#endif
			break;

		case SPORT_GLOBAL_CTL:
		case SPORT_TIMER_CTL:
		case SPORT_TIMER_PERIOD:
			update_serial(space->machine);
			break;
	}
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

static void update_control_lines(running_machine *machine)
{
	int val;

	/* set the IRQ to the main CPU */
	if (cage_irqhandler)
	{
		int reason = 0;

		if ((cage_control & 3) == 3 && !cpu_to_cage_ready)
			reason |= CAGE_IRQ_REASON_BUFFER_EMPTY;
		if ((cage_control & 2) && cage_to_cpu_ready)
			reason |= CAGE_IRQ_REASON_DATA_READY;

		(*cage_irqhandler)(machine, reason);
	}

	/* set the IOF input lines */
	val = cpu_get_reg(cage_cpu, TMS32031_IOF);
	val &= ~0x88;
	if (cpu_to_cage_ready) val |= 0x08;
	if (cage_to_cpu_ready) val |= 0x80;
	cpu_set_reg(cage_cpu, TMS32031_IOF, val);
}


static READ32_HANDLER( cage_from_main_r )
{
	if (LOG_COMM)
		logerror("%06X:CAGE read command = %04X\n", cpu_get_pc(space->cpu), cage_from_main);
	cpu_to_cage_ready = 0;
	update_control_lines(space->machine);
	cpu_set_input_line(cage_cpu, TMS32031_IRQ0, CLEAR_LINE);
	return cage_from_main;
}


static WRITE32_HANDLER( cage_from_main_ack_w )
{
	if (LOG_COMM)
		logerror("%06X:CAGE ack command = %04X\n", cpu_get_pc(space->cpu), cage_from_main);
}


static WRITE32_HANDLER( cage_to_main_w )
{
	if (LOG_COMM)
		logerror("%06X:Data from CAGE = %04X\n", cpu_get_pc(space->cpu), data);
	soundlatch_word_w(space, 0, data, mem_mask);
	cage_to_cpu_ready = 1;
	update_control_lines(space->machine);
}


static READ32_HANDLER( cage_io_status_r )
{
	int result = 0;
	if (cpu_to_cage_ready)
		result |= 0x80;
	if (!cage_to_cpu_ready)
		result |= 0x40;
	return result;
}


UINT16 main_from_cage_r(const address_space *space)
{
	if (LOG_COMM)
		logerror("%s:main read data = %04X\n", cpuexec_describe_context(space->machine), soundlatch_word_r(space, 0, 0));
	cage_to_cpu_ready = 0;
	update_control_lines(space->machine);
	return soundlatch_word_r(space, 0, 0xffff);
}


static TIMER_CALLBACK( deferred_cage_w )
{
	cage_from_main = param;
	cpu_to_cage_ready = 1;
	update_control_lines(machine);
	cpu_set_input_line(cage_cpu, TMS32031_IRQ0, ASSERT_LINE);
}


void main_to_cage_w(UINT16 data)
{
	running_machine *machine = cage_cpu->machine;
	if (LOG_COMM)
		logerror("%s:Command to CAGE = %04X\n", cpuexec_describe_context(machine), data);
	timer_call_after_resynch(machine, NULL, data, deferred_cage_w);
}


UINT16 cage_control_r(void)
{
	UINT16 result = 0;

	if (cpu_to_cage_ready)
		result |= 2;
	if (cage_to_cpu_ready)
		result |= 1;

	return result;
}


void cage_control_w(running_machine *machine, UINT16 data)
{
	cage_control = data;

	/* CPU is reset if both control lines are 0 */
	if (!(cage_control & 3))
	{
		cpu_set_input_line(cage_cpu, INPUT_LINE_RESET, ASSERT_LINE);

		dma_enabled = 0;
		dma_timer_enabled = 0;
		timer_device_adjust_oneshot(dma_timer, attotime_never, 0);

		cage_timer_enabled[0] = 0;
		cage_timer_enabled[1] = 0;
		timer_device_adjust_oneshot(timer[0], attotime_never, 0);
		timer_device_adjust_oneshot(timer[1], attotime_never, 0);

		memset(tms32031_io_regs, 0, 0x60 * 4);

		cpu_to_cage_ready = 0;
		cage_to_cpu_ready = 0;
	}
	else
		cpu_set_input_line(cage_cpu, INPUT_LINE_RESET, CLEAR_LINE);

	/* update the control state */
	update_control_lines(machine);
}



/*************************************
 *
 *  Speedups
 *
 *************************************/

static WRITE32_HANDLER( speedup_w )
{
	cpu_eat_cycles(space->cpu, 100);
	COMBINE_DATA(&speedup_ram[offset]);
}



/*************************************
 *
 *  CPU memory map & config
 *
 *************************************/

static const tms32031_config cage_config =
{
	0x400000
};


static ADDRESS_MAP_START( cage_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM
	AM_RANGE(0x200000, 0x200000) AM_WRITENOP
	AM_RANGE(0x400000, 0x47ffff) AM_ROMBANK("bank10")
	AM_RANGE(0x808000, 0x8080ff) AM_READWRITE(tms32031_io_r, tms32031_io_w) AM_BASE(&tms32031_io_regs)
	AM_RANGE(0x809800, 0x809fff) AM_RAM
	AM_RANGE(0xa00000, 0xa00000) AM_READWRITE(cage_from_main_r, cage_to_main_w)
	AM_RANGE(0xc00000, 0xffffff) AM_ROMBANK("bank11")
ADDRESS_MAP_END


static ADDRESS_MAP_START( cage_map_seattle, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM
	AM_RANGE(0x200000, 0x200000) AM_WRITENOP
	AM_RANGE(0x400000, 0x47ffff) AM_ROMBANK("bank10")
	AM_RANGE(0x808000, 0x8080ff) AM_READWRITE(tms32031_io_r, tms32031_io_w) AM_BASE(&tms32031_io_regs)
	AM_RANGE(0x809800, 0x809fff) AM_RAM
	AM_RANGE(0xa00000, 0xa00000) AM_READWRITE(cage_from_main_r, cage_from_main_ack_w)
	AM_RANGE(0xa00001, 0xa00001) AM_WRITE(cage_to_main_w)
	AM_RANGE(0xa00003, 0xa00003) AM_READ(cage_io_status_r)
	AM_RANGE(0xc00000, 0xffffff) AM_ROMBANK("bank11")
ADDRESS_MAP_END



/*************************************
 *
 *  CAGE machine driver
 *
 *************************************/

MACHINE_DRIVER_START( cage )

	/* basic machine hardware */
	MDRV_CPU_ADD("cage", TMS32031, 33868800)
	MDRV_CPU_CONFIG(cage_config)
	MDRV_CPU_PROGRAM_MAP(cage_map)

	MDRV_TIMER_ADD("cage_dma_timer", dma_timer_callback)
	MDRV_TIMER_ADD("cage_timer0", cage_timer_callback)
	MDRV_TIMER_ADD("cage_timer1", cage_timer_callback)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

#if (DAC_BUFFER_CHANNELS == 4)
	MDRV_SOUND_ADD("dac1", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MDRV_SOUND_ADD("dac2", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MDRV_SOUND_ADD("dac3", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MDRV_SOUND_ADD("dac4", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
#else
	MDRV_SOUND_ADD("dac1", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MDRV_SOUND_ADD("dac2", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
#endif
MACHINE_DRIVER_END


MACHINE_DRIVER_START( cage_seattle )
	MDRV_IMPORT_FROM(cage)

	MDRV_CPU_MODIFY("cage")
	MDRV_CPU_PROGRAM_MAP(cage_map_seattle)
MACHINE_DRIVER_END
