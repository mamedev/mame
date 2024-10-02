// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

schematics can be found at
http://www.citylan.it/wiki/index.php/Fast_Invaders_%286845_version%29
and
http://www.citylan.it/wiki/index.php/Fast_Invaders_%288275_version%29


***************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8257.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "video/i8275.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class fastinvaders_state : public driver_device
{
public:
	fastinvaders_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_crtc8275(*this, "8275"),
		m_crtc6845(*this, "6845"),
		m_pic8259(*this, "pic8259"),
		m_dma8257(*this, "dma8257")
	{ }

	void init_fi6845();

	void fastinvaders(machine_config &config);
	void fastinvaders_8275(machine_config &config);
	void fastinvaders_6845(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(start);
	DECLARE_INPUT_CHANGED_MEMBER(start2);
	DECLARE_INPUT_CHANGED_MEMBER(tilt);
	DECLARE_INPUT_CHANGED_MEMBER(in0);
	DECLARE_INPUT_CHANGED_MEMBER(in1);
	DECLARE_INPUT_CHANGED_MEMBER(in2);
	DECLARE_INPUT_CHANGED_MEMBER(in3);
	DECLARE_INPUT_CHANGED_MEMBER(in4);
	DECLARE_INPUT_CHANGED_MEMBER(in5);
	DECLARE_INPUT_CHANGED_MEMBER(in6);

private:
	void io_40_w(uint8_t data);

	uint8_t io_60_r();
	void io_70_w(uint8_t data);
	void io_90_w(uint8_t data);
	void io_a0_w(uint8_t data);
	void io_b0_w(uint8_t data);
	void io_c0_w(uint8_t data);
	void io_d0_w(uint8_t data);
	void io_e0_w(uint8_t data);
	void io_f0_w(uint8_t data);

	int sid_read();

	virtual void video_start() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(count_ar);
	void vsync_w(int state);
	void hsync_w(int state);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	void dark_1_clr(uint8_t data);
	void dark_2_clr(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void fastinvaders_map(address_map &map) ATTR_COLD;
	void fastinvaders_io_base(address_map &map) ATTR_COLD;
	void fastinvaders_6845_io(address_map &map) ATTR_COLD;
	void fastinvaders_8275_io(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_videoram;

	optional_device<i8275_device> m_crtc8275;
	optional_device<mc6845_device> m_crtc6845;
	required_device<pic8259_device> m_pic8259;
	required_device<i8257_device> m_dma8257;

	uint8_t m_rest55 = 0;
	uint8_t m_rest65 = 0;
	uint8_t m_trap = 0;
	uint8_t m_ar = 0;
	uint8_t m_av = 0;
	uint8_t m_prom[256]{};
	uint8_t m_riga_sup = 0;
	uint8_t m_scudi = 0;
	uint8_t m_cannone = 0;
	uint8_t m_riga_inf = 0;

	uint8_t m_irq0 = 0;
	uint8_t m_irq1 = 0;
	uint8_t m_irq2 = 0;
	uint8_t m_irq3 = 0;
	uint8_t m_irq4 = 0;
	uint8_t m_irq5 = 0;
	uint8_t m_irq6 = 0;

	uint8_t m_start2_value = 0;
	uint8_t m_dma1 = 0;
	uint8_t m_io_40 = 0;
	uint8_t m_hsync = 0;
};


TIMER_DEVICE_CALLBACK_MEMBER(fastinvaders_state::scanline_timer)
{
/*  int scanline = param;

    if (scanline == 16)
    {
        //logerror("scanline\n");
        m_dma8257->dreq1_w(0x01);
        m_dma8257->hlda_w(1);
    }
    */
}

TIMER_DEVICE_CALLBACK_MEMBER(fastinvaders_state::count_ar)
{
	if (m_ar < 255)
	{
		m_riga_sup = BIT(m_prom[m_ar], 3);
		m_scudi =    BIT(m_prom[m_ar], 2);
		m_cannone =  BIT(m_prom[m_ar], 1);
		m_riga_inf = BIT(m_prom[m_ar], 0);
		//logerror("m_ar = %02X m_riga_sup %02X, m_scudi %02X, m_cannone %02X, m_riga_inf %02X\n",m_ar,m_riga_sup,m_scudi,m_cannone,m_riga_inf);

		if (m_riga_sup)
		{
			if (BIT(m_prom[m_ar - 1], 3))
			{
				//logerror("            DMA1            \n");
				//logerror("m_prom[m_ar]=%d m_prom[m_ar-1]= %d ar = %d r_s %d, sc %d, ca %d, ri %d\n",m_prom[m_ar],m_prom[m_ar-1],m_ar,m_riga_sup,m_scudi,m_cannone,m_riga_inf);
				m_dma8257->dreq1_w(0x01);
				m_dma8257->hlda_w(1);
				//m_pic8259->ir1_w(HOLD_LINE);
				m_dma1 = 1;
			}
		}
		m_ar++;
	}

	if (m_av < 255)
	{
		m_av++;
		//logerror("m_av=%02X\n",m_av);
		if (m_av == m_io_40)
		{
			if (m_hsync == 1)
			{
				logerror("          DMA2            \n");
				m_dma8257->dreq2_w(0x01);
				m_dma8257->hlda_w(1);
				//m_pic8259->ir3_w(HOLD_LINE);
			}

		}
	}
}

void fastinvaders_state::dark_1_clr(uint8_t data)
{
	//address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	if (data)
	{
		m_dma8257->dreq1_w(0x00);
	}
	if (!data)
	{
		m_dma1 = 0;
	}

	//logerror("dma 1 clr\n");
	//m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	//m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
	//return prog_space.read_byte(offset);
	//return 0x00;
}

void fastinvaders_state::dark_2_clr(uint8_t data)
{
	//address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	if (data)
	{
		m_dma8257->dreq2_w(0x00);
	}
/*  if (!data)
    {
        m_dma1 = 0;
    }
    */
}

/***************************************************************************

  Video

***************************************************************************/

void fastinvaders_state::video_start()
{
}

uint32_t fastinvaders_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	bitmap.fill(rgb_t::black(), cliprect);

	int count = 0;
	for (int y = 0; y < 19; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			uint8_t tile = m_videoram[count];
			gfx->transpen(bitmap, cliprect, tile, 0, 0, 0, x*16, y*14, 0);
			count++;
		}
	}

	return 0;
}

void fastinvaders_state::io_40_w(uint8_t data)
{
	m_io_40 = data;
	logerror("av target= %02X\n",m_io_40);
}


void fastinvaders_state::io_90_w(uint8_t data)
{
	logerror("Audio write &02X\n",data);
}


uint8_t fastinvaders_state::io_60_r()
{
	//0x60 ds6 input bit 0 DX or SX
	//               bit 1 DX or SX
	//               bit 2-7 dip switch

	uint8_t tmp = ioport("IN1")->read() & 0x03;
	tmp =  tmp | (ioport("DSW1")->read() & 0xfc);

	//logerror("return %02X from 0x60\n",tmp);
	return tmp;
}


void fastinvaders_state::io_70_w(uint8_t data)
{
	//bit 0 rest55 clear
	//bit 1 rest65 clear
	//bit 2 trap clear
	//bit 3 coin counter

	//bit 4 irq0 clear
	//bit 5 8085 reset
	//bit 6             TODO
	//bit 7 both used   TODO

	//IRQ clear
	if (BIT(data, 0))
	{
		m_rest55 = 0;
		m_maincpu->set_input_line(I8085_RST55_LINE, CLEAR_LINE);
	}
	if (BIT(data, 1))
	{
		if (m_rest65)
		{
			//logerror("clear");
			m_rest65 = 0;
			m_maincpu->set_input_line(I8085_RST65_LINE, CLEAR_LINE);
		}
	}
	if (BIT(data, 2))
	{
		m_trap=0;
		m_maincpu->set_input_line(INPUT_LINE_NMI,  CLEAR_LINE);
	}
	if (BIT(data, 4))
	{
		m_irq0=0;
		m_pic8259->ir0_w(CLEAR_LINE);
	}

//self reset
	if (BIT(data, 5))
	{
		logerror("reset\n");
	}

//coin counter
//  if (data&0x08){
//      coin_counter_w(machine(), offset,0x01);
//  }
}


void fastinvaders_state::io_a0_w(uint8_t data)
{
	m_irq1 = 0;
	m_pic8259->ir1_w(CLEAR_LINE);
}

void fastinvaders_state::io_b0_w(uint8_t data)
{
	m_irq2 = 0;
	m_pic8259->ir2_w(CLEAR_LINE);
}

void fastinvaders_state::io_c0_w(uint8_t data)
{
	m_irq3 = 0;
	m_pic8259->ir3_w(CLEAR_LINE);
}

void fastinvaders_state::io_d0_w(uint8_t data)
{
	m_irq5 = 0;
	m_pic8259->ir5_w(CLEAR_LINE);
}

void fastinvaders_state::io_e0_w(uint8_t data)
{
	m_irq4 = 0;
	m_pic8259->ir4_w(CLEAR_LINE);
}

void fastinvaders_state::io_f0_w(uint8_t data)
{
	m_irq6 = 0;
	m_pic8259->ir6_w(CLEAR_LINE);
}

int fastinvaders_state::sid_read()
{
	uint8_t tmp = m_start2_value ? ASSERT_LINE : CLEAR_LINE;
	m_start2_value = 0;
	return tmp;
}

INPUT_CHANGED_MEMBER(fastinvaders_state::tilt)
{
	m_trap = 1;
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, HOLD_LINE);
}


