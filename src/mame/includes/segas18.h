/***************************************************************************

    Sega System 16A/16B/18/Outrun/Hang On/X-Board/Y-Board hardware

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

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/megavdp.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"


// ======================> segas18_state

class segas18_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segas18_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
		  m_mapper(*this, "mapper"),
		  m_maincpu(*this, "maincpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_mcu(*this, "mcu"),
		  m_vdp(*this, "gen_vdp"),
		  m_nvram(*this, "nvram"),
		  m_sprites(*this, "sprites"),
		  m_workram(*this, "workram"),
		  m_romboard(ROM_BOARD_INVALID),
		  m_has_guns(false),
		  m_grayscale_enable(false),
		  m_vdp_enable(false),
		  m_vdp_mixing(0),
		  m_mcu_data(0),
		  m_lghost_value(0),
		  m_lghost_select(0)
	{
		memset(m_misc_io_data, 0, sizeof(m_misc_io_data));
		memset(m_wwally_last_x, 0, sizeof(m_wwally_last_x));
		memset(m_wwally_last_y, 0, sizeof(m_wwally_last_y));
	}

	// driver init
	DECLARE_DRIVER_INIT(ddcrew);
	DECLARE_DRIVER_INIT(lghost);
	DECLARE_DRIVER_INIT(generic_shad);
	DECLARE_DRIVER_INIT(generic_5874);
	DECLARE_DRIVER_INIT(wwally);
	DECLARE_DRIVER_INIT(generic_5987);

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, UINT8 index);
	UINT8 mapper_sound_r();
	void mapper_sound_w(UINT8 data);

	// read/write handlers
	DECLARE_WRITE16_MEMBER( rom_5987_bank_w );
	DECLARE_READ16_MEMBER( io_chip_r );
	DECLARE_WRITE16_MEMBER( io_chip_w );
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );
	DECLARE_WRITE8_MEMBER( soundbank_w );
	DECLARE_WRITE8_MEMBER( mcu_data_w );

	DECLARE_READ16_MEMBER( genesis_vdp_r );
	DECLARE_WRITE16_MEMBER( genesis_vdp_w );


	// custom I/O
	DECLARE_READ16_MEMBER( ddcrew_custom_io_r );
	DECLARE_READ16_MEMBER( lghost_custom_io_r );
	DECLARE_WRITE16_MEMBER( lghost_custom_io_w );
	DECLARE_READ16_MEMBER( wwally_custom_io_r );
	DECLARE_WRITE16_MEMBER( wwally_custom_io_w );

	// video rendering
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// wrappers for legacy functions (to be removed)
	template<read16_space_func _Legacy>
	READ16_MEMBER( legacy_wrapper_r ) { return _Legacy(space, offset, mem_mask); }
	template<write16_space_func _Legacy>
	WRITE16_MEMBER( legacy_wrapper ) { _Legacy(space, offset, data, mem_mask); }

protected:
	// timer IDs
	enum
	{
		TID_INITIAL_BOOST
	};

	// rom board types
	enum segas18_rom_board
	{
		ROM_BOARD_INVALID,
		ROM_BOARD_171_SHADOW,	// 171-???? -- used by shadow dancer
		ROM_BOARD_171_5874,		// 171-5874
		ROM_BOARD_171_5987		// 171-5987
	};

	// device overrides
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers
	void init_generic(segas18_rom_board rom_board);
	void set_grayscale(bool enable);
	void set_vdp_enable(bool enable);
	void set_vdp_mixing(UINT8 mixing);
	void draw_vdp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<sega_genesis_vdp_device> m_vdp;
	required_device<nvram_device> m_nvram;
	required_device<sega_sys16b_sprite_device> m_sprites;

	// memory pointers
	required_shared_ptr<UINT16> m_workram;

	// configuration
	segas18_rom_board	m_romboard;
	read16_delegate		m_custom_io_r;
	write16_delegate	m_custom_io_w;
	bool				m_has_guns;

	// internal state
	bool				m_grayscale_enable;
	bool				m_vdp_enable;
	UINT8				m_vdp_mixing;
	bitmap_ind16		m_temp_bitmap;
	UINT8				m_mcu_data;
	UINT8				m_misc_io_data[0x10];

	// game-specific state
	UINT8				m_wwally_last_x[3];
	UINT8				m_wwally_last_y[3];
	UINT8				m_lghost_value;
	UINT8				m_lghost_select;
};
