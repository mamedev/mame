// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari CAGE Audio Board

    Atari CH31.2 sound board layout:
    --------------------------------

    ATARI GAMES (C) 93 MADE IN U.S.A.
    |-----------------------------------------------------------|
    |         JXBUS|---------------------------------|          |
    |              |---------------------------------|  JPWR    |
    |       |---------|                                         |
    |JROMBUS|   11A   |                                         |
    | |--|  |---------|                                         |
    | |  ||-----------|                                         |
    | |  ||    11B    |                                         |
    | |  ||-----------|                                         |
    | |  ||-----------|                     3B                  |
    | |  ||    11C    | RAM  RAM  RAM  RAM                      |
    | |  ||-----------|                                         |
    | |  ||-----------|                                         |
    | |  ||    11D    |                                         |
    | |  ||-----------|                     3D                  |
    | |  ||-----------|      |-----------|                      |
    | |  ||    11E    |      |           |                      |
    | |  ||-----------|      |    DSP    |                      |
    | |  |                   |           |                 AMP  |
    | |--|OSC                |-----------|  JSPKR               |
    |                                                           |
    |-----------------------------------------------------------|

    DSP: TMS320C31 33.8688MHz
    OSC: 33.8688MHz
    11A: DIP32 EPROM for Boot
    11B, 11C, 11D, 11E: DIP42 Mask ROM for Data
    RAM: 32Kx8 bit SRAM
    3B: AK4316-VS DAC, Not populated in primal rage (no quad channel sound support)
    3D: AK4316-VS DAC
    AMP: TDA1554Q Audio amplifier for connect speaker directly
    JROMBUS: ROM expansion connector
    JXBUS: X-Bus host interface
    JSPKR: Audio output connector, Connect to external speaker or motherboard/AMP board
    JPWR: Power supply connector, Connect to external power distribution board or motherboard

    Pinouts:

    JSPKR:
    ---------------
    01  Channel 1 +
    02  Channel 1 -
    03  Channel 2 +
    04  Channel 2 -
    05  Key
    06  Channel 3 +
    07  Channel 3 -
    08  Channel 4 +
    09  Channel 4 -
    10  Subwoofer +
    11  Subwoofer -

    JPWR:
    ----------------
    01  +5V
    02  +5V
    03  Key
    04  GND
    05  GND
    06  +12V
    07  +12V
    08  -5V
    09  GND

    TODO:
    - Move DSP internal functions into tms32031.cpp
    - Further support for variable sound output channel
    - Support subwoofer output

****************************************************************************/


#include "emu.h"
#include "cage.h"

#define LOG_COMM            (1U << 1)
#define LOG_32031_IOPORTS   (1U << 2)
#define LOG_WARNINGS        (1U << 3)

#define VERBOSE (LOG_WARNINGS)
#include "logmacro.h"


/*************************************
 *
 *  Constants/macros
 *
 *************************************/

#define DAC_BUFFER_CHANNELS     4 // 5; 4 channel + subwoofer?
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

// uses Atari CH31(.2) external sound board
DEFINE_DEVICE_TYPE(ATARI_CAGE, atari_cage_device, "atari_cage", "Atari CAGE")


//-------------------------------------------------
//  atari_cage_device - constructor
//-------------------------------------------------

atari_cage_device::atari_cage_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atari_cage_device(mconfig, ATARI_CAGE, tag, owner, clock)
{
}

atari_cage_device::atari_cage_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_cageram(*this, "cageram"),
	m_soundlatch(*this, "soundlatch"),
	m_dma_timer(*this, "cage_dma_timer"),
	m_timer(*this, "cage_timer%u", 0U),
	m_dmadac(*this, "dac%u", 1U),
	m_bootbank(*this, "bootbank"),
	m_mainbank(*this, "mainbank"),
	m_bootrom(*this, "boot"),
	m_mainrom(*this, DEVICE_SELF),
	m_irqhandler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atari_cage_device::device_start()
{
	attotime cage_cpu_clock_period;

	m_bootbank->set_base(m_bootrom->base());
	m_mainbank->set_base(m_mainrom->base());

	cage_cpu_clock_period = attotime::from_hz(m_cpu->clock());
	m_cpu_h1_clock_period = cage_cpu_clock_period * 2;

	if (m_speedup) {
		m_cpu->space(AS_PROGRAM).install_write_handler(m_speedup, m_speedup, write32s_delegate(*this, FUNC(atari_cage_device::speedup_w)));
		m_speedup_ram = m_cageram + m_speedup;
	}

	save_item(NAME(m_cpu_to_cage_ready));
	save_item(NAME(m_cage_to_cpu_ready));
	save_item(NAME(m_serial_period_per_word));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_dma_timer_enabled));
	save_item(NAME(m_timer_enabled));
	save_item(NAME(m_from_main));
	save_item(NAME(m_control));
	save_item(NAME(m_tms32031_io_regs));
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
	m_tms32031_io_regs[DMA_TRANSFER_COUNT] = 0;
	m_tms32031_io_regs[DMA_SOURCE_ADDR] = param;

	/* set the interrupt */
	m_cpu->set_input_line(TMS3203X_DINT0, ASSERT_LINE);
	m_dma_enabled = 0;
}


