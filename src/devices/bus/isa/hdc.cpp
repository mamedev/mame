// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    ISA 8 bit XT Hard Disk Controller

**********************************************************************/

#include "emu.h"
#include "hdc.h"

#define LOG_HDC_STATUS      (1U << 1)
#define LOG_HDC_CALL        (1U << 2)
#define LOG_HDC_DATA        (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"

#define CMD_TESTREADY   0x00
#define CMD_RECALIBRATE 0x01
#define CMD_SENSE       0x03
#define CMD_FORMATDRV   0x04
#define CMD_VERIFY      0x05
#define CMD_FORMATTRK   0x06
#define CMD_FORMATBAD   0x07
#define CMD_READ        0x08
#define CMD_WRITE       0x0a
#define CMD_SEEK        0x0b

#define CMD_SETPARAM    0x0c
#define CMD_GETECC      0x0d

#define CMD_READSBUFF   0x0e
#define CMD_WRITESBUFF  0x0f

#define CMD_RAMDIAG     0xe0
#define CMD_DRIVEDIAG   0xe3
#define CMD_INTERNDIAG  0xe4
#define CMD_READLONG    0xe5
#define CMD_WRITELONG   0xe6

/* Bits for command status byte */
#define CSB_ERROR       0x02
#define CSB_LUN         0x20

/* XT hard disk controller status bits */
#define STA_READY       0x01
#define STA_INPUT       0x02
#define STA_COMMAND     0x04
#define STA_SELECT      0x08
#define STA_REQUEST     0x10
#define STA_INTERRUPT   0x20

/* XT hard disk controller control bits */
#define CTL_PIO         0x00
#define CTL_DMA         0x01

const char *const s_hdc_command_names[] =
{
	"CMD_TESTREADY",        /* 0x00 */
	"CMD_RECALIBRATE",      /* 0x01 */
	nullptr,                   /* 0x02 */
	"CMD_SENSE",            /* 0x03 */
	"CMD_FORMATDRV",        /* 0x04 */
	"CMD_VERIFY",           /* 0x05 */
	"CMD_FORMATTRK",        /* 0x06 */
	"CMD_FORMATBAD",        /* 0x07 */
	"CMD_READ",             /* 0x08 */
	nullptr,                   /* 0x09 */
	"CMD_WRITE",            /* 0x0A */
	"CMD_SEEK",             /* 0x0B */
	"CMD_SETPARAM",         /* 0x0C */
	"CMD_GETECC",           /* 0x0D */
	"CMD_READSBUFF",        /* 0x0E */
	"CMD_WRITESBUFF",       /* 0x0F */

	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x10-0x17 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x18-0x1F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x20-0x27 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x28-0x2F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x30-0x37 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x38-0x3F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x40-0x47 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x48-0x4F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x50-0x57 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x58-0x5F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x60-0x67 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x68-0x6F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x70-0x77 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x78-0x7F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x80-0x87 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x88-0x8F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x90-0x97 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0x98-0x9F */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xA0-0xA7 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xA8-0xAF */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xB0-0xB7 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xB8-0xBF */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xC0-0xC7 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xC8-0xCF */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xD0-0xD7 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xD8-0xDF */

	"CMD_RAMDIAG",          /* 0xE0 */
	nullptr,                   /* 0xE1 */
	nullptr,                   /* 0xE2 */
	"CMD_DRIVEDIAG",        /* 0xE3 */
	"CMD_INTERNDIAG",       /* 0xE4 */
	"CMD_READLONG",         /* 0xE5 */
	"CMD_WRITELONG",        /* 0xE6 */
	nullptr,                   /* 0xE7 */

	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xE8-0xEF */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, /* 0xF0-0xF7 */
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr  /* 0xF8-0xFF */
};

ROM_START( hdc )
	ROM_REGION(0x02000,"hdc", 0)
	// BIOS taken from WD1002A-WX1
	ROM_LOAD("wdbios.rom",  0x00000, 0x02000, CRC(8e9e2bd4) SHA1(601d7ceab282394ebab50763c267e915a6a2166a)) /* WDC IDE Superbios 2.0 (06/28/89) Expansion Rom C8000-C9FFF  */
