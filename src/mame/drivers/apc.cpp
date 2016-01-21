// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    Advanced Personal Computer (c) 1982 NEC

    preliminary driver by Angelo Salese

    TODO:
    - video emulation (bitmap part)
    - NMI seems valid, dumps a x86 stack to vram?
    - What are exactly APU and MPU devices? They sounds scary ...
    - DMA hook-ups
    - serial ports
    - parallel ports
    - Extract info regarding Hard Disk functionality
    - Various unknown ports
    - What kind of external ROM actually maps at 0xa****?
    - Jumper settings (comms settings and display select)

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
#include "audio/upd1771.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "machine/upd765.h"
#include "machine/upd1990a.h"
#include "machine/nvram.h"
#include "video/upd7220.h"
#include "softlist.h"
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
		m_pit(*this, "pit8253"),
		m_video_ram_1(*this, "video_ram_1"),
		m_video_ram_2(*this, "video_ram_2"),
		m_palette(*this, "palette")
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
	required_device<pit8253_device> m_pit;
	UINT8 *m_char_rom;
	UINT8 *m_aux_pcg;

	required_shared_ptr<UINT16> m_video_ram_1;
	required_shared_ptr<UINT16> m_video_ram_2;

	required_device<palette_device> m_palette;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);


	DECLARE_READ8_MEMBER(apc_port_28_r);
	DECLARE_WRITE8_MEMBER(apc_port_28_w);
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
//  DECLARE_READ8_MEMBER(aux_pcg_r);
//  DECLARE_WRITE8_MEMBER(aux_pcg_w);

	struct {
		UINT8 status; //status
		UINT8 data; //key data
		UINT8 sig; //switch signal port
		UINT8 sh; //shift switches
	}m_keyb;
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

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

	DECLARE_DRIVER_INIT(apc);

	int m_dack;
	UINT8 m_dma_offset[4];

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	inline void set_dma_channel(int channel, int state);
};

void apc_state::video_start()
{
	m_char_rom = memregion("gfx")->base();
	m_aux_pcg = memregion("aux_pcg")->base();
}

UINT32 apc_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	/* graphics */
	m_hgdc2->screen_update(screen, bitmap, cliprect);
	m_hgdc1->screen_update(screen, bitmap, cliprect);

	return 0;
}


UPD7220_DISPLAY_PIXELS_MEMBER( apc_state::hgdc_display_pixels )
{
	// ...
}

UPD7220_DRAW_TEXT_LINE_MEMBER( apc_state::hgdc_draw_text )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int xi,yi,yi_trans;
	int x;
	UINT8 char_size;
//  UINT8 interlace_on;

//  if(m_video_ff[DISPLAY_REG] == 0) //screen is off
//      return;

