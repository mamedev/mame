// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for TXC PCBs

 Here we emulate the following PCBs

 * TXC 01-22000-400 [mapper 36]
 * TXC 01-22003-400 / 01-22111-100 / 01-22270-000 [mapper 132]
 * Super Mega P-4070 (JV001) [mapper 172]
 * Idea-Tek ET-xx [mapper 173]
 * BxROM with 8 KiB WRAM, TXC-MXMDHTWO [mapper 241]

 ***********************************************************************************************************/


#include "emu.h"
#include "txc.h"

#include "speaker.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_TXC_22211,      nes_txc_22211_device,     "nes_txc_22211",   "NES Cart TXC 22211 PCB")
DEFINE_DEVICE_TYPE(NES_TXC_DUMARACING, nes_txc_dumarc_device,    "nes_txc_dumarc",  "NES Cart Super Mega P-4070 PCB (JV001)")
DEFINE_DEVICE_TYPE(NES_TXC_MJBLOCK,    nes_txc_mjblock_device,   "nes_txc_mjblock", "NES Cart Idea-Tek ET-xx PCB (TXC 05-00002-010)")
DEFINE_DEVICE_TYPE(NES_TXC_STRIKEW,    nes_txc_strikew_device,   "nes_txc_strikew", "NES Cart TXC 01-22000-400 PCB")
DEFINE_DEVICE_TYPE(NES_TXC_MXMDHTWO,  nes_txc_mxmdhtwo_device, "nes_txc_mxmdhtwo",    "NES Cart BxROM with WRAM PCB (TXC-MXMDHTWO)") // and others


nes_txc_22211_device::nes_txc_22211_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
{
}

nes_txc_22211_device::nes_txc_22211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_TXC_22211, tag, owner, clock)
{
}

nes_txc_dumarc_device::nes_txc_dumarc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txc_22211_device(mconfig, NES_TXC_DUMARACING, tag, owner, clock)
{
}

nes_txc_mjblock_device::nes_txc_mjblock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txc_22211_device(mconfig, NES_TXC_MJBLOCK, tag, owner, clock)
{
}

nes_txc_strikew_device::nes_txc_strikew_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_TXC_STRIKEW, tag, owner, clock)
{
}

nes_txc_mxmdhtwo_device::nes_txc_mxmdhtwo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_TXC_MXMDHTWO, tag, owner, clock)
	, m_lpc(*this, "lpc")
	, m_lpc_nibble(0)
	, m_lpc_have_nibble(0)
{
}




void nes_txc_22211_device::device_start()
{
	common_start();
	save_item(NAME(m_input));
	save_item(NAME(m_reg));
	save_item(NAME(m_output));
	save_item(NAME(m_s));
	save_item(NAME(m_invert));
	save_item(NAME(m_increment));
}

void nes_txc_22211_device::pcb_reset()
{
	// all chip registers power up as zero
	m_input = m_reg = m_output = m_s = m_invert = m_increment = 0;
	update_banks();
}

void nes_txc_strikew_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_txc_strikew_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);
	m_reg[0] = m_reg[1] = m_reg[2] = m_reg[3] = 0;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 TXC 01-22003-400 / 01-22111-100 / 01-22270-000 boards

 Games: Qi Wang - Chinese Chess, Creatom, Mahjong Block (TXC re-release),
 Rad Racket - Deluxe Tennis II, Xiao Mali

 TXC 05-00002-010 ASIC as latch, adder and inverter (NESdev INES Mapper 132):
 three-bit 'P', 'R' and Output registers, one-bit 'S', inCrement and inVert.
   read  $4100 (mask $E100): [xxxx SRRR], 'S' XOR 'V', upper nybble open bus
   write $4100 (mask $E103): R <- R + 1 if inCrement, else P (or ~P if inVert)
   write $4101: [.... ...V]   write $4102: [.... SPPP]   write $4103: [.... ...C]
   write $8000-$FFFF: Output <- R, driving PRG A15, CHR A14 and CHR A13 in order
 Games check $4100 after increment and inversion sequences as copy protection.

 iNES: mapper 132

 In MAME: Supported.

 -------------------------------------------------*/

void nes_txc_22211_device::update_banks()
{
	prg32(BIT(m_output, 2));
	chr8(m_output & 0x03, CHRROM);
}

uint8_t nes_txc_22211_device::read_l(offs_t offset)
{
	LOG("TXC 22211 read_l, offset: %04x\n", offset);
	if (((offset + 0x4100) & 0xe100) == 0x4100)
		return (get_open_bus() & 0xf0) | ((m_s ^ m_invert) & 1) << 3 | (m_reg & 0x07);
	return get_open_bus();
}

