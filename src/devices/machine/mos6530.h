// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6530 MIOT, 6532 RIOT

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  PA1
                   PA0   2 |             | 39  PA2
                  phi2   3 |             | 38  PA3
                   RS0   4 |             | 37  PA4
                    A9   5 |             | 36  PA5
                    A8   6 |             | 35  PA6
                    A7   7 |             | 34  PA7
                    A6   8 |             | 33  DB0
                   R/W   9 |             | 32  DB1
                    A5  10 |   MCS6530   | 31  DB2
                    A4  11 |             | 30  DB3
                    A3  12 |             | 29  DB4
                    A2  13 |             | 28  DB5
                    A1  14 |             | 27  DB6
                    A0  15 |             | 26  DB7
                  _RES  16 |             | 25  PB0
               IRQ/PB7  17 |             | 24  PB1
               CS1/PB6  18 |             | 23  PB2
               CS2/PB5  19 |             | 22  PB3
                   Vcc  20 |_____________| 21  PB4

                            _____   _____
                   Vss   1 |*    \_/     | 40  A6
                    A5   2 |             | 39  phi2
                    A4   3 |             | 38  CS1
                    A3   4 |             | 37  _CS2
                    A2   5 |             | 36  _RS
                    A1   6 |             | 35  R/W
                    A0   7 |             | 34  _RES
                   PA0   8 |             | 33  D0
                   PA1   9 |             | 32  D1
                   PA2  10 |   MCS6532   | 31  D2
                   PA3  11 |             | 30  D3
                   PA4  12 |             | 29  D4
                   PA5  13 |             | 28  D5
                   PA6  14 |             | 27  D6
                   PA7  15 |             | 26  D7
                   PB7  16 |             | 25  _IRQ
                   PB6  17 |             | 24  PB0
                   PB5  18 |             | 23  PB1
                   PB4  19 |             | 22  PB2
                   Vcc  20 |_____________| 21  PB3

**********************************************************************/

#ifndef MAME_MACHINE_MOS6530_H
#define MAME_MACHINE_MOS6530_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6530_device_base

class mos6530_device_base : public device_t
{
public:
	// port byte callbacks
	auto pa_rd_callback() { return m_in8_pa_cb.bind(); }
	auto pa_wr_callback() { return m_out8_pa_cb.bind(); }
	auto pb_rd_callback() { return m_in8_pb_cb.bind(); }
	auto pb_wr_callback() { return m_out8_pb_cb.bind(); }

	// port bit callbacks
	template <unsigned N> auto pa_rd_callback() { return m_in_pa_cb[N].bind(); }
	template <unsigned N> auto pa_wr_callback() { return m_out_pa_cb[N].bind(); }
	template <unsigned N> auto pb_rd_callback() { return m_in_pb_cb[N].bind(); }
	template <unsigned N> auto pb_wr_callback() { return m_out_pb_cb[N].bind(); }

	// _IRQ pin (on 6530 it's shared with PB7)
	auto irq_wr_callback() { return m_irq_cb.bind(); }

	// write to port inputs (PA7 can trigger an IRQ, the others are normal inputs)
	void pa_w(offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pb_w(offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	template <unsigned N> void pa_bit_w(int state) { pa_w(0, (state & 1) << N, 1 << N); }
	template <unsigned N> void pb_bit_w(int state) { pb_w(0, (state & 1) << N, 1 << N); }

protected:
	// construction/destruction
	mos6530_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u32 rsize);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	enum
	{
		IRQ_EDGE  = 0x40,
		IRQ_TIMER = 0x80
	};

	enum
	{
		TIMER_COUNTING,
		TIMER_SPINNING
	};

	void update_pa();
	virtual void update_pb();
	virtual void update_irq();
	virtual uint8_t get_irq_flags();
	uint8_t get_timer();
	void timer_start(uint8_t data);
	TIMER_CALLBACK_MEMBER(timer_end);
	void edge_detect();

	void timer_w(offs_t offset, uint8_t data, bool ie);
	uint8_t timer_r(bool ie);

	uint8_t rom_r(offs_t offset) { return m_rom[offset]; }
	uint8_t ram_r(offs_t offset) { return m_ram[offset]; }
	void ram_w(offs_t offset, uint8_t data) { m_ram[offset] = data; }
	uint8_t pa_data_r();
	void pa_data_w(uint8_t data);
	uint8_t pb_data_r();
	void pb_data_w(uint8_t data);
	uint8_t pa_ddr_r() { return m_pa_ddr; }
	void pa_ddr_w(uint8_t data);
	uint8_t pb_ddr_r() { return m_pb_ddr; }
	void pb_ddr_w(uint8_t data);
	uint8_t timer_off_r();
	uint8_t timer_on_r();
	uint8_t irq_r();
	void timer_off_w(offs_t offset, uint8_t data);
	void timer_on_w(offs_t offset, uint8_t data);
	void edge_w(offs_t offset, uint8_t data);

	memory_share_creator<uint8_t> m_ram;
	optional_region_ptr<uint8_t> m_rom;

	devcb_write_line m_irq_cb;
	devcb_read8 m_in8_pa_cb;
	devcb_write8 m_out8_pa_cb;
	devcb_read8 m_in8_pb_cb;
	devcb_write8 m_out8_pb_cb;
	devcb_read_line::array<8> m_in_pa_cb;
	devcb_write_line::array<8> m_out_pa_cb;
	devcb_read_line::array<8> m_in_pb_cb;
	devcb_write_line::array<8> m_out_pb_cb;

	uint8_t m_pa_in;
	uint8_t m_pa_out;
	uint8_t m_pa_ddr;
	int m_pa7;
	int m_pa7_dir;

	uint8_t m_pb_in;
	uint8_t m_pb_out;
	uint8_t m_pb_ddr;

	bool m_ie_timer;
	bool m_irq_timer;
	bool m_ie_edge;
	bool m_irq_edge;

	uint8_t m_timershift;
	uint8_t m_timerstate;
	emu_timer *m_timer;
	attotime m_timeout;
};


class mos6530_device : public mos6530_device_base
{
public:
	// construction/destruction
	mos6530_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void rom_map(address_map &map) ATTR_COLD;
	virtual void ram_map(address_map &map) ATTR_COLD;
	virtual void io_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	void update_pb() override;
	void update_irq() override;
	uint8_t get_irq_flags() override;
};


class mos6532_device : public mos6530_device_base
{
public:
	// construction/destruction
	mos6532_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ram_map(address_map &map) ATTR_COLD;
	virtual void io_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MOS6530, mos6530_device)
DECLARE_DEVICE_TYPE(MOS6532, mos6532_device)

#endif // MAME_MACHINE_MOS6530_H
