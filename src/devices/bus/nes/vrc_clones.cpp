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

DEFINE_DEVICE_TYPE(NES_900218,    nes_900218_device,    "nes_900218",    "NES Cart 900218 PCB")
DEFINE_DEVICE_TYPE(NES_AX5705,    nes_ax5705_device,    "nes_ax5705",    "NES Cart AX5705 PCB")
DEFINE_DEVICE_TYPE(NES_CITYFIGHT, nes_cityfight_device, "nes_cityfight", "NES Cart City Fighter PCB")
DEFINE_DEVICE_TYPE(NES_SHUIGUAN,  nes_shuiguan_device,  "nes_shuiguan",  "NES Cart Shui Guan Pipe Pirate PCB")
DEFINE_DEVICE_TYPE(NES_TF1201,    nes_tf1201_device,    "nes_tf1201",    "NES Cart UNL-TF1201 PCB")


nes_900218_device::nes_900218_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_900218, tag, owner, clock)
{
}

nes_ax5705_device::nes_ax5705_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_AX5705, tag, owner, clock)
{
}

nes_cityfight_device::nes_cityfight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_CITYFIGHT, tag, owner, clock)
{
}

nes_shuiguan_device::nes_shuiguan_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_SHUIGUAN, tag, owner, clock)
{
}

nes_tf1201_device::nes_tf1201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_TF1201, tag, owner, clock)
{
}



void nes_900218_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 1;  // A1
	m_vrc_ls_prg_b = 0;  // A0
}

void nes_ax5705_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 1;  // A1
	m_vrc_ls_prg_b = 0;  // A0
}

void nes_cityfight_device::device_start()
{
	nes_konami_vrc4_device::device_start();

	// VRC4 pins 3 and 4
	m_vrc_ls_prg_a = 3;  // A3
	m_vrc_ls_prg_b = 2;  // A2
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

 Board BTL-900218

 Games: Lord of King (Astyanax pirate)

 This board has a VRC2 clone that has been altered to
 add a simple IRQ counter fixed to 1024 CPU cycles.

 NES 2.0: mapper 524

 In MAME: Supported.

 -------------------------------------------------*/

void nes_900218_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			m_irq_count++;
			set_irq_line(BIT(m_irq_count, 10) ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

void nes_900218_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("900218 write_h, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x7000) == 0x7000)
	{
		switch (offset & 0xc)
		{
			case 0x8:
				m_irq_enable = 1;
				break;
			case 0xc:
				m_irq_enable = 0;
				m_irq_count = 0;
				set_irq_line(CLEAR_LINE);
				break;
		}
	}
	else
		nes_konami_vrc4_device::write_h(offset, data);
}

/*-------------------------------------------------

 Board UNL-AX5705

 Games: Super Mario Bros. Pocker Mali (Crayon Shin-chan pirate hack)

 VRC4 clone with a few PRG/CHR address lines swapped.

 NES 2.0: mapper 530

 In MAME: Supported.

 -------------------------------------------------*/

void nes_ax5705_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("ax5705 write_h, offset: %04x, data: %02x\n", offset, data));

	offset = (offset & ~0x1000) | BIT(offset, 3) << 12;
	switch (offset & 0x7001)
	{
		case 0x0000: case 0x0001: case 0x2000: case 0x2001:
			data = bitswap<4>(data, 1, 2, 3, 0);
			break;
		case 0x3001: case 0x4001: case 0x5001: case 0x6001:
			data = bitswap<3>(data, 1, 2, 0);
			break;
	}
	nes_konami_vrc4_device::write_h(offset, data);
}

/*-------------------------------------------------

 UNL-CITYFIGHT

 Games: City Fighter IV

 VRC4 clone with simple 32K PRG banking (it's a minimal
 hack of Master Fighter II banking). More interestingly
 it adds a 4-bit PCM audio register, not emulated yet.

 NES 2.0: mapper 266

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_cityfight_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("unl_cityfight write_h, offset: %04x, data: %02x\n", offset, data));

	offset = (offset & ~0x6000) | bitswap<2>(offset, 13, 14) << 13;
	switch (offset & 0x7800)
	{
		case 0x0000:
		case 0x0800:
			break;  // PRG banking at $8000 ignored
		case 0x1000:
			prg32((data >> 2) & 0x03);
			if (!(offset & 3))  // $9000 is also VRC4 mirroring
				nes_konami_vrc4_device::write_h(offset, data);
			break;
		case 0x1800:
			LOG_MMC(("Extended Audio write, data %x!", data & 0x0f)); // pcmwrite(0x4011, (V & 0xf) << 3); (etabeta's original comment, FCEUX code)
			break;
		default:
			nes_konami_vrc4_device::write_h(offset, data);
			break;
	}
}

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
