// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Terak Fixed Density Disk Drive Contoller (driver QX.SYS)

    Not implemented: deleted data, auto-unload, write protect

***************************************************************************/

#include "emu.h"
#include "terak_qx.h"

#include "formats/rx01_dsk.h"
#include "imagedev/flopdrv.h"
#include "machine/pdp11.h"


#define LOG_DBG     (1U << 1)

// #define VERBOSE (LOG_GENERAL | LOG_DBG)

#include "logmacro.h"

#define LOGDBG(format, ...)   LOGMASKED(LOG_DBG, "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)


namespace {

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define raise_done()	do { \
	m_qxcs |= QXCS_DONE; \
	raise_virq(m_bus->birq4_w, m_qxcs, QXCS_IE, m_done); } while (0)

#define clear_done()	do { \
	m_qxcs &= ~QXCS_DONE; \
	clear_virq(m_bus->birq4_w, m_qxcs, QXCS_IE, m_done); } while (0)

// #defines for easier diff with SIMH

#define QX_NUMTR        77                              /* tracks/disk */
#define QX_NUMSC        26                              /* sectors/track */
#define QX_NUMBY        128                             /* bytes/sector */

#define QXCS_V_FUNC     1                               /* function */
#define QXCS_M_FUNC     7
#define  QXCS_NOP       0                               /* no op */
#define  QXCS_RTC       1                               /* 2ms clock */
#define  QXCS_STEP_IN   2                               /* step in */
#define  QXCS_STEP_OUT  3                               /* step out */
#define  QXCS_READ_ID   4                               /* read track/sector */
#define  QXCS_READ      5                               /* read data */
#define  QXCS_WRITE     6                               /* write data */
#define  QXCS_WRDEL     7                               /* write del data */
#define QXCS_V_HD       4                               /* head down */
#define QXCS_V_IE       6                               /* intr enable */
#define QXCS_V_DONE     7                               /* done */
#define QXCS_V_DRV      8                               /* drive select */
#define QXCS_M_DRV      3
#define QXCS_V_TRK0     9                               /* track 0 */
#define QXCS_V_DD       10                              /* deleted data */
#define QXCS_V_WP       11                              /* write protect */
#define QXCS_V_NR       15                              /* not ready / error */
#define QXCS_M_ERR      077
#define QXCS_FUNC       (QXCS_M_FUNC << QXCS_V_FUNC)
#define QXCS_DRV        (QXCS_M_DRV << QXCS_V_DRV)
#define QXCS_ERR        (QXCS_M_ERR << QXCS_V_DD)
#define QXCS_DONE       (1u << QXCS_V_DONE)
#define QXCS_HD         (1u << QXCS_V_HD)
#define QXCS_IE         (1u << QXCS_V_IE)
#define QXCS_TRK0       (1u << QXCS_V_TRK0)
#define QXCS_WP         (1u << QXCS_V_WP)
#define QXCS_NR         (1u << QXCS_V_NR)
#define QXCS_RD         (QXCS_DONE+QXCS_IE+QXCS_HD+QXCS_ERR)
#define QXCS_WR         (QXCS_IE+QXCS_HD)
#define QXCS_GETFNC(x)  (((x) >> QXCS_V_FUNC) & QXCS_M_FUNC)
#define QXCS_GETDRV(x)  (((x) >> QXCS_V_DRV) & QXCS_M_DRV)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> terak_qx_device

class terak_qx_device : public device_t, public device_qbus_card_interface
{
public:
	terak_qx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset, uint16_t mem_mask);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask);

	virtual void init_w() override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

	enum
	{
		IDLE = 0,
		TIMER_ID_NOP,
		TIMER_ID_SEEK,
		TIMER_ID_READ_ID,
		TIMER_ID_READ_ID_COMPLETE,
		TIMER_ID_READ,
		TIMER_ID_WRITE,
	};

	TIMER_CALLBACK_MEMBER(wait_tick);

private:
	void command(int command);

	required_device_array<legacy_floppy_image_device, 4> m_image;

	bool m_installed;
	int m_vec;

	uint16_t m_qxcs;
	uint16_t m_qxdb;

	std::unique_ptr<uint8_t[]> m_buf;
	int m_count_read, m_count_write;
	int m_qxta, m_qxsa;
	int m_unit, m_state;
	int m_seektime, m_waittime;

	line_state m_done;

	emu_timer *m_timer_wait;
};