void atari_cage_device::update_dma_state()
{
	/* determine the new enabled state */
	int enabled = ((m_tms32031_io_regs[DMA_GLOBAL_CTL] & 3) == 3) && (m_tms32031_io_regs[DMA_TRANSFER_COUNT] != 0);

	/* see if we turned on */
	if (enabled && !m_dma_enabled)
	{
		int16_t sound_data[STACK_SOUND_BUFSIZE];
		uint32_t addr, inc;
		int i;

		/* make sure our assumptions are correct */
		if (m_tms32031_io_regs[DMA_DEST_ADDR] != 0x808048)
			LOGMASKED(LOG_WARNINGS, "CAGE DMA: unexpected dest address %08X!\n", m_tms32031_io_regs[DMA_DEST_ADDR]);
		if ((m_tms32031_io_regs[DMA_GLOBAL_CTL] & 0xfef) != 0xe03)
			LOGMASKED(LOG_WARNINGS, "CAGE DMA: unexpected transfer params %08X!\n", m_tms32031_io_regs[DMA_GLOBAL_CTL]);

		/* do the DMA up front */
		addr = m_tms32031_io_regs[DMA_SOURCE_ADDR];
		inc = (m_tms32031_io_regs[DMA_GLOBAL_CTL] >> 4) & 1;
		for (i = 0; i < m_tms32031_io_regs[DMA_TRANSFER_COUNT]; i++)
		{
			sound_data[i % STACK_SOUND_BUFSIZE] = m_cpu->space(AS_PROGRAM).read_dword(addr);
			addr += inc;
			if (i % STACK_SOUND_BUFSIZE == STACK_SOUND_BUFSIZE - 1)
				for (int j = 0; j < DAC_BUFFER_CHANNELS; j++)
					m_dmadac[j]->transfer(j, 1, DAC_BUFFER_CHANNELS, STACK_SOUND_BUFSIZE / DAC_BUFFER_CHANNELS, sound_data);
		}
		if (m_tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE != 0)
			for (int j = 0; j < DAC_BUFFER_CHANNELS; j++)
				m_dmadac[j]->transfer(j, 1, DAC_BUFFER_CHANNELS, (m_tms32031_io_regs[DMA_TRANSFER_COUNT] % STACK_SOUND_BUFSIZE) / DAC_BUFFER_CHANNELS, sound_data);

		/* compute the time of the interrupt and set the timer */
		if (!m_dma_timer_enabled)
		{
			attotime period = m_serial_period_per_word * m_tms32031_io_regs[DMA_TRANSFER_COUNT];
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
	/* determine the new enabled state */
	int base = 0x10 * which;
	int enabled = ((m_tms32031_io_regs[base + TIMER0_GLOBAL_CTL] & 0xc0) == 0xc0);

	/* see if we turned on */
	if (enabled && !m_timer_enabled[which])
	{
		attotime period = m_cpu_h1_clock_period * (2 * m_tms32031_io_regs[base + TIMER0_PERIOD]);

		/* make sure our assumptions are correct */
		if (m_tms32031_io_regs[base + TIMER0_GLOBAL_CTL] != 0x2c1)
			LOGMASKED(LOG_WARNINGS, "CAGE TIMER%d: unexpected timer config %08X!\n", which, m_tms32031_io_regs[base + TIMER0_GLOBAL_CTL]);

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
	attotime serial_clock_period, bit_clock_period;
	uint32_t freq;

	/* we start out at half the H1 frequency (or 2x the H1 period) */
	serial_clock_period = m_cpu_h1_clock_period * 2;

	/* if we're in clock mode, muliply by another factor of 2 */
	if (m_tms32031_io_regs[SPORT_GLOBAL_CTL] & 4)
		serial_clock_period *= 2;

	/* now multiply by the timer period */
	if ((m_tms32031_io_regs[SPORT_TIMER_PERIOD] & 0xffff) == 0)
		bit_clock_period = (m_tms32031_io_regs[SPORT_GLOBAL_CTL] & 4) ? serial_clock_period : attotime::never;
	else
		bit_clock_period = serial_clock_period * (m_tms32031_io_regs[SPORT_TIMER_PERIOD] & 0xffff);

	/* and times the number of bits per sample */
	m_serial_period_per_word = bit_clock_period * (8 * (((m_tms32031_io_regs[SPORT_GLOBAL_CTL] >> 18) & 3) + 1));

	/* compute the step value to stretch this to the sample_rate */
	freq = m_serial_period_per_word.as_hz() / DAC_BUFFER_CHANNELS;
	if (freq > 0 && freq < 100000)
	{
		for (int i = 0; i < DAC_BUFFER_CHANNELS; i++)
		{
			m_dmadac[i]->set_frequency(freq);
			m_dmadac[i]->enable(1);
		}
	}
}



/*************************************
 *
 *  Master read/write
 *
 *************************************/

uint32_t atari_cage_device::tms32031_io_r(offs_t offset)
{
	uint16_t result = m_tms32031_io_regs[offset];

	switch (offset)
	{
		case DMA_GLOBAL_CTL:
			result = (result & ~0xc) | (m_dma_enabled ? 0xc : 0x0);
			break;
	}

	LOGMASKED(LOG_32031_IOPORTS, "%s CAGE:%s read -> %08X\n", machine().describe_context(), register_names[offset & 0x7f], result);
	return result;
}


void atari_cage_device::tms32031_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tms32031_io_regs[offset]);

	LOGMASKED(LOG_32031_IOPORTS, "%s CAGE:%s write = %08X\n", machine().describe_context(), register_names[offset & 0x7f], m_tms32031_io_regs[offset]);

	switch (offset)
	{
		case DMA_GLOBAL_CTL:
		case DMA_SOURCE_ADDR:
		case DMA_DEST_ADDR:
		case DMA_TRANSFER_COUNT:
			update_dma_state();
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
			if (int(m_serial_period_per_word.as_hz()) == 22050*4 && (m_tms32031_io_regs[SPORT_RX_CTL] & 0xff) == 0x62)
				m_tms32031_io_regs[SPORT_RX_CTL] ^= 0x800;
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

	m_irqhandler(0, reason);
	/* set the IOF input lines */
	val = m_cpu->state_int(TMS3203X_IOF);
	val &= ~0x88;
	if (m_cpu_to_cage_ready) val |= 0x08;
	if (m_cage_to_cpu_ready) val |= 0x80;
	m_cpu->set_state_int(TMS3203X_IOF, val);
}


uint32_t atari_cage_device::cage_from_main_r()
{
	LOGMASKED(LOG_COMM, "%s CAGE read command = %04X\n", machine().describe_context(), m_from_main);
	m_cpu_to_cage_ready = 0;
	update_control_lines();
	m_cpu->set_input_line(TMS3203X_IRQ0, CLEAR_LINE);
	return m_from_main;
}


void atari_cage_device::cage_from_main_ack_w(uint32_t data)
{
	LOGMASKED(LOG_COMM, "%s CAGE ack command = %04X\n", machine().describe_context(), m_from_main);
}


void atari_cage_device::cage_to_main_w(uint32_t data)
{
	LOGMASKED(LOG_COMM, "%s Data from CAGE = %04X\n", machine().describe_context(), data);
	m_soundlatch->write(data);
	m_cage_to_cpu_ready = 1;
	update_control_lines();
}


uint32_t atari_cage_device::cage_io_status_r()
{
	int result = 0;
	if (m_cpu_to_cage_ready)
		result |= 0x80;
	if (!m_cage_to_cpu_ready)
		result |= 0x40;
	return result;
}


uint16_t atari_cage_device::main_r()
{
	LOGMASKED(LOG_COMM, "%s:main read data = %04X\n", machine().describe_context(), m_soundlatch->read());
	m_cage_to_cpu_ready = 0;
	update_control_lines();
	return m_soundlatch->read();
}


TIMER_CALLBACK_MEMBER( atari_cage_device::cage_deferred_w )
{
	m_from_main = param;
	m_cpu_to_cage_ready = 1;
	update_control_lines();
	m_cpu->set_input_line(TMS3203X_IRQ0, ASSERT_LINE);
}


void atari_cage_device::main_w(uint16_t data)
{
	LOGMASKED(LOG_COMM, "%s:Command to CAGE = %04X\n", machine().describe_context(), data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(atari_cage_device::cage_deferred_w),this), data);
}


uint16_t atari_cage_device::control_r()
{
	uint16_t result = 0;

	if (m_cpu_to_cage_ready)
		result |= 2;
	if (m_cage_to_cpu_ready)
		result |= 1;

	return result;
}


void atari_cage_device::control_w(uint16_t data)
{
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

		memset(m_tms32031_io_regs, 0, 0x60 * 4);

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

void atari_cage_device::speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_cpu->eat_cycles(100);
	COMBINE_DATA(&m_speedup_ram[offset]);
}



/*************************************
 *
 *  CPU memory map & config
 *
 *************************************/

void atari_cage_device::cage_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("cageram");
	map(0x200000, 0x200000).nopw();
	map(0x400000, 0x47ffff).bankr("bootbank");
	map(0x808000, 0x8080ff).rw(FUNC(atari_cage_device::tms32031_io_r), FUNC(atari_cage_device::tms32031_io_w));
	map(0xa00000, 0xa00000).rw(FUNC(atari_cage_device::cage_from_main_r), FUNC(atari_cage_device::cage_to_main_w));
	map(0xc00000, 0xffffff).bankr("mainbank");
}


