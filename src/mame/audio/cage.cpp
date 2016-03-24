// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/


#define LOG_COMM            (0)
#define LOG_32031_IOPORTS   (0)


#include "emu.h"
#include "cpu/tms32031/tms32031.h"
#include "cage.h"



/*************************************
 *
 *  Constants/macros
 *
 *************************************/

#define DAC_BUFFER_CHANNELS     4
#define STACK_SOUND_BUFSIZE     (1024)

/*************************************
 *
 *  I/O port definitions
 *
 *************************************/

#define DMA_GLOBAL_CTL          0x00
#define DMA_SOURCE_ADDR         0x04
#define DMA_DEST_ADDR           0x06
#define DMA_TRANSFER_COUNT      0x08

#define TIMER0_GLOBAL_CTL       0x20
#define TIMER0_COUNTER          0x24
#define TIMER0_PERIOD           0x28

#define TIMER1_GLOBAL_CTL       0x30
#define TIMER1_COUNTER          0x34
#define TIMER1_PERIOD           0x38

#define SPORT_GLOBAL_CTL        0x40
#define SPORT_TX_CTL            0x42
#define SPORT_RX_CTL            0x43
#define SPORT_TIMER_CTL         0x44
#define SPORT_TIMER_COUNTER     0x45
#define SPORT_TIMER_PERIOD      0x46
#define SPORT_DATA_TX           0x48
#define SPORT_DATA_RX           0x4c

static const char *const register_names[] =
{
	"TMS32031-DMA global control", nullptr, nullptr, nullptr,
	"TMS32031-DMA source address", nullptr, "TMS32031-DMA destination address", nullptr,
	"TMS32031-DMA transfer counter", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	"TMS32031-Timer 0 global control", nullptr, nullptr, nullptr,
	"TMS32031-Timer 0 counter", nullptr, nullptr, nullptr,
	"TMS32031-Timer 0 period", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	"TMS32031-Timer 1 global control", nullptr, nullptr, nullptr,
	"TMS32031-Timer 1 counter", nullptr, nullptr, nullptr,
	"TMS32031-Timer 1 period", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	"TMS32031-Serial port global control", nullptr, "TMS32031-Serial port TX control", "TMS32031-Serial port RX control",
	"TMS32031-Serial port timer control", "TMS32031-Serial port timer counter", "TMS32031-Serial port timer period", nullptr,
	"TMS32031-Serial port data TX", nullptr, nullptr, nullptr,
	"TMS32031-Serial port data RX", nullptr, nullptr, nullptr,

	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	nullptr, nullptr, nullptr, nullptr,
	"TMS32031-Primary bus control", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,

	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr
};



/*************************************
 *
 *  Initialization
 *
 *************************************/

const device_type ATARI_CAGE = &device_creator<atari_cage_device>;


//-------------------------------------------------
//  atari_cage_device - constructor
//-------------------------------------------------

atari_cage_device::atari_cage_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ATARI_CAGE, "Atari CAGE", tag, owner, clock, "atari_cage", __FILE__),
	m_irqhandler(*this)
{
}

atari_cage_device::atari_cage_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_irqhandler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atari_cage_device::device_start()
{
	attotime cage_cpu_clock_period;
	int chan;

	// resolve callbacks
	m_irqhandler.resolve_safe();

	membank("bank10")->set_base(machine().root_device().memregion("cageboot")->base());
	membank("bank11")->set_base(machine().root_device().memregion("cage")->base());

	m_cpu = subdevice<cpu_device>("cage");
	cage_cpu_clock_period = attotime::from_hz(m_cpu->clock());
	m_cpu_h1_clock_period = cage_cpu_clock_period * 2;

	m_dma_timer = subdevice<timer_device>("cage_dma_timer");
	m_timer[0] = subdevice<timer_device>("cage_timer0");
	m_timer[1] = subdevice<timer_device>("cage_timer1");

	if (m_speedup)
		m_speedup_ram = m_cpu->space(AS_PROGRAM).install_write_handler(m_speedup, m_speedup, write32_delegate(FUNC(atari_cage_device::speedup_w),this));

	for (chan = 0; chan < DAC_BUFFER_CHANNELS; chan++)
	{
		char buffer[10];
		sprintf(buffer, "dac%d", chan + 1);
		m_dmadac[chan] = subdevice<dmadac_sound_device>(buffer);
	}

	save_item(NAME(m_cpu_to_cage_ready));
	save_item(NAME(m_cage_to_cpu_ready));
	save_item(NAME(m_serial_period_per_word));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_dma_timer_enabled));
	save_item(NAME(m_timer_enabled));
	save_item(NAME(m_from_main));
	save_item(NAME(m_control));
}


