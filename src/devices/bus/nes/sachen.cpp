// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Sachen PCBs

 iNES mappers: 133, 136, 137, 138, 139, 141, 143, 145, 147, 148, 149, 150, 160, 243, 533, 553


 Here we emulate the following PCBs

 * Sachen SA-009 [mapper 160]
 * Sachen SA-0036 [mapper 149]
 * Sachen SA-0037 [mapper 148]
 * Sachen SA-72007 [mapper 145]
 * Sachen 3009 / SA-72008 [mapper 133]
 * Sachen TCA-01 [mapper 143]
 * Sachen 3018 (JV001 / TCU-01) [mapper 147]
 * Sachen TCU-02 [mapper 136]
 * Sachen 3013 [mapper 553]
 * Sachen 3014 [mapper 533]
 * Sachen SA-015/SA-630 [mapper 150] and SA-020A [mapper 243]
 * Sachen 8259 [mapper 141 (A), 138 (B), 139 (C), 137 (D)]


 ***********************************************************************************************************/


#include "emu.h"
#include "sachen.h"

#define LOG_HIFREQ (1U << 1)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


static INPUT_PORTS_START(sachen_74x374)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "SA-015 ASIC pin 14")
	PORT_CONFSETTING(0x00, "CPU D2")
	PORT_CONFSETTING(0x01, "Vcc")
INPUT_PORTS_END

static INPUT_PORTS_START(sachen_no_config)
INPUT_PORTS_END


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SACHEN_SA009,      nes_sachen_sa009_device,      "nes_sa009",    "NES Cart Sachen SA009 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_SA0036,     nes_sachen_sa0036_device,     "nes_sa0036",   "NES Cart Sachen SA0036 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_SA0037,     nes_sachen_sa0037_device,     "nes_sa0037",   "NES Cart Sachen SA0037 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_SA72007,    nes_sachen_sa72007_device,    "nes_sa72007",  "NES Cart Sachen SA72007 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_SA72008,    nes_sachen_sa72008_device,    "nes_sa72008",  "NES Cart Sachen 3009 / SA-72008 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_TCA01,      nes_sachen_tca01_device,      "nes_tca01",    "NES Cart Sachen TCA-01 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_TCU01,      nes_sachen_tcu01_device,      "nes_tcu01",    "NES Cart Sachen 3018 (JV001 / TCU-01) PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_TCU02,      nes_sachen_tcu02_device,      "nes_tcu02",    "NES Cart Sachen TCU-02 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_3013,       nes_sachen_3013_device,       "nes_3013",     "NES Cart Sachen 3013 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_3014,       nes_sachen_3014_device,       "nes_3014",     "NES Cart Sachen 3014 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_74X374,     nes_sachen_74x374_device,     "nes_s74x374",  "NES Cart Sachen SA-015/SA-630 PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_74X374_ALT, nes_sachen_74x374_alt_device, "nes_s74x374a", "NES Cart Sachen SA-020A PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_8259A,      nes_sachen_8259a_device,      "nes_s8259a",   "NES Cart Sachen 8259A PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_8259B,      nes_sachen_8259b_device,      "nes_s8259b",   "NES Cart Sachen 8259B PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_8259C,      nes_sachen_8259c_device,      "nes_s8259c",   "NES Cart Sachen 8259C PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_8259D,      nes_sachen_8259d_device,      "nes_s8259d",   "NES Cart Sachen 8259D PCB")


nes_sachen_sa009_device::nes_sachen_sa009_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SACHEN_SA009, tag, owner, clock)
{
}

nes_sachen_sa0036_device::nes_sachen_sa0036_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SACHEN_SA0036, tag, owner, clock)
{
}

nes_sachen_sa0037_device::nes_sachen_sa0037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SACHEN_SA0037, tag, owner, clock)
{
}

nes_sachen_sa72007_device::nes_sachen_sa72007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SACHEN_SA72007, tag, owner, clock)
{
}

nes_sachen_sa72008_device::nes_sachen_sa72008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SACHEN_SA72008, tag, owner, clock), m_3009_latch(0)
{
}

nes_sachen_tca01_device::nes_sachen_tca01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SACHEN_TCA01, tag, owner, clock)
{
}

