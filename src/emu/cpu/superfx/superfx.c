#include "emu.h"
#include "debugger.h"
#include "superfx.h"

struct pixelcache_t
{
	UINT16 offset;
	UINT8 bitpend;
	UINT8 data[8];
};

struct cache_t
{
	UINT8 buffer[0x200];
	UINT8 valid[0x20];
};

struct superfx_state
{
	superfx_config config;

	devcb_resolved_write_line	out_irq_func;

	UINT8  pipeline;
	UINT16 ramaddr;	// RAM Address

	UINT16 r[16];	// GPRs
	UINT16 sfr;		// Status Flag Register
	UINT8  pbr;		// Program Bank Register
	UINT8  rombr;	// Game Pack ROM Bank Register
	UINT8  rambr;	// Game Pack RAM Bank Register
	UINT16 cbr;		// Cache Base Register
	UINT8  scbr;	// Screen Base Register
	UINT8  scmr;	// Screen Mode Register
	UINT8  colr;	// Color Register
	UINT8  por;		// Plot Option Register
	UINT8  bramr;	// Back-Up RAM Register
	UINT8  vcr;		// Version Code Register
	UINT8  cfgr;	// Config Register
	UINT8  clsr;	// Clock Select Register

	UINT32 romcl;	// Clock ticks until ROMDR is valid
	UINT8  romdr;	// ROM Buffer Data Register

	UINT32 ramcl;	// Clock ticks until RAMDR is valid;
	UINT16 ramar;	// RAM Buffer Address Register
	UINT8  ramdr;	// RAM Buffer Data Register

	UINT16 *sreg;	// Source Register (From)
	UINT8  sreg_idx;// Source Register (To), index
	UINT16 *dreg;	// Destination Register (To)
	UINT8  dreg_idx;// Destination Register (To), index
	UINT8  r15_modified;

	UINT8  irq;		// IRQ Pending

	UINT32 cache_access_speed;
	UINT32 memory_access_speed;

	cache_t cache;
	pixelcache_t pixelcache[2];

	legacy_cpu_device *device;
	address_space *program;
	int icount;
};

INLINE superfx_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SUPERFX);
	return (superfx_state *)downcast<legacy_cpu_device *>(device)->token();
}

/*****************************************************************************/

INLINE void superfx_regs_reset(superfx_state *cpustate);
static void superfx_update_speed(superfx_state *cpustate);
static void superfx_cache_flush(superfx_state *cpustate);
static UINT8 superfx_cache_mmio_read(superfx_state *cpustate, UINT32 addr);
static void superfx_cache_mmio_write(superfx_state *cpustate, UINT32 addr, UINT8 data);
static void superfx_memory_reset(superfx_state *cpustate);
INLINE UINT8 superfx_bus_read(superfx_state *cpustate, UINT32 addr);
INLINE void superfx_bus_write(superfx_state *cpustate, UINT32 addr, UINT8 data);
INLINE void superfx_pixelcache_flush(superfx_state *cpustate, INT32 line);
INLINE void superfx_plot(superfx_state *cpustate, UINT8 x, UINT8 y);
static UINT8 superfx_rpix(superfx_state *cpustate, UINT8 r1, UINT8 r2);
INLINE UINT8 superfx_color(superfx_state *cpustate, UINT8 source);

INLINE void superfx_rambuffer_sync(superfx_state *cpustate);
INLINE UINT8 superfx_rambuffer_read(superfx_state *cpustate, UINT16 addr);
INLINE void superfx_rambuffer_write(superfx_state *cpustate, UINT16 addr, UINT8 val);

INLINE void superfx_rombuffer_sync(superfx_state *cpustate);
INLINE void superfx_rombuffer_update(superfx_state *cpustate);
INLINE UINT8 superfx_rombuffer_read(superfx_state *cpustate);

INLINE void superfx_gpr_write(superfx_state *cpustate, UINT8 r, UINT16 data);
INLINE UINT8 superfx_op_read(superfx_state *cpustate, UINT16 addr);
INLINE UINT8 superfx_peekpipe(superfx_state *cpustate);
INLINE UINT8 superfx_pipe(superfx_state *cpustate);
INLINE void superfx_add_clocks_internal(superfx_state *cpustate, UINT32 clocks);
static void superfx_timing_reset(superfx_state *cpustate);

/*****************************************************************************/

#define SUPERFX_SFR_OV_SET		((cpustate->sfr & SUPERFX_SFR_OV) ? 1 : 0)
#define SUPERFX_SFR_OV_CLEAR	((cpustate->sfr & SUPERFX_SFR_OV) ? 0 : 1)
#define SUPERFX_SFR_S_SET		((cpustate->sfr & SUPERFX_SFR_S) ? 1 : 0)
#define SUPERFX_SFR_S_CLEAR		((cpustate->sfr & SUPERFX_SFR_S) ? 0 : 1)
#define SUPERFX_SFR_CY_SET		((cpustate->sfr & SUPERFX_SFR_CY) ? 1 : 0)
#define SUPERFX_SFR_CY_CLEAR	((cpustate->sfr & SUPERFX_SFR_CY) ? 0 : 1)
#define SUPERFX_SFR_Z_SET		((cpustate->sfr & SUPERFX_SFR_Z) ? 1 : 0)
#define SUPERFX_SFR_Z_CLEAR		((cpustate->sfr & SUPERFX_SFR_Z) ? 0 : 1)

INLINE void superfx_regs_reset(superfx_state *cpustate)
{
	cpustate->sfr &= ~(SUPERFX_SFR_B | SUPERFX_SFR_ALT3);

	cpustate->sreg = &cpustate->r[0];
	cpustate->dreg = &cpustate->r[0];
	cpustate->dreg_idx = 0;
	cpustate->sreg_idx = 0;
}

static void superfx_update_speed(superfx_state *cpustate)
{
	cpustate->cache_access_speed = (cpustate->clsr ? 1 : 2);
	cpustate->memory_access_speed = (cpustate->clsr ? 5 : 6);
	if(cpustate->clsr)
	{
		cpustate->cfgr &= ~SUPERFX_CFGR_MS0; // Cannot use high-speed multiplication in 21MHz mode
	}
}

static void superfx_cache_flush(superfx_state *cpustate)
{
	UINT32 n = 0;
	for(n = 0; n < 32; n++)
	{
		cpustate->cache.valid[n] = 0;
	}
}

static UINT8 superfx_cache_mmio_read(superfx_state *cpustate, UINT32 addr)
{
	addr = (addr + cpustate->cbr) & 0x1ff;
	return cpustate->cache.buffer[addr];
}

static void superfx_cache_mmio_write(superfx_state *cpustate, UINT32 addr, UINT8 data)
{
	addr = (addr + cpustate->cbr) & 0x1ff;
	cpustate->cache.buffer[addr] = data;
	if((addr & 15) == 15)
	{
		cpustate->cache.valid[addr >> 4] = 1;
	}
}

static void superfx_memory_reset(superfx_state *cpustate)
{
	UINT32 n = 0;
	for(n = 0; n < 0x200; n++)
	{
		cpustate->cache.buffer[n] = 0x00;
	}
	for(n = 0; n < 0x20; n++)
	{
		cpustate->cache.valid[n] = 0;
	}
	for(n = 0; n < 2; n++)
	{
		cpustate->pixelcache[n].offset = ~0;
		cpustate->pixelcache[n].bitpend = 0x00;
	}
}

INLINE UINT8 superfx_bus_read(superfx_state *cpustate, UINT32 addr)
{
	return cpustate->program->read_byte(addr);
}

INLINE void superfx_bus_write(superfx_state *cpustate, UINT32 addr, UINT8 data)
{
	cpustate->program->write_byte(addr, data);
}

