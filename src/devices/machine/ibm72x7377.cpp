// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/*************************************************************

  IBM 72X7377
  PS/2 Type 1 DMA Controller

**************************************************************/

#include "emu.h"
#include "ibm72x7377.h"

//#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(IBM72X7377, ibm72x7377_device, "ibm72x7377", "IBM 72X7377 PS/2 DMA Controller")

ibm72x7377_device::ibm72x7377_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IBM72X7377, tag, owner, clock),
    m_maincpu(*this, ":maincpu"),
    m_mcabus(*this, ":mb:mcabus"),
    m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2")
{
}

void ibm72x7377_device::device_start()
{
	m_arbus_ch0 = 0;
	m_arbus_ch4 = 0;

    m_dma_high_byte = 0;
    m_dma_channel = -1;
    m_cur_eop = false;
	m_cur_eop2 = false;

    m_byte_pointer = 0;
	m_extended_function = 0;

}

void ibm72x7377_device::device_reset()
{
	m_arbus_ch0 = 0;
	m_arbus_ch4 = 0;

    m_dma_high_byte = 0;
    m_dma_channel = -1;
    m_cur_eop = false;
	m_cur_eop2 = false;

    m_byte_pointer = 0;
	m_extended_function = 0;
}

void ibm72x7377_device::device_config_complete()
{

}

