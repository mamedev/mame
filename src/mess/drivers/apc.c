/***************************************************************************

    Advanced Personal Computer (c) 1982 NEC

    preliminary driver by Angelo Salese

    TODO:
    - video emulation (bitmap part)
    - Understand interrupt sources
    - NMI seems valid, dumps a x86 stack to vram?
    - Unknown RTC device type (upd1990a?);
    - What are exactly APU and MPU devices? They sounds scary ...
    - DMA hook-ups
    - serial ports
    - parallel ports
    - Extract info regarding Hard Disk functionality
    - Various unknown ports
    - What kind of external ROM actually maps at 0xa****?

============================================================================
    front ^
          |
    card
    ----
    69PFCU 7220               PFCU1R 2764
    69PTS  7220
    -
    69PFB2 8086/8087   DFBU2J PFBU2L 2732
    69SNB RAM

----------------------------------------------------------------------------
    i/o memory map (preliminary):
    0x00 - 0x1f DMA
    0x20 - 0x23 i8259 master
    0x28 - 0x2f i8259 slave (even), pit8253 (odd)
    0x30 - 0x37 serial i8251, even #1 / odd #2
    0x38 - 0x3f DMA segments
    0x40 - 0x43 upd7220, even chr / odd bitmap
    0x48 - 0x4f keyboard
    0x50 - 0x53 upd765
    0x58        rtc
    0x5a - 0x5e APU
    0x60        MPU (melody)
    0x61 - 0x67 (Mirror of pit8253?)
    0x68 - 0x6f parallel port

----------------------------------------------------------------------------
0xfe3c2: checks if the floppy has a valid string for booting (either "CP/M-86"
         or "MS-DOS"), if not, branches with the successive jne.

***************************************************************************/


#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "machine/upd765.h"
#include "machine/upd1990a.h"
#include "machine/nvram.h"
#include "video/upd7220.h"
#include "imagedev/flopdrv.h"
#include "formats/mfi_dsk.h"
#include "formats/d88_dsk.h"
#include "formats/imd_dsk.h"

//#include "sound/ay8910.h"

#define MAIN_CLOCK XTAL_5MHz

class apc_state : public driver_device
{
public:
	apc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hgdc1(*this, "upd7220_chr"),
		m_hgdc2(*this, "upd7220_btm"),
		m_rtc(*this, "upd1990a"),
		m_i8259_m(*this, "pic8259_master"),
		m_i8259_s(*this, "pic8259_slave"),
		m_fdc(*this, "upd765"),
		m_dmac(*this, "i8237"),
		m_video_ram_1(*this, "video_ram_1"),
		m_video_ram_2(*this, "video_ram_2")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc1;
	required_device<upd7220_device> m_hgdc2;
	required_device<upd1990a_device> m_rtc;
	required_device<pic8259_device> m_i8259_m;
	required_device<pic8259_device> m_i8259_s;
	required_device<upd765a_device> m_fdc;
	required_device<am9517a_device> m_dmac;
	UINT8 *m_char_rom;

