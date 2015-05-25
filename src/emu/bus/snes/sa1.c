// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, R. Belmont
/***********************************************************************************************************

 SA-1 add-on chip emulation (for SNES/SFC)

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

 Compatibility:
    asahishi: plays OK
    daisenx2: plays OK
    derbyjo2: hangs going into game
    dbzhypd, dbzhypdj: plays OK
    habumeij: boots, goes into game, on-screen timer counts down after SA-1 is enabled but controls aren't responsive
    haruaug3a, pebble, haruaug3: uses SA-1 DMA
    itoibass: boots, some missing gfx
    jikkparo: plays OK
    jl96drem: plays OK
    jumpind: boots and runs, uses SA-1 normal DMA only but has corrupt gfx
    kakinoki: S-CPU crashes after pressing start
    kirby3j, kirby3: uses SA-1 DMA
    kirbysdb, kirbyss, kirbyfun, kirbysd, kirbysda: plays OK
    marvelou: plays OK, uses SA-1 normal DMA only but has corrupt gfx
    miniyonk: plays OK
    panicbw: plays OK
    pgaeuro, pgaeurou, pga96, pga96u, pga, pgaj: plays OK
    przeo, przeou: plays OK
    prokishi: plays OK
    rinkaiho: plays OK
    saikouso: plays OK
    sdf1gpp, sdf1gp: corrupt menu gfx, hangs going into game (I think)
    sdgungnx: plays OK
    shinshog: plays OK
    shogisai: plays OK
    shogisa2: plays OK
    smrpgj, smrpg: needs SA-1 character conversion for level up Bonus Chance (possible to get past now)
    srobotg: some corrupt in-game GFX, may be SNES rendering errors
    sshogi3: plays OK
    taikyoid: plays OK
    takemiya: plays OK
 [Note: for Igo & Shougi games, "plays OK" means you can get ingame and the CPU replies to your moves... subtle bugs
 might indeed exist...]

 ***********************************************************************************************************/

#include "emu.h"
#include "sa1.h"

#define SA1_IRQ_SCPU    (0x80)
#define SA1_IRQ_TIMER   (0x40)
#define SA1_IRQ_DMA     (0x20)
#define SA1_NMI_SCPU    (0x10)

#define SCPU_IRQ_SA1    (0x80)
#define SCPU_IRQV_ALT   (0x40)
#define SCPU_IRQ_CHARCONV (0x20)
#define SCPU_NMIV_ALT   (0x10)

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
	m_scpu_ctrl = 0;
	m_nmi_vector = 0;
	m_bank_c_hi = 0;
	m_bank_c_rom = 0;
}

