// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Toshiba TMPZ84C015, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

    TODO:
    - SIO configuration, or should that be up to the driver?
    - CGC (clock generator/controller)
    - Halt modes

***************************************************************************/

#include "emu.h"
#include "tmpz84c015.h"

DEFINE_DEVICE_TYPE(TMPZ84C015, tmpz84c015_device, "tmpz84c015", "Toshiba TMPZ84C015")

void tmpz84c015_device::internal_io_map(address_map &map) const
{
	map(0x10, 0x13).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x18, 0x1b).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x1c, 0x1f).mirror(0xff00).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xf0, 0xf0).mirror(0xff00).rw(FUNC(tmpz84c015_device::wdtmr_r), FUNC(tmpz84c015_device::wdtmr_w));
	map(0xf1, 0xf1).mirror(0xff00).w(FUNC(tmpz84c015_device::wdtcr_w));
	map(0xf4, 0xf4).mirror(0xff00).w(FUNC(tmpz84c015_device::irq_priority_w));
}

tmpz84c015_device::tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tmpz84c015_device(mconfig, TMPZ84C015, tag, owner, clock, address_map_constructor(FUNC(tmpz84c015_device::internal_io_map), this))
{
}

tmpz84c015_device::tmpz84c015_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor io_map) :
	z80_device(mconfig, type, tag, owner, clock),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 16, 0, io_map),
	m_ctc(*this, "tmpz84c015_ctc"),
	m_sio(*this, "tmpz84c015_sio"),
	m_pio(*this, "tmpz84c015_pio"),
	m_irq_priority(-1), // !
	m_wdtmr(0xfb),
	m_watchdog_timer(nullptr),

	m_out_txda_cb(*this),
	m_out_dtra_cb(*this),
	m_out_rtsa_cb(*this),
	m_out_wrdya_cb(*this),
	m_out_synca_cb(*this),

	m_out_txdb_cb(*this),
	m_out_dtrb_cb(*this),
	m_out_rtsb_cb(*this),
	m_out_wrdyb_cb(*this),
	m_out_syncb_cb(*this),

	m_out_rxdrqa_cb(*this),
	m_out_txdrqa_cb(*this),
	m_out_rxdrqb_cb(*this),
	m_out_txdrqb_cb(*this),

	m_zc_cb(*this),

	m_in_pa_cb(*this, 0),
	m_out_pa_cb(*this),
	m_out_ardy_cb(*this),

	m_in_pb_cb(*this, 0),
	m_out_pb_cb(*this),
	m_out_brdy_cb(*this),

	m_wdtout_cb(*this)
{
}

