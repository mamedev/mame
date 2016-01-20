// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#include "emu.h"
#include "debugger.h"
#include "superfx.h"


const device_type SUPERFX = &device_creator<superfx_device>;

superfx_device::superfx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SUPERFX, "SuperFX", tag, owner, clock, "superfx", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 32, 0)
	, m_out_irq_func(*this), m_pipeline(0), m_ramaddr(0), m_sfr(0), m_pbr(0), m_rombr(0), m_rambr(0), m_cbr(0), m_scbr(0), m_scmr(0), m_colr(0), m_por(0),
	m_bramr(0), m_vcr(0), m_cfgr(0), m_clsr(0), m_romcl(0), m_romdr(0), m_ramcl(0), m_ramar(0), m_ramdr(0), m_sreg(nullptr), m_sreg_idx(0), m_dreg(nullptr),
	m_dreg_idx(0), m_r15_modified(0), m_irq(0), m_cache_access_speed(0), m_memory_access_speed(0), m_program(nullptr), m_icount(0), m_debugger_temp(0)
{
}


#define SUPERFX_SFR_OV_SET      ((m_sfr & SUPERFX_SFR_OV) ? 1 : 0)
#define SUPERFX_SFR_OV_CLEAR    ((m_sfr & SUPERFX_SFR_OV) ? 0 : 1)
#define SUPERFX_SFR_S_SET       ((m_sfr & SUPERFX_SFR_S) ? 1 : 0)
#define SUPERFX_SFR_S_CLEAR     ((m_sfr & SUPERFX_SFR_S) ? 0 : 1)
#define SUPERFX_SFR_CY_SET      ((m_sfr & SUPERFX_SFR_CY) ? 1 : 0)
#define SUPERFX_SFR_CY_CLEAR    ((m_sfr & SUPERFX_SFR_CY) ? 0 : 1)
#define SUPERFX_SFR_Z_SET       ((m_sfr & SUPERFX_SFR_Z) ? 1 : 0)
#define SUPERFX_SFR_Z_CLEAR     ((m_sfr & SUPERFX_SFR_Z) ? 0 : 1)


void superfx_device::superfx_regs_reset()
{
	m_sfr &= ~(SUPERFX_SFR_B | SUPERFX_SFR_ALT3);

	m_sreg = &m_r[0];
	m_dreg = &m_r[0];
	m_dreg_idx = 0;
	m_sreg_idx = 0;
}

void superfx_device::superfx_update_speed()
{
	m_cache_access_speed = (m_clsr ? 1 : 2);
	m_memory_access_speed = (m_clsr ? 5 : 6);
	if(m_clsr)
	{
		m_cfgr &= ~SUPERFX_CFGR_MS0; // Cannot use high-speed multiplication in 21MHz mode
	}
}

void superfx_device::superfx_cache_flush()
{
	UINT32 n = 0;
	for(n = 0; n < 32; n++)
	{
		m_cache.valid[n] = 0;
	}
}

UINT8 superfx_device::superfx_cache_mmio_read(UINT32 addr)
{
	addr = (addr + m_cbr) & 0x1ff;
	return m_cache.buffer[addr];
}

void superfx_device::superfx_cache_mmio_write(UINT32 addr, UINT8 data)
{
	addr = (addr + m_cbr) & 0x1ff;
	m_cache.buffer[addr] = data;
	if((addr & 15) == 15)
	{
		m_cache.valid[addr >> 4] = 1;
	}
}

void superfx_device::superfx_memory_reset()
{
	UINT32 n = 0;
	for(n = 0; n < 0x200; n++)
	{
		m_cache.buffer[n] = 0x00;
	}
	for(n = 0; n < 0x20; n++)
	{
		m_cache.valid[n] = 0;
	}
	for(n = 0; n < 2; n++)
	{
		m_pixelcache[n].offset = ~0;
		m_pixelcache[n].bitpend = 0x00;
	}
}

UINT8 superfx_device::superfx_bus_read(UINT32 addr)
{
	return m_program->read_byte(addr);
}

void superfx_device::superfx_bus_write(UINT32 addr, UINT8 data)
{
	m_program->write_byte(addr, data);
}

void superfx_device::superfx_pixelcache_flush(INT32 line)
{
	UINT8 x = m_pixelcache[line].offset << 3;
	UINT8 y = m_pixelcache[line].offset >> 5;
	UINT32 cn = 0;
	UINT32 bpp = 2 << ((m_scmr & SUPERFX_SCMR_MD) - ((m_scmr & SUPERFX_SCMR_MD) >> 1)); // = [regs.scmr.md]{ 2, 4, 4, 8 };
	UINT32 addr;
	UINT32 n = 0;

	if(m_pixelcache[line].bitpend == 0x00)
	{
		return;
	}

	switch(((m_por & SUPERFX_POR_OBJ) ? SUPERFX_SCMR_HT3 : (m_scmr & SUPERFX_SCMR_HT_MASK)))
	{
		case SUPERFX_SCMR_HT0:
			cn = ((x & 0xf8) << 1) + ((y & 0xf8) >> 3);
			break;
		case SUPERFX_SCMR_HT1:
			cn = ((x & 0xf8) << 1) + ((x & 0xf8) >> 1) + ((y & 0xf8) >> 3);
			break;
		case SUPERFX_SCMR_HT2:
			cn = ((x & 0xf8) << 1) + ((x & 0xf8) << 0) + ((y & 0xf8) >> 3);
			break;
		case SUPERFX_SCMR_HT3:
			cn = ((y & 0x80) << 2) + ((x & 0x80) << 1) + ((y & 0x78) << 1) + ((x & 0x78) >> 3);
			break;
	}

	addr = 0x700000 + (cn * (bpp << 3)) + (m_scbr << 10) + ((y & 0x07) * 2);

	for(n = 0; n < bpp; n++)
	{
		UINT32 byte = ((n >> 1) << 4) + (n & 1);  // = [n]{ 0, 1, 16, 17, 32, 33, 48, 49 };
		UINT8 data = 0x00;
		UINT32 x32 = 0;
		for(x32 = 0; x32 < 8; x32++)
		{
			data |= ((m_pixelcache[line].data[x32] >> n) & 1) << x32;
		}
		if(m_pixelcache[line].bitpend != 0xff)
		{
			superfx_add_clocks_internal(m_memory_access_speed);
			data &= m_pixelcache[line].bitpend;
			data |= superfx_bus_read(addr + byte) & ~m_pixelcache[line].bitpend;
		}
		superfx_add_clocks_internal(m_memory_access_speed);
		superfx_bus_write(addr + byte, data);
	}

	m_pixelcache[line].bitpend = 0x00;
}

