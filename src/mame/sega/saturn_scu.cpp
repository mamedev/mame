// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Sega System Control Unit (c) 1995 Sega

TODO:
- make a screen_device subclass, add pixel timers & irq callbacks to it;
- A-Bus external interrupts;
- Pad signal (lightgun I presume);
- Improve DMA, use DRQ model and timers, make them subdevices?
(old DMA TODO)
- Remove CD transfer DMA hack (tied with CD block bug(s)?)
- Add timings(but how fast are each DMA?).
- Add level priority & DMA status register.

***************************************************************************/
/**********************************************************************************
SCU Register Table
offset,relative address
Registers are in long words.
===================================================================================
0     0000  Level 0 DMA Set Register
1     0004
2     0008
3     000c
4     0010
5     0014
6     0018
7     001c
8     0020  Level 1 DMA Set Register
9     0024
10    0028
11    002c
12    0030
13    0034
14    0038
15    003c
16    0040  Level 2 DMA Set Register
17    0044
18    0048
19    004c
20    0050
21    0054
22    0058
23    005c
24    0060  DMA Forced Stop
25    0064
26    0068
27    006c
28    0070  <Free>
29    0074
30    0078
31    007c  DMA Status Register
32    0080  DSP Program Control Port
33    0084  DSP Program RAM Data Port
34    0088  DSP Data RAM Address Port
35    008c  DSP Data RAM Data Port
36    0090  Timer 0 Compare Register
37    0094  Timer 1 Set Data Register
38    0098  Timer 1 Mode Register
39    009c  <Free>
40    00a0  Interrupt Mask Register
41    00a4  Interrupt Status Register
42    00a8  A-Bus Interrupt Acknowledge
43    00ac  <Free>
44    00b0  A-Bus Set Register
45    00b4
46    00b8  A-Bus Refresh Register
47    00bc  <Free>
48    00c0
49    00c4  SCU SDRAM Select Register
50    00c8  SCU Version Register
51    00cc  <Free>
52    00cf
===================================================================================
DMA Status Register(32-bit):
xxxx xxxx x--- xx-- xx-- xx-- xx-- xx-- UNUSED
---- ---- -x-- ---- ---- ---- ---- ---- DMA DSP-Bus access
---- ---- --x- ---- ---- ---- ---- ---- DMA B-Bus access
---- ---- ---x ---- ---- ---- ---- ---- DMA A-Bus access
---- ---- ---- --x- ---- ---- ---- ---- DMA lv 1 interrupt
---- ---- ---- ---x ---- ---- ---- ---- DMA lv 0 interrupt
---- ---- ---- ---- --x- ---- ---- ---- DMA lv 2 in stand-by
---- ---- ---- ---- ---x ---- ---- ---- DMA lv 2 in operation
---- ---- ---- ---- ---- --x- ---- ---- DMA lv 1 in stand-by
---- ---- ---- ---- ---- ---x ---- ---- DMA lv 1 in operation
---- ---- ---- ---- ---- ---- --x- ---- DMA lv 0 in stand-by
---- ---- ---- ---- ---- ---- ---x ---- DMA lv 0 in operation
---- ---- ---- ---- ---- ---- ---- --x- DSP side DMA in stand-by
---- ---- ---- ---- ---- ---- ---- ---x DSP side DMA in operation

**********************************************************************************/

#include "emu.h"
#include "saturn_scu.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SATURN_SCU, saturn_scu_device, "saturn_scu", "Sega Saturn System Control Unit")

//-------------------------------------------------
//  saturn_scu_device - constructor
//-------------------------------------------------

saturn_scu_device::saturn_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATURN_SCU, tag, owner, clock),
	m_scudsp(*this, "scudsp"),
	m_hostcpu(*this, finder_base::DUMMY_TAG)
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//map(0x0000, 0x0003) src
//map(0x0004, 0x0007) dst
//map(0x0008, 0x000b) size
//map(0x000c, 0x000f) src/dst add values
//map(0x0010, 0x0013) DMA enable
//map(0x0014, 0x0017) DMA start factor

