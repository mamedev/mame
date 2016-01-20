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
 - check 1-line glitches due to IRQ in Sunsoft-3

 ***********************************************************************************************************/


#include "emu.h"
#include "sunsoft.h"

#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_SUNSOFT_1 = &device_creator<nes_sunsoft_1_device>;
const device_type NES_SUNSOFT_2 = &device_creator<nes_sunsoft_2_device>;
const device_type NES_SUNSOFT_3 = &device_creator<nes_sunsoft_3_device>;
const device_type NES_SUNSOFT_4 = &device_creator<nes_sunsoft_4_device>;
const device_type NES_SUNSOFT_FME7 = &device_creator<nes_sunsoft_fme7_device>;
const device_type NES_SUNSOFT_5 = &device_creator<nes_sunsoft_5_device>;


nes_sunsoft_1_device::nes_sunsoft_1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUNSOFT_1, "NES Cart Sunsoft 1 PCB", tag, owner, clock, "nes_sun1", __FILE__)
{
}

nes_sunsoft_2_device::nes_sunsoft_2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUNSOFT_2, "NES Cart Sunsoft 2 PCB", tag, owner, clock, "nes_sun2", __FILE__)
{
}

nes_sunsoft_3_device::nes_sunsoft_3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUNSOFT_3, "NES Cart Sunsoft 3 PCB", tag, owner, clock, "nes_sun3", __FILE__), m_irq_count(0), m_irq_enable(0), m_irq_toggle(0), irq_timer(nullptr)
				{
}

nes_sunsoft_4_device::nes_sunsoft_4_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_reg(0), m_latch1(0), m_latch2(0), m_wram_enable(0)
				{
}

nes_sunsoft_4_device::nes_sunsoft_4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUNSOFT_4, "NES Cart Sunsoft 4 PCB", tag, owner, clock, "nes_sun4", __FILE__), m_reg(0), m_latch1(0), m_latch2(0), m_wram_enable(0)
				{
}

nes_sunsoft_fme7_device::nes_sunsoft_fme7_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr), m_latch(0), m_wram_bank(0)
				{
}

nes_sunsoft_fme7_device::nes_sunsoft_fme7_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUNSOFT_4, "NES Cart Sunsoft FME7 PCB", tag, owner, clock, "nes_fme7", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr), m_latch(0), m_wram_bank(0)
				{
}

nes_sunsoft_5_device::nes_sunsoft_5_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_sunsoft_fme7_device(mconfig, NES_SUNSOFT_5, "NES Cart Sunsoft 5A/5B PCB", tag, owner, clock, "nes_sun5", __FILE__),
						m_ym2149(*this, "ay")
{
}


void nes_sunsoft_1_device::device_start()
{
	common_start();
}

void nes_sunsoft_1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_sunsoft_2_device::device_start()
{
	common_start();
}

void nes_sunsoft_2_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_sunsoft_3_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_toggle));
	save_item(NAME(m_irq_count));
}

void nes_sunsoft_3_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
	save_item(NAME(m_reg));
	save_item(NAME(m_wram_enable));
}

void nes_sunsoft_4_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_reg = 0;
	m_latch1 = 0;
	m_latch2 = 0;
	m_wram_enable = 0;
}

void nes_sunsoft_fme7_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	// this has to be hardcoded because some some scanline code only suits NTSC... it will be fixed with PPU rewrite
	irq_timer->adjust(attotime::zero, 0, attotime::from_hz((21477272.724 / 12)));
//  irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_wram_bank));
	save_item(NAME(m_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_sunsoft_fme7_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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

WRITE8_MEMBER(nes_sunsoft_1_device::write_m)
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

WRITE8_MEMBER(nes_sunsoft_2_device::write_h)
{
	UINT8 helper = (data & 0x07) | ((data & 0x80) ? 0x08 : 0x00);
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

 The two games using this board have incompatible mirroring
 wiring, making necessary two distinct mappers & pcb_id

 iNES: mapper 67

 -------------------------------------------------*/


void nes_sunsoft_3_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (!m_irq_count)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
				m_irq_count = 0xffff;
				m_irq_enable = 0;
			}
			else
				m_irq_count--;
		}
	}
}

