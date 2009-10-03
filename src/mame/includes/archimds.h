/******************************************************************************
 *
 *  Acorn Archimedes custom chips (IOC, MEMC, VIDC)
 *
 *****************************************************************************/

#ifndef _ARCHIMEDES_H_
#define _ARCHIMEDES_H_

// interrupt definitions.  these are for the real Archimedes computer - arcade
// and gambling knockoffs likely are a bit different.

#define ARCHIMEDES_IRQA_PRINTER_BUSY (0x01)
#define ARCHIMEDES_IRQA_SERIAL_RING  (0x02)
#define ARCHIMEDES_IRQA_PRINTER_ACK  (0x04)
#define ARCHIMEDES_IRQA_VBL	       (0x08)
#define ARCHIMEDES_IRQA_RESET        (0x10)
#define ARCHIMEDES_IRQA_TIMER0       (0x20)
#define ARCHIMEDES_IRQA_TIMER1       (0x40)
#define ARCHIMEDES_IRQA_ALWAYS       (0x80)

#define ARCHIMEDES_IRQB_PODULE_FIQ   (0x01)
#define ARCHIMEDES_IRQB_SOUND_EMPTY  (0x02)
#define ARCHIMEDES_IRQB_SERIAL       (0x04)
#define ARCHIMEDES_IRQB_HDD	       (0x08)
#define ARCHIMEDES_IRQB_DISC_CHANGE  (0x10)
#define ARCHIMEDES_IRQB_PODULE_IRQ   (0x20)
#define ARCHIMEDES_IRQB_KBD_XMIT_EMPTY  (0x40)
#define ARCHIMEDES_IRQB_KBD_RECV_FULL   (0x80)

#define ARCHIMEDES_FIQ_FLOPPY_DRQ    (0x01)
#define ARCHIMEDES_FIQ_FLOPPY        (0x02)
#define ARCHIMEDES_FIQ_ECONET        (0x04)
#define ARCHIMEDES_FIQ_PODULE        (0x40)
#define ARCHIMEDES_FIQ_FORCE         (0x80)

/*----------- defined in machine/archimds.c -----------*/

extern UINT32 *archimedes_memc_physmem;

void archimedes_init(running_machine *machine);			// call at MACHINE_INIT
void archimedes_reset(running_machine *machine);		// call at MACHINE_RESET
void archimedes_driver_init(running_machine *machine);		// call at DRIVER_INIT

void archimedes_request_irq_a(running_machine *machine, int mask);
void archimedes_request_irq_b(running_machine *machine, int mask);
void archimedes_request_fiq(running_machine *machine, int mask);
void archimedes_clear_irq_a(running_machine *machine, int mask);
void archimedes_clear_irq_b(running_machine *machine, int mask);
void archimedes_clear_fiq(running_machine *machine, int mask);

extern READ32_HANDLER(archimedes_memc_logical_r);
extern WRITE32_HANDLER(archimedes_memc_logical_w);
extern READ32_HANDLER(archimedes_memc_r);
extern WRITE32_HANDLER(archimedes_memc_w);
extern WRITE32_HANDLER(archimedes_memc_page_w);
extern READ32_HANDLER(archimedes_ioc_r);
extern WRITE32_HANDLER(archimedes_ioc_w);
extern READ32_HANDLER(archimedes_vidc_r);
extern WRITE32_HANDLER(archimedes_vidc_w);

#endif	// _ARCHIMEDES_H_