//  interlace_on = m_video_reg[2] == 0x10; /* TODO: correct? */
	char_size = 19;

	for(x=0;x<pitch;x++)
	{
		UINT8 tile_data;
		UINT8 u_line, o_line, v_line, reverse, blink;
		UINT8 color;
		UINT8 tile,attr,pen;
		UINT32 tile_addr;
		UINT8 tile_sel;

//      tile_addr = addr+(x*(m_video_ff[WIDTH40_REG]+1));
		tile_addr = addr+(x*(1));

		tile = (m_video_ram_1[((tile_addr*2) & 0x1fff) >> 1] >> 8) & 0x00ff;
		tile_sel = m_video_ram_1[((tile_addr*2) & 0x1fff) >> 1] & 0x00ff;
		attr = (m_video_ram_1[((tile_addr*2 & 0x1fff) | 0x2000) >> 1] & 0x00ff);

		u_line = attr & 0x01;
		o_line = attr & 0x02;
		v_line = attr & 0x04;
		blink  = attr & 0x08;
		reverse = attr & 0x10;
//      secret= (attr & 1) ^ 1;
		color = (attr & 0xe0) >> 5;

		for(yi=0;yi<lr;yi++)
		{
			yi_trans = (yi==0)?lr-1:yi-1;
			for(xi=0;xi<8;xi++)
			{
				int res_x,res_y;

				res_x = (x*8+xi);
				res_y = y+yi;

				if(!machine().first_screen()->visible_area().contains(res_x, res_y))
					continue;

				/*
				Addr bus:   C BA98 7654 3210
				            | |||| |\\\ \\\\- character number bits 0-6
				            | |||| \--------- y' bit 0
				            | |||\----------- y' bit 1
				            | ||\------------ y' bit 2
				            | |\------------- y' bit 3
				            | \-------------- character number bit 7
				            \---------------- y' bit 4

				y to y' (assumed; this needs hardware tests since there could be one more 'blank' line between all char rows):
				y  =  0 1 2 3 ... 16 17 18
				y' = 18 0 1 2 ... 15 16 17

				Data bus: 76543210 = pixels, in left->01234567->right order
				*/
				if(tile_sel == 0x89)// Aux character RAM select TODO: correct triggering?
				{
					if(yi & 0x10)
						tile_data = 0;
					else
						tile_data = m_aux_pcg[(tile & 0xff)*0x20+yi*2];
				}
				else
					tile_data = m_char_rom[(tile & 0x7f)+((tile & 0x80)<<4)+((yi_trans & 0xf)*0x80)+((yi_trans & 0x10)<<8)];

				if(reverse) { tile_data^=0xff; }
				if(u_line && yi == lr-1) { tile_data = 0xff; }
				if(o_line && yi == 0) { tile_data = 0xff; }
				if(v_line)  { tile_data|=1; }
				if(blink && machine().first_screen()->frame_number() & 0x20) { tile_data = 0; } // TODO: rate & correct behaviour

				if(cursor_on && cursor_addr == tile_addr && machine().first_screen()->frame_number() & 0x10)
					tile_data^=0xff;

				if(yi >= char_size)
					pen = 0;
				else
					pen = (tile_data >> (xi) & 1) ? color : 0;

				if(pen)
					bitmap.pix32(res_y, res_x) = palette[pen];
			}
		}
	}
}

READ8_MEMBER(apc_state::apc_port_28_r)
{
	UINT8 res;

	if(offset & 1)
		res = m_pit->read(space, (offset & 6) >> 1);
	else
	{
		if(offset & 4)
		{
			printf("Read undefined port %02x\n",offset+0x28);
			res = 0xff;
		}
		else
			res = m_i8259_s->read(space, (offset & 2) >> 1);
	}

	return res;
}

