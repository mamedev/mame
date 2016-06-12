// license:GPL-2.0+
// copyright-holders:Peter Trauner
/* TimeTop - GameKing */
/*
  PeT mess@utanet.at 2015, 2016


  Thanks to Deathadder, Judge, Porchy, Klaus Sommer, James Brolly & Brian Provinciano
  hopefully my work (reverse engineerung, cartridge+bios backup, emulation) will be honored in future
  and my name will not be removed entirely, especially by simple code rewrites of working emulation

  todo:
  search for rockwell r65c02 variant (cb:wai instruction) and several more exceptions
    (with luck microcontroller peripherals match those in gameking)
  incomplete interrupt controller causes hangs
  protection!? prevents 4in1 carts from working
  (improve emulation, timer)
  (add audio)
  work out gameking3 problems (lcd position, additional interrupts/lcd emulation; audio expected to be the same)

  use gameking3 cartridge to get illegal cartridge scroller

  This system appears to be based on the GeneralPlus GPL133 system-on-chip or a close relative.
  Datasheet: http://www.generalplus.com/doc/ds/GPL133AV10_spec.pdf
*/

#include <stddef.h>
#include "emu.h"
#include "cpu/m6502/r65c02gk.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

#include "gameking.lh"
//#include "gking3.lh"

//#define GK3_PIXEL // color pixel are too large, color part red green blue visible, even lines: red left, green top right blue bottom right; odd lines: red right, green left top, blue bottom left

class gameking_state : public driver_device
{
public:
	gameking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_io_joy(*this, "JOY"),
		m_palette(*this, "palette")
		{ }

	DECLARE_DRIVER_INIT(gameking);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(gameking);
	DECLARE_PALETTE_INIT(gameking3);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_READ8_MEMBER(io2_r);
	DECLARE_WRITE8_MEMBER(io2_w);
	DECLARE_WRITE8_MEMBER(gk3_lcd_w);
	INTERRUPT_GEN_MEMBER(gameking_frame_int);
	TIMER_CALLBACK_MEMBER(gameking_timer);
	TIMER_CALLBACK_MEMBER(gameking_timer2);
	TIMER_CALLBACK_MEMBER(gameking_timeruser);
	TIMER_CALLBACK_MEMBER(gameking_timeruser2);

	UINT32 screen_update_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gameking3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(gameking_cart);

	struct Gkio {
		UINT8 input, input2;
		UINT8 timer; // bit 5 red power led
		UINT8 res3[0x1d];
		UINT8 timeruser_on;
		UINT8 timeruser_quit;
		UINT8 res4[0x6];
		UINT8 dma_src_low, dma_src_high, dma_dest_low, dma_dest_high, dma_length_low, dma_length_high;
		UINT8 res5[2+1];
		UINT8 bank4000_irq; //?
		UINT8 bank4000_address; // 32
		UINT8 bank4000_cart; //33 bit 0 only?
		UINT8 bank8000_cart; //34 bit 7; bits 0,1,.. a15,a16,..
		UINT8 bank8000_control;
		UINT8 bank8000_dma_addr;
		UINT8 bank8000_dma_control;
		UINT8 res1[6];
		UINT8 interrupt_enable; // bit 2 irq timer, bit 5 timeruser
		UINT8 res2[1];
		UINT8 lcd_pos_low, lcd_pos_high;
		UINT8 res2a[0xe +0x30];
	};
protected:
	void gk3_pixel(bitmap_ind16 &bitmap, int y, int x, int color);

	UINT8 Bank8000AddrRead(UINT8 bank, UINT8 control, UINT16 addr) {
		if (addr<0x8000)
		m_maincpu->space(AS_PROGRAM).read_byte(addr);

		memory_region *maincpu_rom = memregion("maincpu");
		if (control==1)
			return maincpu_rom->base()[addr];
		return 0;
	}

	void Bank8000AddrWrite(UINT8 bank, UINT8 control, UINT16 addr, UINT8 data) {
		memory_region *maincpu_rom = memregion("maincpu");
		if (addr<0x8000)
			m_maincpu->space(AS_PROGRAM).write_byte(addr, data);
		else
			if (control==1)
				maincpu_rom->base()[addr]=data;
	}
	required_device<r65c02gk_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_ioport m_io_joy;
	required_device<palette_device> m_palette;
	UINT8 *bank4000_normal, *bank4000_irq;

