// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    bufsprite.h

    Buffered Sprite RAM device.

*********************************************************************/

#ifndef MAME_DEVICES_VIDEO_BUFSPRITE_H
#define MAME_DEVICES_VIDEO_BUFSPRITE_H

#pragma once



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type BUFFERED_SPRITERAM8;
extern const device_type BUFFERED_SPRITERAM16;
extern const device_type BUFFERED_SPRITERAM32;
extern const device_type BUFFERED_SPRITERAM64;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BUFFERED_SPRITERAM8_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM8, 0)
#define MCFG_BUFFERED_SPRITERAM16_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM16, 0)
#define MCFG_BUFFERED_SPRITERAM32_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM32, 0)
#define MCFG_BUFFERED_SPRITERAM64_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM64, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> buffered_spriteram_device

// base class to manage buffered spriteram
template<typename _Type>
class buffered_spriteram_device : public device_t
{
public:
	// construction
	buffered_spriteram_device(
			const machine_config &mconfig,
			device_type type,
			const char *name,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			const char *shortname,
			const char *source);

	// getters
	_Type *live() const { return m_spriteram; }
	_Type *buffer() { return &m_buffered[0]; }
	uint32_t bytes() const { return m_spriteram.bytes(); }

	// operations
	_Type *copy(uint32_t srcoffset = 0, uint32_t srclength = 0x7fffffff)
	{
		assert(m_spriteram != nullptr);
		if (m_spriteram != nullptr)
			memcpy(&m_buffered[0], m_spriteram + srcoffset, (std::min<size_t>)(srclength, m_spriteram.bytes() / sizeof(_Type) - srcoffset) * sizeof(_Type));
		return &m_buffered[0];
	}

	// read/write handlers
	void write(address_space &space, offs_t offset, _Type data, _Type mem_mask = ~_Type(0)) { copy(); }

	// VBLANK handlers
	void vblank_copy_rising(screen_device &screen, bool state) { if (state) copy(); }
	void vblank_copy_falling(screen_device &screen, bool state) { if (!state) copy(); }

protected:
	// first-time setup
	virtual void device_start() override;

private:
	// internal state
	required_shared_ptr<_Type>  m_spriteram;
	std::vector<_Type>          m_buffered;
};


// ======================> buffered_spriteram8_device

class buffered_spriteram8_device : public buffered_spriteram_device<uint8_t>
{
public:
	// construction
	buffered_spriteram8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> buffered_spriteram16_device

class buffered_spriteram16_device : public buffered_spriteram_device<uint16_t>
{
public:
	// construction
	buffered_spriteram16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> buffered_spriteram32_device

class buffered_spriteram32_device : public buffered_spriteram_device<uint32_t>
{
public:
	// construction
	buffered_spriteram32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> buffered_spriteram64_device

class buffered_spriteram64_device : public buffered_spriteram_device<uint64_t>
{
public:
	// construction
	buffered_spriteram64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif  // MAME_DEVICES_VIDEO_BUFSPRITE_H