nes_sachen_tcu01_device::nes_sachen_tcu01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SACHEN_TCU01, tag, owner, clock), m_input(0), m_reg(0), m_output(0), m_mode(0), m_invert(0)
{
}

nes_sachen_tcu02_device::nes_sachen_tcu02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SACHEN_TCU02, tag, owner, clock), m_latch(0)
{
}

nes_sachen_3013_device::nes_sachen_3013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SACHEN_3013, tag, owner, clock)
{
}

nes_sachen_3014_device::nes_sachen_3014_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_SACHEN_3014, tag, owner, clock), m_latch(0)
{
}

nes_sachen_74x374_device::nes_sachen_74x374_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_config(*this, "CONFIG"), m_latch(0), m_mmc_vrom_bank(0)
{
}

nes_sachen_74x374_device::nes_sachen_74x374_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sachen_74x374_device(mconfig, NES_SACHEN_74X374, tag, owner, clock)
{
}

nes_sachen_74x374_alt_device::nes_sachen_74x374_alt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sachen_74x374_device(mconfig, NES_SACHEN_74X374_ALT, tag, owner, clock)
{
}

nes_sachen_8259a_device::nes_sachen_8259a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_sachen_74x374_device(mconfig, type, tag, owner, clock)
{
}

nes_sachen_8259a_device::nes_sachen_8259a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sachen_8259a_device(mconfig, NES_SACHEN_8259A, tag, owner, clock)
{
}

nes_sachen_8259b_device::nes_sachen_8259b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sachen_8259a_device(mconfig, NES_SACHEN_8259B, tag, owner, clock)
{
}

nes_sachen_8259c_device::nes_sachen_8259c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sachen_8259a_device(mconfig, NES_SACHEN_8259C, tag, owner, clock)
{
}

nes_sachen_8259d_device::nes_sachen_8259d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sachen_8259a_device(mconfig, NES_SACHEN_8259D, tag, owner, clock)
{
}



ioport_constructor nes_sachen_74x374_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sachen_74x374);
}

ioport_constructor nes_sachen_74x374_alt_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sachen_no_config);
}

ioport_constructor nes_sachen_8259a_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sachen_no_config);
}



void nes_sachen_tca01_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(1);
	chr8(0, m_chr_source);
}

void nes_sachen_sa72008_device::device_start()
{
	common_start();
	save_item(NAME(m_3009_latch));
	save_item(NAME(m_3009_reg));
}

void nes_sachen_sa72008_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	m_3009_latch = 0;
	memset(m_3009_reg, 0, sizeof(m_3009_reg));
}

void nes_sachen_tcu01_device::device_start()
{
	common_start();
	save_item(NAME(m_input));
	save_item(NAME(m_reg));
	save_item(NAME(m_output));
	save_item(NAME(m_mode));
	save_item(NAME(m_invert));
}

void nes_sachen_tcu01_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	m_input = m_reg = m_output = 0;
	m_mode = m_invert = 0;
}

void nes_sachen_tcu02_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_sachen_tcu02_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_sachen_3014_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_sachen_3014_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRROM);

	m_latch = 0;
}

void nes_sachen_74x374_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_reg));
}

void nes_sachen_74x374_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_mmc_vrom_bank = 0;
	memset(m_reg, 0, sizeof(m_reg));
}

void nes_sachen_8259a_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_sachen_8259a_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_latch = 0;
	memset(m_reg, 0, sizeof(m_reg));
	chr_update();   // the CHR windows follow from the registers, all 0 here, through the variant's wiring
}

