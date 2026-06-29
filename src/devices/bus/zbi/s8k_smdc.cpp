// license:BSD-3-Clause
// copyright-holders:A. Lenard
/**********************************************************************

	Storage Module Device Controller

Based on prior work by Christian Corti

NOTE: The actual board uses an AM2910 MPC and 4x AM2901B bit-slice ALUs
(+2 KB SRAM). However, the firmware ROMs (U131-U136) for these have not
been dumped yet, so currently only high-level emulation is possible.

**********************************************************************/

#include "emu.h"
#include "s8k_smdc.h"

#include "imagedev/harddriv.h"

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_CMD     (1U << 1)
#define LOG_INT     (1U << 2)
#define LOG_SEEK    (1U << 3)
#define LOG_DATA    (1U << 4)
#define LOG_READ    (1U << 5)
#define LOG_WRITE   (1U << 6)

//#include <iostream>
//#define LOG_OUTPUT_STREAM std::cout
//#define VERBOSE  (LOG_CMD | LOG_INT | LOG_SEEK | LOG_DATA | LOG_READ | LOG_WRITE)

#include "logmacro.h"

#define LOGCMD(...)   LOGMASKED(LOG_CMD,   __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)
#define LOGSEEK(...)  LOGMASKED(LOG_SEEK,  __VA_ARGS__)
#define LOGDATA(...)  LOGMASKED(LOG_DATA,  __VA_ARGS__)
#define LOGREAD(...)  LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGWRITE(...) LOGMASKED(LOG_WRITE, __VA_ARGS__)

//**************************************************************************
//  CONSTANTS
//**************************************************************************

/* Command Register */
enum : uint16_t
{
	SMD_CR_CMD_MASK	= 0x0007,
	SMD_CR_WK		= 0x0008,	/* wakeup */
	SMD_CR_EI		= 0x0010,	/* enable interrupts (reset by IUS) */
	SMD_CR_DI		= 0x0020,	/* disable interrupts */
	SMD_CR_RI		= 0x0040,	/* reset IP and IUS */
	SMD_CR_INIT		= 0x0080,	/* initialze controller */
	SMD_CR_DTA		= 0x0100,	/* dispatch table address or interrupt vector */
	SMD_CR_DTA_MASK	= 0xff00
};

/* Command Register Command */
enum
{
	SMD_CR_CMD_NOP	= 0,
	SMD_CR_CMD_RPA	= 1,	/* read packet address from DT */
	SMD_CR_CMD_DTA0	= 4,	/* dispatch table address byte 0 (lsb) */
	SMD_CR_CMD_DTA1	= 5,	/* dispatch table address byte 1 */
	SMD_CR_CMD_DTA2	= 6,	/* dispatch table address byte 2 (msb)  */
	SMD_CR_CMD_IV	= 7		/* interrupt vector */
};

/* Status Register */
enum : uint16_t
{
	SMD_SR_BZ		= 0x0001,	/* controller busy from CR_CMD */
	SMD_SR_IUS		= 0x0002,	/* interrupt under service */
	SMD_SR_IP		= 0x0004,	/* interrupt pending */
	SMD_SR_NDT		= 0x0080,	/* no dispatch table/interrupt vector */
	SMD_SR_ES		= 0x0100,	/* packet ST or selftest code */
	SMD_SR_DRV		= 0x4000	/* drive number 0-3 */
};

/* Dipatch Table Packet Status */
enum : uint16_t
{
	/* set by host */
	SMD_DT_PS_IDLE	= 0,
	SMD_DT_PS_GO	= 1,
	/* set by SMDC */
	SMD_DT_PS_BUSY	= 2,
	SMD_DT_PS_DONE	= 4
};

/* Current Command */
enum : uint16_t
{
	SMD_CM_CMD_MASK	= 0x00ff,
	SMD_CM_NO		= 0x0100,	/* no offsets during retries */
	SMD_CM_NE		= 0x0200,	/* no error correction */
	SMD_CM_NR		= 0x0400	/* no retries, use pkt.OF */
};

