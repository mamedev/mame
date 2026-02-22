// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MACHINE_VT82C586B_ISA_H
#define MAME_MACHINE_VT82C586B_ISA_H

#pragma once

#include "pci.h"
#include "bus/isa/isa.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ds128x.h"
#include "machine/at_keybc.h"
#include "sound/spkrdev.h"


class vt82c586b_isa_device : public pci_device
{
public:
	template <typename T>
	vt82c586b_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: vt82c586b_isa_device(mconfig, tag, owner, clock)
	{
		// revisions:
		// 0*h for regular VT82C586
		// 2*h for '586A
		// 3*h for '586B OEM Silicon
		// 4*h for '586B Production Silicon
		set_ids(0x11060586, 0x41, 0x060100, 0x00000000);
		set_multifunction_device(true);
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	vt82c586b_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto boot_state_hook() { return m_boot_state_hook.bind(); }
	auto a20m() { return m_write_a20m.bind(); }
	auto cpureset() { return m_write_cpureset.bind(); }
	auto pcirst() { return m_write_pcirst.bind(); }

	void pc_irq1_w(int state);
	void pc_irq3_w(int state);
	void pc_irq4_w(int state);
	void pc_irq6_w(int state);
	void pc_irq7_w(int state);
	void pc_irq8n_w(int state);
	// TODO: remaps externally for IDE, cfr. config $4a
	void pc_irq14_w(int state);
	void pc_irq15_w(int state);

	void pc_pirqa_w(int state);
	void pc_pirqb_w(int state);
	void pc_pirqc_w(int state);
	void pc_pirqd_w(int state);
	void pc_mirq0_w(int state);
	void pc_mirq1_w(int state);
	void pc_mirq2_w(int state);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_add_mconfig(machine_config & config) override;
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual bool map_first() const override { return true; }

	virtual void config_map(address_map &map) override ATTR_COLD;
	void internal_io_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_host_cpu;
	required_device_array<pic8259_device, 2> m_pic;
	required_device_array<am9517a_device, 2> m_dma;
	required_device<pit8254_device> m_pit;
	required_device<ps2_keyboard_controller_device> m_keybc;
	required_device<pc_kbdc_device> m_ps2_con;
	required_device<pc_kbdc_device> m_aux_con;
	required_device<ds12885ext_device> m_rtc;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;
	devcb_write_line m_write_a20m;
	devcb_write_line m_write_cpureset;
	devcb_write_line m_write_pcirst;
	devcb_write8 m_boot_state_hook;

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);

	u8 m_isa_bus_control;
	u8 m_isa_test_mode;
	u8 m_isa_clock_control;
	u8 m_rom_decode_control;
	u8 m_keybc_control;
	u8 m_dma_control_typef;
	u8 m_misc_control[3];
	u8 m_ide_irq_routing;
	u8 m_pci_memory_hole_bottom;
	u8 m_pci_memory_hole_top;
	u8 m_pci_memory_access_control_3;

	u8 m_rtc_test_mode;
	u8 m_xd_power_on;
	u8 m_dma_linebuffer_disable;
	u16 m_ddma_control[8];

	template <unsigned E> u8 rtc_index_r(offs_t offset);
	template <unsigned E> void rtc_index_w(offs_t offset, u8 data);

	template <unsigned E> u8 rtc_data_r(offs_t offset);
	template <unsigned E> void rtc_data_w(offs_t offset, u8 data);

	u8 port92_r(offs_t offset);
	void port92_w(offs_t offset, u8 data);

	// Southbridge common stuff
	void at_pit8254_out0_changed(int state);
	void at_pit8254_out1_changed(int state);
	void at_pit8254_out2_changed(int state);
	uint8_t pc_dma8237_0_dack_r();
	uint8_t pc_dma8237_1_dack_r();
	uint8_t pc_dma8237_2_dack_r();
	uint8_t pc_dma8237_3_dack_r();
	uint8_t pc_dma8237_5_dack_r();
	uint8_t pc_dma8237_6_dack_r();
	uint8_t pc_dma8237_7_dack_r();
	void pc_dma8237_0_dack_w(uint8_t data);
	void pc_dma8237_1_dack_w(uint8_t data);
	void pc_dma8237_2_dack_w(uint8_t data);
	void pc_dma8237_3_dack_w(uint8_t data);
	void pc_dma8237_5_dack_w(uint8_t data);
	void pc_dma8237_6_dack_w(uint8_t data);
	void pc_dma8237_7_dack_w(uint8_t data);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	void pc_dack4_w(int state);
	void pc_dack5_w(int state);
	void pc_dack6_w(int state);
	void pc_dack7_w(int state);
	void at_dma8237_out_eop(int state);
	void pc_dma_hrq_changed(int state);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t pc_dma_read_word(offs_t offset);
	void pc_dma_write_word(offs_t offset, uint8_t data);
	uint8_t at_portb_r();
	void at_portb_w(uint8_t data);
	void at_speaker_set_spkrdata(uint8_t data);
	uint8_t get_slave_ack(offs_t offset);
	void pc_irq5_w(int state);
	void pc_irq9_w(int state);
	void pc_irq10_w(int state);
	void pc_irq11_w(int state);
	void pc_irq12m_w(int state);
	void iochck_w(int state);
	void pc_select_dma_channel(int channel, bool state);
	uint8_t at_page8_r(offs_t offset);
	void at_page8_w(offs_t offset, uint8_t data);
	uint8_t at_dma8237_2_r(offs_t offset);
	void at_dma8237_2_w(offs_t offset, uint8_t data);

	uint8_t m_at_spkrdata = 0;
	uint8_t m_pit_out2 = 0;
	uint8_t m_at_speaker = 0;
	bool m_refresh = false;
	int m_dma_channel = 0;
	bool m_cur_eop = false;
	uint8_t m_dma_offset[2][4];
	uint8_t m_at_pages[0x10]{};
	uint16_t m_dma_high_byte = 0;
	uint8_t m_channel_check = 0;
	bool m_nmi_enabled = false;

	void keyboard_gatea20(int state);
	void fast_gatea20(int state);
	int m_ext_gatea20, m_fast_gatea20;
	u8 m_port92;

	u8 m_rtc_index;

	void redirect_irq(int irq, int state);

	int pin_mapper(int pin);
	void irq_handler(int line, int state);

	u8 m_pirqrc[4];
	u8 m_pirq_select;
	u8 m_mirq[3];
	u8 m_mirq_pin_config;
};

DECLARE_DEVICE_TYPE(VT82C586B_ISA, vt82c586b_isa_device)


#endif // MAME_MACHINE_VT82C586B_ISA_H
