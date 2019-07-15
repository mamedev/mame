// license:BSD-3-Clause
// copyright-holders:Curt Coder,AJR
/**********************************************************************

    Intel 8155/8156 - 2048-Bit Static MOS RAM with I/O Ports and Timer

    The timer primarily functions as a square-wave generator, but can
    also be programmed for a single-cycle low pulse on terminal count.

    The only difference between 8155 and 8156 is that pin 8 (CE) is
    active low on the former device and active high on the latter.

    National's NSC810 RAM-I/O-Timer is pin-compatible with the Intel
    8156, but has different I/O registers (including a second timer)
    with incompatible mapping.

**********************************************************************/

/*

    TODO:

    - ALT 3 and ALT 4 strobed port modes
    - optional NVRAM backup for CMOS versions

*/

#include "emu.h"
#include "i8155.h"


// device type definitions
DEFINE_DEVICE_TYPE(I8155, i8155_device, "i8155", "Intel 8155 RAM, I/O & Timer")
DEFINE_DEVICE_TYPE(I8156, i8156_device, "i8156", "Intel 8156 RAM, I/O & Timer")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_PORT (1U << 0)
#define LOG_TIMER (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

enum
{
	REGISTER_COMMAND = 0,
	REGISTER_STATUS = 0,
	REGISTER_PORT_A,
	REGISTER_PORT_B,
	REGISTER_PORT_C,
	REGISTER_TIMER_LOW,
	REGISTER_TIMER_HIGH
};

enum
{
	PORT_A = 0,
	PORT_B,
	PORT_C,
	PORT_COUNT
};

enum
{
	PORT_MODE_INPUT = 0,
	PORT_MODE_OUTPUT,
	PORT_MODE_STROBED_PORT_A,   // not supported
	PORT_MODE_STROBED           // not supported
};

enum
{
	MEMORY = 0,
	IO
};

#define COMMAND_PA                  0x01
#define COMMAND_PB                  0x02
#define COMMAND_PC_MASK             0x0c
#define COMMAND_PC_ALT_1            0x00
#define COMMAND_PC_ALT_2            0x0c
#define COMMAND_PC_ALT_3            0x04    // not supported
#define COMMAND_PC_ALT_4            0x08    // not supported
#define COMMAND_IEA                 0x10    // not supported
#define COMMAND_IEB                 0x20    // not supported
#define COMMAND_TM_MASK             0xc0
#define COMMAND_TM_NOP              0x00
#define COMMAND_TM_STOP             0x40
#define COMMAND_TM_STOP_AFTER_TC    0x80
#define COMMAND_TM_START            0xc0

#define STATUS_INTR_A               0x01    // not supported
#define STATUS_A_BF                 0x02    // not supported
#define STATUS_INTE_A               0x04    // not supported
#define STATUS_INTR_B               0x08    // not supported
#define STATUS_B_BF                 0x10    // not supported
#define STATUS_INTE_B               0x20    // not supported
#define STATUS_TIMER                0x40

#define TIMER_MODE_MASK             0xc0
#define TIMER_MODE_AUTO_RELOAD      0x40
#define TIMER_MODE_TC_PULSE         0x80



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline uint8_t i8155_device::get_timer_mode()
{
	return (m_count_loaded >> 8) & TIMER_MODE_MASK;
}

inline void i8155_device::timer_output(int to)
{
	if (to == m_to)
		return;

	m_to = to;
	m_out_to_cb(to);

	LOGMASKED(LOG_TIMER, "Timer output: %u\n", to);
}

inline void i8155_device::timer_stop_count()
{
	// stop counting
	m_timer->enable(0);

	// clear timer output
	timer_output(1);
}