device_memory_interface::space_config_vector tmpz84c015_device::memory_space_config() const
{
	auto r = z80_device::memory_space_config();
	r.back().second = &m_io_space_config;
	return r;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmpz84c015_device::device_start()
{
	z80_device::device_start();

	// setup watchdog timer
	m_watchdog_timer = timer_alloc(FUNC(tmpz84c015_device::watchdog_timeout), this);

	// register for save states
	save_item(NAME(m_irq_priority));
	save_item(NAME(m_wdtmr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmpz84c015_device::device_reset()
{
	irq_priority_w(0);
	m_wdtmr = 0xfb;
	watchdog_clear();

	z80_device::device_reset();
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void tmpz84c015_device::device_post_load()
{
	z80_device::device_post_load();

	// reinit irq priority
	uint8_t prio = m_irq_priority;
	m_irq_priority = -1;
	irq_priority_w(prio);
}


/* CPU interface */
void tmpz84c015_device::irq_priority_w(uint8_t data)
{
	data &= 7;

	if (data > 5)
	{
		logerror("tmpz84c015: irq_priority_w undefined state %X\n", data);
		data &= 3; // guess
	}

	if (m_irq_priority != data)
	{
		static const char *const dev[3] = { "tmpz84c015_ctc", "tmpz84c015_sio", "tmpz84c015_pio" };
		static const int prio[6][3] =
		{
			{ 0, 1, 2 }, // 0: ctc -> sio -> pio -> ext
			{ 1, 0, 2 }, // 1: sio -> ctc -> pio -> ext
			{ 0, 2, 1 }, // 2: ctc -> pio -> sio -> ext
			{ 2, 1, 0 }, // 3: pio -> sio -> ctc -> ext
			{ 2, 0, 1 }, // 4: pio -> ctc -> sio -> ext
			{ 1, 2, 0 }  // 5: sio -> pio -> ctc -> ext
		};

		// reconfigure daisy chain
		const z80_daisy_config daisy_chain[] =
		{
			{ dev[prio[data][0]] },
			{ dev[prio[data][1]] },
			{ dev[prio[data][2]] },
			{ nullptr }
		};

		// insert these 3 entries in order before any externally linked devices
		daisy_init(daisy_chain);

		m_irq_priority = data;
	}
}


uint8_t tmpz84c015_device::wdtmr_r()
{
	return m_wdtmr;
}

void tmpz84c015_device::wdtmr_w(uint8_t data)
{
	if ((data & 0x07) != 0x03)
		logerror("%s: Writing %d%d%d to WDTMR reserved bits\n", machine().describe_context(), BIT(data, 2), BIT(data, 1), BIT(data, 0));

	// check for watchdog timer enable
	uint8_t old_data = std::exchange(m_wdtmr, data);
	if (!BIT(old_data, 7) && BIT(data, 7))
		watchdog_clear();
}

void tmpz84c015_device::wdtcr_w(uint8_t data)
{
	// write specific values only
	switch (data)
	{
	case 0x4e:
		watchdog_clear();
		break;

	case 0xb1:
		// WDTER must be cleared first
		if (!BIT(m_wdtmr, 7))
		{
			logerror("%s: Watchdog timer disabled\n", machine().describe_context());
			m_watchdog_timer->adjust(attotime::never);
		}
		break;

	default:
		logerror("%s: Writing %02X to WDTCR\n", machine().describe_context(), data);
		break;
	}
}

void tmpz84c015_device::watchdog_clear()
{
	m_wdtout_cb(CLEAR_LINE);

	if (BIT(m_wdtmr, 7))
		m_watchdog_timer->adjust(cycles_to_attotime(0x10000 << (BIT(m_wdtmr, 5, 2) * 2)));
}

TIMER_CALLBACK_MEMBER(tmpz84c015_device::watchdog_timeout)
{
	logerror("Watchdog timeout\n");
	m_wdtout_cb(ASSERT_LINE);
}


void tmpz84c015_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	Z80SIO(config, m_sio, DERIVED_CLOCK(1,1));
	m_sio->out_int_callback().set_inputline(DEVICE_SELF, INPUT_LINE_IRQ0);

	m_sio->out_txda_callback().set(FUNC(tmpz84c015_device::out_txda_cb_trampoline_w));
	m_sio->out_dtra_callback().set(FUNC(tmpz84c015_device::out_dtra_cb_trampoline_w));
	m_sio->out_rtsa_callback().set(FUNC(tmpz84c015_device::out_rtsa_cb_trampoline_w));
	m_sio->out_wrdya_callback().set(FUNC(tmpz84c015_device::out_wrdya_cb_trampoline_w));
	m_sio->out_synca_callback().set(FUNC(tmpz84c015_device::out_synca_cb_trampoline_w));

	m_sio->out_txdb_callback().set(FUNC(tmpz84c015_device::out_txdb_cb_trampoline_w));
	m_sio->out_dtrb_callback().set(FUNC(tmpz84c015_device::out_dtrb_cb_trampoline_w));
	m_sio->out_rtsb_callback().set(FUNC(tmpz84c015_device::out_rtsb_cb_trampoline_w));
	m_sio->out_wrdyb_callback().set(FUNC(tmpz84c015_device::out_wrdyb_cb_trampoline_w));
	m_sio->out_syncb_callback().set(FUNC(tmpz84c015_device::out_syncb_cb_trampoline_w));

	m_sio->out_rxdrqa_callback().set(FUNC(tmpz84c015_device::out_rxdrqa_cb_trampoline_w));
	m_sio->out_txdrqa_callback().set(FUNC(tmpz84c015_device::out_txdrqa_cb_trampoline_w));
	m_sio->out_rxdrqb_callback().set(FUNC(tmpz84c015_device::out_rxdrqb_cb_trampoline_w));
	m_sio->out_txdrqb_callback().set(FUNC(tmpz84c015_device::out_txdrqb_cb_trampoline_w));

	Z80CTC(config, m_ctc, DERIVED_CLOCK(1,1));
	m_ctc->intr_callback().set_inputline(DEVICE_SELF, INPUT_LINE_IRQ0);

	m_ctc->zc_callback<0>().set(FUNC(tmpz84c015_device::zc_cb_trampoline_w<0>));
	m_ctc->zc_callback<1>().set(FUNC(tmpz84c015_device::zc_cb_trampoline_w<1>));
	m_ctc->zc_callback<2>().set(FUNC(tmpz84c015_device::zc_cb_trampoline_w<2>));
	m_ctc->zc_callback<3>().set(FUNC(tmpz84c015_device::zc_cb_trampoline_w<3>));

	Z80PIO(config, m_pio, DERIVED_CLOCK(1,1));
	m_pio->out_int_callback().set_inputline(DEVICE_SELF, INPUT_LINE_IRQ0);

	m_pio->in_pa_callback().set(FUNC(tmpz84c015_device::in_pa_cb_trampoline_r));
	m_pio->out_pa_callback().set(FUNC(tmpz84c015_device::out_pa_cb_trampoline_w));
	m_pio->out_ardy_callback().set(FUNC(tmpz84c015_device::out_ardy_cb_trampoline_w));

	m_pio->in_pb_callback().set(FUNC(tmpz84c015_device::in_pb_cb_trampoline_r));
	m_pio->out_pb_callback().set(FUNC(tmpz84c015_device::out_pb_cb_trampoline_w));
	m_pio->out_brdy_callback().set(FUNC(tmpz84c015_device::out_brdy_cb_trampoline_w));
}
