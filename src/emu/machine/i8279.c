// license:BSD-3-Clause
// copyright-holders:Robbbert
/**********************************************************************

    i8279

2012-JAN-08 First draft [Robbbert]
2012-JAN-12 Implemented

Notes:
- All keys MUST be ACTIVE_LOW


ToDo:
- Command 5 (Nibble masking and blanking)
- Command 7 (Error Mode)
- Interrupts
- BD pin
- Sensor ram stuff
- save state


What has been done:
CMD 0:
- Display Mode
-- Left & Right with no increment are the same thing
-- Right with increment is not emulated yet ***
- Keyboard Mode
-- No particular code has been added for 2-key/N-key rollover, no need
-- Sensor mode is not complete yet ***
-- Encoded and Decoded are done
-- Strobe is done
-- Sensor and FIFO may share the same internal RAM, not sure
CMD 1:
- Clock Divider
-- Value is stored, but internally a fixed value is always used
CMD 2:
- Read FIFO/Sensor RAM
-- FIFO works
-- Sensor RAM works
CMD 3:
- Read Display RAM
-- This works
CMD 4:
- Write Display RAM
-- Right with increment does nothing, the rest is working ***
CMD 5:
- Blank Nibble
-- Not done ***
- Mask Nibble
-- Implemented
CMD 6:
-- All implemented
CMD 7:
- Interrupt
-- Not done
- Error Mode
-- No need to do.

Interface:
-- All done except BD pin ***

Status word:
- FIFO bits
-- All done
- Error bit
-- Not done (no need)
- Display unavailable
-- Not done (no need)


Items marked (***) can be added if a system appears
that uses this feature.

**********************************************************************/

#include "i8279.h"

#define LOG 0

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type I8279 = &device_creator<i8279_device>;

//-------------------------------------------------
//  i8279_device - constructor
//-------------------------------------------------

i8279_device::i8279_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8279, "8279 KDC", tag, owner, clock, "i8279", __FILE__),
	m_out_irq_cb(*this),
	m_out_sl_cb(*this),
	m_out_disp_cb(*this),
	m_out_bd_cb(*this),
	m_in_rl_cb(*this),
	m_in_shift_cb(*this),
	m_in_ctrl_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8279_device::device_start()
{
	/* resolve callbacks */
	m_out_irq_cb.resolve();
	m_out_sl_cb.resolve();
	m_out_disp_cb.resolve();
	m_out_bd_cb.resolve();
	m_in_rl_cb.resolve();
	m_in_shift_cb.resolve();
	m_in_ctrl_cb.resolve();
	m_clock = clock();
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(i8279_device::timerproc_callback), this));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8279_device::device_reset()
{
	UINT8 i;

	// startup values are unknown: setting to 0
	for (i = 2; i < 8; i++) m_cmd[i] = 0;
	for (i = 0; i < 8; i++) m_fifo[i] = 0;
	for (i = 0; i < 8; i++) m_s_ram[i] = 0;
	for (i = 0; i < 16; i++) m_d_ram[i] = 0;
	m_status = 0;
	m_autoinc = 1;
	m_d_ram_ptr = 0;
	m_s_ram_ptr = 0;
	m_read_flag = 0;
	m_scanner = 0;
	m_ctrl_key = 1;
	m_key_down = 0xffff;

	// from here is confirmed
	m_cmd[0] = 8;
	m_cmd[1] = 31;
	logerror("Initial clock = 3100kHz\n");
	timer_adjust();
}


void i8279_device::timer_adjust()
{
// Real device runs at about 100kHz internally, clock divider is chosen so that
// this is the case. We do not need such speed, 2000Hz is enough.
// If this is too long, the sensor mode doesn't work correctly.

#if 0
	UINT8 divider = (m_cmd[1]) ? m_cmd[1] : 1;
	UINT32 new_clock = clock() / divider;
#else
	UINT32 new_clock = 2000;
#endif

	if (m_clock != new_clock)
	{
		m_timer->adjust(attotime::from_hz(new_clock), 0, attotime::from_hz(new_clock));

		m_clock = new_clock;
	}
}