ROM_END

static INPUT_PORTS_START( isa_hdc )
	PORT_START("HDD")
	PORT_BIT(     0xb0, 0xb0, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, "IRQ level")
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0c, 0x0c, "Type of 1st drive")
	PORT_DIPSETTING(    0x0c, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x03, 0x03, "Type of 2nd drive")
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START("ROM")
	PORT_DIPNAME( 0x01, 0x01, "Install ROM?")
	PORT_DIPSETTING(    0x01, DEF_STR(Yes) )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
INPUT_PORTS_END

DEFINE_DEVICE_TYPE(XT_HDC,     xt_hdc_device, "xt_hdc", "Generic PC-XT Fixed Disk Controller")
DEFINE_DEVICE_TYPE(EC1841_HDC, ec1841_device, "ec1481", "EX1841 Fixed Disk Controller")
DEFINE_DEVICE_TYPE(ST11M_HDC,  st11m_device,  "st11m",  "Seagate ST11M Fixed Disk Controller")

xt_hdc_device::xt_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xt_hdc_device(mconfig, XT_HDC, tag, owner, clock)
{
	m_type = STANDARD;
}

xt_hdc_device::xt_hdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_buffer_ptr(nullptr),
	m_csb(0),
	m_status(0),
	m_error(0),
	m_type(0),
	m_current_cmd(0),
	m_irq_handler(*this),
	m_drq_handler(*this),
	m_drv(0),
	m_timer(nullptr),
	m_data_cnt(0),
	m_hdc_control(0),
	m_hdcdma_src(nullptr),
	m_hdcdma_dst(nullptr),
	m_hdcdma_read(0),
	m_hdcdma_write(0),
	m_hdcdma_size(0)
{
}

ec1841_device::ec1841_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xt_hdc_device(mconfig, EC1841_HDC, tag, owner, clock),
	m_irq_handler(*this),
	m_drq_handler(*this)
{
	m_type = EC1841;
}

st11m_device::st11m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xt_hdc_device(mconfig, ST11M_HDC, tag, owner, clock),
	m_irq_handler(*this),
	m_drq_handler(*this)
{
	m_type = ST11M;
}

void xt_hdc_device::device_start()
{
	m_buffer = std::make_unique<uint8_t[]>(256 * 512); // maximum possible transfer
	m_timer = timer_alloc(FUNC(xt_hdc_device::process_command), this);
}

void xt_hdc_device::device_reset()
{
	m_drv = 0;
	m_data_cnt = 0;
	m_buffer_ptr = &m_buffer[0];
	m_hdc_control = 0;
	for (int i = 0; i < 2; i++)
	{
		m_cylinders[i] = 612;
		m_rwc[i] = 613;
		m_wp[i] = 613;
		m_heads[i] = 4;
		m_ecc[i] = 11;

		/* indexes */
		m_cylinder[i] = 0;
		m_head[i] = 0;
		m_sector[i] = 0;
		m_sector_cnt[i] = 0;
		m_control[i] = 0;
	}

	m_csb = 0;
	m_status = STA_COMMAND | STA_READY;
	m_error = 0;
}

harddisk_image_device *xt_hdc_device::pc_hdc_file(int id)
{
	harddisk_image_device *img = nullptr;
	switch (id)
	{
	case 0:
		img = subdevice<harddisk_image_device>("primary");
		break;
	case 1:
		img = subdevice<harddisk_image_device>("slave");
		break;
	}
	if (img == nullptr)
		return nullptr;

	if (!img->exists())
		return nullptr;

	return img;
}

