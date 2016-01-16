// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Namcot PCBs


 Here we emulate the following PCBs

 * Namcot 3433 & 3443 (aka DxROM) [mapper 88, 204, 154]
 * Namcot 3446 [mapper 76]
 * Namcot 3425 [mapper 95]
 * Namcot 163 [mapper 19]
 * Namcot 175 [mapper 210]
 * Namcot 340 [mapper 210]

 TODO:
 - add sound feature of Namcot-163
 - Quinty is not working (same issue of Mendel Palace on TxROM boards, of course)

 ***********************************************************************************************************/


#include "emu.h"
#include "namcot.h"
#include "ui/ui.h"

#include "cpu/m6502/m6502.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_NAMCOT3433 = &device_creator<nes_namcot3433_device>;
const device_type NES_NAMCOT3446 = &device_creator<nes_namcot3446_device>;
const device_type NES_NAMCOT3425 = &device_creator<nes_namcot3425_device>;
const device_type NES_NAMCOT175 = &device_creator<nes_namcot175_device>;
const device_type NES_NAMCOT340 = &device_creator<nes_namcot340_device>;
const device_type NES_NAMCOT163 = &device_creator<nes_namcot163_device>;


nes_namcot3433_device::nes_namcot3433_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_latch(0)
				{
}

nes_namcot3433_device::nes_namcot3433_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NAMCOT3433, "NES Cart Namcot 3433 & 3443 / DxROM PCB", tag, owner, clock, "nes_namcot3433", __FILE__), m_latch(0)
				{
}

nes_namcot3446_device::nes_namcot3446_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NAMCOT3446, "NES Cart Namcot 3446 PCB", tag, owner, clock, "nes_namcot3446", __FILE__), m_latch(0)
				{
}

nes_namcot3425_device::nes_namcot3425_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NAMCOT3425, "NES Cart Namcot 3425 PCB", tag, owner, clock, "nes_namcot3425", __FILE__), m_latch(0)
				{
}

nes_namcot340_device::nes_namcot340_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
				{
}

nes_namcot340_device::nes_namcot340_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NAMCOT175, "NES Cart Namcot 340 PCB", tag, owner, clock, "nes_namcot340", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
				{
}

nes_namcot175_device::nes_namcot175_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_namcot340_device(mconfig, NES_NAMCOT340, "NES Cart Namcot 175 PCB", tag, owner, clock, "nes_namcot175", __FILE__), m_wram_protect(0)
				{
}

nes_namcot163_device::nes_namcot163_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_namcot340_device(mconfig, NES_NAMCOT163, "NES Cart Namcot 163 PCB", tag, owner, clock, "nes_namcot163", __FILE__), m_wram_protect(0), m_latch(0), m_chr_bank(0)
				{
}



void nes_namcot3433_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_namcot3433_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 2);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_namcot3446_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_namcot3446_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_namcot3425_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_namcot3425_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	memset(m_reg, 0, sizeof(m_reg));
}

void nes_namcot340_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_n163_ram));

	m_mapper_sram_size = 0x2000;
	m_mapper_sram = m_n163_ram;
}

void nes_namcot340_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_namcot175_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_wram_protect));
	save_item(NAME(m_n163_ram));

	m_mapper_sram_size = 0x2000;
	m_mapper_sram = m_n163_ram;
}

void nes_namcot175_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_wram_protect = 0;
}

void nes_namcot163_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_wram_protect));
	save_item(NAME(m_latch));
	save_item(NAME(m_chr_bank));
	save_item(NAME(m_n163_ram));

	m_mapper_sram_size = 0x2000;
	m_mapper_sram = m_n163_ram;
}

