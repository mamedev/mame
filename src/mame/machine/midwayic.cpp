// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Emulation of various Midway ICs

***************************************************************************/

#include "emu.h"
#include "midwayic.h"


#define LOG_NVRAM           (0)

#define PRINTF_DEBUG        (0)
#define LOG_IOASIC          (0)
#define LOG_FIFO            (0)


/*************************************
 *
 *  Constants
 *
 *************************************/

#define FIFO_SIZE           512

/*************************************
 *
 *  Serial number input kludge
 *
 *************************************/

static INPUT_PORTS_START( pic_serial_adjust )
	PORT_START("SERIAL_DIGIT")
	PORT_DIPNAME( 0x0f, 0x06, "Serial Low Digit")
	PORT_DIPSETTING(    0x00, "0")
	PORT_DIPSETTING(    0x01, "1")
	PORT_DIPSETTING(    0x02, "2")
	PORT_DIPSETTING(    0x03, "3")
	PORT_DIPSETTING(    0x04, "4")
	PORT_DIPSETTING(    0x05, "5")
	PORT_DIPSETTING(    0x06, "6")
	PORT_DIPSETTING(    0x07, "7")
	PORT_DIPSETTING(    0x08, "8")
	PORT_DIPSETTING(    0x09, "9")
	PORT_BIT( 0xf0, 0x00, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor midway_serial_pic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pic_serial_adjust);
}

/*************************************
 *
 *  Serial number encoding
 *
 *************************************/

void midway_serial_pic_device::generate_serial_data(int upper)
{
	int year = atoi(machine().system().year), month = 12, day = 11;
	uint32_t serial_number, temp;
	uint8_t serial_digit[9];

	serial_number = 123450;
	serial_number += upper * 1000000;
	serial_number += m_io_serial_digit->read() & 0x0f;

	serial_digit[0] = (serial_number / 100000000) % 10;
	serial_digit[1] = (serial_number / 10000000) % 10;
	serial_digit[2] = (serial_number / 1000000) % 10;
	serial_digit[3] = (serial_number / 100000) % 10;
	serial_digit[4] = (serial_number / 10000) % 10;
	serial_digit[5] = (serial_number / 1000) % 10;
	serial_digit[6] = (serial_number / 100) % 10;
	serial_digit[7] = (serial_number / 10) % 10;
	serial_digit[8] = (serial_number / 1) % 10;

	m_data[12] = machine().rand() & 0xff;
	m_data[13] = machine().rand() & 0xff;

	m_data[14] = 0; /* ??? */
	m_data[15] = 0; /* ??? */

	temp = 0x174 * (year - 1980) + 0x1f * (month - 1) + day;
	m_data[10] = (temp >> 8) & 0xff;
	m_data[11] = temp & 0xff;

	temp = serial_digit[4] + serial_digit[7] * 10 + serial_digit[1] * 100;
	temp = (temp + 5 * m_data[13]) * 0x1bcd + 0x1f3f0;
	m_data[7] = temp & 0xff;
	m_data[8] = (temp >> 8) & 0xff;
	m_data[9] = (temp >> 16) & 0xff;

	temp = serial_digit[6] + serial_digit[8] * 10 + serial_digit[0] * 100 + serial_digit[2] * 10000;
	temp = (temp + 2 * m_data[13] + m_data[12]) * 0x107f + 0x71e259;
	m_data[3] = temp & 0xff;
	m_data[4] = (temp >> 8) & 0xff;
	m_data[5] = (temp >> 16) & 0xff;
	m_data[6] = (temp >> 24) & 0xff;

	temp = serial_digit[5] * 10 + serial_digit[3] * 100;
	temp = (temp + m_data[12]) * 0x245 + 0x3d74;
	m_data[0] = temp & 0xff;
	m_data[1] = (temp >> 8) & 0xff;
	m_data[2] = (temp >> 16) & 0xff;
}



/*************************************
 *
 *  Original serial number PIC
 *  interface - simulation
 *
 *************************************/

void midway_serial_pic_device::serial_register_state()
{
	save_item(NAME(m_data));
	save_item(NAME(m_buff));
	save_item(NAME(m_idx));
	save_item(NAME(m_status));
	save_item(NAME(m_bits));
}

DEFINE_DEVICE_TYPE(MIDWAY_SERIAL_PIC, midway_serial_pic_device, "midway_serial_pic_sim", "Midway Serial PIC Simulation")


//-------------------------------------------------
//  midway_serial_pic_device - constructor
//-------------------------------------------------

midway_serial_pic_device::midway_serial_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	midway_serial_pic_device(mconfig, MIDWAY_SERIAL_PIC, tag, owner, clock)
{
}

