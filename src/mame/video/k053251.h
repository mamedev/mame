// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef __K053251_H__
#define __K053251_H__

	enum
	{
		K053251_CI0 = 0,
		K053251_CI1,
		K053251_CI2,
		K053251_CI3,
		K053251_CI4
	};

typedef device_delegate<void ()> konami_tilemap_dirty_delegate;
#define K053251_CB_MEMBER(_name)   void _name()

class k053251_device : public device_t
{
public:
	k053251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053251_device() {}

	static void static_set_tilemap_dirty_cb(device_t &device, konami_tilemap_dirty_delegate callback) { downcast<k053251_device &>(device).m_tilemap_dirty_cb = callback; }

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE16_MEMBER( lsb_w );
	DECLARE_WRITE16_MEMBER( msb_w );
	int get_priority(int ci);
	int get_palette_index(int ci);
	int get_tmap_dirty(int tmap_num);
	void set_tmap_dirty(int tmap_num, int data);

	DECLARE_READ16_MEMBER( lsb_r );         // PCU1
	DECLARE_READ16_MEMBER( msb_r );         // PCU1

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	int      m_dirty_tmap[5];

	UINT8    m_ram[16];
	int      m_palette_index[5];

	// automatically called when some palette index changes
	konami_tilemap_dirty_delegate m_tilemap_dirty_cb;

 	void reset_indexes();
};

extern const device_type K053251;

#define MCFG_K053251_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K053251, 0)

#define MCFG_K053251_CB(_device, _class, _method) \
	k053251_device::static_set_tilemap_dirty_cb(*device, konami_tilemap_dirty_delegate(&_class::_method, #_class "::" #_method, _device, (_class *)0));
 
#endif
