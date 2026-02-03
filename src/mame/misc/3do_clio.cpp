// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Wilbert Pol

#include "emu.h"
#include "3do_clio.h"

#define LOG_IRQ   (1U << 1) // enable bits (verbose)
#define LOG_TIMER (1U << 2)
#define LOG_XBUS  (1U << 3)
#define LOG_XBUSV (1U << 4) // verbose XBus stuff
#define LOG_DSPP  (1U << 5)

#define VERBOSE (LOG_GENERAL | LOG_XBUS | LOG_DSPP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIRQ(...)   LOGMASKED(LOG_IRQ,     __VA_ARGS__)
#define LOGTIMER(...) LOGMASKED(LOG_TIMER,   __VA_ARGS__)
#define LOGXBUS(...)  LOGMASKED(LOG_XBUS,    __VA_ARGS__)
#define LOGXBUSV(...) LOGMASKED(LOG_XBUSV,   __VA_ARGS__)
#define LOGDSPP(...)  LOGMASKED(LOG_DSPP,    __VA_ARGS__)


DEFINE_DEVICE_TYPE(CLIO, clio_device, "clio", "3DO MN7A02IUDB \"Clio\" I/O controller")

clio_device::clio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CLIO, tag, owner, clock)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_dspp(*this, "dspp")
	, m_firq_cb(*this)
	, m_vsync_cb(*this)
	, m_hsync_cb(*this)
	, m_xbus_sel_cb(*this)
	, m_xbus_read_cb(*this, 0xff)
	, m_xbus_write_cb(*this)
	, m_exp_dma_enable_cb(*this)
	, m_dac_l(*this)
	, m_dac_r(*this)
//  , m_adb_in_cb(*this)
	, m_adb_out_cb(*this)
{
}

void clio_device::device_add_mconfig(machine_config &config)
{
	DSPP(config, m_dspp, DERIVED_CLOCK(1, 1));
	m_dspp->int_handler().set([this] (int state) {
		if (state)
			request_fiq<0>(1 << IRQ_DSPPINT);
	});
//  m_dspp->dma_read_handler().set(FUNC(m2_bda_device::read_bus8));
//  m_dspp->dma_write_handler().set(FUNC(m2_bda_device::write_bus8));
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

	m_scan_timer = timer_alloc(FUNC(clio_device::scan_timer_cb), this);
	m_system_timer = timer_alloc(FUNC(clio_device::system_timer_cb), this);
	m_dac_timer = timer_alloc(FUNC(clio_device::dac_update_cb), this);
	m_dac_timer->adjust(attotime::from_hz(16.9345));

	save_item(NAME(m_csysbits));
	save_item(NAME(m_vint0));
	save_item(NAME(m_vint1));
	save_item(NAME(m_audin));
	save_item(NAME(m_audout));
	save_item(NAME(m_cstatbits));
	save_item(NAME(m_wdog));
//  save_item(NAME(m_hcnt));
//  save_item(NAME(m_vcnt));
	save_item(NAME(m_seed));
	save_item(NAME(m_random));
	save_item(NAME(m_irq));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_mode));
	save_item(NAME(m_badbits));
	save_item(NAME(m_hdelay));
	save_item(NAME(m_adbio));
	save_item(NAME(m_adbctl));

	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_backup));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_slack));

	save_item(NAME(m_dma_enable));

	save_item(NAME(m_expctl));
	save_item(NAME(m_type0_4));
	save_item(NAME(m_dipir1));
	save_item(NAME(m_dipir2));

	save_item(NAME(m_sel));
	save_item(NAME(m_poll));
	save_item(NAME(m_xfrcnt));
	save_item(NAME(m_xbus_dev));
}

void clio_device::device_reset()
{
	m_cstatbits = 0x01; /* bit 0 = reset of clio caused by power on */

	m_irq_enable[0] = m_irq_enable[1] = 0;
	m_irq[0] = m_irq[1] = 0;
	m_timer_ctrl = 0;
	m_vint0 = m_vint1 = 0xffff'ffff;
	m_slack = 336;
	m_adbio = 0x00;
	std::fill(std::begin(m_xbus_dev), std::end(m_xbus_dev), 0x00);
	m_xfrcnt = 0;
	m_system_timer->adjust(attotime::from_ticks(m_slack, this->clock()));
	m_scan_timer->adjust(m_screen->time_until_pos(0), 0);
}