midway_serial_pic_device::midway_serial_pic_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_io_serial_digit(*this, "SERIAL_DIGIT"),
	m_upper(0),
	m_buff(0),
	m_idx(0),
	m_status(0),
	m_bits(0)
{
	memset(m_data,0,sizeof(m_data));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_serial_pic_device::device_start()
{
	serial_register_state();
}

void midway_serial_pic_device::device_reset()
{
	generate_serial_data(m_upper);
}

WRITE_LINE_MEMBER(midway_serial_pic_device::reset_w)
{
	if (state)
	{
		m_idx = 0;
		m_status = 0;
		m_buff = 0;
	}
}


u8 midway_serial_pic_device::status_r()
{
	return m_status;
}


u8 midway_serial_pic_device::read()
{
	if (!machine().side_effects_disabled())
	{
		logerror("%s:security R = %04X\n", machine().describe_context(), m_buff);
		m_status = 1;
	}
	return m_buff;
}


void midway_serial_pic_device::write(u8 data)
{
	logerror("%s:security W = %04X\n", machine().describe_context(), data);

	/* status seems to reflect the clock bit */
	m_status = (data >> 4) & 1;

	/* on the falling edge, clock the next data byte through */
	if (!m_status)
	{
		/* the self-test writes 1F, 0F, and expects to read an F in the low 4 bits */
		if (data & 0x0f)
			m_buff = data & 0xf;
		else
			m_buff = m_data[m_idx++ % sizeof(m_data)];
	}
}


/*************************************
 *
 *  Original serial number PIC
 *  interface - emulation
 *
 *  PIC16C57 wiring notes:
 *  PORTA - 4bit command in (usually 0 = read SN#)
 *  PORTB - 8bit data out
 *  PORTC bit 7 - access clock in
 *  PORTC bit 6 - status out
 *  PORTC bit 2 - in/out (optional) MK41T56N RTC/NVRAM Data
 *  PORTC bit 1 - out (optional) MK41T56N RTC/NVRAM Clock
 *
 *************************************/


DEFINE_DEVICE_TYPE(MIDWAY_SERIAL_PIC_EMU, midway_serial_pic_emu_device, "midway_serial_pic_emu", "Midway Serial PIC Emulation")


//-------------------------------------------------
//  midway_serial_pic_emu_device - constructor
//-------------------------------------------------

midway_serial_pic_emu_device::midway_serial_pic_emu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MIDWAY_SERIAL_PIC_EMU, tag, owner, clock)
	, m_pic(*this, "pic")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_serial_pic_emu_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_data_out));
	save_item(NAME(m_clk));
	save_item(NAME(m_status));

	m_command = 0;
	m_data_out = 0;
	m_clk = 0;
	m_status = 0;
}

WRITE_LINE_MEMBER(midway_serial_pic_emu_device::reset_w)
{
	if (!state) // fixme, PIC should be stopped while 0 and start running at 0->1 transition
		m_pic->reset();
}

u8 midway_serial_pic_emu_device::status_r()
{
	return m_status;
}

u8 midway_serial_pic_emu_device::read()
{
	return m_data_out;
}

void midway_serial_pic_emu_device::write(u8 data)
{
	// perhaps this should be split in 2 handlers ?
	m_command = data & 0x0f;
	m_clk = BIT(data, 4);
}

u8 midway_serial_pic_emu_device::read_c()
{
	u8 data = 0;
	data |= m_clk << 7;
	// bit 2 RTC Data
	return data;
}

void midway_serial_pic_emu_device::write_c(u8 data)
{
	m_status = BIT(data, 6);
	// bits 1 and 2 is RTC Clock and Data
//  printf("%s: write_c %02x\n", machine().describe_context().c_str(), data);
}

void midway_serial_pic_emu_device::device_add_mconfig(machine_config &config)
{
	PIC16C57(config, m_pic, 4000000);    /* ? Mhz */
	m_pic->read_a().set([this]() { return m_command; });
	m_pic->write_b().set([this](u8 data) { m_data_out = data; });
	m_pic->read_c().set(FUNC(midway_serial_pic_emu_device::read_c));
	m_pic->write_c().set(FUNC(midway_serial_pic_emu_device::write_c));
}


/*************************************
 *
 *  Second generation serial number
 *  PIC interface; this version also
 *  contained some NVRAM and a real
 *  time clock
 *
 *************************************/

static inline uint8_t make_bcd(uint8_t data)
{
	return ((data / 10) << 4) | (data % 10);
}

DEFINE_DEVICE_TYPE(MIDWAY_SERIAL_PIC2, midway_serial_pic2_device, "midway_serial_pic2", "Midway Serial PIC 2")


//-------------------------------------------------
//  midway_serial_pic2_device - constructor
//-------------------------------------------------

midway_serial_pic2_device::midway_serial_pic2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	midway_serial_pic2_device(mconfig, MIDWAY_SERIAL_PIC2, tag, owner, clock)
{
}