void superfx_device::superfx_plot(UINT8 x, UINT8 y)
{
	UINT8 color = m_colr;
	UINT16 offset = (y << 5) + (x >> 3);

	if((m_por & SUPERFX_POR_DITHER) != 0 && (m_scmr & SUPERFX_SCMR_MD) != 3)
	{
		if((x ^ y) & 1)
		{
			color >>= 4;
		}
		color &= 0x0f;
	}

	if((m_por & SUPERFX_POR_TRANSPARENT) == 0)
	{
		if((m_scmr & SUPERFX_SCMR_MD) == 3)
		{
			if(m_por & SUPERFX_POR_FREEZEHIGH)
			{
				if((color & 0x0f) == 0)
				{
					return;
				}
			}
			else
			{
				if(color == 0)
				{
					return;
				}
			}
		}
		else
		{
			if((color & 0x0f) == 0)
			{
				return;
			}
		}
	}

	if(offset != m_pixelcache[0].offset)
	{
		superfx_pixelcache_flush(1);
		m_pixelcache[1] = m_pixelcache[0];
		m_pixelcache[0].bitpend = 0x00;
		m_pixelcache[0].offset = offset;
	}

	x = (x & 7) ^ 7;
	m_pixelcache[0].data[x] = color;
	m_pixelcache[0].bitpend |= 1 << x;
	if(m_pixelcache[0].bitpend == 0xff)
	{
		superfx_pixelcache_flush(1);
		m_pixelcache[1] = m_pixelcache[0];
		m_pixelcache[0].bitpend = 0x00;
	}
}

UINT8 superfx_device::superfx_rpix(UINT8 x, UINT8 y)
{
	UINT32 cn = 0;
	UINT32 bpp;
	UINT32 addr;
	UINT8 data = 0x00;
	UINT32 n = 0;

	superfx_pixelcache_flush(1);
	superfx_pixelcache_flush(0);

	bpp = 2 << ((m_scmr & SUPERFX_SCMR_MD) - ((m_scmr & SUPERFX_SCMR_MD) >> 1)); // = [regs.scmr.md]{ 2, 4, 4, 8 };

	switch((m_por & SUPERFX_POR_OBJ) ? SUPERFX_SCMR_HT3 : (m_scmr & SUPERFX_SCMR_HT_MASK))
	{
		case SUPERFX_SCMR_HT0:
			cn = ((x & 0xf8) << 1) + ((y & 0xf8) >> 3);
			break;
		case SUPERFX_SCMR_HT1:
			cn = ((x & 0xf8) << 1) + ((x & 0xf8) >> 1) + ((y & 0xf8) >> 3);
			break;
		case SUPERFX_SCMR_HT2:
			cn = ((x & 0xf8) << 1) + ((x & 0xf8) << 0) + ((y & 0xf8) >> 3);
			break;
		case SUPERFX_SCMR_HT3:
			cn = ((y & 0x80) << 2) + ((x & 0x80) << 1) + ((y & 0x78) << 1) + ((x & 0x78) >> 3);
			break;
	}

	addr = 0x700000 + (cn * (bpp << 3)) + (m_scbr << 10) + ((y & 0x07) * 2);
	x = (x & 7) ^ 7;

	for(n = 0; n < bpp; n++)
	{
		UINT32 byte = ((n >> 1) << 4) + (n & 1);  // = [n]{ 0, 1, 16, 17, 32, 33, 48, 49 };
		superfx_add_clocks_internal(m_memory_access_speed);
		data |= ((superfx_bus_read(addr + byte) >> x) & 1) << n;
	}

	return data;
}

UINT8 superfx_device::superfx_color(UINT8 source)
{
	if(m_por & SUPERFX_POR_HIGHNIBBLE)
	{
		return (m_colr & 0xf0) | (source >> 4);
	}
	if(m_por & SUPERFX_POR_FREEZEHIGH)
	{
		return (m_colr & 0xf0) | (source & 0x0f);
	}
	return source;
}

void superfx_device::superfx_rambuffer_sync()
{
	if(m_ramcl)
	{
		superfx_add_clocks_internal(m_ramcl);
	}
}

UINT8 superfx_device::superfx_rambuffer_read(UINT16 addr)
{
	superfx_rambuffer_sync();
	return superfx_bus_read(0x700000 + (m_rambr << 16) + addr);
}

void superfx_device::superfx_rambuffer_write(UINT16 addr, UINT8 data)
{
	superfx_rambuffer_sync();
	m_ramcl = m_memory_access_speed;
	m_ramar = addr;
	m_ramdr = data;
}

void superfx_device::superfx_rombuffer_sync()
{
	if(m_romcl)
	{
		superfx_add_clocks_internal(m_romcl);
	}
}

void superfx_device::superfx_rombuffer_update()
{
	m_sfr |= SUPERFX_SFR_R;
	m_romcl = m_memory_access_speed;
}

UINT8 superfx_device::superfx_rombuffer_read()
{
	superfx_rombuffer_sync();
	return m_romdr;
}

void superfx_device::superfx_gpr_write(UINT8 r, UINT16 data)
{
	m_r[r] = data;
	if(r == 14)
	{
		superfx_rombuffer_update();
	}
	else if(r == 15)
	{
		m_r15_modified = 1;
	}
}

