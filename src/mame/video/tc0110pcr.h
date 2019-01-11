// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0110PCR_H
#define MAME_VIDEO_TC0110PCR_H

#pragma once

#include "emupal.h"


class tc0110pcr_device : public device_t
{
public:
	template <typename T> tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag)
		: tc0110pcr_device(mconfig, tag, owner, clock)
	{
		m_palette.set_tag(std::forward<T>(palette_tag));
	}

	tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w ); /* color index goes up in step of 2 */
	DECLARE_WRITE16_MEMBER( step1_word_w );   /* color index goes up in step of 1 */
	DECLARE_WRITE16_MEMBER( step1_rbswap_word_w );    /* swaps red and blue components */
	DECLARE_WRITE16_MEMBER( step1_4bpg_word_w );  /* only 4 bits per color gun */

	void restore_colors();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	std::unique_ptr<uint16_t[]>     m_ram;
	int          m_type;
	int          m_addr;
	required_device<palette_device> m_palette;
};

DECLARE_DEVICE_TYPE(TC0110PCR, tc0110pcr_device)

#endif // MAME_VIDEO_TC0110PCR_H
