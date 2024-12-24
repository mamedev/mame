// license:BSD-3-Clause
// copyright-holders:smf,Stephane Humbert
/***************************************************************************

Ace Video Fruit Machine hardware
(c)1981-1982 ACE Leisure

Driver by SMF & Guddler 04/02/2007
Inputs and Dip Switches by Stephh

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "screen.h"

#include "sidewndr.lh"


namespace {

class acefruit_state : public driver_device
{
public:
	acefruit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_port_payout(*this, "PAYOUT"),
		m_port_coinage(*this, "COINAGE"),
		m_lamps(*this, "lamp%u", 0U),
		m_solenoids(*this, "solenoid%u", 0U),
		m_refresh_timer(nullptr)
	{ }

	void acefruit(machine_config &config);

	void init_sidewndr();

	template <int Mask> int sidewndr_payout_r();
	template <int Mask> int starspnr_coinage_r();
	template <int Mask> int starspnr_payout_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(refresh_tick);

private:
	void colorram_w(offs_t offset, uint8_t data);
	void coin_w(uint8_t data);
	void sound_w(uint8_t data);
	void lamp_w(offs_t offset, uint8_t data);
	void solenoid_w(uint8_t data);

	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank);
	void update_irq(int vpos);

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_ioport m_port_payout;
	optional_ioport m_port_coinage;
	output_finder<16> m_lamps;
	output_finder<8> m_solenoids;
	emu_timer *m_refresh_timer;
};



void acefruit_state::update_irq(int vpos)
{
	int row = vpos / 8;

	for( int col = 0; col < 32; col++ )
	{
		int tile_index = ( col * 32 ) + row;
		int color = m_colorram[ tile_index ];

		switch( color )
		{
		case 0x0c:
			m_maincpu->set_input_line(0, HOLD_LINE);
			break;
		}
	}
}

TIMER_CALLBACK_MEMBER(acefruit_state::refresh_tick)
{
	int vpos = m_screen->vpos();

	m_screen->update_partial(vpos);
	update_irq(vpos);

	vpos = ((vpos / 8) + 1) * 8;

	m_refresh_timer->adjust(m_screen->time_until_pos(vpos));
}

void acefruit_state::machine_start()
{
	m_lamps.resolve();
	m_solenoids.resolve();
}

void acefruit_state::video_start()
{
	m_refresh_timer = timer_alloc(FUNC(acefruit_state::refresh_tick), this);
}

INTERRUPT_GEN_MEMBER(acefruit_state::vblank)
{
	device.execute().set_input_line(0, HOLD_LINE );
	m_refresh_timer->adjust( attotime::zero );
}

uint32_t acefruit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int startrow = cliprect.min_y / 8;
	int endrow = cliprect.max_y / 8;

	for( int row = startrow; row <= endrow; row++ )
	{
		int spriterow = 0;
		int spriteindex = 0;
		int spriteparameter = 0;

		for( int col = 0; col < 32; col++ )
		{
			int tile_index = ( col * 32 ) + row;
			int code = m_videoram[ tile_index ];
			int color = m_colorram[ tile_index ];

			if( color < 0x4 )
			{
				m_gfxdecode->gfx(1)->opaque(bitmap,cliprect, code, color, 0, 0, col * 16, row * 8 );
			}
			else if( color >= 0x5 && color <= 0x7 )
			{
				static const int spriteskip[] = { 1, 2, 4 };
				int spritesize = spriteskip[ color - 5 ];
				gfx_element *gfx =  m_gfxdecode->gfx(0);

				for( int x = 0; x < 16; x++ )
				{
					int sprite = ( m_spriteram[ ( spriteindex / 64 ) % 6 ] & 0xf ) ^ 0xf;
					const uint8_t *gfxdata = gfx->get_data(sprite);

					for( int y = 0; y < 8; y++ )
					{
						uint16_t *dst = &bitmap.pix(y + ( row * 8 ), x + ( col * 16 ) );
						*( dst ) = *( gfxdata + ( ( spriterow + y ) * gfx->rowbytes() ) + ( ( spriteindex % 64 ) >> 1 ) );
					}

					spriteindex += spritesize;
				}
			}
			else
			{
				for( int x = 0; x < 16; x++ )
				{
					for( int y = 0; y < 8; y++ )
					{
						uint16_t *dst = &bitmap.pix(y + ( row * 8 ), x + ( col * 16 ) );
						*( dst ) = 0;
					}
				}

				if( color == 0x8 )
				{
					if( spriteparameter == 0 )
					{
						spriteindex = code & 0xf;
					}
					else
					{
						spriterow = ( ( code >> 0 ) & 0x3 ) * 8;
						spriteindex += ( ( code >> 2 ) & 0x1 ) * 16;
					}

					spriteparameter = !spriteparameter;
				}
				else if( color == 0xc )
				{
					/* irq generated in update_irq() */
				}
			}
		}
	}

	return 0;
}

