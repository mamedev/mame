// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

	RX01 floppy drive interface

***************************************************************************/

#include "emu.h"

#include "rx11.h"

#include "formats/rx01_dsk.h"


//#define LOG_GENERAL (1U <<  0)
#define LOG_DBG     (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_DBG)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGDBG(format, ...)   LOGMASKED(LOG_DBG, "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define raise_done()	do { \
		m_rxcs |= RXCS_DONE; \
		raise_virq(m_bus->birq4_w, m_rxcs, CSR_IE, m_done); } while (0)

#define clear_done()	do { \
		m_rxcs &= ~RXCS_DONE; \
		clear_virq(m_bus->birq4_w, m_rxcs, CSR_IE, m_done); } while (0)


void rx11_device::position_head()
{
	int cur_track = m_image[m_unit]->floppy_drive_get_current_track();
	int dir = (cur_track < m_rxta) ? +1 : -1;

	while (m_rxta != cur_track)
	{
		cur_track += dir;

		m_image[m_unit]->floppy_drive_seek(dir);
	}
}

void rx11_device::read_sector()
{
	m_image[m_unit]->floppy_drive_read_sector_data(0, m_rxsa - 1, &m_buf[0], 128);
	if (m_ddam[m_unit * RX_NUMTR * RX_NUMSC + m_rxta * RX_NUMSC + m_rxsa - 1])
		m_rxes[m_unit] |= RXES_DD;
}

void rx11_device::write_sector(bool ddam)
{
	m_image[m_unit]->floppy_drive_write_sector_data(0, m_rxsa - 1, &m_buf[0], 128, ddam);
	m_ddam[m_unit * RX_NUMTR * RX_NUMSC + m_rxta * RX_NUMSC + m_rxsa - 1] = ddam;
}

READ16_MEMBER(rx11_device::read)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_rxcs & RXCS_RD;
		break;

	case 1:
		switch (m_state)
		{
		case TIMER_ID_EMPTY:
			if (m_count_read < 128)
			{
				data = m_buf[m_count_read++];
				m_timer_wait->adjust(attotime::from_usec(m_waittime));
			}
			if (m_count_read == 128)
			{
				m_state = 0;
				raise_done();
			}
			m_rxcs &= ~RXCS_TR;
			break;

		default:
			if (m_count_write == 0 || (m_rxcs & RXCS_TR)) // diags and manual disagree
				data = m_rxdb;
			break;
		}
		break;
	}

	LOGDBG("R %6o == %06o @ %d; state %d\n",
		0177170 + (offset << 1), data, m_state == TIMER_ID_EMPTY ? (m_count_read - 1) : -1, m_state);

	return data;
}

WRITE16_MEMBER(rx11_device::write)
{
	LOGDBG("W %6o <- %06o & %06o%s @ %d; state %d\n",
		0177170 + (offset << 1), data, mem_mask, mem_mask==0xffff?"":" WARN",
		m_state == TIMER_ID_FILL ? m_count_write : -1, m_state);

	switch (offset)
	{
	case 0:
		if ((data & RXCS_IE) == 0)
		{
			clear_virq(m_bus->birq4_w, 1, 1, m_done);
		}
		else if ((m_rxcs & (RXCS_DONE + RXCS_IE)) == RXCS_DONE)
		{
			raise_virq(m_bus->birq4_w, 1, 1, m_done);
		}
		UPDATE_16BIT(&m_rxcs, data, RXCS_WR);
		if (data & RXCS_INIT)
		{
			device_reset();
		}
		else if (data & CSR_GO)
		{
			m_unit = BIT(data, 4);
			command(RXCS_GETFNC(data));
		}
		else if (data == 0)
		{
			m_rxdb = 0; // per ZRXBF0 line 918
		}
		break;

	case 1:
		switch (m_state)
		{
		case TIMER_ID_FILL:
			if (m_count_write < 128)
			{
				m_buf[m_count_write++] = data;
			}
			if (m_count_write == 128)
			{
				raise_done();
			}
			m_rxcs &= ~RXCS_TR;
			m_timer_wait->adjust(attotime::from_usec(m_waittime));
			break;

		case TIMER_ID_SECTOR:
			m_rxsa = data & RX_M_SECTOR;
			m_state = TIMER_ID_TRACK;
			m_rxcs &= ~RXCS_TR;
			m_timer_wait->adjust(attotime::from_usec(m_waittime));
			break;

		case TIMER_ID_TRACK:
			m_rxta = data & RX_M_TRACK;
			m_state = RXCS_GETFNC(m_rxcs) == RXCS_READ ? TIMER_ID_READ : TIMER_ID_WRITE;
			m_rxcs &= ~RXCS_TR;
			position_head();
			m_timer_wait->adjust(attotime::from_usec(m_waittime));
			break;
		}
	}
}

