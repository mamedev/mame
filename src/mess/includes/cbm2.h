#pragma once

#ifndef __P500__
#define __P500__

#include "emu.h"
#include "cpu/m6502/m6509.h"
#include "formats/cbm_snqk.h"
#include "includes/cbm.h"
#include "machine/6525tpi.h"
#include "machine/6526cia.h"
#include "machine/6551acia.h"
#include "machine/cbm2exp.h"
#include "machine/cbmipt.h"
#include "machine/ieee488.h"
#include "machine/petcass.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "machine/vcsctrl.h"
#include "sound/dac.h"
#include "sound/sid6581.h"
#include "video/mos6566.h"

#define M6509_TAG		"u13"
#define PLA1_TAG		"u78"
#define PLA2_TAG		"u88"
#define MOS6567_TAG		"u23"
#define MOS6569_TAG		"u23"
#define MOS6851_TAG		"u4"
#define MOS6525_1_TAG	"u20"
#define MOS6525_2_TAG	"u102"
#define MOS6551A_TAG	"u19"
#define MOS6526_TAG		"u2"
#define SCREEN_TAG		"screen"
#define CONTROL1_TAG	"joy1"
#define CONTROL2_TAG	"joy2"

class p500_state : public driver_device
{
public:
	p500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, M6509_TAG),
		  m_pla1(*this, PLA1_TAG),
		  m_pla2(*this, PLA2_TAG),
		  m_vic(*this, MOS6569_TAG),
		  m_sid(*this, MOS6851_TAG),
		  m_tpi1(*this, MOS6525_1_TAG),
		  m_tpi2(*this, MOS6525_2_TAG),
		  m_acia(*this, MOS6551A_TAG),
		  m_cia(*this, MOS6526_TAG),
		  m_joy1(*this, CONTROL1_TAG),
		  m_joy2(*this, CONTROL2_TAG),
		  m_exp(*this, CBM2_EXPANSION_SLOT_TAG),
		  m_ram(*this, RAM_TAG),
		  m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		  m_ieee(*this, IEEE488_TAG),
		  m_buffer_ram(*this, "buffer_ram"),
		  m_dramon(1),
		  m_video_ram(*this, "video_ram"),
		  m_color_ram(*this, "color_ram"),
		  m_statvid(1),
		  m_vicdotsel(1),
		  m_vicbnksel(0x03),
		  m_todclk(0),
		  m_vic_irq(CLEAR_LINE),
		  m_tpi1_irq(CLEAR_LINE),
		  m_cass_rd(1),
		  m_user_flag(0),
		  m_tpi2_pa(0),
		  m_tpi2_pb(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pls100_device> m_pla1;
	required_device<pls100_device> m_pla2;
	required_device<mos6566_device> m_vic;
	required_device<sid6581_device> m_sid;
	required_device<tpi6525_device> m_tpi1;
	required_device<tpi6525_device> m_tpi2;
	required_device<acia6551_device> m_acia;
	required_device<mos6526_device> m_cia;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<cbm2_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<ieee488_device> m_ieee;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual void machine_start();
	virtual void machine_reset();

	void check_interrupts();
	void bankswitch(offs_t offset, offs_t va, int srw, int sphi0, int sphi1, int sphi2, int ba, int ae, int bras, int bcas, int busy2, int refen,
		int *datxen, int *dramxen, int *clrniben, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr,
		int *clrnibcs, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *viccs, int *vidmatcs,
		int *csbank1, int *csbank2, int *csbank3, int *basiclocs, int *basichics, int *kernalcs,
		int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs, int *aec, int *vsysaden);
	UINT8 read_memory(address_space &space, offs_t offset, offs_t va, int sphi0, int sphi1, int sphi2, int ba, int ae, int bras, int bcas);
	void write_memory(address_space &space, offs_t offset, UINT8 data, int sphi0, int sphi1, int sphi2, int ba, int ae, int bras, int bcas);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_WRITE_LINE_MEMBER( vic_irq_w );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

	DECLARE_WRITE_LINE_MEMBER( tpi1_irq_w );
	DECLARE_READ8_MEMBER( tpi1_pa_r );
	DECLARE_WRITE8_MEMBER( tpi1_pa_w );
	DECLARE_READ8_MEMBER( tpi1_pb_r );
	DECLARE_WRITE8_MEMBER( tpi1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( tpi1_ca_w );
	DECLARE_WRITE_LINE_MEMBER( tpi1_cb_w );

	DECLARE_WRITE8_MEMBER( tpi2_pa_w );
	DECLARE_WRITE8_MEMBER( tpi2_pb_w );
	DECLARE_READ8_MEMBER( tpi2_pc_r );
	DECLARE_WRITE8_MEMBER( tpi2_pc_w );

	DECLARE_READ8_MEMBER( cia_pa_r );
	DECLARE_WRITE8_MEMBER( cia_pa_w );
	DECLARE_READ8_MEMBER( cia_pb_r );
	DECLARE_WRITE8_MEMBER( cia_pb_w );

	DECLARE_WRITE_LINE_MEMBER( tape_read_w );

	// memory state
	optional_shared_ptr<UINT8> m_buffer_ram;
	UINT8 *m_basic;
	UINT8 *m_kernal;
	UINT8 *m_charom;
	int m_dramon;

	// video state
	optional_shared_ptr<UINT8> m_video_ram;
	optional_shared_ptr<UINT8> m_color_ram;
	int m_statvid;
	int m_vicdotsel;
	int m_vicbnksel;

	// interrupt state
	int m_todclk;
	int m_vic_irq;
	int m_tpi1_irq;
	int m_cass_rd;
	int m_user_flag;

	// keyboard state;
	UINT8 m_tpi2_pa;
	UINT8 m_tpi2_pb;
	UINT8 m_cia_pa;

	// timers
	emu_timer *m_todclk_timer;
};


#endif
