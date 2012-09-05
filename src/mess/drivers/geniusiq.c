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
    ???????? Cartridge port

TODO:
    - Mouse input
    - Sound
    - German keyboard layout
    - Cartridge
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

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/intelfsh.h"

#define MACHINE_RESET_MEMBER(name) void name::machine_reset()

#define KEYBOARD_QUEUE_SIZE 	8

class geniusiq_state : public driver_device
{
public:
	geniusiq_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash(*this, "flash"),
		m_vram(*this, "vram"),
		m_mouse_gfx(*this, "mouse_gfx")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<intelfsh8_device> m_flash;
	required_shared_ptr<UINT16>	m_vram;
	required_shared_ptr<UINT16>	m_mouse_gfx;
	virtual void machine_reset();
	virtual void palette_init();
	virtual UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(flash_r);
	DECLARE_WRITE8_MEMBER(flash_w);
	DECLARE_READ16_MEMBER(input_r);
	DECLARE_WRITE16_MEMBER(mouse_pos_w);
	DECLARE_INPUT_CHANGED_MEMBER(send_input);
	DECLARE_WRITE16_MEMBER(gfx_base_w);
	DECLARE_WRITE16_MEMBER(gfx_dest_w);
	DECLARE_WRITE16_MEMBER(gfx_color_w);
	DECLARE_WRITE16_MEMBER(gfx_idx_w);

	DECLARE_READ16_MEMBER(unk0_r) { return 0; }
	DECLARE_READ16_MEMBER(unk_r) { return machine().rand(); }

private:
	UINT16		m_gfx_y;
	UINT16		m_gfx_x;
	UINT32		m_gfx_base;
	UINT8		m_gfx_color[2];
	UINT16		m_mouse_posx;
	UINT16		m_mouse_posy;
	struct
	{
		UINT16	buffer[KEYBOARD_QUEUE_SIZE];
		int 	head;
		int 	tail;
	} m_keyboard;
};


void geniusiq_state::palette_init()
{
	// shades need to be verified
	const UINT8 palette[] =
	{
		0x00, 0x00, 0x00,    // Black?? (used in the cursor for transparency)
		0xff, 0xff, 0xff,    // White
		0xa0, 0xa0, 0xa0,    // Light grey
		0x7f, 0x7f, 0x7f,    // Dark grey
		0x00, 0x00, 0x00,    // Black
		0x00, 0x60, 0xff,    // Sky blue
		0x00, 0x00, 0xff,    // Blue
		0xff, 0x00, 0x00,    // Red
		0x00, 0xff, 0x00,    // Green
		0x00, 0x7f, 0x00,    // Dark green
		0xff, 0xff, 0x00,    // Yellow
		0xff, 0x7f, 0x00,    // Orange
		0x7f, 0x40, 0x00,    // Brown
		0x60, 0x40, 0x00,    // Dark brown
		0x60, 0x00, 0xff,    // Mauve
		0xff, 0x00, 0xff     // Pink
	};

	for (int i=0; i<ARRAY_LENGTH(palette)/3; i++)
		palette_set_color_rgb(machine(), i, palette[i*3], palette[i*3+1], palette[i*3+2]);
}

UINT32 geniusiq_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y=0; y<256; y++)
		for (int x=0; x<256; x+=2)
		{
			UINT16 data = m_vram[(y*256 + x)>>1];

			for(int b=0; b<4; b++)
			{
				bitmap.pix16(y, x*2 + b) = (data>>12) & 0x0f;
				data <<= 4;
			}
		}

	// mouse cursor
	for (int y=0; y<16; y++)
		for (int x=0; x<6; x+=2)
		{
			UINT16 data = m_mouse_gfx[(y*6 + x)>>1];

			for(int b=0; b<4; b++)
			{
				UINT8 pen = (data>>12) & 0x0f;

				// I assume color 0 is transparent
				if(pen != 0 && screen.visible_area().contains(m_mouse_posx + x*2 + b, m_mouse_posy + y))
					bitmap.pix16(m_mouse_posy + y, m_mouse_posx + x*2 + b) = pen;
				data <<= 4;
			}
		}

	return 0;
}

READ8_MEMBER(geniusiq_state::flash_r)
{
	return m_flash->read(offset);
}

WRITE8_MEMBER(geniusiq_state::flash_w)
{
	m_flash->write(offset, data);
}

WRITE16_MEMBER( geniusiq_state::mouse_pos_w )
{
	switch (offset)
	{
		case 0:		m_mouse_posx = data;	break;
		case 1:		m_mouse_posy = data;	break;
	}
}

WRITE16_MEMBER(geniusiq_state::gfx_color_w)
{
	m_gfx_color[offset & 1] = data & 0x0f;
}

WRITE16_MEMBER(geniusiq_state::gfx_dest_w)
{
	if (offset)
		m_gfx_y = data;
	else
		m_gfx_x = data;
}

