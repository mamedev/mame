/*******************************************************************

Toshiba TLCS-900/H emulation

This code only supports the 900/H mode which is needed for Neogeo
Pocket emulation. The 900 and 900/M modes are not supported yet.


TODO:
- review cycle counts
- implement the remaining internal mcu features (serial transfer, etc)
- add support for 900 and 900/M modes

*******************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tlcs900.h"


const device_type TLCS900H = &device_creator<tlcs900h_device>;
const device_type TMP95C063 = &device_creator<tmp95c063_device>;


static ADDRESS_MAP_START( tlcs900_mem, AS_PROGRAM, 8, tlcs900h_device )
	AM_RANGE( 0x000000, 0x00007f ) AM_READWRITE( tlcs900_internal_r, tlcs900_internal_w )
ADDRESS_MAP_END


static ADDRESS_MAP_START(tmp95c063_mem, AS_PROGRAM, 8, tlcs900h_device )
	AM_RANGE( 0x000000, 0x00009f ) AM_READWRITE( tmp95c063_internal_r, tmp95c063_internal_w )
ADDRESS_MAP_END


tlcs900h_device::tlcs900h_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TLCS900H, "TLCS-900/H", tag, owner, clock, "tlcs900h", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 24, 0, ADDRESS_MAP_NAME(tlcs900_mem) )
	, m_to1(*this)
	, m_to3(*this)
	, m_port_read(*this)
	, m_port_write(*this)
{
}


tlcs900h_device::tlcs900h_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, address_map_constructor internal_map)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 24, 0, internal_map)
	, m_to1(*this)
	, m_to3(*this)
	, m_port_read(*this)
	, m_port_write(*this)
{
}

tmp95c063_device::tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tlcs900h_device(mconfig, TMP95C063, "TMP95C063", tag, owner, clock, "tmp95c063", ADDRESS_MAP_NAME(tmp95c063_mem) )
{
}


offs_t tlcs900h_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tlcs900 );
	return CPU_DISASSEMBLE_NAME(tlcs900)(this, buffer, pc, oprom, opram, options);
}



/* Internal register defines */
#define TLCS900_P1          0x01
#define TLCS900_P1CR        0x02
#define TLCS900_P2          0x06
#define TLCS900_P2FC        0x09
#define TLCS900_P5          0x0d
#define TLCS900_P5CR        0x10
#define TLCS900_P5FC        0x11
#define TLCS900_P6          0x12
#define TLCS900_P7          0x13
#define TLCS900_P6FC        0x15
#define TLCS900_P7CR        0x16
#define TLCS900_P7FC        0x17
#define TLCS900_P8          0x18
#define TLCS900_P9          0x19
#define TLCS900_P8CR        0x1a
#define TLCS900_P8FC        0x1b
#define TLCS900_PA          0x1e
#define TLCS900_PB          0x1f
#define TLCS900_TRUN        0x20
#define TLCS900_TREG0       0x22
#define TLCS900_TREG1       0x23
#define TLCS900_T01MOD      0x24
#define TLCS900_TFFCR       0x25
#define TLCS900_TREG2       0x26
#define TLCS900_TREG3       0x27
#define TLCS900_T23MOD      0x28
#define TLCS900_TRDC        0x29
#define TLCS900_PACR        0x2c
#define TLCS900_PAFC        0x2d
#define TLCS900_PBCR        0x2e
#define TLCS900_PBFC        0x2f
#define TLCS900_TREG4L      0x30
#define TLCS900_TREG4H      0x31
#define TLCS900_TREG5L      0x32
#define TLCS900_TREG5H      0x33
#define TLCS900_CAP1L       0x34
#define TLCS900_CAP1H       0x35
#define TLCS900_CAP2L       0x36
#define TLCS900_CAP2H       0x37
#define TLCS900_T4MOD       0x38
#define TLCS900_T4FFCR      0x39
#define TLCS900_T45CR       0x3a
#define TLCS900_MSAR0       0x3c
#define TLCS900_MAMR0       0x3d
#define TLCS900_MSAR1       0x3e
#define TLCS900_MAMR1       0x3f
#define TLCS900_TREG6L      0x40
#define TLCS900_TREG6H      0x41
#define TLCS900_TREG7L      0x42
#define TLCS900_TREG7H      0x43
#define TLCS900_CAP3L       0x44
#define TLCS900_CAP3H       0x45
#define TLCS900_CAP4L       0x46
#define TLCS900_CAP4H       0x47
#define TLCS900_T5MOD       0x48
#define TLCS900_T5FFCR      0x49
#define TLCS900_PG0REG      0x4c
#define TLCS900_PG1REG      0x4d
#define TLCS900_PG01CR      0x4e
#define TLCS900_SC0BUF      0x50
#define TLCS900_SC0CR       0x51
#define TLCS900_SC0MOD      0x52
#define TLCS900_BR0CR       0x53
#define TLCS900_SC1BUF      0x54
#define TLCS900_SC1CR       0x55
#define TLCS900_SC1MOD      0x56
#define TLCS900_BR1CR       0x57
#define TLCS900_ODE         0x58
#define TLCS900_DREFCR      0x5a
#define TLCS900_DMEMCR      0x5b
#define TLCS900_MSAR2       0x5c
#define TLCS900_MAMR2       0x5d
#define TLCS900_MSAR3       0x5e
#define TLCS900_MAMR3       0x5f
#define TLCS900_ADREG0L     0x60
#define TLCS900_ADREG0H     0x61
#define TLCS900_ADREG1L     0x62
#define TLCS900_ADREG1H     0x63
#define TLCS900_ADREG2L     0x64
#define TLCS900_ADREG2H     0x65
#define TLCS900_ADREG3L     0x66
#define TLCS900_ADREG3H     0x67
#define TLCS900_B0CS        0x68
#define TLCS900_B1CS        0x69
#define TLCS900_B2CS        0x6a
#define TLCS900_B3CS        0x6b
#define TLCS900_BEXCS       0x6c
#define TLCS900_ADMOD       0x6d
#define TLCS900_WDMOD       0x6e
#define TLCS900_WDCR        0x6f
#define TLCS900_INTE0AD     0x70
#define TLCS900_INTE45      0x71
#define TLCS900_INTE67      0x72
#define TLCS900_INTET10     0x73
#define TLCS900_INTET32     0x74
#define TLCS900_INTET54     0x75
#define TLCS900_INTET76     0x76
#define TLCS900_INTES0      0x77
#define TLCS900_INTES1      0x78
#define TLCS900_INTETC10    0x79
#define TLCS900_INTETC32    0x7a
#define TLCS900_IIMC        0x7b
#define TLCS900_DMA0V       0x7c
#define TLCS900_DMA1V       0x7d
#define TLCS900_DMA2V       0x7e
#define TLCS900_DMA3V       0x7f


/* Flag defines */
#define FLAG_CF     0x01
#define FLAG_NF     0x02
#define FLAG_VF     0x04
#define FLAG_HF     0x10
#define FLAG_ZF     0x40
#define FLAG_SF     0x80


#define RDMEM(addr)         m_program->read_byte( addr )
#define WRMEM(addr,data)    m_program->write_byte( addr, data )
#define RDMEMW(addr)            ( RDMEM(addr) | ( RDMEM(addr+1) << 8 ) )
#define RDMEML(addr)            ( RDMEMW(addr) | ( RDMEMW(addr+2) << 16 ) )
#define WRMEMW(addr,data)       { UINT16 dw = data; WRMEM(addr,dw & 0xff); WRMEM(addr+1,(dw >> 8 )); }
#define WRMEML(addr,data)       { UINT32 dl = data; WRMEMW(addr,dl); WRMEMW(addr+2,(dl >> 16)); }


inline UINT8 tlcs900h_device::RDOP()
{
	UINT8 data;

	if ( m_prefetch_clear )
	{
		for ( int i = 0; i < 4; i++ )
		{
			m_prefetch[ i ] = RDMEM( m_pc.d + i );
		}
		m_prefetch_index = 0;
		m_prefetch_clear = false;
	}
	else
	{
		m_prefetch[ m_prefetch_index ] = RDMEM( m_pc.d + 3 );
		m_prefetch_index = ( m_prefetch_index + 1 ) & 0x03;
	}
	data = m_prefetch[ m_prefetch_index ];
	m_pc.d++;
	return data;
}


