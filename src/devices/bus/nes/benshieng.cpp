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


nes_benshieng_device::nes_benshieng_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BENSHIENG, tag, owner, clock), m_dipsetting(0)
{
}




void nes_benshieng_device::device_start()
{
	common_start();
	save_item(NAME(m_dipsetting));
}

void nes_benshieng_device::pcb_reset()
{
	for (int i = 0; i < 4; i++)
	{
		prg8_x(i, 0x0f);
		chr2_x(2 * i, 0x00, CHRROM);
	}

	m_dipsetting = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 BMC-BS-5

 Games: a few 4 in 1 multicarts

 NES 2.0: mapper 286

 -------------------------------------------------*/

void nes_benshieng_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("benshieng write_h, offset: %04x, data: %02x\n", offset, data));
//  m_mmc_dipsetting = ioport("CARTDIPS")->read();

	u8 bank = BIT(offset, 10, 2);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
			chr2_x(2 * bank, offset & 0x1f, CHRROM);
			break;
		case 0x2000:
		case 0x3000:
			if (BIT(offset, m_dipsetting + 4))  // m_dipsetting is always zero atm, given we have no way to add cart-based DIPs
				prg8_x(bank, offset & 0x0f);
			break;
	}
}
