// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************
Video Technology Genius computers:
    VTech Genius PC (France)
    VTech Genius IQ 512 (Germany)
    The French packaging mentions distributions in Switzerland, the Netherlands,
    USA, Canada, and UK as well. Looking for more information and ROM dumps.

System driver:

    Adrien Destugues <pulkomandy@gmail.com>, May 2012
      - First attempt

Memory map:
    00000000 System ROM (2MB)
    00200000 RAM (256K)
    00400000 Flash memory (128K)
    00600000 Some memory mapped hardware
    00a00000 Cartridge port

TODO:
    - Sound
    - Flash cartridge
    - Dump the MCU and rewrites everything using low-level emulation
    - Check with different countries ROMs

Not very much is known about this computer released in 1997.



PCB - German Version:
                       +----------------------------------+   +-------------------+
 +-----------------+   |                                  |   |                   +-------+   +--------------+
 |                 +---+                                  +---+                           +---+              |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                            ###########################   +---------+                                      +-------+
 |                            #                         #   | XTAL    |                           +-------+   -------|
 |    +-------+               # +-------------------+   #   |26.601712|                           | VTECH |   -------|
 |    |LMT324D|               # | PALCE20V8H-25PC   |   #   |KDS17H   |                           |LH537- |   -------|
 |    +-------+               # |                   |   #   +---------+                           |   -NWL|   -------|
 |                            # +-------------------+   ########                   +--------+     |       |   -------|
 |                            #                                #                   |        |     |1997   |   -------|
 |                            # +-------------+     +-------+  #                   |        |     |27-5947|   -------|
 |                            # |MC74HC4060AN |     |MEC/ZTB|  #                   |AM29F010|     |-00    |   -------|
 |                            # |             |     | 307D  |  #                   |-120PC  |     |       |   -------|
 |                            # +-------------+     |       |  #                   |        |     |9741 D |   -------|
 |                            #010                  +-------+  #                   |        |     +-------+  +-------+
 |                            ##################################                   |9731MBM |                |
 |                                         |                  |                    |       A|                |
 |                                         |       VTECH      |                    |        |                |
 |                                         |   27-05793-0-0   |                    |        |                |
 |                                         |     ZKAL9736     |                    |        |                |
 |                                         |                  |                    |        |                |
 |                                         |                  |                    |        |                |
 |                                         |                  |                    |        |                |
 |           +--------+                    |                  |                    |1991 AMD|                |
 |           |BH7236AF|                    |                  |                    +--------+                |
 |           +--------+                    +------------------+                                              |
 |                           +-----+                                                                         |
 |                           |XTAL |                                                                         |
 |                           |     |                                                                         |
 |                           |32.00|                                                                         |
 |                           |0    |                                                                         |
 |                           +-----+                                                            9740         |
 |                                                                +---------+                   GER          |
 |                                     +----------+               |MC68EC000|                   016          |
 |                                     |HY534256AL|               |FU16     |                                |
 |                                     |J-60      |               |         |                                |
 |                                     +----------+               |    0G74K|                                |
 |                                                                | HHIG9728|                                |
 |                                     +----------+               +---------+                                |
 |    +------+                         |HY534256AL|                                                          |
 |    |9727H |                         |J-60      |                                                          |
 |    |C807U-|                         +----------+                                                          |
 |    |1225  |                                                                                               |
 |    +------+                                                                                               |
 |                                                                                           35-13300-28     |
 |                                                                                                           |
 |                                                              35-13300-28 702748-E CS                      |
 |      +----+                                      +----+      E403                         +----+          |
 +------+    +--------------------------------------+    +-----------------------------------+    +----------+



 IQ TV 512 PCB (German version)
                       +----------------------------------+   +-------------------+
 +-----------------+   |                                  |   |                   +-------+   +--------------+
 |                 +---+                                  +---+                           +---+              |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                                                                                                           |
 |                            ###########################   +---------+                                      +-------+
 |                            #                         #   | XTAL    |                           +-------+   -------|
 |    +-------+               # +-------------------+   #   |26.601712|                           | VTECH |   -------|
 |    |LM324D |               # | PALCE20V8H-25PC   |   #   |KDS17H   |                           |LH537- |   -------|
 |    +-------+               # |                   |   #   +---------+                           |   -NUA|   -------|
 |                            # +-------------------+   ########                   +--------+     |       |   -------|
 |                            #                                #                   |        |     |1998   |   -------|
 |                            # +-------------+     +-------+  #                   |        |     |27-0617|   -------|
 |                            # |MC74HC4060AN |     |MEC/ZTB|  #                   |AM29F040|     |1-000  |   -------|
 |                            # |             |     | 307D  |  #                   |B-120PC |     |       |   -------|
 |                            # +-------------+     |       |  #                   |        |     |9824 D |   -------|
 |                            #010                  +-------+  #                   |        |     +-------+  +-------+
 |                            ##################################                   |9808MBM |                |
 |                                         |                  |                    |       A|                |
 |                                         |       VTECH      |                    |        |                |
 |                                         |   27-05793-0-0   |                    |        |                |
 |                                         |     ZKAH9805     |                    |        |                |
 |                                         |                  |                    |        |                |
 |                                         |                  |                    |        |                |
 |                                         |                  |                    |        |                |
 |           +--------+                    |                  |                    |1993 AMD|                |
 |           |BH7236AF|                    |                  |                    +--------+                |
 |           +--------+                    +------------------+                                              |
 |                           +-----+                                                                         |
 |                           |XTAL |                                                                         |
 |                           |     |                                                                         |
 |                           |32.00|                                                                         |
 |                           |0    |                                                                         |
 |                           +-----+                                                            9749         |
 |                                                                +---------+                   GER          |
 |                                     +----------+               |MC68EC000|                   023          |
 |                                     |HY534256AL|               |FU16     |                                |
 |                                     |J-60      |               |         |                                |
 |                                     +----------+               |    0G74K|                                |
 |                                                                | HHIG9728|                                |
 |                                     +----------+               +---------+                                |
 |    +------+                         |HY534256AL|                                                          |
 |    |9805HB|                         |J-60      |                                                          |
 |    |C807U-|                         +----------+                                                          |
 |    |1225  |                                                                                               |
 |    +------+                                                                                               |
 |                                                                                           35-13300-28     |
 |                                                                                                           |
 |                                                              35-13300-28 702749-E CS                      |
 |      +----+                                      +----+      E493                         +----+          |
 +------+    +--------------------------------------+    +-----------------------------------+    +----------+