	required_shared_ptr<UINT8> m_video_ram_1;
	required_shared_ptr<UINT8> m_video_ram_2;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);


	DECLARE_READ8_MEMBER(apc_port_28_r);
	DECLARE_WRITE8_MEMBER(apc_port_28_w);
	DECLARE_READ8_MEMBER(apc_port_60_r);
	DECLARE_WRITE8_MEMBER(apc_port_60_w);
	DECLARE_READ8_MEMBER(apc_gdc_r);
	DECLARE_WRITE8_MEMBER(apc_gdc_w);
	DECLARE_READ8_MEMBER(apc_kbd_r);
	DECLARE_WRITE8_MEMBER(apc_kbd_w);
	DECLARE_WRITE8_MEMBER(apc_dma_segments_w);
	DECLARE_READ8_MEMBER(apc_dma_r);
	DECLARE_WRITE8_MEMBER(apc_dma_w);
	DECLARE_WRITE8_MEMBER(apc_irq_ack_w);
	DECLARE_READ8_MEMBER(apc_rtc_r);
	DECLARE_WRITE8_MEMBER(apc_rtc_w);

	struct {
		UINT8 status; //status
		UINT8 data; //key data
		UINT8 sig; //switch signal port
		UINT8 sh; //shift switches
	}m_keyb;
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

	DECLARE_WRITE_LINE_MEMBER(apc_master_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(apc_dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(apc_tc_w);
	DECLARE_WRITE_LINE_MEMBER(apc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(apc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(apc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(apc_dack3_w);
	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(apc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(apc_dma_write_byte);

	void fdc_irq(bool state);
	void fdc_drq(bool state);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq);

	DECLARE_DRIVER_INIT(apc);
	DECLARE_PALETTE_INIT(apc);

	int m_dack;
	UINT8 m_dma_offset[4];

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void apc_state::video_start()
{
	m_char_rom = memregion("gfx")->base();
}

UINT32 apc_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(get_black_pen(machine()), cliprect);

	/* graphics */
	m_hgdc2->screen_update(screen, bitmap, cliprect);
	m_hgdc1->screen_update(screen, bitmap, cliprect);

	return 0;
}


static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	// ...
}

static UPD7220_DRAW_TEXT_LINE( hgdc_draw_text )
{
	apc_state *state = device->machine().driver_data<apc_state>();
	int xi,yi;
	int x;
	UINT8 char_size;
//  UINT8 interlace_on;

//  if(state->m_video_ff[DISPLAY_REG] == 0) //screen is off
//      return;

//  interlace_on = state->m_video_reg[2] == 0x10; /* TODO: correct? */
	char_size = 19;

	for(x=0;x<pitch;x++)
	{
		UINT8 tile_data;
		UINT8 u_line, o_line, v_line, reverse, blink;
		UINT8 color;
		UINT8 tile,attr,pen;
		UINT32 tile_addr;

//      tile_addr = addr+(x*(state->m_video_ff[WIDTH40_REG]+1));
		tile_addr = addr+(x*(1));

		tile = state->m_video_ram_1[(tile_addr*2+1) & 0x1fff] & 0x00ff;
		attr = (state->m_video_ram_1[(tile_addr*2 & 0x1fff) | 0x2000] & 0x00ff);

		u_line = attr & 0x01;
		o_line = attr & 0x02;
		v_line = attr & 0x04;
		blink  = attr & 0x08;
		reverse = attr & 0x10;
//      secret= (attr & 1) ^ 1;
		color = (attr & 0xe0) >> 5;

		for(yi=0;yi<lr;yi++)
		{
			for(xi=0;xi<8;xi++)
			{
				int res_x,res_y;

//              res_x = (x*8+xi) * (state->m_video_ff[WIDTH40_REG]+1);
				res_x = (x*8+xi) * (1);
				res_y = y*lr+yi;

				if(res_x > 640 || res_y > 494) //TODO
					continue;

//              tile_data = secret ? 0 : (state->m_char_rom[tile*char_size+interlace_on*0x800+yi]);
				tile_data = (state->m_char_rom[(tile & 0x7f)+((tile & 0x80)<<4)+((yi & 0xf)*0x80)+((yi & 0x10)<<8)]);

				if(reverse) { tile_data^=0xff; }
				if(u_line && yi == lr-1) { tile_data = 0xff; }
				if(o_line && yi == 0) { tile_data = 0xff; }
				if(v_line)  { tile_data|=1; }
				if(blink && device->machine().primary_screen->frame_number() & 0x20) { tile_data = 0; } // TODO: rate & correct behaviour

				if(cursor_on && cursor_addr == tile_addr && device->machine().primary_screen->frame_number() & 0x10)
					tile_data^=0xff;

				if(yi >= char_size)
					pen = 0;
				else
					pen = (tile_data >> (xi) & 1) ? color : 0;

				if(pen)
					bitmap.pix16(res_y, res_x) = pen;

//              if(state->m_video_ff[WIDTH40_REG])
//              {
//                  if(res_x+1 > 640 || res_y > char_size*25) //TODO
//                      continue;

//                  bitmap.pix16(res_y, res_x+1) = pen;
//              }
			}
		}
	}
}

READ8_MEMBER(apc_state::apc_port_28_r)
{
	UINT8 res;

	if(offset & 1)
		res = pit8253_r(machine().device("pit8253"), space, (offset & 6) >> 1);
	else
	{
		if(offset & 4)
		{
			printf("Read undefined port %02x\n",offset+0x28);
			res = 0xff;
		}
		else
			res = pic8259_r(machine().device("pic8259_slave"), space, (offset & 2) >> 1);
	}

	return res;
}

WRITE8_MEMBER(apc_state::apc_port_28_w)
{
	if(offset & 1)
		pit8253_w(machine().device("pit8253"), space, (offset & 6) >> 1, data);
	else
	{
		if(offset & 4)
			printf("Write undefined port %02x\n",offset+0x28);
		else
			pic8259_w(machine().device("pic8259_slave"), space, (offset & 2) >> 1, data);
	}
}


READ8_MEMBER(apc_state::apc_port_60_r)
{
	UINT8 res;

	if(offset & 1)
	{
		printf("Read undefined port %02x\n",offset+0x60);
		res = 0xff;
	}
	else
	{
		//printf("Read melody port %02x\n",offset+0x60);
		res = 0x80;
	}

	return res;
}

WRITE8_MEMBER(apc_state::apc_port_60_w)
{
	if(offset & 1)
	{
		printf("Write undefined port %02x\n",offset+0x60);
	}
	else
	{
		printf("Write melody port %02x %02x\n",offset+0x60,data);
	}
}

READ8_MEMBER(apc_state::apc_gdc_r)
{
	UINT8 res;

	if(offset & 1)
		res = m_hgdc2->read(space, (offset & 2) >> 1); // upd7220 bitmap port
	else
		res = m_hgdc1->read(space, (offset & 2) >> 1); // upd7220 character port

	return res;
}

WRITE8_MEMBER(apc_state::apc_gdc_w)
{
	if(offset & 1)
		m_hgdc2->write(space, (offset & 2) >> 1,data); // upd7220 bitmap port
	else
		m_hgdc1->write(space, (offset & 2) >> 1,data); // upd7220 character port
}

READ8_MEMBER(apc_state::apc_kbd_r)
{
	UINT8 res;

	switch(offset & 3)
	{
		case 0: res = m_keyb.data; pic8259_ir4_w(machine().device("pic8259_master"), 0); break; // according to the source, reading there acks the irq
		case 1: res = m_keyb.status; break;
		case 2: res = m_keyb.sig; break; // bit 0: CTRL bit 1: function key (or reversed)
		case 3: res = machine().root_device().ioport("KEY_MOD")->read() & 0xff; break; // sh
	}

	return res;
}

WRITE8_MEMBER(apc_state::apc_kbd_w)
{
	printf("KEYB %08x %02x\n",offset,data);
}

WRITE8_MEMBER(apc_state::apc_dma_segments_w)
{
	m_dma_offset[offset & 3] = data & 0x0f;
}

/*
NEC APC i8237 hook-up looks pretty weird ...

                NEC APC (shift 1) IBM PC
CH0_ADR ==      0X01     0x00       0x00  ; CH-0 address (RW)
CH1_ADR ==      0X03     0x01       0x02  ; CH-1 address (RW)
CH2_ADR ==      0X05     0x02       0x04  ; CH-2 address (RW)
CH3_ADR ==      0X07     0x03       0x06  ; CH-3 address (RW)
DMA_ST  ==      0X09     0x04       0x08  ; status register (R)
DMA_CMD ==      0X09     0x04       0x08  ; command register (W)
DMA_WSM ==      0X0B     0x05       0x0a  ; write single mask (W)
DMA_CFF ==      0X0D     0x06       0x0c  ; clear flip flop (W)

CH0_TC  ==      0X11     0x08       0x01  ; CH-0 terminal count (RW)
CH1_TC  ==      0X13     0x09       0x03  ; CH-1 terminal count (RW)
CH2_TC  ==      0X15     0x0a       0x05  ; CH-2 terminal count (RW)
CH3_TC  ==      0X17     0x0b       0x07  ; CH-3 terminal count (RW)
DMA_WRR ==      0X19     0x0c       0x09  ; write request register (W)
DMA_MODE==      0X1B     0x0d       0x0b  ; write mode (W)
DMA_RTR ==      0X1D     0x0e       0x0d? ; read temp register (R)
DMA_MC  ==      0X1D     0x0e       0x0d  ; master clear (W)
DMA_WAM ==      0X1F     0x0f       0x0f? ; write all mask (W)
CH0_EXA ==      0X38                      ; CH-0 extended address (W)
CH1_EXA ==      0X3A                      ; CH-1 extended address (W)
CH2_EXA ==      0X3C                      ; CH-2 extended address (W)
CH3_EXA ==      0X3E                      ; CH-3 extended address (W)

... apparently, they rotated right the offset, compared to normal hook-up.
*/

READ8_MEMBER(apc_state::apc_dma_r)
{
	return machine().device<am9517a_device>("i8237")->read(space, BITSWAP8(offset,7,6,5,4,2,1,0,3), 0xff);
}

WRITE8_MEMBER(apc_state::apc_dma_w)
{
	machine().device<am9517a_device>("i8237")->write(space, BITSWAP8(offset,7,6,5,4,2,1,0,3), data, 0xff);
}

WRITE8_MEMBER(apc_state::apc_irq_ack_w)
{
	if(data & 4)
		pic8259_ir3_w(machine().device("pic8259_master"), 0);

	if(data & ~4)
		logerror("IRQ ACK %02x\n",data);
}

/* TODO: bit arrangement is completely wrong */
READ8_MEMBER(apc_state::apc_rtc_r)
{
	return m_rtc->data_out_r();
}

WRITE8_MEMBER(apc_state::apc_rtc_w)
{
	m_rtc->data_in_w(BIT(data, 0));
	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));
	m_rtc->oe_w(BIT(data, 3));
	m_rtc->c0_w(BIT(data, 3));
	m_rtc->c1_w(BIT(data, 4));
	m_rtc->c2_w(BIT(data, 5));
}

static ADDRESS_MAP_START( apc_map, AS_PROGRAM, 16, apc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAM
	AM_RANGE(0xa0000, 0xa0fff) AM_RAM AM_SHARE("cmos")
//	AM_RANGE(0xc0000, 0xcffff) standard character ROM
	AM_RANGE(0xd8000, 0xdffff) AM_RAM // AUX character RAM
//	AM_RANGE(0xe0000, 0xeffff) Special Character RAM
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( apc_io, AS_IO, 16, apc_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x1f) AM_READWRITE8(apc_dma_r, apc_dma_w,0xff00)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0x00ff) // i8259
	AM_RANGE(0x28, 0x2f) AM_READWRITE8(apc_port_28_r, apc_port_28_w, 0xffff) // i8259 (even) / pit8253 (odd)
//  0x30, 0x37 serial port 0/1 (i8251) (even/odd)
	AM_RANGE(0x38, 0x3f) AM_WRITE8(apc_dma_segments_w,0x00ff)
	AM_RANGE(0x40, 0x43) AM_READWRITE8(apc_gdc_r, apc_gdc_w, 0xffff)
	AM_RANGE(0x46, 0x47) AM_WRITE8(apc_irq_ack_w, 0x00ff)
	AM_RANGE(0x48, 0x4f) AM_READWRITE8(apc_kbd_r, apc_kbd_w, 0x00ff)
	AM_RANGE(0x50, 0x53) AM_DEVICE8("upd765", upd765a_device, map, 0x00ff ) // upd765
	AM_RANGE(0x58, 0x59) AM_READWRITE8(apc_rtc_r, apc_rtc_w, 0x00ff)
//  0x5a  APU data (Arithmetic Processing Unit!)
//  0x5e  APU status/command
	AM_RANGE(0x60, 0x67) AM_READWRITE8(apc_port_60_r, apc_port_60_w, 0xffff)
//  0x60 Melody Processing Unit
//  AM_RANGE(0x68, 0x6f) i8255 , printer port (A: status (R) B: data (W) C: command (W))
//  AM_DEVREADWRITE8("upd7220_btm", upd7220_device, read, write, 0x00ff)
//	0x92, 0x9a, 0xa2, 0xaa is for a Hard Disk (unknown type)
ADDRESS_MAP_END

/* TODO: key repeat, remove port impulse! */
INPUT_CHANGED_MEMBER(apc_state::key_stroke)
{
	if(newval && !oldval)
	{
		m_keyb.data = (UINT8)(FPTR)(param) & 0xff;
		//m_keyb.status &= ~1;
		pic8259_ir4_w(machine().device("pic8259_master"), 1);
	}

	if(oldval && !newval)
	{
		m_keyb.data = 0xff;
		//m_keyb.status |= 1;
	}
}


static INPUT_PORTS_START( apc )
	PORT_START("KEY0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x30)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x31)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x33)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x34)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x35)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x36)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x37)

	PORT_START("KEY1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('0') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x38)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('1') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x39)

	PORT_START("KEY2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x41)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x42)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x43)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x44)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x45)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x46)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x47)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x48)

	PORT_START("KEY3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x49)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4a)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4b)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4c)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4d)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4e)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4f)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x50)

	PORT_START("KEY4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x51)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x52)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x53)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x54)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x55)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x56)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x57)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x58)

	PORT_START("KEY5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x59)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5a)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ / {") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5b)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ / |") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5c)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] / }") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5d)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(up score) / ^") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5e)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- / _") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5f)
//	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("unk6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x26)

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x20)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; / :") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x3a)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("= / +") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2d)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' / ~") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x40)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' / \"") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x3b)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' / \"") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x3b)

