// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Triton QD TDOS cartridge emulation

**********************************************************************/

/*

PCB Layout
----------

XM-2206-A (top)
XM-2205-A (bottom)

            |===========================|
            |            CN4            |
            |                           |
            |      ULA                  |
            |                           |
            |      6.5MHz               |
            |                           |
            |      SSDA                 |
            |                 LS175     |
            |                 LS367     |
            |            CN3            |
            |=========|||||||||=========|
                      |||||||||
        |=============|||||||||============|
        |                CN2               |
        |       LS00     LS02     LS138    |
|=======|                                  |
|=|             LS245        ROM           |
|=|                                        |
|=|                                        |
|=|                                     CN1|
|=|                                        |
|=|                                        |
|=|                                        |
|=|                             SW1        |
|==========================================|

ROM    - Hitachi HN482764G 8Kx8 EPROM "TDOS 1.2"
ULA    - Ferranti ULA5RB073E1 XZ-2085-1 40-pin custom ULA
SSDA   - Motorola MC68A52P SSDA
CN1    - C64 expansion connector (pass-thru)
CN2,3  - 18x1 flat ribbon cable to other PCB
CN4    - 9 wire cable to Mitsumi Quick Disk 3" drive
SW1    - cartridge on/off switch


Flat ribbon cable pinout
------------------------
1   D7
2   D6
3   D5
4   D4
5   D3
6   LS00 4Y -> LS367 _G1
7   phi2
8   LS00 3Y -> LS175 CP
9   D2
10  D1
11  D0
12  RESET
13  A0
14  R/_W
15  GND
16  +5V
17  LS138 O2 -> LS367 _G1
18  LS138 O0 -> SSDA _CS


Drive cable pinout
------------------
1   WP      Write Protected
2   WD      Write Data
3   WG      Write Gate
4   MO      Motor
5   RD      Read Data
6   RY      Ready
7   MS      Media Set
8   RS      Reset
9   +5V
10  GND


ULA pinout
----------
            _____   _____
         1 |*    \_/     | 40
         2 |             | 39
         3 |             | 38
         4 |             | 37
         5 |             | 36  GND
         6 |             | 35
    RD   7 |             | 34
   _D5   8 |             | 33
   RxC   9 |             | 32
   RxD  10 |  XZ-2085-1  | 31
        11 |             | 30
    WD  12 |             | 29
   TxC  13 |             | 28
   TxD  14 |             | 27
    D7  15 |             | 26
    WG  16 |             | 25  +5V
        17 |             | 24  XTAL2
        18 |             | 23  XTAL1
    RS  19 |             | 22  GND
        20 |_____________| 21


BASIC commands (SYS 32768 to activate)
--------------------------------------
@Dn
@Format
@Dir
@Save
@ASave
@Write
@Load
@Run
@Aload
@Kill
@Quit
@ACopy
@CassCopy

*/

#include "tdos.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC68A52P_TAG        "mc6852"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_TDOS = &device_creator<c64_tdos_cartridge_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_multiscreen )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_tdos )
	MCFG_DEVICE_ADD(MC68A52P_TAG, MC6852, XTAL_6_5MHz)

	MCFG_C64_PASSTHRU_EXPANSION_SLOT_ADD()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_tdos_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_tdos );
}


//-------------------------------------------------
//  INPUT_PORTS( c64_tdos )
//-------------------------------------------------

static INPUT_PORTS_START( c64_tdos )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_tdos_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_tdos );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_tdos_cartridge_device - constructor
//-------------------------------------------------

c64_tdos_cartridge_device::c64_tdos_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_TDOS, "C64 TDOS cartridge", tag, owner, clock, "c64_tdos", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ssda(*this, MC68A52P_TAG),
	m_exp(*this, C64_EXPANSION_SLOT_TAG),
	m_sw1(*this, "SW1"), m_enabled(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_tdos_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_tdos_cartridge_device::device_reset()
{
	m_ssda->reset();
	//m_ula->reset();
	//flip-flop reset

	m_enabled = m_sw1->read();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_tdos_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	data = m_exp->cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);

	if (m_enabled && !roml)
	{
		data = m_roml[offset & 0x1fff];
	}

	if (m_enabled && !io2 && sphi2)
	{
		switch ((offset >> 1) & 0x7f)
		{
		case 0:
			data = m_ssda->read(space, offset & 0x01);
			break;

		case 1:
			break;

		case 2:
			/*

			    bit     description

			    0
			    1
			    2
			    3
			    4
			    5       drive MS
			    6       drive WP
			    7       drive RY

			*/
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_tdos_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	m_exp->cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);

	if (m_enabled && !io2 && sphi2)
	{
		switch ((offset >> 1) & 0x7f)
		{
		case 0:
			m_ssda->write(space, offset & 0x01, data);
			break;

		case 1:
			/*

			    bit     description

			    0
			    1
			    2
			    3
			    4
			    5       ULA pin 8, inverted
			    6       drive MO
			    7       ULA pin 15

			*/
			break;

		case 2:
			break;
		}
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_tdos_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_enabled ? 1 : m_exp->game_r(offset, sphi2, ba, rw, m_slot->hiram());
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_tdos_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_enabled ? 0 : m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->hiram());
}