UINT8 superfx_device::superfx_op_read(UINT16 addr)
{
	UINT16 offset = addr - m_cbr;
	if(offset < 512)
	{
		if(!m_cache.valid[offset >> 4])
		{
			UINT32 dp = offset & 0xfff0;
			UINT32 sp = (m_pbr << 16) + ((m_cbr + dp) & 0xfff0);
			UINT32 n = 0;
			for(n = 0; n < 16; n++)
			{
				superfx_add_clocks_internal(m_memory_access_speed);
				m_cache.buffer[dp++] = superfx_bus_read(sp++);
			}
			m_cache.valid[offset >> 4] = 1;
		}
		else
		{
			superfx_add_clocks_internal(m_memory_access_speed);
		}
		return m_cache.buffer[offset];
	}

	if(m_pbr <= 0x5f)
	{
		//$[00-5f]:[0000-ffff] ROM
		superfx_rombuffer_sync();
		superfx_add_clocks_internal(m_memory_access_speed);
		return superfx_bus_read((m_pbr << 16) + addr);
	}
	else
	{
		//$[60-7f]:[0000-ffff] RAM
		superfx_rambuffer_sync();
		superfx_add_clocks_internal(m_memory_access_speed);
		return superfx_bus_read((m_pbr << 16) + addr);
	}
}

UINT8 superfx_device::superfx_peekpipe()
{
	UINT8 result = m_pipeline;
	m_pipeline = superfx_op_read(m_r[15]);
	m_r15_modified = 0;
	return result;
}

UINT8 superfx_device::superfx_pipe()
{
	UINT8 result = m_pipeline;
	m_pipeline = superfx_op_read(++m_r[15]);
	m_r15_modified = 0;
	return result;
}

/*****************************************************************************/

/* reads to SuperFX RAM only happen if this returns 1 */
int superfx_device::access_ram()
{
	if ((m_sfr & SUPERFX_SFR_G) && (m_scmr & SUPERFX_SCMR_RAN))
		return 0;

	return 1;
}

/* reads to SuperFX ROM only happen if this returns 1 */
int superfx_device::access_rom()
{
	if ((m_sfr & SUPERFX_SFR_G) && (m_scmr & SUPERFX_SCMR_RON))
		return 0;

	return 1;
}

UINT8 superfx_device::mmio_read(UINT32 addr)
{
	addr &= 0xffff;

	if(addr >= 0x3100 && addr <= 0x32ff)
	{
		return superfx_cache_mmio_read(addr - 0x3100);
	}

	if(addr >= 0x3000 && addr <= 0x301f)
	{
		return m_r[(addr >> 1) & 0xf] >> ((addr & 1) << 3);
	}

	switch(addr)
	{
		case 0x3030:
			return m_sfr >> 0;

		case 0x3031:
		{
			UINT8 r = m_sfr >> 8;
			m_sfr &= ~SUPERFX_SFR_IRQ;
			m_irq = 0;
			m_out_irq_func(m_irq);
			return r;
		}

		case 0x3034:
			return m_pbr;

		case 0x3036:
			return m_rombr;

		case 0x303b:
			return m_vcr;

		case 0x303c:
			return m_rambr;

		case 0x303e:
			return m_cbr >> 0;

		case 0x303f:
			return m_cbr >> 8;
	}

	return 0;
}

void superfx_device::mmio_write(UINT32 addr, UINT8 data)
{
	addr &= 0xffff;

	//printf( "superfx_mmio_write: %08x = %02x\n", addr, data );

	if(addr >= 0x3100 && addr <= 0x32ff)
	{
		superfx_cache_mmio_write(addr - 0x3100, data);
		return;
	}

	if(addr >= 0x3000 && addr <= 0x301f)
	{
		UINT32 n = (addr >> 1) & 0xf;
		if((addr & 1) == 0)
		{
			m_r[n] = (m_r[n] & 0xff00) | data;
		}
		else
		{
			m_r[n] = (data << 8) | (m_r[n] & 0xff);
		}

		if(addr == 0x301f)
		{
			m_sfr |= SUPERFX_SFR_G;
		}
		return;
	}

	switch(addr)
	{
		case 0x3030:
		{
			UINT8 g = (m_sfr & SUPERFX_SFR_G) ? 1 : 0;
			m_sfr = (m_sfr & 0xff00) | (data << 0);
			if(g == 1 && !(m_sfr & SUPERFX_SFR_G))
			{
				m_cbr = 0x0000;
				superfx_cache_flush();
			}
			break;
		}

		case 0x3031:
			m_sfr = (data << 8) | (m_sfr & 0x00ff);
			break;

		case 0x3033:
			m_bramr = data & 1;
			break;

		case 0x3034:
			m_pbr = data & 0x7f;
			superfx_cache_flush();
			break;

		case 0x3037:
			m_cfgr = data;
			superfx_update_speed();
			break;

		case 0x3038:
			m_scbr = data;
			break;

		case 0x3039:
			m_clsr = data & 1;
			superfx_update_speed();
			break;

		case 0x303a:
			m_scmr = data;
			break;
	}
}

void superfx_device::superfx_add_clocks_internal(UINT32 clocks)
{
	if(m_romcl)
	{
		m_romcl -= MIN(clocks, m_romcl);
		if(m_romcl == 0)
		{
			m_sfr &= ~SUPERFX_SFR_R;
			m_romdr = superfx_bus_read((m_rombr << 16) + m_r[14]);
		}
	}

	if(m_ramcl)
	{
		m_ramcl -= MIN(clocks, m_ramcl);
		if(m_ramcl == 0)
		{
			superfx_bus_write(0x700000 + (m_rambr << 16) + m_ramar, m_ramdr);
		}
	}
}

void superfx_device::superfx_timing_reset()
{
	superfx_update_speed();
	m_r15_modified = 0;

	m_romcl = 0;
	m_romdr = 0;

	m_ramcl = 0;
	m_ramar = 0;
	m_ramdr = 0;
}

void superfx_device::add_clocks(INT32 clocks)
{
	superfx_add_clocks_internal(clocks);
}

/*****************************************************************************/

