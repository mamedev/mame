// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    rom.c

    Z88 ROM cartridges emulation

***************************************************************************/

#include "emu.h"
#include "rom.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z88_32k_rom_device

class z88_32k_rom_device : public device_t,
							public device_nvram_interface,
							public device_z88cart_interface
{
public:
	// construction/destruction
	z88_32k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	z88_32k_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
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
	virtual bool nvram_can_write() const override
	{
		return m_modified; // Save only if the EPROM has been programmed
	}

	// z88cart_interface implementation
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual void vpp_w(int state) override { m_vpp_state = state; }
	virtual uint8_t* get_cart_base() override;
	virtual uint32_t get_cart_size() override { return 0x8000; }

protected:
	// internal state
	uint8_t *     m_rom;
	int           m_vpp_state;
	bool          m_modified;
};

// ======================> z88_128k_rom_device

class z88_128k_rom_device : public z88_32k_rom_device
{
public:
	// construction/destruction
	z88_128k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// z88cart_interface implementation
	virtual uint32_t get_cart_size() override { return 0x20000; }
};

// ======================> z88_256k_rom_device

class z88_256k_rom_device : public z88_32k_rom_device
{
public:
	// construction/destruction
	z88_256k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// z88cart_interface implementation
	virtual uint32_t get_cart_size() override { return 0x40000; }
};

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88_32k_rom_device - constructor
//-------------------------------------------------

z88_32k_rom_device::z88_32k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_rom_device(mconfig, Z88_32K_ROM, tag, owner, clock)
{
}

z88_32k_rom_device::z88_32k_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_z88cart_interface(mconfig, *this)
	, m_rom(nullptr)
	, m_vpp_state(0)
	, m_modified(false)
{
}

//-------------------------------------------------
//  z88_128k_rom_device - constructor
//-------------------------------------------------

z88_128k_rom_device::z88_128k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_rom_device(mconfig, Z88_128K_ROM, tag, owner, clock)
{
}

//-------------------------------------------------
//  z88_256k_rom_device - constructor
//-------------------------------------------------

z88_256k_rom_device::z88_256k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z88_32k_rom_device(mconfig, Z88_256K_ROM, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88_32k_rom_device::device_start()
{
	m_rom = machine().memory().region_alloc(tag(), get_cart_size(), 1, ENDIANNESS_LITTLE)->base();
	std::fill_n(m_rom, get_cart_size(), 0xff);

	save_item(NAME(m_vpp_state));
	save_item(NAME(m_modified));
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* z88_32k_rom_device::get_cart_base()
{
	return m_rom;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

uint8_t z88_32k_rom_device::read(offs_t offset)
{
	return m_rom[offset & (get_cart_size() - 1)];
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void z88_32k_rom_device::write(offs_t offset, uint8_t data)
{
	if (m_vpp_state)
	{
		const uint32_t offset_mask = get_cart_size() - 1;
		if (m_rom[offset & offset_mask] & ~data)
			m_modified = true;

		m_rom[offset & offset_mask] &= data;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(Z88_32K_ROM,  device_z88cart_interface, z88_32k_rom_device,  "z88_32k_rom",  "Z88 32KB ROM")
DEFINE_DEVICE_TYPE_PRIVATE(Z88_128K_ROM, device_z88cart_interface, z88_128k_rom_device, "z88_128k_rom", "Z88 128KB ROM")
DEFINE_DEVICE_TYPE_PRIVATE(Z88_256K_ROM, device_z88cart_interface, z88_256k_rom_device, "z88_256k_rom", "Z88 256KB ROM")
