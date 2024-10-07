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
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd1990a.h"
#include "machine/upd765.h"
#include "sound/upd1771.h"
#include "video/upd7220.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
//#include "sound/ay8910.h"


namespace {

#define MAIN_CLOCK XTAL(5'000'000)

class apc_state : public driver_device
{
public:
	apc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hgdc1(*this, "upd7220_chr"),
		m_hgdc2(*this, "upd7220_btm"),
		m_rtc(*this, "upd1990a"),
		m_cmos(*this, "cmos"),
		m_i8259_m(*this, "pic8259_master"),
		m_i8259_s(*this, "pic8259_slave"),
		m_fdc(*this, "upd765"),
		m_fdc_connector(*this, "upd765:%u", 0U),
		m_dmac(*this, "i8237"),
		m_pit(*this, "pit8253"),
		m_aux_pcg(*this, "aux_pcg"),
		m_speaker(*this, "mono"),
		m_sound(*this, "upd1771c"),
		m_video_ram_1(*this, "video_ram_1"),
		m_video_ram_2(*this, "video_ram_2"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void apc(machine_config &config);

	void init_apc();

	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc1;
	required_device<upd7220_device> m_hgdc2;
	required_device<upd1990a_device> m_rtc;
	required_device<nvram_device> m_cmos;
	required_device<pic8259_device> m_i8259_m;
	required_device<pic8259_device> m_i8259_s;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_fdc_connector;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	required_shared_ptr<uint16_t> m_aux_pcg;
	uint8_t *m_char_rom = nullptr;

	required_device<speaker_device> m_speaker;
	required_device<upd1771c_device> m_sound;

	required_shared_ptr<uint16_t> m_video_ram_1;
	required_shared_ptr<uint16_t> m_video_ram_2;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);


	uint8_t apc_port_28_r(offs_t offset);
	void apc_port_28_w(offs_t offset, uint8_t data);
	uint8_t apc_gdc_r(offs_t offset);
	void apc_gdc_w(offs_t offset, uint8_t data);
	uint8_t apc_kbd_r(offs_t offset);
	void apc_kbd_w(offs_t offset, uint8_t data);
	void apc_dma_segments_w(offs_t offset, uint8_t data);
	uint8_t apc_dma_r(offs_t offset);
	void apc_dma_w(offs_t offset, uint8_t data);
	void apc_irq_ack_w(uint8_t data);
	uint8_t apc_rtc_r();
	void apc_rtc_w(uint8_t data);
//  uint8_t aux_pcg_r();
//  void aux_pcg_w(uint8_t data);

	struct {
		uint8_t status = 0; //status
		uint8_t data = 0; //key data
		uint8_t sig = 0; //switch signal port
		uint8_t sh = 0; //shift switches
	}m_keyb;

	uint8_t get_slave_ack(offs_t offset);
	void apc_dma_hrq_changed(int state);
	void apc_tc_w(int state);
	void apc_dack0_w(int state);
	void apc_dack1_w(int state);
	void apc_dack2_w(int state);
	void apc_dack3_w(int state);
	uint8_t apc_dma_read_byte(offs_t offset);
	void apc_dma_write_byte(offs_t offset, uint8_t data);

	int m_dack = 0;
	uint8_t m_dma_offset[4]{};

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	void apc_io(address_map &map) ATTR_COLD;
	void apc_map(address_map &map) ATTR_COLD;
	void upd7220_1_map(address_map &map) ATTR_COLD;
	void upd7220_2_map(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
	inline void set_dma_channel(int channel, int state);
};

void apc_state::video_start()
{
	m_char_rom = memregion("gfx")->base();
}

uint32_t apc_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
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
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

//  if(m_video_ff[DISPLAY_REG] == 0) //screen is off
//      return;

//  uint8_t interlace_on = m_video_reg[2] == 0x10; /* TODO: correct? */
	uint8_t char_size = 19;

