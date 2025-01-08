// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K053251_H
#define MAME_KONAMI_K053251_H

class k053251_device : public device_t
{
public:
	enum
	{
		CI0 = 0,
		CI1,
		CI2,
		CI3,
		CI4
	};

	k053251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/*
	Note: k053251_w() automatically does a ALL_TILEMAPS->mark_all_dirty()
	when some palette index changes. If ALL_TILEMAPS is too expensive, use
	k053251_set_tilemaps() to indicate which tilemap is associated with each index.
	*/

	void write(offs_t offset, u8 data);
	int get_priority(int ci);
	int get_palette_index(int ci);

	u8 read(offs_t offset); // PCU1

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	// internal state
	uint8_t  m_ram[16];
	int      m_tilemaps_set;
	int      m_palette_index[5];

	void reset_indexes();
};

DECLARE_DEVICE_TYPE(K053251, k053251_device)


#endif // MAME_KONAMI_K053251_H