inline void i8155_device::timer_reload_count()
{
	m_count_loaded = m_count_length;

	// valid counts range from 2 to 3FFF
	if ((m_count_length & 0x3fff) < 2)
	{
		timer_stop_count();
		return;
	}

	// begin the odd half of the count, with one extra cycle if count is odd
	m_counter = (m_count_loaded & 0x3ffe) | 1;
	m_count_extra = BIT(m_count_loaded, 0);

	// set up our timer
	m_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));
	timer_output(1);

	switch (get_timer_mode())
	{
	case 0:
		// puts out LOW during second half of count
		LOGMASKED(LOG_TIMER, "Timer loaded with %d (Mode: LOW)\n", m_count_loaded & 0x3fff);
		break;

	case TIMER_MODE_AUTO_RELOAD:
		// square wave, i.e. the period of the square wave equals the count length programmed with automatic reload at terminal count
		LOGMASKED(LOG_TIMER, "Timer loaded with %d (Mode: Square wave)\n", m_count_loaded & 0x3fff);
		break;

	case TIMER_MODE_TC_PULSE:
		// single pulse upon TC being reached
		LOGMASKED(LOG_TIMER, "Timer loaded with %d (Mode: Single pulse)\n", m_count_loaded & 0x3fff);
		break;

	case TIMER_MODE_TC_PULSE | TIMER_MODE_AUTO_RELOAD:
		// automatic reload, i.e. single pulse every time TC is reached
		LOGMASKED(LOG_TIMER, "Timer loaded with %d (Mode: Automatic reload)\n", m_count_loaded & 0x3fff);
		break;
	}
}

inline int i8155_device::get_port_mode(int port)
{
	int mode = -1;

	switch (port)
	{
	case PORT_A:
		mode = (m_command & COMMAND_PA) ? PORT_MODE_OUTPUT : PORT_MODE_INPUT;
		break;

	case PORT_B:
		mode = (m_command & COMMAND_PB) ? PORT_MODE_OUTPUT : PORT_MODE_INPUT;
		break;

	case PORT_C:
		switch (m_command & COMMAND_PC_MASK)
		{
		case COMMAND_PC_ALT_1: mode = PORT_MODE_INPUT;          break;
		case COMMAND_PC_ALT_2: mode = PORT_MODE_OUTPUT;         break;
		case COMMAND_PC_ALT_3: mode = PORT_MODE_STROBED_PORT_A; break;
		case COMMAND_PC_ALT_4: mode = PORT_MODE_STROBED;        break;
		}
		break;
	}

	return mode;
}

inline uint8_t i8155_device::read_port(int port)
{
	uint8_t data = 0;

	switch (get_port_mode(port))
	{
	case PORT_MODE_INPUT:
		data = (port == PORT_A) ? m_in_pa_cb(0) : ((port == PORT_B) ? m_in_pb_cb(0) : m_in_pc_cb(0));
		break;

	case PORT_MODE_OUTPUT:
		data = m_output[port];
		break;

	default:
		// strobed mode not implemented yet
		logerror("8155 Unsupported Port C mode!\n");
		break;
	}

	return data;
}

