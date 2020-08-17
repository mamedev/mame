// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Poly-88 machine by Miodrag Milanovic

        18/05/2009 Initial implementation

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/poly88.h"


bool poly88_state::is_onboard(offs_t offset)
{
	return (offset & 0xf000) == m_onboard_config->read() << 13;
}

uint8_t poly88_state::mem_r(offs_t offset)
{
	if (!m_onboard_disable && is_onboard(offset))
	{
		if ((offset & 0xfff) >= 0xc00)
			return m_onboard_ram[offset & 0x1ff]; // mirrored
		else
			return m_onboard_rom[offset & 0xfff];
	}
	else
		return m_s100->smemr_r(offset);
}

void poly88_state::mem_w(offs_t offset, uint8_t data)
{
	if (!m_onboard_disable && is_onboard(offset))
	{
		if ((offset & 0xfff) >= 0xc00)
			m_onboard_ram[offset & 0x1ff] = data; // mirrored
	}
	else
		m_s100->mwrt_w(offset, data);
}

uint8_t poly88_state::in_r(offs_t offset)
{
	offset |= offset << 8;
	if (is_onboard(offset))
		return m_onboard_io->read8(offset & 0x0f);
	else
		return m_s100->sinp_r(offset);
}

void poly88_state::out_w(offs_t offset, uint8_t data)
{
	offset |= offset << 8;
	if (is_onboard(offset))
		m_onboard_io->write8(offset & 0x0f, data);
	else
		m_s100->sout_w(offset, data);
}

// bits 0-3 baud rate; bit 4 (0=cassette, 1=rs232); bit 5 (1=disable rom and ram)
void poly88_state::baud_rate_w(uint8_t data)
{
	logerror("poly88_baud_rate_w %02x\n",data);
	m_brg->control_w(data & 15);

	m_onboard_disable = BIT(data, 5);
}

IRQ_CALLBACK_MEMBER(poly88_state::poly88_irq_callback)
{
	return m_int_vector;
}

TIMER_DEVICE_CALLBACK_MEMBER( poly88_state::kansas_r )
{
	if (BIT(m_linec->read(), 7))
	{
		// Polyphase LOAD
		if (m_dtr)
		{
			m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
			m_casspol = 0;
			return;
		}

		m_cass_data[1]++;
		m_cass_data[2]++;

		uint8_t cass_ws = (m_cassette->input() > +0.04) ? 1 : 0;

		if (cass_ws != m_cass_data[0])
		{
			m_cass_data[0] = cass_ws;
			if (m_cass_data[1] > 13)
				m_casspol ^= 1;
			m_cass_data[1] = 0;
			m_cass_data[2] = 0;
			m_usart->write_rxd(m_casspol);
		}
		if ((m_cass_data[2] & 7)==2)
		{
			m_cass_data[3]++;
			m_usart->write_rxc(BIT(m_cass_data[3], 0));
		}
	}
	else
	{
		// 300 baud LOAD
		// no tape - set uart to idle
		m_cass_data[1]++;
		if (m_dtr || (m_cass_data[1] > 32))
		{
			m_cass_data[1] = 32;
			m_rxd = 1;
		}

		// turn 1200/2400Hz to a bit
		uint8_t cass_ws = (m_cassette->input() > +0.04) ? 1 : 0;

		if (cass_ws != m_cass_data[0])
		{
			m_cass_data[0] = cass_ws;
			m_rxd = (m_cass_data[1] < 12) ? 1 : 0;
			m_cass_data[1] = 0;
		}
	}
}

