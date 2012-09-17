/*****************************************************************************
 *
 * includes/c64.h
 *
 * Commodore C64 Home Computer
 *
 * peter.trauner@jk.uni-linz.ac.at
 *
 * Documentation: www.funet.fi
 *
 ****************************************************************************/

#ifndef C64_H_
#define C64_H_

#include "machine/6526cia.h"
#include "machine/cbmiec.h"
#include "imagedev/cartslot.h"

#define C64_MAX_ROMBANK 64 // .crt files contain multiple 'CHIPs', i.e. rom banks (of variable size) with headers. Known carts have at most 64 'CHIPs'.

struct C64_ROM {
	int addr, size, index, start;
};

struct c64_cart_t {
	C64_ROM     bank[C64_MAX_ROMBANK];
	INT8        game;
	INT8        exrom;
	UINT8       mapper;
	UINT8       n_banks;
};

class legacy_c64_state : public driver_device
{
public:
	legacy_c64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_iec(*this, CBM_IEC_TAG),
		  m_roml_writable(0)
    { }

	optional_device<cbm_iec_device> m_iec;

	DECLARE_READ8_MEMBER( c64_lightpen_x_cb );
	DECLARE_READ8_MEMBER( c64_lightpen_y_cb );
	DECLARE_READ8_MEMBER( c64_lightpen_button_cb );
	DECLARE_READ8_MEMBER( c64_dma_read );
	DECLARE_READ8_MEMBER( c64_dma_read_ultimax );
	DECLARE_READ8_MEMBER( c64_dma_read_color );
	DECLARE_WRITE_LINE_MEMBER( c64_vic_interrupt );
	DECLARE_READ8_MEMBER( c64_rdy_cb );

	int m_old_level;
	int m_old_data;
	int m_old_exrom;
	int m_old_game;
	UINT8 m_vicirq;
	emu_timer *m_datasette_timer;
	emu_timer *m_cartridge_timer;
	UINT8 *m_colorram;
	UINT8 *m_basic;
	UINT8 *m_kernal;
	UINT8 *m_chargen;
	UINT8 *m_memory;
	int m_pal;
	int m_tape_on;
	UINT8 *m_c64_roml;
	UINT8 *m_c64_romh;
	UINT8 *m_vicaddr;
	UINT8 *m_c128_vicaddr;
	UINT8 m_game;
	UINT8 m_exrom;
	UINT8 *m_io_mirror;
	UINT8 m_port_data;
	UINT8 *m_roml;
	UINT8 *m_romh;
	int m_roml_writable;
	int m_ultimax;
	int m_cia1_on;
	int m_io_enabled;
	int m_is_sx64;
	UINT8 *m_io_ram_w_ptr;
	UINT8 *m_io_ram_r_ptr;
	c64_cart_t m_cart;
	int m_nmilevel;

	DECLARE_DRIVER_INIT( c64 );
	DECLARE_DRIVER_INIT( c64pal );
	DECLARE_DRIVER_INIT( ultimax );
	DECLARE_DRIVER_INIT( c64gs );
	DECLARE_DRIVER_INIT( sx64 );
};


/*----------- defined in machine/c64.c -----------*/

/* private area */

extern DECLARE_READ8_DEVICE_HANDLER(c64_m6510_port_read);
extern DECLARE_WRITE8_DEVICE_HANDLER(c64_m6510_port_write);

READ8_HANDLER ( c64_colorram_read );
WRITE8_HANDLER ( c64_colorram_write );

MACHINE_START( c64 );
MACHINE_RESET( c64 );
INTERRUPT_GEN( c64_frame_interrupt );
TIMER_CALLBACK( c64_tape_timer );

/* private area */
WRITE8_HANDLER(c64_roml_w);

READ8_HANDLER(c64_ioarea_r);
WRITE8_HANDLER(c64_ioarea_w);

WRITE8_HANDLER ( c64_write_io );
READ8_HANDLER ( c64_read_io );
int c64_paddle_read (device_t *device, address_space &space, int which);

extern const mos6526_interface c64_ntsc_cia0, c64_pal_cia0;
extern const mos6526_interface c64_ntsc_cia1, c64_pal_cia1;

MACHINE_CONFIG_EXTERN( c64_cartslot );
MACHINE_CONFIG_EXTERN( ultimax_cartslot );

#endif /* C64_H_ */