	memory_region *m_cart_rom;
	memory_bank *m_bank4000;
	memory_bank *m_bank8000;
	emu_timer *timer1;
	emu_timer *timer2;
	emu_timer *timeruser;
	emu_timer *timeruser2;
	bool timeruser_expired, timer_expired;
	UINT8 inputs0, inputs1;
};

WRITE8_MEMBER( gameking_state::io_w )
{
	if (offset!=offsetof(Gkio, bank8000_cart))
		logerror("%.6f io w %x %x\n",machine().time().as_double(), offset, data);
	memory_region *maincpu_rom = memregion("maincpu");

	maincpu_rom->base()[offset]=data;
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	switch (offset) {
		case offsetof(Gkio, dma_length_low):
			{
				UINT16 src=io->dma_src_low|(io->dma_src_high<<8);
				UINT16 dst=io->dma_dest_low|(io->dma_dest_high<<8);
				UINT16 len=io->dma_length_low|(io->dma_length_high<<8);
				logerror("%.6f dma src:%x dst:%x len:%x\n",machine().time().as_double(), src, dst, len);
				for (; len>0; len--, src++, dst++) { // in reality consumes times, on 6502 probably in the 2nd part of the cpu cycle
			Bank8000AddrWrite(io->bank8000_dma_addr, io->bank8000_dma_control, dst, Bank8000AddrRead(io->bank8000_dma_addr, io->bank8000_dma_control, src));
				}
			}
			break;
		case offsetof(Gkio, timer):
			m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMER, CLEAR_LINE); // most likely wrong, removed until real interrupt start lokalized (or permanent active
			timer1->enable(TRUE);
			timer1->reset(m_maincpu->cycles_to_attotime(70000/*?*/)); // something like 64 hz
			break;
		case offsetof(Gkio, timeruser_on):
		case offsetof(Gkio, timeruser_quit):
		//    m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMERUSER, CLEAR_LINE);
		//    timeruser_expired=false;
			timeruser->enable(io->timeruser_on!=0);
			if (io->timeruser_on&0x1f)
				timeruser->reset(m_maincpu->cycles_to_attotime(2000000/(io->timeruser_on&0x1f)/*?*/)); // 1 somethink like 1.5 hz
			break;
		case offsetof(Gkio, bank4000_irq):
			{
				UINT8 bank=io->bank4000_irq^1;
				bool cart=false; //m_cart_rom;
				if (cart) {
		//      m_bank4000->set_base(m_cart_rom->base() + (bank*0x4000&(m_cart_rom->get_size()-1) );
				bank4000_irq=m_cart_rom->base() + (bank*0x4000&(0x80000-1));
				} else {
				if (bank*0x4000>=0x80000)
					bank4000_irq=maincpu_rom->base()+0x4000;
				else
					bank4000_irq=maincpu_rom->base()+0x10000 + (bank*0x4000&(0x80000-1));
				}
			}
			m_maincpu->set_irq_bank(m_bank4000, bank4000_normal, bank4000_irq);
			break;
		case offsetof(Gkio, bank4000_address): case offsetof(Gkio, bank4000_cart):
			{
				UINT8 bank=io->bank4000_address^1;
				bool cart=io->bank4000_cart&1/*?*/ && m_cart_rom;
				if (cart) {
		//      m_bank4000->set_base(m_cart_rom->base() + (bank*0x4000&(m_cart_rom->get_size()-1) );
				bank4000_normal=m_cart_rom->base() + (bank*0x4000&(0x80000-1));
				} else {
				if (bank*0x4000>=0x80000)
					bank4000_normal=maincpu_rom->base()+0x4000;
				else
					bank4000_normal=maincpu_rom->base()+0x10000 + (bank*0x4000&(0x80000-1));
				}
			}
			m_bank4000->set_base(bank4000_normal);
			m_maincpu->set_irq_bank(m_bank4000, bank4000_normal, bank4000_irq);
			break;
		case offsetof(Gkio, bank8000_cart):
		case offsetof(Gkio, bank8000_control):

			{
				bool cart=io->bank8000_cart&0x80/*?*/ && m_cart_rom;
				UINT8 bank=io->bank8000_cart&0x7f;
				if ((io->bank8000_control&7)!=0) {
					m_bank8000->set_base(maincpu_rom->base()+0x8000);
				} else if (cart) {
		//      m_bank8000->set_base(m_cart_rom->base() + (bank*0x8000&(m_cart_rom->get_size()-1) );
				m_bank8000->set_base(m_cart_rom->base() + (bank*0x8000&(0x80000-1)) );
				} else {
				if (bank*0x8000>=0x80000)
					m_bank8000->set_base(maincpu_rom->base()+0x8000);
				else
					m_bank8000->set_base(maincpu_rom->base()+0x10000+ (bank*0x8000&(0x80000-1)) );
				}
			}
			break;
		case offsetof(Gkio, interrupt_enable):
			m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMER, timer_expired && (io->interrupt_enable&4)?ASSERT_LINE:CLEAR_LINE);
			m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMERUSER, timeruser_expired && (io->interrupt_enable&0x20)?ASSERT_LINE:CLEAR_LINE);
			break;
	};
}

