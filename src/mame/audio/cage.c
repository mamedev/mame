/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/


#define LOG_COMM			(0)
#define LOG_32031_IOPORTS	(0)


#include "emu.h"
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


typedef struct
{
	cpu_device *cpu;
	attotime cpu_h1_clock_period;

	UINT8 cpu_to_cage_ready;
	UINT8 cage_to_cpu_ready;

	void (*irqhandler)(running_machine &, int);

	attotime serial_period_per_word;

	UINT8 dma_enabled;
	UINT8 dma_timer_enabled;
	timer_device *dma_timer;

	UINT8 timer_enabled[2];
	timer_device *timer[2];

	UINT32 tms32031_io_regs[0x100];
	UINT16 from_main;
	UINT16 control;

	UINT32 *speedup_ram;
	dmadac_sound_device *dmadac[DAC_BUFFER_CHANNELS];
} cage_t;

static cage_t cage;



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

void cage_init(running_machine &machine, offs_t speedup)
{
	cage_t *state = &cage;
	attotime cage_cpu_clock_period;
	int chan;

	state->irqhandler = NULL;

	memory_set_bankptr(machine, "bank10", machine.region("cageboot")->base());
	memory_set_bankptr(machine, "bank11", machine.region("cage")->base());

	state->cpu = machine.device<cpu_device>("cage");
	cage_cpu_clock_period = attotime::from_hz(state->cpu->clock());
	state->cpu_h1_clock_period = cage_cpu_clock_period * 2;

	state->dma_timer = machine.device<timer_device>("cage_dma_timer");
	state->timer[0] = machine.device<timer_device>("cage_timer0");
	state->timer[1] = machine.device<timer_device>("cage_timer1");

	if (speedup)
		state->speedup_ram = state->cpu->memory().space(AS_PROGRAM)->install_legacy_write_handler(speedup, speedup, FUNC(speedup_w));

	for (chan = 0; chan < DAC_BUFFER_CHANNELS; chan++)
	{
		char buffer[10];
		sprintf(buffer, "dac%d", chan + 1);
		state->dmadac[chan] = machine.device<dmadac_sound_device>(buffer);
	}

	state_save_register_global(machine, cage.cpu_to_cage_ready);
	state_save_register_global(machine, cage.cage_to_cpu_ready);
	state_save_register_global(machine, cage.serial_period_per_word);
	state_save_register_global(machine, cage.dma_enabled);
	state_save_register_global(machine, cage.dma_timer_enabled);
	state_save_register_global_array(machine, cage.timer_enabled);
	state_save_register_global(machine, cage.from_main);
	state_save_register_global(machine, cage.control);
}


void cage_set_irq_handler(void (*irqhandler)(running_machine &, int))
{
	cage_t *state = &cage;
	state->irqhandler = irqhandler;
}


void cage_reset_w(address_space *space, int state)
{
	cage_t *sndstate = &cage;
	if (state)
		cage_control_w(space->machine(), 0);
	device_set_input_line(sndstate->cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  DMA timers
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( dma_timer_callback )
{
	cage_t *state = &cage;
	UINT32 *tms32031_io_regs = state->tms32031_io_regs;

	/* if we weren't enabled, don't do anything, just shut ourself off */
	if (!state->dma_enabled)
	{
		if (state->dma_timer_enabled)
		{
			timer.adjust(attotime::never);
			state->dma_timer_enabled = 0;
		}
		return;
	}

	/* set the final count to 0 and the source address to the final address */
	tms32031_io_regs[DMA_TRANSFER_COUNT] = 0;
	tms32031_io_regs[DMA_SOURCE_ADDR] = param;

	/* set the interrupt */
	device_set_input_line(state->cpu, TMS3203X_DINT, ASSERT_LINE);
	state->dma_enabled = 0;
}


static void update_dma_state(address_space *space)
{
	cage_t *state = &cage;
	UINT32 *tms32031_io_regs = state->tms32031_io_regs;

	/* determine the new enabled state */
	int enabled = ((tms32031_io_regs[DMA_GLOBAL_CTL] & 3) == 3) && (tms32031_io_regs[DMA_TRANSFER_COUNT] != 0);

	/* see if we turned on */
	if (enabled && !state->dma_enabled)
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
			sound_data[i % STACK_SOUND_BUFSIZE] = space->read_dword(addr * 4);
			addr += inc;
			if (i % STACK_SOUND_BUFSIZE == STACK_SOUND_BUFSIZE - 1)
				dmadac_transfer(&state->dmadac[0], DAC_BUFFER_CHANNELS, 1, DAC_BUFFER_CHANNELS, STACK_SOUND_BUFSIZE / DAC_BUFFER_CHANNELS, sound_data);
		}
		if (tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE != 0)
			dmadac_transfer(&state->dmadac[0], DAC_BUFFER_CHANNELS, 1, DAC_BUFFER_CHANNELS, (tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE) / DAC_BUFFER_CHANNELS, sound_data);

		/* compute the time of the interrupt and set the timer */
		if (!state->dma_timer_enabled)
		{
			attotime period = state->serial_period_per_word * tms32031_io_regs[DMA_TRANSFER_COUNT];
			state->dma_timer->adjust(period, addr, period);
			state->dma_timer_enabled = 1;
		}
	}

	/* see if we turned off */
	else if (!enabled && state->dma_enabled)
	{
		state->dma_timer->reset();
		state->dma_timer_enabled = 0;
	}

	/* set the new state */
	state->dma_enabled = enabled;
}



