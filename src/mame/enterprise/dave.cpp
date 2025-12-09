// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Designs DAVE emulation

**********************************************************************/

#include "emu.h"
#include "dave.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define STEP 0x08000



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DAVE, dave_device, "dave", "Intelligent Designs DAVE")


void dave_device::z80_program_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(dave_device::program_r), FUNC(dave_device::program_w));
}

void dave_device::z80_io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(dave_device::io_r), FUNC(dave_device::io_w));
}


void dave_device::program_map(address_map &map)
{
}

void dave_device::io_map(address_map &map)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dave_device - constructor
//-------------------------------------------------

dave_device::dave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DAVE, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_sound_interface(mconfig, *this),
	m_program_space_config("program", ENDIANNESS_LITTLE, 8, 22, 0, address_map_constructor(FUNC(dave_device::program_map), this)),
	m_io_space_config("io", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(dave_device::io_map), this)),
	m_write_irq(*this),
	m_write_lh(*this),
	m_write_rh(*this),
	m_irq_status(0)
{
}

dave_device::~dave_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dave_device::device_start()
{
	// allocate timers
	m_timer_1hz = timer_alloc(FUNC(dave_device::update_1hz_timer), this);
	m_timer_1hz->adjust(attotime::from_hz(2), 0, attotime::from_hz(2));

	m_timer_50hz = timer_alloc(FUNC(dave_device::update_50hz_timer), this);
	m_timer_50hz->adjust(attotime::from_hz(2000), 0, attotime::from_hz(2000));

	// state saving
	save_item(NAME(m_segment));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_period));
	save_item(NAME(m_count));
	save_item(NAME(m_level));
	save_item(NAME(m_level_or));
	save_item(NAME(m_level_and));
	save_item(NAME(m_mame_volumes));

	for (auto & elem : m_period)
		elem = (STEP * machine().sample_rate()) / 125000;

	for (auto & elem : m_count)
		elem = (STEP * machine().sample_rate()) / 125000;

	for (auto & elem : m_level)
		elem = 0;

	for (auto & elem : m_level_or)
		elem = 0;

	for (auto & elem : m_level_and)
		elem = 0;

	for (auto & elem : m_mame_volumes)
		elem = 0;

	/* dave has 3 tone channels and 1 noise channel.
	 the volumes are mixed internally and output as left and right volume */

	/* 3 tone channels + 1 noise channel */
	m_sound_stream_var = stream_alloc(0, 2, machine().sample_rate());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dave_device::device_reset()
{
	m_write_irq(CLEAR_LINE);

	for (auto & elem : m_segment)
		elem = 0;

	m_irq_status = 0;
	m_irq_enable = 0;

	for (auto & elem : m_regs)
		elem = 0;
}


//-------------------------------------------------
//  update_1hz_timer -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dave_device::update_1hz_timer)
{
	m_irq_status ^= IRQ_1HZ_DIVIDER;

	if (m_irq_status & IRQ_1HZ_DIVIDER)
		m_irq_status |= IRQ_1HZ_LATCH;

	update_interrupt();
}


