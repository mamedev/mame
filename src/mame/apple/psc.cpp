// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple PSC (Peripheral Subsystem Controller) system controller ASIC
    Emulation by R. Belmont

    PSC provides a full-function VIA1, a minimal pseudo-VIA2, a DMA engine,
    address decoding for the Curio and NewAge external I/O chips, and interfacing
    for the Singer audio CODEC (similar to AWACS) and the AT&T DSP3210.  Everything
    is roughly similar to the AMIC chip in the first-generation PowerMacs.

    VIA2 interrupt bits:
    slot register (1):
    3 = slot $C
    4 = slot $D
    5 = slot $E
    6 = built-in video VBL
    7 = any of the other bits

    main interrupt register (0xd):
    0 = SCSI
    1 = any slot interrupt
    2 = MUNI
    3 = SCSI (same as bit 0?)
    4 = ?
    5 = New Age
    6 = sound frame
    7 = any of the other bits
*/

#include "emu.h"
#include "psc.h"

#include "formats/ap_dsk35.h"

#define LOG_PSCREGS    (1U << 1)
#define LOG_ENET       (1U << 2)

#define VERBOSE (0)
#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

static constexpr u32 C7M  = 7'833'600;

static constexpr int IRQ_SCSIDRQ    = 0;
static constexpr int IRQ_ANYSLOT    = 1;
[[maybe_unused]] static constexpr int IRQ_MUNIIRQ = 2;
static constexpr int IRQ_SCSIIRQ    = 3;
static constexpr int IRQ_FDCIRQ     = 5;
[[maybe_unused]] static constexpr int IRQ_SNDFRMIRQ = 6;

static constexpr u16 CMD_IE         = 0x1000;
static constexpr u16 CMD_ENABLED    = 0x0800;
static constexpr u16 CMD_TERMCNT    = 0x0400;
static constexpr u16 CMD_DIR        = 0x0200;
static constexpr u16 CMD_IF         = 0x0100;

static constexpr u16 CTRL_FROZEN    = 0x4000;
static constexpr u16 CTRL_BERR      = 0x2000;
static constexpr u16 CTRL_CIE       = 0x1000;
static constexpr u16 CTRL_SWRESET   = 0x0800;
static constexpr u16 CTRL_PAUSE     = 0x0400;
static constexpr u16 CTRL_FLUSH     = 0x0200;
static constexpr u16 CTRL_CIRQ      = 0x0100;

static constexpr u8 LV3_ENETIRQ     = 0x01;

static constexpr u8 LV4_SNDSTAT     = 0x01;
static constexpr u8 LV4_SCCA        = 0x02;
static constexpr u8 LV4_SCCB        = 0x04;
static constexpr u8 LV4_DMA         = 0x08;

static constexpr u8 LV5_DSPIRQ      = 0x01;
static constexpr u8 LV5_FRMOVRNNIRQ = 0x02;

static constexpr u8 LV6_60HZ        = 0x01;
static constexpr u8 LV6_SCCA        = 0x02;
static constexpr u8 LV6_SCCB        = 0x04;

enum
{
	DMA_SCSI = 0,
	DMA_ETHERNET_RX,
	DMA_ETHERNET_TX,
	DMA_FDC,
	DMA_SCCA,
	DMA_SCCB,
	DMA_SCCATX,

	DMA_NUM_CHANNELS
};

DEFINE_DEVICE_TYPE(PSC, psc_device, "applepsc", "Apple Peripheral Subsystem Controller")

void psc_device::map(address_map &map)
{
	map(0x00f0'0000, 0x00f0'1fff).rw(FUNC(psc_device::mac_via_r), FUNC(psc_device::mac_via_w));
	map(0x00f0'2000, 0x00f0'3fff).rw(FUNC(psc_device::mac_via2_r), FUNC(psc_device::mac_via2_w));
	map(0x00f2'a090, 0x00f2'a093).r(FUNC(psc_device::scc_fake_r));
	map(0x00f2'a100, 0x00f2'a103).r(FUNC(psc_device::newage_fake_r));
	map(0x00f3'1000, 0x00f3'2fff).rw(FUNC(psc_device::psc_regs_r), FUNC(psc_device::psc_regs_w));

	map(0x00f3'1c00, 0x00f3'1c6f).rw(FUNC(psc_device::dma_ctrl_r), FUNC(psc_device::dma_ctrl_w));

	map(0x00f3'2000, 0x00f3'20df).rw(FUNC(psc_device::dma_set_r), FUNC(psc_device::dma_set_w));

	// PSC always IDs as 2830, VIA bits differentiate 660AV and 840AV
	map(0x0fff'0000, 0x0fff'ffff).lr32(NAME([](offs_t offset) { return 0xa55a2830; }));
}

