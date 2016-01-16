// license:BSD-3-Clause
// copyright-holders:Ramiro Polla
/*
 * E05A30 Gate Array (used in the Epson ActionPrinter 2000)
 *
 */

#include "emu.h"
#include "e05a30.h"

//#define E05A30DEBUG
#ifdef E05A30DEBUG
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(...)
#endif


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

const device_type E05A30 = &device_creator<e05a30_device>;

e05a30_device::e05a30_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, E05A30, "E05A30", tag, owner, clock, "e05a30", __FILE__),
	m_write_printhead(*this),
	m_write_pf_stepper(*this),
	m_write_cr_stepper(*this),
	m_write_ready(*this),
	m_write_centronics_ack(*this),
	m_write_centronics_busy(*this),
	m_write_centronics_perror(*this),
	m_write_centronics_fault(*this),
	m_write_centronics_select(*this),
	m_printhead(0),
	m_pf_stepper(0),
	m_cr_stepper(0), m_centronics_data(0), m_centronics_busy(0), m_centronics_nack(0), m_centronics_strobe(0), m_centronics_data_latch(0), m_centronics_data_latched(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e05a30_device::device_start()
{
	/* resolve callbacks */
	m_write_printhead.resolve_safe();
	m_write_pf_stepper.resolve_safe();
	m_write_cr_stepper.resolve_safe();
	m_write_ready.resolve_safe();
	m_write_centronics_ack.resolve_safe();
	m_write_centronics_busy.resolve_safe();
	m_write_centronics_perror.resolve_safe();
	m_write_centronics_fault.resolve_safe();
	m_write_centronics_select.resolve_safe();

	/* register for state saving */
	save_item(NAME(m_printhead));
	save_item(NAME(m_pf_stepper));
	save_item(NAME(m_cr_stepper));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e05a30_device::device_reset()
{
	m_printhead  = 0x00;
	m_pf_stepper = 0x00;
	m_cr_stepper = 0x00;

	/* centronics init */
	m_centronics_nack = FALSE;
	m_centronics_busy = FALSE;
	m_write_centronics_ack   (!m_centronics_nack);
	m_write_centronics_busy  ( m_centronics_busy);
	m_write_centronics_perror(FALSE);
	m_write_centronics_fault (TRUE);
	m_write_centronics_select(TRUE);

	m_write_ready(1);
}


/***************************************************************************
    PRINT HEAD
***************************************************************************/

/* The e05a30 controls the printhead through MMIOs 0xC005 and 0xC006.
 * MMIO 0xC006 keeps the first 8 pins.
 * MMIO 0xC005 keeps the 9th pin in the MSB.
 */

void e05a30_device::update_printhead(int pos, UINT8 data)
{
	if (pos == 0) {
		m_printhead &= 0x01fe;
		m_printhead |= (data >> 7);
	} else {
		m_printhead &= 0x0001;
		m_printhead |= (UINT16) (data << 1);
	}
	m_write_printhead(m_printhead);
}

/***************************************************************************
    STEPPER MOTORS
***************************************************************************/

/* The e05a30 controls two stepper motors:
 * - The Paper Feed stepper motor is controlled through MMIO 0xC007
 * - The Carriage Return stepper motor is controlled through MMIO 0xC008
 * The Carriage Return stepper motor is used throug the SLA7020M driver. It
 * is therefore necessary to translate the input data from the SLA7020M
 * format to a format describing the 4 phases of a stepper motor.
 * For the PF motor, the output data is fed directly to the stepper motor.
 */

void e05a30_device::update_pf_stepper(UINT8 data)
{
	m_pf_stepper = data & 0x0f;
	m_write_pf_stepper(m_pf_stepper);
}

static UINT8 cr_sla7020m(UINT8 data)
{
	bool ina = BIT(data, 0);
	bool inb = BIT(data, 1);
	bool tda = BIT(data, 2);
	bool tdb = BIT(data, 3);
	bool outa0 =  ina && tda;
	bool outa1 = !ina && tda;
	bool outb0 =  inb && tdb;
	bool outb1 = !inb && tdb;
	return (outb1<<3)|(outb0<<2)|(outa1<<1)|(outa0<<0);
}
void e05a30_device::update_cr_stepper(UINT8 data)
{
	m_cr_stepper = data & 0x0f;
	m_write_cr_stepper(cr_sla7020m(m_cr_stepper));
}


/***************************************************************************
    Centronics
***************************************************************************/

WRITE_LINE_MEMBER( e05a30_device::centronics_input_strobe )
{
	if (m_centronics_strobe == TRUE && state == FALSE && !m_centronics_busy) {
		m_centronics_data_latch   = m_centronics_data;

		m_centronics_data_latched = TRUE;
		m_centronics_busy         = TRUE;
		m_write_centronics_busy(m_centronics_busy);
	}

	m_centronics_strobe = state;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

WRITE8_MEMBER( e05a30_device::write )
{
	LOG("%s: e05a30_w([0xC0%02x]): %02x\n", space.machine().describe_context(), offset, data);

	switch (offset) {
	case 0x04:
		m_centronics_nack = BIT(data,5);
		m_centronics_busy = BIT(data,0);
		/* The ActionPrinter 2000 firmware might overwrite the busy signal at
		 * address 20AB if the host depends only on the busy signal and
		 * doesn't wait for the ack pulse. To avoid skipping input data, we
		 * assume the busy signal cannot be reset while the data hasn't been
		 * read. */
		if (m_centronics_data_latched)
			m_centronics_busy = TRUE;
		m_write_centronics_ack (!m_centronics_nack);
		m_write_centronics_busy( m_centronics_busy);
		break;
	/* printhead */
	case 0x05: update_printhead(0, data); break;
	case 0x06: update_printhead(1, data); break;
	/* paper feed stepper motor */
	case 0x07: update_pf_stepper(data); break;
	/* carriage return stepper motor */
	case 0x08: update_cr_stepper(data); break;
	}
}

READ8_MEMBER( e05a30_device::read )
{
	UINT8 result = 0;

	LOG("%s: e05a30_r([0xC0%02x]): ", space.machine().describe_context(), offset);

	switch (offset) {
	case 0x02:
		result = m_centronics_data_latched << 7;
		break;
	case 0x03:
		result = m_centronics_data_latch;
		m_centronics_data_latched = FALSE;
		break;
	case 0x04:
		result |= m_centronics_busy << 0;
		result |= m_centronics_nack << 5;
		break;
	/* paper feed stepper motor */
	case 0x07: result = m_pf_stepper; break;
	/* carriage return stepper motor */
	case 0x08: result = m_cr_stepper; break;
	}

	LOG("0x%02x\n", result);

	return result;
}
