/*****************************************************************************
 *
 * includes/compis.h
 *
 * machine driver header
 *
 * Per Ola Ingvarsson
 * Tomas Karlsson
 *
 ****************************************************************************/

#ifndef COMPIS_H_
#define COMPIS_H_

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "video/upd7220.h"
#include "machine/ctronics.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/mm58274c.h"
#include "machine/upd765.h"
#include "machine/compiskb.h"
#include "imagedev/flopdrv.h"
#include "formats/cpis_dsk.h"


struct mem_state
{
	UINT16	lower;
	UINT16	upper;
	UINT16	middle;
	UINT16	middle_size;
	UINT16	peripheral;
};

struct timer_state
{
	UINT16	control;
	UINT16	maxA;
	UINT16	maxB;
	UINT16	count;
	emu_timer *	int_timer;
	emu_timer *	time_timer;
	UINT8	time_timer_active;
	attotime	last_time;
};

struct dma_state
{
	UINT32	source;
	UINT32	dest;
	UINT16	count;
	UINT16	control;
	UINT8	finished;
	emu_timer *	finish_timer;
};

struct intr_state
{
	UINT8	pending;
	UINT16	ack_mask;
	UINT16	priority_mask;
	UINT16	in_service;
	UINT16	request;
	UINT16	status;
	UINT16	poll_status;
	UINT16	timer;
	UINT16	dma[2];
	UINT16	ext[4];
};

struct i186_state
{
	struct timer_state	timer[3];
	struct dma_state	dma[2];
	struct intr_state	intr;
	struct mem_state	mem;
};


/* Keyboard */
struct TYP_COMPIS_KEYBOARD
{
	UINT8 nationality;   /* Character set, keyboard layout (Swedish) */
	UINT8 release_time;  /* Autorepeat release time (0.8)   */
	UINT8 speed;	     /* Transmission speed (14)     */
	UINT8 roll_over;     /* Key roll-over (MKEY)        */
	UINT8 click;	     /* Key click (NO)          */
	UINT8 break_nmi;     /* Keyboard break (NMI)        */
	UINT8 beep_freq;     /* Beep frequency (low)        */
	UINT8 beep_dura;     /* Beep duration (short)       */
	UINT8 password[8];   /* Password            */
	UINT8 owner[16];     /* Owner               */
	UINT8 network_id;    /* Network workstation number (1)  */
	UINT8 boot_order[4]; /* Boot device order (FD HD NW PD) */
	UINT8 key_code;
	UINT8 key_status;
};

/* USART 8251 */
struct TYP_COMPIS_USART
{
	UINT8 status;
	UINT8 bytes_sent;
};

/* Printer */
struct TYP_COMPIS_PRINTER
{
	UINT8 data;
	UINT8 strobe;
};


/* Main emulation */
struct TYP_COMPIS
{
	TYP_COMPIS_PRINTER	printer;	/* Printer */
	TYP_COMPIS_USART	usart;		/* USART 8251 */
	TYP_COMPIS_KEYBOARD	keyboard;	/* Keyboard  */
};


class compis_state : public driver_device
{
public:
	compis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_8253(*this, "pit8253"),
	m_8254(*this, "pit8254"),
	m_8259m(*this, "pic8259_master"),
	m_8259s(*this, "pic8259_slave"),
	m_8255(*this, "ppi8255"),
	m_centronics(*this, "centronics"),
	m_uart(*this, "uart"),
	m_rtc(*this, "mm58274c"),
	m_fdc(*this, "upd765"),
	m_crtc(*this, "upd7220"),
	m_video_ram(*this, "video_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_8253;
	required_device<device_t> m_8254;
	required_device<device_t> m_8259m;
	required_device<device_t> m_8259s;
	required_device<i8255_device> m_8255;
	required_device<centronics_device> m_centronics;
	required_device<i8251_device> m_uart;
	required_device<device_t> m_rtc;
	required_device<device_t> m_fdc;
	required_device<upd7220_device> m_crtc;
	DECLARE_READ16_MEMBER(compis_fdc_dack_r);
	DECLARE_READ16_MEMBER(compis_usart_r);
	DECLARE_WRITE16_MEMBER(compis_usart_w);
	DECLARE_READ16_MEMBER(compis_i186_internal_port_r);
	DECLARE_WRITE16_MEMBER(compis_i186_internal_port_w);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(compis_fdc_w);
	DECLARE_READ8_MEMBER(compis_fdc_r);
	DECLARE_WRITE_LINE_MEMBER(compis_fdc_int);
	DECLARE_WRITE_LINE_MEMBER(compis_fdc_dma_drq);
	DECLARE_READ8_MEMBER(compis_ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(compis_ppi_port_c_w);
	DECLARE_READ16_MEMBER(compis_osp_pit_r);
	DECLARE_WRITE16_MEMBER(compis_osp_pit_w);
	DECLARE_WRITE_LINE_MEMBER(compis_usart_rxready);
	DECLARE_WRITE_LINE_MEMBER(compis_pic8259_master_set_int_line);
	DECLARE_WRITE_LINE_MEMBER(compis_pic8259_slave_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	i186_state m_i186;
	TYP_COMPIS m_compis;
	UINT8 *m_p_videoram;
	void update_dma_control(int which, int new_control);
	void internal_timer_update(int which, int new_count, int new_maxA, int new_maxB, int new_control);
	void internal_timer_sync(int which);
	void handle_eoi(int data);
	void compis_fdc_tc(int state);

	required_shared_ptr<UINT8> m_video_ram;
	DECLARE_DRIVER_INIT(compis);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_compis2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/compis.c -----------*/

extern const i8255_interface compis_ppi_interface;
extern const struct pit8253_config compis_pit8253_config;
extern const struct pit8253_config compis_pit8254_config;
extern const struct pic8259_interface compis_pic8259_master_config;
extern const struct pic8259_interface compis_pic8259_slave_config;
extern const i8251_interface compis_usart_interface;
extern const upd765_interface compis_fdc_interface;



INTERRUPT_GEN(compis_vblank_int);

#endif /* COMPIS_H_ */