WRITE8_MEMBER(nes_sunsoft_3_device::write_h)
{
	LOG_MMC(("Sunsoft 3 write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x0800:
			chr2_0(data, CHRROM);
			break;
		case 0x1800:
			chr2_2(data, CHRROM);
			break;
		case 0x2800:
			chr2_4(data, CHRROM);
			break;
		case 0x3800:
			chr2_6(data, CHRROM);
			break;
		case 0x4000:
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
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
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
			prg16_89ab(data);
			break;
		default:
			LOG_MMC(("Sunsoft 3 write_h uncaught write, offset: %04x, data: %02x\n", offset, data));
			break;
	}
}

/*-------------------------------------------------

 Sunsoft-4 board emulation

 Games: AfterBurner & Maharaja

 iNES: mapper 68

 -------------------------------------------------*/

void nes_sunsoft_4_device::sun4_mirror( int mirror, int mirr0, int mirr1 )
{
	switch (mirror)
	{
		case 0x00:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case 0x01:
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;
		case 0x02:
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		case 0x03:
			set_nt_mirroring(PPU_MIRROR_HIGH);
			break;
		case 0x10:
			set_nt_page(0, VROM, mirr0 | 0x80, 0);
			set_nt_page(1, VROM, mirr1 | 0x80, 0);
			set_nt_page(2, VROM, mirr0 | 0x80, 0);
			set_nt_page(3, VROM, mirr1 | 0x80, 0);
			break;
		case 0x11:
			set_nt_page(0, VROM, mirr0 | 0x80, 0);
			set_nt_page(1, VROM, mirr0 | 0x80, 0);
			set_nt_page(2, VROM, mirr1 | 0x80, 0);
			set_nt_page(3, VROM, mirr1 | 0x80, 0);
			break;
		case 0x12:
			set_nt_page(0, VROM, mirr0 | 0x80, 0);
			set_nt_page(1, VROM, mirr0 | 0x80, 0);
			set_nt_page(2, VROM, mirr0 | 0x80, 0);
			set_nt_page(3, VROM, mirr0 | 0x80, 0);
			break;
		case 0x13:
			set_nt_page(0, VROM, mirr1 | 0x80, 0);
			set_nt_page(1, VROM, mirr1 | 0x80, 0);
			set_nt_page(2, VROM, mirr1 | 0x80, 0);
			set_nt_page(3, VROM, mirr1 | 0x80, 0);
			break;
	}
}

WRITE8_MEMBER(nes_sunsoft_4_device::sun4_write)
{
	LOG_MMC(("Sunsoft 4 write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			chr2_0(data, CHRROM);
			break;
		case 0x1000:
			chr2_2(data, CHRROM);
			break;
		case 0x2000:
			chr2_4(data, CHRROM);
			break;
		case 0x3000:
			chr2_6(data, CHRROM);
			break;
		case 0x4000:
			m_latch1 = data & 0x7f;
			sun4_mirror(m_reg, m_latch1, m_latch2);
			break;
		case 0x5000:
			m_latch2 = data & 0x7f;
			sun4_mirror(m_reg, m_latch1, m_latch2);
			break;
		case 0x6000:
			m_reg = data & 0x13;
			sun4_mirror(m_reg, m_latch1, m_latch2);
			break;
		case 0x7000:
			prg16_89ab(data & 0x0f);
			m_wram_enable = BIT(data, 4);
			break;
		default:
			LOG_MMC(("Sunsoft 4 write_h uncaught write, offset: %04x, data: %02x\n", offset, data));
			break;
	}
}

WRITE8_MEMBER(nes_sunsoft_4_device::write_m)
{
	LOG_MMC(("Sunsoft 4 write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_battery.empty() && m_wram_enable)
		m_battery[offset & (m_battery.size() - 1)] = data;
	if (!m_prgram.empty() && m_wram_enable)
		m_prgram[offset & (m_prgram.size() - 1)] = data;
}

READ8_MEMBER(nes_sunsoft_4_device::read_m)
{
	LOG_MMC(("Sunsoft 4 read_m, offset: %04x\n", offset));

	if (!m_battery.empty() && m_wram_enable)
		return m_battery[offset & (m_battery.size() - 1)];
	if (!m_prgram.empty() && m_wram_enable)
		return m_prgram[offset & (m_prgram.size() - 1)];

	return m_open_bus;   // open bus
}

/*-------------------------------------------------

 JxROM & Sunsoft 5A / 5B / FME7 board emulation

 Notice that Sunsoft-5B = FME7 + sound chip (the latter being
 currently unemulated in MESS)

 iNES: mapper 69

 -------------------------------------------------*/

void nes_sunsoft_fme7_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if ((m_irq_enable & 0x80)) // bit7, counter decrement
		{
			if (!m_irq_count)
			{
				m_irq_count = 0xffff;
				if (m_irq_enable & 0x01) // bit0, trigger enable
					m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
			}
			else
				m_irq_count--;
		}
	}
}

WRITE8_MEMBER(nes_sunsoft_fme7_device::fme7_write)
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
					if (!(m_irq_enable & 1))
						m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
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

WRITE8_MEMBER(nes_sunsoft_fme7_device::write_m)
{
	UINT8 bank = m_wram_bank & 0x3f;
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

READ8_MEMBER(nes_sunsoft_fme7_device::read_m)
{
	UINT8 bank = m_wram_bank & 0x3f;
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

	return m_open_bus;   // open bus
}


/*-------------------------------------------------

 Sunsoft 5B board emulation (FME7 + Sound)

 Games: Gimmick!

 iNES: mapper 69

 -------------------------------------------------*/

WRITE8_MEMBER(nes_sunsoft_5_device::write_h)
{
	LOG_MMC(("sunsoft 5 write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x4000:
			m_ym2149->address_w(space, 0, data & 0x0f);
			break;
		case 0x6000:
			m_ym2149->data_w(space, 0, data);
			break;
		default:
			fme7_write(space, offset, data, mem_mask);
			break;
	}
}

//-------------------------------------------------
//  MACHINE_DRIVER( sun_5b )
//-------------------------------------------------

// From NESdev wiki: The 5B's audio is driven by the CPU clock (1.78977267 MHz),
// but like the NES's APU, the YM2149F has an optional clock divider which
// halves the internal clock speed. By comparison of the produced pitches
// in Gimmick! with the register values used, it appears that the 5B is a
// YM2149F operating in this mode. To use an AY-3-8910 as a substitute,
// you would need an external divider to reduce the clock speed by half.

#define SUN5_NTSC_CLOCK (21477272.724 / 12)

static MACHINE_CONFIG_FRAGMENT( sun_5b )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("ay", YM2149, SUN5_NTSC_CLOCK/2) // divide by 2 for the internal divider
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nes_sunsoft_5_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sun_5b );
}
