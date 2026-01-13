// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol

#include "emu.h"
#include "3do_clio.h"

DEFINE_DEVICE_TYPE(CLIO, clio_device, "clio", "3DO MN7A02IUDB \"Clio\" I/O controller")

clio_device::clio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CLIO, tag, owner, clock)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_firq_cb(*this)
{
}

void clio_device::device_add_mconfig(machine_config &config)
{
	// TODO: convert to emu_timer(s), fix timing
	TIMER(config, "timer_x16").configure_periodic(FUNC(clio_device::timer_x16_cb), attotime::from_hz(12000));
}

void clio_device::device_start()
{
	// Clio Preen (Toshiba/MEC)
	// 0x04000000 for Anvil
	m_revision = 0x02022000 /* 0x04000000 */;
	m_expctl = 0x80;    /* ARM has the expansion bus */
	m_dspp.N = make_unique_clear<uint16_t[]>(0x800);
	m_dspp.EI = make_unique_clear<uint16_t[]>(0x400);
	m_dspp.EO = make_unique_clear<uint16_t[]>(0x400);

	save_pointer(NAME(m_dspp.N), 0x800);
	save_pointer(NAME(m_dspp.EI), 0x400);
	save_pointer(NAME(m_dspp.EO), 0x400);
}

void clio_device::device_reset()
{
	m_cstatbits = 0x01; /* bit 0 = reset of clio caused by power on */
}

// $0340'0000 base
void clio_device::map(address_map &map)
{
	map(0x0000, 0x0003).lr32(NAME([this] () { return m_revision; }));
	map(0x0004, 0x0007).lrw32(
		NAME([this] () { return m_csysbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_csysbits); })
	);
	map(0x0008, 0x000b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_vint0); })
	);
	map(0x000c, 0x000f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_vint1); })
	);
	map(0x0020, 0x0023).lrw32(
		NAME([this] () { return m_audin; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_audin); })
	);
	map(0x0024, 0x0027).lrw32(
		NAME([this] () { return m_audout; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// c0020f0f is written here during boot
			COMBINE_DATA(&m_audout);
		})
	);
	map(0x0028, 0x002b).lrw32(
		NAME([this] () { return m_cstatbits; }),
		// bits 0,1, and 6 are tested (reset source)
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_cstatbits); })
	);
	map(0x002c, 0x002f).lw32(
		// during boot 0000000B is written here, counter reload related?
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_wdog); })
	);
	// writes probably for test purposes only
	map(0x0030, 0x0033).lrw32(
		NAME([this] () { return m_screen->hpos(); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_hcnt); })
	);
	// TODO: needs to moved to a proper timer callback function
	// (or use frame_number for fake interlace readback)
	map(0x0034, 0x0037).lrw32(
		NAME([this] () {
			if ( m_screen->vpos() == 0 && !machine().side_effects_disabled() )
			{
				m_vcnt ^= 0x800;
			}
			return ( m_vcnt & 0x800 ) | m_screen->vpos();
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_vcnt); })
	);
	map(0x0038, 0x003b).lrw32(
		NAME([this] () { return m_seed; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_seed);
			m_seed &= 0x0fff0fff;
		})
	);
	// TODO: should likely follow seed number, and be truly RNG
	map(0x003c, 0x003f).lr32(NAME([this] () { return m_random; }));

	// interrupt control
	map(0x0040, 0x0047).lrw32(
		NAME([this] () { return m_irq0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq0 &= ~data;
			else
				m_irq0 |= data;

			request_fiq(0, 0);
		})
	);
	map(0x0048, 0x004f).lrw32(
		NAME([this] () { return m_irq0_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq0_enable &= ~data;
			else
				m_irq0_enable |= data;

			request_fiq(0, 0);
		})
	);
	map(0x0050, 0x0057).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_mode &= ~data;
			else
				m_mode |= data;
		})
	);
	map(0x0058, 0x005b).lw32(NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_badbits); }));
