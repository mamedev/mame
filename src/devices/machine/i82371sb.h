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

#include "machine/ataintf.h"

#include "sound/spkrdev.h"
#include "machine/ram.h"
#include "bus/isa/isa.h"
#include "machine/nvram.h"

#include "machine/am9517a.h"


class i82371sb_isa_device : public pci_device {
public:
	i82371sb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto smi() { return m_smi_callback.bind(); }
	auto boot_state_hook() { return m_boot_state_hook.bind(); }

	DECLARE_WRITE_LINE_MEMBER(pc_pirqa_w);
	DECLARE_WRITE_LINE_MEMBER(pc_pirqb_w);
	DECLARE_WRITE_LINE_MEMBER(pc_pirqc_w);
	DECLARE_WRITE_LINE_MEMBER(pc_pirqd_w);
	DECLARE_WRITE_LINE_MEMBER(pc_mirq0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_mirq1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_ferr_w);

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
	DECLARE_READ8_MEMBER(pc_dma8237_0_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_1_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_2_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_3_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_5_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_6_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_7_dack_r);
	DECLARE_WRITE8_MEMBER(pc_dma8237_0_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_1_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_2_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_3_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_5_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_6_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_7_dack_w);
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
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
	DECLARE_READ8_MEMBER(pc_dma_read_word);
	DECLARE_WRITE8_MEMBER(pc_dma_write_word);
	DECLARE_READ8_MEMBER(get_slave_ack);

	void internal_io_map(address_map &map);

	DECLARE_WRITE8_MEMBER (boot_state_w);
	DECLARE_WRITE8_MEMBER (nop_w);

	DECLARE_READ8_MEMBER  (iort_r);
	DECLARE_WRITE8_MEMBER (iort_w);
	DECLARE_READ16_MEMBER (xbcs_r);
	DECLARE_WRITE16_MEMBER(xbcs_w);
	DECLARE_READ8_MEMBER  (pirqrc_r);
	DECLARE_WRITE8_MEMBER (pirqrc_w);
	DECLARE_READ8_MEMBER  (tom_r);
	DECLARE_WRITE8_MEMBER (tom_w);
	DECLARE_READ16_MEMBER (mstat_r);
	DECLARE_WRITE16_MEMBER(mstat_w);
	DECLARE_READ8_MEMBER  (mbirq01_r);
	DECLARE_WRITE8_MEMBER (mbirq01_w);
	DECLARE_READ8_MEMBER  (mbdma_r);
	DECLARE_WRITE8_MEMBER (mbdma_w);
	DECLARE_READ16_MEMBER (pcsc_r);
	DECLARE_WRITE16_MEMBER(pcsc_w);
	DECLARE_READ8_MEMBER  (apicbase_r);
	DECLARE_WRITE8_MEMBER (apicbase_w);
	DECLARE_READ8_MEMBER  (dlc_r);
	DECLARE_WRITE8_MEMBER (dlc_w);
	DECLARE_READ8_MEMBER  (smicntl_r);
	DECLARE_WRITE8_MEMBER (smicntl_w);
	DECLARE_READ16_MEMBER (smien_r);
	DECLARE_WRITE16_MEMBER(smien_w);
	DECLARE_READ32_MEMBER (see_r);
	DECLARE_WRITE32_MEMBER(see_w);
	DECLARE_READ8_MEMBER  (ftmr_r);
	DECLARE_WRITE8_MEMBER (ftmr_w);
	DECLARE_READ16_MEMBER (smireq_r);
	DECLARE_WRITE16_MEMBER(smireq_w);
	DECLARE_READ8_MEMBER  (ctltmr_r);
	DECLARE_WRITE8_MEMBER (ctltmr_w);
	DECLARE_READ8_MEMBER  (cthtmr_r);
	DECLARE_WRITE8_MEMBER (cthtmr_w);

	// southbridge
	DECLARE_READ8_MEMBER(at_page8_r);
	DECLARE_WRITE8_MEMBER(at_page8_w);
	DECLARE_READ8_MEMBER(at_portb_r);
	DECLARE_WRITE8_MEMBER(at_portb_w);
	DECLARE_WRITE_LINE_MEMBER(iochck_w);
	DECLARE_READ8_MEMBER(ide_read_cs1_r);
	DECLARE_WRITE8_MEMBER(ide_write_cs1_w);
	DECLARE_READ8_MEMBER(ide2_read_cs1_r);
	DECLARE_WRITE8_MEMBER(ide2_write_cs1_w);
	DECLARE_READ8_MEMBER(at_dma8237_2_r);
	DECLARE_WRITE8_MEMBER(at_dma8237_2_w);
	DECLARE_READ8_MEMBER(eisa_irq_read);
	DECLARE_WRITE8_MEMBER(eisa_irq_write);
	DECLARE_READ8_MEMBER(read_apmcapms);
	DECLARE_WRITE8_MEMBER(write_apmcapms);

	void update_smireq_line();

	devcb_write_line m_smi_callback;
	devcb_write8 m_boot_state_hook;

	uint32_t see;
	uint16_t xbcs, mstat, pcsc, smien, smireq;
	uint8_t apmc, apms;
	uint8_t iort, pirqrc[4], tom, mbirq0, mbirq1, mbdma[2], apicbase;
	uint8_t dlc, smicntl, ftmr, ctlmtr, cthmtr;

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
	DECLARE_READ16_MEMBER(command_r);
	DECLARE_WRITE16_MEMBER(command_w);
	DECLARE_READ32_MEMBER(bmiba_r);
	DECLARE_WRITE32_MEMBER(bmiba_w);
	DECLARE_READ16_MEMBER(idetim_primary_r);
	DECLARE_WRITE16_MEMBER(idetim_primary_w);
	DECLARE_READ16_MEMBER(idetim_secondary_r);
	DECLARE_WRITE16_MEMBER(idetim_secondary_w);
	DECLARE_READ8_MEMBER(sidetim_r);
	DECLARE_WRITE8_MEMBER(sidetim_w);

	DECLARE_READ32_MEMBER(ide1_read32_cs0_r);
	DECLARE_WRITE32_MEMBER(ide1_write32_cs0_w);
	DECLARE_READ32_MEMBER(ide2_read32_cs0_r);
	DECLARE_WRITE32_MEMBER(ide2_write32_cs0_w);
	DECLARE_READ8_MEMBER(ide1_read_cs1_r);
	DECLARE_WRITE8_MEMBER(ide1_write_cs1_w);
	DECLARE_READ8_MEMBER(ide2_read_cs1_r);
	DECLARE_WRITE8_MEMBER(ide2_write_cs1_w);

	void internal_io_map(address_map &map);

	uint16_t command;
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
