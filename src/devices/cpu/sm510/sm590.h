// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
/*

  Sharp SM590 MCU family cores

*/

#ifndef MAME_CPU_SM510_SM590_H
#define MAME_CPU_SM510_SM590_H

#include "sm510base.h"


// pinout reference

/*

16-pin DIP SM590/591/595:
                ____    ____
              _|*   \__/    |_
    R0.0 <>  |_|1         16|_|  -- VDD(5v)
              _|            |_
    R0.1 <>  |_|2         15|_|  <> R2.2
              _|            |_
    R0.2 <>  |_|3         14|_|  <> R2.1
              _|            |_
    R0.3 <>  |_|4         13|_|  <> R2.0
              _|            |_
R3.3/CL2 =>  |_|5         12|_|  <> R1.3
              _|            |_
     CL1 ==  |_|6         11|_|  <> R1.2
              _|            |_
     ACL ->  |_|7         10|_|  <> R1.1
              _|            |_
     GND --  |_|8          9|_|  <> R1.0
               |____________|


18-pin DIP SM590/591/595:
                ____    ____
              _|*   \__/    |_
    R0.0 <>  |_|1         18|_|  -- VDD(5v)
              _|            |_
    R0.1 <>  |_|2         17|_|  <> R2.2
              _|            |_
    R2.3 <>  |_|3         16|_|  <> R3.0
              _|            |_
    R0.2 <>  |_|4         15|_|  <> R2.1
              _|            |_
    R0.3 <>  |_|5         14|_|  <> R2.0
              _|            |_
R3.3/CL2 =>  |_|6         13|_|  <> R1.3
              _|            |_
     CL1 ==  |_|7         12|_|  <> R1.2
              _|            |_
     ACL ->  |_|8         11|_|  <> R1.1
              _|            |_
     GND --  |_|9         10|_|  <> R1.0
               |____________|


20-pin DIP SM590/591/595:
                ____    ____
              _|*   \__/    |_
    R0.0 <>  |_|1         20|_|  -- VDD(5v)
              _|            |_
    R0.1 <>  |_|2         19|_|  <> R2.2
              _|            |_
    R2.3 <>  |_|3         18|_|  <> R3.0
              _|            |_
    R0.2 <>  |_|4         17|_|  <> R2.1
              _|            |_
    R0.3 <>  |_|5         16|_|  <> R2.0
              _|            |_
R3.3/CL2 =>  |_|6         15|_|  <> R1.3
              _|            |_
     CL1 ==  |_|7         14|_|  <> R1.2
              _|            |_
    R3.2 <>  |_|8         13|_|  <> R3.1
              _|            |_
     ACL ->  |_|9         12|_|  <> R1.1
              _|            |_
     GND --  |_|10        11|_|  <> R1.0
               |____________|

*/

class sm590_device : public sm510_base_device
{
public:
	sm590_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// four 4-bit R I/O ports
	using sm510_base_device::write_r;
	template <std::size_t N> auto write_r() { return m_write_rx[N].bind(); }
	template <std::size_t N> auto read_r() { return m_read_rx[N].bind(); }

protected:
	sm590_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_512x8(address_map &map) ATTR_COLD;
	void program_768x8(address_map &map) ATTR_COLD;
	void program_1kx8(address_map &map) ATTR_COLD;
	void data_32x4(address_map &map) ATTR_COLD;
	void data_56x4(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; } // 4 cycles per machine cycle
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); } // "

	virtual void init_divider() override { } // no divider timer
	virtual void init_lcd_driver() override { } // no lcd driver
	virtual void execute_one() override;
	virtual bool op_argument() override;

	virtual void reset_vector() override { do_branch(0, 0, 0); }
	virtual void wakeup_vector() override { do_branch(0, 1, 0); }

	// R ports
	devcb_write8::array<4> m_write_rx;
	devcb_read8::array<4> m_read_rx;
	u8 m_rports[4];

	// opcode handlers
	virtual void do_branch(u8 pu, u8 pm, u8 pl) override;
	void port_w(offs_t offset, u8 data);

	virtual void op_tl() override;
	virtual void op_tls();

	virtual void op_lblx();
	virtual void op_lbmx();
	virtual void op_str();
	virtual void op_lda() override;
	virtual void op_exc() override;
	virtual void op_exax();
	virtual void op_blta();

	virtual void op_adx() override;
	virtual void op_ads();
	virtual void op_adc();
	virtual void op_inbm();
	virtual void op_debm();

	virtual void op_tax();
	virtual void op_tba();
	virtual void op_tc() override;

	virtual void op_atr() override;
	virtual void op_mtr();
	virtual void op_rta();
};

class sm591_device : public sm590_device
{
public:
	sm591_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class sm595_device : public sm590_device
{
public:
	sm595_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(SM590, sm590_device)
DECLARE_DEVICE_TYPE(SM591, sm591_device)
DECLARE_DEVICE_TYPE(SM595, sm595_device)

#endif // MAME_CPU_SM510_SM590_H
