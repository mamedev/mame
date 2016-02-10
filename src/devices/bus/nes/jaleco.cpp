// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Jaleco PCBs


 Here we emulate the following PCBs

 * Jaleco JF-11 [mapper 140]
 * Jaleco JF-13 [mapper 86]
 * Jaleco JF-16 [mapper 78]
 * Jaleco JF-17 [mapper 72] + its variant with samples
 * Jaleco JF-19 [mapper 92] + its variant with samples
 * Jaleco SS88006 [mapper 18] + its variant(s) with samples

 ***********************************************************************************************************/


#include "emu.h"
#include "jaleco.h"

#include "cpu/m6502/m6502.h"
#include "sound/samples.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_JF11 = &device_creator<nes_jf11_device>;
const device_type NES_JF16 = &device_creator<nes_jf16_device>;
const device_type NES_JF17 = &device_creator<nes_jf17_device>;
const device_type NES_JF19 = &device_creator<nes_jf19_device>;
const device_type NES_SS88006 = &device_creator<nes_ss88006_device>;
const device_type NES_JF13 = &device_creator<nes_jf13_device>;
const device_type NES_JF17_ADPCM = &device_creator<nes_jf17_adpcm_device>;
const device_type NES_JF19_ADPCM = &device_creator<nes_jf19_adpcm_device>;
const device_type NES_JF23 = &device_creator<nes_jf23_device>;
const device_type NES_JF24 = &device_creator<nes_jf24_device>;
const device_type NES_JF29 = &device_creator<nes_jf29_device>;
const device_type NES_JF33 = &device_creator<nes_jf33_device>;


nes_jf11_device::nes_jf11_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_JF11, "NES Cart Jaleco JF-11 PCB", tag, owner, clock, "nes_jf11", __FILE__)
{
}

nes_jf13_device::nes_jf13_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_JF13, "NES Cart Jaleco JF-13 PCB", tag, owner, clock, "nes_jf13", __FILE__),
						m_samples(*this, "samples")
{
}

nes_jf16_device::nes_jf16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_JF16, "NES Cart Jaleco JF-16 PCB", tag, owner, clock, "nes_jf16", __FILE__)
{
}

nes_jf17_device::nes_jf17_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_latch(0)
				{
}

nes_jf17_device::nes_jf17_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_JF17, "NES Cart Jaleco JF-17 PCB", tag, owner, clock, "nes_jf17", __FILE__), m_latch(0)
				{
}

nes_jf17_adpcm_device::nes_jf17_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_jf17_device(mconfig, NES_JF17_ADPCM, "NES Cart Jaleco JF-17 + ADPCM (Moero!! Pro Tennis) PCB", tag, owner, clock, "nes_jf17_pcm", __FILE__),
						m_samples(*this, "samples")
{
}

nes_jf19_device::nes_jf19_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_jf19_device::nes_jf19_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_JF19, "NES Cart Jaleco JF-19 (Moero!! Pro Soccer) PCB", tag, owner, clock, "nes_jf19", __FILE__)
{
}

nes_jf19_adpcm_device::nes_jf19_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_jf19_device(mconfig, NES_JF19_ADPCM, "NES Cart Jaleco JF-19 + ADPCM  (Moero!! Pro Yakyuu 88) PCB", tag, owner, clock, "nes_jf19_pcm", __FILE__),
						m_samples(*this, "samples")
{
}

nes_ss88006_device::nes_ss88006_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_irq_count(0), m_irq_count_latch(0), m_irq_mode(0), m_irq_enable(0), irq_timer(nullptr), m_latch(0)
				{
}

nes_ss88006_device::nes_ss88006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SS88006, "NES Cart Jaleco SS88006 PCB", tag, owner, clock, "nes_ss88006", __FILE__), m_irq_count(0), m_irq_count_latch(0), m_irq_mode(0), m_irq_enable(0), irq_timer(nullptr), m_latch(0)
				{
}

nes_ss88006_adpcm_device::nes_ss88006_adpcm_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_ss88006_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_jf23_device::nes_jf23_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_ss88006_adpcm_device(mconfig, NES_JF23, "NES Cart Jaleco Shin Moero Pro Yakyuu PCB", tag, owner, clock, "nes_jf23", __FILE__),
						m_samples(*this, "samples")
{
}

