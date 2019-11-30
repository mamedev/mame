// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/lynx.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_LYNX_H
#define MAME_INCLUDES_LYNX_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "audio/lynx.h"
#include "imagedev/snapquik.h"
#include "machine/bankdev.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define LYNX_CART       0
#define LYNX_QUICKLOAD  1


#define NR_LYNX_TIMERS  8

class lynx_state : public driver_device
{
public:
	lynx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mem_0000(*this, "mem_0000"),
		m_mem_fc00(*this, "mem_fc00"),
		m_mem_fd00(*this, "mem_fd00"),
		m_mem_fe00(*this, "mem_fe00"),
		m_mem_fffa(*this, "mem_fffa"),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_bank_fc00(*this, "bank_fc00"),
		m_bank_fd00(*this, "bank_fd00"),
		m_bank_fe00(*this, "bank_fe00"),
		m_bank_fffa(*this, "bank_fffa")
	{ }

	void lynx(machine_config &config);

private:
	struct BLITTER
	{
		// global
		uint16_t screen;
		uint16_t colbuf;
		uint16_t colpos; // byte where value of collision is written
		int16_t xoff, yoff;
		// in command
		int mode;
		uint8_t spr_coll;
		uint8_t spritenr;
		int16_t x_pos,y_pos;
		uint16_t width, height; // uint16 important for blue lightning
		int16_t tilt_accumulator;
		uint16_t height_accumulator, width_accumulator;
		uint16_t width_offset, height_offset;
		int16_t stretch, tilt;
		uint8_t color[16]; // or stored
		uint16_t bitmap;
		int use_rle;
		int line_color;

		uint8_t spr_ctl0;
		uint8_t spr_ctl1;
		uint16_t scb;
		uint16_t scb_next;
		uint8_t sprite_collide;

		int everon;
		uint8_t fred;
		int memory_accesses;
		attotime time;

		int no_collide;
		int vstretch;
		int lefthanded;
		int busy;
	};

	struct UART
	{
		uint8_t serctl;
		uint8_t data_received, data_to_send, buffer;
		int received;
		int sending;
		int buffer_loaded;
	};

	struct SUZY
	{
		uint8_t data[0x100];
		uint8_t high;
		int low;
		int signed_math;
		int accumulate;
		int accumulate_overflow;
	};

	struct MIKEY
	{
		uint8_t data[0x100];
		uint16_t disp_addr;
		uint8_t vb_rest;
	};

	struct LYNX_TIMER
	{
		uint8_t   bakup;
		uint8_t   cntrl1;
		uint8_t   cntrl2;
		uint8_t   counter;
		emu_timer   *timer;
		int     timer_active;
	};

	enum
	{
		TIMER_BLITTER,
		TIMER_SHOT,
		TIMER_UART_LOOPBACK,
		TIMER_UART
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_shared_ptr<uint8_t> m_mem_0000;
	required_shared_ptr<uint8_t> m_mem_fc00;
	required_shared_ptr<uint8_t> m_mem_fd00;
	required_shared_ptr<uint8_t> m_mem_fe00;
	required_shared_ptr<uint8_t> m_mem_fffa;
	required_device<cpu_device> m_maincpu;
	required_device<lynx_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<address_map_bank_device> m_bank_fc00;
	required_device<address_map_bank_device> m_bank_fd00;
	required_memory_bank m_bank_fe00;
	required_memory_bank m_bank_fffa;
	uint16_t m_granularity;
	int m_sign_AB;
	int m_sign_CD;
	int m_rotate;
	uint8_t m_memory_config;

	BLITTER m_blitter;
	SUZY m_suzy;
	MIKEY m_mikey;
	UART m_uart;
	LYNX_TIMER m_timer[NR_LYNX_TIMERS];

	bitmap_rgb32 m_bitmap;
	bitmap_rgb32 m_bitmap_temp;