void xt_hdc_device::pc_hdc_result(bool set_error_info)
{
	if (m_hdc_control & 0x02)
	{
		// dip switch selected IRQ 5 or 2
		m_irq_handler(1);
	}

	LOGMASKED(LOG_HDC_STATUS, "pc_hdc_result(): $%02x to $%04x\n", m_csb, m_data_cnt);

	m_buffer[m_data_cnt++] = m_csb;

	if (set_error_info && (m_csb & CSB_ERROR))
	{
		m_buffer[m_data_cnt++] = m_error;
		if (m_error & 0x80)
		{
			m_buffer[m_data_cnt++] = (m_drv << 5) | m_head[m_drv];
			m_buffer[m_data_cnt++] = ((m_cylinder[m_drv] >> 2) & 0xc0) | m_sector[m_drv];
			m_buffer[m_data_cnt++] = m_cylinder[m_drv] & 0xff;

			LOGMASKED(LOG_HDC_STATUS, "pc_hdc_result(): result [%02x %02x %02x %02x]\n",
				m_buffer[m_data_cnt - 4], m_buffer[m_data_cnt - 3], m_buffer[m_data_cnt - 2], m_buffer[m_data_cnt - 1]);
		}
		else
		{
			LOGMASKED(LOG_HDC_STATUS, "pc_hdc_result(): result [%02x]\n", m_buffer[m_data_cnt - 1]);
		}
	}
	m_status |= STA_INTERRUPT | STA_INPUT | STA_REQUEST | STA_COMMAND | STA_READY;
}



bool xt_hdc_device::no_dma()
{
	return (m_hdc_control & CTL_DMA) == 0;
}



int xt_hdc_device::get_lbasector()
{
	harddisk_image_device *file = pc_hdc_file(m_drv);
	const auto &info = file->get_info();

	int lbasector = m_cylinder[m_drv];
	lbasector *= info.heads;
	lbasector += m_head[m_drv];
	lbasector *= info.sectors;
	lbasector += m_sector[m_drv];
	return lbasector;
}

/********************************************************************
 *
 * Read a number of sectors to the address set up for DMA chan #3
 *
 ********************************************************************/

/* the following crap is an abomination; it is a relic of the old crappy DMA
 * implementation that threw the idea of "emulating the hardware" to the wind
 */

uint8_t xt_hdc_device::dack_r()
{
	harddisk_image_device *file = pc_hdc_file(m_drv);
	if (!file)
		return 0;
	const auto &info = file->get_info();

	if (m_hdcdma_read == 0)
	{
		file->read(get_lbasector(), m_hdcdma_data);
		m_hdcdma_read = 512;
		m_hdcdma_size -= 512;
		m_hdcdma_src = m_hdcdma_data;
		m_sector[m_drv]++;
	}

	uint8_t result = *(m_hdcdma_src++);

	if (--m_hdcdma_read == 0)
	{
		/* end of cylinder ? */
		if (m_sector[m_drv] >= info.sectors)
		{
			m_sector[m_drv] = 0;
			if (++m_head[m_drv] >= info.heads)  /* beyond heads? */
			{
				m_head[m_drv] = 0;              /* reset head */
				m_cylinder[m_drv]++;            /* next cylinder */
			}
		}
	}

	if (!no_dma())
	{
		m_drq_handler((m_hdcdma_read || m_hdcdma_size) ? 1 : 0);
		if (!(m_hdcdma_read || m_hdcdma_size))
		{
			pc_hdc_result(false);
		}
	}

	return result;
}

uint8_t xt_hdc_device::dack_rs()
{
	logerror("%s dack_rs(%d %d)\n", machine().describe_context(), m_hdcdma_read, m_hdcdma_size);

	if (m_hdcdma_read == 0)
	{
		m_hdcdma_read = 512;
		m_hdcdma_size -= 512;
		m_hdcdma_src = m_hdcdma_data;
	}

	uint8_t m_result = *(m_hdcdma_src++);

	m_hdcdma_read--;

	if (!no_dma())
	{
		m_drq_handler(m_hdcdma_read ? 1 : 0);
		if (!m_hdcdma_read)
		{
			pc_hdc_result(false);
		}
	}

	return m_result;
}



