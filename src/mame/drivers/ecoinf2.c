// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Electrocoin 'OXO' hardware type (Phoenix?)

 at least some of these are multiple part cabs, with both top and bottom units all linked together
 see the 'Top Box Roms' in some of the sets.

 This HW seems similar, but not identical to the Pyramid HW in ecoinf3.c

*/


#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "machine/steppers.h" // stepper motor
#include "machine/meters.h"
#include "video/awpvid.h" // drawing reels
#include "ecoinf2.lh"

class ecoinf2_state : public driver_device
{
public:
	ecoinf2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_reel0(*this, "reel0"),
		m_reel1(*this, "reel1"),
		m_reel2(*this, "reel2"),
		m_reel3(*this, "reel3"),
		m_coins(*this, "COINS"),
		m_key(*this, "PERKEY"),
		m_panel(*this, "PANEL")
	{
		strobe_amount = 0;
		strobe_addr = 0;
	}

	required_device<cpu_device> m_maincpu;
	required_device<stepper_device> m_reel0;
	required_device<stepper_device> m_reel1;
	required_device<stepper_device> m_reel2;
	required_device<stepper_device> m_reel3;
	required_ioport m_coins;
	required_ioport m_key;
	required_ioport m_panel;

	UINT16 m_lamps[16];
	UINT16 m_leds[16];
	//UINT16 m_chars[14];
//  void update_display();
	int m_optic_pattern;
	DECLARE_WRITE_LINE_MEMBER(reel0_optic_cb) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	DECLARE_WRITE_LINE_MEMBER(reel1_optic_cb) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	DECLARE_WRITE_LINE_MEMBER(reel2_optic_cb) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	DECLARE_WRITE_LINE_MEMBER(reel3_optic_cb) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }
	int strobe_addr;
	int strobe_amount;

	DECLARE_WRITE8_MEMBER(ox_port5c_out_w);
	DECLARE_DRIVER_INIT(ecoinf2);

		void update_lamps(void)
	{
		for (int i=0; i<16; i++)
		{
			for (int bit=0;bit<16;bit++)
			{
				int data = ((m_lamps[i] << bit)&0x8000)>>15;

				output_set_indexed_value("lamp", (i*16)+bit, data );
			}
		}
	}
	void update_leds(void)
	{
		for (int i=0; i<16; i++)
		{
			for (int bit=0;bit<16;bit++)
			{
				int data = ((m_leds[i] << bit)&0x8000)>>15;

				output_set_digit_value((i*16)+bit, data );
			}
		}
	}

	DECLARE_WRITE8_MEMBER(ppi8255_ic10_write_a_strobedat0)
	{
		if (strobe_amount)
		{
			m_lamps[strobe_addr] = (m_lamps[strobe_addr] &0xff00) | (data & 0x00ff);
			strobe_amount--;
		}
	}
	DECLARE_WRITE8_MEMBER(ppi8255_ic10_write_b_strobedat1)
	{
		if (strobe_amount)
		{
			m_lamps[strobe_addr] = (m_lamps[strobe_addr] &0x00ff) | (data << 8);
			strobe_amount--;
		}
	}
	DECLARE_WRITE8_MEMBER(ppi8255_ic10_write_c_strobe)
	{
//      if (data>=0xf0)
		{
			strobe_addr = data & 0xf;

			// hack, it writes values for the lamps, then writes 0x00 afterwards, probably giving the bulbs power, then removing the power
			// before switching the strobe to the next line?
			strobe_amount = 2;

			update_lamps();
			update_leds();
		}
	//  else logerror("%04x - ppi8255_ic10_(used)write_c %02x (UNUSUAL?)\n", m_maincpu->pcbase(), data);
	}


	DECLARE_WRITE8_MEMBER(ppi8255_ic13_write_a_strobedat0)
	{
		if (strobe_amount)
		{
			m_leds[strobe_addr] = (m_leds[strobe_addr] &0xff00) | (data & 0x00ff);
		}
	}
	DECLARE_WRITE8_MEMBER(ppi8255_ic13_write_b_strobedat1)
	{
		if (strobe_amount)
		{
			m_leds[strobe_addr] = (m_leds[strobe_addr] &0x00ff) | (data << 8);
		}
	}

	DECLARE_READ8_MEMBER(ppi8255_ic13_read_c_panel)
	{
		return m_panel->read();
	}


	DECLARE_READ8_MEMBER(ppi8255_ic22_read_a_levels)
	{
		return 0;//m_levels->read();
	}
	DECLARE_READ8_MEMBER(ppi8255_ic22_read_b_coins)
	{
		return m_coins->read();
	}
	DECLARE_READ8_MEMBER(ppi8255_ic22_read_c_misc)
	{
		int combined_meter = MechMtr_GetActivity(0) | MechMtr_GetActivity(1) |
							MechMtr_GetActivity(2) | MechMtr_GetActivity(3) |
							MechMtr_GetActivity(4) | MechMtr_GetActivity(5) |
							MechMtr_GetActivity(6) | MechMtr_GetActivity(7);

		if(combined_meter)
		{
			return 0x20;
		}
		else
		{
			return 0x00;
		}

//      return m_misc->read();
	}



	DECLARE_WRITE8_MEMBER(ppi8255_ic24_write_a_meters)
	{
		int meter;
		for (meter = 0; meter < 8; meter ++)
		{
			MechMtr_update(meter, (data & (1 << meter)));
		}

	}

	DECLARE_WRITE8_MEMBER(ppi8255_ic24_write_b_payouts)
	{
		//TODO: Fix up payout enables - all available bits enable one slide each
		output_set_value("coinlamp0", data&0x40 );
		output_set_value("coinlamp1", data&0x80 );
	}

	DECLARE_WRITE8_MEMBER(ppi8255_ic24_write_c_inhibits)
	{
		coin_lockout_w(machine(), 0, (data & 0x01) );
		coin_lockout_w(machine(), 1, (data & 0x02) );
		coin_lockout_w(machine(), 2, (data & 0x04) );
		coin_lockout_w(machine(), 3, (data & 0x08) );
		coin_lockout_w(machine(), 4, (data & 0x10) );

		//int wdog = (data& 0x80);
	}




	DECLARE_WRITE8_MEMBER(ppi8255_ic23_write_a_reel01)
	{
		m_reel0->update( data    &0x0f);
		m_reel1->update((data>>4)&0x0f);

		awp_draw_reel("reel1", m_reel0);
		awp_draw_reel("reel2", m_reel1);
	}

	DECLARE_WRITE8_MEMBER(ppi8255_ic23_write_b_reel23)
	{
		m_reel2->update( data    &0x0f);
		m_reel3->update((data>>4)&0x0f);

		awp_draw_reel("reel3", m_reel2);
		awp_draw_reel("reel4", m_reel3);
	}

	DECLARE_READ8_MEMBER(ppi8255_ic23_read_c_key)
	{
		int data = m_optic_pattern;
		data |= m_key->read();
		return data;
	}

	DECLARE_MACHINE_START(ecoinf2);

};