void atari_cage_device::reset_w(int state)
{
	if (state)
		control_w(0);
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  DMA timers
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER( atari_cage_device::dma_timer_callback )
{
	UINT32 *tms32031_io_regs = m_tms32031_io_regs;

	/* if we weren't enabled, don't do anything, just shut ourself off */
	if (!m_dma_enabled)
	{
		if (m_dma_timer_enabled)
		{
			timer.adjust(attotime::never);
			m_dma_timer_enabled = 0;
		}
		return;
	}

	/* set the final count to 0 and the source address to the final address */
	tms32031_io_regs[DMA_TRANSFER_COUNT] = 0;
	tms32031_io_regs[DMA_SOURCE_ADDR] = param;

	/* set the interrupt */
	m_cpu->set_input_line(TMS3203X_DINT, ASSERT_LINE);
	m_dma_enabled = 0;
}


void atari_cage_device::update_dma_state(address_space &space)
{
	UINT32 *tms32031_io_regs = m_tms32031_io_regs;

	/* determine the new enabled state */
	int enabled = ((tms32031_io_regs[DMA_GLOBAL_CTL] & 3) == 3) && (tms32031_io_regs[DMA_TRANSFER_COUNT] != 0);

	/* see if we turned on */
	if (enabled && !m_dma_enabled)
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
			sound_data[i % STACK_SOUND_BUFSIZE] = space.read_dword(addr * 4);
			addr += inc;
			if (i % STACK_SOUND_BUFSIZE == STACK_SOUND_BUFSIZE - 1)
				dmadac_transfer(&m_dmadac[0], DAC_BUFFER_CHANNELS, 1, DAC_BUFFER_CHANNELS, STACK_SOUND_BUFSIZE / DAC_BUFFER_CHANNELS, sound_data);
		}
		if (tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE != 0)
			dmadac_transfer(&m_dmadac[0], DAC_BUFFER_CHANNELS, 1, DAC_BUFFER_CHANNELS, (tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE) / DAC_BUFFER_CHANNELS, sound_data);

		/* compute the time of the interrupt and set the timer */
		if (!m_dma_timer_enabled)
		{
			attotime period = m_serial_period_per_word * tms32031_io_regs[DMA_TRANSFER_COUNT];
			m_dma_timer->adjust(period, addr, period);
			m_dma_timer_enabled = 1;
		}
	}

	/* see if we turned off */
	else if (!enabled && m_dma_enabled)
	{
		m_dma_timer->reset();
		m_dma_timer_enabled = 0;
	}

	/* set the new state */
	m_dma_enabled = enabled;
}



/*************************************
 *
 *  Internal timers
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER( atari_cage_device::cage_timer_callback )
{
	int which = param;
	/* set the interrupt */
	m_cpu->set_input_line(TMS3203X_TINT0 + which, ASSERT_LINE);
	m_timer_enabled[which] = 0;
	update_timer(which);
}


void atari_cage_device::update_timer(int which)
{
	UINT32 *tms32031_io_regs = m_tms32031_io_regs;

	/* determine the new enabled state */
	int base = 0x10 * which;
	int enabled = ((tms32031_io_regs[base + TIMER0_GLOBAL_CTL] & 0xc0) == 0xc0);

	/* see if we turned on */
	if (enabled && !m_timer_enabled[which])
	{
		attotime period = m_cpu_h1_clock_period * (2 * tms32031_io_regs[base + TIMER0_PERIOD]);

		/* make sure our assumptions are correct */
		if (tms32031_io_regs[base + TIMER0_GLOBAL_CTL] != 0x2c1)
			logerror("CAGE TIMER%d: unexpected timer config %08X!\n", which, tms32031_io_regs[base + TIMER0_GLOBAL_CTL]);

		m_timer[which]->adjust(period, which);
	}

	/* see if we turned off */
	else if (!enabled && m_timer_enabled[which])
	{
		m_timer[which]->adjust(attotime::never, which);
	}

	/* set the new state */
	m_timer_enabled[which] = enabled;
}



/*************************************
 *
 *  Serial port I/O
 *
 *************************************/

