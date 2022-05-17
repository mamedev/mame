// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, R. Belmont
/***********************************************************************************************************

 SA-1 add-on chip emulation (for SNES/SFC)

 Note:
 - SA-1 register description below is based on nocash docs.
 - Bankswitch handling: no matter what ROM size is used, at loading the ROM is mirrored up to 8MB and a
   rom_bank_map[0x100] array is built as a lookup table for 256x32KB banks filling the 8MB accessible ROM
   area; this allows any value from 0-7 being written to CXB/DXB/EXB/FXB SA-1 registers without any masking.
 - about BWRAM "bitmap mode": in 2-bit mode
     600000h.Bit0-1 mirrors to 400000h.Bit0-1
     600001h.Bit0-1 mirrors to 400000h.Bit2-3
     600002h.Bit0-1 mirrors to 400000h.Bit4-5
     600003h.Bit0-1 mirrors to 400000h.Bit6-7
     ...
   in 4-bit mode
     600000h.Bit0-3 mirrors to 400000h.Bit0-3
     600001h.Bit0-3 mirrors to 400000h.Bit4-7
     600002h.Bit0-3 mirrors to 400001h.Bit0-3
     600003h.Bit0-3 mirrors to 400001h.Bit4-7
     ...
   to handle the separate modes, bitmap accesses go to offset + 0x100000

 TODO:
 - Test case for BWRAM write protect (bsnes does not seem to protect either, so it's not implemented
   for the moment)
 - Almost everything CPU related
 - Bus conflicts

 Compatibility:
    asahishi: plays OK
    daisenx2: plays OK
    derbyjo2: hangs going into game
    dbzhypd, dbzhypdj: plays OK
    habumeij: boots, goes into game, on-screen timer counts down after SA-1 is enabled but controls aren't responsive
    haruaug3a, pebble, haruaug3: plays OK
    itoibass: boots, some missing gfx
    jikkparo: plays OK
    jl96drem: plays OK
    jumpind: boots and runs, uses SA-1 normal DMA only but has corrupt gfx
    kakinoki: S-CPU crashes after pressing start
    kirby3j, kirby3: plays OK
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
    smrpgj, smrpg: plays OK
    srobotg: some corrupt in-game GFX, may be SNES rendering errors
    sshogi3: plays OK
    taikyoid: plays OK
    takemiya: plays OK
 Note: for Igo & Shougi games, "plays OK" means you can get ingame and the CPU replies to your moves; subtle bugs
 might indeed exist.

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

DEFINE_DEVICE_TYPE(SNS_LOROM_SA1, sns_sa1_device, "sns_rom_sa1", "SNES Cart + SA-1")


sns_sa1_device::sns_sa1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SNS_LOROM_SA1, tag, owner, clock)
	, device_sns_cart_interface(mconfig, *this)
	, m_sa1(*this, "sa1cpu")
	, m_sa1_timer(nullptr)
	, m_sa1_ctrl(0), m_sa1_reset_flag(true), m_scpu_sie(0), m_sa1_reset_vector(0), m_sa1_nmi_vector(0), m_sa1_irq_vector(0), m_scpu_ctrl(0), m_sa1_sie(0)
	, m_irq_vector(0), m_nmi_vector(0)
	, m_timer_ctrl(0)
	, m_hpos(0), m_vpos(0)
	, m_hcount(0), m_vcount(0)
	, m_bank_hi{false,false,false,false}, m_bank_rom{0,0,0,0}
	, m_bwram_snes(0), m_bwram_sa1(0), m_bwram_sa1_source(false), m_bwram_sa1_format(false), m_bwram_write_snes(false), m_bwram_write_sa1(false), m_bwpa_sa1(0)
	, m_iram_write_snes(0), m_iram_write_sa1(0)
	, m_dma_ctrl(0), m_dma_ccparam(0), m_src_addr(0), m_dst_addr(0), m_dma_cnt(0)
	, m_math_ctlr(0), m_math_overflow(0), m_math_a(0), m_math_b(0), m_math_res(0)
	, m_vda(0), m_vbit(0), m_vlen(0), m_drm(false), m_scpu_flags(0), m_sa1_flags(0), m_hcr(0), m_vcr(0)
	, m_cconv1_dma_active(false), m_cconv2_line(0)
{
}


void sns_sa1_device::device_start()
{
	m_internal_ram = make_unique_clear<u8[]>(0x800);
	m_sa1_timer = timer_alloc(FUNC(sns_sa1_device::timer_tick), this);

	m_scpu_ctrl = 0;
	m_nmi_vector = 0;
	m_bank_hi[0] = false;
	m_bank_rom[0] = 0;

	save_pointer(NAME(m_internal_ram), 0x800);
	save_item(NAME(m_sa1_ctrl));
	save_item(NAME(m_sa1_reset_flag));
	save_item(NAME(m_scpu_sie));
	save_item(NAME(m_sa1_reset_vector));
	save_item(NAME(m_sa1_nmi_vector));
	save_item(NAME(m_sa1_irq_vector));
	save_item(NAME(m_scpu_ctrl));
	save_item(NAME(m_sa1_sie));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_nmi_vector));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_hpos));
	save_item(NAME(m_vpos));
	save_item(NAME(m_hcount));
	save_item(NAME(m_vcount));
	save_item(NAME(m_bank_hi));
	save_item(NAME(m_bank_rom));
	save_item(NAME(m_bwram_snes));
	save_item(NAME(m_bwram_sa1));
	save_item(NAME(m_bwram_sa1_source));
	save_item(NAME(m_bwram_sa1_format));
	save_item(NAME(m_bwram_write_snes));
	save_item(NAME(m_bwram_write_sa1));
	save_item(NAME(m_bwpa_sa1));
	save_item(NAME(m_iram_write_snes));
	save_item(NAME(m_iram_write_sa1));
	save_item(NAME(m_dma_ctrl));
	save_item(NAME(m_dma_ccparam));
	save_item(NAME(m_src_addr));
	save_item(NAME(m_dst_addr));
	save_item(NAME(m_dma_cnt));
	save_item(NAME(m_brf_reg));
	save_item(NAME(m_math_ctlr));
	save_item(NAME(m_math_overflow));
	save_item(NAME(m_math_a));
	save_item(NAME(m_math_b));
	save_item(NAME(m_math_res));
	save_item(NAME(m_vda));
	save_item(NAME(m_vbit));
	save_item(NAME(m_vlen));
	save_item(NAME(m_drm));
	save_item(NAME(m_scpu_flags));
	save_item(NAME(m_sa1_flags));
	save_item(NAME(m_hcr));
	save_item(NAME(m_vcr));
	save_item(NAME(m_cconv1_dma_active));
	save_item(NAME(m_cconv2_line));
}

void sns_sa1_device::device_reset()
{
	std::fill_n(&m_internal_ram[0], 0x800, 0);

	m_sa1_ctrl = 0x20;
	m_sa1_reset_flag = true;
	m_scpu_ctrl = 0;
	m_irq_vector = 0;
	m_nmi_vector = 0;
	m_timer_ctrl = 0;
	m_hpos = 0;
	m_vpos = 0;
	m_hcount = 0;
	m_vcount = 0;
	m_bank_hi[0] = false;
	m_bank_hi[1] = false;
	m_bank_hi[2] = false;
	m_bank_hi[3] = false;
	m_bank_rom[0] = 0;
	m_bank_rom[1] = 1;
	m_bank_rom[2] = 2;
	m_bank_rom[3] = 3;
	m_bwram_snes = 0;
	m_bwram_sa1 = 0;
	m_bwram_sa1_source = false;
	m_bwram_sa1_format = false;
	m_bwram_write_snes = false;
	m_bwram_write_sa1 = false;
	m_bwpa_sa1 = 0x100 << 0x0f;
	m_iram_write_snes = 0;
	m_iram_write_sa1 = 0;
	m_src_addr = 0;
	m_dst_addr = 0;
	std::fill(std::begin(m_brf_reg), std::end(m_brf_reg), 0);
	m_math_ctlr = 0;
	m_math_overflow = 0;
	m_math_a = 0;
	m_math_b = 0;
	m_math_res = 0;
	m_vda = 0;
	m_vbit = 0;
	m_vlen = 0;
	m_drm = false;
	m_hcr = 0;
	m_vcr = 0;
	m_scpu_sie = m_sa1_sie = 0;
	m_scpu_flags = m_sa1_flags = 0;
	m_dma_ctrl = 0;
	m_dma_ccparam = 0;
	m_dma_cnt = 0;
	m_cconv1_dma_active = false;
	m_cconv2_line = 0;

	// SA-1 CPU starts out not running
	m_sa1->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	// Timer is run in sync with the CPU
	m_sa1_timer->adjust(m_sa1->clocks_to_attotime(2), 0, m_sa1->clocks_to_attotime(2));
}

TIMER_CALLBACK_MEMBER(sns_sa1_device::timer_tick)
{
	if (TMC_HVSELB())
	{
		// 18 bit Linear timer
		m_hpos++;
		m_vpos += m_hpos >> 9;
		m_hpos &= 0x1ff;
		m_vpos &= 0x1ff;
	}
	else
	{
		// H/V timer
		if (++m_hpos >= 341)
		{
			m_hpos = 0;
			if (++m_vpos >= scanlines_r())
				m_vpos = 0;
		}
	}
	// send timer IRQ
	if (TMC_HEN() && TMC_VEN()) // both H and V count enabled
	{
		if ((m_hpos == m_hcount) && (m_vpos == m_vcount))
		{
			m_sa1_flags |= SA1_IRQ_TIMER;
			recalc_irqs();
		}
	}
	else if (TMC_HEN()) // H count only
	{
		if (m_hpos == m_hcount)
		{
			m_sa1_flags |= SA1_IRQ_TIMER;
			recalc_irqs();
		}
	}
	else if (TMC_VEN()) // V count only
	{
		if ((m_hpos == 0) && (m_vpos == m_vcount))
		{
			m_sa1_flags |= SA1_IRQ_TIMER;
			recalc_irqs();
		}
	}

	// TODO: Math & DMA timer
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void sns_sa1_device::recalc_irqs()
{
	if (m_scpu_flags & m_scpu_sie & (SCPU_IRQ_SA1|SCPU_IRQ_CHARCONV))
		write_irq(ASSERT_LINE);
	else
		write_irq(CLEAR_LINE);

	if ((!sa1_halted()) && (m_sa1_flags & m_sa1_sie & (SA1_IRQ_SCPU|SA1_IRQ_TIMER|SA1_IRQ_DMA)))
		m_sa1->set_input_line(G65816_LINE_IRQ, ASSERT_LINE);
	else
		m_sa1->set_input_line(G65816_LINE_IRQ, CLEAR_LINE);

	if ((!sa1_halted()) && (m_sa1_flags & m_sa1_sie & SA1_NMI_SCPU))
		m_sa1->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	else
		m_sa1->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
}


/*-------------------------------------------------
  RAM / SRAM / Registers
 -------------------------------------------------*/