void xt_hdc_device::dack_w(uint8_t data)
{
	harddisk_image_device *file = pc_hdc_file(m_drv);
	if (!file)
		return;
	const auto &info = file->get_info();

	*(m_hdcdma_dst++) = data;

	if (--m_hdcdma_write == 0)
	{
		file->write(get_lbasector(), m_hdcdma_data);
		m_hdcdma_write = 512;
		m_hdcdma_size -= 512;

		/* end of cylinder ? */
		if (++m_sector[m_drv] >= info.sectors)
		{
			m_sector[m_drv] = 0;
			if (++m_head[m_drv] >= info.heads)  /* beyond heads? */
			{
				m_head[m_drv] = 0;              /* reset head */
				m_cylinder[m_drv]++;            /* next cylinder */
			}
		}
		m_hdcdma_dst = m_hdcdma_data;
	}

	if (!no_dma())
	{
		m_drq_handler(m_hdcdma_size ? 1 : 0);
		if (!m_hdcdma_size)
		{
			pc_hdc_result(true);
		}
	}
}



void xt_hdc_device::dack_ws(uint8_t data)
{
	*(m_hdcdma_dst++) = data;

	if (--m_hdcdma_write == 0)
	{
		m_hdcdma_write = 512;
		m_hdcdma_size -= 512;
		m_hdcdma_dst = m_hdcdma_data;
	}

	if (!no_dma())
	{
		m_drq_handler(m_hdcdma_size ? 1 : 0);
		if (!m_hdcdma_size)
		{
			pc_hdc_result(true);
		}
	}
}



void xt_hdc_device::execute_read()
{
	int size = m_sector_cnt[m_drv] * 512;

	if (m_sector_cnt[m_drv] == 0)
		size = 256 * 512;

	harddisk_image_device *disk = pc_hdc_file(m_drv);
	if (!disk)
		return;

	m_status |= STA_READY;  // ready to receive data
	m_status &= ~(STA_INPUT | STA_COMMAND);

	m_hdcdma_src = m_hdcdma_data;
	m_hdcdma_read = 0;
	m_hdcdma_size = size;

	if (!no_dma())
	{
		m_drq_handler(1);
		if (!m_hdcdma_size)
		{
			pc_hdc_result(false);
		}
	}
}

void xt_hdc_device::execute_readsbuff()
{
	m_status |= STA_READY;  // ready to receive data
	m_status &= ~(STA_INPUT | STA_COMMAND);

	m_hdcdma_src = m_hdcdma_data;
	m_hdcdma_read = 512;
	m_hdcdma_size = 512;

	if (!no_dma())
	{
		m_drq_handler(1);
		if (!m_hdcdma_size)
		{
			pc_hdc_result(false);
		}
	}
}



void xt_hdc_device::execute_write()
{
	int size = m_sector_cnt[m_drv] * 512;

	if (m_sector_cnt[m_drv] == 0)
		size = 256 * 512;

	harddisk_image_device *disk = pc_hdc_file(m_drv);
	if (!disk)
		return;

	m_status |= STA_READY;  // ready to receive data
	m_status |= STA_INPUT;
	m_status &= ~STA_COMMAND;

	m_hdcdma_dst = m_hdcdma_data;
	m_hdcdma_write = 512;
	m_hdcdma_size = size;

	if (!no_dma())
	{
		m_drq_handler(1);
	}
}



void xt_hdc_device::execute_writesbuff()
{
	m_hdcdma_dst = m_hdcdma_data;
	m_hdcdma_write = 512;
	m_hdcdma_size = 512;

	m_status |= STA_READY;  // ready to receive data
	m_status |= STA_INPUT;
	m_status &= ~STA_COMMAND;

	if (!no_dma())
	{
		m_drq_handler(1);
	}
}



void xt_hdc_device::get_drive()
{
	m_drv = BIT(m_buffer[1], 5);
	m_csb = m_drv ? CSB_LUN : 0x00;
}