****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/intelfsh.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"

#define KEYBOARD_QUEUE_SIZE     0x80

class geniusiq_state : public driver_device
{
public:
	geniusiq_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_rom(*this, "maincpu"),
		m_vram(*this, "vram"),
		m_mouse_gfx(*this, "mouse_gfx"),
		m_cart_state(IQ128_NO_CART)
	{ }

	void iqtv512(machine_config &config);
	void iq128(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(send_input);
	DECLARE_INPUT_CHANGED_MEMBER(send_mouse_input);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	enum
	{
		IQ128_ROM_CART      = 0x00,
		IQ128_ROMLESS1_CART = 0x01,
		IQ128_ROMLESS2_CART = 0x02,
		IQ128_NO_CART       = 0x03
	};

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_region_ptr<uint16_t> m_rom;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_mouse_gfx;

	void geniusiq_palette(palette_device &palette) const;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t input_r();
	void mouse_pos_w(offs_t offset, uint16_t data);
	void gfx_base_w(offs_t offset, uint16_t data);
	void gfx_dest_w(offs_t offset, uint16_t data);
	void gfx_color_w(offs_t offset, uint16_t data);
	void gfx_idx_w(uint16_t data);
	void queue_input(uint16_t data);
	uint16_t cart_state_r();

	uint16_t unk0_r() { return 0; }
	uint16_t unk_r() { return machine().rand(); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(cart_unload);

	void geniusiq_mem(address_map &map);

	uint16_t      m_gfx_y;
	uint16_t      m_gfx_x;
	uint32_t      m_gfx_base;
	uint8_t       m_gfx_color[2];
	uint8_t       m_mouse_posx;
	uint8_t       m_mouse_posy;
	uint16_t      m_mouse_gfx_posx;
	uint16_t      m_mouse_gfx_posy;
	uint8_t       m_cart_state;
	struct
	{
		uint16_t  buffer[KEYBOARD_QUEUE_SIZE];
		int     head;
		int     tail;
	} m_keyboard;
};


void geniusiq_state::geniusiq_palette(palette_device &palette) const
{
	// shades need to be verified
	constexpr rgb_t palette_val[] =
	{
		{ 0x00, 0x00, 0x00 },   // Black?? (used in the cursor for transparency)
		{ 0xff, 0xff, 0xff },   // White
		{ 0xa0, 0xa0, 0xa0 },   // Light grey
		{ 0x7f, 0x7f, 0x7f },   // Dark grey
		{ 0x00, 0x00, 0x00 },   // Black
		{ 0x00, 0x60, 0xff },   // Sky blue
		{ 0x00, 0x00, 0xff },   // Blue
		{ 0xff, 0x00, 0x00 },   // Red
		{ 0x00, 0xff, 0x00 },   // Green
		{ 0x00, 0x7f, 0x00 },   // Dark green
		{ 0xff, 0xff, 0x00 },   // Yellow
		{ 0xff, 0x7f, 0x00 },   // Orange
		{ 0x7f, 0x40, 0x00 },   // Brown
		{ 0x60, 0x40, 0x00 },   // Dark brown
		{ 0x60, 0x00, 0xff },   // Mauve
		{ 0xff, 0x00, 0xff }    // Pink
	};

	palette.set_pen_colors(0, palette_val);
}


uint32_t geniusiq_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y=0; y<256; y++)
		for (int x=0; x<256; x+=2)
		{
			uint16_t data = m_vram[(y*256 + x)>>1];

			for(int b=0; b<4; b++)
			{
				bitmap.pix(y, x*2 + b) = (data>>12) & 0x0f;
				data <<= 4;
			}
		}