midway_serial_pic2_device::midway_serial_pic2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	midway_serial_pic_device(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_latch(0),
	m_state(0),
	m_index(0),
	m_total(0),
	m_nvram_addr(0),
	m_time_index(0),
	m_time_just_written(0),
	m_yearoffs(0),
	m_time_write_timer(nullptr)
{
	memset(m_buffer,0,sizeof(m_buffer));
	memset(m_time_buf,0,sizeof(m_time_buf));
	memset(m_nvram,0,sizeof(m_nvram));
	memset(m_default_nvram,0,sizeof(m_default_nvram));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_serial_pic2_device::device_start()
{
	midway_serial_pic_device::device_start();
	//void midway_serial_pic2_init(running_machine &machine, int upper, int yearoffs)
	pic_register_state();

	//m_yearoffs = yearoffs;
	m_time_just_written = 0;
	m_time_write_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(midway_serial_pic2_device::reset_timer),this));
	memset(m_default_nvram, 0xff, sizeof(m_default_nvram));
}


TIMER_CALLBACK_MEMBER( midway_serial_pic2_device::reset_timer )
{
	m_time_just_written = 0;
}


void midway_serial_pic2_device::pic_register_state()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_latch_expire_time));
	save_item(NAME(m_state));
	save_item(NAME(m_index));
	save_item(NAME(m_total));
	save_item(NAME(m_nvram_addr));
	save_item(NAME(m_buffer));
	save_item(NAME(m_nvram));
	save_item(NAME(m_default_nvram));
	save_item(NAME(m_time_buf));
	save_item(NAME(m_time_index));
	save_item(NAME(m_time_just_written));
	save_item(NAME(m_yearoffs));
}



void midway_serial_pic2_device::set_default_nvram(const uint8_t *nvram)
{
	memcpy(m_default_nvram, nvram, sizeof(m_default_nvram));
}


u8 midway_serial_pic2_device::status_r()
{
	uint8_t result = 0;

	if (!machine().side_effects_disabled())
	{
		/* if we're still holding the data ready bit high, do it */
		if (m_latch & 0xf00)
		{
			if (machine().time() > m_latch_expire_time)
				m_latch &= 0xff;
			else
				m_latch -= 0x100;
			result = 1;
		}

		logerror("%s:PIC status %d\n", machine().describe_context(), result);
	}
	return result;
}


u8 midway_serial_pic2_device::read()
{
	uint8_t result = 0;

	/* PIC data register */
	if (!machine().side_effects_disabled())
		logerror("%s:PIC data read (index=%d total=%d latch=%03X) =", machine().describe_context(), m_index, m_total, m_latch);

	/* return the current result */
	if (m_latch & 0xf00)
		result = m_latch & 0xff;

	/* otherwise, return 0xff if we have data ready */
	else if (m_index < m_total)
		result = 0xff;

	if (!machine().side_effects_disabled())
		logerror("%02X\n", result);
	return result;
}


