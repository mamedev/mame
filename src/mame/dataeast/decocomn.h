// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    decocomn.h

**************************************************************************/
#ifndef MAME_DATAEAST_DECOCOMN_H
#define MAME_DATAEAST_DECOCOMN_H

#pragma once

#include "emupal.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


class decocomn_device : public device_t,
						public device_video_interface
{
public:
	decocomn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }

	void buffered_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_dma_w(uint16_t data);
	void priority_w(uint16_t data);
	uint16_t priority_r();
	uint16_t d_71_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	std::unique_ptr<uint8_t[]> m_dirty_palette;
	uint16_t m_priority;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_generic_paletteram_16;
};

DECLARE_DEVICE_TYPE(DECOCOMN, decocomn_device)


#endif // MAME_DATAEAST_DECOCOMN_H
