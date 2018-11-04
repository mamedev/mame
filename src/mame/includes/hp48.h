// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX and HP49 G

**********************************************************************/
#ifndef MAME_INCLUDES_HP84_H
#define MAME_INCLUDES_HP84_H

#pragma once

#include "sound/dac.h"

/* model */
typedef enum {
	HP48_S,
	HP48_SX,
	HP48_G,
	HP48_GX,
	HP48_GP,
	HP49_G
} hp48_models;

/* memory module configuration */
typedef struct
{
	/* static part */
	uint32_t off_mask;             /* offset bit-mask, indicates the real size */
	read8_delegate read;
	const char *read_name;
	write8_delegate write;
	void* data;                  /* non-NULL for banks */
	int isnop;

	/* configurable part */
	uint8_t  state;                /* one of HP48_MODULE_ */
	uint32_t base;                 /* base address */
	uint32_t mask;                 /* often improperly called size, it is an address select mask */

} hp48_module;


/* screen image averaging */
#define HP48_NB_SCREENS 3

class hp48_state : public driver_device
{
public:
	hp48_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_palette(*this, "palette")  { }

	uint8_t *m_videoram;
	uint8_t m_io[64];
	hp48_models m_model;

	/* OUT register from SATURN (actually 12-bit) */
	uint16_t m_out;

	/* keyboard interrupt */
	uint8_t m_kdn;

	/* from highest to lowest priority: HDW, NCE2, CE1, CE2, NCE3, NCE1 */
	hp48_module m_modules[6];

	/* RAM/ROM extensions, GX/SX only (each uint8_t stores one nibble)
	   port1: SX/GX: 32/128 KB
	   port2: SX:32/128KB, GX:128/512/4096 KB
	*/
	uint32_t m_port_size[2];
	uint8_t m_port_write[2];
	std::unique_ptr<uint8_t[]> m_port_data[2];

	uint32_t m_bank_switch;
	uint32_t m_io_addr;
	uint16_t m_crc;
	uint8_t m_timer1;
	uint32_t m_timer2;
	uint8_t m_screens[ HP48_NB_SCREENS ][ 64 ][ 144 ];
	int m_cur_screen;
	uint8_t* m_rom;
	emu_timer *m_1st_timer;
	emu_timer *m_2nd_timer;
	emu_timer *m_kbd_timer;

	DECLARE_DRIVER_INIT(hp48);
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(hp48);
	DECLARE_MACHINE_START(hp49g);
	DECLARE_MACHINE_START(hp48gx);
	DECLARE_MACHINE_START(hp48g);
	DECLARE_MACHINE_START(hp48gp);
	DECLARE_MACHINE_START(hp48sx);
	DECLARE_MACHINE_START(hp48s);
	uint32_t screen_update_hp48(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void hp48_machine_start(hp48_models model);
	DECLARE_WRITE8_MEMBER(hp48_io_w);
	DECLARE_READ8_MEMBER(hp48_io_r);
	DECLARE_READ8_MEMBER(hp48_bank_r);
	DECLARE_WRITE8_MEMBER(hp49_bank_w);
	TIMER_CALLBACK_MEMBER(hp48_rs232_byte_recv_cb);
	TIMER_CALLBACK_MEMBER(hp48_rs232_byte_sent_cb);
	TIMER_CALLBACK_MEMBER(hp48_kbd_cb);
	TIMER_CALLBACK_MEMBER(hp48_timer1_cb);
	TIMER_CALLBACK_MEMBER(hp48_timer2_cb);
	void hp48_update_annunciators();
	void hp48_apply_modules();
	required_device<cpu_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_device<palette_device> m_palette;
	void hp48_pulse_irq( int irq_line);
	void hp48_rs232_start_recv_byte( uint8_t data );
	void hp48_rs232_send_byte(  );
	int hp48_get_in(  );
	void hp48_update_kdn(  );
	void hp48_reset_modules(  );
	void hp48_decode_nibble( uint8_t* dst, uint8_t* src, int size );
	void hp48_encode_nibble( uint8_t* dst, uint8_t* src, int size );

	/* memory controller */
	DECLARE_WRITE_LINE_MEMBER( hp48_mem_reset );
	DECLARE_WRITE32_MEMBER( hp48_mem_config );
	DECLARE_WRITE32_MEMBER( hp48_mem_unconfig );
	DECLARE_READ32_MEMBER( hp48_mem_id );

	/* CRC computation */
	DECLARE_WRITE32_MEMBER( hp48_mem_crc );

	/* IN/OUT registers */
	DECLARE_READ32_MEMBER( hp48_reg_in );
	DECLARE_WRITE32_MEMBER( hp48_reg_out );

	/* keyboard interrupt system */
	DECLARE_WRITE_LINE_MEMBER( hp48_rsi );
	void hp48_common(machine_config &config);
	void hp48s(machine_config &config);
	void hp48gp(machine_config &config);
	void hp48sx(machine_config &config);
	void hp48g(machine_config &config);
	void hp48gx(machine_config &config);
	void hp49g(machine_config &config);
	void hp48(address_map &map);
};


/***************************************************************************
    MACROS
***************************************************************************/

/* read from I/O memory */
#define HP48_IO_4(x)   (m_io[(x)])
#define HP48_IO_8(x)   (m_io[(x)] | (m_io[(x)+1] << 4))
#define HP48_IO_12(x)  (m_io[(x)] | (m_io[(x)+1] << 4) | (m_io[(x)+2] << 8))
#define HP48_IO_20(x)  (m_io[(x)] | (m_io[(x)+1] << 4) | (m_io[(x)+2] << 8) | \
						(m_io[(x)+3] << 12) | (m_io[(x)+4] << 16))


/*----------- defined in machine/hp48.c -----------*/

/***************************************************************************
    GLOBAL VARIABLES & CONSTANTS
***************************************************************************/

/* I/O memory */



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* list of memory modules from highest to lowest priority */
#define HP48_HDW  0
#define HP48_NCE2 1
#define HP48_CE1  2
#define HP48_CE2  3
#define HP48_NCE3 4
#define HP48_NCE1 5

/****************************** cards ********************************/

class hp48_port_image_device :  public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	hp48_port_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_port_config(device_t &device, int port, int module, int max_size)
	{
		downcast<hp48_port_image_device &>(device).m_port = port;
		downcast<hp48_port_image_device &>(device).m_module = module;
		downcast<hp48_port_image_device &>(device).m_max_size = max_size;
	}

	// image-level overrides
	virtual iodevice_t image_type() const override { return IO_MEMCARD; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *file_extensions() const override { return "crd"; }
	virtual const char *custom_instance_name() const override { return "port"; }
	virtual const char *custom_brief_instance_name() const override { return "p"; }

	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;

protected:
	// device-level overrides
	virtual void device_start() override;
private:
	void hp48_fill_port();
	void hp48_unfill_port();

	int m_port;                 /* port index: 0 or 1 (for port 1 and 2) */
	int m_module;               /* memory module where the port is visible */
	int m_max_size;             /* maximum size, in bytes 128 KB or 4 GB */
};

// device type definition
DECLARE_DEVICE_TYPE(HP48_PORT, hp48_port_image_device)

#define MCFG_HP48_PORT_ADD(_tag, _port, _module, _max_size) \
	MCFG_DEVICE_ADD(_tag, HP48_PORT, 0) \
	hp48_port_image_device::set_port_config(*device, _port, _module, _max_size);

#endif // MAME_INCLUDES_HP84_H
