// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  Fairchild F3853 SMI, F3851 PSU, F3856 PSU, F38T56 PSU

*/

#ifndef MAME_MACHINE_F3853_H
#define MAME_MACHINE_F3853_H

#pragma once

// pinout reference

/*
                  _____   _____
         Vgg   1 |*    \_/     | 40  Vdd
         PHI   2 |             | 39  ROMC4
       WRITE   3 |             | 38  ROMC3
    _INT REQ   4 |             | 37  ROMC2
     _PRI IN   5 |             | 36  ROMC1
  _RAM WRITE   6 |             | 35  ROMC0
    _EXT INT   7 |             | 34  CPU READ
       ADDR7   8 |             | 33  REG DR
       ADDR6   9 |             | 32  ADDR15
       ADDR5  10 |    F3853    | 31  ADDR14
       ADDR4  11 |             | 30  ADDR13
       ADDR3  12 |             | 29  ADDR12
       ADDR2  13 |             | 28  ADDR11
       ADDR1  14 |             | 27  ADDR10
       ADDR0  15 |             | 26  ADDR9
         DB0  16 |             | 25  ADDR8
         DB1  17 |             | 24  DB7
         DB2  18 |             | 23  DB6
         DB3  19 |             | 22  DB5
         Vss  20 |_____________| 21  DB4

                  _____   _____
     _I/O B7   1 |*    \_/     | 40  DB7
     _I/O A7   2 |             | 39  DB6
         Vgg   3 |             | 38  _I/O B6
         Vdd   4 |             | 37  _I/O A6
    _EXT INT   5 |             | 36  _I/O A5
    _PRI OUT   6 |             | 35  _I/O B5
       WRITE   7 |             | 34  DB5
         PHI   8 |             | 33  DB4
    _INT REQ   9 |    F3851    | 32  _I/O B4
     _PRI IN  10 |    F3856    | 31  _I/O A4
       _DBDR  11 |    F38T56   | 30  _I/O A3
      STROBE  12 |             | 29  _I/O B3
       ROMC4  13 |             | 28  DB3
       ROMC3  14 |             | 27  DB2
       ROMC2  15 |             | 26  _I/O B2
       ROMC1  16 |             | 25  _I/O A2
       ROMC0  17 |             | 24  _I/O A1
         Vss  18 |             | 23  _I/O B1
     _I/O A0  19 |             | 22  DB1
     _I/O B0  20 |_____________| 21  DB0

     F38T56 is internal in F3870
     note: STROBE is N/C on F3851
*/


class f3853_device : public device_t
{
public:
	// construction/destruction
	f3853_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_req_callback() { return m_int_req_callback.bind(); }
	auto pri_out_callback() { return m_pri_out_callback.bind(); }
	template <typename... T> void set_int_daisy_chain_callback(T &&... args) { m_int_daisy_chain_callback.set(std::forward<T>(args)...); }

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	void ext_int_w(int state);
	void pri_in_w(int state);

	virtual TIMER_CALLBACK_MEMBER(timer_callback);

	IRQ_CALLBACK_MEMBER(int_acknowledge);

protected:
	f3853_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint16_t timer_interrupt_vector() const { return m_int_vector & ~uint16_t(0x0080); }
	uint16_t external_interrupt_vector() const { return m_int_vector | uint16_t(0x0080); }

	void set_interrupt_request_line();
	virtual void timer_start(uint8_t value);

	devcb_write_line m_int_req_callback;
	devcb_write_line m_pri_out_callback;
	device_irq_acknowledge_delegate m_int_daisy_chain_callback;

	uint16_t m_int_vector; // Bit 7 is set to 0 for timer interrupts, 1 for external interrupts
	u8 m_prescaler;
	bool m_external_int_enable;
	bool m_timer_int_enable;

	bool m_request_flipflop;

	bool m_priority_line;              /* inverted level*/
	bool m_external_interrupt_line;    /* inverted level */

	emu_timer *m_timer;

	uint8_t m_value_to_cycle[0x100];
};

class f3851_device : public f3853_device
{
public:
	f3851_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interrupt vector is a mask option on 3851 and 3856
	void set_int_vector(u16 vector) { m_int_vector = vector; }

	// bidirectional I/O ports A and B
	auto read_a() { return m_read_port[0].bind(); }
	auto read_b() { return m_read_port[1].bind(); }
	auto write_a() { return m_write_port[0].bind(); }
	auto write_b() { return m_write_port[1].bind(); }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	f3851_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	devcb_read8::array<2> m_read_port;
	devcb_write8::array<2> m_write_port;
};

class f3856_device : public f3851_device
{
public:
	f3856_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	virtual TIMER_CALLBACK_MEMBER(timer_callback) override;

protected:
	f3856_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	virtual void timer_start(uint8_t value) override;

	u8 m_timer_count;
	u8 m_timer_modulo;
	bool m_timer_start;
};

class f38t56_device : public f3856_device
{
public:
	f38t56_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};

// device type definition
DECLARE_DEVICE_TYPE(F3853, f3853_device)
DECLARE_DEVICE_TYPE(F3851, f3851_device)
DECLARE_DEVICE_TYPE(F3856, f3856_device)
DECLARE_DEVICE_TYPE(F38T56, f38t56_device)

#endif // MAME_MACHINE_F3853_H