void psc_device::device_add_mconfig(machine_config &config)
{
	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(psc_device::via_in_a));
	m_via1->readpb_handler().set(FUNC(psc_device::via_in_b));
	m_via1->writepa_handler().set(FUNC(psc_device::via_out_a));
	m_via1->writepb_handler().set(FUNC(psc_device::via_out_b));
	m_via1->cb1_handler().set(FUNC(psc_device::via_out_cb1));
	m_via1->cb2_handler().set(FUNC(psc_device::via_out_cb2));
	m_via1->irq_handler().set(FUNC(psc_device::via1_irq));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_16BIT_R2R(config, m_dac_l);
	DAC_16BIT_R2R(config, m_dac_r);
	m_dac_l->add_route(0, "lspeaker", 1.0);
	m_dac_r->add_route(0, "rspeaker", 1.0);
}

psc_device::psc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSC, tag, owner, clock),
	m_cb1(*this),
	m_cb2(*this),
	write_pb4(*this),
	write_pb5(*this),
	read_pb3(*this, 0),
	m_pa1(*this, 0),
	m_pa2(*this, 0),
	m_pa4(*this, 0),
	m_pa6(*this, 0),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_via1(*this, "via1"),
	m_dac_l(*this, "dacl"),
	m_dac_r(*this, "dacr"),
	m_slot_irqs(0xff),
	m_ifr(0),
	m_ier(0),
	m_via_interrupt(0),
	m_via2_interrupt(0),
	m_scc_interrupt(0),
	m_last_taken_interrupt(-1),
	m_drq(0),
	m_scsi_irq(0),
	m_fdc_irq(0),
	m_audio_out_ptr(0),
	m_audio_out_offset(0),
	m_audio_out_length(0),
	m_l3if(0), m_l3ier(0),
	m_l4if(0), m_l4ier(0),
	m_l5if(0), m_l5ier(0),
	m_l6if(0), m_l6ier(0),
	m_dma_irqstat(0),
	m_mace(*this, finder_base::DUMMY_TAG),
	m_enet_timer(nullptr),
	m_enet_tx_drq(false),
	m_enet_rx_drq(false),
	m_enet_rx_offset(0),
	m_enet_rx_status(0),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_ncr(nullptr)
{
	std::fill(std::begin(m_psc_regs), std::end(m_psc_regs), 0);
	for (int dma = 0; dma < DMA_NUM_CHANNELS; dma++)
	{
		m_dma_control[dma] = 0;

		for (int set = 0; set < 2; set++)
		{
			m_dma_addr[dma][set] = 0;
			m_dma_cnt[dma][set] = 0;
			m_dma_cmdstat[dma][set] = 0;
		}
	}
}

void psc_device::device_start()
{
	m_maincpu->set_emmu_enable(true);

	m_6015_timer = timer_alloc(FUNC(psc_device::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	m_singer_timer = timer_alloc(FUNC(psc_device::singer_tick), this);
	m_enet_timer = timer_alloc(FUNC(psc_device::enet_dma_tick), this);

	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_slot_irqs));
	save_item(NAME(m_ifr));
	save_item(NAME(m_ier));
	save_item(NAME(m_psc_regs));
	save_item(NAME(m_l3if));
	save_item(NAME(m_l3ier));
	save_item(NAME(m_l4if));
	save_item(NAME(m_l4ier));
	save_item(NAME(m_l5if));
	save_item(NAME(m_l5ier));
	save_item(NAME(m_l6if));
	save_item(NAME(m_l6ier));
	save_item(NAME(m_dma_control));
	save_item(NAME(m_dma_addr));
	save_item(NAME(m_dma_cnt));
	save_item(NAME(m_dma_cmdstat));
	save_item(NAME(m_dma_irqstat));
	save_item(NAME(m_enet_tx_drq));
	save_item(NAME(m_enet_rx_drq));
	save_item(NAME(m_enet_rx_offset));
	save_item(NAME(m_enet_rx_status));
	save_item(NAME(m_drq));
	save_item(NAME(m_scsi_irq));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_audio_out_ptr));
	save_item(NAME(m_audio_out_offset));
	save_item(NAME(m_audio_out_length));

}

