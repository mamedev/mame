/***************************************************************************

    Sega Outrun hardware

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
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"


// ======================> segaorun_state

class segaorun_state : public driver_device
{
public:
	// construction/destruction
	segaorun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_mapper(*this, "mapper"),
		  m_maincpu(*this, "maincpu"),
		  m_subcpu(*this, "subcpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_i8255(*this, "i8255"),
		  m_nvram(*this, "nvram"),
		  m_workram(*this, "workram"),
		  m_custom_map(NULL),
		  m_shangon_video(false),
		  m_scanline_timer(NULL),
		  m_irq2_state(0),
		  m_adc_select(0),
		  m_vblank_irq_state(0)
	{ }

	// PPI read/write handlers
	DECLARE_READ8_MEMBER( unknown_porta_r );
	DECLARE_READ8_MEMBER( unknown_portb_r );
	DECLARE_READ8_MEMBER( unknown_portc_r );
	DECLARE_WRITE8_MEMBER( unknown_porta_w );
	DECLARE_WRITE8_MEMBER( unknown_portb_w );
	DECLARE_WRITE8_MEMBER( video_control_w );

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, UINT8 index);
	UINT8 mapper_sound_r();
	void mapper_sound_w(UINT8 data);

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );
	DECLARE_WRITE16_MEMBER( nop_w );
	
	// Z80 sound CPU read/write handlers
	DECLARE_READ8_MEMBER( sound_data_r );

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(outrun);
	DECLARE_DRIVER_INIT(outrunb);
	DECLARE_DRIVER_INIT(shangon);

	// video updates
	UINT32 screen_update_outrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_shangon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// wrappers for legacy functions (to be removed)
	template<read16_space_func _Legacy>
	READ16_MEMBER( legacy_wrapper_r ) { return _Legacy(&space, offset, mem_mask); }
	template<write16_space_func _Legacy>
	WRITE16_MEMBER( legacy_wrapper ) { _Legacy(&space, offset, data, mem_mask); }

protected:
	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};
	
	// device overrides
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers
	void update_main_irqs();
	static void m68k_reset_callback(device_t *device);

	// custom I/O
	DECLARE_READ16_MEMBER( outrun_custom_io_r );
	DECLARE_WRITE16_MEMBER( outrun_custom_io_w );
	DECLARE_READ16_MEMBER( shangon_custom_io_r );
	DECLARE_WRITE16_MEMBER( shangon_custom_io_w );
	
	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	required_device<i8255_device> m_i8255;
	optional_device<nvram_device> m_nvram;

	// memory
	required_shared_ptr<UINT16> m_workram;
	
	// configuration
	read16_delegate	 	m_custom_io_r;
	write16_delegate 	m_custom_io_w;
	const UINT8 *		m_custom_map;
	bool				m_shangon_video;

	// internal state
	emu_timer *			m_scanline_timer;
	UINT8 				m_irq2_state;
	UINT8 				m_adc_select;
	UINT8 				m_vblank_irq_state;
};