READ8_MEMBER( gameking_state::io_r )
{
	memory_region *maincpu_rom = memregion("maincpu");
	UINT8 data=maincpu_rom->base()[offset];
	switch (offset) {
		case offsetof(Gkio, input):
			m_maincpu->set_input_line(r65c02gk_device::IRQ_INPUTS, CLEAR_LINE);
			data=m_io_joy->read()>>8;
			break;
		case offsetof(Gkio, input2):
			m_maincpu->set_input_line(r65c02gk_device::IRQ_USER, CLEAR_LINE);
			data=m_io_joy->read();
			break;
		case 0x4c: 
			data=6; break; // bios protection endless loop
	}

	if (offset!=offsetof(Gkio, bank8000_cart) && offset!=offsetof(Gkio, input) )
		logerror("%.6f io r %x %x\n",machine().time().as_double(), offset, data);
	return data;
}

WRITE8_MEMBER( gameking_state::io2_w )
{
	if (offset>=4 && offset<=7)
		logerror("%.6f protection w %x %x\n",machine().time().as_double(), offset, data);
	else
		logerror("%.6f io2 w %x %x\n",machine().time().as_double(), offset, data);
	memory_region *maincpu_rom = memregion("maincpu");

	maincpu_rom->base()[0x100+offset]=data;
}

READ8_MEMBER( gameking_state::io2_r )
{
	memory_region *maincpu_rom = memregion("maincpu");
	UINT8 data=maincpu_rom->base()[0x100+offset];

	if (offset>=4 && offset<=7) {
		UINT8 d=data;
		if (offset==4) {
//  if (data==0xc) data=0x95;
//  if (data==0xce) data=0xd2; // no real improvement, but much slower
		}
		logerror("%.6f protection r %x %x (written %x)\n",machine().time().as_double(), offset, data, d);
	}
	else
		logerror("%.6f io2 r %x %x\n",machine().time().as_double(), offset, data);
	return data;
}

WRITE8_MEMBER( gameking_state::gk3_lcd_w )
{
	memory_region *maincpu_rom = memregion("maincpu");
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	if (io->bank8000_cart==0 && io->bank8000_control==1)
		maincpu_rom->base()[offset+0x8000]=data;
	else
		logerror("%.6f gk3 lcd w %x %x\n",machine().time().as_double(), offset, data);
}

static ADDRESS_MAP_START( gameking_mem , AS_PROGRAM, 8, gameking_state )
	AM_RANGE(0x0000, 0x007f) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x017f) AM_READWRITE(io2_r, io2_w)
	AM_RANGE(0x0180, 0x27ff) AM_RAM
	AM_RANGE(0x2800, 0x2fff) AM_ROMBANK("bank2800") // proove if also write access like on 3000
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK("bank3000")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank4000")
	AM_RANGE(0x8000, 0xffff) AM_READ_BANK("bank8000") AM_WRITE(gk3_lcd_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( gameking )
#if 1
	PORT_START("JOY")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start") // gk3 1 on address 0
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select") // ! 2 on address 1; gk3 2 on address 0
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0xfd01, IP_ACTIVE_LOW, IPT_UNUSED )
#else
	PORT_START("JOY")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start") // gk3 1 on address 0
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select") // ! 2 on address 1; gk3 2 on address 0
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0xfc03, IP_ACTIVE_LOW, IPT_UNUSED )
#endif
INPUT_PORTS_END

