// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Emulation of various Midway ICs

***************************************************************************/

#include "emu.h"
#include "debugger.h"
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
 *  Serial number encoding
 *
 *************************************/

void midway_serial_pic_device::generate_serial_data(int upper)
{
	int year = atoi(machine().system().year), month = 12, day = 11;
	UINT32 serial_number, temp;
	UINT8 serial_digit[9];

	serial_number = 123456;
	serial_number += upper * 1000000;

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

	/* special hack for RevX */
	m_ormask = 0x80;
	if (upper == 419)
		m_ormask = 0x00;
}



/*************************************
 *
 *  Original serial number PIC
 *  interface
 *
 *************************************/

void midway_serial_pic_device::serial_register_state()
{
	save_item(NAME(m_data));
	save_item(NAME(m_buff));
	save_item(NAME(m_idx));
	save_item(NAME(m_status));
	save_item(NAME(m_bits));
	save_item(NAME(m_ormask));
}

const device_type MIDWAY_SERIAL_PIC = &device_creator<midway_serial_pic_device>;


//-------------------------------------------------
//  midway_serial_pic2_device - constructor
//-------------------------------------------------

midway_serial_pic_device::midway_serial_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MIDWAY_SERIAL_PIC2, "Midway Serial Pic", tag, owner, clock, "midway_serial_pic", __FILE__),
	m_upper(0),
	m_buff(0),
	m_idx(0),
	m_status(0),
	m_bits(0),
	m_ormask(0)
{
	memset(m_data,0,sizeof(m_data));
}