void i8279_device::clear_display()
{
	// clear all digits
	UINT8 i,patterns[4] = { 0, 0, 0x20, 0xff };
	UINT8 data = patterns[(m_cmd[6] & 12) >> 2];

	// The CD high bit (also done by CA)
	if (m_cmd[6] & 0x11)
		for (i = 0; i < 16; i++)
			m_d_ram[i] = data;

	m_status &= 0x7f; // bit 7 not emulated, but do it anyway
	m_d_ram_ptr = 0; // not in the datasheet, but needed

	// The CF bit (also done by CA)
	if (m_cmd[6] & 3)
	{
		m_status &= 0xc0; // blow away fifo
		m_s_ram_ptr = 0; // reset sensor pointer
		set_irq(0); // reset irq
	}
}


void i8279_device::set_irq(bool state)
{
	if ( !m_out_irq_cb.isnull() )
		m_out_irq_cb( state );
}


void i8279_device::new_key(UINT8 data, bool skey, bool ckey)
{
	UINT8 i, rl, sl;
	for (i = 0; BIT(data, i); i++);
	rl = i;
	if (BIT(m_cmd[0], 0))
	{
		for (i = 0; !BIT(data, i); i++);
		sl = i;
	}
	else
		sl = m_scanner;

	new_fifo( (ckey << 7) | (skey << 6) | (sl << 3) | rl);
}


void i8279_device::new_fifo(UINT8 data)
{
	// see if already overrun
	if (BIT(m_status, 5))
		return;

	// set overrun flag if full
	if (BIT(m_status, 3))
	{
		m_status |= 0x20;
		return;
	}

	m_fifo[m_status & 7] = data;

	// bump fifo size & turn off underrun
	UINT8 fifo_size = m_status & 7;
	if ((fifo_size)==7)
		m_status |= 8; // full
	else
		m_status = (m_status & 0xe8) + fifo_size + 1;

	if (!fifo_size)
		set_irq(1); // something just went into fifo, so int
}


TIMER_CALLBACK_MEMBER( i8279_device::timerproc_callback )
{
	timer_mainloop();
}


void i8279_device::timer_mainloop()
{
	// control byte 0
	// bit 0 - encoded or decoded keyboard scan
	// bits 1,2 - keyboard type
	// bit 3 - number of digits to display
	// bit 4 - left or right entry

	UINT8 scanner_mask = BIT(m_cmd[0], 0) ? 15 : BIT(m_cmd[0], 3) ? 15 : 7;
	bool decoded = BIT(m_cmd[0], 0);
	UINT8 kbd_type = (m_cmd[0] & 6) >> 1;
	bool shift_key = 1;
	bool ctrl_key = 1;
	bool strobe_pulse = 0;

	// keyboard
	// type 0 = kbd, 2-key lockout
	// type 1 = kdb, n-key
	// type 2 = sensor
	// type 3 = strobed

	// Get shift keys
	if ( !m_in_shift_cb.isnull() )
		shift_key = m_in_shift_cb();

	if ( !m_in_ctrl_cb.isnull() )
		ctrl_key = m_in_ctrl_cb();

	if (ctrl_key && !m_ctrl_key)
		strobe_pulse = 1; // low-to-high is a strobe

	m_ctrl_key = ctrl_key;

	// Read a row of keys

	if ( !m_in_rl_cb.isnull() )
	{
		UINT8 rl = m_in_rl_cb(0);

		// see if key still down from last time
		UINT16 key_down = (m_scanner << 8) | rl;
		if (key_down == m_key_down)
			rl = 0xff;
		else
		if ((rl == 0xff) && (m_scanner == m_key_down >> 8))
			m_key_down = 0xffff;

		// now process new key
		if (rl < 0xff || kbd_type == 2)
		{
			m_key_down = key_down;
			switch (kbd_type)
			{
				case 0:
				case 1:
					new_key(rl, shift_key, ctrl_key);
					break;
				case 2:
					{
						UINT8 addr = m_scanner &7;

						if (decoded)
							for (addr=0; !BIT(m_scanner, addr); addr++);

						rl ^= 0xff;     // inverted
						assert(addr < ARRAY_LENGTH(m_s_ram));
						if (m_s_ram[addr] != rl)
						{
							m_s_ram[addr] = rl;

							// IRQ line goes high if a row change value
							set_irq(1);
						}
					}
					break;
				case 3:
					if (strobe_pulse) new_fifo(rl);
					break;
			}
		}
	}

	// Increment scanline

	if (decoded)
	{
		m_scanner<<= 1;
		if ((m_scanner & 15)==0)
			m_scanner = 1;
	}
	else
		m_scanner++;

	m_scanner &= scanner_mask; // 4-bit port

	if ( !m_out_sl_cb.isnull() )
		m_out_sl_cb((offs_t)0, m_scanner);

	// output a digit

	if ( !m_out_disp_cb.isnull() )
		m_out_disp_cb((offs_t)0, m_d_ram[m_scanner] );
}


