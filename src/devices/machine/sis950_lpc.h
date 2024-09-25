// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS950_LPC_H
#define MAME_MACHINE_SIS950_LPC_H

#pragma once

#include "pci.h"

#include "bus/ata/ataintf.h"
#include "bus/isa/isa.h"
#include "lpc-acpi.h"
#include "sis950_smbus.h"

#include "cpu/i386/i386.h"

#include "bus/pc_kbd/pc_kbdc.h"
#include "machine/at_keybc.h"
#include "machine/am9517a.h"
#include "machine/ds128x.h"
#include "machine/intelfsh.h"
#include "machine/pc_lpt.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/nvram.h"

#include "sound/spkrdev.h"


class sis950_lpc_device : public pci_device
{
public:
	template <typename T, typename U> sis950_lpc_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		T &&cpu_tag, U &&flash_tag
	) : sis950_lpc_device(mconfig, tag, owner, clock)
	{
		// Revision 0 -> A0
		set_ids(0x10390008, 0x00, 0x060100, 0x00);
		//set_multifunction_device(true);
		m_host_cpu.set_tag(std::forward<T>(cpu_tag));
		m_flash_rom.set_tag(std::forward<U>(flash_tag));
	}

	sis950_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::MOUSE; }

	auto fast_reset_cb() { return m_fast_reset_cb.bind(); }

	void pc_irq1_w(int state);
	void pc_irq3_w(int state);
	void pc_irq4_w(int state);
	void pc_irq5_w(int state);
	void pc_irq6_w(int state);
	void pc_irq7_w(int state);
	void pc_irq8n_w(int state);
	void pc_irq9_w(int state);
	void pc_irq10_w(int state);
	void pc_irq11_w(int state);
	void pc_irq12m_w(int state);
	void pc_irq14_w(int state);
	void pc_irq15_w(int state);

protected:
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override;

//  virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	template <unsigned N> void memory_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	virtual bool map_first() const override { return true; }

private:
	required_device<cpu_device> m_host_cpu;
	required_device<intelfsh8_device> m_flash_rom;
	required_device<pic8259_device> m_pic_master;
	required_device<pic8259_device> m_pic_slave;
	required_device<am9517a_device> m_dmac_master;
	required_device<am9517a_device> m_dmac_slave;
	required_device<pit8254_device> m_pit;
	required_device<ps2_keyboard_controller_device> m_keybc;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;
	required_device<ds12885ext_device> m_rtc;
	required_device<pc_kbdc_device> m_ps2_con;
	required_device<pc_kbdc_device> m_aux_con;
	required_device<lpc_acpi_device> m_acpi;
	required_device<sis950_smbus_device> m_smbus;

	devcb_write_line m_fast_reset_cb;

	// PCI interface
	u8 bios_control_r();
	void bios_control_w(u8 data);
	u8 flash_ctrl_r();
	void flash_ctrl_w(u8 data);
	u8 acpi_base_r();
	void acpi_base_w(u8 data);
	u8 init_enable_r();
	void init_enable_w(u8 data);
	u8 keybc_reg_r();
	void keybc_reg_w(u8 data);
	u8 rtc_reg_r();
	void rtc_reg_w(u8 data);
	void rtc_index_w(u8 data);
	u8 rtc_data_r();
	void rtc_data_w(u8 data);
	template <unsigned N> u8 irq_remap_r();
	template <unsigned N> void irq_remap_w(u8 data);
	u8 unmap_log_r(offs_t offset);
	void unmap_log_w(offs_t offset, u8 data);

	u8 m_bios_control = 0;
	u8 m_flash_control = 0;
	u16 m_acpi_base = 0x0000;
	u8 m_init_reg = 0;
	u8 m_keybc_reg = 0;
	u8 m_rtc_reg = 0;
	u8 m_rtc_index = 0;

	enum {
		IRQ_INTA = 0,
		IRQ_INTB,
		IRQ_INTC,
		IRQ_INTD,
		IRQ_IDE,
		IRQ_GPE,
		// or SCI
		IRQ_ACPI,
		IRQ_SMBUS,
		IRQ_SWDOG
	};
	u8 m_irq_remap[9]{};

	// LPC vendor specific, verify if it's common for all
	u8 lpc_fast_init_r();
	void lpc_fast_init_w(offs_t offset, u8 data);
	struct {
		u8 fast_init;
	} m_lpc_legacy;

	// southbridge implementation
	void pit_out0(int state);
	void pit_out1(int state);
	void pit_out2(int state);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t pc_dma_read_word(offs_t offset);
	void pc_dma_write_word(offs_t offset, uint8_t data);
	void pc_dma_hrq_changed(int state);
	void pc_select_dma_channel(int channel, bool state);

	uint8_t m_at_pages[0x10]{};
	uint8_t m_dma_offset[2][4]{};
	uint8_t m_at_speaker = 0;
	uint8_t m_refresh = 0;
	bool m_pit_out2 = 0;
	bool m_at_spkrdata = 0;
	uint8_t m_channel_check = 0;
	int m_dma_channel = -1;
//  bool m_cur_eop = false;
	uint16_t m_dma_high_byte = 0;

	void cpu_a20_w(int state);
	void cpu_reset_w(int state);

	uint8_t at_page8_r(offs_t offset);
	void at_page8_w(offs_t offset, uint8_t data);
	u8 nmi_status_r();
	void nmi_control_w(uint8_t data);
	void at_speaker_set_spkrdata(uint8_t data);
	void iochck_w(int state);
};

DECLARE_DEVICE_TYPE(SIS950_LPC, sis950_lpc_device)


#endif
