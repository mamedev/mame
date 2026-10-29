// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Namcot PCBs

 iNES mappers: 19, 76, 88, 95, 154, 206, 210


 Here we emulate the following PCBs

 * Namcot 3433 & 3443 (aka DxROM) [mapper 88, 206, 154]
 * Namcot 3446 [mapper 76]
 * Namcot 3425 [mapper 95]
 * Namcot 163 [mapper 19]
 * Namcot 175 [mapper 210]
 * Namcot 340 [mapper 210]

 ***********************************************************************************************************/


#include "emu.h"
#include "namcot.h"

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

DEFINE_DEVICE_TYPE(NES_NAMCOT3433, nes_namcot3433_device, "nes_namcot3433", "NES Cart Namcot 3433 & 3443 / DxROM PCB")
DEFINE_DEVICE_TYPE(NES_NAMCOT3446, nes_namcot3446_device, "nes_namcot3446", "NES Cart Namcot 3446 PCB")
DEFINE_DEVICE_TYPE(NES_NAMCOT3425, nes_namcot3425_device, "nes_namcot3425", "NES Cart Namcot 3425 PCB")
DEFINE_DEVICE_TYPE(NES_NAMCOT340,  nes_namcot340_device,  "nes_namcot340",  "NES Cart Namcot 340 PCB")
DEFINE_DEVICE_TYPE(NES_NAMCOT175,  nes_namcot175_device,  "nes_namcot175",  "NES Cart Namcot 175 PCB")
DEFINE_DEVICE_TYPE(NES_NAMCOT163,  nes_namcot163_device,  "nes_namcot163",  "NES Cart Namcot 163 PCB")


nes_namcot3433_device::nes_namcot3433_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0)
{
}

nes_namcot3433_device::nes_namcot3433_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NAMCOT3433, tag, owner, clock), m_latch(0)
{
}

nes_namcot3446_device::nes_namcot3446_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NAMCOT3446, tag, owner, clock), m_latch(0)
{
}

nes_namcot3425_device::nes_namcot3425_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NAMCOT3425, tag, owner, clock), m_latch(0)
{
}

nes_namcot340_device::nes_namcot340_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_irq_count(0), m_irq_enable(0)
{
}

nes_namcot340_device::nes_namcot340_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_namcot340_device(mconfig, NES_NAMCOT340, tag, owner, clock)
{
}

nes_namcot175_device::nes_namcot175_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_namcot340_device(mconfig, NES_NAMCOT175, tag, owner, clock), m_wram_enable(0)
{
}

nes_namcot163_device::nes_namcot163_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_namcot340_device(mconfig, NES_NAMCOT163, tag, owner, clock), m_wram_protect(0), m_latch(0), m_namco163snd(*this, "n163")
{
}



void nes_namcot3433_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_namcot3433_device::pcb_reset()
{
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
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_reg[0] = m_reg[1] = 0;
	update_nt();
}

void nes_namcot340_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_namcot340_device::pcb_reset()
{
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

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_wram_enable));
}

void nes_namcot175_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_wram_enable = 0;
}

void nes_namcot163_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_wram_protect));
	save_item(NAME(m_latch));
	save_item(NAME(m_chr_regs));

	// battery-backed save RAM is the chip's own 128 bytes (sized by the loader only when the board has a battery)
	m_mapper_sram_size = namco_163_sound_device::INTERNAL_RAM_SIZE;
	m_mapper_sram = m_namco163snd->internal_ram();

	// Gains set from a measurement (nes-audio-tests db_n163: full-volume N163 square vs APU square is +16.3 dB at gain
	// 1.0 in this mix) to the middle of each NESdev submapper range
	if (m_n163_vol == 2) // Submapper 2 - No expansion sound
	{
		m_namco163snd->set_output_gain(ALL_OUTPUTS, 0.0f);
	}
	else if (m_n163_vol == 3) // Submapper 3 - N163 expansion sound: 11.0-13.0 dB louder than NES APU
	{
		m_namco163snd->set_output_gain(ALL_OUTPUTS, 0.62f);   // +12.0 dB
	}
	else if (m_n163_vol == 4) // Submapper 4 - N163 expansion sound: 16.0-17.0 dB louder than NES APU
	{
		m_namco163snd->set_output_gain(ALL_OUTPUTS, 1.02f);   // +16.5 dB
	}
	else if (m_n163_vol == 5) // Submapper 5 - N163 expansion sound: 18.0-19.5 dB louder than NES APU
	{
		m_namco163snd->set_output_gain(ALL_OUTPUTS, 1.33f);   // +18.75 dB
	}
}

void nes_namcot163_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_wram_protect = 0;
	m_latch = 0;
	std::fill(std::begin(m_chr_regs), std::end(m_chr_regs), 0);
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

