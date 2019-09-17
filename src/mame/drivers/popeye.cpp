// license:BSD-3-Clause
// copyright-holders:smf, Nicola Salmoria, Couriersud
// thanks-to: Marc Lafontaine
/***************************************************************************

Popeye  (c) 1982 Nintendo

To enter service mode, reset keeping the service button pressed.

Notes:
- The main set has a protection device mapped at E000/E001. The second set
  (which is the same revision of the program code) has the protection disabled
  in a very clean way, so I don't know if it's an original (without the
  protection device to save costs), or a very well done bootleg.

***************************************************************************/

#include "emu.h"
#include "includes/popeye.h"
#include "machine/eepromser.h"
#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "screen.h"
#include "speaker.h"

/* This is the output stage of the audio circuit.
 * It is solely an impedance changer and could be left away
 */

static NETLIST_START(nl_popeye_imp_changer)
	RES(R62, 510000)
	RES(R63, 100)
	RES(R64, 510000)
	RES(R65, 2100)
	RES(R66, 330)
	RES(R67, 51)

	QBJT_EB(Q8, "2SC1815")
	QBJT_EB(Q9, "2SA1015")

	NET_C(V5, R62.1, Q8.C, R66.1)
	NET_C(R62.2, R64.1, R63.1, C7.2)
	NET_C(R63.2, Q8.B)
	NET_C(Q8.E, R65.1, Q9.B)
	NET_C(R66.2, Q9.E, R67.1)

	NET_C(GND, Q9.C, R65.2, R64.2)
NETLIST_END()

static NETLIST_START(nl_popeye)

	/* register hard coded netlists */

	LOCAL_SOURCE(nl_popeye_imp_changer)

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-5)
	ANALOG_INPUT(V5, 5)

	/* AY 8910 internal resistors */

	RES(R_AY1_1, 1000);
	RES(R_AY1_2, 1000);
	RES(R_AY1_3, 1000);

	RES(R52, 2000)
	RES(R55, 2000)
	RES(R58, 2000)
	RES(R53, 2000)
	RES(R56, 2000)
	RES(R59, 2000)
	RES(R51, 20000)
	RES(R57, 30000)
	RES(R60, 30000)

	RES(R61, 120000)

	RES(ROUT, 5000)

	CAP(C4, 0.047e-6)
	CAP(C5, 330e-12)
	CAP(C6, 330e-12)
	CAP(C7, 3.3e-6)
	CAP(C40, 680e-12)

	NET_C(V5, R_AY1_1.1, R_AY1_2.1, R_AY1_3.1)

	NET_C(R_AY1_1.2, R52.1, R53.1)
	NET_C(R_AY1_2.2, R55.1, R56.1)
	NET_C(R_AY1_3.2, R58.1, R59.1)

	NET_C(R53.2, R51.1, C4.1)
	NET_C(R56.2, R57.1, C5.1)
	NET_C(R59.2, R60.1, C6.1)

	NET_C(R51.2, R57.2, R60.2, R61.1, C40.1, C7.1)

	NET_C(GND, R52.2, R55.2, R58.2, C4.2, C5.2, C6.2, R61.2, C40.2)

	INCLUDE(nl_popeye_imp_changer)

	/* output resistor (actually located in TV */

	NET_C(R67.2, ROUT.1)

	NET_C(GND, ROUT.2)

NETLIST_END()



void tnx1_state::driver_start()
{
	decrypt_rom();

	save_item(NAME(m_prot0));
	save_item(NAME(m_prot1));
	save_item(NAME(m_prot_shift));
	save_item(NAME(m_nmi_enabled));

	m_prot0 = 0;
	m_prot1 = 0;
	m_prot_shift = 0;
	m_nmi_enabled = false;
}

void tpp2_state::driver_start()
{
	tnx1_state::driver_start();

	save_item(NAME(m_watchdog_enabled));
	save_item(NAME(m_watchdog_counter));

	m_watchdog_enabled = false;
	m_watchdog_counter = 0;
}

void tnx1_state::decrypt_rom()
{
	uint8_t *rom = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	/* decrypt the program ROMs */
	std::vector<uint8_t> buffer(len);
	for (int i = 0; i < len; i++)
		buffer[i] = bitswap<8>(rom[bitswap<16>(i, 15, 14, 13, 12, 11, 10, 8, 7, 0, 1, 2, 4, 5, 9, 3, 6) ^ 0xfc], 3, 4, 2, 5, 1, 6, 0, 7);
	std::copy_n(buffer.begin(), len, rom);
}

void popeyebl_state::decrypt_rom()
{
	uint8_t* rom = memregion("blprot")->base();
	for (int i = 0; i < 0x80; i++)
	{
		rom[i + 0x00] ^= 0xf; // opcodes
		rom[i + 0x80] ^= 0x3; // data
	}
}