void atari_cage_device::update_serial()
{
	UINT32 *tms32031_io_regs = m_tms32031_io_regs;
	attotime serial_clock_period, bit_clock_period;
	UINT32 freq;

	/* we start out at half the H1 frequency (or 2x the H1 period) */
	serial_clock_period = m_cpu_h1_clock_period * 2;

	/* if we're in clock mode, muliply by another factor of 2 */
	if (tms32031_io_regs[SPORT_GLOBAL_CTL] & 4)
		serial_clock_period *= 2;

	/* now multiply by the timer period */
	bit_clock_period = serial_clock_period * (tms32031_io_regs[SPORT_TIMER_PERIOD] & 0xffff);

	/* and times the number of bits per sample */
	m_serial_period_per_word = bit_clock_period * (8 * (((tms32031_io_regs[SPORT_GLOBAL_CTL] >> 18) & 3) + 1));

	/* compute the step value to stretch this to the sample_rate */
	freq = ATTOSECONDS_TO_HZ(m_serial_period_per_word.attoseconds()) / DAC_BUFFER_CHANNELS;
	if (freq > 0 && freq < 100000)
	{
		dmadac_set_frequency(&m_dmadac[0], DAC_BUFFER_CHANNELS, freq);
		dmadac_enable(&m_dmadac[0], DAC_BUFFER_CHANNELS, 1);
	}
}



/*************************************
 *
 *  Master read/write
 *
 *************************************/

READ32_MEMBER( atari_cage_device::tms32031_io_r )
{
	UINT32 *tms32031_io_regs = m_tms32031_io_regs;
	UINT16 result = tms32031_io_regs[offset];

	switch (offset)
	{
		case DMA_GLOBAL_CTL:
			result = (result & ~0xc) | (m_dma_enabled ? 0xc : 0x0);
			break;
	}

	if (LOG_32031_IOPORTS)
		logerror("CAGE:%06X:%s read -> %08X\n", space.device().safe_pc(), register_names[offset & 0x7f], result);
	return result;
}


WRITE32_MEMBER( atari_cage_device::tms32031_io_w )
{
	UINT32 *tms32031_io_regs = m_tms32031_io_regs;

	COMBINE_DATA(&tms32031_io_regs[offset]);

	if (LOG_32031_IOPORTS)
		logerror("CAGE:%06X:%s write = %08X\n", space.device().safe_pc(), register_names[offset & 0x7f], tms32031_io_regs[offset]);

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
			if ((int)ATTOSECONDS_TO_HZ(m_serial_period_per_word.attoseconds()) == 22050*4 && (tms32031_io_regs[SPORT_RX_CTL] & 0xff) == 0x62)
				tms32031_io_regs[SPORT_RX_CTL] ^= 0x800;
#endif
			break;

		case SPORT_GLOBAL_CTL:
		case SPORT_TIMER_CTL:
		case SPORT_TIMER_PERIOD:
			update_serial();
			break;
	}
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

void atari_cage_device::update_control_lines()
{
	int val;

	/* set the IRQ to the main CPU */
	int reason = 0;

	if ((m_control & 3) == 3 && !m_cpu_to_cage_ready)
		reason |= CAGE_IRQ_REASON_BUFFER_EMPTY;
	if ((m_control & 2) && m_cage_to_cpu_ready)
		reason |= CAGE_IRQ_REASON_DATA_READY;

	m_irqhandler(machine().driver_data()->generic_space(), 0, reason);
	/* set the IOF input lines */
	val = m_cpu->state_int(TMS3203X_IOF);
	val &= ~0x88;
	if (m_cpu_to_cage_ready) val |= 0x08;
	if (m_cage_to_cpu_ready) val |= 0x80;
	m_cpu->set_state_int(TMS3203X_IOF, val);
}


READ32_MEMBER( atari_cage_device::cage_from_main_r )
{
	if (LOG_COMM)
		logerror("%06X:CAGE read command = %04X\n", space.device().safe_pc(), m_from_main);
	m_cpu_to_cage_ready = 0;
	update_control_lines();
	m_cpu->set_input_line(TMS3203X_IRQ0, CLEAR_LINE);
	return m_from_main;
}


WRITE32_MEMBER( atari_cage_device::cage_from_main_ack_w )
{
	if (LOG_COMM)
	{
			logerror("%06X:CAGE ack command = %04X\n", space.device().safe_pc(), m_from_main);
	}
}


WRITE32_MEMBER( atari_cage_device::cage_to_main_w )
{
	if (LOG_COMM)
		logerror("%06X:Data from CAGE = %04X\n", space.device().safe_pc(), data);
	driver_device *drvstate = space.machine().driver_data<driver_device>();
	drvstate->soundlatch_word_w(space, 0, data, mem_mask);
	m_cage_to_cpu_ready = 1;
	update_control_lines();
}


READ32_MEMBER( atari_cage_device::cage_io_status_r )
{
	int result = 0;
	if (m_cpu_to_cage_ready)
		result |= 0x80;
	if (!m_cage_to_cpu_ready)
		result |= 0x40;
	return result;
}


