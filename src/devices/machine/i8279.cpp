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

#include "emu.h"
#include "i8279.h"

//#define VERBOSE 1
#include "logmacro.h"

// MAME updates inputs frame-by-frame, causing lockout to occur too often
#define EMULATE_KEY_LOCKOUT 0

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(I8279, i8279_device, "i8279", "Intel 8279 KDC")

//-------------------------------------------------
//  i8279_device - constructor
//-------------------------------------------------

i8279_device::i8279_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I8279, tag, owner, clock),
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
	m_scanclock = clock();
	m_timer = timer_alloc(FUNC(i8279_device::timerproc_callback), this);

	// save state
	save_item(NAME(m_d_ram));
	save_item(NAME(m_d_ram_ptr));
	save_item(NAME(m_s_ram));
	save_item(NAME(m_s_ram_ptr));
	save_item(NAME(m_fifo));
	save_item(NAME(m_cmd));
	save_item(NAME(m_status));
	save_item(NAME(m_scanclock));
	save_item(NAME(m_scanner));
	save_item(NAME(m_autoinc));
	save_item(NAME(m_read_flag));
	save_item(NAME(m_ctrl_key));
	save_item(NAME(m_se_mode));
	save_item(NAME(m_key_down));
	save_item(NAME(m_debounce));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8279_device::device_reset()
{
	// startup values are unknown: setting to 0
	for (u8 i = 2; i < 8; i++) m_cmd[i] = 0;
	for (u8 i = 0; i < 8; i++) m_fifo[i] = 0;
	for (u8 i = 0; i < 8; i++) m_s_ram[i] = 0;
	for (u8 i = 0; i < 16; i++) m_d_ram[i] = 0;
	m_status = 0;
	m_autoinc = true;
	m_d_ram_ptr = 0;
	m_s_ram_ptr = 0;
	m_read_flag = 0;
	m_scanner = 0;
	m_ctrl_key = 1;
	m_se_mode = 0;
	m_key_down = 0;
	m_debounce = 0;

	// from here is confirmed
	m_cmd[0] = 8;
	m_cmd[1] = 31;
	timer_adjust();
	logerror("Initial clock = %.2f kHz\n", m_scanclock / 1000.0);
}


void i8279_device::timer_adjust()
{
// Real device runs at about 100kHz internally, clock divider is chosen so that
// this is the case. If this is too long, the sensor mode doesn't work correctly.

	u8 divider = (m_cmd[1] >= 2) ? m_cmd[1] : 2;
	u32 new_clock = clock() / divider;

	if (m_scanclock != new_clock)
	{
		m_timer->adjust(attotime::from_ticks(64, new_clock), 0, attotime::from_ticks(64, new_clock));

		m_scanclock = new_clock;
	}
}


void i8279_device::clear_display()
{
	// clear all digits
	u8 i,patterns[4] = { 0, 0, 0x20, 0xff };
	u8 data = patterns[(m_cmd[6] & 12) >> 2];

	// The CD high bit (also done by CA)
	if (m_cmd[6] & 0x11)
		for (i = 0; i < 16; i++)
			m_d_ram[i] = data;

	m_status &= 0x7f; // bit 7 not emulated, but do it anyway
	m_d_ram_ptr = 0; // not in the datasheet, but needed

	// The CF bit (also done by CA)
	if (m_cmd[6] & 3)
	{
		m_status &= 0x80; // blow away fifo
		m_s_ram_ptr = 0; // reset sensor pointer
		m_debounce = 0; // reset debounce logic
		set_irq(0); // reset irq
	}
}


void i8279_device::set_irq(bool state)
{
	if ( !m_out_irq_cb.isnull() )
		m_out_irq_cb( state );
}