	// mouse cursor
	for (int y=0; y<16; y++)
		for (int x=0; x<6; x+=2)
		{
			uint16_t data = m_mouse_gfx[(y*6 + x)>>1];

			for(int b=0; b<8; b++)
			{
				uint8_t pen = (data>>14) & 0x03;

				// I assume color 0 is transparent
				if(pen != 0 && screen.visible_area().contains(m_mouse_gfx_posx + x*4 + b, m_mouse_gfx_posy + y))
					bitmap.pix(m_mouse_gfx_posy + y, m_mouse_gfx_posx + x*4 + b) = pen;
				data <<= 2;
			}
		}

	return 0;
}

uint16_t geniusiq_state::cart_state_r()
{
	return m_cart_state;
}

void geniusiq_state::mouse_pos_w(offs_t offset, uint16_t data)
{
	if (offset)
		m_mouse_gfx_posy = data;
	else
		m_mouse_gfx_posx = data;
}

void geniusiq_state::gfx_color_w(offs_t offset, uint16_t data)
{
	m_gfx_color[offset & 1] = data & 0x0f;
}

void geniusiq_state::gfx_dest_w(offs_t offset, uint16_t data)
{
	if (offset)
		m_gfx_y = data;
	else
		m_gfx_x = data;
}

void geniusiq_state::gfx_base_w(offs_t offset, uint16_t data)
{
	if (offset)
		m_gfx_base = (m_gfx_base & 0xffff0000) | (data<<0);
	else
		m_gfx_base = (m_gfx_base & 0x0000ffff) | (data<<16);
}

void geniusiq_state::gfx_idx_w(uint16_t data)
{
	uint16_t *gfx = m_rom + ((m_gfx_base + (data & 0xff)*32)>>1);

	// first 16 bits are used to define the character size
	uint8_t gfx_heigh = (gfx[0]>>0) & 0xff;
	uint8_t gfx_width = (gfx[0]>>8) & 0xff;

	for(int y=0; y<gfx_heigh; y++)
		for(int x=0; x<gfx_width; x++)
		{
			uint16_t src = gfx[y + 1];
			uint32_t dst = (m_gfx_y + y)*512 + (m_gfx_x + x);
			uint8_t pen = m_gfx_color[BIT(src,15-x)];
			int bit_pos = (3 - (dst & 3)) << 2;

			m_vram[dst>>2] = (m_vram[dst>>2] & ~(0x0f << bit_pos)) | (pen << bit_pos);
		}
}

uint16_t geniusiq_state::input_r()
{
	/*
	    this is guesswork and may not be correct

	    xxxx xxx- ---- ----     unknown
	    ---- ---x ---- ----     used for indicate if the data read is valid (if not set the other bits are discarded)
	    ---- ---- x--- ----     if set indicates a KeyUp otherwise a KeyDown
	    ---- ---- -xxx xxxx     this is the scan code
	*/

	uint16_t data = 0;

	if(m_keyboard.head != m_keyboard.tail)
	{
		data = m_keyboard.buffer[m_keyboard.head];

		m_keyboard.head = (m_keyboard.head+1) % KEYBOARD_QUEUE_SIZE;

		data |= (1<<8);
	}

	if(m_keyboard.head == m_keyboard.tail)
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);

	return data;
}

