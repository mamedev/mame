#include "emu.h"
#include "machine/secflash.h"

device_secure_serial_flash::device_secure_serial_flash(const machine_config &mconfig,
																		device_type type,
																		const char *name, const char *tag,
																		device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
}

void device_secure_serial_flash::device_start()
{
	save_item(NAME(cs));
	save_item(NAME(rst));
	save_item(NAME(scl));
	save_item(NAME(sdaw));
	save_item(NAME(sdar));
}

void device_secure_serial_flash::device_reset()
{
	cs = rst = scl = sdaw = sdar = false;
}

void device_secure_serial_flash::cs_w(bool _cs)
{
	if(cs == _cs)
		return;
	cs = _cs;
	if(cs)
		cs_1();
	else
		cs_0();
}

void device_secure_serial_flash::rst_w(bool _rst)
{
	if(rst == _rst)
		return;
	rst = _rst;
	if(rst)
		rst_1();
	else
		rst_0();
}

void device_secure_serial_flash::scl_w(bool _scl)
{
	if(scl == _scl)
		return;
	scl = _scl;
	if(scl)
		scl_1();
	else
		scl_0();
}

void device_secure_serial_flash::sda_w(bool _sda)
{
	if(sdaw == _sda)
		return;
	sdaw = _sda;
	if(sdaw)
		sda_1();
	else
		sda_0();
}

bool device_secure_serial_flash::sda_r()
{
	return cs ? true : sdar;
}
