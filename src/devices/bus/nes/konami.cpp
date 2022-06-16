// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Konami PCBs


 Here we emulate the following PCBs (multiple mappers needed due to different wirings in the pcbs
 causing different addresses to be used for bankswitch & irq)

 * Konami VRC-1 [mapper 75]
 * Konami VRC-2 [mapper 22,23,25]
 * Konami VRC-3 [mapper 73]
 * Konami VRC-4 [mapper 21,23,25]
 * Konami VRC-6 [mapper 24,26]
 * Konami VRC-7 [mapper 85]

 ***********************************************************************************************************/


#include "emu.h"
#include "konami.h"

#include "speaker.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_VRC1, nes_konami_vrc1_device, "nes_vrc1", "NES Cart Konami VRC-1 PCB")
DEFINE_DEVICE_TYPE(NES_VRC2, nes_konami_vrc2_device, "nes_vrc2", "NES Cart Konami VRC-2 PCB")
DEFINE_DEVICE_TYPE(NES_VRC3, nes_konami_vrc3_device, "nes_vrc3", "NES Cart Konami VRC-3 PCB")
DEFINE_DEVICE_TYPE(NES_VRC4, nes_konami_vrc4_device, "nes_vrc4", "NES Cart Konami VRC-4 PCB")
DEFINE_DEVICE_TYPE(NES_VRC6, nes_konami_vrc6_device, "nes_vrc6", "NES Cart Konami VRC-6 PCB")
DEFINE_DEVICE_TYPE(NES_VRC7, nes_konami_vrc7_device, "nes_vrc7", "NES Cart Konami VRC-7 PCB")


nes_konami_vrc1_device::nes_konami_vrc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VRC1, tag, owner, clock)
{
}

nes_konami_vrc2_device::nes_konami_vrc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0)
{
}

nes_konami_vrc2_device::nes_konami_vrc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_VRC2, tag, owner, clock)
{
}

nes_konami_vrc3_device::nes_konami_vrc3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VRC3, tag, owner, clock), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_enable_latch(0), m_irq_mode(0), irq_timer(nullptr)
{
}

nes_konami_vrc4_device::nes_konami_vrc4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_prg_flip(0), m_wram_enable(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_enable_latch(0), m_irq_mode(0), m_irq_prescale(0), irq_timer(nullptr)
{
}

nes_konami_vrc4_device::nes_konami_vrc4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC4, tag, owner, clock)
{
}

nes_konami_vrc6_device::nes_konami_vrc6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC6, tag, owner, clock), m_vrc6snd(*this, "vrc6snd")
{
}

nes_konami_vrc7_device::nes_konami_vrc7_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC7, tag, owner, clock), m_vrc7snd(*this, "vrc7snd")
{
}



void nes_konami_vrc1_device::device_start()
{
	common_start();
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_konami_vrc1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_mmc_vrom_bank[0] = m_mmc_vrom_bank[1] = 0;
}

void nes_konami_vrc2_device::device_start()
{
	common_start();
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_latch));
}

void nes_konami_vrc2_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	std::fill(std::begin(m_mmc_vrom_bank), std::end(m_mmc_vrom_bank), 0x00);
}

void nes_konami_vrc3_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(FUNC(nes_konami_vrc3_device::irq_timer_tick), this);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_konami_vrc3_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_mode = 0;
	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
}

void nes_konami_vrc4_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(FUNC(nes_konami_vrc4_device::irq_timer_tick), this);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_prescale));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_prg_flip));
	save_item(NAME(m_wram_enable));
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_konami_vrc4_device::pcb_reset()
{
	m_irq_mode = 0;
	m_irq_prescale = 0;
	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;

	m_prg_flip = 0;
	m_wram_enable = 0;
	m_mmc_prg_bank[0] = 0;
	m_mmc_prg_bank[1] = 0;
	set_prg();

	for (int i = 0; i < 8; i++)
		m_mmc_vrom_bank[i] = i;
	set_chr();
}