void ibm72x7377_device::device_add_mconfig(machine_config &config)
{
    PS2_DMA(config, m_dma8237_1, 14.318181_MHz_XTAL / 3);
	m_dma8237_1->out_hreq_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq0_w));
	m_dma8237_1->out_eop_callback().set(FUNC(ibm72x7377_device::dma8237_1_out_eop));
	m_dma8237_1->in_memr_callback().set(FUNC(ibm72x7377_device::dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(ibm72x7377_device::dma_write_byte));
	m_dma8237_1->in_ior_callback<0>().set(FUNC(ibm72x7377_device::dma8237_0_dack_r));
	m_dma8237_1->in_ior_callback<1>().set(FUNC(ibm72x7377_device::dma8237_1_dack_r));
	m_dma8237_1->in_ior_callback<2>().set(FUNC(ibm72x7377_device::dma8237_2_dack_r));
	m_dma8237_1->in_ior_callback<3>().set(FUNC(ibm72x7377_device::dma8237_3_dack_r));
	m_dma8237_1->out_iow_callback<0>().set(FUNC(ibm72x7377_device::dma8237_0_dack_w));
	m_dma8237_1->out_iow_callback<1>().set(FUNC(ibm72x7377_device::dma8237_1_dack_w));
	m_dma8237_1->out_iow_callback<2>().set(FUNC(ibm72x7377_device::dma8237_2_dack_w));
	m_dma8237_1->out_iow_callback<3>().set(FUNC(ibm72x7377_device::dma8237_3_dack_w));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(ibm72x7377_device::dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(ibm72x7377_device::dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(ibm72x7377_device::dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(ibm72x7377_device::dack3_w));

	PS2_DMA(config, m_dma8237_2, 14.318181_MHz_XTAL / 3);
	m_dma8237_2->out_hreq_callback().set(FUNC(ibm72x7377_device::dma_hrq_changed));
	m_dma8237_2->out_eop_callback().set(FUNC(ibm72x7377_device::dma8237_2_out_eop));
	m_dma8237_2->in_memr_callback().set(FUNC(ibm72x7377_device::dma_read_word));
	m_dma8237_2->out_memw_callback().set(FUNC(ibm72x7377_device::dma_write_word));
	m_dma8237_2->in_ior_callback<1>().set(FUNC(ibm72x7377_device::dma8237_5_dack_r));
	m_dma8237_2->in_ior_callback<2>().set(FUNC(ibm72x7377_device::dma8237_6_dack_r));
	m_dma8237_2->in_ior_callback<3>().set(FUNC(ibm72x7377_device::dma8237_7_dack_r));
	m_dma8237_2->out_iow_callback<1>().set(FUNC(ibm72x7377_device::dma8237_5_dack_w));
	m_dma8237_2->out_iow_callback<2>().set(FUNC(ibm72x7377_device::dma8237_6_dack_w));
	m_dma8237_2->out_iow_callback<3>().set(FUNC(ibm72x7377_device::dma8237_7_dack_w));
	m_dma8237_2->out_dack_callback<0>().set(FUNC(ibm72x7377_device::dack4_w));
	m_dma8237_2->out_dack_callback<1>().set(FUNC(ibm72x7377_device::dack5_w));
	m_dma8237_2->out_dack_callback<2>().set(FUNC(ibm72x7377_device::dack6_w));
	m_dma8237_2->out_dack_callback<3>().set(FUNC(ibm72x7377_device::dack7_w));
}

uint8_t ibm72x7377_device::dma8237_0_dack_r() { return m_mcabus->dack_r(0); }
uint8_t ibm72x7377_device::dma8237_1_dack_r() { return m_mcabus->dack_r(1); }
uint8_t ibm72x7377_device::dma8237_2_dack_r() { return m_mcabus->dack_r(2); }
uint8_t ibm72x7377_device::dma8237_3_dack_r() { return m_mcabus->dack_r(3); }
uint8_t ibm72x7377_device::dma8237_5_dack_r() { uint16_t ret = m_mcabus->dack16_r(5); m_dma_high_byte = ret & 0xff00; return ret; }
uint8_t ibm72x7377_device::dma8237_6_dack_r() { uint16_t ret = m_mcabus->dack16_r(6); m_dma_high_byte = ret & 0xff00; return ret; }
uint8_t ibm72x7377_device::dma8237_7_dack_r() { uint16_t ret = m_mcabus->dack16_r(7); m_dma_high_byte = ret & 0xff00; return ret; }

void ibm72x7377_device::dma8237_0_dack_w(uint8_t data) { m_mcabus->dack_w(0, data); }
void ibm72x7377_device::dma8237_1_dack_w(uint8_t data) { m_mcabus->dack_w(1, data); }
void ibm72x7377_device::dma8237_2_dack_w(uint8_t data) { m_mcabus->dack_w(2, data); }
void ibm72x7377_device::dma8237_3_dack_w(uint8_t data) { m_mcabus->dack_w(3, data); }
void ibm72x7377_device::dma8237_5_dack_w(uint8_t data) { m_mcabus->dack16_w(5, m_dma_high_byte | data); }
void ibm72x7377_device::dma8237_6_dack_w(uint8_t data) { m_mcabus->dack16_w(6, m_dma_high_byte | data); }
void ibm72x7377_device::dma8237_7_dack_w(uint8_t data) { m_mcabus->dack16_w(7, m_dma_high_byte | data); }

void ibm72x7377_device::dack0_w(int state) { set_dma_channel(0, state); }
void ibm72x7377_device::dack1_w(int state) { set_dma_channel(1, state); }
void ibm72x7377_device::dack2_w(int state) { set_dma_channel(2, state); }
void ibm72x7377_device::dack3_w(int state) { set_dma_channel(3, state); }
void ibm72x7377_device::dack4_w(int state) { m_dma8237_1->hack_w(state ? 0 : 1); } // it's inverted
void ibm72x7377_device::dack5_w(int state) { set_dma_channel(5, state); }
void ibm72x7377_device::dack6_w(int state) { set_dma_channel(6, state); }
void ibm72x7377_device::dack7_w(int state) { set_dma_channel(7, state); }

void ibm72x7377_device::dma_request(int channel, bool state)
{
	// Bounce this to the correct DMA controller.

	LOG("%s: ch:%d state:%d\n", FUNCNAME, channel, state);

	switch(channel)
	{
		case 0: m_dma8237_1->dreq0_w(state); break;
		case 1: m_dma8237_1->dreq1_w(state); break;
		case 2: m_dma8237_1->dreq2_w(state); break;
		case 3: m_dma8237_1->dreq3_w(state); break;
		case 4: m_dma8237_2->dreq0_w(state); break;
		case 5: m_dma8237_2->dreq1_w(state); break;
		case 6: m_dma8237_2->dreq2_w(state); break;
		case 7: m_dma8237_2->dreq3_w(state); break;
	}
}

uint8_t ibm72x7377_device::dma_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	LOG("%s %06X\n", FUNCNAME, offset+page_offset);

	result = prog_space.read_byte(page_offset + offset);
	return result;
}

void ibm72x7377_device::dma_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	LOG("%s %06X %02X\n", FUNCNAME, offset, data);

	prog_space.write_byte(page_offset + offset, data);
}