void nes_txc_22211_device::write_l(offs_t offset, uint8_t data)
{
	LOG("TXC 22211 write_l, offset: %04x, data: %02x\n", offset, data);
	switch ((offset + 0x4100) & 0xe103)
	{
		case 0x4100:
			if (m_increment)
				m_reg = (m_reg + 1) & 0x07;
			else
				m_reg = (m_invert ? ~m_input : m_input) & 0x07;
			break;
		case 0x4101: m_invert = BIT(data, 0); break;
		case 0x4102: m_s = BIT(data, 3); m_input = data & 0x07; break;
		case 0x4103: m_increment = BIT(data, 0); break;
	}
}

void nes_txc_22211_device::write_h(offs_t offset, uint8_t data)
{
	LOG("TXC 22211 write_h, offset: %04x, data: %02x\n", offset, data);
	m_output = m_reg;   // the written value is ignored
	update_banks();
}

/*-------------------------------------------------

 Super Mega P-4070 board

 Games: 1991 Du Ma Racing, Mahjong Block (original release),
 Venice Beach Volley (Super Mega release)

 JV001 ASIC, six-bit registers, CPU D0-D5 connected in reverse order
 (NESdev INES Mapper 172):
   read  $4100-$4103: Register, bits 4-5 inverted if Invert, bits 6-7 open bus
   write $4100: Mode 0: Register <- Input, bits 0-3 inverted if Invert
                Mode 1: Register bits 0-3 incremented, bits 4-5 unaffected
   write $4101: Invert <- D5   write $4102: Input <- D0-D5   write $4103: Mode <- D5
   write $8000-$FFFF: Output <- Register, and nametable arrangement is set:
                horizontal if Invert is clear, vertical if set
   8 KiB CHR bank = Output & 3; the 32 KiB PRG is not banked

 iNES: mapper 172

 In MAME: Supported.

 -------------------------------------------------*/

static inline uint8_t jv001_rev(uint8_t v) { return bitswap<6>(v, 0, 1, 2, 3, 4, 5); }

void nes_txc_dumarc_device::update_banks()
{
	prg32(0);
	chr8(m_output & 0x03, CHRROM);
}

uint8_t nes_txc_dumarc_device::read_l(offs_t offset)
{
	LOG("TXC Super Mega read_l, offset: %04x\n", offset);
	if (((offset + 0x4100) & 0xe100) == 0x4100)
		return (get_open_bus() & 0xc0) | jv001_rev((m_reg ^ (m_invert ? 0x30 : 0)) & 0x3f);
	return get_open_bus();
}

void nes_txc_dumarc_device::write_l(offs_t offset, uint8_t data)
{
	LOG("TXC Super Mega write_l, offset: %04x, data: %02x\n", offset, data);
	switch ((offset + 0x4100) & 0xe103)
	{
		case 0x4100:
			if (m_increment)
				m_reg = (m_reg & 0x30) | ((m_reg + 1) & 0x0f);
			else
				m_reg = (m_input ^ (m_invert ? 0x0f : 0)) & 0x3f;
			break;
		case 0x4101: m_invert = BIT(data, 5); break;
		case 0x4102: m_input = jv001_rev(data & 0x3f); break;
		case 0x4103: m_increment = BIT(data, 5); break;
	}
}