void tlcs900h_device::device_start()
{
	m_program = &space( AS_PROGRAM );

	m_to1.resolve_safe();
	m_to3.resolve_safe();

	m_port_read.resolve_safe(0);
	m_port_write.resolve_safe();

	save_item( NAME(m_xwa) );
	save_item( NAME(m_xbc) );
	save_item( NAME(m_xde) );
	save_item( NAME(m_xhl) );
	save_item( NAME(m_xix) );
	save_item( NAME(m_xiy) );
	save_item( NAME(m_xiz) );
	save_item( NAME(m_xssp) );
	save_item( NAME(m_xnsp) );
	save_item( NAME(m_pc) );
	save_item( NAME(m_sr) );
	save_item( NAME(m_f2) );
	save_item( NAME(m_dmas) );
	save_item( NAME(m_dmad) );
	save_item( NAME(m_dmac) );
	save_item( NAME(m_dmam) );
	save_item( NAME(m_reg) );
	save_item( NAME(m_timer_pre) );
	save_item( NAME(m_timer) );
	save_item( NAME(m_tff1) );
	save_item( NAME(m_tff3) );
	save_item( NAME(m_timer_change) );
	save_item( NAME(m_level) );
	save_item( NAME(m_check_irqs) );
	save_item( NAME(m_ad_cycles_left) );
	save_item( NAME(m_nmi_state) );
	save_item( NAME(m_prefetch_clear) );
	save_item( NAME(m_prefetch_index) );
	save_item( NAME(m_prefetch) );

	state_add( TLCS900_PC,    "PC",    m_pc.d ).formatstr("%08X");
	state_add( TLCS900_XWA0,  "XWA0",  m_xwa[0].d ).formatstr("%08X");
	state_add( TLCS900_XBC0,  "XBC0",  m_xbc[0].d ).formatstr("%08X");
	state_add( TLCS900_XDE0,  "XDE0",  m_xde[0].d ).formatstr("%08X");
	state_add( TLCS900_XHL0,  "XHL0",  m_xhl[0].d ).formatstr("%08X");
	state_add( TLCS900_XWA1,  "XWA1",  m_xwa[1].d ).formatstr("%08X");
	state_add( TLCS900_XBC1,  "XBC1",  m_xbc[1].d ).formatstr("%08X");
	state_add( TLCS900_XDE1,  "XDE1",  m_xde[1].d ).formatstr("%08X");
	state_add( TLCS900_XHL1,  "XHL1",  m_xhl[1].d ).formatstr("%08X");
	state_add( TLCS900_XWA2,  "XWA2",  m_xwa[2].d ).formatstr("%08X");
	state_add( TLCS900_XBC2,  "XBC2",  m_xbc[2].d ).formatstr("%08X");
	state_add( TLCS900_XDE2,  "XDE2",  m_xde[2].d ).formatstr("%08X");
	state_add( TLCS900_XHL2,  "XHL2",  m_xhl[2].d ).formatstr("%08X");
	state_add( TLCS900_XWA3,  "XWA3",  m_xwa[3].d ).formatstr("%08X");
	state_add( TLCS900_XBC3,  "XBC3",  m_xbc[3].d ).formatstr("%08X");
	state_add( TLCS900_XDE3,  "XDE3",  m_xde[3].d ).formatstr("%08X");
	state_add( TLCS900_XHL3,  "XHL3",  m_xhl[3].d ).formatstr("%08X");
	state_add( TLCS900_XIX,   "XIX",   m_xix.d ).formatstr("%08X");
	state_add( TLCS900_XIY,   "XIY",   m_xiy.d ).formatstr("%08X");
	state_add( TLCS900_XIZ,   "XIZ",   m_xiz.d ).formatstr("%08X");
	state_add( TLCS900_XNSP,  "XNSP",  m_xnsp.d ).formatstr("%08X");
	state_add( TLCS900_XSSP,  "XSSP",  m_xssp.d ).formatstr("%08X");
	state_add( TLCS900_DMAS0, "DMAS0", m_dmas[0].d ).formatstr("%08X");
	state_add( TLCS900_DMAD0, "DMAD0", m_dmad[0].d ).formatstr("%08X");
	state_add( TLCS900_DMAC0, "DMAC0", m_dmac[0].w.l ).formatstr("%04X");
	state_add( TLCS900_DMAM0, "DMAM0", m_dmam[0].b.l ).formatstr("%02X");
	state_add( TLCS900_DMAS1, "DMAS0", m_dmas[1].d ).formatstr("%08X");
	state_add( TLCS900_DMAD1, "DMAD0", m_dmad[1].d ).formatstr("%08X");
	state_add( TLCS900_DMAC1, "DMAC0", m_dmac[1].w.l ).formatstr("%04X");
	state_add( TLCS900_DMAM1, "DMAM0", m_dmam[1].b.l ).formatstr("%02X");
	state_add( TLCS900_DMAS2, "DMAS0", m_dmas[2].d ).formatstr("%08X");
	state_add( TLCS900_DMAD2, "DMAD0", m_dmad[2].d ).formatstr("%08X");
	state_add( TLCS900_DMAC2, "DMAC0", m_dmac[2].w.l ).formatstr("%04X");
	state_add( TLCS900_DMAM2, "DMAM0", m_dmam[2].b.l ).formatstr("%02X");
	state_add( TLCS900_DMAS3, "DMAS0", m_dmas[3].d ).formatstr("%08X");
	state_add( TLCS900_DMAD3, "DMAD0", m_dmad[3].d ).formatstr("%08X");
	state_add( TLCS900_DMAC3, "DMAC0", m_dmac[3].w.l ).formatstr("%04X");
	state_add( TLCS900_DMAM3, "DMAM0", m_dmam[3].b.l ).formatstr("%02X");

	state_add( STATE_GENPC, "GENPC", m_pc.d ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_sr.w.l ).formatstr("%12s").noshow();

	m_icountptr = &m_icount;
}


void tlcs900h_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%d%c%d%c%c%c%c%c%c%c%c",
					m_sr.w.l & 0x8000 ? 'S' : 'U',
					( m_sr.w.l & 0x7000 ) >> 12,
					m_sr.w.l & 0x0800 ? 'M' : 'N',
					( m_sr.w.l & 0x0700 ) >> 8,
					m_sr.w.l & 0x0080 ? 'S' : '.',
					m_sr.w.l & 0x0040 ? 'Z' : '.',
					m_sr.w.l & 0x0020 ? '1' : '.',
					m_sr.w.l & 0x0010 ? 'H' : '.',
					m_sr.w.l & 0x0008 ? '1' : '.',
					m_sr.w.l & 0x0004 ? 'V' : '.',
					m_sr.w.l & 0x0002 ? 'N' : '.',
					m_sr.w.l & 0x0001 ? 'C' : '.' );
			break;
	}
}


void tlcs900h_device::device_reset()
{
	int i;

	m_pc.b.l = RDMEM( 0xFFFF00 );
	m_pc.b.h = RDMEM( 0xFFFF01 );
	m_pc.b.h2 = RDMEM( 0xFFFF02 );
	m_pc.b.h3 = 0;
	/* system mode, iff set to 111, max mode, register bank 0 */
	m_sr.d = 0xF800;
	m_regbank = 0;
	m_xssp.d = 0x0100;
	m_halted = 0;
	m_check_irqs = 0;
	m_ad_cycles_left = 0;
	m_nmi_state = CLEAR_LINE;
	m_timer_pre = 0;
	m_timer_change[0] = 0;
	m_timer_change[1] = 0;
	m_timer_change[2] = 0;
	m_timer_change[3] = 0;

	m_reg[TLCS900_P1] = 0x00;
	m_reg[TLCS900_P1CR] = 0x00;
	m_reg[TLCS900_P2] = 0xff;
	m_reg[TLCS900_P2FC] = 0x00;
	m_reg[TLCS900_P5] = 0x3d;
	m_reg[TLCS900_P5CR] = 0x00;
	m_reg[TLCS900_P5FC] = 0x00;
	m_reg[TLCS900_P6] = 0x3b;
	m_reg[TLCS900_P6FC] = 0x00;
	m_reg[TLCS900_P7] = 0xff;
	m_reg[TLCS900_P7CR] = 0x00;
	m_reg[TLCS900_P7FC] = 0x00;
	m_reg[TLCS900_P8] = 0x3f;
	m_reg[TLCS900_P8CR] = 0x00;
	m_reg[TLCS900_P8FC] = 0x00;
	m_reg[TLCS900_PA] = 0x0f;
	m_reg[TLCS900_PACR] = 0x00;
	m_reg[TLCS900_PAFC] = 0x00;
	m_reg[TLCS900_PB] = 0xff;
	m_reg[TLCS900_PBCR] = 0x00;
	m_reg[TLCS900_PBFC] = 0x00;
	m_reg[TLCS900_MSAR0] = 0xff;
	m_reg[TLCS900_MSAR1] = 0xff;
	m_reg[TLCS900_MSAR2] = 0xff;
	m_reg[TLCS900_MSAR3] = 0xff;
	m_reg[TLCS900_MAMR0] = 0xff;
	m_reg[TLCS900_MAMR1] = 0xff;
	m_reg[TLCS900_MAMR2] = 0xff;
	m_reg[TLCS900_MAMR3] = 0xff;
	m_reg[TLCS900_DREFCR] = 0x00;
	m_reg[TLCS900_DMEMCR] = 0x80;
	m_reg[TLCS900_T01MOD] = 0x00;
	m_reg[TLCS900_T23MOD] = 0x00;
	m_reg[TLCS900_TFFCR] = 0x00;
	m_reg[TLCS900_TRUN] = 0x00;
	m_reg[TLCS900_TRDC] = 0x00;
	m_reg[TLCS900_T4MOD] = 0x20;
	m_reg[TLCS900_T4FFCR] = 0x00;
	m_reg[TLCS900_T5MOD] = 0x20;
	m_reg[TLCS900_T5FFCR] = 0x00;
	m_reg[TLCS900_T45CR] = 0x00;
	m_reg[TLCS900_PG01CR] = 0x00;
	m_reg[TLCS900_PG0REG] = 0x00;
	m_reg[TLCS900_PG1REG] = 0x00;
	m_reg[TLCS900_SC0MOD] = 0x00;
	m_reg[TLCS900_SC0CR] = 0x00;
	m_reg[TLCS900_BR0CR] = 0x00;
	m_reg[TLCS900_SC1MOD] = 0x00;
	m_reg[TLCS900_SC1CR] = 0x00;
	m_reg[TLCS900_BR1CR] = 0x00;
	m_reg[TLCS900_P8FC] = 0x00;
	m_reg[TLCS900_ODE] = 0x00;
	m_reg[TLCS900_ADMOD] = 0x00;
	m_reg[TLCS900_ADREG0L] = 0x3f;
	m_reg[TLCS900_ADREG1L] = 0x3f;
	m_reg[TLCS900_ADREG2L] = 0x3f;
	m_reg[TLCS900_ADREG3L] = 0x3f;
	m_reg[TLCS900_WDMOD] = 0x80;

	for ( i = 0; i < TLCS900_NUM_INPUTS; i++ )
	{
		m_level[i] = CLEAR_LINE;
	}
	m_prefetch_clear = true;
}