void nes_sachen_8259d_device::pcb_reset()
{
	prg32(0);
	chr8(m_vrom_chunks - 1, CHRROM);
	set_nt_mirroring(PPU_MIRROR_HORZ);

	m_latch = 0;
	memset(m_reg, 0, sizeof(m_reg));
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Sachen SA009 bootleg boards

 Games: Pipe 5

 iNES: mapper 160

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_sa009_device::write_l(offs_t offset, uint8_t data)
{
	LOG("SA009 write_l, offset: %04x, data: %02x\n", offset, data);

	chr8(data, m_chr_source);
}

/*-------------------------------------------------

 Sachen SA0036 bootleg boards

 Games: Taiwan Mahjong 16

 iNES: mapper 149

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_sa0036_device::write_h(offs_t offset, u8 data)
{
	LOG("sa0036 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	chr8(data >> 7, CHRROM);
}

/*-------------------------------------------------

 Sachen SA0037 bootleg boards
 Tengen 800008 board

 Games: Mahjong World, Shisen Mahjong, Tengen Tetris

 iNES: mapper 148

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_sa0037_device::write_h(offs_t offset, u8 data)
{
	LOG("sa0037 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg32(data >> 3);
	chr8(data, CHRROM);
}

/*-------------------------------------------------

 Sachen SA72007 bootleg boards

 Games: Sidewinder

 iNES: mapper 145

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_sa72007_device::write_l(offs_t offset, uint8_t data)
{
	LOG("SA72007 write_l, offset: %04x, data: %02x\n", offset, data);

	/* only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		chr8(data >> 7, CHRROM);
}

/*-------------------------------------------------

 Sachen 3009 / SA-72008 boards

 Games: Jovial Race, Qi Wang

 The original 60-pin releases use the Sachen 3009 board;
 the 72-pin release of Jovial Race uses SA-72008.

 iNES: mapper 133

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_sa72008_device::write_l(offs_t offset, uint8_t data)
{
	LOG("SA72008 write_l, offset: %04x, data: %02x\n", offset, data);

	if (!(offset & 0x100))
	{
		prg32(data >> 2);
		chr8(data, CHRROM);
	}
}

void nes_sachen_sa72008_device::write_h(offs_t offset, uint8_t data)
{
	LOG("Sachen 3009 write_h, offset: %04x, data: %02x\n", offset, data);

	if (!(offset & 0x01))
	{
		// The register-select bits are CPU D5, D2 and D0.
		m_3009_latch = BIT(data, 0) | (BIT(data, 2) << 1) | (BIT(data, 5) << 2);
		return;
	}

	m_3009_reg[m_3009_latch] = data & 0x1f;

	// Four registers feed their X bits back into the Namco 108 clone's register selector.
	// Iterate until the path settles. If it oscillates, retain the most recently selected
	// register, matching the observable bank data without attempting analog simulation.
	uint8_t selected = m_3009_latch;
	uint8_t seen = 0;
	for (int i = 0; i < 8; i++)
	{
		const uint8_t bit = 1U << selected;
		if (seen & bit)
			break;
		seen |= bit;

		const uint8_t feedback = BIT(m_3009_reg[selected], 3, 2);
		static constexpr uint8_t next[4] = { 2, 6, 1, 5 }; // raw selects $04, $24, $01, $21
		if (next[feedback] == selected)
			break;
		selected = next[feedback];
	}

	const uint8_t bank = m_3009_reg[selected];
	prg32(BIT(bank, 2));
	chr8(bank & 0x03, CHRROM);
}

/*-------------------------------------------------

 Sachen TCA-01 bootleg boards

 Games: Dancing Blocks, Magic Mathematic

 iNES: mapper 143

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_sachen_tca01_device::read_l(offs_t offset)
{
	LOG("TCA-01 read_l, offset: %04x\n", offset);

	/* the address is read only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		return (get_open_bus() & 0xc0) | (~offset & 0x3f);

	return get_open_bus();
}

/*-------------------------------------------------

 Sachen 3018 board (JV001 / TCU-01)

 Games: Challenge of the Dragon, Chinese Kungfu

 The original 60-pin board uses the JV001 ASIC; the 72-pin
 localization uses a compatible simple latch.

 iNES: mapper 147

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_sachen_tcu01_device::read_l(offs_t offset)
{
	LOG("TCU-01 read_l, offset: %04x\n", offset);

	if (offset & 0x100)
		return get_open_bus();

	uint8_t data = m_reg;
	if (m_invert)
		data ^= 0x30;
	return (data << 2) | (get_open_bus() & 0x03);
}

void nes_sachen_tcu01_device::write_l(offs_t offset, uint8_t data)
{
	LOG("TCU-01 write_l, offset: %04x, data: %02x\n", offset, data);

	if (offset & 0x100)
		return;

	switch (offset & 0x003)
	{
		case 0x000:
			if (m_mode)
				m_reg = (m_reg & 0x30) | ((m_reg + 1) & 0x0f);
			else
				m_reg = (m_input & 0x30) | ((m_input ^ (m_invert ? 0x0f : 0x00)) & 0x0f);
			break;

		case 0x001:
			m_invert = BIT(data, 2);
			break;

		case 0x002:
			m_input = data >> 2;
			break;

		case 0x003:
			m_mode = BIT(data, 2);
			break;
	}
}

void nes_sachen_tcu01_device::write_h(offs_t offset, uint8_t data)
{
	LOG("TCU-01 write_h, offset: %04x, data: %02x\n", offset, data);

	m_output = m_reg;
	prg32((BIT(m_output, 5) << 1) | BIT(m_output, 0));
	chr8(BIT(m_output, 1, 4), CHRROM);
}

/*-------------------------------------------------

 Sachen TCU-02 bootleg boards

 Games: Wei Lai Xiao Zi

 iNES: mapper 136

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_tcu02_device::write_l(offs_t offset, uint8_t data)
{
	LOG("TCU-02 write_l, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x103) == 0x002)
	{
		m_latch = (data & 0x30) | ((data + 3) & 0x0f);
		chr8(m_latch, CHRROM);
	}
}

uint8_t nes_sachen_tcu02_device::read_l(offs_t offset)
{
	LOG("TCU-02 read_l, offset: %04x\n", offset);

	if ((offset & 0x103) == 0x000)
		return m_latch | 0x40;
	else
		return 0x00;
}

/*-------------------------------------------------

 Sachen 3013 board

 Games: Dong Dong Nao 1

 This discrete board basically behaves like standard
 NROM-128 (mapper 0) but has circuitry so that the
 lower half of PRG reads as a constant instead of as
 a mirror of the upper half.

 NES 2.0: mapper 553

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_sachen_3013_device::read_h(offs_t offset)
{
	return (offset < 0x4000) ? 0x3a : hi_access_rom(offset);
}

/*-------------------------------------------------

 Sachen 3014 board (also marked with serial SA-003)

 Games: Dong Dong Nao 2

 This board is a CNROM variant with simple protection.
 The upper nibble of high address writes is latched
 and read back in the lower nibble at 0xe000-0xefff.

 NES 2.0: mapper 533

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_3014_device::write_h(offs_t offset, u8 data)
{
	LOG("Sachen 3014 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	m_latch = data >> 4;
	chr8(m_latch & 1, CHRROM);
}

u8 nes_sachen_3014_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "Sachen 3014 read_h, offset: %04x\n", offset);
	u8 temp = hi_access_rom(offset);

	if ((offset & 0x7000) == 0x6000)
		temp = (temp & 0xf0) | m_latch;
	return temp;
}

/*-------------------------------------------------

 Sachen 74x374 bootleg boards

 Games: Chess Academy, Chinese Checkers Jpn, Mahjong Academy,
 Olympic IQ, Poker II, Tasac [150], Poker III [243]

 iNES: mappers 150 & 243

 In MAME: Supported.

 Tested: Chess Academy (Taiwan) (En) (Unl) (mapper 150) and Honey Peach (Taiwan) (En) (Unl) (mapper 243) play.

 -------------------------------------------------*/

// R7 bits 1-2 (NESdev INES Mapper 150 and 243; NewRisingSun measured all eight R7 values on the ASIC: Talk:INES Mapper 243)
void nes_sachen_74x374_device::set_mirror(uint8_t nt)
{
	switch (nt)
	{
		case 0:     // pages 0, 0, 0, 1 (lower right unique)
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 0, 1);
			set_nt_page(2, CIRAM, 0, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;
		case 1:
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;
		case 2:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case 3:     // single screen, page 1
			set_nt_mirroring(PPU_MIRROR_HIGH);
			break;
	}
}

bool nes_sachen_74x374_device::d2_vcc()
{
	return type() == NES_SACHEN_74X374 && BIT(m_config.read_safe(0), 0);
}

void nes_sachen_74x374_device::write_l(offs_t offset, uint8_t data)
{
	LOG("Sachen SA-015 write_l, offset: %04x, data: %02x\n", offset, data);

	if (offset & 0x100)
		return;

	if (!(offset & 0x01))
	{
		m_latch = (data | (d2_vcc() ? 0x04 : 0x00)) & 0x07;
		return;
	}

	data = (data | (d2_vcc() ? 0x04 : 0x00)) & 0x07;
	m_reg[m_latch] = data;

	switch (m_latch)
	{
		case 0x04:
			m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x04) | ((data << 2) & 0x04);
			chr8(m_mmc_vrom_bank, CHRROM);
			break;
		case 0x05:
			prg32(data & 0x03);
			break;
		case 0x06:
			m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x03) | (data & 0x03);
			chr8(m_mmc_vrom_bank, CHRROM);
			break;
		case 0x07:
			set_mirror(BIT(data, 1, 2));
			break;
	}
}