//-------------------------------------------------
//  update_50hz_timer -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dave_device::update_50hz_timer)
{
	m_irq_status ^= IRQ_50HZ_DIVIDER;

	if (m_irq_status & IRQ_50HZ_DIVIDER)
		m_irq_status |= IRQ_50HZ_LATCH;

	update_interrupt();
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector dave_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_space_config),
		std::make_pair(AS_IO,      &m_io_space_config)
	};
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void dave_device::sound_stream_update(sound_stream &stream)
{
	/* 0 = channel 0 left volume, 1 = channel 0 right volume,
	 2 = channel 1 left volume, 3 = channel 1 right volume,
	 4 = channel 2 left volume, 5 = channel 2 right volume
	 6 = noise channel left volume, 7 = noise channel right volume */
	int output_volumes[8];
	int left_volume;
	int right_volume;

	//logerror("sound update!\n");

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		int vol[4];

		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = 0;

		for (int i = 0; i < 3; i++)
		{
			if ((m_regs[7] & (1 << i))==0)
			{
				if (m_level[i]) vol[i] += m_count[i];
				m_count[i] -= STEP;
				/* Period[i] is the half period of the square wave. Here, in each */
				/* loop I add Period[i] twice, so that at the end of the loop the */
				/* square wave is in the same status (0 or 1) it was at the start. */
				/* vol[i] is also incremented by Period[i], since the wave has been 1 */
				/* exactly half of the time, regardless of the initial position. */
				/* If we exit the loop in the middle, Output[i] has to be inverted */
				/* and vol[i] incremented only if the exit status of the square */
				/* wave is 1. */
				while (m_count[i] <= 0)
				{
					m_count[i] += m_period[i];
					if (m_count[i] > 0)
					{
						m_level[i] ^= 0x0ffffffff;
						if (m_level[i]) vol[i] += m_period[i];
						break;
					}
					m_count[i] += m_period[i];
					vol[i] += m_period[i];
				}
				if (m_level[i])
					vol[i] -= m_count[i];
			}
		}

		/* update volume outputs */

		/* setup output volumes for each channel */
		/* channel 0 */
		output_volumes[0] = ((m_level[0] & m_level_and[0]) | m_level_or[0]) & m_mame_volumes[0];
		output_volumes[1] = ((m_level[0] & m_level_and[1]) | m_level_or[1]) & m_mame_volumes[4];
		/* channel 1 */
		output_volumes[2] = ((m_level[1] & m_level_and[2]) | m_level_or[2]) & m_mame_volumes[1];
		output_volumes[3] = ((m_level[1] & m_level_and[3]) | m_level_or[3]) & m_mame_volumes[5];
		/* channel 2 */
		output_volumes[4] = ((m_level[2] & m_level_and[4]) | m_level_or[4]) & m_mame_volumes[2];
		output_volumes[5] = ((m_level[2] & m_level_and[5]) | m_level_or[5]) & m_mame_volumes[6];
		/* channel 3 */
		output_volumes[6] = ((m_level[3] & m_level_and[6]) | m_level_or[6]) & m_mame_volumes[3];
		output_volumes[7] = ((m_level[3] & m_level_and[7]) | m_level_or[7]) & m_mame_volumes[7];

		left_volume = output_volumes[0] + output_volumes[2] + output_volumes[4] + output_volumes[6];
		right_volume = output_volumes[1] + output_volumes[3] + output_volumes[5] + output_volumes[7];

		stream.put_int(0, sampindex, left_volume, 32768 * 4);
		stream.put_int(1, sampindex, right_volume, 32768 * 4);
	}
}


//-------------------------------------------------
//  int1_w - interrupt 1 write
//-------------------------------------------------

void dave_device::int1_w(int state)
{
	if (!(m_irq_status & IRQ_INT1) && state)
		m_irq_status |= IRQ_INT1_LATCH;

	if (state)
		m_irq_status |= IRQ_INT1;
	else
		m_irq_status &= ~IRQ_INT1;

	update_interrupt();
}


//-------------------------------------------------
//  int2_w - interrupt 2 write
//-------------------------------------------------

void dave_device::int2_w(int state)
{
	if (!(m_irq_status & IRQ_INT2) && state)
		m_irq_status |= IRQ_INT2_LATCH;

	if (state)
		m_irq_status |= IRQ_INT2;
	else
		m_irq_status &= ~IRQ_INT2;

	update_interrupt();
}


//-------------------------------------------------
//  program_r - program space read
//-------------------------------------------------

uint8_t dave_device::program_r(offs_t offset)
{
	uint8_t segment = m_segment[offset >> 14];
	offset = (segment << 14) | (offset & 0x3fff);

	return this->space(AS_PROGRAM).read_byte(offset);
}


//-------------------------------------------------
//  program_w - program space write
//-------------------------------------------------

void dave_device::program_w(offs_t offset, uint8_t data)
{
	uint8_t segment = m_segment[offset >> 14];
	offset = (segment << 14) | (offset & 0x3fff);

	this->space(AS_PROGRAM).write_byte(offset, data);
}


//-------------------------------------------------
//  io_r - I/O space read
//-------------------------------------------------

uint8_t dave_device::io_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 0xff)
	{
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
	case 0xa4:
	case 0xa5:
	case 0xa6:
	case 0xa7:
	case 0xa8:
	case 0xa9:
	case 0xaa:
	case 0xab:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xaf:
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf:
		data = 0xff;
		break;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		data = m_segment[offset & 0x03];
		break;

	case 0xb4:
		data = m_irq_status;
		break;

	default:
		data = this->space(AS_IO).read_byte(offset);
	}

	return data;
}


//-------------------------------------------------
//  io_w - I/O space write
//-------------------------------------------------