/*
;
; Special table for screwed-up keys.  Scan codes are converted.
;
SPECTBL:
	BYTE	0X2D,0X3D
	BYTE	0X40,0X60
	BYTE	0X3A,0X3B
	BYTE	0X3B,0X27
SPECTLN	==	(.-SPECTBL)/2		; length of table
;
; Shift case table
;
CASETBL:
	BYTE	"1!"
	BYTE	"2@"
	BYTE	"3#"
	BYTE	"4$"
	BYTE	"5%"
	BYTE	"6",0XD0
	BYTE	"7&"
	BYTE	"8*"
	BYTE	"9("
	BYTE	"0)"
	BYTE	"-_"
	BYTE	"=+"
	BYTE	"`~"
	BYTE	"[{"
	BYTE	"]}"
	BYTE	"\\|"
	BYTE	",<"
	BYTE	".>"
	BYTE	"/?"
	BYTE	";:"
	BYTE	0X27,0X22
	BYTE	0X18,"^"
*/

/*
	BYTE	0X18			; 5E - Control-X
	BYTE	"-"			; 5F
*/

/*
	#REPEAT	0X96-0X80		; 80 to 95 - function keys
*/
	PORT_START("KEY_PF1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x80)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x81)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x82)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x83)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x84)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF6")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x85)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF7")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x86)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF8")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x87)

	PORT_START("KEY_PF2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF9")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x88)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF10") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x89)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF11") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x8a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF12") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x8b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF13") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x8c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF14") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x8d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF15") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x8e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF16") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x8f)

	PORT_START("KEY_PF3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF17")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x90)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF18")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x91)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF19")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x92)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF20")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x93)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF21")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x94)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF22")  PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x95)