void atari_cage_seattle_device::cage_map_seattle(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("cageram");
	map(0x200000, 0x200000).nopw();
	map(0x400000, 0x47ffff).bankr("bootbank");
	map(0x808000, 0x8080ff).rw(FUNC(atari_cage_seattle_device::tms32031_io_r), FUNC(atari_cage_seattle_device::tms32031_io_w));
	map(0xa00000, 0xa00000).rw(FUNC(atari_cage_seattle_device::cage_from_main_r), FUNC(atari_cage_seattle_device::cage_from_main_ack_w));
	map(0xa00001, 0xa00001).w(FUNC(atari_cage_seattle_device::cage_to_main_w));
	map(0xa00003, 0xa00003).r(FUNC(atari_cage_seattle_device::cage_io_status_r));
	map(0xc00000, 0xffffff).bankr("mainbank");
}


//-------------------------------------------------
//  machine_add_config - add device configuration
//-------------------------------------------------

void atari_cage_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	TMS32031(config, m_cpu, XTAL(33'868'800));
	m_cpu->set_addrmap(AS_PROGRAM, &atari_cage_device::cage_map);
	m_cpu->set_mcbl_mode(true);

	TIMER(config, m_dma_timer).configure_generic(DEVICE_SELF, FUNC(atari_cage_device::dma_timer_callback));
	TIMER(config, m_timer[0]).configure_generic(DEVICE_SELF, FUNC(atari_cage_device::cage_timer_callback));
	TIMER(config, m_timer[1]).configure_generic(DEVICE_SELF, FUNC(atari_cage_device::cage_timer_callback));

	/* sound hardware */
	GENERIC_LATCH_16(config, m_soundlatch);