template <int Mask>
int acefruit_state::sidewndr_payout_r()
{
	if (Mask < 2)
		return BIT(m_port_payout->read(), Mask);

	logerror("sidewndr_payout_r : invalid %02X bit_mask\n",Mask);
	return 0;
}

template <int Mask>
int acefruit_state::starspnr_coinage_r()
{
	if (Mask < 4)
		return BIT(m_port_coinage->read(), Mask);

	logerror("starspnr_coinage_r : invalid %02X bit_mask\n",Mask);
	return 0;
}

template <int Mask>
int acefruit_state::starspnr_payout_r()
{
	if (Mask < 3)
		return BIT(m_port_payout->read(), Mask);

	logerror("starspnr_payout_r : invalid %02X bit_mask\n",Mask);
	return 0;
}

void acefruit_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[ offset ] = data & 0xf;
}

void acefruit_state::coin_w(uint8_t data)
{
	/* TODO: ? */
}

void acefruit_state::sound_w(uint8_t data)
{
	/* TODO: ? */
}

void acefruit_state::lamp_w(offs_t offset, uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_lamps[(offset << 3) | i] = BIT(data, i);
}

void acefruit_state::solenoid_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_solenoids[i] = BIT(data, i);
}

void acefruit_state::palette_init(palette_device &palette) const
{
	/* sprites */
	palette.set_pen_color( 0, rgb_t(0x00, 0x00, 0x00) );
	palette.set_pen_color( 1, rgb_t(0x00, 0x00, 0xff) );
	palette.set_pen_color( 2, rgb_t(0x00, 0xff, 0x00) );
	palette.set_pen_color( 3, rgb_t(0xff, 0x7f, 0x00) );
	palette.set_pen_color( 4, rgb_t(0xff, 0x00, 0x00) );
	palette.set_pen_color( 5, rgb_t(0xff, 0xff, 0x00) );
	palette.set_pen_color( 6, rgb_t(0xff, 0xff, 0xff) );
	palette.set_pen_color( 7, rgb_t(0x7f, 0x3f, 0x1f) );

	/* tiles */
	palette.set_pen_color( 8, rgb_t(0x00, 0x00, 0x00) );
	palette.set_pen_color( 9, rgb_t(0xff, 0xff, 0xff) );
	palette.set_pen_color( 10, rgb_t(0x00, 0x00, 0x00) );
	palette.set_pen_color( 11, rgb_t(0x00, 0x00, 0xff) );
	palette.set_pen_color( 12, rgb_t(0x00, 0x00, 0x00) );
	palette.set_pen_color( 13, rgb_t(0x00, 0xff, 0x00) );
	palette.set_pen_color( 14, rgb_t(0x00, 0x00, 0x00) );
	palette.set_pen_color( 15, rgb_t(0xff, 0x00, 0x00) );
}

void acefruit_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x20ff).ram().share("nvram");
	map(0x4000, 0x43ff).ram().share("videoram");
	map(0x4400, 0x47ff).ram().w(FUNC(acefruit_state::colorram_w)).share("colorram");
	map(0x8000, 0x8000).portr("IN0");
	map(0x8001, 0x8001).portr("IN1");
	map(0x8002, 0x8002).portr("IN2");
	map(0x8003, 0x8003).portr("IN3");
	map(0x8004, 0x8004).portr("IN4");
	map(0x8005, 0x8005).portr("IN5");
	map(0x8006, 0x8006).portr("IN6");
	map(0x8007, 0x8007).portr("IN7");
	map(0x6000, 0x6005).ram().share("spriteram");
	map(0xa000, 0xa001).w(FUNC(acefruit_state::lamp_w));
	map(0xa002, 0xa003).w(FUNC(acefruit_state::coin_w));
	map(0xa004, 0xa004).w(FUNC(acefruit_state::solenoid_w));
	map(0xa005, 0xa006).w(FUNC(acefruit_state::sound_w));
	map(0xc000, 0xc000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xe000, 0xffff).rom();
}

void acefruit_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).noprw(); /* ? */
}

