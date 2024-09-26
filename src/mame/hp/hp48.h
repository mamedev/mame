// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX and HP49 G

**********************************************************************/
#ifndef MAME_HP_HP84_H
#define MAME_HP_HP84_H

#pragma once

#include "cpu/saturn/saturn.h"
#include "hp48_port.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"

/* model */
typedef enum {
	HP48_S,
	HP48_SX,
	HP48_G,
	HP48_GX,
	HP48_GP,
	HP49_G
} hp48_models;

/* screen image averaging */
#define HP48_NB_SCREENS 3

class hp48_state : public driver_device
{
public:
	hp48_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_modules{ { *this }, { *this }, { *this }, { *this }, { *this }, { *this } }
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_port(*this, "port%u", 1U)
		, m_lshift0(*this, "lshift0")
		, m_rshift0(*this, "rshift0")
		, m_alpha0(*this, "alpha0")
		, m_alert0(*this, "alert0")
		, m_busy0(*this, "busy0")
		, m_transmit0(*this, "transmit0")
	{
	}

	void hp48s(machine_config &config);
	void hp48gp(machine_config &config);
	void hp48sx(machine_config &config);
	void hp48g(machine_config &config);
	void hp48gx(machine_config &config);
	void hp49g(machine_config &config);

	void init_hp48();

	void decode_nibble(uint8_t* dst, uint8_t* src, int size);
	void encode_nibble(uint8_t* dst, uint8_t* src, int size);

	void apply_modules();

	/* memory module configuration */
	struct hp48_module
	{
		hp48_module(device_t &owner) : read(owner), write(owner) { }

		/* static part */
		uint32_t off_mask = 0U;          // offset bit-mask, indicates the real size
		read8sm_delegate read;
		const char *read_name = nullptr;
		write8sm_delegate write;
		void* data = nullptr;            // non-NULL for banks
		int isnop = 0;

		/* configurable part */
		uint8_t  state = 0U;             // one of HP48_MODULE_
		uint32_t base = 0U;              // base address
		uint32_t mask = 0U;              // often improperly called size, it is an address select mask

	};

	/* from highest to lowest priority: HDW, NCE2, CE1, CE2, NCE3, NCE1 */
	hp48_module m_modules[6];

private:
	virtual void machine_reset() override ATTR_COLD;
	void base_machine_start(hp48_models model);

	void hp48_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(hp49g);
	DECLARE_MACHINE_START(hp48gx);
	DECLARE_MACHINE_START(hp48g);
	DECLARE_MACHINE_START(hp48gp);
	DECLARE_MACHINE_START(hp48sx);
	DECLARE_MACHINE_START(hp48s);
	uint32_t screen_update_hp48(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	uint8_t bank_r(offs_t offset);
	void hp49_bank_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(rs232_byte_recv_cb);
	TIMER_CALLBACK_MEMBER(rs232_byte_sent_cb);
	TIMER_CALLBACK_MEMBER(kbd_cb);
	TIMER_CALLBACK_MEMBER(timer1_cb);
	TIMER_CALLBACK_MEMBER(timer2_cb);
	void update_annunciators();
	void pulse_irq(int irq_line);
	void rs232_start_recv_byte(uint8_t data);
	void rs232_send_byte();
	int get_in();
	void update_kdn();
	void reset_modules();

	/* memory controller */
	void mem_reset(int state);
	void mem_config(uint32_t data);
	void mem_unconfig(uint32_t data);
	uint32_t mem_id();

	/* CRC computation */
	void mem_crc(offs_t offset, uint32_t data);

	/* IN/OUT registers */
	uint32_t reg_in();
	void reg_out(uint32_t data);

	/* keyboard interrupt system */
	void rsi(int state);
	void hp48_common(machine_config &config);
	void hp48(address_map &map) ATTR_COLD;

	required_device<saturn_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	uint8_t *m_videoram = nullptr;
	uint8_t m_io[64]{};
	hp48_models m_model;

	/* OUT register from SATURN (actually 12-bit) */
	uint16_t m_out = 0U;

	/* keyboard interrupt */
	uint8_t m_kdn = 0U;

	/* RAM/ROM extensions, GX/SX only
	   port1: SX/GX: 32/128 KB
	   port2: SX:32/128KB, GX:128/512/4096 KB
	*/
	optional_device_array<hp48_port_image_device, 2> m_port;

	output_finder<> m_lshift0;
	output_finder<> m_rshift0;
	output_finder<> m_alpha0;
	output_finder<> m_alert0;
	output_finder<> m_busy0;
	output_finder<> m_transmit0;

	uint32_t m_bank_switch = 0U;
	uint32_t m_io_addr = 0U;
	uint16_t m_crc = 0U;
	uint8_t m_timer1 = 0U;
	uint32_t m_timer2 = 0U;
	uint8_t m_screens[HP48_NB_SCREENS][64][144]{};
	int m_cur_screen = 0;
	uint8_t* m_rom = nullptr;
	emu_timer *m_1st_timer = nullptr;
	emu_timer *m_2nd_timer = nullptr;
	emu_timer *m_kbd_timer = nullptr;
	emu_timer *m_recv_done_timer = nullptr;
	emu_timer *m_send_done_timer = nullptr;
	std::unique_ptr<uint8_t[]> m_allocated_ram{};
	std::unique_ptr<uint8_t[]> m_allocated_rom{};
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

#endif // MAME_HP_HP84_H