void saturn_scu_device::regs_map(address_map &map)
{
	map(0x0000, 0x0017).rw(FUNC(saturn_scu_device::dma_lv0_r), FUNC(saturn_scu_device::dma_lv0_w));
	map(0x0020, 0x0037).rw(FUNC(saturn_scu_device::dma_lv1_r), FUNC(saturn_scu_device::dma_lv1_w));
	map(0x0040, 0x0057).rw(FUNC(saturn_scu_device::dma_lv2_r), FUNC(saturn_scu_device::dma_lv2_w));
	// Super Major League and Shin Megami Tensei - Akuma Zensho reads from there (undocumented), DMA status mirror?
	map(0x005c, 0x005f).r(FUNC(saturn_scu_device::dma_status_r));
//  map(0x0060, 0x0063).w(FUNC(saturn_scu_device::dma_force_stop_w));
	map(0x007c, 0x007f).r(FUNC(saturn_scu_device::dma_status_r));
	map(0x0080, 0x0083).rw(m_scudsp, FUNC(scudsp_cpu_device::program_control_r), FUNC(scudsp_cpu_device::program_control_w));
	map(0x0084, 0x0087).w(m_scudsp, FUNC(scudsp_cpu_device::program_w));
	map(0x0088, 0x008b).w(m_scudsp, FUNC(scudsp_cpu_device::ram_address_control_w));
	map(0x008c, 0x008f).rw(m_scudsp, FUNC(scudsp_cpu_device::ram_address_r), FUNC(scudsp_cpu_device::ram_address_w));
	map(0x0090, 0x0093).w(FUNC(saturn_scu_device::t0_compare_w));
	map(0x0094, 0x0097).w(FUNC(saturn_scu_device::t1_setdata_w));
	map(0x009a, 0x009b).w(FUNC(saturn_scu_device::t1_mode_w));
	map(0x00a0, 0x00a3).rw(FUNC(saturn_scu_device::irq_mask_r), FUNC(saturn_scu_device::irq_mask_w));
	map(0x00a4, 0x00a7).rw(FUNC(saturn_scu_device::irq_status_r), FUNC(saturn_scu_device::irq_status_w));
//  map(0x00a8, 0x00ab).w(FUNC(saturn_scu_device::abus_irqack_w));
//  map(0x00b0, 0x00b7).rw(FUNC(saturn_scu_device::abus_set_r), FUNC(saturn_scu_device::abus_set_w));
//  map(0x00b8, 0x00bb).rw(FUNC(saturn_scu_device::abus_refresh_r), FUNC(saturn_scu_device::abus_refresh_w));
//  map(0x00c4, 0x00c7).rw(FUNC(saturn_scu_device::sdram_r), FUNC(saturn_scu_device::sdram_w));
	map(0x00c8, 0x00cb).r(FUNC(saturn_scu_device::version_r));
}


//-------------------------------------------------
//  add_device_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

uint16_t saturn_scu_device::scudsp_dma_r(offs_t offset, uint16_t mem_mask)
{
	//address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = offset;

//  printf("%08x\n",addr);

	return m_hostspace->read_word(addr,mem_mask);
}


void saturn_scu_device::scudsp_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = offset;

//  printf("%08x %02x\n",addr,data);

	m_hostspace->write_word(addr, data,mem_mask);
}

