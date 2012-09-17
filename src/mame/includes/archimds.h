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
#define ARCHIMEDES_IRQA_VBL	         (0x08)
#define ARCHIMEDES_IRQA_RESET        (0x10)
#define ARCHIMEDES_IRQA_TIMER0       (0x20)
#define ARCHIMEDES_IRQA_TIMER1       (0x40)
#define ARCHIMEDES_IRQA_FORCE        (0x80)

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

void archimedes_init(running_machine &machine);			// call at MACHINE_INIT
void archimedes_reset(running_machine &machine);		// call at MACHINE_RESET
void archimedes_driver_init(running_machine &machine);		// call at DRIVER_INIT

void archimedes_request_irq_a(running_machine &machine, int mask);
void archimedes_request_irq_b(running_machine &machine, int mask);
void archimedes_request_fiq(running_machine &machine, int mask);
void archimedes_clear_irq_a(running_machine &machine, int mask);
void archimedes_clear_irq_b(running_machine &machine, int mask);
void archimedes_clear_fiq(running_machine &machine, int mask);

extern DECLARE_READ32_HANDLER(aristmk5_drame_memc_logical_r);
extern DECLARE_READ32_HANDLER(archimedes_memc_logical_r);
extern DECLARE_WRITE32_HANDLER(archimedes_memc_logical_w);
extern DECLARE_READ32_HANDLER(archimedes_memc_r);
extern DECLARE_WRITE32_HANDLER(archimedes_memc_w);
extern DECLARE_WRITE32_HANDLER(archimedes_memc_page_w);
extern DECLARE_READ32_HANDLER(archimedes_ioc_r);
extern DECLARE_WRITE32_HANDLER(archimedes_ioc_w);
extern DECLARE_READ32_HANDLER(archimedes_vidc_r);
extern DECLARE_WRITE32_HANDLER(archimedes_vidc_w);

extern UINT8 i2c_clk;
extern INT16 memc_pages[0x2000];	// the logical RAM area is 32 megs, and the smallest page size is 4k
extern UINT32 vidc_regs[256];
extern UINT8 ioc_regs[0x80/4];
extern UINT8 vidc_bpp_mode;
extern UINT8 vidc_interlace;

/* IOC registers */

#define CONTROL			0x00/4
#define KART			0x04/4 // Keyboard Asynchronous Receiver Transmitter

#define IRQ_STATUS_A	0x10/4
#define IRQ_REQUEST_A   0x14/4
#define IRQ_MASK_A		0x18/4
#define IRQ_STATUS_B	0x20/4
#define IRQ_REQUEST_B   0x24/4
#define IRQ_MASK_B		0x28/4

#define FIQ_STATUS		0x30/4
#define FIQ_REQUEST     0x34/4
#define FIQ_MASK		0x38/4

#define T0_LATCH_LO	0x40/4
#define T0_LATCH_HI 0x44/4
#define T0_GO		0x48/4
#define T0_LATCH	0x4c/4

#define T1_LATCH_LO	0x50/4
#define T1_LATCH_HI 0x54/4
#define T1_GO		0x58/4
#define T1_LATCH	0x5c/4

#define T2_LATCH_LO	0x60/4
#define T2_LATCH_HI 0x64/4
#define T2_GO		0x68/4
#define T2_LATCH	0x6c/4

#define T3_LATCH_LO	0x70/4
#define T3_LATCH_HI 0x74/4
#define T3_GO		0x78/4
#define T3_LATCH	0x7c/4


/*----------- defined in video/archimds.c -----------*/

extern VIDEO_START( archimds_vidc );
extern SCREEN_UPDATE_RGB32( archimds_vidc );

#define VIDC_HCR		0x80
#define VIDC_HSWR		0x84
#define VIDC_HBSR		0x88
#define VIDC_HDSR		0x8c
#define VIDC_HDER		0x90
#define VIDC_HBER		0x94
#define VIDC_HCSR		0x98
#define VIDC_HIR		0x9c

#define VIDC_VCR		0xa0
#define VIDC_VSWR		0xa4
#define VIDC_VBSR		0xa8
#define VIDC_VDSR		0xac
#define VIDC_VDER		0xb0
#define VIDC_VBER		0xb4
#define VIDC_VCSR		0xb8
#define VIDC_VCER		0xbc

#endif	// _ARCHIMEDES_H_
