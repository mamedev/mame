// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Sealie Computing and related PCBs


 Here we emulate the following homebrew PCBs

 * SEALIE RET-CUFROM [mapper 29]
 * SEALIE DPCMcart [mapper 409]
 * SEALIE UNROM 512 [mapper 30]
 * SEALIE 8BIT XMAS [mapper 30]

 ***********************************************************************************************************/


#include "emu.h"
#include "sealie.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_8BITXMAS, nes_8bitxmas_device, "nes_8bitxmas", "NES Cart Sealie 8BIT XMAS PCB")
DEFINE_DEVICE_TYPE(NES_CUFROM,   nes_cufrom_device,   "nes_cufrom",   "NES Cart Sealie RET-CUFROM PCB")
DEFINE_DEVICE_TYPE(NES_DPCMCART, nes_dpcmcart_device, "nes_dpcmcart", "NES Cart Sealie DPCMcart PCB")
DEFINE_DEVICE_TYPE(NES_UNROM512, nes_unrom512_device, "nes_unrom512", "NES Cart Sealie UNROM 512 PCB")


nes_cufrom_device::nes_cufrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_CUFROM, tag, owner, clock)
{
}

nes_dpcmcart_device::nes_dpcmcart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_DPCMCART, tag, owner, clock)
{
}

nes_unrom512_device::nes_unrom512_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
{
}

nes_unrom512_device::nes_unrom512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_unrom512_device(mconfig, NES_UNROM512, tag, owner, clock)
{
}

nes_8bitxmas_device::nes_8bitxmas_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_unrom512_device(mconfig, NES_8BITXMAS, tag, owner, clock), m_led(0)
{
}



void nes_cufrom_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRRAM);
}

void nes_dpcmcart_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRRAM);
}

void nes_unrom512_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRRAM);
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_8bitxmas_device::device_start()
{
	nes_unrom512_device::device_start();
	save_item(NAME(m_led));

	m_bus_conflict = false;
}

void nes_8bitxmas_device::pcb_reset()
{
	nes_unrom512_device::pcb_reset();

	m_led = 0;
	update_led();
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Sealie RET-CUFROM board

 Games: Glider (only?)

 This homebrew mapper supports 8x16k PRG banks at 0x8000,
 8k WRAM at 0x6000, and 4x8k VRAM banks. PRG is stored on
 flash ROM, though unlike mapper 30 it doesn't appear to
 be self-flashable, only through an external tool.

 iNES: mapper 29

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cufrom_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("cufrom write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(BIT(data, 2, 3));
	chr8(data & 0x03, CHRRAM);
}

/*-------------------------------------------------

 Sealie DPCMcart board

 Games: A Winner is You

 This homebrew mapper supports a whopping 64MB which
 is paged in 16K chucks at 0x8000. 0xc000 is fixed.

 NES 2.0: mapper 409

 In MAME: Partially supported.

 TODO: Controls other than 'next track' don't work.

 -------------------------------------------------*/

void nes_dpcmcart_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("dpcmcart write_h, offset: %04x, data: %02x\n", offset, data));
	prg16_89ab(offset & 0x0fff);
}

/*-------------------------------------------------

 Sealie UNROM 512 board

 Games: Battle Kid 1 & 2, E.T., many more

 This board has several variations and jumper configurations.
 Currently we only support the Sealie nonflashable config
 with 32x16k PRG banks at 0x8000, 4x8k VRAM, and three
 mirroring configs (H, V, or PCB selected 1-screen modes).

 iNES: mapper 30

 In MAME: Preliminary partial support.

 -------------------------------------------------*/

void nes_unrom512_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("unrom512 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict in its nonflashable configuration
	data = account_bus_conflict(offset, data);

	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	prg16_89ab(data & 0x1f);
	chr8(BIT(data, 5, 2), CHRRAM);
}

/*-------------------------------------------------

 Sealie 8BIT XMAS revD board

 Games: 8-bit Xmas 2012-2016 and 2018-2021?

 This board is a variant of UNROM512 with 16 LEDs
 in 4 colors, blue, yellow, green, red, which are
 controlled in pairs by each byte written to
 0x8000-0xbfff. Bits are [BYGR bygr] where bygr
 control LEDs 1,2 and BYGR control LEDs 3,4. On
 the 8BIT XMAS revD 2012 board LEDS are arranged:

  _______________________________
 | RED2                     GRN3 |
 |                               |
 | YEL2                     BLU3 |
 |                               |
 | BLU1                     YEL4 |
 |                               |
 | GRN1                     RED4 |
 |                               |
 | RED1                     GRN4 |
 |                               |
 | YEL1                     BLU4 |
  --                           --
    |   GRN2 BLU2 YEL3 RED3   |
    |                         |

 iNES: mapper 30

 In MAME: Preliminary partial support.

 -------------------------------------------------*/

void nes_8bitxmas_device::update_led()
{
	// TODO: add artwork
}

void nes_8bitxmas_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("8bitxmas write_h, offset: %04x, data: %02x\n", offset, data));

	if (BIT(offset, 14))
		nes_unrom512_device::write_h(offset, data);
	else if (m_led != data)
	{
		m_led = data;
		update_led();
	}
}