void i8279_device::new_fifo(u8 data)
{
	// see if already overrun
	if (BIT(m_status, 5))
		return;

	// see if special error
	if (BIT(m_status, 6))
		return;

	// set overrun flag if full
	if (BIT(m_status, 3))
	{
		LOG("FIFO overrun\n");
		m_status |= 0x20;
		return;
	}

	LOG("FIFO[%d] = %02X\n", m_status & 7, data);
	m_fifo[m_status & 7] = data;

	// bump fifo size & turn off underrun
	u8 fifo_size = m_status & 7;
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

	u8 scanner_mask = BIT(m_cmd[0], 0) ? 3 : BIT(m_cmd[0], 3) ? 15 : 7;
	bool decoded = BIT(m_cmd[0], 0);
	u8 kbd_type = (m_cmd[0] & 6) >> 1;
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
		u8 rl = m_in_rl_cb(0) ^ 0xff;     // inverted
		u8 addr = m_scanner & 7;
		assert(addr < std::size(m_s_ram));

		// see if key still down from last time
		u8 keys_down = rl & ~m_s_ram[addr];

		// now process new key
		switch (kbd_type)
		{
		case 0:
#if EMULATE_KEY_LOCKOUT
			// 2-key lockout
			if (keys_down != 0)
			{
				for (int i = 0; i < 8; i++)
				{
					if (BIT(keys_down, i))
					{
						if (m_debounce == 0 || m_key_down != (addr << 3 | i))
						{
							m_key_down = addr << 3 | i;
							m_debounce = 1;
						}
						else if (m_debounce++ > 1)
						{
							new_fifo((ctrl_key << 7) | (shift_key << 6) | m_key_down);
							m_s_ram[addr] |= 1 << i;
							m_debounce = 0;
						}
					}
				}
			}
			if ((m_key_down >> 3) == addr && !BIT(rl, m_key_down & 7))
				m_debounce = 0;
			m_s_ram[addr] &= rl;
			break;
#endif // EMULATE_KEY_LOCKOUT

		case 1:
			// N-key rollover
			if (keys_down != 0)
			{
				for (int i = 0; i < 8; i++)
				{
					if (BIT(keys_down, i))
					{
						if (m_debounce == 0)
						{
							m_key_down = addr << 3 | i;
							m_debounce = 1;
						}
						else if (m_key_down != (addr << 3 | i))
						{
#if EMULATE_KEY_LOCKOUT
							if (m_se_mode && !BIT(m_status, 6))
							{
								m_status |= 0x40;
								set_irq(1);
							}
#endif // EMULATE_KEY_LOCKOUT
						}
						else if (m_debounce++ > 1)
						{
							new_fifo((ctrl_key << 7) | (shift_key << 6) | m_key_down);
							m_s_ram[addr] |= 1 << i;
							m_debounce = 0;
						}
					}
				}
			}
			if ((m_key_down >> 3) == addr && !BIT(rl, m_key_down & 7))
				m_debounce = 0;
			m_s_ram[addr] &= rl;
			break;

		case 2:
			if (keys_down != 0 && !m_se_mode)
				m_status |= 0x40;

			if (m_s_ram[addr] != rl)
			{
				m_s_ram[addr] = rl;

				// IRQ line goes high if a row changes value
				set_irq(1);
			}
			break;

		case 3:
			if (strobe_pulse)
				new_fifo(rl);
			m_s_ram[addr] = rl;
			break;
		}
	}

	// Increment scanline

	m_scanner = (m_scanner + 1) & (decoded ? 3 : 15);

	if ( !m_out_sl_cb.isnull() )
	{
		// Active low strobed output in decoded mode
		if (decoded)
			m_out_sl_cb((offs_t)0, (1 << m_scanner) ^ 15);
		else
			m_out_sl_cb((offs_t)0, m_scanner);
	}

	// output a digit

	if ( !m_out_disp_cb.isnull() )
		m_out_disp_cb((offs_t)0, m_d_ram[m_scanner & scanner_mask] );
}


u8 i8279_device::read(offs_t offset)
{
	// A0 = control/data select
	return (offset & 1) ? status_r() : data_r();
}


u8 i8279_device::status_r()
{
	return m_status;
}


u8 i8279_device::data_r()
{
	u8 i;
	bool sensor_mode = ((m_cmd[0] & 6)==4);
	u8 data;
	if (m_read_flag)
	{
	// read the display ram
		data = m_d_ram[m_d_ram_ptr];
		if (m_autoinc && !machine().side_effects_disabled())
		{
			m_d_ram_ptr++;
		}
	}
	else
	if (sensor_mode)
	{
	// read sensor ram
		assert(m_s_ram_ptr < std::size(m_s_ram));
		data = m_s_ram[m_s_ram_ptr];
		if (!machine().side_effects_disabled())
		{
			if (m_autoinc)
			{
				m_s_ram_ptr++;
			}
			else
			{
				set_irq(0);
			}
		}
	}
	else
	{
	// read a key from fifo
		data = m_fifo[0];
		u8 fifo_size = m_status & 7;
		if (!machine().side_effects_disabled())
		{
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
					[[fallthrough]];
				default:
					logerror("Invalid status: %X\n", m_status);
			}
		}
		m_status = (m_status & 0xd0) | fifo_size; // turn off overrun & full
	}

	m_d_ram_ptr &= 15;
	m_s_ram_ptr &= 7;
	return data;
}


void i8279_device::write(offs_t offset, u8 data)
{
	// A0 = control/data select
	if (offset & 1)
		cmd_w(data);
	else
		data_w(data);
}


void i8279_device::cmd_w(u8 data)
{//printf("Command: %X=%X ",data>>5,data&31);
	u8 cmd = data >> 5;
	data &= 0x1f;
	m_cmd[cmd] = data;
	switch (cmd)
	{
		case 0:
			LOG("I8279 kb mode %x, display mode %x\n", data & 7, (data>>3) & 3);
			break;
		case 1:
			if (data > 1)
			{
				timer_adjust();
				logerror("Clock set to %.2f kHz\n", m_scanclock / 1000.0);
			}
			break;
		case 2:
			m_read_flag = 0;
			if ((m_cmd[0] & 6)==4) // sensor mode only
			{
				m_autoinc = BIT(data, 4);
				m_s_ram_ptr = data & 7;
				LOG("I8279 selct sensor row %x, AI %d\n", m_s_ram_ptr, m_autoinc);
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
			LOG("I8279 clear cmd %x\n", data);
			clear_display();
			break;
		case 7:
			set_irq(0);
			m_se_mode = BIT(data, 4);
			m_status &= 0xbf;
			break;
	}
}


void i8279_device::data_w(u8 data)
{//printf("Data: %X ",data);
	if (BIT(m_cmd[0], 4) && m_autoinc)
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