WRITE_LINE_MEMBER(poly88_state::cassette_clock_w)
{
	// incoming @4800Hz (bit), 2400Hz (polyphase)
	if (BIT(m_linec->read(), 7))
	{
		// polyphase SAVE
		if (!m_rts)
			m_cassette->output((m_txd ^ state) ? -1.0 : 1.0);
	}
	else
	{
		// byte mode 300 baud Kansas City format SAVE
		u8 twobit = m_cass_data[4] & 15;

		if (m_rts && (twobit == 0))
		{
			m_cassette->output(0);
			m_cass_data[4] = 0;     // reset waveforms
		}
		else
		if (state)
		{
			if (twobit == 0)
				m_cassold = m_txd;

			if (m_cassold)
				m_cassette->output(BIT(m_cass_data[4], 0) ? -1.0 : +1.0); // 2400Hz
			else
				m_cassette->output(BIT(m_cass_data[4], 1) ? -1.0 : +1.0); // 1200Hz

			m_cass_data[4]++;
		}

		// byte mode 300 baud Kansas City format LOAD
		if (state && !m_dtr && (twobit == 0))
			m_usart->write_rxd(m_rxd);
	}

	if (!m_rts)
		m_usart->write_txc(state);

	if (!m_dtr && !BIT(m_linec->read(), 7))
		m_usart->write_rxc(state);
}


void poly88_state::machine_start()
{
	m_onboard_ram = make_unique_clear<u8[]>(0x200);
	save_pointer(NAME(m_onboard_ram), 0x200);
	save_item(NAME(m_int_vector));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
	save_item(NAME(m_cassold));
	save_item(NAME(m_casspol));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_onboard_disable));
}

void poly88_state::machine_reset()
{
	m_usart->write_rxd(1);
	m_usart->write_cts(0);
	m_brg->control_w(0);
	m_onboard_disable = false;
	m_dtr = m_rts = m_txd = m_rxd = m_cassold = 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(poly88_state::rtc_tick)
{
	m_int_vector = 0xf7;
	m_maincpu->set_input_line(0, HOLD_LINE);
}

WRITE_LINE_MEMBER(poly88_state::vi2_w)
{
	if (state == ASSERT_LINE)
	{
		m_int_vector = 0xef;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE_LINE_MEMBER(poly88_state::vi5_w)
{
	if (state == ASSERT_LINE)
	{
		m_int_vector = 0xd7;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE_LINE_MEMBER(poly88_state::usart_ready_w)
{
	if (state)
	{
		m_int_vector = 0xe7;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

void poly88_state::intr_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

SNAPSHOT_LOAD_MEMBER(poly88_state::snapshot_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t* data= auto_alloc_array(machine(), uint8_t, snapshot_size);
	uint16_t recordNum;
	uint16_t recordLen;
	uint16_t address;
	uint8_t  recordType;

	int pos = 0x300;
	char name[9];
	int i = 0;
	int theend = 0;

	image.fread( data, snapshot_size);

	while (pos<snapshot_size) {
		for(i=0;i<9;i++) {
			name[i] = (char) data[pos + i];
		}
		pos+=8;
		name[8] = 0;


		recordNum = data[pos]+ data[pos+1]*256; pos+=2;
		recordLen = data[pos]; pos++;
		if (recordLen==0) recordLen=0x100;
		address = data[pos] + data[pos+1]*256; pos+=2;
		recordType = data[pos]; pos++;

		logerror("Block :%s number:%d length: %d address=%04x type:%d\n",name,recordNum,recordLen,address, recordType);
		switch(recordType) {
			case 0 :
					/* 00 Absolute */
					for (uint16_t j = 0; j < recordLen; j++)
						space.write_byte(address + j, data[pos + j]);
					break;
			case 1 :
					/* 01 Comment */
					break;
			case 2 :
					/* 02 End */
					theend = 1;
					break;
			case 3 :
					/* 03 Auto Start @ Address */
					m_maincpu->set_state_int(i8080_cpu_device::I8085_PC, address);
					theend = 1;
					break;
			case 4 :
					/* 04 Data ( used by Assembler ) */
					logerror("ASM load unsupported\n");
					theend = 1;
					break;
			case 5 :
					/* 05 BASIC program file */
					logerror("BASIC load unsupported\n");
					theend = 1;
					break;
			case 6 :
					/* 06 End ( used by Assembler? ) */
					theend = 1;
					break;
			default: break;
		}

		if (theend) {
			break;
		}
		pos+=recordLen;
	}
	m_usart->reset();
	return image_init_result::PASS;
}