/*
	BYTE	"*"			; 6A
	BYTE	"+"			; 6B
	BYTE	0XFF		; 6C - undefined code
	BYTE	"-"			; 6D
	BYTE	"."			; 6E
	BYTE	"/"			; 6F
	BYTE	"0"			; 70
	BYTE	"1"			; 71
	BYTE	"2"			; 72
	BYTE	"3"			; 73
	BYTE	"4"			; 74
	BYTE	"5"			; 75
	BYTE	"6"			; 76
	BYTE	"7"			; 77
	BYTE	"8"			; 78
	BYTE	"9"			; 79
*/

	PORT_START("KEY_PAD1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6a)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6b)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_UNUSED) // 0x6c
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- (PAD)") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6d)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". (PAD)") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6e)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ (PAD)") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6f)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x70)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x71)

	PORT_START("KEY_PAD2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x72)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x73)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x74)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x75)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x76)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x77)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x78)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x79)


/*	BYTE	0X00			; 96 - break stop
	BYTE	0X0D			; 97 - return
	BYTE	0X09			; 98 - tab/back tab
	BYTE	0XFF			; 99 - undefined code
	BYTE	0X1E			; 9A - home/clear
	BYTE	0XFF			; 9B - undefined code
	BYTE	0X08			; 9C - back space
*/
	PORT_START("KEY_S1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CHAR(0x00) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x96)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x97)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("TAB") /*PORT_CODE(KEYCODE_TAB)*/ PORT_CHAR(0x09) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x98)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_UNUSED) //0x99
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) PORT_CHAR(0x1e) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x9a)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_UNUSED) //0x9b
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BACK SPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x9c)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CHAR(0x1b) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x1b)