void nes_konami_vrc7_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();
	prg32((m_prg_chunks >> 1) - 1);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Konami VRC1 and Jaleco JF20, JF22

 Games: Exciting Boxing, Ganbare Goemon!, Tetsuwan Atom

 iNES: mapper 75

 In MAME: Supported.

 -------------------------------------------------*/

void nes_konami_vrc1_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-1 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
		case 0x4000:
			prg8_x(BIT(offset, 13, 2), data & 0x0f);
			break;
		case 0x1000:
			set_nt_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x0f) | BIT(data, 1) << 4;
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x0f) | BIT(data, 2) << 4;
			chr4_0(m_mmc_vrom_bank[0], CHRROM);
			chr4_4(m_mmc_vrom_bank[1], CHRROM);
			break;
		case 0x6000:
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x10) | (data & 0x0f);
			chr4_0(m_mmc_vrom_bank[0], CHRROM);
			break;
		case 0x7000:
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x10) | (data & 0x0f);
			chr4_4(m_mmc_vrom_bank[1], CHRROM);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-2

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_konami_vrc2_device::read_m(offs_t offset)
{
	LOG_MMC(("VRC-2 read_m, offset: %04x\n", offset));

	if (!m_battery.empty() || !m_prgram.empty())
		return device_nes_cart_interface::read_m(offset);
	else    // VRC2 was planned with EEPROM support; the non-working feature behaves as a vestigial 1-bit latch in $6000-$6fff on certain boards (contraj, ggoemon2 depend on this)
		return (offset < 0x1000) ? (get_open_bus() & 0xfe) | (m_latch & 1) : get_open_bus();
}

void nes_konami_vrc2_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-2 write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_battery.empty() || !m_prgram.empty())
		device_nes_cart_interface::write_m(offset, data);
	else if (offset < 0x1000)
		m_latch = data;
}

void nes_konami_vrc2_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-2 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			prg8_x(BIT(offset, 13), data);
			break;
		case 0x1000:
			set_nt_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			int bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(addr_lines, 1);
			int shift = BIT(addr_lines, 0) * 4;
			int mask = 0x0f << shift;
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & ~mask) | (((data >> m_vrc_ls_chr) << shift) & mask);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		}
		default:
			logerror("VRC-2 write_h uncaught write, addr: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC3

 Games: Salamander

 iNES: mapper 73

 In MAME: Supported.

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_konami_vrc3_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		u16 mask = m_irq_mode ? 0x00ff : 0xffff; // 8 or 16 bit mode?

		// upper byte only incremented and reloaded in 16-bit mode
		m_irq_count = (m_irq_count & ~mask) | ((m_irq_count + 1) & mask);
		if (!(m_irq_count & mask))
		{
			set_irq_line(ASSERT_LINE);
			m_irq_count = (m_irq_count & ~mask) | (m_irq_count_latch & mask);
		}
	}
}

void nes_konami_vrc3_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-3 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
		{
			int shift = 4 * BIT(offset, 12, 2);
			m_irq_count_latch &= ~(0x000f << shift);
			m_irq_count_latch |= (data & 0x0f) << shift;
			break;
		}
		case 0x4000:
			m_irq_mode = data & 0x04;
			m_irq_enable = data & 0x02;
			m_irq_enable_latch = data & 0x01;
			if (m_irq_enable)
				m_irq_count = m_irq_count_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x5000:
			m_irq_enable = m_irq_enable_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x7000:
			prg16_89ab(data & 0x07);
			break;
		default:
			logerror("VRC-3 write_h uncaught write, offset %04x, data: %02x\n", offset, data);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-4

 In MAME: Supported

 -------------------------------------------------*/

void nes_konami_vrc4_device::irq_tick()
{
	if (m_irq_count == 0xff)
	{
		m_irq_count = m_irq_count_latch;
		set_irq_line(ASSERT_LINE);
	}
	else
		m_irq_count++;
}

