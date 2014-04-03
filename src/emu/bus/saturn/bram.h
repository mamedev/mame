#ifndef __SAT_BRAM_H
#define __SAT_BRAM_H

#include "sat_slot.h"


// ======================> saturn_bram_device

class saturn_bram_device : public device_t,
							public device_sat_cart_interface,
							public device_nvram_interface
{
public:
	// construction/destruction
	saturn_bram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 size, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file) { if (m_ext_bram != NULL) { file.read(m_ext_bram, m_ext_bram_size); } }
	virtual void nvram_write(emu_file &file) { if (m_ext_bram != NULL) { file.write(m_ext_bram, m_ext_bram_size); } }

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ext_bram);
	virtual DECLARE_WRITE32_MEMBER(write_ext_bram);

	UINT32 m_size;  // this is the size of Battery RAM in bytes
};

class saturn_bram4mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class saturn_bram8mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class saturn_bram16mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class saturn_bram32mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



// device type definition
extern const device_type SATURN_BRAM_4MB;
extern const device_type SATURN_BRAM_8MB;
extern const device_type SATURN_BRAM_16MB;
extern const device_type SATURN_BRAM_32MB;

#endif