WRITE16_MEMBER(geniusiq_state::gfx_base_w)
{
	if (offset)
		m_gfx_base = (m_gfx_base & 0xffff0000) | (data<<0);
	else
		m_gfx_base = (m_gfx_base & 0x0000ffff) | (data<<16);
}

WRITE16_MEMBER(geniusiq_state::gfx_idx_w)
{
	UINT16 *gfx = ((UINT16 *)(*memregion("maincpu"))) + ((m_gfx_base + data*32)>>1);

	// first 16 bits are used to define the character size
	UINT8 gfx_heigh = (gfx[0]>>0) & 0xff;
	UINT8 gfx_width = (gfx[0]>>8) & 0xff;

	for(int y=0; y<gfx_heigh; y++)
		for(int x=0; x<gfx_width; x++)
		{
			UINT16 src = gfx[y + 1];
			UINT32 dst = (m_gfx_y + y)*512 + (m_gfx_x + x);
			UINT8 pen = m_gfx_color[BIT(src,15-x)];
			int bit_pos = (3 - (dst & 3)) << 2;

			m_vram[dst>>2] = (m_vram[dst>>2] & ~(0x0f << bit_pos)) | (pen << bit_pos);
		}
}

READ16_MEMBER( geniusiq_state::input_r )
{
	/*
        this is guesswork and may not be correct

        xxxx xxx- ---- ----     unknown
        ---- ---x ---- ----     used for indicate if the data read is valid (if not set the other bits are discarded)
        ---- ---- x--- ----     if set indicates a KeyUp otherwise a KeyDown
        ---- ---- -xxx xxxx     this is the scan code
    */

	UINT16 data = 0;

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

INPUT_CHANGED_MEMBER( geniusiq_state::send_input )
{
	m_keyboard.buffer[m_keyboard.tail] = (UINT8)(FPTR)param;

	// set bit 7 if the key is released
	if (!newval)
		m_keyboard.buffer[m_keyboard.tail] |= 0x80;

	m_keyboard.tail = (m_keyboard.tail+1) % KEYBOARD_QUEUE_SIZE;

	// new input from keyboard
	m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}


static ADDRESS_MAP_START(geniusiq_mem, AS_PROGRAM, 16, geniusiq_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1FFFFF) AM_ROM
	AM_RANGE(0x200000, 0x23FFFF) AM_RAM
	AM_RANGE(0x300000, 0x30FFFF) AM_RAM		AM_SHARE("vram")
	AM_RANGE(0x310000, 0x31FFFF) AM_RAM
	AM_RANGE(0x400000, 0x41ffff) AM_MIRROR(0x0e0000) AM_READWRITE8(flash_r, flash_w, 0x00ff)
	AM_RANGE(0x600300, 0x600301) AM_READ(input_r)
	//AM_RANGE(0x600500, 0x60050f)                      // read during IRQ 5
	//AM_RANGE(0x600600, 0x600605)                      // sound ??
	AM_RANGE(0x600606, 0x600609) AM_WRITE(gfx_base_w)
	AM_RANGE(0x60060a, 0x60060b) AM_WRITE(gfx_idx_w)
	//AM_RANGE(0x600802, 0x600803)                      // cartridge state
	AM_RANGE(0x600918, 0x600919) AM_READ(unk0_r)        // loop at start if bit 0 is set
	AM_RANGE(0x601008, 0x601009) AM_READ(unk_r)			// unknown, read at start and expect that bit 2 changes several times before continue
	AM_RANGE(0x601010, 0x601011) AM_READ(unk0_r)		// loop at start if bit 1 is set
	AM_RANGE(0x601018, 0x60101b) AM_WRITE(gfx_dest_w)
	AM_RANGE(0x60101c, 0x60101f) AM_WRITE(gfx_color_w)
	AM_RANGE(0x601060, 0x601063) AM_WRITE(mouse_pos_w)
	AM_RANGE(0x601100, 0x6011ff) AM_RAM		AM_SHARE("mouse_gfx")	// mouse cursor gfx (12x16)
	//AM_RANGE(0xa00000, 0xa?????)                      // cartridge ??
	// 0x600000 : some memory mapped hardware
ADDRESS_MAP_END


static INPUT_CHANGED( trigger_irq )
{
	cputag_set_input_line(field.machine(), "maincpu", (int)(FPTR)param, newval ? HOLD_LINE : CLEAR_LINE);
}

/* Input ports */
static INPUT_PORTS_START( geniusiq )
	PORT_START( "IN0" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x00 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x01 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x02 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LSHIFT )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x03 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_RSHIFT )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x04 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_LALT )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x05 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x06 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x07 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x08 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x09 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x0f )

	PORT_START( "IN1" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x10 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x11 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x12 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x13 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x14 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x15 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x16 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_QUOTE )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x17 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSLASH ) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x18 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_EQUALS )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x19 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_CLOSEBRACE ) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_OPENBRACE )  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SLASH )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_M )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_P )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_TILDE )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x1f )

	PORT_START( "IN2" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x20 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_F2 )		PORT_NAME("Enter") PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x21 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSPACE ) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x22 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_L )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x23 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_O )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x24 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_9 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x25 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x26 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x27 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x28 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PGDN )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x29 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PGUP )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_STOP )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COLON )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_K )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_I )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Button 1")	PORT_CODE( KEYCODE_ENTER )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x2f )

	PORT_START( "IN3" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_8 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x30 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ESC )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x31 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_COMMA )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x32 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_J )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x33 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_U )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x34 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x35 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x36 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x37 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x38 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PRTSCR )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x39 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_MINUS )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_DEL )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_N )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_H )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Y )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_NUMLOCK )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x3f )

	PORT_START( "IN4" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x40 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_TAB )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x41 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_B )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x42 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_G )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x43 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_T )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x44 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x45 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x46 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_CAPSLOCK )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x47 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x48 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_HOME )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x49 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_BACKSLASH2 ) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SPACE )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_V )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_F )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_R )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Up")			PORT_CODE( KEYCODE_UP )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x4f )

	PORT_START( "IN5" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x50 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_C )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x51 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_D )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x52 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_E )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x53 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x54 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x55 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x56 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Down")		PORT_CODE( KEYCODE_DOWN )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x57 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_INSERT )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x58 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_X )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x59 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_S )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Z )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Left")		PORT_CODE( KEYCODE_LEFT )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x5f )

	PORT_START( "IN6" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help")				PORT_CODE( KEYCODE_F1 )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x60 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x61 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_END )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x62 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_W )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x63 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_Q )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x64 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_A )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x65 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1 )			PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x66 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mouse Right")		PORT_CODE( KEYCODE_RIGHT )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x67 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x68 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x69 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_DEL_PAD )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_SLASH_PAD )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ASTERISK )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_MINUS_PAD ) PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_PLUS_PAD )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_ENTER_PAD )	PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x6f )

	PORT_START( "IN7" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_0_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x70 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x71 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x72 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x73 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x74 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x75 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x76 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x77 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_8_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x78 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_9_PAD )		PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x79 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7a )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7b )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7c )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7d )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7e )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )		//  PORT_CHANGED_MEMBER( DEVICE_SELF, geniusiq_state, send_input, 0x7f )

	PORT_START( "DEBUG" )	// for debug purposes, to be removed in the end
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_1 ) PORT_NAME( "IRQ 1" ) PORT_CHANGED( trigger_irq, 1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_2 ) PORT_NAME( "IRQ 2" ) PORT_CHANGED( trigger_irq, 2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_3 ) PORT_NAME( "IRQ 3" ) PORT_CHANGED( trigger_irq, 3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_4 ) PORT_NAME( "IRQ 4" ) PORT_CHANGED( trigger_irq, 4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_5 ) PORT_NAME( "IRQ 5" ) PORT_CHANGED( trigger_irq, 5 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_6 ) PORT_NAME( "IRQ 6" ) PORT_CHANGED( trigger_irq, 6 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE( KEYCODE_7 ) PORT_NAME( "IRQ 7" ) PORT_CHANGED( trigger_irq, 7 )
INPUT_PORTS_END


MACHINE_RESET_MEMBER( geniusiq_state )
{
	m_keyboard.head = m_keyboard.tail = 0;

	m_gfx_y = 0;
	m_gfx_x = 0;
	m_gfx_base = 0;
	m_gfx_color[0] = m_gfx_color[1] = 0;
	m_mouse_posx = 0;
	m_mouse_posy = 0;
}

static MACHINE_CONFIG_START( geniusiq, geniusiq_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2) // The main crystal is at 32MHz, not sure whats the CPU freq
	MCFG_CPU_PROGRAM_MAP(geniusiq_mem)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER( geniusiq_state, screen_update )
	MCFG_PALETTE_LENGTH(16)

	/* internal flash */
	MCFG_AMD_29F010_ADD("flash")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( geniusiq )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "geniusiq.bin", 0x0000, 0x200000, CRC(9b06cbf1) SHA1(b9438494a9575f78117c0033761f899e3c14e292) )
ROM_END

ROM_START( geniusiq_de )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD( "german.rom", 0x0000, 0x200000, CRC(a98fc3ff) SHA1(de76a5898182bd0180bd2b3e34c4502f0918a3fa) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                FULLNAME               FLAGS */
COMP( 1997, geniusiq,      0,   		   0,    geniusiq,   geniusiq, driver_device,  0,  "Video Technology", "Genius IQ 128 (France)", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1997, geniusiq_de,   geniusiq,       0,    geniusiq,   geniusiq, driver_device,  0,  "Video Technology", "Genius IQ 128 (Germany)", GAME_NOT_WORKING | GAME_NO_SOUND)