void xt_hdc_device::get_chsn()
{
	m_head[m_drv] = m_buffer[1] & 0x1f;
	m_sector[m_drv] = m_buffer[2] & 0x3f;
	m_cylinder[m_drv] = (m_buffer[2] & 0xc0) << 2;
	m_cylinder[m_drv] |= m_buffer[3];
	m_sector_cnt[m_drv] = m_buffer[4];
	m_control[m_drv] = m_buffer[5];   /* 7: no retry, 6: no ecc retry, 210: step rate */

	m_error = 0x80;   /* a potential error has C/H/S/N info */
}

bool xt_hdc_device::test_ready()
{
	if (!pc_hdc_file(m_drv))
	{
		m_csb |= CSB_ERROR;
		m_error |= 0x04;  /* drive not ready */
		return false;
	}
	return true;
}

TIMER_CALLBACK_MEMBER(xt_hdc_device::process_command)
{
	int old_error = m_error;          /* Previous error data is needed for CMD_SENSE */

	m_csb = 0x00;
	m_error = 0;

	m_buffer_ptr = &m_buffer[0];

	get_drive();
	m_data_cnt = 0;

	LOGMASKED(LOG_HDC_STATUS, "%s pc_hdc_command(): Executing command; cmd=0x%02x (%s) drv=%d\n",
		machine().describe_context(), m_current_cmd, s_hdc_command_names[m_current_cmd] ? s_hdc_command_names[m_current_cmd] : "Unknown", m_drv);

	switch (m_current_cmd)
	{
		case CMD_TESTREADY:
			test_ready();
			if (no_dma())
			{
				pc_hdc_result(false);
			}
			break;
		case CMD_SENSE:
			/* Perform error code translation. This may need to be expanded in the future. */
			m_buffer[m_data_cnt++] = (old_error & 0xc0) | (old_error & 0x04);
			m_buffer[m_data_cnt++] = (m_drv << 5) | m_head[m_drv];
			m_buffer[m_data_cnt++] = ((m_cylinder[m_drv] >> 2) & 0xc0) | m_sector[m_drv];
			m_buffer[m_data_cnt++] = m_cylinder[m_drv] & 0xff;
			if (no_dma())
			{
				pc_hdc_result(false);
			}
			break;
		case CMD_RECALIBRATE:
			get_chsn();
			if (no_dma())
			{
				pc_hdc_result(true);
			}
			break;

		case CMD_FORMATDRV:
		case CMD_VERIFY:
		case CMD_FORMATTRK:
		case CMD_FORMATBAD:
		case CMD_SEEK:
		case CMD_DRIVEDIAG:
			get_chsn();
			test_ready();
			if (no_dma())
			{
				pc_hdc_result(true);
			}
			break;

		case CMD_READ:
		case CMD_READLONG:
			get_chsn();

			LOGMASKED(LOG_HDC_STATUS, "%s hdc read D:%d C:%d H:%d S:%d N:%d CTL:$%02x\n",
				machine().describe_context(), m_drv, m_cylinder[m_drv], m_head[m_drv], m_sector[m_drv], m_sector_cnt[m_drv], m_control[m_drv]);

			if (test_ready())
				execute_read();
			else
				pc_hdc_result(true);
			break;

		case CMD_READSBUFF:
			LOGMASKED(LOG_HDC_STATUS, "%s hdc read sector buffer\n", machine().describe_context());

			execute_readsbuff();
			break;

		case CMD_WRITE:
		case CMD_WRITELONG:
			get_chsn();

			LOGMASKED(LOG_HDC_STATUS, "%s hdc write  D:%d C:%d H:%d S:%d N:%d CTL:$%02x\n",
				machine().describe_context(), m_drv, m_cylinder[m_drv], m_head[m_drv], m_sector[m_drv], m_sector_cnt[m_drv], m_control[m_drv]);

			if (test_ready())
				execute_write();
			break;

		case CMD_WRITESBUFF:
			LOGMASKED(LOG_HDC_STATUS, "%s hdc write sector buffer\n", machine().describe_context());

			execute_writesbuff();
			break;

		case CMD_SETPARAM:
			get_chsn();
			m_cylinders[m_drv] = ((m_buffer[6] & 3) << 8) | m_buffer[7];
			m_heads[m_drv] = m_buffer[8] & 0x1f;
			m_rwc[m_drv] = ((m_buffer[9] & 3) << 8) | m_buffer[10];
			m_wp[m_drv] = ((m_buffer[11] & 3) << 8) | m_buffer[12];
			m_ecc[m_drv] = m_buffer[13];
			if (no_dma())
			{
				pc_hdc_result(true);
			}
			break;

		case CMD_GETECC:
			m_buffer[m_data_cnt++] = m_ecc[m_drv];
			if (no_dma())
			{
				pc_hdc_result(true);
			}
			break;

		case CMD_RAMDIAG:
		case CMD_INTERNDIAG:
			if (no_dma())
			{
				pc_hdc_result(true);
			}
			break;
	}
}

