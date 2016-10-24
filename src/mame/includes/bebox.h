// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
/*****************************************************************************
 *
 * includes/bebox.h
 *
 * BeBox
 *
 ****************************************************************************/

#ifndef BEBOX_H_
#define BEBOX_H_

#include "emu.h"
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
		: driver_device(mconfig, type, tag),
			m_ppc1(*this, "ppc1"),
			m_ppc2(*this, "ppc2"),
			m_lsi53c810(*this, "lsi53c810"),
			m_dma8237_1(*this, "dma8237_1"),
			m_dma8237_2(*this, "dma8237_2"),
			m_pic8259_1(*this, "pic8259_1"),
			m_pic8259_2(*this, "pic8259_2"),
			m_pit8254(*this, "pit8254"),
			m_ram(*this, RAM_TAG),
			m_smc37c78(*this, "smc37c78"),
			m_flash(*this, "flash"),
			m_pcibus(*this, "pcibus")
	{
	}

	required_device<cpu_device> m_ppc1;
	required_device<cpu_device> m_ppc2;
	required_device<lsi53c810_device> m_lsi53c810;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pic8259_device> m_pic8259_1;
	required_device<pic8259_device> m_pic8259_2;
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
	void bebox_pic8259_master_set_int_line(int state);
	void bebox_pic8259_slave_set_int_line(int state);
	uint8_t get_slave_ack(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bebox_dma_hrq_changed(int state);
	uint8_t bebox_dma8237_fdc_dack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bebox_dma8237_fdc_dack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bebox_dma8237_out_eop(int state);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	void bebox_timer0_w(int state);
	uint64_t bebox_cpu0_imask_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t bebox_cpu1_imask_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t bebox_interrupt_sources_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t bebox_crossproc_interrupts_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t bebox_interrupt_ack_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint8_t bebox_page_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bebox_80000480_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t bebox_flash_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void bebox_cpu0_imask_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void bebox_cpu1_imask_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void bebox_crossproc_interrupts_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void bebox_processor_resets_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void bebox_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bebox_80000480_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bebox_flash_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t at_dma8237_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void at_dma8237_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bebox_dma_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bebox_dma_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint64_t scsi53c810_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void scsi53c810_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t bb_slave_64be_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));

	void bebox_ide_interrupt(int state);

	void bebox_keyboard_interrupt(int state);

	void fdc_interrupt(int state);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	LSI53C810_FETCH_CB(scsi_fetch);
	LSI53C810_IRQ_CB(scsi_irq_callback);
	LSI53C810_DMA_CB(scsi_dma_callback);

	void bebox_set_irq_bit(unsigned int interrupt_bit, int val);
	void bebox_update_interrupts();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


/*----------- defined in machine/bebox.c -----------*/

uint32_t scsi53c810_pci_read(device_t *busdevice, device_t *device, int function, int offset, uint32_t mem_mask);
void scsi53c810_pci_write(device_t *busdevice, device_t *device, int function, int offset, uint32_t data, uint32_t mem_mask);

#endif /* BEBOX_H_ */
