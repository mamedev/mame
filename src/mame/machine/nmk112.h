/*************************************************************************

    nmk112.h

**************************************************************************/

#ifndef __NMK112_H__
#define __NMK112_H__

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct nmk112_interface
{
	const char *rgn0, *rgn1;
	UINT8 disable_page_mask;
};

class nmk112_device : public device_t,
									public nmk112_interface
{
public:
	nmk112_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nmk112_device() {}

	DECLARE_WRITE8_MEMBER( okibank_w );
	DECLARE_WRITE16_MEMBER( okibank_lsb_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	void do_bankswitch( int offset, int data );
	void postload_bankswitch();

	// internal state

	/* which chips have their sample address table divided into pages */
	UINT8 m_page_mask;

	UINT8 m_current_bank[8];

	UINT8 *m_rom0, *m_rom1;
	int   m_size0, m_size1;
};

extern const device_type NMK112;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_NMK112_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NMK112, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif /* __NMK112_H__ */