INLINE void superfx_pixelcache_flush(superfx_state *cpustate, INT32 line)
{
	UINT8 x = cpustate->pixelcache[line].offset << 3;
	UINT8 y = cpustate->pixelcache[line].offset >> 5;
	UINT32 cn = 0;
	UINT32 bpp = 2 << ((cpustate->scmr & SUPERFX_SCMR_MD) - ((cpustate->scmr & SUPERFX_SCMR_MD) >> 1)); // = [regs.scmr.md]{ 2, 4, 4, 8 };
	UINT32 addr = 0;
	UINT32 n = 0;

	if(cpustate->pixelcache[line].bitpend == 0x00)
	{
		return;
	}

	switch(((cpustate->por & SUPERFX_POR_OBJ) ? SUPERFX_SCMR_HT3 : (cpustate->scmr & SUPERFX_SCMR_HT_MASK)))
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

	addr = 0x700000 + (cn * (bpp << 3)) + (cpustate->scbr << 10) + ((y & 0x07) * 2);

	for(n = 0; n < bpp; n++)
	{
		UINT32 byte = ((n >> 1) << 4) + (n & 1);  // = [n]{ 0, 1, 16, 17, 32, 33, 48, 49 };
		UINT8 data = 0x00;
		UINT32 x32 = 0;
		for(x32 = 0; x32 < 8; x32++)
		{
			data |= ((cpustate->pixelcache[line].data[x32] >> n) & 1) << x32;
		}
		if(cpustate->pixelcache[line].bitpend != 0xff)
		{
			superfx_add_clocks_internal(cpustate, cpustate->memory_access_speed);
			data &= cpustate->pixelcache[line].bitpend;
			data |= superfx_bus_read(cpustate, addr + byte) & ~cpustate->pixelcache[line].bitpend;
		}
		superfx_add_clocks_internal(cpustate, cpustate->memory_access_speed);
		superfx_bus_write(cpustate, addr + byte, data);
	}

	cpustate->pixelcache[line].bitpend = 0x00;
}