void geniusiq_state::queue_input(uint16_t data)
{
	m_keyboard.buffer[m_keyboard.tail] = data;

	m_keyboard.tail = (m_keyboard.tail+1) % KEYBOARD_QUEUE_SIZE;

	// new data in queue
	m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( geniusiq_state::send_mouse_input )
{
	uint8_t new_mouse_x = ioport("MOUSEX")->read();
	uint8_t new_mouse_y = ioport("MOUSEY")->read();
	uint8_t mouse_buttons = ioport("MOUSE")->read();

	uint8_t delta_x = (uint8_t)(new_mouse_x - m_mouse_posx);
	uint8_t delta_y = (uint8_t)(new_mouse_y - m_mouse_posy);
	m_mouse_posx = new_mouse_x;
	m_mouse_posy = new_mouse_y;

	queue_input(0x1000 | 0x40 | mouse_buttons | ((delta_y>>4) & 0x0c) | ((delta_x>>6) & 0x03));
	queue_input(0x1000 | (delta_x & 0x3f));
	queue_input(0x1000 | (delta_y & 0x3f));
}

INPUT_CHANGED_MEMBER( geniusiq_state::send_input )
{
	uint16_t data = (uint16_t)param;

	// set bit 7 if the key is released
	if (!newval)
		data |= 0x80;

	queue_input(data);
}


void geniusiq_state::geniusiq_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1FFFFF).rom();
	map(0x200000, 0x23FFFF).ram();
	map(0x300000, 0x30FFFF).ram().share("vram");
	map(0x310000, 0x31FFFF).ram();
	map(0x400000, 0x41ffff).mirror(0x0e0000).rw("flash", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x600300, 0x600301).r(FUNC(geniusiq_state::input_r));
	//map(0x600500, 0x60050f)                      // read during IRQ 5
	//map(0x600600, 0x600605)                      // sound ??
	map(0x600606, 0x600609).w(FUNC(geniusiq_state::gfx_base_w));
	map(0x60060a, 0x60060b).w(FUNC(geniusiq_state::gfx_idx_w));
	map(0x600802, 0x600803).r(FUNC(geniusiq_state::cart_state_r));  // cartridge state
	map(0x600108, 0x600109).r(FUNC(geniusiq_state::unk0_r));        // read before run a BASIC program
	map(0x600918, 0x600919).r(FUNC(geniusiq_state::unk0_r));        // loop at start if bit 0 is set
	map(0x601008, 0x601009).r(FUNC(geniusiq_state::unk_r));         // unknown, read at start and expect that bit 2 changes several times before continue
	map(0x601010, 0x601011).r(FUNC(geniusiq_state::unk0_r));        // loop at start if bit 1 is set
	map(0x601018, 0x60101b).w(FUNC(geniusiq_state::gfx_dest_w));
	map(0x60101c, 0x60101f).w(FUNC(geniusiq_state::gfx_color_w));
	map(0x601060, 0x601063).w(FUNC(geniusiq_state::mouse_pos_w));
	map(0x601100, 0x6011ff).ram().share("mouse_gfx");   // mouse cursor gfx (24x16)
	map(0xa00000, 0xafffff).r(m_cart, FUNC(generic_slot_device::read16_rom));
	// 0x600000 : some memory mapped hardware
}