uint8_t ibm72x7377_device::dma_read_word(offs_t offset)
{
	LOG("%s %d\n", FUNCNAME, offset);

	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xff00;

	return result & 0xff;
}


void ibm72x7377_device::dma_write_word(offs_t offset, uint8_t data)
{
	LOG("%s %d %d\n", FUNCNAME, offset, data);

	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}

void ibm72x7377_device::dma_hrq_changed(int state)
{
	LOG("%s %d\n", FUNCNAME, state);

	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w(state);
}

void ibm72x7377_device::dma8237_1_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;

	LOG("DMA1 EOP channel %d\n", m_dma_channel);

	if(m_dma_channel != -1)
	{
		m_mcabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
		
	}
		
}

void ibm72x7377_device::dma8237_2_out_eop(int state)
{
	m_cur_eop2 = state == ASSERT_LINE;

	LOG("DMA2 EOP channel %d\n", m_dma_channel);

	if(m_dma_channel != -1)
		m_mcabus->eop_w(m_dma_channel, m_cur_eop2 ? ASSERT_LINE : CLEAR_LINE );
}

void ibm72x7377_device::set_dma_channel(int channel, int state)
{
	LOG("%s %d %d\n", FUNCNAME, channel, state);

	if(!state) {
		m_dma_channel = channel;
		if(m_cur_eop)
			m_mcabus->eop_w(channel, ASSERT_LINE );
	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_eop)
			m_mcabus->eop_w(channel, CLEAR_LINE );
	}
}

uint8_t ibm72x7377_device::page8_r(offs_t offset)
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = m_dma_offset[BIT(offset, 3)][2];
		break;
	case 2:
		data = m_dma_offset[BIT(offset, 3)][3];
		break;
	case 3:
		data = m_dma_offset[BIT(offset, 3)][1];
		break;
	case 7:
		data = m_dma_offset[BIT(offset, 3)][0];
		break;
	}

	LOG("%s o:%02X d:%02X\n", FUNCNAME, offset, data);

	return data;
}


void ibm72x7377_device::page8_w(offs_t offset, uint8_t data)
{
	uint8_t dma_channel = -1;

	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		dma_channel = (BIT(offset, 3) ? 4 : 0) + 2;
		m_dma_offset[BIT(offset, 3)][2] = data; // channel 2 or 6
		break;
	case 2:
		dma_channel = (BIT(offset, 3) ? 4 : 0) + 3;
		m_dma_offset[BIT(offset, 3)][3] = data;	// channel 3 or 7
		break;
	case 3:
		dma_channel = (BIT(offset, 3) ? 4 : 0) + 1;
		m_dma_offset[BIT(offset, 3)][1] = data;	// channel 1 or 5
		break;
	case 7:
		dma_channel = (BIT(offset, 3) ? 4 : 0) + 0;
		m_dma_offset[BIT(offset, 3)][0] = data;	// channel 0 or 4
		break;
	}
	
	if(dma_channel != 4) LOG("%s o:%02X d:%02X (ch:%d)\n", FUNCNAME, offset, data, dma_channel);
}