// TODO: Handle this separately to avoid accessing the regs recursively

u8 sns_sa1_device::var_length_read(offs_t offset)
{
	// TODO: memory access cycle
	// handle 0xffea/0xffeb/0xffee/0xffef
	if ((offset & 0xffffe0) == 0x00ffe0)
	{
		if (offset == 0xffea && SCNT_SNESCPU_NVSW()) return (m_nmi_vector >> 0) & 0xff;
		if (offset == 0xffeb && SCNT_SNESCPU_NVSW()) return (m_nmi_vector >> 8) & 0xff;
		if (offset == 0xffee && SCNT_SNESCPU_IVSW()) return (m_irq_vector >> 0) & 0xff;
		if (offset == 0xffef && SCNT_SNESCPU_IVSW()) return (m_irq_vector >> 8) & 0xff;
	}

	if ((offset & 0xc08000) == 0x008000)  //$00-3f:8000-ffff
		return read_l(offset & 0x7fffff);

	if ((offset & 0xc08000) == 0x808000)  //$80-bf:8000-ffff
		return read_h(offset & 0x7fffff);

	if ((offset & 0xc00000) == 0xc00000)  //$c0-ff:0000-ffff
		return read_h(offset & 0x7fffff);

	if ((offset & 0x40e000) == 0x006000)  //$00-3f|80-bf:6000-7fff
		return read_bwram<true>((m_bwram_snes * 0x2000) + (offset & 0x1fff));

	if ((offset & 0xf00000) == 0x400000)  //$40-4f:0000-ffff
		return read_bwram<true>(offset & 0xfffff);

	if ((offset & 0x40f800) == 0x000000)  //$00-3f|80-bf:0000-07ff
		return read_iram(offset);

	if ((offset & 0x40f800) == 0x003000)  //$00-3f|80-bf:3000-37ff
		return read_iram(offset);

	return 0;
}

