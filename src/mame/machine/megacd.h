/* Sega CD / Mega CD */

#include "machine/lc89510.h"

class sega_segacd_device : public device_t
{
public:
	sega_segacd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, device_type type);

	emu_timer *m_segacd_pwm_timer;
protected:
	virtual void device_start();
	virtual void device_reset();

	// optional information overrides
//  virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
//  virtual void device_config_complete();

};


class sega_segacd_us_device : public sega_segacd_device
{
	public:
		sega_segacd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:

};

class sega_segacd_japan_device : public sega_segacd_device
{
	public:
		sega_segacd_japan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};

class sega_segacd_europe_device : public sega_segacd_device
{
	public:
		sega_segacd_europe_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};


extern const device_type SEGA_SEGACD_US;
extern const device_type SEGA_SEGACD_JAPAN;
extern const device_type SEGA_SEGACD_EUROPE;
