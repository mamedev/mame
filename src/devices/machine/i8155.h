// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8155/8156 - 2048-Bit Static MOS RAM with I/O Ports and Timer

**********************************************************************
                            _____   _____
                   PC3   1 |*    \_/     | 40  Vcc
                   PC4   2 |             | 39  PC2
              TIMER IN   3 |             | 38  PC1
                 RESET   4 |             | 37  PC0
                   PC5   5 |             | 36  PB7
            _TIMER OUT   6 |             | 35  PB6
                 IO/_M   7 |             | 34  PB5
             CE or _CE   8 |             | 33  PB4
                   _RD   9 |             | 32  PB3
                   _WR  10 |    8155     | 31  PB2
                   ALE  11 |    8156     | 30  PB1
                   AD0  12 |             | 29  PB0
                   AD1  13 |             | 28  PA7
                   AD2  14 |             | 27  PA6
                   AD3  15 |             | 26  PA5
                   AD4  16 |             | 25  PA4
                   AD5  17 |             | 24  PA3
                   AD6  18 |             | 23  PA2
                   AD7  19 |             | 22  PA1
                   Vss  20 |_____________| 21  PA0

**********************************************************************/

#ifndef MAME_MACHINE_I8155_H
#define MAME_MACHINE_I8155_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i8155_device

class i8155_device : public device_t
{
public:
	// construction/destruction
	i8155_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto in_pa_callback()  { return m_in_pa_cb.bind(); }
	auto in_pb_callback()  { return m_in_pb_cb.bind(); }
	auto in_pc_callback()  { return m_in_pc_cb.bind(); }
	auto out_pa_callback() { return m_out_pa_cb.bind(); }
	auto out_pb_callback() { return m_out_pb_cb.bind(); }
	auto out_pc_callback() { return m_out_pc_cb.bind(); }
	auto out_to_callback() { return m_out_to_cb.bind(); }

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	uint8_t memory_r(offs_t offset);
	void memory_w(offs_t offset, uint8_t data);

	void ale_w(offs_t offset, uint8_t data);
	uint8_t data_r();
	void data_w(uint8_t data);

protected:
	i8155_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read8        m_in_pa_cb;
	devcb_read8        m_in_pb_cb;
	devcb_read8        m_in_pc_cb;

	devcb_write8       m_out_pa_cb;
	devcb_write8       m_out_pb_cb;
	devcb_write8       m_out_pc_cb;

	// this gets called for each change of the TIMER OUT pin (pin 6)
	devcb_write_line   m_out_to_cb;

	// CPU interface
	int m_io_m;                 // I/O or memory select
	uint8_t m_ad;               // address

	// registers
	uint8_t m_command;          // command register
	uint8_t m_status;           // status register
	uint8_t m_output[3];        // output latches

	// RAM
	std::unique_ptr<uint8_t[]> m_ram;

	// counter
	uint16_t m_count_length;    // count length register (assigned)
	uint16_t m_count_loaded;    // count length register (loaded)
	int m_to;                   // timer output
	bool m_count_even_phase;

	// timers
	emu_timer *m_timer;         // counter timer
	emu_timer *m_timer_tc;      // counter timer (for TC)

	const address_space_config      m_space_config;

	inline uint8_t get_timer_mode() const;
	inline uint16_t get_timer_count() const;
	inline void timer_output(int to);
	inline void timer_stop_count();
	inline void timer_reload_count();
	inline int get_port_mode(int port);
	inline uint8_t read_port(int port);
	inline void write_port(int port, uint8_t data);
	void write_command(uint8_t data);

	void register_w(int offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(timer_half_counted);
	TIMER_CALLBACK_MEMBER(timer_tc);
};


class i8156_device : public i8155_device
{
public:
	// construction/destruction
	i8156_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(I8155, i8155_device)
DECLARE_DEVICE_TYPE(I8156, i8156_device)


#endif