void sns_sa1_device::dma_transfer()
{
	while (m_dma_cnt--)
	{
		u8 data = 0; // TODO: open bus
		const u32 dma_src = m_src_addr++;
		const u32 dma_dst = m_dst_addr++;

		// source and destination cannot be the same
		// source = { 0=ROM, 1=BWRAM, 2=IRAM }
		// destination = { 0=IRAM, 1=BWRAM }
		if (((DCNT_SD()) == 1) && (DCNT_DD())) continue;
		if (((DCNT_SD()) == 2) && (!(DCNT_DD()))) continue;

		int cycle = 1; // 1 cycle per memory access
		switch (DCNT_SD())
		{
			case 0: // ROM
				if (bus_conflict_rom() && (cycle < 2)) // wait 1 cycle if conflict
					cycle = 2;
				data = rom_r(dma_src & 0xffffff);
				break;

			case 1: // BWRAM
				if (bus_conflict_bwram() && (cycle < 4)) // wait 3 cycle if conflict
					cycle = 4;
				else if (cycle < 2)
					cycle = 2;
				data = read_bwram<true>(dma_src & 0xfffff);
				break;

			case 2: // IRAM
				if (bus_conflict_iram() && (cycle < 3)) // wait 2 cycle if conflict
					cycle = 3;
				data = read_iram(dma_src);
				break;
		}

		if (DCNT_DD())  // BWRAM
		{
			if (bus_conflict_bwram() && (cycle < 4)) // wait 3 cycle if conflict
				cycle = 4;
			else if (cycle < 2)
				cycle = 2;
			write_bwram(dma_dst & 0xfffff, data);
		}
		else  // IRAM
		{
			if (bus_conflict_iram() && (cycle < 3)) // wait 2 cycle if conflict
				cycle = 3;
			write_iram(dma_dst, data);
		}
		m_sa1->adjust_icount(-cycle); // progress
	}

	m_sa1_flags |= SA1_IRQ_DMA;
	recalc_irqs();
}

void sns_sa1_device::dma_cctype1_transfer()
{
	m_cconv1_dma_active = true;
	m_scpu_flags |= SCPU_IRQ_CHARCONV;
	recalc_irqs();
}

void sns_sa1_device::dma_cctype2_transfer()
{
	const u8 bank = BIT(m_cconv2_line, 0) << 3;
	const u8 bpp = 2 << (2 - m_dma_cconv_bits);
	const u32 tx = BIT(m_cconv2_line, 3) << (6 - m_dma_cconv_bits);
	const u32 ty = (BIT(m_cconv2_line, 0, 3) << 1);
	const u32 dst_addr = (m_dst_addr & ~((1 << (7 - m_dma_cconv_bits)) - 1)) + tx + ty;

	// TODO: memory access/process cycle
	for (u8 bit = 0; bit < bpp; bit++)
	{
		u8 byte = 0;
		const offs_t plane = BIT(bit, 0) | (BIT(bit, 1, 2) << 4);
		for (u8 x = 0; x < 8; x++)
			byte |= BIT(m_brf_reg[bank | x], bit) << (7 - x);

		write_iram(dst_addr + plane, byte);
	}

	m_cconv2_line = (m_cconv2_line + 1) & 0xf;
}

u8 sns_sa1_device::host_r(offs_t offset)
{
	u8 value = read_open_bus();
	offset &= 0x1ff;    // $2200 + offset gives the reg value to compare with docs

	switch (offset)
	{
		case 0x100:
			// S-CPU Flag Read
			value = SCNT_CMEG() | m_scpu_flags;
			break;
		case 0x10e:
			// SNES  VC    Version Code Register (R)
			// value = read_open_bus(); // verified
			break;
		default:
			logerror("S-CPU Read access to an unmapped reg (%x)", offset);
			break;
	}
	return value;
}

