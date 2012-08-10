/***************************************************************************

    Sega X-Board hardware

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
#include "machine/segaic16.h"
#include "video/segaic16.h"


// ======================> segaxbd_state

class segaxbd_state : public driver_device
{
public:
	// construction/destruction
	segaxbd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_subcpu(*this, "subcpu"),
		  m_soundcpu(*this, "soundcpu"),
		  m_soundcpu2(*this, "soundcpu2"),
		  m_mcu(*this, "mcu"),
		  m_cmptimer_1(*this, "cmptimer_main"),
		  m_gprider_hack(false),
		  m_timer_irq_state(0),
		  m_vblank_irq_state(0),
		  m_road_priority(0),
		  m_loffire_sync(NULL),
		  m_lastsurv_mux(0)
	{
		memset(m_adc_reverse, 0, sizeof(m_adc_reverse));
		memset(m_iochip_custom_io_r, 0, sizeof(m_iochip_custom_io_r));
		memset(m_iochip_custom_io_w, 0, sizeof(m_iochip_custom_io_w));
		memset(m_iochip_regs, 0, sizeof(m_iochip_regs));
	}

//protected:
	// internal helpers
	void sound_data_w(UINT8 data);
	void timer_ack_callback();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_soundcpu2;
	optional_device<i8751_device> m_mcu;
	required_device<sega_315_5250_compare_timer_device> m_cmptimer_1;

	// configuration
	bool			m_gprider_hack;
	bool 			m_adc_reverse[8];
	UINT8 (*m_iochip_custom_io_r[2][8])(running_machine &machine, UINT8 data);
	void (*m_iochip_custom_io_w[2][8])(running_machine &machine, UINT8 data);
	
	// internal state
	emu_timer *		m_scanline_timer;
	UINT8 			m_timer_irq_state;
	UINT8 			m_vblank_irq_state;
	UINT8 			m_iochip_regs[2][8];
	UINT8 			m_road_priority;

	// game-specific state
	UINT16 *		m_loffire_sync;
	UINT8			m_lastsurv_mux;
	DECLARE_DRIVER_INIT(generic_xboard);
	DECLARE_DRIVER_INIT(loffire);
	DECLARE_DRIVER_INIT(smgp);
	DECLARE_DRIVER_INIT(aburner2);
	DECLARE_DRIVER_INIT(gprider);
	DECLARE_DRIVER_INIT(rascot);
	DECLARE_DRIVER_INIT(lastsurv);
};


/*----------- defined in video/segaxbd.c -----------*/

VIDEO_START( xboard );
SCREEN_UPDATE_IND16( xboard );
