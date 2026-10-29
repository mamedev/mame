// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for PCBs mostly based on discrete components

 iNES mappers: 11, 38, 70, 87, 111, 152

 Here we emulate the following PCBs

 * PCB with IC 74x161x161x32 [mapper 70 & 152]
 * PCB with IC 74x139x74 [mapper 87]
 * PCB with IC 74x377 [mapper 11]
 * PCB with IC 74x161x138 [mapper 38]
 * GTROM (Cheapocabra) with IC 74x377, flash PRG-ROM and PPU RAM [mapper 111]

 ***********************************************************************************************************/


#include "emu.h"
#include "discrete.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_74X161X161X32, nes_74x161x161x32_device, "nes_74x161",      "NES Cart Discrete Logic (74*161/161/32) PCB")
DEFINE_DEVICE_TYPE(NES_74X139X74,     nes_74x139x74_device,     "nes_74x139",      "NES Cart Discrete Logic (74*139/74) PCB")
DEFINE_DEVICE_TYPE(NES_74X377,        nes_74x377_device,        "nes_74x377",      "NES Cart Discrete Logic (74*377) PCB")
DEFINE_DEVICE_TYPE(NES_74X161X138,    nes_74x161x138_device,    "nes_bitcorp_dis", "NES Cart Discrete Logic (74*161/138) PCB")
DEFINE_DEVICE_TYPE(NES_GTROM,         nes_gtrom_device,         "nes_gtrom",       "NES Cart Discrete Logic (GTROM) PCB")


nes_74x161x161x32_device::nes_74x161x161x32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X161X161X32, tag, owner, clock)
{
}

nes_74x139x74_device::nes_74x139x74_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X139X74, tag, owner, clock)
{
}

nes_74x377_device::nes_74x377_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X377, tag, owner, clock)
{
}

nes_74x161x138_device::nes_74x161x138_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X161X138, tag, owner, clock)
{
}

nes_gtrom_device::nes_gtrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_GTROM, tag, owner, clock), device_nvram_interface(mconfig, *this)
{
}




void nes_74x161x161x32_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_gtrom_device::device_start()
{
	nes_nrom_device::device_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_flash_step));
	save_item(NAME(m_flash_cmd));
}

void nes_gtrom_device::update_banks()
{
	prg32(BIT(m_reg, 0, 4));
	chr8(BIT(m_reg, 4), CHRRAM);

	// the nametables are one of the last two 8 KiB pages of the board's 32 KiB of PPU RAM: all four screens are separate RAM
	// (the $3000-$3EFF "bonus RAM" is not separate here, it mirrors the nametables as MAME's PPU map does)
	const int base = 16 + (BIT(m_reg, 5) ? 8 : 0);
	for (int page = 0; page < 4; page++)
		set_nt_page(page, VRAM_NT, base + page, 1);
}

void nes_gtrom_device::pcb_reset()
{
	m_reg = 0;
	m_flash_step = 0;
	update_banks();
}

// SST39SF040 command sequences (NESdev GTROM "Self-flashing"): the flash address is the current 32 KiB PRG page (register
// bits 0-3) with CPU A0-A14, so the unlock addresses $D555 and $AAAA are the same in every page. Program and erase
// complete at once, so polling reads return the final data
void nes_gtrom_device::flash_w(offs_t offset, u8 data)
{
	const u32 size = m_prg_chunks * 0x4000;
	const u32 addr = ((BIT(m_reg, 0, 4) << 15) | (offset & 0x7fff)) & (size - 1);
	const u16 cmd = addr & 0x7fff;
	u8 *prg = get_prg_base();

	if (data == 0xf0)
	{
		m_flash_step = 0;   // reset / exit
		return;
	}
	switch (m_flash_step)
	{
		case 0: m_flash_step = (cmd == 0x5555 && data == 0xaa) ? 1 : 0; break;
		case 1: m_flash_step = (cmd == 0x2aaa && data == 0x55) ? 2 : 0; break;
		case 2:
			if (cmd == 0x5555 && (data == 0xa0 || data == 0x80))
			{
				m_flash_cmd = data;
				m_flash_step = (data == 0xa0) ? 3 : 4;
			}
			else
				m_flash_step = 0;
			break;
		case 3:   // byte program: a flash cell can only change 1s to 0s
			prg[addr] &= data;
			m_flash_step = 0;
			break;
		case 4: m_flash_step = (cmd == 0x5555 && data == 0xaa) ? 5 : 0; break;
		case 5: m_flash_step = (cmd == 0x2aaa && data == 0x55) ? 6 : 0; break;
		case 6:
			if (data == 0x30)   // 4 KiB sector erase
				std::fill_n(prg + (addr & ~0xfff), 0x1000, 0xff);
			else if (data == 0x10 && cmd == 0x5555)   // chip erase
				std::fill_n(prg, size, 0xff);
			m_flash_step = 0;
			break;
		default: m_flash_step = 0; break;
	}
}

bool nes_gtrom_device::nvram_read(util::read_stream &file)
{
	const u32 size = m_prg_chunks * 0x4000;
	auto const [err, actual] = util::read(file, get_prg_base(), size);
	return !err && (actual == size);
}

