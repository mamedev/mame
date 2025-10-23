// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DVK KZD MFM hard disk controller HLE (decimal 3.057.316)

    Derived from DEC Professional disk controller.

    CSR 174000, vector 300, device driver DW.SYS

***************************************************************************/

#include "emu.h"
#include "dvk_dwhle.h"

#include "imagedev/harddriv.h"
#include "machine/pdp11.h"


#define LOG_DBG     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_DBG)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGDBG(format, ...)   LOGMASKED(LOG_DBG, "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)


namespace {

enum
{
	REGISTER_ID = 0,
	REGISTER_ERR = 2,
	REGISTER_SECTOR,
	REGISTER_DATA,
	REGISTER_CYL,
	REGISTER_HEAD,
	REGISTER_CSR,
	REGISTER_SI
};

static constexpr int KZDERR_DM    = 0000400;
static constexpr int KZDERR_TRK0  = 0001000;
static constexpr int KZDERR_FAULT = 0002000;
static constexpr int KZDERR_AM    = 0010000;
static constexpr int KZDERR_ACRC  = 0020000;
static constexpr int KZDERR_DCRC  = 0040000;

static constexpr int KZDERR_RD    = 0177400;
static constexpr int KZDERR_WR    = 0000377;

static constexpr int KZDSEC_RDWR  = 0177437;
static constexpr int KZDCYL_RDWR  = 0001777;
static constexpr int KZDHED_RDWR  = 0000007;

static constexpr int KZDCMD_TRK0  = 0020;
static constexpr int KZDCMD_READ  = 0040;
static constexpr int KZDCMD_WRITE = 0060;
static constexpr int KZDCMD_FORMAT= 0120;

static constexpr int KZDCSR_ERR   = 0000400;
static constexpr int KZDCSR_DRQ2  = 0004000;
static constexpr int KZDCSR_DONE  = 0010000;
static constexpr int KZDCSR_WERR  = 0020000;
static constexpr int KZDCSR_DRDY  = 0040000;

static constexpr int KZDCSR_RD    = 0177400;
static constexpr int KZDCSR_WR    = 0000377;

static constexpr int KZDSI_DONE   = 0000001;
static constexpr int KZDSI_INIT   = 0000010;
static constexpr int KZDSI_IE     = CSR_IE;
static constexpr int KZDSI_DRQ1   = CSR_DONE;
static constexpr int KZDSI_SLOW   = 0000400;
static constexpr int KZDSI_BUSY   = 0100000;

static constexpr int KZDSI_RD     = 0100711;
static constexpr int KZDSI_WR     = 0000510;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> dvk_dwhle_device

class dvk_dwhle_device : public device_t,
	public device_qbus_card_interface
{
public:
	// construction/destruction
	dvk_dwhle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	std::error_condition load_hd(device_image_interface &image);
	void unload_hd(device_image_interface &image);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

	virtual void init_w() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

	TIMER_CALLBACK_MEMBER(wait_tick);
	TIMER_CALLBACK_MEMBER(seek_tick);
	TIMER_CALLBACK_MEMBER(read_tick);
	TIMER_CALLBACK_MEMBER(write_tick);

private:
	bool m_installed;

	line_state m_drqa, m_drqb;

	void command(int command);
	int sector_to_lba(unsigned int cylinder, unsigned int head, unsigned int sector, uint32_t *lba);

	inline void raise_drqa();
	inline void clear_drqa();
	inline void raise_drqb();
	inline void clear_drqb();

	harddisk_image_device *m_image;
	const hard_disk_file::info *m_hd_geom;

	uint16_t m_regs[9];
	std::unique_ptr<uint16_t[]> m_buf;
	int m_count_read, m_count_write;

	int m_track, m_seektime, m_waittime;

	emu_timer *m_timer_seek, *m_timer_read, *m_timer_write, *m_timer_wait;
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dvk_dwhle_device - constructor
//-------------------------------------------------

dvk_dwhle_device::dvk_dwhle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DVK_DWHLE, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_installed(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dvk_dwhle_device::device_start()
{
	// save state
	save_item(NAME(m_installed));
	save_item(NAME(m_regs));
	save_item(NAME(m_count_read));
	save_item(NAME(m_count_write));
	save_item(NAME(m_track));
	save_item(NAME(m_seektime));
	save_item(NAME(m_waittime));

	m_timer_wait = timer_alloc(FUNC(dvk_dwhle_device::wait_tick), this);
	m_timer_seek = timer_alloc(FUNC(dvk_dwhle_device::seek_tick), this);
	m_timer_read = timer_alloc(FUNC(dvk_dwhle_device::read_tick), this);
	m_timer_write = timer_alloc(FUNC(dvk_dwhle_device::write_tick), this);

	m_buf = std::make_unique<uint16_t[]>(256);
	m_seektime = 8; // ms. ST251-1
	m_waittime = 100; // us.

	m_installed = false;
	m_image = nullptr;
	m_hd_geom = nullptr;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dvk_dwhle_device::device_reset()
{
	if (!m_installed)
	{
		m_bus->install_device(0174000, 0174037, read16sm_delegate(*this, FUNC(dvk_dwhle_device::read)),
			write16sm_delegate(*this, FUNC(dvk_dwhle_device::write)));
		m_installed = true;
	}
}

void dvk_dwhle_device::init_w()
{
	memset(m_regs, 0, sizeof(m_regs));
	m_regs[REGISTER_ERR] = 128/4;
	m_regs[REGISTER_CSR] = KZDCSR_DONE|KZDCSR_DRDY;
	m_regs[REGISTER_SI] = KZDSI_DONE|KZDSI_SLOW;

	m_drqa = m_drqb = CLEAR_LINE;
	m_bus->birq4_w(CLEAR_LINE);

	m_timer_seek->adjust(attotime::never, 0, attotime::never);
	m_timer_read->adjust(attotime::never, 0, attotime::never);
	m_timer_write->adjust(attotime::never, 0, attotime::never);
	m_timer_wait->adjust(attotime::never, 0, attotime::never);

	m_count_read = m_count_write = m_track = 0;
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dvk_dwhle_device::device_add_mconfig(machine_config &config)
{
	harddisk_image_device &harddisk(HARDDISK(config, "harddisk1"));
	harddisk.set_device_load(FUNC(dvk_dwhle_device::load_hd));
	harddisk.set_device_unload(FUNC(dvk_dwhle_device::unload_hd));
}


int dvk_dwhle_device::z80daisy_irq_state()
{
	if ((m_drqa | m_drqb) == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int dvk_dwhle_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_drqa == ASSERT_LINE)
	{
		m_drqa = CLEAR_LINE;
		vec = 0300;
	}
	else if (m_drqb == ASSERT_LINE)
	{
		m_drqb = CLEAR_LINE;
		vec = 0300;
	}

	return vec;
}


std::error_condition dvk_dwhle_device::load_hd(device_image_interface &image)
{
	m_image = downcast<harddisk_image_device *>(&image);
	if (!m_image->exists())
		return image_error::UNSPECIFIED;

	m_hd_geom = &m_image->get_info();
	if (m_hd_geom->cylinders > 1024 ||
		m_hd_geom->heads > 4 ||
		m_hd_geom->sectors > 16 ||
		m_hd_geom->sectorbytes != 512)
	{
		return image_error::INVALIDIMAGE;
	}

	return std::error_condition();
}

void dvk_dwhle_device::unload_hd(device_image_interface &image)
{
}


inline void dvk_dwhle_device::raise_drqa()
{
	m_regs[REGISTER_SI] |= KZDSI_DONE;
	raise_virq(m_bus->birq4_w, m_regs[REGISTER_SI], CSR_IE, m_drqa);
}

inline void dvk_dwhle_device::clear_drqa()
{
	m_regs[REGISTER_SI] &= ~KZDSI_DONE;
	clear_virq(m_bus->birq4_w, m_regs[REGISTER_SI], CSR_IE, m_drqa);
}

inline void dvk_dwhle_device::raise_drqb()
{
	m_regs[REGISTER_SI] |= KZDSI_DRQ1;
	raise_virq(m_bus->birq4_w, m_regs[REGISTER_SI], CSR_IE, m_drqb);
}

inline void dvk_dwhle_device::clear_drqb()
{
	m_regs[REGISTER_SI] &= ~KZDSI_DRQ1;
	clear_virq(m_bus->birq4_w, m_regs[REGISTER_SI], CSR_IE, m_drqb);
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t dvk_dwhle_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case REGISTER_ID:
		data = 0401;
		clear_drqb();
		break;

	case REGISTER_ERR:
		data = m_regs[offset] & KZDERR_RD;
		break;

	case REGISTER_SECTOR:
		data = m_regs[offset] & KZDSEC_RDWR;
		clear_drqa();
		break;

	case REGISTER_DATA:
		if (m_count_read < 256)
		{
			data = m_buf[m_count_read++];
			clear_drqb();
		}
		if (m_count_read == 256)
		{
			m_count_read = 0;
			m_regs[REGISTER_CSR] &= ~KZDCSR_DRQ2;
			m_timer_wait->adjust(attotime::never, 0, attotime::never);
			raise_drqa();
		}
		else
			m_timer_wait->adjust(attotime::from_usec(m_waittime));
		break;

	case REGISTER_CYL:
		data = m_regs[offset] & KZDCYL_RDWR;
		break;

	case REGISTER_HEAD:
		data = m_regs[offset] & KZDHED_RDWR;
		break;

	case REGISTER_CSR:
		data = m_regs[offset] & KZDCSR_RD;
		// fails interrupt test
//		clear_drqa();
		break;

	case REGISTER_SI:
		data = m_regs[offset] & KZDSI_RD;
		break;
	}

	LOGDBG("R %06o == %06o @ %d\n", 0174000 + (offset << 1), data, m_count_read);

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void dvk_dwhle_device::write(offs_t offset, uint16_t data)
{
	LOGDBG("W %06o <- %06o\n", 0174000 + (offset << 1), data);

	switch (offset)
	{
	case REGISTER_ID:
		clear_drqb();
		break;

	case REGISTER_ERR:
		UPDATE_16BIT(&m_regs[offset], data, KZDERR_WR);
		break;

	case REGISTER_SECTOR:
		UPDATE_16BIT(&m_regs[offset], data, KZDSEC_RDWR);
		m_regs[REGISTER_SI] &= ~KZDSI_DONE;
		clear_drqa();
		break;

	case REGISTER_CYL:
		UPDATE_16BIT(&m_regs[offset], data, KZDCYL_RDWR);
		break;

	case REGISTER_HEAD:
		UPDATE_16BIT(&m_regs[offset], data, KZDHED_RDWR);
		break;

	case REGISTER_DATA:
		if (m_count_write < 256)
		{
			m_buf[m_count_write++] = data;
			clear_drqb();
		}
		if (m_count_write == 256)
		{
			m_regs[REGISTER_SI] |= KZDSI_BUSY;
			m_timer_wait->adjust(attotime::never, 0, attotime::never);
			m_timer_write->adjust(attotime::from_msec(m_seektime));
		}
		else
			m_timer_wait->adjust(attotime::from_usec(m_waittime));
		break;

	case REGISTER_CSR:
		UPDATE_16BIT(&m_regs[offset], data, KZDCSR_WR);
		clear_drqa();
		command(data & KZDCSR_WR);
		break;

	case REGISTER_SI:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_bus->birq4_w, 1, 1, m_drqa);
			clear_virq(m_bus->birq4_w, 1, 1, m_drqb);
		}
		else if ((m_regs[offset] & (KZDSI_DONE + CSR_IE)) == KZDSI_DONE)
		{
			raise_virq(m_bus->birq4_w, 1, 1, m_drqa);
		}
		else if ((m_regs[offset] & (KZDSI_DRQ1 + CSR_IE)) == KZDSI_DRQ1)
		{
			raise_virq(m_bus->birq4_w, 1, 1, m_drqb);
		}
		UPDATE_16BIT(&m_regs[offset], data, KZDSI_WR);
		if (data & KZDSI_INIT) init_w();
		break;
	}
}

void dvk_dwhle_device::command(int command)
{
	m_regs[REGISTER_ERR] &= ~KZDERR_WR;
	m_regs[REGISTER_CSR] &= ~KZDCSR_ERR; // assuming

	switch (command)
	{
	case KZDCMD_TRK0:
		m_regs[REGISTER_SI] |= KZDSI_BUSY;
		m_timer_seek->adjust(attotime::from_msec(m_seektime));
		break;

	case KZDCMD_FORMAT:
		m_regs[REGISTER_CSR] |= KZDCSR_DRQ2;
		m_count_write = 0;
		raise_drqb();
		break;

	case KZDCMD_READ:
		m_regs[REGISTER_SI] |= KZDSI_BUSY;
		m_count_read = 0;
		m_timer_read->adjust(attotime::from_msec(m_seektime));
		break;

	case KZDCMD_WRITE:
		m_regs[REGISTER_CSR] |= KZDCSR_DRQ2;
		m_count_write = 0;
		raise_drqb();
		break;

	default:
		LOG("invalid command %03o\n", command);
		m_regs[REGISTER_ERR] |= KZDERR_FAULT;
		m_regs[REGISTER_CSR] |= KZDCSR_ERR;
		break;
	}
}

int dvk_dwhle_device::sector_to_lba(unsigned int cylinder, unsigned int head, unsigned int sector, uint32_t *lba)
{
	if (!m_hd_geom || cylinder >= m_hd_geom->cylinders || head >= m_hd_geom->heads || sector >= m_hd_geom->sectors)
		return 1;

	*lba = ((cylinder * m_hd_geom->heads + head) * m_hd_geom->sectors) + sector;

	return 0;
}

TIMER_CALLBACK_MEMBER(dvk_dwhle_device::wait_tick)
{
	raise_drqb();
}

TIMER_CALLBACK_MEMBER(dvk_dwhle_device::seek_tick)
{
	m_track = 0;
	m_regs[REGISTER_SI] &= ~KZDSI_BUSY;
	raise_drqa();
}

TIMER_CALLBACK_MEMBER(dvk_dwhle_device::read_tick)
{
	uint32_t lba;

	if (!sector_to_lba(m_regs[REGISTER_CYL], m_regs[REGISTER_HEAD], m_regs[REGISTER_SECTOR], &lba))
	{
		m_track = m_regs[REGISTER_CYL];
		if (m_image->exists() && m_image->read(lba, (void *)&m_buf[0]))
		{
			m_regs[REGISTER_SI] &= ~KZDSI_BUSY;
			m_regs[REGISTER_CSR] |= KZDCSR_DRQ2;
			raise_drqb();
		}
		else
		{
			LOG("FAILED(1) read c:h:s %d:%d:%d lba %d\n", m_regs[REGISTER_CYL],
				m_regs[REGISTER_HEAD], m_regs[REGISTER_SECTOR], lba);
		}
	}
	else
	{
		LOG("FAILED(2) read c:h:s %d:%d:%d lba %d\n", m_regs[REGISTER_CYL],
			m_regs[REGISTER_HEAD], m_regs[REGISTER_SECTOR], lba);
		m_regs[REGISTER_SI] &= ~KZDSI_BUSY;
		m_regs[REGISTER_ERR] |= KZDERR_AM;
		m_regs[REGISTER_CSR] |= KZDCSR_ERR;
	}
}

TIMER_CALLBACK_MEMBER(dvk_dwhle_device::write_tick)
{
	uint32_t lba;
	m_count_write = 0;
	if (!sector_to_lba(m_regs[REGISTER_CYL], m_regs[REGISTER_HEAD], m_regs[REGISTER_SECTOR], &lba) &&
		m_image->exists() && m_image->write(lba, (void *)&m_buf[0]))
	{
		m_track = m_regs[REGISTER_CYL];
		m_regs[REGISTER_SI] &= ~KZDSI_BUSY;
		m_regs[REGISTER_CSR] &= ~KZDCSR_DRQ2;
		raise_drqa();
	}
	else
	{
		LOG("FAILED write c:h:s %d:%d:%d lba %d\n", m_regs[REGISTER_CYL],
			m_regs[REGISTER_HEAD], m_regs[REGISTER_SECTOR], lba);
		m_regs[REGISTER_SI] &= ~KZDSI_BUSY;
		m_regs[REGISTER_ERR] |= KZDERR_AM;
		m_regs[REGISTER_CSR] |= KZDCSR_ERR;
//		raise_drqb();
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(DVK_DWHLE, device_qbus_card_interface, dvk_dwhle_device, "dvk_dwhle", "DVK MFM disk controller")
