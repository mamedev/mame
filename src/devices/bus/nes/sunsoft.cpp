// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Sunsoft PCBs


 Here we emulate the following PCBs

 * Sunsoft-1 [mapper 184]
 * Sunsoft-2 [mapper 89 & 93]
 * Sunsoft-3 [mapper 67]
 * Sunsoft-4 [mapper 68]
 * Sunsoft FME7 & Sunsoft-5A [mapper 69]
 * Sunsoft-5B [mapper 69]

 TODO:
 - 1-line glitches in Fantasy Zone II (Sunsoft-3) seem to be PPU timing related. The cycle-based IRQ timer below "should" be ok.

 ***********************************************************************************************************/


#include "emu.h"
#include "sunsoft.h"

#include "sound/ay8910.h"
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

DEFINE_DEVICE_TYPE(NES_SUNSOFT_1,    nes_sunsoft_1_device,    "nes_sun1", "NES Cart Sunsoft 1 PCB")
DEFINE_DEVICE_TYPE(NES_SUNSOFT_2,    nes_sunsoft_2_device,    "nes_sun2", "NES Cart Sunsoft 2 PCB")
DEFINE_DEVICE_TYPE(NES_SUNSOFT_3,    nes_sunsoft_3_device,    "nes_sun3", "NES Cart Sunsoft 3 PCB")
DEFINE_DEVICE_TYPE(NES_SUNSOFT_4,    nes_sunsoft_4_device,    "nes_sun4", "NES Cart Sunsoft 4 PCB")
DEFINE_DEVICE_TYPE(NES_SUNSOFT_FME7, nes_sunsoft_fme7_device, "nes_fme7", "NES Cart Sunsoft FME7 PCB")
DEFINE_DEVICE_TYPE(NES_SUNSOFT_5,    nes_sunsoft_5_device,    "nes_sun5", "NES Cart Sunsoft 5A/5B PCB")


nes_sunsoft_1_device::nes_sunsoft_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SUNSOFT_1, tag, owner, clock)
{
}

nes_sunsoft_2_device::nes_sunsoft_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SUNSOFT_2, tag, owner, clock)
{
}

nes_sunsoft_3_device::nes_sunsoft_3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SUNSOFT_3, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_irq_toggle(0), irq_timer(nullptr)
{
}

nes_sunsoft_4_device::nes_sunsoft_4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_wram_enable(0)
{
}

nes_sunsoft_4_device::nes_sunsoft_4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sunsoft_4_device(mconfig, NES_SUNSOFT_4, tag, owner, clock)
{
}

nes_sunsoft_fme7_device::nes_sunsoft_fme7_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr), m_latch(0), m_wram_bank(0)
{
}

nes_sunsoft_fme7_device::nes_sunsoft_fme7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sunsoft_fme7_device(mconfig, NES_SUNSOFT_FME7, tag, owner, clock)
{
}

nes_sunsoft_5_device::nes_sunsoft_5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_sunsoft_fme7_device(mconfig, NES_SUNSOFT_5, tag, owner, clock)
	, m_ym2149(*this, "ay")
{
}


void nes_sunsoft_1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_sunsoft_2_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_sunsoft_3_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(FUNC(nes_sunsoft_3_device::irq_timer_tick), this);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_toggle));
	save_item(NAME(m_irq_count));
}

void nes_sunsoft_3_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_toggle = 0;
	m_irq_count = 0;
	m_irq_enable = 0;
}

void nes_sunsoft_4_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_wram_enable));
}

void nes_sunsoft_4_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_reg[0] = m_reg[1] = m_reg[2] = 0;
	m_wram_enable = 0;
}

void nes_sunsoft_fme7_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(FUNC(nes_sunsoft_fme7_device::irq_timer_tick), this);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_wram_bank));
	save_item(NAME(m_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_sunsoft_fme7_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_wram_bank = 0;

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Sunsoft-1 board emulation

 Games: Atlantis no Nazo, Kanshakudama Nage Kantarou no
 Toukaidou Gojuusan Tsugi, Wing of Madoola, Fantasy Zone

 iNES: mapper 184 (Fantasy Zone uses this board with no
 CHRROM, and the register switches PRG banks)

 -------------------------------------------------*/

void nes_sunsoft_1_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("Sunsoft 1 write_m, offset: %04x, data: %02x\n", offset, data));

	if (m_vrom_chunks)
	{
		chr4_0(data & 0x0f, CHRROM);
		chr4_4(data >> 4, CHRROM);
	}
	else
		prg16_89ab(data & 0x0f);
}

