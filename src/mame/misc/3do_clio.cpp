// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol

#include "emu.h"
#include "3do_clio.h"

#define LOG_IRQ   (1U << 1) // enable bits (verbose)
#define LOG_TIMER (1U << 2)
#define LOG_XBUS  (1U << 3)
#define LOG_DSPP  (1U << 4)

#define VERBOSE (LOG_GENERAL | LOG_XBUS | LOG_DSPP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIRQ(...)   LOGMASKED(LOG_IRQ,     __VA_ARGS__)
#define LOGTIMER(...) LOGMASKED(LOG_TIMER,   __VA_ARGS__)
#define LOGXBUS(...)  LOGMASKED(LOG_XBUS,    __VA_ARGS__)
#define LOGDSPP(...)  LOGMASKED(LOG_DSPP,    __VA_ARGS__)


DEFINE_DEVICE_TYPE(CLIO, clio_device, "clio", "3DO MN7A02IUDB \"Clio\" I/O controller")

clio_device::clio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CLIO, tag, owner, clock)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_dspp(*this, "dspp")
	, m_firq_cb(*this)
	, m_xbus_read_cb(*this, 0xff)
	, m_xbus_write_cb(*this)
	, m_dac_l(*this)
	, m_dac_r(*this)
{
}

void clio_device::device_add_mconfig(machine_config &config)
{
	DSPP(config, m_dspp, DERIVED_CLOCK(1, 1));
//	m_dspp->int_handler().set([this] (int state) { printf("%d\n", state); });
//	m_dspp->dma_read_handler().set(FUNC(m2_bda_device::read_bus8));
//	m_dspp->dma_write_handler().set(FUNC(m2_bda_device::write_bus8));
}

void clio_device::device_start()
{
	// Clio Green (Toshiba/MEC)
	m_revision = 0x02020000;
	// Clio Preen
//  m_revision = 0x02022000;
	// Anvil
//  m_revision = 0x04000000;
	m_expctl = 0x80;    /* ARM has the expansion bus */

	m_system_timer = timer_alloc(FUNC(clio_device::system_timer_cb), this);
	m_dac_timer = timer_alloc(FUNC(clio_device::dac_update_cb), this);
	m_dac_timer->adjust(attotime::from_hz(16.9345));

	save_item(NAME(m_irq0));
	save_item(NAME(m_irq1));
	save_item(NAME(m_irq0_enable));
	save_item(NAME(m_irq1_enable));
	save_item(NAME(m_timer_ctrl));
}

void clio_device::device_reset()
{
	m_cstatbits = 0x01; /* bit 0 = reset of clio caused by power on */

	m_irq0_enable = m_irq1_enable = 0;
	m_irq0 = m_irq1 = 0;
	m_timer_ctrl = 0;
	m_system_timer->adjust(attotime::never);
}

void clio_device::vint1_w(int state)
{
	// currently used by the BIOS to set periodic stuff up (namely watchdog)
	if (state && m_screen->frame_number() & 1)
	{
		request_fiq(1 << 1, 0);
	}
}

void clio_device::dply_w(int state)
{
	if (state)
		request_fiq(1 << 0, 1);
}

void clio_device::xbus_int_w(int state)
{
	if (state && m_poll & 7)
	{
		request_fiq(1 << 2, 0);
	}

	//if ((m_sel & 0x0f) == 0)
	//{
	//	if (state)
	//		m_poll |= 0x10;
	//	else
	//		m_poll &= ~0x10;
	//}
}