INPUT_CHANGED_MEMBER(fastinvaders_state::coin_inserted)
{
	m_rest65 = 1;
	if (newval)
		m_maincpu->set_input_line(I8085_RST65_LINE, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::start)
{
	m_rest55 = 1;
	if (newval)
		m_maincpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::start2)
{
	m_rest55 = 1;
	m_start2_value = 1;
	if (newval)
		m_maincpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in0)
{
	m_irq0 = 1;
	if (newval)
		m_pic8259->ir0_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in1)
{
	m_irq1 = 1;
	if (newval)
		m_pic8259->ir1_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in2)
{
	m_irq2 = 1;
	if (newval)
		m_pic8259->ir2_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in3)
{
	m_irq3 = 1;
	if (newval)
		m_pic8259->ir3_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in4)
{
	m_irq4 = 1;
	if (newval)
		m_pic8259->ir4_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in5)
{
	m_irq5 = 1;
	if (newval)
		m_pic8259->ir5_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in6)
{
	m_irq6 = 1;
	if (newval)
		m_pic8259->ir6_w(HOLD_LINE);
}

void fastinvaders_state::vsync_w(int state)
{
	//logerror("p8257_drq_w\n");
	if (!state)
	{
		m_dma8257->dreq0_w(0x01);
		m_dma8257->hlda_w(1);

		m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
		m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
		//machine().scheduler().abort_timeslice(); // transfer occurs immediately
		//machine().scheduler().perfect_quantum(attotime::from_usec(100)); // smooth things out a bit
		m_av=0;
	}
}

void fastinvaders_state::hsync_w(int state)
{
	//m_hsync=1;
	if (!state)
	{
		m_hsync=0;
		m_ar=0;
	}
	else
	{
		m_hsync=1;
	}
}

uint8_t fastinvaders_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	logerror("dma read\n");
	//m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	//m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
	return prog_space.read_byte(offset);
	//return 0x00;
}