/*  Command format
 *  Bits     Description
 *  7      0
 *  xxxxxxxx command
 *  dddhhhhh drive / head
 *  ccssssss cylinder h / sector
 *  cccccccc cylinder l
 *  nnnnnnnn count
 *  xxxxxxxx control
 *
 *  Command format extra for set drive characteristics
 *  000000cc cylinders h
 *  cccccccc cylinders l
 *  000hhhhh heads
 *  000000cc reduced write h
 *  cccccccc reduced write l
 *  000000cc write precomp h
 *  cccccccc write precomp l
 *  eeeeeeee ecc
 */
void xt_hdc_device::data_w(uint8_t data)
{
	if (!(m_status & STA_COMMAND) && m_current_cmd != CMD_SETPARAM)
	{
		LOGMASKED(LOG_HDC_DATA, "hdc_data_w PIO $%02x (%i) (%s): \n", data, m_data_cnt, s_hdc_command_names[m_current_cmd] ? s_hdc_command_names[m_current_cmd] : "Unknown");
		// PIO data transfer
		m_buffer[m_data_cnt++] = data;
		if (m_data_cnt >= m_hdcdma_size)
		{
			m_data_cnt = 0;
			// write to disk
			do
			{
				if (m_current_cmd == CMD_WRITESBUFF)
					dack_ws(m_buffer[m_data_cnt++]);
				else
					dack_w(m_buffer[m_data_cnt++]);
			}
			while (m_hdcdma_size);
			m_data_cnt = 0;
			pc_hdc_result(true);
		}
		return;
	}

	if (m_data_cnt == 0)
	{
		m_buffer_ptr = &m_buffer[0];
		m_current_cmd = data;
		m_data_cnt = 6;   /* expect 6 bytes including this one */
		m_status &= ~(STA_READY | STA_INPUT);
		switch (data)
		{
			case CMD_SETPARAM:
				m_data_cnt += 8;
				break;

			case CMD_TESTREADY:
			case CMD_RECALIBRATE:
			case CMD_SENSE:
			case CMD_FORMATDRV:
			case CMD_VERIFY:
			case CMD_FORMATTRK:
			case CMD_FORMATBAD:
			case CMD_READ:
			case CMD_WRITE:
			case CMD_SEEK:
			case CMD_GETECC:
			case CMD_READSBUFF:
			case CMD_WRITESBUFF:
			case CMD_RAMDIAG:
			case CMD_DRIVEDIAG:
			case CMD_INTERNDIAG:
			case CMD_READLONG:
			case CMD_WRITELONG:
				break;

			default:
				m_data_cnt = 0;
				m_status |= STA_INPUT;
				m_csb |= CSB_ERROR | 0x20; /* unknown command */
				pc_hdc_result(true);
				break;
		}
		if (m_data_cnt)
			m_status |= STA_REQUEST;
	}

	if (m_data_cnt)
	{
		LOGMASKED(LOG_HDC_DATA, "hdc_data_w $%02x (%i) (%s): \n", data, m_data_cnt, s_hdc_command_names[m_current_cmd] ? s_hdc_command_names[m_current_cmd] : "Unknown");

		*m_buffer_ptr++ = data;
		// XXX ec1841 wants this
		if (m_current_cmd == CMD_SETPARAM && m_data_cnt == 9 && m_type == EC1841)
		{
			m_status &= ~STA_READY;
		}
		else
		{
			m_status |= STA_READY;
			if (m_current_cmd == CMD_SETPARAM && m_data_cnt == 9)  // some controllers want geometry info as data, not as a command (true for the Seagate ST11M?)
				m_status &= ~STA_COMMAND;
		}
		if (--m_data_cnt == 0)
		{
			LOGMASKED(LOG_HDC_STATUS, "%s pc_hdc_data_w(): Launching command\n", machine().describe_context());

			m_status &= ~(STA_COMMAND | STA_REQUEST | STA_READY | STA_INPUT);
			m_timer->adjust(attotime::from_msec(1));
		}
	}
}



