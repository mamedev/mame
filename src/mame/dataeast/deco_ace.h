// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    deco_ace.h

    Data East 99 "ACE" Chip Emulation

    Original source (from deco32.cpp) by Bryan McPhail, split by cam900.

**************************************************************************/
#ifndef MAME_DATAEAST_DECO_ACE_H
#define MAME_DATAEAST_DECO_ACE_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


class deco_ace_device : public device_t, public device_video_interface, public device_palette_interface
{
public:
	deco_ace_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t buffered_palette_r(offs_t offset);
	uint16_t buffered_palette16_r(offs_t offset);
	uint16_t ace_r(offs_t offset);
	void buffered_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void buffered_palette16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ace_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_update();
	uint16_t get_aceram(uint8_t val);
	uint8_t get_alpha(uint8_t val);
	void palette_dma_w(uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 2048 * 2; }

private:
	// internal state
	std::unique_ptr<uint32_t[]> m_paletteram;
	std::unique_ptr<uint32_t[]> m_paletteram_buffered;
	std::unique_ptr<uint16_t[]> m_ace_ram;
};

DECLARE_DEVICE_TYPE(DECO_ACE, deco_ace_device)


#endif // MAME_DATAEAST_DECO_ACE_H
