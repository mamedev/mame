// Common interface for all secure serial flashes

#ifndef __SECFLASH_H__
#define __SECFLASH_H__

#include "emu.h"

class device_secure_serial_flash_config : public device_config,
										  public device_config_nvram_interface
{
public:
	device_secure_serial_flash_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 param = 0);
};

class device_secure_serial_flash : public device_t,
								   public device_nvram_interface
{
public:
	void cs_w(bool cs);
	void rst_w(bool rst);
	void scl_w(bool scl);
	void sda_w(bool sda);
	bool sda_r();

protected:
	bool cs, rst, scl, sdaw, sdar;

	virtual void cs_0() = 0;
	virtual void cs_1() = 0;
	virtual void rst_0() = 0;
	virtual void rst_1() = 0;
	virtual void scl_0() = 0;
	virtual void scl_1() = 0;
	virtual void sda_0() = 0;
	virtual void sda_1() = 0;

	device_secure_serial_flash(running_machine &_machine, const device_secure_serial_flash_config &config);
	virtual void device_start();
	virtual void device_reset();
};

#endif