void saturn_scu_device::device_add_mconfig(machine_config &config)
{
	SCUDSP(config, m_scudsp, XTAL(57'272'727) / 4); // 14 MHz
	m_scudsp->out_irq_callback().set(DEVICE_SELF, FUNC(saturn_scu_device::scudsp_end_w));
	m_scudsp->in_dma_callback().set(FUNC(saturn_scu_device::scudsp_dma_r));
	m_scudsp->out_dma_callback().set(FUNC(saturn_scu_device::scudsp_dma_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_scu_device::device_start()
{
	save_item(NAME(m_ist));
	save_item(NAME(m_ism));
	save_item(NAME(m_t0c));
	save_item(NAME(m_t1s));
	save_item(NAME(m_t1md));

	save_item(NAME(m_dma[0].src));
	save_item(NAME(m_dma[0].dst));
	save_item(NAME(m_dma[0].src_add));
	save_item(NAME(m_dma[0].dst_add));
	save_item(NAME(m_dma[0].size));
	save_item(NAME(m_dma[0].index));
	save_item(NAME(m_dma[0].start_factor));
	save_item(NAME(m_dma[0].enable_mask));
	save_item(NAME(m_dma[0].indirect_mode));
	save_item(NAME(m_dma[0].rup));
	save_item(NAME(m_dma[0].wup));
	save_item(NAME(m_dma[1].src));
	save_item(NAME(m_dma[1].dst));
	save_item(NAME(m_dma[1].src_add));
	save_item(NAME(m_dma[1].dst_add));
	save_item(NAME(m_dma[1].size));
	save_item(NAME(m_dma[1].index));
	save_item(NAME(m_dma[1].start_factor));
	save_item(NAME(m_dma[1].enable_mask));
	save_item(NAME(m_dma[1].indirect_mode));
	save_item(NAME(m_dma[1].rup));
	save_item(NAME(m_dma[1].wup));
	save_item(NAME(m_dma[2].src));
	save_item(NAME(m_dma[2].dst));
	save_item(NAME(m_dma[2].src_add));
	save_item(NAME(m_dma[2].dst_add));
	save_item(NAME(m_dma[2].size));
	save_item(NAME(m_dma[2].index));
	save_item(NAME(m_dma[2].start_factor));
	save_item(NAME(m_dma[2].enable_mask));
	save_item(NAME(m_dma[2].indirect_mode));
	save_item(NAME(m_dma[2].rup));
	save_item(NAME(m_dma[2].wup));

	m_hostspace = &m_hostcpu->space(AS_PROGRAM);

	m_dma_timer[0] = timer_alloc(FUNC(saturn_scu_device::dma_tick<DMALV0_ID>), this);
	m_dma_timer[1] = timer_alloc(FUNC(saturn_scu_device::dma_tick<DMALV1_ID>), this);
	m_dma_timer[2] = timer_alloc(FUNC(saturn_scu_device::dma_tick<DMALV2_ID>), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void saturn_scu_device::device_reset()
{
	m_ism = 0xbfff;
	m_ist = 0;

	for(int i = 0; i < 3; i++)
	{
		m_dma[i].start_factor = 7;
		m_dma_timer[i]->reset();
	}

	m_status = 0;

}

//-------------------------------------------------
//  device_reset_after_children
//-------------------------------------------------

void saturn_scu_device::device_reset_after_children()
{
	m_scudsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

template <int Level>
TIMER_CALLBACK_MEMBER(saturn_scu_device::dma_tick)
{
	const int irqlevel = Level == 0 ? 5 : 6;
	const int irqvector = 0x4b - Level;
	const uint16_t irqmask = 1 << (11 - Level);

	if(!(m_ism & irqmask))
		m_hostcpu->set_input_line_and_vector(irqlevel, HOLD_LINE, irqvector); // SH2
	else
		m_ist |= (irqmask);

	update_dma_status((uint8_t)Level, false);
	machine().scheduler().synchronize(); // force resync
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//**************************************************************************
//  DMA
//**************************************************************************

uint32_t saturn_scu_device::dma_common_r(uint8_t offset,uint8_t level)
{
	switch(offset)
	{
		case 0x00/4: // source
			return m_dma[level].src;
		case 0x04/4: // destination
			return m_dma[level].dst;
		case 0x08/4: // size
			return m_dma[level].size;
		case 0x0c/4:
			return 0; // DxAD, write only
		case 0x10/4:
			return 0; // DxEN / DxGO, write only
		case 0x14/4:
			return 0; // DxMOD / DxRUP / DxWUP / DxFT, write only
	}

	// can't happen anyway
	return 0;
}

void saturn_scu_device::dma_common_w(uint8_t offset,uint8_t level,uint32_t data)
{
	switch(offset)
	{
		case 0x00/4: // source
			m_dma[level].src = data & 0x07ffffff;
			break;
		case 0x04/4: // destination
			m_dma[level].dst = data & 0x07ffffff;
			break;
		case 0x08/4: // size, lv0 is bigger than the others
			m_dma[level].size = data & ((level == 0) ? 0x000fffff : 0xfff);
			break;
		case 0x0c/4: // DxAD
			m_dma[level].src_add = (data & 0x100) ? 4 : 0;
			m_dma[level].dst_add = 1 << (data & 7);
			if(m_dma[level].dst_add == 1) { m_dma[level].dst_add = 0; }
			break;
		case 0x10/4: // DxEN / DxGO
			m_dma[level].enable_mask = BIT(data,8);

			// check if DxGO is enabled for start factor = 7
			if(m_dma[level].enable_mask == true && data & 1 && m_dma[level].start_factor == 7)
			{
				if(m_dma[level].indirect_mode == true)
					handle_dma_indirect(level);
				else
					handle_dma_direct(level);
			}
			break;
		case 0x14/4: // DxMOD / DxRUP / DxWUP / DxFT, write only
			m_dma[level].indirect_mode = BIT(data,24);
			m_dma[level].rup = BIT(data,16);
			m_dma[level].wup = BIT(data,8);
			m_dma[level].start_factor = data & 7;
			if(m_dma[level].indirect_mode == true)
			{
				//if(LOG_SCU) logerror("Indirect Mode DMA lv %d set\n",level);
				if(m_dma[level].wup == false)
					m_dma[level].index = m_dma[level].dst;
			}

			break;
	}
}

inline void saturn_scu_device::update_dma_status(uint8_t level,bool state)
{
	if(state)
		m_status |= (0x10 << 4 * level);
	else
		m_status &= ~(0x10 << 4 * level);
}

void saturn_scu_device::handle_dma_direct(uint8_t level)
{
	uint32_t tmp_src,tmp_dst,total_size;
	uint8_t cd_transfer_flag;

	#if 0
	if(m_dma[level].src_add == 0 || (m_dma[level].dst_add != 2 && m_dma[level].dst_add != 4))
	{
	if(LOG_SCU) printf("DMA lv %d transfer START\n"
							"Start %08x End %08x Size %04x\n",dma_ch,m_scu.src[dma_ch],m_scu.dst[dma_ch],m_scu.size[dma_ch]);
	if(LOG_SCU) printf("Start Add %04x Destination Add %04x\n",m_scu.src_add[dma_ch],m_scu.dst_add[dma_ch]);
	}
	#endif

	// gamebas, wc98 and batmanfu trips this
	// according to the docs the SCU can't transfer from BIOS area (can't communicate from/to that bus)
	if((m_dma[level].src & 0x07f00000) == 0)
	{
		//popmessage("Warning: SCU transfer from BIOS area, contact MAMEdev");
		if(!(m_ism & IRQ_DMAILL))
			m_hostcpu->set_input_line_and_vector(3, HOLD_LINE, 0x4c); // SH2
		else
			m_ist |= (IRQ_DMAILL);
		return;
	}

	update_dma_status(level,true);

	/* max size */
	if(m_dma[level].size == 0) { m_dma[level].size  = (level == 0) ? 0x00100000 : 0x1000; }

	tmp_src = tmp_dst = 0;

	total_size = m_dma[level].size;
	if(m_dma[level].rup == false) tmp_src = m_dma[level].src;
	if(m_dma[level].wup == false) tmp_dst = m_dma[level].dst;

	cd_transfer_flag = m_dma[level].src_add == 0 && m_dma[level].src == 0x05818000;

	/* TODO: Many games directly accesses CD-ROM register 0x05818000, it must be a dword access with current implementation otherwise it won't work */
	if(cd_transfer_flag)
	{
		int i;
		if((m_dma[level].dst & 0x07000000) == 0x06000000)
			m_dma[level].dst_add = 4;
		else
			m_dma[level].dst_add <<= 1;

		//printf("%d: %08x %08x %d\n", level, m_dma[level].dst, m_dma[level].size, m_dma[level].dst_add);

		for (i = 0; i < m_dma[level].size; i+=m_dma[level].dst_add)
		{
			m_hostspace->write_dword(m_dma[level].dst,m_hostspace->read_dword(m_dma[level].src));
			if(m_dma[level].dst_add == 8)
				m_hostspace->write_dword(m_dma[level].dst+4,m_hostspace->read_dword(m_dma[level].src));

			m_dma[level].src += m_dma[level].src_add;
			m_dma[level].dst += m_dma[level].dst_add;
		}
	}
	else
	{
		int i;
		uint8_t  src_shift;

		src_shift = ((m_dma[level].src & 2) >> 1) ^ 1;

		for (i = 0; i < m_dma[level].size; i+=2)
		{
			dma_single_transfer(m_dma[level].src, m_dma[level].dst, &src_shift);

			if(src_shift)
				m_dma[level].src+= m_dma[level].src_add;

			// if target is Work RAM H, the add value is fixed, behaviour confirmed by fromanc2, stv:vmahjong and burningru
			m_dma[level].dst += ((m_dma[level].dst & 0x07000000) == 0x06000000) ? 2 : m_dma[level].dst_add;
		}
	}

	// burningru doesn't want to zero existing size
//  m_scu.size[dma_ch] = 0;
	if(m_dma[level].rup == false) m_dma[level].src = tmp_src;
	if(m_dma[level].wup == false) m_dma[level].dst = tmp_dst;

	// TODO: Timing is a guess.
	m_dma_timer[level]->adjust(m_hostcpu->cycles_to_attotime(total_size/4));
}

void saturn_scu_device::handle_dma_indirect(uint8_t level)
{
	/*Helper to get out of the cycle*/
	uint8_t job_done = 0;
	/*temporary storage for the transfer data*/
	uint32_t tmp_src;
	uint32_t indirect_src,indirect_dst;
	int32_t indirect_size;
	uint32_t total_size = 0;

	update_dma_status(level,true);

	m_dma[level].index = m_dma[level].dst;

	do{
		tmp_src = m_dma[level].index;

		indirect_size = m_hostspace->read_dword(m_dma[level].index);
		indirect_dst  = m_hostspace->read_dword(m_dma[level].index+4);
		indirect_src  = m_hostspace->read_dword(m_dma[level].index+8);

		/*Indirect Mode end factor*/
		if(indirect_src & 0x80000000)
			job_done = 1;

		#if 0
		if(m_dma[level].src_add == 0 || (m_dma[level].dst_add != 2))
		{
			if(LOG_SCU) printf("DMA lv %d indirect mode transfer START\n"
								"Index %08x Start %08x End %08x Size %04x\n",dma_ch,tmp_src,indirect_src,indirect_dst,indirect_size);
			if(LOG_SCU) printf("Start Add %04x Destination Add %04x\n",m_scu.src_add[dma_ch],m_scu.dst_add[dma_ch]);
		}
		#endif

		indirect_src &=0x07ffffff;
		indirect_dst &=0x07ffffff;
		indirect_size &= ((level == 0) ? 0xfffff : 0x3ffff); //TODO: Guardian Heroes sets up a 0x23000 transfer for the FMV?

		if(indirect_size == 0) { indirect_size = (level == 0) ? 0x00100000 : 0x2000; }

		{
			int i;
			uint8_t  src_shift;

			src_shift = ((indirect_src & 2) >> 1) ^ 1;

			for (i = 0; i < indirect_size;i+=2)
			{
				dma_single_transfer(indirect_src,indirect_dst,&src_shift);

				if(src_shift)
					indirect_src+=m_dma[level].src_add;

				indirect_dst += ((m_dma[level].dst & 0x07000000) == 0x06000000) ? 2 : m_dma[level].dst_add;
			}
		}

		/* Guess: Size + data acquire (1 cycle for src/dst/size) */
		total_size += indirect_size + 3*4;

		//if(DRUP(0)) space.write_dword(tmp_src+8,m_scu.src[0]|job_done ? 0x80000000 : 0);
		//if(DWUP(0)) space.write_dword(tmp_src+4,m_scu.dst[0]);

		m_dma[level].index = tmp_src+0xc;

	}while(job_done == 0);

	// TODO: Timing is a guess.
	m_dma_timer[level]->adjust(m_hostcpu->cycles_to_attotime(total_size/4));
}


inline void saturn_scu_device::dma_single_transfer(uint32_t src, uint32_t dst,uint8_t *src_shift)
{
	uint32_t src_data;

	if(src & 1)
	{
		/* Road Blaster does a work ram h to color ram with offsetted source address, do some data rotation */
		src_data = ((m_hostspace->read_dword(src & 0x07fffffc) & 0x00ffffff)<<8);
		src_data |= ((m_hostspace->read_dword((src & 0x07fffffc)+4) & 0xff000000) >> 24);
		src_data >>= (*src_shift)*16;
	}
	else
		src_data = m_hostspace->read_dword(src & 0x07fffffc) >> (*src_shift)*16;

	m_hostspace->write_word(dst,src_data);

	*src_shift ^= 1;
}

inline void saturn_scu_device::dma_start_factor_ack(uint8_t event)
{
	int i;

	for(i=0;i<3;i++)
	{
		if(m_dma[i].enable_mask == true && m_dma[i].start_factor == event)
		{
			if(m_dma[i].indirect_mode == true)      { handle_dma_indirect(i); }
			else                                    { handle_dma_direct(i); }
		}
	}
}

uint32_t saturn_scu_device::dma_lv0_r(offs_t offset)  { return dma_common_r(offset,0); }
void saturn_scu_device::dma_lv0_w(offs_t offset, uint32_t data) { dma_common_w(offset,0,data); }
uint32_t saturn_scu_device::dma_lv1_r(offs_t offset)  { return dma_common_r(offset,1); }
void saturn_scu_device::dma_lv1_w(offs_t offset, uint32_t data) { dma_common_w(offset,1,data); }
uint32_t saturn_scu_device::dma_lv2_r(offs_t offset)  { return dma_common_r(offset,2); }
void saturn_scu_device::dma_lv2_w(offs_t offset, uint32_t data) { dma_common_w(offset,2,data); }

uint32_t saturn_scu_device::dma_status_r()
{
	return m_status;
}

//**************************************************************************
//  Timers
//**************************************************************************

void saturn_scu_device::t0_compare_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_t0c);
}

void saturn_scu_device::t1_setdata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_t1s);
}

/*
 ---- ---x ---- ---- T1MD Timer 1 mode (0=each line, 1=only at timer 0 lines)
 ---- ---- ---- ---x TENB Timers enable
 */
void saturn_scu_device::t1_mode_w(uint16_t data)
{
	m_t1md = BIT(data,8);
	m_tenb = BIT(data,0);
}


void saturn_scu_device::check_scanline_timers(int scanline, int y_step)
{
	if(m_tenb == false)
		return;

	int timer0_compare = (m_t0c & 0x3ff) * y_step;

	// Timer 0
	if(scanline == timer0_compare)
	{
		if(!(m_ism & IRQ_TIMER_0))
		{
			m_hostcpu->set_input_line_and_vector(0xc, HOLD_LINE, 0x43 ); // SH2
			dma_start_factor_ack(3);
		}
		else
			m_ist |= (IRQ_TIMER_0);
	}

	// Timer 1
	// TODO: should happen after some time when hblank-in is received then counts down t1s
	if(
	  ((m_t1md == false) && ((scanline % y_step) == 0)) ||
	  ((m_t1md == true) && (scanline == timer0_compare)) )
	{
		if(!(m_ism & IRQ_TIMER_1))
		{
			m_hostcpu->set_input_line_and_vector(0xb, HOLD_LINE, 0x44 ); // SH2
			dma_start_factor_ack(4);
		}
		else
			m_ist |= (IRQ_TIMER_1);
	}

}


//**************************************************************************
//  Interrupt
//**************************************************************************

uint32_t saturn_scu_device::irq_mask_r()
{
	return m_ism;
}

uint32_t saturn_scu_device::irq_status_r()
{
	return m_ist;
}

void saturn_scu_device::irq_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ism);
	test_pending_irqs();
}

void saturn_scu_device::irq_status_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if(mem_mask != 0xffffffff)
		logerror("%s IST write %08x with %08x\n",this->tag(),data,mem_mask);

	m_ist &= data;
	test_pending_irqs();
}

