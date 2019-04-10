// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
/*****************************************************************************
 *
 * includes/bebox.h
 *
 * BeBox
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_BEBOX_H
#define MAME_INCLUDES_BEBOX_H

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

#include "bus/lpci/pci.h"


class bebox_state : public driver_device
{
public:
	enum
	{
		TIMER_GET_DEVICES
	};

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
	{
	}

	required_device_array<ppc_device, 2> m_ppc;
	required_device<lsi53c810_device> m_lsi53c810;
	required_device_array<am9517a_device, 2> m_dma8237;
	required_device_array<pic8259_device, 2> m_pic8259;
	required_device<pit8254_device> m_pit8254;
	required_device<ram_device> m_ram;
	required_device<smc37c78_device> m_smc37c78;
	required_device<fujitsu_29f016a_device> m_flash;
	required_device<pci_bus_device> m_pcibus;
	uint32_t m_cpu_imask[2];
	uint32_t m_interrupts;
	uint32_t m_crossproc_interrupts;
	int m_dma_channel;
	uint16_t m_dma_offset[2][4];
	uint8_t m_at_pages[0x10];
	uint32_t m_scsi53c810_data[0x100 / 4];
	void init_bebox();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_WRITE_LINE_MEMBER(bebox_pic8259_master_set_int_line);
	DECLARE_WRITE_LINE_MEMBER(bebox_pic8259_slave_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(bebox_dma_hrq_changed);
	DECLARE_READ8_MEMBER(bebox_dma8237_fdc_dack_r);
	DECLARE_WRITE8_MEMBER(bebox_dma8237_fdc_dack_w);
	DECLARE_WRITE_LINE_MEMBER(bebox_dma8237_out_eop);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_WRITE_LINE_MEMBER(bebox_timer0_w);
	DECLARE_READ64_MEMBER(bebox_cpu0_imask_r);
	DECLARE_READ64_MEMBER(bebox_cpu1_imask_r);
	DECLARE_READ64_MEMBER(bebox_interrupt_sources_r);
	DECLARE_READ64_MEMBER(bebox_crossproc_interrupts_r);
	DECLARE_READ64_MEMBER(bebox_interrupt_ack_r);
	DECLARE_READ8_MEMBER(bebox_page_r);
	DECLARE_READ8_MEMBER(bebox_80000480_r);
	DECLARE_READ8_MEMBER(bebox_flash_r);

	DECLARE_WRITE64_MEMBER(bebox_cpu0_imask_w);
	DECLARE_WRITE64_MEMBER(bebox_cpu1_imask_w);
	DECLARE_WRITE64_MEMBER(bebox_crossproc_interrupts_w);
	DECLARE_WRITE64_MEMBER(bebox_processor_resets_w);
	DECLARE_WRITE8_MEMBER(bebox_page_w);
	DECLARE_WRITE8_MEMBER(bebox_80000480_w);
	DECLARE_WRITE8_MEMBER(bebox_flash_w);
	DECLARE_READ8_MEMBER(at_dma8237_1_r);
	DECLARE_WRITE8_MEMBER(at_dma8237_1_w);
	DECLARE_READ8_MEMBER(bebox_dma_read_byte);
	DECLARE_WRITE8_MEMBER(bebox_dma_write_byte);
	DECLARE_READ64_MEMBER(scsi53c810_r);
	DECLARE_WRITE64_MEMBER(scsi53c810_w);
	DECLARE_READ64_MEMBER(bb_slave_64be_r);

	DECLARE_WRITE_LINE_MEMBER(bebox_ide_interrupt);

	DECLARE_WRITE_LINE_MEMBER(bebox_keyboard_interrupt);

	DECLARE_WRITE_LINE_MEMBER( fdc_interrupt );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	uint32_t scsi_fetch(uint32_t dsp);
	void scsi_irq_callback(int state);
	void scsi_dma_callback(uint32_t src, uint32_t dst, int length, int byteswap);

	void bebox_set_irq_bit(unsigned int interrupt_bit, int val);
	void bebox_update_interrupts();

	static void mpc105_config(device_t *device);

	pci_connector_device & add_pci_slot(machine_config &config, const char *tag, size_t index, const char *default_tag);
	void bebox_peripherals(machine_config &config);
	void bebox(machine_config &config);
	void bebox2(machine_config &config);

	void main_mem(address_map &map);
	void slave_mem(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

#ifdef UNUSED_LEGACY_CODE
	uint32_t scsi53c810_pci_read(int function, int offset, uint32_t mem_mask);
	void scsi53c810_pci_write(int function, int offset, uint32_t data, uint32_t mem_mask);
#endif
};


#endif // MAME_INCLUDES_BEBOX_H
