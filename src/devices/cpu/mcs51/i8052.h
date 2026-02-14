// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#ifndef MAME_CPU_MCS51_I8052_H
#define MAME_CPU_MCS51_I8052_H

#include "i8051.h"

// variants with no internal rom and 256 byte internal memory
DECLARE_DEVICE_TYPE(I8032, i8032_device)

// variants 8k internal rom and 256 byte internal memory and more registers
DECLARE_DEVICE_TYPE(I8052, i8052_device)
DECLARE_DEVICE_TYPE(I8752, i8752_device)


class i8052_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8052_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	enum {
		T2CON_TF2   = 7,  //Indicated Timer 2 Overflow Int Triggered
		T2CON_EXF2  = 6,  //Indicates Timer 2 External Flag
		T2CON_RCLK  = 5,  //Receive Clock
		T2CON_TCLK  = 4,  //Transmit Clock
		T2CON_EXEN2 = 3,  //Timer 2 External Interrupt Enable
		T2CON_TR2   = 2,  //Indicates Timer 2 is running
		T2CON_CT2   = 1,  //Sets Timer 2 Counter/Timer Mode
		T2CON_CP    = 0,  //Sets Timer 2 Capture/Reload Mode
	};

	u16 m_rcap2, m_t2;
	u8 m_t2con;

	i8052_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void handle_8bit_uart_clock(int source) override;
	virtual void irqs_complete_and_mask(u8 &ints, u8 int_mask) override;
	virtual void handle_irq(int irqline, int state, u32 new_state, u32 tr_state) override;
	virtual void sfr_map(address_map &map) override ATTR_COLD;
	virtual void update_timer_t2(int cycles) override;

	void set_tf2 (bool state) { set_bit<T2CON_TF2 >(m_t2con, state); }
	void set_exf2(bool state) { set_bit<T2CON_EXF2>(m_t2con, state); }

	u8 t2con_r();
	void t2con_w(u8 data);
	u8 rcap2_r(offs_t offset);
	void rcap2_w(offs_t offset, u8 data);
	u8 t2_r(offs_t offset);
	void t2_w(offs_t offset, u8 data);
};

class i8032_device : public i8052_device
{
public:
	// construction/destruction
	i8032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8752_device : public i8052_device
{
public:
	// construction/destruction
	i8752_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


#endif