// $0340'0000 base
void clio_device::map(address_map &map)
{
	map(0x0000, 0x0003).lr32(NAME([this] () { return m_revision; }));
	map(0x0004, 0x0007).lrw32(
		NAME([this] () { return m_csysbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("csysbits: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_csysbits);
		})
	);
	map(0x0008, 0x000b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("vint0: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_vint0);
		})
	);
	map(0x000c, 0x000f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("vint1: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_vint1);
		})
	);
	map(0x0020, 0x0023).lrw32(
		NAME([this] () { return m_audin; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("audin: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_audin);
		})
	);
	map(0x0024, 0x0027).lrw32(
		NAME([this] () { return m_audout; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("audout: %08x & %08x\n", data, mem_mask);
			// c0020f0f is written here during boot
			COMBINE_DATA(&m_audout);
		})
	);
	map(0x0028, 0x002b).lrw32(
		NAME([this] () { return m_cstatbits; }),
		// bits 0,1, and 6 are tested (reset source)
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("cstatbits: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_cstatbits);
		})
	);
	map(0x002c, 0x002f).lw32(
		// during boot and vblanks 0000000B is written here, counter reload related?
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (data != 0xb)
				LOG("watchdog: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_wdog);
		})
	);
	// writes probably for test purposes only
	map(0x0030, 0x0033).lrw32(
		NAME([this] () { return m_screen->hpos(); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("hcnt (?): %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_hcnt);
		})
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
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("vcnt (?): %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_vcnt);
		})
	);
	map(0x0038, 0x003b).lrw32(
		NAME([this] () { return m_seed; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (data != 0xc693b70f)
				LOG("seed: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_seed);
			m_seed &= 0x0fff0fff;
		})
	);
	// TODO: should likely follow seed number, and be truly RNG
	map(0x003c, 0x003f).lr32(NAME([this] () {
		LOG("random read (!)\n");
		return m_random;
	}));

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

			LOGIRQ("irq0 enable %s: %08x & %08x -> %08x\n", offset ? "clear" : "set", data, mem_mask, m_irq0_enable);
			request_fiq(0, 0);
		})
	);
	map(0x0050, 0x0057).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_mode &= ~data;
			else
				m_mode |= data;

			LOG("mode %s: %08x & %08x -> %08x\n", offset ? "clear" : "set", data, mem_mask, m_mode);
		})
	);
	map(0x0058, 0x005b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("badbits: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_badbits);
		})
	);

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

			LOGIRQ("irq1 enable %s: %08x & %08x -> %08x\n", offset ? "clear" : "set", data, mem_mask, m_irq1_enable);
			request_fiq(0, 1);
		})
	);

	// expansion control
	map(0x0080, 0x0083).lrw32(
		NAME([this] () { return m_hdelay; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("hdelay %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_hdelay);
		})
	);
	// xxxx ---- DDR for below
	// ---- x--- Watchdog reset output
	// ---- -x-- Alternate ROM bank select (kanji ROM at $300'0000)
	// ---- --x- Audio mute output
	// ---- ---x <unused>
	map(0x0084, 0x0087).lrw32(
		NAME([this] () { return m_adbio; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (data != 0x62)
				LOG("adbio %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_adbio);
			m_adbio &= 0xff;
		})
	);
	map(0x0088, 0x008b).lrw32(
		NAME([this] () { return m_adbctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("adbctl %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_adbctl);
		})
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
			LOGTIMER("timer %s [%d]: %08x & %08x\n", offset & 1 ? "backup" : "count", which, data, mem_mask);
		})
	);
	map(0x0200, 0x020f).lrw32(
		NAME([this] (offs_t offset) {
			const u8 shift = (offset & 2) * 32;
			// TODO: reading clears timer?
			return m_timer_ctrl >> shift;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u8 shift = (offset & 2) * 32;
			const u64 mask = ((u64)data << shift);
			if (offset & 1)
				m_timer_ctrl &= ~mask;
			else
				m_timer_ctrl |= mask;
			LOGTIMER("timer control %s: %08x & %08x (shift=%d)\n", offset & 1 ? "clear" : "set", data, mem_mask, shift);
			m_system_timer->adjust(m_timer_ctrl ? attotime::from_ticks(64, this->clock()) : attotime::never);
		})
	);
	map(0x0220, 0x0223).lrw32(
		NAME([this] () { return m_slack; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("slack: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_slack);
		})
	);

//	map(0x0300, 0x0303) FIFO init
//	map(0x0304, 0x0307) DMA request enable
//	map(0x0308, 0x030b) DMA request disable
	// NOTE: not readable on Red revision apparently
	map(0x0304, 0x030b).lrw32(
		NAME([this] () { return m_dma_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_dma_enable &= ~data;
			else
				m_dma_enable |= data;
			LOG("DMA request %s: %08x & %08x\n", offset ? "clear" : "set", data, mem_mask);
		})
	);
//	map(0x0380, 0x0383) FIFO status

	// XBus
	map(0x0400, 0x0407).lrw32(
		NAME([this] () { return m_expctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_8_15)
			{
				if (offset)
					m_expctl &= ~(data & 0xca00);
				else
					m_expctl |= (data & 0xca00);
				LOGXBUS("xbus expctl %s: %08x & %08x\n", offset ? "clear" : "set", data, mem_mask);
			}
		})
	);
	map(0x0408, 0x040b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGXBUS("xbus type0_4: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_type0_4);
		})
	);
	// TODO: these are identification bits
	// x--- ---- ---- ---- active
	// -x-- ---- ---- ---- "happened before reset"
	// ---- ---- xxxx xxxx device number
	map(0x0410, 0x0413).lr32(
		NAME([this] () {
			LOGXBUS("Read dipir1\n");
			return m_dipir1;
		})
	);
	map(0x0414, 0x0417).lr32(
		NAME([this] () {
			LOGXBUS("Read dipir2\n");
			return m_dipir2;
		})
	);

	map(0x0500, 0x053f).lrw32(
		NAME([this] () { return m_sel; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_sel);
			m_sel &= 0xff;
			/* Start WRSEL cycle */

			LOGXBUS("xbus sel: %02x & %08x\n", data, mem_mask);
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
		NAME([this] () {
			// HACK: until I understand semantics
			if (m_sel == 0)
				return (m_poll & 0xef) | (machine().rand() & 0x10);
			return m_poll;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGXBUS("xbus poll: %02x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_poll);
			m_poll &= 0xf8;
			m_poll |= data & 7;
		})
	);
	// 1--- 1111 external device
	// 0--- xxxx internal device
	map(0x0580, 0x05bf).lrw32(
		NAME([this] () { return m_xbus_read_cb(m_sel & 0x8f); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_xbus_write_cb(m_sel & 0x8f, data & 0xff); })
	);
//	map(0x05c0, 0x05ff) Data

	// TODO: should really map these directly in DSPP core
//	map(0x17d0, 0x17d3) Semaphore
//	map(0x17d4, 0x17d7) Semaphore ACK
//	map(0x17e0, 0x17ff) DSPP DMA and state
	map(0x17e8, 0x17eb).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// reset?
			m_dspp->write(0x6074 >> 2, 1);
			m_dspp->write(0x6074 >> 2, 0);
			//m_dspp->write((0x1300 + 8) >> 2, 568);
			//m_dspp->write((0x1340 + 8) >> 2, 568);
			LOGDSPP("DSPP $17e8 %08x & %08x\n", data, mem_mask);
		})
	);
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
			LOGDSPP("DSPP $17fc %08x & %08x\n", data, mem_mask);
			m_dspp->write(0x6070 >> 2, data);
			//if (data & 1)
			//	machine().debug_break();
		})
	);
	// DSPP N paths (code)
	map(0x1800, 0x1fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp->write((offset << 1) + 0, data >> 16);
			if (ACCESSING_BITS_0_15)
				m_dspp->write((offset << 1) + 1, data & 0xffff);
		})
	);
	map(0x2000, 0x2fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// code path
			if (ACCESSING_BITS_0_15)
				m_dspp->write(offset, data);
		})
	);
	// EI paths (data writes)
	map(0x3000, 0x31ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp->write(((offset<<1) + 0) | 0x400, data);
			if (ACCESSING_BITS_0_15)
				m_dspp->write(((offset<<1) + 1) | 0x400, data);
		})
	);
	map(0x3400, 0x37ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
				m_dspp->write(offset | 0x400, data);
		})
	);
	// EO paths (data reads)
	map(0x3800, 0x39ff).lr32(
		NAME([this] (offs_t offset) {
			uint32_t res = 0;
			res =  m_dspp->read(((offset << 1) + 0) | 0x400) << 16;
			res |= m_dspp->read(((offset << 1) + 1) | 0x400);
			return res;
		})
	);
	map(0x3c00, 0x3fff).lr32(
		NAME([this] (offs_t offset) {
			return m_dspp->read(offset | 0x400);
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

/*
 * x--- "flablode" flag (unknown, is it even implemented?)
 * -x-- cascade flag
 * --x- reload flag
 * ---x decrement flag (enable timer)
 */
TIMER_CALLBACK_MEMBER( clio_device::system_timer_cb )
{
	u8 timer_flag;
	u8 carry_val;

	// TODO: carry behaviour on first timer with cascade enable
	carry_val = 1;

	for(int i = 0; i < 16; i++)
	{
		timer_flag = (m_timer_ctrl >> (i * 4)) & 0xf;

		// TODO: confirm carry on non-sequential cascading
		if(BIT(timer_flag, 0))
		{
			if(BIT(timer_flag, 2))
				m_timer_count[i]-= carry_val;
			else
				m_timer_count[i]--;

			// timer hit on underflows
			if(BIT(m_timer_count[i], 31))
			{
				// only odd numbered timers causes irqs
				if(i & 1)
					request_fiq(8 << (7 - (i >> 1)), 0);

				carry_val = 1;

				if(BIT(timer_flag, 1))
					m_timer_count[i] = m_timer_backup[i];
				else
				{
					m_timer_count[i] = 0xffff;
					m_timer_ctrl &= ~(1 << (i * 4));
				}
			}
			else
				carry_val = 0;
		}
	}

	// TODO: 64 or from spare register?
	m_system_timer->adjust(m_timer_ctrl ? attotime::from_ticks(64, this->clock()) : attotime::never);
}

TIMER_CALLBACK_MEMBER(clio_device::dac_update_cb)
{
	m_dac_l(m_dspp->read_output_fifo());
	m_dac_r(m_dspp->read_output_fifo());
	m_dac_timer->adjust(attotime::from_hz(44100));
}

