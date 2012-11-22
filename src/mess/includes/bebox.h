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
#include "machine/ins8250.h"
#include "machine/8237dma.h"
#include "machine/53c810.h"
#include "machine/upd765.h"

struct bebox_devices_t
{
	device_t *pic8259_master;
	device_t *pic8259_slave;
	device_t *dma8237_1;
	device_t *dma8237_2;
};


class bebox_state : public driver_device
{
public:
	bebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_lsi53c810(*this, "scsi:lsi53c810"){ }

	required_device<lsi53c810_device> m_lsi53c810;
	UINT32 m_cpu_imask[2];
	UINT32 m_interrupts;
	UINT32 m_crossproc_interrupts;
	bebox_devices_t m_devices;
	int m_dma_channel;
	UINT16 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT32 m_scsi53c810_data[0x100 / 4];
	DECLARE_DRIVER_INIT(bebox);
	virtual void machine_start();
	virtual void machine_reset();
	TIMER_CALLBACK_MEMBER(bebox_get_devices);
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
	DECLARE_READ8_MEMBER(bebox_800001F0_r);
	DECLARE_READ64_MEMBER(bebox_800003F0_r);
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

	void fdc_interrupt(bool state);
	void fdc_dma_drq(bool state);
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};


/*----------- defined in machine/bebox.c -----------*/

extern const struct pit8253_config bebox_pit8254_config;
extern const i8237_interface bebox_dma8237_1_config;
extern const i8237_interface bebox_dma8237_2_config;
extern const struct pic8259_interface bebox_pic8259_master_config;
extern const struct pic8259_interface bebox_pic8259_slave_config;
extern const ins8250_interface bebox_uart_inteface_0;
extern const ins8250_interface bebox_uart_inteface_1;
extern const ins8250_interface bebox_uart_inteface_2;
extern const ins8250_interface bebox_uart_inteface_3;

void bebox_set_irq_bit(running_machine &machine, unsigned int interrupt_bit, int val);

UINT32 scsi53c810_pci_read(device_t *busdevice, device_t *device, int function, int offset, UINT32 mem_mask);
void scsi53c810_pci_write(device_t *busdevice, device_t *device, int function, int offset, UINT32 data, UINT32 mem_mask);

#endif /* BEBOX_H_ */
