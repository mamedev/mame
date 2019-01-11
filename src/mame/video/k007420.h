// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_VIDEO_K007420_H
#define MAME_VIDEO_K007420_H

#pragma once


typedef device_delegate<void (int *code, int *color)> k007420_delegate;

class k007420_device : public device_t
{
public:
	k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_palette_tag(const char *tag) { m_palette.set_tag(tag); }
	void set_bank_limit(int limit) { m_banklimit = limit; }
	void set_callback(k007420_delegate callback) { m_callback = callback; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	std::unique_ptr<uint8_t[]>        m_ram;

	int          m_flipscreen;    // current code uses the 7342 flipscreen!!
	uint8_t        m_regs[8];   // current code uses the 7342 regs!! (only [2])
	required_device<palette_device> m_palette;
	int                m_banklimit;
	k007420_delegate m_callback;
};

DECLARE_DEVICE_TYPE(K007420, k007420_device)

#define MCFG_K007420_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K007420, 0)

#define MCFG_K007420_PALETTE(_palette_tag) \
	downcast<k007420_device &>(*device).set_palette_tag(_palette_tag);

#define MCFG_K007420_BANK_LIMIT(_limit) \
	downcast<k007420_device &>(*device).set_bank_limit(_limit);

#define MCFG_K007420_CALLBACK_OWNER(_class, _method) \
	downcast<k007420_device &>(*device).set_callback(k007420_delegate(&_class::_method, #_class "::" #_method, this));

// function definition for a callback
#define K007420_CALLBACK_MEMBER(_name)     void _name(int *code, int *color)


#endif // MAME_VIDEO_K007420_H
