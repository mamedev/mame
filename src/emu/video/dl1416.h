/*****************************************************************************
 *
 *  DL1416
 *
 *  license: MAME, GPL-2.0+
 *  copyright-holders: Dirk Best
 *
 * 4-Digit 16-Segment Alphanumeric Intelligent Display
 * with Memory/Decoder/Driver
 *
 * See video/dl1416.c for more info
 *
 ****************************************************************************/

#ifndef DL1416_H_
#define DL1416_H_


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct dl1416_interface
{
	devcb_write16 m_update;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DL1416B_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, DL1416B, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_DL1416T_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, DL1416T, 0) \
	MCFG_DEVICE_CONFIG(_config)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* device get info callback */
class dl1416_device : public device_t,
										public dl1416_interface
{
public:
	dl1416_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~dl1416_device() {}

	/* inputs */
	DECLARE_WRITE_LINE_MEMBER( wr_w ); /* write enable */
	DECLARE_WRITE_LINE_MEMBER( ce_w ); /* chip enable */
	DECLARE_WRITE_LINE_MEMBER( cu_w ); /* cursor enable */
	DECLARE_WRITE8_MEMBER( data_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	int m_write_enable;
	int m_chip_enable;
	int m_cursor_enable;
	devcb_resolved_write16 m_update_func;

	UINT16 m_digit_ram[4]; // holds the digit code for each position
	UINT8 m_cursor_state[4]; // holds the cursor state for each position, 0=off, 1=on
};

class dl1416b_device : public dl1416_device
{
public:
	dl1416b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type DL1416B;

class dl1416t_device : public dl1416_device
{
public:
	dl1416t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type DL1416T;


#endif /* DL1416_H_ */
