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

#include "video/i8275.h"
#include "video/mc6845.h"
#include "machine/pic8259.h"
#include "machine/i8257.h"


class fastinvaders_state : public driver_device
{
public:
	fastinvaders_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_crtc8275(*this, "8275"),
		m_crtc6845(*this, "6845"),
		m_pic8259(*this, "pic8259"),
		m_dma8257(*this, "dma8257")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<UINT8> m_videoram;
	
	optional_device<i8275_device> m_crtc8275;
	optional_device<mc6845_device> m_crtc6845;
	required_device<pic8259_device> m_pic8259;
	required_device<i8257_device> m_dma8257;

	UINT8 m_rest55;
	UINT8 m_rest65;
	UINT8 m_trap;
	UINT8 m_ar;
	UINT8 m_av;
	UINT8 m_prom[256];
	UINT8 m_riga_sup;
	UINT8 m_scudi;
	UINT8 m_cannone;
	UINT8 m_riga_inf;
	
	UINT8 m_irq0;
	UINT8 m_irq1;
	UINT8 m_irq2;
	UINT8 m_irq3;
	UINT8 m_irq4;
	UINT8 m_irq5;
	UINT8 m_irq6;
	UINT8 m_irq7;
	
	
	UINT8 m_start2_value;
	UINT8 m_dma1;
	UINT8 m_io_40;
	UINT8 m_hsync;
	
	DECLARE_WRITE8_MEMBER(io_40_w);
	
	DECLARE_READ8_MEMBER(io_60_r);
	DECLARE_WRITE8_MEMBER(io_70_w);
	DECLARE_WRITE8_MEMBER(io_90_w);
	DECLARE_WRITE8_MEMBER(io_a0_w);
	DECLARE_WRITE8_MEMBER(io_b0_w);
	DECLARE_WRITE8_MEMBER(io_c0_w);
	DECLARE_WRITE8_MEMBER(io_d0_w);
	DECLARE_WRITE8_MEMBER(io_e0_w);
	DECLARE_WRITE8_MEMBER(io_f0_w);
		
	
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
	
	DECLARE_READ_LINE_MEMBER(sid_read);

	
	virtual void video_start() override;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(count_ar);
	DECLARE_WRITE_LINE_MEMBER(vsync);
	DECLARE_WRITE_LINE_MEMBER(hsync);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_WRITE8_MEMBER(dark_1_clr);
	DECLARE_WRITE8_MEMBER(dark_2_clr);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	
	
	DECLARE_DRIVER_INIT(fi6845);
	
};


TIMER_DEVICE_CALLBACK_MEMBER(fastinvaders_state::scanline_timer)
{
/*	int scanline = param;




	if(scanline == 16){
		//logerror("scanline\n");
		m_dma8257->dreq1_w(0x01);
	m_dma8257->hlda_w(1);
		}
	*/	
}

TIMER_DEVICE_CALLBACK_MEMBER(fastinvaders_state::count_ar)
{
	if (m_ar<255){
		
		m_riga_sup= ((m_prom[m_ar]&0x08)>>3)&0x01;
		m_scudi= 	((m_prom[m_ar]&0x04)>>2)&0x01;
		m_cannone= 	((m_prom[m_ar]&0x02)>>1)&0x01;
		m_riga_inf= ((m_prom[m_ar]&0x01))&0x01;
		//logerror("m_ar = %02X m_riga_sup %02X, m_scudi %02X, m_cannone %02X, m_riga_inf %02X\n",m_ar,m_riga_sup,m_scudi,m_cannone,m_riga_inf);
	
		if(m_riga_sup==0x01){
			if(((m_prom[m_ar-1]&0x08)>>3)==0x01){
				//logerror("			DMA1			\n");
				//logerror("m_prom[m_ar]=%d m_prom[m_ar-1]= %d ar = %d r_s %d, sc %d, ca %d, ri %d\n",m_prom[m_ar],m_prom[m_ar-1],m_ar,m_riga_sup,m_scudi,m_cannone,m_riga_inf);
				m_dma8257->dreq1_w(0x01);
				m_dma8257->hlda_w(1);
				//m_pic8259->ir1_w(HOLD_LINE);
				m_dma1=1;
			}
		}
		m_ar++;
	}
	
	if (m_av<255){
		m_av++;
		//logerror("m_av=%02X\n",m_av);
		if (m_av == m_io_40){
			if (m_hsync==1){
				logerror("			DMA2			\n");
				m_dma8257->dreq2_w(0x01);
				m_dma8257->hlda_w(1);
				//m_pic8259->ir3_w(HOLD_LINE);
			}
		
		}
	}	
}

