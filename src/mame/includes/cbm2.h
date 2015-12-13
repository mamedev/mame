// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __P500__
#define __P500__

#include "emu.h"
#include "bus/cbm2/exp.h"
#include "bus/cbm2/user.h"
#include "bus/ieee488/ieee488.h"
#include "bus/pet/cass.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6509.h"
#include "cpu/i86/i86.h"
#include "machine/cbm_snqk.h"
#include "machine/6525tpi.h"
#include "machine/ds75160a.h"
#include "machine/ds75161a.h"
#include "machine/mos6526.h"
#include "machine/mos6551.h"
#include "machine/pic8259.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "sound/mos6581.h"
#include "video/mc6845.h"
#include "video/mos6566.h"

#define M6509_TAG       "u13"
#define PLA1_TAG        "u78"
#define PLA2_TAG        "u88"
#define MOS6567_TAG     "u23"
#define MOS6569_TAG     "u23"
#define MC68B45_TAG     "u10"
#define MOS6581_TAG     "u4"
#define MOS6525_1_TAG   "u20"
#define MOS6525_2_TAG   "u102"
#define MOS6551A_TAG    "u19"
#define MOS6526_TAG     "u2"
#define DS75160A_TAG    "u3"
#define DS75161A_TAG    "u7"
#define SCREEN_TAG      "screen"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"
#define RS232_TAG       "rs232"

#define EXT_I8088_TAG   "ext_u1"
#define EXT_I8087_TAG   "ext_u4"
#define EXT_I8259A_TAG  "ext_u3"
#define EXT_MOS6526_TAG "ext_u15"
#define EXT_MOS6525_TAG "ext_u16"