void fastinvaders_state::memory_write_byte(offs_t offset, uint8_t data)
{
	//address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	logerror("dma write\n");
	//m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	//m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
	//return prog_space.read_byte(offset);
	//return 0x00;
}


/***************************************************************************

  I/O

***************************************************************************/

void fastinvaders_state::fastinvaders_map(address_map &map)
{
	//map(0x0000, 0x1fff).rom();   .mirror(0x8000);
	map(0x0000, 0x27ff).rom().mirror(0x8000);
	map(0x2800, 0x2fff).ram().mirror(0x8000).share("videoram");
	map(0x3000, 0x33ff).ram().mirror(0x8000);
}

void fastinvaders_state::fastinvaders_io_base(address_map &map)
{
}

void fastinvaders_state::fastinvaders_6845_io(address_map &map)
{
	fastinvaders_io_base(map);

	map(0x10, 0x1f).rw(m_dma8257, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x20, 0x20).w(m_crtc6845, FUNC(mc6845_device::address_w));
	map(0x21, 0x21).rw(m_crtc6845, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x30, 0x33).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x40, 0x4f).w(FUNC(fastinvaders_state::io_40_w));  //ds4   //latch
	//map(0x50, 0x50).r(FUNC(fastinvaders_state::io_50_r));//ds5   //latch
	map(0x60, 0x60).r(FUNC(fastinvaders_state::io_60_r));
	map(0x70, 0x70).w(FUNC(fastinvaders_state::io_70_w));  //ds7   rest55,rest65,trap, irq0 clear
	map(0x80, 0x80).noprw(); //ds8  write here a LOT ?????
	map(0x90, 0x90).w(FUNC(fastinvaders_state::io_90_w));  //ds9       sound command
	map(0xa0, 0xa0).w(FUNC(fastinvaders_state::io_a0_w));  //ds10 irq1 clear
	map(0xb0, 0xb0).w(FUNC(fastinvaders_state::io_b0_w));  //ds11 irq2 clear
	map(0xc0, 0xc0).w(FUNC(fastinvaders_state::io_c0_w));  //ds12 irq3 clear
	map(0xd0, 0xd0).w(FUNC(fastinvaders_state::io_d0_w));  //ds13 irq5 clear
	map(0xe0, 0xe0).w(FUNC(fastinvaders_state::io_e0_w));  //ds14 irq4 clear
	map(0xf0, 0xf0).w(FUNC(fastinvaders_state::io_f0_w));  //ds15 irq6 clear
}

