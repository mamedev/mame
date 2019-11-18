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

	DECLARE_READ8_MEMBER( host_r );
	DECLARE_WRITE8_MEMBER( host_w );
	DECLARE_READ8_MEMBER( sound_r );
	DECLARE_WRITE8_MEMBER( sound_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

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