/* Input ports */
static INPUT_PORTS_START( geniusiq )
	PORT_START( "IN0" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x00 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x01 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LCONTROL )  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x02 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LSHIFT )    PORT_CHAR(UCHAR_SHIFT_1)    PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x03 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_RSHIFT )    PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x04 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LALT )      PORT_CHAR(UCHAR_MAMEKEY(LALT))  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x05 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x06 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x07 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x08 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x09 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F7")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F8")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F9")                PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F10")               PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0f )

	PORT_START( "IN1" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x10 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x11 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x12 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x13 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x14 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x15 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x16 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_QUOTE )     PORT_CHAR('\'') PORT_CHAR('~')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x17 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSLASH ) PORT_CHAR('$')                  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x18 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_EQUALS )    PORT_CHAR('=')  PORT_CHAR('+')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x19 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_CLOSEBRACE ) PORT_CHAR(')') PORT_CHAR(0x00b0)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_OPENBRACE )  PORT_CHAR(0x00f9)  PORT_CHAR('%')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SLASH )     PORT_CHAR('^')                  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_M )         PORT_CHAR('m')  PORT_CHAR('M')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_P )         PORT_CHAR('p')  PORT_CHAR('P')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_TILDE )     PORT_CHAR('!')  PORT_CHAR('*')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1f )

	PORT_START( "IN2" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0 )         PORT_CHAR(0x00e0)   PORT_CHAR('0')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x20 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE( KEYCODE_ENTER )     PORT_CHAR(13)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x21 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Backspace") PORT_CODE( KEYCODE_BACKSPACE ) PORT_CHAR(8)  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x22 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_L )         PORT_CHAR('l')  PORT_CHAR('L')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x23 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_O )         PORT_CHAR('o')  PORT_CHAR('O')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x24 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_9 )         PORT_CHAR(0x00e7)   PORT_CHAR('9')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x25 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x26 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Button 2 (keyboard)") PORT_CODE( KEYCODE_F3 ) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x27 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x28 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PGDN )      PORT_CHAR(UCHAR_MAMEKEY(PGDN))  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x29 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PGUP )      PORT_CHAR(UCHAR_MAMEKEY(PGUP))  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_STOP )      PORT_CHAR(':')  PORT_CHAR('/')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COLON )     PORT_CHAR(';')  PORT_CHAR('.')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_K )         PORT_CHAR('k')  PORT_CHAR('K')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_I )         PORT_CHAR('i')  PORT_CHAR('I')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Button 1 (keyboard)") PORT_CODE( KEYCODE_F2 ) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2f )

	PORT_START( "IN3" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_8 )         PORT_CHAR('_')  PORT_CHAR('8')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x30 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ESC )       PORT_CHAR(UCHAR_MAMEKEY(ESC))   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x31 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COMMA )     PORT_CHAR(',')  PORT_CHAR('?')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x32 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_J )         PORT_CHAR('j')  PORT_CHAR('J')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x33 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_U )         PORT_CHAR('u')  PORT_CHAR('U')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x34 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7 )         PORT_CHAR(0x00e8)   PORT_CHAR('7')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x35 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x36 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x37 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x38 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PRTSCR )    PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))    PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x39 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_MINUS )     PORT_CHAR('-')  PORT_CHAR(0x00a3)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_DEL )       PORT_CHAR(UCHAR_MAMEKEY(DEL))   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_N )         PORT_CHAR('n')  PORT_CHAR('N')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_H )         PORT_CHAR('h')  PORT_CHAR('H')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Y )         PORT_CHAR('y')  PORT_CHAR('Y')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_NUMLOCK )   PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3f )

	PORT_START( "IN4" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6 )         PORT_CHAR('-')  PORT_CHAR('6')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x40 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_TAB )       PORT_CHAR('\t')                 PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x41 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_B )         PORT_CHAR('b')  PORT_CHAR('B')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x42 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_G )         PORT_CHAR('g')  PORT_CHAR('G')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x43 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_T )         PORT_CHAR('t')  PORT_CHAR('T')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x44 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5 )         PORT_CHAR('(')  PORT_CHAR('5')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x45 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x46 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_CAPSLOCK )  PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x47 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x48 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_HOME )      PORT_CHAR(UCHAR_MAMEKEY(HOME))  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x49 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSLASH2 ) /*PORT_CHAR('')*/  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SPACE )     PORT_CHAR(' ')                  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_V )         PORT_CHAR('v')  PORT_CHAR('V')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_F )         PORT_CHAR('f')  PORT_CHAR('F')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_R )         PORT_CHAR('r')  PORT_CHAR('R')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Up Button")   PORT_CODE( KEYCODE_UP )         PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4f )

	PORT_START( "IN5" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4 )         PORT_CHAR('\'') PORT_CHAR('4')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x50 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_C )         PORT_CHAR('c')  PORT_CHAR('C')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x51 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_D )         PORT_CHAR('d')  PORT_CHAR('D')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x52 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_E )         PORT_CHAR('e')  PORT_CHAR('E')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x53 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3 )         PORT_CHAR('"')  PORT_CHAR('3')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x54 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x55 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x56 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Down Button") PORT_CODE( KEYCODE_DOWN )       PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x57 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_INSERT )    PORT_CHAR(UCHAR_MAMEKEY(INSERT))    PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x58 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_X )         PORT_CHAR('x')  PORT_CHAR('X')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x59 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_S )         PORT_CHAR('s')  PORT_CHAR('S')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Z )         PORT_CHAR('z')  PORT_CHAR('Z')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2 )         PORT_CHAR(0x00e9)   PORT_CHAR('2')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Left Button") PORT_CODE( KEYCODE_LEFT )       PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5f )

	PORT_START( "IN6" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help")              PORT_CODE( KEYCODE_F1 )         PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x60 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x61 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_END )       PORT_CHAR(UCHAR_MAMEKEY(END))   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x62 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_W )         PORT_CHAR('w')  PORT_CHAR('W')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x63 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Q )         PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x64 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_A )         PORT_CHAR('a')  PORT_CHAR('A')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x65 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1 )         PORT_CHAR('&')  PORT_CHAR('1')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x66 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Right Button") PORT_CODE( KEYCODE_RIGHT )     PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x67 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x68 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_F4 )        PORT_CHAR(0x00a4)               PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x69 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_DEL_PAD )   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SLASH_PAD ) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ASTERISK )  PORT_CHAR('*')                      PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_MINUS_PAD ) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PLUS_PAD )  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ENTER_PAD ) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6f )

	PORT_START( "IN7" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0_PAD )     PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x70 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1_PAD )     PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x71 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2_PAD )     PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x72 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3_PAD )     PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x73 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4_PAD )     PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x74 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5_PAD )     PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x75 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6_PAD )     PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x76 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7_PAD )     PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x77 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_8_PAD )     PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x78 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_9_PAD )     PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x79 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )      //  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7f )

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)       PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_mouse_input, 0 )

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)       PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_mouse_input, 0 )

	PORT_START("MOUSE")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Mouse Button 2")     PORT_CODE(MOUSECODE_BUTTON2)    PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_mouse_input, 0 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Mouse Button 1")     PORT_CODE(MOUSECODE_BUTTON1)    PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_mouse_input, 0 )
