// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
/*
MB89372
Fujitsu
Multi-Protocol Controller
                _____   _____
     READY   1 |*    \_/     | 64  Vcc
     M/IO#   2 |             | 63  CLK
       CS#   3 |             | 62  RESET
       RD#   4 |             | 61  HRQ
       WR#   5 |             | 60  HLDA
     ADSTB   6 |             | 59  INT
    A23/D7   7 |             | 58  INTA#
    A22/D6   8 |             | 57  SSEL#
    A21/D5   9 |             | 56  RxCB
    A20/D4  10 |             | 55  RxDB
    A19/D3  11 |             | 54  LOCB#
    A18/D2  12 |             | 53  TxDB
    A17/D1  13 |             | 52  TxCB#
    A16/D0  14 |             | 51  CTSB#
       Vcc  15 |             | 50  Vss
       A15  16 |   MB89372   | 49  SCLK
       A14  17 |             | 48  SCLKX
       A13  18 |             | 47  HALFRATE#
       A12  19 |             | 46  DCDB#
       A11  20 |             | 45  RTSB#
       A10  21 |             | 44  DSRB#/SYNDETB#
        A9  22 |             | 43  DTRB#
        A8  23 |             | 42  RxCA#
        A7  24 |             | 41  RxDA
        A6  25 |             | 40  LOCA#
        A5  26 |             | 39  TxDA
        A4  27 |             | 38  TxCA#
        A3  28 |             | 37  CTSA#
        A2  29 |             | 36  DCDA#
        A1  30 |             | 35  RTSA#
        A0  31 |             | 34  DSRA#/SYNDETA#
       Vss  32 |_____________| 33  DTRA#
*/

#ifndef MAME_MACHINE_MB89372_H
#define MAME_MACHINE_MB89372_H

#pragma once


class mb89372_device : public device_t,
					   public device_execute_interface
{
public:
	// construction/destruction
	mb89372_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_hreq_callback() { return m_out_hreq_cb.bind(); }
	auto out_irq_callback() { return m_out_irq_cb.bind(); }

	auto in_memr_callback() { return m_in_memr_cb.bind(); }
	auto out_memw_callback() { return m_out_memw_cb.bind(); }

	// read/write handlers
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void hack_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	int m_icount;

private:
	// internal state
	inline void set_hreq(int state);
	inline void set_irq(int state);

	// pins
	int m_hreq;
	int m_hack;
	int m_irq;

	// registers
	uint8_t m_reg[0x40];

	devcb_write_line m_out_hreq_cb;
	devcb_write_line m_out_irq_cb;

	devcb_read8      m_in_memr_cb;
	devcb_write8     m_out_memw_cb;

	// ints
	void check_ints();

	// dma
	struct
	{
		uint32_t m_address;
		uint32_t m_count;
		uint32_t m_base_address;
		uint32_t m_base_flags;
		uint8_t m_state;
	} m_channel[4];
	int m_current_channel;
	int m_last_channel;
	void check_dma();

	uint16_t m_dma_delay;
	uint16_t m_intr_delay;
	uint16_t m_sock_delay;

	// 512 byte fifos (not in hardware)
	uint8_t  m_rx_buffer[0x200];
	uint16_t m_rx_offset;
	uint16_t m_rx_length;

	uint8_t  m_tx_buffer[0x200];
	uint16_t m_tx_offset;

	class context;
	std::unique_ptr<context> m_context;

	uint8_t m_socket_buffer[0x200];

	void    rx_reset();
	uint8_t rx_read();

	void tx_reset();
	void tx_write(uint8_t data);
	void tx_complete();

	void comm_tick();
	unsigned read_frame();
	void send_frame(unsigned data_size);
};


// device type definition
DECLARE_DEVICE_TYPE(MB89372, mb89372_device)

#endif // MAME_MACHINE_MB89372_H