void superfx_device::device_start()
{
	for(auto & elem : m_r)
	{
		elem = 0;
	}

	m_sfr = 0;
	m_pbr = 0;
	m_rombr = 0;
	m_rambr = 0;
	m_cbr = 0;
	m_scbr = 0;
	m_scmr = 0;
	m_colr = 0;
	m_por = 0;
	m_bramr = 0;
	m_vcr = 0x04;
	m_cfgr = 0;
	m_clsr = 0;
	m_pipeline = 0x01; // nop
	m_ramaddr = 0;
	m_r15_modified = 0;
	m_irq = 0;
	m_cache_access_speed = 0;
	m_memory_access_speed = 0;
	m_debugger_temp = 0;
	m_romcl = 0;
	m_romdr = 0;
	m_ramcl = 0;
	m_ramar = 0;
	m_ramdr = 0;

	superfx_regs_reset();
	superfx_memory_reset();
	superfx_update_speed();

	m_program = &space(AS_PROGRAM);

	m_out_irq_func.resolve();

	save_item(NAME(m_pipeline));
	save_item(NAME(m_ramaddr));

	save_item(NAME(m_r));
	save_item(NAME(m_sfr));
	save_item(NAME(m_pbr));
	save_item(NAME(m_rombr));
	save_item(NAME(m_rambr));
	save_item(NAME(m_cbr));
	save_item(NAME(m_scbr));
	save_item(NAME(m_scmr));
	save_item(NAME(m_colr));
	save_item(NAME(m_por));
	save_item(NAME(m_bramr));
	save_item(NAME(m_vcr));
	save_item(NAME(m_cfgr));
	save_item(NAME(m_clsr));

	save_item(NAME(m_romcl));
	save_item(NAME(m_romdr));

	save_item(NAME(m_ramcl));
	save_item(NAME(m_ramar));
	save_item(NAME(m_ramdr));

	save_item(NAME(m_sreg_idx));
	save_item(NAME(m_dreg_idx));
	save_item(NAME(m_r15_modified));

	save_item(NAME(m_irq));

	save_item(NAME(m_cache_access_speed));
	save_item(NAME(m_memory_access_speed));

	save_item(NAME(m_cache.buffer));
	save_item(NAME(m_cache.valid));

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_pixelcache[i].offset), i);
		save_item(NAME(m_pixelcache[i].bitpend), i);
		save_item(NAME(m_pixelcache[i].data), i);
	}

	state_add( SUPERFX_PC,      "PC",      m_debugger_temp).callimport().callexport().formatstr("%06X");
	state_add( SUPERFX_DREG,    "DREG",    m_dreg_idx).mask(0xf).formatstr("%02u");
	state_add( SUPERFX_SREG,    "SREG",    m_sreg_idx).mask(0xf).formatstr("%02u");
	state_add( SUPERFX_R0,      "R0",      m_r[0]).formatstr("%04X");
	state_add( SUPERFX_R1,      "R1",      m_r[1]).formatstr("%04X");
	state_add( SUPERFX_R2,      "R2",      m_r[2]).formatstr("%04X");
	state_add( SUPERFX_R3,      "R3",      m_r[3]).formatstr("%04X");
	state_add( SUPERFX_R4,      "R4",      m_r[4]).formatstr("%04X");
	state_add( SUPERFX_R5,      "R5",      m_r[5]).formatstr("%04X");
	state_add( SUPERFX_R6,      "R6",      m_r[6]).formatstr("%04X");
	state_add( SUPERFX_R7,      "R7",      m_r[7]).formatstr("%04X");
	state_add( SUPERFX_R8,      "R8",      m_r[8]).formatstr("%04X");
	state_add( SUPERFX_R9,      "R9",      m_r[9]).formatstr("%04X");
	state_add( SUPERFX_R10,     "R10",     m_r[10]).formatstr("%04X");
	state_add( SUPERFX_R11,     "R11",     m_r[11]).formatstr("%04X");
	state_add( SUPERFX_R12,     "R12",     m_r[12]).formatstr("%04X");
	state_add( SUPERFX_R13,     "R13",     m_r[13]).formatstr("%04X");
	state_add( SUPERFX_R14,     "R14",     m_r[14]).formatstr("%04X");
	state_add( SUPERFX_R15,     "R15",     m_r[15]).formatstr("%04X");
	state_add( SUPERFX_SFR,     "SFR",     m_sfr).formatstr("%04X");
	state_add( SUPERFX_PBR,     "PBR",     m_pbr).formatstr("%02X");
	state_add( SUPERFX_ROMBR,   "ROMBR",   m_rombr).formatstr("%02X");
	state_add( SUPERFX_RAMBR,   "RAMBR",   m_rambr).formatstr("%02X");
	state_add( SUPERFX_CBR,     "CBR",     m_cbr).formatstr("%04X");
	state_add( SUPERFX_SCBR,    "SCBR",    m_scbr).formatstr("%02X");
	state_add( SUPERFX_SCMR,    "SCMR",    m_scmr).formatstr("%02X");
	state_add( SUPERFX_COLR,    "COLR",    m_colr).formatstr("%02X");
	state_add( SUPERFX_POR,     "POR",     m_por).formatstr("%02X");
	state_add( SUPERFX_BRAMR,   "BRAMR",   m_bramr).formatstr("%02X");
	state_add( SUPERFX_VCR,     "VCR",     m_vcr).formatstr("%02X");
	state_add( SUPERFX_CFGR,    "CFGR",    m_cfgr).formatstr("%02X");
	state_add( SUPERFX_CLSR,    "CLSR",    m_clsr).formatstr("%02X");
	state_add( SUPERFX_ROMCL,   "ROMCL",   m_romcl).formatstr("%08X");
	state_add( SUPERFX_ROMDR,   "ROMDR",   m_romdr).formatstr("%02X");
	state_add( SUPERFX_RAMCL,   "RAMCL",   m_ramcl).formatstr("%08X");
	state_add( SUPERFX_RAMAR,   "RAMAR",   m_ramar).formatstr("%04X");
	state_add( SUPERFX_RAMDR,   "RAMDR",   m_ramdr).formatstr("%02X");
	state_add( SUPERFX_RAMADDR, "RAMADDR", m_ramaddr).formatstr("%04X");

	m_icountptr = &m_icount;
}


void superfx_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SUPERFX_PC:
			m_r[15] = m_debugger_temp;
			break;
	}
}