void fastinvaders_state::fastinvaders_8275_io(address_map &map)
{
	fastinvaders_io_base(map);

	map(0x10, 0x1f).rw(m_dma8257, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x20, 0x21).rw(m_crtc8275, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x30, 0x33).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x40, 0x4f).w(FUNC(fastinvaders_state::io_40_w));  //ds4   //latch
	//map(0x50, 0x50).r(FUNC(fastinvaders_state::io_50_r));//latch
	map(0x60, 0x60).r(FUNC(fastinvaders_state::io_60_r));
	map(0x70, 0x70).w(FUNC(fastinvaders_state::io_70_w));  //ds7   rest55,rest65,trap, irq0 clear
	map(0x80, 0x80).noprw(); //write here a LOT
	//map(0x80, 0x80).w(FUNC(fastinvaders_state::io_80_w));//ds8 ????
	map(0x90, 0x90).w(FUNC(fastinvaders_state::io_90_w));  //ds9       sound command
	map(0xa0, 0xa0).w(FUNC(fastinvaders_state::io_a0_w));  //ds10 irq1 clear
	map(0xb0, 0xb0).w(FUNC(fastinvaders_state::io_b0_w));  //ds11 irq2 clear
	map(0xc0, 0xc0).w(FUNC(fastinvaders_state::io_c0_w));  //ds12 irq3 clear
	map(0xd0, 0xd0).w(FUNC(fastinvaders_state::io_d0_w));  //ds13 irq5 clear
	map(0xe0, 0xe0).w(FUNC(fastinvaders_state::io_e0_w));  //ds14 irq4 clear
	map(0xf0, 0xf0).w(FUNC(fastinvaders_state::io_f0_w));  //ds15 irq6 clear
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( fastinvaders )

PORT_START("COIN")  /* FAKE async input */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state, coin_inserted, 0)   //I8085_RST65_LINE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state, start, 0)          //I8085_RST55_LINE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state, start2, 0)         //I8085_RST55_LINE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in0, 0) // int0, sparo
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,tilt, 0)    //INPUT_LINE_NMI tilt

	PORT_START("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("1") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in1, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("2") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in2, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("3") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in3, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("4") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in4, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("5") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in5, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("6") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in6, 0)

	PORT_START("IN1")   //0x60 io port
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")  //0x60 io port
	PORT_DIPNAME(   0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME(   0xf0, 0x80, "Ships/lives number" )
	PORT_DIPSETTING(      0x10, "1 Ship"  )
	PORT_DIPSETTING(      0x20, "2 Ships" )
	PORT_DIPSETTING(      0x30, "3 Ships"  )
	PORT_DIPSETTING(      0x40, "4 Ships" )
	PORT_DIPSETTING(      0x50, "5 Ships" )
	PORT_DIPSETTING(      0x60, "6 Ships" )
	PORT_DIPSETTING(      0x70, "7 Ships" )
	PORT_DIPSETTING(      0x80, "8 Ships" )
	PORT_DIPSETTING(      0x90, "9 Ships" )



INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout charlayout =
{
	16,14,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) +1, RGN_FRAC(1,2) +2, RGN_FRAC(1,2) +3, RGN_FRAC(1,2) +4, RGN_FRAC(1,2) +5, RGN_FRAC(1,2) +6, RGN_FRAC(1,2) +7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 /*, 14*8, 15*8*/ },
	16*8
};