	for(int x=0;x<pitch;x++)
	{
//      uint32_t tile_addr = addr+(x*(m_video_ff[WIDTH40_REG]+1));
		uint32_t tile_addr = addr+(x*(1));

		uint8_t tile = (m_video_ram_1[((tile_addr*2) & 0x1fff) >> 1] >> 8) & 0x00ff;
		uint8_t tile_sel = m_video_ram_1[((tile_addr*2) & 0x1fff) >> 1] & 0x00ff;
		uint8_t attr = (m_video_ram_1[((tile_addr*2 & 0x1fff) | 0x2000) >> 1] & 0x00ff);

		uint8_t u_line = attr & 0x01;
		uint8_t o_line = attr & 0x02;
		uint8_t v_line = attr & 0x04;
		uint8_t blink  = attr & 0x08;
		uint8_t reverse = attr & 0x10;
//      secret= (attr & 1) ^ 1;
		uint8_t color = (attr & 0xe0) >> 5;

		for(int yi=0;yi<lr;yi++)
		{
			int yi_trans = (yi==0)?lr-1:yi-1;
			for(int xi=0;xi<8;xi++)
			{
				int res_x = (x*8+xi);
				int res_y = y+yi;

				if(!m_screen->visible_area().contains(res_x, res_y))
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
				uint8_t tile_data;
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
				if(blink && m_screen->frame_number() & 0x20) { tile_data = 0; } // TODO: rate & correct behaviour

				if(cursor_on && cursor_addr == tile_addr && m_screen->frame_number() & 0x10)
					tile_data^=0xff;

				uint8_t pen;
				if(yi >= char_size)
					pen = 0;
				else
					pen = (tile_data >> (xi) & 1) ? color : 0;

				if(pen)
					bitmap.pix(res_y, res_x) = palette[pen];
			}
		}
	}
}

uint8_t apc_state::apc_port_28_r(offs_t offset)
{
	uint8_t res;

	if(offset & 1)
		res = m_pit->read((offset & 6) >> 1);
	else
	{
		if(offset & 4)
		{
			printf("Read undefined port %02x\n",offset+0x28);
			res = 0xff;
		}
		else
			res = m_i8259_s->read((offset & 2) >> 1);
	}

	return res;
}

void apc_state::apc_port_28_w(offs_t offset, uint8_t data)
{
	if(offset & 1)
		m_pit->write((offset & 6) >> 1, data);
	else
	{
		if(offset & 4)
			printf("Write undefined port %02x\n",offset+0x28);
		else
			m_i8259_s->write((offset & 2) >> 1, data);
	}
}


uint8_t apc_state::apc_gdc_r(offs_t offset)
{
	uint8_t res;

	if(offset & 1)
		res = m_hgdc2->read((offset & 2) >> 1); // upd7220 bitmap port
	else
		res = m_hgdc1->read((offset & 2) >> 1); // upd7220 character port

	return res;
}

void apc_state::apc_gdc_w(offs_t offset, uint8_t data)
{
	if(offset & 1)
		m_hgdc2->write((offset & 2) >> 1,data); // upd7220 bitmap port
	else
		m_hgdc1->write((offset & 2) >> 1,data); // upd7220 character port
}

uint8_t apc_state::apc_kbd_r(offs_t offset)
{
	uint8_t res = 0;

	switch(offset & 3)
	{
		case 0: res = m_keyb.data; m_i8259_m->ir4_w(0); break; // according to the source, reading there acks the irq
		case 1: res = m_keyb.status; break;
		case 2: res = m_keyb.sig; break; // bit 0: CTRL bit 1: function key (or reversed)
		case 3: res = ioport("KEY_MOD")->read() & 0xff; break; // sh
	}

	return res;
}

void apc_state::apc_kbd_w(offs_t offset, uint8_t data)
{
	printf("KEYB %08x %02x\n",offset,data);
}

void apc_state::apc_dma_segments_w(offs_t offset, uint8_t data)
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

uint8_t apc_state::apc_dma_r(offs_t offset)
{
	return m_dmac->read(bitswap<4>(offset,2,1,0,3));
}

void apc_state::apc_dma_w(offs_t offset, uint8_t data)
{
	m_dmac->write(bitswap<4>(offset,2,1,0,3), data);
}

void apc_state::apc_irq_ack_w(uint8_t data)
{
	/*
	    x--- GDC
	    -x-- TM
	    --x- APU
	    ---x CRT
	*/
	if(data & 4)
		m_i8259_m->ir3_w(0);

	if(data & ~4)
		logerror("IRQ ACK %02x\n",data);
}

uint8_t apc_state::apc_rtc_r()
{
	/*
	bit 1 high: low battery.
	*/
	//fprintf(stderr, "RTC Read: %d\n", m_rtc->data_out_r());
	return m_rtc->data_out_r();
}

void apc_state::apc_rtc_w(uint8_t data)
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

void apc_state::apc_map(address_map &map)
{
	map(0x00000, 0x9ffff).ram();
	map(0xa0000, 0xa0fff).ram().share("cmos");
//  map(0xa1000, 0xbffff) mirror CMOS
//  map(0xc0000, 0xcffff) standard character ROM
	map(0xd8000, 0xd9fff).ram().share("aux_pcg"); // AUX character RAM
//  map(0xe0000, 0xeffff) Special Character RAM
	map(0xfe000, 0xfffff).rom().region("ipl", 0);
}

void apc_state::apc_io(address_map &map)
{
//  map.global_mask(0xff);
	map(0x00, 0x1f).rw(FUNC(apc_state::apc_dma_r), FUNC(apc_state::apc_dma_w)).umask16(0xff00);
	map(0x20, 0x23).rw(m_i8259_m, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff); // i8259
	map(0x28, 0x2f).rw(FUNC(apc_state::apc_port_28_r), FUNC(apc_state::apc_port_28_w)); // i8259 (even) / pit8253 (odd)
//  0x30, 0x37 serial port 0/1 (i8251) (even/odd)
	map(0x38, 0x3f).w(FUNC(apc_state::apc_dma_segments_w)).umask16(0x00ff);
	map(0x40, 0x43).rw(FUNC(apc_state::apc_gdc_r), FUNC(apc_state::apc_gdc_w));
	map(0x46, 0x46).w(FUNC(apc_state::apc_irq_ack_w));
	map(0x48, 0x4f).rw(FUNC(apc_state::apc_kbd_r), FUNC(apc_state::apc_kbd_w)).umask16(0x00ff);
	map(0x50, 0x53).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff); // upd765
	map(0x58, 0x58).rw(FUNC(apc_state::apc_rtc_r), FUNC(apc_state::apc_rtc_w));
//  0x59 CMOS enable
//  0x5a  APU data (Arithmetic Processing Unit!)
//  0x5b, Power Off
//  0x5e  APU status/command
	map(0x60, 0x60).rw(m_sound, FUNC(upd1771c_device::read), FUNC(upd1771c_device::write));
//  map(0x68, 0x6f) i8255 , ODA printer port (A: status (R) B: data (W) C: command (W))
//  map(0x70, 0x76).rw("upd7220_btm", FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask16(0x00ff);
//  0x71, 0x77 IDA Controller
//  0x80, 0x90 Communication Adapter
//  0xf0, 0xf6 ASOP Controller
}

