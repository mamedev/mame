/**********************************************************************

    Commodore Magic Voice cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

	- wow graphics broken
	- T6721A speech synthesis

*/

#include "magic_voice.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define T6721A_TAG  	"u5"
#define MOS6525_TAG 	"u2"
#define CMOS40105_TAG 	"u1"



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

	data |= m_exp->game_r(get_offset(0xdf80), 1, 0, 1, 0) << 5;
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

	data |= m_exp->exrom_r(get_offset(0xdf81), 1, 0, 1, 0) << 7;

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

	m_vslsi_data = data & 0x0f;

	if (!BIT(data, 4))
	{
		m_vslsi->write(space, 0, m_vslsi_data);
	}

	m_da_ca = BIT(data, 5);
	m_pb6 = BIT(data, 6);
}

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::tpi_ca_w )
{
	m_eprom = state;
}

WRITE_LINE_MEMBER( c64_magic_voice_cartridge_device::tpi_cb_w )
{
	m_exrom = state;
}

static const tpi6525_interface tpi_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_magic_voice_cartridge_device, tpi_irq_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_magic_voice_cartridge_device, tpi_pa_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_magic_voice_cartridge_device, tpi_pa_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_magic_voice_cartridge_device, tpi_pb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c64_magic_voice_cartridge_device, tpi_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_magic_voice_cartridge_device, tpi_ca_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_magic_voice_cartridge_device, tpi_cb_w),
};


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
	MCFG_TPI6525_ADD(MOS6525_TAG, tpi_intf)
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
	m_eprom(1),
	m_da_ca(1),
	m_pb6(1),
	m_vslsi_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_magic_voice_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_magic_voice_cartridge_device::device_reset()
{
	m_tpi->reset();

	m_exrom = 1;

	m_eprom = 1;
	m_da_ca = 1;
	m_pb6 = 1;
	m_vslsi_data = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_magic_voice_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && sphi2)
	{
		data = m_tpi->read(space, offset & 0x07);
	}

	if (m_eprom && m_pb6 && sphi2 && ((offset >= 0xa000 && offset < 0xc000) || (offset >= 0xe000)))
	{
		data = m_romh[(BIT(offset, 14) << 13) | (offset & 0x1fff)];
	}

	if (!m_pb6)
	{
		data = m_exp->cd_r(space, get_offset(offset), data, sphi2, ba, roml, romh, io1, 1);
	}

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

	if (!m_pb6)
	{
		m_exp->cd_w(space, get_offset(offset), data, sphi2, ba, roml, romh, io1, 1);
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_magic_voice_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	return !(m_eprom && ((offset >= 0xa000 && offset < 0xc000) || (offset >= 0xe000)));
}


//-------------------------------------------------
//  get_offset -
//-------------------------------------------------

offs_t c64_magic_voice_cartridge_device::get_offset(offs_t offset)
{
	if (!m_da_ca)
	{
		offset = (m_vslsi_data << 12) | (offset & 0xfff);
	}

	return offset;
}
