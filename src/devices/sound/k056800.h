// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*********************************************************

    Konami 056800 MIRAC sound interface

*********************************************************/

#ifndef MAME_SOUND_K056800_H
#define MAME_SOUND_K056800_H


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K056800_ADD(tag, clock) \
	MCFG_DEVICE_ADD((tag), K056800, (clock))

#define MCFG_K056800_INT_HANDLER(cb) \
	devcb = &downcast<k056800_device &>(*device).set_int_handler((DEVCB_##cb));



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class k056800_device : public device_t
{
public:
	// construction/destruction
	k056800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_int_handler(Object &&cb) { return m_int_handler.set_callback(std::forward<Object>(cb)); }

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