static INPUT_PORTS_START( sidewndr )
	PORT_START("IN0")   // 0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Stop Nudge/Nudge Up or Down" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Gamble" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )              /* "Cash in" */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) /* active low or high?? */
	PORT_BIT( 0xd8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   // 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Sidewind" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )              /* "Cash in" */
	PORT_DIPNAME( 0x08, 0x00, "Accountacy System Texts" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   // 2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Cancel/Clear" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME( "Refill" ) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )              /* "Token in" - also "Refill" when "Refill" mode ON */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::sidewndr_payout_r<0x00>))
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   // 3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME( "Hold/Nudge 1" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME( "Accountancy System" ) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )              /* "50P in" */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::sidewndr_payout_r<0x01>))
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")   // 4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME( "Hold/Nudge 2" )
	PORT_DIPNAME( 0x02, 0x00, "Allow Clear Data" )          /* in "Accountancy System" mode */
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Lamp 11 always ON" )         /* code at 0x173a - write lamp status at 0x01ed */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Sounds" )                    /* data in 0x206b and 0x206c - out sound at 0x193e */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")   // 5
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME( "Hold/Nudge 3" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME( "Test Program" ) PORT_TOGGLE
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")   // 6
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME( "Hold/Nudge 4" )
	/* I don't know exactly what this bit is supposed to do :(
	   I only found that when bit is LOW, no data is updated
	   (check "Accountancy System" mode). And when you switch
	   it from LOW to HIGH, previous saved values are back
	   (check for example the number of credits). */
	PORT_DIPNAME( 0x02, 0x02, "Save Data" )                 /* code at 0x1773 */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")   // 7
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )             /* next in "Accountancy System" mode */
	PORT_DIPNAME( 0x02, 0x00, "Clear Credits on Reset" )    /* also affects rolls */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PAYOUT")    // fake port to handle settings via multiple input ports
	PORT_DIPNAME( 0x03, 0x00, "Payout %" )
	PORT_DIPSETTING(    0x00, "74%" )
	PORT_DIPSETTING(    0x02, "78%" )
	PORT_DIPSETTING(    0x01, "82%" )
	PORT_DIPSETTING(    0x03, "86%" )
INPUT_PORTS_END

static INPUT_PORTS_START( spellbnd )
	PORT_INCLUDE(sidewndr)

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          /* before COIN4 test - code at 0x0994 */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xd0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x08, 0x08, "Accountacy System Texts" )   /* bit test is inverted compared to 'sidewndr' */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Cancel" )          /* see IN4 bit 0 in "Accountancy System" mode */

	PORT_MODIFY("IN4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME( "Clear Data" )     /* in "Accountancy System" mode */
	/* Similar to 'sidewndr' but different addresses */
	PORT_DIPNAME( 0x04, 0x04, "Lamp 11 always ON" )         /* code at 0x072a - write lamp status at 0x00ff */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	/* Similar to 'sidewndr' but different addresses */
	PORT_DIPNAME( 0x08, 0x00, "Sounds" )                    /* data in 0x2088 and 0x2089 - out sound at 0x012d */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN6")
	/* I don't know exactly what this bit is supposed to do :(
	   I only found that when bit is LOW, no data is updated
	   (check "Accountancy System" mode). */
	PORT_DIPNAME( 0x02, 0x02, "Save Data" )                 /* code at 0x0763 (similar to 'sidewndr') and 0x18db */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	PORT_MODIFY("IN7")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          /* code at 0x04a8 */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf4, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* I've only mapped the known inputs after comparison with 'spellbnd' and the ones known to do something */
static INPUT_PORTS_START( starspnr )
	PORT_START("IN0")   // 0
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Gamble" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	/* tested at 0xef77 after IN5 bit 1 and before IN2 bit 2 - after coins are tested - table at 0xefa5 (3 bytes) */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) /* active low or high?? */

	PORT_START("IN1")   // 1
	/* tested at 0xe77c - call from 0x012c */
	/* tested at 0xeffb after IN6 bit 2 - invalid code after 0xf000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect/Cancel" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	/* tested at 0xeed7 with IN1 bit 3 - before coins are tested - table at 0xef55 (4 * 3 bytes) */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::starspnr_coinage_r<0x03>)) /* to be confirmed */

	PORT_START("IN2")   // 2
	/* tested at 0xe83c */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0xe5ab - after "Collect" and "Gamble" buttons */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0xef82 after IN5 bit 1 and after IN1 bit 3 - after coins are tested - table at 0xefa8 (3 bytes) */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0xeeba with IN3 bit 3 - before coins are tested - table at 0xef55 (4 * 3 bytes) */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::starspnr_coinage_r<0x01>)) /* to be confirmed */
	/* tested at 0x1b0f */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   // 3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME( "Hold 1" )
	/* tested at 0xe8ea and 0xecbe */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0xeeba with IN2 bit 3 - before coins are tested - table at 0xef55 (4 * 3 bytes) */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::starspnr_coinage_r<0x00>)) /* to be confirmed */
	/* tested at 0x0178 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")   // 4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME( "Hold 2" )
	/* tested at 0x064e */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0xed86 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0xeed7 with IN1 bit 3 - before coins are tested - table at 0xef55 (4 * 3 bytes) */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::starspnr_coinage_r<0x02>)) /* to be confirmed */

	PORT_START("IN5")   // 5
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME( "Hold 3" )
	/* tested at 0xef68 before IN1 bit 3 and before IN2 bit 2 - after coins are tested - table at 0xefa2 (3 bytes) */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0xec6f */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0x1d60 with IN6 bit 3 and IN7 bit 3 - table at 0x1d90 (8 * 3 bytes) */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::starspnr_payout_r<0x00>)) /* to be confirmed */
	/* tested at 0xe312 and 0xe377 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")   // 6
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME( "Hold 4" )
	/* tested at 0xee42, 0xee5e and 0xeff5 before IN1 bit 0 - invalid code after 0xf000 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0x1d60 with IN5 bit 3 and IN7 bit 3 - table at 0x1d90 (8 * 3 bytes) */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::starspnr_payout_r<0x01>)) /* to be confirmed */
	/* tested at 0xe8dd and 0xec1c */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")   // 7
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x00, "Clear Credits on Reset" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* tested at 0xedcb */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* tested at 0x1d60 with IN5 bit 3 and IN6 bit 3 - table at 0x1d90 (8 * 3 bytes) */
	PORT_BIT( 0x08, 0x00, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(acefruit_state::starspnr_payout_r<0x02>)) /* to be confirmed */
	/* tested at 0xec2a */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINAGE")   // fake port to handle settings via multiple input ports
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/25 Credits" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/25 Credits" )

	PORT_START("PAYOUT")    // fake port to handle settings via multiple input ports
	PORT_DIPNAME( 0x07, 0x07, "Payout %" )
	PORT_DIPSETTING(    0x00, "30%" )
	PORT_DIPSETTING(    0x01, "40%" )
	PORT_DIPSETTING(    0x02, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x07, "80%" )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	16,8, /* 8*8 characters doubled horizontally */
	256, /* 256 characters */
	1, /* 1 bit per pixel */
	{ 0 },
	{ 0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	32,32, /* 32*32 sprites */
	16, /* 16 sprites */
	3, /* 3 bits per pixel */
	/* Offset to the start of each bit */
	{ 0, 256*8*8, 256*8*8*2 },
	/* Offset to the start of each byte */
	{
		0,  1,   2,  3,  4,  5,  6,  7,
		8,  9,  10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23,
		24, 25, 26, 27, 28, 29, 30, 31
	},
	/* Offset to the start of each line */
	{
		0*32,   1*32,  2*32,  3*32,  4*32,  5*32,  6*32,  7*32,
		8*32,   9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32,
		24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32
	},
	/* Offset to next sprite (also happens to be number of bits per sprite) */
	32*32 /* every sprite takes 128 bytes */
};

