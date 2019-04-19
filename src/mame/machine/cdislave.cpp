// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i Mono-I SLAVE MCU simulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Decapping and proper emulation.

*******************************************************************************/

#include "emu.h"
#include "machine/cdislave.h"

#define LOG_IRQS		(1 << 0)
#define LOG_COMMANDS	(1 << 1)
#define LOG_READS		(1 << 2)
#define LOG_WRITES		(1 << 3)
#define LOG_UNKNOWNS	(1 << 4)

#define VERBOSE			(0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(CDI_SLAVE, cdislave_device, "cdislave", "CD-i Mono-I Slave")


//**************************************************************************
//  MEMBER FUNCTIONS
//**************************************************************************

TIMER_CALLBACK_MEMBER( cdislave_device::trigger_readback_int )
{
	LOGMASKED(LOG_IRQS, "Asserting IRQ2\n");
	m_int_callback(ASSERT_LINE);
	m_interrupt_timer->adjust(attotime::never);
}

void cdislave_device::prepare_readback(const attotime &delay, uint8_t channel, uint8_t count, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t cmd)
{
	m_channel[channel].m_out_index = 0;
	m_channel[channel].m_out_count = count;
	m_channel[channel].m_out_buf[0] = data0;
	m_channel[channel].m_out_buf[1] = data1;
	m_channel[channel].m_out_buf[2] = data2;
	m_channel[channel].m_out_buf[3] = data3;
	m_channel[channel].m_out_cmd = cmd;

	m_interrupt_timer->adjust(delay);
}

void cdislave_device::perform_mouse_update()
{
	uint16_t x = m_mousex->read();
	uint16_t y = m_mousey->read();
	uint8_t buttons = m_mousebtn->read();

	uint16_t old_mouse_x = m_real_mouse_x;
	uint16_t old_mouse_y = m_real_mouse_y;

	if (m_real_mouse_x == 0xffff)
	{
		old_mouse_x = x & 0x3ff;
		old_mouse_y = y & 0x3ff;
	}

	m_real_mouse_x = x & 0x3ff;
	m_real_mouse_y = y & 0x3ff;

	m_fake_mouse_x += (m_real_mouse_x - old_mouse_x);
	m_fake_mouse_y += (m_real_mouse_y - old_mouse_y);

	while (m_fake_mouse_x > 0x3ff)
	{
		m_fake_mouse_x += 0x400;
	}

	while (m_fake_mouse_y > 0x3ff)
	{
		m_fake_mouse_y += 0x400;
	}

	x = m_fake_mouse_x;
	y = m_fake_mouse_y;

	if (m_polling_active)
	{
		prepare_readback(attotime::zero, 0, 4, ((x & 0x380) >> 7) | (buttons << 4), x & 0x7f, (y & 0x380) >> 7, y & 0x7f, 0xf7);
	}
}

INPUT_CHANGED_MEMBER( cdislave_device::mouse_update )
{
	perform_mouse_update();
}

static INPUT_PORTS_START(cdislave_mouse)
	PORT_START("MOUSEX")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2) PORT_CHANGED_MEMBER(DEVICE_SELF, cdislave_device, mouse_update, 0)

	PORT_START("MOUSEY")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2) PORT_CHANGED_MEMBER(DEVICE_SELF, cdislave_device, mouse_update, 0)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CHANGED_MEMBER(DEVICE_SELF, cdislave_device, mouse_update, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CHANGED_MEMBER(DEVICE_SELF, cdislave_device, mouse_update, 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor cdislave_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cdislave_mouse);
}

