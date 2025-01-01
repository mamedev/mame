// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Raster Elite Tickee Tickats hardware
     + variations

    driver by Aaron Giles

    Games supported:
        * Tickee Tickats
        * Ghost Hunter
        * Tuts Tomb
        * Mouse Attack
        * Rapid Fire
        * Mallet Madness

    TODO:
        * (Tickee) gun sometimes misfires
        * maletmad some animation seems to run unthrottled (eg. the turtle rum barrels level),
          though rate of game timer and music is ok, suspect tms34010 gfxcycles timing

***************************************************************************/

#include "emu.h"

#include "cpu/tms34010/tms34010.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/tlc34076.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tickee_state : public driver_device
{
public:
	tickee_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_oki(*this, "oki"),
		m_tlc34076(*this, "tlc34076"),
		m_ticket(*this, "ticket%u", 1)
	{
	}

	// Machine drivers
	void mouseatk(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	required_shared_ptr<uint16_t> m_vram;

	required_device<tms34010_device> m_maincpu;
	required_device<screen_device> m_screen;

	optional_device<okim6295_device> m_oki;
	required_device<tlc34076_device> m_tlc34076;
	optional_device_array<ticket_dispenser_device, 2> m_ticket;

	int m_palette_blank = 0;

	// Video update
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);

	// Miscellaneous control bits
	void tickee_control_w(offs_t offset, uint8_t data);

private:
	// Memory maps
	void mouseatk_map(address_map &map) ATTR_COLD;
};


class tickee_gun_state : public tickee_state
{
public:
	tickee_gun_state(const machine_config &mconfig, device_type type, const char *tag) :
		tickee_state(mconfig, type, tag),
		m_gun_axis_x(*this, "GUNX%u", 1U),
		m_gun_axis_y(*this, "GUNY%u", 1U)
	{
	}