/*
	BYTE	0X0B			; F7 - up arrow
	BYTE	0X0A			; F8 - down arrow
	BYTE	0X0C			; F9 - right arrow
	BYTE	0X08			; FA - left arrow
	BYTE	0XFF (?)		; FB - ins
	BYTE	0X7F			; FC - del
	BYTE	0X0D			; FD - enter
*/
	PORT_START("KEY_S2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(0x0b) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xf7)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(0x0a) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xf8)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(0x0c) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xf9)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(0x08) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfa)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS") PORT_CHAR(0xff) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfb)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CHAR(0x7f) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfc)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER (PAD)") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(0x0d) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfd)

	PORT_START("KEY_MOD")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
INPUT_PORTS_END

void apc_state::fdc_drq(bool state)
{
//  printf("%02x DRQ\n",state);
//  i8237_dreq0_w(m_dma, state);
	m_dmac->dreq1_w(state);

}

void apc_state::fdc_irq(bool state)
{
//  printf("IRQ %d\n",state);
	pic8259_ir3_w(machine().device("pic8259_slave"), state);
}

static IRQ_CALLBACK(irq_callback)
{
	return pic8259_acknowledge( device->machine().device( "pic8259_master" ));
}

void apc_state::machine_start()
{
	machine().device("maincpu")->execute().set_irq_acknowledge_callback(irq_callback);

	m_fdc->set_rate(500000);
	m_fdc->setup_intrq_cb(upd765a_device::line_cb(FUNC(apc_state::fdc_irq), this));
	m_fdc->setup_drq_cb(upd765a_device::line_cb(FUNC(apc_state::fdc_drq), this));

	m_rtc->cs_w(1);
//	m_rtc->oe_w(1);
}