class cbm2_state : public driver_device
{
public:
	cbm2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M6509_TAG),
			m_pla1(*this, PLA1_TAG),
			m_crtc(*this, MC68B45_TAG),
			m_palette(*this, "palette"),
			m_sid(*this, MOS6581_TAG),
			m_tpi1(*this, MOS6525_1_TAG),
			m_tpi2(*this, MOS6525_2_TAG),
			m_acia(*this, MOS6551A_TAG),
			m_cia(*this, MOS6526_TAG),
			m_ieee1(*this, DS75160A_TAG),
			m_ieee2(*this, DS75161A_TAG),
			m_joy1(*this, CONTROL1_TAG),
			m_joy2(*this, CONTROL2_TAG),
			m_exp(*this, CBM2_EXPANSION_SLOT_TAG),
			m_user(*this, CBM2_USER_PORT_TAG),
			m_ram(*this, RAM_TAG),
			m_cassette(*this, PET_DATASSETTE_PORT_TAG),
			m_ieee(*this, IEEE488_TAG),
			m_ext_cpu(*this, EXT_I8088_TAG),
			m_ext_pic(*this, EXT_I8259A_TAG),
			m_ext_cia(*this, EXT_MOS6526_TAG),
			m_ext_tpi(*this, EXT_MOS6525_TAG),
			m_basic(*this, "basic"),
			m_kernal(*this, "kernal"),
			m_charom(*this, "charom"),
			m_buffer_ram(*this, "buffer_ram"),
			m_extbuf_ram(*this, "extbuf_ram"),
			m_video_ram(*this, "video_ram"),
			m_pa0(*this, "PA0"),
			m_pa1(*this, "PA1"),
			m_pa2(*this, "PA2"),
			m_pa3(*this, "PA3"),
			m_pa4(*this, "PA4"),
			m_pa5(*this, "PA5"),
			m_pa6(*this, "PA6"),
			m_pa7(*this, "PA7"),
			m_pb0(*this, "PB0"),
			m_pb1(*this, "PB1"),
			m_pb2(*this, "PB2"),
			m_pb3(*this, "PB3"),
			m_pb4(*this, "PB4"),
			m_pb5(*this, "PB5"),
			m_pb6(*this, "PB6"),
			m_pb7(*this, "PB7"),
			m_lock(*this, "LOCK"),
			m_dramon(1),
			m_video_ram_size(0x800),
			m_graphics(1),
			m_todclk(0),
			m_tpi1_irq(CLEAR_LINE),
			m_acia_irq(CLEAR_LINE),
			m_user_irq(CLEAR_LINE),
			m_tpi2_pa(0),
			m_tpi2_pb(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pla_device> m_pla1;
	optional_device<mc6845_device> m_crtc;
	optional_device<palette_device> m_palette;
	required_device<mos6581_device> m_sid;
	required_device<tpi6525_device> m_tpi1;
	required_device<tpi6525_device> m_tpi2;
	required_device<mos6551_device> m_acia;
	required_device<mos6526_device> m_cia;
	required_device<ds75160a_device> m_ieee1;
	required_device<ds75161a_device> m_ieee2;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<cbm2_expansion_slot_device> m_exp;
	required_device<cbm2_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;
	required_device<ieee488_device> m_ieee;
	optional_device<cpu_device> m_ext_cpu;
	optional_device<pic8259_device> m_ext_pic;
	optional_device<mos6526_device> m_ext_cia;
	optional_device<tpi6525_device> m_ext_tpi;
	required_memory_region m_basic;
	required_memory_region m_kernal;
	required_memory_region m_charom;
	optional_shared_ptr<UINT8> m_buffer_ram;
	optional_shared_ptr<UINT8> m_extbuf_ram;
	optional_shared_ptr<UINT8> m_video_ram;
	required_ioport m_pa0;
	required_ioport m_pa1;
	required_ioport m_pa2;
	required_ioport m_pa3;
	required_ioport m_pa4;
	required_ioport m_pa5;
	required_ioport m_pa6;
	required_ioport m_pa7;
	required_ioport m_pb0;
	required_ioport m_pb1;
	required_ioport m_pb2;
	required_ioport m_pb3;
	required_ioport m_pb4;
	required_ioport m_pb5;
	required_ioport m_pb6;
	required_ioport m_pb7;
	required_ioport m_lock;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_MACHINE_START( cbm2 );
	DECLARE_MACHINE_START( cbm2_ntsc );
	DECLARE_MACHINE_START( cbm2_pal );
	DECLARE_MACHINE_START( cbm2x_ntsc );
	DECLARE_MACHINE_START( cbm2x_pal );
	DECLARE_MACHINE_RESET( cbm2 );

	virtual void read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
		int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4);

	void bankswitch(offs_t offset, int eras, int ecas, int refen, int cas, int ras, int *sysioen, int *dramen,
		int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *buframcs, int *extbufcs, int *vidramcs,
		int *diskromcs, int *csbank1, int *csbank2, int *csbank3, int *basiccs, int *knbcs, int *kernalcs,
		int *crtccs, int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs);

	UINT8 read_keyboard();
	void set_busy2(int state);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( ext_read );
	DECLARE_WRITE8_MEMBER( ext_write );

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

	DECLARE_READ8_MEMBER( cia_pa_r );
	DECLARE_WRITE8_MEMBER( cia_pa_w );
	DECLARE_READ8_MEMBER( cia_pb_r );

	DECLARE_WRITE_LINE_MEMBER( tape_read_w );

	DECLARE_READ8_MEMBER( ext_tpi_pb_r );
	DECLARE_WRITE8_MEMBER( ext_tpi_pb_w );
	DECLARE_WRITE8_MEMBER( ext_tpi_pc_w );

	DECLARE_WRITE_LINE_MEMBER( ext_cia_irq_w );
	DECLARE_READ8_MEMBER( ext_cia_pb_r );
	DECLARE_WRITE8_MEMBER( ext_cia_pb_w );

	DECLARE_WRITE_LINE_MEMBER( user_irq_w );

	MC6845_UPDATE_ROW( crtc_update_row );

	DECLARE_QUICKLOAD_LOAD_MEMBER( cbmb );
	// memory state
	int m_dramon;
	int m_busen1;
	int m_busy2;

	// video state
	size_t m_video_ram_size;
	int m_graphics;
	int m_ntsc;

	// interrupt state
	int m_todclk;
	int m_tpi1_irq;
	int m_acia_irq;
	int m_user_irq;

	// keyboard state;
	UINT8 m_tpi2_pa;
	UINT8 m_tpi2_pb;
	UINT8 m_cia_pa;

	UINT8 m_ext_cia_pb;
	UINT8 m_ext_tpi_pb;

	// timers
	emu_timer *m_todclk_timer;
};


