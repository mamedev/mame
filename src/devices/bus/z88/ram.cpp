// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    z88.c

    Z88 RAM cartridges emulation

***************************************************************************/

#include "emu.h"
#include "ram.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z88_32k_ram_device

class z88_32k_ram_device : public device_t,
							public device_nvram_interface,
							public device_z88cart_interface
{
public:
	// construction/destruction
	z88_32k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	z88_32k_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementations
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override
	{
	}
	virtual bool nvram_read(util::read_stream &file) override
	{
		auto const [err, actual] = util::read(file, get_cart_base(), get_cart_size());
		return !err && (actual == get_cart_size());
	}
	virtual bool nvram_write(util::write_stream &file) override
	{
		auto const [err, actual] = util::write(file, get_cart_base(), get_cart_size());
		return !err;
	}

	// z88cart_interface implementation
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual uint8_t* get_cart_base() override;
	virtual uint32_t get_cart_size() override { return 0x8000; }

protected:
	// internal state
	uint8_t *     m_ram;
};

// ======================> z88_128k_ram_device

class z88_128k_ram_device : public z88_32k_ram_device
{
public:
	// construction/destruction
	z88_128k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// z88cart_interface implementation
	virtual uint32_t get_cart_size() override { return 0x20000; }
};

// ======================> z88_512k_ram_device

class z88_512k_ram_device : public z88_32k_ram_device
{
public:
	// construction/destruction
	z88_512k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// z88cart_interface implementation
	virtual uint32_t get_cart_size() override { return 0x80000; }
};

// ======================> z88_1024k_ram_device

class z88_1024k_ram_device : public z88_32k_ram_device
{
public:
	// construction/destruction
	z88_1024k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// z88cart_interface overrides
	virtual uint32_t get_cart_size() override { return 0x100000; }
};

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88_32k_ram_device - constructor
//-------------------------------------------------

z88_32k_ram_device::z88_32k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_ram_device(mconfig, Z88_32K_RAM, tag, owner, clock)
{
}

z88_32k_ram_device::z88_32k_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_z88cart_interface(mconfig, *this)
	, m_ram(nullptr)
{
}

//-------------------------------------------------
//  z88_128k_ram_device - constructor
//-------------------------------------------------

z88_128k_ram_device::z88_128k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_ram_device(mconfig, Z88_128K_RAM, tag, owner, clock)
{
}

//-------------------------------------------------
//  z88_512k_ram_device - constructor
//-------------------------------------------------

z88_512k_ram_device::z88_512k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_ram_device(mconfig, Z88_512K_RAM, tag, owner, clock)
{
}

//-------------------------------------------------
//  z88_1024k_ram_device - constructor
//-------------------------------------------------

z88_1024k_ram_device::z88_1024k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_ram_device(mconfig, Z88_1024K_RAM, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_32k_ram_device::device_start()
{
	m_ram = machine().memory().region_alloc(tag(), get_cart_size(), 1, ENDIANNESS_LITTLE)->base();
	memset(m_ram, 0, get_cart_size());
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* z88_32k_ram_device::get_cart_base()
{
	return m_ram;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

uint8_t z88_32k_ram_device::read(offs_t offset)
{
	return m_ram[offset & (get_cart_size() - 1)];
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void z88_32k_ram_device::write(offs_t offset, uint8_t data)
{
	m_ram[offset & (get_cart_size() - 1)] = data;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(Z88_32K_RAM,   device_z88cart_interface, z88_32k_ram_device,   "z88_32k_ram",   "Z88 32KB RAM")
DEFINE_DEVICE_TYPE_PRIVATE(Z88_128K_RAM,  device_z88cart_interface, z88_128k_ram_device,  "z88_128k_ram",  "Z88 128KB RAM")
DEFINE_DEVICE_TYPE_PRIVATE(Z88_512K_RAM,  device_z88cart_interface, z88_512k_ram_device,  "z88_512k_ram",  "Z88 512KB RAM")
DEFINE_DEVICE_TYPE_PRIVATE(Z88_1024K_RAM, device_z88cart_interface, z88_1024k_ram_device, "z88_1024k_ram", "Z88 1024KB RAM")
