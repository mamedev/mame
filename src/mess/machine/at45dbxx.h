/*

    Atmel Serial DataFlash

    (c) 2001-2007 Tim Schuerewegen

    AT45DB041 -  528 KByte
    AT45DB081 - 1056 KByte
    AT45DB161 - 2112 KByte

*/

#ifndef _AT45DBXX_H_
#define _AT45DBXX_H_

#include "emu.h"


/***************************************************************************
    MACROS
***************************************************************************/

class at45db041_device : public device_t
{
public:
	at45db041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	at45db041_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~at45db041_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type AT45DB041;


#define MCFG_AT45DB041_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB041, 0) \

class at45db081_device : public at45db041_device
{
public:
	at45db081_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type AT45DB081;


#define MCFG_AT45DB081_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB081, 0) \

class at45db161_device : public at45db041_device
{
public:
	at45db161_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type AT45DB161;


#define MCFG_AT45DB161_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AT45DB161, 0) \


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
// pins
void at45dbxx_pin_cs(device_t *device, int data);
void at45dbxx_pin_sck(device_t *device,  int data);
void at45dbxx_pin_si(device_t *device,  int data);
int  at45dbxx_pin_so(device_t *device);

// load/save
void at45dbxx_load(device_t *device, emu_file *file);
void at45dbxx_save(device_t *device, emu_file *file);

// non-volatile ram handler
//NVRAM_HANDLER( at45dbxx );

#endif