READ16_MEMBER( cdislave_device::slave_r )
{
	if (m_channel[offset].m_out_count)
	{
		uint8_t ret = m_channel[offset].m_out_buf[m_channel[offset].m_out_index];
		LOGMASKED(LOG_READS, "slave_r: Channel %d: %d, %02x\n", offset, m_channel[offset].m_out_index, ret);
		if (m_channel[offset].m_out_index == 0)
		{
			switch (m_channel[offset].m_out_cmd)
			{
				case 0xb0:
				case 0xb1:
				case 0xf0:
				case 0xf3:
				case 0xf4:
				case 0xf7:
					LOGMASKED(LOG_IRQS, "slave_r: De-asserting IRQ2\n");
					m_int_callback(CLEAR_LINE);
					break;
			}
		}
		m_channel[offset].m_out_index++;
		m_channel[offset].m_out_count--;
		if (!m_channel[offset].m_out_count)
		{
			m_channel[offset].m_out_index = 0;
			m_channel[offset].m_out_cmd = 0;
			memset(m_channel[offset].m_out_buf, 0, 4);
		}
		return ret;
	}
	LOGMASKED(LOG_READS, "slave_r: Channel %d: %d (nothing to output)\n", offset, m_channel[offset].m_out_index);
	return 0xff;
}

void cdislave_device::set_mouse_position()
{
//    uint16_t x, y;

	//printf( "Set mouse position: %02x %02x %02x\n", m_in_buf[0], m_in_buf[1], m_in_buf[2] );

	m_fake_mouse_y = ((m_in_buf[1] & 0x0f) << 6) | (m_in_buf[0] & 0x3f);
	m_fake_mouse_x = ((m_in_buf[1] & 0x70) << 3) | m_in_buf[2];

//    x = m_fake_mouse_x;
//    y = m_fake_mouse_y;

	if (m_polling_active)
	{
		//prepare_readback(attotime::zero, 0, 4, (x & 0x380) >> 7, x & 0x7f, (y & 0x380) >> 7, y & 0x7f, 0xf7);
	}
}