/*************************************
 *
 *  Internal timers
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( cage_timer_callback )
{
	cage_t *state = &cage;
	int which = param;

	/* set the interrupt */
	device_set_input_line(state->cpu, TMS3203X_TINT0 + which, ASSERT_LINE);
	state->timer_enabled[which] = 0;
	update_timer(which);
}


static void update_timer(int which)
{
	cage_t *state = &cage;
	UINT32 *tms32031_io_regs = state->tms32031_io_regs;

	/* determine the new enabled state */
	int base = 0x10 * which;
	int enabled = ((tms32031_io_regs[base + TIMER0_GLOBAL_CTL] & 0xc0) == 0xc0);

	/* see if we turned on */
	if (enabled && !state->timer_enabled[which])
	{
		attotime period = state->cpu_h1_clock_period * (2 * tms32031_io_regs[base + TIMER0_PERIOD]);

		/* make sure our assumptions are correct */
		if (tms32031_io_regs[base + TIMER0_GLOBAL_CTL] != 0x2c1)
			logerror("CAGE TIMER%d: unexpected timer config %08X!\n", which, tms32031_io_regs[base + TIMER0_GLOBAL_CTL]);

		state->timer[which]->adjust(period, which);
	}

	/* see if we turned off */
	else if (!enabled && state->timer_enabled[which])
	{
		state->timer[which]->adjust(attotime::never, which);
	}

	/* set the new state */
	state->timer_enabled[which] = enabled;
}



/*************************************
 *
 *  Serial port I/O
 *
 *************************************/

static void update_serial(running_machine &machine)
{
	cage_t *state = &cage;
	UINT32 *tms32031_io_regs = state->tms32031_io_regs;
	attotime serial_clock_period, bit_clock_period;
	UINT32 freq;

	/* we start out at half the H1 frequency (or 2x the H1 period) */
	serial_clock_period = state->cpu_h1_clock_period * 2;

	/* if we're in clock mode, muliply by another factor of 2 */
	if (tms32031_io_regs[SPORT_GLOBAL_CTL] & 4)
		serial_clock_period *= 2;

	/* now multiply by the timer period */
	bit_clock_period = serial_clock_period * (tms32031_io_regs[SPORT_TIMER_PERIOD] & 0xffff);

	/* and times the number of bits per sample */
	state->serial_period_per_word = bit_clock_period * (8 * (((tms32031_io_regs[SPORT_GLOBAL_CTL] >> 18) & 3) + 1));

	/* compute the step value to stretch this to the sample_rate */
	freq = ATTOSECONDS_TO_HZ(state->serial_period_per_word.attoseconds) / DAC_BUFFER_CHANNELS;
	if (freq > 0 && freq < 100000)
	{
		dmadac_set_frequency(&state->dmadac[0], DAC_BUFFER_CHANNELS, freq);
		dmadac_enable(&state->dmadac[0], DAC_BUFFER_CHANNELS, 1);
	}
}



/*************************************
 *
 *  Master read/write
 *
 *************************************/

