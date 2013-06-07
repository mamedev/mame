/***********************************************************************************************************
 
 SA-1 add-on chip emulation (for SNES/SFC)
  
 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.

 Note:
 - SA-1 register description below is based on no$cash docs. 
 - about bankswitch handling: no matter what is ROM size, at loading the ROM is mirrored up to 8MB and a
   rom_bank_map[0x100] array is built as a lookup table for 256x32KB banks filling the 8MB accessible ROM
   area; this allows to handle any 0-7 value written to CXB/DXB/EXB/FXB SA-1 registers without any masking!
 - about BWRAM "bitmap mode": in 2bits mode 
     600000h.Bit0-1 mirrors to 400000h.Bit0-1
     600001h.Bit0-1 mirrors to 400000h.Bit2-3
     600002h.Bit0-1 mirrors to 400000h.Bit4-5
     600003h.Bit0-1 mirrors to 400000h.Bit6-7
     ...
   in 4bits mode
     600000h.Bit0-3 mirrors to 400000h.Bit0-3
     600001h.Bit0-3 mirrors to 400000h.Bit4-7
     600002h.Bit0-3 mirrors to 400001h.Bit0-3
     600003h.Bit0-3 mirrors to 400001h.Bit4-7
     ...
   to handle the separate modes, bitmap accesses go to offset + 0x100000

 TODO:
 - test case for BWRAM & IRAM write protect (bsnes does not seem to ever protect either, so it's not implemented 
   for the moment)
 - almost everything CPU related!
 
 ***********************************************************************************************************/


#include "emu.h"
#include "machine/sns_sa1.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type SNS_LOROM_SA1 = &device_creator<sns_sa1_device>;


sns_sa1_device::sns_sa1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SNS_LOROM_SA1, "SNES Cart + SA-1", tag, owner, clock, "sns_rom_sa1", __FILE__),
						device_sns_cart_interface( mconfig, *this ),
						m_sa1(*this, "sa1cpu")
{
}


