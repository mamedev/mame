// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_BUS_LPCI_SOUTHBRIDGE_H
#define MAME_BUS_LPCI_SOUTHBRIDGE_H

#pragma once

#include "pci.h"

#include "machine/ins8250.h"
#include "machine/ds128x.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"

#include "bus/ata/ataintf.h"
#include "machine/at_keybc.h"
#include "machine/idectrl.h"

#include "imagedev/harddriv.h"

#include "sound/spkrdev.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "bus/isa/isa.h"

#include "machine/pc_lpt.h"
#include "bus/pc_kbd/pc_kbdc.h"

#include "machine/am9517a.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> southbridge_device

class southbridge_device :
		public device_t
{
public:


protected:
	// construction/destruction
	southbridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void port80_debug_write(uint8_t value) {}

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259_master;
	required_device<pic8259_device> m_pic8259_slave;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pit8254_device> m_pit8254;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;
	required_device<bus_master_ide_controller_device> m_ide;
	required_device<bus_master_ide_controller_device> m_ide2;

	uint8_t m_at_spkrdata;
	uint8_t m_pit_out2;
	int m_dma_channel;
	bool m_cur_eop;
	uint8_t m_dma_offset[2][4];
	uint8_t m_at_pages[0x10];
	uint16_t m_dma_high_byte;
	uint8_t m_at_speaker;
	bool m_refresh;
	uint16_t m_eisa_irq_mode;
	void at_speaker_set_spkrdata(uint8_t data);

	uint8_t m_channel_check;
	uint8_t m_nmi_enabled;
	bool m_ide_io_ports_enabled;
	address_space *spaceio;

	void pc_select_dma_channel(int channel, bool state);

private:
	uint8_t at_page8_r(offs_t offset);
	void at_page8_w(offs_t offset, uint8_t data);
	uint8_t at_portb_r();
	void at_portb_w(uint8_t data);
	void iochck_w(int state);
	uint8_t get_slave_ack(offs_t offset);
	void at_pit8254_out0_changed(int state);
	void at_pit8254_out1_changed(int state);
	void at_pit8254_out2_changed(int state);
	void pc_dma_hrq_changed(int state);
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
	void at_dma8237_out_eop(int state);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	void pc_dack4_w(int state);
	void pc_dack5_w(int state);
	void pc_dack6_w(int state);
	void pc_dack7_w(int state);

	uint32_t ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ide1_read_cs1_r();
	void ide1_write_cs1_w(uint8_t data);
	uint8_t ide2_read_cs1_r();
	void ide2_write_cs1_w(uint8_t data);

	uint8_t at_dma8237_2_r(offs_t offset);
	void at_dma8237_2_w(offs_t offset, uint8_t data);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t pc_dma_read_word(offs_t offset);
	void pc_dma_write_word(offs_t offset, uint8_t data);
	uint8_t eisa_irq_read(offs_t offset);
	void eisa_irq_write(offs_t offset, uint8_t data);
};

// ======================> southbridge_extended_device

class southbridge_extended_device :
	public southbridge_device
{
public:


protected:
	// construction/destruction
	southbridge_extended_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	required_device<at_keyboard_controller_device> m_keybc;
	required_device<ds12885_device> m_ds12885;
	required_device<pc_kbdc_device> m_pc_kbdc;

private:
	void rtc_nmi_w(uint8_t data);
};

#endif  // MAME_BUS_LPCI_SOUTHBRIDGE_H