void clio_device::dply_w(int state)
{
	if (state)
		request_fiq<1>(1 << IRQ_PLYINT);
}

void clio_device::xbus_int_w(int state)
{
	if (state)
	{
		m_xbus_dev[0] |= 0x10;
		if (m_xbus_dev[0] & 1)
			request_fiq<0>(1 << IRQ_EXINT);
	}
	else
		m_xbus_dev[0] &= ~0x10;
}

void clio_device::xbus_wr_w(int state)
{
//  printf("%d WR %02x\n", state, m_xbus_dev[0]);
	if (state)
	{
		m_xbus_dev[0] |= 0x20;
		if (m_xbus_dev[0] & 2)
			request_fiq<0>(1 << IRQ_EXINT);
	}
	else
		m_xbus_dev[0] &= ~0x20;
}

void clio_device::dexp_w(int state)
{
	//printf("dexp_w %d\n", state);
	if (state)
	{
		m_expctl &= ~(1 << 10);
		request_fiq<0>(1 << IRQ_DEXINT);
	}
}

void clio_device::arm_ctl_w(int state)
{
	if (state)
	{
		m_expctl |= 0x80;
		// TODO: is this really a thing?
		// avoids starting the DMA again (with count already underflowed),
		// but BIOS clears this status on receiving dexp_w anyway ...
		m_dma_enable &= ~(1 << 20);
	}
	else
	{
		m_expctl &= ~0x80;
		m_expctl |= (1 << 10);
	}
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
			// COMBINE_DATA(&m_hcnt);
		})
	);
	map(0x0034, 0x0037).lrw32(
		NAME([this] () {
			// 0: even field
			return ((m_screen->frame_number() & 1) << 11) | m_screen->vpos();
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("vcnt (?): %08x & %08x\n", data, mem_mask);
			// COMBINE_DATA(&m_vcnt);
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
		if (!machine().side_effects_disabled())
			LOG("random read (!)\n");
		return m_random;
	}));

	// interrupt control
	map(0x0040, 0x0047).lrw32(
		NAME([this] () { return m_irq[0]; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq[0] &= ~data;
			else
				m_irq[0] |= data;

			request_fiq<0>(0);
		})
	);
	map(0x0048, 0x004f).lrw32(
		// TODO: second priority bit 31 should always read 1
		// this however causes continous irq loop cycles during POST as soon as IRQ_PLYINT is requested
		// (never actually enabled in irq1, just IRQ_BADBITS is)
		NAME([this] () { return m_irq_enable[0]; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq_enable[0] &= ~data;
			else
				m_irq_enable[0] |= data;

			LOGIRQ("irq0 enable %s: %08x & %08x -> %08x\n", offset ? "clear" : "set", data, mem_mask, m_irq_enable[0]);
			request_fiq<0>(0);
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

//  map(0x005c, 0x005f) unknown if used at all
	map(0x0060, 0x0067).lrw32(
		NAME([this] () { return m_irq[1]; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq[1] &= ~data;
			else
				m_irq[1] |= data;

			request_fiq<1>(0);
		})
	);
	map(0x0068, 0x006f).lrw32(
		NAME([this] () { return m_irq_enable[1]; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_irq_enable[1] &= ~data;
			else
				m_irq_enable[1] |= data;

			LOGIRQ("irq1 enable %s: %08x & %08x -> %08x\n", offset ? "clear" : "set", data, mem_mask, m_irq_enable[1]);
			request_fiq<1>(0);
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
			if (ACCESSING_BITS_0_7 && data != m_adbio)
			{
				LOG("adbio %08x & %08x\n", data, mem_mask);
				if (BIT(data, 7))
					m_adb_out_cb[3](BIT(data, 3));
				if (BIT(data, 6))
					m_adb_out_cb[2](BIT(data, 2));
				if (BIT(data, 5))
					m_adb_out_cb[1](BIT(data, 1));
				if (BIT(data, 4))
					m_adb_out_cb[0](BIT(data, 0));

				COMBINE_DATA(&m_adbio);
				m_adbio &= 0xff;
			}
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
			//m_system_timer->adjust(m_timer_ctrl ? attotime::from_ticks(64, this->clock()) : attotime::never);
		})
	);
	map(0x0220, 0x0223).lrw32(
		NAME([this] () { return m_slack; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("slack: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_slack);
			m_slack &= 0x7ff;
			// NOTE: Kernel forbids slack times less than 64 anyway
			// TODO: is it really +64?
			m_slack = std::max<unsigned>(m_slack, 64);
			m_system_timer->adjust(attotime::from_ticks(m_slack, this->clock()));
		})
	);

//  map(0x0300, 0x0303) FIFO init
//  map(0x0304, 0x0307) DMA request enable
//  map(0x0308, 0x030b) DMA request disable
	// NOTE: not readable on Red revision apparently
	map(0x0304, 0x030b).lrw32(
		NAME([this] () { return m_dma_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_dma_enable &= ~data;
			else
				m_dma_enable |= data;
			LOG("DMA request %s: %08x & %08x\n", offset ? "clear" : "set", data, mem_mask);
			m_exp_dma_enable_cb(BIT(m_dma_enable, 20) && BIT(m_expctl, 11));
		})
	);
//  map(0x0380, 0x0383) FIFO status

	// XBus
	map(0x0400, 0x0407).lrw32(
		NAME([this] () {
			return m_expctl;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
			{
				if (offset)
					m_expctl &= ~(data & 0xca80);
				else
					m_expctl |= (data & 0xca80);
				LOGXBUS("xbus expctl %s: %08x & %08x\n", offset ? "clear" : "set", data, mem_mask);
				m_exp_dma_enable_cb(BIT(m_dma_enable, 20) && BIT(m_expctl, 11));
			}
		})
	);
	map(0x0408, 0x040b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGXBUS("xbus type0_4: %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_type0_4);
		})
	);
	map(0x040c, 0x040f).lrw32(
		NAME([this] () { return m_xfrcnt; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGXBUS("DMA xfercnt %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_xfrcnt);
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

			m_xbus_sel_cb(data);
			LOGXBUSV("xbus sel: %02x & %08x\n", data, mem_mask);
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
	/*
	 * x--- ---- media access (read clear)
	 * -x-- ---- write valid (r/o)
	 * --x- ---- read valid (r/o)
	 * ---x ---- status valid (r/o)
	 * ---- x--- reset
	 * ---- -x-- write irq enable
	 * ---- --x- read irq enable
	 * ---- ---x status irq enable
	 */
	map(0x0540, 0x057f).lrw32(
		NAME([this] () -> u8 {
			u8 res = m_poll;
			//m_poll &= ~0x80;
			if (m_sel == 0)
				return m_xbus_dev[0]; //(res & 0xcf) | (m_xbus_dev[0] & 0x30);
			return (res);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGXBUS("xbus poll: %02x & %08x (sel=%d)\n", data, mem_mask, m_sel);
			//COMBINE_DATA(&m_poll);
			//m_poll &= 0xf8;
			if (ACCESSING_BITS_0_7)
			{
				if (m_sel == 0)
				{
					m_xbus_dev[0] = (data & 0xf) | (m_xbus_dev[0] & 0xf0);
				}
				else
					m_poll = data & 0xff;
			}
		})
	);
	// 1--- 1111 external device
	// 0--- xxxx internal device
	map(0x0580, 0x05bf).lrw32(
		NAME([this] () { return m_xbus_read_cb(m_sel & 0x8f); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_xbus_write_cb(m_sel & 0x8f, data & 0xff); })
	);
//  map(0x05c0, 0x05ff) Data

	// TODO: should really map these directly in DSPP core
//  map(0x17d0, 0x17d3) Semaphore
	// HACK: temporary to allow 3do_gdo101 boot
	map(0x17d0, 0x17d3).lr32(NAME([] () { return 0x0004'0000; }));
//  map(0x17d4, 0x17d7) Semaphore ACK
//  map(0x17e0, 0x17e3) DSPP DMA
//  map(0x17e4, 0x17e7) DSPPRST0 (use current reload)
	// DSPPRST1 (set 568 tick cycle)
	map(0x17e8, 0x17eb).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			//m_dspp->write(0x6074 >> 2, 1);
			//m_dspp->write(0x6074 >> 2, 0);
			//m_dspp->write((0x1300 + 8) >> 2, 568);
			//m_dspp->write((0x1340 + 8) >> 2, 568);
			LOGDSPP("DSPP $17e8 %08x & %08x\n", data, mem_mask);
		})
	);
//  map(0x17f0, 0x17f3) Read noise value (Red only?)
//  map(0x17f4, 0x17f7) Read DSPP PC (bits 15:0 only)
//  map(0x17f8, 0x17fb) Read DSPP NR (bits 15:0 only)
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
			m_dspp->host_gw_control_write(0, data);
		})
	);
	// DSPP iNstruction paths (code)
	map(0x1800, 0x1fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp->host_n_write((offset << 1) + 0, data >> 16);
			if (ACCESSING_BITS_0_15)
				m_dspp->host_n_write((offset << 1) + 1, data & 0xffff);
		})
	);
	map(0x2000, 0x2fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// code path
			if (ACCESSING_BITS_0_15)
				m_dspp->host_n_write(offset, data);
		})
	);
	// EI paths (data writes from host)
	map(0x3000, 0x31ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp->host_ei_write(((offset<<1) + 0), data);
			if (ACCESSING_BITS_0_15)
				m_dspp->host_ei_write(((offset<<1) + 1), data);
		})
	);
	map(0x3400, 0x37ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
				m_dspp->host_ei_write(offset, data);
		})
	);
	// EO paths (data reads from host)
	map(0x3800, 0x39ff).lr32(
		NAME([this] (offs_t offset) {
			uint32_t res = 0;
			res =  m_dspp->host_eo_read(((offset << 1) + 0)) << 16;
			res |= m_dspp->host_eo_read(((offset << 1) + 1));
			return res;
		})
	);
	map(0x3c00, 0x3fff).lr32(
		NAME([this] (offs_t offset) {
			return m_dspp->host_eo_read(offset);
		})
	);
}