uint16_t terak_qx_device::read(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_qxcs & QXCS_RD;
		if (!m_image[m_unit]->floppy_tk00_r()) data |= QXCS_TRK0;
		break;

	case 1:
		if (m_state && m_state != TIMER_ID_READ_ID_COMPLETE)
		{
			m_bus->bus_error_w(0);
			break;
		}
		if (m_state == TIMER_ID_READ_ID_COMPLETE)
		{
			data = (m_qxsa << 8) | m_qxta;
			break;
		}
		if (mem_mask == 0xffff)
		{
			data = m_buf[m_count_read++];
			data |= m_buf[m_count_read++] << 8;
		}
		else if (mem_mask == 0x00ff)
		{
			data = m_buf[m_count_read++];
		}
		else
		{
			data = m_buf[m_count_read++] << 8;
		}
		m_count_read %= 128;
		break;
	}

	if ((!offset && (data & QXCS_DONE))) // || offset)
	LOGDBG("R %6o/%06o == %06o @ %d; state %d\n",
		0177000 + (offset << 1), mem_mask, data, m_count_read, m_state);

	return data;
}

void terak_qx_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGDBG("W %6o/%06o <- %06o (%06o) @ %d; state %d\n",
		0177000 + (offset << 1), mem_mask, data, m_qxcs, -1, m_state);

	if (m_state && m_state != TIMER_ID_READ_ID_COMPLETE)
	{
		m_bus->bus_error_w(0);
		return;
	}

	switch (offset)
	{
	case 0:
		m_count_read = m_count_write = 0;
		if ((data & QXCS_IE) == 0)
		{
			clear_virq(m_bus->birq4_w, 1, 1, m_done);
		}
		else if ((m_qxcs & (QXCS_DONE + QXCS_IE)) == QXCS_DONE)
		{
			raise_virq(m_bus->birq4_w, 1, 1, m_done);
		}
		UPDATE_16BIT(&m_qxcs, data, QXCS_WR);
		if (data & CSR_GO)
		{
			m_unit = QXCS_GETDRV(data);
			command(QXCS_GETFNC(data));
		}
		break;

	case 1:
		if (mem_mask == 0xffff)
		{
			m_buf[m_count_write++] = data;
			m_buf[m_count_write++] = data >> 8;
		}
		m_count_write %= 128;
		break;
	}
}

void terak_qx_device::command(int command)
{
	static const char *t[8] = { "nop", "rtc", "step+", "step-", "read id", "read", "write", "wrdel" };

	LOG("drive %d command %s (%o)\n", m_unit, t[command], command);

	m_qxcs &= ~QXCS_ERR;
	m_count_write = m_count_read = 0;

	clear_done();

	switch (command)
	{
	case QXCS_NOP:
		m_state = TIMER_ID_NOP;
		m_timer_wait->adjust(attotime::from_msec(400));
		break;

	case QXCS_RTC:
		m_state = TIMER_ID_NOP;
		m_timer_wait->adjust(attotime::from_msec(2));
		break;

	case QXCS_STEP_IN:
		m_state = TIMER_ID_SEEK;
		m_timer_wait->adjust(attotime::from_msec(m_seektime));
		m_image[m_unit]->floppy_drive_seek(+1);
		break;

	case QXCS_STEP_OUT:
		m_state = TIMER_ID_SEEK;
		m_timer_wait->adjust(attotime::from_msec(m_seektime));
		m_image[m_unit]->floppy_drive_seek(-1);
		break;

	case QXCS_READ_ID:
		m_qxsa++;
		if (m_qxsa > QX_NUMSC) m_qxsa = 1;
		m_state = TIMER_ID_READ_ID;
		m_timer_wait->adjust(attotime::from_msec(8));
		break;

	case QXCS_READ: // read disk sector to buffer
		m_state = TIMER_ID_READ;
		m_timer_wait->adjust(attotime::from_msec(m_waittime));
		break;

	case QXCS_WRITE: // write buffer to disk
	case QXCS_WRDEL: // write buffer to disk (deleted data)
		m_state = TIMER_ID_WRITE;
		m_timer_wait->adjust(attotime::from_msec(m_waittime));
		break;
	}
}