/* TODO: key repeat, remove port impulse! */
INPUT_CHANGED_MEMBER(apc_state::key_stroke)
{
	if(newval && !oldval)
	{
		m_keyb.data = uint8_t(param & 0xff);
		//m_keyb.status &= ~1;
		m_i8259_m->ir4_w(1);
	}

	if(oldval && !newval)
	{
		m_keyb.data = 0xff;
		//m_keyb.status |= 1;
	}
}


static INPUT_PORTS_START( apc )
	PORT_START("KEY0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x30)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x31)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x33)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x34)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x35)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x36)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x37)

	PORT_START("KEY1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x38)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x39)

	PORT_START("KEY2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x41)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x42)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x43)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x44)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x45)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x46)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x47)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x48)

	PORT_START("KEY3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x49)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4a)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4b)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4c)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4d)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4e)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x4f)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x50)

	PORT_START("KEY4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x51)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x52)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x53)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x54)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x55)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x56)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x57)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x58)

	PORT_START("KEY5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x59)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5a)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ / {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5b)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ / |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5c)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] / }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5d)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(up score) / ^") PORT_CHAR('^') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5e)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- / _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x5f)
//  PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("unk6") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x26)

	PORT_START("KEY6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x20)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; / :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x3a)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("= / +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2d)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("` / ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x40)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' / \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x3b)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", / <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2c)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". / >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ / ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x2f)


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
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6a)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6b)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_UNUSED) // 0x6c
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- (PAD)") PORT_CODE(KEYCODE_MINUS_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6d)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". (PAD)") PORT_CODE(KEYCODE_DEL_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6e)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ (PAD)") PORT_CODE(KEYCODE_SLASH_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x6f)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x70)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x71)

	PORT_START("KEY_PAD2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x72)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x73)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x74)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x75)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x76)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x77)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x78)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0x79)


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
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xf7)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xf8)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xf9)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfa)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS") PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfb)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfc)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER (PAD)") PORT_CODE(KEYCODE_ENTER_PAD) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, apc_state, key_stroke, 0xfd)

	PORT_START("KEY_MOD")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
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

