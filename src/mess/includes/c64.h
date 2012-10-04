#pragma once

#ifndef __C64__
#define __C64__


#include "emu.h"
#include "formats/cbm_snqk.h"
#include "includes/cbm.h"
#include "machine/c64exp.h"
#include "machine/c64user.h"
#include "machine/cbmiec.h"
#include "machine/cbmipt.h"
#include "machine/mos6526.h"
#include "machine/petcass.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "machine/vcsctrl.h"
#include "sound/dac.h"
#include "sound/sid6581.h"
#include "video/mos6566.h"

#define M6510_TAG		"u7"
#define MOS6567_TAG		"u19"
#define MOS6569_TAG		"u19"
#define MOS6851_TAG		"u18"
#define MOS6526_1_TAG	"u1"
#define MOS6526_2_TAG	"u2"
#define PLA_TAG			"u17"
#define SCREEN_TAG		"screen"
#define CONTROL1_TAG	"joy1"
#define CONTROL2_TAG	"joy2"

class c64_state : public driver_device
{
public:
	c64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, M6510_TAG),
		  m_pla(*this, PLA_TAG),
		  m_vic(*this, MOS6569_TAG),
		  m_sid(*this, MOS6851_TAG),
		  m_cia1(*this, MOS6526_1_TAG),
		  m_cia2(*this, MOS6526_2_TAG),
		  m_iec(*this, CBM_IEC_TAG),
		  m_joy1(*this, CONTROL1_TAG),
		  m_joy2(*this, CONTROL2_TAG),
		  m_exp(*this, C64_EXPANSION_SLOT_TAG),
		  m_user(*this, C64_USER_PORT_TAG),
		  m_ram(*this, RAM_TAG),
		  m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		  m_loram(1),
		  m_hiram(1),
		  m_charen(1),
		  m_color_ram(*this, "color_ram"),
		  m_va14(1),
		  m_va15(1),
		  m_cia1_irq(CLEAR_LINE),
		  m_cia2_irq(CLEAR_LINE),
		  m_vic_irq(CLEAR_LINE),
		  m_exp_irq(CLEAR_LINE),
		  m_exp_nmi(CLEAR_LINE),
		  m_cass_rd(1),
		  m_iec_srq(1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pls100_device> m_pla;
	required_device<mos6566_device> m_vic;
	required_device<sid6581_device> m_sid;
	required_device<mos6526_device> m_cia1;
	required_device<mos6526_device> m_cia2;
	optional_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<c64_expansion_slot_device> m_exp;
	required_device<c64_user_port_device> m_user;
	required_device<ram_device> m_ram;
	optional_device<pet_datassette_port_device> m_cassette;

	virtual void machine_start();
	virtual void machine_reset();

	void check_interrupts();
	void bankswitch(offs_t offset, offs_t va, int rw, int aec, int ba, int cas, int *casram, int *basic, int *kernal, int *charom, int *grw, int *io, int *roml, int *romh);
	UINT8 read_memory(address_space &space, offs_t offset, int ba, int casram, int basic, int kernal, int charom, int io, int roml, int romh);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_WRITE_LINE_MEMBER( vic_irq_w );
	DECLARE_READ8_MEMBER( vic_lightpen_x_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_y_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_button_cb );
	DECLARE_READ8_MEMBER( vic_rdy_cb );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

	DECLARE_WRITE_LINE_MEMBER( cia1_irq_w );
	DECLARE_READ8_MEMBER( cia1_pa_r );
	DECLARE_READ8_MEMBER( cia1_pb_r );
	DECLARE_WRITE8_MEMBER( cia1_pb_w );

	DECLARE_WRITE_LINE_MEMBER( cia2_irq_w );
	DECLARE_READ8_MEMBER( cia2_pa_r );
	DECLARE_WRITE8_MEMBER( cia2_pa_w );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_WRITE_LINE_MEMBER( tape_read_w );

	DECLARE_WRITE_LINE_MEMBER( iec_srq_w );

	DECLARE_READ8_MEMBER( exp_dma_r );
	DECLARE_WRITE8_MEMBER( exp_dma_w );
	DECLARE_WRITE_LINE_MEMBER( exp_irq_w );
	DECLARE_WRITE_LINE_MEMBER( exp_nmi_w );
	DECLARE_WRITE_LINE_MEMBER( exp_reset_w );

	// memory state
	int m_loram;
	int m_hiram;
	int m_charen;
	UINT8 *m_basic;
	UINT8 *m_kernal;
	UINT8 *m_charom;

	// video state
	required_shared_ptr<UINT8> m_color_ram;
	int m_va14;
	int m_va15;

	// interrupt state
	int m_cia1_irq;
	int m_cia2_irq;
	int m_vic_irq;
	int m_exp_irq;
	int m_exp_nmi;
	int m_cass_rd;
	int m_iec_srq;
	DECLARE_DRIVER_INIT(c64pal);
	INTERRUPT_GEN_MEMBER(c64_frame_interrupt);
};


class sx64_state : public c64_state
{
public:
	sx64_state(const machine_config &mconfig, device_type type, const char *tag)
		: c64_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );
};


class c64c_state : public c64_state
{
public:
	c64c_state(const machine_config &mconfig, device_type type, const char *tag)
		: c64_state(mconfig, type, tag)
	{ }

	virtual void machine_start();
};


class c64gs_state : public c64c_state
{
public:
	c64gs_state(const machine_config &mconfig, device_type type, const char *tag)
		: c64c_state(mconfig, type, tag)
	{ }

	virtual void machine_start();

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );
};


int c64_paddle_read (device_t *device, address_space &space, int which);


#endif
