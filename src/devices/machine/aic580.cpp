// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec AIC-580

    This 84-pin PLCC ASIC provides 16-bit DMA bus mastering capabilities
    for the AIC-1540A/1542A and AIC-1540B/1542B PC/AT SCSI host adapters
    and the AIC-1640 Micro Channel SCSI host adapter.

    No documentation for this IC has been found. It might function somewhat
    similarly to the AIC-560, which is also poorly documented.

***************************************************************************/

#include "emu.h"
#include "aic580.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(AIC580, aic580_device, "aic580", "AIC-580 DMA Bus Master")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  aic580_device - constructor
//-------------------------------------------------

aic580_device::aic580_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AIC580, tag, owner, clock)
	, m_bdin_callback(*this)
	, m_bdout_callback(*this)
	, m_back_callback(*this)
	, m_sread_callback(*this)
	, m_swrite_callback(*this)
	, m_dma_mode(0)
	, m_channel_addr{0}
	, m_fifo_read_index(0)
	, m_fifo_write_index(0)
	, m_fifo_data{0}
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void aic580_device::device_resolve_objects()
{
	// resolve callbacks
	m_bdin_callback.resolve_safe(0);
	m_bdout_callback.resolve_safe();
	m_back_callback.resolve_safe();
	m_sread_callback.resolve_safe(0);
	m_swrite_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aic580_device::device_start()
{
	// register state
	save_item(NAME(m_dma_mode));
	save_item(NAME(m_channel_addr));
	save_item(NAME(m_fifo_read_index));
	save_item(NAME(m_fifo_write_index));
	save_item(NAME(m_fifo_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void aic580_device::device_reset()
{
	m_fifo_read_index = 0;
	m_fifo_write_index = 0;
	m_dma_mode = 0;
}


//-------------------------------------------------
//  breq_w - handle transfer requests for port B
//-------------------------------------------------

WRITE_LINE_MEMBER(aic580_device::breq_w)
{
	if (state)
	{
		if (m_dma_mode == 0x80)
		{
			m_fifo_data[m_fifo_write_index] = m_bdin_callback();
			m_fifo_write_index = (m_fifo_write_index + 1) & 15;
			m_back_callback(0);
		}
		else if (m_dma_mode == 0x90)
		{
			m_bdout_callback(m_fifo_data[m_fifo_read_index]);
			m_fifo_read_index = (m_fifo_read_index + 1) & 15;
			m_back_callback(0);
		}
		else
			logerror("BREQ with unemulated DMA mode %02X\n", m_dma_mode);
	}
}


//**************************************************************************
//  REGISTER HANDLERS
//**************************************************************************

//-------------------------------------------------
//  r80_r - local read from register 80h
//-------------------------------------------------

u8 aic580_device::r80_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Read from register 80h\n", machine().describe_context());
	return 0;
}


//-------------------------------------------------
//  transfer_speed_w - local write to register 80h
//-------------------------------------------------

void aic580_device::transfer_speed_w(u8 data)
{
	logerror("Transfer speed: %dns read pulse, %dns strobe off, %dns write pulse (%.2f MB/s)\n",
		int(1.0E9 * (((data & 0x30) >> 4) + 2) / clock()),
		int(1.0E9 * (((data & 0x08) >> 3) + 2) / clock()),
		int(1.0E9 * ((data & 0x07) + 2) / clock()),
		clock() * 2.0E-6 / ((BIT(data, 3) ? 5 : 4) + (data & 7)));
}


//-------------------------------------------------
//  dma_mode_w - local write to register 82h
//-------------------------------------------------

void aic580_device::dma_mode_w(u8 data)
{
	m_dma_mode = data;
}


//-------------------------------------------------
//  r83_w - local write to register 83h
//-------------------------------------------------

void aic580_device::r83_w(u8 data)
{
	logerror("%s: Write %02Xh to register 83h\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  r86_w - local write to register 86h
//-------------------------------------------------

void aic580_device::r86_w(u8 data)
{
	logerror("%s: Write %02Xh to register 86h\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  r88_r - local read from register 88h
//-------------------------------------------------

u8 aic580_device::r88_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Read from register 88h\n", machine().describe_context());
	return 0;
}


//-------------------------------------------------
//  r88_w - local write to register 88h
//-------------------------------------------------

void aic580_device::r88_w(u8 data)
{
	logerror("%s: Write %02Xh to register 88h\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  r8a_w - local write to register 8Ah
//-------------------------------------------------

void aic580_device::r8a_w(u8 data)
{
	logerror("%s: Write %02Xh to register 8Ah\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  r8b_w - local write to register 8Bh
//-------------------------------------------------

void aic580_device::r8b_w(u8 data)
{
	logerror("%s: Write %02Xh to register 8Bh\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  bus_on_time_w - local write to register 8Ch
//-------------------------------------------------

void aic580_device::bus_on_time_w(u8 data)
{
	logerror("Bus on time = %d microseconds\n", data & 0x0f);
}


//-------------------------------------------------
//  bus_off_time_w - local write to register 8Dh
//-------------------------------------------------

void aic580_device::bus_off_time_w(u8 data)
{
	logerror("Bus off time = %d microseconds\n", std::max(1, (data & 0x0f) * 4));
}


//-------------------------------------------------
//  ch_addrl_w - set bits 7:0 of buffer address
//  for channel 1 or 2
//-------------------------------------------------

template<int Channel>
void aic580_device::ch_addrl_w(u8 data)
{
	m_channel_addr[Channel] = (m_channel_addr[Channel] & 0xffff00) | data;
}


//-------------------------------------------------
//  ch_addrm_w - set bits 15:8 of buffer address
//  for channel 1 or 2
//-------------------------------------------------

template<int Channel>
void aic580_device::ch_addrm_w(u8 data)
{
	m_channel_addr[Channel] = (m_channel_addr[Channel] & 0xff00ff) | u32(data) << 8;
}


//-------------------------------------------------
//  ch_addrh_w - set bits 23:16 of buffer address
//  for channel 1 or 2
//-------------------------------------------------

template<int Channel>
void aic580_device::ch_addrh_w(u8 data)
{
	m_channel_addr[Channel] = (m_channel_addr[Channel] & 0x00ffff) | u32(data) << 16;
}


//-------------------------------------------------
//  ra2_w - local write to register A2h
//-------------------------------------------------

void aic580_device::ra2_w(u8 data)
{
	logerror("%s: Write %02Xh to register A2h\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  ra3_w - local write to register A3h
//-------------------------------------------------

void aic580_device::ra3_w(u8 data)
{
	logerror("%s: Write %02Xh to register A3h\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  ra4_w - local write to register A4h
//-------------------------------------------------

void aic580_device::ra4_w(u8 data)
{
	logerror("%s: Write %02Xh to register A4h\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  fifo_data_r - local read from register ACh
//-------------------------------------------------

u8 aic580_device::fifo_data_r()
{
	const u8 data = m_fifo_data[m_fifo_read_index];
	if (!machine().side_effects_disabled())
		m_fifo_read_index = (m_fifo_read_index + 1) & 15;
	return data;
}


//-------------------------------------------------
//  fifo_data_w - local write to register ACh
//-------------------------------------------------

void aic580_device::fifo_data_w(u8 data)
{
	m_fifo_data[m_fifo_write_index] = data;
	if (!machine().side_effects_disabled())
		m_fifo_write_index = (m_fifo_write_index + 1) & 15;
}


//-------------------------------------------------
//  buffer_r - local read from registers C0h–CFh
//-------------------------------------------------

u8 aic580_device::buffer_r(offs_t offset)
{
	return m_fifo_data[offset];
}


//-------------------------------------------------
//  buffer_w - local write to registers C0h–CFh
//-------------------------------------------------

void aic580_device::buffer_w(offs_t offset, u8 data)
{
	m_fifo_data[offset] = data;
}


//-------------------------------------------------
//  mpu_map - address map for local microprocessor
//-------------------------------------------------

void aic580_device::mpu_map(address_map &map)
{
	map(0x80, 0x80).rw(FUNC(aic580_device::r80_r), FUNC(aic580_device::transfer_speed_w));
	map(0x82, 0x82).w(FUNC(aic580_device::dma_mode_w));
	map(0x83, 0x83).w(FUNC(aic580_device::r83_w));
	map(0x86, 0x86).w(FUNC(aic580_device::r86_w));
	map(0x88, 0x88).rw(FUNC(aic580_device::r88_r), FUNC(aic580_device::r88_w));
	map(0x8a, 0x8a).w(FUNC(aic580_device::r8a_w));
	map(0x8b, 0x8b).w(FUNC(aic580_device::r8b_w));
	map(0x8c, 0x8c).w(FUNC(aic580_device::bus_on_time_w));
	map(0x8d, 0x8d).w(FUNC(aic580_device::bus_off_time_w));
	map(0x98, 0x98).w(FUNC(aic580_device::ch_addrl_w<0>));
	map(0x99, 0x99).w(FUNC(aic580_device::ch_addrm_w<0>));
	map(0x9a, 0x9a).w(FUNC(aic580_device::ch_addrh_w<0>));
	map(0x9c, 0x9c).w(FUNC(aic580_device::ch_addrl_w<1>));
	map(0x9d, 0x9d).w(FUNC(aic580_device::ch_addrm_w<1>));
	map(0x9e, 0x9e).w(FUNC(aic580_device::ch_addrh_w<1>));
	map(0xa2, 0xa2).w(FUNC(aic580_device::ra2_w));
	map(0xa3, 0xa3).w(FUNC(aic580_device::ra3_w));
	map(0xa4, 0xa4).w(FUNC(aic580_device::ra4_w));
	map(0xac, 0xac).rw(FUNC(aic580_device::fifo_data_r), FUNC(aic580_device::fifo_data_w));
	map(0xc0, 0xcf).rw(FUNC(aic580_device::buffer_r), FUNC(aic580_device::buffer_w));
}