void psc_device::device_reset()
{
	m_enet_timer->enable(false);
	m_enet_tx_drq = m_enet_rx_drq = false;
	m_enet_rx_offset = m_enet_rx_status = 0;
	for (int channel = 0; channel < DMA_NUM_CHANNELS; ++channel)
	{
		m_dma_control[channel] = 0;
		for (int set = 0; set < 2; ++set)
		{
			m_dma_addr[channel][set] = m_dma_cnt[channel][set] = 0;
			m_dma_cmdstat[channel][set] = 0;
		}
	}
	m_l3if = m_l3ier = m_l4if = m_l4ier = 0;
	m_l5if = m_l5ier = m_l6if = m_l6ier = 0;
	device_post_load();

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
}

TIMER_CALLBACK_MEMBER(psc_device::mac_6015_tick)
{
	m_via1->write_ca1(CLEAR_LINE);
	m_via1->write_ca1(ASSERT_LINE);

	m_l6if |= LV6_60HZ;
	recalc_lv6();
}

u16 psc_device::dma_ctrl_r(offs_t offset)
{
	return m_dma_control[offset >> 3] | 0x8000;
}

void psc_device::device_post_load()
{
	recalc_dma_irqs();
	recalc_lv3();
	recalc_lv5();
	recalc_lv6();
}

void psc_device::dma_ctrl_w(offs_t offset, u16 data)
{
	const unsigned channel = offset >> 3;
	const bool ethernet = channel == DMA_ETHERNET_RX || channel == DMA_ETHERNET_TX;
	u16 &control = m_dma_control[channel];
	if (BIT(data, 15))
	{
		control |= data & (CTRL_CIE | CTRL_PAUSE);
		if (data & CTRL_SWRESET)
		{
			// Software reset preserves programmed sets and interrupt enables,
			// disables both sets, and selects set zero with the channel paused.
			control = (control & CTRL_CIE) | CTRL_FROZEN | CTRL_PAUSE;
			for (auto &command : m_dma_cmdstat[channel])
				command &= ~CMD_ENABLED;
			if (channel == DMA_ETHERNET_RX)
				m_enet_rx_offset = m_enet_rx_status = 0;
		}
		if (data & CTRL_PAUSE)
			control |= CTRL_FROZEN;
		if ((data & CTRL_FLUSH) && ethernet)
		{
			// Flush abandons the current chain and advances to the other set.
			m_dma_cmdstat[channel][control & 1] &= ~CMD_ENABLED;
			control = (control ^ 1) | CTRL_PAUSE | CTRL_FROZEN;
			if (channel == DMA_ETHERNET_RX)
				m_enet_rx_offset = m_enet_rx_status = 0;
		}
	}
	else
	{
		control &= ~(data & (CTRL_CIE | CTRL_PAUSE | CTRL_BERR));
		if (data & CTRL_PAUSE)
			control &= ~CTRL_FROZEN;
	}
	recalc_dma_irqs();
	if (ethernet)
		enet_dma_kick();
}

u32 psc_device::dma_set_r(offs_t offset)
{
	const unsigned channel = offset >> 3;
	const unsigned set = BIT(offset, 2);
	switch (offset & 3)
	{
	case 0: return m_dma_addr[channel][set];
	case 1: return m_dma_cnt[channel][set];
	case 2:
		return u32(m_dma_cmdstat[channel][set] | (m_dma_cnt[channel][set] ? 0 : CMD_TERMCNT)) << 16;
	default: return 0;
	}
}

