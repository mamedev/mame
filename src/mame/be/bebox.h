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

#include "cpu/powerpc/ppc.h"

#include "imagedev/floppy.h"
#include "machine/53c810.h"
#include "machine/am9517a.h"
#include "machine/idectrl.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/intelfsh.h"
#include "video/pc_vga.h"

#include "bus/lpci/pci.h"


class bebox_state : public driver_device
{
public:
	bebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ppc(*this, "ppc%u", 1U)
		, m_lsi53c810(*this, "lsi53c810")
		, m_dma8237(*this, "dma8237_%u", 1U)
		, m_pic8259(*this, "pic8259_%u", 1U)
		, m_pit8254(*this, "pit8254")
		, m_ram(*this, RAM_TAG)
		, m_smc37c78(*this, "smc37c78")
		, m_flash(*this, "flash")
		, m_pcibus(*this, "pcibus")
		, m_vga(*this, "vga")
	{
	}


	void bebox_peripherals(machine_config &config);
	void bebox(machine_config &config);
	void bebox2(machine_config &config);

	void init_bebox();

	int m_dma_channel = 0; // TODO: move to private once possible

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device_array<ppc_device, 2> m_ppc;
	required_device<lsi53c810_device> m_lsi53c810;
	required_device_array<am9517a_device, 2> m_dma8237;
	required_device_array<pic8259_device, 2> m_pic8259;
	required_device<pit8254_device> m_pit8254;
	required_device<ram_device> m_ram;
	required_device<smc37c78_device> m_smc37c78;
	required_device<fujitsu_29f016a_device> m_flash;
	required_device<pci_bus_device> m_pcibus;
	required_device<vga_device> m_vga;
	uint32_t m_cpu_imask[2]{};
	uint32_t m_interrupts = 0U;
	uint32_t m_crossproc_interrupts = 0U;
	uint16_t m_dma_offset[2][4]{};
	uint8_t m_at_pages[0x10]{};
	uint32_t m_scsi53c810_data[0x100 / 4]{};
	void bebox_pic8259_master_set_int_line(int state);
	void bebox_pic8259_slave_set_int_line(int state);
	void bebox_dma_hrq_changed(int state);
	void bebox_dma8237_out_eop(int state);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	void bebox_timer0_w(int state);
	uint64_t bebox_cpu0_imask_r();
	uint64_t bebox_cpu1_imask_r();
	uint64_t bebox_interrupt_sources_r();
	uint64_t bebox_crossproc_interrupts_r(address_space &space);
	uint64_t bebox_interrupt_ack_r();
	uint8_t bebox_page_r(offs_t offset);
	uint8_t bebox_80000480_r();
	uint8_t bebox_flash_r(offs_t offset);

	void bebox_cpu0_imask_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void bebox_cpu1_imask_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void bebox_crossproc_interrupts_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void bebox_processor_resets_w(uint64_t data);
	void bebox_page_w(offs_t offset, uint8_t data);
	void bebox_80000480_w(offs_t offset, uint8_t data);
	void bebox_flash_w(offs_t offset, uint8_t data);
	uint8_t at_dma8237_1_r(offs_t offset);
	void at_dma8237_1_w(offs_t offset, uint8_t data);
	uint8_t bebox_dma_read_byte(offs_t offset);
	void bebox_dma_write_byte(offs_t offset, uint8_t data);
	inline void set_dma_channel(int channel, int state);
	uint64_t scsi53c810_r(offs_t offset, uint64_t mem_mask = ~0);
	void scsi53c810_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t bb_slave_64be_r(offs_t offset, uint64_t mem_mask = ~0);

	void bebox_ide_interrupt(int state);

	void bebox_keyboard_interrupt(int state);

	void fdc_interrupt(int state);

	uint32_t scsi_fetch(uint32_t dsp);
	void scsi_irq_callback(int state);
	void scsi_dma_callback(uint32_t src, uint32_t dst, int length, int byteswap);

	void bebox_set_irq_bit(unsigned int interrupt_bit, int val);
	void bebox_update_interrupts();

	void mpc105_config(device_t *device);
	void cirrus_config(device_t *device);

	pci_connector_device & add_pci_slot(machine_config &config, const char *tag, size_t index, const char *default_tag);

	void main_mem(address_map &map) ATTR_COLD;
	void slave_mem(address_map &map) ATTR_COLD;

	[[maybe_unused]] uint32_t scsi53c810_pci_read(int function, int offset, uint32_t mem_mask);
	[[maybe_unused]] void scsi53c810_pci_write(int function, int offset, uint32_t data, uint32_t mem_mask);
};


#endif // MAME_BE_BEBOX_H