void superfx_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case SUPERFX_PC:
			m_debugger_temp = ( (m_pbr << 16) | m_r[15] ) - 1;
			break;
	}
}


void superfx_device::device_reset()
{
	int i;

	for(i = 0; i < 16; i++)
	{
		m_r[i] = 0;
	}

	m_sfr = 0;
	m_pbr = 0;
	m_rombr = 0;
	m_rambr = 0;
	m_cbr = 0;
	m_scbr = 0;
	m_scmr = 0;
	m_colr = 0;
	m_por = 0;
	m_bramr = 0;
	m_vcr = 0x04;
	m_cfgr = 0;
	m_clsr = 0;
	m_pipeline = 0x01; // nop
	m_ramaddr = 0;

	superfx_regs_reset();
	superfx_timing_reset();
}

void superfx_device::superfx_dreg_sfr_sz_update()
{
	m_sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
	m_sfr |= (*(m_dreg) & 0x8000) ? SUPERFX_SFR_S : 0;
	m_sfr |= (*(m_dreg) == 0) ? SUPERFX_SFR_Z : 0;
}

void superfx_device::execute_run()
{
	UINT8 op;

	if(!(m_sfr & SUPERFX_SFR_G))
	{
		superfx_add_clocks_internal(6);
		m_icount = MIN(m_icount, 0);
	}

	while (m_icount > 0 && (m_sfr & SUPERFX_SFR_G))
	{
		if(!(m_sfr & SUPERFX_SFR_G))
		{
			superfx_add_clocks_internal(6);
			m_icount = MIN(m_icount, 0);
			break;
		}

		debugger_instruction_hook(this, (m_pbr << 16) | m_r[15]);

		op = superfx_peekpipe();

		switch(op)
		{
			case 0x00: // STOP
				if((m_cfgr & SUPERFX_CFGR_IRQ) == 0)
				{
					m_sfr |= SUPERFX_SFR_IRQ;
					m_irq = 1;
					m_out_irq_func(m_irq ? ASSERT_LINE : CLEAR_LINE );
				}
				m_sfr &= ~SUPERFX_SFR_G;
				m_pipeline = 0x01;
				superfx_regs_reset();
				break;
			case 0x01: // NOP
				superfx_regs_reset();
				break;
			case 0x02: // CACHE
				if(m_cbr != (m_r[15] & 0xfff0))
				{
					m_cbr = m_r[15] & 0xfff0;
					superfx_cache_flush();
				}
				superfx_regs_reset();
				break;
			case 0x03: // LSR
				m_sfr &= ~SUPERFX_SFR_CY;
				m_sfr |= (*(m_sreg) & 1) ? SUPERFX_SFR_CY : 0;
				superfx_gpr_write(m_dreg_idx, *(m_sreg) >> 1);
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;
			case 0x04: // ROL
			{
				UINT16 carry = *(m_sreg) & 0x8000;
				superfx_gpr_write(m_dreg_idx, (*(m_sreg) << 1) | SUPERFX_SFR_CY_SET);
				m_sfr &= ~SUPERFX_SFR_CY;
				m_sfr |= carry ? SUPERFX_SFR_CY : 0;
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;
			}
			case 0x05: // BRA
			{
				INT32 e = (INT8)superfx_pipe();
				superfx_gpr_write(15, m_r[15] + e);
				break;
			}
			case 0x06: // BLT
			{
				INT32 e = (INT8)superfx_pipe();
				if((SUPERFX_SFR_S_SET ^ SUPERFX_SFR_OV_SET) == 0)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x07: // BGE
			{
				INT32 e = (INT8)superfx_pipe();
				if((SUPERFX_SFR_S_SET ^ SUPERFX_SFR_OV_SET) == 1)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x08: // BNE
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_Z_SET == 0)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x09: // BEQ
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_Z_SET == 1)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x0a: // BPL
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_S_SET == 0)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x0b: // BMI
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_S_SET == 1)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x0c: // BCC
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_CY_SET == 0)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x0d: // BCS
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_CY_SET == 1)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x0e: // BVC
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_OV_SET == 0)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}
			case 0x0f: // BVS
			{
				INT32 e = (INT8)superfx_pipe();
				if(SUPERFX_SFR_OV_SET == 1)
				{
					superfx_gpr_write(15, m_r[15] + e);
				}
				break;
			}

			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: // TO
				if((m_sfr & SUPERFX_SFR_B) == 0)
				{
					m_dreg = &m_r[op & 0xf];
					m_dreg_idx = op & 0xf;
				}
				else
				{
					superfx_gpr_write(op & 0xf, *(m_sreg));
					superfx_regs_reset();
				}
				break;

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // WITH
				m_sreg = &m_r[op & 0xf];
				m_sreg_idx = op & 0xf;
				m_dreg = &m_r[op & 0xf];
				m_dreg_idx = op & 0xf;
				m_sfr |= SUPERFX_SFR_B;
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35:
			case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b:   // STW_IR / STB_IR
				if((m_sfr & SUPERFX_SFR_ALT1) == 0)
				{ // STW_IR
					m_ramaddr = m_r[op & 0xf];
					superfx_rambuffer_write(m_ramaddr ^ 0, (*(m_sreg)) >> 0);
					superfx_rambuffer_write(m_ramaddr ^ 1, (*(m_sreg)) >> 8);
					superfx_regs_reset();
				}
				else
				{ // STB_IR
					m_ramaddr = m_r[op & 0xf];
					superfx_rambuffer_write(m_ramaddr, *(m_sreg));
					superfx_regs_reset();
				}
				break;

			case 0x3c: // LOOP
				superfx_gpr_write(12, m_r[12] - 1);
				m_sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				m_sfr |= (m_r[12] & 0x8000) ? SUPERFX_SFR_S : 0;
				m_sfr |= (m_r[12] == 0) ? SUPERFX_SFR_Z : 0;
				if(!(m_sfr & SUPERFX_SFR_Z))
				{
					superfx_gpr_write(15, m_r[13]);
				}
				superfx_regs_reset();
				break;
			case 0x3d: // ALT1
				m_sfr &= ~SUPERFX_SFR_B;
				m_sfr |= SUPERFX_SFR_ALT1;
				break;
			case 0x3e: // ALT2
				m_sfr &= ~SUPERFX_SFR_B;
				m_sfr |= SUPERFX_SFR_ALT2;
				break;
			case 0x3f: // ALT3
				m_sfr &= ~SUPERFX_SFR_B;
				m_sfr |= SUPERFX_SFR_ALT1;
				m_sfr |= SUPERFX_SFR_ALT2;
				break;

			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45:
			case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b:   // LDW_IR / LDB_IR
				if((m_sfr & SUPERFX_SFR_ALT1) == 0)
				{ // LDW_IR
					UINT16 data;
					m_ramaddr = m_r[op & 0xf];
					data  = superfx_rambuffer_read(m_ramaddr ^ 0) << 0;
					data |= superfx_rambuffer_read(m_ramaddr ^ 1) << 8;
					superfx_gpr_write(m_dreg_idx, data);
					superfx_regs_reset();
				}
				else
				{ // LDB_IR
					m_ramaddr = m_r[op & 0xf];
					superfx_gpr_write(m_dreg_idx, superfx_rambuffer_read(m_ramaddr));
					superfx_regs_reset();
				}
				break;

			case 0x4c: // PLOT / RPIX
				if((m_sfr & SUPERFX_SFR_ALT1) == 0)
				{ // PLOT
					superfx_plot(m_r[1], m_r[2]);
					superfx_gpr_write(1, m_r[1] + 1);
					superfx_regs_reset();
				}
				else
				{ // RPIX
					superfx_gpr_write(m_dreg_idx, superfx_rpix(m_r[1], m_r[2]));
					superfx_dreg_sfr_sz_update();
					superfx_regs_reset();
				}
				break;

			case 0x4d: // SWAP
				superfx_gpr_write(m_dreg_idx, (*(m_sreg) >> 8) | (*(m_sreg) << 8));
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;

			case 0x4e: // COLOR / CMODE
				if((m_sfr & SUPERFX_SFR_ALT1) == 0)
				{ // COLOR
					m_colr = superfx_color(*(m_sreg));
					superfx_regs_reset();
				}
				else
				{ // CMODE
					m_por = *(m_sreg);
					superfx_regs_reset();
				}
				break;

			case 0x4f: // NOT
				superfx_gpr_write(m_dreg_idx, ~(*(m_sreg)));
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;

			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: // ADD / ADC / ADDI / ADCI
			{
				INT32 r = *(m_sreg);
				m_sfr &= ~(SUPERFX_SFR_OV | SUPERFX_SFR_S | SUPERFX_SFR_Z);
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // ADD
						r += m_r[op & 0xf];
						m_sfr |= (~(*(m_sreg) ^ m_r[op & 0xf]) & (m_r[op & 0xf] ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
					case SUPERFX_SFR_ALT1: // ADC
						r += m_r[op & 0xf] + SUPERFX_SFR_CY_SET;
						m_sfr |= (~(*(m_sreg) ^ m_r[op & 0xf]) & (m_r[op & 0xf] ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
					case SUPERFX_SFR_ALT2: // ADDI
						r += op & 0xf;
						m_sfr |= (~(*(m_sreg) ^ (op & 0xf)) & ((op & 0xf) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
					case SUPERFX_SFR_ALT3: // ADCI
						r += (op & 0xf) + SUPERFX_SFR_CY_SET;
						m_sfr |= (~(*(m_sreg) ^ (op & 0xf)) & ((op & 0xf) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
				}
				m_sfr &= ~SUPERFX_SFR_CY;
				m_sfr |= (r & 0x8000) ? SUPERFX_SFR_S : 0;
				m_sfr |= (r >= 0x10000) ? SUPERFX_SFR_CY : 0;
				m_sfr |= ((UINT16)r == 0) ? SUPERFX_SFR_Z : 0;
				superfx_gpr_write(m_dreg_idx, r);
				superfx_regs_reset();
				break;
			}

			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: // SUB / SBC / SUBI / CMP
			{
				INT32 r = 0;
				m_sfr &= ~(SUPERFX_SFR_OV | SUPERFX_SFR_S | SUPERFX_SFR_Z);
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // SUB
						r = *(m_sreg) - m_r[op & 0xf];
						m_sfr |= ((*(m_sreg) ^ m_r[op & 0xf]) & (*(m_sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						superfx_gpr_write(m_dreg_idx, r);
						break;
					case SUPERFX_SFR_ALT1: // SBC
						r = *(m_sreg) - m_r[op & 0xf] - SUPERFX_SFR_CY_CLEAR;
						m_sfr |= ((*(m_sreg) ^ m_r[op & 0xf]) & (*(m_sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						superfx_gpr_write(m_dreg_idx, r);
						break;
					case SUPERFX_SFR_ALT2: // SUBI
						r = *(m_sreg) - (op & 0xf);
						m_sfr |= ((*(m_sreg) ^ (op & 0xf)) & (*(m_sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						superfx_gpr_write(m_dreg_idx, r);
						break;
					case SUPERFX_SFR_ALT3: // CMP
						r = *(m_sreg) - m_r[op & 0xf];
						m_sfr |= ((*(m_sreg) ^ m_r[op & 0xf]) & (*(m_sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
				}
				m_sfr &= ~SUPERFX_SFR_CY;
				m_sfr |= (r & 0x8000) ? SUPERFX_SFR_S : 0;
				m_sfr |= (r >= 0x0) ? SUPERFX_SFR_CY : 0;
				m_sfr |= ((UINT16)r == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset();
				break;
			}

			case 0x70: // MERGE
				superfx_gpr_write(m_dreg_idx, (m_r[7] & 0xff00) | (m_r[8] >> 8));
				m_sfr &= ~(SUPERFX_SFR_OV | SUPERFX_SFR_S | SUPERFX_SFR_CY | SUPERFX_SFR_Z);
				m_sfr |= (*(m_dreg) & 0xc0c0) ? SUPERFX_SFR_OV : 0;
				m_sfr |= (*(m_dreg) & 0x8080) ? SUPERFX_SFR_S : 0;
				m_sfr |= (*(m_dreg) & 0xe0e0) ? SUPERFX_SFR_CY : 0;
				m_sfr |= (*(m_dreg) & 0xf0f0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset();
				break;

			case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // AND / BIC / ANDI / BICI
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // AND
						superfx_gpr_write(m_dreg_idx, *(m_sreg) & m_r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT1: // BIC
						superfx_gpr_write(m_dreg_idx, *(m_sreg) & ~m_r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT2: // ANDI
						superfx_gpr_write(m_dreg_idx, *(m_sreg) & (op & 0xf));
						break;
					case SUPERFX_SFR_ALT3: // BICI
						superfx_gpr_write(m_dreg_idx, *(m_sreg) & ~(op & 0xf));
						break;
				}
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;

			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f: // MULT / UMULT / MULTI / UMULTI
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // MULT
						superfx_gpr_write(m_dreg_idx, (INT8)(*(m_sreg)) * (INT8)(m_r[op & 0xf]));
						break;
					case SUPERFX_SFR_ALT1: // UMULT
						superfx_gpr_write(m_dreg_idx, (UINT8)(*(m_sreg)) * (UINT8)(m_r[op & 0xf]));
						break;
					case SUPERFX_SFR_ALT2: // MULTI
						superfx_gpr_write(m_dreg_idx, (INT8)(*(m_sreg)) * (INT8)(op & 0xf));
						break;
					case SUPERFX_SFR_ALT3: // UMULTI
						superfx_gpr_write(m_dreg_idx, (UINT8)(*(m_sreg)) * (UINT8)(op & 0xf));
						break;
				}
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				if(!(m_cfgr & SUPERFX_CFGR_MS0))
				{
					superfx_add_clocks_internal(2);
				}
				break;

			case 0x90: // SBK
				superfx_rambuffer_write(m_ramaddr ^ 0, *(m_sreg) >> 0);
				superfx_rambuffer_write(m_ramaddr ^ 1, *(m_sreg) >> 8);
				superfx_regs_reset();
				break;

			case 0x91: case 0x92: case 0x93: case 0x94: // LINK
				superfx_gpr_write(11, m_r[15] + (op & 0xf));
				superfx_regs_reset();
				break;

			case 0x95: // SEX
				superfx_gpr_write(m_dreg_idx, (INT8)(*(m_sreg)));
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;

			case 0x96: // ASR / DIV2
				if((m_sfr & SUPERFX_SFR_ALT1) == 0)
				{ // ASR
					m_sfr &= ~SUPERFX_SFR_CY;
					m_sfr |= (*(m_sreg) & 1) ? SUPERFX_SFR_CY : 0;
					superfx_gpr_write(m_dreg_idx, (INT16)(*(m_sreg)) >> 1);
					superfx_dreg_sfr_sz_update();
					superfx_regs_reset();
				}
				else
				{ // DIV2
					m_sfr &= ~SUPERFX_SFR_CY;
					m_sfr |= (*(m_sreg) & 1) ? SUPERFX_SFR_CY : 0;
					superfx_gpr_write(m_dreg_idx, ((INT16)(*(m_sreg)) >> 1) + ((UINT32)(*(m_sreg) + 1) >> 16));
					superfx_dreg_sfr_sz_update();
					superfx_regs_reset();
				}
				break;

			case 0x97: // ROR
			{
				UINT16 carry = *(m_sreg) & 1;
				superfx_gpr_write(m_dreg_idx, (SUPERFX_SFR_CY_SET << 15) | ((UINT16)(*(m_sreg)) >> 1));
				m_sfr &= ~SUPERFX_SFR_CY;
				m_sfr |= carry ? SUPERFX_SFR_CY : 0;
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;
			}

			case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: // JMP / LJMP
				if((m_sfr & SUPERFX_SFR_ALT1) == 0)
				{ // JMP
					superfx_gpr_write(15, m_r[op & 0xf]);
					superfx_regs_reset();
				}
				else
				{ // LJMP
					m_pbr = m_r[op & 0xf] & 0x7f;
					superfx_gpr_write(15, *(m_sreg));
					m_cbr = m_r[15] & 0xfff0;
					superfx_cache_flush();
					superfx_regs_reset();
				}
				break;

			case 0x9e: // LOB
				superfx_gpr_write(m_dreg_idx, (UINT16)(*(m_sreg)) & 0x00ff);
				m_sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				m_sfr |= (*(m_dreg) & 0x80) ? SUPERFX_SFR_S : 0;
				m_sfr |= (*(m_dreg) == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset();
				break;

			case 0x9f: // FMULT / LMULT
			{
				UINT32 result = (INT16)(*(m_sreg)) * (INT16)(m_r[6]);
				if(m_sfr & SUPERFX_SFR_ALT1)
				{ // LMULT
					superfx_gpr_write(4, result);
				}
				superfx_gpr_write(m_dreg_idx, result >> 16);
				m_sfr &= ~SUPERFX_SFR_CY;
				m_sfr |= (result & 0x8000) ? SUPERFX_SFR_CY : 0;
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				superfx_add_clocks_internal(4 + ((m_cfgr & SUPERFX_CFGR_MS0) ? 4 : 0));
				break;
			}

			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf: // IBT / LMS / SMS / LMS
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // IBT
						superfx_gpr_write(op & 0xf, (INT8)superfx_pipe());
						superfx_regs_reset();
						break;
					case SUPERFX_SFR_ALT2: // SMS
						m_ramaddr = superfx_pipe() << 1;
						superfx_rambuffer_write(m_ramaddr ^ 0, m_r[op & 0xf] >> 0);
						superfx_rambuffer_write(m_ramaddr ^ 1, m_r[op & 0xf] >> 8);
						superfx_regs_reset();
						break;
					case SUPERFX_SFR_ALT1: // LMS
					case SUPERFX_SFR_ALT3: // LMS
					{
						UINT16 data;
						m_ramaddr = superfx_pipe() << 1;
						data  = superfx_rambuffer_read(m_ramaddr ^ 0) << 0;
						data |= superfx_rambuffer_read(m_ramaddr ^ 1) << 8;
						superfx_gpr_write(op & 0xf, data);
						superfx_regs_reset();
						break;
					}
				}
				break;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf: // FROM
				if((m_sfr & SUPERFX_SFR_B) == 0)
				{
					m_sreg = &(m_r[op & 0xf]);
					m_sreg_idx = op & 0xf;
				}
				else
				{
					superfx_gpr_write(m_dreg_idx, m_r[op & 0xf]);
					m_sfr &= ~SUPERFX_SFR_OV;
					m_sfr |= (*(m_dreg) & 0x80) ? SUPERFX_SFR_OV : 0;
					superfx_dreg_sfr_sz_update();
					superfx_regs_reset();
				}
				break;

			case 0xc0: // HIB
				superfx_gpr_write(m_dreg_idx, (*(m_sreg)) >> 8);
				m_sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				m_sfr |= (*(m_dreg) & 0x80) ? SUPERFX_SFR_S : 0;
				m_sfr |= (*(m_dreg) == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset();
				break;

			case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: // OR / XOR / ORI / XORI
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // OR
						superfx_gpr_write(m_dreg_idx, *(m_sreg) | m_r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT1: // XOR
						superfx_gpr_write(m_dreg_idx, *(m_sreg) ^ m_r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT2: // ORI
						superfx_gpr_write(m_dreg_idx, *(m_sreg) | (op & 0xf));
						break;
					case SUPERFX_SFR_ALT3: // XORI
						superfx_gpr_write(m_dreg_idx, *(m_sreg) ^ (op & 0xf));
						break;
				}
				superfx_dreg_sfr_sz_update();
				superfx_regs_reset();
				break;

			case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde:            // INC
				superfx_gpr_write(op & 0xf, m_r[op & 0xf] + 1);
				m_sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				m_sfr |= (m_r[op & 0xf] & 0x8000) ? SUPERFX_SFR_S : 0;
				m_sfr |= (m_r[op & 0xf] == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset();
				break;

			case 0xdf: // GETC / RAMB / ROMB
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // GETC
					case SUPERFX_SFR_ALT1: // GETC
						m_colr = superfx_color(superfx_rombuffer_read());
						superfx_regs_reset();
						break;
					case SUPERFX_SFR_ALT2: // RAMB
						superfx_rambuffer_sync();
						m_rambr = ((*(m_sreg)) & 1) ? 1 : 0;
						superfx_regs_reset();
						break;
					case SUPERFX_SFR_ALT3: // ROMB
						superfx_rombuffer_sync();
						m_rombr = *(m_sreg) & 0x7f;
						superfx_regs_reset();
						break;
				}
				break;

			case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee:            // DEC
				superfx_gpr_write(op & 0xf, m_r[op & 0xf] - 1);
				m_sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				m_sfr |= (m_r[op & 0xf] & 0x8000) ? SUPERFX_SFR_S : 0;
				m_sfr |= (m_r[op & 0xf] == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset();
				break;

			case 0xef: // GETB / GETBH / GETBL / GETBS
			{
				UINT8 byte = superfx_rombuffer_read();
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // GETB
						superfx_gpr_write(m_dreg_idx, byte);
						break;
					case SUPERFX_SFR_ALT1: // GETBH
						superfx_gpr_write(m_dreg_idx, (byte << 8) | (*(m_sreg) & 0x00ff));
						break;
					case SUPERFX_SFR_ALT2: // GETBL
						superfx_gpr_write(m_dreg_idx, (*(m_sreg) & 0xff00) | (byte << 0));
						break;
					case SUPERFX_SFR_ALT3: // GETBS
						superfx_gpr_write(m_dreg_idx, (INT8)byte);
						break;
				}
				superfx_regs_reset();
				break;
			}

			case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
			case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: // IWT / LM / SM / LM
			{
				UINT16 data;
				switch(m_sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // IWT
						data  = superfx_pipe() << 0;
						data |= superfx_pipe() << 8;
						superfx_gpr_write(op & 0xf, data);
						superfx_regs_reset();
						break;
					case SUPERFX_SFR_ALT2: // SM
						m_ramaddr  = superfx_pipe() << 0;
						m_ramaddr |= superfx_pipe() << 8;
						superfx_rambuffer_write(m_ramaddr ^ 0, m_r[op & 0xf] >> 0);
						superfx_rambuffer_write(m_ramaddr ^ 1, m_r[op & 0xf] >> 8);
						superfx_regs_reset();
						break;
					case SUPERFX_SFR_ALT1:
					case SUPERFX_SFR_ALT3: // LM
						m_ramaddr  = superfx_pipe() << 0;
						m_ramaddr |= superfx_pipe() << 8;
						data  = superfx_rambuffer_read(m_ramaddr ^ 0) << 0;
						data |= superfx_rambuffer_read(m_ramaddr ^ 1) << 8;
						superfx_gpr_write(op & 0xf, data);
						superfx_regs_reset();
						break;
				}
				break;
			}
		}

		if(!m_r15_modified)
		{
			m_r[15]++;
		}

		//printf( " r0:%04x  r1:%04x  r2:%04x  r3:%04x  r4:%04x  r5:%04x  r6:%04x  r7:%04x\n",  m_r[0],  m_r[1],  m_r[2],  m_r[3],  m_r[4],  m_r[5],  m_r[6],  m_r[7] );
		//printf( " r8:%04x  r9:%04x r10:%04x r11:%04x r12:%04x r13:%04x r14:%04x r15:%04x\n",  m_r[8],  m_r[9], m_r[10], m_r[11], m_r[12], m_r[13], m_r[14], m_r[15] );
		//printf( "sfr:%04x\n", m_sfr );

		--m_icount;
	}
}


offs_t superfx_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
extern offs_t superfx_dasm_one(char *buffer, offs_t pc, UINT8 op, UINT8 param0, UINT8 param1, UINT16 alt);

	UINT8  op = *(UINT8 *)(opram + 0);
	UINT8  param0 = *(UINT8 *)(opram + 1);
	UINT8  param1 = *(UINT8 *)(opram + 2);
	UINT16 alt = m_sfr & SUPERFX_SFR_ALT;

	return superfx_dasm_one(buffer, pc, op, param0, param1, alt);
}