void rx11_device::command(int command)
{
	LOG("command %o drive %d\n", command, m_unit);

	m_rxes[m_unit] &= ~RXES_ID;
	m_rxcs &= ~RXCS_ERR;
	m_count_write = m_count_read = 0;

	switch (command)
	{
	case RXCS_FILL: // write from host to buffer
		clear_done();
		m_state = TIMER_ID_FILL;
		m_timer_wait->adjust(attotime::from_usec(m_waittime));
		break;

	case RXCS_EMPTY: // read buffer to host
		clear_done();
		m_state = TIMER_ID_EMPTY;
		m_timer_wait->adjust(attotime::from_usec(m_waittime));
		break;

	case RXCS_READ: // read disk sector to buffer
		m_rxes[m_unit] &= ~(RXES_CRC | RXES_PAR | RXES_DD);
		clear_done();
		m_state = TIMER_ID_SECTOR;
		m_timer_wait->adjust(attotime::from_usec(m_waittime));
		break;

	case RXCS_WRITE: // write buffer to disk
	case RXCS_WRDEL: // write buffer to disk (deleted data)
		m_rxes[m_unit] &= ~(RXES_CRC | RXES_PAR | RXES_DD);
		clear_done();
		m_state = TIMER_ID_SECTOR;
		m_timer_wait->adjust(attotime::from_usec(m_waittime));
		break;

	case RXCS_RXES:
		clear_done();
		m_state = TIMER_ID_RXES;
		m_timer_wait->adjust(attotime::from_msec(250));
		break;

	case RXCS_ECODE:
		m_rxes[m_unit] &= ~0177;
		clear_done();
		m_state = TIMER_ID_ECODE;
		m_timer_wait->adjust(attotime::from_usec(m_waittime));
		break;
	}
}

void rx11_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	bool deleted = RXCS_GETFNC(m_rxcs) == RXCS_WRDEL;

	LOGDBG("Timer %d fired, state %d\n", id, m_state);

	switch (m_state)
	{
	case TIMER_ID_RESET:
		m_rxes[m_unit] = RXES_ID;
		m_rxta = 1;
		position_head();
		m_rxsa = 1;
		read_sector();
		m_rxdb = m_rxes[m_unit] |
				 (m_image[m_unit]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY) ? RXES_DRDY : 0);
		logerror("reset[%d]: %06o\n", m_unit, m_rxdb);
		m_state = 0;
		raise_done();
		break;

	case TIMER_ID_RXES:
		m_rxdb = m_rxes[m_unit] |
				 (m_image[m_unit]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY) ? RXES_DRDY : 0);
		logerror("rxes[%d]: %06o\n", m_unit, m_rxdb);
		m_state = 0;
		raise_done();
		break;

	case TIMER_ID_FILL:
	case TIMER_ID_EMPTY:
	case TIMER_ID_SECTOR:
	case TIMER_ID_TRACK:
		m_rxcs |= RXCS_TR;
		break;

	case TIMER_ID_READ:
		m_state = 0;
		if (m_rxta >= RX_NUMTR)
		{
			m_rxcs |= RXCS_ERR;
			m_rxec = RXEC_TRACK77;
		}
		else if (m_rxsa > RX_NUMSC || m_rxsa == 0)
		{
			m_rxcs |= RXCS_ERR;
			m_rxec = RXEC_SECTERR;
		}
		else
		{
			read_sector();
		}
		LOG("read from drive %d c:h:s %d:%d:%d (error %04o)\n",
			m_unit, m_rxta, 0, m_rxsa, (m_rxcs & RXCS_ERR) ? m_rxec : 0);
		m_rxdb = m_rxes[m_unit];
		raise_done();
		break;

	case TIMER_ID_WRITE:
		m_state = 0;
		if (m_rxta >= RX_NUMTR)
		{
			m_rxcs |= RXCS_ERR;
			m_rxec = RXEC_TRACK77;
		}
		else if (m_rxsa > RX_NUMSC || m_rxsa == 0)
		{
			m_rxcs |= RXCS_ERR;
			m_rxec = RXEC_SECTERR;
		}
		else
		{
			write_sector(deleted);
		}
		LOG("write%s to drive %d c:h:s %d:%d:%d (error %04o)\n", deleted ? " deleted" : "",
				 m_unit, m_rxta, 0, m_rxsa, (m_rxcs & RXCS_ERR) ? m_rxec : 0);
		m_rxdb = m_rxes[m_unit] | (deleted ? RXES_DD : 0);
		raise_done();
		break;

	case TIMER_ID_ECODE:
		m_state = 0;
		m_rxdb = m_rxec;
		raise_done();
		break;
	}
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static const floppy_interface rx01_floppy_interface =
{
	FLOPPY_STANDARD_8_SSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(rx01),
	"floppy_8"
};