#include "900tbl.c"


#define TLCS900_NUM_MASKABLE_IRQS   22
static const struct {
	UINT8 reg;
	UINT8 iff;
	UINT8 vector;
} tlcs900_irq_vector_map[TLCS900_NUM_MASKABLE_IRQS] =
{
	{ TLCS900_INTETC32, 0x80, 0x80 },   /* INTTC3 */
	{ TLCS900_INTETC32, 0x08, 0x7c },   /* INTTC2 */
	{ TLCS900_INTETC10, 0x80, 0x78 },   /* INTTC1 */
	{ TLCS900_INTETC10, 0x08, 0x74 },   /* INTTC0 */
	{ TLCS900_INTE0AD, 0x80, 0x70 },    /* INTAD */
	{ TLCS900_INTES1, 0x80, 0x6c },     /* INTTX1 */
	{ TLCS900_INTES1, 0x08, 0x68 },     /* INTRX1 */
	{ TLCS900_INTES0, 0x80, 0x64 },     /* INTTX0 */
	{ TLCS900_INTES0, 0x08, 0x60 },     /* INTRX0 */
	{ TLCS900_INTET76, 0x80, 0x5c },    /* INTTR7 */
	{ TLCS900_INTET76, 0x08, 0x58 },    /* INTTR6 */
	{ TLCS900_INTET54, 0x80, 0x54 },    /* INTTR5 */
	{ TLCS900_INTET54, 0x08, 0x50 },    /* INTTR4 */
	{ TLCS900_INTET32, 0x80, 0x4c },    /* INTT3 */
	{ TLCS900_INTET32, 0x08, 0x48 },    /* INTT2 */
	{ TLCS900_INTET10, 0x80, 0x44 },    /* INTT1 */
	{ TLCS900_INTET10, 0x08, 0x40 },    /* INTT0 */
								/* 0x3c - reserved */
	{ TLCS900_INTE67, 0x80, 0x38 },     /* INT7 */
	{ TLCS900_INTE67, 0x08, 0x34 },     /* INT6 */
	{ TLCS900_INTE45, 0x80, 0x30 },     /* INT5 */
	{ TLCS900_INTE45, 0x08, 0x2c },     /* INT4 */
	{ TLCS900_INTE0AD, 0x08, 0x28 }     /* INT0 */
};


