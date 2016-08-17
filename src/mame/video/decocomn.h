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
	decocomn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~decocomn_device() {}

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);

	DECLARE_WRITE16_MEMBER( nonbuffered_palette_w );
	DECLARE_WRITE16_MEMBER( buffered_palette_w );
	DECLARE_WRITE16_MEMBER( palette_dma_w );
	DECLARE_WRITE16_MEMBER( priority_w );
	DECLARE_READ16_MEMBER( priority_r );
	DECLARE_READ16_MEMBER( d_71_r );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<UINT8[]> m_dirty_palette;
	UINT16 m_priority;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT16> m_generic_paletteram_16;
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