UINT16 atari_cage_device::main_r()
{
	driver_device *drvstate = machine().driver_data<driver_device>();
	if (LOG_COMM)
		logerror("%s:main read data = %04X\n", machine().describe_context(), drvstate->soundlatch_word_r(drvstate->generic_space(), 0, 0));
	m_cage_to_cpu_ready = 0;
	update_control_lines();
	return drvstate->soundlatch_word_r(drvstate->generic_space(), 0, 0xffff);
}


TIMER_CALLBACK_MEMBER( atari_cage_device::cage_deferred_w )
{
	m_from_main = param;
	m_cpu_to_cage_ready = 1;
	update_control_lines();
	m_cpu->set_input_line(TMS3203X_IRQ0, ASSERT_LINE);
}


void atari_cage_device::main_w(UINT16 data)
{
	if (LOG_COMM)
		logerror("%s:Command to CAGE = %04X\n", machine().describe_context(), data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(atari_cage_device::cage_deferred_w),this), data);
}


UINT16 atari_cage_device::control_r()
{
	UINT16 result = 0;

	if (m_cpu_to_cage_ready)
		result |= 2;
	if (m_cage_to_cpu_ready)
		result |= 1;

	return result;
}


void atari_cage_device::control_w(UINT16 data)
{
	UINT32 *tms32031_io_regs = m_tms32031_io_regs;

	m_control = data;

	/* CPU is reset if both control lines are 0 */
	if (!(m_control & 3))
	{
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_cpu->set_input_line(TMS3203X_IRQ1, ASSERT_LINE);

		m_dma_enabled = 0;
		m_dma_timer_enabled = 0;
		m_dma_timer->reset();

		m_timer_enabled[0] = 0;
		m_timer_enabled[1] = 0;
		m_timer[0]->reset();
		m_timer[1]->reset();

		memset(tms32031_io_regs, 0, 0x60 * 4);

		m_cpu_to_cage_ready = 0;
		m_cage_to_cpu_ready = 0;
	}
	else
	{
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_cpu->set_input_line(TMS3203X_IRQ1, CLEAR_LINE);
	}

	/* update the control state */
	update_control_lines();
}



/*************************************
 *
 *  Speedups
 *
 *************************************/

WRITE32_MEMBER( atari_cage_device::speedup_w )
{
	space.device().execute().eat_cycles(100);
	COMBINE_DATA(&m_speedup_ram[offset]);
}



/*************************************
 *
 *  CPU memory map & config
 *
 *************************************/

static ADDRESS_MAP_START( cage_map, AS_PROGRAM, 32, atari_cage_device )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM
	AM_RANGE(0x200000, 0x200000) AM_WRITENOP
	AM_RANGE(0x400000, 0x47ffff) AM_ROMBANK("bank10")
	AM_RANGE(0x808000, 0x8080ff) AM_READWRITE(tms32031_io_r, tms32031_io_w)
	AM_RANGE(0x809800, 0x809fff) AM_RAM
	AM_RANGE(0xa00000, 0xa00000) AM_READWRITE(cage_from_main_r, cage_to_main_w)
	AM_RANGE(0xc00000, 0xffffff) AM_ROMBANK("bank11")
ADDRESS_MAP_END


static ADDRESS_MAP_START( cage_map_seattle, AS_PROGRAM, 32, atari_cage_seattle_device )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM
	AM_RANGE(0x200000, 0x200000) AM_WRITENOP
	AM_RANGE(0x400000, 0x47ffff) AM_ROMBANK("bank10")
	AM_RANGE(0x808000, 0x8080ff) AM_READWRITE(tms32031_io_r, tms32031_io_w)
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

MACHINE_CONFIG_FRAGMENT( cage )

	/* basic machine hardware */
	MCFG_CPU_ADD("cage", TMS32031, 33868800)
	MCFG_CPU_PROGRAM_MAP(cage_map)
	MCFG_TMS3203X_MCBL(true)

	MCFG_TIMER_DEVICE_ADD("cage_dma_timer", DEVICE_SELF, atari_cage_device, dma_timer_callback)
	MCFG_TIMER_DEVICE_ADD("cage_timer0", DEVICE_SELF, atari_cage_device, cage_timer_callback)
	MCFG_TIMER_DEVICE_ADD("cage_timer1", DEVICE_SELF, atari_cage_device, cage_timer_callback)

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

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor atari_cage_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cage );
}


const device_type ATARI_CAGE_SEATTLE = &device_creator<atari_cage_seattle_device>;


//-------------------------------------------------
//  atari_cage_seattle_device - constructor
//-------------------------------------------------

atari_cage_seattle_device::atari_cage_seattle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	atari_cage_device(mconfig, ATARI_CAGE_SEATTLE, "Atari CAGE Seattle", tag, owner, clock, "atari_cage_seattle", __FILE__)
{
}

machine_config_constructor atari_cage_seattle_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cage_seattle );
}
