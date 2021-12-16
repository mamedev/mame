// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

    TS-Conf (ZX-Evolution) DMA

***************************************************************************/

#ifndef MAME_MACHINE_TSCONF_DMA_H
#define MAME_MACHINE_TSCONF_DMA_H

#pragma once

DECLARE_DEVICE_TYPE(TSCONF_DMA, tsconfdma_device)

class tsconfdma_device : public device_t
{
public:
    tsconfdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto in_mreq_callback() { return m_in_mreq_cb.bind(); }
	auto out_mreq_callback() { return m_out_mreq_cb.bind(); }
    auto in_spireq_callback() { return m_in_mspi_cb.bind(); }
	auto out_cram_callback() { return m_out_cram_cb.bind(); }

    bool is_ready();
    void set_saddr_l(uint8_t addr_l);
    void set_saddr_h(uint8_t addr_h);
    void set_saddr_x(uint8_t addr_x);
    void set_daddr_l(uint8_t addr_l);
    void set_daddr_h(uint8_t addr_h);
    void set_daddr_x(uint8_t addr_x);
    void set_block_len(uint8_t len);
    void set_block_num_l(uint8_t num_l);
    void set_block_num_h(uint8_t num_h);
    void start_tx(uint8_t dev, bool s_align, bool d_align, bool blitting_opt);
private:
	virtual void device_start() override;
	virtual void device_reset() override;

    devcb_read16    m_in_mreq_cb;
    devcb_write16   m_out_mreq_cb;
    devcb_read16    m_in_mspi_cb;
    devcb_write16   m_out_cram_cb;

    bool m_ready;

    offs_t m_address_s;
    offs_t m_address_d;
    uint8_t m_block_len;
    uint16_t m_block_num;
    bool m_align_s;
    bool m_align_d;
    uint16_t m_align;
};

#endif // MAME_MACHINE_TSCONF_DMA_H