void sns_sa1_device::device_reset()
{
	memset(m_internal_ram, 0, sizeof(m_internal_ram));

	m_sa1_ctrl = 0x20;
	m_scpu_ctrl = 0;
	m_irq_vector = 0;
	m_nmi_vector = 0;
	m_hcount = 0;
	m_vcount = 0;
	m_bank_c_hi = 0;
	m_bank_c_rom = 0;
	m_bank_d_hi = 0;
	m_bank_d_rom = 1;
	m_bank_e_hi = 0;
	m_bank_e_rom = 2;
	m_bank_f_hi = 0;
	m_bank_f_rom = 3;
	m_bwram_snes = 0;
	m_bwram_sa1 = 0;
	m_bwram_sa1_source = 0;
	m_bwram_sa1_format = 0;
	m_bwram_write_snes = 1;
	m_bwram_write_sa1 = 1;
	m_bwpa_sa1 = 0x0f;
	m_iram_write_snes = 1;
	m_iram_write_sa1 = 1;
	m_src_addr = 0;
	m_dst_addr = 0;
	memset(m_brf_reg, 0, sizeof(m_brf_reg));
	m_math_ctlr = 0;
	m_math_overflow = 0;
	m_math_a = 0;
	m_math_b = 0;
	m_math_res = 0;
	m_vda = 0;
	m_vbit = 0;
	m_vlen = 0;
	m_drm = 0;
	m_hcr = 0;
	m_vcr = 0;
	m_scpu_sie = m_sa1_sie = 0;
	m_scpu_flags = m_sa1_flags = 0;
	m_dma_ctrl = 0;
	m_dma_ccparam = 0;
	m_dma_cnt = 0;

	// sa-1 CPU starts out not running?
	m_sa1->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void sns_sa1_device::recalc_irqs()
{
	if (m_scpu_flags & m_scpu_sie & (SCPU_IRQ_SA1|SCPU_IRQ_CHARCONV))
	{
		machine().device("maincpu")->execute().set_input_line(G65816_LINE_IRQ, ASSERT_LINE);
	}
	else
	{
		machine().device("maincpu")->execute().set_input_line(G65816_LINE_IRQ, CLEAR_LINE);
	}

	if (m_sa1_flags & m_sa1_sie & (SA1_IRQ_SCPU|SA1_IRQ_TIMER|SA1_IRQ_DMA))
	{
		m_sa1->set_input_line(G65816_LINE_IRQ, ASSERT_LINE);
	}
	else
	{
		m_sa1->set_input_line(G65816_LINE_IRQ, CLEAR_LINE);
	}

	if (m_sa1_flags & m_sa1_sie & SA1_NMI_SCPU)
	{
		m_sa1->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	}
	else
	{
		m_sa1->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
	}
}


/*-------------------------------------------------
  RAM / SRAM / Registers
 -------------------------------------------------*/


// handle this separately to avoid accessing recursively the regs?

UINT8 sns_sa1_device::var_length_read(address_space &space, UINT32 offset)
{
	// handle 0xffea/0xffeb/0xffee/0xffef
	if ((offset & 0xffffe0) == 0x00ffe0)
	{
		if (offset == 0xffea && BIT(m_scpu_ctrl, 4)) return (m_nmi_vector >> 0) & 0xff;
		if (offset == 0xffeb && BIT(m_scpu_ctrl, 4)) return (m_nmi_vector >> 8) & 0xff;
		if (offset == 0xffee && BIT(m_scpu_ctrl, 6)) return (m_irq_vector >> 0) & 0xff;
		if (offset == 0xffef && BIT(m_scpu_ctrl, 6)) return (m_irq_vector >> 8) & 0xff;
	}

	if ((offset & 0xc08000) == 0x008000)  //$00-3f:8000-ffff
		return read_l(space, (offset & 0x7fffff));

	if ((offset & 0xc08000) == 0x808000)  //$80-bf:8000-ffff
		return read_h(space, (offset & 0x7fffff));

	if ((offset & 0xc00000) == 0xc00000)  //$c0-ff:0000-ffff
		return read_h(space, (offset & 0x7fffff));

	if ((offset & 0x40e000) == 0x006000)  //$00-3f|80-bf:6000-7fff
		return read_bwram((m_bwram_snes * 0x2000) + (offset & 0x1fff));

	if ((offset & 0xf00000) == 0x400000)  //$40-4f:0000-ffff
		return read_bwram(offset & 0xfffff);

	if ((offset & 0x40f800) == 0x000000)  //$00-3f|80-bf:0000-07ff
		return read_iram(offset);

	if ((offset & 0x40f800) == 0x003000)  //$00-3f|80-bf:3000-37ff
		return read_iram(offset);

	return 0;
}

void sns_sa1_device::dma_transfer(address_space &space)
{
//  printf("DMA src %08x (%d), dst %08x (%d) cnt %d\n", m_src_addr, m_dma_ctrl & 3, m_dst_addr, m_dma_ctrl & 4, m_dma_cnt);

	while (m_dma_cnt--)
	{
		UINT8 data = 0; // open bus?
		UINT32 dma_src = m_src_addr++;
		UINT32 dma_dst = m_dst_addr++;

		// source and destination cannot be the same
		// source = { 0=ROM, 1=BWRAM, 2=IRAM }
		// destination = { 0=IRAM, 1=BWRAM }
		if ((m_dma_ctrl & 0x03) == 1 && (m_dma_ctrl & 0x04) == 0x04) continue;
		if ((m_dma_ctrl & 0x03) == 2 && (m_dma_ctrl & 0x04) == 0x00) continue;

		switch (m_dma_ctrl & 0x03)
		{
			case 0: // ROM
				if ((dma_src & 0x408000) == 0x008000 && (dma_src & 0x800000) == 0x000000)
				{
					data = read_l(space, (dma_src & 0x7fffff));
				}
				if ((dma_src & 0x408000) == 0x008000 && (dma_src & 0x800000) == 0x800000)
				{
					data = read_h(space, (dma_src & 0x7fffff));
				}
				if ((dma_src & 0xc00000) == 0xc00000)
				{
					data = read_h(space, (dma_src & 0x7fffff));
				}
				break;

			case 1: // BWRAM
				if ((dma_src & 0x40e000) == 0x006000)
				{
					data = read_bwram((m_bwram_sa1 * 0x2000) + (dma_src & 0x1fff));
				}
				if ((dma_src & 0xf00000) == 0x400000)
				{
					data = read_bwram(dma_src & 0xfffff);
				}
				break;

			case 2: // IRAM
				data = read_iram(dma_src);
				break;
		}

		switch (m_dma_ctrl & 0x04)
		{
			case 0x00:  // IRAM
				write_iram(dma_dst, data);
				break;

			case 0x04:  // BWRAM
				if ((dma_dst & 0x40e000) == 0x006000)
				{
					write_bwram((m_bwram_sa1 * 0x2000) + (dma_dst & 0x1fff), data);
				}
				if ((dma_dst & 0xf00000) == 0x400000)
				{
					write_bwram(dma_dst & 0xfffff, data);
				}
				break;
		}
	}

	m_sa1_flags |= SA1_IRQ_DMA;
	recalc_irqs();
}

void sns_sa1_device::dma_cctype1_transfer(address_space &space)
{
	m_scpu_flags |= SCPU_IRQ_CHARCONV;
	recalc_irqs();
}

void sns_sa1_device::dma_cctype2_transfer(address_space &space)
{
}

UINT8 sns_sa1_device::read_regs(address_space &space, UINT32 offset)
{
	UINT8 value = 0xff;
	offset &= 0x1ff;    // $2200 + offset gives the reg value to compare with docs

	switch (offset)
	{
		case 0x100:
			// S-CPU Flag Read
			value = (m_scpu_ctrl & 0x0f) | m_scpu_flags;
			break;
		case 0x101:
			// SA-1 Flag Read
			value = (m_sa1_ctrl & 0x0f) | m_sa1_flags;
			break;
		case 0x102:
			// H-Count Read Low
			//latch counters
			m_hcr = m_hcount >> 2;
			m_vcr = m_vcount;
			//then return h-count
			value = (m_hcr >> 0) & 0xff;
			break;
		case 0x103:
			// H-Count Read High
			value = (m_hcr >> 8) & 0xff;
			break;
		case 0x104:
			// V-Count Read Low
			value = (m_vcr >> 0) & 0xff;
			break;
		case 0x105:
			// V-Count Read High
			value = (m_vcr >> 8) & 0xff;
			break;
		case 0x106:
			// Math Result bits0-7
			value = (UINT64)(m_math_res >> 0) & 0xff;
			break;
		case 0x107:
			// Math Result bits8-15
			value = (UINT64)(m_math_res >> 8) & 0xff;
			break;
		case 0x108:
			// Math Result bits16-23
			value = (UINT64)(m_math_res >> 16) & 0xff;
			break;
		case 0x109:
			// Math Result bits24-31
			value = (UINT64)(m_math_res >> 24) & 0xff;
			break;
		case 0x10a:
			// Math Result bits32-39
			value = (UINT64)(m_math_res >> 32) & 0xff;
			break;
		case 0x10b:
			// Math Overflow (above 40bit result)
			value = m_math_overflow;
			break;
		case 0x10c:
			// Var-Length Read Port Low
			{
				UINT32 data = (var_length_read(space, m_vda + 0) <<  0) | (var_length_read(space, m_vda + 1) <<  8)
															| (var_length_read(space, m_vda + 2) << 16);
				data >>= m_vbit;
				value = (data >> 0) & 0xff;
			}
			break;
		case 0x10d:
			// Var-Length Read Port High
			{
				UINT32 data = (var_length_read(space, m_vda + 0) <<  0) | (var_length_read(space, m_vda + 1) <<  8)
															| (var_length_read(space, m_vda + 2) << 16);
				data >>= m_vbit;

				if (m_drm == 1)
				{
					//auto-increment mode
					m_vbit += m_vlen;
					m_vda += (m_vbit >> 3);
					m_vbit &= 7;
				}

				value = (data >> 8) & 0xff;
			}
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

void sns_sa1_device::write_regs(address_space &space, UINT32 offset, UINT8 data)
{
	offset &= 0x1ff;    // $2200 + offset gives the reg value to compare with docs

	switch (offset)
	{
		case 0x000:
			// SA-1 control flags
//          printf("%02x to SA-1 control\n", data);
			if ((BIT(data, 5)) && !(BIT(m_sa1_ctrl, 5)))
			{
//              printf("Engaging SA-1 reset\n");
				m_sa1->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			}
			else if (!(BIT(data, 5)) && (BIT(m_sa1_ctrl, 5)))
			{
//              printf("Releasing SA-1 reset\n");
				m_sa1->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m_sa1->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_sa1->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}

			m_sa1_ctrl = data;

			// message to S-CPU
			m_scpu_ctrl &= 0xf0;
			m_scpu_ctrl |= (data & 0x0f);

			if (BIT(m_sa1_ctrl, 7))
			{
				m_sa1_flags |= SA1_IRQ_SCPU;
			}
			if (BIT(m_sa1_ctrl, 4))
			{
				m_sa1_flags |= SA1_NMI_SCPU;
			}
			recalc_irqs();
			break;
		case 0x001:
			// SNES  SIE   00h   SNES CPU Int Enable (W)
			m_scpu_sie = data;
//          printf("S-CPU IE = %02x\n", data);
			recalc_irqs();
			break;
		case 0x002:
			// SNES  SIC   00h   SNES CPU Int Clear  (W)
			if (BIT(data, 7))   // ack IRQ from SA-1
			{
				m_scpu_flags &= ~SCPU_IRQ_SA1;
			}
			if (BIT(data, 5))   // ack character conversion IRQ
			{
				m_scpu_flags &= ~SCPU_IRQ_CHARCONV;
			}
			recalc_irqs();
			break;
		case 0x003:
			// SNES  CRV   -     SA-1 CPU Reset Vector Lsb (W)
			m_sa1_reset &= 0xff00;
			m_sa1_reset |= data;
			break;
		case 0x004:
			// SNES  CRV   -     SA-1 CPU Reset Vector Msb (W)
			m_sa1_reset &= 0x00ff;
			m_sa1_reset |= (data<<8);
			break;
		case 0x005:
			// SNES  CNV   -     SA-1 CPU NMI Vector Lsb (W)
			m_sa1_nmi &= 0xff00;
			m_sa1_nmi |= data;
			break;
		case 0x006:
			// SNES  CNV   -     SA-1 CPU NMI Vector Msb (W)
			m_sa1_nmi &= 0x00ff;
			m_sa1_nmi |= (data<<8);
			break;
		case 0x007:
			// SNES  CIV   -     SA-1 CPU IRQ Vector Lsb (W)
			m_sa1_irq &= 0xff00;
			m_sa1_irq |= data;
			break;
		case 0x008:
			// SNES  CIV   -     SA-1 CPU IRQ Vector Msb (W)
			m_sa1_irq &= 0x00ff;
			m_sa1_irq |= (data<<8);
			break;
		case 0x009:
			// S-CPU control flags
			m_scpu_ctrl = data;
			if (m_scpu_ctrl & 0x80)
			{
				m_scpu_flags |= SCPU_IRQ_SA1;
//              printf("SA-1 cause S-CPU IRQ\n");
			}

			// message to SA-1
			m_sa1_ctrl &= 0xf0;
			m_sa1_ctrl |= (data & 0x0f);

			// clear IRQ/NMI override flags in flags word
			m_scpu_flags &= ~(SCPU_IRQV_ALT|SCPU_NMIV_ALT);

			// and set them
			m_scpu_flags |= (data & (SCPU_IRQV_ALT|SCPU_NMIV_ALT));

			recalc_irqs();
			break;
		case 0x00a:
			// SA-1  CIE   00h   SA-1 CPU Int Enable (W)
			m_sa1_sie = data;
//          printf("SA-1 IE = %02x\n", data);
			recalc_irqs();
			break;
		case 0x00b:
			// SA-1  CIC   00h   SA-1 CPU Int Clear  (W)
			if (BIT(data, 7))
			{
				m_sa1_flags &= ~SA1_IRQ_SCPU;
			}
			if (BIT(data, 6))
			{
				m_sa1_flags &= ~SA1_IRQ_TIMER;
			}
			if (BIT(data, 5))
			{
				m_sa1_flags &= ~SA1_IRQ_DMA;
			}
			if (BIT(data, 4))
			{
				m_sa1_flags &= ~SA1_NMI_SCPU;
			}
			recalc_irqs();
			break;
		case 0x00c:
			// NMI Vector Low
			m_nmi_vector = (m_nmi_vector & 0xff00) | (data << 0);
			break;
		case 0x00d:
			// NMI Vector High
			m_nmi_vector = (m_nmi_vector & 0x00ff) | (data << 8);
			break;
		case 0x00e:
			// IRQ Vector Low
			m_irq_vector = (m_irq_vector & 0xff00) | (data << 0);
			break;
		case 0x00f:
			// IRQ Vector High
			m_irq_vector = (m_irq_vector & 0x00ff) | (data << 8);
			break;
		case 0x010:
			// SA-1  TMC   00h   H/V Timer Control (W)
			break;
		case 0x011:
			// SA-1  CTR   -     SA-1 CPU Timer Restart (W)
			break;
		case 0x012:
			// H-Count Low
			m_hcount = (m_hcount & 0xff00) | (data << 0);
			break;
		case 0x013:
			// H-Count High
			m_hcount = (m_hcount & 0x00ff) | (data << 8);
			break;
		case 0x014:
			// V-Count Low
			m_vcount = (m_vcount & 0xff00) | (data << 0);
			break;
		case 0x015:
			// V-Count High
			m_vcount = (m_vcount & 0x00ff) | (data << 8);
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
			m_bwram_snes = data & 0x1f; // max 32x8K banks
			break;
		case 0x025:
			// BWRAM bank & type from SA-1 side
			m_bwram_sa1_source = BIT(data, 7);  // 0 = normal, 1 = bitmap?
			m_bwram_sa1 = data & 0x7f;  // up to 128x8K banks here?
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
//          printf("%02x to SA-1 DMA control\n", data);
			m_dma_ctrl = data;
			break;
		case 0x031:
			// Both  CDMA  00h   Character Conversion DMA Parameters (W)
			m_dma_ccparam = data;
			break;
		case 0x032:
			// DMA Source Device Start Address Low
			m_src_addr = (m_src_addr & 0xffff00) | (data << 0);
			break;
		case 0x033:
			// DMA Source Device Start Address Mid
			m_src_addr = (m_src_addr & 0xff00ff) | (data << 8);
			break;
		case 0x034:
			// DMA Source Device Start Address High
			m_src_addr = (m_src_addr & 0x00ffff) | (data << 16);
			break;
		case 0x035:
			// DMA Dest Device Start Address Low
			m_dst_addr = (m_dst_addr & 0xffff00) | (data << 0);
			break;
		case 0x036:
			// DMA Dest Device Start Address Mid
			m_dst_addr = (m_dst_addr & 0xff00ff) | (data << 8);
			if (m_dma_ctrl & 0x80)
			{
				if (!(m_dma_ctrl & 0x20) && !(m_dma_ctrl & 0x04)) // Normal DMA to IRAM
				{
					dma_transfer(space);
//                  printf("SA-1: normal DMA to IRAM\n");
				}

				if (m_dma_ctrl & 0x20 && m_dma_ctrl & 0x10) // CC DMA Type 1
				{
//                  printf("SA-1: CC DMA type 1\n");
					dma_cctype1_transfer(space);
				}
			}
			break;
		case 0x037:
			// DMA Dest Device Start Address High
			m_dst_addr = (m_dst_addr & 0xffff00) | (data << 16);
			if (m_dma_ctrl & 0x80)
			{
				if (!(m_dma_ctrl & 0x20) && m_dma_ctrl & 0x04)  // Normal DMA to BWRAM
				{
//                  printf("SA-1: normal DMA to BWRAM\n");
					dma_transfer(space);
				}
			}
			break;
		case 0x038:
			// SA-1  DTC   -     DMA Terminal Counter Lsb (W)
			m_dma_cnt &= 0xff00;
			m_dma_cnt |= data;
			break;
		case 0x039:
			// SA-1  DTC   -     DMA Terminal Counter Msb (W)
			m_dma_cnt &= 0x00ff;
			m_dma_cnt |= (data<<8);
			break;
		case 0x03f:
			// Format for BWRAM when mapped to bitmap
			m_bwram_sa1_format = BIT(data, 7);  // 0 = 4bit, 1 = 2bit
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
			// Bit Map Register File (2240h..224Fh)
			m_brf_reg[offset & 0x0f] = data;
			if ((offset & 0x07) == 7 && m_dma_ctrl & 0x80)
			{
				if (m_dma_ctrl & 0x20 && !(m_dma_ctrl & 0x10))  // CC DMA Type 2
				{
//                  printf("SA-1: CC DMA type 2\n");
					dma_cctype2_transfer(space);
				}
			}
			break;
		case 0x050:
			// Math control
			m_math_ctlr = data & 0x03;
			if (data & 0x02)
				m_math_res = 0;
			break;
		case 0x051:
			// Math A Low
			m_math_a = (m_math_a & 0xff00) | data;
			break;
		case 0x052:
			// Math A High
			m_math_a = (data << 8) | (m_math_a & 0x00ff);
			break;
		case 0x053:
			// Math B Low
			m_math_b = (m_math_b & 0xff00) | data;
			break;
		case 0x054:
			// Math B High
			m_math_b = (data << 8) | (m_math_b & 0x00ff);
			// After Math B has been written, we do math
			switch (m_math_ctlr)
			{
				case 0: //signed multiplication
					m_math_res = (INT16)m_math_a * (INT16)m_math_b;
					m_math_b = 0;
					break;
				case 1: //unsigned division
					if (m_math_b == 0)
						m_math_res = 0;
					else
					{
						INT16  quotient  = (INT16)m_math_a / (UINT16)m_math_b;
						UINT16 remainder = (INT16)m_math_a % (UINT16)m_math_b;
						m_math_res = (UINT64)((remainder << 16) | quotient);
					}
					break;
				case 2: //sigma (accumulative multiplication)
				case 3:
					UINT64 acum = (INT16)m_math_a * (INT16)m_math_b;
					UINT64 mask = U64(0xffffffffff);
					m_math_res += acum;
					m_math_overflow = (m_math_res > mask) ? 0x80 : 0;
					m_math_res &= mask;
					m_math_b = 0;
					break;
			}
			break;
		case 0x058:
			// Var-Length Bit Processing
			m_drm = BIT(data, 7);   // Data Read Mode
			m_vlen = (data & 0x0f);
			if (m_vlen == 0)
				m_vlen = 16;

			if (m_drm == 0)
			{
				//fixed mode
				m_vbit += m_vlen;
				m_vda += (m_vbit >> 3);
				m_vbit &= 7;
			}
			break;
		case 0x059:
			// Var-Length Read Start Address Low
			m_vda = (m_vda & 0xffff00) | (data << 0);
			break;
		case 0x05a:
			// Var-Length Read Start Address Mid
			m_vda = (m_vda & 0xff00ff) | (data << 8);
			break;
		case 0x05b:
			// Var-Length Read Start Address High
			m_vda = (m_vda & 0x00ffff) | (data << 16);
			m_vbit = 0;
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

	if (m_nvram.empty())
		return 0xff;    // this should probably never happen, or are there SA-1 games with no BWRAM?

	if (offset < 0x100000)
		return m_nvram[offset & (m_nvram.size() - 1)];

	// Bitmap BWRAM
	offset -= 0x100000;

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
	return (m_nvram[offset & (m_nvram.size() - 1)] >> shift) & mask;
}

void sns_sa1_device::write_bwram(UINT32 offset, UINT8 data)
{
	UINT8 mask = 0xff;

	if (m_nvram.empty())
		return; // this should probably never happen, or are there SA-1 games with no BWRAM?

	if (offset < 0x100000)
	{
		m_nvram[offset & (m_nvram.size() - 1)] = data;
		return;
	}

	// Bitmap BWRAM
	offset -= 0x100000;

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
	m_nvram[offset & (m_nvram.size() - 1)] = (m_nvram[offset & (m_nvram.size() - 1)] & ~mask) | data;
}



/*-------------------------------------------------
  Accesses from SNES CPU
 -------------------------------------------------*/


READ8_MEMBER(sns_sa1_device::read_l)
{
	int bank = 0;

	if (offset == 0xffea && BIT(m_scpu_ctrl, 4)) return (m_nmi_vector >> 0) & 0xff;
	if (offset == 0xffeb && BIT(m_scpu_ctrl, 4)) return (m_nmi_vector >> 8) & 0xff;
	if (offset == 0xffee && BIT(m_scpu_ctrl, 6)) return (m_irq_vector >> 0) & 0xff;
	if (offset == 0xffef && BIT(m_scpu_ctrl, 6)) return (m_irq_vector >> 8) & 0xff;

	// ROM is mapped to [00-3f][8000-ffff] only here
	if (offset < 0x200000)
	{
		if (!m_bank_c_hi)   // when HiROM mapping is disabled, we always access first 1MB here
			bank = (offset / 0x10000) + 0x00;
		else    // when HiROM mapping is enabled, we mirror [c0-cf][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_c_rom * 0x20);

		bank &= 0xff;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else if (offset < 0x400000)
	{
		offset -= 0x200000;
		if (!m_bank_d_hi)   // when HiROM mapping is disabled, we always access second 1MB here
			bank = (offset / 0x10000) + 0x20;
		else    // when HiROM mapping is enabled, we mirror [d0-df][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_d_rom * 0x20);

		bank &= 0xff;
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
		if (!m_bank_e_hi)   // when HiROM mapping is disabled, we always access third 1MB here
			bank = (offset / 0x10000) + 0x40;
		else    // when HiROM mapping is enabled, we mirror [e0-ef][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_e_rom * 0x20);

		bank &= 0xff;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else if (offset < 0x400000)
	{
		offset -= 0x200000;
		if (!m_bank_f_hi)   // when HiROM mapping is disabled, we always access fourth 1MB here
			bank = (offset / 0x10000) + 0x60;
		else    // when HiROM mapping is enabled, we mirror [f0-ff][0000-ffff] bank
			bank = (offset / 0x10000) + (m_bank_f_rom * 0x20);

		bank &= 0xff;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else if (offset < 0x500000)
		return m_rom[rom_bank_map[(m_bank_c_rom * 0x20) + ((offset - 0x400000) / 0x8000)] * 0x8000 + (offset & 0x7fff)];
	else if (offset < 0x600000)
		return m_rom[rom_bank_map[(m_bank_d_rom * 0x20) + ((offset - 0x500000) / 0x8000)] * 0x8000 + (offset & 0x7fff)];
	else if (offset < 0x700000)
		return m_rom[rom_bank_map[(m_bank_e_rom * 0x20) + ((offset - 0x600000) / 0x8000)] * 0x8000 + (offset & 0x7fff)];
	else
		return m_rom[rom_bank_map[(m_bank_f_rom * 0x20) + ((offset - 0x700000) / 0x8000)] * 0x8000 + (offset & 0x7fff)];
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
		return read_regs(space, address & 0x1ff);   // SA-1 Regs

	if (offset < 0x400000 && address >= 0x3000 && address < 0x3800)
		return read_iram(address & 0x7ff);  // Internal SA-1 RAM (2K)

	if (offset < 0x400000 && address >= 0x6000 && address < 0x8000)
		return read_bwram((m_bwram_snes * 0x2000) + (offset & 0x1fff)); // SA-1 BWRAM

	if (offset >= 0x400000 && offset < 0x500000)
		return read_bwram(offset & 0xfffff);  // SA-1 BWRAM again (but not called for the [c0-cf] range, because it's not mirrored)

	return 0xff;
}


WRITE8_MEMBER( sns_sa1_device::chip_write )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000 && address >= 0x2200 && address < 0x2400)
		write_regs(space, address & 0x1ff, data);  // SA-1 Regs

	if (offset < 0x400000 && address >= 0x3000 && address < 0x3800)
		write_iram(address & 0x7ff, data);  // Internal SA-1 RAM (2K)

	if (offset < 0x400000 && address >= 0x6000 && address < 0x8000)
		write_bwram((m_bwram_snes * 0x2000) + (offset & 0x1fff), data); // SA-1 BWRAM

	if (offset >= 0x400000 && offset < 0x500000)
		write_bwram(offset & 0xfffff, data);  // SA-1 BWRAM again (but not called for the [c0-cf] range, because it's not mirrored)
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
				return read_iram(offset);   // Internal SA-1 RAM (2K)
			else if (address >= 0x2200 && address < 0x2400)
				return read_regs(space, offset & 0x1ff);    // SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				return read_iram(offset);   // Internal SA-1 RAM (2K)
		}
		else if (address < 0x8000)
			return read_bwram((m_bwram_sa1 * 0x2000) + (offset & 0x1fff) + (m_bwram_sa1_source * 0x100000));        // SA-1 BWRAM
		else
			return read_h(space, offset);   // ROM

		return 0xff;    // maybe open bus? same as the main system one or diff? (currently not accessible from carts anyway...)
	}
	else
		return read_h(space, offset);   // ROM
}

READ8_MEMBER( sns_sa1_device::sa1_lo_r )
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				return read_iram(offset);   // Internal SA-1 RAM (2K)
			else if (address >= 0x2200 && address < 0x2400)
				return read_regs(space, offset & 0x1ff);    // SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				return read_iram(offset);   // Internal SA-1 RAM (2K)
		}
		else if (address < 0x8000)
			return read_bwram((m_bwram_sa1 * 0x2000) + (offset & 0x1fff) + (m_bwram_sa1_source * 0x100000));        // SA-1 BWRAM
		else if (offset == 0xffee)
		{
			return m_sa1_irq & 0xff;
		}
		else if (offset == 0xffef)
		{
			return m_sa1_irq>>8;
		}
		else if (offset == 0xffea)
		{
			return m_sa1_nmi & 0xff;
		}
		else if (offset == 0xffeb)
		{
			return m_sa1_nmi>>8;
		}
		else if (offset == 0xfffc)
		{
			return m_sa1_reset & 0xff;
		}
		else if (offset == 0xfffd)
		{
			return m_sa1_reset>>8;
		}
		else
			return read_l(space, offset);   // ROM

		return 0xff;    // maybe open bus? same as the main system one or diff? (currently not accessible from carts anyway...)
	}
	else if (offset < 0x500000)
		return read_bwram(offset & 0xfffff);      // SA-1 BWRAM (not mirrored above!)
	else if (offset >= 0x600000 && offset < 0x700000)
		return read_bwram((offset & 0xfffff) + 0x100000);       // SA-1 BWRAM Bitmap mode
	else
		return 0xff;    // nothing should be mapped here, so maybe open bus?
}