static GFXDECODE_START( gfx_fastinvaders )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
GFXDECODE_END

void fastinvaders_state::fastinvaders(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 6144100/2); // 6144100 Xtal /2 internaly
	m_maincpu->set_addrmap(AS_PROGRAM, &fastinvaders_state::fastinvaders_map);
//  m_maincpu->set_addrmap(AS_IO, &fastinvaders_state::fastinvaders_io_map);
//  m_maincpu->set_vblank_int("screen", FUNC(fastinvaders_state::irq0_line_hold));
	m_maincpu->in_sid_func().set(FUNC(fastinvaders_state::sid_read));
	m_maincpu->in_inta_func().set("pic8259", FUNC(pic8259_device::acknowledge));

	TIMER(config, "scantimer").configure_scanline(FUNC(fastinvaders_state::scanline_timer), "screen", 0, 1);

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	I8257(config, m_dma8257, 6144100);
	m_dma8257->in_memr_cb().set(FUNC(fastinvaders_state::memory_read_byte));
	m_dma8257->out_memw_cb().set(FUNC(fastinvaders_state::memory_write_byte));
	m_dma8257->out_dack_cb<1>().set(FUNC(fastinvaders_state::dark_1_clr));
	m_dma8257->out_dack_cb<2>().set(FUNC(fastinvaders_state::dark_2_clr));

	TIMER(config, "count_ar").configure_periodic(FUNC(fastinvaders_state::count_ar), attotime::from_hz(11500000/2));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0*16, 40*16-1, 0*14, 19*14-1);
	screen.set_screen_update(FUNC(fastinvaders_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_fastinvaders);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	// TODO
}

void fastinvaders_state::fastinvaders_8275(machine_config &config)
{
	fastinvaders(config);
	m_maincpu->set_addrmap(AS_IO, &fastinvaders_state::fastinvaders_8275_io);

	I8275(config, m_crtc8275, 10000000); /* guess */ // does not configure a very useful resolution(!)
	m_crtc8275->set_character_width(16);
//  m_crtc8275->set_display_callback(FUNC(apogee_state::display_pixels));
//  m_crtc8275->drq_wr_callback().set("dma8257", FUNC(i8257_device::dreq2_w));
}

void fastinvaders_state::fastinvaders_6845(machine_config &config)
{
	fastinvaders(config);
	m_maincpu->set_addrmap(AS_IO, &fastinvaders_state::fastinvaders_6845_io);

	MC6845(config, m_crtc6845, 11500000/16); /* confirmed */
	m_crtc6845->set_screen("screen");
	m_crtc6845->set_show_border_area(false);
	m_crtc6845->set_char_width(16);
	m_crtc6845->out_vsync_callback().set(FUNC(fastinvaders_state::vsync_w));
	m_crtc6845->out_hsync_callback().set(FUNC(fastinvaders_state::hsync_w));
}

void fastinvaders_state::init_fi6845()
{
	const uint8_t *prom = memregion("prom")->base();
	for (int i = 0; i < 256; i++)
	{
		m_prom[i] = prom[i];
	}
	m_dma1 = 0;
	m_io_40 = 0;
}


/***************************************************************************

  Game drivers

***************************************************************************/

// the last pair of gfx roms were mixed up in each set (each set contained 2 identical roms rather than one of each) hopefully nothing else was

