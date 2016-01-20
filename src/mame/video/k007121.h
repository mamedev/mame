// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#pragma once
#ifndef __K007121_H__
#define __K007121_H__

class k007121_device : public device_t
{
public:
	k007121_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k007121_device() {}

	static void static_set_palette_tag(device_t &device, std::string tag);

	DECLARE_READ8_MEMBER( ctrlram_r );
	DECLARE_WRITE8_MEMBER( ctrl_w );

	/* shall we move source in the interface? */
	/* also notice that now we directly pass *gfx[chip] instead of **gfx !! */
	void sprites_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, palette_device *palette, const UINT8 *source, int base_color, int global_x_offset, int bank_base, bitmap_ind8 &priority_bitmap, UINT32 pri_mask );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT8    m_ctrlram[8];
	int      m_flipscreen;
	required_device<palette_device> m_palette;
};

extern const device_type K007121;

#define MCFG_K007121_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K007121, 0)

#define MCFG_K007121_PALETTE(_palette_tag) \
	k007121_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
