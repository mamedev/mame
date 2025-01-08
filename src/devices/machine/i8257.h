// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Intel 8257 DMA Controller emulation

****************************************************************************
                            _____   _____
                 _I/OR   1 |*    \_/     | 40  A7
                 _I/OW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                  MARK   5 |             | 36  TC
                 READY   6 |             | 35  A3
                  HLDA   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                   HRQ  10 |     8257    | 31  Vcc
                   _CS  11 |             | 30  D0
                   CLK  12 |             | 29  D1
                 RESET  13 |             | 28  D2
                _DACK2  14 |             | 27  D3
                _DACK3  15 |             | 26  D4
                  DRQ3  16 |             | 25  _DACK0
                  DRQ2  17 |             | 24  _DACK1
                  DRQ1  18 |             | 23  D5
                  DRQ0  19 |             | 22  D6
                   GND  20 |_____________| 21  D7

***************************************************************************/

#ifndef MAME_MACHINE_I8257_H
#define MAME_MACHINE_I8257_H

#pragma once

class i8257_device : public device_t, public device_execute_interface
{
public:
	// construction/destruction
	i8257_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void hlda_w(int state);
	void ready_w(int state);

	void dreq0_w(int state);
	void dreq1_w(int state);
	void dreq2_w(int state);
	void dreq3_w(int state);

	auto out_hrq_cb() { return m_out_hrq_cb.bind(); }
	auto out_tc_cb() { return m_out_tc_cb.bind(); }
	auto in_memr_cb() { return m_in_memr_cb.bind(); }
	auto out_memw_cb() { return m_out_memw_cb.bind(); }
	template <unsigned Ch> auto in_ior_cb() { return m_in_ior_cb[Ch].bind(); }
	template <unsigned Ch> auto out_iow_cb() { return m_out_iow_cb[Ch].bind(); }
	template <unsigned Ch> auto out_dack_cb() { return m_out_dack_cb[Ch].bind(); }

	// This should be set for systems that map the DMAC registers into the memory space rather than as I/O ports (e.g. radio86)
	void set_reverse_rw_mode(bool flag) { m_reverse_rw = flag; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	int m_icount;

private:
	inline void dma_request(int channel, int state);
	inline bool is_request_active(int channel) const;
	inline void set_hreq(int state);
	inline void set_dack();
	inline void dma_read();
	inline void dma_write();
	inline void advance();
	inline void set_tc(int state);
	bool next_channel();

	bool m_reverse_rw;
	bool m_tc;
	int m_msb;
	int m_hreq;
	int m_hack;
	int m_ready;
	int m_state;
	int m_current_channel;
	int m_last_channel;
	uint8_t m_transfer_mode;
	uint8_t m_status;
	uint8_t m_request;
	uint8_t m_temp;

	devcb_write_line   m_out_hrq_cb;
	devcb_write_line   m_out_tc_cb;

	// accessors to main memory
	devcb_read8        m_in_memr_cb;
	devcb_write8       m_out_memw_cb;

	// channel accessors
	devcb_read8::array<4> m_in_ior_cb;
	devcb_write8::array<4> m_out_iow_cb;
	devcb_write_line::array<4> m_out_dack_cb;

	struct
	{
		uint16_t m_address;
		uint16_t m_count;
		uint8_t m_mode;
	} m_channel[4];
};


// device type definition
DECLARE_DEVICE_TYPE(I8257, i8257_device)

#endif // MAME_MACHINE_I8257_H