uint8_t ibm72x7377_device::extended_function_register_r(offs_t offset)
{
	LOG("%s o:%02X\n", FUNCNAME, offset);

	return 0xFF; // write-only register
}

void ibm72x7377_device::extended_function_register_w(offs_t offset, uint8_t data)
{
	// All OUTs to this register reset the byte pointer.
	m_byte_pointer = 0;
	m_extended_function = data;

	LOG("PS/2 DMA: ext function %d channel %d\n", (data & 0xF0) >> 4, data & 0x07);
}

u32 ibm72x7377_device::get_address(uint8_t channel)
{
	offs_t address;
	
	switch(channel)
	{
		case 0: address = m_dma8237_1->get_address<0>(); break;
		case 1: address = m_dma8237_1->get_address<1>(); break;
		case 2: address = m_dma8237_1->get_address<2>(); break;
		case 3: address = m_dma8237_1->get_address<3>(); break;
		case 4: address = m_dma8237_2->get_address<0>(); break;
		case 5: address = m_dma8237_2->get_address<1>(); break;
		case 6: address = m_dma8237_2->get_address<2>(); break;
		case 7: address = m_dma8237_2->get_address<3>(); break;
		default: address = 0xFF; break; // Invalid channel?
	}

	return address;
}

u32 ibm72x7377_device::get_count(uint8_t channel)
{
	u32 count;
	
	switch(channel)
	{
		case 0: count = m_dma8237_1->get_count<0>(); break;
		case 1: count = m_dma8237_1->get_count<1>(); break;
		case 2: count = m_dma8237_1->get_count<2>(); break;
		case 3: count = m_dma8237_1->get_count<3>(); break;
		case 4: count = m_dma8237_2->get_count<0>(); break;
		case 5: count = m_dma8237_2->get_count<1>(); break;
		case 6: count = m_dma8237_2->get_count<2>(); break;
		case 7: count = m_dma8237_2->get_count<3>(); break;
		default: count = 0xFF; break; // Invalid channel?
	}

	return count;
}

void ibm72x7377_device::set_address(uint8_t channel, uint8_t data, uint8_t byte_pointer)
{
	// Get the current address.

	u32 shifted_data = data;
	shifted_data = data << (8 * byte_pointer);

	u32 address = get_address(channel);
	switch(byte_pointer)
	{
		case 0: address = (address & 0x00FFFF00) | shifted_data; break;
		case 1: address = (address & 0x00FF00FF) | shifted_data; break;
		case 2: address = (address & 0x0000FFFF) | shifted_data; break;
		default: LOG("%s: Illegal byte pointer %d\n", FUNCNAME, byte_pointer);
	}

	switch(channel)
	{
		case 0: m_dma8237_1->set_address<0>(address); break;
		case 1: m_dma8237_1->set_address<1>(address); break;
		case 2: m_dma8237_1->set_address<2>(address); break;
		case 3: m_dma8237_1->set_address<3>(address); break;
		case 4: m_dma8237_2->set_address<0>(address); break;
		case 5: m_dma8237_2->set_address<1>(address); break;
		case 6: m_dma8237_2->set_address<2>(address); break;
		case 7: m_dma8237_2->set_address<3>(address); break;
	}	
}

void ibm72x7377_device::set_count(uint8_t channel, uint8_t data, uint8_t byte_pointer)
{
	// Get the current count, shift our data into it, rewrite it.

	u32 shifted_data = data;
	shifted_data = data << (8 * byte_pointer);

	u32 count = get_count(channel);
	switch(byte_pointer)
	{
		case 0: count = (count & 0x0000FF00) | shifted_data; break;
		case 1: count = (count & 0x000000FF) | shifted_data; break;
		default: LOG("%s: Illegal byte pointer %d\n", FUNCNAME, byte_pointer);
	}

	switch(channel)
	{
		case 0: m_dma8237_1->set_count<0>(count); break;
		case 1: m_dma8237_1->set_count<1>(count); break;
		case 2: m_dma8237_1->set_count<2>(count); break;
		case 3: m_dma8237_1->set_count<3>(count); break;
		case 4: m_dma8237_2->set_count<0>(count); break;
		case 5: m_dma8237_2->set_count<1>(count); break;
		case 6: m_dma8237_2->set_count<2>(count); break;
		case 7: m_dma8237_2->set_count<3>(count); break;
	}
}