static INPUT_PORTS_START( gameking3 )
	PORT_START("JOY")
	PORT_BIT( 0xfc03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("B")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
INPUT_PORTS_END

static const unsigned char gameking_palette[] =
{
	255, 255, 255,
	127, 127, 127,
	63, 63, 63,
	0, 0, 0
};

static const unsigned char gameking3_palette[64*3] =
{
	255, 255, 255,
	127, 127, 127,
	63, 63, 63,
	0, 0, 0,
	255, 0, 0,
	127, 0, 0,
	63, 0, 0,
	31, 0, 0,
	0, 255, 0,
	0, 127, 127,
	0, 63, 0,
	0, 31, 0,
	0, 0, 255,
	0, 0, 127,
	0, 0, 63,
	0, 0, 31
};

PALETTE_INIT_MEMBER(gameking_state, gameking)
{
	for (int i = 0; i < sizeof(gameking_palette) / 3; i++)
		palette.set_pen_color(i, gameking_palette[i*3], gameking_palette[i*3+1], gameking_palette[i*3+2]);
}

PALETTE_INIT_MEMBER(gameking_state, gameking3)
{
	for (int i = 0; i < sizeof(gameking3_palette) / 3; i++)
		palette.set_pen_color(i, ((i&0xc)>>2)*(255/3), (i&3)*(255/3), ((i&0x30)>>4)*(255/3) ); // not quite as contrast rich on the real handheld, but not bad
}


UINT32 gameking_state::screen_update_gameking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	memory_region *maincpu_rom = memregion("maincpu");
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	UINT16 pos=io->lcd_pos_low|(io->lcd_pos_high<<8);

	for (int y=31, i=0;i<32;i++,y--)
	{
		for (int x=0, j=0;j<48/4;x+=4, j++)
		{
			UINT8 data=machine().device("maincpu")->memory().space(AS_PROGRAM).read_byte(pos+j+i*12);
			bitmap.pix16(y, x+3)=data&3;
			bitmap.pix16(y, x+2)=(data>>2)&3;
			bitmap.pix16(y, x+1)=(data>>4)&3;
			bitmap.pix16(y, x)=(data>>6)&3;
		}
	}
	return 0;
}

void gameking_state::gk3_pixel(bitmap_ind16 &bitmap, int y, int x, int color)
{
	if (!(y&1)) {
	bitmap.pix16(y*2+1, x*2)=(color&0xc)==color? color: 0; // red
	bitmap.pix16(y*2, x*2+1)=color&3? color: 0; // 0x3 green
	bitmap.pix16(y*2+1, x*2+1)=color&0x30? color: 0; // 0x30 blue
	} else {
	bitmap.pix16(y*2+1, x*2+1)=(color&0xc)==color? color: 0;
	bitmap.pix16(y*2, x*2)=color&3? color: 0;
	bitmap.pix16(y*2+1, x*2)=color&0x30? color: 0;
	}
}

UINT32 gameking_state::screen_update_gameking3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	memory_region *maincpu_rom = memregion("maincpu");
	for (int y=0, i=0;y<56;i+=40*6,y+=4)
	{
		for (int j=0, x=0;x<80;j++, x+=2)
		{
#if 1
			UINT8 d1=maincpu_rom->base()[0x8000+i+j];
			UINT8 d2=maincpu_rom->base()[0x8000+i+j+40];
			UINT8 d3=maincpu_rom->base()[0x8000+i+j+80];
			UINT8 d4=maincpu_rom->base()[0x8000+i+j+120];
			UINT8 d5=maincpu_rom->base()[0x8000+i+j+160];
			UINT8 d6=maincpu_rom->base()[0x8000+i+j+200];
#else // galaxy crisis stopped working
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	UINT16 pos=io->lcd_pos_low|(io->lcd_pos_high<<8);
			UINT8 d1=m_maincpu->space(AS_PROGRAM).read_byte(pos+i+j);
			UINT8 d2=m_maincpu->space(AS_PROGRAM).read_byte(pos+i+j+40);
			UINT8 d3=m_maincpu->space(AS_PROGRAM).read_byte(pos+i+j+80);
			UINT8 d4=m_maincpu->space(AS_PROGRAM).read_byte(pos+i+j+120);
			UINT8 d5=m_maincpu->space(AS_PROGRAM).read_byte(pos+i+j+160);
			UINT8 d6=m_maincpu->space(AS_PROGRAM).read_byte(pos+i+j+200);
#endif
			UINT32 data=((d1&0xf0)>>4)|(d2&0xf0)|((d3&0xf0)<<4)|((d4&0xf0)<<8)|((d5&0xf0)<<12)|((d6&0xf0)<<16);
#ifdef GK3_PIXEL
			gk3_pixel(bitmap, y+0, x, data&0x3f);
			gk3_pixel(bitmap, y+1, x, (data>>6)&0x3f);
			gk3_pixel(bitmap, y+2, x, (data>>12)&0x3f);
			gk3_pixel(bitmap, y+3, x, (data>>18)&0x3f);
#else
			bitmap.pix16(y+0, x)=data&0x3f;
			bitmap.pix16(y+1, x)=(data>>6)&0x3f;
			bitmap.pix16(y+2, x)=(data>>12)&0x3f;
			bitmap.pix16(y+3, x)=(data>>18)&0x3f;
#endif
			data=(d1&0xf)|((d2&0xf)<<4)|((d3&0xf)<<8)|((d4&0xf)<<12)|((d5&0xf)<<16)|((d6&0xf)<<20);
#ifdef GK3_PIXEL
			gk3_pixel(bitmap, y+0, x+1, data&0x3f);
			gk3_pixel(bitmap, y+1, x+1, (data>>6)&0x3f);
			gk3_pixel(bitmap, y+2, x+1, (data>>12)&0x3f);
			gk3_pixel(bitmap, y+3, x+1, (data>>18)&0x3f);
#else
			bitmap.pix16(y+0, x+1)=data&0x3f;
			bitmap.pix16(y+1, x+1)=(data>>6)&0x3f;
			bitmap.pix16(y+2, x+1)=(data>>12)&0x3f;
			bitmap.pix16(y+3, x+1)=(data>>18)&0x3f;
#endif
		}
	}
	return 0;
}

