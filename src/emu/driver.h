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

	// general palette helpers
	void set_color_332(pen_t color, int rshift, int gshift, int bshift, UINT32 data);
	void set_color_444(pen_t color, int rshift, int gshift, int bshift, UINT32 data);
	void set_color_555(pen_t color, int rshift, int gshift, int bshift, UINT32 data);
	void set_color_565(pen_t color, int rshift, int gshift, int bshift, UINT32 data);
	void set_color_888(pen_t color, int rshift, int gshift, int bshift, UINT32 data);
	void set_color_4444(pen_t color, int ishift, int rshift, int gshift, int bshift, UINT16 data);

	// 3-3-2 RGB palette write handlers
	DECLARE_WRITE8_MEMBER( paletteram_BBGGGRRR_w );
	DECLARE_WRITE8_MEMBER( paletteram_RRRGGGBB_w );
	DECLARE_WRITE8_MEMBER( paletteram_BBGGRRII_w );
	DECLARE_WRITE8_MEMBER( paletteram_IIBBGGRR_w );

	// 4-4-4 RGB palette write handlers
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_split1_w );	// uses paletteram
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBGGGGRRRR_split2_w );	// uses paletteram2
	DECLARE_WRITE16_MEMBER( paletteram16_xxxxBBBBGGGGRRRR_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_split1_w );	// uses paletteram
	DECLARE_WRITE8_MEMBER( paletteram_xxxxBBBBRRRRGGGG_split2_w );	// uses paletteram2
	DECLARE_WRITE16_MEMBER( paletteram16_xxxxBBBBRRRRGGGG_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRBBBBGGGG_split1_w );	// uses paletteram
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRBBBBGGGG_split2_w );	// uses paletteram2

	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_split1_w );	// uses paletteram
	DECLARE_WRITE8_MEMBER( paletteram_xxxxRRRRGGGGBBBB_split2_w );	// uses paletteram2
	DECLARE_WRITE16_MEMBER( paletteram16_xxxxRRRRGGGGBBBB_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_RRRRGGGGBBBBxxxx_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_RRRRGGGGBBBBxxxx_split1_w );	// uses paletteram
	DECLARE_WRITE8_MEMBER( paletteram_RRRRGGGGBBBBxxxx_split2_w );	// uses paletteram2
	DECLARE_WRITE16_MEMBER( paletteram16_RRRRGGGGBBBBxxxx_word_w );

	// 4-4-4-4 IRGB palette write handlers
	DECLARE_WRITE16_MEMBER( paletteram16_IIIIRRRRGGGGBBBB_word_w );
	DECLARE_WRITE16_MEMBER( paletteram16_RRRRGGGGBBBBIIII_word_w );

	// 5-5-5 RGB palette write handlers
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_split1_w );	// uses paletteram
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBGGGGGRRRRR_split2_w );	// uses paletteram2
	DECLARE_WRITE16_MEMBER( paletteram16_xBBBBBGGGGGRRRRR_word_w );

	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBRRRRRGGGGG_split1_w );  // uses paletteram
	DECLARE_WRITE8_MEMBER( paletteram_xBBBBBRRRRRGGGGG_split2_w );  // uses paletteram2

	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_le_w );
	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_be_w );
	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_split1_w );
	DECLARE_WRITE8_MEMBER( paletteram_xRRRRRGGGGGBBBBB_split2_w );
	DECLARE_WRITE16_MEMBER( paletteram16_xRRRRRGGGGGBBBBB_word_w );

	DECLARE_WRITE16_MEMBER( paletteram16_xGGGGGRRRRRBBBBB_word_w );
	DECLARE_WRITE16_MEMBER( paletteram16_xGGGGGBBBBBRRRRR_word_w );

	DECLARE_WRITE16_MEMBER( paletteram16_RRRRRGGGGGBBBBBx_word_w );
	DECLARE_WRITE16_MEMBER( paletteram16_GGGGGRRRRRBBBBBx_word_w );
	DECLARE_WRITE16_MEMBER( paletteram16_RRRRGGGGBBBBRGBx_word_w );

	// 8-8-8 RGB palette write handlers
	DECLARE_WRITE16_MEMBER( paletteram16_xrgb_word_be_w );
	DECLARE_WRITE16_MEMBER( paletteram16_xbgr_word_be_w );

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
};


// this template function creates a stub which constructs a device
template<class _DriverClass>
device_t *driver_device_creator(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
{
	assert(owner == NULL);
	assert(clock == 0);
	return global_alloc_clear(_DriverClass(mconfig, &driver_device_creator<_DriverClass>, tag));
}


#endif	/* __DRIVER_H__ */
