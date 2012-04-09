/***************************************************************************

    driver.h

    Core driver device base class.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DRIVER_H__
#define __DRIVER_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> driver_device

// base class for machine driver-specific devices
class driver_device : public device_t
{
public:
	// construction/destruction
	driver_device(const machine_config &mconfig, device_type type, const char *tag);
	virtual ~driver_device();

	// indexes into our generic callbacks
	enum callback_type
	{
		CB_MACHINE_START,
		CB_MACHINE_RESET,
		CB_SOUND_START,
		CB_SOUND_RESET,
		CB_VIDEO_START,
		CB_VIDEO_RESET,
		CB_COUNT
	};

	// inline configuration helpers
	static void static_set_game(device_t &device, const game_driver &game);
	static void static_set_callback(device_t &device, callback_type type, legacy_callback_func callback);
	static void static_set_palette_init(device_t &device, palette_init_func callback);

	// generic helpers
	template<class _DriverClass, void (_DriverClass::*_Function)()>
	static void static_wrapper(driver_device &device)
	{
		(downcast<_DriverClass &>(device).*_Function)();
	}
	
	// generic video
	void flip_screen_set(UINT32 on);
	void flip_screen_set_no_update(UINT32 on);
	void flip_screen_x_set(UINT32 on);
	void flip_screen_y_set(UINT32 on);
	UINT32 flip_screen() const { return m_flip_screen_x; }
	UINT32 flip_screen_x() const { return m_flip_screen_x; }
	UINT32 flip_screen_y() const { return m_flip_screen_y; }

	// watchdog read/write handlers
	DECLARE_WRITE8_MEMBER( watchdog_reset_w );
	DECLARE_READ8_MEMBER( watchdog_reset_r );
	DECLARE_WRITE16_MEMBER( watchdog_reset16_w );
	DECLARE_READ16_MEMBER( watchdog_reset16_r );
	DECLARE_WRITE32_MEMBER( watchdog_reset32_w );
	DECLARE_READ32_MEMBER( watchdog_reset32_r );

	// sound latch readers
	DECLARE_READ8_MEMBER( soundlatch_r );
	DECLARE_READ8_MEMBER( soundlatch2_r );
	DECLARE_READ8_MEMBER( soundlatch3_r );
	DECLARE_READ8_MEMBER( soundlatch4_r );
	DECLARE_READ16_MEMBER( soundlatch_word_r );
	DECLARE_READ16_MEMBER( soundlatch2_word_r );
	DECLARE_READ16_MEMBER( soundlatch3_word_r );
	DECLARE_READ16_MEMBER( soundlatch4_word_r );

	// sound latch writers
	DECLARE_WRITE8_MEMBER( soundlatch_w );
	DECLARE_WRITE8_MEMBER( soundlatch2_w );
	DECLARE_WRITE8_MEMBER( soundlatch3_w );
	DECLARE_WRITE8_MEMBER( soundlatch4_w );
	DECLARE_WRITE16_MEMBER( soundlatch_word_w );
	DECLARE_WRITE16_MEMBER( soundlatch2_word_w );
	DECLARE_WRITE16_MEMBER( soundlatch3_word_w );
	DECLARE_WRITE16_MEMBER( soundlatch4_word_w );

	// sound latch clearers
	DECLARE_WRITE8_MEMBER( soundlatch_clear_w );
	DECLARE_WRITE8_MEMBER( soundlatch2_clear_w );
	DECLARE_WRITE8_MEMBER( soundlatch3_clear_w );
	DECLARE_WRITE8_MEMBER( soundlatch4_clear_w );

	// templatized palette writers for 8-bit palette data
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE8_MEMBER( palette_8bit_byte_w );

	// templatized palette writers for 16-bit palette data
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE8_MEMBER( palette_16bit_byte_le_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE8_MEMBER( palette_16bit_byte_be_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE8_MEMBER( palette_16bit_byte_split_lo_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE8_MEMBER( palette_16bit_byte_split_hi_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE16_MEMBER( palette_16bit_word_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE32_MEMBER( palette_16bit_dword_le_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE32_MEMBER( palette_16bit_dword_be_w );

	// templatized palette writers for 32-bit palette data
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE16_MEMBER( palette_32bit_word_le_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE16_MEMBER( palette_32bit_word_be_w );
	template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift> DECLARE_WRITE32_MEMBER( palette_32bit_dword_w );

	// 3-3-2 RGB palette write handlers
	DECLARE_WRITE8_MEMBER( paletteram_BBGGGRRR_byte_w );
	DECLARE_WRITE8_MEMBER( paletteram_RRRGGGBB_byte_w );
	DECLARE_WRITE8_MEMBER( paletteram_BBGGRRII_byte_w );

	// 4-4-4 RGB palette write handlers
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_byte_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_byte_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_byte_split_hi_w );
	DECLARE_WRITE16_MEMBER( paletteram_xxxxBBBBGGGGRRRR_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_byte_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_byte_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_byte_split_hi_w );
	DECLARE_WRITE16_MEMBER( paletteram_xxxxBBBBRRRRGGGG_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRBBBBGGGG_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRBBBBGGGG_byte_split_hi_w );

	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_byte_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_byte_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_byte_split_hi_w );
	DECLARE_WRITE16_MEMBER( paletteram_xxxxRRRRGGGGBBBB_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_RRRRGGGGBBBBxxxx_byte_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_RRRRGGGGBBBBxxxx_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_RRRRGGGGBBBBxxxx_byte_split_hi_w );
	DECLARE_WRITE16_MEMBER( paletteram_RRRRGGGGBBBBxxxx_word_w );

	// 4-4-4-4 IRGB palette write handlers
	DECLARE_WRITE16_MEMBER( paletteram_IIIIRRRRGGGGBBBB_word_w );
	DECLARE_WRITE16_MEMBER( paletteram_RRRRGGGGBBBBIIII_word_w );

	// 5-5-5 RGB palette write handlers
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_byte_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_byte_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_byte_split_hi_w );
	DECLARE_WRITE16_MEMBER( paletteram_xBBBBBGGGGGRRRRR_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBRRRRRGGGGG_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBRRRRRGGGGG_byte_split_hi_w );

	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_byte_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_byte_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_byte_split_lo_w );
	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_byte_split_hi_w );
	DECLARE_WRITE16_MEMBER( paletteram_xRRRRRGGGGGBBBBB_word_w );
	DECLARE_WRITE32_MEMBER( paletteram_xRRRRRGGGGGBBBBB_dword_be_w );
	DECLARE_WRITE32_MEMBER( paletteram_xRRRRRGGGGGBBBBB_dword_le_w );

	DECLARE_WRITE16_MEMBER( paletteram_xGGGGGRRRRRBBBBB_word_w );
	DECLARE_WRITE16_MEMBER( paletteram_xGGGGGBBBBBRRRRR_word_w );
	DECLARE_WRITE16_MEMBER( paletteram_RRRRRGGGGGBBBBBx_word_w );
	DECLARE_WRITE16_MEMBER( paletteram_GGGGGRRRRRBBBBBx_word_w );
	DECLARE_WRITE16_MEMBER( paletteram_RRRRGGGGBBBBRGBx_word_w );

	// 8-8-8 RGB palette write handlers
	DECLARE_WRITE16_MEMBER( paletteram_xrgb_word_be_w );
	DECLARE_WRITE16_MEMBER( paletteram_xbgr_word_be_w );
	
	// generic input port helpers
	// custom handler
	DECLARE_CUSTOM_INPUT_MEMBER( custom_port_read );

protected:
	// helpers called at startup
	virtual void driver_start();
	virtual void machine_start();
	virtual void sound_start();
	virtual void video_start();

	// helpers called at reset
	virtual void driver_reset();
	virtual void machine_reset();
	virtual void sound_reset();
	virtual void video_reset();

	// device-level overrides
	virtual const rom_entry *device_rom_region() const;
	virtual ioport_constructor device_input_ports() const;
	virtual void device_start();
	virtual void device_reset_after_children();

	// internal helpers
	inline UINT16 paletteram16_le(offs_t offset) const { return m_generic_paletteram_8[offset & ~1] | (m_generic_paletteram_8[offset |  1] << 8); }
	inline UINT16 paletteram16_be(offs_t offset) const { return m_generic_paletteram_8[offset |  1] | (m_generic_paletteram_8[offset & ~1] << 8); }
	inline UINT16 paletteram16_split(offs_t offset) const { return m_generic_paletteram_8[offset] | (m_generic_paletteram2_8[offset] << 8); }
	inline UINT32 paletteram32_be(offs_t offset) const { return m_generic_paletteram_16[offset | 1] | (m_generic_paletteram_16[offset & ~1] << 16); }

	// internal state
	const game_driver *		m_system;					// pointer to the game driver

	legacy_callback_func	m_callbacks[CB_COUNT];		// generic legacy callbacks
	palette_init_func		m_palette_init;				// one-time palette init callback

public:
	// generic pointers
	optional_shared_ptr<UINT8> m_generic_paletteram_8;
	optional_shared_ptr<UINT8> m_generic_paletteram2_8;
	optional_shared_ptr<UINT16> m_generic_paletteram_16;
	optional_shared_ptr<UINT16> m_generic_paletteram2_16;
	optional_shared_ptr<UINT32> m_generic_paletteram_32;
	optional_shared_ptr<UINT32> m_generic_paletteram2_32;

private:
	// helpers
	void updateflip();

	// generic video
	UINT32 m_flip_screen_x;
	UINT32 m_flip_screen_y;
};


// this template function creates a stub which constructs a device
template<class _DriverClass>
device_t *driver_device_creator(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
{
	assert(owner == NULL);
	assert(clock == 0);
	return global_alloc_clear(_DriverClass(mconfig, &driver_device_creator<_DriverClass>, tag));
}



//**************************************************************************
//  PALETTE WRITER TEMPLATES
//**************************************************************************

// write 8-bit palette data
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE8_MEMBER(driver_device::palette_8bit_byte_w)
{
	m_generic_paletteram_8[offset] = data;
	UINT8 paldata = m_generic_paletteram_8[offset];
	palette_set_color_rgb(machine(), offset, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 16-bit palette data to consecutive 8-bit addresses with LSB first
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE8_MEMBER(driver_device::palette_16bit_byte_le_w)
{
	m_generic_paletteram_8[offset] = data;
	UINT16 paldata = m_generic_paletteram_8[offset & ~1] | (m_generic_paletteram_8[offset | 1] << 8);
	palette_set_color_rgb(machine(), offset / 2, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 16-bit palette data to consecutive 8-bit addresses with MSB first
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE8_MEMBER(driver_device::palette_16bit_byte_be_w)
{
	m_generic_paletteram_8[offset] = data;
	UINT16 paldata = m_generic_paletteram_8[offset | 1] | (m_generic_paletteram_8[offset & ~1] << 8);
	palette_set_color_rgb(machine(), offset / 2, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 16-bit palette data to split 8-bit addresses (LSB)
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE8_MEMBER(driver_device::palette_16bit_byte_split_lo_w)
{
	m_generic_paletteram_8[offset] = data;
	UINT16 paldata = m_generic_paletteram_8[offset] | (m_generic_paletteram2_8[offset] << 8);
	palette_set_color_rgb(machine(), offset, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 16-bit palette data to split 8-bit addresses (MSB)
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE8_MEMBER(driver_device::palette_16bit_byte_split_hi_w)
{
	m_generic_paletteram2_8[offset] = data;
	UINT16 paldata = m_generic_paletteram_8[offset] | (m_generic_paletteram2_8[offset] << 8);
	palette_set_color_rgb(machine(), offset, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 16-bit palette data to 16-bit addresses
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE16_MEMBER(driver_device::palette_16bit_word_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	UINT16 paldata = m_generic_paletteram_16[offset];
	palette_set_color_rgb(machine(), offset, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 16-bit palette data to packed 32-bit addresses (lower entry in lower 16 bits)
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE32_MEMBER(driver_device::palette_16bit_dword_le_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	if (ACCESSING_BITS_0_15)
	{
		UINT16 paldata = m_generic_paletteram_32[offset];
		palette_set_color_rgb(machine(), offset * 2 + 0, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_GreenBits>(paldata >> _BlueShift));
	}
	if (ACCESSING_BITS_16_31)
	{
		UINT16 paldata = m_generic_paletteram_32[offset] >> 16;
		palette_set_color_rgb(machine(), offset * 2 + 1, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_GreenBits>(paldata >> _BlueShift));
	}
}

// write 16-bit palette data to packed 32-bit addresses (lower entry in upper 16 bits)
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE32_MEMBER(driver_device::palette_16bit_dword_be_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	if (ACCESSING_BITS_16_31)
	{
		UINT16 paldata = m_generic_paletteram_32[offset] >> 16;
		palette_set_color_rgb(machine(), offset * 2 + 0, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
	}
	if (ACCESSING_BITS_0_15)
	{
		UINT16 paldata = m_generic_paletteram_32[offset];
		palette_set_color_rgb(machine(), offset * 2 + 1, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
	}
}

// write 32-bit palette data to consecutive 16-bit addresses with LSW first
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE16_MEMBER(driver_device::palette_32bit_word_le_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	UINT32 paldata = m_generic_paletteram_16[offset & ~1] | (m_generic_paletteram_16[offset | 1] << 16);
	palette_set_color_rgb(machine(), offset / 2, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 32-bit palette data to consecutive 16-bit addresses with MSW first
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE16_MEMBER(driver_device::palette_32bit_word_be_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	UINT32 paldata = m_generic_paletteram_16[offset | 1] | (m_generic_paletteram_16[offset & ~1] << 16);
	palette_set_color_rgb(machine(), offset / 2, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

// write 32-bit palette data to 32-bit addresses
template<int _RedBits, int _GreenBits, int _BlueBits, int _RedShift, int _GreenShift, int _BlueShift>
WRITE32_MEMBER(driver_device::palette_32bit_dword_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	UINT32 paldata = m_generic_paletteram_32[offset];
	palette_set_color_rgb(machine(), offset, palexpand<_RedBits>(paldata >> _RedShift), palexpand<_GreenBits>(paldata >> _GreenShift), palexpand<_BlueBits>(paldata >> _BlueShift));
}

#endif	/* __DRIVER_H__ */