TIMER_CALLBACK_MEMBER(terak_qx_device::wait_tick)
{
	bool deleted = QXCS_GETFNC(m_qxcs) == QXCS_WRDEL;
	int next_state = IDLE;

	LOGDBG("Timer fired, state %d\n", m_state);

	switch (m_state)
	{
	case TIMER_ID_NOP:
	case TIMER_ID_READ_ID_COMPLETE:
		break;

	case TIMER_ID_SEEK:
		m_qxta = m_image[m_unit]->floppy_drive_get_current_track();
		m_qxsa = 0;
		break;

	case TIMER_ID_READ_ID:
		LOG("read id from drive %d = c:h:s %d:0:%d (%06o)\n", m_unit, m_qxta, m_qxsa, (m_qxsa << 8) | m_qxta);
		next_state = TIMER_ID_READ_ID_COMPLETE;
		m_timer_wait->adjust(attotime::from_usec(300));
		break;

	case TIMER_ID_READ:
		m_image[m_unit]->floppy_drive_read_sector_data(0, (m_qxsa - 1), &m_buf[0], 128);
		LOG("read from drive %d c:h:s %d:0:%d\n", m_unit, m_qxta, m_qxsa);
		break;

	case TIMER_ID_WRITE:
		m_image[m_unit]->floppy_drive_write_sector_data(0, (m_qxsa - 1), &m_buf[0], 128, deleted);
		LOG("write%s to drive %d c:h:s %d:0:%d\n", deleted ? " deleted" : "", m_unit,
			m_qxta, m_qxsa);
		break;
	}

	m_state = next_state;
	raise_done();
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static const floppy_interface qx_floppy_interface =
{
	FLOPPY_STANDARD_8_SSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(rx01),
	"floppy_8"
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  terak_qx_device - constructor
//-------------------------------------------------

terak_qx_device::terak_qx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TERAK_QX, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_image(*this, "floppy%u", 0U)
	, m_installed(false)
	, m_vec(0250)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void terak_qx_device::device_add_mconfig(machine_config &config)
{
	for (auto &floppy : m_image)
		LEGACY_FLOPPY(config, floppy, 0, &qx_floppy_interface);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void terak_qx_device::device_start()
{
	// save state
	save_item(NAME(m_installed));

	save_item(NAME(m_qxcs));
	save_item(NAME(m_qxdb));
	save_item(NAME(m_qxta));
	save_item(NAME(m_qxsa));

	save_item(NAME(m_count_read));
	save_item(NAME(m_count_write));
	save_item(NAME(m_unit));
	save_item(NAME(m_state));
	save_item(NAME(m_seektime));
	save_item(NAME(m_waittime));

	m_timer_wait = timer_alloc(FUNC(terak_qx_device::wait_tick), this);

	m_buf = std::make_unique<uint8_t[]>(128);
	m_seektime = 6; // ms
	m_waittime = 8; // ms FIXME estimate
	m_state = IDLE;

	m_installed = false;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void terak_qx_device::device_reset()
{
	if (!m_installed)
	{
		m_bus->program_space().install_readwrite_handler(0177000, 0177003, emu::rw_delegate(*this, FUNC(terak_qx_device::read)),
			emu::rw_delegate(*this, FUNC(terak_qx_device::write)));
		init_w();
		m_installed = true;
	}
	for (auto &elem : m_image)
	{
		elem->floppy_mon_w(0); // turn it on
		elem->floppy_drive_set_controller(this);
		elem->floppy_drive_set_rpm(360.);
		elem->floppy_drive_set_ready_state(ASSERT_LINE, 1);
	}
	m_state = IDLE;
}

void terak_qx_device::init_w()
{
	if (m_installed)
	{
		m_qxcs &= ~(QXCS_IE | QXCS_ERR);
		return;
	}

	m_qxcs = m_qxdb = 0;
	m_qxta = m_qxsa = 0;

	m_done = CLEAR_LINE;

	m_count_read = m_count_write = m_unit = 0;
}

int terak_qx_device::z80daisy_irq_state()
{
	if (m_done == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int terak_qx_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_done == ASSERT_LINE)
	{
		m_done = CLEAR_LINE;
		vec = m_vec;
	}

	return vec;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(TERAK_QX, device_qbus_card_interface, terak_qx_device, "terak_qx", "Terak fixed density disk drive")