static READ32_HANDLER( tms32031_io_r )
{
	cage_t *state = &cage;
	UINT32 *tms32031_io_regs = state->tms32031_io_regs;
	UINT16 result = tms32031_io_regs[offset];

	switch (offset)
	{
		case DMA_GLOBAL_CTL:
			result = (result & ~0xc) | (state->dma_enabled ? 0xc : 0x0);
			break;
	}

	if (LOG_32031_IOPORTS)
		logerror("CAGE:%06X:%s read -> %08X\n", cpu_get_pc(&space->device()), register_names[offset & 0x7f], result);
	return result;
}


static WRITE32_HANDLER( tms32031_io_w )
{
	cage_t *state = &cage;
	UINT32 *tms32031_io_regs = state->tms32031_io_regs;

	COMBINE_DATA(&tms32031_io_regs[offset]);

	if (LOG_32031_IOPORTS)
		logerror("CAGE:%06X:%s write = %08X\n", cpu_get_pc(&space->device()), register_names[offset & 0x7f], tms32031_io_regs[offset]);

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
			if ((int)ATTOSECONDS_TO_HZ(state->serial_period_per_word.attoseconds) == 22050*4 && (tms32031_io_regs[SPORT_RX_CTL] & 0xff) == 0x62)
				tms32031_io_regs[SPORT_RX_CTL] ^= 0x800;
#endif
			break;

		case SPORT_GLOBAL_CTL:
		case SPORT_TIMER_CTL:
		case SPORT_TIMER_PERIOD:
			update_serial(space->machine());
			break;
	}
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

static void update_control_lines(running_machine &machine)
{
	cage_t *state = &cage;
	int val;

	/* set the IRQ to the main CPU */
	if (state->irqhandler)
	{
		int reason = 0;

		if ((state->control & 3) == 3 && !state->cpu_to_cage_ready)
			reason |= CAGE_IRQ_REASON_BUFFER_EMPTY;
		if ((state->control & 2) && state->cage_to_cpu_ready)
			reason |= CAGE_IRQ_REASON_DATA_READY;

		(*state->irqhandler)(machine, reason);
	}

	/* set the IOF input lines */
	val = cpu_get_reg(state->cpu, TMS3203X_IOF);
	val &= ~0x88;
	if (state->cpu_to_cage_ready) val |= 0x08;
	if (state->cage_to_cpu_ready) val |= 0x80;
	state->cpu->set_state(TMS3203X_IOF, val);
}


static READ32_HANDLER( cage_from_main_r )
{
	cage_t *state = &cage;
	if (LOG_COMM)
		logerror("%06X:CAGE read command = %04X\n", cpu_get_pc(&space->device()), state->from_main);
	state->cpu_to_cage_ready = 0;
	update_control_lines(space->machine());
	device_set_input_line(state->cpu, TMS3203X_IRQ0, CLEAR_LINE);
	return state->from_main;
}


static WRITE32_HANDLER( cage_from_main_ack_w )
{
	if (LOG_COMM)
	{
		cage_t *state = &cage;
		logerror("%06X:CAGE ack command = %04X\n", cpu_get_pc(&space->device()), state->from_main);
	}
}


static WRITE32_HANDLER( cage_to_main_w )
{
	cage_t *state = &cage;
	if (LOG_COMM)
		logerror("%06X:Data from CAGE = %04X\n", cpu_get_pc(&space->device()), data);
	driver_device *drvstate = space->machine().driver_data<driver_device>();
	drvstate->soundlatch_word_w(*space, 0, data, mem_mask);
	state->cage_to_cpu_ready = 1;
	update_control_lines(space->machine());
}


static READ32_HANDLER( cage_io_status_r )
{
	cage_t *state = &cage;
	int result = 0;
	if (state->cpu_to_cage_ready)
		result |= 0x80;
	if (!state->cage_to_cpu_ready)
		result |= 0x40;
	return result;
}


UINT16 cage_main_r(address_space *space)
{
	cage_t *state = &cage;
	driver_device *drvstate = space->machine().driver_data<driver_device>();	
	if (LOG_COMM)
		logerror("%s:main read data = %04X\n", space->machine().describe_context(), drvstate->soundlatch_word_r(*space, 0, 0));
	state->cage_to_cpu_ready = 0;
	update_control_lines(space->machine());
	return drvstate->soundlatch_word_r(*space, 0, 0xffff);
}


static TIMER_CALLBACK( cage_deferred_w )
{
	cage_t *state = &cage;
	state->from_main = param;
	state->cpu_to_cage_ready = 1;
	update_control_lines(machine);
	device_set_input_line(state->cpu, TMS3203X_IRQ0, ASSERT_LINE);
}