WRITE16_MEMBER( cdislave_device::slave_w )
{
	LOGMASKED(LOG_WRITES, "slave_w: Channel %d: %d = %02x\n", offset, m_in_index, data & 0x00ff);
	switch (offset)
	{
		case 0:
			if (m_in_index)
			{
				m_in_buf[m_in_index] = data & 0x00ff;
				m_in_index++;
				if (m_in_index == m_in_count)
				{
					switch (m_in_buf[0])
					{
						case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
						case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
						case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
						case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
						case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
						case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
						case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
						case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: // Update Mouse Position
							set_mouse_position();
							memset(m_in_buf, 0, 17);
							m_in_index = 0;
							m_in_count = 0;
							break;
					}
				}
			}
			else
			{
				m_in_buf[m_in_index] = data & 0x00ff;
				m_in_index++;
				switch (data & 0x00ff)
				{
					case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
					case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
					case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
					case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
					case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
					case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
					case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
					case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
						LOGMASKED(LOG_COMMANDS, "slave_w: Channel %d: Update Mouse Position (0x%02x)\n", offset, data & 0x00ff);
						m_in_count = 3;
						break;
					default:
						LOGMASKED(LOG_COMMANDS | LOG_UNKNOWNS, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff);
						m_in_index = 0;
						break;
				}
			}
			break;
		case 1:
			if (m_in_index)
			{
				m_in_buf[m_in_index] = data & 0x00ff;
				m_in_index++;
				if (m_in_index == m_in_count)
				{
					switch (m_in_buf[0])
					{
						case 0xf0: // Set Front Panel LCD
							memcpy(m_lcd_state, m_in_buf + 1, 16);
							memset(m_in_buf, 0, 17);
							m_in_index = 0;
							m_in_count = 0;
							break;
						default:
							memset(m_in_buf, 0, 17);
							m_in_index = 0;
							m_in_count = 0;
							break;
					}
				}
			}
			else
			{
				switch (data & 0x00ff)
				{
					default:
						LOGMASKED(LOG_COMMANDS | LOG_UNKNOWNS, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff);
						memset(m_in_buf, 0, 17);
						m_in_index = 0;
						m_in_count = 0;
						break;
				}
			}
			break;
		case 2:
			if (m_in_index)
			{
				m_in_buf[m_in_index] = data & 0x00ff;
				m_in_index++;
				if (m_in_index == m_in_count)
				{
					switch (m_in_buf[0])
					{
						case 0xf0: // Set Front Panel LCD
							memset(m_in_buf + 1, 0, 16);
							m_in_count = 17;
							break;
						default:
							memset(m_in_buf, 0, 17);
							m_in_index = 0;
							m_in_count = 0;
							break;
					}
				}
			}
			else
			{
				m_in_buf[m_in_index] = data & 0x00ff;
				m_in_index++;
				switch (data & 0x00ff)
				{
					case 0x82: // Mute Audio
						LOGMASKED(LOG_COMMANDS, "slave_w: Channel %d: Mute Audio (0x82)\n", offset);
						m_dmadac[0]->enable(0);
						m_dmadac[1]->enable(0);
						m_in_index = 0;
						m_in_count = 0;
						//cdic->audio_sample_timer->adjust(attotime::never);
						break;
					case 0x83: // Unmute Audio
						LOGMASKED(LOG_COMMANDS, "slave_w: Channel %d: Unmute Audio (0x83)\n", offset);
						m_dmadac[0]->enable(1);
						m_dmadac[1]->enable(1);
						m_in_index = 0;
						m_in_count = 0;
						break;
					case 0xf0: // Set Front Panel LCD
						LOGMASKED(LOG_COMMANDS, "slave_w: Channel %d: Set Front Panel LCD (0xf0)\n", offset);
						m_in_count = 17;
						break;
					default:
						LOGMASKED(LOG_COMMANDS | LOG_UNKNOWNS, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff);
						memset(m_in_buf, 0, 17);
						m_in_index = 0;
						m_in_count = 0;
						break;
				}
			}
			break;
		case 3:
			if (m_in_index)
			{
				m_in_buf[m_in_index] = data & 0x00ff;
				m_in_index++;
				if (m_in_index == m_in_count)
				{
					switch (m_in_buf[0])
					{
						case 0xb0: // Request Disc Status
							memset(m_in_buf, 0, 17);
							m_in_index = 0;
							m_in_count = 0;
							prepare_readback(attotime::from_hz(4), 3, 4, 0xb0, 0x00, 0x02, 0x15, 0xb0);
							break;
						//case 0xb1: // Request Disc Base
							//memset(m_in_buf, 0, 17);
							//m_in_index = 0;
							//m_in_count = 0;
							//prepare_readback(attotime::from_hz(10000), 3, 4, 0xb1, 0x00, 0x00, 0x00, 0xb1);
							//break;
						default:
							memset(m_in_buf, 0, 17);
							m_in_index = 0;
							m_in_count = 0;
							break;
					}
				}
			}
			else
			{
				m_in_buf[m_in_index] = data & 0x00ff;
				m_in_index++;
				switch (data & 0x00ff)
				{
					case 0xb0: // Request Disc Status
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: Request Disc Status (0xb0)\n", offset);
						m_in_count = 4;
						break;
					case 0xb1: // Request Disc Base
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: Request Disc Base (0xb1)\n", offset);
						m_in_count = 4;
						break;
					case 0xf0: // Request SLAVE Revision
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: Request SLAVE Revision (0xf0)\n", offset);
						prepare_readback(attotime::from_hz(10000), 2, 2, 0xf0, 0x32, 0x31, 0, 0xf0);
						m_in_index = 0;
						break;
					case 0xf3: // Request Pointer Type
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: Request Pointer Type (0xf3)\n", offset);
						m_in_index = 0;
						prepare_readback(attotime::from_hz(10000), 2, 2, 0xf3, 1, 0, 0, 0xf3);
						break;
					case 0xf4: // Request Test Plug Status
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: Request Test Plug Status (0xf4)\n", offset);
						m_in_index = 0;
						prepare_readback(attotime::from_hz(10000), 2, 2, 0xf4, 0, 0, 0, 0xf4);
						break;
					case 0xf6: // Request NTSC/PAL Status
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: Request NTSC/PAL Status (0xf6)\n", offset);
						prepare_readback(attotime::never, 2, 2, 0xf6, 2, 0, 0, 0xf6);
						m_in_index = 0;
						break;
					case 0xf7: // Enable Input Polling
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: Activate Input Polling (0xf7)\n", offset);
						m_polling_active = 1;
						m_in_index = 0;
						break;
					case 0xfa: // Enable X-Bus Interrupts
						LOGMASKED(LOG_COMMANDS | LOG_WRITES, "slave_w: Channel %d: X-Bus Interrupt Enable (0xfa)\n", offset);
						m_xbus_interrupt_enable = 1;
						m_in_index = 0;
						break;
					default:
						LOGMASKED(LOG_COMMANDS | LOG_UNKNOWNS, "slave_w: Channel %d: Unknown register: %02x\n", offset, data & 0x00ff);
						memset(m_in_buf, 0, 17);
						m_in_index = 0;
						m_in_count = 0;
						break;
				}
			}
			break;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdislave_device - constructor
//-------------------------------------------------

cdislave_device::cdislave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_SLAVE, tag, owner, clock)
	, m_int_callback(*this)
	, m_dmadac(*this, ":dac%u", 1U)
	, m_mousex(*this, "MOUSEX")
	, m_mousey(*this, "MOUSEY")
	, m_mousebtn(*this, "MOUSEBTN")
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void cdislave_device::device_resolve_objects()
{
	m_int_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdislave_device::device_start()
{
	save_item(NAME(m_channel[0].m_out_buf[0]));
	save_item(NAME(m_channel[0].m_out_buf[1]));
	save_item(NAME(m_channel[0].m_out_buf[2]));
	save_item(NAME(m_channel[0].m_out_buf[3]));
	save_item(NAME(m_channel[0].m_out_index));
	save_item(NAME(m_channel[0].m_out_count));
	save_item(NAME(m_channel[0].m_out_cmd));
	save_item(NAME(m_channel[1].m_out_buf[0]));
	save_item(NAME(m_channel[1].m_out_buf[1]));
	save_item(NAME(m_channel[1].m_out_buf[2]));
	save_item(NAME(m_channel[1].m_out_buf[3]));
	save_item(NAME(m_channel[1].m_out_index));
	save_item(NAME(m_channel[1].m_out_count));
	save_item(NAME(m_channel[1].m_out_cmd));
	save_item(NAME(m_channel[2].m_out_buf[0]));
	save_item(NAME(m_channel[2].m_out_buf[1]));
	save_item(NAME(m_channel[2].m_out_buf[2]));
	save_item(NAME(m_channel[2].m_out_buf[3]));
	save_item(NAME(m_channel[2].m_out_index));
	save_item(NAME(m_channel[2].m_out_count));
	save_item(NAME(m_channel[2].m_out_cmd));
	save_item(NAME(m_channel[3].m_out_buf[0]));
	save_item(NAME(m_channel[3].m_out_buf[1]));
	save_item(NAME(m_channel[3].m_out_buf[2]));
	save_item(NAME(m_channel[3].m_out_buf[3]));
	save_item(NAME(m_channel[3].m_out_index));
	save_item(NAME(m_channel[3].m_out_count));
	save_item(NAME(m_channel[3].m_out_cmd));

	save_item(NAME(m_in_buf));
	save_item(NAME(m_in_index));
	save_item(NAME(m_in_count));

	save_item(NAME(m_polling_active));

	save_item(NAME(m_xbus_interrupt_enable));

	save_item(NAME(m_lcd_state));

	save_item(NAME(m_real_mouse_x));
	save_item(NAME(m_real_mouse_y));

	save_item(NAME(m_fake_mouse_x));
	save_item(NAME(m_fake_mouse_y));

	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cdislave_device::trigger_readback_int), this));
	m_interrupt_timer->adjust(attotime::never);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdislave_device::device_reset()
{
	for (auto & elem : m_channel)
	{
		elem.m_out_buf[0] = 0;
		elem.m_out_buf[1] = 0;
		elem.m_out_buf[2] = 0;
		elem.m_out_buf[3] = 0;
		elem.m_out_index = 0;
		elem.m_out_count = 0;
		elem.m_out_cmd = 0;
	}

	memset(m_in_buf, 0, 17);
	m_in_index = 0;
	m_in_count = 0;

	m_polling_active = 0;

	m_xbus_interrupt_enable = 0;

	memset(m_lcd_state, 0, 16);

	m_real_mouse_x = 0xffff;
	m_real_mouse_y = 0xffff;

	m_fake_mouse_x = 0;
	m_fake_mouse_y = 0;

	m_int_callback(CLEAR_LINE);
}