//	map(0x005c, 0x005f) unknown if used at all
	map(0x0060, 0x0067).lrw32(
		NAME([this] () { return m_irq1; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq1 &= ~data;
			else
				m_irq1 |= data;

			request_fiq(0, 1);
		})
	);
	map(0x0068, 0x006f).lrw32(
		NAME([this] () { return m_irq1_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq1_enable &= ~data;
			else
				m_irq1_enable |= data;

			request_fiq(0, 1);
		})
	);

	// expansion control
	map(0x0080, 0x0083).lrw32(
		NAME([this] () { return m_hdelay; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_hdelay); })
	);
	// xxxx ---- DDR for below
	// ---- x--- Watchdog reset output
	// ---- -x-- Alternate ROM bank select (kanji ROM at $300'0000)
	// ---- --x- Audio mute output
	// ---- ---x <unused>
	map(0x0084, 0x0087).lrw32(
		NAME([this] () { return m_adbio; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_adbio);
			m_adbio &= 0xff;
		})
	);
	map(0x0088, 0x008b).lrw32(
		NAME([this] () { return m_adbctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_adbctl); })
	);

	// Timer section
	map(0x0100, 0x017f).lrw32(
		NAME([this] (offs_t offset) {
			if (offset & 1)
				return m_timer_backup[(offset & 0x3f) >> 1];
			return m_timer_count[(offset & 0x3f) >> 1];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// only lower 16 bits can be written to
			const u8 which = (offset & 0x3f) >> 1;
			if (offset & 1)
			{
				COMBINE_DATA(&m_timer_backup[which]);
				m_timer_backup[which] &= 0xffff;
			}
			else
			{
				COMBINE_DATA(&m_timer_count[which]);
				m_timer_count[which] &= 0xffff;
			}
		})
	);
	map(0x0200, 0x020f).lrw32(
		NAME([this] (offs_t offset) {
			const u8 shift = (offset & 2) * 32;
			return m_timer_ctrl >> shift;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u8 shift = (offset & 2) * 32;
			const u64 mask = ((u64)data << shift);
			if (offset & 1)
				m_timer_ctrl &= ~mask;
			else
				m_timer_ctrl |= mask;
		})
	);
	map(0x0220, 0x0223).lrw32(
		NAME([this] () { return m_slack; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_slack); })
	);

//	map(0x0300, 0x0303) FIFO init
//	map(0x0304, 0x0307) DMA request enable
	// TODO: likely set/clear like above
	map(0x0308, 0x030b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_dmareqdis); })
	);
//	map(0x0380, 0x0383) FIFO status

	// XBus
	map(0x0400, 0x0407).lrw32(
		NAME([this] () { return m_expctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_8_15)
			{
				if (offset)
					m_expctl |= data & 0xca00;
				else
					m_expctl &= ~(data & 0xca00);
			}
		})
	);
	map(0x0408, 0x040b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_type0_4); })
	);
	// TODO: these are identification bits
	// x--- ---- ---- ---- active
	// -x-- ---- ---- ---- "happened before reset"
	// ---- ---- xxxx xxxx device number
	map(0x0410, 0x0413).lr32(NAME([this] () { return m_dipir1; }));
	map(0x0414, 0x0417).lr32(NAME([this] () { return m_dipir2; }));

	map(0x0500, 0x053f).lrw32(
		NAME([this] () { return m_sel; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_sel);
			m_sel &= 0xff;
			/* Start WRSEL cycle */

			/* Detection of too many devices on the bus */
			switch ( data & 0xff )
			{
			case 0x8f:
				/* Everything is fine, there are not too many devices in the system */
				m_poll = ( m_poll & 0x0f );
				break;
			default:
				m_poll = ( m_poll & 0x0f ) | 0x90;
			}
		})
	);
	map(0x0540, 0x057f).lrw32(
		NAME([this] () { return m_poll; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_poll);
			m_poll &= 0xf8;
			m_poll |= data & 7;
		})
	);
//	map(0x0580, 0x05bf) Command Stat
//	map(0x05c0, 0x05ff) Data

	// TODO: for debug, to be removed once that we hookup the CPU core
