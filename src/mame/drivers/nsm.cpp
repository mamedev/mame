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

#include "machine/genpin.h"

#include "cpu/tms9900/tms9995.h"
#include "sound/ay8910.h"
#include "nsm.lh"


class nsm_state : public driver_device
{
public:
	nsm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(ff_r);
	DECLARE_WRITE8_MEMBER(cru_w);
	DECLARE_WRITE8_MEMBER(oe_w);
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
private:
	UINT8 m_cru_data[9];
	UINT8 m_cru_count;
};

static ADDRESS_MAP_START( nsm_map, AS_PROGRAM, 8, nsm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xffec, 0xffed) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xffee, 0xffef) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nsm_io_map, AS_IO, 8, nsm_state )
	// 00-71 selected by IC600 (74LS151)
	AM_RANGE(0x0000, 0x0001) AM_READ(ff_r) // 5v supply
	AM_RANGE(0x0010, 0x0011) AM_READNOP // antenna
	AM_RANGE(0x0020, 0x0021) AM_READNOP // reset circuit
	AM_RANGE(0x0030, 0x0031) AM_READ(ff_r) // service plug
	AM_RANGE(0x0040, 0x0041) AM_READ(ff_r) // service plug
	AM_RANGE(0x0050, 0x0051) AM_READ(ff_r) // test of internal battery
	AM_RANGE(0x0060, 0x0061) AM_READ(ff_r) // sum of analog outputs of ay2
	//AM_RANGE(0x0070, 0x0071) AM_READNOP // serial data in
	AM_RANGE(0x0f70, 0x0f7d) AM_WRITENOP
	AM_RANGE(0x0fe4, 0x0fff) AM_READNOP
	AM_RANGE(0x7fb0, 0x7fbf) AM_WRITE(cru_w)
	AM_RANGE(0x7fd0, 0x7fd1) AM_WRITE(oe_w)
ADDRESS_MAP_END

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

	UINT8 i,j;
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
					output().set_digit_value(j * 10 + i, BITSWAP16(segments, 8, 8, 8, 8, 8, 8, 0, 0, 1, 1, 2, 3, 4, 5, 6, 7));
				}
			}
		}
	}
}

void nsm_state::machine_reset()
{
	// Disable auto wait state generation by raising the READY line on reset
	static_cast<tms9995_device*>(machine().device("maincpu"))->set_ready(ASSERT_LINE);
}

static MACHINE_CONFIG_START( nsm, nsm_state )
	// CPU TMS9995, standard variant; no line connection
	MCFG_TMS99xx_ADD("maincpu", TMS9995, 11052000, nsm_map, nsm_io_map)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_nsm)

	/* Sound */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ay1", AY8912, 11052000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_SOUND_ADD("ay2", AY8912, 11052000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)
MACHINE_CONFIG_END

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

GAME(1985,  firebird,  0,  nsm,  nsm, driver_device, 0,  ROT0, "NSM", "Hot Fire Birds", MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