/* Curent Command Code */
enum
{
	SMD_CM_CMD_NOP	= 0,
	SMD_CM_CMD_WRAM,		/* write RAM */
	SMD_CM_CMD_RRAM,		/* read RAM */
	SMD_CM_CMD_SELECT,		/* select drive */
	SMD_CM_CMD_PRISEL,		/* priority select */
	SMD_CM_CMD_RELEASE,		/* release */
	SMD_CM_CMD_RESET,		/* reset fault */
	SMD_CM_CMD_SEEK,		/* position (seek/rezero) */
	SMD_CM_CMD_WFMT,		/* write format */
	SMD_CM_CMD_WLONG,		/* write long */
	SMD_CM_CMD_WRITE,		/* write */
	/* 11 = reserved */
	SMD_CM_CMD_RFMT	= 12,	/* read format */
	SMD_CM_CMD_RLONG,		/* read long */
	SMD_CM_CMD_READ,		/* read */
	SMD_CM_CMD_SIZE			/* size disk */
};

/* Ending Status */
enum : uint8_t
{
	SMD_ES_NOERR 	= 0,	/* no error */
	SMD_ES_ERRINIT,			/* initialization error (no DT, IV or bad CR) */
	SMD_ES_ERRSECTOR,		/* sector overrun error */
	SMD_ES_ERRDMA,			/* DMA memory error returned in SR_ES */
	SMD_ES_ERRSEL,			/* select error */
	SMD_ES_BADCT,			/* CT invalid */
	SMD_ES_BUSY,			/* SMD dual access busy */
	SMD_ES_ERRSEEK,			/* multiple rezero error */
	SMD_ES_ERRST,			/* SMD status error (see DS) */
	SMD_ES_BADDMA,			/* odd DMA address */
	SMD_ES_OVERFLOW,		/* pack overflow error */
	SMD_ES_PWRFAIL,			/* power failure during read/write */
	SMD_ES_BADCMD,			/* undefined CM_CMD */
	SMD_ES_ERRDATA,			/* unrecovered data error */
	SMD_ES_NOSECTOR,		/* sector not found */
	SMD_ES_VIOL,			/* write protect violation */
	/* timeout errors */
	SMD_ES_WAITIDLE = 32,	/* idle loop */
	SMD_ES_WAITINT,			/* waiting for IP and IUS to clear */
	SMD_ES_WAITDMA,			/* waiting for DMA to complete */
	SMD_ES_WAITCYL,			/* waiting for SMD on-cylinder */
	SMD_ES_WAITSRV,			/* waiting for SMD servo clock */
	SMD_ES_WAITDCLK,		/* waiting for SMD data clock */
	SMD_ES_WAITSECTOR,		/* waiting for sector/index mark */
	SMD_ES_WAITID,			/* waiting for ID sync */
	SMD_ES_WAITDATA			/* waiting for data sync */
};

/* Drive Select */
enum : uint16_t
{
	SMD_DS_RY		= 0x0001,	/* selected drive ready */
	SMD_DS_OC		= 0x0002,	/* selected drive on cylinder */
	SMD_DS_SE		= 0x0004,	/* selected drive seek error */
	SMD_DS_FT		= 0x0008,	/* selected drive fault */
	SMD_DS_RO		= 0x0010,	/* selected drive read only */
	SMD_DS_BZ		= 0x0020,	/* selected drive busy (dual access) */
	SMD_DS_XM		= 0x0040,	/* index mark */
	SMD_DS_SM		= 0x0080,	/* sector mark */
	SMD_DS_SEL		= 0x0100,	/* ports 0-3 selected */
	SMD_DS_SKE		= 0x1000	/* ports 0-3 seek end */
};

constexpr uint16_t SMD_FW_VERSION = 0x1234;

namespace {

class zbi_s8k_smdc_card_device : public device_t, public device_zbi_card_interface
{
public:
	zbi_s8k_smdc_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read();
	void write(uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_buffers);

	// zbi_card_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;
	virtual int busdaisy_req_state() override;
	virtual void busdaisy_req_ack() override;

private:
	struct smd_dispatch_table
	{
		uint16_t PS[4];		/* packet status */
		struct
		{
			uint16_t PH;	/* packet address msh */
			uint16_t PL;	/* packet address lsh */
		} PA[4];
	};

	struct smd_packet
	{
		uint16_t CM, ST, SB, DS;
		uint16_t CT, AH, AL, UN;
		uint16_t CY, HD, VS, SC;
		uint16_t OF;
		uint16_t rsv1, rsv2, rsv3;
	};

