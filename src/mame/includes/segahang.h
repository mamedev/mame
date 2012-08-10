/***************************************************************************

    Sega System Hang On hardware

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
#include "machine/i8255.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"


// ======================> segahang_state

class segahang_state : public driver_device
{
public:
	// construction/destruction
	segahang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_subcpu(*this, "subcpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_mcu(*this, "mcu"),
		  m_i8255_1(*this, "i8255_1"),
		  m_i8255_2(*this, "i8255_2"),
		  m_workram(*this, "workram"),
		  m_sharrier_video(false),
		  m_adc_select(0)
	{ }

	// PPI read/write callbacks
	DECLARE_WRITE8_MEMBER( video_lamps_w );
	DECLARE_WRITE8_MEMBER( tilemap_sound_w );
	DECLARE_WRITE8_MEMBER( sub_control_adc_w );
	DECLARE_READ8_MEMBER( adc_status_r );
	
	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( hangon_io_r );
	DECLARE_WRITE16_MEMBER( hangon_io_w );
	DECLARE_READ16_MEMBER( sharrier_io_r );
	DECLARE_WRITE16_MEMBER( sharrier_io_w );

	// Z80 sound CPU read/write handlers
	DECLARE_READ8_MEMBER( sound_data_r );
	DECLARE_WRITE_LINE_MEMBER( sound_irq );

	// I8751-related VBLANK interrupt hanlders
	INTERRUPT_GEN_MEMBER( i8751_main_cpu_vblank );

	// game-specific driver init
	void init_generic();
	void init_sharrier();
	void init_enduror();
	void init_endurobl();
	void init_endurob2();

	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<void ()> i8751_sim_delegate;

	// timer IDs
	enum
	{
		TID_INIT_I8751,
		TID_PPI_WRITE
	};
	
	// driver overrides
	virtual void video_start();
	virtual void machine_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// I8751 simulations
	void sharrier_i8751_sim();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<i8255_device> m_i8255_1;
	required_device<i8255_device> m_i8255_2;
	
	// memory pointers	
	required_shared_ptr<UINT16> m_workram;
	
	// configuration
	bool					m_sharrier_video;
	i8751_sim_delegate		m_i8751_vblank_hook;

	// internal state
	UINT8 					m_adc_select;
};