u8 sns_sa1_device::read_regs(offs_t offset)
{
	u8 value = 0xff; // unverified
	offset &= 0x1ff; // $2200 + offset gives the reg value to compare with docs

	switch (offset)
	{
		case 0x101:
			// SA-1 Flag Read
			value = CCNT_SMEG() | m_sa1_flags;
			break;
		case 0x102:
			// H-Count Read Low
			if (!machine().side_effects_disabled())
			{
				// latch counters
				m_hcr = m_hpos;
				m_vcr = m_vpos;
			}
			// return h-count
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
			// Math Result bits 0-7
			value = (u64)(m_math_res >> 0) & 0xff;
			break;
		case 0x107:
			// Math Result bits 8-15
			value = (u64)(m_math_res >> 8) & 0xff;
			break;
		case 0x108:
			// Math Result bits 16-23
			value = (u64)(m_math_res >> 16) & 0xff;
			break;
		case 0x109:
			// Math Result bits 24-31
			value = (u64)(m_math_res >> 24) & 0xff;
			break;
		case 0x10a:
			// Math Result bits 32-39
			value = (u64)(m_math_res >> 32) & 0xff;
			break;
		case 0x10b:
			// Math Overflow (above 40-bit result)
			value = m_math_overflow;
			break;
		case 0x10c:
			// Var-Length Read Port Low
			{
				u32 data = (var_length_read(m_vda + 0) <<  0) | (var_length_read(m_vda + 1) <<  8) | (var_length_read(m_vda + 2) << 16);
				data >>= m_vbit;
				value = (data >> 0) & 0xff;
			}
			break;
		case 0x10d:
			// Var-Length Read Port High
			{
				u32 data = (var_length_read(m_vda + 0) <<  0) | (var_length_read(m_vda + 1) <<  8) | (var_length_read(m_vda + 2) << 16);
				data >>= m_vbit;

				if (!machine().side_effects_disabled())
				{
					if (m_drm)
					{
						// auto-increment mode
						m_vbit += m_vlen;
						m_vda += (m_vbit >> 3);
						m_vbit &= 7;
					}
				}

				value = (data >> 8) & 0xff;
			}
			break;
		default:
			logerror("SA-1 Read access to an unmapped reg (%x)", offset);
			break;
	}
	return value;
}

void sns_sa1_device::host_w(offs_t offset, u8 data)
{
	offset &= 0x1ff;    // $2200 + offset gives the reg value to compare with docs

	switch (offset)
	{
		case 0x000:
			// SA-1 control flags
			if (CCNT_SA1_CPU_RDYB() && BIT(data, 5)) // Pull up reset pin
				m_sa1_reset_flag = true;

			if ((BIT(data, 5, 2) != 0) && (BIT(m_sa1_ctrl, 5, 2) == 0))
			{
				m_sa1->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
				if (BIT(data, 5)) // Pull up reset pin
					m_sa1_reset_flag = true;
			}
			else if ((BIT(data, 5, 2) == 0) && (BIT(m_sa1_ctrl, 5, 2) != 0))
			{
				m_sa1->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				if (m_sa1_reset_flag && (!BIT(data, 5)))
				{
					m_sa1->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
					m_sa1->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
					m_sa1_reset_flag = false;
				}
			}

			m_sa1_ctrl = data;

			if (CCNT_SA1_CPU_IRQ())
				m_sa1_flags |= SA1_IRQ_SCPU;

			if (CCNT_SA1_CPU_NMI())
				m_sa1_flags |= SA1_NMI_SCPU;

			recalc_irqs();
			break;
		case 0x001:
			// SNES CPU Int Enable
			m_scpu_sie = data;
			recalc_irqs();
			break;
		case 0x002:
			// SNES CPU Int Clear
			if (BIT(data, 7))   // ack IRQ from SA-1
				m_scpu_flags &= ~SCPU_IRQ_SA1;

			if (BIT(data, 5))   // ack character conversion IRQ
				m_scpu_flags &= ~SCPU_IRQ_CHARCONV;

			recalc_irqs();
			break;
		case 0x003:
			// SA-1 CPU Reset Vector LSB
			m_sa1_reset_vector &= 0xff00;
			m_sa1_reset_vector |= data;
			break;
		case 0x004:
			// SA-1 CPU Reset Vector MSB
			m_sa1_reset_vector &= 0x00ff;
			m_sa1_reset_vector |= (data<<8);
			break;
		case 0x005:
			// SA-1 CPU NMI Vector LSB
			m_sa1_nmi_vector &= 0xff00;
			m_sa1_nmi_vector |= data;
			break;
		case 0x006:
			// SA-1 CPU NMI Vector MSB
			m_sa1_nmi_vector &= 0x00ff;
			m_sa1_nmi_vector |= (data<<8);
			break;
		case 0x007:
			// SA-1 CPU IRQ Vector LSB
			m_sa1_irq_vector &= 0xff00;
			m_sa1_irq_vector |= data;
			break;
		case 0x008:
			// SA-1 CPU IRQ Vector MSB
			m_sa1_irq_vector &= 0x00ff;
			m_sa1_irq_vector |= (data<<8);
			break;
		case 0x020:
			// Super MMC Bank C
			m_bank_hi[0] = BIT(data, 7); // [00-1f][8000-ffff] is mirror of [c0-cf][0000-ffff] bank or first 1MB of ROM
			m_bank_rom[0] = data & 0x07; // ROM 1MB bank for [c0-cf]
			break;
		case 0x021:
			// Super MMC Bank D
			m_bank_hi[1] = BIT(data, 7); // [20-3f][8000-ffff] is mirror of [d0-df][0000-ffff] bank or second 1MB of ROM
			m_bank_rom[1] = data & 0x07; // ROM 1MB bank for [d0-df]
			break;
		case 0x022:
			// Super MMC Bank E
			m_bank_hi[2] = BIT(data, 7); // [80-9f][8000-ffff] is mirror of [e0-ef][0000-ffff] bank or third 1MB of ROM
			m_bank_rom[2] = data & 0x07; // ROM 1MB bank for [e0-ef]
			break;
		case 0x023:
			// Super MMC Bank F
			m_bank_hi[3] = BIT(data, 7); // [a0-bf][8000-ffff] is mirror of [e0-ef][0000-ffff] bank or fourth 1MB of ROM
			m_bank_rom[3] = data & 0x07; // ROM 1MB bank for [f0-ff]
			break;
		case 0x024:
			// BWRAM bank from SNES side
			m_bwram_snes = data & 0x1f; // max 32x8K banks
			break;
		case 0x026:
			// enable writing to BWRAM from SNES
			m_bwram_write_snes = BIT(data, 7);
			break;
		case 0x028:
			// write protected area at bottom of BWRAM
			m_bwpa_sa1 = 0x100 << (data & 0x0f);
			break;
		case 0x029:
			// enable writing to IRAM from SNES (1 bit for each 0x100 chunk)
			m_iram_write_snes = data;
			break;
		case 0x031: // Character Conversion DMA Parameters
		case 0x032: // DMA Source Device Start Address Low
		case 0x033: // DMA Source Device Start Address Mid
		case 0x034: // DMA Source Device Start Address High
		case 0x035: // DMA Dest Device Start Address Low
		case 0x036: // DMA Dest Device Start Address Mid
		case 0x037: // DMA Dest Device Start Address High
			shared_regs_w(offset, data);
			break;
		default:
			logerror("S-CPU Write access to an unmapped reg (%x) with data %x", offset, data);
			break;
	}
}

