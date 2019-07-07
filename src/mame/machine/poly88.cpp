// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Poly-88 machine by Miodrag Milanovic

        18/05/2009 Initial implementation

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "includes/poly88.h"


void poly88_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_USART:
		poly88_usart_timer_callback(ptr, param);
		break;
	case TIMER_KEYBOARD:
		keyboard_callback(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in poly88_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(poly88_state::poly88_usart_timer_callback)
{
	m_int_vector = 0xe7;
	m_maincpu->set_input_line(0, HOLD_LINE);
}

void poly88_state::baud_rate_w(uint8_t data)
{
	logerror("poly88_baud_rate_w %02x\n",data);
	m_brg->control_w(data & 15);

	// FIXME
	m_usart_timer->adjust(attotime::zero, 0, attotime::from_hz(300));
}

uint8_t poly88_state::row_number(uint8_t code) {
	if (BIT(code,0)) return 0;
	if (BIT(code,1)) return 1;
	if (BIT(code,2)) return 2;
	if (BIT(code,3)) return 3;
	if (BIT(code,4)) return 4;
	if (BIT(code,5)) return 5;
	if (BIT(code,6)) return 6;
	if (BIT(code,7)) return 7;
	return 0;
}

TIMER_CALLBACK_MEMBER(poly88_state::keyboard_callback)
{
	int i;
	uint8_t code;
	uint8_t key_code = 0;
	uint8_t shift = m_linec->read() & 0x02 ? 1 : 0;
	uint8_t ctrl =  m_linec->read() & 0x01 ? 1 : 0;

	for(i = 0; i < 7; i++)
	{
		switch ( i )
		{
			case 0: code = m_line0->read(); break;
			case 1: code = m_line1->read(); break;
			case 2: code = m_line2->read(); break;
			case 3: code = m_line3->read(); break;
			case 4: code = m_line4->read(); break;
			case 5: code = m_line5->read(); break;
			case 6: code = m_line6->read(); break;
			default: code = 0;
		}
		if (code != 0)
		{
			if (i==0 && shift==0) {
				key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
			}
			if (i==0 && shift==1) {
				key_code = 0x20 + row_number(code) + 8*i; // for shifted numbers
			}
			if (i==1 && shift==0) {
				if (row_number(code) < 4) {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i==1 && shift==1) {
				if (row_number(code) < 4) {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i>=2 && i<=4 && shift==1 && ctrl==0) {
				key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
			}
			if (i>=2 && i<=4 && shift==0 && ctrl==0) {
				key_code = 0x40 + row_number(code) + (i-2)*8; // for big letters
			}
			if (i>=2 && i<=4 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for CTRL + letters
			}
			if (i==5 && shift==1 && ctrl==0) {
				if (row_number(code)<7) {
					key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
				} else {
					key_code = 0x40 + row_number(code) + (i-2)*8; // for signs it is switched
				}
			}
			if (i==5 && shift==0 && ctrl==0) {
				if (row_number(code)<7) {
					key_code = 0x40 + row_number(code) + (i-2)*8; // for small letters
				} else {
					key_code = 0x60 + row_number(code) + (i-2)*8; // for signs it is switched
				}
			}
			if (i==5 && shift==0 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for letters + ctrl
			}
			if (i==6) {
				switch(row_number(code))
				{
					case 0: key_code = 0x11; break;
					case 1: key_code = 0x12; break;
					case 2: key_code = 0x13; break;
					case 3: key_code = 0x14; break;
					case 4: key_code = 0x20; break; // Space
					case 5: key_code = 0x0D; break; // Enter
					case 6: key_code = 0x09; break; // TAB
					case 7: key_code = 0x0A; break; // LF
				}
			}
		}
	}
	if (key_code==0 && m_last_code !=0){
		m_int_vector = 0xef;
		m_maincpu->set_input_line(0, HOLD_LINE);
	} else {
		m_last_code = key_code;
	}
}

IRQ_CALLBACK_MEMBER(poly88_state::poly88_irq_callback)
{
	return m_int_vector;
}

WRITE_LINE_MEMBER(poly88_state::write_cas_tx)
{
	m_cas_tx = state;
}

WRITE_LINE_MEMBER(poly88_state::cassette_txc_rxc_w)
{
	int data;
	int current_level;

//  if (!(ioport("DSW0")->read() & 0x02)) /* V.24 / Tape Switch */
	//{
		/* tape reading */
		if (m_cassette->get_state()&CASSETTE_PLAY)
		{
			if (m_clk_level_tape)
			{
				m_previous_level = (m_cassette->input() > 0.038) ? 1 : 0;
				m_clk_level_tape = 0;
			}
			else
			{
				current_level = (m_cassette->input() > 0.038) ? 1 : 0;

				if (m_previous_level!=current_level)
				{
					data = (!m_previous_level && current_level) ? 1 : 0;
//data = current_level;
					m_usart->write_rxd(data);

					m_clk_level_tape = 1;
				}
			}

			m_usart->write_rxc(m_clk_level_tape);
		}

		/* tape writing */
		if (m_cassette->get_state()&CASSETTE_RECORD)
		{
			data = m_cas_tx;
			data ^= m_clk_level_tape;
			m_cassette->output(data&0x01 ? 1 : -1);

			m_clk_level_tape = m_clk_level_tape ? 0 : 1;
			m_usart->write_txc(m_clk_level_tape);

			return;
		}

		m_clk_level_tape = 1;

		m_usart->write_txc(state);
//  }
}


void poly88_state::init_poly88()
{
	m_previous_level = 0;
	m_clk_level_tape = 1;

	m_usart_timer = timer_alloc(TIMER_USART);
	m_keyboard_timer = timer_alloc(TIMER_KEYBOARD);
	m_keyboard_timer->adjust(attotime::from_hz(24000), 0, attotime::from_hz(24000));
}

void poly88_state::machine_reset()
{
	m_intr = 0;
	m_last_code = 0;
	m_usart->write_cts(0);
	m_brg->control_w(0);
}

INTERRUPT_GEN_MEMBER(poly88_state::poly88_interrupt)
{
	m_int_vector = 0xf7;
	device.execute().set_input_line(0, HOLD_LINE);
}

WRITE_LINE_MEMBER(poly88_state::poly88_usart_rxready)
{
	//drvm_int_vector = 0xe7;
	//execute().set_input_line(0, HOLD_LINE);
}

uint8_t poly88_state::keyboard_r()
{
	uint8_t retVal = m_last_code;
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_last_code = 0x00;
	return retVal;
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
					memcpy(space.get_read_ptr(address ), data + pos ,recordLen);
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
