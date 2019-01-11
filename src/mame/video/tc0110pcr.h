// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0110PCR_H
#define MAME_VIDEO_TC0110PCR_H

#pragma once


class tc0110pcr_device : public device_t
{
public:
	tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w ); /* color index goes up in step of 2 */
	DECLARE_WRITE16_MEMBER( step1_word_w );   /* color index goes up in step of 1 */
	DECLARE_WRITE16_MEMBER( step1_rbswap_word_w );    /* swaps red and blue components */
	DECLARE_WRITE16_MEMBER( step1_4bpg_word_w );  /* only 4 bits per color gun */

	void restore_colors();

	void set_palette_tag(const char *tag) { m_palette.set_tag(tag); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::unique_ptr<uint16_t[]>     m_ram;
	int          m_type;
	int          m_addr;
	required_device<palette_device> m_palette;
};

DECLARE_DEVICE_TYPE(TC0110PCR, tc0110pcr_device)

#define MCFG_TC0110PCR_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TC0110PCR, 0)
#define MCFG_TC0110PCR_PALETTE(_palette_tag) \
	downcast<tc0110pcr_device &>(*device).set_palette_tag(_palette_tag);

#endif // MAME_VIDEO_TC0110PCR_H
