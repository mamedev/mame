/***************************************************************************

    Toshiba TMPZ84C015, TLCS-Z80 ASSP Family
    Z80 CPU, SIO, CTC, CGC(6/8MHz), PIO, WDT
    
    TODO:
    - SIO configuration, or should that be up to the driver?
    - CGC (clock generator/controller)
    - WDT (watchdog timer)

***************************************************************************/

#include "tmpz84c015.h"

const device_type TMPZ84C015 = &device_creator<tmpz84c015_device>;

static ADDRESS_MAP_START( tmpz84c015_internal_io_map, AS_IO, 8, tmpz84c015_device )
	AM_RANGE(0x10, 0x13) AM_MIRROR(0xff00) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xff00) AM_DEVREADWRITE("sio", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x1c, 0x1f) AM_MIRROR(0xff00) AM_DEVREADWRITE("pio", z80pio_device, read_alt, write_alt)
	AM_RANGE(0xf4, 0xf4) AM_MIRROR(0xff00) AM_WRITE(irq_priority_w)
ADDRESS_MAP_END


tmpz84c015_device::tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, TMPZ84C015, "TMPZ84C015", tag, owner, clock, "tmpz84c015", __FILE__),
	m_ctc(*this, "ctc"),
	m_sio(*this, "sio"),
	m_pio(*this, "pio"),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME( tmpz84c015_internal_io_map ) ),
	m_irq_priority(-1) // !
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmpz84c015_device::device_start()
{
	z80_device::device_start();
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
		static const char *dev[3] = { "ctc", "sio", "pio" };
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
		const char *daisy[4] = { dev[prio[data][0]], dev[prio[data][1]], dev[prio[data][2]], NULL };
		m_daisy.init(this, (const z80_daisy_config *)daisy);
		
		m_irq_priority = data;
	}
}

static MACHINE_CONFIG_FRAGMENT( tmpz84c015 )
	
	/* basic machine hardware */
	MCFG_DEVICE_ADD("ctc", Z80CTC, DERIVED_CLOCK(1,1) )
	MCFG_Z80CTC_INTR_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD("sio", DERIVED_CLOCK(1,1), 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("pio", Z80PIO, DERIVED_CLOCK(1,1) )
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))
MACHINE_CONFIG_END

machine_config_constructor tmpz84c015_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tmpz84c015 );
}
