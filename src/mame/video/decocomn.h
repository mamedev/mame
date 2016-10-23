// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    decocomn.h

**************************************************************************/

#pragma once
#ifndef __DECOCOMN_H__
#define __DECOCOMN_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


class decocomn_device : public device_t,
						public device_video_interface
{
public:
	decocomn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~decocomn_device() {}

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);

	void nonbuffered_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void buffered_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void palette_dma_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void priority_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t priority_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t d_71_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<uint8_t[]> m_dirty_palette;
	uint16_t m_priority;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_generic_paletteram_16;
};

extern const device_type DECOCOMN;



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DECOCOMN_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECOCOMN, 0)

#define MCFG_DECOCOMN_PALETTE(_palette_tag) \
	decocomn_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