/*-------------------------------------------------

 Sunsoft-2 board emulation

 The two games using this board have incompatible mirroring
 wiring, making necessary two distinct mappers

 iNES: mapper 89 & 93

 -------------------------------------------------*/

// there are two 'variants' depending on hardwired or mapper ctrl mirroring

void nes_sunsoft_2_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t helper = (data & 0x07) | ((data & 0x80) ? 0x08 : 0x00);
	LOG_MMC(("Sunsoft 2 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);

	if (m_vrom_chunks)
		chr8(helper, CHRROM);

	prg16_89ab(data >> 4);
}

/*-------------------------------------------------

 Sunsoft-3 board emulation

 Games: Fantasy Zone II, Mito Koumon II - Sekai Manyuki

 iNES: mapper 67

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_sunsoft_3_device::irq_timer_tick)
{
	if (m_irq_enable && --m_irq_count == 0xffff)
	{
		set_irq_line(ASSERT_LINE);
		m_irq_enable = 0;
	}
}

void nes_sunsoft_3_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("Sunsoft 3 write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x0800:
		case 0x1800:
		case 0x2800:
		case 0x3800:
			chr2_x((offset >> 11) & 0x06, data & 0x3f, CHRROM);
			break;
		case 0x4800:
			m_irq_toggle ^= 1;
			if (m_irq_toggle)
				m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			else
				m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x5800:
			m_irq_enable = BIT(data, 4);
			m_irq_toggle = 0;
			break;
		case 0x6800:
			switch (data & 3)
			{
				case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;
		case 0x7800:
			prg16_89ab(data & 0x0f);
			break;
		default:
			set_irq_line(CLEAR_LINE);
			break;
	}
}

/*-------------------------------------------------

 Sunsoft-4 board emulation

 Games: AfterBurner & Maharaja

 iNES: mapper 68

 -------------------------------------------------*/

void nes_sunsoft_4_device::sun4_mirror()
{
	static constexpr u8 ciram_lut[4] =
	{
		PPU_MIRROR_VERT, PPU_MIRROR_HORZ, PPU_MIRROR_LOW, PPU_MIRROR_HIGH
	};

	static constexpr u8 vrom_lut[4][4] =
	{
		{ 0, 1, 0, 1 }, // vert
		{ 0, 0, 1, 1 }, // horz
		{ 0, 0, 0, 0 }, // low
		{ 1, 1, 1, 1 }  // high
	};

	int mirr = m_reg[2] & 0x03;

	if (BIT(m_reg[2], 4))
		for (int i = 0; i < 4; i++)
			set_nt_page(i, VROM, m_reg[vrom_lut[mirr][i]], 0);
	else
		set_nt_mirroring(ciram_lut[mirr]);
}

void nes_sunsoft_4_device::sun4_write(offs_t offset, u8 data)
{
	LOG_MMC(("Sunsoft 4 write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			chr2_x((offset >> 11) & 0x06, data, CHRROM);
			break;
		case 0x4000:
		case 0x5000:
		case 0x6000:
			m_reg[(offset >> 12) - 4] = data | 0x80;
			sun4_mirror();
			break;
		case 0x7000:
			prg16_89ab(data & 0x0f);
			m_wram_enable = BIT(data, 4);
			break;
	}
}

void nes_sunsoft_4_device::write_m(offs_t offset, u8 data)
{
	LOG_MMC(("Sunsoft 4 write_m, offset: %04x, data: %02x\n", offset, data));

	if (m_wram_enable)
		device_nes_cart_interface::write_m(offset, data);
}

u8 nes_sunsoft_4_device::read_m(offs_t offset)
{
	LOG_MMC(("Sunsoft 4 read_m, offset: %04x\n", offset));

	if (m_wram_enable)
		return device_nes_cart_interface::read_m(offset);

	return get_open_bus();
}

/*-------------------------------------------------

 JxROM & Sunsoft 5A / 5B / FME7 board emulation

 Notice that Sunsoft-5B = FME7 + sound chip

 iNES: mapper 69

 -------------------------------------------------*/

TIMER_CALLBACK_MEMBER(nes_sunsoft_fme7_device::irq_timer_tick)
{
	if (BIT(m_irq_enable, 7)) // counter decrement enabled
	{
		if (--m_irq_count == 0xffff)
		{
			if (BIT(m_irq_enable, 0)) // IRQs enabled
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_sunsoft_fme7_device::fme7_write(offs_t offset, uint8_t data)
{
	LOG_MMC(("fme7_write, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x0000:
			m_latch = data & 0x0f;
			break;

		case 0x2000:
			switch (m_latch)
			{
				case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
					chr1_x(m_latch, data, CHRROM);
					break;

				case 8:
					m_wram_bank = data;
					break;
				case 9:
					prg8_89(data);
					break;
				case 0x0a:
					prg8_ab(data);
					break;
				case 0x0b:
					prg8_cd(data);
					break;
				case 0x0c:
					switch (data & 0x03)
					{
						case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
						case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
						case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
						case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
					}
					break;
				case 0x0d:
					m_irq_enable = data;
					set_irq_line(CLEAR_LINE);
					break;
				case 0x0e:
					m_irq_count = (m_irq_count & 0xff00) | data;
					break;
				case 0x0f:
					m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
					break;
			}
			break;

		default:
			logerror("Sunsoft FME7 write_h uncaught %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

void nes_sunsoft_fme7_device::write_m(offs_t offset, uint8_t data)
{
	uint8_t bank = m_wram_bank & 0x3f;
	LOG_MMC(("Sunsoft FME7 write_m, offset: %04x, data: %02x\n", offset, data));

	if (!(m_wram_bank & 0x40))  // is PRG ROM, no write
		return;
	else if (m_wram_bank & 0x80)    // is PRG RAM
	{
		if (!m_battery.empty())
			m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)] = data;
		if (!m_prgram.empty())
			m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
	}
}

uint8_t nes_sunsoft_fme7_device::read_m(offs_t offset)
{
	uint8_t bank = m_wram_bank & 0x3f;
	LOG_MMC(("Sunsoft FME7 read_m, offset: %04x\n", offset));

	if (!(m_wram_bank & 0x40))  // is PRG ROM
		return m_prg[((bank * 0x2000) + offset) & (m_prg_size - 1)];
	else if (m_wram_bank & 0x80)    // is PRG RAM
	{
		if (!m_battery.empty())
			return m_battery[((bank * 0x2000) + offset) & (m_battery.size() - 1)];
		if (!m_prgram.empty())
			return m_prgram[((bank * 0x2000) + offset) & (m_prgram.size() - 1)];
	}

	return get_open_bus();
}


/*-------------------------------------------------

 Sunsoft 5B board emulation (FME7 + Sound)

 Games: Gimmick!

 iNES: mapper 69

 -------------------------------------------------*/

void nes_sunsoft_5_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("sunsoft 5 write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x4000:
			m_ym2149->address_w(data & 0x0f);
			break;
		case 0x6000:
			m_ym2149->data_w(data);
			break;
		default:
			fme7_write(offset, data);
			break;
	}
}


// From NESdev wiki: The 5B's audio is driven by the CPU clock (1.78977267 MHz),
// but like the NES's APU, the YM2149F has an optional clock divider which
// halves the internal clock speed. By comparison of the produced pitches
// in Gimmick! with the register values used, it appears that the 5B is a
// YM2149F operating in this mode. To use an AY-3-8910 as a substitute,
// you would need an external divider to reduce the clock speed by half.


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_sunsoft_5_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware
	SPEAKER(config, "addon").front_center();

	// TODO: this is not how Sunsoft 5B clock signaling works!
	// The board uses the CLK pin in reality, not hardcoded NTSC values!
	SUNSOFT_5B_SOUND(config, m_ym2149, XTAL(21'477'272)/12).add_route(ALL_OUTPUTS, "addon", 0.50);
}
