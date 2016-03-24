// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Toshiba TMPZ84C015, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

    TODO:
    - SIO configuration, or should that be up to the driver?
    - CGC (clock generator/controller)
    - WDT (watchdog timer)

***************************************************************************/

#include "tmpz84c015.h"

const device_type TMPZ84C015 = &device_creator<tmpz84c015_device>;

static ADDRESS_MAP_START( tmpz84c015_internal_io_map, AS_IO, 8, tmpz84c015_device )
	AM_RANGE(0x10, 0x13) AM_MIRROR(0xff00) AM_DEVREADWRITE("tmpz84c015_ctc", z80ctc_device, read, write)
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xff00) AM_DEVREADWRITE("tmpz84c015_sio", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x1c, 0x1f) AM_MIRROR(0xff00) AM_DEVREADWRITE("tmpz84c015_pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0xf4, 0xf4) AM_MIRROR(0xff00) AM_WRITE(irq_priority_w)
ADDRESS_MAP_END


tmpz84c015_device::tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, TMPZ84C015, "TMPZ84C015", tag, owner, clock, "tmpz84c015", __FILE__),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME( tmpz84c015_internal_io_map ) ),
	m_ctc(*this, "tmpz84c015_ctc"),
	m_sio(*this, "tmpz84c015_sio"),
	m_pio(*this, "tmpz84c015_pio"),
	m_irq_priority(-1), // !

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

	m_zc0_cb(*this),
	m_zc1_cb(*this),
	m_zc2_cb(*this),

	m_in_pa_cb(*this),
	m_out_pa_cb(*this),
	m_out_ardy_cb(*this),

	m_in_pb_cb(*this),
	m_out_pb_cb(*this),
	m_out_brdy_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmpz84c015_device::device_start()
{
	z80_device::device_start();

	// resolve callbacks
	m_out_txda_cb.resolve_safe();
	m_out_dtra_cb.resolve_safe();
	m_out_rtsa_cb.resolve_safe();
	m_out_wrdya_cb.resolve_safe();
	m_out_synca_cb.resolve_safe();

	m_out_txdb_cb.resolve_safe();
	m_out_dtrb_cb.resolve_safe();
	m_out_rtsb_cb.resolve_safe();
	m_out_wrdyb_cb.resolve_safe();
	m_out_syncb_cb.resolve_safe();

	m_out_rxdrqa_cb.resolve_safe();
	m_out_txdrqa_cb.resolve_safe();
	m_out_rxdrqb_cb.resolve_safe();
	m_out_txdrqb_cb.resolve_safe();

	m_zc0_cb.resolve_safe();
	m_zc1_cb.resolve_safe();
	m_zc2_cb.resolve_safe();

	m_in_pa_cb.resolve_safe(0);
	m_out_pa_cb.resolve_safe();
	m_out_ardy_cb.resolve_safe();

	m_in_pb_cb.resolve_safe(0);
	m_out_pb_cb.resolve_safe();
	m_out_brdy_cb.resolve_safe();

	// register for save states
	save_item(NAME(m_irq_priority));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmpz84c015_device::device_reset()
{
	irq_priority_w(*m_io, 0, 0);
	z80_device::device_reset();
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void tmpz84c015_device::device_post_load()
{
	// reinit irq priority
	UINT8 prio = m_irq_priority;
	m_irq_priority = -1;
	irq_priority_w(*m_io, 0, prio);
}


/* CPU interface */
WRITE8_MEMBER(tmpz84c015_device::irq_priority_w)
{
	data &= 7;

	if (data > 5)
	{
		logerror("tmpz84c015: irq_priority_w undefined state %X\n", data);
		data &= 3; // guess
	}

	if (m_irq_priority != data)
	{
		static const char *dev[3] = { "tmpz84c015_ctc", "tmpz84c015_sio", "tmpz84c015_pio" };
		static const int prio[6][3] =
		{
			{ 0, 1, 2 }, // 0: ctc -> sio -> pio -> ext
			{ 1, 0, 2 }, // 1: sio -> ctc -> pio -> ext
			{ 0, 2, 1 }, // 2: ctc -> pio -> sio -> ext
			{ 2, 1, 0 }, // 3: pio -> sio -> ctc -> ext
			{ 2, 0, 1 }, // 4: pio -> ctc -> sio -> ext
			{ 1, 2, 0 }  // 5: sio -> pio -> ctc -> ext
		};

		// reconfigure first 3 entries in daisy chain
		const z80_daisy_config daisy_chain[] =
		{
			{ dev[prio[data][0]] },
			{ dev[prio[data][1]] },
			{ dev[prio[data][2]] },
			{ nullptr }
		};
		m_daisy.init(this, daisy_chain);

		m_irq_priority = data;
	}
}

static MACHINE_CONFIG_FRAGMENT( tmpz84c015 )

	/* basic machine hardware */
	MCFG_Z80SIO0_ADD("tmpz84c015_sio", DERIVED_CLOCK(1,1), 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))

	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE(tmpz84c015_device, out_txda_cb_trampoline_w))
	MCFG_Z80DART_OUT_DTRA_CB(WRITELINE(tmpz84c015_device, out_dtra_cb_trampoline_w))
	MCFG_Z80DART_OUT_RTSA_CB(WRITELINE(tmpz84c015_device, out_rtsa_cb_trampoline_w))
	MCFG_Z80DART_OUT_WRDYA_CB(WRITELINE(tmpz84c015_device, out_wrdya_cb_trampoline_w))
	MCFG_Z80DART_OUT_SYNCA_CB(WRITELINE(tmpz84c015_device, out_synca_cb_trampoline_w))

	MCFG_Z80DART_OUT_TXDB_CB(WRITELINE(tmpz84c015_device, out_txdb_cb_trampoline_w))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE(tmpz84c015_device, out_dtrb_cb_trampoline_w))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE(tmpz84c015_device, out_rtsb_cb_trampoline_w))
	MCFG_Z80DART_OUT_WRDYB_CB(WRITELINE(tmpz84c015_device, out_wrdyb_cb_trampoline_w))
	MCFG_Z80DART_OUT_SYNCB_CB(WRITELINE(tmpz84c015_device, out_syncb_cb_trampoline_w))

	MCFG_Z80DART_OUT_RXDRQA_CB(WRITELINE(tmpz84c015_device, out_rxdrqa_cb_trampoline_w))
	MCFG_Z80DART_OUT_TXDRQA_CB(WRITELINE(tmpz84c015_device, out_txdrqa_cb_trampoline_w))
	MCFG_Z80DART_OUT_RXDRQB_CB(WRITELINE(tmpz84c015_device, out_rxdrqb_cb_trampoline_w))
	MCFG_Z80DART_OUT_TXDRQB_CB(WRITELINE(tmpz84c015_device, out_txdrqb_cb_trampoline_w))

	MCFG_DEVICE_ADD("tmpz84c015_ctc", Z80CTC, DERIVED_CLOCK(1,1) )
	MCFG_Z80CTC_INTR_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))

	MCFG_Z80CTC_ZC0_CB(WRITELINE(tmpz84c015_device, zc0_cb_trampoline_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(tmpz84c015_device, zc1_cb_trampoline_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(tmpz84c015_device, zc2_cb_trampoline_w))

	MCFG_DEVICE_ADD("tmpz84c015_pio", Z80PIO, DERIVED_CLOCK(1,1) )
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))

	MCFG_Z80PIO_IN_PA_CB(READ8(tmpz84c015_device, in_pa_cb_trampoline_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(tmpz84c015_device, out_pa_cb_trampoline_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(tmpz84c015_device, out_ardy_cb_trampoline_w))

	MCFG_Z80PIO_IN_PB_CB(READ8(tmpz84c015_device, in_pb_cb_trampoline_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(tmpz84c015_device, out_pb_cb_trampoline_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(tmpz84c015_device, out_brdy_cb_trampoline_w))
MACHINE_CONFIG_END

machine_config_constructor tmpz84c015_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tmpz84c015 );
}
