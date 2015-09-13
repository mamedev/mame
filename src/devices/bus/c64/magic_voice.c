// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Magic Voice cartridge emulation

**********************************************************************/

/*

LA05-123 Pinout
---------------
                _____   _____
     NEXTP   1 |*    \_/     | 28  +5V
       PD0   2 |             | 27  _ROML2
       PD1   3 |             | 26  _ROML
       PD2   4 |             | 25  _I/O2
       PD3   5 |             | 24  _GAME
     CLEAR   6 |             | 23  PHI2
_RAM/EPROM   7 |  LA05-123   | 22  _ROMH2
       PB5   8 |  LA05-124   | 21  _ROMH
       PB6   9 |             | 20  CLOCK
     _6525  10 |             | 19  SDO
    _EPROM  11 |             | 18  NEXTS
      CA12  12 |             | 17  _DA/CA
      CA14  13 |             | 16  CA15
       GND  14 |_____________| 15  CA13


http://www.stefan-uhlmann.de/cbm/MVM/index.html

*/

/*

    TODO:

    - T6721A speech synthesis

*/

#include "magic_voice.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define T6721A_TAG      "u5"
#define MOS6525_TAG     "u2"
#define CMOS40105_TAG   "u1"

#define A12 BIT(offset, 12)
#define A13 BIT(offset, 13)
#define A14 BIT(offset, 14)
#define A15 BIT(offset, 15)
#define PB5 BIT(m_tpi_pb, 5)
#define PB6 BIT(m_tpi_pb, 6)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MAGIC_VOICE = &device_creator<c64_magic_voice_cartridge_device>;


//-------------------------------------------------
//  tpi6525_interface tpi_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::tpi_irq_w )
{
	m_slot->nmi_w(state);
}

READ8_MEMBER( c64_magic_voice_cartridge_device::tpi_pa_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       J1 _GAME
	    6       T6721 _EOS
	    7       FIFO DIR

	*/

	UINT8 data = 0;

	data |= m_exp->game_r(get_offset(m_ca), 1, 1, 1, 0) << 5;
	data |= m_vslsi->eos_r() << 6;
	data |= m_fifo->dir_r() << 7;

	return data;
}

WRITE8_MEMBER( c64_magic_voice_cartridge_device::tpi_pa_w )
{
	/*

	    bit     description

	    0       FIFO D0
	    1       FIFO D1
	    2       FIFO D2
	    3       FIFO D3
	    4       FIFO SI
	    5
	    6
	    7

	*/

	m_fifo->write(data & 0x0f);
	m_fifo->si_w(BIT(data, 4));
}

READ8_MEMBER( c64_magic_voice_cartridge_device::tpi_pb_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       J1 _EXROM

	*/

	UINT8 data = 0;

	data |= m_exp->exrom_r(get_offset(m_ca), 1, 1, 1, 0) << 7;

	return data;
}

WRITE8_MEMBER( c64_magic_voice_cartridge_device::tpi_pb_w )
{
	/*

	    bit     description

	    0       T6721 D0
	    1       T6721 D1
	    2       T6721 D2
	    3       T6721 D3
	    4       T6721 _WR
	    5       LA05-124 pin 8 (DA/CA)
	    6       LA05-124 pin 9 (passthru)
	    7

	*/

	if (!BIT(m_tpi_pb, 4) && BIT(data, 4))
	{
		m_vslsi->write(space, 0, data & 0x0f);
	}

	m_tpi_pb = data;
}

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::tpi_ca_w )
{
	m_tpi_pc6 = state;
}

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::tpi_cb_w )
{
	m_exrom = state;
}