WRITE8_MEMBER(apc_state::apc_port_28_w)
{
	if(offset & 1)
		m_pit->write(space, (offset & 6) >> 1, data);
	else
	{
		if(offset & 4)
			printf("Write undefined port %02x\n",offset+0x28);
		else
			m_i8259_s->write(space, (offset & 2) >> 1, data);
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
	UINT8 res = 0;

	switch(offset & 3)
	{
		case 0: res = m_keyb.data; machine().device<pic8259_device>("pic8259_master")->ir4_w(0); break; // according to the source, reading there acks the irq
		case 1: res = m_keyb.status; break;
		case 2: res = m_keyb.sig; break; // bit 0: CTRL bit 1: function key (or reversed)
		case 3: res = ioport("KEY_MOD")->read() & 0xff; break; // sh
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
	return m_dmac->read(space, BITSWAP8(offset,7,6,5,4,2,1,0,3), 0xff);
}

WRITE8_MEMBER(apc_state::apc_dma_w)
{
	m_dmac->write(space, BITSWAP8(offset,7,6,5,4,2,1,0,3), data, 0xff);
}

WRITE8_MEMBER(apc_state::apc_irq_ack_w)
{
	/*
	    x--- GDC
	    -x-- TM
	    --x- APU
	    ---x CRT
	*/
	if(data & 4)
		machine().device<pic8259_device>("pic8259_master")->ir3_w(0);

	if(data & ~4)
		logerror("IRQ ACK %02x\n",data);
}

READ8_MEMBER(apc_state::apc_rtc_r)
{
	/*
	bit 1 high: low battery.
	*/
	//fprintf(stderr, "RTC Read: %d\n", m_rtc->data_out_r());
	return m_rtc->data_out_r();
}

WRITE8_MEMBER(apc_state::apc_rtc_w)
{
/*
RTC write: 0x01 0001
RTC write: 0x03 0011
RTC write: 0x0b 1011 <- cmd: read clock to shifter latch
RTC write: 0x03 0011
RTC write: 0x01 0001
RTC write: 0x09 1001 <- cmd: begin shifting latch data out
RTC write: 0x01 0001
RTC write: 0x11
RTC write: 0x01
RTC write: 0x11
RTC write: 0x01
RTC write: 0x11
...

RTC write bits: 76543210
                |||||||\- c0
                ||||||\-- c1
                |||||\--- c2
                ||||\---- STB
                |||\----- CLK
                ||\------ DATA_IN
                |\------- "don't care"
                \-------- ///
*/
	if (data&0xc0) fprintf(stderr,"RTC write: 0x%02x\n", data);
	m_rtc->c0_w(BIT(data, 0)); // correct assuming theres a delay for changing command lines before stb
	m_rtc->c1_w(BIT(data, 1)); // "
	m_rtc->c2_w(BIT(data, 2)); // "
	m_rtc->stb_w(BIT(data, 3)); // seems correct assuming delay for changing command line
	m_rtc->clk_w(BIT(data, 4)); // correct for sure
	m_rtc->data_in_w(BIT(data, 5)); // ? no idea about this.
	m_rtc->oe_w(1);
}

static ADDRESS_MAP_START( apc_map, AS_PROGRAM, 16, apc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAM
	AM_RANGE(0xa0000, 0xa0fff) AM_RAM AM_SHARE("cmos")
//  AM_RANGE(0xa1000, 0xbffff) mirror CMOS
//  AM_RANGE(0xc0000, 0xcffff) standard character ROM
	AM_RANGE(0xd8000, 0xd9fff) AM_RAM AM_REGION("aux_pcg", 0) // AUX character RAM
//  AM_RANGE(0xe0000, 0xeffff) Special Character RAM
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( apc_io, AS_IO, 16, apc_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x1f) AM_READWRITE8(apc_dma_r, apc_dma_w,0xff00)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE8("pic8259_master", pic8259_device, read, write, 0x00ff) // i8259
	AM_RANGE(0x28, 0x2f) AM_READWRITE8(apc_port_28_r, apc_port_28_w, 0xffff) // i8259 (even) / pit8253 (odd)
//  0x30, 0x37 serial port 0/1 (i8251) (even/odd)
	AM_RANGE(0x38, 0x3f) AM_WRITE8(apc_dma_segments_w,0x00ff)
	AM_RANGE(0x40, 0x43) AM_READWRITE8(apc_gdc_r, apc_gdc_w, 0xffff)
	AM_RANGE(0x46, 0x47) AM_WRITE8(apc_irq_ack_w, 0x00ff)
	AM_RANGE(0x48, 0x4f) AM_READWRITE8(apc_kbd_r, apc_kbd_w, 0x00ff)
	AM_RANGE(0x50, 0x53) AM_DEVICE8("upd765", upd765a_device, map, 0x00ff ) // upd765
	AM_RANGE(0x58, 0x59) AM_READWRITE8(apc_rtc_r, apc_rtc_w, 0x00ff)
//  0x59 CMOS enable
//  0x5a  APU data (Arithmetic Processing Unit!)
//  0x5b, Power Off
//  0x5e  APU status/command
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE8("upd1771c", upd1771c_device, read, write, 0x00ff)
//  AM_RANGE(0x68, 0x6f) i8255 , ODA printer port (A: status (R) B: data (W) C: command (W))
//  0x70, 0x76 AM_DEVREADWRITE8("upd7220_btm", upd7220_device, read, write, 0x00ff)
//  0x71, 0x77 IDA Controller
//  0x80, 0x90 Communication Adapter
//  0xf0, 0xf6 ASOP Controller
ADDRESS_MAP_END

/* TODO: key repeat, remove port impulse! */
INPUT_CHANGED_MEMBER(apc_state::key_stroke)
{
	if(newval && !oldval)
	{
		m_keyb.data = (UINT8)(FPTR)(param) & 0xff;
		//m_keyb.status &= ~1;
		machine().device<pic8259_device>("pic8259_master")->ir4_w(1);
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
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x38)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x39)

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
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ / {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5b)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ / |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5c)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] / }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5d)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(up score) / ^") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5e)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- / _") PORT_CODE(KEYCODE_MINUS) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5f)
//  PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("unk6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x26)

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x20)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; / :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x3a)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("= / +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2d)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("` / ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x40)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' / \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x3b)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", / <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2c)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". / >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ / ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2f)


/*
;
; Special table for screwed-up keys.  Scan codes are converted.
;
SPECTBL:
    BYTE    0X2D,0X3D
    BYTE    0X40,0X60
    BYTE    0X3A,0X3B
    BYTE    0X3B,0X27
SPECTLN ==  (.-SPECTBL)/2       ; length of table
;
; Shift case table
;
CASETBL:
    BYTE    "1!"
    BYTE    "2@"
    BYTE    "3#"
    BYTE    "4$"
    BYTE    "5%"
    BYTE    "6",0XD0
    BYTE    "7&"
    BYTE    "8*"
    BYTE    "9("
    BYTE    "0)"
    BYTE    "-_"
    BYTE    "=+"
    BYTE    "`~"
    BYTE    "[{"
    BYTE    "]}"
    BYTE    "\\|"
    BYTE    ",<"
    BYTE    ".>"
    BYTE    "/?"
    BYTE    ";:"
    BYTE    0X27,0X22
    BYTE    0X18,"^"
*/

/*
    BYTE    0X18            ; 5E - Control-X
    BYTE    "-"         ; 5F
*/

/*
    #REPEAT 0X96-0X80       ; 80 to 95 - function keys
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
    BYTE    "*"         ; 6A
    BYTE    "+"         ; 6B
    BYTE    0XFF        ; 6C - undefined code
    BYTE    "-"         ; 6D
    BYTE    "."         ; 6E
    BYTE    "/"         ; 6F
    BYTE    "0"         ; 70
    BYTE    "1"         ; 71
    BYTE    "2"         ; 72
    BYTE    "3"         ; 73
    BYTE    "4"         ; 74
    BYTE    "5"         ; 75
    BYTE    "6"         ; 76
    BYTE    "7"         ; 77
    BYTE    "8"         ; 78
    BYTE    "9"         ; 79
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


/*  BYTE    0X00            ; 96 - break stop
    BYTE    0X0D            ; 97 - return
    BYTE    0X09            ; 98 - tab/back tab
    BYTE    0XFF            ; 99 - undefined code
    BYTE    0X1E            ; 9A - home/clear
    BYTE    0XFF            ; 9B - undefined code
    BYTE    0X08            ; 9C - back space
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
    BYTE    0X0B            ; F7 - up arrow
    BYTE    0X0A            ; F8 - down arrow
    BYTE    0X0C            ; F9 - right arrow
    BYTE    0X08            ; FA - left arrow
    BYTE    0XFF (?)        ; FB - ins
    BYTE    0X7F            ; FC - del
    BYTE    0X0D            ; FD - enter
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
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
INPUT_PORTS_END

void apc_state::machine_start()
{
	m_fdc->set_rate(500000);

	m_rtc->cs_w(1);
//  m_rtc->oe_w(1);
}

void apc_state::machine_reset()
{
	m_keyb.status = 0;
	m_keyb.data = 0;
	m_keyb.sig = 0;
}

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

#if 0
static const gfx_layout charset_pcg =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ STEP16(0,16) },
	8*32
};
#endif

static GFXDECODE_START( apc )
	GFXDECODE_ENTRY( "gfx", 0x0000, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x0800, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x1000, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x1800, charset_8x16, 0, 128 )
//  GFXDECODE_ENTRY( "aux_pcg", 0x0000, charset_pcg, 0, 128 )
GFXDECODE_END



static ADDRESS_MAP_START( upd7220_1_map, AS_0, 16, apc_state)
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_2_map, AS_0, 16, apc_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_2")
ADDRESS_MAP_END

/*
irq assignment:
(note: documentation shows ODA Printer at ir7 master, but clearly everything is shifted one place due of the
 master-slave irq comms. This is trusted also because MS-DOS effectively wants FDC irq at ir4 slave)

8259 master:
ir0 all stop (enabled at POST, unknown purpose)
ir1 Communication
ir2 Option
ir3 Timer
ir4 keyboard (almost trusted, check code at fe64a)
ir5 Option
ir6 Option
ir7 slave irq

8259 slave:
ir0 ODA Printer
ir1 Option
ir2 Option
ir3 CRT
ir4 FDD
ir5 Option
ir6 Option
ir7 APU
*/

READ8_MEMBER(apc_state::get_slave_ack)
{
	if (offset==7) { // IRQ = 7
		return machine().device<pic8259_device>( "pic8259_slave" )->acknowledge();
	}
	return 0x00;
}

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

//  printf("TC %02x\n",state);
}