static GFXDECODE_START( gfx_apc )
	GFXDECODE_ENTRY( "gfx", 0x0000, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x0800, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x1000, charset_8x16, 0, 128 )
	GFXDECODE_ENTRY( "gfx", 0x1800, charset_8x16, 0, 128 )
GFXDECODE_END



void apc_state::upd7220_1_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share(m_video_ram_1);
}

void apc_state::upd7220_2_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share(m_video_ram_2);
}

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

uint8_t apc_state::get_slave_ack(offs_t offset)
{
	if (offset==7) { // IRQ = 7
		return m_i8259_s->acknowledge();
	}
	return 0x00;
}

/****************************************
*
* I8237 DMA interface
*
****************************************/

void apc_state::apc_dma_hrq_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);

//  printf("%02x HLDA\n",state);
}

void apc_state::apc_tc_w(int state)
{
	/* floppy terminal count */
	m_fdc->tc_w(state);

//  printf("TC %02x\n",state);
}

uint8_t apc_state::apc_dma_read_byte(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;

//  printf("%08x\n",addr);

	return program.read_byte(addr);
}


void apc_state::apc_dma_write_byte(offs_t offset, uint8_t data)
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

void apc_state::apc_dack0_w(int state) { /*printf("%02x 0\n",state);*/ set_dma_channel(0, state); }
void apc_state::apc_dack1_w(int state) { /*printf("%02x 1\n",state);*/ set_dma_channel(1, state); }
void apc_state::apc_dack2_w(int state) { /*printf("%02x 2\n",state);*/ set_dma_channel(2, state); }
void apc_state::apc_dack3_w(int state) { /*printf("%02x 3\n",state);*/ set_dma_channel(3, state); }

/*
CH0: CRT
CH1: FDC
CH2: ("reserved for future graphics expansion")
CH3: AUX
*/

static void apc_floppies(device_slot_interface &device)
{
	device.option_add("8", FLOPPY_8_DSDD);
}