WRITE8_MEMBER(fastinvaders_state::dark_1_clr)
{
	//address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	if(data){
		m_dma8257->dreq1_w(0x00);
	}
	if(!data){
		m_dma1=0;
	}
	
//logerror("dma 1 clr\n");
	//m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	//m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
	//return prog_space.read_byte(offset);
	//return 0x00;
}

WRITE8_MEMBER(fastinvaders_state::dark_2_clr)
{
	//address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	if(data){
		m_dma8257->dreq2_w(0x00);
	}
/*	if(!data){
		m_dma1=0;
	}
	*/
}

/***************************************************************************

  Video

***************************************************************************/

void fastinvaders_state::video_start()
{
}

UINT32 fastinvaders_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	bitmap.fill(0, cliprect);

	int count = 0;

	for (int y = 0;y < 19;y++)
	{
		for (int x = 0;x < 40;x++)
		{
			UINT8 tile = m_videoram[count];


			gfx->transpen(
				bitmap,
				cliprect,
				tile,
				0,
				0, 0,
				x*16, y*14, 0
				);

			count++;
		
		}

	}

	return 0;
}

WRITE8_MEMBER(fastinvaders_state::io_40_w)
{
	m_io_40=data;
	logerror("av target= %02X\n",m_io_40);
}


WRITE8_MEMBER(fastinvaders_state::io_90_w)
{
	logerror("Audio write %02X\n",data);
}


READ8_MEMBER(fastinvaders_state::io_60_r)
{
	UINT8 tmp=0;
	//0x60 ds6 input bit 0 DX or SX
	//				 bit 1 DX or SX
//				 bit 2-7 dip switch

	tmp=ioport("IN1")->read()&0x03;
	tmp=tmp | (ioport("DSW1")->read()&0xfc);

	//logerror("return %02X from 0x60\n",tmp);
	return tmp;
}


WRITE8_MEMBER(fastinvaders_state::io_70_w)
{
//bit 0	rest55 clear
//bit 1 rest65 clear
//bit 2 trap clear
//bit 3	coin counter

//bit 4 irq0 clear
//bit 5	8085 reset
//bit 6				TODO
//bit 7	both used 	TODO

//IRQ clear
	if (data&0x01){
		m_rest55=0;
		m_maincpu->set_input_line(I8085_RST55_LINE, CLEAR_LINE);
	}
	if (data&0x02){
		
		if (m_rest65){
			//logerror("clear");
			m_rest65=0;
			m_maincpu->set_input_line(I8085_RST65_LINE, CLEAR_LINE);
		}
	}	
	if (data&0x04){
		m_trap=0;
		m_maincpu->set_input_line(INPUT_LINE_NMI,  CLEAR_LINE);
	}	
	if (data&0x10){
		m_irq0=0;
		m_pic8259->ir0_w(CLEAR_LINE);
	}		

//self reset
	if (data&0x20){
		logerror("RESET!!!!!\n");
	}	

	
//coin counter
//	if (data&0x08){
//		coin_counter_w(machine(), offset,0x01);
//	}			

}


WRITE8_MEMBER(fastinvaders_state::io_a0_w)
{
	m_irq1=0;
	m_pic8259->ir1_w(CLEAR_LINE);
}

WRITE8_MEMBER(fastinvaders_state::io_b0_w)
{
	m_irq2=0;
	m_pic8259->ir2_w(CLEAR_LINE);
}