//ROM_START( fi6845 )
ROM_START( fi8275 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r2.1a",     0x0000, 0x0200, CRC(6180d652) SHA1(3aac67f52897059c8366f52c41464052ce860ae8) )
	ROM_LOAD( "r2.1b",     0x0200, 0x0200, CRC(f10baf3e) SHA1(4a1702c24e949d9bef990033b5507a573abd7bc3) )
	ROM_LOAD( "r2.2a",     0x0400, 0x0200, CRC(f446ef0d) SHA1(2be337c1197d14e5ffc33ea05b5262f1ea17d442) )
	ROM_LOAD( "r2.2b",     0x0600, 0x0200, CRC(b97e35a3) SHA1(0878a83c7f9f0645749fdfb1ff372d0e04833c9e) )
	ROM_LOAD( "r2.3a",     0x0800, 0x0200, CRC(988f36da) SHA1(7229f660a6a9cf9f66d0924c63772daabd09710e) )
	ROM_LOAD( "r2.3b",     0x0a00, 0x0200, CRC(be7dc34d) SHA1(e4aa1617629869c9ff5f39b656001a43020f0cb8) )
	ROM_LOAD( "r2.4a",     0x0c00, 0x0200, CRC(199cb227) SHA1(064f6005a3f1afe9ca04e93e6bc999735a12a05b) )
	ROM_LOAD( "r2.4b",     0x0e00, 0x0200, CRC(ca41218a) SHA1(01529e21c44669dc96df4331e87d45098a263772) )
	ROM_LOAD( "r2.5a",     0x1000, 0x0200, CRC(e8ecf0da) SHA1(723edaa6f069a21ab7496a24831f23b4b4d73629) )
	ROM_LOAD( "r2.5b",     0x1200, 0x0200, CRC(cb2d8029) SHA1(cac067accddfcce6014d2a425b4291ed8226d169) )
	ROM_LOAD( "r2.6a",     0x1400, 0x0200, CRC(e4d4cc96) SHA1(2dc3d7e4cbd93220285938aec31011b685563cf7) )
	ROM_LOAD( "r2.6b",     0x1600, 0x0200, CRC(0c96ba4a) SHA1(0da104472c33523d4002cab0c77ca20fc2998a2c) )
	ROM_LOAD( "r2.7a",     0x1800, 0x0200, CRC(c9207fbd) SHA1(bf388e26ee1e2073b8a641ba6fb551c24d471a70) )

	ROM_LOAD( "r2.1a",     0x2000, 0x0200, CRC(6180d652) SHA1(3aac67f52897059c8366f52c41464052ce860ae8) )
	ROM_LOAD( "r2.1b",     0x2200, 0x0200, CRC(f10baf3e) SHA1(4a1702c24e949d9bef990033b5507a573abd7bc3) )
	ROM_LOAD( "r2.2a",     0x2400, 0x0200, CRC(f446ef0d) SHA1(2be337c1197d14e5ffc33ea05b5262f1ea17d442) )
	ROM_LOAD( "r2.2b",     0x2600, 0x0200, CRC(b97e35a3) SHA1(0878a83c7f9f0645749fdfb1ff372d0e04833c9e) )


	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "c2.1f",     0x0000, 0x0200, CRC(9feca88a) SHA1(14a8c46eb51eed01b7b537a9931cd092cec2019f) )
	ROM_LOAD( "c2.1g",     0x0200, 0x0200, CRC(79fc3963) SHA1(25651d1031895a01a2a4751b355ff1200a899ac5) )
	ROM_LOAD( "c2.1h",     0x0400, 0x0200, CRC(936171e4) SHA1(d0756b49bfd5d58a79f735d4a98a99cce7604b0e) )
	ROM_LOAD( "c2.2f",     0x0600, 0x0200, CRC(3bb16f55) SHA1(b1cc1e2346acd0e5c84861b414b4677871079844) )
	ROM_LOAD( "c2.2g",     0x0800, 0x0200, CRC(19828c47) SHA1(f215ce55be32b3564e1b7cc19500d38a93117051) )
	ROM_LOAD( "c2.2h",     0x0a00, 0x0200, CRC(284ae4eb) SHA1(6e28fcd9d481d37f47728f22f6048b29266f4346) )

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "93427.bin",     0x0000, 0x0100, CRC(f59c8573) SHA1(5aed4866abe1690fd0f088af1cfd99b3c85afe9a) )
ROM_END