void psc_device::dma_set_w(offs_t offset, u32 data, u32 mem_mask)
{
	const unsigned channel = offset >> 3;
	const unsigned set = BIT(offset, 2);
	switch (offset & 3)
	{
	case 0:
		COMBINE_DATA(&m_dma_addr[channel][set]);
		break;
	case 1:
		COMBINE_DATA(&m_dma_cnt[channel][set]);
		break;
	case 2:
		if (ACCESSING_BITS_16_31)
		{
			const u16 bits = (data & mem_mask) >> 16;
			const u16 writable = bits & (CMD_IE | CMD_ENABLED | CMD_DIR | CMD_IF);
			if (BIT(bits, 15))
				m_dma_cmdstat[channel][set] |= writable;
			else
				m_dma_cmdstat[channel][set] &= ~writable;
		}
		break;
	}
	recalc_dma_irqs();
	if (channel == DMA_SCSI && m_ncr)
		scsi_drq_w(m_drq);
	else if (channel == DMA_ETHERNET_RX || channel == DMA_ETHERNET_TX)
		enet_dma_kick();
}

void psc_device::enet_irq_w(int state)
{
	m_l3if = (m_l3if & ~LV3_ENETIRQ) | (state ? LV3_ENETIRQ : 0);
	recalc_lv3();
}

void psc_device::enet_tx_drq_w(int state)
{
	m_enet_tx_drq = bool(state);
	enet_dma_kick();
}

void psc_device::enet_rx_drq_w(int state)
{
	m_enet_rx_drq = bool(state);
	enet_dma_kick();
}

bool psc_device::enet_dma_ready(int channel) const
{
	const unsigned set = m_dma_control[channel] & 1;
	return !(m_dma_control[channel] & (CTRL_PAUSE | CTRL_FROZEN | CTRL_BERR)) &&
		(m_dma_cmdstat[channel][set] & CMD_ENABLED) && m_dma_cnt[channel][set];
}

void psc_device::enet_dma_kick()
{
	// Never call back into the MACE FIFO from a request callback.  Requests can
	// change while the chip is still updating its data/status cursors.
	if (m_enet_timer && !m_enet_timer->enabled() && m_mace &&
		((m_enet_tx_drq && enet_dma_ready(DMA_ETHERNET_TX)) ||
		 ((m_enet_rx_drq || m_enet_rx_offset || m_enet_rx_status) && enet_dma_ready(DMA_ETHERNET_RX))))
		m_enet_timer->adjust(attotime::from_usec(1));
}

void psc_device::enet_dma_complete(int channel)
{
	const unsigned set = m_dma_control[channel] & 1;
	m_dma_cmdstat[channel][set] |= CMD_IF;
	if (!m_dma_cnt[channel][set])
	{
		m_dma_cmdstat[channel][set] &= ~CMD_ENABLED;
		m_dma_control[channel] ^= 1;
	}
	LOGMASKED(LOG_ENET, "Ethernet %s set %u complete: address %08x count %u\n",
		channel == DMA_ETHERNET_RX ? "RX" : "TX", set, m_dma_addr[channel][set], m_dma_cnt[channel][set]);
	recalc_dma_irqs();
}