DEFINE_DEVICE_TYPE(DEC_RX11, rx11_device, "rx11", "DEC RX11 controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rx11_device - constructor
//-------------------------------------------------

rx11_device::rx11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DEC_RX11, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_image(*this, "floppy%u", 0U)
	, m_vec(0264)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void rx11_device::device_add_mconfig(machine_config &config)
{
	for (auto &floppy : m_image)
		LEGACY_FLOPPY(config, floppy, 0, &rx01_floppy_interface);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rx11_device::device_start()
{
	// save state
	save_item(NAME(m_rxcs));
	save_item(NAME(m_rxdb));
	save_item(NAME(m_rxta));
	save_item(NAME(m_rxsa));
	save_item(NAME(m_rxes));
	save_item(NAME(m_rxec));

	save_item(NAME(m_count_read));
	save_item(NAME(m_count_write));
	save_item(NAME(m_unit));
	save_item(NAME(m_state));
	save_item(NAME(m_seektime));
	save_item(NAME(m_waittime));

	m_timer_wait = timer_alloc(TIMER_ID_WAIT);

	m_buf = std::make_unique<uint8_t[]>(128);
	m_ddam = std::make_unique<bool[]>(77 * 26 * 2);
	m_seektime = 10; // ms, per EK-RX01-OP-001 page 1-13
	m_waittime = 80; // us, per EK-RX01-OP-001 page 4-1: 18 us for PDP-8 card

	m_bus->install_device(0177170, 0177173, read16_delegate(*this, FUNC(rx11_device::read)),
		write16_delegate(*this, FUNC(rx11_device::write)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rx11_device::device_reset()
{
	for (auto &elem : m_image)
	{
		elem->floppy_mon_w(0); // turn it on
		elem->floppy_drive_set_controller(this);
		elem->floppy_drive_set_rpm(360.);
		elem->floppy_drive_set_ready_state(ASSERT_LINE, 1);
	}

	m_rxcs = 0;
	m_rxta = m_rxsa = 0;
	m_rxdb = m_rxes[0] = m_rxes[1] = m_rxec = 0;

	m_done = CLEAR_LINE;

	m_count_read = m_count_write = m_unit = 0;

	m_state = TIMER_ID_RESET;
	m_timer_wait->adjust(attotime::from_usec(m_waittime));
}

int rx11_device::z80daisy_irq_state()
{
	if (m_done == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int rx11_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_done == ASSERT_LINE)
	{
		m_done = CLEAR_LINE;
		vec = m_vec;
	}

	if (vec > 0)
		LOGDBG("IRQ ACK vec %03o\n", vec);

	return vec;
}

void rx11_device::z80daisy_irq_reti()
{
}