TIMER_CALLBACK_MEMBER(nes_konami_vrc4_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		if (m_irq_mode) // cycle mode
			irq_tick();
		else    // scanline mode
		{
			// A prescaler divides the passing CPU cycles by 114, 114, then 113 (and repeats that order).
			// This approximates 113+2/3 CPU cycles, which is one NTSC scanline.
			// Since this is a CPU-based IRQ, though, it is triggered also during non visible scanlines...
			m_irq_prescale -= 3;

			if (m_irq_prescale <= 0)
			{
				m_irq_prescale += 341;
				irq_tick();
			}
		}
	}
}

void nes_konami_vrc4_device::irq_ack_w()
{
	m_irq_enable = m_irq_enable_latch;
	set_irq_line(CLEAR_LINE);
}

void nes_konami_vrc4_device::irq_ctrl_w(u8 data)
{
	m_irq_mode = data & 0x04;
	m_irq_enable = data & 0x02;
	m_irq_enable_latch = data & 0x01;
	if (m_irq_enable)
	{
		m_irq_count = m_irq_count_latch;
		m_irq_prescale = 341;
	}
	set_irq_line(CLEAR_LINE);
}

void nes_konami_vrc4_device::set_mirror(u8 data)
{
	switch (data & 0x03)
	{
		case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}

void nes_konami_vrc4_device::set_prg(int prg_base, int prg_mask)
{
	prg8_x(0 ^ m_prg_flip, prg_base | (m_mmc_prg_bank[0] & prg_mask));
	prg8_x(1, prg_base | (m_mmc_prg_bank[1] & prg_mask));
	prg8_x(2 ^ m_prg_flip, prg_base | (prg_mask & ~1));
	prg8_x(3, prg_base | prg_mask);
}

void nes_konami_vrc4_device::set_chr(int chr_base, int chr_mask)
{
	for (int i = 0; i < 8; i++)
		chr1_x(i, chr_base | (m_mmc_vrom_bank[i] & chr_mask), m_chr_source);
}

u8 nes_konami_vrc4_device::read_m(offs_t offset)
{
	LOG_MMC(("VRC-4 read_m, offset: %04x\n", offset));

	if (m_wram_enable)
		return device_nes_cart_interface::read_m(offset);
	else
		return get_open_bus();
}

void nes_konami_vrc4_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-4 write_m, offset: %04x, data: %02x\n", offset, data));

	if (m_wram_enable)
		device_nes_cart_interface::write_m(offset, data);
}

void nes_konami_vrc4_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-4 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			m_mmc_prg_bank[BIT(offset, 13)] = data;
			set_prg();
			break;
		case 0x1000:
			if (BIT(addr_lines, 1))
			{
				m_wram_enable = data & 0x01;
				m_prg_flip = data & 0x02;
				set_prg();
			}
			else
				set_mirror(data);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			int bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(addr_lines, 1);
			int shift = BIT(addr_lines, 0) * 4;
			int mask = shift ? 0x1f0 : 0x0f;
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & ~mask) | ((data << shift) & mask);
			set_chr();
			break;
		}
		case 0x7000:
			switch (addr_lines)
			{
				case 0:
					m_irq_count_latch = (m_irq_count_latch & 0xf0) | (data & 0x0f);
					break;
				case 1:
					m_irq_count_latch = (m_irq_count_latch & 0x0f) | (data & 0x0f) << 4;
					break;
				case 2:
					irq_ctrl_w(data);
					break;
				case 3:
					irq_ack_w();
					break;
			}
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-6

 In MAME: Supported. It also uses konami_irq (there are IRQ
 issues though: see Akumajou Densetsu intro).

 TODO: Implement other CHR modes not used by any of
 the three released VRC6 games?

 -------------------------------------------------*/