template <unsigned N> void clio_device::request_fiq(uint32_t irq_req)
{
	if (N)
		m_irq[1] |= irq_req;
	else
		m_irq[0] |= irq_req;

	if(m_irq[1])
		m_irq[0] |= 1 << IRQ_SCNDPINT; // Second Priority
	else
		m_irq[0] &= ~(1 << IRQ_SCNDPINT);

	m_firq_cb((m_irq[0] & m_irq_enable[0]) || (m_irq[1] & m_irq_enable[1]));
}

// TODO: this actually generates from Amy not from Clio
TIMER_CALLBACK_MEMBER(clio_device::scan_timer_cb)
{
	int scanline = param;

	// TODO: does it triggers on odd fields only?
	if (scanline == m_vint1 && m_screen->frame_number() & 1)
	{
		request_fiq<0>(1 << IRQ_VINT1);
	}

	// 22, 262
	if (scanline == 0)
	{
		m_vsync_cb(1);
	}
	else if (scanline == 22 - 6)
	{
		m_vsync_cb(0);
	}
	else if (scanline > 22 - 6)
	{
		m_hsync_cb(1);
		m_hsync_cb(0);
	}

	scanline ++;
	scanline %= m_screen->height();

	m_scan_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


/*
 * x--- "flablode" flag (unknown, is it even implemented?)
 * -x-- cascade flag
 * --x- reload flag
 * ---x decrement flag (enable timer)
 */
TIMER_CALLBACK_MEMBER( clio_device::system_timer_cb )
{
	static const u32 timer_irqs[8] = {
		1 << IRQ_TIMINT1, 1 << IRQ_TIMINT3,  1 << IRQ_TIMINT5,  1 << IRQ_TIMINT7,
		1 << IRQ_TIMINT9, 1 << IRQ_TIMINT11, 1 << IRQ_TIMINT13, 1 << IRQ_TIMINT15
	};
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
					request_fiq<0>(timer_irqs[i >> 1]);

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

	// Opera specification goes this lengthy explaination about "64" being the unit of time
	// but 3do_fc2 and 3do_gdo101 won't boot with a timer tick this small ...
	// TODO: causes "PAL system detected" in most BIOSes
	// - slack may just be the start time
	// - in an ideal world we would split this implementation in separate timers,
	//   however that will give more scheduler roundtrips, in an already crowded scenario ...
	m_system_timer->adjust(attotime::from_ticks(m_slack, this->clock()));
}

TIMER_CALLBACK_MEMBER(clio_device::dac_update_cb)
{
	m_dac_l(m_dspp->read_output_fifo());
	m_dac_r(m_dspp->read_output_fifo());
	m_dac_timer->adjust(attotime::from_hz(44100));
}

