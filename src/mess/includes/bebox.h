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
	UINT32 m_cpu_imask[2];
	UINT32 m_interrupts;
	UINT32 m_crossproc_interrupts;
	int m_dma_channel;
	UINT16 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT32 m_scsi53c810_data[0x100 / 4];
	DECLARE_DRIVER_INIT(bebox);
	virtual void machine_start();
	virtual void machine_reset();
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
	DECLARE_WRITE8_MEMBER(bebox_800001F0_w);
	DECLARE_WRITE64_MEMBER(bebox_800003F0_w);
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

	LSI53C810_FETCH_CB(scsi_fetch);
	LSI53C810_IRQ_CB(scsi_irq_callback);
	LSI53C810_DMA_CB(scsi_dma_callback);

	void bebox_set_irq_bit(unsigned int interrupt_bit, int val);
	void bebox_update_interrupts();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};


/*----------- defined in machine/bebox.c -----------*/

UINT32 scsi53c810_pci_read(device_t *busdevice, device_t *device, int function, int offset, UINT32 mem_mask);
void scsi53c810_pci_write(device_t *busdevice, device_t *device, int function, int offset, UINT32 data, UINT32 mem_mask);

#endif /* BEBOX_H_ */