//ROM_START( fi8275 )
ROM_START( fi6845 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r1.1a",     0x0000, 0x0200, CRC(fef96dfe) SHA1(f6df0cf6b5d90ea07ee890985c8cbf0f68e08550) )
	ROM_LOAD( "r1.1b",     0x0200, 0x0200, CRC(c48c6ebc) SHA1(f1f86839819b6abce9ff55c1b02bbf2c4036c51a) )
	ROM_LOAD( "r1.2a",     0x0400, 0x0200, CRC(626a740c) SHA1(3a1df1d71acc207b1b952ad5176804fce27ea97e) )
	ROM_LOAD( "r1.2b",     0x0600, 0x0200, CRC(fbe9782e) SHA1(3661bd03e029e4d2092d259f38a7dec9e763761c) )
	ROM_LOAD( "r1.3a",     0x0800, 0x0200, CRC(6e10de0d) SHA1(8d937f6f2fe1a79b62e6e75536889416ec0071e3) )
	ROM_LOAD( "r1.3b",     0x0a00, 0x0200, CRC(ee1bac50) SHA1(f723b2d1c2a1194aa2df67d48f2b669f5076d857) )
	ROM_LOAD( "r1.4a",     0x0c00, 0x0200, CRC(7faff8f1) SHA1(9275123d6513ab917506f6e9d929935ed1bef429) )
	ROM_LOAD( "r1.4b",     0x0e00, 0x0200, CRC(205ca0c1) SHA1(edf68e9c75523e1b6a485b27af60592fdfb78e04) )
	ROM_LOAD( "r1.5a",     0x1000, 0x0200, CRC(9ada6666) SHA1(f965a08c75fa87e8e3fd7595dcd98231e976e072) )
	ROM_LOAD( "r1.5b",     0x1200, 0x0200, CRC(0f617215) SHA1(b342c783335ab26c036ae77f63a2e932a590c2fa) )
	ROM_LOAD( "r1.6a",     0x1400, 0x0200, CRC(75ea69ae) SHA1(edd9bf686c169ca64373ea87ba92fab4e8c6ee4d) )
	//ROM_LOAD( "r1.6b",   0x1600, 0x0200, NO_DUMP) ) // not populated
	ROM_LOAD( "r1.7a",     0x1800, 0x0200, CRC(6e12538f) SHA1(aa08a2db2e5570b431afc967ea5fd749c4f82e33) )
	ROM_LOAD( "r1.7b",     0x1a00, 0x0200, CRC(7270d194) SHA1(7cef9c420c3c3cbc5846bd22137213a78506a8d3) )

	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "c1.1b",     0x0000, 0x0200, CRC(9feca88a) SHA1(14a8c46eb51eed01b7b537a9931cd092cec2019f) )
	ROM_LOAD( "c1.2b",     0x0200, 0x0200, CRC(79fc3963) SHA1(25651d1031895a01a2a4751b355ff1200a899ac5) )
	ROM_LOAD( "c1.3b",     0x0400, 0x0200, CRC(936171e4) SHA1(d0756b49bfd5d58a79f735d4a98a99cce7604b0e) )
	ROM_LOAD( "c1.1a",     0x0600, 0x0200, CRC(3bb16f55) SHA1(b1cc1e2346acd0e5c84861b414b4677871079844) )
	ROM_LOAD( "c1.2a",     0x0800, 0x0200, CRC(19828c47) SHA1(f215ce55be32b3564e1b7cc19500d38a93117051) )
	ROM_LOAD( "c1.3a",     0x0a00, 0x0200, CRC(284ae4eb) SHA1(6e28fcd9d481d37f47728f22f6048b29266f4346) )

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "93427.bin",     0x0000, 0x0100, CRC(f59c8573) SHA1(5aed4866abe1690fd0f088af1cfd99b3c85afe9a) )
ROM_END

} // anonymous namespace


//   YEAR   NAME    PARENT  MACHINE            INPUT         STATE               INIT         ROT     COMPANY       FULLNAME                        FLAGS
GAME( 1979, fi6845, 0,      fastinvaders_6845, fastinvaders, fastinvaders_state, init_fi6845, ROT270, "Fiberglass", "Fast Invaders (6845 version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1979, fi8275, fi6845, fastinvaders_8275, fastinvaders, fastinvaders_state, init_fi6845, ROT270, "Fiberglass", "Fast Invaders (8275 version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