READ8_MEMBER(apc_state::apc_dma_read_byte)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

//  printf("%08x\n",addr);

	return program.read_byte(addr);
}


WRITE8_MEMBER(apc_state::apc_dma_write_byte)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

//  printf("%08x %02x\n",addr,data);

	program.write_byte(addr, data);
}

inline void apc_state::set_dma_channel(int channel, int state)
{
	if (!state) m_dack = channel;
}

WRITE_LINE_MEMBER(apc_state::apc_dack0_w){ /*printf("%02x 0\n",state);*/ set_dma_channel(0, state); }
WRITE_LINE_MEMBER(apc_state::apc_dack1_w){ /*printf("%02x 1\n",state);*/ set_dma_channel(1, state); }
WRITE_LINE_MEMBER(apc_state::apc_dack2_w){ /*printf("%02x 2\n",state);*/ set_dma_channel(2, state); }
WRITE_LINE_MEMBER(apc_state::apc_dack3_w){ /*printf("%02x 3\n",state);*/ set_dma_channel(3, state); }

READ8_MEMBER(apc_state::fdc_r)
{
	return m_fdc->dma_r();
}

WRITE8_MEMBER(apc_state::fdc_w)
{
	m_fdc->dma_w(data);
}

/*
CH0: CRT
CH1: FDC
CH2: ("reserved for future graphics expansion")
CH3: AUX
*/