uint8_t nes_sachen_74x374_device::read_l(offs_t offset)
{
	LOG("Sachen SA-015 read_l, offset: %04x\n", offset);

	if (offset & 0x100)
		return get_open_bus();

	const uint8_t value = (offset & 0x01) ? m_reg[m_latch] : m_latch;
	return d2_vcc()
		? ((get_open_bus() & 0xfc) | (value & 0x03))
		: ((get_open_bus() & 0xf8) | (value & 0x07));
}

void nes_sachen_74x374_alt_device::write_l(offs_t offset, uint8_t data)
{
	LOG("Sachen SA-020A write_l, offset: %04x, data: %02x\n", offset, data);

	if (offset & 0x100)
		return;

	if (!(offset & 0x01))
	{
		m_latch = data & 0x07;
		return;
	}

	data &= 0x07;
	m_reg[m_latch] = data;

	switch (m_latch)
	{
		case 0x02:
			m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x01) | (data & 0x01);
			chr8(m_mmc_vrom_bank, CHRROM);
			break;
		case 0x04:
			m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x02) | ((data << 1) & 0x02);
			chr8(m_mmc_vrom_bank, CHRROM);
			break;
		case 0x05:
			prg32(data & 0x03);
			break;
		case 0x06:
			m_mmc_vrom_bank = (m_mmc_vrom_bank & ~0x0c) | ((data << 2) & 0x0c);
			chr8(m_mmc_vrom_bank, CHRROM);
			break;
		case 0x07:
			set_mirror(BIT(data, 1, 2));
			break;
	}
}