#if (DAC_BUFFER_CHANNELS == 4)
	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, *this, 0.50, 0);

	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, *this, 0.50, 1);

	DMADAC(config, m_dmadac[2]).add_route(ALL_OUTPUTS, *this, 0.50, 2);

	DMADAC(config, m_dmadac[3]).add_route(ALL_OUTPUTS, *this, 0.50, 3);
#else
	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, *this, 1.0, 0);

	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, *this, 1.0, 1);
#endif
	//add_route(ALL_OUTPUTS, *this, 0.50, 4); Subwoofer output
}

// Embedded in San francisco Rush Motherboard, 4 channel output connected to Quad Amp PCB and expanded to 5 channel (4 channel + subwoofer)
DEFINE_DEVICE_TYPE(ATARI_CAGE_SEATTLE, atari_cage_seattle_device, "atari_cage_seattle", "Atari CAGE Seattle")


//-------------------------------------------------
//  atari_cage_seattle_device - constructor
//-------------------------------------------------

atari_cage_seattle_device::atari_cage_seattle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atari_cage_device(mconfig, ATARI_CAGE_SEATTLE, tag, owner, clock)
{
}


//-------------------------------------------------
//  machine_add_config - add device configuration
//-------------------------------------------------


void atari_cage_seattle_device::device_add_mconfig(machine_config &config)
{
	atari_cage_device::device_add_mconfig(config);

	m_cpu->set_addrmap(AS_PROGRAM, &atari_cage_seattle_device::cage_map_seattle);
}