void apc_state::machine_reset()
{
	m_keyb.status = 0;
	m_keyb.data = 0;
	m_keyb.sig = 0;
}


void apc_state::palette_init()
{

}

static UPD7220_INTERFACE( hgdc_1_intf )
{
	"screen",
	NULL,
	hgdc_draw_text,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static UPD7220_INTERFACE( hgdc_2_intf )
{
	"screen",
	hgdc_display_pixels,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const gfx_layout charset_8x16 =
{
	8, 16,
	128,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*1024, 1*1024, 2*1024, 3*1024, 4*1024, 5*1024, 6*1024, 7*1024, 8*1024, 9*1024, 10*1024, 11*1024, 12*1024, 13*1024, 14*1024, 15*1024 },
	8
};

static GFXDECODE_START( apc )
	GFXDECODE_ENTRY( "gfx", 0x0000, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x0800, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x1000, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x1800, charset_8x16, 0, 128 )
GFXDECODE_END



static ADDRESS_MAP_START( upd7220_1_map, AS_0, 8, apc_state)
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_2_map, AS_0, 8, apc_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_2")
ADDRESS_MAP_END

static const struct pit8253_config pit8253_config =
{
	{
		{
			MAIN_CLOCK,              /* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir3_w)
		}, {
			MAIN_CLOCK,              /* Memory Refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			MAIN_CLOCK,              /* RS-232c */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};
/*
irq assignment:

8259 master:
ir0 all stop (enabled at POST, unknown purpose)
ir1 Communication
ir2 Option
ir3 Timer (enabled after CP/M loading, serial?)
ir4 keyboard (almost trusted, check code at fe64a)
ir5 Option
ir6 Option
ir7 (ODA Printer?)

8259 slave:
ir0 Option
ir1 Option
ir2 CRT
ir3 FDD
ir4 Option
ir5 Option
ir6 APU
ir7 Option
*/

WRITE_LINE_MEMBER(apc_state::apc_master_set_int_line)
{
	//printf("%02x\n",interrupt);
//	printf("irq %d\n",state);
	machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER(apc_state::get_slave_ack)
{
	if (offset==7) { // IRQ = 7
		return pic8259_acknowledge( machine().device( "pic8259_slave" ));
	}
	return 0x00;
}

static const struct pic8259_interface pic8259_master_config =
{
	DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_master_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_MEMBER(apc_state,get_slave_ack)
};

static const struct pic8259_interface pic8259_slave_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir7_w), //TODO: check me
	DEVCB_LINE_GND,
	DEVCB_NULL
};

/****************************************
*
* I8237 DMA interface
*
****************************************/

WRITE_LINE_MEMBER(apc_state::apc_dma_hrq_changed)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);

//  printf("%02x HLDA\n",state);
}

