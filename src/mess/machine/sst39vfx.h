/*

    SST Multi-Purpose Flash (MPF)

    (c) 2001-2007 Tim Schuerewegen

    SST39VF020  - 256 KByte
    SST39VF400A - 512 Kbyte

*/

#ifndef _SST39VFX_H_
#define _SST39VFX_H_

#include "emu.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sst39vfx_config sst39vfx_config;
struct _sst39vfx_config
{
	int cpu_datawidth;
	int cpu_endianess;
};

/***************************************************************************
    MACROS
***************************************************************************/

class sst39vf020_device : public device_t
{
public:
	sst39vf020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	sst39vf020_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~sst39vf020_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type SST39VF020;


#define MCFG_SST39VF020_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, SST39VF020, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_SST39VF020_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

class sst39vf400a_device : public sst39vf020_device
{
public:
	sst39vf400a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type SST39VF400A;


#define MCFG_SST39VF400A_ADD(_tag,_config) \
	MCFG_DEVICE_ADD(_tag, SST39VF400A, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_SST39VF400A_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
// get base/size
UINT8* sst39vfx_get_base( device_t *device );
UINT32 sst39vfx_get_size( device_t *device );

// read/write handler
#if 0
READ8_HANDLER( sst39vfx_r );
WRITE8_HANDLER( sst39vfx_w );
#endif

// load/save
void sst39vfx_load(device_t *device, emu_file *file);
void sst39vfx_save(device_t *device, emu_file *file);

// non-volatile ram handler
//NVRAM_HANDLER( sst39vfx );

#endif
