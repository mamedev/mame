// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/**********************************************************************

    i3001.cpp

    Intel 3001 Microprogram Control Unit

**********************************************************************/

#include "emu.h"
#include "i3001.h"

// Device type definition
DEFINE_DEVICE_TYPE(I3001, i3001_device, "i3001", "Intel i3001 MCU")

i3001_device::i3001_device(const machine_config &mconfig , const char *tag , device_t *owner , uint32_t clock)
	: device_t(mconfig , I3001 , tag , owner , clock)
	, m_fo_handler(*this)
	, m_px_handler(*this)
	, m_sx_handler(*this)
{
}

void i3001_device::fc_w(uint8_t fc)
{
	m_fc = fc;

	switch (fc & 0b1100) {
	case 0:
		// FF0
		m_fo = false;
		break;

	case 0b0100:
		// FFC
		m_fo = m_carry;
		break;

	case 0b1000:
		// FFZ
		m_fo = m_zero;
		break;

	case 0b1100:
		// FF1
		m_fo = true;
		break;
	}
	if (!m_fo_handler.isnull()) {
		m_fo_handler(m_fo);
	}
}

WRITE_LINE_MEMBER(i3001_device::clk_w)
{
	update();
}

void i3001_device::device_start()
{
	m_fo_handler.resolve();
	m_px_handler.resolve();
	m_sx_handler.resolve();

	save_item(NAME(m_addr));
	save_item(NAME(m_pr));
	save_item(NAME(m_ac));
	save_item(NAME(m_fc));
	save_item(NAME(m_fi));
	save_item(NAME(m_carry));
	save_item(NAME(m_zero));
	save_item(NAME(m_flag));
}

void i3001_device::update()
{
	m_flag = m_fi;

	// Decode FC10
	switch (m_fc & 0b11) {
	case 0:
		// SCZ
		m_carry = m_zero = m_flag;
		break;

	case 0b01:
		// STZ
		m_zero = m_flag;
		break;

	case 0b10:
		// STC
		m_carry = m_flag;
		break;

	default:
		// HCZ
		break;
	}

	// Decode AC
	if ((m_ac & 0b1100000) == 0b0000000) {
		// JCC
		m_addr = pack_row_col(m_ac & 0b11111 , get_col(m_addr));
	} else if ((m_ac & 0b1110000) == 0b0100000) {
		// JZR
		m_addr = pack_row_col(0 , m_ac & 0b1111);
	} else if ((m_ac & 0b1110000) == 0b0110000) {
		// JCR
		m_addr = pack_row_col(get_row(m_addr) , m_ac & 0b1111);
	} else if ((m_ac & 0b1111000) == 0b1110000) {
		// JCE
		m_addr = pack_row_col((get_row(m_addr) & 0b11000) | (m_ac & 0b00111) , get_col(m_addr));
	} else if ((m_ac & 0b1110000) == 0b1000000) {
		// JFL
		m_addr = pack_row_col((get_row(m_addr) & 0b10000) | (m_ac & 0b01111) , (get_col(m_addr) & 0b1000) | 0b0010 | m_flag);
	} else if ((m_ac & 0b1111000) == 0b1010000) {
		// JCF
		m_addr = pack_row_col((get_row(m_addr) & 0b11000) | (m_ac & 0b00111) , (get_col(m_addr) & 0b1000) | 0b0010 | m_carry);
	} else if ((m_ac & 0b1111000) == 0b1011000) {
		// JZF
		m_addr = pack_row_col((get_row(m_addr) & 0b11000) | (m_ac & 0b00111) , (get_col(m_addr) & 0b1000) | 0b0010 | m_zero);
	} else if ((m_ac & 0b1111000) == 0b1100000) {
		// JPR
		m_addr = pack_row_col((get_row(m_addr) & 0b11000) | (m_ac & 0b00111) , m_pr);
	} else if ((m_ac & 0b1111000) == 0b1101000) {
		// JLL
		m_addr = pack_row_col((get_row(m_addr) & 0b11000) | (m_ac & 0b00111) , 0b0100 | (m_pr >> 2));
	} else if ((m_ac & 0b1111100) == 0b1111100) {
		// JRL
		m_addr = pack_row_col((get_row(m_addr) & 0b11000) | 0b00100 | (m_ac & 0b00011) , 0b1100 | (m_pr & 0b0011));
	} else {
		// JPX
		uint8_t px = !m_px_handler.isnull() ? m_px_handler() & 0b1111 : 0;
		m_pr = !m_sx_handler.isnull() ? m_sx_handler() & 0b1111 : 0;
		m_addr = pack_row_col((get_row(m_addr) & 0b11100) | (m_ac & 0b00011) , px);
	}
}