WRITE8_MEMBER(ecoinf2_state::ox_port5c_out_w)
{
	// Watchdog?
}

static ADDRESS_MAP_START( oxo_memmap, AS_PROGRAM, 8, ecoinf2_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( oxo_portmap, AS_IO, 8, ecoinf2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_RAM // z180 internal area?

	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE("ic10_lamp", i8255_device, read, write) //*
	AM_RANGE(0x44, 0x47) AM_DEVREADWRITE("ic24_coin", i8255_device, read, write) //*
	AM_RANGE(0x48, 0x4b) AM_DEVREADWRITE("ic22_inpt", i8255_device, read, write) //*
	AM_RANGE(0x4c, 0x4f) AM_DEVREADWRITE("ic23_reel", i8255_device, read, write)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE("ic13_leds", i8255_device, read, write) //*
//  AM_RANGE(0x54, 0x57) AM_DEVREADWRITE("ic25_dips", i8255_device, read, write) // is this an 8255, or a mirrored byte read?


//  AM_RANGE(0x5c, 0x5c) AM_WRITE(ox_port5c_out_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( ecoinf2 )
	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN5) PORT_NAME("200p?")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN1:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN1:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN1:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN1:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN1:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN1:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN1:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN2:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN2:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN2:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN2:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN2:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN2:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN2:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN3:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN3:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN3:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN3:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN3:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN3:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN3:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN4:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN4:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN4:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN4:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN4:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN4:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN4:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN5:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN5:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN5:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN5:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN5:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN5:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN5:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, "IN6:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN6:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN6:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN6:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN6:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN6:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN6:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN6:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN7")
	PORT_DIPNAME( 0x01, 0x01, "IN7:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN7:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN7:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN7:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN7:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN7:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN7:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN7:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("PANEL")
	PORT_DIPNAME( 0x01, 0x01, "IN8:01" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN8:02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN8:04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN8:08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN8:10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN8:20" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN8:40" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN8:80" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("PERKEY")
	PORT_CONFNAME( 0x0F, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 68% (Invalid for UK Games)"  )
	PORT_CONFSETTING(    0x01, "70" )
	PORT_CONFSETTING(    0x02, "72" )
	PORT_CONFSETTING(    0x03, "74" )
	PORT_CONFSETTING(    0x04, "76" )
	PORT_CONFSETTING(    0x05, "78" )
	PORT_CONFSETTING(    0x06, "80" )
	PORT_CONFSETTING(    0x07, "82" )
	PORT_CONFSETTING(    0x08, "84" )
	PORT_CONFSETTING(    0x09, "86" )
	PORT_CONFSETTING(    0x0A, "88" )
	PORT_CONFSETTING(    0x0B, "90" )
	PORT_CONFSETTING(    0x0C, "92" )
	PORT_CONFSETTING(    0x0D, "94" )
	PORT_CONFSETTING(    0x0E, "96" )
	PORT_CONFSETTING(    0x0F, "98" )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL) //reel opto
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SPECIAL) //reel opto
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SPECIAL) //reel opto
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SPECIAL) //reel opto

INPUT_PORTS_END

MACHINE_START_MEMBER(ecoinf2_state,ecoinf2)
{
	MechMtr_config(machine(),8);
}


