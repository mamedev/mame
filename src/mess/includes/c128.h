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

#include "includes/c64_legacy.h"
#include "machine/6526cia.h"

class c128_state : public legacy_c64_state
{
public:
	c128_state(const machine_config &mconfig, device_type type, const char *tag)
		: legacy_c64_state(mconfig, type, tag) { }

	DECLARE_READ8_MEMBER( vic_lightpen_x_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_y_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_button_cb );
	DECLARE_READ8_MEMBER( vic_dma_read );
	DECLARE_READ8_MEMBER( vic_dma_read_color );
	DECLARE_WRITE_LINE_MEMBER( vic_interrupt );
	DECLARE_READ8_MEMBER( vic_rdy_cb );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

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
	int m_monitor;
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

WRITE8_HANDLER(c128_mmu8722_port_w);
READ8_HANDLER(c128_mmu8722_port_r);
WRITE8_HANDLER(c128_mmu8722_ff00_w);
READ8_HANDLER(c128_mmu8722_ff00_r);
WRITE8_HANDLER(c128_write_0000);
WRITE8_HANDLER(c128_write_1000);
WRITE8_HANDLER(c128_write_4000);
WRITE8_HANDLER(c128_write_8000);
WRITE8_HANDLER(c128_write_a000);
WRITE8_HANDLER(c128_write_c000);
WRITE8_HANDLER(c128_write_d000);
WRITE8_HANDLER(c128_write_e000);
WRITE8_HANDLER(c128_write_ff00);
WRITE8_HANDLER(c128_write_ff05);


extern MACHINE_START( c128 );
extern MACHINE_RESET( c128 );
extern INTERRUPT_GEN( c128_frame_interrupt );

void c128_bankswitch_64(running_machine &machine, int reset);

extern READ8_DEVICE_HANDLER(c128_m6510_port_read);
extern WRITE8_DEVICE_HANDLER(c128_m6510_port_write);

extern const mos6526_interface c128_ntsc_cia0, c128_pal_cia0;
extern const mos6526_interface c128_ntsc_cia1, c128_pal_cia1;

WRITE_LINE_DEVICE_HANDLER( c128_iec_srq_w );
WRITE_LINE_DEVICE_HANDLER( c128_iec_data_w );

#endif /* __C128_H__ */
