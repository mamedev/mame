// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82371sb southbridge (PIIX3)

#ifndef MAME_MACHINE_I82371SB_H
#define MAME_MACHINE_I82371SB_H

#pragma once

#include "pci.h"

#include "machine/ins8250.h"
#include "machine/ds128x.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"

#include "machine/ataintf.h"
#include "machine/at_keybc.h"

#include "sound/spkrdev.h"
#include "machine/ram.h"
#include "machine/nvram.h"

#include "machine/pc_lpt.h"
#include "bus/pc_kbd/pc_kbdc.h"

#include "machine/am9517a.h"


#define MCFG_I82371SB_ISA_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, I82371SB_ISA, 0x80867000, 0x03, 0x060100, 0x00000000)

#define MCFG_I82371SB_BOOT_STATE_HOOK(_devcb) \
	devcb = &i82371sb_isa_device::set_boot_state_hook(*device, DEVCB_##_devcb);

class i82371sb_isa_device : public pci_device {
public:
	i82371sb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_boot_state_hook(device_t &device, Object &&cb) { return downcast<i82371sb_isa_device &>(device).m_boot_state_hook.set_callback(std::forward<Object>(cb)); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

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
	DECLARE_READ8_MEMBER  (mbirq0_r);
	DECLARE_WRITE8_MEMBER (mbirq0_w);
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
	DECLARE_READ8_MEMBER(ide_read_cs1_r);
	DECLARE_WRITE8_MEMBER(ide_write_cs1_w);
	DECLARE_READ8_MEMBER(ide2_read_cs1_r);
	DECLARE_WRITE8_MEMBER(ide2_write_cs1_w);
	DECLARE_READ8_MEMBER(at_dma8237_2_r);
	DECLARE_WRITE8_MEMBER(at_dma8237_2_w);
	DECLARE_READ8_MEMBER(at_keybc_r);
	DECLARE_WRITE8_MEMBER(at_keybc_w);
	DECLARE_WRITE8_MEMBER(write_rtc);

	devcb_write8 m_boot_state_hook;

	uint32_t see;
	uint16_t xbcs, mstat, pcsc, smien, smireq;
	uint8_t iort, pirqrc[4], tom, mbirq0, mbdma[2], apicbase;
	uint8_t dlc, smicntl, ftmr, ctlmtr, cthmtr;

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);


	//southbridge
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259_master;
	required_device<pic8259_device> m_pic8259_slave;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pit8254_device> m_pit8254;
	required_device<at_keyboard_controller_device> m_keybc;
	required_device<speaker_sound_device> m_speaker;
	required_device<ds12885_device> m_ds12885;
	required_device<pc_kbdc_device> m_pc_kbdc;

	uint8_t m_at_spkrdata;
	uint8_t m_pit_out2;
	int m_dma_channel;
	bool m_cur_eop;
	uint8_t m_dma_offset[2][4];
	uint8_t m_at_pages[0x10];
	uint16_t m_dma_high_byte;
	uint8_t m_at_speaker;
	bool m_refresh;
	void at_speaker_set_spkrdata(uint8_t data);

	uint8_t m_channel_check;
	uint8_t m_nmi_enabled;

	void pc_select_dma_channel(int channel, bool state);
	// VGA-HACK
	optional_memory_region m_vga_region;
};

DECLARE_DEVICE_TYPE(I82371SB_ISA, i82371sb_isa_device)

#endif // MAME_MACHINE_I82371SB_H
