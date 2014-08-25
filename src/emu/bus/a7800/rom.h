#ifndef __A78_ROM_H
#define __A78_ROM_H

#include "a78_slot.h"
#include "sound/pokey.h"


// ======================> a78_rom_device

class a78_rom_device : public device_t,
						public device_a78_cart_interface
{
public:
	// construction/destruction
	a78_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
};


// ======================> a78_rom_pokey_device

class a78_rom_pokey_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
	
protected:
	required_device<pokey_device> m_pokey;
};


// ======================> a78_rom_sg_device

class a78_rom_sg_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_sg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	int m_bank;
};


// ======================> a78_rom_sg_pokey_device

class a78_rom_sg_pokey_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
	
protected:
	required_device<pokey_device> m_pokey;
};


// ======================> a78_rom_sg_ram_device

class a78_rom_sg_ram_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
};


// ======================> a78_rom_bankram_device

class a78_rom_bankram_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_bankram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

protected:
	int m_ram_bank;
};


// ======================> a78_rom_sg_9banks_device

class a78_rom_sg_9banks_device : public a78_rom_sg_device
{
public:
	// construction/destruction
	a78_rom_sg_9banks_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a78_rom_sg_9banks_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
};


// ======================> a78_rom_xm_device

class a78_rom_xm_device : public a78_rom_sg_9banks_device
{
public:
	// construction/destruction
	a78_rom_xm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
	
protected:
	required_device<pokey_device> m_pokey;
};


// ======================> a78_rom_abs_device

class a78_rom_abs_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_abs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
	
protected:
	int m_bank;
};


// ======================> a78_rom_act_device

class a78_rom_act_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_rom_act_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);
	
protected:
	int m_bank;
};


// device type definition
extern const device_type A78_ROM;
extern const device_type A78_ROM_SG;
extern const device_type A78_ROM_POKEY;
extern const device_type A78_ROM_SG_POKEY;
extern const device_type A78_ROM_SG_RAM;
extern const device_type A78_ROM_BANKRAM;
extern const device_type A78_ROM_SG_9BANKS;
extern const device_type A78_ROM_XM;
extern const device_type A78_ROM_ABSOLUTE;
extern const device_type A78_ROM_ACTIVISION;


#endif
