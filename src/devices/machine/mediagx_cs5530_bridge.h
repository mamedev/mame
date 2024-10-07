// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_MEDIAGX_CS5530_BRIDGE_H
#define MAME_MACHINE_MEDIAGX_CS5530_BRIDGE_H

#pragma once

#include "pci.h"
#include "mediagx_cs5530_ide.h"

#include "bus/isa/isa.h"
#include "cpu/i386/i386.h"
#include "machine/8042kbdc.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "sound/spkrdev.h"


class mediagx_cs5530_bridge_device : public pci_device
{
public:
	template <typename T, typename U>
	mediagx_cs5530_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&ide_tag)
		: mediagx_cs5530_bridge_device(mconfig, tag, owner, clock)
	{
		set_ids(0x10780100, 0x00, 0x060100, 0x00000000);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ide_tag(std::forward<U>(ide_tag));
	}

	mediagx_cs5530_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto boot_state_hook() { return m_boot_state_hook.bind(); }
	auto rtcale() { return m_rtcale.bind(); }
	auto rtccs_read() { return m_rtccs_read.bind(); }
	auto rtccs_write() { return m_rtccs_write.bind(); }
	void pc_irq1_w(int state);
	void pc_irq8n_w(int state);
	void pc_irq14_w(int state);
	void pc_irq15_w(int state);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_ide_tag(T &&tag) { m_ide.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_kbdc_tag(T &&tag) { m_kbdc.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_add_mconfig(machine_config & config) override;
	virtual void device_config_complete() override;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual bool map_first() const override { return true; }

	virtual void config_map(address_map &map) override ATTR_COLD;
private:
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
	void pc_irq3_w(int state);
	void pc_irq4_w(int state);
	void pc_irq5_w(int state);
	void pc_irq6_w(int state);
	void pc_irq7_w(int state);
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

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);
	void internal_io_map(address_map &map) ATTR_COLD;

	devcb_write8 m_boot_state_hook;
	devcb_write8 m_rtcale;
	devcb_read8 m_rtccs_read;
	devcb_write8 m_rtccs_write;

	required_device<cpu_device> m_host_cpu;
	required_device<mediagx_cs5530_ide_device> m_ide;
	required_device<kbdc8042_device> m_kbdc;
	// southbridge internals
	required_device<pic8259_device> m_pic8259_master;
	required_device<pic8259_device> m_pic8259_slave;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pit8254_device> m_pit8254;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

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

	u8 m_fast_init;
	u8 m_decode_control[2]{};
};

DECLARE_DEVICE_TYPE(MEDIAGX_CS5530_BRIDGE, mediagx_cs5530_bridge_device)

#endif