WRITE8_MEMBER( sns_sa1_device::sa1_hi_w )
{
	UINT16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				write_iram(offset, data);   // Internal SA-1 RAM (2K)
			else if (address >= 0x2200 && address < 0x2400)
				write_regs(space, offset & 0x1ff, data);   // SA-1 Regs
			else if (address >= 0x3000 && address < 0x3800)
				write_iram(offset, data);   // Internal SA-1 RAM (2K)
		}
		else if (address < 0x8000)
			write_bwram((m_bwram_sa1 * 0x2000) + (offset & 0x1fff) + (m_bwram_sa1_source * 0x100000), data);        // SA-1 BWRAM
	}
}

WRITE8_MEMBER( sns_sa1_device::sa1_lo_w )
{
	if (offset >= 0x400000 && offset < 0x500000)
		write_bwram(offset & 0xfffff, data);        // SA-1 BWRAM (not mirrored above!)
	else if (offset >= 0x600000 && offset < 0x700000)
		write_bwram((offset & 0xfffff) + 0x100000, data);       // SA-1 BWRAM Bitmap mode
	else
		sa1_hi_w(space, offset, data);
}

static ADDRESS_MAP_START( sa1_map, AS_PROGRAM, 8, sns_sa1_device )
	AM_RANGE(0x000000, 0x7dffff) AM_READWRITE(sa1_lo_r, sa1_lo_w)
	AM_RANGE(0x7e0000, 0x7fffff) AM_NOP
	AM_RANGE(0x800000, 0xffffff) AM_READWRITE(sa1_hi_r, sa1_hi_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( snes_sa1 )
	MCFG_CPU_ADD("sa1cpu", G65816, 10000000)
	MCFG_CPU_PROGRAM_MAP(sa1_map)
MACHINE_CONFIG_END

machine_config_constructor sns_sa1_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_sa1 );
}
