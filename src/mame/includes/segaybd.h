/***************************************************************************

    Sega Y-Board hardware

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
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"


// ======================> segaybd_state

class segaybd_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaybd_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_subx(*this, "subx"),
		  m_suby(*this, "suby"),
		  m_soundcpu(*this, "soundcpu"),
		  m_bsprites(*this, "bsprites"),
		  m_ysprites(*this, "ysprites"),
		  m_pdrift_bank(0),
		  m_scanline_timer(NULL),
		  m_irq2_scanline(0),
		  m_timer_irq_state(0),
		  m_vblank_irq_state(0),
		  m_tmp_bitmap(512, 512)
	{
		memset(m_analog_data, 0, sizeof(m_analog_data));
		memset(m_misc_io_data, 0, sizeof(m_misc_io_data));
	}

	// YM2151 chip callbacks
	WRITE_LINE_MEMBER( sound_cpu_irq );

	// main CPU read/write handlers
	READ16_MEMBER( analog_r );
	WRITE16_MEMBER( analog_w );
	READ16_MEMBER( io_chip_r );
	WRITE16_MEMBER( io_chip_w );
	WRITE16_MEMBER( sound_data_w );

	// sound Z80 CPU read/write handlers
	READ8_MEMBER( sound_data_r );

	// game-specific output handlers
	void gforce2_output_cb2(UINT16 data);
	void gloc_output_cb1(UINT16 data);
	void gloc_output_cb2(UINT16 data);
	void r360_output_cb2(UINT16 data);
	void pdrift_output_cb1(UINT16 data);
	void pdrift_output_cb2(UINT16 data);
	void rchase_output_cb2(UINT16 data);

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(pdrift);
	DECLARE_DRIVER_INIT(r360);
	DECLARE_DRIVER_INIT(gforce2);
	DECLARE_DRIVER_INIT(rchase);
	DECLARE_DRIVER_INIT(gloc);

	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<void (UINT16)> output_delegate;

	// timer IDs
	enum
	{
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers
	void update_irqs();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subx;
	required_device<m68000_device> m_suby;
	required_device<z80_device> m_soundcpu;
	required_device<sega_sys16b_sprite_device> m_bsprites;
	required_device<sega_yboard_sprite_device> m_ysprites;

	// configuration
	output_delegate	m_output_cb1;
	output_delegate	m_output_cb2;

	// internal state
	UINT16			m_pdrift_bank;
	emu_timer *		m_scanline_timer;
	UINT8			m_analog_data[4];
	int 			m_irq2_scanline;
	UINT8			m_timer_irq_state;
	UINT8			m_vblank_irq_state;
	UINT8			m_misc_io_data[0x10];
	bitmap_ind16	m_tmp_bitmap;
};