INLINE void superfx_plot(superfx_state *cpustate, UINT8 x, UINT8 y)
{
	UINT8 color = cpustate->colr;
	UINT16 offset = (y << 5) + (x >> 3);

	if((cpustate->por & SUPERFX_POR_DITHER) != 0 && (cpustate->scmr & SUPERFX_SCMR_MD) != 3)
	{
		if((x ^ y) & 1)
		{
			color >>= 4;
		}
		color &= 0x0f;
	}

	if((cpustate->por & SUPERFX_POR_TRANSPARENT) == 0)
	{
		if((cpustate->scmr & SUPERFX_SCMR_MD) == 3)
		{
			if(cpustate->por & SUPERFX_POR_FREEZEHIGH)
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

	if(offset != cpustate->pixelcache[0].offset)
	{
		superfx_pixelcache_flush(cpustate, 1);
		cpustate->pixelcache[1] = cpustate->pixelcache[0];
		cpustate->pixelcache[0].bitpend = 0x00;
		cpustate->pixelcache[0].offset = offset;
	}

	x = (x & 7) ^ 7;
	cpustate->pixelcache[0].data[x] = color;
	cpustate->pixelcache[0].bitpend |= 1 << x;
	if(cpustate->pixelcache[0].bitpend == 0xff)
	{
		superfx_pixelcache_flush(cpustate, 1);
		cpustate->pixelcache[1] = cpustate->pixelcache[0];
		cpustate->pixelcache[0].bitpend = 0x00;
	}
}

static UINT8 superfx_rpix(superfx_state *cpustate, UINT8 x, UINT8 y)
{
	UINT32 cn = 0;
	UINT32 bpp = 0;
	UINT32 addr = 0;
	UINT8 data = 0x00;
	UINT32 n = 0;

	superfx_pixelcache_flush(cpustate, 1);
	superfx_pixelcache_flush(cpustate, 0);

	bpp = 2 << ((cpustate->scmr & SUPERFX_SCMR_MD) - ((cpustate->scmr & SUPERFX_SCMR_MD) >> 1)); // = [regs.scmr.md]{ 2, 4, 4, 8 };

	switch((cpustate->por & SUPERFX_POR_OBJ) ? SUPERFX_SCMR_HT3 : (cpustate->scmr & SUPERFX_SCMR_HT_MASK))
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

	addr = 0x700000 + (cn * (bpp << 3)) + (cpustate->scbr << 10) + ((y & 0x07) * 2);
	x = (x & 7) ^ 7;

	for(n = 0; n < bpp; n++)
	{
		UINT32 byte = ((n >> 1) << 4) + (n & 1);  // = [n]{ 0, 1, 16, 17, 32, 33, 48, 49 };
		superfx_add_clocks_internal(cpustate, cpustate->memory_access_speed);
		data |= ((superfx_bus_read(cpustate, addr + byte) >> x) & 1) << n;
	}

	return data;
}

INLINE UINT8 superfx_color(superfx_state *cpustate, UINT8 source)
{
	if(cpustate->por & SUPERFX_POR_HIGHNIBBLE)
	{
		return (cpustate->colr & 0xf0) | (source >> 4);
	}
	if(cpustate->por & SUPERFX_POR_FREEZEHIGH)
	{
		return (cpustate->colr & 0xf0) | (source & 0x0f);
	}
	return source;
}

INLINE void superfx_rambuffer_sync(superfx_state *cpustate)
{
	if(cpustate->ramcl)
	{
		superfx_add_clocks_internal(cpustate, cpustate->ramcl);
	}
}

INLINE UINT8 superfx_rambuffer_read(superfx_state *cpustate, UINT16 addr)
{
	superfx_rambuffer_sync(cpustate);
	return superfx_bus_read(cpustate, 0x700000 + (cpustate->rambr << 16) + addr);
}

INLINE void superfx_rambuffer_write(superfx_state *cpustate, UINT16 addr, UINT8 data)
{
	superfx_rambuffer_sync(cpustate);
	cpustate->ramcl = cpustate->memory_access_speed;
	cpustate->ramar = addr;
	cpustate->ramdr = data;
}

INLINE void superfx_rombuffer_sync(superfx_state *cpustate)
{
	if(cpustate->romcl)
	{
		superfx_add_clocks_internal(cpustate, cpustate->romcl);
	}
}

INLINE void superfx_rombuffer_update(superfx_state *cpustate)
{
	cpustate->sfr |= SUPERFX_SFR_R;
	cpustate->romcl = cpustate->memory_access_speed;
}

INLINE UINT8 superfx_rombuffer_read(superfx_state *cpustate)
{
	superfx_rombuffer_sync(cpustate);
	return cpustate->romdr;
}

INLINE void superfx_gpr_write(superfx_state *cpustate, UINT8 r, UINT16 data)
{
	cpustate->r[r] = data;
	if(r == 14)
	{
		superfx_rombuffer_update(cpustate);
	}
	else if(r == 15)
	{
		cpustate->r15_modified = 1;
	}
}

INLINE UINT8 superfx_op_read(superfx_state *cpustate, UINT16 addr)
{
	UINT16 offset = addr - cpustate->cbr;
	if(offset < 512)
	{
		if(!cpustate->cache.valid[offset >> 4])
		{
			UINT32 dp = offset & 0xfff0;
			UINT32 sp = (cpustate->pbr << 16) + ((cpustate->cbr + dp) & 0xfff0);
			UINT32 n = 0;
			for(n = 0; n < 16; n++)
			{
				superfx_add_clocks_internal(cpustate, cpustate->memory_access_speed);
				cpustate->cache.buffer[dp++] = superfx_bus_read(cpustate, sp++);
			}
			cpustate->cache.valid[offset >> 4] = 1;
		}
		else
		{
			superfx_add_clocks_internal(cpustate, cpustate->memory_access_speed);
		}
		return cpustate->cache.buffer[offset];
	}

	if(cpustate->pbr <= 0x5f)
	{
		//$[00-5f]:[0000-ffff] ROM
		superfx_rombuffer_sync(cpustate);
		superfx_add_clocks_internal(cpustate, cpustate->memory_access_speed);
		return superfx_bus_read(cpustate, (cpustate->pbr << 16) + addr);
	}
	else
	{
		//$[60-7f]:[0000-ffff] RAM
		superfx_rambuffer_sync(cpustate);
		superfx_add_clocks_internal(cpustate, cpustate->memory_access_speed);
		return superfx_bus_read(cpustate, (cpustate->pbr << 16) + addr);
	}
}

INLINE UINT8 superfx_peekpipe(superfx_state *cpustate)
{
	UINT8 result = cpustate->pipeline;
	cpustate->pipeline = superfx_op_read(cpustate, cpustate->r[15]);
	cpustate->r15_modified = 0;
	return result;
}

INLINE UINT8 superfx_pipe(superfx_state *cpustate)
{
	UINT8 result = cpustate->pipeline;
	cpustate->pipeline = superfx_op_read(cpustate, ++cpustate->r[15]);
	cpustate->r15_modified = 0;
	return result;
}

/*****************************************************************************/

/* reads to SuperFX RAM only happen if this returns 1 */
int superfx_access_ram(device_t *cpu)
{
	superfx_state *cpustate = get_safe_token(cpu);

	if ((cpustate->sfr & SUPERFX_SFR_G) && (cpustate->scmr & SUPERFX_SCMR_RAN))
		return 0;

	return 1;
}

/* reads to SuperFX ROM only happen if this returns 1 */
int superfx_access_rom(device_t *cpu)
{
	superfx_state *cpustate = get_safe_token(cpu);

	if ((cpustate->sfr & SUPERFX_SFR_G) && (cpustate->scmr & SUPERFX_SCMR_RON))
		return 0;

	return 1;
}

UINT8 superfx_mmio_read(device_t *cpu, UINT32 addr)
{
	superfx_state *cpustate = get_safe_token(cpu);

	addr &= 0xffff;

	if(addr >= 0x3100 && addr <= 0x32ff)
	{
		return superfx_cache_mmio_read(cpustate, addr - 0x3100);
	}

	if(addr >= 0x3000 && addr <= 0x301f)
	{
		return cpustate->r[(addr >> 1) & 0xf] >> ((addr & 1) << 3);
	}

	switch(addr)
	{
		case 0x3030:
			return cpustate->sfr >> 0;

		case 0x3031:
		{
			UINT8 r = cpustate->sfr >> 8;
			cpustate->sfr &= ~SUPERFX_SFR_IRQ;
			cpustate->irq = 0;
			cpustate->out_irq_func(cpustate->irq);
			return r;
		}

		case 0x3034:
			return cpustate->pbr;

		case 0x3036:
			return cpustate->rombr;

		case 0x303b:
			return cpustate->vcr;

		case 0x303c:
			return cpustate->rambr;

		case 0x303e:
			return cpustate->cbr >> 0;

		case 0x303f:
			return cpustate->cbr >> 8;
	}

	return 0;
}

void superfx_mmio_write(device_t *cpu, UINT32 addr, UINT8 data)
{
	superfx_state *cpustate = get_safe_token(cpu);

	addr &= 0xffff;

	//printf( "superfx_mmio_write: %08x = %02x\n", addr, data );

	if(addr >= 0x3100 && addr <= 0x32ff)
	{
		superfx_cache_mmio_write(cpustate, addr - 0x3100, data);
		return;
	}

	if(addr >= 0x3000 && addr <= 0x301f)
	{
		UINT32 n = (addr >> 1) & 0xf;
		if((addr & 1) == 0)
		{
			cpustate->r[n] = (cpustate->r[n] & 0xff00) | data;
		}
		else
		{
			cpustate->r[n] = (data << 8) | (cpustate->r[n] & 0xff);
		}

		if(addr == 0x301f)
		{
			cpustate->sfr |= SUPERFX_SFR_G;
		}
		return;
	}

	switch(addr)
	{
		case 0x3030:
		{
			UINT8 g = (cpustate->sfr & SUPERFX_SFR_G) ? 1 : 0;
			cpustate->sfr = (cpustate->sfr & 0xff00) | (data << 0);
			if(g == 1 && !(cpustate->sfr & SUPERFX_SFR_G))
			{
				cpustate->cbr = 0x0000;
				superfx_cache_flush(cpustate);
			}
			break;
		}

		case 0x3031:
			cpustate->sfr = (data << 8) | (cpustate->sfr & 0x00ff);
			break;

		case 0x3033:
			cpustate->bramr = data & 1;
			break;

		case 0x3034:
			cpustate->pbr = data & 0x7f;
			superfx_cache_flush(cpustate);
			break;

		case 0x3037:
			cpustate->cfgr = data;
			superfx_update_speed(cpustate);
			break;

		case 0x3038:
			cpustate->scbr = data;
			break;

		case 0x3039:
			cpustate->clsr = data & 1;
			superfx_update_speed(cpustate);
			break;

		case 0x303a:
			cpustate->scmr = data;
			break;
	}
}

INLINE void superfx_add_clocks_internal(superfx_state *cpustate, UINT32 clocks)
{
	if(cpustate->romcl)
	{
		cpustate->romcl -= MIN(clocks, cpustate->romcl);
		if(cpustate->romcl == 0)
		{
			cpustate->sfr &= ~SUPERFX_SFR_R;
			cpustate->romdr = superfx_bus_read(cpustate, (cpustate->rombr << 16) + cpustate->r[14]);
		}
	}

	if(cpustate->ramcl)
	{
		cpustate->ramcl -= MIN(clocks, cpustate->ramcl);
		if(cpustate->ramcl == 0)
		{
			superfx_bus_write(cpustate, 0x700000 + (cpustate->rambr << 16) + cpustate->ramar, cpustate->ramdr);
		}
	}
}

static void superfx_timing_reset(superfx_state *cpustate)
{
	superfx_update_speed(cpustate);
	cpustate->r15_modified = 0;

	cpustate->romcl = 0;
	cpustate->romdr = 0;

	cpustate->ramcl = 0;
	cpustate->ramar = 0;
	cpustate->ramdr = 0;
}

void superfx_add_clocks(device_t *cpu, INT32 clocks)
{
	superfx_state *cpustate = get_safe_token(cpu);

	superfx_add_clocks_internal(cpustate, clocks);
}

/*****************************************************************************/

static void superfx_register_save( legacy_cpu_device *device )
{
	superfx_state *cpustate = get_safe_token(device);
	int i;

	device->save_item(NAME(cpustate->pipeline));
	device->save_item(NAME(cpustate->ramaddr));

	device->save_item(NAME(cpustate->r));
	device->save_item(NAME(cpustate->sfr));
	device->save_item(NAME(cpustate->pbr));
	device->save_item(NAME(cpustate->rombr));
	device->save_item(NAME(cpustate->rambr));
	device->save_item(NAME(cpustate->cbr));
	device->save_item(NAME(cpustate->scbr));
	device->save_item(NAME(cpustate->scmr));
	device->save_item(NAME(cpustate->colr));
	device->save_item(NAME(cpustate->por));
	device->save_item(NAME(cpustate->bramr));
	device->save_item(NAME(cpustate->vcr));
	device->save_item(NAME(cpustate->cfgr));
	device->save_item(NAME(cpustate->clsr));

	device->save_item(NAME(cpustate->romcl));
	device->save_item(NAME(cpustate->romdr));

	device->save_item(NAME(cpustate->ramcl));
	device->save_item(NAME(cpustate->ramar));
	device->save_item(NAME(cpustate->ramdr));

	device->save_item(NAME(cpustate->sreg_idx));
	device->save_item(NAME(cpustate->dreg_idx));
	device->save_item(NAME(cpustate->r15_modified));

	device->save_item(NAME(cpustate->irq));

	device->save_item(NAME(cpustate->cache_access_speed));
	device->save_item(NAME(cpustate->memory_access_speed));

	device->save_item(NAME(cpustate->cache.buffer));
	device->save_item(NAME(cpustate->cache.valid));

	for (i = 0; i < 2; i++)
	{
		device->save_item(NAME(cpustate->pixelcache[i].offset), i);
		device->save_item(NAME(cpustate->pixelcache[i].bitpend), i);
		device->save_item(NAME(cpustate->pixelcache[i].data), i);
	}

	device->save_item(NAME(cpustate->icount));
}

static CPU_INIT( superfx )
{
	int i;
	superfx_state *cpustate = get_safe_token(device);

	for(i = 0; i < 16; i++)
	{
		cpustate->r[i] = 0;
	}

	cpustate->sfr = 0;
	cpustate->pbr = 0;
	cpustate->rombr = 0;
	cpustate->rambr = 0;
	cpustate->cbr = 0;
	cpustate->scbr = 0;
	cpustate->scmr = 0;
	cpustate->colr = 0;
	cpustate->por = 0;
	cpustate->bramr = 0;
	cpustate->vcr = 0x04;
	cpustate->cfgr = 0;
	cpustate->clsr = 0;
	cpustate->pipeline = 0x01; // nop
	cpustate->ramaddr = 0;
	cpustate->r15_modified = 0;

	superfx_regs_reset(cpustate);
	superfx_memory_reset(cpustate);
	superfx_update_speed(cpustate);

	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);

	if (device->static_config() != NULL)
	{
		cpustate->config = *(superfx_config *)device->static_config();
	}

	cpustate->out_irq_func.resolve(cpustate->config.out_irq_func, *device);

	superfx_register_save(device);
}

static CPU_EXIT( superfx )
{
}

static CPU_RESET( superfx )
{
	int i;

	superfx_state *cpustate = get_safe_token(device);

	for(i = 0; i < 16; i++)
	{
		cpustate->r[i] = 0;
	}

	cpustate->sfr = 0;
	cpustate->pbr = 0;
	cpustate->rombr = 0;
	cpustate->rambr = 0;
	cpustate->cbr = 0;
	cpustate->scbr = 0;
	cpustate->scmr = 0;
	cpustate->colr = 0;
	cpustate->por = 0;
	cpustate->bramr = 0;
	cpustate->vcr = 0x04;
	cpustate->cfgr = 0;
	cpustate->clsr = 0;
	cpustate->pipeline = 0x01; // nop
	cpustate->ramaddr = 0;

	superfx_regs_reset(cpustate);
	superfx_timing_reset(cpustate);
}

INLINE void superfx_dreg_sfr_sz_update(superfx_state *cpustate)
{
	cpustate->sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
	cpustate->sfr |= (*(cpustate->dreg) & 0x8000) ? SUPERFX_SFR_S : 0;
	cpustate->sfr |= (*(cpustate->dreg) == 0) ? SUPERFX_SFR_Z : 0;
}

static CPU_EXECUTE( superfx )
{
	superfx_state *cpustate = get_safe_token(device);
	UINT8 op;

	if(!(cpustate->sfr & SUPERFX_SFR_G))
	{
		superfx_add_clocks_internal(cpustate, 6);
		cpustate->icount = MIN(cpustate->icount, 0);
	}

	while (cpustate->icount > 0 && (cpustate->sfr & SUPERFX_SFR_G))
	{
		if(!(cpustate->sfr & SUPERFX_SFR_G))
		{
			superfx_add_clocks_internal(cpustate, 6);
			cpustate->icount = MIN(cpustate->icount, 0);
			break;
		}

		debugger_instruction_hook(device, (cpustate->pbr << 16) | cpustate->r[15]);

		op = superfx_peekpipe(cpustate);

		switch(op)
		{
			case 0x00: // STOP
				if((cpustate->cfgr & SUPERFX_CFGR_IRQ) == 0)
				{
					cpustate->sfr |= SUPERFX_SFR_IRQ;
					cpustate->irq = 1;
					cpustate->out_irq_func(cpustate->irq ? ASSERT_LINE : CLEAR_LINE );
				}
				cpustate->sfr &= ~SUPERFX_SFR_G;
				cpustate->pipeline = 0x01;
				superfx_regs_reset(cpustate);
				break;
			case 0x01: // NOP
				superfx_regs_reset(cpustate);
				break;
			case 0x02: // CACHE
				if(cpustate->cbr != (cpustate->r[15] & 0xfff0))
				{
					cpustate->cbr = cpustate->r[15] & 0xfff0;
					superfx_cache_flush(cpustate);
				}
				superfx_regs_reset(cpustate);
				break;
			case 0x03: // LSR
				cpustate->sfr &= ~SUPERFX_SFR_CY;
				cpustate->sfr |= (*(cpustate->sreg) & 1) ? SUPERFX_SFR_CY : 0;
				superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) >> 1);
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;
			case 0x04: // ROL
			{
				UINT16 carry = *(cpustate->sreg) & 0x8000;
				superfx_gpr_write(cpustate, cpustate->dreg_idx, (*(cpustate->sreg) << 1) | (SUPERFX_SFR_CY_SET ? 1 : 0));
				cpustate->sfr &= ~SUPERFX_SFR_CY;
				cpustate->sfr |= carry ? SUPERFX_SFR_CY : 0;
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;
			}
			case 0x05: // BRA
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				break;
			}
			case 0x06: // BLT
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if((SUPERFX_SFR_S_SET ^ SUPERFX_SFR_OV_SET) == 0)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x07: // BGE
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if((SUPERFX_SFR_S_SET ^ SUPERFX_SFR_OV_SET) == 1)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x08: // BNE
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_Z_SET == 0)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x09: // BEQ
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_Z_SET == 1)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x0a: // BPL
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_S_SET == 0)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x0b: // BMI
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_S_SET == 1)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x0c: // BCC
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_CY_SET == 0)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x0d: // BCS
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_CY_SET == 1)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x0e: // BVC
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_OV_SET == 0)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}
			case 0x0f: // BVS
			{
				INT32 e = (INT8)superfx_pipe(cpustate);
				if(SUPERFX_SFR_OV_SET == 1)
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[15] + e);
				}
				break;
			}

			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:	// TO
				if((cpustate->sfr & SUPERFX_SFR_B) == 0)
				{
					cpustate->dreg = &cpustate->r[op & 0xf];
					cpustate->dreg_idx = op & 0xf;
				}
				else
				{
					superfx_gpr_write(cpustate, op & 0xf, *(cpustate->sreg));
					superfx_regs_reset(cpustate);
				}
				break;

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f: // WITH
				cpustate->sreg = &cpustate->r[op & 0xf];
				cpustate->sreg_idx = op & 0xf;
				cpustate->dreg = &cpustate->r[op & 0xf];
				cpustate->dreg_idx = op & 0xf;
				cpustate->sfr |= SUPERFX_SFR_B;
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35:
			case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b:	// STW_IR / STB_IR
				if((cpustate->sfr & SUPERFX_SFR_ALT1) == 0)
				{ // STW_IR
					cpustate->ramaddr = cpustate->r[op & 0xf];
					superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 0, (*(cpustate->sreg)) >> 0);
					superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 1, (*(cpustate->sreg)) >> 8);
					superfx_regs_reset(cpustate);
				}
				else
				{ // STB_IR
					cpustate->ramaddr = cpustate->r[op & 0xf];
					superfx_rambuffer_write(cpustate, cpustate->ramaddr, *(cpustate->sreg));
					superfx_regs_reset(cpustate);
				}
				break;

			case 0x3c: // LOOP
				superfx_gpr_write(cpustate, 12, cpustate->r[12] - 1);
				cpustate->sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				cpustate->sfr |= (cpustate->r[12] & 0x8000) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (cpustate->r[12] == 0) ? SUPERFX_SFR_Z : 0;
				if(!(cpustate->sfr & SUPERFX_SFR_Z))
				{
					superfx_gpr_write(cpustate, 15, cpustate->r[13]);
				}
				superfx_regs_reset(cpustate);
				break;
			case 0x3d: // ALT1
				cpustate->sfr &= ~SUPERFX_SFR_B;
				cpustate->sfr |= SUPERFX_SFR_ALT1;
				break;
			case 0x3e: // ALT2
				cpustate->sfr &= ~SUPERFX_SFR_B;
				cpustate->sfr |= SUPERFX_SFR_ALT2;
				break;
			case 0x3f: // ALT3
				cpustate->sfr &= ~SUPERFX_SFR_B;
				cpustate->sfr |= SUPERFX_SFR_ALT1;
				cpustate->sfr |= SUPERFX_SFR_ALT2;
				break;

			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45:
			case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b:	// LDW_IR / LDB_IR
				if((cpustate->sfr & SUPERFX_SFR_ALT1) == 0)
				{ // LDW_IR
					UINT16 data = 0;
					cpustate->ramaddr = cpustate->r[op & 0xf];
					data  = superfx_rambuffer_read(cpustate, cpustate->ramaddr ^ 0) << 0;
					data |= superfx_rambuffer_read(cpustate, cpustate->ramaddr ^ 1) << 8;
					superfx_gpr_write(cpustate, cpustate->dreg_idx, data);
					superfx_regs_reset(cpustate);
				}
				else
				{ // LDB_IR
					cpustate->ramaddr = cpustate->r[op & 0xf];
					superfx_gpr_write(cpustate, cpustate->dreg_idx, superfx_rambuffer_read(cpustate, cpustate->ramaddr));
					superfx_regs_reset(cpustate);
				}
				break;

			case 0x4c: // PLOT / RPIX
				if((cpustate->sfr & SUPERFX_SFR_ALT1) == 0)
				{ // PLOT
					superfx_plot(cpustate, cpustate->r[1], cpustate->r[2]);
					superfx_gpr_write(cpustate, 1, cpustate->r[1] + 1);
					superfx_regs_reset(cpustate);
				}
				else
				{ // RPIX
					superfx_gpr_write(cpustate, cpustate->dreg_idx, superfx_rpix(cpustate, cpustate->r[1], cpustate->r[2]));
					superfx_dreg_sfr_sz_update(cpustate);
					superfx_regs_reset(cpustate);
				}
				break;

			case 0x4d: // SWAP
				superfx_gpr_write(cpustate, cpustate->dreg_idx, (*(cpustate->sreg) >> 8) | (*(cpustate->sreg) << 8));
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;

			case 0x4e: // COLOR / CMODE
				if((cpustate->sfr & SUPERFX_SFR_ALT1) == 0)
				{ // COLOR
					cpustate->colr = superfx_color(cpustate, *(cpustate->sreg));
					superfx_regs_reset(cpustate);
				}
				else
				{ // CMODE
					cpustate->por = *(cpustate->sreg);
					superfx_regs_reset(cpustate);
				}
				break;

			case 0x4f: // NOT
				superfx_gpr_write(cpustate, cpustate->dreg_idx, ~(*(cpustate->sreg)));
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;

			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f: // ADD / ADC / ADDI / ADCI
			{
				INT32 r = *(cpustate->sreg);
				cpustate->sfr &= ~(SUPERFX_SFR_OV | SUPERFX_SFR_S | SUPERFX_SFR_Z);
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // ADD
						r += cpustate->r[op & 0xf];
						cpustate->sfr |= (~(*(cpustate->sreg) ^ cpustate->r[op & 0xf]) & (cpustate->r[op & 0xf] ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
					case SUPERFX_SFR_ALT1: // ADC
						r += cpustate->r[op & 0xf] + ((cpustate->sfr & SUPERFX_SFR_CY) ? 1 : 0);
						cpustate->sfr |= (~(*(cpustate->sreg) ^ cpustate->r[op & 0xf]) & (cpustate->r[op & 0xf] ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
					case SUPERFX_SFR_ALT2: // ADDI
						r += op & 0xf;
						cpustate->sfr |= (~(*(cpustate->sreg) ^ (op & 0xf)) & ((op & 0xf) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
					case SUPERFX_SFR_ALT3: // ADCI
						r += (op & 0xf) + ((cpustate->sfr & SUPERFX_SFR_CY) ? 1 : 0);
						cpustate->sfr |= (~(*(cpustate->sreg) ^ (op & 0xf)) & ((op & 0xf) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
				}
				cpustate->sfr &= ~SUPERFX_SFR_CY;
				cpustate->sfr |= (r & 0x8000) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (r >= 0x10000) ? SUPERFX_SFR_CY : 0;
				cpustate->sfr |= ((UINT16)r == 0) ? SUPERFX_SFR_Z : 0;
				superfx_gpr_write(cpustate, cpustate->dreg_idx, r);
				superfx_regs_reset(cpustate);
				break;
			}

			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f: // SUB / SBC / SUBI / CMP
			{
				INT32 r = 0;
				cpustate->sfr &= ~(SUPERFX_SFR_OV | SUPERFX_SFR_S | SUPERFX_SFR_Z);
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // SUB
						r = *(cpustate->sreg) - cpustate->r[op & 0xf];
						cpustate->sfr |= ((*(cpustate->sreg) ^ cpustate->r[op & 0xf]) & (*(cpustate->sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						superfx_gpr_write(cpustate, cpustate->dreg_idx, r);
						break;
					case SUPERFX_SFR_ALT1: // SBC
						r = *(cpustate->sreg) - cpustate->r[op & 0xf] - ((cpustate->sfr & SUPERFX_SFR_CY) ? 0 : 1);
						cpustate->sfr |= ((*(cpustate->sreg) ^ cpustate->r[op & 0xf]) & (*(cpustate->sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						superfx_gpr_write(cpustate, cpustate->dreg_idx, r);
						break;
					case SUPERFX_SFR_ALT2: // SUBI
						r = *(cpustate->sreg) - (op & 0xf);
						cpustate->sfr |= ((*(cpustate->sreg) ^ (op & 0xf)) & (*(cpustate->sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						superfx_gpr_write(cpustate, cpustate->dreg_idx, r);
						break;
					case SUPERFX_SFR_ALT3: // CMP
						r = *(cpustate->sreg) - cpustate->r[op & 0xf];
						cpustate->sfr |= ((*(cpustate->sreg) ^ cpustate->r[op & 0xf]) & (*(cpustate->sreg) ^ r) & 0x8000) ? SUPERFX_SFR_OV : 0;
						break;
				}
				cpustate->sfr &= ~SUPERFX_SFR_CY;
				cpustate->sfr |= (r & 0x8000) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (r >= 0x0) ? SUPERFX_SFR_CY : 0;
				cpustate->sfr |= ((UINT16)r == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset(cpustate);
				break;
			}

			case 0x70: // MERGE
				superfx_gpr_write(cpustate, cpustate->dreg_idx, (cpustate->r[7] & 0xff00) | (cpustate->r[8] >> 8));
				cpustate->sfr &= ~(SUPERFX_SFR_OV | SUPERFX_SFR_S | SUPERFX_SFR_CY | SUPERFX_SFR_Z);
				cpustate->sfr |= (*(cpustate->dreg) & 0xc0c0) ? SUPERFX_SFR_OV : 0;
				cpustate->sfr |= (*(cpustate->dreg) & 0x8080) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (*(cpustate->dreg) & 0xe0e0) ? SUPERFX_SFR_CY : 0;
				cpustate->sfr |= (*(cpustate->dreg) & 0xf0f0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset(cpustate);
				break;

			case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f: // AND / BIC / ANDI / BICI
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // AND
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) & cpustate->r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT1: // BIC
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) & ~cpustate->r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT2: // ANDI
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) & (op & 0xf));
						break;
					case SUPERFX_SFR_ALT3: // BICI
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) & ~(op & 0xf));
						break;
				}
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;

			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f: // MULT / UMULT / MULTI / UMULTI
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // MULT
						superfx_gpr_write(cpustate, cpustate->dreg_idx, (INT8)(*(cpustate->sreg)) * (INT8)(cpustate->r[op & 0xf]));
						break;
					case SUPERFX_SFR_ALT1: // UMULT
						superfx_gpr_write(cpustate, cpustate->dreg_idx, (UINT8)(*(cpustate->sreg)) * (UINT8)(cpustate->r[op & 0xf]));
						break;
					case SUPERFX_SFR_ALT2: // MULTI
						superfx_gpr_write(cpustate, cpustate->dreg_idx, (INT8)(*(cpustate->sreg)) * (INT8)(op & 0xf));
						break;
					case SUPERFX_SFR_ALT3: // UMULTI
						superfx_gpr_write(cpustate, cpustate->dreg_idx, (UINT8)(*(cpustate->sreg)) * (UINT8)(op & 0xf));
						break;
				}
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				if(!(cpustate->cfgr & SUPERFX_CFGR_MS0))
				{
					superfx_add_clocks_internal(cpustate, 2);
				}
				break;

			case 0x90: // SBK
				superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 0, *(cpustate->sreg) >> 0);
				superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 1, *(cpustate->sreg) >> 8);
				superfx_regs_reset(cpustate);
				break;

			case 0x91: case 0x92: case 0x93: case 0x94: // LINK
				superfx_gpr_write(cpustate, 11, cpustate->r[15] + (op & 0xf));
				superfx_regs_reset(cpustate);
				break;

			case 0x95: // SEX
				superfx_gpr_write(cpustate, cpustate->dreg_idx, (INT8)(*(cpustate->sreg)));
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;

			case 0x96: // ASR / DIV2
				if((cpustate->sfr & SUPERFX_SFR_ALT1) == 0)
				{ // ASR
					cpustate->sfr &= ~SUPERFX_SFR_CY;
					cpustate->sfr |= (*(cpustate->sreg) & 1) ? SUPERFX_SFR_CY : 0;
					superfx_gpr_write(cpustate, cpustate->dreg_idx, (INT16)(*(cpustate->sreg)) >> 1);
					superfx_dreg_sfr_sz_update(cpustate);
					superfx_regs_reset(cpustate);
				}
				else
				{ // DIV2
					cpustate->sfr &= ~SUPERFX_SFR_CY;
					cpustate->sfr |= (*(cpustate->sreg) & 1) ? SUPERFX_SFR_CY : 0;
					superfx_gpr_write(cpustate, cpustate->dreg_idx, ((INT16)(*(cpustate->sreg)) >> 1) + ((UINT32)(*(cpustate->sreg) + 1) >> 16));
					superfx_dreg_sfr_sz_update(cpustate);
					superfx_regs_reset(cpustate);
				}
				break;

			case 0x97: // ROR
			{
				UINT16 carry = *(cpustate->sreg) & 1;
				superfx_gpr_write(cpustate, cpustate->dreg_idx, (SUPERFX_SFR_CY_SET << 15) | ((UINT16)(*(cpustate->sreg)) >> 1));
				cpustate->sfr &= ~SUPERFX_SFR_CY;
				cpustate->sfr |= carry ? SUPERFX_SFR_CY : 0;
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;
			}

			case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: // JMP / LJMP
				if((cpustate->sfr & SUPERFX_SFR_ALT1) == 0)
				{ // JMP
					superfx_gpr_write(cpustate, 15, cpustate->r[op & 0xf]);
					superfx_regs_reset(cpustate);
				}
				else
				{ // LJMP
					cpustate->pbr = cpustate->r[op & 0xf] & 0x7f;
					superfx_gpr_write(cpustate, 15, *(cpustate->sreg));
					cpustate->cbr = cpustate->r[15] & 0xfff0;
					superfx_cache_flush(cpustate);
					superfx_regs_reset(cpustate);
				}
				break;

			case 0x9e: // LOB
				superfx_gpr_write(cpustate, cpustate->dreg_idx, (UINT16)(*(cpustate->sreg)) & 0x00ff);
				cpustate->sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				cpustate->sfr |= (*(cpustate->dreg) & 0x80) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (*(cpustate->dreg) == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset(cpustate);
				break;

			case 0x9f: // FMULT / LMULT
			{
				UINT32 result = (INT16)(*(cpustate->sreg)) * (INT16)(cpustate->r[6]);
				if(cpustate->sfr & SUPERFX_SFR_ALT1)
				{ // LMULT
					superfx_gpr_write(cpustate, 4, result);
				}
				superfx_gpr_write(cpustate, cpustate->dreg_idx, result >> 16);
				cpustate->sfr &= ~SUPERFX_SFR_CY;
				cpustate->sfr |= (result & 0x8000) ? SUPERFX_SFR_CY : 0;
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				superfx_add_clocks_internal(cpustate, 4 + ((cpustate->cfgr & SUPERFX_CFGR_MS0) ? 4 : 0));
				break;
			}

			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf: // IBT / LMS / SMS / LMS
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // IBT
						superfx_gpr_write(cpustate, op & 0xf, (INT8)superfx_pipe(cpustate));
						superfx_regs_reset(cpustate);
						break;
					case SUPERFX_SFR_ALT2: // SMS
						cpustate->ramaddr = superfx_pipe(cpustate) << 1;
						superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 0, cpustate->r[op & 0xf] >> 0);
						superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 1, cpustate->r[op & 0xf] >> 8);
						superfx_regs_reset(cpustate);
						break;
					case SUPERFX_SFR_ALT1: // LMS
					case SUPERFX_SFR_ALT3: // LMS
					{
						UINT16 data = 0;
						cpustate->ramaddr = superfx_pipe(cpustate) << 1;
						data  = superfx_rambuffer_read(cpustate, cpustate->ramaddr ^ 0) << 0;
						data |= superfx_rambuffer_read(cpustate, cpustate->ramaddr ^ 1) << 8;
						superfx_gpr_write(cpustate, op & 0xf, data);
						superfx_regs_reset(cpustate);
						break;
					}
				}
				break;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf: // FROM
				if((cpustate->sfr & SUPERFX_SFR_B) == 0)
				{
					cpustate->sreg = &(cpustate->r[op & 0xf]);
					cpustate->sreg_idx = op & 0xf;
				}
				else
				{
					superfx_gpr_write(cpustate, cpustate->dreg_idx, cpustate->r[op & 0xf]);
					cpustate->sfr &= ~SUPERFX_SFR_OV;
					cpustate->sfr |= (*(cpustate->dreg) & 0x80) ? SUPERFX_SFR_OV : 0;
					superfx_dreg_sfr_sz_update(cpustate);
					superfx_regs_reset(cpustate);
				}
				break;

			case 0xc0: // HIB
				superfx_gpr_write(cpustate, cpustate->dreg_idx, (*(cpustate->sreg)) >> 8);
				cpustate->sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				cpustate->sfr |= (*(cpustate->dreg) & 0x80) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (*(cpustate->dreg) == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset(cpustate);
				break;

			case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf: // OR / XOR / ORI / XORI
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // OR
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) | cpustate->r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT1: // XOR
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) ^ cpustate->r[op & 0xf]);
						break;
					case SUPERFX_SFR_ALT2: // ORI
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) | (op & 0xf));
						break;
					case SUPERFX_SFR_ALT3: // XORI
						superfx_gpr_write(cpustate, cpustate->dreg_idx, *(cpustate->sreg) ^ (op & 0xf));
						break;
				}
				superfx_dreg_sfr_sz_update(cpustate);
				superfx_regs_reset(cpustate);
				break;

			case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde:            // INC
				superfx_gpr_write(cpustate, op & 0xf, cpustate->r[op & 0xf] + 1);
				cpustate->sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				cpustate->sfr |= (cpustate->r[op & 0xf] & 0x8000) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (cpustate->r[op & 0xf] == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset(cpustate);
				break;

			case 0xdf: // GETC / RAMB / ROMB
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // GETC
					case SUPERFX_SFR_ALT1: // GETC
						cpustate->colr = superfx_color(cpustate, superfx_rombuffer_read(cpustate));
						superfx_regs_reset(cpustate);
						break;
					case SUPERFX_SFR_ALT2: // RAMB
						superfx_rambuffer_sync(cpustate);
						cpustate->rambr = ((*(cpustate->sreg)) & 1) ? 1 : 0;
						superfx_regs_reset(cpustate);
						break;
					case SUPERFX_SFR_ALT3: // ROMB
						superfx_rombuffer_sync(cpustate);
						cpustate->rombr = *(cpustate->sreg) & 0x7f;
						superfx_regs_reset(cpustate);
						break;
				}
				break;

			case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee:            // DEC
				superfx_gpr_write(cpustate, op & 0xf, cpustate->r[op & 0xf] - 1);
				cpustate->sfr &= ~(SUPERFX_SFR_S | SUPERFX_SFR_Z);
				cpustate->sfr |= (cpustate->r[op & 0xf] & 0x8000) ? SUPERFX_SFR_S : 0;
				cpustate->sfr |= (cpustate->r[op & 0xf] == 0) ? SUPERFX_SFR_Z : 0;
				superfx_regs_reset(cpustate);
				break;

			case 0xef: // GETB / GETBH / GETBL / GETBS
			{
				UINT8 byte = superfx_rombuffer_read(cpustate);
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // GETB
						superfx_gpr_write(cpustate, cpustate->dreg_idx, byte);
						break;
					case SUPERFX_SFR_ALT1: // GETBH
						superfx_gpr_write(cpustate, cpustate->dreg_idx, (byte << 8) | (*(cpustate->sreg) & 0x00ff));
						break;
					case SUPERFX_SFR_ALT2: // GETBL
						superfx_gpr_write(cpustate, cpustate->dreg_idx, (*(cpustate->sreg) & 0xff00) | (byte << 0));
						break;
					case SUPERFX_SFR_ALT3: // GETBS
						superfx_gpr_write(cpustate, cpustate->dreg_idx, (INT8)byte);
						break;
				}
				superfx_regs_reset(cpustate);
				break;
			}

			case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
			case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff: // IWT / LM / SM / LM
			{
				UINT16 data;
				switch(cpustate->sfr & SUPERFX_SFR_ALT)
				{
					case SUPERFX_SFR_ALT0: // IWT
						data  = superfx_pipe(cpustate) << 0;
						data |= superfx_pipe(cpustate) << 8;
						superfx_gpr_write(cpustate, op & 0xf, data);
						superfx_regs_reset(cpustate);
						break;
					case SUPERFX_SFR_ALT2: // SM
						cpustate->ramaddr  = superfx_pipe(cpustate) << 0;
						cpustate->ramaddr |= superfx_pipe(cpustate) << 8;
						superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 0, cpustate->r[op & 0xf] >> 0);
						superfx_rambuffer_write(cpustate, cpustate->ramaddr ^ 1, cpustate->r[op & 0xf] >> 8);
						superfx_regs_reset(cpustate);
						break;
					case SUPERFX_SFR_ALT1:
					case SUPERFX_SFR_ALT3: // LM
						cpustate->ramaddr  = superfx_pipe(cpustate) << 0;
						cpustate->ramaddr |= superfx_pipe(cpustate) << 8;
						data  = superfx_rambuffer_read(cpustate, cpustate->ramaddr ^ 0) << 0;
						data |= superfx_rambuffer_read(cpustate, cpustate->ramaddr ^ 1) << 8;
						superfx_gpr_write(cpustate, op & 0xf, data);
						superfx_regs_reset(cpustate);
						break;
				}
				break;
			}
		}

		if(!cpustate->r15_modified)
		{
			cpustate->r[15]++;
		}

		//printf( " r0:%04x  r1:%04x  r2:%04x  r3:%04x  r4:%04x  r5:%04x  r6:%04x  r7:%04x\n",  cpustate->r[0],  cpustate->r[1],  cpustate->r[2],  cpustate->r[3],  cpustate->r[4],  cpustate->r[5],  cpustate->r[6],  cpustate->r[7] );
		//printf( " r8:%04x  r9:%04x r10:%04x r11:%04x r12:%04x r13:%04x r14:%04x r15:%04x\n",  cpustate->r[8],  cpustate->r[9], cpustate->r[10], cpustate->r[11], cpustate->r[12], cpustate->r[13], cpustate->r[14], cpustate->r[15] );
		//printf( "sfr:%04x\n", cpustate->sfr );

		--cpustate->icount;
	}
}

