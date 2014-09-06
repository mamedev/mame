#ifndef __A78_CPUWIZ_H
#define __A78_CPUWIZ_H

#include "a78_slot.h"
#include "rom.h"


// ======================> a78_versaboard_device

class a78_versaboard_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_versaboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_versaboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
	
protected:
	int m_ram_bank;
};


// ======================> a78_versapokey_device

class a78_versapokey_device : public a78_versaboard_device
{
public:
	// construction/destruction
	a78_versapokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_04xx);
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	required_device<pokey_device> m_pokey;
};


// ======================> a78_megacart_device

class a78_megacart_device : public a78_versaboard_device
{
public:
	// construction/destruction
	a78_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// reading and writing
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
};



// device type definition
extern const device_type A78_ROM_VERSABOARD;
extern const device_type A78_ROM_VERSAPOKEY;
extern const device_type A78_ROM_MEGACART;


#endif
