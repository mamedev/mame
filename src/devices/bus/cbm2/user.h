// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II User Port emulation

**********************************************************************

                    GND       1      14      2D0
                    PB2       2      15      1D7
                    GND       3      16      1D6
                    PB3       4      17      1D5
                     PC       5      18      1D4
                   FLAG       6      19      1D3
                    2D7       7      20      1D2
                    2D6       8      21      1D1
                    2D5       9      22      1D0
                    2D4      10      23      CNT
                    2D3      11      24      +5V
                    2D2      12      25      IRQ
                    2D1      13      26      SP

**********************************************************************/

#ifndef MAME_BUS_CBM2_USER_H
#define MAME_BUS_CBM2_USER_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cbm2_user_port_device;

// ======================> device_cbm2_user_port_interface

// class representing interface-specific live cbm2_expansion card
class device_cbm2_user_port_interface : public device_interface
{
public:
	virtual uint8_t cbm2_d1_r() { return 0xff; }
	virtual void cbm2_d1_w(uint8_t data) { }

	virtual uint8_t cbm2_d2_r() { return 0xff; }
	virtual void cbm2_d2_w(uint8_t data) { }

	virtual int cbm2_pb2_r() { return 1; }
	virtual void cbm2_pb2_w(int state) { }
	virtual int cbm2_pb3_r() { return 1; }
	virtual void cbm2_pb3_w(int state) { }

	virtual void cbm2_pc_w(int state) { }
	virtual void cbm2_cnt_w(int state) { }
	virtual void cbm2_sp_w(int state) { }

protected:
	// construction/destruction
	device_cbm2_user_port_interface(const machine_config &mconfig, device_t &device);

	cbm2_user_port_device *m_slot;
};


// ======================> cbm2_user_port_device

class cbm2_user_port_device : public device_t, public device_single_card_slot_interface<device_cbm2_user_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	cbm2_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: cbm2_user_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	cbm2_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_write_irq.bind(); }
	auto sp_callback() { return m_write_sp.bind(); }
	auto cnt_callback() { return m_write_cnt.bind(); }
	auto flag_callback() { return m_write_flag.bind(); }

	// computer interface
	uint8_t d1_r() { uint8_t data = 0xff; if (m_card != nullptr) data = m_card->cbm2_d1_r(); return data; }
	void d1_w(uint8_t data) { if (m_card != nullptr) m_card->cbm2_d1_w(data); }
	uint8_t d2_r() { uint8_t data = 0xff; if (m_card != nullptr) data = m_card->cbm2_d2_r(); return data; }
	void d2_w(uint8_t data) { if (m_card != nullptr) m_card->cbm2_d2_w(data); }
	int pb2_r() { return m_card ? m_card->cbm2_pb2_r() : 1; }
	void pb2_w(int state) { if (m_card != nullptr) m_card->cbm2_pb2_w(state); }
	int pb3_r() { return m_card ? m_card->cbm2_pb3_r() : 1; }
	void pb3_w(int state) { if (m_card != nullptr) m_card->cbm2_pb3_w(state); }
	void pc_w(int state) { if (m_card != nullptr) m_card->cbm2_pc_w(state); }
	void cnt_w(int state) { if (m_card != nullptr) m_card->cbm2_cnt_w(state); }
	void sp_w(int state) { if (m_card != nullptr) m_card->cbm2_sp_w(state); }

	// cartridge interface
	void irq_w(int state) { m_write_irq(state); }
	void cia_sp_w(int state) { m_write_sp(state); }
	void cia_cnt_w(int state) { m_write_cnt(state); }
	void flag_w(int state) { m_write_flag(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_sp;
	devcb_write_line   m_write_cnt;
	devcb_write_line   m_write_flag;

	device_cbm2_user_port_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(CBM2_USER_PORT, cbm2_user_port_device)


// slot devices
void cbm2_user_port_cards(device_slot_interface &device);

#endif // MAME_BUS_CBM2_USER_H