void saturn_scu_device::test_pending_irqs()
{
	int i;
	const int irq_level[32] = { 0xf, 0xe, 0xd, 0xc,
								0xb, 0xa, 0x9, 0x8,
								0x8, 0x6, 0x6, 0x5,
								0x3, 0x2,  -1,  -1,
								0x7, 0x7, 0x7, 0x7,
								0x4, 0x4, 0x4, 0x4,
								0x1, 0x1, 0x1, 0x1,
								0x1, 0x1, 0x1, 0x1  };

	for(i=0;i<32;i++)
	{
		if((!(m_ism & 1 << i)) && (m_ist & 1 << i))
		{
			if(irq_level[i] != -1) /* TODO: cheap check for undefined irqs */
			{
				m_hostcpu->set_input_line_and_vector(irq_level[i], HOLD_LINE, 0x40 + i); // SH2
				m_ist &= ~(1 << i);
				return; /* avoid spurious irqs, correct? */
			}
		}
	}
}

void saturn_scu_device::vblank_out_w(int state)
{
	if(!state)
		return;

	if(!(m_ism & IRQ_VBLANK_OUT))
	{
		m_hostcpu->set_input_line_and_vector(0xe, HOLD_LINE, 0x41); // SH2
		dma_start_factor_ack(1);
	}
	else
		m_ist |= (IRQ_VBLANK_OUT);
}