WRITE_LINE_MEMBER( apc_state::apc_tc_w )
{
	/* floppy terminal count */
	m_fdc->tc_w(state);

//	printf("TC %02x\n",state);
}

READ8_MEMBER(apc_state::apc_dma_read_byte)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

//	printf("%08x\n",addr);

	return program.read_byte(addr);
}


WRITE8_MEMBER(apc_state::apc_dma_write_byte)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

//  printf("%08x %02x\n",addr,data);

	program.write_byte(addr, data);
}

static void set_dma_channel(running_machine &machine, int channel, int state)
{
	apc_state *drvstate = machine.driver_data<apc_state>();
	if (!state) drvstate->m_dack = channel;
}

WRITE_LINE_MEMBER(apc_state::apc_dack0_w){ /*printf("%02x 0\n",state);*/ set_dma_channel(machine(), 0, state); }
WRITE_LINE_MEMBER(apc_state::apc_dack1_w){ /*printf("%02x 1\n",state);*/ set_dma_channel(machine(), 1, state); }
WRITE_LINE_MEMBER(apc_state::apc_dack2_w){ /*printf("%02x 2\n",state);*/ set_dma_channel(machine(), 2, state); }
WRITE_LINE_MEMBER(apc_state::apc_dack3_w){ /*printf("%02x 3\n",state);*/ set_dma_channel(machine(), 3, state); }