uint8_t ibm72x7377_device::extended_function_execute_r(offs_t offset)
{
	if(machine().side_effects_disabled()) return 0xFF;

	LOG("%s o:%02X\n", FUNCNAME, offset);

	// Extended Mode read.
	uint8_t func = (m_extended_function & 0xF0) >> 4;
	uint8_t channel = m_extended_function & 0x07;
	uint8_t retval = 0xFF;

	switch(func)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			{
				retval = (get_address(channel) >> (m_byte_pointer * 8)) & 0xFF;
				m_byte_pointer++;
				break;
			}
		case 3:
		case 4:
			{
				retval = (get_count(channel) >> (m_byte_pointer * 8)) & 0xFF;
				m_byte_pointer++;
				break;
			}
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			switch(channel)
			{
				case 0: retval = m_arbus_ch0; break;
				case 4: retval = m_arbus_ch4; break;
				default: LOG("DMA Arbus read invalid channel %02X\n", channel);
			}
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		case 13:
			break;	
		case 14:
			break;
		case 15:
			break;
		default:
			LOG("Unhandled DMA Extended Function Read %02X\n", m_extended_function);
	}

	LOG("%s returning %02X\n", FUNCNAME, retval);

	return retval;
}
void ibm72x7377_device::extended_function_execute_w(offs_t offset, uint8_t data)
{
	// Extended Mode write.
	LOG("%s o:%02X\n", FUNCNAME, offset);

	uint8_t func = (m_extended_function & 0xF0) >> 4;
	uint8_t channel = m_extended_function & 0x07;

	switch(func)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			set_address(channel, data, m_byte_pointer);
			m_byte_pointer++;
			break;
		case 3:
			break;
		case 4:
			set_count(channel, data, m_byte_pointer);
			m_byte_pointer++;
			break;
		case 5:
			break;
		case 6:	
			break;
		case 7:
			break;
		case 8:
			switch(channel)
			{
				case 0: m_arbus_ch0 = data; break;
				case 4: m_arbus_ch4 = data; break;
				default: break;
			}
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		case 13:
			break;
		case 14:
			break;
		case 15:
			break;
		default:
			LOG("Unhandled DMA Extended Function Write %02X\n", m_extended_function);
			break;
	}
}

uint8_t ibm72x7377_device::dma_arbiter_r(offs_t offset)
{
	//LOG("%s o:%02X\n", FUNCNAME, offset);
	return 0xFF;
}

void ibm72x7377_device::dma_arbiter_w(offs_t offset, uint8_t data)
{
	//LOG("%s o:%02X d:%02X\n", FUNCNAME, offset, data);
}

void ibm72x7377_device::dma_feedback_w(offs_t offset, uint8_t data)
{
	//LOG("%s o:%02X d:%02X\n", FUNCNAME, offset, data);
}

uint8_t ibm72x7377_device::dmac1_r(offs_t offset)
{
	return m_dma8237_1->read(offset);
}

void ibm72x7377_device::dmac1_w(offs_t offset, uint8_t data)
{
	m_dma8237_1->write(offset, data);
}

uint8_t ibm72x7377_device::dmac2_r(offs_t offset)
{
	return m_dma8237_2->read(offset);
}

void ibm72x7377_device::dmac2_w(offs_t offset, uint8_t data)
{
	m_dma8237_2->write(offset, data);
}