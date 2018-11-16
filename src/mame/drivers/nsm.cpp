// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

    PINBALL
    NSM (Lowen) : Hot Fire Birds

    Schematic and PinMAME used as references

    Everything in this machine is controlled by a serial bus based on the
    processor's CRU pins (serial i/o).

ToDo:
- Inputs (i have no idea how CRU inputs work)
- Mechanical sounds
- Further testing, etc

*********************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/tms9900/tms9995.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "nsm.lh"


class nsm_state : public driver_device
{
public:
	nsm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
		{ }

	void nsm(machine_config &config);

private:

	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_WRITE8_MEMBER(cru_w);
	DECLARE_WRITE8_MEMBER(oe_w);
	void nsm_io_map(address_map &map);
	void nsm_map(address_map &map);

	uint8_t m_cru_data[9];
	uint8_t m_cru_count;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<tms9995_device> m_maincpu;
	output_finder<60> m_digits;
};

void nsm_state::nsm_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xefff).ram();
	map(0xffec, 0xffed).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xffee, 0xffef).w("ay2", FUNC(ay8910_device::address_data_w));
}

void nsm_state::nsm_io_map(address_map &map)
{
	// 00-71 selected by IC600 (74LS151)
	map(0x0000, 0x0001).r(FUNC(nsm_state::ff_r)); // 5v supply
	map(0x0010, 0x0011).nopr(); // antenna
	map(0x0020, 0x0021).nopr(); // reset circuit
	map(0x0030, 0x0031).r(FUNC(nsm_state::ff_r)); // service plug
	map(0x0040, 0x0041).r(FUNC(nsm_state::ff_r)); // service plug
	map(0x0050, 0x0051).r(FUNC(nsm_state::ff_r)); // test of internal battery
	map(0x0060, 0x0061).r(FUNC(nsm_state::ff_r)); // sum of analog outputs of ay2
	//AM_RANGE(0x0070, 0x0071) AM_READNOP // serial data in
	map(0x0f70, 0x0f7d).nopw();
	map(0x0fe4, 0x0fff).nopr();
	map(0x7fb0, 0x7fbf).w(FUNC(nsm_state::cru_w));
	map(0x7fd0, 0x7fd1).w(FUNC(nsm_state::oe_w));
}

static INPUT_PORTS_START( nsm )
INPUT_PORTS_END

READ8_MEMBER( nsm_state::ff_r ) { return 1; }

WRITE8_MEMBER( nsm_state::oe_w )
{
	m_cru_count = 9;
}

WRITE8_MEMBER( nsm_state::cru_w )
{
	offset &= 7;
	if (!offset)
	{
		m_cru_count--;
		m_cru_data[m_cru_count] = 0;
	}
	m_cru_data[m_cru_count] |= (data << offset);

	uint8_t i,j;
	int segments;
	if (!m_cru_count && (offset == 7))
	{
		m_cru_count = 9;
		//for (i = 0; i < 9; i++) printf("%02X ",m_cru_data[i]);printf("\n");
		for (i = 0; i < 8; i++)
		{
			if (BIT(m_cru_data[0], i))
			{
				for (j = 0; j < 5; j++)
				{
					segments = m_cru_data[8-j]^0xff;
					m_digits[j * 10 + i] = bitswap<16>(segments, 8, 8, 8, 8, 8, 8, 0, 0, 1, 1, 2, 3, 4, 5, 6, 7);
				}
			}
		}
	}
}

void nsm_state::machine_reset()
{
	// Disable auto wait state generation by raising the READY line on reset
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

void nsm_state::nsm(machine_config &config)
{
	// CPU TMS9995, standard variant; no line connection
	TMS9995(config, m_maincpu, 11052000);
	m_maincpu->set_addrmap(AS_PROGRAM, &nsm_state::nsm_map);
	m_maincpu->set_addrmap(AS_IO, &nsm_state::nsm_io_map);

	/* Video */
	config.set_default_layout(layout_nsm);

	/* Sound */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AY8912(config, "ay1", 11052000/8).add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	AY8912(config, "ay2", 11052000/8).add_route(ALL_OUTPUTS, "rspeaker", 0.75);
}

/*-------------------------------------------------------------------
/ Cosmic Flash (1985)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Hot Fire Birds (1985)
/-------------------------------------------------------------------*/
ROM_START(firebird)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nsmf02.764", 0x0000, 0x2000, CRC(236b5780) SHA1(19ef6e1fc900e5d94f615a4316f0383ed5ee939c))
	ROM_LOAD("nsmf03.764", 0x2000, 0x2000, CRC(d88c6ef5) SHA1(00edeefaab7e1141741aa132e6f7e56a911573be))
	ROM_LOAD("nsmf04.764", 0x4000, 0x2000, CRC(38a8add4) SHA1(74f781edc31aad07411feacad53c5f6cc73d09f4))
ROM_END

/*-------------------------------------------------------------------
/ Tag-Team Pinball (1986)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ The Games (1985)
/-------------------------------------------------------------------*/

GAME(1985,  firebird,  0,  nsm,  nsm, nsm_state, empty_init, ROT0, "NSM", "Hot Fire Birds", MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
