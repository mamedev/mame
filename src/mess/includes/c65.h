/*****************************************************************************
 *
 * includes/c65.h
 *
 ****************************************************************************/

#ifndef C65_H_
#define C65_H_

#include "machine/mos6526.h"
#include "bus/cbmiec/cbmiec.h"
#include "imagedev/cartslot.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "sound/mos6581.h"
#include "video/vic4567.h"

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

struct dma_t
{
	int version;
	UINT8 data[4];
};

struct fdc_t
{
	int state;

	UINT8 reg[0x0f];

	UINT8 buffer[0x200];
	int cpu_pos;
	int fdc_pos;

	UINT16 status;

	attotime time;
	int head,track,sector;
};

struct expansion_ram_t
{
	UINT8 reg;
};

class c65_state : public driver_device
{
public:
	c65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_cia0(*this, "cia_0"),
			m_cia1(*this, "cia_1"),
			m_sid_r(*this, "sid_r"),
			m_vic(*this, "vic3"),
			m_iec(*this, CBM_IEC_TAG),
			m_colorram(*this, "colorram"),
			m_basic(*this, "basic"),
			m_chargen(*this, "chargen"),
			m_kernal(*this, "kernal"),
			m_c65_chargen(*this, "c65_chargen"),
			m_interface(*this, "interface"),
			m_roml_writable(0),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	required_device<mos6526_device> m_cia0;
	required_device<mos6526_device> m_cia1;
	required_device<mos6581_device> m_sid_r;
	required_device<vic3_device> m_vic;
	optional_device<cbm_iec_device> m_iec;

	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_basic;
	required_shared_ptr<UINT8> m_chargen;
	required_shared_ptr<UINT8> m_kernal;
	required_shared_ptr<UINT8> m_c65_chargen;
	required_shared_ptr<UINT8> m_interface;
	int m_old_level;
	int m_old_data;
	int m_old_exrom;
	int m_old_game;
	UINT8 m_vicirq;
	emu_timer *m_datasette_timer;
	emu_timer *m_cartridge_timer;
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
	int m_nmilevel; int m_charset_select;
	int m_c64mode;
	UINT8 m_6511_port;
	UINT8 m_keyline;
	int m_old_value;
	dma_t m_dma;
	int m_dump_dma;
	fdc_t m_fdc;
	expansion_ram_t m_expansion_ram;
	int m_io_on;
	int m_io_dc00_on;
	int m_cia0_irq, m_cia1_irq;
	DECLARE_DRIVER_INIT(c65);
	DECLARE_DRIVER_INIT(c65pal);

	DECLARE_READ8_MEMBER( c64_lightpen_x_cb );
	DECLARE_READ8_MEMBER( c64_lightpen_y_cb );
	DECLARE_READ8_MEMBER( c64_lightpen_button_cb );
	DECLARE_READ8_MEMBER( c64_dma_read );
	DECLARE_READ8_MEMBER( c64_dma_read_ultimax );
	DECLARE_READ8_MEMBER( c64_dma_read_color );
	DECLARE_WRITE_LINE_MEMBER( c64_vic_interrupt );
	DECLARE_READ8_MEMBER( c64_rdy_cb );
	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );
	DECLARE_MACHINE_START(c65);
	UINT32 screen_update_c65(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vic3_raster_irq);
	INTERRUPT_GEN_MEMBER(c65_frame_interrupt);
	DECLARE_READ8_MEMBER(c65_cia0_port_a_r);
	DECLARE_READ8_MEMBER(c65_cia0_port_b_r);
	DECLARE_WRITE8_MEMBER(c65_cia0_port_b_w);
	DECLARE_READ8_MEMBER(c65_cia1_port_a_r);
	DECLARE_WRITE8_MEMBER(c65_cia1_port_a_w);
	DECLARE_WRITE_LINE_MEMBER(c65_cia1_interrupt);
	void c64_legacy_driver_init();
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( c64_cart );
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER( c64_cart );
	DECLARE_WRITE_LINE_MEMBER(c65_cia0_interrupt);
	DECLARE_READ8_MEMBER(c65_lightpen_x_cb);
	DECLARE_READ8_MEMBER(c65_lightpen_y_cb);
	DECLARE_READ8_MEMBER(c65_lightpen_button_cb);
	DECLARE_READ8_MEMBER(c65_c64_mem_r);
	DECLARE_READ8_MEMBER(c65_dma_read);
	DECLARE_READ8_MEMBER(c65_dma_read_color);
	DECLARE_WRITE_LINE_MEMBER(c65_vic_interrupt);
	DECLARE_WRITE8_MEMBER(c65_bankswitch_interface);
	DECLARE_READ8_MEMBER( c65_read_mem );
	DECLARE_WRITE8_MEMBER( c65_write_mem );
	DECLARE_READ8_MEMBER( c65_ram_expansion_r );
	DECLARE_WRITE8_MEMBER( c65_ram_expansion_w );
	DECLARE_WRITE8_MEMBER( c65_write_io );
	DECLARE_WRITE8_MEMBER( c65_write_io_dc00 );
	DECLARE_READ8_MEMBER( c65_read_io );
	DECLARE_READ8_MEMBER( c65_read_io_dc00 );
	DECLARE_QUICKLOAD_LOAD_MEMBER( cbm_c65 );

	int c64_paddle_read( device_t *device, address_space &space, int which );
	void c65_nmi(  );
	void c65_irq( int level );
	void c65_dma_port_w( int offset, int value );
	int c65_dma_port_r( int offset );
	void c65_6511_port_w( int offset, int value );
	int c65_6511_port_r( int offset );
	void c65_fdc_state(void);
	void c65_fdc_w( int offset, int data );
	int c65_fdc_r( int offset );
	void c65_bankswitch(  );
	void c65_colorram_write( int offset, int value );
	void c65_common_driver_init(  );

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};

MACHINE_CONFIG_EXTERN( c64_cartslot );

#endif /* C65_H_ */
