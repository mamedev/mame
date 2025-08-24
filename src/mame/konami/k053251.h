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

	void write(offs_t offset, u8 data);
	u8 get_priority(u8 ci);
	u8 get_palette_index(u8 ci);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	// internal state
	u8 m_ram[16];
	u8 m_palette_index[5];

	void reset_indexes();
};

DECLARE_DEVICE_TYPE(K053251, k053251_device)


#endif // MAME_KONAMI_K053251_H