void dave_device::io_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
		/* channel 0 down-counter */
		case 0xa0:
		case 0xa1:
		/* channel 1 down-counter */
		case 0xa2:
		case 0xa3:
		/* channel 2 down-counter */
		case 0xa4:
		case 0xa5:
			{
				int count = 0;
				int channel_index = (offset>>1)&3;

				/* Fout = 125,000 / (n+1) Hz */

				/* sample rate/clock */


				/* get down-count */
				switch (offset & 0x01)
				{
					case 0:
					{
						count = (data & 0x0ff) | ((m_regs[(offset & 0x1f) + 1] & 0x0f)<<8);
					}
					break;

					case 1:
					{
						count = (m_regs[(offset & 0x1f) - 1] & 0x0ff) | ((data & 0x0f)<<8);

					}
					break;
				}

				count++;


				m_period[channel_index] = ((STEP  * machine().sample_rate())/125000) * count;

				m_regs[offset & 0x1f] = data;
			}
			break;

		/* channel 0 left volume */
		case 0xa8:
		/* channel 1 left volume */
		case 0xa9:
		/* channel 2 left volume */
		case 0xaa:
		/* noise channel left volume */
		case 0xab:
		/* channel 0 right volume */
		case 0xac:
		/* channel 1 right volume */
		case 0xad:
		/* channel 2 right volume */
		case 0xae:
		/* noise channel right volume */
		case 0xaf:
			{
				/* update mame version of volume from data written */
				/* 0x03f->0x07e00. Max is 0x07fff */
				/* I believe the volume is linear - to be checked! */
				m_mame_volumes[(offset & 0x1f) - 8] = (data & 0x03f) << 9;

				m_regs[offset & 0x1f] = data;
			}
			break;

		case 0xa6:
			break;

		case 0xa7:
		{
			/*  force => the value of this register is forced regardless of the wave
			        state,
			    remove => this value is force to zero so that it has no influence over
			        the final volume calculation, regardless of wave state
			    use => the volume value is dependant on the wave state and is included
			        in the final volume calculation */

			//logerror("selectable int ");
			switch ((data>>5) & 0x03)
			{
				case 0:
				{
					//logerror("1kHz\n");
					m_timer_50hz->adjust(attotime::from_hz(2000), 0, attotime::from_hz(2000));
				}
				break;

				case 1:
				{
					//logerror("50Hz\n");
					m_timer_50hz->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
				}
				break;

				case 2:
				{
					//logerror("tone channel 0\n");
				}
				break;

				case 3:
				{
					//logerror("tone channel 1\n");
				}
				break;
			}


			/* turn L.H audio output into D/A, outputting value in R8 */
			if (data & (1<<3))
			{
				/* force r8 value */
				m_level_or[0] = 0x0ffff;
				m_level_and[0] = 0x00;

				/* remove r9 value */
				m_level_or[2] = 0x000;
				m_level_and[2] = 0x00;

				/* remove r10 value */
				m_level_or[4] = 0x000;
				m_level_and[4] = 0x00;

				/* remove r11 value */
				m_level_or[6] = 0x000;
				m_level_and[6] = 0x00;
			}
			else
			{
				/* use r8 value */
				m_level_or[0] = 0x000;
				m_level_and[0] = 0xffff;

				/* use r9 value */
				m_level_or[2] = 0x000;
				m_level_and[2] = 0xffff;

				/* use r10 value */
				m_level_or[4] = 0x000;
				m_level_and[4] = 0xffff;

				/* use r11 value */
				m_level_or[6] = 0x000;
				m_level_and[6] = 0xffff;
			}

			/* turn L.H audio output into D/A, outputting value in R12 */
			if (data & (1<<4))
			{
				/* force r12 value */
				m_level_or[1] = 0x0ffff;
				m_level_and[1] = 0x00;

				/* remove r13 value */
				m_level_or[3] = 0x000;
				m_level_and[3] = 0x00;

				/* remove r14 value */
				m_level_or[5] = 0x000;
				m_level_and[5] = 0x00;

				/* remove r15 value */
				m_level_or[7] = 0x000;
				m_level_and[7] = 0x00;
			}
			else
			{
				/* use r12 value */
				m_level_or[1] = 0x000;
				m_level_and[1] = 0xffff;

				/* use r13 value */
				m_level_or[3] = 0x000;
				m_level_and[3] = 0xffff;

				/* use r14 value */
				m_level_or[5] = 0x000;
				m_level_and[5] = 0xffff;

				/* use r15 value */
				m_level_or[7] = 0x000;
				m_level_and[7] = 0xffff;
			}

			m_regs[offset & 0x1f] = data;
		}
		break;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		m_segment[offset & 0x03] = data;

		m_regs[offset & 0x1f] = data;
		break;

	case 0xb4:
		m_irq_enable = data;
		m_irq_status &= ~(m_irq_enable & IRQ_LATCH);
		update_interrupt();

		m_regs[offset & 0x1f] = data;
		break;

	case 0xbf:
		m_regs[offset & 0x1f] = data;
		break;

	default:
		this->space(AS_IO).write_byte(offset, data);
	}
}


//-------------------------------------------------
//  update_interrupt -
//-------------------------------------------------

void dave_device::update_interrupt()
{
	int state = ((m_irq_status & (m_irq_enable << 1)) & IRQ_LATCH) ? ASSERT_LINE : CLEAR_LINE;

	m_write_irq(state);
}