READ8_MEMBER(apc_state::fdc_r)
{
//  printf("2dd DACK R\n");

	return m_fdc->dma_r();
}

WRITE8_MEMBER(apc_state::fdc_w)
{
	//	printf("2dd DACK W\n");
	m_fdc->dma_w(data);
}

static I8237_INTERFACE( dmac_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_dma_hrq_changed),
	DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_tc_w),
	DEVCB_DRIVER_MEMBER(apc_state, apc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(apc_state, apc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_DRIVER_MEMBER(apc_state,fdc_r), DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_DRIVER_MEMBER(apc_state,fdc_w), DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_dack3_w) }
};

static UPD1990A_INTERFACE( apc_upd1990a_intf )
{
	DEVCB_NULL,
	DEVCB_NULL
};

static const floppy_format_type apc_floppy_formats[] = {
	FLOPPY_D88_FORMAT,
	FLOPPY_IMD_FORMAT,
	FLOPPY_MFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( apc_floppies )
	SLOT_INTERFACE( "8", FLOPPY_8_DSDD )
SLOT_INTERFACE_END

PALETTE_INIT_MEMBER(apc_state,apc)
{
	int i;

	for(i=0;i<8;i++)
		palette_set_color_rgb(machine(), i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
	for(i=8;i<machine().total_colors();i++)
		palette_set_color_rgb(machine(), i, pal1bit(0), pal1bit(0), pal1bit(0));
}

static MACHINE_CONFIG_START( apc, apc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8086,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(apc_map)
	MCFG_CPU_IO_MAP(apc_io)

	MCFG_PIT8253_ADD( "pit8253", pit8253_config )
	MCFG_PIC8259_ADD( "pic8259_master", pic8259_master_config )
	MCFG_PIC8259_ADD( "pic8259_slave", pic8259_slave_config )
	MCFG_I8237_ADD("i8237", MAIN_CLOCK, dmac_intf)

	MCFG_NVRAM_ADD_1FILL("cmos")
	MCFG_UPD1990A_ADD("upd1990a", XTAL_32_768kHz, apc_upd1990a_intf)

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", apc_floppies, "8", 0, apc_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", apc_floppies, "8", 0, apc_floppy_formats)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(apc_state, screen_update)
	MCFG_SCREEN_SIZE(640, 494)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 640-1, 0*8, 494-1)

	MCFG_GFXDECODE(apc)

	MCFG_UPD7220_ADD("upd7220_chr", 5000000/2, hgdc_1_intf, upd7220_1_map)
	MCFG_UPD7220_ADD("upd7220_btm", 5000000/2, hgdc_2_intf, upd7220_2_map)

	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT_OVERRIDE(apc_state,apc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( apc )
	ROM_REGION( 0x2000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pfbu2j.bin",   0x00000, 0x001000, CRC(86970df5) SHA1(be59c5dad3bd8afc21e9f2f1404553d4371978be) )
    ROM_LOAD16_BYTE( "pfbu2l.bin",   0x00001, 0x001000, CRC(38df2e70) SHA1(a37ccaea00c2b290610d354de08b489fa897ec48) )

//  ROM_REGION( 0x10000, "file", ROMREGION_ERASE00 )
//  ROM_LOAD( "sioapc.o", 0, 0x10000, CRC(1) SHA1(1) )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
    ROM_LOAD("pfcu1r.bin",   0x000000, 0x002000, CRC(683efa94) SHA1(43157984a1746b2e448f3236f571011af9a3aa73) )
ROM_END

DRIVER_INIT_MEMBER(apc_state,apc)
{
	// ...
}

GAME( 1982, apc,  0,   apc,  apc, apc_state,  apc,       ROT0, "NEC",      "APC", GAME_NOT_WORKING | GAME_NO_SOUND )