class cbm2hp_state : public cbm2_state
{
public:
	cbm2hp_state(const machine_config &mconfig, device_type type, const char *tag)
		: cbm2_state(mconfig, type, tag)
	{ }

	virtual void read_pla(offs_t offset, int ras, int cas, int refen, int eras, int ecas,
		int *casseg1, int *casseg2, int *casseg3, int *casseg4, int *rasseg1, int *rasseg2, int *rasseg3, int *rasseg4) override;

	DECLARE_READ8_MEMBER( tpi2_pc_r );
};


class p500_state : public cbm2_state
{
public:
	p500_state(const machine_config &mconfig, device_type type, const char *tag)
		: cbm2_state(mconfig, type, tag),
			m_pla2(*this, PLA2_TAG),
			m_vic(*this, MOS6569_TAG),
			m_color_ram(*this, "color_ram"),
			m_statvid(1),
			m_vicdotsel(1),
			m_vicbnksel(0x03),
			m_vic_irq(CLEAR_LINE)
	{ }

	required_device<pla_device> m_pla2;
	required_device<mos6566_device> m_vic;
	optional_shared_ptr<UINT8> m_color_ram;

	DECLARE_MACHINE_START( p500 );
	DECLARE_MACHINE_START( p500_ntsc );
	DECLARE_MACHINE_START( p500_pal );
	DECLARE_MACHINE_RESET( p500 );

	void read_pla1(offs_t offset, int busy2, int clrnibcsb, int procvid, int refen, int ba, int aec, int srw,
		int *datxen, int *dramxen, int *clrniben, int *segf, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr);

	void read_pla2(offs_t offset, offs_t va, int ba, int vicen, int ae, int segf, int bank0,
		int *clrnibcsb, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *procvid, int *viccs, int *vidmatcs);

	void bankswitch(offs_t offset, offs_t va, int srw, int ba, int ae, int busy2, int refen,
		int *datxen, int *dramxen, int *clrniben, int *_64kcasen, int *casenb, int *viddaten, int *viddat_tr,
		int *clrnibcs, int *extbufcs, int *discromcs, int *buframcs, int *charomcs, int *viccs, int *vidmatcs,
		int *csbank1, int *csbank2, int *csbank3, int *basiclocs, int *basichics, int *kernalcs,
		int *cs1, int *sidcs, int *extprtcs, int *ciacs, int *aciacs, int *tript1cs, int *tript2cs, int *aec, int *vsysaden);

	UINT8 read_memory(address_space &space, offs_t offset, offs_t va, int ba, int ae);
	void write_memory(address_space &space, offs_t offset, UINT8 data, int ba, int ae);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_colorram_r );
	DECLARE_WRITE_LINE_MEMBER( vic_irq_w );

	DECLARE_WRITE_LINE_MEMBER( tpi1_irq_w );
	DECLARE_WRITE_LINE_MEMBER( tpi1_ca_w );
	DECLARE_WRITE_LINE_MEMBER( tpi1_cb_w );

	DECLARE_READ8_MEMBER( tpi2_pc_r );
	DECLARE_WRITE8_MEMBER( tpi2_pc_w );

	DECLARE_WRITE_LINE_MEMBER( user_irq_w );

	DECLARE_QUICKLOAD_LOAD_MEMBER( p500 );
	// video state
	int m_statvid;
	int m_vicdotsel;
	int m_vicbnksel;

	// interrupt state
	int m_vic_irq;
};



#endif
