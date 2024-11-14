// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Standard Microsystems Corp. COM52C50 Twinax Interface Circuit (TIC)

****************************************************************************
                           _____    _____
                   D2   1 |*    \__/     | 28  Vcc
                   D3   2 |              | 27  D1
                  /WR   3 |              | 26  D0
                /TXEN   4 |              | 25  /CS
                 /DTX   5 |              | 24  /RD
                   TX   6 |              | 23  A0
               TX DMA   7 |              | 22  A1
                   RX   8 |   COM52C50   | 21  A2
                   D4   9 |              | 20  /INT1
                   D5  10 |              | 19  /INT2
               /RESET  11 |              | 18  RX DMA
                XTAL2  12 |              | 17  CLKOUT
                XTAL1  13 |              | 16  D7
                  GND  14 |______________| 15  D6

***************************************************************************/

#ifndef MAME_MACHINE_COM52C50_H
#define MAME_MACHINE_COM52C50_H

#pragma once


// ======================> com52c50_device

class com52c50_device : public device_t
{
public:
	// device type constructor
	com52c50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto int1_callback() { return m_int1_callback.bind(); }
	auto int2_callback() { return m_int2_callback.bind(); }
	auto rx_dma_callback() { return m_rx_dma_callback.bind(); }
	auto tx_dma_callback() { return m_tx_dma_callback.bind(); }

	// bus interface
	u8 rx_buffer_r();
	void tx_buffer_w(u8 data);
	void tx_buffer_eom_w(u8 data);
	void zero_fill_w(u8 data);
	void int_mask_w(u8 data);
	void address_select_w(u8 data);
	u8 present_address_r();
	void present_address_w(u8 data);
	u8 int_status_r();
	u8 rx_status_r();
	u8 tx_status_r();
	void control_w(u8 data);
	void mode_w(u8 data);
	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// device callbacks
	devcb_write_line m_int1_callback;
	devcb_write_line m_int2_callback;
	devcb_write_line m_rx_dma_callback;
	devcb_write_line m_tx_dma_callback;

	// internal state
	u8 m_int_mask;
	u8 m_int_status;
	u8 m_zero_fill;
	u8 m_present_address;
	u8 m_rx_status;
	u8 m_tx_status;
	u8 m_control;
	u8 m_mode;
};

// device type declaration
DECLARE_DEVICE_TYPE(COM52C50, com52c50_device)

#endif // MAME_MACHINE_COM52C50_H