void xt_hdc_device::reset_w(uint8_t data)
{
	m_cylinder[0] = m_cylinder[1] = 0;
	m_head[0] = m_head[1] = 0;
	m_sector[0] = m_sector[1] = 0;
	m_csb = 0;
	m_status = STA_COMMAND | STA_READY;
	std::fill_n(&m_buffer[0], 256*512, 0);
	m_buffer_ptr = &m_buffer[0];
	m_data_cnt = 0;
}



void xt_hdc_device::select_w(uint8_t data)
{
	m_status &= ~STA_INTERRUPT;
	m_status |= STA_SELECT;
	m_status |= STA_READY;
}



void xt_hdc_device::control_w(uint8_t data)
{
	LOGMASKED(LOG_HDC_STATUS, "%s: pc_hdc_control_w(): control write %d\n", machine().describe_context(), data);

	m_hdc_control = data;

	if (!(m_hdc_control & 0x02))
	{
		m_irq_handler(0);
	}
}



uint8_t xt_hdc_device::data_r()
{
	uint8_t data = 0xff;

	if (!(m_status & STA_COMMAND) && (m_current_cmd == CMD_READ || m_current_cmd == CMD_READLONG || m_current_cmd == CMD_READSBUFF))
	{
		// PIO data transfer
		if (m_data_cnt == 0)
		{
			do
			{
				if (m_current_cmd == CMD_READSBUFF)
					m_buffer[m_data_cnt++] = dack_rs();
				else
					m_buffer[m_data_cnt++] = dack_r();
			} while (m_hdcdma_read);
			m_data_cnt = 0;
		}
		data = m_buffer[m_data_cnt++];
		if (m_data_cnt >= ((m_sector_cnt[m_drv] != 0) ? (m_sector_cnt[m_drv] * 512) : (256 * 512)))
		{
			m_data_cnt = 0;
			pc_hdc_result(true);
		}
		LOGMASKED(LOG_HDC_DATA, "hdc_data_r PIO $%02x (%i): \n", data, m_data_cnt);
		return data;
	}

	if (m_data_cnt)
	{
		data = *m_buffer_ptr++;
		m_status &= ~STA_INTERRUPT;
		if (--m_data_cnt == 0)
		{
			m_status &= ~(STA_INPUT | STA_REQUEST | STA_SELECT);
			m_status |= STA_COMMAND;
		}
		LOGMASKED(LOG_HDC_DATA, "hdc_data_r $%02x (%i): \n", data, m_data_cnt);
	}
	return data;
}



uint8_t xt_hdc_device::status_r()
{
	return m_status;
}