DRIVER_INIT_MEMBER(gameking_state, gameking)
{
	timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gameking_state::gameking_timer),this));
	timer2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gameking_state::gameking_timer2),this));
	timeruser = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gameking_state::gameking_timeruser),this));
	timeruser2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gameking_state::gameking_timeruser2),this));
}

TIMER_CALLBACK_MEMBER(gameking_state::gameking_timer)
{
//  m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE); // in reality int for vector at fff4
	memory_region *maincpu_rom = memregion("maincpu");
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	timer_expired=true;
	m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMER, io->interrupt_enable&4?ASSERT_LINE:CLEAR_LINE);
	timer1->enable(FALSE);
	timer2->enable(TRUE);
	timer2->reset(m_maincpu->cycles_to_attotime(10/*?*/));
}

TIMER_CALLBACK_MEMBER(gameking_state::gameking_timer2)
{
//  memory_region *maincpu_rom = memregion("maincpu");
//  m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE); // in reality int for vector at fff4
	m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMER, CLEAR_LINE);
	timer_expired=false;
	timer2->enable(FALSE);
	timer1->enable(TRUE);
	//Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	timer1->reset(m_maincpu->cycles_to_attotime(50000/*?*/));
}

TIMER_CALLBACK_MEMBER(gameking_state::gameking_timeruser)
{
	memory_region *maincpu_rom = memregion("maincpu");
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	timeruser_expired=true;
	m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMERUSER, io->interrupt_enable&0x20?ASSERT_LINE:CLEAR_LINE);
	timeruser->enable(FALSE);
	timeruser2->enable(TRUE);
	timeruser2->reset(m_maincpu->cycles_to_attotime(30/*?*/)); // 10 to few
}

TIMER_CALLBACK_MEMBER(gameking_state::gameking_timeruser2)
{
	memory_region *maincpu_rom = memregion("maincpu");
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	timeruser_expired=false;
	m_maincpu->set_input_line(r65c02gk_device::IRQ_TIMERUSER, CLEAR_LINE);
	timeruser->enable(io->timeruser_on!=0);
	if (io->timeruser_on&0x1f)
	timeruser->reset(m_maincpu->cycles_to_attotime(2000000/(io->timeruser_on&0x1f)/*?*/)); // 1 somethink like 1.5 hz
	timeruser->enable(FALSE);
}

DEVICE_IMAGE_LOAD_MEMBER( gameking_state, gameking_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size > 0x80000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

void gameking_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	m_bank4000 = membank("bank4000");
	m_bank8000 = membank("bank8000");
	timeruser_expired=false;
	timer_expired=false;
}

void gameking_state::machine_reset()
{
	memory_region *maincpu_rom = memregion("maincpu");


	maincpu_rom->base()[0x32] = 0; // neccessary to boot correctly
	maincpu_rom->base()[0x33] = 0;
	maincpu_rom->base()[0x3e] = 0;
	m_bank4000->set_base(maincpu_rom->base() + 0x10000+0x4000);
	bank4000_normal=bank4000_irq=maincpu_rom->base() + 0x10000+0x4000;
	m_maincpu->set_irq_bank(m_bank4000, bank4000_normal, bank4000_irq);

	//m_bank8000->set_base(maincpu_rom->base()+0x10000); //? no reason to enforce this yet
	memory_bank *bank2800=membank("bank2800");
	memory_bank *bank3000=membank("bank3000");
	if (m_cart_rom) {
		bank2800->set_base(m_cart_rom->base()+0x2800);
		bank3000->set_base(m_cart_rom->base()+0x3000);
	} else {
		bank2800->set_base(maincpu_rom->base()+0x10000+0x2800); // most likely access to cartridge space/antenne
		bank3000->set_base(maincpu_rom->base()+0x10000+0x3000);
	}
}

