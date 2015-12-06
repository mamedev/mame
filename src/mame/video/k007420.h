// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#pragma once
#ifndef __K007420_H__
#define __K007420_H__

typedef device_delegate<void (int *code, int *color)> k007420_delegate;

class k007420_device : public device_t
{
public:
	k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k007420_device() {}

	static void static_set_palette_tag(device_t &device, const char *tag);
	static void static_set_bank_limit(device_t &device, int limit) { downcast<k007420_device &>(device).m_banklimit = limit; }
	static void static_set_callback(device_t &device, k007420_delegate callback) { downcast<k007420_device &>(device).m_callback = callback; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	UINT8        *m_ram;

	int          m_flipscreen;    // current code uses the 7342 flipscreen!!
	UINT8        m_regs[8];   // current code uses the 7342 regs!! (only [2])
	required_device<palette_device> m_palette;
	int                m_banklimit;
	k007420_delegate m_callback;
};

extern const device_type K007420;

#define MCFG_K007420_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K007420, 0)

#define MCFG_K007420_PALETTE(_palette_tag) \
	k007420_device::static_set_palette_tag(*device, "^" _palette_tag);

#define MCFG_K007420_BANK_LIMIT(_limit) \
	k007420_device::static_set_bank_limit(*device, _limit);

#define MCFG_K007420_CALLBACK_OWNER(_class, _method) \
	k007420_device::static_set_callback(*device, k007420_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

// function definition for a callback
#define K007420_CALLBACK_MEMBER(_name)     void _name(int *code, int *color)


#endif
