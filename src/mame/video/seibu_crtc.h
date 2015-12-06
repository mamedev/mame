// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    Seibu CRTC device

***************************************************************************/

#pragma once

#ifndef __SEIBU_CRTCDEV_H__
#define __SEIBU_CRTCDEV_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEIBU_CRTC_LAYER_EN_CB(_devcb) \
	devcb = &seibu_crtc_device::set_layer_en_callback(*device, DEVCB_##_devcb);

#define MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(_devcb) \
	devcb = &seibu_crtc_device::set_layer_scroll_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> seibu_crtc_device

class seibu_crtc_device : public device_t,
							public device_memory_interface,
							public device_video_interface
{
public:
	// construction/destruction
	seibu_crtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_layer_en_callback(device_t &device, _Object object) { return downcast<seibu_crtc_device &>(device).m_layer_en_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_layer_scroll_callback(device_t &device, _Object object) { return downcast<seibu_crtc_device &>(device).m_layer_scroll_cb.set_callback(object); }

	// I/O operations
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_WRITE16_MEMBER( write_alt );
	DECLARE_WRITE16_MEMBER( write_xor );
	DECLARE_READ16_MEMBER( read );
	DECLARE_READ16_MEMBER( read_alt );
	DECLARE_READ16_MEMBER( read_xor );
	DECLARE_WRITE16_MEMBER(layer_en_w);
	DECLARE_WRITE16_MEMBER(layer_scroll_w);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	devcb_write16       m_layer_en_cb;
	devcb_write16       m_layer_scroll_cb;
	const address_space_config      m_space_config;
	inline UINT16 read_word(offs_t address);
	inline void write_word(offs_t address, UINT16 data);
};


// device type definition
extern const device_type SEIBU_CRTC;

#endif