bool nes_gtrom_device::nvram_write(util::write_stream &file)
{
	const u32 size = m_prg_chunks * 0x4000;
	auto const [err, actual] = util::write(file, get_prg_base(), size);
	return !err;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Discrete Logic board IC 74x161x161x32

 There are two variants (one with hardwired mirroring, the
 other with a mirroring control), making necessary two distinct
 mappers & pcb_id

 The register is at 0x8000-0xffff and the last 16K of PRG is fixed at 0xc000.
 Mapper 70 is PPPP CCCC (16K PRG, 8K CHR). Mapper 152 is MPPP CCCC, where M
 selects the one-screen mirroring page and PRG has only 3 bits.
 The real boards are subject to bus conflict.

 iNES: mappers 70 & 152

 -------------------------------------------------*/

// there are two 'variants' depending on hardwired or mapper ctrl mirroring
void nes_74x161x161x32_device::write_h(offs_t offset, u8 data)
{
	LOG("74x161x161x32 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8(data & 0x0f, CHRROM);
	prg16_89ab((data >> 4) & (m_pcb_ctrl_mirror ? 0x07 : 0x0f));
}

/*-------------------------------------------------

 Discrete Logic board IC 74x139x74 by Konami & Jaleco

 The register at 0x6000-0x7fff is xxxx xxLH and selects the 8K CHR bank
 as H*2 + L (the two bits are swapped). There are no bus conflicts.

 iNES: mapper 87

 -------------------------------------------------*/

void nes_74x139x74_device::write_m(offs_t offset, u8 data)
{
	LOG("74x139x74 write_m, offset: %04x, data: %02x\n", offset, data);

	chr8(bitswap<2>(data, 0, 1), CHRROM);
}

/*-------------------------------------------------

 Discrete Logic board IC 74x377 by Color Dreams / Nina-007 emulation

 Games: many Color Dreams and Wisdom Tree titles

 The register is CCCC LLPP (CHR bank, lockout defeat, PRG bank). The real boards
 are subject to bus conflict: NES 2.0 submapper 2 enables them and submapper 1
 disables them, while iNES images have none, because of the Free Fall prototype.

 iNES: mapper 11

 In MAME: Supported

 Note: bit2 & bit3 are actually related to CIC lockout
 defeating, and real Color Dreams titles use only
 bit0 & bit1 for PRG switching. Our usage allows for support
 of extended PRG games (e.g. homebrew)

 -------------------------------------------------*/

void nes_74x377_device::write_h(offs_t offset, u8 data)
{
	LOG("74x377 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict, but not the prototype of Secret Scout, which actually breaks in case of conflict...
	data = account_bus_conflict(offset, data);

	chr8(data >> 4, m_chr_source);
	prg32(data & 0x0f);
}

/*-------------------------------------------------

 Discrete Logic board IC 74x161x138

 Games: Crime Busters

 The register is xxxx CCPP and is decoded only at 0x7000-0x7fff. There are no
 bus conflicts and mirroring is hardwired. We do not mask the bank numbers, to
 allow for oversize ROMs like other emulators do.

 iNES: mapper 38

 -------------------------------------------------*/

void nes_74x161x138_device::write_m(offs_t offset, u8 data)
{
	LOG("74x161x138 write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x1000)    // the register is not decoded at 0x6000-0x6fff
		return;

	chr8(data >> 2, CHRROM);
	prg32(data);
}

/*-------------------------------------------------

 Discrete Logic board GTROM (Cheapocabra), by Membler Industries

 Games: Anguna Zero, Beyond the Pins, Storied Sword...

 The register is GRNC PPPP: PRG-ROM page (32 KiB), CHR-RAM page, nametable page, and the red and green LEDs. It is latched
 when A15 is low and A14 and A12 are high, so it is at 0x5000-0x5fff and 0x7000-0x7fff alike (reading it would latch
 the open bus value, which is not emulated: a correct program keeps a copy of the value in RAM). There are no bus
 conflicts. The 32 KiB of PPU RAM holds the CHR-RAM pages (the first two 8 KiB) and the nametable pages (the last two),
 so the board is always four-screen. PRG-ROM is an SST39SF040 flash chip, self-flashable when the battery flag is set.
 The LEDs are not emulated.

 iNES: mapper 111

 -------------------------------------------------*/

void nes_gtrom_device::write_l(offs_t offset, u8 data)
{
	const offs_t addr = 0x4100 + offset;
	LOG("gtrom write_l, addr: %04x, data: %02x\n", addr, data);

	if ((addr & 0x5000) == 0x5000)
	{
		m_reg = data;
		update_banks();
	}
}

void nes_gtrom_device::write_m(offs_t offset, u8 data)
{
	const offs_t addr = 0x6000 + offset;
	LOG("gtrom write_m, addr: %04x, data: %02x\n", addr, data);

	if ((addr & 0x5000) == 0x5000)
	{
		m_reg = data;
		update_banks();
	}
}

void nes_gtrom_device::write_h(offs_t offset, u8 data)
{
	LOG("gtrom write_h, offset: %04x, data: %02x\n", offset, data);

	if (m_flashable)
		flash_w(offset, data);
}