nes_jf24_device::nes_jf24_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_ss88006_adpcm_device(mconfig, NES_JF24, "NES Cart Jaleco Terao no Dosukoi Oozumou PCB", tag, owner, clock, "nes_jf24", __FILE__),
						m_samples(*this, "samples")
{
}

nes_jf29_device::nes_jf29_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_ss88006_adpcm_device(mconfig, NES_JF29, "NES Cart Jaleco Moe Pro! '90 PCB", tag, owner, clock, "nes_jf29", __FILE__),
						m_samples(*this, "samples")
{
}

nes_jf33_device::nes_jf33_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_ss88006_adpcm_device(mconfig, NES_JF33, "NES Cart Jaleco Moe Pro! Saikyou-hen PCB", tag, owner, clock, "nes_jf33", __FILE__),
						m_samples(*this, "samples")
{
}



void nes_jf11_device::device_start()
{
	common_start();
}

void nes_jf11_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_jf13_device::device_start()
{
	common_start();
}

void nes_jf13_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_jf16_device::device_start()
{
	common_start();
}

void nes_jf16_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_jf17_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_jf17_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	m_latch = 0;
}

void nes_jf19_device::device_start()
{
	common_start();
}

void nes_jf19_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_ss88006_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_irq_mode));
	save_item(NAME(m_latch));
}

void nes_ss88006_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	memset(m_mmc_prg_bank, 0, sizeof(m_mmc_prg_bank));
	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
	m_irq_enable = 0;
	m_irq_mode = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
	m_latch = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Jaleco JF-11, JF-12 & JF-14 boards emulation

 Games: Bio Senshi Dan, Mississippi Satsujin Jiken

 iNES: mapper 140

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_jf11_device::write_m)
{
	LOG_MMC(("jf11 write_m, offset: %04x, data: %02x\n", offset, data));
	chr8(data, CHRROM);
	prg32(data >> 4);
}

/*-------------------------------------------------

 Jaleco JF-13 board emulation

 Games: Moero Pro Yakyuu

 Note: we don't emulate the additional sound hardware.

 iNES: mapper 86

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_jf13_device::write_m)
{
	LOG_MMC(("jf13 write_m, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
	{
		prg32((data >> 4) & 0x03);
		chr8(((data >> 4) & 0x04) | (data & 0x03), CHRROM);
	}
	else
	{
//      printf("sample write: offset: %04x, data: %02x\n", offset, data);
		if (data & 0x20)
			m_samples->start(data & 0x0f, data & 0x0f);
		else
			m_samples->stop_all();
	}
}

/*-------------------------------------------------

 Jaleco JF-16 board emulation

 iNES: mapper 78 (shared with a diff Irem board)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_jf16_device::write_h)
{
	LOG_MMC(("jf16 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8(data >> 4, CHRROM);
	prg16_89ab(data);
}


/*-------------------------------------------------

 Jaleco JF-17 boards emulation

 Note: we don't emulate the additional sound hardware
       for Moero!! Pro Tennis

 Games: Moero!! Juudou Warriors, Moero!! Pro Tennis, Pinball
 Quest Jpn

 iNES: mapper 72

 In MESS: Supported, see below for the Moero Pro Tennis

 -------------------------------------------------*/