void midway_serial_pic2_device::write(u8 data)
{
	static FILE *nvramlog;
	if (LOG_NVRAM && !nvramlog)
		nvramlog = fopen("nvram.log", "w");

	/* PIC command register */
	if (m_state == 0)
		logerror("%s:PIC command %02X\n", machine().describe_context(), data);
	else
		logerror("%s:PIC data %02X\n", machine().describe_context(), data);

	/* store in the latch, along with a bit to indicate we have data */
	m_latch = (data & 0x00f) | 0x480;
	m_latch_expire_time = machine().time() + attotime::from_msec(1);
	if (data & 0x10)
	{
		int cmd = m_state ? (m_state & 0x0f) : (m_latch & 0x0f);
		switch (cmd)
		{
			/* written to latch the next byte of data */
			case 0:
				if (m_index < m_total)
					m_latch = 0x400 | m_buffer[m_index++];
				break;

			/* fetch the serial number */
			case 1:
				/* note: Biofreaks assumes that it can latch the next byte this way */
				if (m_index < m_total)
					m_latch = 0x400 | m_buffer[m_index++];
				else
				{
					memcpy(m_buffer, m_data, 16);
					m_total = 16;
					m_index = 0;
					machine().debug_break();
				}
				break;

			/* read the clock */
			case 3:
			{
				/* stuff it into the data bytes */
				m_index = 0;
				m_total = 0;

				/* if we haven't written a new time recently, use the real live time */
				if (!m_time_just_written)
				{
					system_time systime;
					machine().base_datetime(systime);

					m_buffer[m_total++] = make_bcd(systime.local_time.second);
					m_buffer[m_total++] = make_bcd(systime.local_time.minute);
					m_buffer[m_total++] = make_bcd(systime.local_time.hour);
					m_buffer[m_total++] = make_bcd(systime.local_time.weekday + 1);
					m_buffer[m_total++] = make_bcd(systime.local_time.mday);
					m_buffer[m_total++] = make_bcd(systime.local_time.month + 1);
					m_buffer[m_total++] = make_bcd(systime.local_time.year - 1900 - m_yearoffs);
				}

				/* otherwise, just parrot back what was written to pass self tests */
				else
				{
					m_buffer[m_total++] = m_time_buf[0];
					m_buffer[m_total++] = m_time_buf[1];
					m_buffer[m_total++] = m_time_buf[2];
					m_buffer[m_total++] = m_time_buf[3];
					m_buffer[m_total++] = m_time_buf[4];
					m_buffer[m_total++] = m_time_buf[5];
					m_buffer[m_total++] = m_time_buf[6];
				}
				break;
			}

			/* write the clock */
			case 4:

				/* if coming from state 0, go to state 1 (this is just the command byte) */
				if (m_state == 0)
				{
					m_state = 0x14;
					m_time_index = 0;
				}

				/* if in states 1-2 put data in the buffer until it's full */
				else if (m_state == 0x14)
				{
					m_time_buf[m_time_index] = m_latch & 0x0f;
					m_state = 0x24;
				}
				else if (m_state == 0x24)
				{
					m_time_buf[m_time_index++] |= m_latch << 4;

					/* if less than 7 bytes accumulated, go back to state 1 */
					if (m_time_index < 7)
						m_state = 0x14;

					/* otherwise, flag the time as having just been written for 1/2 second */
					else
					{
						m_time_write_timer->adjust(attotime::from_msec(500));
						m_time_just_written = 1;
						m_state = 0;
					}
				}
				break;

			/* write to NVRAM */
			case 5:

				/* if coming from state 0, go to state 1 (this is just the command byte) */
				if (m_state == 0)
					m_state = 0x15;

				/* coming from state 1, go to state 2 and latch the low 4 address bits */
				else if (m_state == 0x15)
				{
					m_nvram_addr = m_latch & 0x0f;
					m_state = 0x25;
				}

				/* coming from state 2, go to state 3 and latch the high 4 address bits */
				else if (m_state == 0x25)
				{
					m_state = 0x35;
					m_nvram_addr |= m_latch << 4;
				}

				/* coming from state 3, go to state 4 and write the low 4 bits */
				else if (m_state == 0x35)
				{
					m_state = 0x45;
					m_nvram[m_nvram_addr] = m_latch & 0x0f;
				}

				/* coming from state 4, reset the states and write the upper 4 bits */
				else if (m_state == 0x45)
				{
					m_state = 0;
					m_nvram[m_nvram_addr] |= m_latch << 4;
					if (nvramlog)
						fprintf(nvramlog, "Write byte %02X = %02X\n", m_nvram_addr, m_nvram[m_nvram_addr]);
				}
				break;

			/* read from NVRAM */
			case 6:

				/* if coming from state 0, go to state 1 (this is just the command byte) */
				if (m_state == 0)
					m_state = 0x16;

				/* coming from state 1, go to state 2 and latch the low 4 address bits */
				else if (m_state == 0x16)
				{
					m_nvram_addr = m_latch & 0x0f;
					m_state = 0x26;
				}

				/* coming from state 2, reset the states and make the data available */
				else if (m_state == 0x26)
				{
					m_state = 0;
					m_nvram_addr |= m_latch << 4;

					m_total = 0;
					m_index = 0;
					m_buffer[m_total++] = m_nvram[m_nvram_addr];
					if (nvramlog)
						fprintf(nvramlog, "Read byte %02X = %02X\n", m_nvram_addr, m_nvram[m_nvram_addr]);
				}
				break;

			/* reflect inverted? (Cruisin' Exotica) */
			case 8:
				m_latch = 0x400 | (~cmd & 0xff);
				break;
		}
	}
}

void midway_serial_pic2_device::nvram_default()
{
	memcpy(m_nvram, m_default_nvram, sizeof(m_nvram));
}

bool midway_serial_pic2_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_nvram, sizeof(m_nvram), actual) && actual == sizeof(m_nvram);
}

bool midway_serial_pic2_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_nvram, sizeof(m_nvram), actual) && actual == sizeof(m_nvram);
}


/*************************************
 *
 *  The I/O ASIC was first introduced
 *  in War Gods, then later used on
 *  the Seattle hardware
 *
 *************************************/

enum
{
	IOASIC_PORT0,       /* 0: input port 0 */
	IOASIC_PORT1,       /* 1: input port 1 */
	IOASIC_PORT2,       /* 2: input port 2 */
	IOASIC_PORT3,       /* 3: input port 3 */
	IOASIC_UARTCONTROL, /* 4: controls some UART behavior */
	IOASIC_UARTOUT,     /* 5: UART output */
	IOASIC_UARTIN,      /* 6: UART input */
	IOASIC_COIN,        /* 7: triggered on coin insertion */
	IOASIC_SOUNDCTL,    /* 8: sound communications control */
	IOASIC_SOUNDOUT,    /* 9: sound output port */
	IOASIC_SOUNDSTAT,   /* a: sound status port */
	IOASIC_SOUNDIN,     /* b: sound input port */
	IOASIC_PICOUT,      /* c: PIC output port */
	IOASIC_PICIN,       /* d: PIC input port */
	IOASIC_INTSTAT,     /* e: interrupt status */
	IOASIC_INTCTL       /* f: interrupt control */
};

