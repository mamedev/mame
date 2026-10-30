// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
/*****************************************************************************
 *
 * includes/bebox.h
 *
 * BeBox
 *
 ****************************************************************************/

#ifndef MAME_BE_BEBOX_H
#define MAME_BE_BEBOX_H

#include "bus/isa/isa.h"
#include "cpu/powerpc/ppc.h"
#include "machine/8042kbdc.h"
#include "machine/i82378zb_sio.h"
#include "machine/idectrl.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/max186.h"
#include "machine/mc146818.h"
#include "machine/mpc105.h"
#include "machine/ram.h"

class bebox_state : public driver_device
{
public:
	bebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ppc(*this, "ppc%u", 1U)
		, m_sio(*this, "pci:0b.0")
		, m_isabus(*this, "pci:0b.0:isabus")
		, m_ram(*this, RAM_TAG)
		, m_flash(*this, "flash")
		, m_mpc105(*this, "pci:00.0")
		, m_adc(*this, "adc")
		, m_kbdc(*this, "kbdc")
		, m_uart(*this, "ns16550_%u", 2U)
		, m_rtc(*this, "rtc")
		, m_ide(*this, "ide")
		, m_joy_axes(*this, "joyport:JOY%u", 0U)
		, m_joy_buttons(*this, "joyport:JOYBUTTONS")
	{
	}


	void bebox_peripherals(machine_config &config);
	void bebox(machine_config &config);
	void bebox2(machine_config &config);

	void init_bebox();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device_array<ppc_device, 2> m_ppc;
	required_device<i82378zb_sio_device> m_sio;
	required_device<isa16_device> m_isabus;
	required_device<ram_device> m_ram;
	required_device<fujitsu_29f016a_device> m_flash;
	required_device<mpc105_host_device> m_mpc105;
	required_device<max186_device> m_adc;
	required_device<kbdc8042_device> m_kbdc;
	required_device_array<ns16550_device, 2> m_uart;
	required_device<mc146818_device> m_rtc;
	required_device<ide_controller_device> m_ide;
	required_ioport_array<8> m_joy_axes;
	required_ioport m_joy_buttons;

	emu_timer *m_adc_timer = nullptr;
	u16 m_adc_control = 0;
	u16 m_adc_result = 0;
	u16 m_adc_shift = 0;
	u16 m_joy_mux = 0;
	u8 m_adc_cycle = 0;
	bool m_adc_done = false;
	uint32_t m_cpu_imask[2]{};
	uint32_t m_interrupts = 0U;
	uint32_t m_crossproc_interrupts = 0U;

	u8 joy_buttons_r() { return m_joy_buttons->read(); }
	u16 adc_control_r();
	void adc_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 adc_data_r() { return m_adc_result; }
	u16 joy_mux_r() { return m_joy_mux; }
	void joy_mux_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_joy_mux); }
	double adc_input(u8 channel);
	TIMER_CALLBACK_MEMBER(adc_tick);

	uint64_t bebox_cpu0_imask_r();
	uint64_t bebox_cpu1_imask_r();
	uint64_t bebox_interrupt_sources_r();
	uint64_t bebox_crossproc_interrupts_r(address_space &space);
	uint64_t bebox_interrupt_ack_r();
	uint8_t bebox_flash_r(offs_t offset);

	void bebox_cpu0_imask_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void bebox_cpu1_imask_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void bebox_crossproc_interrupts_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void bebox_processor_resets_w(uint64_t data);
	void bebox_flash_w(offs_t offset, uint8_t data);

	void bebox_keyboard_interrupt(int state);

	int map_pci_pin(int pin) { return pin; }
	void pci_irq_w(int line, int state) { bebox_set_irq_bit(line, state); }

	void bebox_set_irq_bit(unsigned int interrupt_bit, int val);
	void bebox_update_interrupts();

	void main_mem(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


#endif // MAME_BE_BEBOX_H
