// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    awacs.c

    AWACS/Singer style 16-bit audio I/O for '040 and PowerPC Macs

    Emulation by R. Belmont

***************************************************************************/

#include "emu.h"
#include "awacs.h"

// device type definition
DEFINE_DEVICE_TYPE(AWACS, awacs_device, "awacs", "AWACS")

const u8 awacs_device::divider[4] = { 16, 12, 8, 16 };


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  awacs_device - constructor
//-------------------------------------------------

awacs_device::awacs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AWACS, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_out_cb(*this)
	, m_irq_in_cb(*this)
	, m_output_cb(*this, 0)
	, m_input_cb(*this)
	, m_input_port_cb(*this, 0)
	, m_output_port_cb(*this)
	, m_stream(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void awacs_device::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 2, clock()/64/divider[0], STREAM_SYNCHRONOUS);

	m_last_sample = attotime::zero;

	save_item(NAME(m_extend));
	save_item(NAME(m_ext_command));
	save_item(NAME(m_ext_address));
	save_item(NAME(m_ext_data));
	save_item(NAME(m_codec0));
	save_item(NAME(m_codec1));
	save_item(NAME(m_codec2));
	save_item(NAME(m_ctrl0));
	save_item(NAME(m_ctrl1));
	save_item(NAME(m_out_irq));
	save_item(NAME(m_in_irq));
	save_item(NAME(m_output_buffer));
	save_item(NAME(m_input_buffer));
	save_item(NAME(m_phase));
	save_item(NAME(m_buffer_size));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void awacs_device::device_reset()
{
	m_extend = false;
	m_ext_command = false;
	m_ext_address = 0;
	m_ext_data = 0;
	m_codec0 = 0;
	m_codec1 = 0;
	m_codec2 = 0;
	m_ctrl0 = 0;
	m_ctrl1 = 0;
	m_out_irq = 0;
	m_in_irq = 0;
	m_output_buffer = false;
	m_input_buffer = false;
	m_phase = 0;
	m_buffer_size = 0;

	m_irq_out_cb(false);
	m_irq_in_cb(false);

	m_stream->set_sample_rate(clock()/64/divider[(m_ctrl0 >> 1) & 3]);
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void awacs_device::sound_stream_update(sound_stream &stream)
{
	m_last_sample = machine().time();

	if(m_phase == 0) {
		m_active = 0;
		if(m_ctrl0 & 1)
			m_active |= ACTIVE_OUT;
		if(m_ctrl1 & 0x80)
			m_active |= ACTIVE_IN;
	}

	if(m_active & ACTIVE_OUT) {
		u32 data = m_output_cb(m_phase | (m_output_buffer ? 0x10000 : 0));
		s16 left = data >> 16;
		s16 right = data;
		stream.put_int(0, 0, left, 32768);
		stream.put_int(1, 0, right, 32768);

	} else {
		m_output_buffer = false;

		stream.put_int(0, 0, 0, 32768);
		stream.put_int(1, 0, 0, 32768);
	}

	if(m_active & ACTIVE_IN)
		m_input_cb(m_phase | (m_input_buffer ? 0x10000 : 0), 0);
	else
		m_input_buffer = false;

	m_phase = (m_phase + 1) & 0xfff;
	if(m_phase >= m_buffer_size) {
		m_phase = 0;

		if(m_active & ACTIVE_OUT) {
			if(m_output_buffer) {
				m_output_buffer = false;
				if(m_out_irq & 0x40)
					m_out_irq |= 0x20;
				else
					m_out_irq |= 0x40;
			} else {
				m_output_buffer = true;
				if(m_out_irq & 0x80)
					m_out_irq |= 0x20;
				else
					m_out_irq |= 0x80;
			}
		}

		if(m_active & ACTIVE_IN) {
			if(m_input_buffer) {
				m_input_buffer = false;
				if(m_in_irq & 0x40)
					m_in_irq |= 0x20;
				else
					m_in_irq |= 0x40;
			} else {
				m_input_buffer = true;
				if(m_in_irq & 0x80)
					m_in_irq |= 0x20;
				else
					m_in_irq |= 0x80;
			}
		}
		update_irq();
	}
}

void awacs_device::update_irq()
{
	m_irq_out_cb(((m_out_irq >> 4) & m_out_irq) != 0);
	m_irq_in_cb(((m_in_irq >> 4) & m_in_irq) != 0);
}

//-------------------------------------------------
//  read - read from the chip's registers
//-------------------------------------------------

uint8_t awacs_device::read(offs_t offset)
{
	switch(offset) {
	case 0x00:
		if(m_extend)
			return 0x80 | (m_ext_command ? 0x40 : 0x00) | (m_ext_address >> 4);
		else
			return m_codec0;

	case 0x01:
		if(m_extend)
			return ((m_ext_address & 0xf) << 4) | ((m_ext_data & 0xf00) >> 8);
		else
			return m_codec1;

	case 0x02:
		if(m_extend)
			return m_ext_data & 0x0ff;
		else
			return m_codec2;

	case 0x06: return m_input_port_cb() & 0xf;

	case 0x0c: {
		u64 cycles = (machine().time() - m_last_sample).as_ticks(clock())/divider[(m_ctrl0 >> 1) & 3];
		if(cycles > 63)
			cycles = 63;
		return cycles | ((m_phase & 3) << 6);
	}

	case 0x0d: return m_phase >> 2;
	case 0x0e: return m_phase >> 10;

	case 0x10: return m_ctrl0;
	case 0x11: return m_ctrl1;

	case 0x14: return m_in_irq;
	case 0x18: return m_out_irq;

	default:
		logerror("reg_r %02x\n", offset);
		return 0;
	}
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

void awacs_device::write(offs_t offset, uint8_t data)
{
	switch (offset) {
	case 0x00:
		m_extend = data & 0x80;
		if(m_extend) {
			m_ext_command = data & 0x40;
			m_ext_address = (m_ext_address & 0xf) | ((data & 0x1f) << 4);
			logerror("extended command=%d adr=%03x data=%03x\n", m_ext_command, m_ext_address, m_ext_data);
		} else {
			m_codec0 = data & 0x7f;
			logerror("codec mute=%s iml=%x imr=%x gain_l=%x\n",
					 m_codec0 & 0x40 ? "on" : "off",
					 m_codec0 & 0x20 ? "on" : "off",
					 m_codec0 & 0x10 ? "on" : "off",
					 m_codec0 & 0xf);
		}
		break;

	case 0x01:
		if(m_extend) {
			m_ext_address = (m_ext_address & 0x3f0) | ((data & 0xf0) >> 4);
			m_ext_data = (m_ext_data & 0x0ff) | ((data & 0xf) << 8);
			logerror("extended command=%d adr=%03x data=%03x\n", m_ext_command, m_ext_address, m_ext_data);
		} else {
			m_codec1 = data;
			logerror("codec gain_r=%x att_l=%x\n",
					 (m_codec1 & 0xf0) >> 4,
					 m_codec1 & 0xf);
		}
		break;

	case 0x02:
		if(m_extend) {
			m_ext_data = (m_ext_data & 0xf00) | data;
			logerror("extended command=%d adr=%03x data=%03x\n", m_ext_command, m_ext_address, m_ext_data);
		} else {
			m_codec2 = data;
			m_output_port_cb(data & 0xf);
			logerror("codec att_r=%x output_port=%x\n",
					 (m_codec2 & 0xf0) >> 4,
					 m_codec2 & 0xf);
		}
		break;

	case 0x08:
		m_buffer_size = (m_buffer_size & 0xfc) | ((data & 7) << 8);
		logerror("buffer size %03x\n", m_buffer_size);
		break;

	case 0x09:
		m_buffer_size = (m_buffer_size & 0x700) | (data & 0xfc);
		logerror("buffer size %03x\n", m_buffer_size);
		break;

	case 0x10:
		m_ctrl0 = data;
		m_stream->set_sample_rate(clock()/64/divider[(m_ctrl0 >> 1) & 3]);

		logerror("ctr0_w %02x - play=%s rate=%d\n", m_ctrl0, m_ctrl0 & 1 ? "on" : "off", clock()/64/divider[(m_ctrl0 >> 1) & 3]);
		break;

	case 0x11:
		m_ctrl1 = data;
		logerror("ctr1_w %02x - record=%s frame_irq=%s sf_out=%x sf_in=%x\n", m_ctrl1,
				 m_ctrl1 & 0x80 ? "on" : "off",
				 m_ctrl1 & 0x40 ? "on" : "off",
				 (m_ctrl1 >> 2) & 0xf,
				 m_ctrl1 & 3);
		break;


	case 0x14:
		m_in_irq = ((m_in_irq & 0xf0) & (~data)) | (data & 0x0f);
		logerror("input irq if1=%d if0=%d cerr=%d ovr=%d ie1=%d ie0=%d oe=%d, ce=%d\n",
				 BIT(m_in_irq, 7),
				 BIT(m_in_irq, 6),
				 BIT(m_in_irq, 5),
				 BIT(m_in_irq, 4),
				 BIT(m_in_irq, 3),
				 BIT(m_in_irq, 2),
				 BIT(m_in_irq, 1),
				 BIT(m_in_irq, 0));
		update_irq();
		break;

	case 0x18:
		m_out_irq = ((m_out_irq & 0xe0) & (~data)) | (data & 0x0e);
		logerror("output irq if1=%d if0=%d und=%d ie1=%d ie0=%d ue=%d\n",
				 BIT(m_out_irq, 7),
				 BIT(m_out_irq, 6),
				 BIT(m_out_irq, 5),
				 BIT(m_out_irq, 3),
				 BIT(m_out_irq, 2),
				 BIT(m_out_irq, 1));
		update_irq();
		break;

	default:
		logerror("reg_w %02x, %02x\n", offset, data);
		break;
	}
}