void midway_ioasic_device::ioasic_register_state()
{
	save_item(NAME(m_reg));
	save_item(NAME(m_shuffle_active));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_sound_irq_state));
	save_item(NAME(m_auto_ack));
	save_item(NAME(m_force_fifo_full));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_in));
	save_item(NAME(m_fifo_out));
	save_item(NAME(m_fifo_bytes));
	save_item(NAME(m_fifo_force_buffer_empty_pc));
}

DEFINE_DEVICE_TYPE(MIDWAY_IOASIC, midway_ioasic_device, "midway_ioasic", "Midway IOASIC")


//-------------------------------------------------
//  midway_serial_pic2_device - constructor
//-------------------------------------------------

midway_ioasic_device::midway_ioasic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	midway_serial_pic2_device(mconfig, MIDWAY_IOASIC, tag, owner, clock),
	m_io_dips(*this, ":DIPS"),
	m_io_system(*this, ":SYSTEM"),
	m_io_in1(*this, ":IN1"),
	m_io_in2(*this, ":IN2"),
	m_serial_tx_cb(*this),
	m_aux_output_cb(*this),
	m_has_dcs(0),
	m_has_cage(0),
	m_dcs_cpu(nullptr),
	m_shuffle_type(0),
	m_shuffle_default(0),
	m_shuffle_active(0),
	m_shuffle_map(nullptr),
	m_irq_callback(*this),
	m_irq_state(0),
	m_sound_irq_state(0),
	m_auto_ack(0),
	m_force_fifo_full(0),
	m_fifo_in(0),
	m_fifo_out(0),
	m_fifo_bytes(0),
	m_fifo_force_buffer_empty_pc(0),
	m_cage(*this, ":cage"),
	m_dcs(*this, ":dcs")
{
	memset(m_fifo,0,sizeof(m_fifo));
	memset(m_reg,0,sizeof(m_reg));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_ioasic_device::device_start()
//void midway_ioasic_init(running_machine &machine, int shuffle, int upper, int yearoffs, void (*irq_callback)(running_machine &, int))
{
	static const uint8_t shuffle_maps[][16] =
	{
		{ 0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf },    /* WarGods, WG3DH, SFRush, MK4 */
		{ 0x4,0x5,0x6,0x7,0xb,0xa,0x9,0x8,0x3,0x2,0x1,0x0,0xf,0xe,0xd,0xc },    /* Blitz, Blitz99 */
		{ 0x7,0x3,0x2,0x0,0x1,0xc,0xd,0xe,0xf,0x4,0x5,0x6,0x8,0x9,0xa,0xb },    /* Carnevil */
		{ 0x8,0x9,0xa,0xb,0x0,0x1,0x2,0x3,0xf,0xe,0xc,0xd,0x4,0x5,0x6,0x7 },    /* Calspeed, Gauntlet Legends */
		{ 0xf,0xe,0xd,0xc,0x4,0x5,0x6,0x7,0x9,0x8,0xa,0xb,0x2,0x3,0x1,0x0 },    /* Mace */
		{ 0xc,0xd,0xe,0xf,0x0,0x1,0x2,0x3,0x7,0x8,0x9,0xb,0xa,0x5,0x6,0x4 },    /* Gauntlet Dark Legacy */
		{ 0x7,0x4,0x5,0x6,0x2,0x0,0x1,0x3,0x8,0x9,0xa,0xb,0xd,0xc,0xe,0xf },    /* Vapor TRX */
		{ 0x7,0x4,0x5,0x6,0x2,0x0,0x1,0x3,0x8,0x9,0xa,0xb,0xd,0xc,0xe,0xf },    /* San Francisco Rush: The Rock */
		{ 0x1,0x2,0x3,0x0,0x4,0x5,0x6,0x7,0xa,0xb,0x8,0x9,0xc,0xd,0xe,0xf },    /* Hyperdrive */
	};

	ioasic_register_state();

	/* do we have a DCS2 sound chip connected? */
	m_has_dcs = (m_dcs != nullptr);
	m_has_cage = (m_cage != nullptr);

	if (m_has_dcs)
	{
		m_dcs_cpu = m_dcs->get_cpu();
	}

	m_shuffle_map = &shuffle_maps[m_shuffle_type][0];
	// resolve callbacks
	m_irq_callback.resolve_safe();
	m_serial_tx_cb.resolve_safe();
	m_aux_output_cb.resolve();

	/* initialize the PIC */
	midway_serial_pic2_device::device_start();

	/* reset the chip */
	ioasic_reset();

	m_reg[IOASIC_SOUNDCTL] = 0x0001;


	/* configure the fifo */
	if (m_has_dcs)
	{
		m_dcs->set_fifo_callbacks(
				read16smo_delegate(*this, FUNC(midway_ioasic_device::fifo_r)),
				read16mo_delegate(*this, FUNC(midway_ioasic_device::fifo_status_r)),
				write_line_delegate(*this, FUNC(midway_ioasic_device::fifo_reset_w)));
		m_dcs->set_io_callbacks(
				write_line_delegate(*this, FUNC(midway_ioasic_device::ioasic_output_full)),
				write_line_delegate(*this, FUNC(midway_ioasic_device::ioasic_input_empty)));
	}

	fifo_reset_w(1);
}

void midway_ioasic_device::set_shuffle_state(int state)
{
	m_shuffle_active = state;
}


void midway_ioasic_device::ioasic_reset()
{
	m_shuffle_active = m_shuffle_default;
	m_sound_irq_state = 0x0080;
	m_reg[IOASIC_INTCTL] = 0;
	if (m_has_dcs)
		fifo_reset_w(1);
	update_ioasic_irq();
	midway_serial_pic_device::reset_w(1);
}


void midway_ioasic_device::update_ioasic_irq()
{
	uint16_t fifo_state = get_fifo_status();
	uint16_t irqbits = 0x2000;
	uint8_t new_state;

	irqbits |= m_sound_irq_state & 0xff;
	irqbits |= m_reg[IOASIC_UARTIN] & 0x3f00;
	if (fifo_state & 8)
		irqbits |= 0x0008;
	if (irqbits)
		irqbits |= 0x0001;

	m_reg[IOASIC_INTSTAT] = irqbits;

	new_state = ((m_reg[IOASIC_INTCTL] & 0x0001) != 0) && ((m_reg[IOASIC_INTSTAT] & m_reg[IOASIC_INTCTL] & 0x3ffe) != 0);
	if (new_state != m_irq_state)
	{
		m_irq_state = new_state;
		if (!m_irq_callback.isnull())
			m_irq_callback(m_irq_state ? ASSERT_LINE : CLEAR_LINE);
		if (m_irq_state && (m_reg[IOASIC_UARTIN] & 0x1000))
			logerror("IOASIC: Asserting IRQ INTCTRL=%04x INTSTAT=%04X\n", m_reg[IOASIC_INTCTL], m_reg[IOASIC_INTSTAT]);
	}
}


void midway_ioasic_device::cage_irq_handler(uint8_t data)
{
	logerror("CAGE irq handler: %d\n", data);
	m_sound_irq_state = 0;
	if (data & atari_cage_device::CAGE_IRQ_REASON_DATA_READY)
		m_sound_irq_state |= 0x0040;
	if (data & atari_cage_device::CAGE_IRQ_REASON_BUFFER_EMPTY)
		m_sound_irq_state |= 0x0080;
	update_ioasic_irq();
}


WRITE_LINE_MEMBER(midway_ioasic_device::ioasic_input_empty)
{
//  logerror("ioasic_input_empty(%d)\n", state);
	if (state)
		m_sound_irq_state |= 0x0080;
	else
		m_sound_irq_state &= ~0x0080;
	update_ioasic_irq();
}


WRITE_LINE_MEMBER(midway_ioasic_device::ioasic_output_full)
{
//  logerror("ioasic_output_full(%d)\n", state);
	if (state)
		m_sound_irq_state |= 0x0040;
	else
		m_sound_irq_state &= ~0x0040;
	update_ioasic_irq();
}



/*************************************
 *
 *  ASIC sound FIFO; used by CarnEvil
 *
 *************************************/

uint16_t midway_ioasic_device::fifo_r()
{
	uint16_t result = 0;

	/* we can only read data if there's some to read! */
	if (m_fifo_bytes != 0)
	{
		/* fetch the data from the buffer and update the IOASIC state */
		result = m_fifo[m_fifo_out++ % FIFO_SIZE];
		m_fifo_bytes--;
		update_ioasic_irq();

		if (LOG_FIFO && (m_fifo_bytes < 4 || m_fifo_bytes >= FIFO_SIZE - 4))
			logerror("fifo_r(%04X): FIFO bytes = %d!\n", result, m_fifo_bytes);

		/* if we just cleared the buffer, this may generate an IRQ on the master CPU */
		/* because of the way the streaming code works, we need to make sure that the */
		/* next status read indicates an empty buffer, even if we've timesliced and the */
		/* main CPU is handling the I/O ASIC interrupt */
		if (m_fifo_bytes == 0 && m_has_dcs)
		{
			m_fifo_force_buffer_empty_pc = m_dcs_cpu->pc();
			if (LOG_FIFO)
				logerror("fifo_r(%04X): FIFO empty, PC = %04X\n", result, m_fifo_force_buffer_empty_pc);
		}
	}
	else
	{
		if (LOG_FIFO)
			logerror("fifo_r(): nothing to read!\n");
	}
	return result;
}


uint16_t midway_ioasic_device::get_fifo_status()
{
	uint16_t result = 0;

	if (m_fifo_bytes == 0 && !m_force_fifo_full)
		result |= 0x08;
	if (m_fifo_bytes >= FIFO_SIZE/2)
		result |= 0x10;
	if (m_fifo_bytes >= FIFO_SIZE || m_force_fifo_full)
		result |= 0x20;

	return result;
}


uint16_t midway_ioasic_device::fifo_status_r(address_space &space)
{
	uint16_t result = get_fifo_status();

	// kludge alert: if we're reading this from the DCS CPU itself, and we recently cleared
	// the FIFO, and we're within 16 instructions of the read that cleared the FIFO, make
	// sure the FIFO clear bit is set
	if (m_fifo_force_buffer_empty_pc && &space.device() == m_dcs_cpu)
	{
		offs_t currpc = m_dcs_cpu->pc();
		if (currpc >= m_fifo_force_buffer_empty_pc && currpc < m_fifo_force_buffer_empty_pc + 0x10)
		{
			m_fifo_force_buffer_empty_pc = 0;
			result |= 0x08;
			if (LOG_FIFO)
				logerror("ioasic_fifo_status_r(%04X): force empty, PC = %04X\n", result, currpc);
		}
	}

	return result;
}


WRITE_LINE_MEMBER(midway_ioasic_device::fifo_reset_w)
{
	/* on the high state, reset the FIFO data */
	if (state)
	{
		m_fifo_in = 0;
		m_fifo_out = 0;
		m_fifo_bytes = 0;
		m_force_fifo_full = 0;
		update_ioasic_irq();
	}
	if (LOG_FIFO)
		logerror("%s:fifo_reset(%d)\n", machine().describe_context(), state);
}


void midway_ioasic_device::fifo_w(uint16_t data)
{
	/* if we have room, add it to the FIFO buffer */
	if (m_fifo_bytes < FIFO_SIZE)
	{
		m_fifo[m_fifo_in++ % FIFO_SIZE] = data;
		m_fifo_bytes++;
		update_ioasic_irq();
		if (LOG_FIFO && (m_fifo_bytes < 4 || m_fifo_bytes >= FIFO_SIZE - 4))
			logerror("fifo_w(%04X): FIFO bytes = %d!\n", data, m_fifo_bytes);
	}
	else
	{
		if (LOG_FIFO)
			logerror("fifo_w(%04X): out of space!\n", data);
	}
	m_dcs->fifo_notify(m_fifo_bytes, FIFO_SIZE);
}


void midway_ioasic_device::fifo_full_w(uint16_t data)
{
	if (LOG_FIFO)
		logerror("fifo_full_w(%04X)\n", data);
	m_force_fifo_full = 1;
	update_ioasic_irq();
	m_dcs->fifo_notify(m_fifo_bytes, FIFO_SIZE);
}



/*************************************
 *
 *  I/O ASIC master read/write
 *
 *************************************/

uint32_t midway_ioasic_device::packed_r(address_space &space, offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_15)
		result |= read(space, offset*2) & 0xffff;
	if (ACCESSING_BITS_16_31)
		result |= (read(space, offset*2+1) & 0xffff) << 16;
	return result;
}