INPUT_PORTS_END


static INPUT_PORTS_START( geniusiq_de )
	PORT_INCLUDE(geniusiq)

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_QUOTE )     PORT_CHAR('^')  PORT_CHAR(0x00b0)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x17 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSLASH ) PORT_CHAR('+')  PORT_CHAR('*')      PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x18 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_EQUALS )    PORT_CHAR('}')  PORT_CHAR('{')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x19 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_CLOSEBRACE ) PORT_CHAR(0x00b0)  PORT_CHAR('?')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_OPENBRACE )  PORT_CHAR(0x00e4)  PORT_CHAR(0x00c4)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SLASH )     PORT_CHAR(0x00fc)   PORT_CHAR(0x00dc)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COLON )     PORT_CHAR(0x00f6)   PORT_CHAR(0x00d6)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1d )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_TILDE )     PORT_CHAR('-')  PORT_CHAR('_')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1f )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0 )         PORT_CHAR('0')  PORT_CHAR('=')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x20 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_9 )         PORT_CHAR('9')  PORT_CHAR(')')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x25 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_STOP )      PORT_CHAR('.')  PORT_CHAR(':')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COMMA )     PORT_CHAR(',')  PORT_CHAR(';')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2c )

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_8 )         PORT_CHAR('8')  PORT_CHAR('(')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x30 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_M )         PORT_CHAR('m')  PORT_CHAR('M')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x32 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7 )         PORT_CHAR('7')  PORT_CHAR('/')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x35 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_MINUS )     PORT_CHAR('#')  PORT_CHAR('\'') PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3a )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Z )         PORT_CHAR('z')  PORT_CHAR('Z')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3e )

	PORT_MODIFY( "IN4" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6 )         PORT_CHAR('6')  PORT_CHAR('&')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x40 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5 )         PORT_CHAR('5')  PORT_CHAR('/')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x45 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSLASH2 ) PORT_CHAR('<') PORT_CHAR('>')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4a )

	PORT_MODIFY( "IN5" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4 )         PORT_CHAR('4')  PORT_CHAR('$')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x50 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3 )         PORT_CHAR('3')  PORT_CHAR(0x00a7)   PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x54 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_W )         PORT_CHAR('w')  PORT_CHAR('W')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2 )         PORT_CHAR('2')  PORT_CHAR('"')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5c )

	PORT_MODIFY( "IN6" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Y )         PORT_CHAR('y')  PORT_CHAR('Y')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x63 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_A )         PORT_CHAR('a')  PORT_CHAR('A')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x64 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Q )         PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x65 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1 )         PORT_CHAR('1')  PORT_CHAR('!')  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x66 )
