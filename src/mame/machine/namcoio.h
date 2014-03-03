#ifndef __NAMCOIO_H__
#define __NAMCOIO_H__

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct namcoio_interface
{
	devcb_read8 m_in[4];
	devcb_write8 m_out[2];
};

class namcoio_device : public device_t,
						public namcoio_interface
{
public:
	namcoio_device(const machine_config &mconfig, device_type type, const char* name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);


	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	WRITE_LINE_MEMBER( set_reset_line );
	READ_LINE_MEMBER( read_reset_line );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	int m_device_type;

	enum {
			TYPE_NAMCO56XX,
			TYPE_NAMCO58XX,
			TYPE_NAMCO59XX,
	};

	// internal state
	UINT8          m_ram[16];

	devcb_resolved_read8    m_in_func[4];
	devcb_resolved_write8   m_out_func[2];

	int            m_reset;
	INT32          m_lastcoins, m_lastbuttons;
	INT32          m_credits;
	INT32          m_coins[2];
	INT32          m_coins_per_cred[2];
	INT32          m_creds_per_coin[2];
	INT32          m_in_count;

	void handle_coins( int swap );

	virtual void customio_run() {}

private:

};

class namco56xx_device : public namcoio_device
{
public:
	namco56xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void customio_run();
};

class namco58xx_device : public namcoio_device
{
public:
	namco58xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void customio_run();
};

class namco59xx_device : public namcoio_device
{
public:
	namco59xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void customio_run();
};

extern const device_type NAMCO56XX;
extern const device_type NAMCO58XX;
extern const device_type NAMCO59XX;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_NAMCO56XX_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO56XX, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_NAMCO58XX_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO58XX, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_NAMCO59XX_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO59XX, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif  /* __NAMCOIO_H__ */