TIMER_CALLBACK_MEMBER(psc_device::enet_dma_tick)
{
	// A bounded burst models the FIFO service, not individual PSC bus cycles.
	// MACE supplies wire timing and backpressure.  PSC wiring presents the
	// first memory byte on D7:0 (the driver leaves MACE BSWP clear).
	for (unsigned word = 0; word < 8 && m_enet_tx_drq && enet_dma_ready(DMA_ETHERNET_TX); ++word)
	{
		const unsigned set = m_dma_control[DMA_ETHERNET_TX] & 1;
		u32 &address = m_dma_addr[DMA_ETHERNET_TX][set];
		u32 &count = m_dma_cnt[DMA_ETHERNET_TX][set];
		const unsigned bytes = std::min<u32>(2, count);
		u16 data = m_space->read_byte(address);
		if (bytes == 2)
			data |= u16(m_space->read_byte(address + 1)) << 8;
		if (!m_mace->tx_dma_w(data, bytes == 2 ? 0xffff : 0x00ff, count == bytes))
			break;
		address += bytes;
		count -= bytes;
		if (!count)
			enet_dma_complete(DMA_ETHERNET_TX);
	}

	for (unsigned word = 0; word < 8 && enet_dma_ready(DMA_ETHERNET_RX) &&
		(m_enet_rx_drq || m_enet_rx_offset || m_enet_rx_status); ++word)
	{
		const auto result = m_mace->rx_dma_r();
		if (!result.valid)
			break;
		const unsigned set = m_dma_control[DMA_ETHERNET_RX] & 1;
		u32 &address = m_dma_addr[DMA_ETHERNET_RX][set];
		if (result.status)
		{
			m_space->write_word(address + 2 * m_enet_rx_status, result.data);
			++m_enet_rx_status;
		}
		else
		{
			for (unsigned byte = 0; byte < result.bytes; ++byte)
			{
				// A malformed oversized frame must never overwrite the next slot.
				if (m_enet_rx_offset < 2048 - 16)
					m_space->write_byte(address + 16 + m_enet_rx_offset, result.data >> (8 * byte));
				++m_enet_rx_offset;
			}
		}
		if (result.frame_done)
		{
			// Publish the slot only after all four duplicated status bytes exist.
			address += 2048;
			--m_dma_cnt[DMA_ETHERNET_RX][set];
			m_enet_rx_offset = m_enet_rx_status = 0;
			enet_dma_complete(DMA_ETHERNET_RX);
		}
	}
	// Poll partial packets through temporary FIFO boundaries, including the
	// final status cycles after RREQ drops.  Pause/reset stops this timer.
	enet_dma_kick();
}

uint32_t psc_device::scc_fake_r()
{
	return 0x40000000; // fake ESCC AppleTalk stuff?
}

uint32_t psc_device::newage_fake_r()
{
	return 0x00800000; // fake New Age status
}

uint8_t psc_device::via_in_a()
{
	u8 result = m_pa1() << 1;
	result |= m_pa2() << 2;
	result |= m_pa4() << 4;
	result |= m_pa6() << 6;
	return result | 0x80;
}

uint8_t psc_device::via_in_b()
{
	return (read_pb3() << 3) | 0x80;
}

void psc_device::via_out_cb1(int state)
{
	m_cb1(state & 1);
}

void psc_device::via_out_cb2(int state)
{
	m_cb2(state & 1);
}

void psc_device::via_out_a(uint8_t data)
{
}

void psc_device::via_out_b(uint8_t data)
{
	write_pb4(BIT(data, 4));
	write_pb5(BIT(data, 5));
}

void psc_device::via1_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

template <u8 mask>
void psc_device::via2_irq_w(int state)
{
	if (state)
	{
		m_slot_irqs &= ~mask;
	}
	else
	{
		m_slot_irqs |= mask;
	}

	recalc_via2_irqs();
}

template void psc_device::via2_irq_w<0x40>(int state);
template void psc_device::via2_irq_w<0x20>(int state);
template void psc_device::via2_irq_w<0x10>(int state);
template void psc_device::via2_irq_w<0x08>(int state);
template void psc_device::via2_irq_w<0x01>(int state);

void psc_device::recalc_via2_irqs()
{
	// are any slot interrupts active?
	if ((m_slot_irqs & 0x7f) != 0x7f)
	{
		if (!BIT(m_ifr, IRQ_ANYSLOT))
		{
			m_ifr |= (1 << IRQ_ANYSLOT);
		}
	}
	else
	{
		m_ifr &= ~(1 << IRQ_ANYSLOT);
	}

	if (m_ifr & m_ier & 0x7f)
	{
		m_via2_interrupt = ASSERT_LINE;
		field_interrupts();
	}
	else
	{
		m_via2_interrupt = CLEAR_LINE;
		field_interrupts();
	}
}

u16 psc_device::mac_via2_r(offs_t offset)
{
	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	switch (offset)
	{
		case 11:
			return 0xc0 << 8;

		case 12:
			return 0x26 << 8;

		case 13:    // IFR
			return m_ifr << 8;

		case 14:    // IER
			return m_ier << 8;

		case 15:    // Port A (slot interrupts)
			return (m_slot_irqs & 0x7f) << 8;

		default:
			printf("PSC: Read unhandled VIA2 register %d\n", offset);
			break;
	}
	return 0xffff;
}