void nes_txc_dumarc_device::write_h(offs_t offset, uint8_t data)
{
	LOG("TXC Super Mega write_h, offset: %04x, data: %02x\n", offset, data);
	m_output = m_reg;   // the written value is ignored
	set_nt_mirroring(m_invert ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	update_banks();
}

/*-------------------------------------------------

 Idea-Tek ET-xx board

 Games: Puzzle, F-15 City War, Poker Jingling, Zhanguo Sichuan Sheng
 (original Idea-Tek releases), Xiao Mali

 Same TXC 05-00002-010 ASIC and registers as mapper 132, wired differently
 (NESdev INES Mapper 173): CHR A13 comes from Output bit 0, CHR A14 is the
 inverted inVert bit (applied as soon as $4101 is written) and CHR A15 from
 Output bit 1; the 32 KiB PRG is not banked.

 iNES: mapper 173

 In MAME: Supported.

 -------------------------------------------------*/

void nes_txc_mjblock_device::update_banks()
{
	prg32(0);
	chr8(BIT(m_output, 0) | (m_invert ? 0 : 2) | BIT(m_output, 1) << 2, CHRROM);
}

void nes_txc_mjblock_device::write_l(offs_t offset, uint8_t data)
{
	nes_txc_22211_device::write_l(offset, data);
	if (((offset + 0x4100) & 0xe103) == 0x4101)
		update_banks();   // CHR A14 follows inVert immediately
}

/*-------------------------------------------------

 TXC 01-22000-400 Board

 Games: F-15 City War, Policeman, Puzzle, Strike Wolf,
 Venice Beach Volley

 iNES: mapper 36

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_txc_strikew_device::read_l(offs_t offset)
{
	LOG("TXC 01-22000-400 read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset & 0x100)
		return (get_open_bus() & 0xcf) | m_reg[0] << 4;

	return get_open_bus();
}

void nes_txc_strikew_device::write_l(offs_t offset, u8 data)
{
	LOG("TXC 01-22000-400 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	switch (offset & 0x103)
	{
		case 0x100:
			if (m_reg[3] & 1)
				m_reg[0] = (m_reg[0] + 1) & 0x03;
			else if (m_reg[1] & 1)
				m_reg[0] = m_reg[2] ^ 0x03;
			else
				m_reg[0] = m_reg[2];
			break;
		case 0x101:
		case 0x102:
		case 0x103:
			m_reg[offset & 0x03] = BIT(data, 4, 2);
			break;
	}

	if (offset & 0x200)
		chr8(data, CHRROM);
}

void nes_txc_strikew_device::write_h(offs_t offset, u8 data)
{
	LOG("TXC 01-22000-400 write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(m_reg[0]);
}

/*-------------------------------------------------

 BxROM-like board with 8 KiB of WRAM at $6000-$7FFF that can be
 battery-backed (software list PCB TXC-MXMDHTWO); NESdev INES Mapper 241

 Used natively by Hengge Dianzi games and educational computer
 cartridges (some with an LPC speech chip, see below), and as the
 target of mapper hacks of mapper 164, 178 and 227 games, which may
 also access $4800-$5FFF. Writes to $8000-$FFFF select the 32 KiB
 PRG bank, without bus conflicts; mirroring is hard-wired.

 Software list entries on this board include Si Lu Chuan Qi, Ma Bu
 Mi Zhen & Qu Wei Cheng Yu Wu, Subor v6.0 and Suborwin 2000.
 Commandos (China, Waixing ES-1092) is also assigned to it, but its
 only dump is a bad dump and does not run, so its real board is not
 known.

 iNES: mapper 241

 In MAME: Supported.

 -------------------------------------------------*/

/*
 LPC speech chip (NESdev INES Mapper 241), mounted on some educational computer carts:
   read  $5000-$5FFF: [.R.. ....] 1 = ready to receive data
   write $5000-$5FFF: [.R.R DDDD] four bits of TMS5220C-PE LPC data; with both R bits set
                      the nibble is accepted, with both clear the chip is reset
 The interface differs from a regular TMS5220's: there is no command byte. As the speech
 routine of "Educational Computer 2000 - 48-in-1" shows, each LPC byte is sent low nibble
 first, the chip speaks as soon as data arrives after a reset, and a message is closed with
 zero nibbles followed by a reset. MAME's TMS5220C is used in speak-external mode with the
 nibbles paired into bytes. The chip clock is not documented; 640 kHz is the TMS5220's
 nominal clock.
*/

void nes_txc_mxmdhtwo_device::device_start()
{
	common_start();
	save_item(NAME(m_lpc_nibble));
	save_item(NAME(m_lpc_have_nibble));
}

void nes_txc_mxmdhtwo_device::pcb_reset()
{
	nes_nrom_device::pcb_reset();
	lpc_reset();
}

void nes_txc_mxmdhtwo_device::lpc_reset()
{
	m_lpc->reset();
	m_lpc->data_w(0x60);   // speak external: following data goes to the FIFO
	m_lpc_nibble = 0;
	m_lpc_have_nibble = 0;
}

uint8_t nes_txc_mxmdhtwo_device::read_l(offs_t offset)
{
	if (offset >= 0x0f00)   // $5000-$5FFF: bit 6 = ready (the chip's READY output is active low)
		return (get_open_bus() & ~0x40) | (m_lpc->readyq_r() ? 0x00 : 0x40);
	return get_open_bus();
}

void nes_txc_mxmdhtwo_device::write_l(offs_t offset, uint8_t data)
{
	if (offset < 0x0f00)
		return;
	switch (data & 0x50)
	{
		case 0x50:   // normal operation: one nibble of LPC data, low nibble of each byte first
			if (!m_lpc_have_nibble)
			{
				m_lpc_nibble = data & 0x0f;
				m_lpc_have_nibble = 1;
			}
			else
			{
				m_lpc->data_w(m_lpc_nibble | (data & 0x0f) << 4);
				m_lpc_have_nibble = 0;
			}
			break;
		case 0x00:   // both clear: reset the chip
			lpc_reset();
			break;
	}
}

void nes_txc_mxmdhtwo_device::device_add_mconfig(machine_config &config)
{
	TMS5220C(config, m_lpc, 640000).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5);
}

void nes_txc_mxmdhtwo_device::write_h(offs_t offset, uint8_t data)
{
	LOG("TXC-MXMDHTWO write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(data);
}
