// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*********************************************************

    Konami 056800 MIRAC sound interface

*********************************************************/

#ifndef MAME_SOUND_K056800_H
#define MAME_SOUND_K056800_H


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class k056800_device : public device_t
{
public:
	// construction/destruction
	k056800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto int_callback() { return m_int_handler.bind(); }

	uint8_t host_r(offs_t offset);
	void host_w(offs_t offset, uint8_t data);
	uint8_t sound_r(offs_t offset);
	void sound_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	bool                m_int_pending;
	bool                m_int_enabled;
	uint8_t               m_host_to_snd_regs[4];
	uint8_t               m_snd_to_host_regs[2];

	devcb_write_line   m_int_handler;
};

DECLARE_DEVICE_TYPE(K056800, k056800_device)

#endif // MAME_SOUND_K056800_H