void tpp2_state::decrypt_rom()
{
	uint8_t *rom = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	/* decrypt the program ROMs */
	std::vector<uint8_t> buffer(len);
	for (int i = 0; i < len; i++)
		buffer[i] = bitswap<8>(rom[bitswap<16>(i, 15, 14, 13, 12, 11, 10, 8, 7, 6, 3, 9, 5, 4, 2, 1, 0) ^ 0x3f], 3, 4, 2, 5, 1, 6, 0, 7);
	std::copy_n(buffer.begin(), len, rom);
}

WRITE8_MEMBER(tnx1_state::refresh_w)
{
	const bool nmi_enabled = ((offset >> 8) & 1) != 0;
	if (m_nmi_enabled != nmi_enabled)
	{
		m_nmi_enabled = nmi_enabled;

		if (!m_nmi_enabled)
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

WRITE8_MEMBER(tpp2_state::refresh_w)
{
	tnx1_state::refresh_w(space, offset, data, mem_mask);

	m_watchdog_enabled = ((offset >> 9) & 1) != 0;
}

WRITE_LINE_MEMBER(tnx1_state::screen_vblank)
{
	if (state)
	{
		std::copy_n(m_dmasource.target(), m_dmasource.bytes(), m_sprite_ram.begin());
		std::copy_n(m_dmasource.target(), 3, m_background_scroll);
		m_palette_bank = m_dmasource[3];

		m_field ^= 1;
		if (m_nmi_enabled)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(tpp2_state::screen_vblank)
{
	tnx1_state::screen_vblank(state);

	if (state)
	{
		uint8_t watchdog_counter = m_watchdog_counter;

		if (m_nmi_enabled || !m_watchdog_enabled)
			watchdog_counter = 0;
		else
			watchdog_counter = (watchdog_counter + 1) & 0xf;

		if ((watchdog_counter ^ m_watchdog_counter) & 4)
		{
			m_maincpu->set_input_line(INPUT_LINE_RESET, watchdog_counter & 4 ? ASSERT_LINE : CLEAR_LINE);
			m_aysnd->reset();
		}

		m_watchdog_counter = watchdog_counter;
	}
}

/* the protection device simply returns the last two values written shifted left */
/* by a variable amount. */

READ8_MEMBER(tnx1_state::protection_r)
{
	if (offset == 0)
	{
		return ((m_prot1 << m_prot_shift) | (m_prot0 >> (8-m_prot_shift))) & 0xff;
	}
	else    /* offset == 1 */
	{
		/* the game just checks if bit 2 is clear. Returning 0 seems to be enough. */
		return 0;
	}
}

WRITE8_MEMBER(tnx1_state::protection_w)
{
	if (offset == 0)
	{
		/* this is the same as the level number (1-3) */
		m_prot_shift = data & 0x07;
	}
	else    /* offset == 1 */
	{
		m_prot0 = m_prot1;
		m_prot1 = data;
	}
}


void tnx1_state::maincpu_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu",0);
	map(0x8000, 0x87ff).ram().share("ramlow");
	map(0x8800, 0x8bff).nopw(); // Attempts to initialize this area with 00 on boot
	map(0x8c00, 0x8e7f).ram().share("dmasource");
	map(0x8e80, 0x8fff).ram().share("ramhigh");
	map(0xa000, 0xa3ff).w(FUNC(tnx1_state::popeye_videoram_w)).share("videoram");
	map(0xa400, 0xa7ff).w(FUNC(tnx1_state::popeye_colorram_w)).share("colorram");
	map(0xc000, 0xcfff).w(FUNC(tnx1_state::background_w));
	map(0xe000, 0xe001).rw(FUNC(tnx1_state::protection_r), FUNC(tnx1_state::protection_w));
}

void tpp2_state::maincpu_program_map(address_map &map)
{
	tpp1_state::maincpu_program_map(map);
	map(0x8000, 0x87ff).unmaprw(); // 7f (unpopulated)
	map(0x8800, 0x8bff).ram().share("ramlow"); // 7h
	map(0xc000, 0xdfff).w(FUNC(tpp2_state::background_w));
}

void tpp2_noalu_state::maincpu_program_map(address_map &map)
{
	tpp2_state::maincpu_program_map(map);
	map(0xe000, 0xe001).noprw(); // game still writes level number & reads status, but then discards it
}

void popeyebl_state::maincpu_program_map(address_map &map)
{
	tnx1_state::maincpu_program_map(map);
	map(0xe000, 0xe01f).rom().region("blprot", 0x80);
}

void popeyebl_state::decrypted_opcodes_map(address_map& map)
{
	tnx1_state::maincpu_program_map(map);
	map(0xe000, 0xe01f).rom().region("blprot", 0);
}

void tnx1_state::maincpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x00, 0x00).portr("P1");
	map(0x01, 0x01).portr("P2");
	map(0x02, 0x02).portr("SYSTEM");
	map(0x03, 0x03).r("aysnd", FUNC(ay8910_device::data_r));
}


template<typename T>
class brazehs : public T
{
public:
	brazehs(const machine_config &mconfig, device_type type, const char *tag) :
		T(mconfig, type, tag),
		m_eeprom(*this, "eeprom")
	{
	}

	virtual void config(machine_config &config) override
	{
		T::config(config);
		EEPROM_93C46_8BIT(config, "eeprom");
	}

protected:
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	virtual void driver_start() override
	{
		T::driver_start();

		uint8_t *rom = this->memregion("brazehs")->base();
		int len = this->memregion("brazehs")->bytes();

		/* decrypt the program ROMs */
		std::vector<uint8_t> buffer(len);
		for (int i = 0; i < len; i++)
			buffer[i] = bitswap<8>(rom[bitswap<16>(i, 15, 10, 8, 9, 13, 14, 12, 11, 7, 6, 5, 4, 3, 2, 1, 0)], 1, 4, 5, 7, 6, 0, 3, 2);
		std::copy_n(buffer.begin(), len, rom);
	}

	DECLARE_READ8_MEMBER(eeprom_r)
	{
		return m_eeprom->do_read();
	}

	DECLARE_WRITE8_MEMBER(eeprom_w)
	{
		m_eeprom->di_write(data & 0x01);
		m_eeprom->cs_write(data & 0x04 ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write(data & 0x02 ? ASSERT_LINE : CLEAR_LINE);
	}

	virtual void maincpu_program_map(address_map &map) override
	{
		T::maincpu_program_map(map);
		map(0x0000, 0x7fff).rom().region("brazehs", 0);
		map(0x9000, 0x9000).rw(FUNC(brazehs::eeprom_r), FUNC(brazehs::eeprom_w));
	}
};


READ_LINE_MEMBER(tnx1_state::dsw1_read)
{
	return ioport("DSW1")->read() >> m_dswbit;
}


static INPUT_PORTS_START( skyskipr )
	PORT_START("P1")    /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("P2")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("SYSTEM")   /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW0")  /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x03, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "A 2/1 B 2/5" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x07, "A 1/1 B 2/1" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 1/2" )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, "A 1/2 B 1/4" )
	PORT_DIPSETTING(    0x0b, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x02, "A 2/5 B 1/1" )
	PORT_DIPSETTING(    0x0a, "A 1/3 B 1/1" )
	PORT_DIPSETTING(    0x09, "A 1/4 B 1/1" )
	PORT_DIPSETTING(    0x05, "A 1/5 B 1/1" )
	PORT_DIPSETTING(    0x08, "A 1/6 B 1/1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(tnx1_state, dsw1_read)

	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x1c, 0x0c, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x1c, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium_Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_SERVICE_DIPLOC(  0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

READ_LINE_MEMBER( tnx1_state::pop_field_r )
{
	return m_field ^ 1;
}

static INPUT_PORTS_START( popeye )
	PORT_START("P1")    /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("P2")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("SYSTEM")   /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(tnx1_state, pop_field_r) // inverted init e/o signal (even odd)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW0")  /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME( 0x60, 0x40, "Copyright" )
	PORT_CONFSETTING(    0x40, "Nintendo" )
	PORT_CONFSETTING(    0x20, "Nintendo Co.,Ltd" )
	PORT_CONFSETTING(    0x60, "Nintendo of America" )
//  PORT_CONFSETTING(    0x00, "Nintendo of America" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(tnx1_state, dsw1_read)

	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( popeyef )
	PORT_INCLUDE( popeye )

	PORT_MODIFY("DSW0")  /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x03, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "A 2/1 B 2/5" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x07, "A 1/1 B 2/1" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 1/2" )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, "A 1/2 B 1/4" )
	PORT_DIPSETTING(    0x0b, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x02, "A 2/5 B 1/1" )
	PORT_DIPSETTING(    0x0a, "A 1/3 B 1/1" )
	PORT_DIPSETTING(    0x09, "A 1/4 B 1/1" )
	PORT_DIPSETTING(    0x05, "A 1/5 B 1/1" )
	PORT_DIPSETTING(    0x08, "A 1/6 B 1/1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_MODIFY("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x10, "50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ 0, RGN_FRAC(1,2) },
	{RGN_FRAC(1,4)+7,RGN_FRAC(1,4)+6,RGN_FRAC(1,4)+5,RGN_FRAC(1,4)+4,
		RGN_FRAC(1,4)+3,RGN_FRAC(1,4)+2,RGN_FRAC(1,4)+1,RGN_FRAC(1,4)+0,
		7,6,5,4,3,2,1,0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
		7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	16*8
};

static GFXDECODE_START( gfx_popeye )
	GFXDECODE_SCALE( "gfx1", 0, charlayout,   16, 16, 2, 2 ) /* chars */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16+16*2, 8 ) /* sprites */
GFXDECODE_END




WRITE8_MEMBER(tnx1_state::popeye_portB_w)
{
	/* bit 0 flips screen */
	flip_screen_set(data & 1);

	/* bits 1-3 select DSW1 bit to read */
	m_dswbit = (data & 0x0e) >> 1;
}

void tnx1_state::config(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000)/2); /* 4 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &tnx1_state::maincpu_program_map);
	m_maincpu->set_addrmap(AS_IO, &tnx1_state::maincpu_io_map);
	m_maincpu->refresh_cb().set(FUNC(tnx1_state::refresh_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0*16, 32*16-1, 2*16, 30*16-1);
	screen.set_screen_update(FUNC(tnx1_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(tnx1_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_popeye);
	PALETTE(config, m_palette, FUNC(tnx1_state::tnx1_palette), 16 + 16*2 + 8*4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_aysnd, XTAL(8'000'000)/4);
	m_aysnd->port_a_read_callback().set_ioport("DSW0");
	m_aysnd->port_b_write_callback().set(FUNC(tnx1_state::popeye_portB_w));
	m_aysnd->add_route(ALL_OUTPUTS, "mono", 0.40);
}

void tpp2_state::config(machine_config &config)
{
	tpp1_state::config(config);

	m_aysnd->reset_routes();
	m_aysnd->set_flags(AY8910_RESISTOR_OUTPUT); /* Does tnx1, tpp1 & popeyebl have the same filtering? */
	m_aysnd->set_resistors_load(2000.0, 2000.0, 2000.0);
	m_aysnd->add_route(0, "snd_nl", 1.0, 0);
	m_aysnd->add_route(1, "snd_nl", 1.0, 1);
	m_aysnd->add_route(2, "snd_nl", 1.0, 2);

	/* NETLIST configuration using internal AY8910 resistor values */

	NETLIST_SOUND(config, "snd_nl", 48000)
		.set_source(netlist_nl_popeye)
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_STREAM_INPUT(config, "snd_nl:cin0", 0, "R_AY1_1.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin1", 1, "R_AY1_2.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin2", 2, "R_AY1_3.R");

	NETLIST_STREAM_OUTPUT(config, "snd_nl:cout0", 0, "ROUT.1").set_mult_offset(30000.0, -65000.0);
}