uint32_t midway_ioasic_device::read(address_space &space, offs_t offset)
{
	uint32_t result;

	offset = m_shuffle_active ? m_shuffle_map[offset & 15] : offset;
	result = m_reg[offset];

	switch (offset)
	{
		case IOASIC_PORT0:
			// bit 0 is PIC ready flag before shuffling happens
			// bits 15:13 == 001
			if (!m_shuffle_active)
			{
				/* blitz99 wants bit bits 13-15 to be 1 */
				result = 0x2001;
			}
			else {
				result = m_io_dips->read();
			}
			break;

		case IOASIC_PORT1:
			result = m_io_system->read();
			break;

		case IOASIC_PORT2:
			result = m_io_in1->read();
			break;

		case IOASIC_PORT3:
			result = m_io_in2->read();
			break;

		case IOASIC_UARTIN:
			m_reg[offset] &= ~0x1000;
			if (result & 0x1000)
				logerror("%s: ioasic_r(%d) = %08X\n", machine().describe_context(), offset, result);
			// Add lf
			if ((result & 0xff)==0x0d)
				m_reg[offset] = 0x300a;
			update_ioasic_irq();
			break;

		case IOASIC_SOUNDSTAT:
			/* status from sound CPU */
			result = 0;
			if (m_has_dcs)
			{
				result |= ((m_dcs->control_r() >> 4) ^ 0x40) & 0x00c0;
				result |= fifo_status_r(space) & 0x0038;
				result |= m_dcs->data2_r() & 0xff00;
			}
			else if (m_has_cage)
			{
				result |= (m_cage->control_r() << 6) ^ 0x80;
			}
			else
				result |= 0x48;
			break;

		case IOASIC_SOUNDIN:
			result = 0;
			if (m_has_dcs)
			{
				result = m_dcs->data_r();
				if (m_auto_ack)
					m_dcs->ack_w();
			}
			else if (m_has_cage)
				result = m_cage->main_r();
			else
			{
				static uint16_t val = 0;
				result = val = ~val;
			}
			break;

		case IOASIC_PICIN:
			result = midway_serial_pic2_device::read() | (midway_serial_pic2_device::status_r() << 8);
			break;

		default:
			break;
	}

	if (LOG_IOASIC && offset != IOASIC_SOUNDSTAT && offset != IOASIC_SOUNDIN)
		logerror("%s:ioasic_r(%d) = %08X\n", machine().describe_context(), offset, result);

	return result;
}


