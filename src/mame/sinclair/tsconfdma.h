// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

    TS-Conf (ZX-Evolution) DMA

***************************************************************************/

#ifndef MAME_SINCLAIR_TSCONF_DMA_H
#define MAME_SINCLAIR_TSCONF_DMA_H

#pragma once

class tsconfdma_device : public device_t
{
public:
	tsconfdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto in_mreq_callback() { return m_in_mreq_cb.bind(); }
	auto out_mreq_callback() { return m_out_mreq_cb.bind(); }
	auto in_spireq_callback() { return m_in_mspi_cb.bind(); }
	auto out_cram_callback() { return m_out_cram_cb.bind(); }
	auto out_sfile_callback() { return m_out_sfile_cb.bind(); }
	auto on_ready_callback() { return m_on_ready_cb.bind(); }

	int is_ready();

	void set_saddr_l(uint8_t addr_l);
	void set_saddr_h(uint8_t addr_h);
	void set_saddr_x(uint8_t addr_x);
	void set_daddr_l(uint8_t addr_l);
	void set_daddr_h(uint8_t addr_h);
	void set_daddr_x(uint8_t addr_x);
	void set_block_len(uint8_t len);
	void set_block_num_l(uint8_t num_l);
	void set_block_num_h(uint8_t num_h);
	void start_tx(uint8_t task, bool s_align, bool d_align, bool blitting_opt);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_read16 m_in_mreq_cb;
	devcb_write16 m_out_mreq_cb;
	devcb_read16 m_in_mspi_cb;
	devcb_write16 m_out_cram_cb;
	devcb_write16 m_out_sfile_cb;
	devcb_write_line m_on_ready_cb;

private:
	TIMER_CALLBACK_MEMBER(dma_clock);

	u8 m_ready;

	offs_t m_address_s;
	offs_t m_address_d;
	u8 m_block_len;
	u16 m_block_num;

	offs_t m_tx_s_addr;
	offs_t m_tx_d_addr;
	u16 m_tx_data;
	u16 m_tx_block_num;
	u16 m_tx_block;

	emu_timer *m_dma_clock;
	u8 m_task;
	bool m_align_s;
	bool m_align_d;
	bool m_asz;
	u16 m_align;
	u32 m_m1;
	u32 m_m2;
};

DECLARE_DEVICE_TYPE(TSCONF_DMA, tsconfdma_device)
#endif // MAME_SINCLAIR_TSCONF_DMA_H
