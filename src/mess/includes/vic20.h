#pragma once

#ifndef __VIC20__
#define __VIC20__


#include "emu.h"
#include "includes/cbm.h"
#include "formats/cbm_snqk.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cartslot.h"
#include "machine/6522via.h"
#include "machine/cbmiec.h"
#include "machine/cbmipt.h"
#include "machine/ieee488.h"
#include "machine/petcass.h"
#include "machine/ram.h"
#include "machine/vcsctrl.h"
#include "machine/vic20exp.h"
#include "machine/vic20user.h"
#include "sound/dac.h"
#include "sound/mos6560.h"

#define M6502_TAG		"ue10"
#define M6522_0_TAG		"uab3"
#define M6522_1_TAG		"uab1"
#define M6560_TAG		"ub7"
#define M6561_TAG		"ub7"
#define IEC_TAG			"iec"
#define SCREEN_TAG		"screen"
#define CONTROL1_TAG	"joy1"

class vic20_state : public driver_device
{
public:
	vic20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, M6502_TAG),
		  m_via0(*this, M6522_0_TAG),
		  m_via1(*this, M6522_1_TAG),
		  m_vic(*this, M6560_TAG),
		  m_iec(*this, CBM_IEC_TAG),
		  m_joy1(*this, CONTROL1_TAG),
		  m_exp(*this, VIC20_EXPANSION_SLOT_TAG),
		  m_user(*this, VIC20_USER_PORT_TAG),
		  m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		  m_ram(*this, RAM_TAG),
		  m_color_ram(*this, "color_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<mos6560_device> m_vic;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vic20_expansion_slot_device> m_exp;
	required_device<vic20_user_port_device> m_user;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<ram_device> m_ram;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_lightx_cb );
	DECLARE_READ8_MEMBER( vic_lighty_cb );
	DECLARE_READ8_MEMBER( vic_lightbut_cb );

	DECLARE_READ8_MEMBER( via0_pa_r );
	DECLARE_WRITE8_MEMBER( via0_pa_w );

	DECLARE_READ8_MEMBER( via1_pa_r );
	DECLARE_READ8_MEMBER( via1_pb_r );
	DECLARE_WRITE8_MEMBER( via1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via1_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( via1_cb2_w );

	DECLARE_WRITE_LINE_MEMBER( exp_reset_w );

	// memory state
	UINT8 *m_basic;
	UINT8 *m_kernal;
	UINT8 *m_charom;

	// video state
	required_shared_ptr<UINT8> m_color_ram;

	// keyboard state
	int m_key_col;
	INTERRUPT_GEN_MEMBER(vic20_raster_interrupt);
};

#endif