/*****************************************************************************/

CPU_DISASSEMBLE( superfx )
{
	superfx_state *cpustate = get_safe_token(device);

	UINT8  op = *(UINT8 *)(opram + 0);
	UINT8  param0 = *(UINT8 *)(opram + 1);
	UINT8  param1 = *(UINT8 *)(opram + 2);
	UINT16 alt = cpustate->sfr & SUPERFX_SFR_ALT;

	return superfx_dasm_one(buffer, pc, op, param0, param1, alt);
}

/*****************************************************************************/

static CPU_SET_INFO( superfx )
{
	superfx_state *cpustate = get_safe_token(device);

	switch (state)
	{
	/* --- the following bits of info are set as 64-bit signed integers --- */
	case CPUINFO_INT_PC:
	case CPUINFO_INT_REGISTER + SUPERFX_PC:     	cpustate->r[15] = info->i;      break;
	case CPUINFO_INT_REGISTER + SUPERFX_DREG:		info->i = cpustate->dreg_idx;	break;
	case CPUINFO_INT_REGISTER + SUPERFX_SREG:		info->i = cpustate->sreg_idx;	break;
	case CPUINFO_INT_REGISTER + SUPERFX_R0:         cpustate->r[0] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R1:         cpustate->r[1] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R2:         cpustate->r[2] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R3:         cpustate->r[3] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R4:         cpustate->r[4] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R5:         cpustate->r[5] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R6:         cpustate->r[6] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R7:         cpustate->r[7] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R8:         cpustate->r[8] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R9:         cpustate->r[9] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R10:        cpustate->r[10] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R11:        cpustate->r[11] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R12:        cpustate->r[12] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R13:        cpustate->r[13] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R14:        cpustate->r[14] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R15:        cpustate->r[15] = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_SFR:    	cpustate->sfr = info->i;    	break;
	case CPUINFO_INT_REGISTER + SUPERFX_PBR:    	cpustate->pbr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_ROMBR:		cpustate->rombr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMBR:		cpustate->rambr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_CBR:		cpustate->cbr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_SCBR:		cpustate->scbr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_SCMR:		cpustate->scmr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_COLR:		cpustate->colr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_POR:		cpustate->por = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_BRAMR:		cpustate->bramr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_VCR:		cpustate->vcr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_CFGR:		cpustate->cfgr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_CLSR:		cpustate->clsr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_ROMCL:		cpustate->romcl = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_ROMDR:		cpustate->romdr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMCL:		cpustate->ramcl = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMAR:		cpustate->ramar = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMDR:		cpustate->ramdr = info->i;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMADDR:	cpustate->ramaddr = info->i;	break;
    }
}