void sns_sa1_device::device_start()
{
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------
  RAM / SRAM / Registers
 -------------------------------------------------*/

UINT8 sns_sa1_device::read_regs(UINT32 offset)
{
	UINT8 value = 0xff;
	offset &= 0x1ff;	// $2200 + offset gives the reg value to compare with docs
	
	switch (offset)
	{
		case 0x100:
			// SNES  SFR   SNES CPU Flag Read (R)
			break;
		case 0x101:
			// SA-1  CFR   SA-1 CPU Flag Read (R)
			break;
		case 0x102:
			// SA-1  HCR   H-Count Read Lsb / Do Latching (R)
			break;
		case 0x103:
			// SA-1  HCR   H-Count Read Msb (R)
			break;
		case 0x104:
			// SA-1  VCR   V-Count Read Lsb (R)
			break;
		case 0x105:
			// SA-1  VCR   V-Count Read Msb (R)
			break;
		case 0x106:
			// SA-1  MR    Arithmetic Result, bit0-7   (Sum/Product/Quotient) (R)
			break;
		case 0x107:
			// SA-1  MR    Arithmetic Result, bit8-15  (Sum/Product/Quotient) (R)
			break;
		case 0x108:
			// SA-1  MR    Arithmetic Result, bit16-23 (Sum/Product/Remainder) (R)
			break;
		case 0x109:
			// SA-1  MR    Arithmetic Result, bit24-31 (Sum/Product/Remainder) (R)
			break;
		case 0x10a:
			// SA-1  MR    Arithmetic Result, bit32-39 (Sum) (R)
			break;
		case 0x10b:
			// SA-1  OF    Arithmetic Overflow Flag (R)
			break;
		case 0x10c:
			// SA-1  VDP   Variable-Length Data Read Port Lsb (R)
			break;
		case 0x10d:
			// SA-1  VDP   Variable-Length Data Read Port Msb (R)
			break;
		case 0x10e:
			// SNES  VC    Version Code Register (R)
			break;
		default:
			logerror("SA-1 Read access to an unmapped reg (%x)", offset);
			break;
	}
	return value;
}

void sns_sa1_device::write_regs(UINT32 offset, UINT8 data)
{
	offset &= 0x1ff;	// $2200 + offset gives the reg value to compare with docs
	
	switch (offset)
	{
		case 0x000:
			// SNES  CCNT  20h   SA-1 CPU Control (W)
			break;
		case 0x001:
			// SNES  SIE   00h   SNES CPU Int Enable (W)
			break;
		case 0x002:
			// SNES  SIC   00h   SNES CPU Int Clear  (W)
			break;
		case 0x003:
			// SNES  CRV   -     SA-1 CPU Reset Vector Lsb (W)
			break;
		case 0x004:
			// SNES  CRV   -     SA-1 CPU Reset Vector Msb (W)
			break;
		case 0x005:
			// SNES  CNV   -     SA-1 CPU NMI Vector Lsb (W)
			break;
		case 0x006:
			// SNES  CNV   -     SA-1 CPU NMI Vector Msb (W)
			break;
		case 0x007:
			// SNES  CIV   -     SA-1 CPU IRQ Vector Lsb (W)
			break;
		case 0x008:
			// SNES  CIV   -     SA-1 CPU IRQ Vector Msb (W)
			break;
		case 0x009:
			// SA-1  SCNT  00h   SNES CPU Control (W)
			break;
		case 0x00a:
			// SA-1  CIE   00h   SA-1 CPU Int Enable (W)
			break;
		case 0x00b:
			// SA-1  CIC   00h   SA-1 CPU Int Clear  (W)
			break;
		case 0x00c:
			// SA-1  SNV   -     SNES CPU NMI Vector Lsb (W)
			break;
		case 0x00d:
			// SA-1  SNV   -     SNES CPU NMI Vector Msb (W)
			break;
		case 0x00e:
			// SA-1  SIV   -     SNES CPU IRQ Vector Lsb (W)
			break;
		case 0x00f:
			// SA-1  SIV   -     SNES CPU IRQ Vector Msb (W)
			break;
		case 0x010:
			// SA-1  TMC   00h   H/V Timer Control (W)
			break;
		case 0x011:
			// SA-1  CTR   -     SA-1 CPU Timer Restart (W)
			break;
		case 0x012:
			// SA-1  HCNT  -     Set H-Count Lsb (W)
			break;
		case 0x013:
			// SA-1  HCNT  -     Set H-Count Msb (W)
			break;
		case 0x014:
			// SA-1  VCNT  -     Set V-Count Lsb (W)
			break;
		case 0x015:
			// SA-1  VCNT  -     Set V-Count Msb (W)
			break;
		case 0x020:
			// ROM 1MB bank for [c0-cf]
			m_bank_c_hi = BIT(data, 7);
			m_bank_c_rom = data & 0x07;
			break;
		case 0x021:
			// ROM 1MB bank for [d0-df]
			m_bank_d_hi = BIT(data, 7);
			m_bank_d_rom = data & 0x07;
			break;
		case 0x022:
			// ROM 1MB bank for [e0-ef]
			m_bank_e_hi = BIT(data, 7);
			m_bank_e_rom = data & 0x07;
			break;
		case 0x023:
			// ROM 1MB bank for [f0-ff]
			m_bank_f_hi = BIT(data, 7);
			m_bank_f_rom = data & 0x07;
			break;
		case 0x024:
			// BWRAM bank from SNES side
			m_bwram_snes = data & 0x1f;	// max 32x8K banks
			break;
		case 0x025:
			// BWRAM bank & type from SA-1 side
			m_bwram_sa1_source = BIT(data, 7);	// 0 = normal, 1 = bitmap?
			m_bwram_sa1 = data & 0x7f;	// up to 128x8K banks here?
			break;
		case 0x026:
			// enable writing to BWRAM from SNES
			m_bwram_write_snes = BIT(data, 7);
			break;
		case 0x027:
			// enable writing to BWRAM from SA-1
			m_bwram_write_sa1 = BIT(data, 7);
			break;
		case 0x028:
			// write protected area at bottom of BWRAM
			m_bwpa_sa1 = 0x100 * (data & 0x0f);
			break;
		case 0x029:
			// enable writing to IRAM from SNES (1 bit for each 0x100 chunk)
			m_iram_write_snes = data;
			break;
		case 0x02a:
			// enable writing to IRAM from SA-1 (1 bit for each 0x100 chunk)
			m_iram_write_sa1 = data;
			break;
		case 0x030:
			// SA-1  DCNT  00h   DMA Control (W)
			break;
		case 0x031:
			// Both  CDMA  00h   Character Conversion DMA Parameters (W)
			break;
		case 0x032:
			// Both  SDA   -     DMA Source Device Start Address Lsb (W)
			break;
		case 0x033:
			// Both  SDA   -     DMA Source Device Start Address Mid (W)
			break;
		case 0x034:
			// Both  SDA   -     DMA Source Device Start Address Msb (W)
			break;
		case 0x035:
			// Both  DDA   -     DMA Dest Device Start Address Lsb (W)
			break;
		case 0x036:
			// Both  DDA   -     DMA Dest Device Start Address Mid (Start/I-RAM) (W)
			break;
		case 0x037:
			// Both  DDA   -     DMA Dest Device Start Address Msb (Start/BW-RAM)(W)
			break;
		case 0x038:
			// SA-1  DTC   -     DMA Terminal Counter Lsb (W)
			break;
		case 0x039:
			// SA-1  DTC   -     DMA Terminal Counter Msb (W)
			break;
		case 0x03f:
			// Format for BWRAM when mapped to bitmap
			m_bwram_sa1_format = BIT(data, 7);	// 0 = 4bit, 1 = 2bit
			break;
		case 0x040:
		case 0x041:
		case 0x042:
		case 0x043:
		case 0x044:
		case 0x045:
		case 0x046:
		case 0x047:
		case 0x048:
		case 0x049:
		case 0x04a:
		case 0x04b:
		case 0x04c:
		case 0x04d:
		case 0x04e:
		case 0x04f:
			// SA-1  BRF   -     Bit Map Register File (2240h..224Fh) (W)
			break;
		case 0x050:
			// SA-1  MCNT  00h   Arithmetic Control (W)
			break;
		case 0x051:
			// SA-1  MA    -     Arithmetic Param A Lsb (Multiplicand/Dividend) (W)
			break;
		case 0x052:
			// SA-1  MA    -     Arithmetic Param A Msb (Multiplicand/Dividend) (W)
			break;
		case 0x053:
			// SA-1  MB    -     Arithmetic Param B Lsb (Multiplier/Divisor) (W)
			break;
		case 0x054:
			// SA-1  MB    -     Arithmetic Param B Msb (Multiplier/Divisor)/Start (W)
			break;
		case 0x058:
			// SA-1  VBD   -     Variable-Length Bit Processing (W)
			break;
		case 0x059:
			// SA-1  VDA   -     Var-Length Bit Game Pak ROM Start Address Lsb (W)
			break;
		case 0x05a:
			// SA-1  VDA   -     Var-Length Bit Game Pak ROM Start Address Mid (W)
			break;
		case 0x05b:
			// SA-1  VDA   -     Var-Length Bit Game Pak ROM Start Address Msb & Kick
			break;
		default:
			logerror("SA-1 Write access to an unmapped reg (%x) with data %x", offset, data);
			break;
	}
}

UINT8 sns_sa1_device::read_iram(UINT32 offset)
{
	return m_internal_ram[offset & 0x7ff];
}

void sns_sa1_device::write_iram(UINT32 offset, UINT8 data)
{
	m_internal_ram[offset & 0x7ff] = data;
}

UINT8 sns_sa1_device::read_bwram(UINT32 offset)
{
	int shift = 0;
	UINT8 mask = 0xff;
	
	if (!m_nvram)
		return 0xff;	// this should probably never happen, or are there SA-1 games with no BWRAM?
	
	if (offset < 0x100000)
		return m_nvram[offset & (m_nvram_size - 1)];
	
	// Bitmap BWRAM
	if (m_bwram_sa1_format)
	{
		// 2bits mode
		offset /= 4;
		shift = ((offset % 4) * 2);
		mask = 0x03;
	}
	else
	{
		// 4bits mode
		offset /= 2;
		shift = ((offset % 2) * 4);
		mask = 0x0f;
	}
	
	// only return the correct bits
	return (m_nvram[offset & (m_nvram_size - 1)] >> shift) & mask;
}

void sns_sa1_device::write_bwram(UINT32 offset, UINT8 data)
{
	UINT8 mask = 0xff;
	
	if (!m_nvram)
		return;	// this should probably never happen, or are there SA-1 games with no BWRAM?
	
	if (offset < 0x100000)
	{
		m_nvram[offset & (m_nvram_size - 1)] = data;
		return;
	}
	
	// Bitmap BWRAM
	if (m_bwram_sa1_format)
	{
		// 2bits mode
		offset /= 4;
		data = (data & 0x03) << ((offset % 4) * 2);
		mask = 0x03 << ((offset % 4) * 2);
	}
	else
	{
		// 4bits mode
		offset /= 2;
		data = (data & 0x0f) << ((offset % 2) * 4);
		mask = 0x0f << ((offset % 2) * 4);
	}
	
	// only change the correct bits, keeping the rest untouched
	m_nvram[offset & (m_nvram_size - 1)] = (m_nvram[offset & (m_nvram_size - 1)] & ~mask) | data;
}



/*-------------------------------------------------
  Accesses from SNES CPU
 -------------------------------------------------*/


READ8_MEMBER(sns_sa1_device::read_l)
{
	int bank = 0;

	// ROM is mapped to [00-3f][8000-ffff] only here
	if (offset < 0x200000)
	{
		if (!m_bank_c_hi)	// when HiROM mapping is disabled, we always access first 1MB here
			bank = (offset / 0x10000) + 0x00;
		else	// when HiROM mapping is enabled, we mirror [c0-cf][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_c_rom * 0x20);

		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else if (offset < 0x400000)
	{
		offset -= 0x200000;
		if (!m_bank_d_hi)	// when HiROM mapping is disabled, we always access second 1MB here
			bank = (offset / 0x10000) + 0x20;
		else	// when HiROM mapping is enabled, we mirror [d0-df][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_d_rom * 0x20);
		
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else
		return 0; // this should not happen (the driver should only call read_l in the above case)
}

READ8_MEMBER(sns_sa1_device::read_h)
{
	int bank = 0;
	
	// ROM is mapped to [80-bf][8000-ffff] & [c0-ff][0000-ffff]
	if (offset < 0x200000)
	{
		if (!m_bank_e_hi)	// when HiROM mapping is disabled, we always access third 1MB here
			bank = (offset / 0x10000) + 0x40;
		else	// when HiROM mapping is enabled, we mirror [e0-ef][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_e_rom * 0x20);
		
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else if (offset < 0x400000)
	{
		offset -= 0x200000;
		if (!m_bank_f_hi)	// when HiROM mapping is disabled, we always access fourth 1MB here
			bank = (offset / 0x10000) + 0x60;
		else	// when HiROM mapping is enabled, we mirror [f0-ff][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_f_rom * 0x20);
		
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else if (offset < 0x500000)
		return m_rom[rom_bank_map[(m_bank_c_rom * 0x20) + (offset / 0x8000)] * 0x8000 + (offset & 0x7fff)];
	else if (offset < 0x600000)
		return m_rom[rom_bank_map[(m_bank_d_rom * 0x20) + (offset / 0x8000)] * 0x8000 + (offset & 0x7fff)];
	else if (offset < 0x700000)
		return m_rom[rom_bank_map[(m_bank_e_rom * 0x20) + (offset / 0x8000)] * 0x8000 + (offset & 0x7fff)];
	else
		return m_rom[rom_bank_map[(m_bank_f_rom * 0x20) + (offset / 0x8000)] * 0x8000 + (offset & 0x7fff)];
}

WRITE8_MEMBER(sns_sa1_device::write_l)
{
}

WRITE8_MEMBER(sns_sa1_device::write_h)
{
}

READ8_MEMBER( sns_sa1_device::chip_read )
{
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000 && address >= 0x2200 && address < 0x2400)
		return read_regs(address & 0x1ff);	// SA-1 Regs
	
	if (offset < 0x400000 && address >= 0x3000 && address < 0x3800)
		return read_iram(address & 0x7ff);	// Internal SA-1 RAM (2K)

	if (offset < 0x400000 && address >= 0x6000 && address < 0x8000)
		return read_bwram((m_bwram_snes * 0x2000) + (offset & 0x1fff));	// SA-1 BWRAM

	if (offset >= 0x400000 && offset < 0x500000)
		return read_bwram(offset);	// SA-1 BWRAM again	(but not called for the [c0-cf] range, because it's not mirrored)
	
	return 0xff;
}


WRITE8_MEMBER( sns_sa1_device::chip_write )
{
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000 && address >= 0x2200 && address < 0x2400)
		write_regs(address & 0x1ff, data);	// SA-1 Regs
	
	if (offset < 0x400000 && address >= 0x3000 && address < 0x3800)
		write_iram(address & 0x7ff, data);	// Internal SA-1 RAM (2K)
	
	if (offset < 0x400000 && address >= 0x6000 && address < 0x8000)
		write_bwram((m_bwram_snes * 0x2000) + (offset & 0x1fff), data);	// SA-1 BWRAM
	
	if (offset >= 0x400000 && offset < 0x500000)
		write_bwram(offset, data);	// SA-1 BWRAM again	(but not called for the [c0-cf] range, because it's not mirrored)
}


/*-------------------------------------------------
  Accesses from SA-1 CPU
 -------------------------------------------------*/

// These handlers basically match the SNES CPU ones, but there is no access to internal
// I/O regs or WRAM, and there are a few additional accesses to IRAM (in [00-3f][0000-07ff])
// and to BWRAM (in [60-6f][0000-ffff], so-called bitmap mode)

READ8_MEMBER( sns_sa1_device::sa1_hi_r )
{
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				return read_iram(offset);	// Internal SA-1 RAM (2K)
			else if (address >= 0x2200 && address < 0x2400)
				return read_regs(offset & 0x1ff);	// SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				return read_iram(offset);	// Internal SA-1 RAM (2K)
		}
		else if (address < 0x8000)
			return read_bwram((m_bwram_sa1 * 0x2000) + (offset & 0x1fff) + (m_bwram_sa1_source * 0x100000));		// SA-1 BWRAM
		else
			return read_h(space, offset);	// ROM
		
		return 0xff;	// maybe open bus? same as the main system one or diff? (currently not accessible from carts anyway...)
	}
	else
		return read_h(space, offset);	// ROM
}

READ8_MEMBER( sns_sa1_device::sa1_lo_r )
{
	UINT16 address = offset & 0xffff;
	
	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				return read_iram(offset);	// Internal SA-1 RAM (2K)
			else if (address >= 0x2200 && address < 0x2400)
				return read_regs(offset & 0x1ff);	// SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				return read_iram(offset);	// Internal SA-1 RAM (2K)
		}
		else if (address < 0x8000)
			return read_bwram((m_bwram_sa1 * 0x2000) + (offset & 0x1fff) + (m_bwram_sa1_source * 0x100000));		// SA-1 BWRAM
		else
			return read_l(space, offset);	// ROM
		
		return 0xff;	// maybe open bus? same as the main system one or diff? (currently not accessible from carts anyway...)
	}
	else if (offset < 0x500000)
		return read_bwram(offset);		// SA-1 BWRAM (not mirrored above!)
	else if (offset >= 0x600000 && offset < 0x700000)
		return read_bwram((offset & 0xfffff) + 0x100000);		// SA-1 BWRAM Bitmap mode
	else		
		return 0xff;	// nothing should be mapped here, so maybe open bus?
}

