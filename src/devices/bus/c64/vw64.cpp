// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VizaWrite 64 cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|      LS10    LS122  C   |
    |=|                  R      |
    |=|                         |
    |=|                         |
    |=|      ROM0  ROM2  ROM1   |
    |=|                         |
    |=|                         |
    |=|                         |
    |===========================|

    ROM0    - Mitsubishi M5L2764K 8Kx8 EPROM "U"
    ROM1    - Mitsubishi M5L2764K 8Kx8 EPROM "3"
    ROM2    - Mitsubishi M5L2764K 8Kx8 EPROM "2" (located on solder side)
    R       - 56K
    C       - 47uF

*/

#include "emu.h"
#include "vw64.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define UNSCRAMBLE_ADDRESS(_offset) \
	bitswap<16>(_offset,15,14,13,12,6,2,8,10,11,9,7,5,4,3,1,0)

#define UNSCRAMBLE_DATA(_data) \
	bitswap<8>(_data,7,6,0,5,1,4,2,3)


// 74LS122 tW=0.45*R*C = 1.1844s
#define TIMER_PERIOD    1184



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_VW64, c64_vizawrite_cartridge_device, "c64_vizawrite", "VizaWrite 64")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_vizawrite_cartridge_device - constructor
//-------------------------------------------------

c64_vizawrite_cartridge_device::c64_vizawrite_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_VW64, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this), m_game_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_vizawrite_cartridge_device::device_start()
{
	// allocate timer
	m_game_timer = timer_alloc();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_vizawrite_cartridge_device::device_reset()
{
	m_game = 0;

	m_game_timer->adjust(attotime::from_msec(TIMER_PERIOD), 0);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c64_vizawrite_cartridge_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_game = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_vizawrite_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = (m_game << 13) | (offset & 0x1fff);
		data = UNSCRAMBLE_DATA(m_roml[UNSCRAMBLE_ADDRESS(addr)]);
	}
	else if (!romh)
	{
		offs_t addr = offset & 0x1fff;
		data = UNSCRAMBLE_DATA(m_romh[UNSCRAMBLE_ADDRESS(addr)]);
	}

	return data;
}
