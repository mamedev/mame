// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    bufsprite.h

    Buffered Sprite RAM device.

*********************************************************************/

#ifndef MAME_VIDEO_BUFSPRITE_H
#define MAME_VIDEO_BUFSPRITE_H

#pragma once



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(BUFFERED_SPRITERAM8,  buffered_spriteram8_device)
DECLARE_DEVICE_TYPE(BUFFERED_SPRITERAM16, buffered_spriteram16_device)
DECLARE_DEVICE_TYPE(BUFFERED_SPRITERAM32, buffered_spriteram32_device)
DECLARE_DEVICE_TYPE(BUFFERED_SPRITERAM64, buffered_spriteram64_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> buffered_spriteram_device

// base class to manage buffered spriteram
template <typename Type>
class buffered_spriteram_device : public device_t
{
public:
	// getters
	Type *live() const { return m_spriteram; }
	Type *buffer() { return &m_buffered[0]; }
	uint32_t bytes() const { return m_spriteram.bytes(); }

	// operations
	Type *copy(uint32_t srcoffset = 0, uint32_t srclength = 0x7fffffff)
	{
		assert(m_spriteram != nullptr);
		if (m_spriteram != nullptr)
			memcpy(&m_buffered[0], m_spriteram + srcoffset, (std::min<size_t>)(srclength, m_spriteram.bytes() / sizeof(Type) - srcoffset) * sizeof(Type));
		return &m_buffered[0];
	}

	// read/write handlers
	void write(address_space &space, offs_t offset, Type data, Type mem_mask = ~Type(0)) { copy(); }

	// VBLANK handlers
	DECLARE_WRITE_LINE_MEMBER(vblank_copy_rising) { if (state) copy(); }
	DECLARE_WRITE_LINE_MEMBER(vblank_copy_falling) { if (!state) copy(); }

protected:
	// construction
	buffered_spriteram_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// first-time setup
	virtual void device_start() override;

private:
	// internal state
	required_shared_ptr<Type>   m_spriteram;
	std::vector<Type>           m_buffered;
};


// ======================> buffered_spriteram8_device

class buffered_spriteram8_device : public buffered_spriteram_device<uint8_t>
{
public:
	// construction
	buffered_spriteram8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> buffered_spriteram16_device

class buffered_spriteram16_device : public buffered_spriteram_device<uint16_t>
{
public:
	// construction
	buffered_spriteram16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> buffered_spriteram32_device

class buffered_spriteram32_device : public buffered_spriteram_device<uint32_t>
{
public:
	// construction
	buffered_spriteram32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> buffered_spriteram64_device

class buffered_spriteram64_device : public buffered_spriteram_device<uint64_t>
{
public:
	// construction
	buffered_spriteram64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


#endif  // MAME_VIDEO_BUFSPRITE_H