WRITE8_MEMBER(fastinvaders_state::io_c0_w)
{
	m_irq3=0;
	m_pic8259->ir3_w(CLEAR_LINE);
}

WRITE8_MEMBER(fastinvaders_state::io_d0_w)
{
	m_irq5=0;
	m_pic8259->ir5_w(CLEAR_LINE);
}

WRITE8_MEMBER(fastinvaders_state::io_e0_w)
{
	m_irq4=0;
	m_pic8259->ir4_w(CLEAR_LINE);
}

WRITE8_MEMBER(fastinvaders_state::io_f0_w)
{
	m_irq6=0;
	m_pic8259->ir6_w(CLEAR_LINE);
}

READ_LINE_MEMBER(fastinvaders_state::sid_read)
{
	UINT8 tmp= m_start2_value ? ASSERT_LINE : CLEAR_LINE;
	m_start2_value=0;
	return tmp; 
}

INPUT_CHANGED_MEMBER(fastinvaders_state::tilt)
{
	m_trap=1;
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, HOLD_LINE);
}


INPUT_CHANGED_MEMBER(fastinvaders_state::coin_inserted)
{
	m_rest65=1;
	if (newval)
		m_maincpu->set_input_line(I8085_RST65_LINE, HOLD_LINE);
}


INPUT_CHANGED_MEMBER(fastinvaders_state::start)
{
	m_rest55=1;
	if (newval)
		m_maincpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::start2)
{
	m_rest55=1;
	m_start2_value=1;
	if (newval)
		m_maincpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in0)
{
	m_irq0=1;
	if (newval)
		m_pic8259->ir0_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in1)
{
	m_irq1=1;
	if (newval)
		m_pic8259->ir1_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in2)
{
	m_irq2=1;
	if (newval)
		m_pic8259->ir2_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in3)
{
	m_irq3=1;
	if (newval)
		m_pic8259->ir3_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in4)
{
	m_irq4=1;
	if (newval) 
		m_pic8259->ir4_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in5)
{
	m_irq5=1;
	if (newval)
		m_pic8259->ir5_w(HOLD_LINE);
}

INPUT_CHANGED_MEMBER(fastinvaders_state::in6)
{
	m_irq6=1;
	if (newval)	
		m_pic8259->ir6_w(HOLD_LINE);
}




DECLARE_WRITE_LINE_MEMBER( fastinvaders_state::vsync)
{
	//logerror("p8257_drq_w\n");
	if (!state){
		m_dma8257->dreq0_w(0x01);
		m_dma8257->hlda_w(1);
	
		m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
		m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
	//machine().scheduler().abort_timeslice(); // transfer occurs immediately
	//machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100)); // smooth things out a bit
		m_av=0;
	}

	if (state){

	}	
}

DECLARE_WRITE_LINE_MEMBER( fastinvaders_state::hsync)
{
	//m_hsync=1;
	if (!state){
		m_hsync=0;
		m_ar=0;
	}

	if (state){
		m_hsync=1;
	}	
}



READ8_MEMBER(fastinvaders_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
logerror("dma read\n");
	//m_maincpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	//m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
	return prog_space.read_byte(offset);
	//return 0x00;
}

WRITE8_MEMBER(fastinvaders_state::memory_write_byte)
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

static ADDRESS_MAP_START( fastinvaders_map, AS_PROGRAM, 8, fastinvaders_state )
	//AM_RANGE(0x0000, 0x1fff) AM_ROM	AM_MIRROR(0x8000)
	AM_RANGE(0x0000, 0x27ff) AM_ROM	AM_MIRROR(0x8000)
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_MIRROR(0x8000) AM_SHARE("videoram")
	AM_RANGE(0x3000, 0x33ff) AM_RAM AM_MIRROR(0x8000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fastinvaders_io_base, AS_IO, 8, fastinvaders_state )
ADDRESS_MAP_END

