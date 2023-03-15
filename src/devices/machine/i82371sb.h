// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82371sb southbridge (PIIX3)
#ifndef MAME_MACHINE_I82371SB_H
#define MAME_MACHINE_I82371SB_H

#pragma once

#include "pci.h"
#include "machine/pci-ide.h"

#include "machine/ins8250.h"
#include "machine/ds128x.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"

#include "bus/ata/ataintf.h"

#include "sound/spkrdev.h"
#include "machine/ram.h"
#include "bus/isa/isa.h"
#include "machine/nvram.h"

#include "machine/am9517a.h"


class i82371sb_isa_device : public pci_device {
public:
	i82371sb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto smi() { return m_smi_callback.bind(); }
	auto nmi() { return m_nmi_callback.bind(); }
	auto stpclk() { return m_stpclk_callback.bind(); }
	auto boot_state_hook() { return m_boot_state_hook.bind(); }

	DECLARE_WRITE_LINE_MEMBER(pc_pirqa_w);
	DECLARE_WRITE_LINE_MEMBER(pc_pirqb_w);
	DECLARE_WRITE_LINE_MEMBER(pc_pirqc_w);
	DECLARE_WRITE_LINE_MEMBER(pc_pirqd_w);
	DECLARE_WRITE_LINE_MEMBER(pc_mirq0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_mirq1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_ferr_w);
	DECLARE_WRITE_LINE_MEMBER(pc_extsmi_w);

	DECLARE_WRITE_LINE_MEMBER(pc_irq1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq3_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq4_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq5_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq6_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq7_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq8n_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq9_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq10_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq11_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq12m_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq14_w);
	DECLARE_WRITE_LINE_MEMBER(pc_irq15_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual bool map_first() const override { return true; }

	virtual void config_map(address_map &map) override;

private:
	DECLARE_WRITE_LINE_MEMBER(at_pit8254_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(at_pit8254_out1_changed);
	DECLARE_WRITE_LINE_MEMBER(at_pit8254_out2_changed);
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
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack4_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack5_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack6_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack7_w);
	DECLARE_WRITE_LINE_MEMBER(at_dma8237_out_eop);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t pc_dma_read_word(offs_t offset);
	void pc_dma_write_word(offs_t offset,uint8_t data);
	uint8_t get_slave_ack(offs_t offset);

	void internal_io_map(address_map &map);

	void boot_state_w(uint8_t data);
	void nop_w(uint8_t data);

	void status_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t iort_r();
	void iort_w(uint8_t data);
	uint16_t xbcs_r();
	void xbcs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t pirqrc_r(offs_t offset);
	void pirqrc_w(offs_t offset, uint8_t data);
	uint8_t tom_r();
	void tom_w(uint8_t data);
	uint16_t mstat_r();
	void mstat_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t mbirq01_r(offs_t offset);
	void mbirq01_w(offs_t offset, uint8_t data);
	uint8_t mbdma_r(offs_t offset);
	void mbdma_w(offs_t offset, uint8_t data);
	uint16_t pcsc_r();
	void pcsc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t apicbase_r();
	void apicbase_w(uint8_t data);
	uint8_t dlc_r();
	void dlc_w(uint8_t data);
	uint8_t smicntl_r();
	void smicntl_w(uint8_t data);
	uint16_t smien_r();
	void smien_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t see_r();
	void see_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ftmr_r();
	void ftmr_w(uint8_t data);
	uint16_t smireq_r();
	void smireq_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t ctltmr_r();
	void ctltmr_w(uint8_t data);
	uint8_t cthtmr_r();
	void cthtmr_w(uint8_t data);

	// southbridge
	uint8_t at_page8_r(offs_t offset);
	void at_page8_w(offs_t offset, uint8_t data);
	uint8_t at_portb_r();
	void at_portb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(iochck_w);
	uint8_t at_dma8237_2_r(offs_t offset);
	void at_dma8237_2_w(offs_t offset, uint8_t data);
	uint8_t eisa_irq_read(offs_t offset);
	void eisa_irq_write(offs_t offset, uint8_t data);
	uint8_t read_apmcapms(offs_t offset);
	void write_apmcapms(offs_t offset, uint8_t data);
	uint8_t reset_control_r(offs_t offset);
	void reset_control_w(offs_t offset, uint8_t data);

	void update_smireq_line();

	devcb_write_line m_smi_callback;
	devcb_write_line m_nmi_callback;
	devcb_write_line m_stpclk_callback;
	devcb_write8 m_boot_state_hook;

	uint32_t see;
	uint16_t xbcs, mstat, pcsc, smien, smireq;
	uint8_t apmc, apms;
	uint8_t iort, pirqrc[4], tom, mbirq0, mbirq1, mbdma[2], apicbase;
	uint8_t dlc, smicntl, ftmr, ctlmtr, cthmtr;
	uint8_t reset_control;

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);

	//southbridge
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259_master;
	required_device<pic8259_device> m_pic8259_slave;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pit8254_device> m_pit8254;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	uint8_t m_at_spkrdata;
	uint8_t m_pit_out2;
	int m_dma_channel;
	bool m_cur_eop;
	uint8_t m_dma_offset[2][4];
	uint8_t m_at_pages[0x10];
	uint16_t m_dma_high_byte;
	uint16_t m_eisa_irq_mode;
	uint8_t m_at_speaker;
	bool m_refresh;
	void at_speaker_set_spkrdata(uint8_t data);

	uint8_t m_channel_check;
	uint8_t m_nmi_enabled;

	void pc_select_dma_channel(int channel, bool state);
	void redirect_irq(int irq, int state);
};

DECLARE_DEVICE_TYPE(I82371SB_ISA, i82371sb_isa_device)


class i82371sb_ide_device : public pci_device {
public:
	i82371sb_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_pri() { return m_irq_pri_callback.bind(); }
	auto irq_sec() { return m_irq_sec_callback.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	DECLARE_WRITE_LINE_MEMBER(primary_int);
	DECLARE_WRITE_LINE_MEMBER(secondary_int);

private:
	void status_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	virtual uint8_t latency_timer_r() override;
	void latency_timer_w(uint8_t data);
	uint32_t bmiba_r();
	void bmiba_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t idetim_primary_r();
	void idetim_primary_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t idetim_secondary_r();
	void idetim_secondary_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t sidetim_r();
	void sidetim_w(uint8_t data);

	uint32_t ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ide1_read_cs1_r();
	void ide1_write_cs1_w(uint8_t data);
	uint8_t ide2_read_cs1_r();
	void ide2_write_cs1_w(uint8_t data);

	void internal_io_map(address_map &map);

	uint8_t latency_timer;
	uint32_t bmiba;
	int idetim_primary, idetim_secondary;
	int sidetim;

	devcb_write_line m_irq_pri_callback;
	devcb_write_line m_irq_sec_callback;

	required_device<bus_master_ide_controller_device> m_ide1;
	required_device<bus_master_ide_controller_device> m_ide2;
};

DECLARE_DEVICE_TYPE(I82371SB_IDE, i82371sb_ide_device)

#endif // MAME_MACHINE_I82371SB_H
