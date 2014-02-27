#pragma once
#ifndef __K007420_H__
#define __K007420_H__

typedef void (*k007420_callback)(running_machine &machine, int *code, int *color);

struct k007420_interface
{
	int                m_banklimit;
	k007420_callback   m_callback;
};

class k007420_device : public device_t,
										public k007420_interface
{
public:
	k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k007420_device() {}

	static void static_set_palette_tag(device_t &device, const char *tag);
	
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8        *m_ram;

	int          m_flipscreen;    // current code uses the 7342 flipscreen!!
	UINT8        m_regs[8];   // current code uses the 7342 regs!! (only [2])
	required_device<palette_device> m_palette;
};

extern const device_type K007420;

#define MCFG_K007420_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K007420, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K007420_PALETTE(_palette_tag) \
	k007420_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