INPUT_PORTS_END


void geniusiq_state::machine_start()
{
}

void geniusiq_state::machine_reset()
{
	m_keyboard.head = m_keyboard.tail = 0;

	m_gfx_y = 0;
	m_gfx_x = 0;
	m_gfx_base = 0;
	m_gfx_color[0] = m_gfx_color[1] = 0;
	m_mouse_posx = 0;
	m_mouse_posy = 0;
	m_mouse_gfx_posx = 0;
	m_mouse_gfx_posy = 0;
}

DEVICE_IMAGE_LOAD_MEMBER(geniusiq_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	// we always a 0x100000 region, for easier mapping in the memory map
	m_cart->rom_alloc(0x100000, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	m_cart_state = IQ128_ROM_CART;

	if (image.loaded_through_softlist())
	{
		const char *pcb_type = image.get_feature("pcb_type");
		if (pcb_type)
		{
			if (!core_stricmp(pcb_type, "romless1"))
				m_cart_state = IQ128_ROMLESS1_CART;
			if (!core_stricmp(pcb_type, "romless2"))
				m_cart_state = IQ128_ROMLESS2_CART;
		}
	}

	return image_init_result::PASS;
}

DEVICE_IMAGE_UNLOAD_MEMBER(geniusiq_state::cart_unload)
{
	m_cart_state = IQ128_NO_CART;
}


void geniusiq_state::iq128(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2); // The main crystal is at 32MHz, not sure whats the CPU freq
	m_maincpu->set_addrmap(AS_PROGRAM, &geniusiq_state::geniusiq_mem);
	m_maincpu->set_periodic_int(FUNC(geniusiq_state::irq6_line_hold), attotime::from_hz(125));  // the internal clock is increased by 1 sec every 125 interrupts

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(geniusiq_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(geniusiq_state::geniusiq_palette), 16);

	/* internal flash */
	AMD_29F010(config, "flash");

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "iq128_cart"));
	cartslot.set_device_load(FUNC(geniusiq_state::cart_load));
	cartslot.set_device_unload(FUNC(geniusiq_state::cart_unload));

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("iq128");
}

void geniusiq_state::iqtv512(machine_config &config)
{
	iq128(config);
	/* internal flash */
	AMD_29F040(config.replace(), "flash");
}

/* ROM definition */

ROM_START( iq128 )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "27-5947-00.bin", 0x0000, 0x200000, CRC(a98fc3ff) SHA1(de76a5898182bd0180bd2b3e34c4502f0918a3fa) )
ROM_END

ROM_START( iq128_fr )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "geniusiq.bin", 0x0000, 0x200000, CRC(9b06cbf1) SHA1(b9438494a9575f78117c0033761f899e3c14e292) )
ROM_END

ROM_START( iqtv512 )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "27-06171-000.bin", 0x0000, 0x200000, CRC(2597af70) SHA1(9db8151a84517407d380424410b6fa0003ceb1eb) )
ROM_END

ROM_START( itunlim )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "27-06124-002.u3", 0x000000, 0x200000, CRC(0c0753ce) SHA1(d22504d583ca8d6a9d2f56fbaa3e1d52c442a1e9) )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT        CLASS           INIT        COMPANY             FULLNAME                      FLAGS
COMP( 1997, iq128,    0,      0,      iq128,   geniusiq_de, geniusiq_state, empty_init, "Video Technology", "Genius IQ 128 (Germany)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1997, iq128_fr, iq128,  0,      iq128,   geniusiq,    geniusiq_state, empty_init, "Video Technology", "Genius IQ 128 (France)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1998, iqtv512,  0,      0,      iqtv512, geniusiq_de, geniusiq_state, empty_init, "Video Technology", "Genius IQ TV 512 (Germany)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1998, itunlim,  0,      0,      iq128,   geniusiq_de, geniusiq_state, empty_init, "Video Technology", "VTech IT Unlimited (UK)",    MACHINE_NO_SOUND)