void nes_namcot3433_device::dxrom_write(offs_t offset, uint8_t data)
{
	LOG("dxrom_write, offset: %04x, data: %02x\n", offset, data);

	// mapper 154: the one-screen mirroring bit is decoded over the whole $8000-$FFFF range, odd and even addresses alike
	// (NESdev INES Mapper 154)
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);

	if (offset >= 0x2000)
		return;

	switch (offset & 1)
	{
		case 1:
			switch (m_latch & 0x07)
			{
				// mapper 88/154: CHR A16 follows PPU A12, so R0/R1 ($0xxx) see the first 64K and R2-R5 ($1xxx) the second
				case 0: chr2_0((data & 0x3f) >> 1, CHRROM); break;
				case 1: chr2_2((data & 0x3f) >> 1, CHRROM); break;
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

void nes_namcot3446_device::write_h(offs_t offset, uint8_t data)
{
	LOG("namcot3446 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x2000)
		return;

	if (offset & 1)
	{
		switch (m_latch & 0x07)
		{
			case 2: chr2_0(data, CHRROM); break;
			case 3: chr2_2(data, CHRROM); break;
			case 4: chr2_4(data, CHRROM); break;
			case 5: chr2_6(data, CHRROM); break;
			case 6: prg8_89(data); break;
			case 7: prg8_ab(data); break;
		}
	}
	else
		m_latch = data;
}

/*-------------------------------------------------

 Namcot 3425 board emulation

 Games: Dragon Buster

 These are similar Namcot 34x3, but with NT mirroring (two
 different modes)

 iNES: mapper 95

 -------------------------------------------------*/

void nes_namcot3425_device::update_nt()
{
	set_nt_page(0, CIRAM, m_reg[0], 1);
	set_nt_page(1, CIRAM, m_reg[0], 1);
	set_nt_page(2, CIRAM, m_reg[1], 1);
	set_nt_page(3, CIRAM, m_reg[1], 1);
}

void nes_namcot3425_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t mode;
	LOG("namcot3425 write_h, offset: %04x, data: %02x\n", offset, data);
	if (offset >= 0x2000)
		return;

	switch (offset & 1)
	{
		case 1:
			mode = m_latch & 0x07;
			switch (mode)
			{
				case 0:
				case 1:
					// CHR A15 (bit 5) of R0/R1 also drives CIRAM A10: nametable fetches have PPU A12 = 0, so R0 selects the
					// nametable for $2000-$27FF and R1 for $2800-$2FFF (NESdev INES Mapper 095)
					chr2_x(mode * 2, data >> 1, CHRROM);
					m_reg[mode] = BIT(data, 5);
					update_nt();
					break;
				case 2:
				case 3:
				case 4:
				case 5:
					chr1_x(2 + mode, data, CHRROM);
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
 of the Namcot-163 chip, without the sound capabilities,
 the IRQ counter or the internal RAM.
 They also cannot use NTRAM as VRAM and differ for
 the mirroring handling

 iNES: mapper 210

 In MAME: Supported

 -------------------------------------------------*/

void nes_namcot340_device::irq_clock()
{
	if (m_irq_enable)
	{
		if (m_irq_count == 0x7fff)  // counter does not wrap to 0!
			set_irq_line(ASSERT_LINE);
		else
			m_irq_count++;
	}
}

void nes_namcot340_device::n340_lowrite(offs_t offset, uint8_t data)
{
	LOG("n340_lowrite, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x1000: /* low byte of IRQ */
			m_irq_count = (m_irq_count & 0x7f00) | data;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x1800: /* high byte of IRQ, IRQ enable in high bit */
			m_irq_count = (m_irq_count & 0xff) | ((data & 0x7f) << 8);
			m_irq_enable = data & 0x80;
			set_irq_line(CLEAR_LINE);
			break;
	}
}

uint8_t nes_namcot340_device::n340_loread(offs_t offset)
{
	LOG("n340_loread, offset: %04x\n", offset);
	offset += 0x100;

	switch (offset & 0x1800)
	{
		// reading the counter does not acknowledge the IRQ: only writes to $5000/$5800 do (NESdev INES Mapper 019)
		case 0x1000:
			return m_irq_count & 0xff;
		case 0x1800:
			return (m_irq_count >> 8) | (m_irq_enable ? 0x80 : 0x00);   // NESdev: bit 7 reads back the IRQ enable
		default:
			return 0x00;
	}
}

void nes_namcot340_device::n340_hiwrite(offs_t offset, uint8_t data)
{
	LOG("n340_hiwrite, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7800)
	{
		case 0x0000: case 0x0800:
		case 0x1000: case 0x1800:
		case 0x2000: case 0x2800:
		case 0x3000: case 0x3800:
			chr1_x(offset >> 11, data, CHRROM);
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

 In MAME: Supported

 -------------------------------------------------*/

uint8_t nes_namcot175_device::read_m(offs_t offset)
{
	// Family Circuit '91 has 2KB of battery RAM mirrored up to 8KB (see Sprint Race -> Back Up menu breakage if not);
	// $C000 bit 0 = 1 enables it (NESdev INES Mapper 210)
	if (!m_battery.empty() && m_wram_enable)
		return m_battery[offset & (m_battery.size() - 1)];

	return get_open_bus();
}

void nes_namcot175_device::write_m(offs_t offset, uint8_t data)
{
	if (!m_battery.empty() && m_wram_enable)
		m_battery[offset & (m_battery.size() - 1)] = data;
}

void nes_namcot175_device::write_h(offs_t offset, uint8_t data)
{
	LOG("namcot175 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7800)
	{
		case 0x4000:
			m_wram_enable = BIT(data, 0);
			break;
		case 0x6000:
			prg8_89(data & 0x3f);
			break;
		default:
			n340_hiwrite(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Namcot-163 board emulation

 Games: Battle Fleet, Famista '90, Megami Tensei II,
 Juvei Quest, etc.

 Compared to Namcot-175 here we have mapper controlled
 mirroring, NTRAM mapping to VRAM and additional
 sound hw inside the chip and some internal RAM.

 iNES: mapper 19

 In MAME: Supported

 -------------------------------------------------*/

// NESdev N163: each CHR register $E0-$FF selects nametable RAM (CIRAM page = bit 0) instead of CHR-ROM, unless disabled by
// $E800 bit 6 (registers for $0000-$0FFF) or bit 7 (registers for $1000-$1FFF)
uint8_t *nes_namcot163_device::chr_ciram_ptr(offs_t offset)
{
	const int bank = (offset >> 10) & 7;
	const uint8_t reg = m_chr_regs[bank];
	if (reg < 0xe0 || (m_latch & (bank < 4 ? 0x40 : 0x80)))
		return nullptr;
	return get_ciram_base() + (reg & 0x01) * 0x400 + (offset & 0x3ff);
}

void nes_namcot163_device::chr_w(offs_t offset, uint8_t data)
{
	if (uint8_t *p = chr_ciram_ptr(offset))
		*p = data;
	// or ROM, so no write
}

uint8_t nes_namcot163_device::chr_r(offs_t offset)
{
	if (const uint8_t *p = chr_ciram_ptr(offset))
		return *p;
	// or ROM, accessed as usual
	return m_chr_access[offset >> 10][offset & 0x3ff];
}


uint8_t nes_namcot163_device::read_m(offs_t offset)
{
	if (!m_battery.empty() && offset < m_battery.size())
		return m_battery[offset & (m_battery.size() - 1)];

	return get_open_bus();
}

void nes_namcot163_device::write_m(offs_t offset, uint8_t data)
{
	// the pcb can separately protect each 2KB chunk of the external wram from writes
	// NESdev N163 $F800: writes need the upper nybble to be 0100 ($40-$4E) and the 2KB window's protect bit clear
	int bank = BIT(offset, 11, 2);
	if (!m_battery.empty() && (m_wram_protect & 0xf0) == 0x40 && !BIT(m_wram_protect, bank))
		m_battery[offset & (m_battery.size() - 1)] = data;
}

void nes_namcot163_device::write_l(offs_t offset, uint8_t data)
{
	LOG("namcot163 write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x0800:
			m_namco163snd->data_w(data);
			break;
		default:
			n340_lowrite(offset, data);
			break;
	}
}

uint8_t nes_namcot163_device::read_l(offs_t offset)
{
	LOG("namcot163 read_l, offset: %04x\n", offset);
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x0800:
			return m_namco163snd->data_r();
		default:
			return n340_loread(offset);
	}
}

void nes_namcot163_device::set_mirror(uint8_t page, uint8_t data)
{
	if (data < 0xe0)
		set_nt_page(page, VROM, data, 0);
	else
		set_nt_page(page, CIRAM, data & 0x01, 1);
}

void nes_namcot163_device::write_h(offs_t offset, uint8_t data)
{
	LOG("namcot163 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7800)
	{
		case 0x0000: case 0x0800:
		case 0x1000: case 0x1800:
		case 0x2000: case 0x2800:
		case 0x3000: case 0x3800:
			m_chr_regs[(offset >> 11) & 7] = data;
			chr1_x(offset >> 11, data, CHRROM);
			break;
		case 0x4000:
		case 0x4800:
		case 0x5000:
		case 0x5800:
			set_mirror(BIT(offset, 11, 2), data);
			break;
		case 0x6000:
			m_namco163snd->disable_w((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
			prg8_89(data & 0x3f);
			break;
		case 0x6800:
			m_latch = data & 0xc0;
			prg8_ab(data & 0x3f);
			break;
		case 0x7800:
			// the lower 4 bits work *BOTH* as WRAM write protect *AND* as sound address!
			m_wram_protect = data;    // full value: the upper nybble gates writes (NESdev)
			m_namco163snd->addr_w(data);
			break;
		default:
			n340_hiwrite(offset, data);
			break;
	}
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_namcot163_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware, mixed into the console audio through the cartridge connector

	// the N163 runs off M2, the NTSC CPU clock (21.477272 MHz / 12), and updates one channel every 15 cycles, which the
	// sound device models with its clock / 15 sample rate; all N163 games are NTSC-only Famicom releases
	NAMCO_163(config, m_namco163snd, XTAL(21'477'272)/12).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5);
}