void nes_namcot163_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_wram_protect = 0;
	m_latch = 0;
	m_chr_bank = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 DxROM & Namcot 3433 - 3443 - 3453 board emulation

 Games: Dragon Spirit - Aratanaru Densetsu, Namcot Mahjong, Quinty,
 Devilman

 These are the same board, but DRROM (and Tengen 800004) have
 4-screen mirroring

 iNES: mappers 88, 206 (same as 88 but possibly 4-screen mirroring),
 and 154 (same as 88 but with additional mirroring control)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_namcot3433_device::dxrom_write)
{
	LOG_MMC(("dxrom_write, offset: %04x, data: %02x\n", offset, data));

	if (!(offset & 1) && m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);

	if (offset >= 0x2000)
		return;

	switch (offset & 1)
	{
		case 1:
			switch (m_latch & 0x07)
			{
				case 0: chr2_0(data >> 1, CHRROM); break;
				case 1: chr2_2(data >> 1, CHRROM); break;
				case 2: chr1_4(data | 0x40, CHRROM); break;
				case 3: chr1_5(data | 0x40, CHRROM); break;
				case 4: chr1_6(data | 0x40, CHRROM); break;
				case 5: chr1_7(data | 0x40, CHRROM); break;
				case 6: prg8_89(data); break;
				case 7: prg8_ab(data); break;
			}
			break;
		case 0:
			m_latch = data;
			break;
	}
}


/*-------------------------------------------------

 Namcot 3446 board emulation

 Games: Digital Devil Monogatari - Megami Tensei

 These are similar Namcot 34x3, but different bankswitch capabilities

 iNES: mapper 76

 -------------------------------------------------*/

WRITE8_MEMBER(nes_namcot3446_device::write_h)
{
	LOG_MMC(("namcot3446 write_h, offset: %04x, data: %02x\n", offset, data));

	// NEStopia does not have this!
	if (offset >= 0x2000)
	{
		if (!(offset & 1))
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		return;
	}

	switch (offset & 1)
	{
		case 1:
			switch (m_latch & 0x07)
		{
			case 2: chr2_0(data, CHRROM); break;
			case 3: chr2_2(data, CHRROM); break;
			case 4: chr2_4(data, CHRROM); break;
			case 5: chr2_6(data, CHRROM); break;
			case 6: BIT(m_latch, 6) ? prg8_cd(data) : prg8_89(data); break;
			case 7: prg8_ab(data); break;
		}
			break;
		case 0:
			m_latch = data;
			break;
	}
}

/*-------------------------------------------------

 Namcot 3425 board emulation

 Games: Dragon Buster

 These are similar Namcot 34x3, but with NT mirroring (two
 different modes)

 iNES: mapper 95

 -------------------------------------------------*/

WRITE8_MEMBER(nes_namcot3425_device::write_h)
{
	UINT8 mode;
	LOG_MMC(("namcot3425 write_h, offset: %04x, data: %02x\n", offset, data));
	if (offset >= 0x2000)
		return;

	switch (offset & 1)
	{
		case 1:
			mode = m_latch & 0x07;
			switch (mode)
			{
				case 0: chr2_0(data >> 1, CHRROM); break;
				case 1: chr2_2(data >> 1, CHRROM); break;
				case 2:
				case 3:
				case 4:
				case 5:
					chr1_x(2 + mode, data, CHRROM);
					m_reg[mode - 2] = BIT(data, 5);
					if (!BIT(m_latch, 7))
					{
						set_nt_page(0, CIRAM, m_reg[0], 1);
						set_nt_page(1, CIRAM, m_reg[1], 1);
						set_nt_page(2, CIRAM, m_reg[2], 1);
						set_nt_page(3, CIRAM, m_reg[3], 1);
					}
					else
						set_nt_mirroring(PPU_MIRROR_HORZ);
					break;
				case 6: prg8_89(data); break;
				case 7: prg8_ab(data); break;
		}
			break;
		case 0:
			m_latch = data;
			break;
	}
}

/*-------------------------------------------------

 Namcot-340 board emulation

 Games: Famista '92, '93 & '94, Top Striker,
        Wagyan Land 2 & 3

 This (and Namcot-175 below) is a cut-down version
 of the Namcot-163 chip, without the sound capabilities.
 They also cannot use NTRAM as VRAM and differ for
 the mirroring handling

 iNES: mapper 210

 In MESS: Supported

 -------------------------------------------------*/