inline void i8155_device::write_port(int port, uint8_t data)
{
	switch (get_port_mode(port))
	{
	case PORT_MODE_OUTPUT:
		m_output[port] = data;
		if (port == PORT_A)
			m_out_pa_cb((offs_t)0, m_output[port]);
		else if (port == PORT_B)
			m_out_pb_cb((offs_t)0, m_output[port]);
		else
			m_out_pc_cb((offs_t)0, m_output[port]);
		break;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8155_device - constructor
//-------------------------------------------------

i8155_device::i8155_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8155_device(mconfig, I8155, tag, owner, clock)
{
}

i8155_device::i8155_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		m_in_pa_cb(*this),
		m_in_pb_cb(*this),
		m_in_pc_cb(*this),
		m_out_pa_cb(*this),
		m_out_pb_cb(*this),
		m_out_pc_cb(*this),
		m_out_to_cb(*this),
		m_command(0),
		m_status(0),
		m_count_length(0),
		m_count_loaded(0),
		m_counter(0),
		m_count_extra(false),
		m_to(0)
{
}


//-------------------------------------------------
//  i8156_device - constructor
//-------------------------------------------------

i8156_device::i8156_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8155_device(mconfig, I8156, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8155_device::device_start()
{
	// resolve callbacks
	m_in_pa_cb.resolve_safe(0);
	m_in_pb_cb.resolve_safe(0);
	m_in_pc_cb.resolve_safe(0);
	m_out_pa_cb.resolve_safe();
	m_out_pb_cb.resolve_safe();
	m_out_pc_cb.resolve_safe();
	m_out_to_cb.resolve_safe();

	// allocate RAM
	m_ram = make_unique_clear<uint8_t[]>(256);

	// allocate timers
	m_timer = timer_alloc();

	// register for state saving
	save_item(NAME(m_io_m));
	save_item(NAME(m_ad));
	save_item(NAME(m_command));
	save_item(NAME(m_status));
	save_item(NAME(m_output));
	save_pointer(NAME(m_ram), 256);
	save_item(NAME(m_count_length));
	save_item(NAME(m_count_loaded));
	save_item(NAME(m_count_extra));
	save_item(NAME(m_counter));
	save_item(NAME(m_to));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8155_device::device_reset()
{
	// clear output registers
	m_output[PORT_A] = 0;
	m_output[PORT_B] = 0;
	m_output[PORT_C] = 0;

	// set ports to input mode
	register_w(REGISTER_COMMAND, m_command & ~(COMMAND_PA | COMMAND_PB | COMMAND_PC_MASK));

	// clear timer flag
	m_status &= ~STATUS_TIMER;

	// stop timer
	timer_stop_count();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void i8155_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (m_count_extra)
	{
		m_count_extra = false;
		return;
	}

	// count down by twos
	m_counter -= 2;

	if (m_counter == 1)
	{
		LOGMASKED(LOG_TIMER, "Timer count half finished\n");

		// reload the even half of the count
		m_counter = m_count_loaded & 0x3ffe;

		// square wave modes produce a low output in the second half of the counting period
		if ((get_timer_mode() & TIMER_MODE_TC_PULSE) == 0)
			timer_output(0);
	}
	else if (m_counter == 2)
	{
		if ((get_timer_mode() & TIMER_MODE_TC_PULSE) != 0)
		{
			// pulse low on TC being reached
			timer_output(0);
		}

		// set timer flag
		m_status |= STATUS_TIMER;
	}
	else if (m_counter == 0)
	{
		timer_output(1);

		if ((get_timer_mode() & TIMER_MODE_AUTO_RELOAD) == 0 || (m_command & COMMAND_TM_MASK) == COMMAND_TM_STOP_AFTER_TC)
		{
			// stop timer
			timer_stop_count();
			LOGMASKED(LOG_TIMER, "Timer stopped\n");
		}
		else
		{
			// automatically reload the counter
			timer_reload_count();
		}
	}
}


//-------------------------------------------------
//  io_r - register read
//-------------------------------------------------

uint8_t i8155_device::io_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 0x07)
	{
	case REGISTER_STATUS:
		data = m_status;

		// clear timer flag
		m_status &= ~STATUS_TIMER;
		break;

	case REGISTER_PORT_A:
		data = read_port(PORT_A);
		break;

	case REGISTER_PORT_B:
		data = read_port(PORT_B);
		break;

	case REGISTER_PORT_C:
		data = read_port(PORT_C) | 0xc0;
		break;

	case REGISTER_TIMER_LOW:
		data = m_counter & 0xff;
		break;

	case REGISTER_TIMER_HIGH:
		data = (m_counter >> 8 & 0x3f) | get_timer_mode();
		break;
	}

	return data;
}


//-------------------------------------------------
//  register_w - register write
//-------------------------------------------------

void i8155_device::register_w(int offset, uint8_t data)
{
	switch (offset & 0x07)
	{
	case REGISTER_COMMAND:
		m_command = data;

		LOGMASKED(LOG_PORT, "Port A Mode: %s\n", (data & COMMAND_PA) ? "output" : "input");
		LOGMASKED(LOG_PORT, "Port B Mode: %s\n", (data & COMMAND_PB) ? "output" : "input");

		LOGMASKED(LOG_PORT, "Port A Interrupt: %s\n", (data & COMMAND_IEA) ? "enabled" : "disabled");
		LOGMASKED(LOG_PORT, "Port B Interrupt: %s\n", (data & COMMAND_IEB) ? "enabled" : "disabled");

		switch (data & COMMAND_PC_MASK)
		{
		case COMMAND_PC_ALT_1:
			LOGMASKED(LOG_PORT, "Port C Mode: Alt 1\n");
			break;

		case COMMAND_PC_ALT_2:
			LOGMASKED(LOG_PORT, "Port C Mode: Alt 2\n");
			break;

		case COMMAND_PC_ALT_3:
			LOGMASKED(LOG_PORT, "Port C Mode: Alt 3\n");
			break;

		case COMMAND_PC_ALT_4:
			LOGMASKED(LOG_PORT, "Port C Mode: Alt 4\n");
			break;
		}

		switch (data & COMMAND_TM_MASK)
		{
		case COMMAND_TM_NOP:
			// do not affect counter operation
			break;

		case COMMAND_TM_STOP:
			// NOP if timer has not started, stop counting if the timer is running
			LOGMASKED(LOG_PORT, "Timer Command: Stop\n");
			timer_stop_count();
			break;

		case COMMAND_TM_STOP_AFTER_TC:
			// stop immediately after present TC is reached (NOP if timer has not started)
			LOGMASKED(LOG_PORT, "Timer Command: Stop after TC\n");
			break;

		case COMMAND_TM_START:
			LOGMASKED(LOG_PORT, "Timer Command: Start\n");

			if (m_timer->enabled())
			{
				// if timer is running, start the new mode and CNT length immediately after present TC is reached
			}
			else
			{
				// load mode and CNT length and start immediately after loading (if timer is not running)
				timer_reload_count();
			}
			break;
		}
		break;

	case REGISTER_PORT_A:
		write_port(PORT_A, data);
		break;

	case REGISTER_PORT_B:
		write_port(PORT_B, data);
		break;

	case REGISTER_PORT_C:
		write_port(PORT_C, data & 0x3f);
		break;

	case REGISTER_TIMER_LOW:
		m_count_length = (m_count_length & 0xff00) | data;
		break;

	case REGISTER_TIMER_HIGH:
		m_count_length = (data << 8) | (m_count_length & 0xff);
		break;
	}
}

//-------------------------------------------------
//  io_w - register write
//-------------------------------------------------

void i8155_device::io_w(offs_t offset, uint8_t data)
{
	register_w(offset, data);
}


//-------------------------------------------------
//  memory_r - internal RAM read
//-------------------------------------------------

uint8_t i8155_device::memory_r(offs_t offset)
{
	return m_ram[offset & 0xff];
}


//-------------------------------------------------
//  memory_w - internal RAM write
//-------------------------------------------------

void i8155_device::memory_w(offs_t offset, uint8_t data)
{
	m_ram[offset & 0xff] = data;
}


//-------------------------------------------------
//  ale_w - address latch write
//-------------------------------------------------

void i8155_device::ale_w(offs_t offset, uint8_t data)
{
	// I/O / memory select
	m_io_m = BIT(offset, 0);

	// address
	m_ad = data;
}


//-------------------------------------------------
//  data_r - memory or I/O read
//-------------------------------------------------

uint8_t i8155_device::data_r()
{
	uint8_t data = 0;

	switch (m_io_m)
	{
	case MEMORY:
		data = memory_r(m_ad);
		break;

	case IO:
		data = io_r(m_ad);
		break;
	}

	return data;
}


//-------------------------------------------------
//  data_w - memory or I/O write
//-------------------------------------------------

void i8155_device::data_w(uint8_t data)
{
	switch (m_io_m)
	{
	case MEMORY:
		memory_w(m_ad, data);
		break;

	case IO:
		io_w(m_ad, data);
		break;
	}
}
