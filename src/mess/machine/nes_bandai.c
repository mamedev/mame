/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bandai PCBs

 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.


 Here we emulate the following PCBs

 * Bandai LZ93D50 [mapper 16]
 * Bandai FCG (alt name of the LZ93D50 or something different?) [mapper 16]
 * Bandai LZ93D50 + 24C01 EEPROM [mapper 159]
 * Bandai LZ93D50 + 24C02 EEPROM [mapper 16]
 * Bandai Famicom Jump 2 (aka LZ93D50 + SRAM) [mapper 153]
 * Bandai LZ93D50 + Datach Barcode reader [mapper 34]
 * Bandai Karaoke Studio [mapper 188]
 * Bandai Oeka Kids [mapper 96]

 TODO:
 - investigate why EEPROM does not work
 - try to implement some sort of Datach emulation
 - try to implement some sort of Karaoke emulation
 - add support to the PPU for the code necessary to Oeka Kids games (also needed by UNL-DANCE2000 PCB)
 - check the cause for the flickering in Famicom Jump 2

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/nes_bandai.h"

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

const device_type NES_KARAOKESTUDIO = &device_creator<nes_karaokestudio_device>;
const device_type NES_OEKAKIDS = &device_creator<nes_oekakids_device>;
const device_type NES_LZ93D50 = &device_creator<nes_lz93d50_device>;
const device_type NES_LZ93D50_24C01 = &device_creator<nes_lz93d50_24c01_device>;
const device_type NES_LZ93D50_24C02 = &device_creator<nes_lz93d50_24c02_device>;
const device_type NES_FCG = &device_creator<nes_fcg_device>;
const device_type NES_DATACH = &device_creator<nes_datach_device>;
const device_type NES_FJUMP2 = &device_creator<nes_fjump2_device>;


nes_karaokestudio_device::nes_karaokestudio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KARAOKESTUDIO, "NES Cart Bandai Karaoke Studio PCB", tag, owner, clock, "nes_karaoke", __FILE__)
{
}

nes_oekakids_device::nes_oekakids_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_OEKAKIDS, "NES Cart Bandai Oeka Kids PCB", tag, owner, clock, "nes_oeka", __FILE__)
{
}

nes_lz93d50_device::nes_lz93d50_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_lz93d50_device::nes_lz93d50_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_LZ93D50, "NES Cart Bandai LZ93D50 PCB", tag, owner, clock, "nes_lz93d50", __FILE__)
{
}

nes_lz93d50_24c01_device::nes_lz93d50_24c01_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_lz93d50_device(mconfig, type, name, tag, owner, clock, shortname, source),
						m_i2cmem(*this, "i2cmem")
{
}

nes_lz93d50_24c01_device::nes_lz93d50_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_lz93d50_device(mconfig, NES_LZ93D50_24C01, "NES Cart Bandai LZ93D50 + 24C01 PCB", tag, owner, clock, "nes_lz93d50_ep1", __FILE__),
						m_i2cmem(*this, "i2cmem")
{
}

nes_lz93d50_24c02_device::nes_lz93d50_24c02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_lz93d50_24c01_device(mconfig, NES_LZ93D50_24C02, "NES Cart Bandai LZ93D50 + 24C02 PCB", tag, owner, clock, "nes_lz93d50_ep2", __FILE__)
{
}

nes_fcg_device::nes_fcg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_lz93d50_device(mconfig, NES_FCG, "NES Cart Bandai FCG PCB", tag, owner, clock, "nes_fcg", __FILE__)
{
}

nes_datach_device::nes_datach_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_lz93d50_device(mconfig, NES_DATACH, "NES Cart Bandai Datach PCB", tag, owner, clock, "nes_datach", __FILE__)
{
}

nes_fjump2_device::nes_fjump2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_lz93d50_device(mconfig, NES_FJUMP2, "NES Cart Bandai Famicom Jump II PCB", tag, owner, clock, "nes_fjump2", __FILE__)
{
}




void nes_karaokestudio_device::device_start()
{
	common_start();
}

void nes_karaokestudio_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef((m_prg_chunks - 1) ^ 0x08);
	chr8(0, m_chr_source);
}

void nes_oekakids_device::device_start()
{
	common_start();
}

void nes_oekakids_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_lz93d50_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
}

void nes_lz93d50_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
}

void nes_lz93d50_24c01_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_i2c_mem));
	save_item(NAME(m_i2c_clk));
}

void nes_lz93d50_24c01_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_i2c_mem = 0;
	m_i2c_clk = 0;
}