static const floppy_format_type apc_floppy_formats[] = {
	FLOPPY_D88_FORMAT,
	FLOPPY_IMD_FORMAT,
	FLOPPY_MFI_FORMAT,
	nullptr
};

static SLOT_INTERFACE_START( apc_floppies )
	SLOT_INTERFACE( "8", FLOPPY_8_DSDD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( apc, apc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8086,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(apc_map)
	MCFG_CPU_IO_MAP(apc_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(MAIN_CLOCK) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259_master", pic8259_device, ir3_w))
	MCFG_PIT8253_CLK1(MAIN_CLOCK) /* Memory Refresh */
	MCFG_PIT8253_CLK2(MAIN_CLOCK) /* RS-232c */

	MCFG_PIC8259_ADD( "pic8259_master", INPUTLINE("maincpu", 0), VCC, READ8(apc_state,get_slave_ack) )
	MCFG_PIC8259_ADD( "pic8259_slave", DEVWRITELINE("pic8259_master", pic8259_device, ir7_w), GND, NULL ) // TODO: check ir7_w
	MCFG_DEVICE_ADD("i8237", AM9517A, MAIN_CLOCK)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(apc_state, apc_dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(apc_state, apc_tc_w))
	MCFG_I8237_IN_MEMR_CB(READ8(apc_state, apc_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(apc_state, apc_dma_write_byte))
	MCFG_I8237_IN_IOR_1_CB(READ8(apc_state, fdc_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(apc_state, fdc_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(apc_state, apc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(apc_state, apc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(apc_state, apc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(apc_state, apc_dack3_w))

	MCFG_NVRAM_ADD_1FILL("cmos")
	MCFG_UPD1990A_ADD("upd1990a", XTAL_32_768kHz, NULL, NULL)

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("pic8259_slave", pic8259_device, ir4_w))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("i8237", am9517a_device, dreq1_w))
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", apc_floppies, "8", apc_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", apc_floppies, "8", apc_floppy_formats)
	MCFG_SOFTWARE_LIST_ADD("disk_list","apc")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(apc_state, screen_update)
	MCFG_SCREEN_SIZE(640, 494)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 640-1, 0*8, 494-1)

	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", apc)

	MCFG_DEVICE_ADD("upd7220_chr", UPD7220, 3579545) // unk clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_1_map)
	MCFG_UPD7220_DRAW_TEXT_CALLBACK_OWNER(apc_state, hgdc_draw_text)

	MCFG_DEVICE_ADD("upd7220_btm", UPD7220, 3579545) // unk clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_2_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(apc_state, hgdc_display_pixels)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD( "upd1771c", UPD1771C, MAIN_CLOCK ) //uPD1771C-006
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )
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

	ROM_REGION( 0x2000, "aux_pcg", ROMREGION_ERASE00 )
ROM_END

DRIVER_INIT_MEMBER(apc_state,apc)
{
	// ...
}

COMP( 1982, apc,  0,   0, apc,  apc, apc_state,  apc,      "NEC",      "APC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