/*-------------------------------------------------

 Sachen S8259 bootleg boards

 NESdev Sachen 8259, with registers masked $C101: $4100 selects a
 register (0-7), $4101 writes it (3 bits). Registers 0-3 are the low 3
 bits of the 2 KiB CHR block of each PPU window, 4 the high 3 bits of
 all four, 5 the 32 KiB PRG bank, 7 the mode: bit 0 is simple mode
 (vertical mirroring, and all four windows use register 0), bits 2-1
 the mirroring (0 vertical, 1 horizontal, 2 as 0,1,1,1, 3 one screen).
 The A and C boards shift the CHR registers left by 1 or 2 bits and
 pass that many PPU address bits through, also in simple mode. The
 registers power up as 0 (not documented).

 Checked: a synthetic image on the A board whose 2 KiB CHR blocks are
 numbered maps the four windows to blocks 0 1 0 1 at power-on, to 2 5 6 9
 once registers 0-3 are 1 2 3 4, and to 2 3 2 3 once simple mode is on.
 Five mapper 141 dumps, the CHR-RAM Q Boy among them, draw the same as
 before. No B, C or D dump was run.

 iNES: mapper 141 (A), 138 (B), 139 (C), 137 (D)

 -------------------------------------------------*/

void nes_sachen_8259a_device::set_mirror(uint8_t nt)
{
	switch (nt)
	{
		case 0:
		case 1:
			set_nt_mirroring(nt ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 2:
			set_nt_page(0, CIRAM, 0, 1);
			set_nt_page(1, CIRAM, 1, 1);
			set_nt_page(2, CIRAM, 1, 1);
			set_nt_page(3, CIRAM, 1, 1);
			break;
		case 3:
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;
	}
}

void nes_sachen_8259a_device::chr_update()
{
	uint8_t bank_helper1 = m_reg[7] & 0x01;
	uint8_t bank_helper2 = (m_reg[4] & 0x07) << 3;

	if (m_chr_source == CHRROM)
	{
		chr2_0(((m_reg[bank_helper1 ? 0 : 0] & 0x07) | bank_helper2) << 1 | 0, CHRROM);
		chr2_2(((m_reg[bank_helper1 ? 0 : 1] & 0x07) | bank_helper2) << 1 | 1, CHRROM);
		chr2_4(((m_reg[bank_helper1 ? 0 : 2] & 0x07) | bank_helper2) << 1 | 0, CHRROM);
		chr2_6(((m_reg[bank_helper1 ? 0 : 3] & 0x07) | bank_helper2) << 1 | 1, CHRROM);
	}
}

void nes_sachen_8259a_device::write_l(offs_t offset, uint8_t data)
{
	LOG("Sachen 8259 write_l, offset: %04x, data: %02x\n", offset, data);

	/* write happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
	{
		if (!(offset & 0x01))
			m_latch = data & 0x07;
		else
		{
			m_reg[m_latch] = data & 0x07;
			data &= 0x07;

			switch (m_latch)
			{
				case 0x05:
					prg32(data);
					break;
				case 0x07:
					set_mirror(BIT(data, 0) ? 0 : (data >> 1) & 0x03);
					chr_update();   // simple mode makes all four CHR windows use register 0
					break;
				default:
					chr_update();
					break;
			}
		}
	}
}

void nes_sachen_8259b_device::chr_update()
{
	uint8_t bank_helper1 = m_reg[7] & 0x01;
	uint8_t bank_helper2 = (m_reg[4] & 0x07) << 3;

	if (m_chr_source == CHRROM)
	{
		chr2_0(((m_reg[bank_helper1 ? 0 : 0] & 0x07) | bank_helper2), CHRROM);
		chr2_2(((m_reg[bank_helper1 ? 0 : 1] & 0x07) | bank_helper2), CHRROM);
		chr2_4(((m_reg[bank_helper1 ? 0 : 2] & 0x07) | bank_helper2), CHRROM);
		chr2_6(((m_reg[bank_helper1 ? 0 : 3] & 0x07) | bank_helper2), CHRROM);
	}
}

void nes_sachen_8259c_device::chr_update()
{
	uint8_t bank_helper1 = m_reg[7] & 0x01;
	uint8_t bank_helper2 = (m_reg[4] & 0x07) << 3;

	if (m_chr_source == CHRROM)
	{
		chr2_0(((m_reg[bank_helper1 ? 0 : 0] & 0x07) | bank_helper2) << 2 | 0, CHRROM);
		chr2_2(((m_reg[bank_helper1 ? 0 : 1] & 0x07) | bank_helper2) << 2 | 1, CHRROM);
		chr2_4(((m_reg[bank_helper1 ? 0 : 2] & 0x07) | bank_helper2) << 2 | 2, CHRROM);
		chr2_6(((m_reg[bank_helper1 ? 0 : 3] & 0x07) | bank_helper2) << 2 | 3, CHRROM);
	}
}

void nes_sachen_8259d_device::set_mirror(uint8_t nt)
{
	if (nt < 2)
		nt ^= 1;
	nes_sachen_8259a_device::set_mirror(nt);
}

void nes_sachen_8259d_device::chr_update()
{
	if (m_chr_source == CHRROM)
	{
		const uint8_t bank0 = m_reg[0] & 0x07;
		const bool simple = BIT(m_reg[7], 0);
		chr1_0(bank0, CHRROM);
		chr1_1((simple ? bank0 : (m_reg[1] & 0x07)) | (m_reg[4] << 4 & 0x10), CHRROM);
		chr1_2((simple ? bank0 : (m_reg[2] & 0x07)) | (m_reg[4] << 3 & 0x10), CHRROM);
		chr1_3((simple ? bank0 : (m_reg[3] & 0x07)) | (m_reg[4] << 2 & 0x10) | (m_reg[6] << 3 & 0x08), CHRROM);
	}
}
