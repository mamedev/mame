#pragma once

#ifndef __VIC10__
#define __VIC10__


#include "emu.h"
#include "includes/cbm.h"
#include "machine/6526cia.h"
#include "machine/cbmipt.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "sound/sid6581.h"
#include "video/mos6566.h"

#define M6510_TAG		"u3"
#define MOS6566_TAG		"u2"
#define MOS6581_TAG		"u6"
#define MOS6526_TAG		"u9"
#define SCREEN_TAG		"screen"
#define TIMER_C1531_TAG	"c1531"

class vic10_state : public driver_device
{
public:
	vic10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, M6510_TAG),
		  m_vic(*this, MOS6566_TAG),
		  m_sid(*this, MOS6581_TAG),
		  m_cia(*this, MOS6526_TAG),
		  m_exp(*this, VIC10_EXPANSION_SLOT_TAG),
		  m_ram(*this, RAM_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_cassette_timer(*this, TIMER_C1531_TAG),
		  m_cia_irq(CLEAR_LINE),
		  m_vic_irq(CLEAR_LINE),
		  m_exp_irq(CLEAR_LINE)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mos6566_device> m_vic;
	required_device<device_t> m_sid;
	required_device<mos6526_device> m_cia;
	required_device<vic10_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	optional_device<cassette_image_device> m_cassette;
	optional_device<timer_device> m_cassette_timer;

	virtual void machine_start();
	virtual void machine_reset();

	void check_interrupts();
	UINT8 paddle_read(int which);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( vic_irq_w );
	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_colorram_r );
	DECLARE_READ8_MEMBER( vic_lightpen_x_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_y_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_button_cb );
	DECLARE_READ8_MEMBER( vic_rdy_cb );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

	DECLARE_WRITE_LINE_MEMBER( cia_irq_w );
	DECLARE_READ8_MEMBER( cia_pa_r );
	DECLARE_READ8_MEMBER( cia_pb_r );
	DECLARE_WRITE8_MEMBER( cia_pb_w );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_WRITE_LINE_MEMBER( exp_irq_w );

	// video state
	UINT8 *m_color_ram;

	// interrupt state
	int m_cia_irq;
	int m_vic_irq;
	int m_exp_irq;
};



#endif