static ADDRESS_MAP_START( fastinvaders_6845_io, AS_IO, 8, fastinvaders_state )
	AM_RANGE(0x10, 0x1f) AM_DEVREADWRITE("dma8257", i8257_device, read, write)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("6845", mc6845_device, address_w)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("6845", mc6845_device, register_r, register_w)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x40, 0x4f) AM_WRITE(io_40_w)	//ds4	//latch
	//AM_RANGE(0x50, 0x50) AM_READ(io_50_r)	//ds5	//latch
	AM_RANGE(0x60, 0x60) AM_READ(io_60_r)
	AM_RANGE(0x70, 0x70) AM_WRITE(io_70_w)	//ds7	rest55,rest65,trap, irq0 clear
	AM_RANGE(0x80, 0x80) AM_NOP	//ds8  write here a LOT ?????
	AM_RANGE(0x90, 0x90) AM_WRITE(io_90_w)	//ds9		sound command
	AM_RANGE(0xa0, 0xa0) AM_WRITE(io_a0_w)	//ds10 irq1 clear
	AM_RANGE(0xb0, 0xb0) AM_WRITE(io_b0_w)	//ds11 irq2 clear
	AM_RANGE(0xc0, 0xc0) AM_WRITE(io_c0_w)	//ds12 irq3 clear
	AM_RANGE(0xd0, 0xd0) AM_WRITE(io_d0_w)	//ds13 irq5 clear
	AM_RANGE(0xe0, 0xe0) AM_WRITE(io_e0_w)	//ds14 irq4 clear
	AM_RANGE(0xf0, 0xf0) AM_WRITE(io_f0_w)	//ds15 irq6 clear
	
	AM_IMPORT_FROM(fastinvaders_io_base)
ADDRESS_MAP_END


static ADDRESS_MAP_START( fastinvaders_8275_io, AS_IO, 8, fastinvaders_state )
	AM_RANGE( 0x20, 0x21 ) AM_DEVREADWRITE("8275", i8275_device, read, write) 
	
AM_RANGE(0x10, 0x1f) AM_DEVREADWRITE("dma8257", i8257_device, read, write)	
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x40, 0x4f) AM_WRITE(io_40_w)	//ds4	//latch
	//AM_RANGE(0x50, 0x50) AM_READ(io_50_r)	//ds5	//latch
	AM_RANGE(0x60, 0x60) AM_READ(io_60_r)
	AM_RANGE(0x70, 0x70) AM_WRITE(io_70_w)	//ds7	rest55,rest65,trap, irq0 clear
	AM_RANGE(0x80, 0x80) AM_NOP	//write here a LOT
	//AM_RANGE(0x80, 0x80) AM_WRITE(io_80_w)	//ds8 ????
	AM_RANGE(0x90, 0x90) AM_WRITE(io_90_w)	//ds9		sound command
	AM_RANGE(0xa0, 0xa0) AM_WRITE(io_a0_w)	//ds10 irq1 clear
	AM_RANGE(0xb0, 0xb0) AM_WRITE(io_b0_w)	//ds11 irq2 clear
	AM_RANGE(0xc0, 0xc0) AM_WRITE(io_c0_w)	//ds12 irq3 clear
	AM_RANGE(0xd0, 0xd0) AM_WRITE(io_d0_w)	//ds13 irq5 clear
	AM_RANGE(0xe0, 0xe0) AM_WRITE(io_e0_w)	//ds14 irq4 clear
	AM_RANGE(0xf0, 0xf0) AM_WRITE(io_f0_w)	//ds15 irq6 clear	
	AM_IMPORT_FROM(fastinvaders_io_base)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( fastinvaders )

PORT_START("COIN")  /* FAKE async input */	
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state, coin_inserted, 0)	//I8085_RST65_LINE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state, start, 0)			//I8085_RST55_LINE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state, start2, 0)			//I8085_RST55_LINE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in0, 0)	// int0, sparo
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,tilt, 0)	//INPUT_LINE_NMI tilt
	

	PORT_START("IN0")
	
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("1") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in1, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("2") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in2, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("3") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in3, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("4") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in4, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("5") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in5, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("6") PORT_CHANGED_MEMBER(DEVICE_SELF, fastinvaders_state,in6, 0)
	
	
	PORT_START("IN1")	//0x60 io port
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	
	PORT_START("DSW1")	//0x60 io port
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

