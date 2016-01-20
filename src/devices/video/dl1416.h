// license:GPL-2.0+
// copyright-holders:Dirk Best
/*****************************************************************************
 *
 *  DL1416
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
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DL1416_UPDATE_HANDLER(_devcb) \
	devcb = &dl1416_device::set_update_handler(*device, DEVCB_##_devcb);


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* device get info callback */
class dl1416_device : public device_t
{
public:
	dl1416_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	~dl1416_device() {}

	template<class _Object> static devcb_base &set_update_handler(device_t &device, _Object object) { return downcast<dl1416_device &>(device).m_update.set_callback(object); }

	/* inputs */
	DECLARE_WRITE_LINE_MEMBER( wr_w ); /* write enable */
	DECLARE_WRITE_LINE_MEMBER( ce_w ); /* chip enable */
	DECLARE_WRITE_LINE_MEMBER( cu_w ); /* cursor enable */
	DECLARE_WRITE8_MEMBER( data_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	int m_write_enable;
	int m_chip_enable;
	int m_cursor_enable;
	devcb_write16 m_update;

	UINT16 m_digit_ram[4]; // holds the digit code for each position
	UINT8 m_cursor_state[4]; // holds the cursor state for each position, 0=off, 1=on
};

class dl1416b_device : public dl1416_device
{
public:
	dl1416b_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type DL1416B;

class dl1416t_device : public dl1416_device
{
public:
	dl1416t_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type DL1416T;


#endif /* DL1416_H_ */
