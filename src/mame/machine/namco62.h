#ifndef NAMCO62_H
#define NAMCO62_H



struct namco_62xx_interface
{
	devcb_read8     m_in[4];      /* read handlers for ports A-D */
	devcb_write8    m_out[2];     /* write handlers for ports A-B */
};

class namco_62xx_device : public device_t,
							public namco_62xx_interface
{
public:
	namco_62xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_62xx_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	// internal state
	device_t* m_cpu;
	devcb_resolved_read8 m_in_func[4];
	devcb_resolved_write8 m_out_func[2];
};

extern const device_type NAMCO_62XX;

#define MCFG_NAMCO_62XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_62XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)

#endif  /* NAMCO62_H */