static MACHINE_CONFIG_START( ecoinf2_oxo, ecoinf2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180,4000000) // some of these hit invalid opcodes with a plain z80, some don't?
	MCFG_CPU_PROGRAM_MAP(oxo_memmap)
	MCFG_CPU_IO_MAP(oxo_portmap)

	MCFG_DEFAULT_LAYOUT(layout_ecoinf2)

	MCFG_MACHINE_START_OVERRIDE(ecoinf2_state, ecoinf2 )

	MCFG_DEVICE_ADD("ic10_lamp", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(ecoinf2_state, ppi8255_ic10_write_a_strobedat0))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(ecoinf2_state, ppi8255_ic10_write_b_strobedat1))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(ecoinf2_state, ppi8255_ic10_write_c_strobe))

	// IC24 is the workhorse of the Phoenix, it seems to handle meters, payslides, coin lamps, inhibits and the watchdog! */
	MCFG_DEVICE_ADD("ic24_coin", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(ecoinf2_state, ppi8255_ic24_write_a_meters))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(ecoinf2_state, ppi8255_ic24_write_b_payouts))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(ecoinf2_state, ppi8255_ic24_write_c_inhibits))

	MCFG_DEVICE_ADD("ic22_inpt", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(ecoinf2_state, ppi8255_ic22_read_a_levels))    // manual says level switches
	MCFG_I8255_IN_PORTB_CB(READ8(ecoinf2_state, ppi8255_ic22_read_b_coins))
	MCFG_I8255_IN_PORTC_CB(READ8(ecoinf2_state, ppi8255_ic22_read_c_misc))  // 0x20 appears to be meter power

	MCFG_DEVICE_ADD("ic23_reel", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(ecoinf2_state, ppi8255_ic23_write_a_reel01))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(ecoinf2_state, ppi8255_ic23_write_b_reel23))
	MCFG_I8255_IN_PORTC_CB(READ8(ecoinf2_state, ppi8255_ic23_read_c_key))   // optos and keys

	MCFG_DEVICE_ADD("ic13_leds", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(ecoinf2_state, ppi8255_ic13_write_a_strobedat0))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(ecoinf2_state, ppi8255_ic13_write_b_strobedat1))
	MCFG_I8255_IN_PORTC_CB(READ8(ecoinf2_state, ppi8255_ic13_read_c_panel))

	MCFG_ECOIN_200STEP_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinf2_state, reel0_optic_cb))
	MCFG_ECOIN_200STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinf2_state, reel1_optic_cb))
	MCFG_ECOIN_200STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinf2_state, reel2_optic_cb))
	MCFG_ECOIN_200STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(ecoinf2_state, reel3_optic_cb))

//  MCFG_DEVICE_ADD("ic25_dips", I8255, 0)
MACHINE_CONFIG_END



/********************************************************************************************************************
 ROMs for OXO Hw type
********************************************************************************************************************/