WRITE8_MEMBER(nes_jf17_device::write_h)
{
	LOG_MMC(("jf17 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (BIT(m_latch, 7) && !BIT(data, 7))
		prg16_89ab(m_latch & 0x07);
	if (BIT(m_latch, 6) && !BIT(data, 6))
		chr8(m_latch & 0x0f, CHRROM);

	m_latch = data;
}

WRITE8_MEMBER(nes_jf17_adpcm_device::write_h)
{
	LOG_MMC(("jf17 + ADPCM write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (BIT(m_latch, 7) && !BIT(data, 7))
		prg16_89ab(m_latch & 0x07);
	if (BIT(m_latch, 6) && !BIT(data, 6))
		chr8(m_latch & 0x0f, CHRROM);
	if (BIT(data, 5) && !BIT(data,4))
	{
//      printf("sample write: offset: %04x, data: %02x\n", offset, data);
		m_samples->start(offset & 0x1f, offset & 0x1f);
	}

	m_latch = data;
}

/*-------------------------------------------------

 Jaleco JF-19 boards emulation

 Note: we don't emulate the additional sound hardware.

 Games: Moero Pro Soccer, Moero Pro Yakyuu '88

 iNES: mapper 92

 In MESS: Supported, see below for the Moero Pro Yakyuu '88

 -------------------------------------------------*/

WRITE8_MEMBER(nes_jf19_device::write_h)
{
	LOG_MMC(("jf19 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (BIT(data, 7))
		prg16_cdef(data & 0x0f);
	if (BIT(data, 6))
		chr8(data & 0x0f, CHRROM);
}

WRITE8_MEMBER(nes_jf19_adpcm_device::write_h)
{
	LOG_MMC(("jf19 + ADPCM write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (BIT(data, 7))
		prg16_cdef(data & 0x0f);
	if (BIT(data, 6))
		chr8(data & 0x0f, CHRROM);
	if (BIT(data, 5) && !BIT(data,4))
	{
//      printf("sample write: offset: %04x, data: %02x\n", offset, data);
		m_samples->start(offset & 0x1f, offset & 0x1f);
	}
}

/*-------------------------------------------------

 Jaleco SS88006 board emulation, aka JF-27, JF-29, JF-30, ...,
 JF-38, JF-40, JF-41

 Games: Lord of King, Magic John, Moe Pro '90, Ninja Jajamaru,
 Pizza Pop, Plasma Ball

 iNES: mapper 18

 In MESS: Supported, see below for the games with samples

 -------------------------------------------------*/

void nes_ss88006_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_mode & 0x08)  // 4bits counter
			{
				if (!(m_irq_count & 0x000f))
				{
					m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
					m_irq_count = (m_irq_count & 0xfff0) | 0x000f;
				}
				else
					m_irq_count = (m_irq_count & 0xfff0) | ((m_irq_count & 0x000f) - 1);
			}
			else if (m_irq_mode & 0x04) // 8bits counter
			{
				if (!(m_irq_count & 0x00ff))
				{
					m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
					m_irq_count = (m_irq_count & 0xff00) | 0x00ff;
				}
				else
					m_irq_count = (m_irq_count & 0xff00) | ((m_irq_count & 0x00ff) - 1);
			}
			else if (m_irq_mode & 0x02) // 12bits counter
			{
				if (!(m_irq_count & 0x0fff))
				{
					m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
					m_irq_count = (m_irq_count & 0xf000) | 0x0fff;
				}
				else
					m_irq_count = (m_irq_count & 0xf000) | ((m_irq_count & 0x0fff) - 1);
			}
			else    // 16bits counter
			{
				if (!m_irq_count)
				{
					m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
					m_irq_count = 0xffff;
				}
				else
					m_irq_count = m_irq_count - 1;
			}
		}
	}
}

WRITE8_MEMBER(nes_ss88006_device::ss88006_write)
{
	UINT8 bank;
	LOG_MMC(("ss88006 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			m_mmc_prg_bank[0] = (m_mmc_prg_bank[0] & 0xf0) | (data & 0x0f);
			prg8_89(m_mmc_prg_bank[0]);
			break;
		case 0x0001:
			m_mmc_prg_bank[0] = (m_mmc_prg_bank[0] & 0x0f) | (data << 4);
			prg8_89(m_mmc_prg_bank[0]);
			break;
		case 0x0002:
			m_mmc_prg_bank[1] = (m_mmc_prg_bank[1] & 0xf0) | (data & 0x0f);
			prg8_ab(m_mmc_prg_bank[1]);
			break;
		case 0x0003:
			m_mmc_prg_bank[1] = (m_mmc_prg_bank[1] & 0x0f) | (data << 4);
			prg8_ab(m_mmc_prg_bank[1]);
			break;
		case 0x1000:
			m_mmc_prg_bank[2] = (m_mmc_prg_bank[2] & 0xf0) | (data & 0x0f);
			prg8_cd(m_mmc_prg_bank[2]);
			break;
		case 0x1001:
			m_mmc_prg_bank[2] = (m_mmc_prg_bank[2] & 0x0f) | (data << 4);
			prg8_cd(m_mmc_prg_bank[2]);
			break;

			/* $9002, 3 (1002, 3) uncaught = Jaleco Baseball writes 0 */
			/* believe it's related to battery-backed ram enable/disable */

		case 0x2000: case 0x2001: case 0x2002: case 0x2003:
		case 0x3000: case 0x3001: case 0x3002: case 0x3003:
		case 0x4000: case 0x4001: case 0x4002: case 0x4003:
		case 0x5000: case 0x5001: case 0x5002: case 0x5003:
			bank = ((offset & 0x7000) - 0x2000) / 0x0800 + ((offset & 0x0002) >> 1);
			if (offset & 0x0001)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f)<< 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);

			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;

		case 0x6000:
			m_irq_count_latch = (m_irq_count_latch & 0xfff0) | (data & 0x0f);
			break;
		case 0x6001:
			m_irq_count_latch = (m_irq_count_latch & 0xff0f) | ((data & 0x0f) << 4);
			break;
		case 0x6002:
			m_irq_count_latch = (m_irq_count_latch & 0xf0ff) | ((data & 0x0f) << 8);
			break;
		case 0x6003:
			m_irq_count_latch = (m_irq_count_latch & 0x0fff) | ((data & 0x0f) << 12);
			break;
		case 0x7000:
			m_irq_count = m_irq_count_latch;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;
		case 0x7001:
			m_irq_enable = data & 0x01;
			m_irq_mode = data & 0x0e;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;

		case 0x7002:
			switch (data & 0x03)
			{
				case 0: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 1: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;

		default:
			logerror("Jaleco SS88006 uncaught write, addr: %04x, value: %02x\n", offset + 0x8000, data);
			break;
	}
}


// bits2-bits6 are sample number, bit1 is setup/enable/disable
// program first write sample # + bit1 set to 'init' the sample
// then it writes sample # + bit1 clear to 'start' the sample
void nes_ss88006_adpcm_device::ss88006_adpcm_write(address_space &space, offs_t offset, UINT8 data, samples_device *dev)
{
	LOG_MMC(("ss88006 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x7003:
			if ((m_latch & 0x7c) == (data & 0x7c))
			{
//              printf("sample write: data: %02x\n", data);
				if ((m_latch & 2) && !(data & 2))
					dev->start((data >> 2) & 0x1f, (data >> 2) & 0x1f);
			}
			m_latch = data;
			break;

		default:
			ss88006_write(space, offset, data);
			break;
	}
}

/**********************************************************

 Boards with external samples
 (due to undumpable UPD7755C/UPD7756C)

 JF-13 (Moero!! Pro Yakyuu) / iNES mapper 86
 JF-17 (Moero!! Pro Tennis) / iNES mapper 72 + ADPCM
 JF-19 (Moero!! Pro Yakyuu '88 - Kettei Ban) / iNES mapper 92 + ADPCM
 JF-23 (Shin Moero Pro Yakyuu) / iNES mapper 18 + ADPCM
 JF-24 (Terao no Dosukoi Oozumou) / iNES mapper 18 + ADPCM
 JF-29 (Moe Pro 90) / iNES mapper 18 + ADPCM
 JF-33 (Moe Pro Saikyou-hen) / iNES mapper 18 + ADPCM


***********************************************************/

static const char *const jf13_sample_names[] =
{
	"*moepro",
	"00",   // strike
	"01",   // ball
	"02",   // time
	"03",   // out
	"04",   // safe
	"05",   // foul
	"06",   // (catcher obtains the ball)
	"07",   // you're out
	"08",   // play ball
	"09",   // ball four
	"10",   // home run
	"11",   // new pitcher
	"12",   // ouch (pitcher hits batter)
	"13",   // ??
	"14",   // (bat hits the ball)
	"15",   // (crowd)
	nullptr
};

static const char *const jf17_sample_names[] =
{
	"*mptennis",
	"00",   // NOT EXISTING?
	"01",   // NOT EXISTING?
	"02",   // "love" (points)
	"03",   // 15 (points)
	"04",   // 30 (points)
	"05",   // 40 (points)
	"06",   // advantage
	"07",   // (advantage) server
	"08",   // (advantage) receiver
	"09",   // all (equal points)
	"10",   // deuce
	"11",   // game
	"12",   // (racket hits ball)
	"13",   // (crowd?)
	"14",   // fault
	"15",   // net
	"16",   // out
	"17",   // ??
	"18",   // (ball hits player)
	"19",   // NOT EXISTING?
	nullptr
};

static const char *const jf19_sample_names[] =
{
	"*moepro88",
	"00",   // out
	"01",   // safe
	"02",   // foul
	"03",   // fair
	"04",   // strike
	"05",   // ball
	"06",   // time
	"07",   // batter out
	"08",   // ball four
	"09",   // home run
	"10",   // play ball
	"11",   // new pitcher
	"12",   // pinch-hit
	"13",   // (hit by pitch)
	"14",   // (bat hits the ball)
	"15",   // (bunt)
	"16",   // NOT EXISTING?
	"17",   // (catcher obtains the ball)
	"18",   // (pitcher obtains the ball)
	"19",   // (crowd)
	nullptr
};

static const char *const jf23_sample_names[] =
{
	"*smoepro",
	"00",   // out
	"01",   // safe
	"02",   // foul
	"03",   // NOT EXISTING?
	"04",   // strike
	"05",   // ball
	"06",   // time
	"07",   // batter out
	"08",   // ball four
	"09",   // home run
	"10",   // play ball
	"11",   // new pitcher
	"12",   // pinch-hit
	"13",   // (hit by pitch)
	"14",   // (bat hits the ball)
	"15",   // (bunt)
	"16",   // (catcher obtains the ball)
	"17",   // fair
	"18",   // (catcher obtains the ball, alt)
	"19",   // (crowd)
	nullptr
};

static const char *const jf24_sample_names[] =
{
	"*terao",
	"00",   // (tree beating sound)
	"01",   // Hakkyoyoi
	"02",   // Nokotta
	"03",   // Matta Nashi
	"04",   // Nokotta Nokotta
	"05",   // Matta Arimasen
	nullptr
};

static const char *const jf29_sample_names[] =
{
	"*moepro90",
	"00",   // out
	"01",   // safe
	"02",   // foul
	"03",   // Not existing?
	"04",   // strike
	"05",   // ball
	"06",   // time
	"07",   // batter out
	"08",   // ball four
	"09",   // home run
	"10",   // play ball
	"11",   // new pitcher
	"12",   // pinch-hit
	"13",   // (hit by pitch)
	"14",   // (bat hits the ball)
	"15",   // (bunt)
	"16",   // (catcher obtains the ball, alt)
	"17",   // (catcher obtains the ball)
	"18",   // (catcher obtains the ball, alt 2)
	"19",   // (crowd)
	nullptr
};

static const char *const jf33_sample_names[] =
{
	"*mpsaikyo",
	"00",   // out
	"01",   // safe
	"02",   // foul
	"03",   // ???
	"04",   // strike
	"05",   // ball
	"06",   // time
	"07",   // batter out
	"08",   // ball four
	"09",   // home run
	"10",   // play ball
	"11",   // new pitcher
	"12",   // pinch-hit
	"13",   // (hit by pitch)
	"14",   // (bat hits the ball)
	"15",   // (bunt)
	"16",   // (catcher obtains the ball)
	"17",   // (catcher obtains the ball, alt)
	"18",   // ??
	"19",   // (crowd)
	nullptr
};

//-------------------------------------------------
//  MACHINE_DRIVER
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( jf13 )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(16)
	MCFG_SAMPLES_NAMES(jf13_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( jf17 )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(20)
	MCFG_SAMPLES_NAMES(jf17_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( jf19 )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(20)
	MCFG_SAMPLES_NAMES(jf19_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( jf23 )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(20)
	MCFG_SAMPLES_NAMES(jf23_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( jf24 )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(6)
	MCFG_SAMPLES_NAMES(jf24_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( jf29 )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(20)
	MCFG_SAMPLES_NAMES(jf29_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( jf33 )

	// additional sound hardware
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(20)
	MCFG_SAMPLES_NAMES(jf33_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 0.50)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions
//-------------------------------------------------

machine_config_constructor nes_jf13_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jf13 );
}

machine_config_constructor nes_jf17_adpcm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jf17 );
}

machine_config_constructor nes_jf19_adpcm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jf19 );
}

machine_config_constructor nes_jf23_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jf23 );
}

machine_config_constructor nes_jf24_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jf24 );
}

machine_config_constructor nes_jf29_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jf29 );
}

machine_config_constructor nes_jf33_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( jf33 );
}
