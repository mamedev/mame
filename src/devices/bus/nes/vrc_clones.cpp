// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Konami VRC clone PCBs


 Here we emulate several pirate PCBs based on VRC2/4 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "vrc_clones.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SHUIGUAN, nes_shuiguan_device, "nes_shuiguan", "NES Cart Shui Guan Pipe Pirate PCB")
DEFINE_DEVICE_TYPE(NES_TF1201,   nes_tf1201_device,   "nes_tf1201",   "NES Cart UNL-TF1201 PCB")


nes_shuiguan_device::nes_shuiguan_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_SHUIGUAN, tag, owner, clock)
{
}

nes_tf1201_device::nes_tf1201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_TF1201, tag, owner, clock)
{
}



void nes_shuiguan_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_reg));

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A3
	m_vrc_ls_prg_b = 2;  // A2
}

void nes_shuiguan_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();
	m_reg = 0;
}

void nes_tf1201_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 0;  // A0
	m_vrc_ls_prg_b = 1;  // A1
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 BTL-SHUIGUANPIPE

 Games: Shui Guan Pipe (Gimmick Pirate)

 VRC4 clone that differs in its PRG banking only.

 iNES: mapper 183

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_shuiguan_device::read_m(offs_t offset)
{
//	LOG_MMC(("shuiguan read_m, offset: %04x\n", offset));
	return m_prg[(m_reg * 0x2000 + offset) & (m_prg_size - 1)];
}

void nes_shuiguan_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("shuiguan write_m, offset: %04x, data: %02x\n", offset, data));
	if ((offset & 0x1800) == 0x800)
		m_reg = offset & 0x1f;
}

void nes_shuiguan_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("shuiguan write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x0000:
		case 0x1000:
			break;  // writes to $8000 and $9000 ignored
		case 0x0800:
			prg8_89(data);
			break;
		case 0x2000:
			prg8_cd(data);
			break;
		case 0x2800:
			prg8_ab(data);
			break;
		default:
			nes_konami_vrc4_device::write_h(offset, data);
			break;
	}
}

/*-------------------------------------------------

 UNL-TF1201

 Games: Leathal Weapon (Leathal Enforcers clone)

 VRC4 clone where the only known difference is in the
 IRQ behavior. This clone does not copy the IRQ reload
 bit to the enable bit on writes to IRQ acknowledge.

 NES 2.0: mapper 298

 In MAME: Supported.

 -------------------------------------------------*/

void nes_tf1201_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("unl_tf1201 write_h, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x7003) == 0x7003)
		set_irq_line(CLEAR_LINE);
	else
		nes_konami_vrc4_device::write_h(offset, data);
}


/*-------------------------------------------------

 MULTIGAME CARTS BASED ON VRC

 -------------------------------------------------*/
