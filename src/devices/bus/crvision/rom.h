// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __CRVISION_ROM_H
#define __CRVISION_ROM_H

#include "slot.h"


// ======================> crvision_rom_device

class crvision_rom_device : public device_t,
						public device_crvision_cart_interface
{
public:
	// construction/destruction
	crvision_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	crvision_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom80) override;
};

// ======================> crvision_rom6k_device

class crvision_rom6k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom6k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom80) override;
};

// ======================> crvision_rom8k_device

class crvision_rom8k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom8k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom80) override;
};

// ======================> crvision_rom10k_device

class crvision_rom10k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom10k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom40) override;
	virtual DECLARE_READ8_MEMBER(read_rom80) override;
};

// ======================> crvision_rom12k_device

class crvision_rom12k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom12k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom40) override;
	virtual DECLARE_READ8_MEMBER(read_rom80) override;
};

// ======================> crvision_rom16k_device

class crvision_rom16k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom16k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom80) override;
};

// ======================> crvision_rom18k_device

class crvision_rom18k_device : public crvision_rom_device
{
public:
	// construction/destruction
	crvision_rom18k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom40) override;
	virtual DECLARE_READ8_MEMBER(read_rom80) override;
};





// device type definition
extern const device_type CRVISION_ROM_4K;
extern const device_type CRVISION_ROM_6K;
extern const device_type CRVISION_ROM_8K;
extern const device_type CRVISION_ROM_10K;
extern const device_type CRVISION_ROM_12K;
extern const device_type CRVISION_ROM_16K;
extern const device_type CRVISION_ROM_18K;


#endif