static GFXDECODE_START( fastinvaders )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( fastinvaders, fastinvaders_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, 6144100/2 ) // 6144100 Xtal /2 internaly
	MCFG_CPU_PROGRAM_MAP(fastinvaders_map)
//	MCFG_CPU_IO_MAP(fastinvaders_io_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", fastinvaders_state, irq0_line_hold)
	MCFG_I8085A_SID(READLINE(fastinvaders_state, sid_read)) 
MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)
MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", fastinvaders_state, scanline_timer, "screen", 0, 1)
	
	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL)

	MCFG_DEVICE_ADD("dma8257", I8257, 6144100)
	MCFG_I8257_IN_MEMR_CB(READ8(fastinvaders_state, memory_read_byte))
	MCFG_I8257_OUT_MEMW_CB(WRITE8(fastinvaders_state, memory_write_byte))
	MCFG_I8257_OUT_DACK_1_CB(WRITE8(fastinvaders_state, dark_1_clr))
	MCFG_I8257_OUT_DACK_2_CB(WRITE8(fastinvaders_state, dark_2_clr))
	
	
	MCFG_TIMER_DRIVER_ADD_PERIODIC("count_ar", fastinvaders_state, count_ar,  attotime::from_hz(11500000/2))
	
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(64*16, 32*16)	
	MCFG_SCREEN_VISIBLE_AREA(0*16, 40*16-1, 0*14, 19*14-1)		
	MCFG_SCREEN_UPDATE_DRIVER(fastinvaders_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fastinvaders)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	// TODO
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fastinvaders_8275, fastinvaders )
	MCFG_CPU_MODIFY("maincpu" ) // guess
	MCFG_CPU_IO_MAP(fastinvaders_8275_io)
	
	MCFG_DEVICE_ADD("8275", I8275, 10000000 ) /* guess */ // does not configure a very useful resolution(!)
	MCFG_I8275_CHARACTER_WIDTH(16)
//	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(apogee_state, display_pixels)
//	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE("dma8257",i8257_device, dreq2_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fastinvaders_6845, fastinvaders )
	MCFG_CPU_MODIFY("maincpu" ) // guess
	MCFG_CPU_IO_MAP(fastinvaders_6845_io)

	MCFG_MC6845_ADD("6845", MC6845, "screen", 11500000/16) /* confirmed */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(16)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(fastinvaders_state,vsync))
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(fastinvaders_state,hsync))
MACHINE_CONFIG_END




DRIVER_INIT_MEMBER(fastinvaders_state, fi6845)
{
const UINT8 *prom = memregion("prom")->base();
	int i;
	for (i=0;i<256;i++){
		m_prom[i]=prom[i];
	}
	m_dma1=0;
	m_io_40=0;
}


/***************************************************************************

  Game drivers

***************************************************************************/

// the last pair of gfx roms were mixed up in each set (each set contained 2 identical roms rather than one of each) hopefully nothing else was

