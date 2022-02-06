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
	sm590_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	sm590_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_1x128x4(address_map &map);
	void data_16x2x4(address_map &map);

	virtual void device_reset() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void init_divider() override { }
	virtual void init_lcd_driver() override { }
	virtual void init_melody() override { }
	virtual void increment_pc() override;
	virtual void execute_one() override;
	virtual bool op_argument() override;
	virtual void do_branch(u8 pu, u8 pm, u8 pl) override;

	virtual void reset_vector() override { do_branch(0, 0, 0); }
	virtual void wakeup_vector() override { do_branch(0, 1, 0); }

	// opcode handlers
	// 00-3f
	virtual void op_adx() override;
	virtual void op_tax();
	virtual void op_lblx();
	// lax (same as sm510)

	// 40-43
	virtual void op_lda() override;
	virtual void op_exc() override;
	// exci (same as sm510)
	// excd (same as sm510)

	// 44-47
	// coma (same as sm510)
	// tam (same as sm510)
	virtual void op_atr() override;
	virtual void op_mtr();

	// 48-4b
	// rc (same as sm510)
	// sc (same as sm510)
	virtual void op_str();
	// cctrl (same as sm510 cend)

	// 4c-4f
	// rtn (same as sm510 rtn0)
	// rtns (same as sm510 rtn1)
	// 4e illegal
	// 4f illegal

	// 50-53
	virtual void op_inbm();
	virtual void op_debm();
	// inbl (same as sm510 incb)
	// debl (same as sm510 decb)

	// 54-57
	virtual void op_tc() override;
	virtual void op_rta();
	virtual void op_blta();
	// xbla (same as sm510 exbla)

	// 58-5b are all illegal

	// 5c-5f
	// atx (same as sm510)
	virtual void op_exax();
	// 5e illegal???
	// 5f illegal

	// 60-6f
	// tm (same as sm510 tmi)
	virtual void op_tba();
	// rm (same as sm510)
	// sm (same as sm510)

	// 70-73
	// add (same as sm510)
	virtual void op_ads();
	virtual void op_adc();
	// adcs (same as sm510 add11)

	// 74-7f
	virtual void op_lbmx();
	virtual void op_tl() override;
	virtual void op_tml() override; // aka TLS

	// 80-ff
	virtual void op_t() override; // aka TR

	u8 m_rports[4];
};


DECLARE_DEVICE_TYPE(SM590, sm590_device)

#endif // MAME_CPU_SM510_SM590_H
