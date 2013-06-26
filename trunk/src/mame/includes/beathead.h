/***************************************************************************

    Atari "Stella on Steroids" hardware

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

#include "machine/atarigen.h"
#include "cpu/asap/asap.h"
#include "audio/atarijsa.h"

class beathead_state : public atarigen_state
{
public:
	beathead_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_nvram(*this, "nvram"),
			m_videoram(*this, "videoram"),
			m_paletteram(*this, "paletteram"),
			m_vram_bulk_latch(*this, "vram_bulk_latch"),
			m_palette_select(*this, "palette_select"),
			m_ram_base(*this, "ram_base"),
			m_rom_base(*this, "rom_base") { }

	virtual void machine_reset();

	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<asap_device> m_maincpu;

	required_shared_ptr<UINT32> m_nvram;

	required_shared_ptr<UINT32> m_videoram;
	required_shared_ptr<UINT32> m_paletteram;

	required_shared_ptr<UINT32> m_vram_bulk_latch;
	required_shared_ptr<UINT32> m_palette_select;

	UINT32          m_finescroll;
	offs_t          m_vram_latch_offset;

	offs_t          m_hsyncram_offset;
	offs_t          m_hsyncram_start;
	UINT8           m_hsyncram[0x800];

	required_shared_ptr<UINT32> m_ram_base;
	required_shared_ptr<UINT32> m_rom_base;

	attotime        m_hblank_offset;

	UINT8           m_irq_line_state;
	UINT8           m_irq_enable[3];
	UINT8           m_irq_state[3];

	UINT8           m_eeprom_enabled;

	UINT32 *        m_speedup_data;
	UINT32 *        m_movie_speedup_data;

	// in drivers/beathead.c
	virtual void update_interrupts();
	DECLARE_WRITE32_MEMBER( interrupt_control_w );
	DECLARE_READ32_MEMBER( interrupt_control_r );
	DECLARE_WRITE32_MEMBER( eeprom_data_w );
	DECLARE_WRITE32_MEMBER( eeprom_enable_w );
	DECLARE_READ32_MEMBER( input_2_r );
	DECLARE_READ32_MEMBER( sound_data_r );
	DECLARE_WRITE32_MEMBER( sound_data_w );
	DECLARE_WRITE32_MEMBER( sound_reset_w );
	DECLARE_WRITE32_MEMBER( coin_count_w );
	DECLARE_READ32_MEMBER( speedup_r );
	DECLARE_READ32_MEMBER( movie_speedup_r );

	// in video/beathead.c
	DECLARE_WRITE32_MEMBER( vram_transparent_w );
	DECLARE_WRITE32_MEMBER( vram_bulk_w );
	DECLARE_WRITE32_MEMBER( vram_latch_w );
	DECLARE_WRITE32_MEMBER( vram_copy_w );
	DECLARE_WRITE32_MEMBER( finescroll_w );
	DECLARE_WRITE32_MEMBER( palette_w );
	DECLARE_READ32_MEMBER( hsync_ram_r );
	DECLARE_WRITE32_MEMBER( hsync_ram_w );
	DECLARE_DRIVER_INIT(beathead);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
};