void nes_konami_vrc6_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-6 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
			prg16_89ab(data & 0x0f);
			break;
		case 0x4000:
			prg8_cd(data & 0x1f);
			break;
		case 0x1000:    // pulse 1 & global control
			m_vrc6snd->write(addr_lines, data);
			break;
		case 0x2000:    // pulse 2
			m_vrc6snd->write(addr_lines | 0x100, data);
			break;
		case 0x3000:
			if (addr_lines == 3)
			{
				set_mirror(data >> 2);
				m_wram_enable = BIT(data, 7);
			}
			else    // saw
				m_vrc6snd->write(addr_lines | 0x200, data);
			break;
		case 0x5000:
		case 0x6000:
			chr1_x(4 * BIT(offset, 13) + addr_lines, data, CHRROM);
			break;
		case 0x7000:
			switch (addr_lines)
			{
				case 0:
					m_irq_count_latch = data;
					break;
				case 1:
					irq_ctrl_w(data);
					break;
				case 2:
					irq_ack_w();
					break;
				default:
					logerror("VRC-6 write_h uncaught write, addr: %04x value: %02x\n", ((offset & 0x7000) | addr_lines) + 0x8000, data);
					break;
			}
			break;
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_konami_vrc6_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware
	SPEAKER(config, "addon").front_center();

	// TODO: this is not how VRC6 clock signaling works!
	// The board uses the CLK pin in reality, not hardcoded NTSC values!
	VRC6(config, m_vrc6snd, XTAL(21'477'272)/12).add_route(ALL_OUTPUTS, "addon", 1.0);
}

/*-------------------------------------------------

 Konami VRC7

 Games: Lagrange Point, Tiny Toon Adventures 2

 iNES: mapper 85

 In MAME: Supported. It also uses konami_irq.

 -------------------------------------------------*/

void nes_konami_vrc7_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("VRC-7 write_h, offset: %04x, data: %02x\n", offset, data));

	u8 reg = bitswap<4>(offset, 14, 13, 12, m_vrc_ls_prg_a);

	switch (reg)
	{
		case 0x0:
		case 0x1:
		case 0x2:
			prg8_x(reg, data & 0x3f);
			break;
		case 0x3:
			if (BIT(offset, 5))
				m_vrc7snd->data_w(data);
			else
				m_vrc7snd->address_w(data);
			break;
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
			chr1_x(reg - 4, data, m_chr_source);
			break;
		case 0xc:
			set_mirror(data);
			m_wram_enable = BIT(data, 7);
			break;
		case 0xd:
			m_irq_count_latch = data;
			break;
		case 0xe:
			irq_ctrl_w(data);
			break;
		case 0xf:
			irq_ack_w();
			break;
	}
}


//-------------------------------------------------
//  MACHINE_DRIVER( vrc7 )
//-------------------------------------------------

// From NESdev wiki: The VRC7, in addition to being a mapper chip, also produces 6 channels of
// 2-operator FM Synthesis Audio. It is a derivative of the Yamaha YM2413 OPLL, implementing a
// subset of its features and containing a custom fixed patch set.
// The synthesis core is related to the Yamaha YM2413 OPLL:
// - Register layout is the same.
// - VRC7 has 6 channels, OPLL has 9.
// - VRC7 has no rhythm channels, OPLL does (the last 3 channels are either FM or Rhythm on OPLL).
// - VRC7 built-in instruments are not the same as OPLL instruments.
// - VRC7 has no readily-accessible status register, under normal circumstances it is write-only;
//   OPLL has an undocumented, 2-bit 'internal state' register.
// - VRC7 has an internal state output pin (may be serial version of the 2 bit internal state register)
//   and has one output pin for audio, multiplexed for all 6 channels; OPLL has two output pins, one for
//   FM and one for Rhythm, and has no special status pin.

void nes_konami_vrc7_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware
	SPEAKER(config, "addon").front_center();

	// TODO: this is not how VRC7 clock signaling works!
	// The board uses the CLK pin in reality, not hardcoded NTSC values!
	DS1001(config, m_vrc7snd, XTAL(21'477'272)/6).add_route(0, "addon", 1.0).add_route(1, "addon", 0.0);
}