void cage_main_w(address_space *space, UINT16 data)
{
	if (LOG_COMM)
		logerror("%s:Command to CAGE = %04X\n", space->machine().describe_context(), data);
	space->machine().scheduler().synchronize(FUNC(cage_deferred_w), data);
}


UINT16 cage_control_r(running_machine &machine)
{
	cage_t *state = &cage;
	UINT16 result = 0;

	if (state->cpu_to_cage_ready)
		result |= 2;
	if (state->cage_to_cpu_ready)
		result |= 1;

	return result;
}


void cage_control_w(running_machine &machine, UINT16 data)
{
	cage_t *state = &cage;
	UINT32 *tms32031_io_regs = state->tms32031_io_regs;

	state->control = data;

	/* CPU is reset if both control lines are 0 */
	if (!(state->control & 3))
	{
		device_set_input_line(state->cpu, INPUT_LINE_RESET, ASSERT_LINE);

		state->dma_enabled = 0;
		state->dma_timer_enabled = 0;
		state->dma_timer->reset();

		state->timer_enabled[0] = 0;
		state->timer_enabled[1] = 0;
		state->timer[0]->reset();
		state->timer[1]->reset();

		memset(tms32031_io_regs, 0, 0x60 * 4);

		state->cpu_to_cage_ready = 0;
		state->cage_to_cpu_ready = 0;
	}
	else
		device_set_input_line(state->cpu, INPUT_LINE_RESET, CLEAR_LINE);

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
	cage_t *state = &cage;

	device_eat_cycles(&space->device(), 100);
	COMBINE_DATA(&state->speedup_ram[offset]);
}



/*************************************
 *
 *  CPU memory map & config
 *
 *************************************/

static const tms3203x_config cage_config =
{
	0x400000
};


static ADDRESS_MAP_START( cage_map, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM
	AM_RANGE(0x200000, 0x200000) AM_WRITENOP
	AM_RANGE(0x400000, 0x47ffff) AM_ROMBANK("bank10")
	AM_RANGE(0x808000, 0x8080ff) AM_READWRITE_LEGACY(tms32031_io_r, tms32031_io_w)
	AM_RANGE(0x809800, 0x809fff) AM_RAM
	AM_RANGE(0xa00000, 0xa00000) AM_READWRITE_LEGACY(cage_from_main_r, cage_to_main_w)
	AM_RANGE(0xc00000, 0xffffff) AM_ROMBANK("bank11")
ADDRESS_MAP_END


static ADDRESS_MAP_START( cage_map_seattle, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM
	AM_RANGE(0x200000, 0x200000) AM_WRITENOP
	AM_RANGE(0x400000, 0x47ffff) AM_ROMBANK("bank10")
	AM_RANGE(0x808000, 0x8080ff) AM_READWRITE_LEGACY(tms32031_io_r, tms32031_io_w)
	AM_RANGE(0x809800, 0x809fff) AM_RAM
	AM_RANGE(0xa00000, 0xa00000) AM_READWRITE_LEGACY(cage_from_main_r, cage_from_main_ack_w)
	AM_RANGE(0xa00001, 0xa00001) AM_WRITE_LEGACY(cage_to_main_w)
	AM_RANGE(0xa00003, 0xa00003) AM_READ_LEGACY(cage_io_status_r)
	AM_RANGE(0xc00000, 0xffffff) AM_ROMBANK("bank11")
ADDRESS_MAP_END



/*************************************
 *
 *  CAGE machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( cage )

	/* basic machine hardware */
	MCFG_CPU_ADD("cage", TMS32031, 33868800)
	MCFG_TMS3203X_CONFIG(cage_config)
	MCFG_CPU_PROGRAM_MAP(cage_map)

	MCFG_TIMER_ADD("cage_dma_timer", dma_timer_callback)
	MCFG_TIMER_ADD("cage_timer0", cage_timer_callback)
	MCFG_TIMER_ADD("cage_timer1", cage_timer_callback)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

#if (DAC_BUFFER_CHANNELS == 4)
	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MCFG_SOUND_ADD("dac3", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MCFG_SOUND_ADD("dac4", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
#else
	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
#endif
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( cage_seattle, cage )

	MCFG_CPU_MODIFY("cage")
	MCFG_CPU_PROGRAM_MAP(cage_map_seattle)
MACHINE_CONFIG_END