ROM_START( ec_oxocg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// all just Z80 roms, no header information the 'TOP' rom is rather different to the rest
	ROM_LOAD( "ocla-4.1", 0x0000, 0x010000, CRC(fe1db86d) SHA1(7718ecafc562bad39cefa15a0df46f081e6045af) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ocla-4.1p", 0x0000, 0x010000, CRC(f24b2cac) SHA1(96f026df3f3915bee89ecc26725e4a7e861fddce) )
	ROM_LOAD( "ocsd-5.2", 0x0000, 0x010000, CRC(28c86aae) SHA1(cafdff7ebc57ef4163b40381e84dd2ac2c24937d) )
	ROM_LOAD( "ocsd-5.3p", 0x0000, 0x010000, CRC(9d422e21) SHA1(9e71ca53054c02c9fb6b23055fa7a5747648bac3) )
	ROM_LOAD( "oxo-btm4.0", 0x0000, 0x010000, CRC(70c8e340) SHA1(4219a493215e2e296a867a3c7ea4cf48356a8842) )
	ROM_LOAD( "oxo-btm4.1p", 0x0000, 0x010000, CRC(b970d6f2) SHA1(df2896bb8e540b67b7427c26f247b0627f6f5f15) )
	ROM_LOAD( "oxo-top4.0", 0x0000, 0x010000, CRC(1b3d8225) SHA1(1951849b3b6966019d5c4c7debef8c5cc6b0259c) )
ROM_END

/*
     ELECTROCOIN  OXO  CLUB

  Oxo-2.3n ---------- 54AE     ?25
  Oxo-2.3p ---------- 55AD    ?25
  Oxo-2-2T.box ----  9976     ?25
  Oxo-nv7.2-3 ------  3E15    ?25

  Oxo-1.6n ---------- EC97      ?5 / ?15
  Oxo-1.8p ---------- 13BD     ?5 / ?15
  Oxo-1-2T.box ---- 9D35      ?5 / ?15
*/

ROM_START( ec_oxocl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// looks like a similar config to set above, the 't' roms being the TOP roms
	ROM_LOAD( "oxo-1.6n", 0x0000, 0x010000, CRC(5c4637c5) SHA1(923a8d50b2b8a7d97d6d1994dafde3aafe0f8c45) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ocn7 v18 non protocol.hex", 0x0000, 0x02680d, CRC(91755ca8) SHA1(38dea02258e4cf731680621c96ebd473e74ae0f6) ) // convert from HEX and check
//  ROM_LOAD( "oxo club.txt", 0x0000, 0x000127, CRC(2ae1750e) SHA1(e15bcc78bcdb4672a77dd46b8f40313dc4a88c59) )
	ROM_LOAD( "oxo-1-2t.box", 0x0000, 0x010000, CRC(8fd03d19) SHA1(b3df92a8a4e0f4b8f813758aa4e881f45a04c8e4) )
	ROM_LOAD( "oxo-1.8p", 0x0000, 0x010000, CRC(26a40f47) SHA1(2c61fa010efc4684e2c53d58a81bd8071246b3f1) )
	ROM_LOAD( "oxo-2-2t.box", 0x0000, 0x010000, CRC(5fac6c82) SHA1(94b9db912fe85dd4bff099492dedd0b2edbec954) )
	ROM_LOAD( "oxo-2.3n", 0x0000, 0x010000, CRC(37bdce39) SHA1(5f38a09a4acfddd63b9fb88eb429390bccec6d9c) )
	ROM_LOAD( "oxo-2.3p", 0x0000, 0x010000, CRC(123e733d) SHA1(41fcb8a15742115ad69d861685f9dffb6242c563) )
	ROM_LOAD( "oxo-nv7.2-3", 0x0000, 0x010000, CRC(7d53520b) SHA1(33af51b9e3ae9f4d923058a79850cb95a141a9a6) )
ROM_END


ROM_START( ec_oxogb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ocla54 non protocol.hex", 0x0000, 0x02680d, CRC(08c18728) SHA1(6cc004db3f7c43b8b7a685becc5de1c84c131048) ) // convert from HEX and check
ROM_END


ROM_START( ec_oxorl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// again same type of thing as ec_oxocg / ec_oxocl
	ROM_LOAD( "oxo-btm4.0", 0x0000, 0x010000, CRC(70c8e340) SHA1(4219a493215e2e296a867a3c7ea4cf48356a8842) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "or25 v4.2 dereg non protocol.hex", 0x0000, 0x02680d, CRC(9a9489f5) SHA1(4587fe7bb0123559930726d9b7197d7a525218f8) ) // convert from HEX and check
	ROM_LOAD( "or25 v4.2 dereg protocol.hex", 0x0000, 0x02680d, CRC(4c3a2b4e) SHA1(e18c8c1b8c2fbc8c84c9632d6fcda76ed8a9303a) ) // convert from HEX and check
	ROM_LOAD( "or5 np.hex", 0x0000, 0x02680d, CRC(15a501eb) SHA1(b66209c02183a222f82a4671962348ae137dc162) ) // convert from HEX and check
	ROM_LOAD( "oxo-btm4.1p", 0x0000, 0x010000, CRC(b970d6f2) SHA1(df2896bb8e540b67b7427c26f247b0627f6f5f15) )
	ROM_LOAD( "oxo-top4.0", 0x0000, 0x010000, CRC(1b3d8225) SHA1(1951849b3b6966019d5c4c7debef8c5cc6b0259c) )
	ROM_LOAD( "oxoreels.2bt", 0x0000, 0x010000, CRC(bfa178ff) SHA1(d433c1f5bc216d76f311566cc80d148fb76eab71) )
	ROM_LOAD( "oxoreels.3dr", 0x0000, 0x010000, CRC(d629133b) SHA1(2a25540885d34bf38528cecd360953818beb6197) )
	ROM_LOAD( "oxoreels.btm", 0x0000, 0x010000, CRC(db408784) SHA1(e53d3419fc6fa04970c7ce52bf7afb9baf022a27) )
	ROM_LOAD( "oxoreels.top", 0x0000, 0x010000, CRC(1b3d8225) SHA1(1951849b3b6966019d5c4c7debef8c5cc6b0259c) )
ROM_END


ROM_START( ec_oxorv )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// again same type of thing as ec_oxocg / ec_oxocl
	ROM_LOAD( "rev-10-0.btm", 0x0000, 0x010000, CRC(dea90334) SHA1(1023e193fa0973e09e8fbbc559935ce5dd32a093) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "nrev 13.0 gala compak.hex", 0x0000, 0x02680d, CRC(1537716f) SHA1(0f9d2cd7387fca7db355fea69bede0b15dcb9c2f) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 gala connexus.hex", 0x0000, 0x02680d, CRC(11eb0066) SHA1(4e836d1a05ba3d7b7ab2fa8e6decc7307daa0b6d) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 non protocol.hex", 0x0000, 0x02680d, CRC(bd2145d5) SHA1(a15cf6081e2b6f4763bf577f31b7b8cc06e8e3de) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 protocol.hex", 0x0000, 0x02680d, CRC(5ae33e51) SHA1(fdabedec9c9adde51fcd3a2ebe000b15c663bcfb) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 rank non protocol.hex", 0x0000, 0x02680d, CRC(35d14c07) SHA1(a7a4a1dc71fe197e97704bcc971893123eb2bc55) ) // convert from HEX and check
	ROM_LOAD( "nrev 13.0 rank protocol.hex", 0x0000, 0x02680d, CRC(e37feebc) SHA1(185dc87b0187b89cc9bc66c8bd8b83217bdff82a) ) // convert from HEX and check
	ROM_LOAD( "rev-10-0.top", 0x0000, 0x010000, CRC(7ed49cd2) SHA1(45fc13d4fbd3d9839ad0c5ac1db391199f1d571e) )
	ROM_LOAD( "rev12-0.top", 0x0000, 0x010000, CRC(029b2036) SHA1(f94409de013d189074d1f64f80d211c888413c28) )
	ROM_LOAD( "rev13-0.bin", 0x0000, 0x010000, CRC(90741b8d) SHA1(5496e6e79efae6a657524b5ce050cae9ccbdd981) )
	ROM_LOAD( "rev13-0p.bin", 0x0000, 0x010000, CRC(9fafd48c) SHA1(f34130233e68fe84e5d4941619a93ebbb6c4f900) )
	ROM_LOAD( "revo120 top.hex", 0x0000, 0x02680d, CRC(0b578ff6) SHA1(956e5ce9fe91d28043fbcff83163663f5aa71909) )
	ROM_LOAD( "revo2-1.btm", 0x0000, 0x010000, CRC(5d30662f) SHA1(f808c925732c5802ba377034d88c3840cae11cb0) )
	ROM_LOAD( "revo2-1p.btm", 0x0000, 0x010000, CRC(52eba92e) SHA1(5223e69d5c9fa7b8819e7a0267c25fa79c020c64) )
ROM_END


ROM_START( ec_suprl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	// again same type of thing as ec_oxocg / ec_oxocl with the top / bottom roms
	ROM_LOAD( "srv11.btm", 0x0000, 0x010000, CRC(e68b5a8a) SHA1(b9a1b76f93ab62b5c5d8d56a1210e2d8194bb5b6) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sr0520p.0 non protocol.hex", 0x0000, 0x02680d, CRC(864baa72) SHA1(3212dd51b5fe98b9c0b16f8285397c3d68ca4fd4) ) // convert from HEX and check
	ROM_LOAD( "sr0520p.0 protocol.hex", 0x0000, 0x02680d, CRC(afbbbef4) SHA1(a060db1b8d648b8890ed68f0cf9934b64abdb9fa) ) // convert from HEX and check
	ROM_LOAD( "sr05b1.8hex", 0x0000, 0x02680d, CRC(12fca690) SHA1(8408159ff7b4a5db6db5fcb08ae636a7e6a1a9b8) ) // convert from HEX and check
	ROM_LOAD( "sr25b16.hex", 0x0000, 0x02680d, CRC(87c33f5f) SHA1(f1ff058b8f670503f73b1fddb5a58becd671294b) ) // convert from HEX and check
	ROM_LOAD( "srle v1.0 protocol.hex", 0x0000, 0x02680d, CRC(57bec009) SHA1(ebf99f6ca5f20e9a30ba694cb3d17f6c8b5827f5) ) // convert from HEX and check
	ROM_LOAD( "srt30.hex", 0x0000, 0x02680d, CRC(d6b970fa) SHA1(d31cc4ae7a920b73f2b377d4e36be56422bc3632) ) // convert from HEX and check


	ROM_LOAD( "srv11.top", 0x0000, 0x010000, CRC(05712727) SHA1(b2e29faa7babe560ba928870e96afa3893ba8955) )
	ROM_LOAD( "srv3-0.btm", 0x0000, 0x010000, CRC(d629133b) SHA1(2a25540885d34bf38528cecd360953818beb6197) )
	ROM_LOAD( "srv3-0.top", 0x0000, 0x010000, CRC(05712727) SHA1(b2e29faa7babe560ba928870e96afa3893ba8955) )

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "supersnd.hex", 0x0000, 0x26812e, CRC(90d96c92) SHA1(18d73c1dc9fe6c26ff832d024ddb9824ddeacf90) )
	ROM_LOAD( "srv3-0.snd", 0x0000, 0x100000, CRC(c40e0609) SHA1(00a2fe56786517b7fa3338918cb8a3bb226f09d8) )
	ROM_LOAD( "srv11.snd", 0x0000, 0x100000, CRC(cf4d217a) SHA1(28eec63bd0c8bab7524e4e939485d174a6852b10) )
ROM_END

// this has no rom for a top box.. might be missing
ROM_START( ec_rcc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// Just Z80 roms, no identification
	ROM_LOAD( "rcas20p4.5", 0x0000, 0x010000, CRC(54a1ddde) SHA1(e98b6dbf0256324fe1cdddbe4b89958d3d5f1233) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rcas20p4.5d", 0x0000, 0x010000, CRC(b42e2415) SHA1(fcc76977a920b6116c5e9029340aa51abb2ab713) )
	ROM_LOAD( "rcas25p4.5", 0x0000, 0x010000, CRC(0aeb0332) SHA1(1b2f2332ac30736892f72b7771fa0825a95f19ad) )

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "rcas4-5.snd", 0x0000, 0x100000, CRC(8d9403e1) SHA1(8a8da6f99a524646a8c689861a5cd6aafeef700b) )
ROM_END


ROM_START( ec_sumnd )
//This may be misidentified, it looks like Z80 code, and the ROM names suggest multiple boxes
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "smn11v0.bin", 0x0000, 0x010000, CRC(0efd44db) SHA1(e99406b04b0f2115141bfdedd1474db7829aeb6d) )
	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "reelok.h", 0x0000, 0x010000, CRC(0fe094b7) SHA1(3c32d7b3423a57bfd7d971d24cd1a2101c87effa) )
	ROM_LOAD( "smn1v0t.bin", 0x0000, 0x010000, CRC(a43c6bda) SHA1(6cae63e9c60d3d9ea4db288af306198b94db3a5d) )
	ROM_LOAD( "smn1v1.bin", 0x0000, 0x010000, CRC(b0530966) SHA1(bbe15702595f19e9c736639ce119638d60c3f483) )
	ROM_LOAD( "smn1v2a.bin", 0x0000, 0x010000, CRC(ae16f6ea) SHA1(1bab5cc61d7cace24be0f1fb9de0182c76e95560) )
	ROM_LOAD( "smn1v2c.bin", 0x0000, 0x010000, CRC(a2157f3c) SHA1(7b71026aea1e12952afe4196799816cf2caf93cc) )
	ROM_LOAD( "smn1v2t.bin", 0x0000, 0x010000, CRC(6a9414e4) SHA1(65145fa1f838437a60d0c03c203adfc094563430) )
	ROM_LOAD( "smn1v3a.bin", 0x0000, 0x010000, CRC(ea5bc6d9) SHA1(b2f19c157ca2c7db66f0828a5ae8b82336ea2077) )
	ROM_LOAD( "smn1_2v3.bin", 0x0000, 0x010000, CRC(05a9b753) SHA1(4b712392428720e8d7d4d1b6f24f2eb674d8540a) )
	ROM_LOAD( "smn2v0.bin", 0x0000, 0x010000, CRC(fc6319b1) SHA1(b61a9fe970662fb12df6398b5b9e11ff40bcf5ea) )
	ROM_LOAD( "smn2v0a.bin", 0x0000, 0x010000, CRC(b7bfc7f9) SHA1(be1567793e4e6201765a8c1e90b77c251cc374b5) )
	ROM_LOAD( "smn2v0c.bin", 0x0000, 0x010000, CRC(9247f887) SHA1(508ea90efdf0d482f9244217b2688f286ffafad4) )
	ROM_LOAD( "smn2v3.bin", 0x0000, 0x010000, CRC(7a9091c5) SHA1(90394549f5f7b33bfb79433355b36700aa63c039) )
	ROM_LOAD( "smn30.bin", 0x0000, 0x010000, CRC(4d593f2b) SHA1(5ac560b45a0c13d90f310ef0651bd895e769163d) )
	ROM_LOAD( "smn3v0ch.bin", 0x0000, 0x010000, CRC(d803a179) SHA1(9e217dedf4cd1a6de6ba6631505f7faf79811cad) )
	ROM_LOAD( "smn3v0cl.bin", 0x0000, 0x010000, CRC(518e66f1) SHA1(a48940fa64f614977f6089d9f40a9d512adb3646) )
	ROM_LOAD( "smn3v2ch.bin", 0x0000, 0x010000, CRC(490adc0e) SHA1(294a45e4aa9405ffac71c819fd86fac21933fde7) )
	ROM_LOAD( "smn3v2cl.bin", 0x0000, 0x010000, CRC(ac22520b) SHA1(09170f56587b68a869bd1e92b5f7815fa38e1d63) )
	ROM_LOAD( "smn3v2gh.bin", 0x0000, 0x010000, CRC(2288820d) SHA1(179987585babe0bb7833b5f8ee9529abf4227662) )
	ROM_LOAD( "smn3v2nh.bin", 0x0000, 0x010000, CRC(43f49af5) SHA1(9d6965b91c3e7775a6219c6638c7b60615f9fb97) )
	ROM_LOAD( "smn3v2nl.bin", 0x0000, 0x010000, CRC(2a3d012c) SHA1(13fc47f623d8878120ae5da09700149d170d6fa3) )
	ROM_LOAD( "smn3v2ghc.bin", 0x0000, 0x010000, CRC(7cd53bbe) SHA1(1e21eec98c4af93636e13e1542e868e3fa8a14a3) )
	ROM_LOAD( "smn3v2glc.bin", 0x0000, 0x010000, CRC(99fdb5bb) SHA1(547f37a3df0477dfad2a47050946cd09c874fd98) )
	ROM_LOAD( "smn3v2wh.bin", 0x0000, 0x010000, CRC(ad8ec7e6) SHA1(0d9d7905e93df94c6884c3f28f475155eec3d3d7) )
	ROM_LOAD( "smn3v2wl.bin", 0x0000, 0x010000, CRC(48a649e3) SHA1(c292bccb4e5be6f11ff6d48ec3f0d178a3ec2b01) )
	ROM_LOAD( "smn3v3ch.bin", 0x0000, 0x010000, CRC(badd6d54) SHA1(2a2884223eab364e12857bc44c66c5e23a0a6e9c) )
	ROM_LOAD( "smn3v3cl.bin", 0x0000, 0x010000, CRC(21df3d6b) SHA1(ba4e69281811d04a9f718cd4cf93fec80e3cda68) )
	ROM_LOAD( "smn3v3gh.bin", 0x0000, 0x010000, CRC(654ab282) SHA1(4c38d6ff89064fcf701ba7f0efed20f1dd0b06e8) )
	ROM_LOAD( "smn3v3gl.bin", 0x0000, 0x010000, CRC(fe48e2bd) SHA1(e143e4de50afaeefd8db47932e73c030df1c8356) )
	ROM_LOAD( "smn3v3ih.bin", 0x0000, 0x010000, CRC(3c26ff83) SHA1(01a725a06b46395e6cded68ab5c5cd45b9e00399) )
	//smn3v3il.bin identical
	ROM_LOAD( "smn3v3nh.bin", 0x0000, 0x010000, CRC(0df5e68a) SHA1(d4a5b6cde48a52065304c8f8381a19525f764a26) )
	//smn3v3nl.bin identical
	ROM_LOAD( "smn3v3wh.bin", 0x0000, 0x010000, CRC(1d7dd629) SHA1(69071d44b3eb2502e043a46d54a61e30136cec22) )
	ROM_LOAD( "smn3v3wl.bin", 0x0000, 0x010000, CRC(867f8616) SHA1(5a271274f097b1e6109cf700266d49dbe8269c5e) )
	ROM_LOAD( "smn3v4ch.bin", 0x0000, 0x010000, CRC(b8355c8b) SHA1(f592adec5c6e1a81f16fb2b5bf2a3a29a1f4e913) )
	ROM_LOAD( "smn3v4cl.bin", 0x0000, 0x010000, CRC(af2bd193) SHA1(ba8096e6287c779ef7a5b5d096efd695ff4b8e63) )
	ROM_LOAD( "smn3v4gh.bin", 0x0000, 0x010000, CRC(1f876f04) SHA1(7b880cc948143f198deac6c53190189c993d238e) )
	ROM_LOAD( "smn3v4gl.bin", 0x0000, 0x010000, CRC(0899e21c) SHA1(6134c9d4ab1d4c922c2e9f751f5d6bc404b9ca8a) )
	ROM_LOAD( "smn3v4hh.bin", 0x0000, 0x010000, CRC(f59b5a79) SHA1(d19d0927efdbf5f9710b3171c212cb847861b86e) )
	ROM_LOAD( "smn3v4hl.bin", 0x0000, 0x010000, CRC(e285d761) SHA1(1d5aebebd41d388bc69777610dc3ee449e4a504e) )
	ROM_LOAD( "smn3v4ih.bin", 0x0000, 0x010000, CRC(67d390bb) SHA1(c27d5cf5bb649a2b9b5ec69340bfce6fdc6cc6a4) )
	ROM_LOAD( "smn3v4il.bin", 0x0000, 0x010000, CRC(50a4d344) SHA1(95c8fd09234f33cfd7787e35482fe33a1ccc4c1f) )
	ROM_LOAD( "smn3v4nh.bin", 0x0000, 0x010000, CRC(fee55c6d) SHA1(308ec20acb5100db79c4a4ff3d06cf4eca26d944) )
	ROM_LOAD( "smn3v4nl.bin", 0x0000, 0x010000, CRC(c9921f92) SHA1(7903264df3a9abb05ce38f39a95d917b22a584d1) )
	ROM_LOAD( "smn3v4wh.bin", 0x0000, 0x010000, CRC(f6f8b1e5) SHA1(351318ac23f1ce89297b1525683271e05447d44b) )
	ROM_LOAD( "smn3v4wl.bin", 0x0000, 0x010000, CRC(e1e63cfd) SHA1(1e966758eb890eb8515bd943e7f8077e2948e22c) )
	ROM_LOAD( "smn3v5th", 0x0000, 0x010000, CRC(355b1784) SHA1(2341b973071d415353597e670b40b65b08a31a08) )
	ROM_LOAD( "smn3v5tl", 0x0000, 0x010000, CRC(d6e144eb) SHA1(f4e69662cbe95eba82b6f35fa298660aa605c9e4) )
	ROM_LOAD( "smn3v6hi.bin", 0x0000, 0x010000, CRC(3f31c2b2) SHA1(e0937800e7e964d48859a7fac8b5e918833384a3) )
	ROM_LOAD( "smn3v6lo.bin", 0x0000, 0x010000, CRC(918b69b4) SHA1(04e5a30dbbdec4a38d8a3466271df6f46e9365dd) )
	ROM_LOAD( "smn3v6lok.bin", 0x0000, 0x010000, CRC(349206f7) SHA1(ab0858f84ef2eb8b229c008322c304e0763bf91b) )
	ROM_LOAD( "smn3v7ch.bin", 0x0000, 0x010000, CRC(61738063) SHA1(ecdc3856b01b82a38a4911380a8a298ba4eedfad) )
	ROM_LOAD( "smn3v7cl.bin", 0x0000, 0x010000, CRC(ab8c5750) SHA1(9068589b702145f779f4bbacabf4a1ad35e679e9) )
	ROM_LOAD( "smn3v7hh.bin", 0x0000, 0x010000, CRC(a15760bf) SHA1(c4421e108c40714766adb532626997ca44d63b93) )
	ROM_LOAD( "smn3v7hl.bin", 0x0000, 0x010000, CRC(6ba8b78c) SHA1(2151f716a86400f76c91138f78eae757bdc832af) )
	ROM_LOAD( "smn3v7nh.bin", 0x0000, 0x010000, CRC(1190d0ef) SHA1(4a6a413581f26b8f190be1be3dac6371e1d585ed) )
	ROM_LOAD( "smn3v7nl.bin", 0x0000, 0x010000, CRC(db6f07dc) SHA1(76c2930db2671658062999e92de9cf953a9cc3d3) )
	ROM_LOAD( "smn3v7th.bin", 0x0000, 0x010000, CRC(d2645c84) SHA1(e183b8fa3c02fa39c71bcfca9f346de4abb8f0c1) )
	ROM_LOAD( "smn3v7tl.bin", 0x0000, 0x010000, CRC(507d75b8) SHA1(85e1bc727d009ffee04e97dfc7190168bd67b252) )
	ROM_LOAD( "smn40.bin", 0x0000, 0x010000, CRC(0fe094b7) SHA1(3c32d7b3423a57bfd7d971d24cd1a2101c87effa) )
	ROM_LOAD( "smn52hi.bin", 0x0000, 0x010000, CRC(6fef9ef1) SHA1(162e20658864da55b40a2a6075d0ab48dc649973) )
	ROM_LOAD( "smn52lo.bin", 0x0000, 0x010000, CRC(7a9d5818) SHA1(c4c56e9dd71ef9080a8fecdd50648260b93b256c) )
	ROM_LOAD( "smn60lo.h", 0x0000, 0x010000, CRC(e0eae706) SHA1(b68b22cd43d7a96195524fb4f73e9182e8b1418d) )
	ROM_LOAD( "smn60lo.t", 0x0000, 0x010000, CRC(2987cb8f) SHA1(23b903f8046939790f78de8650af7208a0dd6cfc) )
	ROM_LOAD( "smn60t", 0x0000, 0x010000, CRC(2e147a73) SHA1(da619206396e898d4f5a75e16994821d050602e0) )
	ROM_LOAD( "smn61lo.t", 0x0000, 0x010000, CRC(a6412635) SHA1(32c169e8bc85823c4bd4fe716c5a291bd9d64120) )
	ROM_LOAD( "smn63.bin", 0x0000, 0x010000, CRC(90f77ecb) SHA1(3066c5d91257280bb30d83c35339d1e5263b84f3) )
	ROM_LOAD( "smn67.bin", 0x0000, 0x010000, CRC(0fdf3530) SHA1(9336edc2b8f61f9c9c93fd839a1397b00cb1255e) )
	ROM_LOAD( "smn68.bin", 0x0000, 0x010000, CRC(68212a60) SHA1(8b1feefcaabb2a799a1841f8e9b4a8bec7244d34) )
	ROM_LOAD( "smn72.bin", 0x0000, 0x010000, CRC(69953657) SHA1(6e7ed86a40e12bfdba41276c1ea0f1532c2f9586) )
	ROM_LOAD( "smn74.bin", 0x0000, 0x010000, CRC(2cbc46d4) SHA1(0bc712ca0975901f9493b134a22bcc1e9d01fc8f) )
	ROM_LOAD( "smn74.fp", 0x0000, 0x010000, CRC(2a102b0d) SHA1(188343a023768c1c15497a643916fa81966d1de9) )
	ROM_LOAD( "smn75.bin", 0x0000, 0x010000, CRC(d9aef9f7) SHA1(51d39860a80da914e013e429adf71e5b17c84b5d) )
	ROM_LOAD( "smn80.bin", 0x0000, 0x010000, CRC(c7ed4a1d) SHA1(8433771bfa23e54410f822c9a9d61c922ae25539) )
	ROM_LOAD( "smn9.1l", 0x0000, 0x010000, CRC(fc6319b1) SHA1(b61a9fe970662fb12df6398b5b9e11ff40bcf5ea) )
	ROM_LOAD( "supermultinudge3v6.bin", 0x0000, 0x010000, CRC(b293d582) SHA1(1294ddca75d95da15cb07e33095d76503494b86e) )
ROM_END

ROM_START( ec_sumnc )
//As above, but a casino version. Doesn't share any ROMs though (?)
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "casino_smn50.bin", 0x0000, 0x010000, CRC(3f54fcd5) SHA1(24bbdb9b1878d2110c02c84ff4b6440e6f4dae0d) )
	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "cassmnv5", 0x0000, 0x010000, CRC(2cae60b1) SHA1(add4e126dc1542035968a0b54e2d172ad514f93c) )
	ROM_LOAD( "lesdes10.bin", 0x0000, 0x010000, CRC(39060e53) SHA1(4561b651ea851e779473fd42af76afce703b7e16) )
	ROM_LOAD( "lesdes51.bin", 0x0000, 0x010000, CRC(3d0d8539) SHA1(382bec50cd79ed6e157e28ac9e738a99d80d14f8) )
	ROM_LOAD( "smn12v0.bin", 0x0000, 0x010000, CRC(9e922bd0) SHA1(cdaeea14ace481bf65b8f8478c9ff2c49a83df87) )
	ROM_LOAD( "smn13ld.bin", 0x0000, 0x010000, CRC(28a258e2) SHA1(5344827e498b812411787b14d884e217f12847fd) )
	ROM_LOAD( "smn13v0.bin", 0x0000, 0x010000, CRC(9e922bd0) SHA1(cdaeea14ace481bf65b8f8478c9ff2c49a83df87) )
	ROM_LOAD( "smn5.2", 0x0000, 0x010000, CRC(f7f599ac) SHA1(a8ed0d88c26edeb353d7894fed41a8ea81851be3) )
	ROM_LOAD( "smn51.bin", 0x0000, 0x010000, CRC(53bd1f6e) SHA1(1709fc9d93611cd25a3ca7def2f412326ab06f99) )
	ROM_LOAD( "smn52.bin", 0x0000, 0x010000, CRC(7b97a88e) SHA1(8155608610f7d36c78802bb2ca39d34d7c8bffe7) )
	ROM_LOAD( "smn93.bin", 0x0000, 0x010000, CRC(c2992a34) SHA1(817f579b553d6be8cfc16e85feb2f6ac174ad22d) )
	ROM_LOAD( "smn94.bin", 0x0000, 0x010000, CRC(9ade016a) SHA1(1c75dc46436253c4e6730f40523d016098c20683) )
	ROM_LOAD( "smncscst", 0x0000, 0x010000, CRC(1147531a) SHA1(c303187452afdcb79e0f182d26d2c27693f69d76) )
ROM_END
DRIVER_INIT_MEMBER(ecoinf2_state,ecoinf2)
{
}

// OXO wh type (Phoenix?) (watchdog on port 5c?)
GAME( 19??, ec_oxocg,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Electrocoin", "Oxo Classic Gold (Electrocoin) (?)"      , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_oxocl,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Electrocoin", "Oxo Club (Electrocoin) (?)"     , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_oxogb,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Electrocoin", "Oxo Golden Bars (Electrocoin) (?)"       , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_oxorl,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Electrocoin", "Oxo Reels (Electrocoin) (?)"     , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_oxorv,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Electrocoin", "Oxo Revolution (Electrocoin) (?)"        , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_suprl,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Electrocoin", "Super Reels (Electrocoin) (?)"       , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_rcc,     0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Electrocoin", "Royal Casino Club (Electrocoin) (?)"     , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)

GAME( 19??, ec_sumnd,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Concept Games Ltd", "Super Multi Nudger (Concept / Electrocoin Oxo) (?)"        , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
GAME( 19??, ec_sumnc,   0        , ecoinf2_oxo,   ecoinf2, ecoinf2_state,   ecoinf2,    ROT0,  "Concept Games Ltd", "Casino Super Multi Nudger (Concept / Electrocoin Oxo) (?)"     , MACHINE_NO_SOUND|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING|MACHINE_MECHANICAL)
