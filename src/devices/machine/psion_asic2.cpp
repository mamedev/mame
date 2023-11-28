// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC2

    ASIC2 is the peripheral controller chip for the SIBO architecture. It contains
    the system clock oscillator and controls switching between the standby and
    operating states. ASIC2 provides an interface to the power supply, keyboard,
    buzzer and SSDs. ASIC 2 includes the eight-channel SIBO serial protocol
    controller and provides interface circuitry to both the reduced external and
    extended internal peripheral expansion ports.

******************************************************************************/

#include "emu.h"
#include "psion_asic2.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC2, psion_asic2_device, "psion_asic2", "Psion ASIC2")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic2_device::psion_asic2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_ASIC2, tag, owner, clock)
	, m_int_cb(*this)
	, m_nmi_cb(*this)
	, m_cbusy_cb(*this)
	, m_buz_cb(*this)
	, m_buzvol_cb(*this)
	, m_dr_cb(*this)
	, m_col_cb(*this, 0xff)
	, m_read_pd_cb(*this, 0x00)
	, m_write_pd_cb(*this)
	, m_data_r(*this, 0x00)
	, m_data_w(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic2_device::device_start()
{
	m_a2_status = 0x00;

	m_busy_timer = timer_alloc(FUNC(psion_asic2_device::busy), this);

	save_item(NAME(m_a2_index));
	save_item(NAME(m_a2_icontrol0));
	save_item(NAME(m_a2_icontrol1));
	save_item(NAME(m_a2_iddr));
	save_item(NAME(m_a2_control1));
	save_item(NAME(m_a2_control2));
	save_item(NAME(m_a2_control3));
	save_item(NAME(m_a2_serial_data));
	save_item(NAME(m_a2_serial_control));
	save_item(NAME(m_a2_interrupt_status));
	save_item(NAME(m_a2_status));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic2_device::device_reset()
{
	m_a2_index = 0x00;
	m_a2_icontrol0 = 0x00;
	m_a2_icontrol1 = 0x00;
	m_a2_iddr = 0x00;
	m_a2_control1 = 0x00;
	m_a2_control2 = 0x00;
	m_a2_control3 = 0x00;
	m_a2_serial_data = 0x00;
	m_a2_serial_control = 0x00;
	m_a2_interrupt_status = 0x00;
	m_a2_status = 0x00;
	m_a2_channel_control = 0x00;
}


void psion_asic2_device::on_clr_w(int state)
{
	if (state)
		m_a2_status |= 0x01; // A1OnKey
	else
		m_a2_status &= ~0x01;

	update_interrupts();
}

void psion_asic2_device::sds_int_w(int state)
{
	if (state)
		m_a2_status |= 0x20; // A2Sdis
	else
		m_a2_status &= ~0x20;

	update_interrupts();
}

void psion_asic2_device::dnmi_w(int state)
{
	if (state)
		m_a2_interrupt_status |= 0x01; // DNMI
	else
		m_a2_interrupt_status &= ~0x01;

	update_interrupts();
}

void psion_asic2_device::frcovl_w(int state)
{
	if (BIT(m_a2_control2, 4))
	{
		m_buz_cb(state);
	}
}

void psion_asic2_device::reset_w(int state)
{
	if (!state)
		m_a2_status |= 0x04; // A1ResetFlag
}

void psion_asic2_device::update_interrupts()
{
	int irq = m_a2_status & 0x21;
	int nmi = BIT(m_a2_interrupt_status, 0, 2) & BIT(m_a2_control3, 4, 2);

	m_int_cb(irq ? ASSERT_LINE : CLEAR_LINE);
	m_nmi_cb(nmi ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(psion_asic2_device::busy)
{
	m_a2_status &= ~0x10; // clear data serial controller is busy
	m_cbusy_cb(1);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t psion_asic2_device::io_r(offs_t offset)
{
	uint8_t data = 0x00;

	switch (offset & 7)
	{
	case 0x00: // A2Index - Index Register
		data = m_a2_index;
		LOG("%s io_r: A2Index => %02x\n", machine().describe_context(), data);
		break;

	case 0x01: // A2Control - Indexed Control Register
		switch (m_a2_index)
		{
		case 0: // A2IControl0 - System Control Register 0
			data = m_a2_icontrol0;
			LOG("%s io_r: A2IControl0 => %02x\n", machine().describe_context(), data);
			break;
		case 1: // A2IControl1 - Serial Clock Control Register
			data = m_a2_icontrol1;
			LOG("%s io_r: A2IControl1 => %02x\n", machine().describe_context(), data);
			break;
		case 2: // A2IWrite - Port Output Data Register (Write only)
			LOG("%s io_r: A2IWrite => %02x\n", machine().describe_context(), data);
			break;
		case 3: // A2IDDR - Port Data Direction Register
			data = m_a2_iddr;
			LOG("%s io_r: A2IDDR => %02x\n", machine().describe_context(), data);
			break;
		}
		break;

	case 0x02: // A2External - External status register
		switch (m_a2_control1 & 0x0f)
		{
		case 0x0f: // all keyboard columns
			for (int i = 0; i < 10; i++)
				data |= m_col_cb(i);
			break;

		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0a:
			data = m_col_cb((m_a2_control1 & 0x0f) - 1);
			break;

		default:
			data = 0x00;
			break;
		}
		LOG("%s io_r: A2External => %02x\n", machine().describe_context(), data);
		break;

	case 0x03: // A2InterruptStatus - Interrupt status register
		// b0 DoorInterrupt      - Status of pack doors, 1 if open
		// b1 ExpansionInterrupt - Status of expansion interface doors, 1 if open
		// b2 SlaveDataValid     - Slave data valid, 1 if valid frame arrived
		// b3 SlaveDataControl   - Slave control or data frame, 1 if control frame
		// b4 SlaveDataOverrun   - Slave data overrun, 1 if overrun
		// b5 LowBatteryNMI      - Low battery NMI, 1 if NMI occurred
		// b6 0
		// b7 0
		data = m_a2_interrupt_status;
		LOG("%s io_r: A2InterruptStatus => %02x\n", machine().describe_context(), data);
		break;

	case 0x04: // A2Status - Status register
		// b0 A1OnKey          - 1 if on key pressed
		// b1 WakeUp           - 1 if external wakeup valid
		// b2 A1ResetFlag      - 1 if system reset
		// b3 SerialClockState - 1 if slave clock is high
		// b4 SerialBusy       - 1 while data serial controller is busy
		// b5 A2Sdis           - 1 if slave data signal is high
		// b6 Ext              - 1 in Extended Mode, 0 in Compatible Mode
		// b7 RevId            - 1 if Revision 4 part
		data = m_a2_status;
		if (!machine().side_effects_disabled())
		{
			m_a2_status &= ~0x01; // clear A1OnKey
			m_a2_status &= ~0x04; // clear A1ResetFlag
		}
		LOG("%s io_r: A2Status => %02x\n", machine().describe_context(), data);
		break;

	case 0x05: // A2SerialData - Serial channel read data register
		data = m_a2_serial_data;
		if ((m_a2_serial_control & 0x10) == 0x10)
			m_a2_serial_data = receive_frame();
		LOG("%s io_r: A2SerialData => %02x\n", machine().describe_context(), data);
		break;

	case 0x06: // A2KeyData - Keyboard poll register
		// Extended mode read from Port I/O lines
		data = m_read_pd_cb();
		LOG("%s io_r: A2KeyData => %02x\n", machine().describe_context(), data);
		break;

	case 0x07: // A2SlaveData - Slave data read register
		data = 0x00;
		LOG("%s io_r: A2SlaveData => %02x\n", machine().describe_context(), data);
		break;

	default:
		LOG("%s io_r: Unhandled register %02x => %02x\n", machine().describe_context(), offset, data);
		break;
	}
	return data;
}

void psion_asic2_device::io_w(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case 0x00: // A2Index - Index Register
		// b0-b1 INDEX0,1 - Register 1 Index
		LOG("%s io_w: A2Index <= %02x\n", machine().describe_context(), data);
		m_a2_index = data & 3;
		break;

	case 0x01: // A2Control - Indexed Control Register
		switch (m_a2_index)
		{
		case 0: // A2IControl0 - System Control Register 0
			// b0-b1 CLKSEL0-1 - Crystal Divider Select (0,1 - 23.04MHz divider = 3, 2 - 15.36MHz divider = 2, 3 - 7.68MHz divider = 1)
			// b2    CHEN3     - Serial Channel 3 is enabled when this bit is set
			// b3    CHEN4     - Serial Channel 4 is enabled when this bit is set
			// b4    CHEN6     - Serial Channel 6 is enabled when this bit is set
			// b5    EXONOFF   - EXON & SCKS witch-on is disabled when this bit is set
			// b6    INTSEL    - Interrupt Source Select (0 - Frame received, 1 - SDIS direct)
			LOG("%s io_w: A2IControl0 => %02x\n", machine().describe_context(), data);
			m_a2_icontrol0 = data;
			break;
		case 1: // A2IControl1 - Serial Clock Control Register
			// b0 CKEN1 - Serial Channel 1 continuous clock is enabled when this bit is set
			// b1 CKEN2 - Serial Channel 2 continuous clock is enabled when this bit is set
			// b2 CKEN3 - Serial Channel 3 continuous clock is enabled when this bit is set
			// b3 CKEN4 - Serial Channel 4 continuous clock is enabled when this bit is set
			LOG("%s io_w: A2IControl1 => %02x\n", machine().describe_context(), data);
			m_a2_icontrol1 = data;
			break;
		case 2: // A2IWrite - Port Output Data Register
			LOG("%s io_w: A2IWrite => %02x\n", machine().describe_context(), data);
			m_write_pd_cb(data & m_a2_iddr);
			break;
		case 3: // A2IDDR - Port Data Direction Register
			// b0-b7 DO0-7 - 0 = PD* is and input, 1 = PD* is an output
			LOG("%s io_w: A2IDDR => %02x\n", machine().describe_context(), data);
			m_a2_iddr = data;
			break;
		}
		break;

	case 0x02: // A2Control1 - Control register 1
		// b0-b3 KeyScan         - Values 0-15 drive the keyboard poll columns
		// b4-b5 SerialClockRate - Sets the clock frequency for channels 1-4 and 7 as below:
		//                           0 = ClockRateMedium 1.536 MHz
		//                           2 = ClockRateSlow   256 KHz
		//                           3 = ClockRateFast   3.84 MHz
		LOG("%s io_w: A2Control1 <= %02x\n", machine().describe_context(), data);
		m_a2_control1 = data;
		break;

	case 0x03: // A2Control2 - Control register 2
		// b0 DigitizerEnable  - 1 to enable digitizer drivers
		// b1 XySwitch         - 0 to drive digitizer X scan, 1 to drive Y
		// b2 BuzzerToggle     - Toggles the piezo speaker
		// b3 BuzzerVolume     - 1 if loud buzzer volume, 0 is soft
		// b4 BuzzerMode       - Piezo driving mode, 0 - toggled by BZTOG, 1 - driven by FRC
		// b5 ClockEnable5     - Enable continuous clock on serial channel 5
		// b6 ClockEnable6     - Enable continuous clock on serial channel 6
		// b7 ClockEnable7     - Enable continuous clock on serial channel 7
		LOG("%s io_w: A2Control2 <= %02x\n", machine().describe_context(), data);
		m_a2_control2 = data;
		m_dr_cb(BIT(data, 1));
		if (!BIT(data, 4))
		{
			m_buz_cb(BIT(data, 2));
		}
		m_buzvol_cb(BIT(data, 3));
		break;

	case 0x04: // A2Control3 - Control register 3
		// b0 SerialNull       - 1 to send a null frame
		// b1 Ps34Acknowledge  - 1 to reset PS34 on power up
		// b2 A2SldEnable      - 1 to enable SLD bus clock
		// b3 SerialEnable     - 1 to enable access to pack channels 1-4
		// b4 DoorEnable       - 1 to enable pack door NMIs
		// b5 ExpansionEnable  - 1 to enable expansion door NMIs
		// b6 VhControl        - 1 to enable VPP to packs
		// b7 ElEnable         - 1 to enable LCD back light
		LOG("%s io_w: A2Control3 <= %02x\n", machine().describe_context(), data);
		m_a2_control3 = data;
		if (BIT(m_a2_control3, 0))
			transmit_frame(NULL_FRAME);
		break;

	case 0x05: // A2SerialData - Serial channel write data register
		LOG("%s io_w: A2SerialData <= %02x\n", machine().describe_context(), data);
		if ((m_a2_serial_control & 0xc0) == 0x80)
			transmit_frame(DATA_FRAME | data);
		break;

	case 0x06: // A2SerialControl - Serial channel write control register
		// WriteSingle  10000000b;  ReadSingle   11000000b
		// WriteMulti   10010000b;  ReadMulti    11010000b
		// Reset        00000000b;  Select       01000000b
		// Asic2SlaveId      001h;  Asic5PackId       002h
		// Asic5NormalId     003h;  Asic6Id           004h
		// Asic8Id           005h;  Asic4Id           006h
		LOG("%s io_w: A2SerialControl <= %02x\n", machine().describe_context(), data);
		m_a2_serial_control = data;
		transmit_frame(CONTROL_FRAME | data);

		if ((m_a2_serial_control & 0x40) == 0x40)
			m_a2_serial_data = receive_frame();
		break;

	case 0x07: // A2ChannelControl - Serial channel select register
		// b0    Pack1Enable         - 1 to select pack 1
		// b1    Pack2Enable         - 1 to select pack 2
		// b2    Pack3Enable         - 1 to select pack 3
		// b3    Pack4Enable         - 1 to select pack 4
		// b4-b6 ChannelSelect       - 0-7 to select channel 0,5-8 as below:
		//                               4 = PK0, 5 = PK5, 6 = PK6, 7 = PK7
		// b7    MultiplexEnable     - 1 to loop slave channel to pack channel
		LOG("%s io_w: A2ChannelControl <= %02x Channels %c %c %c %c %c %c %c %c\n", machine().describe_context(), data,
			BIT(data, 4, 3) == 4 ? '0' : ' ',
			BIT(data, 0) ? '1' : ' ',
			BIT(data, 1) ? '2' : ' ',
			BIT(data, 2) ? '3' : ' ',
			BIT(data, 3) ? '4' : ' ',
			BIT(data, 4, 3) == 5 ? '5' : ' ',
			BIT(data, 4, 3) == 6 ? '6' : ' ',
			BIT(data, 4, 3) == 7 ? '7' : ' ');
		m_a2_channel_control = data;
		break;

	default:
		LOG("%s io_w: Unhandled register %02x <= %02x\n", machine().describe_context(), offset, data);
		break;
	}
}


//-------------------------------------------------
//  SIBO Serial Protocol Controller
//-------------------------------------------------

bool psion_asic2_device::channel_active(int channel)
{
	switch (channel)
	{
	case 0:
		return BIT(m_a2_channel_control, 4, 3) == 4;
	case 1: case 2: case 3: case 4:
		return BIT(m_a2_channel_control, channel - 1);
	case 5: case 6: case 7:
		return BIT(m_a2_channel_control, 4, 3) == channel;
	}
	return false;
}

void psion_asic2_device::transmit_frame(uint16_t data)
{
	m_a2_status |= 0x10; // set data serial controller is busy
	m_busy_timer->adjust(attotime::from_ticks(12, clock() / 2));
	m_cbusy_cb(0);

	for (int ch = 0; ch < 8; ch++)
	{
		if (channel_active(ch))
		{
			LOG("%s Channel %d Transmit %s frame %02x\n", machine().describe_context(), ch, (data & DATA_FRAME) ? "Data" : (data & CONTROL_FRAME) ? "Control" : "Null", data & 0xff);
			m_data_w[ch](data);
		}
	}
}

uint8_t psion_asic2_device::receive_frame()
{
	uint8_t data = 0x00;

	m_a2_status |= 0x10; // set data serial controller is busy
	m_busy_timer->adjust(attotime::from_ticks(12, clock() / 2));
	m_cbusy_cb(0);

	for (int ch = 0; ch < 8; ch++)
	{
		if (channel_active(ch))
		{
			data |= m_data_r[ch]();
			LOG("%s Channel %d Receive Data frame %02x\n", machine().describe_context(), ch, data);
		}
	}

	return data;
}
