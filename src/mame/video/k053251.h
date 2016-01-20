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

class k053251_device : public device_t
{
public:
	k053251_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k053251_device() {}

	/*
	Note: k053251_w() automatically does a ALL_TILEMAPS->mark_all_dirty()
	when some palette index changes. If ALL_TILEMAPS is too expensive, use
	k053251_set_tilemaps() to indicate which tilemap is associated with each index.
	*/

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
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	int      m_dirty_tmap[5];

	UINT8    m_ram[16];
	int      m_tilemaps_set;
	int      m_palette_index[5];

	void reset_indexes();
};

extern const device_type K053251;

#define MCFG_K053251_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K053251, 0)

#endif