int tlcs900h_device::tlcs900_process_hdma( int channel )
{
	UINT8 vector = ( m_reg[0x7c + channel] & 0x1f ) << 2;

	/* Check if any HDMA actions should be performed */
	if ( vector >= 0x28 && vector != 0x3C && vector < 0x74 )
	{
		int irq = 0;

		while( irq < TLCS900_NUM_MASKABLE_IRQS && tlcs900_irq_vector_map[irq].vector != vector )
			irq++;

		/* Check if our interrupt flip-flop is set */
		if ( irq < TLCS900_NUM_MASKABLE_IRQS && m_reg[tlcs900_irq_vector_map[irq].reg] & tlcs900_irq_vector_map[irq].iff )
		{
			switch( m_dmam[channel].b.l & 0x1f )
			{
			case 0x00:
				WRMEM( m_dmad[channel].d, RDMEM( m_dmas[channel].d ) );
				m_dmad[channel].d += 1;
				m_cycles += 8;
				break;
			case 0x01:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmad[channel].d += 2;
				m_cycles += 8;
				break;
			case 0x02:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmad[channel].d += 4;
				m_cycles += 12;
				break;
			case 0x04:
				WRMEM( m_dmad[channel].d, RDMEM( m_dmas[channel].d ) );
				m_dmad[channel].d -= 1;
				m_cycles += 8;
				break;
			case 0x05:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmad[channel].d -= 2;
				m_cycles += 8;
				break;
			case 0x06:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmad[channel].d -= 4;
				m_cycles += 12;
				break;
			case 0x08:
				WRMEM( m_dmad[channel].d, RDMEM( m_dmas[channel].d ) );
				m_dmas[channel].d += 1;
				m_cycles += 8;
				break;
			case 0x09:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmas[channel].d += 2;
				m_cycles += 8;
				break;
			case 0x0a:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmas[channel].d += 4;
				m_cycles += 12;
				break;
			case 0x0c:
				WRMEM( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmas[channel].d -= 1;
				m_cycles += 8;
				break;
			case 0x0d:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_dmas[channel].d -= 2;
				m_cycles += 8;
				break;
			case 0x0e:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_dmas[channel].d -= 4;
				m_cycles += 12;
				break;
			case 0x10:
				WRMEM( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_cycles += 8;
				break;
			case 0x11:
				WRMEMW( m_dmad[channel].d, RDMEMW( m_dmas[channel].d ) );
				m_cycles += 8;
				break;
			case 0x12:
				WRMEML( m_dmad[channel].d, RDMEML( m_dmas[channel].d ) );
				m_cycles += 12;
				break;
			case 0x14:
				m_dmas[channel].d += 1;
				m_cycles += 5;
				break;
			}

			m_dmac[channel].w.l -= 1;

			if ( m_dmac[channel].w.l == 0 )
			{
				m_reg[0x7c + channel] = 0;
				switch( channel )
				{
				case 0:
					m_reg[TLCS900_INTETC10] |= 0x08;
					break;
				case 1:
					m_reg[TLCS900_INTETC10] |= 0x80;
					break;
				case 2:
					m_reg[TLCS900_INTETC32] |= 0x08;
					break;
				case 3:
					m_reg[TLCS900_INTETC32] |= 0x80;
					break;
				}
			}

			/* Clear the interrupt flip-flop */
			m_reg[tlcs900_irq_vector_map[irq].reg] &= ~tlcs900_irq_vector_map[irq].iff;

			return 1;
		}
	}
	return 0;
}


void tlcs900h_device::tlcs900_check_hdma()
{
	/* HDMA can only be performed if interrupts are allowed */
	if ( ( m_sr.b.h & 0x70 ) != 0x70 )
	{
		if ( ! tlcs900_process_hdma( 0 ) )
		{
			if ( ! tlcs900_process_hdma( 1 ) )
			{
				if ( ! tlcs900_process_hdma( 2 ) )
				{
					tlcs900_process_hdma( 3 );
				}
			}
		}
	}
}


void tlcs900h_device::tlcs900_check_irqs()
{
	int irq_vectors[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int level = 0;
	int irq = -1;
	int i;

	/* Check for NMI */
	if ( m_nmi_state == ASSERT_LINE )
	{
		m_xssp.d -= 4;
		WRMEML( m_xssp.d, m_pc.d );
		m_xssp.d -= 2;
		WRMEMW( m_xssp.d, m_sr.w.l );
		m_pc.d = RDMEML( 0xffff00 + 0x20 );
		m_cycles += 18;
		m_prefetch_clear = true;

		m_halted = 0;

		m_nmi_state = CLEAR_LINE;

		return;
	}

	/* Check regular irqs */
	for( i = 0; i < TLCS900_NUM_MASKABLE_IRQS; i++ )
	{
		if ( m_reg[tlcs900_irq_vector_map[i].reg] & tlcs900_irq_vector_map[i].iff )
		{
			switch( tlcs900_irq_vector_map[i].iff )
			{
			case 0x80:
				irq_vectors[ ( m_reg[ tlcs900_irq_vector_map[i].reg ] >> 4 ) & 0x07 ] = i;
				break;
			case 0x08:
				irq_vectors[ m_reg[ tlcs900_irq_vector_map[i].reg ] & 0x07 ] = i;
				break;
			}
		}
	}

	/* Check highest allowed priority irq */
	for ( i = MAX( 1, ( ( m_sr.b.h & 0x70 ) >> 4 ) ); i < 7; i++ )
	{
		if ( irq_vectors[i] >= 0 )
		{
			irq = irq_vectors[i];
			level = i + 1;
		}
	}

	/* Take irq */
	if ( irq >= 0 )
	{
		UINT8 vector = tlcs900_irq_vector_map[irq].vector;

		m_xssp.d -= 4;
		WRMEML( m_xssp.d, m_pc.d );
		m_xssp.d -= 2;
		WRMEMW( m_xssp.d, m_sr.w.l );

		/* Mask off any lower priority interrupts  */
		m_sr.b.h = ( m_sr.b.h & 0x8f ) | ( level << 4 );

		m_pc.d = RDMEML( 0xffff00 + vector );
		m_cycles += 18;
		m_prefetch_clear = true;

		m_halted = 0;

		/* Clear taken IRQ */
		m_reg[ tlcs900_irq_vector_map[irq].reg ] &= ~ tlcs900_irq_vector_map[irq].iff;
	}
}


void tlcs900h_device::tlcs900_handle_ad()
{
	if ( m_ad_cycles_left > 0 )
	{
		m_ad_cycles_left -= m_cycles;
		if ( m_ad_cycles_left <= 0 )
		{
			/* Store A/D converted value */
			switch( m_reg[TLCS900_ADMOD] & 0x03 )
			{
			case 0x00:  /* AN0 */
				m_reg[TLCS900_ADREG0L] |= 0xc0;
				m_reg[TLCS900_ADREG0H] = 0xff;
				break;
			case 0x01:  /* AN1 */
			case 0x02:  /* AN2 */
			case 0x03:  /* AN3 */
				break;
			}

			/* Clear BUSY flag, set END flag */
			m_reg[TLCS900_ADMOD] &= ~ 0x40;
			m_reg[TLCS900_ADMOD] |= 0x80;

			m_reg[TLCS900_INTE0AD] |= 0x80;
			m_check_irqs = 1;
		}
	}
}


enum ff_change
{
	FF_CLEAR,
	FF_SET,
	FF_INVERT
};


void tlcs900h_device::tlcs900_change_tff( int which, int change )
{
	switch( which )
	{
	case 1:
		switch( change )
		{
		case FF_CLEAR:
			m_tff1 = 0;
			break;
		case FF_SET:
			m_tff1 = 1;
			break;
		case FF_INVERT:
			m_tff1 ^= 1;
			break;
		}
		m_to1( m_tff1 );
		break;

	case 3:
		switch( change )
		{
		case FF_CLEAR:
			m_tff3 = 0;
			break;
		case FF_SET:
			m_tff3 = 1;
			break;
		case FF_INVERT:
			m_tff3 ^= 1;
			break;
		}
		m_to3( m_tff3 );
		break;
	}
}


void tlcs900h_device::tlcs900_handle_timers()
{
	UINT32  old_pre = m_timer_pre;

	/* Is the pre-scaler active */
	if ( m_reg[TLCS900_TRUN] & 0x80 )
		m_timer_pre += m_cycles;

	/* Timer 0 */
	if ( m_reg[TLCS900_TRUN] & 0x01 )
	{
		switch( m_reg[TLCS900_T01MOD] & 0x03 )
		{
		case 0x00:  /* TIO */
			break;
		case 0x01:  /* T1 */
			m_timer_change[0] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T4 */
			m_timer_change[0] += ( m_timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:  /* T16 */
			m_timer_change[0] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; m_timer_change[0] > 0; m_timer_change[0]-- )
		{
//printf("timer0 = %02x, TREG0 = %02x\n", m_timer[0], m_reg[TREG0] );
			m_timer[0] += 1;
			if ( m_timer[0] == m_reg[TLCS900_TREG0] )
			{
				if ( ( m_reg[TLCS900_T01MOD] & 0x0c ) == 0x00 )
				{
					m_timer_change[1] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_reg[TLCS900_T01MOD] & 0xc0 ) != 0x40 )
				{
					m_timer[0] = 0;
					m_reg[TLCS900_INTET10] |= 0x08;
				}
			}
		}
	}

	/* Timer 1 */
	if ( m_reg[TLCS900_TRUN] & 0x02 )
	{
		switch( ( m_reg[TLCS900_T01MOD] >> 2 ) & 0x03 )
		{
		case 0x00:  /* TO0TRG */
			break;
		case 0x01:  /* T1 */
			m_timer_change[1] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T16 */
			m_timer_change[1] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:  /* T256 */
			m_timer_change[1] += ( m_timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; m_timer_change[1] > 0; m_timer_change[1]-- )
		{
			m_timer[1] += 1;
			if ( m_timer[1] == m_reg[TLCS900_TREG1] )
			{
				m_timer[1] = 0;
				m_reg[TLCS900_INTET10] |= 0x80;

				if ( m_reg[TLCS900_TFFCR] & 0x02 )
				{
					tlcs900_change_tff( 1, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 0 */
				if ( ( m_reg[TLCS900_T01MOD] & 0xc0 ) == 0x40 )
				{
					m_timer[0] = 0;
				}
			}
		}
	}

	/* Timer 2 */
	if ( m_reg[TLCS900_TRUN] & 0x04 )
	{
		switch( m_reg[TLCS900_T23MOD] & 0x03 )
		{
		case 0x00:  /* invalid */
		case 0x01:  /* T1 */
			m_timer_change[2] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T4 */
			m_timer_change[2] += ( m_timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:  /* T16 */
			m_timer_change[2] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; m_timer_change[2] > 0; m_timer_change[2]-- )
		{
			m_timer[2] += 1;
			if ( m_timer[2] == m_reg[TLCS900_TREG2] )
			{
				if ( ( m_reg[TLCS900_T23MOD] & 0x0c ) == 0x00 )
				{
					m_timer_change[3] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_reg[TLCS900_T23MOD] & 0xc0 ) != 0x40 )
				{
					m_timer[2] = 0;
					m_reg[TLCS900_INTET32] |= 0x08;
				}
			}
		}
	}

	/* Timer 3 */
	if ( m_reg[TLCS900_TRUN] & 0x08 )
	{
		switch( ( m_reg[TLCS900_T23MOD] >> 2 ) & 0x03 )
		{
		case 0x00:  /* TO2TRG */
			break;
		case 0x01:  /* T1 */
			m_timer_change[3] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T16 */
			m_timer_change[3] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:  /* T256 */
			m_timer_change[3] += ( m_timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; m_timer_change[3] > 0; m_timer_change[3]-- )
		{
			m_timer[3] += 1;
			if ( m_timer[3] == m_reg[TLCS900_TREG3] )
			{
				m_timer[3] = 0;
				m_reg[TLCS900_INTET32] |= 0x80;

				if ( m_reg[TLCS900_TFFCR] & 0x20 )
				{
					tlcs900_change_tff( 3, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 2 */
				if ( ( m_reg[TLCS900_T23MOD] & 0xc0 ) == 0x40 )
				{
					m_timer[2] = 0;
				}
			}
		}
	}

	m_timer_pre &= 0xffffff;
}


void tlcs900h_device::execute_run()
{
	do
	{
		const tlcs900inst *inst;

		m_cycles = 0;

		if ( m_check_irqs )
		{
			tlcs900_check_irqs();
			m_check_irqs = 0;
		}

		debugger_instruction_hook( this, m_pc.d );

		if ( m_halted )
		{
			m_cycles += 8;
		}
		else
		{
			m_op = RDOP();
			inst = &s_mnemonic[m_op];
			prepare_operands( inst );

			/* Execute the instruction */
			(this->*inst->opfunc)();
			m_cycles += inst->cycles;
		}

		tlcs900_handle_ad();

		tlcs900_handle_timers();

		tlcs900_check_hdma();

		m_icount -= m_cycles;
	} while ( m_icount > 0 );
}


void tlcs900h_device::execute_set_input(int input, int level)
{
	switch( input )
	{
	case INPUT_LINE_NMI:
	case TLCS900_NMI:
		if ( m_level[TLCS900_NMI] == CLEAR_LINE && level == ASSERT_LINE )
		{
			m_nmi_state = level;
		}
		m_level[TLCS900_NMI] = level;
		break;

	case TLCS900_INTWD:
		break;

	case TLCS900_INT0:
		/* Is INT0 functionality enabled? */
		if ( m_reg[TLCS900_IIMC] & 0x04 )
		{
			if ( m_reg[TLCS900_IIMC] & 0x02 )
			{
				/* Rising edge detect */
				if ( m_level[TLCS900_INT0] == CLEAR_LINE && level == ASSERT_LINE )
				{
					/* Leave HALT state */
					m_halted = 0;
					m_reg[TLCS900_INTE0AD] |= 0x08;
				}
			}
			else
			{
				/* Level detect */
				if ( level == ASSERT_LINE )
					m_reg[TLCS900_INTE0AD] |= 0x08;
				else
					m_reg[TLCS900_INTE0AD] &= ~ 0x08;
			}
		}
		m_level[TLCS900_INT0] = level;
		break;

	case TLCS900_INT4:
		if ( ! ( m_reg[TLCS900_PBCR] & 0x01 ) )
		{
			if ( m_level[TLCS900_INT4] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_reg[TLCS900_INTE45] |= 0x08;
			}
		}
		m_level[TLCS900_INT4] = level;
		break;

	case TLCS900_INT5:
		if ( ! ( m_reg[TLCS900_PBCR] & 0x02 ) )
		{
			if ( m_level[TLCS900_INT5] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_reg[TLCS900_INTE45] |= 0x80;
			}
		}
		m_level[TLCS900_INT5] = level;
		break;

	case TLCS900_TIO:   /* External timer input for timer 0 */
		if ( ( m_reg[TLCS900_TRUN] & 0x01 ) && ( m_reg[TLCS900_T01MOD] & 0x03 ) == 0x00 )
		{
			if ( m_level[TLCS900_TIO] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_timer_change[0] += 1;
			}
		}
		m_level[TLCS900_TIO] = level;
		break;
	}
	m_check_irqs = 1;
}


READ8_MEMBER( tlcs900h_device::tlcs900_internal_r )
{
	return m_reg[ offset ];
}


WRITE8_MEMBER( tlcs900h_device::tlcs900_internal_w )
{
	switch ( offset )
	{
	case TLCS900_TRUN:
		if ( ! ( data & 0x01 ) )
		{
			m_timer[0] = 0;
			m_timer_change[0] = 0;
		}
		if ( ! ( data & 0x02 ) )
		{
			m_timer[1] = 0;
			m_timer_change[1] = 0;
		}
		if ( ! ( data & 0x04 ) )
		{
			m_timer[2] = 0;
			m_timer_change[2] = 0;
		}
		if ( ! ( data & 0x08 ) )
		{
			m_timer[3] = 0;
			m_timer_change[3] = 0;
		}
		if ( ! ( data & 0x10 ) )
			m_timer[4] = 0;
		if ( ! ( data & 0x20 ) )
			m_timer[5] = 0;
		break;

	case TLCS900_TFFCR:
		switch( data & 0x0c )
		{
		case 0x00:
			tlcs900_change_tff( 1, FF_INVERT );
			break;
		case 0x04:
			tlcs900_change_tff( 1, FF_SET );
			break;
		case 0x08:
			tlcs900_change_tff( 1, FF_CLEAR );
			break;
		}
		switch( data & 0xc0 )
		{
		case 0x00:
			tlcs900_change_tff( 3, FF_INVERT );
			break;
		case 0x40:
			tlcs900_change_tff( 3, FF_SET );
			break;
		case 0x80:
			tlcs900_change_tff( 3, FF_CLEAR );
			break;
		}
		break;
	case TLCS900_MSAR0:
	case TLCS900_MAMR0:
	case TLCS900_MSAR1:
	case TLCS900_MAMR1:
		break;

	case TLCS900_SC0BUF:
		// Fake finish sending data
		m_reg[TLCS900_INTES0] |= 0x80;
		break;

	case TLCS900_ADMOD:
		/* Preserve read-only bits */
		data = ( m_reg[TLCS900_ADMOD] & 0xc0 ) | ( data & 0x3f );

		/* Check for A/D request start */
		if ( data & 0x04 )
		{
			data &= ~0x04;
			data |= 0x40;
			m_ad_cycles_left = ( data & 0x08 ) ? 640 : 320;
		}
		break;

	case TLCS900_WDMOD:
	case TLCS900_WDCR:
		break;

	case TLCS900_INTE0AD:
	case TLCS900_INTE45:
	case TLCS900_INTE67:
	case TLCS900_INTET10:
	case TLCS900_INTET32:
	case TLCS900_INTET54:
	case TLCS900_INTET76:
	case TLCS900_INTES0:
	case TLCS900_INTES1:
	case TLCS900_INTETC10:
	case TLCS900_INTETC32:
		if ( data & 0x80 )
			data = ( data & 0x7f ) | ( m_reg[offset] & 0x80 );
		if ( data & 0x08 )
			data = ( data & 0xf7 ) | ( m_reg[offset] & 0x08 );
		break;

	case TLCS900_IIMC:
		break;

	default:
		break;
	}

	m_check_irqs = 1;
	m_reg[ offset ] = data;
}



// Toshiba TMP95C063

/* TMP95C063 Internal register defines */

#define TMP95C063_P1        0x01
#define TMP95C063_P1CR      0x04
#define TMP95C063_P2        0x06
#define TMP95C063_P2FC      0x09
#define TMP95C063_P5        0x0d
#define TMP95C063_P5CR      0x10
#define TMP95C063_P5FC      0x11
#define TMP95C063_P6        0x12
#define TMP95C063_P7        0x13
#define TMP95C063_P6FC      0x15
#define TMP95C063_P7CR      0x16
#define TMP95C063_P7FC      0x17
#define TMP95C063_P8        0x18
#define TMP95C063_P9        0x19
#define TMP95C063_P8CR      0x1a
#define TMP95C063_P8FC      0x1b
#define TMP95C063_P9CR      0x1c
#define TMP95C063_P9FC      0x1d
#define TMP95C063_PA        0x1e
#define TMP95C063_PB        0x1f
#define TMP95C063_T8RUN     0x20
#define TMP95C063_TRDC      0x21
#define TMP95C063_TREG0     0x22
#define TMP95C063_TREG1     0x23
#define TMP95C063_T01MOD    0x24
#define TMP95C063_T02FFCR   0x25
#define TMP95C063_TREG2     0x26
#define TMP95C063_TREG3     0x27
#define TMP95C063_T23MOD    0x28
#define TMP95C063_TREG4     0x29
#define TMP95C063_TREG5     0x2a
#define TMP95C063_T45MOD    0x2b
#define TMP95C063_T46FFCR   0x2c
#define TMP95C063_TREG6     0x2d
#define TMP95C063_TREG7     0x2e
#define TMP95C063_T67MOD    0x2f
#define TMP95C063_TREG8L    0x30
#define TMP95C063_TREG8H    0x31
#define TMP95C063_TREG9L    0x32
#define TMP95C063_TREG9H    0x33
#define TMP95C063_CAP1L     0x34
#define TMP95C063_CAP1H     0x35
#define TMP95C063_CAP2L     0x36
#define TMP95C063_CAP2H     0x37
#define TMP95C063_T8MOD     0x38
#define TMP95C063_T8FFCR    0x39
#define TMP95C063_T89CR     0x3a
#define TMP95C063_T16RUN    0x3b
#define TMP95C063_TREGAL    0x40
#define TMP95C063_TREGAH    0x41
#define TMP95C063_TREGBL    0x42
#define TMP95C063_TREGBH    0x43
#define TMP95C063_CAP3L     0x44
#define TMP95C063_CAP3H     0x45
#define TMP95C063_CAP4L     0x46
#define TMP95C063_CAP4H     0x47
#define TMP95C063_T9MOD     0x48
#define TMP95C063_T9FFCR    0x49
#define TMP95C063_DAREG0    0x4a
#define TMP95C063_DAREG1    0x4b
#define TMP95C063_PG0REG    0x4c
#define TMP95C063_PG1REG    0x4d
#define TMP95C063_PG01CR    0x4e
#define TMP95C063_DADRV     0x4f
#define TMP95C063_SC0BUF    0x50
#define TMP95C063_SC0CR     0x51
#define TMP95C063_SC0MOD    0x52
#define TMP95C063_BR0CR     0x53
#define TMP95C063_SC1BUF    0x54
#define TMP95C063_SC1CR     0x55
#define TMP95C063_SC1MOD    0x56
#define TMP95C063_BR1CR     0x57
#define TMP95C063_ODE       0x58
#define TMP95C063_DMA0V     0x5a
#define TMP95C063_DMA1V     0x5b
#define TMP95C063_DMA2V     0x5c
#define TMP95C063_DMA3V     0x5d
#define TMP95C063_ADMOD1    0x5e
#define TMP95C063_ADMOD2    0x5f
#define TMP95C063_ADREG04L  0x60
#define TMP95C063_ADREG04H  0x61
#define TMP95C063_ADREG15L  0x62
#define TMP95C063_ADREG15H  0x63
#define TMP95C063_ADREG26L  0x64
#define TMP95C063_ADREG26H  0x65
#define TMP95C063_ADREG37L  0x66
#define TMP95C063_ADREG37H  0x67
#define TMP95C063_SDMACR0   0x6a
#define TMP95C063_SDMACR1   0x6b
#define TMP95C063_SDMACR2   0x6c
#define TMP95C063_SDMACR3   0x6d
#define TMP95C063_WDMOD     0x6e
#define TMP95C063_WDCR      0x6f
#define TMP95C063_INTE0AD   0x70
#define TMP95C063_INTE12    0x71
#define TMP95C063_INTE34    0x72
#define TMP95C063_INTE56    0x73
#define TMP95C063_INTE78    0x74
#define TMP95C063_INTET01   0x75
#define TMP95C063_INTET23   0x76
#define TMP95C063_INTET45   0x77
#define TMP95C063_INTET67   0x78
#define TMP95C063_INTET89   0x79
#define TMP95C063_INTETAB   0x7a
#define TMP95C063_INTES0    0x7b
#define TMP95C063_INTES1    0x7c
#define TMP95C063_INTETC01  0x7d
#define TMP95C063_INTETC23  0x7e
#define TMP95C063_IIMC      0x7f
#define TMP95C063_PACR      0x80
#define TMP95C063_PAFC      0x81
#define TMP95C063_PBCR      0x82
#define TMP95C063_PBFC      0x83
#define TMP95C063_PC        0x84
#define TMP95C063_PD        0x85
#define TMP95C063_PDCR      0x88
#define TMP95C063_PE        0x8a
#define TMP95C063_PECR      0x8c
#define TMP95C063_BEXCS     0x8f
#define TMP95C063_B0CS      0x90
#define TMP95C063_B1CS      0x91
#define TMP95C063_B2CS      0x92
#define TMP95C063_B3CS      0x93
#define TMP95C063_MSAR0     0x94
#define TMP95C063_MAMR0     0x95
#define TMP95C063_MSAR1     0x96
#define TMP95C063_MAMR1     0x97
#define TMP95C063_MSAR2     0x98
#define TMP95C063_MAMR2     0x99
#define TMP95C063_MSAR3     0x9a
#define TMP95C063_MAMR3     0x9b
#define TMP95C063_DREFCR1   0x9c
#define TMP95C063_DMEMCR1   0x9d
#define TMP95C063_DREFCR3   0x9e
#define TMP95C063_DMEMCR3   0x9f


#define TMP95C063_NUM_MASKABLE_IRQS 30
static const struct {
	UINT8 reg;
	UINT8 iff;
	UINT8 vector;
} tmp95c063_irq_vector_map[TMP95C063_NUM_MASKABLE_IRQS] =
{
	{ TMP95C063_INTETC23, 0x80, 0xa0 },     /* INTTC3 */
	{ TMP95C063_INTETC23, 0x08, 0x9c },     /* INTTC2 */
	{ TMP95C063_INTETC01, 0x80, 0x98 },     /* INTTC1 */
	{ TMP95C063_INTETC01, 0x08, 0x94 },     /* INTTC0 */
	{ TMP95C063_INTE0AD, 0x80, 0x90 },      /* INTAD */
	{ TMP95C063_INTES1, 0x80, 0x8c },       /* INTTX1 */
	{ TMP95C063_INTES1, 0x08, 0x88 },       /* INTRX1 */
	{ TMP95C063_INTES0, 0x80, 0x84 },       /* INTTX0 */
	{ TMP95C063_INTES0, 0x08, 0x80 },       /* INTRX0 */
	{ TMP95C063_INTETAB, 0x80, 0x7c },      /* INTTRB */
	{ TMP95C063_INTETAB, 0x08, 0x78 },      /* INTTRA */
	{ TMP95C063_INTET89, 0x80, 0x74 },      /* INTTR9 */
	{ TMP95C063_INTET89, 0x80, 0x70 },      /* INTTR8 */
	{ TMP95C063_INTET67, 0x80, 0x6c },      /* INTT7 */
	{ TMP95C063_INTET67, 0x08, 0x68 },      /* INTT6 */
	{ TMP95C063_INTET45, 0x80, 0x64 },      /* INTT5 */
	{ TMP95C063_INTET45, 0x08, 0x60 },      /* INTT4 */
	{ TMP95C063_INTET23, 0x80, 0x5c },      /* INTT3 */
	{ TMP95C063_INTET23, 0x08, 0x58 },      /* INTT2 */
	{ TMP95C063_INTET01, 0x80, 0x54 },      /* INTT1 */
	{ TMP95C063_INTET01, 0x08, 0x50 },      /* INTT0 */
	{ TMP95C063_INTE78, 0x80, 0x4c },       /* INT8 */
	{ TMP95C063_INTE78, 0x08, 0x48 },       /* INT7 */
	{ TMP95C063_INTE56, 0x80, 0x44 },       /* INT6 */
	{ TMP95C063_INTE56, 0x08, 0x40 },       /* INT5 */
								/* 0x3c - reserved */
	{ TMP95C063_INTE34, 0x80, 0x38 },       /* INT4 */
	{ TMP95C063_INTE34, 0x08, 0x34 },       /* INT3 */
	{ TMP95C063_INTE12, 0x80, 0x30 },       /* INT2 */
	{ TMP95C063_INTE12, 0x08, 0x2c },       /* INT1 */
	{ TMP95C063_INTE0AD, 0x08, 0x28 }       /* INT0 */
};

void tmp95c063_device::tlcs900_handle_timers()
{
	// TODO: implement timers 4-7

	UINT32  old_pre = m_timer_pre;

	/* Is the pre-scaler active */
	if ( m_reg[TMP95C063_T8RUN] & 0x80 )
		m_timer_pre += m_cycles;

	/* Timer 0 */
	if ( m_reg[TMP95C063_T8RUN] & 0x01 )
	{
		switch( m_reg[TMP95C063_T01MOD] & 0x03 )
		{
		case 0x00:  /* TIO */
			break;
		case 0x01:  /* T1 */
			m_timer_change[0] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T4 */
			m_timer_change[0] += ( m_timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:  /* T16 */
			m_timer_change[0] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; m_timer_change[0] > 0; m_timer_change[0]-- )
		{
//printf("timer0 = %02x, TREG0 = %02x\n", m_timer[0], m_reg[TREG0] );
			m_timer[0] += 1;
			if ( m_timer[0] == m_reg[TMP95C063_TREG0] )
			{
				if ( ( m_reg[TMP95C063_T01MOD] & 0x0c ) == 0x00 )
				{
					m_timer_change[1] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_reg[TMP95C063_T01MOD] & 0xc0 ) != 0x40 )
				{
					m_timer[0] = 0;
					m_reg[TMP95C063_INTET01] |= 0x08;
				}
			}
		}
	}

	/* Timer 1 */
	if ( m_reg[TMP95C063_T8RUN] & 0x02 )
	{
		switch( ( m_reg[TMP95C063_T01MOD] >> 2 ) & 0x03 )
		{
		case 0x00:  /* TO0TRG */
			break;
		case 0x01:  /* T1 */
			m_timer_change[1] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T16 */
			m_timer_change[1] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:  /* T256 */
			m_timer_change[1] += ( m_timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; m_timer_change[1] > 0; m_timer_change[1]-- )
		{
			m_timer[1] += 1;
			if ( m_timer[1] == m_reg[TMP95C063_TREG1] )
			{
				m_timer[1] = 0;
				m_reg[TMP95C063_INTET01] |= 0x80;

				if ( m_reg[TMP95C063_T02FFCR] & 0x02 )
				{
					tlcs900_change_tff( 1, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 0 */
				if ( ( m_reg[TMP95C063_T01MOD] & 0xc0 ) == 0x40 )
				{
					m_timer[0] = 0;
				}
			}
		}
	}

	/* Timer 2 */
	if ( m_reg[TMP95C063_T8RUN] & 0x04 )
	{
		switch( m_reg[TMP95C063_T23MOD] & 0x03 )
		{
		case 0x00:  /* invalid */
		case 0x01:  /* T1 */
			m_timer_change[2] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T4 */
			m_timer_change[2] += ( m_timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:  /* T16 */
			m_timer_change[2] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; m_timer_change[2] > 0; m_timer_change[2]-- )
		{
			m_timer[2] += 1;
			if ( m_timer[2] == m_reg[TMP95C063_TREG2] )
			{
				if ( ( m_reg[TMP95C063_T23MOD] & 0x0c ) == 0x00 )
				{
					m_timer_change[3] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( m_reg[TMP95C063_T23MOD] & 0xc0 ) != 0x40 )
				{
					m_timer[2] = 0;
					m_reg[TMP95C063_INTET23] |= 0x08;
				}
			}
		}
	}

	/* Timer 3 */
	if ( m_reg[TMP95C063_T8RUN] & 0x08 )
	{
		switch( ( m_reg[TMP95C063_T23MOD] >> 2 ) & 0x03 )
		{
		case 0x00:  /* TO2TRG */
			break;
		case 0x01:  /* T1 */
			m_timer_change[3] += ( m_timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:  /* T16 */
			m_timer_change[3] += ( m_timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:  /* T256 */
			m_timer_change[3] += ( m_timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; m_timer_change[3] > 0; m_timer_change[3]-- )
		{
			m_timer[3] += 1;
			if ( m_timer[3] == m_reg[TLCS900_TREG3] )
			{
				m_timer[3] = 0;
				m_reg[TMP95C063_INTET23] |= 0x80;

				if ( m_reg[TMP95C063_T02FFCR] & 0x20 )
				{
					tlcs900_change_tff( 3, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 2 */
				if ( ( m_reg[TMP95C063_T23MOD] & 0xc0 ) == 0x40 )
				{
					m_timer[2] = 0;
				}
			}
		}
	}

	m_timer_pre &= 0xffffff;
}

void tmp95c063_device::tlcs900_check_hdma()
{
	// TODO
}

void tmp95c063_device::tlcs900_check_irqs()
{
	int irq_vectors[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int level = 0;
	int irq = -1;
	int i;

	/* Check for NMI */
	if ( m_nmi_state == ASSERT_LINE )
	{
		m_xssp.d -= 4;
		WRMEML( m_xssp.d, m_pc.d );
		m_xssp.d -= 2;
		WRMEMW( m_xssp.d, m_sr.w.l );
		m_pc.d = RDMEML( 0xffff00 + 0x20 );
		m_cycles += 18;
		m_prefetch_clear = true;

		m_halted = 0;

		m_nmi_state = CLEAR_LINE;

		return;
	}

	/* Check regular irqs */
	for( i = 0; i < TMP95C063_NUM_MASKABLE_IRQS; i++ )
	{
		if ( m_reg[tmp95c063_irq_vector_map[i].reg] & tmp95c063_irq_vector_map[i].iff )
		{
			switch( tmp95c063_irq_vector_map[i].iff )
			{
			case 0x80:
				irq_vectors[ ( m_reg[ tmp95c063_irq_vector_map[i].reg ] >> 4 ) & 0x07 ] = i;
				break;
			case 0x08:
				irq_vectors[ m_reg[ tmp95c063_irq_vector_map[i].reg ] & 0x07 ] = i;
				break;
			}
		}
	}

	/* Check highest allowed priority irq */
	for ( i = MAX( 1, ( ( m_sr.b.h & 0x70 ) >> 4 ) ); i < 7; i++ )
	{
		if ( irq_vectors[i] >= 0 )
		{
			irq = irq_vectors[i];
			level = i + 1;
		}
	}

	/* Take irq */
	if ( irq >= 0 )
	{
		UINT8 vector = tmp95c063_irq_vector_map[irq].vector;

		m_xssp.d -= 4;
		WRMEML( m_xssp.d, m_pc.d );
		m_xssp.d -= 2;
		WRMEMW( m_xssp.d, m_sr.w.l );

		/* Mask off any lower priority interrupts  */
		m_sr.b.h = ( m_sr.b.h & 0x8f ) | ( level << 4 );

		m_pc.d = RDMEML( 0xffff00 + vector );
		m_cycles += 18;
		m_prefetch_clear = true;

		m_halted = 0;

		/* Clear taken IRQ */
		m_reg[ tmp95c063_irq_vector_map[irq].reg ] &= ~ tmp95c063_irq_vector_map[irq].iff;
	}
}


void tmp95c063_device::tlcs900_handle_ad()
{
	// TODO
}


void tmp95c063_device::device_reset()
{
	int i;

	m_pc.b.l = RDMEM( 0xFFFF00 );
	m_pc.b.h = RDMEM( 0xFFFF01 );
	m_pc.b.h2 = RDMEM( 0xFFFF02 );
	m_pc.b.h3 = 0;
	/* system mode, iff set to 111, max mode, register bank 0 */
	m_sr.d = 0xF800;
	m_regbank = 0;
	m_xssp.d = 0x0100;
	m_halted = 0;
	m_check_irqs = 0;
	m_ad_cycles_left = 0;
	m_nmi_state = CLEAR_LINE;
	m_timer_pre = 0;
	m_timer_change[0] = 0;
	m_timer_change[1] = 0;
	m_timer_change[2] = 0;
	m_timer_change[3] = 0;

	m_reg[TMP95C063_P1] = 0x00;
	m_reg[TMP95C063_P1CR] = 0x00;
	m_reg[TMP95C063_P2] = 0xff;
	m_reg[TMP95C063_P2FC] = 0x00;
	m_reg[TMP95C063_P5] = 0x3d;
	m_reg[TMP95C063_P5CR] = 0x00;
	m_reg[TMP95C063_P5FC] = 0x00;
	m_reg[TMP95C063_P6] = 0x3b;
	m_reg[TMP95C063_P6FC] = 0x00;
	m_reg[TMP95C063_P7] = 0xff;
	m_reg[TMP95C063_P7CR] = 0x00;
	m_reg[TMP95C063_P7FC] = 0x00;
	m_reg[TMP95C063_P8] = 0x3f;
	m_reg[TMP95C063_P8CR] = 0x00;
	m_reg[TMP95C063_P8FC] = 0x00;
	m_reg[TMP95C063_PA] = 0x0f;
	m_reg[TMP95C063_PACR] = 0x00;
	m_reg[TMP95C063_PAFC] = 0x00;
	m_reg[TMP95C063_PB] = 0xff;
	m_reg[TMP95C063_PBCR] = 0x00;
	m_reg[TMP95C063_PBFC] = 0x00;
	m_reg[TMP95C063_MSAR0] = 0xff;
	m_reg[TMP95C063_MSAR1] = 0xff;
	m_reg[TMP95C063_MSAR2] = 0xff;
	m_reg[TMP95C063_MSAR3] = 0xff;
	m_reg[TMP95C063_MAMR0] = 0xff;
	m_reg[TMP95C063_MAMR1] = 0xff;
	m_reg[TMP95C063_MAMR2] = 0xff;
	m_reg[TMP95C063_MAMR3] = 0xff;
	m_reg[TMP95C063_DREFCR1] = 0x00;
	m_reg[TMP95C063_DMEMCR1] = 0x80;
	m_reg[TMP95C063_DREFCR3] = 0x00;
	m_reg[TMP95C063_DMEMCR3] = 0x80;
	m_reg[TMP95C063_T01MOD] = 0x00;
	m_reg[TMP95C063_T23MOD] = 0x00;
	m_reg[TMP95C063_T02FFCR] = 0x00;
	m_reg[TMP95C063_T46FFCR] = 0x00;
	m_reg[TMP95C063_T8RUN] = 0x00;
	m_reg[TMP95C063_TRDC] = 0x00;
	m_reg[TMP95C063_T45MOD] = 0x20;
	m_reg[TMP95C063_T46FFCR] = 0x00;
	m_reg[TMP95C063_PG01CR] = 0x00;
	m_reg[TMP95C063_PG0REG] = 0x00;
	m_reg[TMP95C063_PG1REG] = 0x00;
	m_reg[TMP95C063_SC0MOD] = 0x00;
	m_reg[TMP95C063_SC0CR] = 0x00;
	m_reg[TMP95C063_BR0CR] = 0x00;
	m_reg[TMP95C063_SC1MOD] = 0x00;
	m_reg[TMP95C063_SC1CR] = 0x00;
	m_reg[TMP95C063_BR1CR] = 0x00;
	m_reg[TMP95C063_P8FC] = 0x00;
	m_reg[TMP95C063_ODE] = 0x00;
	m_reg[TMP95C063_ADMOD1] = 0x00;
	m_reg[TMP95C063_ADMOD2] = 0x00;
	m_reg[TMP95C063_ADREG04L] = 0x3f;
	m_reg[TMP95C063_ADREG04H] = 0x00;
	m_reg[TMP95C063_ADREG15L] = 0x3f;
	m_reg[TMP95C063_ADREG15H] = 0x00;
	m_reg[TMP95C063_ADREG26L] = 0x3f;
	m_reg[TMP95C063_ADREG26H] = 0x00;
	m_reg[TMP95C063_ADREG37L] = 0x3f;
	m_reg[TMP95C063_ADREG37H] = 0x00;
	m_reg[TMP95C063_WDMOD] = 0x80;

	for ( i = 0; i < TLCS900_NUM_INPUTS; i++ )
	{
		m_level[i] = CLEAR_LINE;
	}
}

READ8_MEMBER( tlcs900h_device::tmp95c063_internal_r )
{
	switch (offset)
	{
		case TMP95C063_P1: m_reg[offset] = m_port_read(0x1); break;
		case TMP95C063_P2: m_reg[offset] = m_port_read(0x2); break;
		case TMP95C063_P5: m_reg[offset] = m_port_read(0x5); break;
		case TMP95C063_P6: m_reg[offset] = m_port_read(0x6); break;
		case TMP95C063_P7: m_reg[offset] = m_port_read(0x7); break;
		case TMP95C063_P8: m_reg[offset] = m_port_read(0x8); break;
		case TMP95C063_P9: m_reg[offset] = m_port_read(0x9); break;
		case TMP95C063_PA: m_reg[offset] = m_port_read(0xa); break;
		case TMP95C063_PB: m_reg[offset] = m_port_read(0xb); break;
		case TMP95C063_PC: m_reg[offset] = m_port_read(0xc); break;
		case TMP95C063_PD: m_reg[offset] = m_port_read(0xd); break;
		case TMP95C063_PE: m_reg[offset] = m_port_read(0xe); break;
	}
	return m_reg[ offset ];
}


WRITE8_MEMBER( tlcs900h_device::tmp95c063_internal_w )
{
	switch ( offset )
	{
	case TMP95C063_T8RUN:
		if ( ! ( data & 0x01 ) )
		{
			m_timer[0] = 0;
			m_timer_change[0] = 0;
		}
		if ( ! ( data & 0x02 ) )
		{
			m_timer[1] = 0;
			m_timer_change[1] = 0;
		}
		if ( ! ( data & 0x04 ) )
		{
			m_timer[2] = 0;
			m_timer_change[2] = 0;
		}
		if ( ! ( data & 0x08 ) )
		{
			m_timer[3] = 0;
			m_timer_change[3] = 0;
		}
		if ( ! ( data & 0x10 ) )
			m_timer[4] = 0;
		if ( ! ( data & 0x20 ) )
			m_timer[5] = 0;
		break;

	case TMP95C063_T02FFCR:
		switch( data & 0x0c )
		{
		case 0x00:
			tlcs900_change_tff( 1, FF_INVERT );
			break;
		case 0x04:
			tlcs900_change_tff( 1, FF_SET );
			break;
		case 0x08:
			tlcs900_change_tff( 1, FF_CLEAR );
			break;
		}
		switch( data & 0xc0 )
		{
		case 0x00:
			tlcs900_change_tff( 3, FF_INVERT );
			break;
		case 0x40:
			tlcs900_change_tff( 3, FF_SET );
			break;
		case 0x80:
			tlcs900_change_tff( 3, FF_CLEAR );
			break;
		}
		break;
	case TMP95C063_MSAR0:
	case TMP95C063_MAMR0:
	case TMP95C063_MSAR1:
	case TMP95C063_MAMR1:
		break;

	case TMP95C063_WDMOD:
	case TMP95C063_WDCR:
		break;

	case TMP95C063_INTE0AD:
	case TMP95C063_INTE12:
	case TMP95C063_INTE34:
	case TMP95C063_INTE56:
	case TMP95C063_INTE78:
	case TMP95C063_INTET01:
	case TMP95C063_INTET23:
	case TMP95C063_INTET45:
	case TMP95C063_INTET67:
	case TMP95C063_INTET89:
	case TMP95C063_INTETAB:
	case TMP95C063_INTES0:
	case TMP95C063_INTES1:
	case TMP95C063_INTETC01:
	case TMP95C063_INTETC23:
		if ( data & 0x80 )
			data = ( data & 0x7f ) | ( m_reg[offset] & 0x80 );
		if ( data & 0x08 )
			data = ( data & 0xf7 ) | ( m_reg[offset] & 0x08 );
		break;

	case TMP95C063_IIMC:
		break;

	default:
		break;
	}

	if (!m_port_write.isnull())
	{
		switch (offset)
		{
			case TMP95C063_P1: m_port_write(0x1, data, 0xff); break;
			case TMP95C063_P2: m_port_write(0x2, data, 0xff); break;
			case TMP95C063_P5: m_port_write(0x5, data, 0xff); break;
			case TMP95C063_P6: m_port_write(0x6, data, 0xff); break;
			case TMP95C063_P7: m_port_write(0x7, data, 0xff); break;
			case TMP95C063_P8: m_port_write(0x8, data, 0xff); break;
			case TMP95C063_P9: m_port_write(0x9, data, 0xff); break;
			case TMP95C063_PA: m_port_write(0xa, data, 0xff); break;
			case TMP95C063_PB: m_port_write(0xb, data, 0xff); break;
			case TMP95C063_PC: m_port_write(0xc, data, 0xff); break;
			case TMP95C063_PD: m_port_write(0xd, data, 0xff); break;
			case TMP95C063_PE: m_port_write(0xe, data, 0xff); break;
		}
	}

	m_check_irqs = 1;
	m_reg[ offset ] = data;
}


void tmp95c063_device::execute_set_input(int input, int level)
{
	switch( input )
	{
	case INPUT_LINE_NMI:
	case TLCS900_NMI:
		if ( m_level[TLCS900_NMI] == CLEAR_LINE && level == ASSERT_LINE )
		{
			m_nmi_state = level;
		}
		m_level[TLCS900_NMI] = level;
		break;

	case TLCS900_INTWD:
		break;

	case TLCS900_INT0:
		/* Is INT0 functionality enabled? */
		if (m_reg[TMP95C063_IIMC] & 0x04)
		{
			if (m_reg[TMP95C063_IIMC] & 0x02)
			{
				/* Rising edge detect */
				if (m_level[TLCS900_INT0] == CLEAR_LINE && level == ASSERT_LINE)
				{
					/* Leave HALT state */
					m_halted = 0;
					m_reg[TMP95C063_INTE0AD] |= 0x08;
				}
			}
			else
			{
				/* Level detect */
				if (level == ASSERT_LINE)
					m_reg[TMP95C063_INTE0AD] |= 0x08;
				else
					m_reg[TMP95C063_INTE0AD] &= ~ 0x08;
			}
		}
		m_level[TLCS900_INT0] = level;
		break;

	case TLCS900_INT1:
		if (m_level[TLCS900_INT1] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_reg[TMP95C063_INTE12] |= 0x08;
		}
		else if (m_level[TLCS900_INT1] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_reg[TMP95C063_INTE12] &= ~0x08;
		}
		m_level[TLCS900_INT1] = level;
		break;

	case TLCS900_INT2:
		if (m_level[TLCS900_INT2] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_reg[TMP95C063_INTE12] |= 0x80;
		}
		else if (m_level[TLCS900_INT2] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_reg[TMP95C063_INTE12] &= ~0x80;
		}
		m_level[TLCS900_INT2] = level;
		break;

	case TLCS900_INT3:
		if (m_level[TLCS900_INT3] == CLEAR_LINE && level == ASSERT_LINE)
		{
			m_reg[TMP95C063_INTE34] |= 0x08;
		}
		else if (m_level[TLCS900_INT3] == ASSERT_LINE && level == CLEAR_LINE)
		{
			m_reg[TMP95C063_INTE34] &= ~0x08;
		}
		m_level[TLCS900_INT3] = level;
		break;

	case TLCS900_INT4:
		if ( ! ( m_reg[TMP95C063_PBCR] & 0x01 ) )
		{
			if ( m_level[TLCS900_INT4] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_reg[TMP95C063_INTE34] |= 0x80;
			}
		}
		m_level[TLCS900_INT4] = level;
		break;

	case TLCS900_INT5:
		if ( ! ( m_reg[TMP95C063_PBCR] & 0x02 ) )
		{
			if ( m_level[TLCS900_INT5] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_reg[TMP95C063_INTE56] |= 0x08;
			}
		}
		m_level[TLCS900_INT5] = level;
		break;

	case TLCS900_TIO:   /* External timer input for timer 0 */
		if ( ( m_reg[TMP95C063_T8RUN] & 0x01 ) && ( m_reg[TMP95C063_T01MOD] & 0x03 ) == 0x00 )
		{
			if ( m_level[TLCS900_TIO] == CLEAR_LINE && level == ASSERT_LINE )
			{
				m_timer_change[0] += 1;
			}
		}
		m_level[TLCS900_TIO] = level;
		break;
	}
	m_check_irqs = 1;
}

