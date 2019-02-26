// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Benshieng PCBs


 Here we emulate the following PCBs used by Benshieng multigame carts series

 ***********************************************************************************************************/


#include "emu.h"
#include "benshieng.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_BENSHIENG, nes_benshieng_device, "nes_benshieng", "NES Cart Benshieng PCB")


nes_benshieng_device::nes_benshieng_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BENSHIENG, tag, owner, clock)
	, m_dipsetting(0)
{
}




void nes_benshieng_device::device_start()
{
	common_start();
	save_item(NAME(m_dipsetting));
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_benshieng_device::pcb_reset()
{
	m_dipsetting = 0;

	m_mmc_prg_bank[0] = 0xff;
	m_mmc_prg_bank[1] = 0xff;
	m_mmc_prg_bank[2] = 0xff;
	m_mmc_prg_bank[3] = 0xff;
	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
	update_banks();
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 BMC-BS-5

 Games: a few 4 in 1 multicarts

 -------------------------------------------------*/

void nes_benshieng_device::update_banks()
{
	prg8_89(m_mmc_prg_bank[0]);
	prg8_ab(m_mmc_prg_bank[1]);
	prg8_cd(m_mmc_prg_bank[2]);
	prg8_ef(m_mmc_prg_bank[3]);
	chr2_0(m_mmc_vrom_bank[0], CHRROM);
	chr2_2(m_mmc_vrom_bank[1], CHRROM);
	chr2_4(m_mmc_vrom_bank[2], CHRROM);
	chr2_6(m_mmc_vrom_bank[3], CHRROM);
}

void nes_benshieng_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t helper = (offset & 0xc00) >> 10;
	LOG_MMC(("benshieng write_h, offset: %04x, data: %02x\n", offset, data));
//  m_mmc_dipsetting = ioport("CARTDIPS")->read();

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_mmc_vrom_bank[helper] = offset & 0x1f;
			break;
		case 0x2000:
			if (BIT(offset, m_dipsetting + 4))  // mmc_dipsetting is always zero atm, given we have no way to add cart-based DIPs
				m_mmc_prg_bank[helper] = offset & 0x0f;
			break;
	}
	update_banks();
}
