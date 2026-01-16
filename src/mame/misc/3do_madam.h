// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol

#ifndef MAME_MISC_3DO_MADAM_H
#define MAME_MISC_3DO_MADAM_H

#pragma once

class madam_device : public device_t
{
public:
	// construction/destruction
	madam_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto diag_cb()            { return m_diag_cb.bind(); }
	auto dma_read_cb()        { return m_dma_read_cb.bind(); }
	auto dma_write_cb()       { return m_dma_write_cb.bind(); }
	auto irq_dply_cb()        { return m_irq_dply_cb.bind(); }

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	devcb_write8     m_diag_cb;
	devcb_read32     m_dma_read_cb;
	devcb_write32    m_dma_write_cb;
	devcb_write_line m_irq_dply_cb;

	uint32_t  m_revision = 0;       /* 03300000 */
	uint32_t  m_msysbits = 0;       /* 03300004 */
	uint32_t  m_mctl = 0;           /* 03300008 */
	uint32_t  m_sltime = 0;         /* 0330000c */
	uint32_t  m_abortbits = 0;      /* 03300020 */
	uint32_t  m_privbits = 0;       /* 03300024 */
	uint32_t  m_statbits = 0;       /* 03300028 */
	uint32_t  m_diag = 0;           /* 03300040 */

	uint32_t  m_ccobctl0 = 0;       /* 03300110 */
	uint32_t  m_ppmpc = 0;          /* 03300120 */

	uint32_t  m_regctl0 = 0;        /* 03300130 */
	uint32_t  m_regctl1 = 0;        /* 03300134 */
	uint32_t  m_regctl2 = 0;        /* 03300138 */
	uint32_t  m_regctl3 = 0;        /* 0330013c */
	uint32_t  m_xyposh = 0;         /* 03300140 */
	uint32_t  m_xyposl = 0;         /* 03300144 */
	uint32_t  m_linedxyh = 0;       /* 03300148 */
	uint32_t  m_linedxyl = 0;       /* 0330014c */
	uint32_t  m_dxyh = 0;           /* 03300150 */
	uint32_t  m_dxyl = 0;           /* 03300154 */
	uint32_t  m_ddxyh = 0;          /* 03300158 */
	uint32_t  m_ddxyl = 0;          /* 0330015c */

	uint32_t  m_pip[16]{};          /* 03300180-033001bc (W); 03300180-033001fc (R) */
	uint32_t  m_fence[4]{};         /* 03300200-0330023c (W); 03300200-0330027c (R) */
	uint32_t  m_mmu[64]{};          /* 03300300-033003fc */
	uint32_t  m_dma[32][4]{};       /* 03300400-033005fc */
	uint32_t  m_mult[40]{};         /* 03300600-0330069c */
	uint32_t  m_mult_control = 0;   /* 033007f0-033007f4 */
	uint32_t  m_mult_status = 0;    /* 033007f8 */

	u32 mctl_r();
	void mctl_w(offs_t offset, u32 data, u32 mem_mask);

	TIMER_CALLBACK_MEMBER(dma_playerbus_cb);
	emu_timer *m_dma_playerbus_timer;
};

DECLARE_DEVICE_TYPE(MADAM, madam_device)


#endif // MAME_MISC_3DO_MADAM_H