	void smd_init();
	void smd_do_drive(int drv);
	int get_lbasector();

	optional_device_array<harddisk_image_device, 4> m_drives;
	int m_cyl[4];

	emu_timer *m_update_timer;

	/* data buffers */
	uint8_t m_buffer[512];
	uint16_t m_dt[12];	/* dispatch table */
	uint16_t m_pkt[16];	/* packet */

	/* dispatch table address */
	uint16_t m_dta_seg;	/* address high word */
	uint16_t m_dta_ofs; /* address low word */
	offs_t m_dta_idx;

	uint16_t m_status;	/* status register */
	uint8_t m_drv;		/* drive number 0-3 */
	uint8_t m_es;		/* packet command ending status */
	uint8_t m_iv;		/* interrupt vector */
	bool m_ie;			/* interrupt enable */
	bool m_wakeup;

	int m_init_left;
	int m_busreq_state;
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zbi_s8k_smdc_card_device - constructor
//-------------------------------------------------

zbi_s8k_smdc_card_device::zbi_s8k_smdc_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZBI_S8K_SMDC, tag, owner, clock)
	, device_zbi_card_interface(mconfig, *this)
	, m_drives(*this, "smd%u", 0U)
	, m_dta_seg(0)
	, m_dta_ofs(0)
	, m_dta_idx(0)
	, m_status(SMD_SR_NDT)
	, m_drv(0)
	, m_es(0)
	, m_iv(0)
	, m_ie(false)
	, m_wakeup(false)
	, m_init_left(5)
	, m_busreq_state(0)
{
}

//-------------------------------------------------
//  smd_init - initialize internal state
//-------------------------------------------------

void zbi_s8k_smdc_card_device::smd_init()
{
	memset(&m_buffer[0], 0, 512);
	memset(&m_dt[0], 0, 24);
	memset(&m_pkt[0], 0, 32);

	m_cyl[0] = m_cyl[1] = m_cyl[2] = m_cyl[3] = 0;

	m_ie = false;
	m_iv = 0;
	m_es = 0;
	m_drv = 0;

	m_dta_seg = 0;
	m_dta_ofs = 0;
	m_dta_idx = 0;

	m_status = SMD_SR_NDT;
	m_init_left = 5;

	m_wakeup = false;
	m_busreq_state = 0;
}

//-------------------------------------------------
//  read - read device status
//-------------------------------------------------

uint16_t zbi_s8k_smdc_card_device::read()
{
	uint16_t s;

	if (m_init_left <= 0)
	{
		m_init_left = 0;
		m_status &= ~SMD_SR_NDT;
	}
	else if (m_init_left > 0)
		m_status |= SMD_SR_NDT;

	s = (m_drv << 14) | (m_es << 8) | m_status;

	return s;
}

//-------------------------------------------------
//  write - write device command
//-------------------------------------------------

void zbi_s8k_smdc_card_device::write(uint16_t data)
{
	if (data & SMD_CR_INIT)
	{
		smd_init();
	}

	if (data & SMD_CR_RI)
	{
		m_status &= ~(SMD_SR_IP | SMD_SR_IUS);
		m_ie = false;
	}

	if (data & SMD_CR_WK)
	{
		m_wakeup = true;
		m_status |= SMD_SR_BZ;
		m_update_timer->adjust(attotime::from_nsec(180));	/* bus frequency */
	}

	if (data & SMD_CR_EI)
		m_ie = true;
	else if (data & SMD_CR_DI)
		m_ie = false;

	switch (data & SMD_CR_CMD_MASK)
	{
	case SMD_CR_CMD_NOP:
		break;
	case SMD_CR_CMD_RPA:
		m_dta_idx = (m_dta_seg << 16 | m_dta_ofs);
		m_init_left--;
		break;
	case SMD_CR_CMD_DTA0:
		m_dta_ofs = (m_dta_ofs & 0xff00) | (data >> 8);
		m_init_left--;
		break;
	case SMD_CR_CMD_DTA1:
		m_dta_ofs = (m_dta_ofs & 0x00ff) | (data & 0xff00);
		m_init_left--;
		break;
	case SMD_CR_CMD_DTA2:
		m_dta_seg = (data >> 8) & 0x3f;
		m_init_left--;
		break;
	case SMD_CR_CMD_IV:
		m_iv = data >> 8;
		m_init_left--;
		break;

	default:
		LOG("%s ERROR invalid SMDC command: %02x\n", machine().describe_context(), data & SMD_CR_CMD_MASK);
	}

	LOGCMD("%s SMDC command: %02x\n", machine().describe_context(), data & SMD_CR_CMD_MASK);
}

//-------------------------------------------------
//  update_buffers -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(zbi_s8k_smdc_card_device::update_buffers)
{
	if (m_wakeup)
	{
		/* tell the CPU to relinquish the bus */
		m_busreq_state = ASSERT_LINE;
		m_bus->busreq_w(m_busreq_state);

		/* reset wakeup bit */
		m_wakeup = false;
	}
	else
	{
		m_status &= ~SMD_SR_BZ;
	}
}

//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int zbi_s8k_smdc_card_device::z80daisy_irq_state()
{
	int state = 0;
	bool ius = m_status & SMD_SR_IUS;
	bool ip = m_status & SMD_SR_IP;

	if (ius)
		state |= Z80_DAISY_IEO;
	else
		state = m_ie * (ip * Z80_DAISY_INT);

	return state;
}

//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int zbi_s8k_smdc_card_device::z80daisy_irq_ack()
{
	m_status |= SMD_SR_IUS;
	m_status &= ~SMD_SR_IP;
	m_ie = false;

	LOGINT("%s SMD interrupt: iv=%02x, es=%02x, drv=%d\n", machine().describe_context(), m_iv, m_es, m_drv);

	return (uint16_t)(m_iv | ((m_es & 0x3f) << 8) | (m_drv << 14));
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void zbi_s8k_smdc_card_device::z80daisy_irq_reti()
{
	if (m_status & SMD_SR_IUS)
	{
		/* clear the IEO state and update the IRQs */
		m_status &= ~SMD_SR_IUS;

		int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
		m_bus->vi_w(state);
	}
}

//-------------------------------------------------
//  busdaisy_req_state - return the bus request state
//-------------------------------------------------

int zbi_s8k_smdc_card_device::busdaisy_req_state()
{
	return m_busreq_state;
}

//-------------------------------------------------
//  busdaisy_req_ack - do processing as bus master
//-------------------------------------------------

void zbi_s8k_smdc_card_device::busdaisy_req_ack()
{
	int i, j;
	offs_t pkt_idx;

	smd_dispatch_table *dt;

	/* read dispatch table */
	for (i = 0; i < 12; i++)
	{
		m_dt[i] = m_bus->ram16_r(m_dta_idx + 2*i);
	}

	dt = reinterpret_cast<smd_dispatch_table*>(m_dt);

	/* process dispatch table */
	for (i = 0; i < 4; i++)
	{
		if (dt->PS[i] == SMD_DT_PS_GO)
		{
			pkt_idx = (dt->PA[i].PH << 16) | dt->PA[i].PL;

			/* read the packet */
			for (j = 0; j < 16; j++)
			{
				m_pkt[j] = m_bus->ram16_r(pkt_idx + 2*j);
			}

			/* set BUSY state */
			dt->PS[i] = SMD_DT_PS_BUSY;
			m_bus->ram16_w(m_dta_idx+i*2, dt->PS[i]);

			smd_do_drive(i);

			/* set DONE state */
			dt->PS[i] = SMD_DT_PS_DONE;
			m_bus->ram16_w(m_dta_idx+i*2, dt->PS[i]);

			m_status |= SMD_SR_IP;
			if (m_ie)
			{
				m_bus->vi_w(ASSERT_LINE);
			}

			/* write the packet back */
			for (j = 0; j < 16; j++)
			{
				m_bus->ram16_w(pkt_idx + 2*j, m_pkt[j]);
			}
		}
	}

	/* return the bus to the CPU */
	m_busreq_state = CLEAR_LINE;
	m_bus->busreq_w(m_busreq_state);

	m_update_timer->adjust(attotime::from_nsec(180));	/* bus frequency */
}

//-------------------------------------------------
//  get_lbasector - translate to lba
//-------------------------------------------------

int zbi_s8k_smdc_card_device::get_lbasector()
{
	harddisk_image_device *file = m_drives[m_drv];
	const hard_disk_file::info &info = file->get_info();
	smd_packet *pkt = reinterpret_cast<smd_packet*>(m_pkt);
	int lbasector;

	if (pkt->CY > info.cylinders)
	{
		LOGDATA("%s: Unexpected cylinder %d for range 0 to %d\n", machine().describe_context(), pkt->CY, info.cylinders - 1);
	}

	if (pkt->HD >= info.heads)
	{
		LOGDATA("%s: Unexpected head %d for range 0 to %d\n", machine().describe_context(), pkt->HD, info.heads - 1);
	}

	if (pkt->SC >= info.sectors)
	{
		LOGDATA("%s: Unexpected sector number %d for range 0 to %d\n", machine().describe_context(), pkt->SC, info.sectors - 1);
	}

	if ((pkt->CT == 0) || (pkt->CT > info.sectorbytes))
	{
		LOGDATA("%s: Unexpected sector bytes %d for range 1 to %d\n", machine().describe_context(), pkt->CT, info.sectorbytes);
		pkt->CT = 512;
	}

	lbasector = pkt->CY;
	lbasector *= info.heads;
	lbasector += pkt->HD;
	lbasector *= info.sectors;
	lbasector += pkt->SC;

	return lbasector;
}

//-------------------------------------------------
//  smd_do_drive - peform drive operations
//-------------------------------------------------

void zbi_s8k_smdc_card_device::smd_do_drive(int drv)
{
	offs_t dma_idx;
	int unit, block;
	smd_dispatch_table *dt = reinterpret_cast<smd_dispatch_table*>(m_dt);
	smd_packet *pkt = reinterpret_cast<smd_packet*>(m_pkt);
	harddisk_image_device *file;
	bool drv_good;

	unit = pkt->UN & 3;

	LOGCMD("%s smd_do_drive: drv=%d unit=%d cmd=%04x pkt_idx=%06x\n",
		   machine().describe_context(), drv, unit, pkt->CM, ((dt->PA[drv].PH << 16) | dt->PA[drv].PL));

	m_drv = unit;
	file = m_drives[unit];
	drv_good = (file && file->exists());

	m_es = SMD_ES_NOERR;

	switch (pkt->CM & SMD_CM_CMD_MASK)
	{
	case SMD_CM_CMD_NOP:		/* NOP */
		pkt->CT = SMD_FW_VERSION;
		break;

	case SMD_CM_CMD_SELECT:		/* select drive */
		LOGCMD("%s SMDC select unit %d\n", machine().describe_context(), unit);
		if (!drv_good)
		{
			pkt->DS |= SMD_DS_FT;
			m_es = SMD_ES_ERRSEL;
		}
		break;

	case SMD_CM_CMD_RESET:		/* reset fault */
		pkt->DS &= ~SMD_DS_FT;
		break;

	case SMD_CM_CMD_SEEK:		/* seek */
		if (pkt->CY == 0xffff)
		{
			/* rezero drive and reset fault */
			pkt->CY = 0;
			pkt->HD = 0;
			pkt->SC = 0;
			pkt->DS &= ~SMD_DS_FT;

			m_cyl[unit] = 0;
		}
		else
		{
			m_cyl[unit] = pkt->CY;
		}
		break;

	case SMD_CM_CMD_WRITE:		/* write */
		LOGWRITE("%s SMD write: unit=%d cyl=%d hd=%d sec=%d count=%d addr=%04x:%04x\n", machine().describe_context(),
				 unit, pkt->CY, pkt->HD, pkt->SC, pkt->CT, pkt->AH, pkt->AL);

		if (pkt->CT & 1)
		{
			m_es = SMD_ES_BADCT;
			pkt->DS |= SMD_DS_FT;
			LOGDATA("Invalid write byte count: %d!\n", pkt->CT);
			break;
		}

		if (!drv_good)
		{
			m_es = SMD_ES_NOSECTOR;
			pkt->DS |= SMD_DS_FT;
			break;
		}

		dma_idx = (pkt->AH << 16) | pkt->AL;

		if (dma_idx & 1)
		{
			m_es = SMD_ES_BADDMA;
			pkt->DS |= SMD_DS_FT;
			break;
		}

		block = get_lbasector();

		LOGSEEK(" --> seek to block $%d (offset %d)\n", block, block*512);

		/* DMA from main memory */
		for (unsigned int n = 0; n < pkt->CT; n++)
		{
			m_buffer[n] = m_bus->ram8_r(dma_idx++);
		}

		if (!file->write(block, m_buffer))
		{
			m_es = SMD_ES_NOSECTOR;
			pkt->DS |= SMD_DS_FT;
		}
		break;

	case SMD_CM_CMD_READ:		/* read */
		LOGREAD("%s SMD read: unit=%d cyl=%d hd=%d sec=%d count=%d addr=%04x:%04x\n", machine().describe_context(),
				unit, pkt->CY, pkt->HD, pkt->SC, pkt->CT, pkt->AH, pkt->AL);

		if (pkt->CT & 1)
		{
			m_es = SMD_ES_BADCT;
			pkt->DS |= SMD_DS_FT;
			LOGDATA("Invalid read byte count: %d!\n", pkt->CT);
			break;
		}

		if (!drv_good)
		{
			m_es = SMD_ES_NOSECTOR;
			pkt->DS |= SMD_DS_FT;
			break;
		}

		dma_idx = (pkt->AH << 16) | pkt->AL;

		if (dma_idx & 1)
		{
			m_es = SMD_ES_BADDMA;
			pkt->DS |= SMD_DS_FT;
			break;
		}

		block = get_lbasector();

		LOGSEEK(" --> seek to block $%d (offset %d)\n", block, block*512);

		if (!file->read(block, m_buffer))
		{
			m_es = SMD_ES_NOSECTOR;
			pkt->DS |= SMD_DS_FT;
		}
		else
		{
			/* DMA to main memory */
			for (unsigned int n = 0; n < pkt->CT; n++)
			{
				m_bus->ram8_w(dma_idx++, m_buffer[n]);
			}
		}
		break;

	case SMD_CM_CMD_SIZE:	/* size disk */
		if (drv_good)
		{
			pkt->CY = file->get_info().cylinders;
			pkt->HD = file->get_info().heads;
			pkt->SC = file->get_info().sectors;
		}
		else
		{
			pkt->CY = 0;
			pkt->HD = 0;
			pkt->SC = 0;
		}
		break;

	default:
		pkt->DS = SMD_DS_FT;
	}

	if (drv_good)
		pkt->DS |= SMD_DS_OC;

	pkt->DS &= 0x003f;
	pkt->DS |= SMD_DS_RY | ((1<<drv) << 8) | ((1<<drv) << 12);
}

//**************************************************************************
//  DEVICE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void zbi_s8k_smdc_card_device::device_resolve_objects()
{
	m_bus->add_to_daisy_chain(tag());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zbi_s8k_smdc_card_device::device_start()
{
	m_update_timer = timer_alloc(FUNC(zbi_s8k_smdc_card_device::update_buffers), this);

	save_pointer(NAME(m_dt), 12);
	save_pointer(NAME(m_pkt), 16);
	save_pointer(NAME(m_buffer), 512);
	save_item(NAME(m_status));
	save_item(NAME(m_drv));
	save_item(NAME(m_es));
	save_item(NAME(m_iv));
	save_item(NAME(m_ie));
	save_item(NAME(m_wakeup));
	save_item(NAME(m_init_left));
	save_item(NAME(m_dta_seg));
	save_item(NAME(m_dta_ofs));
	save_item(NAME(m_dta_idx));
	save_item(NAME(m_busreq_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void zbi_s8k_smdc_card_device::device_reset()
{
	smd_init();

	m_bus->iospace()->install_readwrite_handler(0x7f00, 0x7f01,
		read16smo_delegate(*this, FUNC(zbi_s8k_smdc_card_device::read)), write16smo_delegate(*this, FUNC(zbi_s8k_smdc_card_device::write)));
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void zbi_s8k_smdc_card_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, m_drives[0], 0);
	HARDDISK(config, m_drives[1], 0);
	HARDDISK(config, m_drives[2], 0);
	HARDDISK(config, m_drives[3], 0);
}

} // anonymous namespace

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ZBI_S8K_SMDC, device_zbi_card_interface, zbi_s8k_smdc_card_device, "s8k_smdc", "Storage Module Device Controller")
