// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************

 Atari Lynx

******************************************************************************/
#ifndef MAME_ATARI_LYNX_H
#define MAME_ATARI_LYNX_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "sound/lynx.h"
#include "imagedev/snapquik.h"
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
		m_dram(*this, "dram"),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_suzy_view(*this, "suzy_view"),
		m_mikey_view(*this, "mikey_view"),
		m_rom_view(*this, "rom_view"),
		m_vector_view(*this, "vector_view"),
		m_joy_io(*this, "JOY"),
		m_pause_io(*this, "PAUSE")
	{ }

	void lynx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual void device_post_load() override;

private:
	struct BLITTER
	{
		// 0xfc80 SPRCTL0 Sprite control bits 0
		u8     BPP()            const { return BIT(spr_ctl0, 6, 2); } // Bit per pixel - 1
		bool   HFLIP()          const { return BIT(spr_ctl0, 5); }    // Horizontal flip
		bool   VFLIP()          const { return BIT(spr_ctl0, 4); }    // Vertical flip
		u8     SPRITE_TYPE()    const { return BIT(spr_ctl0, 0, 3); } // Sprite type

		// 0xfc81 SPRCTL1 Sprite control bits 1
		bool   RLE()            const { return BIT(spr_ctl1, 7); }    // Normal(1) or RLE(0) sprite
		//bool ALGO3()          const { return BIT(spr_ctl1, 6); }    // Size algorithm, Adder(0, Algo4), Shifter(1, algo3) (not implemented)
		u8     RELOAD_SCALE()   const { return BIT(spr_ctl1, 4, 2); } // Reload scale factors
		bool   REUSE_PALETTE()  const { return BIT(spr_ctl1, 3); }    // Reload(0) or Reuse(1) palette
		bool   SKIP_SPRITE()    const { return BIT(spr_ctl1, 2); }    // Skip this sprite
		u8     DRAW_ORIGIN()    const { return BIT(spr_ctl1, 0, 2); } // Start draw origin
		//bool DRAW_UP()        const { return BIT(spr_ctl1, 1); }    // Start drawing up(1) or down(0)
		//bool DRAW_LEFT()      const { return BIT(spr_ctl1, 0); }    // Start drawing left(1) or right(0)

		// 0xfc82 SPRCOLL Sprite collision number
		bool   SPRITE_COLLEN()  const { return BIT(spr_coll, 5); }    // Sprite collide(0) or Don't collide(1) flag
		u8     SPRITE_COLNUM()  const { return BIT(spr_coll, 0, 4); } // Sprite collision number

		bool   SPRITE_COLLIDE() const { return sprite_collide && (!(no_collide)); } // collision enabled?

		// global
		u16 screen = 0;
		u16 colbuf = 0;
		u16 colpos = 0; // byte where value of collision is written
		s16 xoff = 0;
		s16 yoff = 0;
		// in command
		u8 mode = 0;
		u8 spr_coll = 0;
		u8 spritenr = 0;
		s16 x_pos = 0;
		s16 y_pos = 0;
		u16 width = 0;
		u16 height = 0; // uint16 important for blue lightning
		s16 tilt_accumulator = 0;
		u16 height_accumulator = 0;
		u16 width_accumulator = 0;
		u16 width_offset = 0;
		u16 height_offset = 0;
		s16 stretch = 0;
		s16 tilt = 0;
		u8 color[16] = {0}; // or stored
		u16 bitmap = 0;
		bool use_rle = false;
		u8 line_color = 0;

		u8 spr_ctl0 = 0;
		u8 spr_ctl1 = 0;
		u16 scb = 0;
		u16 scb_next = 0;
		bool sprite_collide = false;

		bool everon = false;
		u8 fred = 0;
		u32 memory_accesses = 0;
		//attotime time;

		bool no_collide = false;
		bool vstretch = false;
		bool lefthanded = false;
		bool busy = false;
	};

	struct UART
	{
		// 0xfd8c SERCTL Serial control register (Write)
		bool   TXINTEN()  const { return BIT(serctl, 7); } // Transmit interrupt enable
		bool   RXINTEN()  const { return BIT(serctl, 6); } // Receive interrupt enable
		//bool PAREN()    const { return BIT(serctl, 4); } // Xmit parity bit enable (not implemented)
		//bool RESETERR() const { return BIT(serctl, 3); } // Reset all errors (not implemented)
		//bool TXOPEN()   const { return BIT(serctl, 2); } // Open collector driver(1) or TTL driver(0) (not implemented)
		//bool TXBRK()    const { return BIT(serctl, 1); } // Send a break (not implemented)
		//bool PAREVEN()  const { return BIT(serctl, 0); } // Send/Recevie even parity (not implemented)

		u8 serctl = 0;
		u8 data_received = 0;
		u8 data_to_send = 0;
		u8 buffer = 0;
		bool received = false;
		bool sending = false;
		bool buffer_loaded = false;
	};

	struct SUZY
	{
		// 0xfc91 SPRGO Sprite process start bit
		bool EVER_ON()   const { return BIT(data[0x91], 2); } // Everon detector enable
		bool SPRITE_GO() const { return BIT(data[0x91], 0); } // Sprite process enable

		u8 data[0x100] = {0};
		bool signed_math = false;
		bool accumulate = false;
		bool accumulate_overflow = false;
	};

	struct MIKEY
	{
		// 0xfd8a IODIR
		u8 IODIR() const { return data[0x8a]; } // Parallel I/O direction

		// 0xfd8b IODAT
		u8 IODAT() const { return data[0x8b]; } // Parallel I/O data

		// 0xfd92 DISPCTL Video bus request enable
		//bool COLOR()       const { return BIT(data[0x92], 3); } // Color(1) or Monochrome(0) display (not implemented)
		//bool FOURBIT()     const { return BIT(data[0x92], 2); } // 4bpp(1) or 2bpp(0) color (not implemented)
		bool   FLIP_SCREEN() const { return BIT(data[0x92], 1); } // Flipped screen
		//bool VIDEO_DMA()   const { return BIT(data[0x92], 0); } // Video DMA enabled (not implemented)

		u8 data[0x100] = {0};
		u16 disp_addr = 0;
		bool vb_rest = false;
		u8 interrupt = 0;
	};

	struct LYNX_TIMER
	{
		bool   int_en()      const { return BIT(cntrl1, 7); }         // Interrupt enable
		//bool reset_done()  const { return BIT(cntrl1, 6); }         // Reset timer done flag
		bool   reload_en()   const { return BIT(cntrl1, 4); }         // Reload enable
		bool   count_en()    const { return BIT(cntrl1, 3); }         // Count enable
		u8     timer_clock() const { return BIT(cntrl1, 0, 3); }      // Timer clock
		bool   linked()      const { return timer_clock() == 0b111; } // Linked timer?

		bool   timer_done()  const { return BIT(cntrl2, 3); }         // Timer done flag
		//bool last_clock()  const { return BIT(cntrl2, 2); }         // Last clock (not implemented)
		//bool borrow_in()   const { return BIT(cntrl2, 1); }         // Borrow in (not implemented)
		bool   borrow_out()  const { return BIT(cntrl2, 0); }         // Borrow out

		// set timer done flag
		void set_timer_done(bool set)
		{
			if (set) // set
				cntrl2 |= 8;
			else // clear
				cntrl2 &= ~8;
		}

		// set borrow out flag
		void set_borrow_out(bool set)
		{
			if (set) // set
				cntrl2 |= 1;
			else // clear
				cntrl2 &= ~1;
		}

		u8   bakup = 0;
		u8   cntrl1 = 0;
		u8   cntrl2 = 0;
		u8   counter = 0;
		emu_timer *timer = nullptr;
		bool      timer_active = false;
	};

	// devices
	required_shared_ptr<u8> m_dram; // 2 64Kx4 bit DRAMs connected to CPU
	required_device<cpu_device> m_maincpu;
	required_device<lynx_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	// memory views
	memory_view m_suzy_view;
	memory_view m_mikey_view;
	memory_view m_rom_view;
	memory_view m_vector_view;

	// io ports
	required_ioport m_joy_io;
	required_ioport m_pause_io;

	// timers
	emu_timer *m_blitter_timer = nullptr;
	emu_timer *m_loopback_timer = nullptr;
	emu_timer *m_uart_timer = nullptr;

	// connected to cartridge slot
	inline u32 get_cart_addr() { return (BIT(m_mikey.IODAT(), 4) * m_audin_offset) + (m_cart_addr_block * m_granularity) + m_cart_addr_counter; }
	u8 m_cart_addr_block = 0; // Address block (74HC164 at schematics, connected to cartridge slot A12...A19)
	u16 m_cart_addr_counter = 0; // Address counter (4040 at schematics, connected to cartridge slot A0...A10)
	u16 m_granularity = 0; // Address counter granularity
	u32 m_audin_offset = 0; // Some cartridge uses AUDIN pin for bankswitch
	// AUDIN pin, can be input or output

	// internal states
	int m_sign_AB = 1;
	int m_sign_CD = 1;
	int m_rotate = 0;
	u8 m_memory_config;
	u32 m_pixclock = ~0;
	u8 m_hcount = ~0;
	u8 m_vcount = ~0;

	BLITTER m_blitter;
	SUZY m_suzy;
	MIKEY m_mikey;
	UART m_uart;
	LYNX_TIMER m_timer[NR_LYNX_TIMERS];

	bitmap_rgb32 m_bitmap;
	bitmap_rgb32 m_bitmap_temp;

	void cpu_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 suzy_read(offs_t offset);
	void suzy_write(offs_t offset, u8 data);
	void uart_w(offs_t offset, u8 data);
	u8 uart_r(offs_t offset);
	u8 mikey_read(offs_t offset);
	void mikey_write(offs_t offset, u8 data);
	u8 memory_config_r();
	void memory_config_w(u8 data);
	void suzy_divide();
	void suzy_multiply();
	u8 timer_read(int which, int offset);
	void timer_write(int which, int offset, u8 data);
	void update_screen_timing();
	void sound_cb();
	TIMER_CALLBACK_MEMBER(blitter_timer);
	TIMER_CALLBACK_MEMBER(timer_shot);
	TIMER_CALLBACK_MEMBER(uart_loopback_timer);
	TIMER_CALLBACK_MEMBER(uart_timer);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	// DRAM accessor, 0xfff8-0xfff9 Reserved?
	inline u8 dram_byte_r(u16 addr) { return ((addr & 0xfffe) != 0xfff8) ? m_dram[addr] : 0; }
	inline void dram_byte_w(u16 addr, u8 data, u8 mem_mask = ~0) { if ((addr & 0xfffe) != 0xfff8) { COMBINE_DATA(&m_dram[addr]); } }
	inline u16 dram_word_r(u16 addr) { return dram_byte_r(addr) | (u16(dram_byte_r((addr + 1) & 0xffff)) << 8); }

	inline void plot_pixel(const s16 x, const s16 y, const u8 color);
	void blit_do_work(const s16 y, const int xdir, const int bits_per_pixel, const u8 mask);
	void blit_rle_do_work(const s16 y, const int xdir, const int bits_per_pixel, const u8 mask);
	void blit_lines();
	void blitter();
	void draw_line();
	void timer_init(int which);
	void timer_signal_irq(int which);
	void timer_count_down(int which);
	u32 time_factor(int val);
	void uart_reset();
	void interrupt_set(u8 line);
	void interrupt_update();
	std::pair<std::error_condition, std::string> verify_cart(const char *header, int kind);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
};


/*---------- suzy registers ------------- */
#define TMPADRL     0x00    // Temporary address (not sure what this is used for)
#define TMPADRH     0x01
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
#define SPRDLINEL   0x12    // Sprite data start address
#define SPRDLINEH   0x13
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
#define SCBADRL     0x2C    // Address of Current SCB
#define SCBADRH     0x2D
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

#define SPRCTL0     0x80 // Sprite Control Bits 0 (W)(U)
#define SPRCTL1     0x81 // Sprite Control Bits 1 (W)(U)
#define SPRCOLL     0x82 // Sprite Collision Number (W)
#define SPRINIT     0x83 // Sprite Initialization Bits (W)(U)

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
#define SCB_VPOSSTRT    0x09    // L,H Starting Vpos
#define SCB_SPRHSIZ     0x0B    // L,H H Size
#define SCB_SPRVSIZ     0x0D    // L,H V Size
#define SCB_STRETCH     0x0F    // L H H/V Size Adder
#define SCB_TILT        0x11    // L,H H Position Adder

#endif // MAME_ATARI_LYNX_H
