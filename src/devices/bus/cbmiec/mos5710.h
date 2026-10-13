// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS5710 Custom Floppy Controller and Gate Array

    Used in 1571CR

**********************************************************************
                            _____   _____
               SRQ OUT   1 |*    \_/     | 48  *DATA OUT
                   TED   2 |             | 47  SRQ IN
                  phi0   3 |             | 46  *DATA IN
                    CK   4 |             | 45  *IRQ
                 *ACCL   5 |             | 44  *RESET
              VIA1 PA5   6 |             | 43
                  phi2   7 |             | 42
                    D7   8 |             | 41  *INDEX
                    D6   9 |             | 40  WG
                    D5  10 |             | 39  *WPRT
                    D4  11 |             | 38  *RD
                   Vss  12 |   MOS5710   | 37  WD
                   Vcc  13 |             | 36  Vcc
                    D3  14 |             | 35  Vss
                    D2  15 |             | 34  *VIA1
                    D1  16 |             | 33  *RAM
                    D0  17 |             | 32  *VIA2
                   A15  18 |             | 31  R/W
                   A14  19 |             | 30  16MHz
                   A13  20 |             | 29  XTL1
                   A12  21 |             | 28  XTL2
                   A10  22 |             | 27  A0
                    A4  23 |             | 26  A1
                    A3  24 |_____________| 25  A2

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MOS5710_H
#define MAME_BUS_CBMIEC_MOS5710_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/fdc_pll.h"

class mos5710_device : public device_t
{
public:
	mos5710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto sp_wr_callback() { return m_write_sp.bind(); }
	auto cnt_wr_callback() { return m_write_cnt.bind(); }

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t cia_r(offs_t offset);
	void cia_w(offs_t offset, uint8_t data);
	uint8_t fdc2_r(offs_t offset);
	void fdc2_w(offs_t offset, uint8_t data);

	void sp_w(int state);
	void cnt_w(int state);

	void set_floppy(floppy_image_device *floppy);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

private:
	enum
	{
		CTRL0_WG = 0x02,
		CTRL0_MARK = 0x04,
		CTRL0_DATA = 0x08,
		CTRL0_CRC = 0x10
	};

	enum
	{
		CTRL1_CLOCK_MASK = 0x07,
		CTRL1_CRC_CHECK = 0x08,
		CTRL1_READ = 0x10
	};

	enum
	{
		ICR_SP = 0x08
	};

	TIMER_CALLBACK_MEMBER(live_tick);
	TIMER_CALLBACK_MEMBER(sp_tick);

	void update_irq();
	void set_sp_out(int state);
	void set_cnt_out(int state);
	void sp_start();
	void sp_stop();

	uint16_t sync_pattern() const;
	static uint16_t crc_byte(uint16_t crc, uint8_t data);
	void live_start_read();
	void live_start_write();
	void live_stop_write();
	void live_run(const attotime &limit);
	void live_sync();
	void live_schedule();
	void write_load_byte();
	void read_deliver(uint8_t data, bool sync);

	devcb_write_line m_write_irq;
	devcb_write_line m_write_sp;
	devcb_write_line m_write_cnt;

	emu_timer *m_live_timer;
	emu_timer *m_sp_timer;

	floppy_image_device *m_floppy;
	fdc_pll_t m_pll;

	uint8_t m_fdc2[8];

	uint8_t m_ctrl0;
	uint8_t m_ctrl1;
	uint8_t m_sync_clock;
	uint8_t m_sync_data;
	uint8_t m_reg5;

	attotime m_tm;
	bool m_writing;
	bool m_reading;
	bool m_synced;
	uint16_t m_shift;
	int m_bits;
	uint16_t m_crc;

	uint8_t m_rdata;
	bool m_rbr;
	bool m_rsync;
	bool m_crc_error;
	bool m_crc_checked;

	uint8_t m_wdata;
	bool m_wbr;
	bool m_context;

	uint8_t m_sdr;
	bool m_sdr_full;
	uint8_t m_sp_shift;
	int m_sp_bits;
	bool m_sp_shifting;
	uint8_t m_icr;
	uint8_t m_imr;
	uint8_t m_cra;
	int m_sp_in;
	int m_cnt_in;
	int m_sp_out;
	int m_cnt_out;
	bool m_irq;
};

// device type definition
DECLARE_DEVICE_TYPE(MOS5710, mos5710_device)

#endif // MAME_BUS_CBMIEC_MOS5710_H