void midway_ioasic_device::packed_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
		write(offset*2, data & 0xffff, 0x0000ffff);
	if (ACCESSING_BITS_16_31)
		write(offset*2+1, data >> 16, 0x0000ffff);
}

void midway_ioasic_device::serial_rx_w(u8 data)
{
	// Break Detect        0x0100
	// Frame Error         0x0200
	// Overrun             0x0400
	// Rx FIFO FULL        0x0800
	// Rx Ready            0x1000
	// Tx EMPTY            0x2000
	// CTS IN              0x4000
	// CTS OUT             0x8000
	if (m_reg[IOASIC_UARTCONTROL] & 0x200) {
		m_reg[IOASIC_UARTIN] = data | 0x3000;
		update_ioasic_irq();
	}

}

void midway_ioasic_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t oldreg, newreg;

	offset = m_shuffle_active ? m_shuffle_map[offset & 15] : offset;
	oldreg = m_reg[offset];
	// Block register updates until ioasic is unlocked
	// mwskins and thegrid use this as test to see if the ioasic is unlocked
	if (m_shuffle_active)
		COMBINE_DATA(&m_reg[offset]);
	newreg = m_reg[offset];

	if (LOG_IOASIC && offset != IOASIC_SOUNDOUT)
		logerror("%s ioasic_w(%d) = %08X\n", machine().describe_context(), offset, data);

	switch (offset)
	{
		case IOASIC_PORT0:
			/* the last write here seems to turn on shuffling */
			if (data == 0xe2)
			{
				m_shuffle_active = 1;
				logerror("*** I/O ASIC unlocked!\n");
				m_reg[IOASIC_INTCTL] = 0;
				m_reg[IOASIC_UARTCONTROL] = 0; /* bug in 10th Degree assumes this */
			}
			break;

		case IOASIC_PORT2:
		case IOASIC_PORT3:
			/* ignore writes here if we're not shuffling yet */
			if (!m_shuffle_active)
				break;
			break;

		case IOASIC_UARTCONTROL:
			logerror("%s: IOASIC uart control = %04X INTCTRL=%04x\n", machine().describe_context(), data, m_reg[IOASIC_INTCTL]);
			break;

		case IOASIC_UARTOUT:
			if (m_reg[IOASIC_UARTCONTROL] & 0x800)
			{
				/* we're in loopback mode -- copy to the input */
				m_reg[IOASIC_UARTIN] = (newreg & 0x00ff) | 0x3000;
				update_ioasic_irq();
			}
			else {
				m_serial_tx_cb(data);
				m_reg[IOASIC_UARTIN] |= 0x2000;
				update_ioasic_irq();
				if (PRINTF_DEBUG) {
					osd_printf_info("%c", data & 0xff);
					logerror("%c", data & 0xff);
				}
			}
			//logerror("IOASIC uart tx data = %04X\n", data);
			break;

		case IOASIC_SOUNDCTL:
			if (LOG_IOASIC)
				logerror("%s: write IOASIC_SOUNDCTL=%04x\n", machine().describe_context(), data);
			/* sound reset? */
			if (m_has_dcs)
			{
				m_dcs->reset_w(newreg & 1);
			}
			else if (m_has_cage)
			{
				if ((oldreg ^ newreg) & 1)
				{
					m_cage->control_w(0);
					if (!(~newreg & 1))
						m_cage->control_w(3);
				}
			}

			/* FIFO reset? */
			fifo_reset_w(~newreg & 4);
			break;

		case IOASIC_SOUNDOUT:
			if (m_has_dcs)
				m_dcs->data_w(newreg);
			else if (m_has_cage)
				m_cage->main_w(newreg);
			break;

		case IOASIC_SOUNDIN:
			m_dcs->ack_w();
			/* acknowledge data read */
			break;

		case IOASIC_PICOUT:
			if (m_shuffle_type == MIDWAY_IOASIC_VAPORTRX)
				midway_serial_pic2_device::write(newreg ^ 0x0a);
			else if (m_shuffle_type == MIDWAY_IOASIC_SFRUSHRK)
				midway_serial_pic2_device::write(newreg ^ 0x05);
			else
				midway_serial_pic2_device::write(newreg);
			break;

		case IOASIC_PICIN:
			/* This is P15 on vegas boards */
			if (!m_aux_output_cb.isnull())
				m_aux_output_cb(data);
			break;

		case IOASIC_INTCTL:
			/* interrupt enables */
			/* bit  0 = global interrupt enable */
			/* bit  3 = FIFO empty */
			/* bit  6 = sound input buffer full */
			/* bit  7 = sound output buffer empty */
			/* bit 14 = LED */
			/* bit 15 = TI320Cx Mode Enable */
			if (LOG_IOASIC && ((oldreg ^ newreg) & 0x3ff6))
				logerror("IOASIC interrupt control = %04X\n", data);
			update_ioasic_irq();
			break;

		default:
			break;
	}
}