//ROM_START( fi6845 )
ROM_START( fi8275 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "R2.1A",     0x0000, 0x0200, CRC(6180d652) SHA1(3aac67f52897059c8366f52c41464052ce860ae8) )
	ROM_LOAD( "R2.1B",     0x0200, 0x0200, CRC(f10baf3e) SHA1(4a1702c24e949d9bef990033b5507a573abd7bc3) )
	ROM_LOAD( "R2.2A",     0x0400, 0x0200, CRC(f446ef0d) SHA1(2be337c1197d14e5ffc33ea05b5262f1ea17d442) )
	ROM_LOAD( "R2.2B",     0x0600, 0x0200, CRC(b97e35a3) SHA1(0878a83c7f9f0645749fdfb1ff372d0e04833c9e) )
	ROM_LOAD( "R2.3A",     0x0800, 0x0200, CRC(988f36da) SHA1(7229f660a6a9cf9f66d0924c63772daabd09710e) )
	ROM_LOAD( "R2.3B",     0x0a00, 0x0200, CRC(be7dc34d) SHA1(e4aa1617629869c9ff5f39b656001a43020f0cb8) )
	ROM_LOAD( "R2.4A",     0x0c00, 0x0200, CRC(199cb227) SHA1(064f6005a3f1afe9ca04e93e6bc999735a12a05b) )
	ROM_LOAD( "R2.4B",     0x0e00, 0x0200, CRC(ca41218a) SHA1(01529e21c44669dc96df4331e87d45098a263772) )
	ROM_LOAD( "R2.5A",     0x1000, 0x0200, CRC(e8ecf0da) SHA1(723edaa6f069a21ab7496a24831f23b4b4d73629) )
	ROM_LOAD( "R2.5B",     0x1200, 0x0200, CRC(cb2d8029) SHA1(cac067accddfcce6014d2a425b4291ed8226d169) )
	ROM_LOAD( "R2.6A",     0x1400, 0x0200, CRC(e4d4cc96) SHA1(2dc3d7e4cbd93220285938aec31011b685563cf7) )
	ROM_LOAD( "R2.6B",     0x1600, 0x0200, CRC(0c96ba4a) SHA1(0da104472c33523d4002cab0c77ca20fc2998a2c) )
	ROM_LOAD( "R2.7A",     0x1800, 0x0200, CRC(c9207fbd) SHA1(bf388e26ee1e2073b8a641ba6fb551c24d471a70) )

	ROM_LOAD( "R2.1A",     0x2000, 0x0200, CRC(6180d652) SHA1(3aac67f52897059c8366f52c41464052ce860ae8) )
	ROM_LOAD( "R2.1B",     0x2200, 0x0200, CRC(f10baf3e) SHA1(4a1702c24e949d9bef990033b5507a573abd7bc3) )
	ROM_LOAD( "R2.2A",     0x2400, 0x0200, CRC(f446ef0d) SHA1(2be337c1197d14e5ffc33ea05b5262f1ea17d442) )
	ROM_LOAD( "R2.2B",     0x2600, 0x0200, CRC(b97e35a3) SHA1(0878a83c7f9f0645749fdfb1ff372d0e04833c9e) )

	
	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "C2.1F",     0x0000, 0x0200, CRC(9feca88a) SHA1(14a8c46eb51eed01b7b537a9931cd092cec2019f) )
	ROM_LOAD( "C2.1G",     0x0200, 0x0200, CRC(79fc3963) SHA1(25651d1031895a01a2a4751b355ff1200a899ac5) )
	ROM_LOAD( "C2.1H",     0x0400, 0x0200, CRC(936171e4) SHA1(d0756b49bfd5d58a79f735d4a98a99cce7604b0e) )
	ROM_LOAD( "C2.2F",     0x0600, 0x0200, CRC(3bb16f55) SHA1(b1cc1e2346acd0e5c84861b414b4677871079844) )
	ROM_LOAD( "C2.2G",     0x0800, 0x0200, CRC(19828c47) SHA1(f215ce55be32b3564e1b7cc19500d38a93117051) )
	ROM_LOAD( "C2.2H",     0x0a00, 0x0200, CRC(284ae4eb) SHA1(6e28fcd9d481d37f47728f22f6048b29266f4346) )
	
	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "93427.bin",     0x0000, 0x0100, CRC(f59c8573) SHA1(5aed4866abe1690fd0f088af1cfd99b3c85afe9a) )
ROM_END