void psc_device::mac_via2_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;
	data >>= 8;

	// what makes VIA2 "pseudo" is that regs 4-10 can't be written, and 11 and 12 have canned values
	switch (offset)
	{
		case 13: // IFR
			// As per the PSC ERS, writing a 1 to bit 6 will clear it.
			// All other bits must be turned off at their source and writing here
			// will have no effect.
			if (BIT(data, 6))
			{
				m_ifr &= ~(1 << 6);
			}
			break;

		case 14: // IER
			// The one remaining trace of actual 6522 semantics:
			// Bit 7 set = bits 0-6 that are 1 set the corresponding bits
			// Bit 7 clear = bits 0-6 that are 1 clear the corresponding bits
			if (BIT(data, 7))
			{
				m_ier |= data & 0x7f;
			}
			else
			{
				m_ier &= ~(data & 0x7f);
			}
			break;
	}

	via_sync();
}

void psc_device::field_interrupts()
{
	int take_interrupt = -1;

	if (m_scc_interrupt)
	{
		take_interrupt = 4;
	}
	else if (m_via2_interrupt)
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

void psc_device::scc_irq_w(int state)
{
	m_scc_interrupt = (state == ASSERT_LINE) ? 1 : 0;
	field_interrupts();
}

void psc_device::vbl_irq_w(int state)
{
	via2_irq_w<0x40>(state);
}


void psc_device::scsi_irq_w(int state)
{
	m_scsi_irq = state;

	//printf("SCSI IRQ: %d\n", state);
	m_ifr &= ~(1 << IRQ_SCSIIRQ);
	m_ifr |= state ? (1 << IRQ_SCSIIRQ) : 0;
	recalc_via2_irqs();
	//printf("recalc_via2_irqs: slot %02x IFR %02x IER %02x lvl %d\n", m_slot_irqs, m_ifr, m_ier, m_via2_interrupt);
}

void psc_device::fdc_irq_w(int state)
{
	m_fdc_irq = state;
	m_ifr &= ~(1 << IRQ_FDCIRQ);
	m_ifr |= state ? (1 << IRQ_FDCIRQ) : 0;
	recalc_via2_irqs();
}

void psc_device::cb1_w(int state)
{
	m_via1->write_cb1(state);
}

void psc_device::cb2_w(int state)
{
	m_via1->write_cb2(state);
}

uint16_t psc_device::mac_via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void psc_device::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

void psc_device::via_sync()
{
	// The via runs at 783.36KHz while the main cpu runs at 15MHz or
	// more, so we need to sync the access with the via clock.  Plus
	// the whole access takes half a (via) cycle and ends when synced
	// with the main cpu again.

	// Get the main cpu time
	u64 cycle = m_maincpu->total_cycles();

	// Get the number of the cycle the via is in at that time
	u64 via_cycle = cycle * m_via1->clock() / m_maincpu->clock();

	// The access is going to start at via_cycle+1 and end at
	// via_cycle+1.5, compute what that means in maincpu cycles (the
	// +1 rounds up, since the clocks are too different to ever be
	// synced).
	u64 main_cycle = (via_cycle * 2 + 3) * m_maincpu->clock() / (2 * m_via1->clock()) + 1;

	// Finally adjust the main cpu icount as needed.
	m_maincpu->adjust_icount(-int(main_cycle - cycle));
}

void psc_device::scsi_drq_w(int state)
{
	m_drq = state;
	m_ifr &= ~(1 << IRQ_SCSIDRQ);
	m_ifr |= state ? (1 << IRQ_SCSIDRQ) : 0;
	recalc_via2_irqs();

	const int active_set = m_dma_control[DMA_SCSI] & 1;
	while ((m_drq == ASSERT_LINE) && (m_dma_cmdstat[DMA_SCSI][active_set] & CMD_ENABLED))
	{
		const u32 xfer_size = (m_dma_cnt[DMA_SCSI][active_set] == 1) ? 1 : 2;
		if (m_dma_cmdstat[DMA_SCSI][active_set] & CMD_DIR)
		{
			if (xfer_size == 1)
			{
				m_space->write_byte(m_dma_addr[DMA_SCSI][active_set], m_ncr->dma_r());
			}
			else
			{
				m_space->write_word(m_dma_addr[DMA_SCSI][active_set], m_ncr->dma16_swap_r());
			}
		}
		else
		{
			if (xfer_size == 1)
			{
				m_ncr->dma_w(m_space->read_byte(m_dma_addr[DMA_SCSI][active_set]));
			}
			else
			{
				const u16 word = m_space->read_word(m_dma_addr[DMA_SCSI][active_set]);
				m_ncr->dma16_swap_w(word);
			}
		}

//      printf("SCSI DMA to %08x, %08x left\n", m_dma_addr[DMA_SCSI][active_set], m_dma_cnt[DMA_SCSI][active_set]);

		m_dma_cnt[DMA_SCSI][active_set] -= xfer_size;
		m_dma_addr[DMA_SCSI][active_set] += xfer_size;

		if (m_dma_cnt[DMA_SCSI][active_set] == 0)
		{
			m_dma_cmdstat[DMA_SCSI][active_set] &= ~CMD_ENABLED;
			m_dma_cmdstat[DMA_SCSI][active_set] |= CMD_IF;

			recalc_dma_irqs();
		}
	}
}

void psc_device::recalc_dma_irqs()
{
	// start off with no IRQs
	m_dma_irqstat = 0;

	// walk each DMA channel and set to see if anyone's interrupting
	for (int dma = 0; dma < DMA_NUM_CHANNELS; dma++)
	{
		m_dma_control[dma] &= ~CTRL_CIRQ;
		for (int active_set = 0; active_set < 2; active_set++)
		{
			if ((m_dma_cmdstat[dma][active_set] & (CMD_IE | CMD_IF)) == (CMD_IE | CMD_IF))
			{
				m_dma_control[dma] |= CTRL_CIRQ;
				if (m_dma_control[dma] & CTRL_CIE)
					m_dma_irqstat |= 0x80000000U >> dma;
			}
		}
	}

	if (m_dma_irqstat)
	{
		m_l4if |= LV4_DMA;
	}
	else
	{
		m_l4if &= ~LV4_DMA;
	}

	recalc_lv4();
}

void psc_device::recalc_lv3()
{
	const u8 lv3 = m_l3if & m_l3ier & 0x01;
	if (lv3)
	{
		m_l3if |= 0x80;
		m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
	}
	else
	{
		m_l3if &= ~0x80;
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
	}
}

void psc_device::recalc_lv4()
{
	const u8 lv4 = m_l4if & m_l4ier & 0x0f;
	if (lv4)
	{
		m_l4if |= 0x80;
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
	}
	else
	{
		m_l4if &= ~0x80;
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
	}
}

void psc_device::recalc_lv5()
{
	const u8 lv5 = m_l5if & m_l5ier & 0x03;
	if (lv5)
	{
		m_l5if |= 0x80;
		m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
	}
	else
	{
		m_l5if &= ~0x80;
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
	}
}

void psc_device::recalc_lv6()
{
	const u8 lv6 = m_l6if & m_l6ier & 0x07;
	if (lv6)
	{
		m_l6if |= 0x80;
		m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	}
	else
	{
		m_l6if &= ~0x80;
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}

u32 psc_device::psc_regs_r(offs_t offset)
{
	switch (offset << 2)
	{
		case 0x130:
			return m_l3if << 24;

		case 0x134:
			return m_l3ier << 24;

		case 0x140:
			return m_l4if << 24;

		case 0x144:
			return m_l4ier << 24;

		case 0x150:
			return m_l5if << 24;

		case 0x154:
			return m_l5ier << 24;

		case 0x160:
			return m_l6if << 24;

		case 0x164:
			return m_l6ier << 24;

		case 0x20c:
			if (m_audio_out_offset >= m_audio_out_length)
			{
				return 0;
			}

//          printf("Ofs %08x ret %08x\n", m_audio_out_offset, (m_audio_out_offset >> 2) << 11);
			return (m_audio_out_offset >> 2) << 11;

		case 0x804:
			return m_dma_irqstat;
	}

	return 0;
}

void psc_device::psc_regs_w(offs_t offset, u32 data, u32 mem_mask)
{
//  LOGMASKED(LOG_PSCREGS, "psc_regs_w: %08x @ %x, mask %04x (%s)\n", data, offset << 2, mem_mask, machine().describe_context());

	// PSC interrupt registers occupy the first (most significant) byte lane.
	if ((offset << 2) >= 0x130 && (offset << 2) <= 0x164)
	{
		if (!ACCESSING_BITS_24_31)
			return;
		data >>= 24;
	}

	switch (offset << 2)
	{
		// no bits are writable in this register
		case 0x0130:
			break;

		case 0x0134:
			if (BIT(data, 7))
			{
				m_l3ier |= (data & 0x1);
			}
			else
			{
				m_l3ier &= ~(data & 0x1);
			}
			recalc_lv3();
			break;

		case 0x0140:
			m_l4if &= ~1;
			m_l4if |= data & 1;
			recalc_lv4();
			break;

		case 0x0144:
			if (BIT(data, 7))
			{
				m_l4ier |= (data & 0xf);
			}
			else
			{
				m_l4ier &= ~(data & 0xf);
			}
			recalc_lv4();
			break;

		case 0x0150:
			m_l5if &= ~1;
			m_l5if |= data & 1;
			recalc_lv5();
			break;

		case 0x0154:
			if (BIT(data, 7))
			{
				m_l5ier |= (data & 0x3);
			}
			else
			{
				m_l5ier &= ~(data & 0x3);
			}
			recalc_lv5();
			break;

		// Level 6 interrupt flag - only bit 1 is writable
		case 0x0160:
			m_l6if &= ~1;
			m_l6if |= data & 1;
			recalc_lv6();
			break;

		case 0x0164:
			if (BIT(data, 7))
			{
				m_l6ier |= (data & 0x7);
			}
			else
			{
				m_l6ier &= ~(data & 0x7);
			}
			recalc_lv6();
			break;

		case 0x0200:
		{
			data >>= 16;

			if (BIT(data, 8))
			{   // start audio out DMA
				u8 rate = (data >> 9) & 3;
				int hz = 0;
				switch (rate)
				{
					case 0:
						hz = 24000;
						break;

					case 1:
						hz = 22050;
						break;

					case 2:
						hz = 48000;
						break;

					case 3:
						hz = 44100;
						break;
				}

				printf("Starting audio DMA @ %d Hz\n", hz);
				// force the first transfer to happen immediately
				m_singer_timer->adjust(attotime::zero, 0, attotime::from_hz(hz));
			}
			else // stop audio out DMA
			{
				m_singer_timer->adjust(attotime::never);
			}
		}
		break;

		case 0x0214:    // address
			m_audio_out_ptr = data;
			m_audio_out_offset = 0;
			break;

		case 0x0218: // count
			m_audio_out_length = data >> 13;
			//printf("length to %08x bytes\n", m_audio_out_length);
			break;

		case 0x021c: // DSPOVERRUN
			//printf("DSPOVERRUN (%x): RESETEN %d DSPRESET %d\n", data, BIT(data, 1), BIT(data, 0));
			if (!BIT(data, 31) && BIT(data, 24))
			{
				// DSP enable
				machine().debug_break();
			}
			break;
	}
}

TIMER_CALLBACK_MEMBER(psc_device::singer_tick)
{
	const u16 l = m_maincpu->space(AS_PROGRAM).read_word(m_audio_out_ptr + m_audio_out_offset);
	const u16 r = m_maincpu->space(AS_PROGRAM).read_word(m_audio_out_ptr + m_audio_out_offset + 2);
	m_dac_l->write(l ^ 0x8000);
	m_dac_r->write(r ^ 0x8000);
	m_audio_out_offset += 4;

	if (m_audio_out_offset > m_audio_out_length)
	{
		m_audio_out_offset = m_audio_out_length;
	}
}