void xt_hdc_device::set_ready()
{
	m_status |= STA_READY; // XXX
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_HDC,        isa8_hdc_device,        "isa_hdc",        "Fixed Disk Controller Card")
DEFINE_DEVICE_TYPE(ISA8_HDC_EC1841, isa8_hdc_ec1841_device, "isa_hdc_ec1841", "EC1841 HDC Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_hdc_device::device_add_mconfig(machine_config &config)
{
	XT_HDC(config, m_hdc,0);
	m_hdc->irq_handler().set(FUNC(isa8_hdc_device::irq_w));
	m_hdc->drq_handler().set(FUNC(isa8_hdc_device::drq_w));
	HARDDISK(config, "hdc:primary", "st_hdd");
	HARDDISK(config, "hdc:slave", "st_hdd");
}

void isa8_hdc_ec1841_device::device_add_mconfig(machine_config &config)
{
	EC1841_HDC(config, m_hdc,0);
	m_hdc->irq_handler().set(FUNC(isa8_hdc_ec1841_device::irq_w));
	m_hdc->drq_handler().set(FUNC(isa8_hdc_ec1841_device::drq_w));
	HARDDISK(config, "hdc:primary", "st_hdd");
	HARDDISK(config, "hdc:slave", "st_hdd");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_hdc_device::device_rom_region() const
{
	return ROM_NAME( hdc );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa8_hdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( isa_hdc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_hdc_device - constructor
//-------------------------------------------------

isa8_hdc_device::isa8_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_hdc_device(mconfig, ISA8_HDC, tag, owner, clock)
{
}

isa8_hdc_device::isa8_hdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_hdc(*this,"hdc"),
	m_dip(0)
{
}

isa8_hdc_ec1841_device::isa8_hdc_ec1841_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_hdc_device( mconfig, ISA8_HDC_EC1841, tag, owner, clock),
	m_hdc(*this,"hdc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_hdc_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0320, 0x0323, read8sm_delegate(*this, FUNC(isa8_hdc_device::pc_hdc_r)), write8sm_delegate(*this, FUNC(isa8_hdc_device::pc_hdc_w)));
	m_isa->set_dma_channel(3, this, false);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_hdc_device::device_reset()
{
	m_dip = ioport("HDD")->read();

	if (ioport("ROM")->read() == 1 && m_hdc->install_rom())
		m_isa->install_rom(this, 0xc8000, 0xc9fff, "hdc");
}

/*************************************************************************
 *
 *      HDC
 *      hard disk controller
 *
 *************************************************************************/

uint8_t isa8_hdc_device::pc_hdc_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
		case 0: data = m_hdc->data_r();     break;
		case 1: data = m_hdc->status_r();   break;
		case 2: data = pc_hdc_dipswitch_r(); break;
		case 3: break;
	}

	LOGMASKED(LOG_HDC_CALL, "%s pc_hdc_r(): offs=%d result=0x%02x\n", machine().describe_context(), offset, data);

	return data;
}

void isa8_hdc_device::pc_hdc_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_HDC_CALL, "%s pc_hdc_w(): offs=%d data=0x%02x\n", machine().describe_context(), offset, data);

	switch (offset)
	{
		case 0: m_hdc->data_w(data);    break;
		case 1: m_hdc->reset_w(data);   break;
		case 2: m_hdc->select_w(data);  break;
		case 3: m_hdc->control_w(data); break;
	}
}


uint8_t isa8_hdc_device::dack_r(int line)
{
	if (m_hdc->get_command() == CMD_READSBUFF)
		return m_hdc->dack_rs();
	else
		return m_hdc->dack_r();
}

void isa8_hdc_device::dack_w(int line, uint8_t data)
{
	if (m_hdc->get_command() == CMD_WRITESBUFF)
		m_hdc->dack_ws(data);
	else
		m_hdc->dack_w(data);
}

/*
    Dipswitch configuration


    Tandon/Western Digital Fixed Disk Controller
    bit0-1 : Determine disk size(?)
        Causes geometry data to be read from c8043, c8053, c8063, c8073 (?)
        00 - 40 Mbytes
        01 - 30 Mbytes
        10 - 10 Mbytes
        11 - 20 Mbytes
    bit2-7 : unknown

 */

uint8_t isa8_hdc_device::pc_hdc_dipswitch_r()
{
	m_hdc->set_ready();
	LOGMASKED(LOG_HDC_STATUS, "%s: pc_hdc_dipswitch_r: status $%02X\n", machine().describe_context(), m_hdc->status_r());
	return m_dip;
}

void isa8_hdc_device::irq_w(int state)
{
	if (BIT(m_dip, 6))
		m_isa->irq5_w(state);
	else
		m_isa->irq2_w(state);
}

void isa8_hdc_device::drq_w(int state)
{
	m_isa->drq3_w(state);
}