//ROM_START( fi8275 )
ROM_START( fi6845 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "R1.1A",     0x0000, 0x0200, CRC(fef96dfe) SHA1(f6df0cf6b5d90ea07ee890985c8cbf0f68e08550) )
	ROM_LOAD( "R1.1B",     0x0200, 0x0200, CRC(c48c6ebc) SHA1(f1f86839819b6abce9ff55c1b02bbf2c4036c51a) )
	ROM_LOAD( "R1.2A",     0x0400, 0x0200, CRC(626a740c) SHA1(3a1df1d71acc207b1b952ad5176804fce27ea97e) )
	ROM_LOAD( "R1.2B",     0x0600, 0x0200, CRC(fbe9782e) SHA1(3661bd03e029e4d2092d259f38a7dec9e763761c) )
	ROM_LOAD( "R1.3A",     0x0800, 0x0200, CRC(6e10de0d) SHA1(8d937f6f2fe1a79b62e6e75536889416ec0071e3) )
	ROM_LOAD( "R1.3B",     0x0a00, 0x0200, CRC(ee1bac50) SHA1(f723b2d1c2a1194aa2df67d48f2b669f5076d857) )
	ROM_LOAD( "R1.4A",     0x0c00, 0x0200, CRC(7faff8f1) SHA1(9275123d6513ab917506f6e9d929935ed1bef429) )
	ROM_LOAD( "R1.4B",     0x0e00, 0x0200, CRC(205ca0c1) SHA1(edf68e9c75523e1b6a485b27af60592fdfb78e04) )
	ROM_LOAD( "R1.5A",     0x1000, 0x0200, CRC(9ada6666) SHA1(f965a08c75fa87e8e3fd7595dcd98231e976e072) )
	ROM_LOAD( "R1.5B",     0x1200, 0x0200, CRC(0f617215) SHA1(b342c783335ab26c036ae77f63a2e932a590c2fa) )
	ROM_LOAD( "R1.6A",     0x1400, 0x0200, CRC(75ea69ae) SHA1(edd9bf686c169ca64373ea87ba92fab4e8c6ee4d) )
	//ROM_LOAD( "R1.6B",   0x1600, 0x0200, CRC(11111111) SHA1(1111111111111111111111111111111111111111) ) // not populated
	ROM_LOAD( "R1.7A",     0x1800, 0x0200, CRC(6e12538f) SHA1(aa08a2db2e5570b431afc967ea5fd749c4f82e33) )
	ROM_LOAD( "R1.7B",     0x1a00, 0x0200, CRC(7270d194) SHA1(7cef9c420c3c3cbc5846bd22137213a78506a8d3) )

	ROM_REGION( 0x0c00, "gfx1", 0 )
	ROM_LOAD( "C1.1B",     0x0000, 0x0200, CRC(9feca88a) SHA1(14a8c46eb51eed01b7b537a9931cd092cec2019f) )
	ROM_LOAD( "C1.2B",     0x0200, 0x0200, CRC(79fc3963) SHA1(25651d1031895a01a2a4751b355ff1200a899ac5) )
	ROM_LOAD( "C1.3B",     0x0400, 0x0200, CRC(936171e4) SHA1(d0756b49bfd5d58a79f735d4a98a99cce7604b0e) )
	ROM_LOAD( "C1.1A",     0x0600, 0x0200, CRC(3bb16f55) SHA1(b1cc1e2346acd0e5c84861b414b4677871079844) )
	ROM_LOAD( "C1.2A",     0x0800, 0x0200, CRC(19828c47) SHA1(f215ce55be32b3564e1b7cc19500d38a93117051) )
	ROM_LOAD( "C1.3A",     0x0a00, 0x0200, CRC(284ae4eb) SHA1(6e28fcd9d481d37f47728f22f6048b29266f4346) )
	
	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "93427.bin",     0x0000, 0x0100, CRC(f59c8573) SHA1(5aed4866abe1690fd0f088af1cfd99b3c85afe9a) )
ROM_END

/*	 YEAR  NAME    PARENT	MACHINE				INPUT	    STATE          INIT    ROT     COMPANY     			FULLNAME    								FLAGS*/
GAME( 1979, fi6845, 0,      fastinvaders_6845, fastinvaders, fastinvaders_state, fi6845, ROT270, "Fiberglass", "Fast Invaders (6845 version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1979, fi8275, fi6845, fastinvaders_8275, fastinvaders, fastinvaders_state,fi6845, 	ROT270, "Fiberglass", "Fast Invaders (8275 version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )