// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bandai PCBs

 iNES mappers: 16, 96, 153, 159


 Here we emulate the following PCBs

 * Bandai FCG [mapper 16] (older design, regs are only accessed in 0x6000-0x7fff range)
 * Bandai LZ93D50 [mapper 16] (this extends FCG by solving issues when writing to 0x8000-0xffff)
 * Bandai LZ93D50 + 24C01 EEPROM [mapper 159]
 * Bandai LZ93D50 + 24C02 EEPROM [mapper 16]
 * Bandai Famicom Jump 2 (aka LZ93D50 + SRAM) [mapper 153]
 * Bandai Oeka Kids [mapper 96]

 * Bandai Datach Joint ROM System [mapper 157] is emulated in a separate source file
   to implement also the subslot, but the PCB is basically a Bandai LZ93D50 + 24C02 EEPROM
   PCB with added barcode reader and subslot

 * Bandai Karaoke Studio [mapper 188] is emulated in a separate source file
   to implement also the subslot and the mic inputs


 The 24C02 (mapper 16 submapper 5) and X24C01 (mapper 159) EEPROMs were verified by writing a byte through
 the $800D/$6000 registers, restarting and reading it back.


 ***********************************************************************************************************/


#include "emu.h"
#include "bandai.h"

#define LOG_UNHANDLED (1U << 1)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_UNHANDLED | LOG_GENERAL)
#else
#define VERBOSE (LOG_UNHANDLED)
#endif
#include "logmacro.h"



//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_OEKAKIDS,      nes_oekakids_device,      "nes_oeka",        "NES Cart Bandai Oeka Kids PCB")
DEFINE_DEVICE_TYPE(NES_FCG,           nes_fcg_device,           "nes_fcg",         "NES Cart Bandai FCG PCB")
DEFINE_DEVICE_TYPE(NES_LZ93D50,       nes_lz93d50_device,       "nes_lz93d50",     "NES Cart Bandai LZ93D50 PCB")
DEFINE_DEVICE_TYPE(NES_LZ93D50_24C01, nes_lz93d50_24c01_device, "nes_lz93d50_ep1", "NES Cart Bandai LZ93D50 + 24C01 PCB")
DEFINE_DEVICE_TYPE(NES_LZ93D50_24C02, nes_lz93d50_24c02_device, "nes_lz93d50_ep2", "NES Cart Bandai LZ93D50 + 24C02 PCB")
DEFINE_DEVICE_TYPE(NES_FJUMP2,        nes_fjump2_device,        "nes_fjump2",      "NES Cart Bandai Famicom Jump II PCB")


nes_oekakids_device::nes_oekakids_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_OEKAKIDS, tag, owner, clock), m_reg(0), m_latch(0)
{
}

nes_fcg_device::nes_fcg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_irq_count(0), m_irq_enable(0), m_irq_latch(0)
{
}

nes_fcg_device::nes_fcg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_fcg_device(mconfig, NES_FCG, tag, owner, clock)
{
}

nes_lz93d50_device::nes_lz93d50_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_fcg_device(mconfig, type, tag, owner, clock)
{
}

nes_lz93d50_device::nes_lz93d50_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_lz93d50_device(mconfig, NES_LZ93D50, tag, owner, clock)
{
}

nes_lz93d50_24c01_device::nes_lz93d50_24c01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_lz93d50_device(mconfig, type, tag, owner, clock), m_i2cmem(*this, "i2cmem"), m_i2c_dir(0)
{
}

nes_lz93d50_24c01_device::nes_lz93d50_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_lz93d50_24c01_device(mconfig, NES_LZ93D50_24C01, tag, owner, clock)
{
}

nes_lz93d50_24c02_device::nes_lz93d50_24c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_lz93d50_24c01_device(mconfig, NES_LZ93D50_24C02, tag, owner, clock)
{
}

nes_fjump2_device::nes_fjump2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_lz93d50_device(mconfig, NES_FJUMP2, tag, owner, clock)
{
}



void nes_oekakids_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_oekakids_device::pcb_reset()
{
	prg32(0);
	chr4_0(0, CHRRAM);
	chr4_4(3, CHRRAM);
	m_latch = 0;
	m_reg = 0;
}

void nes_fcg_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_latch));
}

void nes_fcg_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_latch = 0;
}

void nes_lz93d50_24c01_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_latch));
	save_item(NAME(m_i2c_dir));
}

void nes_lz93d50_24c01_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_latch = 0;
	m_i2c_dir = 0;
}

void nes_fjump2_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_latch));
	save_item(NAME(m_reg));
	save_item(NAME(m_wram_enable));
}

void nes_fjump2_device::pcb_reset()
{
	m_wram_enable = 0;
	chr8(0, CHRRAM);
	memset(m_reg, 0, sizeof(m_reg));
	set_prg();

	m_irq_enable = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bandai Oeka Kids board emulation

 Games: Oeka Kids - Anpanman no Hiragana Daisuki, Oeka
 Kids - Anpanman to Oekaki Shiyou!!

 CHR RAM banking follows the PPU address bus (see
 ppu_addr_bus() below); the Oeka Kids tablet is not emulated.

 iNES: mapper 96

 In MAME: Preliminary Support.

 -------------------------------------------------*/


// NESdev INES Mapper 096: PPU A8-A9 are latched as the inner 4 KiB CHR RAM bank for $0000-$0FFF whenever PPU A13/A12
// change from anything else to 1/0. This follows the address bus itself, not the read or write strobes, so $2006 writes
// outside rendering also latch (the games select the CHR page they upload to that way); during rendering it is the
// nametable fetch that latches, and the attribute fetch right after it keeps A13/A12 at 1/0
void nes_oekakids_device::ppu_addr_bus(u16 addr)
{
	if ((addr & 0x3000) == 0x2000)
	{
		m_latch = BIT(addr, 8, 2);
		update_chr();
	}
}

void nes_oekakids_device::update_chr()
{
	chr4_0(m_reg | m_latch, CHRRAM);
	chr4_4(m_reg | 0x03, CHRRAM);
}

void nes_oekakids_device::write_h(offs_t offset, uint8_t data)
{
	LOG("oeka kids write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(data & 0x03);
	m_reg = data & 0x04;
	update_chr();
}

/*-------------------------------------------------

 Bandai FCG / LZ93D50 boards emulation

 There are several variants: plain board with or without SRAM,
 board + 24C01 EEPROM, board + 24C02 EEPROM, board + Barcode
 Reader (DATACH, see datach.cpp).

 Games: Crayon Shin-Chan - Ora to Poi Poi, Dragon Ball Z Gaiden,
 Dragon Ball Z II & III, Rokudenashi Blues, SD Gundam
 Gaiden - KGM2, Dragon Ball Z, Magical Taruruuto-kun, SD Gundam
 Gaiden [with EEPROM], Dragon Ball, Dragon Ball 3, Famicom Jump,
 Famicom Jump II [no EEPROM], Datach Games

 At the moment, we don't support EEPROM I/O

 iNES: mappers 16, 153 (see below), 157 & 159

 In MAME: Supported

 -------------------------------------------------*/

// 16bit counter clocked by M2 (m2_w calls this).  Which M2 edge clocks it is not documented in anything we found: the rise,
// i.e. before the CPU access of the same cycle, is used
void nes_fcg_device::irq_clock()
{
	if (m_irq_enable)
	{
		// 16bit counter, IRQ fired when the counter goes from 1 to 0
		// after firing, the counter is *not* reloaded, but next clock
		// counter wraps around from 0 to 0xffff
		if (!m_irq_count)
			m_irq_count = 0xffff;
		else
			m_irq_count--;

		// NESdev Bandai FCG: there is no reload and the counter does not stop - after 0 it wraps to $FFFF and keeps
		// counting; the IRQ stays asserted until a write to the IRQ control register acknowledges it
		if (!m_irq_count)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_fcg_device::fcg_write(offs_t offset, uint8_t data, bool latched)
{
	LOG("lz93d50_write, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x0f)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			chr1_x(offset & 0x07, data, m_chr_source);
			break;
		case 8:
			prg16_89ab(data);
			break;
		case 9:
			switch (data & 0x03)
			{
				case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;
		case 0x0a:
			// NESdev INES Mapper 016: the LZ93D50 counter is latched, the FCG-1/2 one is not.  Famicom Jump II (mapper 153) glitches its
			// title screen split and corrupts CHR RAM otherwise (NESdev forum, "Famicom Jump II (Mapper 153) IRQ"): $800B/$800C write a
			// shadow value that this write loads into the counter
			if (latched)
				m_irq_count = m_irq_latch;
			m_irq_enable = data & 0x01;
			set_irq_line(CLEAR_LINE);
			// NESdev INES Mapper 016: enabling counting while the counter holds zero generates an IRQ immediately
			if (m_irq_enable && !m_irq_count)
				set_irq_line(ASSERT_LINE);
			break;
		case 0x0b:
			if (latched)
				m_irq_latch = (m_irq_latch & 0xff00) | data;
			else
				m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x0c:
			if (latched)
				m_irq_latch = (m_irq_latch & 0x00ff) | (data << 8);
			else
				m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			break;
		default:
			LOGMASKED(LOG_UNHANDLED, "lz93d50_write uncaught write, offset: %04x, data: %02x\n", offset, data);
			break;
	}
}

void nes_fcg_device::write_m(offs_t offset, uint8_t data)
{
	LOG("lz93d50 write_m, offset: %04x, data: %02x\n", offset, data);

	if (m_battery.empty() && m_prgram.empty())
	{
		if (m_regs_at_6000)
			fcg_write(offset & 0x0f, data, false);  // FCG-1/2 registers: $6000-$7FFF, counter not latched
	}
	else if (!m_battery.empty())
		m_battery[offset & (m_battery.size() - 1)] = data;
	else
		m_prgram[offset & (m_prgram.size() - 1)] = data;
}

// FCG board does not access regs in 0x8000-0xffff space!
// only later design lz93d50 (and its variants do)!

void nes_lz93d50_24c01_device::write_h(offs_t offset, uint8_t data)
{
	LOG("lz93d50_24c01 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x0f)
	{
		case 0x0d:
			m_i2cmem->write_scl(BIT(data, 5));
			m_i2cmem->write_sda(BIT(data, 6));
			m_i2c_dir = BIT(data, 7);
			break;
		default:
			fcg_write(offset & 0x0f, data);
			break;
	}
}

uint8_t nes_lz93d50_24c01_device::read_m(offs_t offset)
{
	LOG("lz93d50 EEPROM read, offset: %04x\n", offset);
	if (m_i2c_dir)
		return (m_i2cmem->read_sda() & 1) << 4;
	else
		return 0;
}

//-------------------------------------------------
//  SERIAL I2C DEVICE
//-------------------------------------------------

void nes_lz93d50_24c01_device::device_add_mconfig(machine_config &config)
{
	I2C_X24C01(config, m_i2cmem);
}

void nes_lz93d50_24c02_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);
}


/*-------------------------------------------------

 Bandai BANDAI-JUMP2 boards emulation

 This is a variant of LZ93D50, with SRAM and no EEPROM
 The board is only used by Famicom Jump II, which
 has no CHR and 512K of PRG, so it is not completely
 clear if the CHR regs of LZ93D50 (i.e. offset & 0xf < 8)
 would switch also CHR or if they are only used to select
 upper 256K of PRG

 iNES: mappers 153

 In MAME: Supported

 -------------------------------------------------*/

void nes_fjump2_device::set_prg()
{
	uint8_t prg_base = 0;

	for (int i = 0; i < 4; i++)
		prg_base |= (m_reg[i] << 4);

	prg16_89ab(prg_base | m_reg[4]);
	prg16_cdef(prg_base | 0x0f);
}

uint8_t nes_fjump2_device::read_m(offs_t offset)
{
	if (!m_wram_enable)
		return get_open_bus();
	LOG("fjump2 read_m, offset: %04x\n", offset);
	// the WRAM is battery-backed on Famicom Jump II; an image without the battery flag gets it as plain PRG-RAM
	std::vector<u8> &ram = m_battery.empty() ? m_prgram : m_battery;
	if (ram.empty())
		return get_open_bus();
	return ram[offset & (ram.size() - 1)];
}

void nes_fjump2_device::write_m(offs_t offset, uint8_t data)
{
	if (!m_wram_enable)
		return;
	LOG("fjump2 write_m, offset: %04x, data: %02x\n", offset, data);
	std::vector<u8> &ram = m_battery.empty() ? m_prgram : m_battery;
	if (!ram.empty())
		ram[offset & (ram.size() - 1)] = data;
}

void nes_fjump2_device::write_h(offs_t offset, uint8_t data)
{
	LOG("fjump2 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x0f)
	{
		case 0: case 1: case 2: case 3:
			m_reg[offset & 0x0f] = BIT(data,0);
			set_prg();
			break;
		case 4: case 5: case 6: case 7:
			// these have been verified to be disabled in this board
			break;
		case 8:
			m_reg[4] = data & 0x0f;
			set_prg();
			break;
		case 0x0d:
			m_wram_enable = BIT(data, 5);
			break;
		default:
			fcg_write(offset & 0x0f, data);
			break;
	}
}