	// Machine drivers
	void tickee(machine_config &config);
	void ghoshunt(machine_config &config);
	void rapidfir(machine_config &config);
	void maletmad(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_ioport_array<2> m_gun_axis_x;
	required_ioport_array<2> m_gun_axis_y;

	emu_timer *m_setup_gun_timer;
	emu_timer *m_set_gun_int_timer[2];
	emu_timer *m_clear_gun_int_timer[2];

	int m_beamxadd = 0;
	int m_beamyadd = 0;
	uint8_t m_gunx[2] = { };

	void set_beamadd(int x, int y) { m_beamxadd = x; m_beamyadd = y; }

	// Compute X/Y coordinates
	void get_crosshair_xy(int player, int &x, int &y);

	// Light gun interrupts
	TIMER_CALLBACK_MEMBER(trigger_gun_interrupt);
	TIMER_CALLBACK_MEMBER(clear_gun_interrupt);
	TIMER_CALLBACK_MEMBER(setup_gun_interrupts);

	// Video related
	void rapidfir_transparent_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rapidfir_transparent_r(offs_t offset);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(rapidfir_to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(rapidfir_from_shiftreg);

	// Control
	uint16_t rapidfir_gun1_r();
	uint16_t rapidfir_gun2_r();
	void palette_blank_w(uint8_t data);
	void rapidfir_output_w(uint8_t data);

	// Sound
	void sound_bank_w(uint8_t data);

	// Memory maps
	void tickee_map(address_map &map) ATTR_COLD;
	void ghoshunt_map(address_map &map) ATTR_COLD;
	void rapidfir_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

inline void tickee_gun_state::get_crosshair_xy(int player, int &x, int &y)
{
	const rectangle &visarea = m_screen->visible_area();

	x = (((m_gun_axis_x[player]->read() & 0xff) * visarea.width()) >> 8) + visarea.min_x;
	y = (((m_gun_axis_y[player]->read() & 0xff) * visarea.height()) >> 8) + visarea.min_y;
}



/*************************************
 *
 *  Light gun interrupts
 *
 *************************************/

TIMER_CALLBACK_MEMBER(tickee_gun_state::trigger_gun_interrupt)
{
	int const which = param & 1;
	int const beamx = (m_screen->hpos() / 2) - 58;

	// once we're ready to fire, set the X coordinate and assert the line
	m_gunx[which] = beamx;

	// fire the IRQ at the correct moment
	m_maincpu->set_input_line(param, ASSERT_LINE);
}


TIMER_CALLBACK_MEMBER(tickee_gun_state::clear_gun_interrupt)
{
	// clear the IRQ on the next scanline?
	m_maincpu->set_input_line(param, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(tickee_gun_state::setup_gun_interrupts)
{
	// set a timer to do this again next frame
	m_setup_gun_timer->adjust(m_screen->time_until_pos(0));

	// only do work if the palette is flashed
	if (m_palette_blank)
	{
		int beamx, beamy;

		// generate interrupts for player 1's gun
		get_crosshair_xy(0, beamx, beamy);
		m_set_gun_int_timer[0]->adjust(m_screen->time_until_pos(beamy + m_beamyadd, beamx + m_beamxadd), 0);
		m_clear_gun_int_timer[0]->adjust(m_screen->time_until_pos(beamy + m_beamyadd + 1, beamx + m_beamxadd), 0);

		// generate interrupts for player 2's gun
		get_crosshair_xy(1, beamx, beamy);
		m_set_gun_int_timer[1]->adjust(m_screen->time_until_pos(beamy + m_beamyadd, beamx + m_beamxadd), 1);
		m_clear_gun_int_timer[1]->adjust(m_screen->time_until_pos(beamy + m_beamyadd + 1, beamx + m_beamxadd), 1);
	}
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void tickee_state::machine_start()
{
	save_item(NAME(m_palette_blank));
}

void tickee_gun_state::machine_start()
{
	tickee_state::machine_start();

	// start a timer going on the first scanline of every frame
	m_setup_gun_timer = timer_alloc(FUNC(tickee_gun_state::setup_gun_interrupts), this);

	// initialize gun set/clear interrupts for both players
	m_set_gun_int_timer[0] = timer_alloc(FUNC(tickee_gun_state::trigger_gun_interrupt), this);
	m_set_gun_int_timer[1] = timer_alloc(FUNC(tickee_gun_state::trigger_gun_interrupt), this);
	m_clear_gun_int_timer[0] = timer_alloc(FUNC(tickee_gun_state::clear_gun_interrupt), this);
	m_clear_gun_int_timer[1] = timer_alloc(FUNC(tickee_gun_state::clear_gun_interrupt), this);

	save_item(NAME(m_gunx));
}

void tickee_gun_state::machine_reset()
{
	tickee_state::machine_reset();

	m_setup_gun_timer->adjust(m_screen->time_until_pos(0));

	m_set_gun_int_timer[0]->adjust(attotime::never);
	m_set_gun_int_timer[1]->adjust(attotime::never);
	m_clear_gun_int_timer[0]->adjust(attotime::never);
	m_clear_gun_int_timer[1]->adjust(attotime::never);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(tickee_state::scanline_update)
{
	uint16_t const *const src = &m_vram[(params->rowaddr << 8) & 0x3ff00];
	uint32_t *const dest = &bitmap.pix(scanline);
	pen_t const *const pens = m_tlc34076->pens();
	int coladdr = params->coladdr << 1;

	if (m_palette_blank)
	{
		// blank palette: fill with pen 255
		for (int x = params->heblnk; x < params->hsblnk; x += 2)
		{
			dest[x + 0] = pens[0xff];
			dest[x + 1] = pens[0xff];
		}
	}
	else
	{
		// copy the non-blanked portions of this scanline
		for (int x = params->heblnk; x < params->hsblnk; x += 2)
		{
			uint16_t pixels = src[coladdr++ & 0xff];
			dest[x + 0] = pens[pixels & 0xff];
			dest[x + 1] = pens[pixels >> 8];
		}
	}
}


/*************************************
 *
 *  Video related
 *
 *************************************/

void tickee_gun_state::rapidfir_transparent_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!(data & 0xff00)) mem_mask &= 0x00ff;
	if (!(data & 0x00ff)) mem_mask &= 0xff00;
	COMBINE_DATA(&m_vram[offset]);
}


uint16_t tickee_gun_state::rapidfir_transparent_r(offs_t offset)
{
	return m_vram[offset];
}


TMS340X0_TO_SHIFTREG_CB_MEMBER(tickee_gun_state::rapidfir_to_shiftreg)
{
	if (address < 0x800000)
		memcpy(shiftreg, &m_vram[address >> 4], 0x400);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(tickee_gun_state::rapidfir_from_shiftreg)
{
	if (address < 0x800000)
		memcpy(&m_vram[address >> 4], shiftreg, 0x400);
}



/*************************************
 *
 *  Miscellaneous control bits
 *
 *************************************/

void tickee_state::tickee_control_w(offs_t offset, uint8_t data)
{
	/* offsets:

	    2 = palette flash (0 normally, 1 when trigger is pressed)
	    3 = ticket motor (bit 3 = 0 for left motor, bit 2 = 0 for right motor)
	    6 = lamps? (changing all the time)
	*/

	switch (offset)
	{
		case 2:
			m_palette_blank = data & 1;
			break;

		case 3:
			m_ticket[0]->motor_w(BIT(~data, 3));
			m_ticket[1]->motor_w(BIT(~data, 2));
			break;

		default:
			break;
	}
}


void tickee_gun_state::rapidfir_output_w(uint8_t data)
{
	/* bits:

	    0, 1 = coin counters
	    2, 3 = recoil?
	    4 = ticket motor
	*/

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	if (m_ticket[0])
		m_ticket[0]->motor_w(BIT(data, 4));
}


void tickee_gun_state::palette_blank_w(uint8_t data)
{
	m_palette_blank = data & 1;
}


uint16_t tickee_gun_state::rapidfir_gun1_r()
{
	return m_gunx[0];
}


uint16_t tickee_gun_state::rapidfir_gun2_r()
{
	return m_gunx[1];
}



/*************************************
 *
 *  Sound
 *
 *************************************/

void tickee_gun_state::sound_bank_w(uint8_t data)
{
	switch (data)
	{
		case 0x2c:
			m_oki->set_rom_bank(0);
			break;

		case 0x2d:
			m_oki->set_rom_bank(1);
			break;

		case 0x1c:
			m_oki->set_rom_bank(2);
			break;

		case 0x1d:
			m_oki->set_rom_bank(3);
			break;

		default:
			logerror("sound_bank_w %02X\n", data);
			break;
	}
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

void tickee_gun_state::tickee_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("vram");
	map(0x02000000, 0x02ffffff).rom().region("user1", 0);
	map(0x04000000, 0x04003fff).ram().share("nvram");
	map(0x04100000, 0x041000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
	map(0x04200000, 0x0420000f).r("ym1", FUNC(ay8910_device::data_r)).umask16(0x00ff);
	map(0x04200000, 0x0420001f).w("ym1", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x04200100, 0x0420010f).r("ym2", FUNC(ay8910_device::data_r)).umask16(0x00ff);
	map(0x04200100, 0x0420011f).w("ym2", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x04400000, 0x0440007f).w(FUNC(tickee_gun_state::tickee_control_w)).umask16(0x00ff);
	map(0x04400040, 0x0440004f).portr("IN2");
	map(0xc0000240, 0xc000025f).nopw(); // seems to be a bug in their code
	map(0xff000000, 0xffffffff).rom().region("user1", 0);
}


// addresses in the 04x range shifted slightly...
void tickee_gun_state::ghoshunt_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("vram");
	map(0x02000000, 0x02ffffff).rom().region("user1", 0);
	map(0x04100000, 0x04103fff).ram().share("nvram");
	map(0x04200000, 0x042000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
	map(0x04300000, 0x0430000f).r("ym1", FUNC(ay8910_device::data_r)).umask16(0x00ff);
	map(0x04300000, 0x0430001f).w("ym1", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x04300100, 0x0430010f).r("ym2", FUNC(ay8910_device::data_r)).umask16(0x00ff);
	map(0x04300100, 0x0430011f).w("ym2", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x04500000, 0x0450007f).w(FUNC(tickee_gun_state::tickee_control_w)).umask16(0x00ff);
	map(0xc0000240, 0xc000025f).nopw(); // seems to be a bug in their code
	map(0xff000000, 0xffffffff).rom().region("user1", 0);
}


void tickee_state::mouseatk_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("vram");
	map(0x02000000, 0x02ffffff).rom().region("user1", 0);
	map(0x04000000, 0x04003fff).ram().share("nvram");
	map(0x04100000, 0x041000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
	map(0x04200000, 0x0420000f).r("ym", FUNC(ay8910_device::data_r)).umask16(0x00ff);
	map(0x04200000, 0x0420000f).w("ym", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x04200100, 0x0420010f).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x04400000, 0x0440007f).w(FUNC(tickee_state::tickee_control_w)).umask16(0x00ff);
	map(0x04400040, 0x0440004f).portr("IN2"); // ?
	map(0xc0000240, 0xc000025f).nopw(); // seems to be a bug in their code
	map(0xff000000, 0xffffffff).rom().region("user1", 0);
}


// newer hardware
void tickee_gun_state::rapidfir_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x007fffff).ram().share("vram");
	map(0x02000000, 0x027fffff).rw(FUNC(tickee_gun_state::rapidfir_transparent_r), FUNC(tickee_gun_state::rapidfir_transparent_w));
	map(0xfc000000, 0xfc00000f).r(FUNC(tickee_gun_state::rapidfir_gun1_r));
	map(0xfc000100, 0xfc00010f).r(FUNC(tickee_gun_state::rapidfir_gun2_r));
	map(0xfc000400, 0xfc00040f).nopr();
	map(0xfc000500, 0xfc00050f).w(FUNC(tickee_gun_state::rapidfir_output_w)).umask16(0x00ff).nopr();
	map(0xfc000600, 0xfc00060f).w(FUNC(tickee_gun_state::palette_blank_w)).umask16(0x00ff);
	map(0xfc000700, 0xfc00070f).w(FUNC(tickee_gun_state::sound_bank_w)).umask16(0x00ff);
	map(0xfc000800, 0xfc00080f).portr("IN0");
	map(0xfc000900, 0xfc00090f).portr("IN1");
	map(0xfc000a00, 0xfc000a0f).portr("IN2");
	map(0xfc000b00, 0xfc000b0f).portr("DSW0");
	map(0xfc000c00, 0xfc000c1f).portr("DSW1");
	map(0xfc000e00, 0xfc000e1f).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0xfc100000, 0xfc1000ff).mirror(0x80000).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
	map(0xfc200000, 0xfc207fff).ram().share("nvram");
	map(0xfc300000, 0xfc30000f).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0xfc400000, 0xfc40001f).rw("ym", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0xfe000000, 0xffffffff).rom().region("user1", 0);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( lightguns )
	PORT_START("GUNX1") // fake analog X
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1") // fake analog Y
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("GUNX2") // fake analog X
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2") // fake analog Y
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( tickee )
	PORT_INCLUDE( lightguns )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Game Time/Diff" )
	PORT_DIPSETTING(    0x03, "Very Fast/Very Easy" )
	PORT_DIPSETTING(    0x02, "Fast/Easy" )
	PORT_DIPSETTING(    0x01, "Average/Hard" )
	PORT_DIPSETTING(    0x00, "Slow/Very Hard" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "Last Box Tickets" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "25" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", FUNC(ticket_dispenser_device::line_r)) // right ticket status
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", FUNC(ticket_dispenser_device::line_r)) // left ticket status
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( ghoshunt )
	PORT_INCLUDE( lightguns )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Messages in Play")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "Fixed Ticketing")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "Setting")
	PORT_DIPSETTING(    0x04, "Custom")
	PORT_DIPSETTING(    0x00, DEF_STR( Standard ))
	PORT_DIPNAME( 0x08, 0x08, "Messages")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x30, 0x00, "Tickets")
	PORT_DIPSETTING(    0x30, "5")
	PORT_DIPSETTING(    0x20, "10")
	PORT_DIPSETTING(    0x10, "15")
	PORT_DIPSETTING(    0x00, "20")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", FUNC(ticket_dispenser_device::line_r)) // right ticket status
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", FUNC(ticket_dispenser_device::line_r)) // left ticket status
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xd8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", FUNC(ticket_dispenser_device::line_r)) // right ticket status
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", FUNC(ticket_dispenser_device::line_r)) // left ticket status
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xd8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( mouseatk )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Number of Extra Cheese" )    PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "1 Extra Cheese" )
	PORT_DIPSETTING(    0x02, "2 Extra Cheese" )
	PORT_DIPSETTING(    0x03, "3 Extra Cheese" )
	PORT_DIPNAME( 0x0c, 0x0c, "Number of Mice & Cheese" )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "4 Mice - 5 Cheese" )
	PORT_DIPSETTING(    0x04, "5 Mice - 6 Cheese" )
	PORT_DIPSETTING(    0x08, "6 Mice - 7 Cheese" )
	PORT_DIPSETTING(    0x0c, "7 Mice - 8 Cheese" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Ticket Game" )     PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Ticket Payout" )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", FUNC(ticket_dispenser_device::line_r)) // right ticket status
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", FUNC(ticket_dispenser_device::line_r)) // left ticket status
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rapidfir )
	PORT_INCLUDE( lightguns )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, "Reset NVRAM" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPUNKNOWN( 0x80, 0x80 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( maletmad )
	PORT_INCLUDE( rapidfir )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, "Payout" )
	PORT_DIPSETTING(    0x03, "1 Hit - 1 Ticket" )
	PORT_DIPSETTING(    0x02, "2 Hits - 1 Ticket" )
	PORT_DIPSETTING(    0x01, "3 Hits - 1 Ticket" )
	PORT_DIPSETTING(    0x00, "5 Hits - 1 Ticket" )
	PORT_DIPNAME( 0x0c, 0x0c, "Jackpot Amount" )
	PORT_DIPSETTING(    0x0c, "100 Tickets" )
	PORT_DIPSETTING(    0x08, "200 Tickets" )
	PORT_DIPSETTING(    0x04, "300 Tickets" )
	PORT_DIPSETTING(    0x00, "400 Tickets" )
	PORT_DIPNAME( 0x30, 0x30, "Game Time")
	PORT_DIPSETTING(    0x30, "10 seconds")
	PORT_DIPSETTING(    0x20, "15 seconds")
	PORT_DIPSETTING(    0x10, "20 seconds")
	PORT_DIPSETTING(    0x00, "30 seconds")
	PORT_DIPNAME( 0xc0, 0xc0, "Jackpot Trigger" )
	PORT_DIPSETTING(    0xc0, "4 Coins" )
	PORT_DIPSETTING(    0x80, "5 Coins" )
	PORT_DIPSETTING(    0x40, "6 Coins" )
	PORT_DIPSETTING(    0x00, "7 Coins" )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ))
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	// 1 mallet, so no there is no 'gun 2'
	PORT_MODIFY("GUNX2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("GUNY2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void tickee_gun_state::tickee(machine_config &config)
{
	// basic machine hardware
	TMS34010(config, m_maincpu, 40_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tickee_gun_state::tickee_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(14.318181_MHz_XTAL/2);
	m_maincpu->set_pixels_per_clock(1);
	m_maincpu->set_scanline_rgb32_callback(FUNC(tickee_gun_state::scanline_update));

	set_beamadd(50, 0);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TICKET_DISPENSER(config, m_ticket[0], attotime::from_msec(100));
	TICKET_DISPENSER(config, m_ticket[1], attotime::from_msec(100));

	// video hardware
	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL/2, 444, 0, 320, 233, 0, 200);
	m_screen->set_screen_update("maincpu", FUNC(tms34010_device::tms340x0_rgb32));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &ym1(YM2149(config, "ym1", 14.318181_MHz_XTAL/8));
	ym1.port_a_read_callback().set_ioport("DSW");
	ym1.port_b_read_callback().set_ioport("IN1");
	ym1.add_route(ALL_OUTPUTS, "mono", 0.50);

	ym2149_device &ym2(YM2149(config, "ym2", 14.318181_MHz_XTAL/8));
	ym2.port_a_read_callback().set_ioport("IN0");
	ym2.port_b_read_callback().set_ioport("IN2");
	ym2.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void tickee_gun_state::ghoshunt(machine_config &config)
{
	tickee(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &tickee_gun_state::ghoshunt_map);
}


void tickee_state::mouseatk(machine_config &config)
{
	// basic machine hardware
	TMS34010(config, m_maincpu, 40_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tickee_state::mouseatk_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(14.318181_MHz_XTAL/2);
	m_maincpu->set_pixels_per_clock(1);
	m_maincpu->set_scanline_rgb32_callback(FUNC(tickee_state::scanline_update));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TICKET_DISPENSER(config, m_ticket[0], attotime::from_msec(100));
	TICKET_DISPENSER(config, m_ticket[1], attotime::from_msec(100));

	// video hardware
	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL/2, 444, 0, 320, 233, 0, 200);
	m_screen->set_screen_update("maincpu", FUNC(tms34010_device::tms340x0_rgb32));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &ym(YM2149(config, "ym", 1_MHz_XTAL));
	ym.port_a_read_callback().set_ioport("DSW");
	ym.port_b_read_callback().set_ioport("IN1");
	ym.add_route(ALL_OUTPUTS, "mono", 0.50);

	OKIM6295(config, m_oki, 1_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00);
}


void tickee_gun_state::rapidfir(machine_config &config)
{
	// basic machine hardware
	TMS34010(config, m_maincpu, 50_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tickee_gun_state::rapidfir_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(18_MHz_XTAL/2);
	m_maincpu->set_pixels_per_clock(1);
	m_maincpu->set_scanline_rgb32_callback(FUNC(tickee_gun_state::scanline_update));
	m_maincpu->set_shiftreg_in_callback(FUNC(tickee_gun_state::rapidfir_to_shiftreg));
	m_maincpu->set_shiftreg_out_callback(FUNC(tickee_gun_state::rapidfir_from_shiftreg));

	set_beamadd(0, -5);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(18_MHz_XTAL/2, 556, 0, 384, 293, 0, 240);
	m_screen->set_screen_update("maincpu", FUNC(tms34010_device::tms340x0_rgb32));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.75);

	OKIM6295(config, m_oki, 8_MHz_XTAL/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void tickee_gun_state::maletmad(machine_config &config)
{
	rapidfir(config);

	TICKET_DISPENSER(config, m_ticket[0], attotime::from_msec(100));
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Tickee Tickats
Raster Elite, 1994

This is a gun shooting game similar to Point Blank, with ticket redemption on game completion.
The PCB is around 6" square and contains only a few conponents.

CPU  : TMS34010FNL-40
SOUND: AY-3-8910 (x2)
OSC  : 40.000MHz, 14.31818MHz
RAM  : TOSHIBA TC524258BZ-80 (x4)
DIPSW: 8 position (x1)
PROMs: None
PALs : None
OTHER: ADV476KN50E (DIP28)
       MACH110 (CPLD, PLCC44)
       DALLAS DS1220Y-150 (NVRAM)
       4-pin header for standard light gun (x2)

ROMS :
-----------------------------------------
ds1220y.ic1  NVRAM       located near ic2
1.ic2        27C040  \
2.ic3        27C040   |
3.ic4        27C040   |  main program
4.ic5        27C040   /
*/

ROM_START( tickee )
	ROM_REGION16_LE( 0x200000, "user1", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "3.ic4",  0x000000, 0x80000, CRC(5b1e399c) SHA1(681608f06bbaf3d258e9f4768a8a6c5047ad08ec) )
	ROM_LOAD16_BYTE( "2.ic3",  0x000001, 0x80000, CRC(1b26d4bb) SHA1(40266ec0fe5897eba85072e5bb39973d34f97546) )
	ROM_LOAD16_BYTE( "1.ic2",  0x100000, 0x80000, CRC(f7f0309e) SHA1(4a93e0e203f5a340a56b770a40b9ab00e131644d) )
	ROM_LOAD16_BYTE( "4.ic5",  0x100001, 0x80000, CRC(ceb0f559) SHA1(61923fe09e1dfde1eaae297ccbc672bc74a70397) )
ROM_END


/*

Ghost Hunter
Hanaho Games, 1996

  and

Tut's Tomb
Island Design Inc., 1996

PCB Layout
----------

|-----------------------------------------------|
|VOL                              CN106  CN103  |
|        NTE1423      ADV476KN50E    74LS175    |
|                                               |
|   YM2149            V52C4258Z80    74LS138    |
|                                               |
|   YM2149            V52C4258Z80    74LS373    |
|                                               |
|         DIPSW(8)    V52C4258Z80    74LS373    |
|J                                              |
|A         74LS161    V52C4258Z80    GHOSTHUN.7K|
|M  74LS74                                      |
|M         74LS273    |------|       GHOSTHUN.7J|
|A                    |TMS   |                  |
|          74LS74     |34010 |       GHOSTHUN.7H|
|                     |------|                  |
|          74LS14                    GHOSTHUN.7G|
|   ULN2803           |---|                     |
| TIP122   TIP122     |110|          DS1220Y    |
| TIP122   TIP122     |---|    40MHz 14.31818MHz|
| TIP122   TIP122    74LS273                    |
|                              74LS245  74LS374 |
|-------------------------------------|--CN2--|-|
(All IC's shown)                      |-------|

Notes:
      34010 clocks - INCLK: 40.000MHz, VCLK: 7.15909MHz, HSYNC: 16.1kHz, VSYNC: 69Hz, BLANK: 69Hz
      YM2149 clock - 1.7897725MHz [14.31818/8]
      VSync        - 69Hz
      HSync        - 15.78kHz
      110          - AMD MACH110 High Density Electrically-Erasable CMOS Programmable Logic (PLCC44)
      ADV476KN50E  - Analog Devices ADV476KN50E CMOS Monolithic 256x18 Color Palette RAM-DAC (DIP28)
      DS1220Y      - Dallas Semiconductor DS1220Y 16K Nonvolatile SRAM (DIP24)
      V52C4258Z80  - Vitelic V52C4258Z80 ?? possibly 256K x8 SRAM (ZIP28)
      NTE1423      - NTE1423 5.7W Power Amplifier (SIP8)
      CN2          - DB25 connector
      CN103/106    - 4-pin connector for gun hookup
      ULN2803      - Motorola ULN2803 Octal High Voltage, High Current Darlington Transistor Arrays (DIP18)
      TIP122       - Motorola TIP122 General-Purpose NPN Darlington Transistor (TO-220)
*/

ROM_START( ghoshunt )
	ROM_REGION16_LE( 0x200000, "user1", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "ghosthun.7g",  0x000001, 0x80000, CRC(d59716c2) SHA1(717a1a1c5c559569f9e7bc4ae4356d112f0cf4eb) )
	ROM_LOAD16_BYTE( "ghosthun.7h",  0x000000, 0x80000, CRC(ef38bfc8) SHA1(12b8f29f4da120f14126cbcdf4019bedd97063c3) )
	ROM_LOAD16_BYTE( "ghosthun.7j",  0x100001, 0x80000, CRC(763d7c79) SHA1(f0dec99feeeefeddda6a88276dc306a30a58f4e4) )
	ROM_LOAD16_BYTE( "ghosthun.7k",  0x100000, 0x80000, CRC(71e6099e) SHA1(2af6f1aa304eed849c90d95d17643cb12b05baab) )
ROM_END


ROM_START( tutstomb )
	ROM_REGION16_LE( 0x200000, "user1", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "tutstomb.7g",  0x000001, 0x80000, CRC(b74d3cf2) SHA1(2221b565362183a97a959389e8a0a026ca89e0ce) )
	ROM_LOAD16_BYTE( "tutstomb.7h",  0x000000, 0x80000, CRC(177f3afb) SHA1(845f982a66a8b69b0ea0045399102e8bb33f7fbf) )
	ROM_LOAD16_BYTE( "tutstomb.7j",  0x100001, 0x80000, CRC(69094f31) SHA1(eadae8847d0ff1568e63f71bf09a84dc443fdc1c))
	ROM_LOAD16_BYTE( "tutstomb.7k",  0x100000, 0x80000, CRC(bc362df8) SHA1(7b15c646e99c916d850629e4e758b1dbb329639a) )
ROM_END


/*

Mouse Attack
ICE, 1996

MA Video Board C ICE Inc. 1996
+--------------------------------------+
|                                 VOL  |
|      U202                            |
|   M6295             ADV476           |
|   1MHz               RAM             |
|J                     RAM             |
|A  YM2149             RAM        U10  |
|M                     RAM         U9  |
|M     DSW8                        U8  |
|A                  TMS34010-50    U7  |
|                                      |
|                  MACH110       40MHz |
|   +--------+ +------+                |
|   |  DB25  | | DB15 |     14.31818MHz|
+---+--------+-+------+----------------+

DB25 and DB15 are not populated.

*/

ROM_START( mouseatk )
	ROM_REGION16_LE( 0x200000, "user1", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "ms-attk2-rev.a1.u8",  0x000000, 0x80000, CRC(a150525c) SHA1(a6be8524ae435502eeeeeaafa856dc812121d4e3) )
	ROM_LOAD16_BYTE( "ms-attk3-rev.a1.u9",  0x000001, 0x80000, CRC(f060091d) SHA1(b3a2099d5ca5a658a7a87bb11a20c27a6a2f11f2) )
	ROM_LOAD16_BYTE( "ms-attk4-rev.a1.u10", 0x100000, 0x80000, CRC(19806349) SHA1(3431dc70897f50e1be7578dd4ef99fa9be4450cf) )
	ROM_LOAD16_BYTE( "ms-attk1-rev.a1.u7",  0x100001, 0x80000, CRC(b936194b) SHA1(33a2038a56fb4a4301ee04ca7a32a70ab5870fad) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "ms-snd-rev.a.u202", 0x000000, 0x80000, CRC(ce4af0f2) SHA1(f054ac27faf52ae5ec6910ecfca164a21eefe4ca) )
ROM_END


/*

Rapid Fire (v1.1)
Hanaho Games, 1998

BE 1V-0 VER3
+-----------------------------------------------+
|         U510 U507    M6295    MACH210         |
|   VOL DS1232                          RAM RAM |
|                               MACH210         |
|GUN1     YM3012 18MHz                  RAM RAM |
|GUN2                                           |
+-+         YM2151 8MHz  TMS34010               |
  |                3.5795MHz                    |
+-+                      MACH110                |
|                                    ADV476     |
|J            DSW2               50MHz          |
|A            DSW1                              |
|M                                  U3 U5 U7 U9 |
|M                                              |
|A     +------------+        DS1220 U2 U4 U6 U8 |
|      | CN110 DB25 |                           |
+------+------------+---------------------------+


PCB No: BE 1V-0 VER3: 7/30/98 (Sticker says PVG/SEMCO)
CPU   : TMS34010FNL-50
SND   : BS901 + CA5102 (YM2151 + YM3012), AD-65 (OKI M6295)
OSC   : 50.000MHz, 3.5795MHz, 8.000MHz, 18.000MHz
RAM   : Toshiba TC528267J-70 (x 4, surface mounted)
DIPs  : 8 position (x2)
OTHER : MACH110 (x1, CPLD)
        MACH210 (x2, CPLD)
        DS1232 (Dallas Semi, MicroMonitor Chip / Watchdog timer)
        ADV476KN50 (28 Pin DIP, Analog Devices CMOS Monolithic 256x18 color palette RAM-DAC @ 50MHz)
        DALLAS DS1220Y-200 (NONVOLATILE SRAM)

There is also a standard DB25 female connector (looks like a Parallel connector)

ROMs  : RF11.U2     27C040      \
        RF11.U3       "         |
        RF10.U4       "         |
        RF10.U5       "         |
        RF10.U6       "         |   Grouped together, PRG/GFX
        RF10.U7       "         |
        RF11.U8       "         |
        RF11.U9       "         |
        DS1220Y.U504  *1        /

        RF11.U507    27C040     \
        RF11.U510      "        /   Grouped together, SND/OKI SAMPLES

*1: Contents of Dallas DS1220 NVRAM (2K)

*/

ROM_START( rapidfir ) // Version 1.1, test menu shows "Build 239"
	ROM_REGION16_LE( 0x400000, "user1", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rf11.u8",  0x000000, 0x80000, CRC(f7d8df33) SHA1(0abd54bbccfa90d830cbbdbcf2058197af980981) )
	ROM_LOAD16_BYTE( "rf11.u9",  0x000001, 0x80000, CRC(a72af935) SHA1(ed0deb6f51681f70e07ad7c05a92f6a0f2063f7a) )
	ROM_LOAD16_BYTE( "rf10.u6",  0x100000, 0x80000, CRC(e8d2e5d2) SHA1(db93014598f7b76785e0fd5c0ac8808a3be06435) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf10.u7",  0x100001, 0x80000, CRC(0e33f2ed) SHA1(9b2533e001b94ccc97b95e31762186f59d5a3b9c) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf10.u4",  0x200000, 0x80000, CRC(8a088468) SHA1(f94dc78158e5656657d3b26c5b0ca88f39eb5ff4) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf10.u5",  0x200001, 0x80000, CRC(593b3df2) SHA1(301fa06031eff54fb2d9e08f80fc3c26e5c51da9) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf11.u2",  0x300000, 0x80000, CRC(ffa0c695) SHA1(bccdefe7cee15999c416fdcb16a65b1bc6e12d13) )
	ROM_LOAD16_BYTE( "rf11.u3",  0x300001, 0x80000, CRC(ac63b863) SHA1(c9160aec6179d1f550279b80fd4c2a14ce94fdab) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "rf11.u507", 0x000000, 0x80000, CRC(899d1e15) SHA1(ca22b4ad714a5212bc9347eb3a5b660c02bad7e5) )
	ROM_LOAD( "rf11.u510", 0x080000, 0x80000, CRC(6209c8fe) SHA1(bfbd63445b4ac2d4253c4b5354e1058070290084) )
ROM_END

ROM_START( rapidfira ) // Version 1.1, test menu shows "Build 238"
	ROM_REGION16_LE( 0x400000, "user1", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rf11.u8",  0x000000, 0x80000, CRC(f7d8df33) SHA1(0abd54bbccfa90d830cbbdbcf2058197af980981) )
	ROM_LOAD16_BYTE( "rf11.u9",  0x000001, 0x80000, CRC(a72af935) SHA1(ed0deb6f51681f70e07ad7c05a92f6a0f2063f7a) )
	ROM_LOAD16_BYTE( "rf10.u6",  0x100000, 0x80000, CRC(e8d2e5d2) SHA1(db93014598f7b76785e0fd5c0ac8808a3be06435) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf10.u7",  0x100001, 0x80000, CRC(0e33f2ed) SHA1(9b2533e001b94ccc97b95e31762186f59d5a3b9c) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf10.u4",  0x200000, 0x80000, CRC(8a088468) SHA1(f94dc78158e5656657d3b26c5b0ca88f39eb5ff4) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf10.u5",  0x200001, 0x80000, CRC(593b3df2) SHA1(301fa06031eff54fb2d9e08f80fc3c26e5c51da9) ) // Can be labeled V1.0 or V1.1
	ROM_LOAD16_BYTE( "rf11-u2",  0x300000, 0x80000, CRC(bcb7195a) SHA1(04b14facfe84973737f94b5278fb59668af0a47c) ) // Only U2 & U3 differ from Build 239, labeled the same
	ROM_LOAD16_BYTE( "rf11-u3",  0x300001, 0x80000, CRC(e455a0b5) SHA1(5568f55ccfd6cba373f4b4f3e61f0a4f391b01c0) ) // Only U2 & U3 differ from Build 239, labeled the same

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "rf11.u507", 0x000000, 0x80000, CRC(899d1e15) SHA1(ca22b4ad714a5212bc9347eb3a5b660c02bad7e5) )
	ROM_LOAD( "rf11.u510", 0x080000, 0x80000, CRC(6209c8fe) SHA1(bfbd63445b4ac2d4253c4b5354e1058070290084) )
ROM_END


ROM_START( rapidfire ) // Version 1.0, test menu shows "Build 236"
	ROM_REGION16_LE( 0x400000, "user1", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "rf10.u8",  0x000000, 0x80000, CRC(71d42125) SHA1(e34bdb08aa1b85ba4dd219b7f6de6f99a1ed8758) )
	ROM_LOAD16_BYTE( "rf10.u9",  0x000001, 0x80000, CRC(d70e67c2) SHA1(e0876027f58584dae949a4f3c9391bd013912ee8) )
	ROM_LOAD16_BYTE( "rf10.u6",  0x100000, 0x80000, CRC(e8d2e5d2) SHA1(db93014598f7b76785e0fd5c0ac8808a3be06435) )
	ROM_LOAD16_BYTE( "rf10.u7",  0x100001, 0x80000, CRC(0e33f2ed) SHA1(9b2533e001b94ccc97b95e31762186f59d5a3b9c) )
	ROM_LOAD16_BYTE( "rf10.u4",  0x200000, 0x80000, CRC(8a088468) SHA1(f94dc78158e5656657d3b26c5b0ca88f39eb5ff4) )
	ROM_LOAD16_BYTE( "rf10.u5",  0x200001, 0x80000, CRC(593b3df2) SHA1(301fa06031eff54fb2d9e08f80fc3c26e5c51da9) )
	ROM_LOAD16_BYTE( "rf10.u2",  0x300000, 0x80000, CRC(5ef404dd) SHA1(cf060567822c4c02baf4b1948d6e50b480bfb7da) )
	ROM_LOAD16_BYTE( "rf10.u3",  0x300001, 0x80000, CRC(d8d664db) SHA1(cd63fdc6fe4beb68ced57a2547f8302c1d2544dc) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "rf10.u507", 0x000000, 0x80000, CRC(7eab2af4) SHA1(bbb4b2b9f96add56c26f334a7242cdc81a64ce2d) )
	ROM_LOAD( "rf10.u510", 0x080000, 0x80000, CRC(ecd70be6) SHA1(5a26703e822776fecb3f6d729bf91e76db7a7141) )
ROM_END


/*

Mallet Madness (v2.1)
Hanaho Games, 1999 licensed to Capcom

Same exact PCB as Rapid Fire

*/

ROM_START( maletmad ) // Version 2.1
	ROM_REGION16_LE( 0x400000, "user1", 0 ) // 34010 code
	// U8 & U9 not populated
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u6",  0x100000, 0x80000, CRC(83309174) SHA1(d387c8bc4d3c640f16525241892cc8d5d5da7f60) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u7",  0x100001, 0x80000, CRC(4642587e) SHA1(076eda538d570074028e9b4394f1a8a459678137) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u4",  0x200000, 0x80000, CRC(70ca968c) SHA1(74c66a67568b428ae5e20377038c7ea0cd33b25e) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u5",  0x200001, 0x80000, CRC(c771418a) SHA1(cab360103a4d4195f5a9920746ae5df2866e24dc) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u2",  0x300000, 0x80000, CRC(08825aee) SHA1(d7c2098ce5d73e0ca4b02a654d5c958d999337e3) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u3",  0x300001, 0x80000, CRC(49a6bd62) SHA1(40d67fd7a3dded708366246d897682ffee88a2e1) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "malletmadness_v2.1.u507", 0x000000, 0x80000, CRC(6a2c9021) SHA1(bff61ef696a2104a32aab6fdc51f504385d0c769) )
	// U510 not populated
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, tickee,    0,        tickee,   tickee,   tickee_gun_state, empty_init, ROT0, "Raster Elite",                  "Tickee Tickats",               MACHINE_SUPPORTS_SAVE )
GAME( 1996, ghoshunt,  0,        ghoshunt, ghoshunt, tickee_gun_state, empty_init, ROT0, "Hanaho Games",                  "Ghost Hunter",                 MACHINE_SUPPORTS_SAVE )
GAME( 1996, tutstomb,  0,        ghoshunt, ghoshunt, tickee_gun_state, empty_init, ROT0, "Island Design",                 "Tut's Tomb",                   MACHINE_SUPPORTS_SAVE )

GAME( 1996, mouseatk,  0,        mouseatk, mouseatk, tickee_state,     empty_init, ROT0, "ICE",                           "Mouse Attack",                 MACHINE_SUPPORTS_SAVE )

GAME( 1998, rapidfir,  0,        rapidfir, rapidfir, tickee_gun_state, empty_init, ROT0, "Hanaho Games",                  "Rapid Fire (v1.1, Build 239)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, rapidfira, rapidfir, rapidfir, rapidfir, tickee_gun_state, empty_init, ROT0, "Hanaho Games",                  "Rapid Fire (v1.1, Build 238)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, rapidfire, rapidfir, rapidfir, rapidfir, tickee_gun_state, empty_init, ROT0, "Hanaho Games",                  "Rapid Fire (v1.0, Build 236)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, maletmad,  0,        maletmad, maletmad, tickee_gun_state, empty_init, ROT0, "Hanaho Games (Capcom license)", "Mallet Madness (v2.1)",        MACHINE_SUPPORTS_SAVE )