void apc_state::apc(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &apc_state::apc_map);
	m_maincpu->set_addrmap(AS_IO, &apc_state::apc_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(MAIN_CLOCK); // heartbeat IRQ
	m_pit->out_handler<0>().set(m_i8259_m, FUNC(pic8259_device::ir3_w));
	m_pit->set_clk<1>(MAIN_CLOCK); // Memory Refresh
	m_pit->set_clk<2>(MAIN_CLOCK); // RS-232c

	PIC8259(config, m_i8259_m, 0);
	m_i8259_m->out_int_callback().set_inputline(m_maincpu, 0);
	m_i8259_m->in_sp_callback().set_constant(1);
	m_i8259_m->read_slave_ack_callback().set(FUNC(apc_state::get_slave_ack));

	PIC8259(config, m_i8259_s, 0);
	m_i8259_s->out_int_callback().set(m_i8259_m, FUNC(pic8259_device::ir7_w)); // TODO: check ir7_w
	m_i8259_s->in_sp_callback().set_constant(0);

	AM9517A(config, m_dmac, MAIN_CLOCK);
	m_dmac->out_hreq_callback().set(FUNC(apc_state::apc_dma_hrq_changed));
	m_dmac->out_eop_callback().set(FUNC(apc_state::apc_tc_w));
	m_dmac->in_memr_callback().set(FUNC(apc_state::apc_dma_read_byte));
	m_dmac->out_memw_callback().set(FUNC(apc_state::apc_dma_write_byte));
	m_dmac->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dmac->out_dack_callback<0>().set(FUNC(apc_state::apc_dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(apc_state::apc_dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(apc_state::apc_dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(apc_state::apc_dack3_w));

	NVRAM(config, m_cmos, nvram_device::DEFAULT_ALL_1);
	UPD1990A(config, m_rtc);

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set(m_i8259_s, FUNC(pic8259_device::ir4_w));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w));
	FLOPPY_CONNECTOR(config, m_fdc_connector[0], apc_floppies, "8", floppy_image_device::default_fm_floppy_formats, "8");
	FLOPPY_CONNECTOR(config, m_fdc_connector[1], apc_floppies, "8", floppy_image_device::default_fm_floppy_formats, "8");
	SOFTWARE_LIST(config, "disk_list").set_original("apc");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(apc_state::screen_update));
	m_screen->set_size(640, 494);
	m_screen->set_visarea(0*8, 640-1, 0*8, 494-1);

	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_apc);

	UPD7220(config, m_hgdc1, 3579545); // unk clock
	m_hgdc1->set_addrmap(0, &apc_state::upd7220_1_map);
	m_hgdc1->set_draw_text(FUNC(apc_state::hgdc_draw_text));

	UPD7220(config, m_hgdc2, 3579545); // unk clock
	m_hgdc2->set_addrmap(0, &apc_state::upd7220_2_map);
	m_hgdc2->set_display_pixels(FUNC(apc_state::hgdc_display_pixels));

	/* sound hardware */
	SPEAKER(config, m_speaker).front_center();
	UPD1771C(config, m_sound, MAIN_CLOCK).add_route(ALL_OUTPUTS, "mono", 1.00); //uPD1771C-006
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( apc )
	ROM_REGION16_LE( 0x2000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pfbu2j.bin",   0x00000, 0x001000, CRC(86970df5) SHA1(be59c5dad3bd8afc21e9f2f1404553d4371978be) )
	ROM_LOAD16_BYTE( "pfbu2l.bin",   0x00001, 0x001000, CRC(38df2e70) SHA1(a37ccaea00c2b290610d354de08b489fa897ec48) )

//  ROM_REGION( 0x10000, "file", ROMREGION_ERASE00 )
//  ROM_LOAD( "sioapc.bin", 0, 0x10000, NO_DUMP )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD("pfcu1r.bin",   0x000000, 0x002000, CRC(683efa94) SHA1(43157984a1746b2e448f3236f571011af9a3aa73) )
ROM_END

void apc_state::init_apc()
{
	// ...
}

} // anonymous namespace


COMP( 1982, apc, 0, 0, apc, apc, apc_state, init_apc, "NEC", "APC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