WRITE8_MEMBER( sns_sa1_device::sa1_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				write_iram(offset, data);	// Internal SA-1 RAM (2K)
			else if (address >= 0x2200 && address < 0x2400)
				write_regs(offset & 0x1ff, data);	// SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				write_iram(offset, data);	// Internal SA-1 RAM (2K)
		}
		else if (address < 0x8000)
			write_bwram((m_bwram_sa1 * 0x2000) + (offset & 0x1fff) + (m_bwram_sa1_source * 0x100000), data);		// SA-1 BWRAM
	}
}

WRITE8_MEMBER( sns_sa1_device::sa1_lo_w )
{
	if (offset >= 0x400000 && offset < 0x500000)
		write_bwram(offset & 0xfffff, data);		// SA-1 BWRAM (not mirrored above!)
	else if (offset >= 0x600000 && offset < 0x700000)
		write_bwram((offset & 0xfffff) + 0x100000, data);		// SA-1 BWRAM Bitmap mode
	else
		sa1_hi_w(space, offset, data);
}

static ADDRESS_MAP_START( sa1_map, AS_PROGRAM, 8, sns_sa1_device )
	AM_RANGE(0x000000, 0x7dffff) AM_READWRITE(sa1_lo_r, sa1_lo_w)
	AM_RANGE(0x7e0000, 0x7fffff) AM_NOP
	AM_RANGE(0x800000, 0xffffff) AM_READWRITE(sa1_hi_r, sa1_hi_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( snes_sa1 )
	MCFG_CPU_ADD("sa1cpu", _5A22, 10000000)
	MCFG_CPU_PROGRAM_MAP(sa1_map)
MACHINE_CONFIG_END

machine_config_constructor sns_sa1_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_sa1 );
}