void nes_namcot340_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0x7fff)  // counter does not wrap to 0!
				m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
			else
				m_irq_count++;
		}
	}
}

WRITE8_MEMBER(nes_namcot340_device::n340_lowrite)
{
	LOG_MMC(("n340_lowrite, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x1000: /* low byte of IRQ */
			m_irq_count = (m_irq_count & 0x7f00) | data;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;
		case 0x1800: /* high byte of IRQ, IRQ enable in high bit */
			m_irq_count = (m_irq_count & 0xff) | ((data & 0x7f) << 8);
			m_irq_enable = data & 0x80;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;
	}
}

READ8_MEMBER(nes_namcot340_device::n340_loread)
{
	LOG_MMC(("n340_loread, offset: %04x\n", offset));
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x1000:
			return m_irq_count & 0xff;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
		case 0x1800:
			return (m_irq_count >> 8) & 0xff;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
		default:
			return 0x00;
	}
}

WRITE8_MEMBER(nes_namcot340_device::n340_hiwrite)
{
	LOG_MMC(("n340_hiwrite, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x0000: case 0x0800:
		case 0x1000: case 0x1800:
		case 0x2000: case 0x2800:
		case 0x3000: case 0x3800:
			chr1_x(offset / 0x800, data, CHRROM);
			break;
		case 0x4000:
			// no cart found with wram, so it is not clear if this could work as in Namcot-175...
			break;
		case 0x6000:
			switch (data & 0xc0)
			{
				case 0x00:
					set_nt_mirroring(PPU_MIRROR_LOW);
						break;
				case 0x40:
					set_nt_mirroring(PPU_MIRROR_VERT);
					break;
				case 0x80:
					set_nt_mirroring(PPU_MIRROR_HIGH);
					break;
				case 0xc0:
					set_nt_mirroring(PPU_MIRROR_HORZ);
					break;
			}
			prg8_89(data & 0x3f);
			break;
		case 0x6800:
			prg8_ab(data & 0x3f);
			break;
		case 0x7000:
			prg8_cd(data & 0x3f);
			break;
	}
}


/*-------------------------------------------------

 Namcot-175 board emulation

 Games: Chibi Maruko-chan, Family Circuit '91,
        Famista '91

 This (and Namcot-340 above) is a cut-down version
 of the Namcot-163 chip, without the sound capabilities.
 They also cannot use NTRAM as VRAM and differ for
 the mirroring handling

 iNES: mapper 210

 In MESS: Supported

 -------------------------------------------------*/

READ8_MEMBER(nes_namcot175_device::read_m)
{
	// the only game supporting this is Family Circuit '91, and it has 2KB of battery
	// but it's mirrored up to 8KB (see Sprint Race -> Back Up menu breakage if not)
	if (!m_battery.empty() && !m_wram_protect)
		return m_battery[offset & (m_battery.size() - 1)];

	return m_open_bus;   // open bus
}

WRITE8_MEMBER(nes_namcot175_device::write_m)
{
	// the only game supporting this is Family Circuit '91, and it has 2KB of battery
	// but it's mirrored up to 8KB (see Sprint Race -> Back Up menu breakage if not)
	if (!m_battery.empty() && !m_wram_protect)
		m_battery[offset & (m_battery.size() - 1)] = data;
}

WRITE8_MEMBER(nes_namcot175_device::write_h)
{
	LOG_MMC(("namcot175 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x4000:
			m_wram_protect = data & 1;
			break;
		case 0x6000:
			prg8_89(data & 0x3f);
			break;
		default:
			n340_hiwrite(space, offset, data, mem_mask);
			break;
	}
}

/*-------------------------------------------------

 Namcot-163 board emulation

 Games: Battle Fleet, Famista '90, Megami Tensei II,
 Juvei Quest, etc.

 Compared to Namcot-175 here we have mapper controlled
 mirroring, NTRAM mapping to VRAM and additional
 sound hw inside the chip (currently unemulated) and
 some internal RAM.

 iNES: mapper 19

 In MESS: Supported (with no emulation of the
 sound component)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_namcot163_device::chr_w)
{
	int bank = offset >> 10;

	if (!(m_latch & 0x40) && m_chr_bank >= 0xe0)
	{
		// CIRAM!!!
		machine().ui().popup_time(10, "CIRAM mapped to VRAM. Please contact MAMEDevs.");

		if (!m_nt_writable[bank & 0x03])
			return;
		m_nt_access[bank & 0x03][offset & 0x3ff] = data;
	}
	// or ROM, so no write
}

READ8_MEMBER(nes_namcot163_device::chr_r)
{
	int bank = offset >> 10;
	if (!(m_latch & 0x40) && m_chr_bank >= 0xe0)
	{
		// CIRAM!!!
		machine().ui().popup_time(10, "CIRAM mapped to VRAM. Please contact MAMEDevs.");
		return m_nt_access[bank & 0x03][offset & 0x3ff];
	}
	// or ROM, accessed as usual
	return m_chr_access[bank][offset & 0x3ff];
}


READ8_MEMBER(nes_namcot163_device::read_m)
{
	if (!m_battery.empty() && offset < m_battery.size())
		return m_battery[offset & (m_battery.size() - 1)];

	return m_open_bus;   // open bus
}

WRITE8_MEMBER(nes_namcot163_device::write_m)
{
	// the pcb can separately protect each 2KB chunk of the external wram from writes
	int bank = (offset & 0x1800) >> 11;
	if (!m_battery.empty() && !BIT(m_wram_protect, bank))
		m_battery[offset & (m_battery.size() - 1)] = data;
}

WRITE8_MEMBER(nes_namcot163_device::write_l)
{
	LOG_MMC(("namcot163 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x0800:
			LOG_MMC(("Namcot-163 sound reg write, data: %02x\n", data));
			break;
		default:
			n340_lowrite(space, offset, data, mem_mask);
			break;
	}
}

READ8_MEMBER(nes_namcot163_device::read_l)
{
	LOG_MMC(("namcot163 read_l, offset: %04x\n", offset));
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x0800:
			LOG_MMC(("Namcot-163 sound reg read\n"));
			return 0;
		default:
			return n340_loread(space, offset, mem_mask);
	}
}

void nes_namcot163_device::set_mirror(UINT8 page, UINT8 data)
{
	if (data < 0xe0)
		set_nt_page(page, VROM, data, 0);
	else
		set_nt_page(page, CIRAM, data & 0x01, 1);
}

WRITE8_MEMBER(nes_namcot163_device::write_h)
{
	int page;
	LOG_MMC(("namcot163 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x0000: case 0x0800:
		case 0x1000: case 0x1800:
		case 0x2000: case 0x2800:
		case 0x3000: case 0x3800:
			m_chr_bank = data;
			chr1_x(offset / 0x800, m_chr_bank, CHRROM);
			break;
		case 0x4000:
		case 0x4800:
		case 0x5000:
		case 0x5800:
			page = (offset & 0x1800) >> 11;
			set_mirror(page, data);
			break;
		case 0x6000:
			// TODO: data & 40 (or data & c0) disable sound if set
			prg8_89(data & 0x3f);
			break;
		case 0x6800:
			m_latch = data & 0xc0;
			prg8_ab(data & 0x3f);
			break;
		case 0x7800:
			// the lower 4 bits work *BOTH* as WRAM write protect *AND* as sound address!
			m_wram_protect = data & 0x0f;
			LOG_MMC(("Namcot-163 sound address write, data: %02x\n", data));
			break;
		default:
			n340_hiwrite(space, offset, data, mem_mask);
			break;
	}
}