void sns_sa1_device::write_regs(offs_t offset, u8 data)
{
	offset &= 0x1ff;    // $2200 + offset gives the reg value to compare with docs

	switch (offset)
	{
		case 0x009:
			// S-CPU control flags
			m_scpu_ctrl = data;
			if (SCNT_SNESCPU_IRQ())
			{
				m_scpu_flags |= SCPU_IRQ_SA1;
			}

			// acquire IRQ/NMI override flags from data
			m_scpu_flags &= ~(SCPU_IRQV_ALT|SCPU_NMIV_ALT);
			m_scpu_flags |= (data & (SCPU_IRQV_ALT|SCPU_NMIV_ALT));

			recalc_irqs();
			break;
		case 0x00a:
			// SA-1 CPU Int Enable
			m_sa1_sie = data;
			recalc_irqs();
			break;
		case 0x00b:
			// SA-1 CPU Int Clear
			if (BIT(data, 7))
				m_sa1_flags &= ~SA1_IRQ_SCPU;

			if (BIT(data, 6))
				m_sa1_flags &= ~SA1_IRQ_TIMER;

			if (BIT(data, 5))
				m_sa1_flags &= ~SA1_IRQ_DMA;

			if (BIT(data, 4))
				m_sa1_flags &= ~SA1_NMI_SCPU;

			recalc_irqs();
			break;
		case 0x00c:
			// SA-1 NMI Vector Low
			m_nmi_vector = (m_nmi_vector & 0xff00) | (data << 0);
			break;
		case 0x00d:
			// SA-1 NMI Vector High
			m_nmi_vector = (m_nmi_vector & 0x00ff) | (data << 8);
			break;
		case 0x00e:
			// SA-1 IRQ Vector Low
			m_irq_vector = (m_irq_vector & 0xff00) | (data << 0);
			break;
		case 0x00f:
			// SA-1 IRQ Vector High
			m_irq_vector = (m_irq_vector & 0x00ff) | (data << 8);
			break;
		case 0x010:
			// H/V Timer Control
			m_timer_ctrl = data;
			break;
		case 0x011:
			// CPU Timer Restart
			m_hpos = m_vpos = 0;
			m_sa1_timer->adjust(m_sa1->clocks_to_attotime(2), 0, m_sa1->clocks_to_attotime(2));
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
		case 0x025:
			// BWRAM bank & type from SA-1 side
			m_bwram_sa1_source = BIT(data, 7);  // 0 = normal, 1 = bitmap
			m_bwram_sa1 = data & 0x7f;  // up to 128x8K banks here
			break;
		case 0x027:
			// enable writing to BWRAM from SA-1
			m_bwram_write_sa1 = BIT(data, 7);
			break;
		case 0x02a:
			// enable writing to IRAM from SA-1 (1 bit for each 0x100 chunk)
			m_iram_write_sa1 = data;
			break;
		case 0x030:
			// DMA Control (W)
			m_dma_ctrl = data;
			break;
		case 0x031: // Character Conversion DMA Parameters (W)
		case 0x032: // DMA Source Device Start Address Low
		case 0x033: // DMA Source Device Start Address Mid
		case 0x034: // DMA Source Device Start Address High
		case 0x035: // DMA Dest Device Start Address Low
		case 0x036: // DMA Dest Device Start Address Mid
		case 0x037: // DMA Dest Device Start Address High
			shared_regs_w(offset, data);
			break;
		case 0x038:
			// DMA Terminal Counter LSB
			m_dma_cnt &= 0xff00;
			m_dma_cnt |= data;
			break;
		case 0x039:
			// DMA Terminal Counter MSB
			m_dma_cnt &= 0x00ff;
			m_dma_cnt |= (data<<8);
			break;
		case 0x03f:
			// Format for BWRAM when mapped to bitmap
			m_bwram_sa1_format = BIT(data, 7);  // 0 = 4-bit, 1 = 2-bit
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
			if (((offset & 0x07) == 7) && DCNT_DMAEN())
			{
				if (DCNT_CDEN() && (!(DCNT_CDSEL())))  // CC DMA Type 2
				{
					dma_cctype2_transfer();
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
			// After Math B has been written, perform the operation
			switch (m_math_ctlr)
			{
				case 0: // signed multiplication (5 cycles required)
					m_math_res = (s16)m_math_a * (s16)m_math_b;
					m_math_b = 0;
					break;
				case 1: // unsigned division (5 cycles required)
					if (m_math_b == 0)
						m_math_res = 0;
					else
					{
						s16  quotient  = (s16)m_math_a / (u16)m_math_b;
						u16 remainder = (s16)m_math_a % (u16)m_math_b;
						m_math_res = (u64)((remainder << 16) | quotient);
					}
					break;
				case 2: // sigma (accumulative multiplication) (6 cycles required)
				case 3:
					u64 acum = (s16)m_math_a * (s16)m_math_b;
					u64 mask = 0xffffffffffU;
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

			if (!m_drm)
			{
				// fixed mode
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

void sns_sa1_device::shared_regs_w(offs_t offset, u8 data)
{
	offset &= 0x1ff;    // $2200 + offset gives the reg value to compare with docs

	switch (offset)
	{
		case 0x031:
			// Character Conversion DMA Parameters (W)
			m_dma_ccparam = data;
			m_dma_cconv_size = CDMA_SIZE();
			m_dma_cconv_bits = CDMA_CB();
			if (CDMA_CHDEND())
				m_cconv1_dma_active = false;
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
			if (DCNT_DMAEN())
			{
				if ((!(DCNT_CDEN())) && (!(DCNT_DD()))) // Normal DMA to IRAM
					dma_transfer();

				if (DCNT_CDEN() && DCNT_CDSEL()) // CC DMA Type 1
					dma_cctype1_transfer();
			}
			break;
		case 0x037:
			// DMA Dest Device Start Address High
			m_dst_addr = (m_dst_addr & 0x00ffff) | (data << 16);
			if (DCNT_DMAEN())
			{
				if ((!(DCNT_CDEN())) && DCNT_DD())  // Normal DMA to BWRAM
				{
					dma_transfer();
				}
			}
			break;
		default:
			logerror("SA-1 Write access to an unmapped reg (%x) with data %x", offset, data);
			break;
	}
}

u8 sns_sa1_device::read_iram(offs_t offset)
{
	return m_internal_ram[offset & 0x7ff];
}

void sns_sa1_device::write_iram(offs_t offset, u8 data)
{
	m_internal_ram[offset & 0x7ff] = data;
}

u8 sns_sa1_device::read_cconv1_dma(offs_t offset)
{
	const u32 store_mask = (1 << (6 - m_dma_cconv_bits)) - 1;

	if (!machine().side_effects_disabled())
	{
		if ((offset & store_mask) == 0)
		{
			const u32 bpp = 2 << (2 - m_dma_cconv_bits);
			const u32 tile_stride = (8 << m_dma_cconv_size) >> m_dma_cconv_bits;
			const u32 bwram_addr_mask = m_nvram.size() - 1;
			const u32 tile = ((offset - m_src_addr) & bwram_addr_mask) >> (6 - m_dma_cconv_bits);
			const u32 ty = (tile >> m_dma_cconv_size);
			const u32 tx = tile & ((1 << m_dma_cconv_size) - 1);
			u32 bwram_src = m_src_addr + ty * 8 * tile_stride + tx * bpp;

			// TODO: memory access/process cycle
			for (u32 y = 0; y < 8; y++)
			{
				u64 raw_pixels = 0;
				for (u64 bit = 0; bit < bpp; bit++)
					raw_pixels |= (u64)m_nvram[(bwram_src + bit) & bwram_addr_mask] << (bit << 3);

				bwram_src += tile_stride;

				u8 linear[8] = {0, 0, 0, 0, 0, 0, 0, 0};
				for (u8 x = 0; x < 8; x++)
				{
					for (u8 bit = 0; bit < bpp; bit++)
						linear[bit] |= BIT(raw_pixels, bit) << (7 - x);

					raw_pixels >>= bpp;
				}

				const u32 dy = (y << 1);
				for (u8 byte = 0; byte < bpp; byte++)
				{
					const u32 plane = BIT(byte, 0) | (BIT(byte, 1, 2) << 4);
					write_iram(m_dst_addr + dy + plane, linear[byte]);
				}
			}
		}
	}

	return read_iram(m_dst_addr + (offset & store_mask));
}

template<bool SA1Read>
u8 sns_sa1_device::read_bwram(offs_t offset, bool bitmap)
{
	if (m_nvram.empty())
		return 0xff;    // this should probably never happen, or are there SA-1 games with no BWRAM?

	if (m_cconv1_dma_active && !SA1Read)
		return read_cconv1_dma(offset);

	const offs_t bwram_addr_mask = (m_nvram.size() - 1);
	if (!bitmap)
		return m_nvram[offset & bwram_addr_mask];

	// Bitmap BWRAM
	u8 shift, mask;

	if (m_bwram_sa1_format)
	{
		// 2-bit mode
		shift = ((offset & 3) << 1);
		mask = 0x03;
		offset >>= 2;
	}
	else
	{
		// 4-bit mode
		shift = ((offset & 1) << 2);
		mask = 0x0f;
		offset >>= 1;
	}

	// only return the correct bits
	return (m_nvram[offset & bwram_addr_mask] >> shift) & mask;
}

void sns_sa1_device::write_bwram(offs_t offset, u8 data, bool bitmap)
{
	if (m_nvram.empty())
		return; // this should probably never happen, or are there SA-1 games with no BWRAM?

	const offs_t bwram_addr_mask = (m_nvram.size() - 1);
	if (!bitmap)
	{
		m_nvram[offset & bwram_addr_mask] = data;
		return;
	}

	// Bitmap BWRAM
	u8 mask;

	if (m_bwram_sa1_format)
	{
		// 2-bit mode
		data = (data & 0x03) << ((offset & 3) << 1);
		mask = 0x03 << ((offset & 3) << 1);
		offset >>= 2;
	}
	else
	{
		// 4-bit mode
		data = (data & 0x0f) << ((offset & 1) << 2);
		mask = 0x0f << ((offset & 1) << 2);
		offset >>= 1;
	}

	// only change the correct bits, keeping the rest untouched
	m_nvram[offset & bwram_addr_mask] = (m_nvram[offset & bwram_addr_mask] & ~mask) | data;
}



/*-------------------------------------------------
  Accesses from SNES CPU
 -------------------------------------------------*/


u8 sns_sa1_device::rom_r(offs_t offset)
{
	u8 ret = 0; // TODO: unverified & unknown value

	if ((offset & 0xc00000) == 0xc00000) // [c0-ff][0000-ffff]
		ret = m_rom[(rom_bank_map[(m_bank_rom[BIT(offset, 20, 2)] << 5) | BIT(offset, 15, 5)] << 15) | (offset & 0x7fff)];
	else if ((offset & 0x408000) == 0x008000) // [00-3f][8000-ffff], [80-bf][8000-ffff]
	{
		const u8 slot = (BIT(offset, 23) << 1) | BIT(offset, 21);
		u8 bank; // 256 banks (64 Mbit limited)

		if (!m_bank_hi[slot]) // when HiROM mapping is disabled, we always access each 1MB here
			bank = BIT(offset, 16, 5) | (slot << 5);
		else // when HiROM mapping is enabled, we mirror [(cx,dx,ex,fx)][0000-ffff] bank
			bank = BIT(offset, 16, 5) | (m_bank_rom[slot] << 5);

		ret = m_rom[(rom_bank_map[bank] << 15) | (offset & 0x7fff)];
	}
	return ret;
}

u8 sns_sa1_device::read_l(offs_t offset)
{
	if (offset == 0xffea && SCNT_SNESCPU_NVSW()) return (m_nmi_vector >> 0) & 0xff;
	if (offset == 0xffeb && SCNT_SNESCPU_NVSW()) return (m_nmi_vector >> 8) & 0xff;
	if (offset == 0xffee && SCNT_SNESCPU_IVSW()) return (m_irq_vector >> 0) & 0xff;
	if (offset == 0xffef && SCNT_SNESCPU_IVSW()) return (m_irq_vector >> 8) & 0xff;

	// ROM is mapped to [00-3f][8000-ffff] only here
	if (offset < 0x400000)
		return rom_r(offset);
	else
		return 0; // this should not happen (the driver should only call read_l in the above case)
}

u8 sns_sa1_device::read_h(offs_t offset)
{
	// ROM is mapped to [80-bf][8000-ffff] & [c0-ff][0000-ffff]
	return rom_r(offset | 0x800000);
}

void sns_sa1_device::write_l(offs_t offset, u8 data)
{
}

void sns_sa1_device::write_h(offs_t offset, u8 data)
{
}

u8 sns_sa1_device::chip_read(offs_t offset)
{
	u16 address = offset & 0xffff;

	if (offset < 0x400000 && address >= 0x2200 && address < 0x2400)
		return host_r(address & 0x1ff); // SA-1 Regs

	if (offset < 0x400000 && address >= 0x3000 && address < 0x3800)
		return read_iram(address & 0x7ff);

	if (offset < 0x400000 && address >= 0x6000 && address < 0x8000)
		return read_bwram<false>((m_bwram_snes * 0x2000) + (offset & 0x1fff));

	if (offset >= 0x400000 && offset < 0x500000)
		return read_bwram<false>(offset & 0xfffff);  // SA-1 BWRAM again (but not called for the [c0-cf] range, because it's not mirrored)

	return 0xff;
}


void sns_sa1_device::chip_write(offs_t offset, u8 data)
{
	u16 address = offset & 0xffff;

	if (offset < 0x400000 && address >= 0x2200 && address < 0x2400)
		host_w(address & 0x1ff, data); // SA-1 Regs

	if (offset < 0x400000 && address >= 0x3000 && address < 0x3800)
	{
		if (BIT(m_iram_write_snes, BIT(address, 8, 3)))
			write_iram(address & 0x7ff, data);
	}

	if (offset < 0x400000 && address >= 0x6000 && address < 0x8000)
		write_bwram((m_bwram_snes * 0x2000) + (offset & 0x1fff), data);

	if (offset >= 0x400000 && offset < 0x500000)
		write_bwram(offset & 0xfffff, data);  // SA-1 BWRAM again (but not called for the [c0-cf] range, because it's not mirrored)
}


/*-------------------------------------------------
  Accesses from SA-1 CPU
 -------------------------------------------------*/

// These handlers basically match the SNES CPU ones, but there is no access to internal
// I/O regs or WRAM, and there are a few additional accesses to IRAM (in [00-3f][0000-07ff])
// and to BWRAM (in [60-6f][0000-ffff], so-called bitmap mode)

u8 sns_sa1_device::sa1_rom_r(offs_t offset)
{
	if (bus_conflict_rom())
		m_sa1->adjust_icount(-1); // wait 1 cycle

	return rom_r(offset);
}

u8 sns_sa1_device::sa1_iram_r(offs_t offset)
{
	if (bus_conflict_iram())
		m_sa1->adjust_icount(-2); // wait 2 cycles
	return read_iram(offset & 0x7ff);
}

void sns_sa1_device::sa1_iram_w(offs_t offset, u8 data)
{
	if (bus_conflict_iram())
		m_sa1->adjust_icount(-2); // wait 2 cycles

	if (BIT(m_iram_write_sa1, BIT(offset, 8, 3)))
		write_iram(offset & 0x7ff, data);
}

u8 sns_sa1_device::sa1_bwram_r(offs_t offset, bool bitmap)
{
	if (bus_conflict_bwram())
		m_sa1->adjust_icount(-3); // wait 3 cycles
	else
		m_sa1->adjust_icount(-1); // wait 1 cycle

	return read_bwram<true>(offset, bitmap);
}

void sns_sa1_device::sa1_bwram_w(offs_t offset, u8 data, bool bitmap)
{
	if (bus_conflict_bwram())
		m_sa1->adjust_icount(-3); // wait 3 cycles
	else
		m_sa1->adjust_icount(-1); // wait 1 cycle

	// TODO: write protectable?
	write_bwram(offset, data, bitmap);
}

u8 sns_sa1_device::sa1_hi_r(offs_t offset)
{
	u16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				return sa1_iram_r(offset);
			else if (address >= 0x2200 && address < 0x2400)
				return read_regs(offset & 0x1ff);
			else if (address >= 0x3000 && address < 0x3800)
				return sa1_iram_r(offset);
		}
		else if (address < 0x8000)
			return sa1_bwram_r((m_bwram_sa1 * 0x2000) + (offset & 0x1fff), m_bwram_sa1_source);
		else
			return sa1_rom_r(offset | 0x800000);

		return 0xff; // TOOD: Maybe open bus. Check if same as the main system or different (currently not accessible from carts anyway).
	}
	else
		return sa1_rom_r(offset | 0xc00000);
}

u8 sns_sa1_device::sa1_lo_r(offs_t offset)
{
	u16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				return sa1_iram_r(offset);
			else if (address >= 0x2200 && address < 0x2400)
				return read_regs(offset & 0x1ff);
			else if (address >= 0x3000 && address < 0x3800)
				return sa1_iram_r(offset);
		}
		else if (address < 0x8000)
			return sa1_bwram_r((m_bwram_sa1 * 0x2000) + (offset & 0x1fff), m_bwram_sa1_source); // SA-1 BWRAM
		else
		{
			if (bus_conflict_rom())
				m_sa1->adjust_icount(-1); // wait 1 cycle

			if (offset == 0xffee)
			{
				return m_sa1_irq_vector & 0xff;
			}
			else if (offset == 0xffef)
			{
				return m_sa1_irq_vector >> 8;
			}
			else if (offset == 0xffea)
			{
				return m_sa1_nmi_vector & 0xff;
			}
			else if (offset == 0xffeb)
			{
				return m_sa1_nmi_vector >> 8;
			}
			else if (offset == 0xfffc)
			{
				return m_sa1_reset_vector & 0xff;
			}
			else if (offset == 0xfffd)
			{
				return m_sa1_reset_vector >> 8;
			}
			else
				return rom_r(offset);
		}

		return 0xff; // TOOD: Maybe open bus. Check if same as the main system or different (currently not accessible from carts anyway).
	}
	else if (offset < 0x500000)
		return sa1_bwram_r(offset & 0xfffff, false);      // SA-1 BWRAM (not mirrored above)
	else if (offset >= 0x600000 && offset < 0x700000)
		return sa1_bwram_r(offset & 0xfffff, true);       // SA-1 BWRAM Bitmap mode
	else
		return 0xff;    // nothing should be mapped here, so maybe open bus
}

void sns_sa1_device::sa1_hi_w(offs_t offset, u8 data)
{
	u16 address = offset & 0xffff;
	if (offset < 0x400000)
	{
		if (address < 0x6000)
		{
			if (address < 0x0800)
				sa1_iram_w(offset, data);
			else if (address >= 0x2200 && address < 0x2400)
				write_regs(offset & 0x1ff, data);
			else if (address >= 0x3000 && address < 0x3800)
				sa1_iram_w(offset, data);
		}
		else if (address < 0x8000)
			sa1_bwram_w((m_bwram_sa1 * 0x2000) + (offset & 0x1fff), data, m_bwram_sa1_source);
	}
}

void sns_sa1_device::sa1_lo_w(offs_t offset, u8 data)
{
	if (offset >= 0x400000 && offset < 0x500000)
		sa1_bwram_w(offset & 0xfffff, data, false);	// SA-1 BWRAM (not mirrored above)
	else if (offset >= 0x600000 && offset < 0x700000)
		sa1_bwram_w(offset & 0xfffff, data, true);	// SA-1 BWRAM Bitmap mode
	else
		sa1_hi_w(offset, data);
}

void sns_sa1_device::sa1_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(sns_sa1_device::sa1_lo_r), FUNC(sns_sa1_device::sa1_lo_w));
	map(0x7e0000, 0x7fffff).noprw();
	map(0x800000, 0xffffff).rw(FUNC(sns_sa1_device::sa1_hi_r), FUNC(sns_sa1_device::sa1_hi_w));
}


void sns_sa1_device::device_add_mconfig(machine_config &config)
{
	G65816(config, m_sa1, DERIVED_CLOCK(1,2)); // Nintendo SA1 RF5A123, 10.738636MHz (21.477272MHz XTAL / 2)
	m_sa1->set_addrmap(AS_PROGRAM, &sns_sa1_device::sa1_map);
}