midway_serial_pic_device::midway_serial_pic_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_upper(0),
	m_buff(0),
	m_idx(0),
	m_status(0),
	m_bits(0),
	m_ormask(0)
{
	memset(m_data,0,sizeof(m_data));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_serial_pic_device::device_start()
{
	serial_register_state();
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


READ8_MEMBER(midway_serial_pic_device::status_r)
{
	return m_status;
}


READ8_MEMBER(midway_serial_pic_device::read)
{
	logerror("%s:security R = %04X\n", machine().describe_context(), m_buff);
	m_status = 1;
	return m_buff;
}


WRITE8_MEMBER(midway_serial_pic_device::write)
{
	logerror("%s:security W = %04X\n", machine().describe_context(), data);

	/* status seems to reflect the clock bit */
	m_status = (data >> 4) & 1;

	/* on the falling edge, clock the next data byte through */
	if (!m_status)
	{
		/* the self-test writes 1F, 0F, and expects to read an F in the low 4 bits */
		/* Cruis'n World expects the high bit to be set as well */
		if (data & 0x0f)
			m_buff = m_ormask | data;
		else
			m_buff = m_data[m_idx++ % sizeof(m_data)];
	}
}



/*************************************
 *
 *  Second generation serial number
 *  PIC interface; this version also
 *  contained some NVRAM and a real
 *  time clock
 *
 *************************************/

INLINE UINT8 make_bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}

const device_type MIDWAY_SERIAL_PIC2 = &device_creator<midway_serial_pic2_device>;


//-------------------------------------------------
//  midway_serial_pic2_device - constructor
//-------------------------------------------------

midway_serial_pic2_device::midway_serial_pic2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	midway_serial_pic_device(mconfig, MIDWAY_SERIAL_PIC2, "Midway Serial Pic 2", tag, owner, clock, "midway_serial_pic2", __FILE__),
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

midway_serial_pic2_device::midway_serial_pic2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	midway_serial_pic_device(mconfig, type, name, tag, owner, clock, shortname, source),
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



void midway_serial_pic2_device::set_default_nvram(const UINT8 *nvram)
{
	memcpy(m_default_nvram, nvram, sizeof(m_default_nvram));
}


READ8_MEMBER(midway_serial_pic2_device::status_r)
{
	UINT8 result = 0;

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
	return result;
}


READ8_MEMBER(midway_serial_pic2_device::read)
{
	UINT8 result = 0;

	/* PIC data register */
	logerror("%s:PIC data read (index=%d total=%d latch=%03X) =", machine().describe_context(), m_index, m_total, m_latch);

	/* return the current result */
	if (m_latch & 0xf00)
		result = m_latch & 0xff;

	/* otherwise, return 0xff if we have data ready */
	else if (m_index < m_total)
		result = 0xff;

	logerror("%02X\n", result);
	return result;
}


WRITE8_MEMBER(midway_serial_pic2_device::write)
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
					debugger_break(machine());
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

void midway_serial_pic2_device::nvram_read(emu_file &file)
{
	file.read(m_nvram, sizeof(m_nvram));
}

void midway_serial_pic2_device::nvram_write(emu_file &file)
{
	file.write(m_nvram, sizeof(m_nvram));
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
	IOASIC_UNKNOWN7,    /* 7: ??? */
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

const device_type MIDWAY_IOASIC = &device_creator<midway_ioasic_device>;


//-------------------------------------------------
//  midway_serial_pic2_device - constructor
//-------------------------------------------------

midway_ioasic_device::midway_ioasic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	midway_serial_pic2_device(mconfig, MIDWAY_IOASIC, "Midway IOASIC", tag, owner, clock, "midway_ioasic", __FILE__),
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
	m_cage(nullptr),
	m_dcs(nullptr)
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
	static const UINT8 shuffle_maps[][16] =
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

	/* do we have a DCS2 sound chip connected? (most likely) */
	m_dcs = machine().device<dcs_audio_device>("dcs");
	m_has_dcs = (m_dcs != nullptr);
	m_cage = machine().device<atari_cage_device>("cage");
	m_has_cage = (m_cage != nullptr);

	m_dcs_cpu = m_dcs->subdevice("dcs2");
	if (m_dcs_cpu == nullptr)
		m_dcs_cpu = m_dcs->subdevice("dsio");
	if (m_dcs_cpu == nullptr)
		m_dcs_cpu = m_dcs->subdevice("denver");
	m_shuffle_map = &shuffle_maps[m_shuffle_type][0];
	// resolve callbacks
	m_irq_callback.resolve_safe();

	/* initialize the PIC */
	midway_serial_pic2_device::device_start();

	/* reset the chip */
	ioasic_reset();

	m_reg[IOASIC_SOUNDCTL] = 0x0001;

	/* configure the fifo */
	if (m_has_dcs)
	{
		m_dcs->set_fifo_callbacks(read16_delegate(FUNC(midway_ioasic_device::fifo_r),this),
			read16_delegate(FUNC(midway_ioasic_device::fifo_status_r),this),
			write_line_delegate(FUNC(midway_ioasic_device::fifo_reset_w),this));
		m_dcs->set_io_callbacks(write_line_delegate(FUNC(midway_ioasic_device::ioasic_output_full),this),
			write_line_delegate(FUNC(midway_ioasic_device::ioasic_input_empty),this));
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
	UINT16 fifo_state = fifo_status_r(machine().driver_data()->generic_space(),0);
	UINT16 irqbits = 0x2000;
	UINT8 new_state;

	irqbits |= m_sound_irq_state;
	if (m_reg[IOASIC_UARTIN] & 0x1000)
		irqbits |= 0x1000;
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
	}
}


WRITE8_MEMBER(midway_ioasic_device::cage_irq_handler)
{
	logerror("CAGE irq handler: %d\n", data);
	m_sound_irq_state = 0;
	if (data & CAGE_IRQ_REASON_DATA_READY)
		m_sound_irq_state |= 0x0040;
	if (data & CAGE_IRQ_REASON_BUFFER_EMPTY)
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

READ16_MEMBER(midway_ioasic_device::fifo_r)
{
	UINT16 result = 0;

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
			m_fifo_force_buffer_empty_pc = m_dcs_cpu->safe_pc();
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


READ16_MEMBER(midway_ioasic_device::fifo_status_r)
{
	UINT16 result = 0;

	if (m_fifo_bytes == 0 && !m_force_fifo_full)
		result |= 0x08;
	if (m_fifo_bytes >= FIFO_SIZE/2)
		result |= 0x10;
	if (m_fifo_bytes >= FIFO_SIZE || m_force_fifo_full)
		result |= 0x20;

	/* kludge alert: if we're reading this from the DCS CPU itself, and we recently cleared */
	/* the FIFO, and we're within 16 instructions of the read that cleared the FIFO, make */
	/* sure the FIFO clear bit is set */
	if (m_fifo_force_buffer_empty_pc && &space.device() == m_dcs_cpu)
	{
		offs_t currpc = m_dcs_cpu->safe_pc();
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


void midway_ioasic_device::fifo_w(UINT16 data)
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


void midway_ioasic_device::fifo_full_w(UINT16 data)
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

READ32_MEMBER( midway_ioasic_device::packed_r )
{
	UINT32 result = 0;
	if (ACCESSING_BITS_0_15)
		result |= read(space, offset*2, 0x0000ffff) & 0xffff;
	if (ACCESSING_BITS_16_31)
		result |= (read(space, offset*2+1, 0x0000ffff) & 0xffff) << 16;
	return result;
}


READ32_MEMBER( midway_ioasic_device::read )
{
	UINT32 result;

	offset = m_shuffle_active ? m_shuffle_map[offset & 15] : offset;
	result = m_reg[offset];

	switch (offset)
	{
		case IOASIC_PORT0:
			result = machine().root_device().ioport("DIPS")->read();
			/* bit 0 seems to be a ready flag before shuffling happens */
			if (!m_shuffle_active)
			{
				result |= 0x0001;
				/* blitz99 wants bit bits 13-15 to be 1 */
				result &= ~0xe000;
				result |= 0x2000;
			}
			break;

		case IOASIC_PORT1:
			result = machine().root_device().ioport("SYSTEM")->read();
			break;

		case IOASIC_PORT2:
			result = machine().root_device().ioport("IN1")->read();
			break;

		case IOASIC_PORT3:
			result = machine().root_device().ioport("IN2")->read();
			break;

		case IOASIC_UARTIN:
			m_reg[offset] &= ~0x1000;
			break;

		case IOASIC_SOUNDSTAT:
			/* status from sound CPU */
			result = 0;
			if (m_has_dcs)
			{
				result |= ((m_dcs->control_r() >> 4) ^ 0x40) & 0x00c0;
				result |= fifo_status_r(space,0) & 0x0038;
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
				static UINT16 val = 0;
				result = val = ~val;
			}
			break;

		case IOASIC_PICIN:
			result = midway_serial_pic2_device::read(space,0) | (midway_serial_pic2_device::status_r(space,0) << 8);
			break;

		default:
			break;
	}

	if (LOG_IOASIC && offset != IOASIC_SOUNDSTAT && offset != IOASIC_SOUNDIN)
		logerror("%06X:ioasic_r(%d) = %08X\n", space.device().safe_pc(), offset, result);

	return result;
}


WRITE32_MEMBER( midway_ioasic_device::packed_w )
{
	if (ACCESSING_BITS_0_15)
		write(space, offset*2, data & 0xffff, 0x0000ffff);
	if (ACCESSING_BITS_16_31)
		write(space, offset*2+1, data >> 16, 0x0000ffff);
}


WRITE32_MEMBER( midway_ioasic_device::write )
{
	UINT32 oldreg, newreg;

	offset = m_shuffle_active ? m_shuffle_map[offset & 15] : offset;
	oldreg = m_reg[offset];
	COMBINE_DATA(&m_reg[offset]);
	newreg = m_reg[offset];

	if (LOG_IOASIC && offset != IOASIC_SOUNDOUT)
		logerror("%06X:ioasic_w(%d) = %08X\n", space.device().safe_pc(), offset, data);

	switch (offset)
	{
		case IOASIC_PORT0:
			/* the last write here seems to turn on shuffling */
			if (data == 0xe2)
			{
				m_shuffle_active = 1;
				logerror("*** I/O ASIC shuffling enabled!\n");
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

		case IOASIC_UARTOUT:
			if (m_reg[IOASIC_UARTCONTROL] & 0x800)
			{
				/* we're in loopback mode -- copy to the input */
				m_reg[IOASIC_UARTIN] = (newreg & 0x00ff) | 0x1000;
				update_ioasic_irq();
			}
			else if (PRINTF_DEBUG)
				osd_printf_debug("%c", data & 0xff);
			break;

		case IOASIC_SOUNDCTL:
			/* sound reset? */
			if (m_has_dcs)
			{
				m_dcs->reset_w(~newreg & 1);
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
				midway_serial_pic2_device::write(space, 0, newreg ^ 0x0a);
			else if (m_shuffle_type == MIDWAY_IOASIC_SFRUSHRK)
				midway_serial_pic2_device::write(space, 0, newreg ^ 0x05);
			else
				midway_serial_pic2_device::write(space, 0, newreg);
			break;

		case IOASIC_INTCTL:
			/* interrupt enables */
			/* bit  0 = global interrupt enable */
			/* bit  3 = FIFO empty */
			/* bit  6 = sound input buffer full */
			/* bit  7 = sound output buffer empty */
			/* bit 14 = LED? */
			if ((oldreg ^ newreg) & 0x3ff6)
				logerror("IOASIC int control = %04X\n", data);
			update_ioasic_irq();
			break;

		default:
			break;
	}
}
