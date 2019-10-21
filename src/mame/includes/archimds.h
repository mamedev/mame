// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller
/******************************************************************************
 *
 *  Acorn Archimedes custom chips (IOC, MEMC, VIDC)
 *
 *****************************************************************************/

#ifndef MAME_INCLUDES_ARCHIMEDES_H
#define MAME_INCLUDES_ARCHIMEDES_H

#include "cpu/arm/arm.h"
#include "imagedev/floppy.h"
#include "machine/aakart.h"
#include "machine/i2cmem.h"
#include "machine/wd_fdc.h"
#include "machine/acorn_vidc.h"

// interrupt definitions.  these are for the real Archimedes computer - arcade
// and gambling knockoffs likely are a bit different.

#define ARCHIMEDES_IRQA_PRINTER_BUSY (0x01)
#define ARCHIMEDES_IRQA_SERIAL_RING  (0x02)
#define ARCHIMEDES_IRQA_PRINTER_ACK  (0x04)
#define ARCHIMEDES_IRQA_VBL          (0x08)
#define ARCHIMEDES_IRQA_RESET        (0x10)
#define ARCHIMEDES_IRQA_TIMER0       (0x20)
#define ARCHIMEDES_IRQA_TIMER1       (0x40)
#define ARCHIMEDES_IRQA_FORCE        (0x80)

#define ARCHIMEDES_IRQB_PODULE_FIQ   (0x01)
#define ARCHIMEDES_IRQB_SOUND_EMPTY  (0x02)
#define ARCHIMEDES_IRQB_SERIAL       (0x04)
#define ARCHIMEDES_IRQB_HDD        (0x08)
#define ARCHIMEDES_IRQB_DISC_CHANGE  (0x10)
#define ARCHIMEDES_IRQB_PODULE_IRQ   (0x20)
#define ARCHIMEDES_IRQB_KBD_XMIT_EMPTY  (0x40)
#define ARCHIMEDES_IRQB_KBD_RECV_FULL   (0x80)

#define ARCHIMEDES_FIQ_FLOPPY_DRQ    (0x01)
#define ARCHIMEDES_FIQ_FLOPPY        (0x02)
#define ARCHIMEDES_FIQ_ECONET        (0x04)
#define ARCHIMEDES_FIQ_PODULE        (0x40)
#define ARCHIMEDES_FIQ_FORCE         (0x80)

class archimedes_state : public driver_device
{
public:
	archimedes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_kart(*this, "kart"),
		m_maincpu(*this, "maincpu"),
		m_i2cmem(*this, "i2cmem"),
		m_vidc(*this, "vidc"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_region_maincpu(*this, "maincpu"),
		m_joy(*this, "joy_p%u",1)
		{ }

	optional_device<aakart_device> m_kart;
	void archimedes_init();
	void archimedes_reset();
	void archimedes_driver_init();

	void archimedes_request_irq_a(int mask);
	void archimedes_request_irq_b(int mask);
	void archimedes_request_fiq(int mask);
	void archimedes_clear_irq_a(int mask);
	void archimedes_clear_irq_b(int mask);
	void archimedes_clear_fiq(int mask);

	DECLARE_READ32_MEMBER(aristmk5_drame_memc_logical_r);
	DECLARE_READ32_MEMBER(archimedes_memc_logical_r);
	DECLARE_WRITE32_MEMBER(archimedes_memc_logical_w);
	DECLARE_WRITE32_MEMBER(archimedes_memc_w);
	DECLARE_WRITE32_MEMBER(archimedes_memc_page_w);
	DECLARE_READ32_MEMBER(archimedes_ioc_r);
	DECLARE_WRITE32_MEMBER(archimedes_ioc_w);
	DECLARE_WRITE_LINE_MEMBER( a310_kart_rx_w );
	DECLARE_WRITE_LINE_MEMBER( a310_kart_tx_w );

	uint8_t m_i2c_clk;
	int16_t m_memc_pages[0x2000]; // the logical RAM area is 32 megs, and the smallest page size is 4k
	uint8_t m_ioc_regs[0x80/4];

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

protected:
	required_device<arm_cpu_device> m_maincpu;
	optional_device<i2cmem_device> m_i2cmem;
	required_device<acorn_vidc10_device> m_vidc;
	optional_device<wd1772_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	required_memory_region m_region_maincpu;
	optional_ioport_array<2> m_joy;

	DECLARE_WRITE_LINE_MEMBER( vblank_irq );
	DECLARE_WRITE_LINE_MEMBER( sound_drq );

private:

	static const device_timer_id TIMER_IOC = 3;

//  void vidc_vblank();
	void vidc_video_tick();
	void vidc_audio_tick();
	void ioc_timer(int param);

	void latch_timer_cnt(int tmr);
	void a310_set_timer(int tmr);
	DECLARE_READ32_MEMBER(ioc_ctrl_r);
	DECLARE_WRITE32_MEMBER(ioc_ctrl_w);

	uint32_t *m_archimedes_memc_physmem;
	uint32_t m_memc_pagesize;
	int m_memc_latchrom;
	uint32_t m_ioc_timercnt[4], m_ioc_timerout[4];
	uint32_t m_vidc_vidstart, m_vidc_vidend, m_vidc_vidinit, m_vidc_vidcur, m_vidc_cinit;
	uint32_t m_vidc_sndstart, m_vidc_sndend, m_vidc_sndcur, m_vidc_sndendcur;
	uint8_t m_video_dma_on,m_audio_dma_on;
	bool m_cursor_enabled;
	emu_timer *m_timer[4];
	uint8_t m_floppy_select;
	bool check_floppy_ready();
	uint8_t m_joy_serial_data;
};

/* IOC registers */

#define CONTROL         0x00/4
#define KART            0x04/4 // Keyboard Asynchronous Receiver Transmitter

#define IRQ_STATUS_A    0x10/4
#define IRQ_REQUEST_A   0x14/4
#define IRQ_MASK_A      0x18/4
#define IRQ_STATUS_B    0x20/4
#define IRQ_REQUEST_B   0x24/4
#define IRQ_MASK_B      0x28/4

#define FIQ_STATUS      0x30/4
#define FIQ_REQUEST     0x34/4
#define FIQ_MASK        0x38/4

#define T0_LATCH_LO 0x40/4
#define T0_LATCH_HI 0x44/4
#define T0_GO       0x48/4
#define T0_LATCH    0x4c/4

#define T1_LATCH_LO 0x50/4
#define T1_LATCH_HI 0x54/4
#define T1_GO       0x58/4
#define T1_LATCH    0x5c/4

#define T2_LATCH_LO 0x60/4
#define T2_LATCH_HI 0x64/4
#define T2_GO       0x68/4
#define T2_LATCH    0x6c/4

#define T3_LATCH_LO 0x70/4
#define T3_LATCH_HI 0x74/4
#define T3_GO       0x78/4
#define T3_LATCH    0x7c/4

#endif // MAME_INCLUDES_ARCHIMEDES_H
