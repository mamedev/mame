// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_TC0110PCR_H
#define MAME_TAITO_TC0110PCR_H

#pragma once


class tc0110pcr_device : public device_t, public device_palette_interface
{
public:
	tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u16 word_r(offs_t offset);
	void word_w(offs_t offset, u16 data); /* color index goes up in step of 2 */
	void step1_word_w(offs_t offset, u16 data);   /* color index goes up in step of 1 */
	void step1_rbswap_word_w(offs_t offset, u16 data);    /* swaps red and blue components */
	void step1_4bpg_word_w(offs_t offset, u16 data);  /* only 4 bits per color gun */

	void restore_colors();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_palette_interface overrides
	virtual u32 palette_entries() const noexcept override { return TC0110PCR_RAM_SIZE; }

private:
	static const unsigned TC0110PCR_RAM_SIZE = 0x2000 / 2;

	std::unique_ptr<uint16_t[]>     m_ram;
	int          m_type;
	int          m_addr;
};

DECLARE_DEVICE_TYPE(TC0110PCR, tc0110pcr_device)

#endif // MAME_TAITO_TC0110PCR_H