//	map(0x17d0, 0x17d3) Semaphore
//	map(0x17d4, 0x17d7) Semaphore ACK
//	map(0x17e0, 0x17ff) DSPP DMA and state
	/*
	---- x--- DSPPError
	---- -x-- DSPPReset
	---- --x- DSPPSleep
	---- ---x DSPPGW
	(Red revision)
	---x ---- DSPPError
	---- x--- DSPPReset
	---- -x-- DSPPSleep
	---- --x- DSPPGW-Step
	---- ---x DSPPGW
	*/
	map(0x17fc, 0x17ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (data & 1)
				machine().debug_break();
		})
	);
	map(0x1800, 0x1fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp.N[(offset<<1)+0] = data >> 16;
			if (ACCESSING_BITS_0_15)
				m_dspp.N[(offset<<1)+1] = data & 0xffff;
		})
	);
	map(0x2000, 0x2fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
				m_dspp.N[offset] = data & 0xffff;
		})
	);
	map(0x3000, 0x31ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp.EI[(offset<<1)+0] = data >> 16;
			if (ACCESSING_BITS_0_15)
				m_dspp.EI[(offset<<1)+1] = data & 0xffff;
		})
	);
	map(0x3400, 0x37ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
				m_dspp.EI[offset] = data & 0xffff;
		})
	);
	map(0x3800, 0x39ff).lr32(
		NAME([this] (offs_t offset) {
			uint32_t res = 0;
			res = (m_dspp.EO[(offset << 1) + 0] << 16);
			res |= (m_dspp.EO[(offset << 1) + 1] & 0xffff);
			return res;
		})
	);
	map(0x3c00, 0x3fff).lr32(
		NAME([this] (offs_t offset) {
			return m_dspp.EO[offset] & 0xffff;
		})
	);
}

/*
IRQ0
0x80000000 Second Priority
0x40000000 SW irq
0x20000000 DMA<->EXP
0x1fff0000 DMA RAM->DSPP *
0x0000f000 DMA DSPP->RAM *
0x00000800 DSPP
0x00000400 Timer  1
0x00000200 Timer  3 <- needed to surpass current hang point
0x00000100 Timer  5
0x00000080 Timer  7
0x00000040 Timer  9
0x00000020 Timer 11
0x00000010 Timer 13
0x00000008 Timer 15
0x00000004 Expansion Bus
0x00000002 Vertical 1
0x00000001 Vertical 0

---
IRQ1
0x00000400 DSPPOVER (Red rev. only)
0x00000200 DSPPUNDER (Red rev. only)
0x00000100 BadBits
0x00000080 DMA<-External
0x00000040 DMA->External
0x00000020 DMA<-Uncle
0x00000010 DMA->Uncle
0x00000008 DMA RAM->DSPP N
0x00000004 SlowBus
0x00000002 Disk Inserted
0x00000001 DMA Player bus

*/
void clio_device::request_fiq(uint32_t irq_req, uint8_t type)
{
	if(type)
		m_irq1 |= irq_req;
	else
		m_irq0 |= irq_req;

	if(m_irq1)
		m_irq0 |= 1 << 31; // Second Priority
	else
		m_irq0 &= ~(1 << 31);

	if((m_irq0 & m_irq0_enable) || (m_irq1 & m_irq1_enable))
	{
		//printf("Go irq %08x & %08x %08x & %08x\n",m_irq0, m_irq0_enable, m_irq1, m_irq1_enable);
		m_firq_cb(1);
		//m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( clio_device::timer_x16_cb )
{
	/*
	    x--- fablode flag (wtf?)
	    -x-- cascade flag
	    --x- reload flag
	    ---x decrement flag (enable)
	*/
	uint8_t timer_flag;
	uint8_t carry_val;

	carry_val = 1;

	for(int i = 0;i < 16; i++)
	{
		timer_flag = (m_timer_ctrl >> i*4) & 0xf;

		if(timer_flag & 1)
		{
			if(timer_flag & 4)
				m_timer_count[i]-=carry_val;
			else
				m_timer_count[i]--;

			if(m_timer_count[i] == 0xffffffff) // timer hit
			{
				if(i & 1) // odd timer irq fires
					request_fiq(8 << (7-(i >> 1)), 0);

				carry_val = 1;

				if(timer_flag & 2)
				{
					m_timer_count[i] = m_timer_backup[i];
				}
				else
					m_timer_ctrl &= ~(1 << i*4);
			}
			else
				carry_val = 0;
		}
	}
}

