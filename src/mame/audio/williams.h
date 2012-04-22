/***************************************************************************

    audio/williams.h

    Functions to emulate general the various Williams/Midway sound cards.

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

****************************************************************************/

#include "emu.h"
#include "machine/6821pia.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/hc55516.h"
#include "sound/dac.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type WILLIAMS_NARC_SOUND;
extern const device_type WILLIAMS_CVSD_SOUND;
extern const device_type WILLIAMS_ADPCM_SOUND;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_WILLIAMS_NARC_SOUND_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, WILLIAMS_NARC_SOUND, 0) \

#define MCFG_WILLIAMS_CVSD_SOUND_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, WILLIAMS_CVSD_SOUND, 0) \

#define MCFG_WILLIAMS_ADPCM_SOUND_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, WILLIAMS_ADPCM_SOUND, 0) \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> williams_cvsd_sound_device

class williams_cvsd_sound_device :	public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_cvsd_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(talkback_w);
	DECLARE_WRITE8_MEMBER(cvsd_digit_clock_clear_w);
	DECLARE_WRITE8_MEMBER(cvsd_clock_set_w);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(pia_irqa);
	DECLARE_WRITE_LINE_MEMBER(pia_irqb);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// devices
	required_device<m6809e_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<hc55516_device> m_hc55516;

	// internal state
	UINT8 m_talkback;
};


// ======================> williams_narc_sound_device

class williams_narc_sound_device :	public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_narc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(master_bank_select_w);
	DECLARE_WRITE8_MEMBER(slave_bank_select_w);
	DECLARE_READ8_MEMBER(command_r);
	DECLARE_WRITE8_MEMBER(command2_w);
	DECLARE_READ8_MEMBER(command2_r);
	DECLARE_WRITE8_MEMBER(master_talkback_w);
	DECLARE_WRITE8_MEMBER(master_sync_w);
	DECLARE_WRITE8_MEMBER(slave_talkback_w);
	DECLARE_WRITE8_MEMBER(slave_sync_w);
	DECLARE_WRITE8_MEMBER(cvsd_digit_clock_clear_w);
	DECLARE_WRITE8_MEMBER(cvsd_clock_set_w);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// timer IDs
	enum
	{
		TID_MASTER_COMMAND,
		TID_SLAVE_COMMAND,
		TID_SYNC_CLEAR
	};

	// devices
	required_device<m6809e_device> m_cpu0;
	required_device<m6809e_device> m_cpu1;
	required_device<hc55516_device> m_hc55516;

	// internal state
	UINT8 m_latch;
	UINT8 m_latch2;
	UINT8 m_talkback;
	UINT8 m_audio_sync;
	UINT8 m_sound_int_state;
};


// ======================> williams_adpcm_sound_device

class williams_adpcm_sound_device :	public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_adpcm_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);
	DECLARE_READ_LINE_MEMBER(irq_read);

	// internal communications
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(oki6295_bank_select_w);
	DECLARE_READ8_MEMBER(command_r);
	DECLARE_WRITE8_MEMBER(talkback_w);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);

protected:
	// timer IDs
	enum
	{
		TID_COMMAND,
		TID_IRQ_CLEAR
	};

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// devices
	required_device<m6809e_device> m_cpu;

	// internal state
	UINT8 m_latch;
	UINT8 m_talkback;
	UINT8 m_sound_int_state;
};