READ8_MEMBER( i8279_device::status_r )
{
	return m_status;
}


READ8_MEMBER( i8279_device::data_r )
{
	UINT8 i;
	bool sensor_mode = ((m_cmd[0] & 6)==4);
	UINT8 data;
	if (m_read_flag)
	{
	// read the display ram
		data = m_d_ram[m_d_ram_ptr];
		if (m_autoinc)
			m_d_ram_ptr++;
	}
	else
	if (sensor_mode)
	{
	// read sensor ram
		assert(m_s_ram_ptr < ARRAY_LENGTH(m_s_ram));
		data = m_s_ram[m_s_ram_ptr];
		if (m_autoinc)
			m_s_ram_ptr++;
		else
			set_irq(0);
	}
	else
	{
	// read a key from fifo
		data = m_fifo[0];
		UINT8 fifo_size = m_status & 7;
		switch (m_status & 0x38)
		{
			case 0x00: // no errors
				if (!fifo_size)
					m_status |= 0x10; // underrun
				else
				{
					for (i = 1; i < 8; i++)
						m_fifo[i-1] = m_fifo[i];
					fifo_size--;
					if (!fifo_size)
						set_irq(0);
				}
				break;
			case 0x28: // overrun
			case 0x08: // fifo full
				for (i = 1; i < 8; i++)
					m_fifo[i-1] = m_fifo[i];
				break;
			case 0x10: // underrun
				if (!fifo_size)
					break;
			default:
				printf("Invalid status: %X\n", m_status);
		}
		m_status = (m_status & 0xd0) | fifo_size; // turn off overrun & full
	}

	m_d_ram_ptr &= 15;
	m_s_ram_ptr &= 7;
	return data;
}


WRITE8_MEMBER( i8279_device::cmd_w )
{//printf("Command: %X=%X ",data>>5,data&31);
	UINT8 cmd = data >> 5;
	data &= 0x1f;
	m_cmd[cmd] = data;
	switch (cmd)
	{
		case 0:
			if (LOG) logerror("I8279 '%s' kb mode %x, display mode %x\n", tag(), data & 7, (data>>3) & 3);
			break;
		case 1:
			if (data > 1)
			{
				logerror("Clock set to %dkHz\n",data*100);
				timer_adjust();
			}
			break;
		case 2:
			m_read_flag = 0;
			if ((m_cmd[0] & 6)==4) // sensor mode only
			{
				m_autoinc = BIT(data, 4);
				m_s_ram_ptr = data & 7;
				if (LOG) logerror("I8279 '%s' selct sensor row %x, AI %d\n", tag(), m_s_ram_ptr, m_autoinc);
			}
			break;
		case 3:
			m_read_flag = 1;
			m_d_ram_ptr = data & 15;
			m_autoinc = BIT(data, 4);
			break;
		case 4:
			m_d_ram_ptr = data & 15;
			m_autoinc = BIT(data, 4);
			break;
		case 6:
			if (LOG) logerror("I8279 '%s' clear cmd %x\n", tag(), data);
			clear_display();
			break;
	}
}


WRITE8_MEMBER( i8279_device::data_w )
{//printf("Data: %X ",data);
	if (BIT(m_cmd[0], 4) & m_autoinc)
	{
	// right-entry autoincrement not implemented yet
	}
	else
	{
		if (!(m_cmd[5] & 0x04))
			m_d_ram[m_d_ram_ptr] = (m_d_ram[m_d_ram_ptr] & 0xf0) | (data & 0x0f);
		if (!(m_cmd[5] & 0x08))
			m_d_ram[m_d_ram_ptr] = (m_d_ram[m_d_ram_ptr] & 0x0f) | (data & 0xf0);

		if (m_autoinc)
			m_d_ram_ptr++;
	}
	m_d_ram_ptr &= 15;
}
