/*****************************************************************************
 *
 * includes/c128.h
 *
 * Commodore C128 Home Computer
 *
 * peter.trauner@jk.uni-linz.ac.at
 *
 * Documentation: iDOC (http://www.softwolves.pp.se/idoc)
 *   Christian Janoff <mepk@c64.org>
 *
 ****************************************************************************/

#ifndef __C128_H__
#define __C128_H__

#include "emu.h"
#include "formats/cbm_snqk.h"
#include "includes/cbm.h"
#include "machine/6526cia.h"
#include "machine/c64exp.h"
#include "machine/c64user.h"
#include "machine/cbmiec.h"
#include "machine/cbmipt.h"
#include "machine/petcass.h"
#include "machine/ram.h"
#include "machine/vcsctrl.h"
#include "sound/dac.h"
#include "sound/sid6581.h"
#include "video/mos6566.h"
#include "video/vdc8563.h"

// TODO remove
#include "includes/c64_legacy.h"

#define Z80A_TAG		"u10"
#define M8502_TAG		"u6"
#define MOS8563_TAG		"u22"
#define MOS8564_TAG		"u21"
#define MOS8566_TAG		"u21"
#define MOS6581_TAG		"u5"
#define MOS6526_1_TAG	"u1"
#define MOS6526_2_TAG	"u4"
#define MOS8721_TAG		"u11"
#define MOS8722_TAG		"u7"
#define SCREEN_VIC_TAG	"screen"
#define SCREEN_VDC_TAG	"screen80"
#define CONTROL1_TAG	"joy1"
#define CONTROL2_TAG	"joy2"

class c128_state : public legacy_c64_state
{
public:
	c128_state(const machine_config &mconfig, device_type type, const char *tag)
		: legacy_c64_state(mconfig, type, tag),
		  m_maincpu(*this, Z80A_TAG),
		  m_subcpu(*this, M8502_TAG),
		  m_vdc(*this, MOS8563_TAG),
		  m_vic(*this, MOS8564_TAG),
		  m_sid(*this, MOS6581_TAG),
		  m_cia1(*this, MOS6526_1_TAG),
		  m_cia2(*this, MOS6526_2_TAG),
		  //m_iec(*this, CBM_IEC_TAG),
		  //m_joy1(*this, CONTROL1_TAG),
		  //m_joy2(*this, CONTROL2_TAG),
		  //m_exp(*this, C128_EXPANSION_SLOT_TAG),
		  //m_user(*this, C128_USER_PORT_TAG),
		  //m_ram(*this, RAM_TAG),
		  m_cassette(*this, PET_DATASSETTE_PORT_TAG)
	{ }

	required_device<legacy_cpu_device> m_maincpu;
	required_device<legacy_cpu_device> m_subcpu;
	required_device<mos8563_device> m_vdc;
	required_device<mos6566_device> m_vic;
	required_device<sid6581_device> m_sid;
	required_device<mos6526_device> m_cia1;
	required_device<mos6526_device> m_cia2;
	//required_device<cbm_iec_device> m_iec;
	//required_device<vcs_control_port_device> m_joy1;
	//required_device<vcs_control_port_device> m_joy2;
	//required_device<c64_expansion_slot_device> m_exp;
	//required_device<c64_user_port_device> m_user;
	//required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( read_io );
	DECLARE_READ8_MEMBER( mmu8722_port_r );
	DECLARE_WRITE8_MEMBER( mmu8722_port_w );
	DECLARE_READ8_MEMBER( mmu8722_ff00_r );
	DECLARE_WRITE8_MEMBER( mmu8722_ff00_w );
	DECLARE_READ8_MEMBER( dma8726_port_r );
	DECLARE_WRITE8_MEMBER( dma8726_port_w );
	DECLARE_WRITE8_MEMBER( write_0000 );
	DECLARE_WRITE8_MEMBER( write_1000 );
	DECLARE_WRITE8_MEMBER( write_4000 );
	DECLARE_WRITE8_MEMBER( write_8000 );
	DECLARE_WRITE8_MEMBER( write_a000 );
	DECLARE_WRITE8_MEMBER( write_c000 );
	DECLARE_WRITE8_MEMBER( write_d000 );
	DECLARE_WRITE8_MEMBER( write_e000 );
	DECLARE_WRITE8_MEMBER( write_ff00 );
	DECLARE_WRITE8_MEMBER( write_ff05 );

	DECLARE_READ8_MEMBER( vic_lightpen_x_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_y_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_button_cb );
	DECLARE_READ8_MEMBER( vic_dma_read );
	DECLARE_READ8_MEMBER( vic_dma_read_color );
	DECLARE_WRITE_LINE_MEMBER( vic_interrupt );
	DECLARE_READ8_MEMBER( vic_rdy_cb );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

	DECLARE_WRITE_LINE_MEMBER( cia1_irq_w );
	DECLARE_WRITE_LINE_MEMBER( cia1_cnt_w );
	DECLARE_WRITE_LINE_MEMBER( cia1_sp_w );
	DECLARE_READ8_MEMBER( cia1_pa_r );
	DECLARE_READ8_MEMBER( cia1_pb_r );
	DECLARE_WRITE8_MEMBER( cia1_pb_w );

	DECLARE_WRITE_LINE_MEMBER( cia2_irq_w );
	DECLARE_READ8_MEMBER( cia2_pa_r );
	DECLARE_WRITE8_MEMBER( cia2_pa_w );

	DECLARE_WRITE_LINE_MEMBER( iec_srq_w );
	DECLARE_WRITE_LINE_MEMBER( iec_data_w );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	void nmi();
	void irq(int level);
	void iec_data_out_w();
	void iec_srq_out_w();
	void bankswitch_64(int reset);
	void bankswitch_z80();
	void bankswitch_128(int reset);
	void bankswitch(int reset);
	void mmu8722_reset();

	UINT8 *m_c128_basic;
	UINT8 *m_c128_kernal;
	UINT8 *m_c128_chargen;
	UINT8 *m_z80;
	UINT8 *m_editor;
	UINT8 *m_internal_function;
	UINT8 *m_external_function;
	UINT8 *m_vdcram;
	UINT8 m_mmu[0x0b];
	int m_mmu_cpu;
	int m_mmu_page0;
	int m_mmu_page1;
	int m_c64mode;
	int m_write_io;
	int m_ram_bottom;
	int m_ram_top;
	UINT8 *m_ram;
	UINT8 m_c64_port_data;
	UINT8 m_keyline[3];
	int m_cnt1;
	int m_sp1;
	int m_data_out;
	int m_va1617;
	int m_nmilevel;
	DECLARE_DRIVER_INIT(c128pal);
	DECLARE_DRIVER_INIT(c128dcrp);
	DECLARE_DRIVER_INIT(c128dcr);
	DECLARE_DRIVER_INIT(c128dpal);
	DECLARE_DRIVER_INIT(c128d);
	DECLARE_DRIVER_INIT(c128);
	DECLARE_DRIVER_INIT(c128d81);
};


/*----------- defined in machine/c128.c -----------*/

extern INTERRUPT_GEN( c128_frame_interrupt );

extern const mos6526_interface c128_ntsc_cia0, c128_pal_cia0;
extern const mos6526_interface c128_ntsc_cia1, c128_pal_cia1;

#endif /* __C128_H__ */