static GFXDECODE_START( gfx_acefruit )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1800, charlayout, 8, 4 )
GFXDECODE_END

void acefruit_state::acefruit(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2500000); /* 2.5MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &acefruit_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &acefruit_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(acefruit_state::vblank));

	WATCHDOG_TIMER(config, "watchdog");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_acefruit);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(512, 256);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(acefruit_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(acefruit_state::palette_init), 16);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* sound hardware */
}

void acefruit_state::init_sidewndr()
{
	uint8_t *ROM = memregion("maincpu")->base();
	/* replace "ret nc" ( 0xd0 ) with "di" */
	ROM[0] = 0xf3;
	/* this is either a bad dump or the cpu core should set the carry flag on reset */
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sidewndr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2_h09.bin",    0x000000, 0x000800, BAD_DUMP CRC(141f3b0c) SHA1(1704feba950fe7aa939b9ed54c37264d10527d11) )
	ROM_LOAD( "2_h10.bin",    0x000800, 0x000800, CRC(36a2d4af) SHA1(2388e22245497240e5721895d94d2ccd1f579eff) )
	ROM_LOAD( "2_h11.bin",    0x001000, 0x000800, CRC(e2932643) SHA1(e1c0cd5d0cd332519432cbefa8718362a6cd1ccc) )
	ROM_LOAD( "2_h12.bin",    0x001800, 0x000800, CRC(26af0b1f) SHA1(36f0e54982688b9d5a24a6986a847ac69ee0a355) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* 8k for graphics */
	ROM_LOAD( "2_h05.bin",    0x000000, 0x000800, CRC(64b64cff) SHA1(c11f2bd2af68ae7f104b711deb7f6509fdbaeb8f) )
	ROM_LOAD( "2_h06.bin",    0x000800, 0x000800, CRC(6b96a586) SHA1(6d5ab8fefe37ca4dbc5057ebf31f12b33dbdf5c0) )
	ROM_LOAD( "2_h07.bin",    0x001000, 0x000800, CRC(3a8e68a2) SHA1(2ffe07360f57f0f11ecf326f00905747d9b66811) )
	ROM_LOAD( "2_h08.bin",    0x001800, 0x000800, CRC(bd19a758) SHA1(3fa812742f34643f66c67cb9bdb1d4d732c4f44d) )
ROM_END

ROM_START( spellbnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h9.bin",       0x000000, 0x000800, CRC(9919fcfa) SHA1(04167b12ee9e60ef891893a305a35d3f2eccb0bb) )
	ROM_LOAD( "h10.bin",      0x000800, 0x000800, CRC(90502d00) SHA1(3bdd859d9146df2eb97b4517c446182569a55a46) )
	ROM_LOAD( "h11.bin",      0x001000, 0x000800, CRC(7375166c) SHA1(f05b01941423fd36e0a5d3aa913a594e4e7aa5d4) )
	ROM_LOAD( "h12.bin",      0x001800, 0x000800, CRC(4546c68c) SHA1(92104e2005fc772ea9f70451d9d674f95d3f0ba9) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* 8k for graphics */
	ROM_LOAD( "h5.bin",       0x000000, 0x000800, CRC(198da32c) SHA1(bf6c4ddcda0503095d310e08057dd88154952ef4) )
	ROM_LOAD( "h6.bin",       0x000800, 0x000800, CRC(e777130f) SHA1(3421c6f399e5ec749f1908f6b4ebff7761c6c5d9) )
	ROM_LOAD( "h7.bin",       0x001000, 0x000800, CRC(bfed5b8f) SHA1(f95074e8809297eec67da9d7e33ae1dd1c5eabc0) )
	ROM_LOAD( "h8.bin",       0x001800, 0x000800, CRC(05da2b71) SHA1(3a263f605ecc9e4dca9ce0ba815af16e28bf9bc8) )
ROM_END

/*
Starspinner
ACE, 1982?

PCB Layout
----------

|---------------------------------------------------------------------------|
|                                                                           |
|   XTAL    BAT                2114                                         |
|                                                                           |
|                                                   14-1-102          P1    |
|                                                                           |
|                                                                           |
|                                                                           |
|                                                   14-1-102                |
|                                                                           |
|   5       2114                                                            |
|                                                                           |
|   6       2114                                    14-1-102                |
|                                                                           |
|   7                                                                       |
|                                                                           |
|   8       5501    5501                            14-1-102                |
|                                                                           |
|   h9                                              16-1-101                |
|                                                                           |
|   h10                                             16-1-101                |
|                                                                           |
|   h11         Z80                                 16-1-101                |
|                                                                           |
|   h12                                                               P2    |
|                                                                           |
|                                           DSWA    DSWB                    |
|                                                                           |
|---------------------------------------------------------------------------|

Notes:
    Z80  - NEC D780C running at ? MHz (DIP40)
    5501 - Toshiba TC5501P 256 x4 SRAM (DIP22)
    2114 - NEC uPD2114LC 1k x8 DRAM (DIP18)
    XTAL - ? MHz
    BAT  - VARTA Ni-Cd 3.6V 100 mAh
    DSWA - 8-way DIP switch
    DSWB - 8-way DIP switch
    P1   - 4x10 pin connector to power supply
    P2   - 4x10 pin connector to control panel
*/

ROM_START( starspnr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h9.h9",        0x00e000, 0x0800, CRC(083068aa) SHA1(160a5f3bf33d0a53354f98295cd67022762928b6) )
	ROM_CONTINUE(             0x000000, 0x0800 )
	ROM_LOAD( "h10.h10",      0x00e800, 0x0800, CRC(a0a96e55) SHA1(de4dc0da5a1f358085817690cc6bdc8d94a849f8) )
	ROM_CONTINUE(             0x000800, 0x0800 )
	ROM_LOAD( "h11.h11",      0x00f000, 0x0800, BAD_DUMP CRC(ab045396) SHA1(8b3aea0b0d55f62d5b6fbd39664beb93559d2213) ) /* bad dump : invalid code in both halves ! */
	ROM_CONTINUE(             0x001000, 0x0800 )
	ROM_LOAD( "h12.h12",      0x00f800, 0x0800, CRC(8571f3f5) SHA1(e8b60a604a4a0368b6063b15b328c68f351cb740) ) /* bad dump ? nothing of interest 0xf800-0xffff */
	ROM_CONTINUE(             0x001800, 0x0800 )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* 8k for graphics */
	ROM_LOAD( "5.h5",         0x000000, 0x000800, CRC(df49876f) SHA1(68077304f096491baeddc1d6b4dc62f90de71903) )
	ROM_LOAD( "6.h6",         0x000800, 0x000800, CRC(d992e2f6) SHA1(7841efec7d81689c82b8da501cce743436e7e8d4) )
	ROM_LOAD( "7.h7",         0x001000, 0x000800, CRC(d5a40e88) SHA1(5cac8d85123720cdbb8b4630b14a27cf0ceef33f) )
	ROM_LOAD( "8.h8",         0x001800, 0x000800, CRC(0dd38c3c) SHA1(4da0cd00c76d3be2164f141ccd8c72dd9578ee61) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "16-1-101.b9",  0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "16-1-101.b10", 0x0100, 0x0100, NO_DUMP )
	ROM_LOAD( "16-1-101.b11", 0x0200, 0x0100, NO_DUMP )
ROM_END

/* This has been identified as Silhouette with the help of a flyer */
ROM_START( acefruit  )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vp-h9.bin",    0x00e000, 0x0800, CRC(f595daf7) SHA1(be5abd34fd06f73cd80f5b15902d158e33705c8f) )
	ROM_CONTINUE(             0x000000, 0x0800 )
	ROM_LOAD( "vp-h10.bin",   0x00e800, 0x0800, CRC(b0539100) SHA1(763f31f72f55c3322b24e127b37130d37daa5216) )
	ROM_CONTINUE(             0x000800, 0x0800 )
	ROM_LOAD( "vp-h11.bin",   0x00f000, 0x0800, CRC(fa176072) SHA1(18203278bb9c505f07390f7b95ecf9ab6d7b7122) )
	ROM_CONTINUE(             0x001000, 0x0800 )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* 8k for graphics */
	ROM_LOAD( "vp-h5.bin",         0x000000, 0x000800, CRC(dfffe063) SHA1(1b860323fe93b7d010fa35167769555a6bd4a49c) )
	ROM_LOAD( "vp-h6.bin",         0x000800, 0x000800, CRC(355203b8) SHA1(959f3599a24293f392e8b10061c39d3244f34c05) )
	ROM_LOAD( "vp-h7.bin",         0x001000, 0x000800, CRC(7784de8a) SHA1(40851724c9b7ef26964462b5e97ad943df4d56e2) )
	ROM_LOAD( "vp-h8.bin",         0x001800, 0x000800, CRC(d587e541) SHA1(902b6c4673b8b989d034d60d3c47f2499f100ba2) )

	/* there were no proms in the set */
	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "16-1-101.b9",  0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "16-1-101.b10", 0x0100, 0x0100, NO_DUMP )
	ROM_LOAD( "16-1-101.b11", 0x0200, 0x0100, NO_DUMP )
ROM_END

} // anonymous namespace


GAMEL( 1981?, sidewndr, 0, acefruit, sidewndr, acefruit_state, init_sidewndr, ROT270, "ACE", "Sidewinder", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND, layout_sidewndr )
GAMEL( 1981?, spellbnd, 0, acefruit, spellbnd, acefruit_state, empty_init,    ROT270, "ACE", "Spellbound", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND, layout_sidewndr )
GAME(  1982?, starspnr, 0, acefruit, starspnr, acefruit_state, empty_init,    ROT270, "ACE", "Starspinner (Dutch/Nederlands)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME(  1982?, acefruit, 0, acefruit, spellbnd, acefruit_state, empty_init,    ROT270, "ACE", "Silhouette", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // inputs and video in bonus game need fixing on this one
// not dumped: Magnum?
