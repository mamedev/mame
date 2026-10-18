// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD MMU SuperAdapter emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |-------------------------|
    | |-----------|  |-----|  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  | UC1 |  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  |-----|  |
    | |    U1     |  |-----|  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  | UC2 |  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |           |  |     |  |
    | |-----------|  |-----|  |
    | |-----------|    R1     |
    | |  GAL16V8  |    R2     |
    | |-----------|    C3     |
    |-----------------|||||---|
                      |||||
                      to 8502

    U1      - 48-pin DIP socket for the 8722 MMU
    UC1     - CMD MMU128-UC1-A (C)1998
    UC2     - CMD MMU128-UC2-A (C)1998
    GAL16V8 - Lattice GAL16V8B-15LP
    R1,R2   - resistors
    C3      - ceramic capacitor
    leads   - IC test clips to the 8502 (U6)

    Clip    8502 pin    Signal
    ------  --------    ------
    red     39          R/W
    yellow  30          P0
    blue    29          P1
    green   28          P2
    black   24          P6

    The board plugs into the C128 MMU socket through two 24-pin header
    strips on its underside.

*/

#include "emu.h"
#include "supercpu128_mmu.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CMD_MMU_SUPERADAPTER, cmd_mmu_superadapter_device, "cmd_mmu_superadapter", "CMD MMU SuperAdapter")


//-------------------------------------------------
//  ROM( cmd_mmu_superadapter )
//-------------------------------------------------

ROM_START( cmd_mmu_superadapter )
	ROM_REGION( 0x100, "gal", 0 )
	ROM_LOAD( "mmu128-uc1-a.uc1", 0x000, 0x001, NO_DUMP )
	ROM_LOAD( "mmu128-uc2-a.uc2", 0x000, 0x001, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cmd_mmu_superadapter_device::device_rom_region() const
{
	return ROM_NAME( cmd_mmu_superadapter );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cmd_mmu_superadapter_device::cmd_mmu_superadapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c128_mmu_mos8722_device(mconfig, CMD_MMU_SUPERADAPTER, tag, owner, clock),
	m_dma(false),
	m_drive(false),
	m_latch(0x07)
{
}

void cmd_mmu_superadapter_device::device_start()
{
	c128_mmu_mos8722_device::device_start();

	save_item(NAME(m_dma));
	save_item(NAME(m_drive));
	save_item(NAME(m_latch));
}

void cmd_mmu_superadapter_device::device_reset()
{
	m_latch = 0x07;

	update();
}

void cmd_mmu_superadapter_device::update()
{
	if (m_dma)
		m_slot->port_w(m_latch & 0x07);

	if (m_dma != m_drive)
	{
		m_drive = m_dma;
		m_slot->drive_w(m_drive);
	}
}

uint8_t cmd_mmu_superadapter_device::read(offs_t offset, uint8_t data)
{
	if (m_dma && (offset == 0xd1ff))
		data = (m_latch & ~0x40) | (m_slot->caps_r() << 6);

	return c128_mmu_mos8722_device::read(offset, data);
}

void cmd_mmu_superadapter_device::write(offs_t offset, uint8_t data)
{
	if (m_dma && (offset == 0xd1ff))
	{
		m_latch = data;

		update();
	}

	c128_mmu_mos8722_device::write(offset, data);
}

void cmd_mmu_superadapter_device::dma_w(int state)
{
	m_dma = state;

	update();
}
