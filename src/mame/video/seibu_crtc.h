// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    Seibu CRTC device

***************************************************************************/

#ifndef MAME_VIDEO_SEIBU_CRTC_H
#define MAME_VIDEO_SEIBU_CRTC_H

#pragma once


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
	seibu_crtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto decrypt_key_callback() { return m_decrypt_key_cb.bind(); }
	auto layer_en_callback() { return m_layer_en_cb.bind(); }
	auto layer_scroll_callback() { return m_layer_scroll_cb.bind(); }
	auto reg_1a_callback() { return m_reg_1a_cb.bind(); }
	auto layer_scroll_base_callback() { return m_layer_scroll_base_cb.bind(); }

	// I/O operations
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_WRITE16_MEMBER( write_alt );
	DECLARE_READ16_MEMBER( read );
	DECLARE_READ16_MEMBER( read_alt );
	DECLARE_WRITE16_MEMBER(decrypt_key_w);
	DECLARE_WRITE16_MEMBER(layer_en_w);
	DECLARE_READ16_MEMBER(reg_1a_r);
	DECLARE_WRITE16_MEMBER(reg_1a_w);
	DECLARE_WRITE16_MEMBER(layer_scroll_w);
	DECLARE_WRITE16_MEMBER(layer_scroll_base_w);

	void seibu_crtc_vregs(address_map &map);
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;

private:
	devcb_write16       m_decrypt_key_cb;
	devcb_write16       m_layer_en_cb;
	devcb_write16       m_layer_scroll_cb;
	devcb_write16       m_reg_1a_cb;
	devcb_write16       m_layer_scroll_base_cb;
	const address_space_config      m_space_config;
	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	uint16_t m_reg_1a;
};


// device type definition
DECLARE_DEVICE_TYPE(SEIBU_CRTC, seibu_crtc_device)

#endif // MAME_VIDEO_SEIBU_CRTC_H