//-------------------------------------------------
//  t6721_interface
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::phi2_w )
{
	if (state)
	{
		m_vslsi->di_w(m_pd & 0x01);

		m_pd >>= 1;
	}
}

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::dtrd_w )
{
	m_fifo->so_w(!state);

	m_pd = m_fifo->read();
}

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::apd_w )
{
	if (state)
	{
		m_fifo->reset();
		m_pd = 0;
	}
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_magic_voice )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_magic_voice )
	MCFG_DEVICE_ADD(MOS6525_TAG, TPI6525, 0)
	MCFG_TPI6525_OUT_IRQ_CB(WRITELINE(c64_magic_voice_cartridge_device, tpi_irq_w))
	MCFG_TPI6525_IN_PA_CB(READ8(c64_magic_voice_cartridge_device, tpi_pa_r))
	MCFG_TPI6525_OUT_PA_CB(WRITE8(c64_magic_voice_cartridge_device, tpi_pa_w))
	MCFG_TPI6525_IN_PB_CB(READ8(c64_magic_voice_cartridge_device, tpi_pb_r))
	MCFG_TPI6525_OUT_PB_CB(WRITE8(c64_magic_voice_cartridge_device, tpi_pb_w))
	MCFG_TPI6525_OUT_CA_CB(WRITELINE(c64_magic_voice_cartridge_device, tpi_ca_w))
	MCFG_TPI6525_OUT_CA_CB(WRITELINE(c64_magic_voice_cartridge_device, tpi_cb_w))
	MCFG_40105_ADD(CMOS40105_TAG, DEVWRITELINE(MOS6525_TAG, tpi6525_device, i3_w), NULL)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(T6721A_TAG, T6721A, XTAL_640kHz)
	MCFG_T6721A_EOS_HANDLER(DEVWRITELINE(MOS6525_TAG, tpi6525_device, i2_w))
	MCFG_T6721A_PHI2_HANDLER(DEVWRITELINE(DEVICE_SELF, c64_magic_voice_cartridge_device, phi2_w))
	MCFG_T6721A_DTRD_HANDLER(DEVWRITELINE(DEVICE_SELF, c64_magic_voice_cartridge_device, dtrd_w))
	MCFG_T6721A_APD_HANDLER(DEVWRITELINE(DEVICE_SELF, c64_magic_voice_cartridge_device, apd_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_C64_PASSTHRU_EXPANSION_SLOT_ADD()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_magic_voice_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_magic_voice );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_magic_voice_cartridge_device - constructor
//-------------------------------------------------

c64_magic_voice_cartridge_device::c64_magic_voice_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MAGIC_VOICE, "C64 Magic Voice cartridge", tag, owner, clock, "c64_magic_voice", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_vslsi(*this, T6721A_TAG),
	m_tpi(*this, MOS6525_TAG),
	m_fifo(*this, CMOS40105_TAG),
	m_exp(*this, C64_EXPANSION_SLOT_TAG),
	m_tpi_pb(0x60),
	m_tpi_pc6(1),
	m_pd(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_magic_voice_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_tpi_pb));
	save_item(NAME(m_tpi_pc6));
	save_item(NAME(m_pd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_magic_voice_cartridge_device::device_reset()
{
	m_tpi->reset();

	m_exrom = 1;

	m_tpi_pb = 0x60;
	m_tpi_pc6 = 1;
	m_pd = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_magic_voice_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && sphi2)
	{
		m_ca = offset;
		data = m_tpi->read(space, offset & 0x07);
	}

	if (PB6 && A13 && A15)
	{
		data = m_romh[(A14 << 13) | (offset & 0x1fff)];
	}

	int roml2 = !(!roml || (roml && !PB5 && A12 && A13 && !A14 && A15));
	int romh2 = !((!romh && !PB6) || (!PB5 && A12 && A13 && !A14 && !A15));

	data = m_exp->cd_r(space, get_offset(offset), data, sphi2, ba, roml2, romh2, io1, 1);

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_magic_voice_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && sphi2)
	{
		m_tpi->write(space, offset & 0x07, data);
	}

	int roml2 = !(!roml || (roml && !PB5 && A12 && A13 && !A14 && A15));
	int romh2 = !((!romh && !PB6) || (!PB5 && A12 && A13 && !A14 && !A15));

	m_exp->cd_w(space, get_offset(offset), data, sphi2, ba, roml2, romh2, io1, 1);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_magic_voice_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return !((m_tpi_pc6 && sphi2) || (!m_tpi_pc6 && sphi2 && !PB5 && A12 && A13 && !A14));
}


//-------------------------------------------------
//  get_offset -
//-------------------------------------------------

offs_t c64_magic_voice_cartridge_device::get_offset(offs_t offset)
{
	if (!PB5 && A12 && A13 && !A14)
	{
		offset = ((m_tpi_pb & 0x0f) << 12) | (offset & 0xfff);
	}

	return offset;
}