void popeyebl_state::config(machine_config& config)
{
	tpp1_state::config(config);

	m_maincpu->set_addrmap(AS_OPCODES, &popeyebl_state::decrypted_opcodes_map);
}



/***************************************************************************

  Game ROMset(s)

***************************************************************************/

ROM_START( skyskipr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tnx1-c.2a",    0x0000, 0x1000, CRC(bdc7f218) SHA1(9a5f5959b9228912f810568ddad70daa81c4daac) )
	ROM_LOAD( "tnx1-c.2b",    0x1000, 0x1000, CRC(cbe601a8) SHA1(78edc384b75b7958906f887d11eb7cf235d6dc44) )
	ROM_LOAD( "tnx1-c.2c",    0x2000, 0x1000, CRC(5ca79abf) SHA1(0712364ad8785a146c4a146cc688c4892dd59c93) )
	ROM_LOAD( "tnx1-c.2d",    0x3000, 0x1000, CRC(6b7a7071) SHA1(949a8106a5b750bc15a5919786fb99f15cd9424e) )
	ROM_LOAD( "tnx1-c.2e",    0x4000, 0x1000, CRC(6b0c0525) SHA1(e4e12ea9e3140736d7543a274f3b266e58059356) )
	ROM_LOAD( "tnx1-c.2f",    0x5000, 0x1000, CRC(d1712424) SHA1(2de42c379f18bfbd68fc64db24c9b0d38de26c29) )
	ROM_LOAD( "tnx1-c.2g",    0x6000, 0x1000, CRC(8b33c4cf) SHA1(86d51b5098dffc69330b28662b04bd906d962792) )
	ROM_FILL( 0x7000, 0x1000, 0xff )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tnx1-v.3h",    0x0000, 0x0800, CRC(ecb6a046) SHA1(7fd2312d39fefe6237699e2916e0c313165755ad) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tnx1-t.1e",    0x0000, 0x1000, CRC(01c1120e) SHA1(c1ab2af9c582e647e98836a87cb09fe4bcdaf19f) )
	ROM_LOAD( "tnx1-t.2e",    0x1000, 0x1000, CRC(70292a71) SHA1(51a5677351e438e7ef2b3812ef5e0e610da6ef4d) )
	ROM_LOAD( "tnx1-t.3e",    0x2000, 0x1000, CRC(92b6a0e8) SHA1(d11ff39932cec9b5408dc957bf825dcf1d00c027) )
	ROM_LOAD( "tnx1-t.5e",    0x3000, 0x1000, CRC(cc5f0ac3) SHA1(3374b1c387df3a3eba6c523f7c13711260074a89) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tnx1-t.4a",    0x0000, 0x0020, CRC(98846924) SHA1(89688d05c4c2d6f389da4e9c0c13e0f022f2d376) ) /* background palette */
	ROM_LOAD( "tnx1-t.1a",    0x0020, 0x0020, CRC(c2bca435) SHA1(e7d3e68d153d646eaf12b263d58b07da2615399c) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tnx1-t.3a", 0x0000, 0x0100, CRC(8abf9de4) SHA1(6e5500639a2dca3c288619fb8bdd120eb49bf8e0) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tnx1-t.2a", 0x0000, 0x0100, CRC(aa7ff322) SHA1(522d21854aa11e24f3679163354ae4fb35619eff) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tnx1-t.3j",    0x0000, 0x0100, CRC(1c5c8dea) SHA1(5738303b2a9c79b7d06bcf20fdb4d9b29f6e2d96) ) /* video timing prom */
ROM_END

/*
    Popeye

    CPU/Sound Board: TPP2-01-CPU
    Video Board:     TPP2-01-VIDEO
*/

ROM_START( popeye )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tpp2-c.7a", 0x0000, 0x2000, CRC(9af7c821) SHA1(592acfe221b5c3bd9b920f639b141f37a56d6997) )
	ROM_LOAD( "tpp2-c.7b", 0x2000, 0x2000, CRC(c3704958) SHA1(af96d10fa9bdb86b00c89d10f67cb5ca5586f446) )
	ROM_LOAD( "tpp2-c.7c", 0x4000, 0x2000, CRC(5882ebf9) SHA1(5531229b37f9ba0ede7fdc24909e3c3efbc8ade4) )
	ROM_LOAD( "tpp2-c.7e", 0x6000, 0x2000, CRC(ef8649ca) SHA1(a0157f91600e56e2a953dadbd76da4330652e5c8) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n", 0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )   /* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e", 0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f", 0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j", 0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k", 0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tpp2-c.5b", 0x0000, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tpp2-c.5a", 0x0000, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tpp2-v.7j", 0x0000, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* video timing prom */
ROM_END

ROM_START( popeyeu )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "7a",        0x0000, 0x2000, CRC(0bd04389) SHA1(3b08186c9b20dd4dfb92df98941b18999f23aece) )
	ROM_LOAD( "7b",        0x2000, 0x2000, CRC(efdf02c3) SHA1(4fa616bdb4e21f752e46890d007c911fff9ceadc) )
	ROM_LOAD( "7c",        0x4000, 0x2000, CRC(8eee859e) SHA1(a597d5655d06d565653c64b18ed8842625e15088) )
	ROM_LOAD( "7e",        0x6000, 0x2000, CRC(b64aa314) SHA1(b5367f518350223e191d94434dc535873efb4c74) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n", 0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )   /* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e", 0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f", 0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j", 0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k", 0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tpp2-c.5b", 0x0000, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tpp2-c.5a", 0x0000, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tpp2-v.7j", 0x0000, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* video timing prom */
ROM_END

ROM_START( popeyef )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tpp2-c_f.7a", 0x0000, 0x2000, CRC(5fc5264d) SHA1(6c3d4df748c55293b6de58bd874a08f8164b878d) )
	ROM_LOAD( "tpp2-c_f.7b", 0x2000, 0x2000, CRC(51de48e8) SHA1(7573931c6fcb53ee5ab9408906cd8eb2ba271c64) )
	ROM_LOAD( "tpp2-c_f.7c", 0x4000, 0x2000, CRC(62df9647) SHA1(65d043b4142aa3ad2db7a1d4e1a2c22656ca7ade) )
	ROM_LOAD( "tpp2-c_f.7e", 0x6000, 0x2000, CRC(f31e7916) SHA1(0f54ea7b1691b7789067fe880ffc56fac1d9523a) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n",   0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) ) /* first half is empty */
	ROM_CONTINUE(            0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e",   0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f",   0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j",   0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k",   0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a",   0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a",   0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tpp2-c.5b", 0x0000, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tpp2-c.5a", 0x0000, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tpp2-v.7j",   0x0000, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* video timing prom */
ROM_END

ROM_START( popeyebl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "app_exe.3j.2764", 0x0000, 0x2000, CRC(6e267c48) SHA1(d4984eedb12d15867f0fdef4b525e8288d656962) )
	ROM_LOAD( "2.3l.2764",       0x2000, 0x2000, CRC(995475ff) SHA1(5cd5ac23a73722e32c80cd6ffc435584750a46c9) )
	ROM_LOAD( "3.3m.2764",       0x4000, 0x2000, CRC(99d6a04a) SHA1(b683a5bb1ac4f6bec7478760c8ad0ff7c00bc652) )
	ROM_LOAD( "4.3p.2764",       0x6000, 0x2000, CRC(548a6514) SHA1(006e076781a3e5c3afa084c723247365358e3187) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "5.10gh.2732",  0x0000, 0x0800, CRC(ce6c9f8e) SHA1(d52058c71c1769d9b9c4e713ac347096ac97bf1e) )    /* first and second half identical */
	ROM_CONTINUE(             0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "6.6n.2764",    0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "7.6r.2764",    0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "8.6s.2764",    0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "9.6u.2764",    0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", ROMREGION_INVERT )
	ROM_LOAD( "6.2u.18s030",  0x0000, 0x0020, CRC(d138e8a4) SHA1(eba7f870ccab72105593007f5cd7e0b863912402) ) /* background palette */
	ROM_LOAD( "5.2t.18s030",  0x0020, 0x0020, CRC(0f364007) SHA1(b77d71df391a9ac9e778e84475627e72de2b8507) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", ROMREGION_INVERT )
	ROM_LOAD_NIB_LOW(  "3.2r.24s10", 0x0000, 0x0100, BAD_DUMP CRC(ca4d7b6a) SHA1(ec979fffea9db5a327a5270241e376c21516e343) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "4.2s.24s10", 0x0000, 0x0100, CRC(cab9bc53) SHA1(e63ba8856190187996e405f6fcee254c8ca6e81f) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "blprot", 0 )
	ROM_LOAD_NIB_LOW(  "1.1d.24s10", 0x0000, 0x0100, CRC(2e1b143a) SHA1(7e0fd19328ccd6f2b2148739ef64703ade585060) )
	ROM_LOAD_NIB_HIGH( "2.1e.24s10", 0x0000, 0x0100, CRC(978b1c63) SHA1(ae67a4ac554e84c970c0acc82f4bc6a490f9d6ef) )

	ROM_REGION(0x0100, "timing", 0)
	ROM_LOAD( "7.11s.24s10",  0x0000, 0x0100, CRC(1c5c8dea) SHA1(5738303b2a9c79b7d06bcf20fdb4d9b29f6e2d96) ) /* video timing prom */
ROM_END

ROM_START( popeyeb2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "po1",          0x0000, 0x2000, CRC(b14a07ca) SHA1(b8666a4c6b833f60905692774e30e73f0795df11) )
	ROM_LOAD( "po2",          0x2000, 0x2000, CRC(995475ff) SHA1(5cd5ac23a73722e32c80cd6ffc435584750a46c9) )
	ROM_LOAD( "po3",          0x4000, 0x2000, CRC(99d6a04a) SHA1(b683a5bb1ac4f6bec7478760c8ad0ff7c00bc652) )
	ROM_LOAD( "po4",          0x6000, 0x2000, CRC(548a6514) SHA1(006e076781a3e5c3afa084c723247365358e3187) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "po5",          0x0000, 0x0800, CRC(ce6c9f8e) SHA1(d52058c71c1769d9b9c4e713ac347096ac97bf1e) )    /* first and second half identical */
	ROM_CONTINUE(             0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "po6",          0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "po7",          0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "po8",          0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "po9",          0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", ROMREGION_INVERT )
	ROM_LOAD( "popeye.pr1",   0x0000, 0x0020, CRC(d138e8a4) SHA1(eba7f870ccab72105593007f5cd7e0b863912402) ) /* background palette */
	ROM_LOAD( "popeye.pr2",   0x0020, 0x0020, CRC(0f364007) SHA1(b77d71df391a9ac9e778e84475627e72de2b8507) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", ROMREGION_INVERT )
	ROM_LOAD_NIB_LOW(  "popeye.pr3", 0x0000, 0x0100, CRC(ca4d7b6a) SHA1(ec979fffea9db5a327a5270241e376c21516e343) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "popeye.pr4", 0x0000, 0x0100, CRC(cab9bc53) SHA1(e63ba8856190187996e405f6fcee254c8ca6e81f) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "blprot", 0 )
	ROM_LOAD_NIB_LOW(  "popeye.d1",  0x0000, 0x0100, CRC(2e1b143a) SHA1(7e0fd19328ccd6f2b2148739ef64703ade585060) )
	ROM_LOAD_NIB_HIGH( "popeye.e1",  0x0000, 0x0100, CRC(978b1c63) SHA1(ae67a4ac554e84c970c0acc82f4bc6a490f9d6ef) )
ROM_END

ROM_START( popeyeb3 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bdf-5",     0x0000, 0x2000, CRC(c02b5e95) SHA1(c3bf184777971943f08859b9e664507d6d11876a) )
	ROM_LOAD( "bdf-6",     0x2000, 0x2000, CRC(efdf02c3) SHA1(4fa616bdb4e21f752e46890d007c911fff9ceadc) )
	ROM_LOAD( "bdf-7",     0x4000, 0x2000, CRC(8eee859e) SHA1(a597d5655d06d565653c64b18ed8842625e15088) )
	ROM_LOAD( "bdf-8",     0x6000, 0x2000, CRC(bac64fdd) SHA1(ed87ecc6509c634950d27e87d4694896d0e41052) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "bdf-9",     0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )   /* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "bdf-4",     0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "bdf-3",     0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "bdf-2",     0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "bdf-1",     0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, BAD_DUMP CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, BAD_DUMP CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tpp2-c.5b", 0x0000, 0x0100, BAD_DUMP CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tpp2-c.5a", 0x0000, 0x0100, BAD_DUMP CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tpp2-v.7j", 0x0000, 0x0100, BAD_DUMP CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* video timing prom */
ROM_END

ROM_START( popeyej )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tpp1-c.2a,2732",    0x0000, 0x1000, CRC(4176761e) SHA1(6977da294e88bb2d08ce02901f35c99df69dd6a5) )
	ROM_LOAD( "tpp1-c.2b,2732",    0x1000, 0x1000, CRC(4e0b7f06) SHA1(02667e297ef2112cf4150c0c18e5715c9010e4ac) )
	ROM_LOAD( "tpp1-c.2c,2732",    0x2000, 0x1000, CRC(b1c18b7e) SHA1(12a067e2914a9d2051b2507de0673ac4a5198329) )
	ROM_LOAD( "tpp1-c.2d,2732",    0x3000, 0x1000, CRC(79d0e988) SHA1(7c8ee46918996dab39b417e4361edef259935dbf) )
	ROM_LOAD( "tpp1-c.2e,2732",    0x4000, 0x1000, CRC(74854ca1) SHA1(9b9e5112e32105a9726fff23162be38b5b9e3f7b) )
	ROM_LOAD( "tpp1-c.2f,2732",    0x5000, 0x1000, CRC(e2b08891) SHA1(e73b1d355f543772c79e1970426e3e9696b2b23c) )
	ROM_LOAD( "tpp1-c.2g,2732",    0x6000, 0x1000, CRC(b74a1a97) SHA1(3a51b08f80d378403b1e0df8446057b69157ea9b) )
	ROM_LOAD( "tpp1-c.2h,2732",    0x7000, 0x1000, CRC(30e84104) SHA1(c781cb47d940e8e5d4f3cb799222db634d99e054) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp1-v.3h,2716",    0x0000, 0x0800, CRC(fa52a752) SHA1(45b8f52e4c15f9fe761d7c7f7109b4a70ada0cda) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp1-e.1e,2763",    0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp1-e.2e,2763",    0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp1-e.3e,2763",    0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp1-e.5e,2763",    0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tpp1-t.4a,82s123",  0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp1-t.1a,82s123",  0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tpp1-t.3a,82s129", 0x0000, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tpp1-t.2a,82s129", 0x0000, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tpp1-t.3j,82s129", 0x0000, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* video timing prom */
ROM_END

ROM_START( popeyejo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tpp1-c.2a.bin",     0x0000, 0x1000, CRC(4176761e) SHA1(6977da294e88bb2d08ce02901f35c99df69dd6a5) )
	ROM_LOAD( "tpp1-c.2b.bin",     0x1000, 0x1000, CRC(2cc76c54) SHA1(56b9970c930e83dc9d5c6d5581160c42382f5c89) ) // diff from popeyej
	ROM_LOAD( "tpp1-c.2c,2732",    0x2000, 0x1000, CRC(b1c18b7e) SHA1(12a067e2914a9d2051b2507de0673ac4a5198329) )
	//  ROM_LOAD( "tpp1-c.2c.bin", 0x2000, 0x1000, CRC(d3061b82) SHA1(52fe1ab8f1dc79383894eb3c3a8b4039ab7dfaf0) )
	//  Actual Dump had Fixed Bits but when compared the stuck bit accounted for all the errors compared to popeyej's 2C, so we use that one.
	ROM_LOAD( "tpp1-c.2d.bin",     0x3000, 0x1000, CRC(79d0e988) SHA1(7c8ee46918996dab39b417e4361edef259935dbf) )
	ROM_LOAD( "tpp1-c.2e.bin",     0x4000, 0x1000, CRC(74854ca1) SHA1(9b9e5112e32105a9726fff23162be38b5b9e3f7b) )
	ROM_LOAD( "tpp1-c.2f.bin",     0x5000, 0x1000, CRC(e2b08891) SHA1(e73b1d355f543772c79e1970426e3e9696b2b23c) )
	ROM_LOAD( "tpp1-c.2g.bin",     0x6000, 0x1000, CRC(b74a1a97) SHA1(3a51b08f80d378403b1e0df8446057b69157ea9b) )
	ROM_LOAD( "tpp1-c.2h.bin",     0x7000, 0x1000, CRC(a1dcf54d) SHA1(de2574c0437eba6d01658f8936a1f9285a362b4b) ) // diff from popeyej

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp1-v.3h.bin",     0x0000, 0x0800, CRC(fa52a752) SHA1(45b8f52e4c15f9fe761d7c7f7109b4a70ada0cda) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp1-e.1e.bin",     0x0000, 0x2000, CRC(90889e1d) SHA1(11af14ce7e2583a1aced942031284219eaf63e66) )
	ROM_LOAD( "tpp1-e.2e.bin",     0x2000, 0x2000, CRC(ed06af50) SHA1(68db835d9747e11ca85d0dd945e52bc1f538e251) )
	ROM_LOAD( "tpp1-e.3e.bin",     0x4000, 0x2000, CRC(72b258f2) SHA1(a1fe2a380f3c46bd54043dc001c168734873d854) )
	ROM_LOAD( "tpp1-e.5e.bin",     0x6000, 0x2000, CRC(7355ff16) SHA1(15d3cb701ad360a36c26eb5c34b2e89b5972ba82) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tpp1-t.4a.82s123",  0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp1-t.1a.82s123",  0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tpp1-t.3a.82s129", 0x0000, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tpp1-t.2a.82s129", 0x0000, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tpp1-t.3j.82s129", 0x0000, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* video timing prom */
ROM_END

ROM_START( popeyehs )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tpp2-c.7a", 0x0000, 0x2000, CRC(9af7c821) SHA1(592acfe221b5c3bd9b920f639b141f37a56d6997) )
	ROM_LOAD( "tpp2-c.7b", 0x2000, 0x2000, CRC(c3704958) SHA1(af96d10fa9bdb86b00c89d10f67cb5ca5586f446) )
	ROM_LOAD( "tpp2-c.7c", 0x4000, 0x2000, CRC(5882ebf9) SHA1(5531229b37f9ba0ede7fdc24909e3c3efbc8ade4) )
	ROM_LOAD( "tpp2-c.7e", 0x6000, 0x2000, CRC(ef8649ca) SHA1(a0157f91600e56e2a953dadbd76da4330652e5c8) )

	ROM_REGION( 0x8000, "brazehs", 0 )
	ROM_LOAD( "p100d.bin", 0x0000, 0x8000, CRC(ab8d7821) SHA1(368352af26caaac8abd95c391263c59c1358fd28) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n", 0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )   /* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e", 0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f", 0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j", 0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k", 0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */

	ROM_REGION( 0x0100, "sprpal", 0 )
	ROM_LOAD_NIB_LOW(  "tpp2-c.5b", 0x0000, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD_NIB_HIGH( "tpp2-c.5a", 0x0000, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */

	ROM_REGION( 0x0100, "timing", 0 )
	ROM_LOAD( "tpp2-v.7j", 0x0000, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* video timing prom */
ROM_END


GAME( 1981, skyskipr, 0,        config,  skyskipr, tnx1_state,       empty_init, ROT0, "Nintendo", "Sky Skipper",                          MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeye,   0,        config,  popeye,   tpp2_state,       empty_init, ROT0, "Nintendo", "Popeye (revision D)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyeu,  popeye,   config,  popeye,   tpp2_noalu_state, empty_init, ROT0, "Nintendo", "Popeye (revision D not protected)",    MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyef,  popeye,   config,  popeyef,  tpp2_noalu_state, empty_init, ROT0, "Nintendo", "Popeye (revision F)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyebl, popeye,   config,  popeye,   popeyebl_state,   empty_init, ROT0, "bootleg",  "Popeye (bootleg set 1)",               MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyeb2, popeye,   config,  popeye,   popeyebl_state,   empty_init, ROT0, "bootleg",  "Popeye (bootleg set 2)",               MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyeb3, popeye,   config,  popeye,   tpp2_noalu_state, empty_init, ROT0, "bootleg",  "Popeye (bootleg set 3)",               MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyej,  popeye,   config,  popeye,   tpp1_state,       empty_init, ROT0, "Nintendo", "Popeye (Japan)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyejo, popeye,   config,  popeye,   tpp1_state,       empty_init, ROT0, "Nintendo", "Popeye (Japan, Older)",                MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyehs, popeye,   config,  popeye,   brazehs<tpp2_noalu_state>, empty_init, ROT0, "hack (Braze Technologies)", "Popeye (Braze High Score Kit P1.00D)", MACHINE_SUPPORTS_SAVE )