void saturn_scu_device::vblank_in_w(int state)
{
	if(!state)
		return;

	if(!(m_ism & IRQ_VBLANK_IN))
	{
		m_hostcpu->set_input_line_and_vector(0xf, HOLD_LINE ,0x40); // SH2
		dma_start_factor_ack(0);
	}
	else
		m_ist |= (IRQ_VBLANK_IN);
}

void saturn_scu_device::hblank_in_w(int state)
{
	if(!state)
		return;

	if(!(m_ism & IRQ_HBLANK_IN))
	{
		m_hostcpu->set_input_line_and_vector(0xd, HOLD_LINE, 0x42); // SH2
		dma_start_factor_ack(2);
	}
	else
		m_ist |= (IRQ_HBLANK_IN);
}

void saturn_scu_device::vdp1_end_w(int state)
{
	if(!state)
		return;

	if(!(m_ism & IRQ_VDP1_END))
	{
		m_hostcpu->set_input_line_and_vector(0x2, HOLD_LINE, 0x4d); // SH2
		dma_start_factor_ack(6);
	}
	else
		m_ist |= (IRQ_VDP1_END);
}

void saturn_scu_device::sound_req_w(int state)
{
	if(!state)
		return;

	if(!(m_ism & IRQ_SOUND_REQ))
	{
		m_hostcpu->set_input_line_and_vector(9, HOLD_LINE, 0x46); // SH2
		dma_start_factor_ack(5);
	}
	else
		m_ist |= (IRQ_SOUND_REQ);
}

void saturn_scu_device::smpc_irq_w(int state)
{
	if(!state)
		return;

	if(!(m_ism & IRQ_SMPC))
		m_hostcpu->set_input_line_and_vector(8, HOLD_LINE, 0x47); // SH2
	else
		m_ist |= (IRQ_SMPC);
}

void saturn_scu_device::scudsp_end_w(int state)
{
	if(!state)
		return;

	if(!(m_ism & IRQ_DSP_END))
		m_hostcpu->set_input_line_and_vector(0xa, HOLD_LINE, 0x45); // SH2
	else
		m_ist |= (IRQ_DSP_END);

}

//**************************************************************************
//  Miscellanea
//**************************************************************************

uint32_t saturn_scu_device::version_r()
{
	return 4; // correct for stock Saturn at least
}