INTERRUPT_GEN_MEMBER(gameking_state::gameking_frame_int) // guess to get over bios wai
{
	memory_region *maincpu_rom = memregion("maincpu");
	Gkio *io=reinterpret_cast<Gkio*>(maincpu_rom->base());
	UINT8 i=inputs0;
	inputs1=m_io_joy->read()>>8;
	if (i!=inputs0 && inputs0!=0xff)
	m_maincpu->set_input_line(r65c02gk_device::IRQ_INPUTS, io->interrupt_enable&0x10?ASSERT_LINE:CLEAR_LINE); //? 4in1 popper depends on some effects on $90 bits effected by protection
	i=inputs1;
	inputs1=m_io_joy->read();
	if (i!=inputs1 && inputs1!=0xff)
	m_maincpu->set_input_line(r65c02gk_device::IRQ_USER, io->interrupt_enable&8?ASSERT_LINE:CLEAR_LINE); //? 4in1 popper depends on some effects on $90 bits effected by protection
}


static MACHINE_CONFIG_START( gameking, gameking_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R65C02GK, 6000000)
	MCFG_CPU_PROGRAM_MAP(gameking_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gameking_state,  gameking_frame_int)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(48, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 48-1, 0, 32-1)
	MCFG_SCREEN_UPDATE_DRIVER(gameking_state, screen_update_gameking)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(gameking_palette) * 3)
	MCFG_PALETTE_INIT_OWNER(gameking_state, gameking )

	MCFG_DEFAULT_LAYOUT(layout_gameking)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "gameking_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(gameking_state, gameking_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "gameking")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gameking3, gameking_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R65C02GK, 6000000)
	MCFG_CPU_PROGRAM_MAP(gameking_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gameking_state,  gameking_frame_int)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
#ifdef GK3_PIXEL
	MCFG_SCREEN_SIZE(80*2, 56*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*2-1, 0, 53*2-1)
#else
	MCFG_SCREEN_SIZE(80, 56)
	MCFG_SCREEN_VISIBLE_AREA(0, 80-1, 0, 53-1)
#endif
	MCFG_SCREEN_UPDATE_DRIVER(gameking_state, screen_update_gameking3)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(gameking3_palette) * 3)
	MCFG_PALETTE_INIT_OWNER(gameking_state, gameking3 )

//  MCFG_DEFAULT_LAYOUT(layout_gking3)
	MCFG_DEFAULT_LAYOUT(layout_gameking)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "gameking_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(gameking_state, gameking_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "gameking")
MACHINE_CONFIG_END

ROM_START(gameking)
	ROM_REGION(0x10000+0x80000, "maincpu", ROMREGION_ERASEVAL(0x0d))
//  ROM_LOAD("gm218.bin", 0x10000, 0x80000, CRC(8f52a928) SHA1(2e791fc7b642440d36820d2c53e1bb732375eb6e) ) // a14 inversed
	ROM_LOAD("gm218.bin", 0x10000, 0x80000, CRC(5a1ade3d) SHA1(e0d056f8ebfdf52ef6796d0375eba7fcc4a6a9d3) )
ROM_END

ROM_START(gking3)
	ROM_REGION(0x10000+0x80000, "maincpu", ROMREGION_ERASEVAL(0x0D))
	ROM_LOAD("gm220.bin", 0x10000, 0x80000, CRC(1dc43bd5) SHA1(f9dcd3cb76bb7cb10565a1acb070ab375c082b4c) )
ROM_END

CONS(2003,  gameking,    0,  0,  gameking,    gameking, gameking_state, gameking,    "TimeTop",   "GameKing GM-218", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// the GameKing 2 (GM-219) is probably identical HW

CONS(2005,  gking3,   0,  gameking,  gameking3,    gameking3, gameking_state, gameking,    "TimeTop",   "GameKing III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// gameking 3: similiar cartridges, accepts gameking cartridges, gameking3 cartridges not working on gameking (illegal cartridge scroller)