	void lynx_mem(address_map &map);
	void lynx_fc00_mem(address_map &map);
	void lynx_fd00_mem(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(suzy_read);
	DECLARE_WRITE8_MEMBER(suzy_write);
	DECLARE_WRITE8_MEMBER(lynx_uart_w);
	DECLARE_READ8_MEMBER(lynx_uart_r);
	DECLARE_READ8_MEMBER(mikey_read);
	DECLARE_WRITE8_MEMBER(mikey_write);
	DECLARE_READ8_MEMBER(lynx_memory_config_r);
	DECLARE_WRITE8_MEMBER(lynx_memory_config_w);
	void lynx_divide();
	void lynx_multiply();
	uint8_t lynx_timer_read(int which, int offset);
	void lynx_timer_write(int which, int offset, uint8_t data);
	void sound_cb();
	TIMER_CALLBACK_MEMBER(lynx_blitter_timer);
	TIMER_CALLBACK_MEMBER(lynx_timer_shot);
	TIMER_CALLBACK_MEMBER(lynx_uart_loopback_timer);
	TIMER_CALLBACK_MEMBER(lynx_uart_timer);
	void lynx_postload();
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	uint8_t lynx_read_ram(uint16_t address);
	void lynx_write_ram(uint16_t address, uint8_t data);
	inline void lynx_plot_pixel(const int mode, const int16_t x, const int y, const int color);
	void lynx_blit_do_work(const int y, const int xdir, const int bits_per_pixel, const int mask );
	void lynx_blit_rle_do_work(  const int16_t y, const int xdir, const int bits_per_pixel, const int mask );
	void lynx_blit_lines();
	void lynx_blitter();
	void lynx_draw_line();
	void lynx_timer_init(int which);
	void lynx_timer_signal_irq(int which);
	void lynx_timer_count_down(int which);
	uint32_t lynx_time_factor(int val);
	void lynx_uart_reset();
	image_verify_result lynx_verify_cart(char *header, int kind);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
};


/*---------- suzy registers ------------- */
#define TMPADRL 0x00    // Temporary address (not sure what this is used for)
#define TMPADRH 0x01
#define TILTACUML   0x02    // Tilt accumulator (signed fixed-point, eight bits to the left of decimal)
#define TILTACUMH   0x03
#define HOFFL       0x04    // X offset to edge of visible window
#define HOFFH       0x05
#define VOFFL       0x06    // Y offset to edge of visible window
#define VOFFH       0x07
#define VIDBASL     0x08    // Video buffer address
#define VIDBASH     0x09
#define COLLBASL    0x0A    // Collision buffer address
#define COLLBASH    0x0B
#define VIDADRL     0x0C    // Current Video Build Address
#define VIDADRH     0x0D
#define COLLADRL    0x0E    // Current Collision Build Address
#define COLLADRH    0x0F
#define SCBNEXTL    0x10    // Address of next SCB
#define SCBNEXTH    0x11
#define SPRDLINEL 0x12  // Sprite data start address
#define SPRDLINEH 0x13
#define HPOSSTRTL   0x14    // Starting Hpos
#define HPOSSTRTH   0x15
#define VPOSSTRTL   0x16    // Starting Vpos
#define VPOSSTRTH   0x17
#define SPRHSIZL    0x18    // H Size
#define SPRHSIZH    0x19
#define SPRVSIZL    0x1A    // V Size
#define SPRVSIZH    0x1B
#define STRETCHL    0x1C    // H/V Size Adder (signed fixed-point)
#define STRETCHH    0x1D
#define TILTL       0x1E    // H Position Adder (signed fixed-point)
#define TILTH       0x1F
#define SPRDOFFL    0x20    // Offset to Next Sprite Data Line
#define SPRDOFFH    0x21
#define SPRVPOSL    0x22    // Current Vpos
#define SPRVPOSH    0x23
#define COLLOFFL    0x24    // Offset to Collision Depository
#define COLLOFFH    0x25
#define VSIZACUML   0x26    // Vertical Size Accumulator
#define VSIZACUMH   0x27
#define HSIZOFFL    0x28    // Horizontal Size Offset
#define HSIZOFFH    0x29
#define VSIZOFFL    0x2A    // Vertical Size Offset
#define VSIZOFFH    0x2B
#define SCBADRL 0x2C    // Address of Current SCB
#define SCBADRH 0x2D
#define PROCADRL    0x2E    // Current Spr Data Proc Address

#define MATH_D      0x52
#define MATH_C      0x53
#define MATH_B      0x54
#define MATH_A      0x55
#define MATH_P      0x56
#define MATH_N      0x57
#define MATH_H      0x60
#define MATH_G      0x61
#define MATH_F      0x62
#define MATH_E      0x63

#define MATH_M      0x6c
#define MATH_L      0x6d
#define MATH_K      0x6e
#define MATH_J      0x6f

#define SPRCTL0 0x80 // Sprite Control Bits 0 (W)(U)
#define SPRCTL1 0x81 // Sprite Control Bits 1 (W)(U)
#define SPRCOLL 0x82 // Sprite Collision Number (W)
#define SPRINIT 0x83 // Sprite Initialization Bits (W)(U)

#define SUZYHREV    0x88 // Suzy Hardware Revision (R) = '01'

#define SUZYBUSEN   0x90 // Suzy Bus Enable (W)
#define SPRGO       0x91 // Sprite Process Start Bit (W)
#define SPRSYS      0x92 // System Control Bits (R/W)

#define JOYSTICK    0xB0 // Read Joystick and Switches(R)
#define SWITCHES    0xB1 // Read Other Switches (R)
#define RCART       0xB2 // Read / Write Cartridge Bank 0 (R/W)
#define RCART_BANK1 0xB3 // Read / Write Cartridge Bank 1 (R/W) (Unused in existing cartridges?)

//0xC0 LEDs (W)
//0xC2 Parallel Port Status(R/W)
//0xC3 Parallel Port Data (R/W)
//0xC4 Howie (R/W)

// SCB offsets

//8-bit
#define SCB_SPRCTL0     0x00 // 8 bit
#define SCB_SPRCTL1     0x01 // 8 bit
#define SCB_SPRCOLL     0x02 // 4 bit

//16-bit
#define SCB_SCBNEXT     0x03    // L,H Address of Next SCB
#define SCB_SPRDLINE    0x05    // L,H Start of Sprite Data Line Address
#define SCB_HPOSSTRT    0x07    // L,H Starting Hpos
#define SCB_VPOSSTRT    0x09    //  L,H Starting Vpos
#define SCB_SPRHSIZ     0x0B    // L,H H Size
#define SCB_SPRVSIZ 0x0D    // L,H V Size
#define SCB_STRETCH 0x0F    //  L H H/V Size Adder
#define SCB_TILT        0x11    //  L,H H Position Adder

#endif // MAME_INCLUDES_LYNX_H