void nes_fjump2_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_fjump2_device::pcb_reset()
{
	chr8(0, CHRRAM);
	memset(m_reg, 0, sizeof(m_reg));
	set_prg();
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bandai Karaoke Studio board emulation

 Games: Karaoke Studio

 Note: we currently do not emulate the mic

 iNES: mapper 188

 -------------------------------------------------*/

WRITE8_MEMBER(nes_karaokestudio_device::write_h)
{
	LOG_MMC(("karaoke studio write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(data ^ 0x08);
}

/*-------------------------------------------------

 Bandai Oeka Kids board emulation

 Games: Oeka Kids - Anpanman no Hiragana Daisuki, Oeka
 Kids - Anpanman to Oekaki Shiyou!!

 This board can swap CHR whenever a PPU address line is
 changed and we still do not emulate this.

 iNES: mapper 96

 In MESS: Preliminary Support.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_oekakids_device::write_h)
{
	LOG_MMC(("oeka kids write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
	chr4_0(0x00 | (data & 0x04), CHRRAM);
	chr4_4(0x03 | (data & 0x04), CHRRAM);
}

/*-------------------------------------------------

 Bandai LZ93D50 boards emulation

 There are several variants: plain board with or without SRAM,
 board + 24C01 EEPROM, board + 24C02 EEPROM, board + Barcode
 Reader (DATACH).
 We currently only emulate the base hardware.

 Games: Crayon Shin-Chan - Ora to Poi Poi, Dragon Ball Z Gaiden,
 Dragon Ball Z II & III, Rokudenashi Blues, SD Gundam
 Gaiden - KGM2, Dragon Ball Z, Magical Taruruuto-kun, SD Gundam
 Gaiden [with EEPROM], Dragon Ball, Dragon Ball 3, Famicom Jump,
 Famicom Jump II [no EEPROM], Datach Games

 At the moment, we don't support EEPROM I/O

 iNES: mappers 16, 153 (see below), 157 & 159

 In MESS: Supported

 -------------------------------------------------*/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
 we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
 there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
void nes_lz93d50_device::hblank_irq(int scanline, int vblank, int blanked)
{
	/* 114 is the number of cycles per scanline */
	/* TODO: change to reflect the actual number of cycles spent */
	if (m_irq_enable)
	{
		if (m_irq_count <= 114)
		{
			machine().device("maincpu")->execute().set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			m_irq_count = (0xffff - 114 + m_irq_count);   // wrap around the 16 bits counter
		}
		m_irq_count -= 114;
	}
}

WRITE8_MEMBER(nes_lz93d50_device::lz93d50_write)
{
	LOG_MMC(("lz93d50_write, offset: %04x, data: %02x\n", offset, data));

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
			m_irq_enable = data & 0x01;
			break;
		case 0x0b:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x0c:
			m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			break;
		default:
			logerror("lz93d50_write uncaught write, offset: %04x, data: %02x\n", offset, data);
			break;
	}
}

WRITE8_MEMBER(nes_lz93d50_device::write_m)
{
	LOG_MMC(("lz93d50 write_m, offset: %04x, data: %02x\n", offset, data));

	if (!m_battery && !m_prgram)
		lz93d50_write(space, offset & 0x0f, data, mem_mask);
	else if (m_battery)
		m_battery[offset] = data;
	else
		m_prgram[offset] = data;
}


WRITE8_MEMBER(nes_lz93d50_24c01_device::write_h)
{
	LOG_MMC(("lz93d50_24c01 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x0f)
	{
		case 0x0d:
			m_i2c_clk = BIT(data, 5);
			m_i2c_mem = BIT(data, 6);
			i2cmem_scl_write(m_i2cmem, m_i2c_clk);
			i2cmem_sda_write(m_i2cmem, m_i2c_mem);
			break;
		default:
			lz93d50_write(space, offset & 0x0f, data, mem_mask);
			break;
	}
}

READ8_MEMBER(nes_lz93d50_24c01_device::read_m)
{
	LOG_MMC(("lz93d50 EEPROM read, offset: %04x\n", offset));
	return (i2cmem_sda_read(m_i2cmem) & 1) << 4;
}

//-------------------------------------------------
//  SERIAL I2C DEVICE
//-------------------------------------------------

static const i2cmem_interface bandai_24c01_i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 4, 0x80
};

MACHINE_CONFIG_FRAGMENT( bandai_i2c_24c01 )
	MCFG_I2CMEM_ADD("i2cmem", bandai_24c01_i2cmem_interface)
MACHINE_CONFIG_END

machine_config_constructor nes_lz93d50_24c01_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bandai_i2c_24c01 );
}



static const i2cmem_interface bandai_24c02_i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 4, 0x100
};


MACHINE_CONFIG_FRAGMENT( bandai_i2c_24c02 )
	MCFG_I2CMEM_ADD("i2cmem", bandai_24c02_i2cmem_interface)
MACHINE_CONFIG_END

machine_config_constructor nes_lz93d50_24c02_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bandai_i2c_24c02 );
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

 In MESS: Supported

 -------------------------------------------------*/

void nes_fjump2_device::set_prg()
{
	UINT8 prg_base = 0;

	for (int i = 0; i < 8; i++)
		prg_base |= ((m_reg[i] & 0x01) << 4);

	prg16_89ab(prg_base | m_reg[8]);
	prg16_cdef(prg_base | 0x0f);
}

WRITE8_MEMBER(nes_fjump2_device::write_h)
{
	LOG_MMC(("fjump2 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x0f)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
		case 8:
			m_reg[offset & 0x0f] = data & 0x0f;
			set_prg();
			break;
		default:
			lz93d50_write(space, offset & 0x0f, data, mem_mask);
			break;
	}
}