CPU_GET_INFO( superfx )
{
	superfx_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:          	info->i = sizeof(superfx_state);					break;
	case CPUINFO_INT_INPUT_LINES:           	info->i = 0;                    					break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:    	info->i = 0;                    					break;
	case CPUINFO_INT_ENDIANNESS:            	info->i = ENDIANNESS_LITTLE;    					break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:      	info->i = 1;                    					break;
	case CPUINFO_INT_CLOCK_DIVIDER:         	info->i = 1;                    					break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES: 	info->i = 1;                    					break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES: 	info->i = 3;                    					break;
	case CPUINFO_INT_MIN_CYCLES:            	info->i = 1;                    					break;
	case CPUINFO_INT_MAX_CYCLES:            	info->i = 1;                    					break;

	case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;                						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 32;               						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;                						break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:   	info->i = 0;                						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:   	info->i = 0;                						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:   	info->i = 0;                						break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     	info->i = 0;                						break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     	info->i = 0;                						break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     	info->i = 0;                						break;

	case CPUINFO_INT_PC:    /* intentional fallthrough */
	case CPUINFO_INT_REGISTER + SUPERFX_PC: 		info->i = ((cpustate->pbr << 16) | cpustate->r[15]) - 1; break;
	case CPUINFO_INT_REGISTER + SUPERFX_DREG:		info->i = cpustate->dreg_idx;	break;
	case CPUINFO_INT_REGISTER + SUPERFX_SREG:		info->i = cpustate->sreg_idx;	break;
	case CPUINFO_INT_REGISTER + SUPERFX_R0:         info->i = cpustate->r[0];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R1:         info->i = cpustate->r[1];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R2:         info->i = cpustate->r[2];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R3:         info->i = cpustate->r[3];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R4:         info->i = cpustate->r[4];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R5:         info->i = cpustate->r[5];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R6:         info->i = cpustate->r[6];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R7:         info->i = cpustate->r[7];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R8:         info->i = cpustate->r[8];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R9:         info->i = cpustate->r[9];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R10:        info->i = cpustate->r[10];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R11:        info->i = cpustate->r[11];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R12:        info->i = cpustate->r[12];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R13:        info->i = cpustate->r[13];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R14:        info->i = cpustate->r[14];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_R15:        info->i = cpustate->r[15];		break;
	case CPUINFO_INT_REGISTER + SUPERFX_SFR:		info->i = cpustate->sfr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_PBR:    	info->i = cpustate->sfr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_ROMBR:		info->i = cpustate->rombr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMBR:		info->i = cpustate->rambr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_CBR:		info->i = cpustate->cbr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_SCBR:		info->i = cpustate->scbr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_SCMR:		info->i = cpustate->scmr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_COLR:		info->i = cpustate->colr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_POR:		info->i = cpustate->por;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_BRAMR:		info->i = cpustate->bramr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_VCR:		info->i = cpustate->vcr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_CFGR:		info->i = cpustate->cfgr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_CLSR:		info->i = cpustate->clsr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_ROMCL:		info->i = cpustate->romcl;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_ROMDR:		info->i = cpustate->romdr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMCL:		info->i = cpustate->ramcl;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMAR:		info->i = cpustate->ramar;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMDR:		info->i = cpustate->ramdr;		break;
	case CPUINFO_INT_REGISTER + SUPERFX_RAMADDR:	info->i = cpustate->ramaddr;	break;

        /* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_SET_INFO:              info->setinfo = CPU_SET_INFO_NAME(superfx);     		break;
	case CPUINFO_FCT_INIT:                  info->init = CPU_INIT_NAME(superfx);            		break;
	case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(superfx);          		break;
	case CPUINFO_FCT_EXIT:                  info->exit = CPU_EXIT_NAME(superfx);            		break;
	case CPUINFO_FCT_EXECUTE:               info->execute = CPU_EXECUTE_NAME(superfx);      		break;
	case CPUINFO_FCT_BURN:                  info->burn = NULL;                              		break;
	case CPUINFO_FCT_DISASSEMBLE:           info->disassemble = CPU_DISASSEMBLE_NAME(superfx);		break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:   info->icount = &cpustate->icount;               		break;

        /* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME:                          strcpy(info->s, "SuperFX");						break;
	case CPUINFO_STR_FAMILY:                		strcpy(info->s, "SuperFX");						break;
	case CPUINFO_STR_VERSION:               		strcpy(info->s, "1.0");							break;
	case CPUINFO_STR_SOURCE_FILE:                   strcpy(info->s, __FILE__);						break;
	case CPUINFO_STR_CREDITS:               		strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team");	break;

	case CPUINFO_STR_FLAGS:                         strcpy(info->s, " ");							break;

	case CPUINFO_STR_REGISTER + SUPERFX_PC:         sprintf(info->s, "PC:      %06X", (cpustate->pbr << 16) | cpustate->r[15]); break;
	case CPUINFO_STR_REGISTER + SUPERFX_DREG:		sprintf(info->s, "DREG:    R%d",  cpustate->dreg_idx);	break;
	case CPUINFO_STR_REGISTER + SUPERFX_SREG:		sprintf(info->s, "SREG:    R%d",  cpustate->sreg_idx);	break;
	case CPUINFO_STR_REGISTER + SUPERFX_R0:         sprintf(info->s, "R0:      %04X", cpustate->r[0]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R1:         sprintf(info->s, "R1:      %04X", cpustate->r[1]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R2:         sprintf(info->s, "R2:      %04X", cpustate->r[2]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R3:         sprintf(info->s, "R3:      %04X", cpustate->r[3]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R4:         sprintf(info->s, "R4:      %04X", cpustate->r[4]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R5:         sprintf(info->s, "R5:      %04X", cpustate->r[5]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R6:         sprintf(info->s, "R6:      %04X", cpustate->r[6]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R7:         sprintf(info->s, "R7:      %04X", cpustate->r[7]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R8:         sprintf(info->s, "R8:      %04X", cpustate->r[8]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R9:         sprintf(info->s, "R9:      %04X", cpustate->r[9]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R10:        sprintf(info->s, "R10:     %04X", cpustate->r[10]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R11:        sprintf(info->s, "R11:     %04X", cpustate->r[11]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R12:        sprintf(info->s, "R12:     %04X", cpustate->r[12]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R13:        sprintf(info->s, "R13:     %04X", cpustate->r[13]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R14:        sprintf(info->s, "R14:     %04X", cpustate->r[14]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_R15:        sprintf(info->s, "R15:     %04X", cpustate->r[15]);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_SFR:    	sprintf(info->s, "SFR:     %04X", cpustate->sfr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_PBR:    	sprintf(info->s, "PBR:     %02X", cpustate->sfr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_ROMBR:		sprintf(info->s, "ROMBR:   %02X", cpustate->rombr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_RAMBR:		sprintf(info->s, "RAMBR:   %02X", cpustate->rambr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_CBR:		sprintf(info->s, "CBR:     %04X", cpustate->cbr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_SCBR:		sprintf(info->s, "SCBR:    %02X", cpustate->scbr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_SCMR:		sprintf(info->s, "SCMR:    %02X", cpustate->scmr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_COLR:		sprintf(info->s, "COLR     %02X", cpustate->colr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_POR:		sprintf(info->s, "POR:     %02X", cpustate->por);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_BRAMR:		sprintf(info->s, "BRAMR:   %02X", cpustate->bramr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_VCR:		sprintf(info->s, "VCR:     %02X", cpustate->vcr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_CFGR:		sprintf(info->s, "CFGR:    %02X", cpustate->cfgr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_CLSR:		sprintf(info->s, "CLSR:    %02X", cpustate->clsr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_ROMCL:		sprintf(info->s, "ROMCL:   %08X", cpustate->romcl);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_ROMDR:		sprintf(info->s, "ROMDR:   %02X", cpustate->romdr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_RAMCL:		sprintf(info->s, "RAMCL:   %08X", cpustate->ramcl);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_RAMAR:		sprintf(info->s, "RAMAR:   %04X", cpustate->ramar);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_RAMDR:		sprintf(info->s, "RAMDR:   %02X", cpustate->ramdr);		break;
	case CPUINFO_STR_REGISTER + SUPERFX_RAMADDR:	sprintf(info->s, "RAMADDR: %04X", cpustate->ramaddr);	break;
    }
}

DEFINE_LEGACY_CPU_DEVICE(SUPERFX, superfx);
