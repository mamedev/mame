// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    SD Card emulation, SPI interface.
    Emulation by R. Belmont

    This emulates either an SDHC (SPI_SDCARD) or an SDV2 card (SPI_SDCARDV2).  SDHC has a fixed
    512 byte block size and the arguments to the read/write commands are block numbers.  SDV2
    has a variable block size defaulting to 512 and the arguments to the read/write commands
    are byte offsets.

    The block size set with CMD16 must match the underlying CHD block size if it's not 512.

    Adding the native 4-bit-wide SD interface is also possible; this should be broken up into a base
    SD Card class with SPI and SD frontends in that case.

    Multiple block read/write commands are not supported but would be straightforward to add.

    References:
    https://www.sdcard.org/downloads/pls/ (Physical Layer Simplified Specification)
    REF: tags are referring to the spec form above. 'Physical Layer Simplified Specification v8.00'

    http://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf
    https://embdev.net/attachment/39390/TOSHIBA_SD_Card_Specification.pdf
    http://elm-chan.org/docs/mmc/mmc_e.html
*/

#include "emu.h"
#include "spi_sdcard.h"
#include "imagedev/harddriv.h"

#include "multibyte.h"

#include <algorithm>

#define LOG_COMMAND (1U << 1)
#define LOG_SPI     (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_COMMAND)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

constexpr u8 DATA_RESPONSE_OK        = 0x05;
constexpr u8 DATA_RESPONSE_IO_ERROR  = 0x0d;

enum {
	CSD_STRUCTURE_V10    = 0x0,
	CSD_STRUCTURE_V20    = 0x1,

	TAAC_UNIT_1NS        = 0x00,
	TAAC_UNIT_10NS       = 0x01,
	TAAC_UNIT_100NS      = 0x02,
	TAAC_UNIT_1US        = 0x03,
	TAAC_UNIT_10US       = 0x04,
	TAAC_UNIT_100US      = 0x05,
	TAAC_UNIT_1MS        = 0x06,
	TAAC_UNIT_10MS       = 0x07,

	TAAC_VALUE_1_0       = 0x08,
	TAAC_VALUE_1_2       = 0x10,
	TAAC_VALUE_1_3       = 0x18,
	TAAC_VALUE_1_5       = 0x20,
	TAAC_VALUE_2_0       = 0x28,
	TAAC_VALUE_2_5       = 0x30,
	TAAC_VALUE_3_0       = 0x38,
	TAAC_VALUE_3_5       = 0x40,
	TAAC_VALUE_4_0       = 0x48,
	TAAC_VALUE_4_5       = 0x50,
	TAAC_VALUE_5_0       = 0x58,
	TAAC_VALUE_5_5       = 0x60,
	TAAC_VALUE_6_0       = 0x68,
	TAAC_VALUE_7_0       = 0x70,
	TAAC_VALUE_8_0       = 0x78,

	TRAN_SPEED_UNIT_100K = 0x00,
	TRAN_SPEED_UNIT_1M   = 0x01,
	TRAN_SPEED_UNIT_10M  = 0x02,
	TRAN_SPEED_UNIT_100M = 0x03,

	TRAN_SPEED_VALUE_1_0 = 0x08,
	TRAN_SPEED_VALUE_1_2 = 0x10,
	TRAN_SPEED_VALUE_1_3 = 0x18,
	TRAN_SPEED_VALUE_1_5 = 0x20,
	TRAN_SPEED_VALUE_2_0 = 0x28,
	TRAN_SPEED_VALUE_2_5 = 0x30,
	TRAN_SPEED_VALUE_3_0 = 0x38,
	TRAN_SPEED_VALUE_3_5 = 0x40,
	TRAN_SPEED_VALUE_4_0 = 0x48,
	TRAN_SPEED_VALUE_4_5 = 0x50,
	TRAN_SPEED_VALUE_5_0 = 0x58,
	TRAN_SPEED_VALUE_5_5 = 0x60,
	TRAN_SPEED_VALUE_6_0 = 0x68,
	TRAN_SPEED_VALUE_7_0 = 0x70,
	TRAN_SPEED_VALUE_8_0 = 0x78,

	VDD_CURR_MIN_0_5MA   = 0x00,
	VDD_CURR_MIN_1MA     = 0x01,
	VDD_CURR_MIN_5MA     = 0x02,
	VDD_CURR_MIN_10MA    = 0x03,
	VDD_CURR_MIN_25MA    = 0x04,
	VDD_CURR_MIN_35MA    = 0x05,
	VDD_CURR_MIN_60MA    = 0x06,
	VDD_CURR_MIN_100MA   = 0x07,

	VDD_CURR_MAX_1MA     = 0x00,
	VDD_CURR_MAX_5MA     = 0x01,
	VDD_CURR_MAX_10MA    = 0x02,
	VDD_CURR_MAX_25MA    = 0x03,
	VDD_CURR_MAX_35MA    = 0x04,
	VDD_CURR_MAX_45MA    = 0x05,
	VDD_CURR_MAX_80MA    = 0x06,
	VDD_CURR_MAX_200MA   = 0x07
};

} // anonymous namespace

enum spi_sdcard_device::sd_state : u8
{
	//REF Table 4-1:Overview of Card States vs. Operation Mode
	SD_STATE_IDLE = 0,
	SD_STATE_READY,
	SD_STATE_IDENT,
	SD_STATE_STBY,
	SD_STATE_TRAN,
	SD_STATE_DATA,
	SD_STATE_DATA_MULTI, // synthetical state for this implementation
	SD_STATE_RCV,
	SD_STATE_PRG,
	SD_STATE_DIS,
	SD_STATE_INA,

	//FIXME Existing states which must be revisited
	SD_STATE_WRITE_WAITFE,
	SD_STATE_WRITE_DATA
};

ALLOW_SAVE_TYPE(spi_sdcard_device::sd_state);


DEFINE_DEVICE_TYPE(SPI_SDCARD, spi_sdcard_device, "spi_sdcard", "SD Card (SPI interface)")

spi_sdcard_device::spi_sdcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	spi_sdcard_device(mconfig, SPI_SDCARD, tag, owner, clock)
{
}

spi_sdcard_device::spi_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	write_miso(*this),
	m_image(*this, "image"),
	m_preferred_type(SD_TYPE_V2),
	m_ignore_stop_bit(false),
	m_blksize(512),
	m_type(SD_TYPE_V2),
	m_state(SD_STATE_IDLE),
	m_ss(0), m_in_bit(0), m_clk_state(0),
	m_in_latch(0), m_out_latch(0xff), m_cur_bit(0),
	m_out_delay(0), m_out_count(0), m_out_ptr(0), m_write_ptr(0), m_xferblk(512), m_blknext(0),
	m_bACMD(false)
{
	std::fill(std::begin(m_csd), std::end(m_csd), 0);
}

spi_sdcard_device::~spi_sdcard_device()
{
}

void spi_sdcard_device::device_start()
{
	m_data = make_unique_clear<u8 []>(2048 + 8);

	save_pointer(NAME(m_data), 2048 + 8);
	save_item(NAME(m_cmd));
	save_item(NAME(m_state));
	save_item(NAME(m_ss));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_cur_bit));
	save_item(NAME(m_out_delay));
	save_item(NAME(m_out_count));
	save_item(NAME(m_out_ptr));
	save_item(NAME(m_write_ptr));
	save_item(NAME(m_xferblk));
	save_item(NAME(m_blknext));
	save_item(NAME(m_bACMD));
}

std::error_condition spi_sdcard_device::image_loaded(device_image_interface &image)
{
	// need block size and total blocks to create CSD
	auto const info = m_image->get_info();
	u64 const total_blocks = u64(info.cylinders) * info.heads * info.sectors;
	if (!total_blocks)
	{
		osd_printf_error("%s: SD Card cannot mount a zero-block image\n", tag());
		return image_error::INVALIDIMAGE;
	}

	// ensure block size can be expressed in the CSD
	if ((info.sectorbytes & (info.sectorbytes - 1)) || !info.sectorbytes || (512 > info.sectorbytes) || (2048 < info.sectorbytes))
	{
		osd_printf_error("%s: SD Card cannot use sector size %u (must be a power of 2 from 512 to 2048)\n", tag());
		return image_error::INVALIDIMAGE;
	}
	u8 block_size_exp = 0;
	for (auto i = info.sectorbytes; !BIT(i, 0); i >>= 1)
		++block_size_exp;

	// see how we can express the total block count
	u64 total_mant = total_blocks;
	u8 total_exp = 0;
	while (!BIT(total_mant, 0))
	{
		total_mant >>= 1;
		++total_exp;
	}
	bool const sd_ok = (2 <= total_exp) && ((1 << 12) >= (total_mant << ((9 < total_exp) ? (total_exp - 9) : 0)));
	bool const sdhc_ok = (512 == info.sectorbytes) && (10 <= total_exp) && ((u32(1) << 16) >= (total_mant << (total_exp - 10)));
	if (!sd_ok && !sdhc_ok)
	{
		osd_printf_error("%s: SD Card image size %u blocks of %u bytes is not supported by SD or SDHC\n", tag(), total_blocks, info.sectorbytes);
		return image_error::INVALIDIMAGE;
	}

	try
	{
		m_sectorbuf.resize(info.sectorbytes);
	}
	catch (std::bad_alloc const &)
	{
		osd_printf_error("%s: Error allocating %u-byte SD Card sector buffer\n", tag(), info.sectorbytes);
		return std::errc::not_enough_memory;
	}

	m_blksize = m_xferblk = info.sectorbytes;

	// set up common CSD fields
	m_csd[0]  =  0x00;                                       // 127: CSD_STRUCTURE:2 (00b) 0:6
	m_csd[1]  =  0x00;                                       // 119: TAAC:8
	m_csd[2]  =  0x00;                                       // 111: NSAC:8
	m_csd[3]  =  TRAN_SPEED_UNIT_10M | TRAN_SPEED_VALUE_2_5; // 103: TRAN_SPEED:8 (32h for 25MHz or 5Ah for 50MHz)
	m_csd[4]  =  0x5b;                                       //  95: CCC:12 (01x110110101b)
	m_csd[5]  =  0x50;                                       //      .. READ_BL_LN:4
	m_csd[5]  |= block_size_exp;
	m_csd[6]  =  0x00;                                       //  79: READ_BL_PARTIAL:1 WRITE_BLK_MISALIGN:1 READ_BLK_MISALIGN:1 DSR_IMP:1 0:2 C_SIZE:12
	m_csd[7]  =  0x00;                                       //      ..
	m_csd[8]  =  0x00;                                       //      .. VDD_R_CURR_MIN:3 VDD_R_CURR_MAX:3
	m_csd[9]  =  0x00;                                       //  55: VDD_W_CURR_MIN:3 VDD_W_CURR_MAX:3 C_SIZE_MUL:3
	m_csd[10] =  0x3f;                                       //      .. ERASE_BLK_EN:1 SECTOR_SIZE:7
	m_csd[11] =  0x80;                                       //      .. WP_GRP_SIZE:7
	m_csd[12] =  0x04;                                       //  31: WP_GRP_ENABLE:1 0:2 R2W_FACTOR:3 WRITE_BL_LEN:4
	m_csd[12] |= BIT(block_size_exp, 2, 2);
	m_csd[13] =  0x00;                                       //      .. WRITE_BL_PARTIAL:1 0:5
	m_csd[13] |= BIT(block_size_exp, 0, 2) << 6;
	m_csd[14] =  0x00;                                       //  15: FILE_FORMAT_GRP:1 COPY:1 PERM_WRITE_PROTECT:1 TMP_WRITE_PROTECT:1 FILE_FORMAT:2 WP_UPC:1 0:1
	m_csd[15] =  0x01;                                       //   7: CRC7 1:1

	if (sdhc_ok && ((SD_TYPE_HC == m_preferred_type) || !sd_ok))
	{
		u32 const c_size = (total_blocks >> 10) - 1;
		osd_printf_verbose(
				"%s: SD Card image mounted as SDHC, %u blocks of %u bytes, device size ((%u + 1) << 10) * (1 << %u)\n",
				tag(),
				total_blocks, info.sectorbytes,
				c_size, block_size_exp);

		m_type = SD_TYPE_HC;

		// set up CSD Version 2.0
		m_csd[0]  |= CSD_STRUCTURE_V20 << 6;                 // 127: CSD_STRUCTURE:2 (00b) 0:6
		m_csd[1]  =  TAAC_UNIT_1MS | TAAC_VALUE_1_0;         // 119: TAAC:8

		m_csd[7]  |= BIT(c_size, 16, 6);                     //      .. C_SIZE:22
		m_csd[8]  |= BIT(c_size, 8, 8);                      //      ..
		m_csd[9]  |= BIT(c_size, 0, 8);                      //      ..
	}
	else
	{
		u8 const c_size_mult = std::min<u8>(total_exp, 9) - 2;
		u16 const c_size = (total_blocks >> (c_size_mult + 2)) - 1;
		osd_printf_verbose(
				"%s: SD Card image mounted as SD, %u blocks of %u bytes, device size ((%u + 1) << (%u + 2)) * (1 << %u)\n",
				tag(),
				total_blocks, info.sectorbytes,
				c_size, c_size_mult, block_size_exp);

		m_type = SD_TYPE_V2;

		// set up CSD Version 1.0
		m_csd[0]  |= CSD_STRUCTURE_V10 << 6;                 // 127: CSD_STRUCTURE:2 (00b) 0:6
		m_csd[1]  =  TAAC_UNIT_1MS | TAAC_VALUE_1_5;         // 119: TAAC:8

		m_csd[6]  |= 0x80;                                   //  79: READ_BL_PARTIAL:1 WRITE_BLK_MISALIGN:1 READ_BLK_MISALIGN:1 DSR_IMP:1 0:2 C_SIZE:12
		m_csd[6]  |= BIT(c_size, 10, 2);
		m_csd[7]  |= BIT(c_size, 2, 8);                      //      ..
		m_csd[8]  |= BIT(c_size, 0, 2) << 6;                 //      .. VDD_R_CURR_MIN:3 VDD_R_CURR_MAX:3
		m_csd[8]  |= VDD_CURR_MIN_100MA << 3;
		m_csd[8]  |= VDD_CURR_MAX_80MA;
		m_csd[9]  |= VDD_CURR_MIN_100MA << 5;                //  55: VDD_W_CURR_MIN:3 VDD_W_CURR_MAX:3 C_SIZE_MUL:3
		m_csd[9]  |= VDD_CURR_MAX_80MA << 2;
		m_csd[9]  |= BIT(c_size_mult, 1, 2);
		m_csd[10] |= BIT(c_size_mult, 0, 1) << 7;            //      .. ERASE_BLK_EN:1 SECTOR_SIZE:7
		m_csd[11] |= 0x3f;                                   //      .. WP_GRP_SIZE:7
	}

	// TODO: calculate CRC7

	LOG("Generated CSD %016x%016x\n", get_u64be(&m_csd[0]), get_u64be(&m_csd[8]));

	return std::error_condition();
}

void spi_sdcard_device::image_unloaded(device_image_interface &image)
{
	std::fill(std::begin(m_csd), std::end(m_csd), 0);
}

void spi_sdcard_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, m_image).set_interface("sdcard");
	m_image->set_device_load(FUNC(spi_sdcard_device::image_loaded));
	m_image->set_device_unload(FUNC(spi_sdcard_device::image_unloaded));
}

void spi_sdcard_device::send_data(u16 count, sd_state new_state, u8 delay)
{
	m_out_delay = delay;
	m_out_ptr = 0;
	m_out_count = count;
	change_state(new_state);
}

void spi_sdcard_device::spi_clock_w(int state)
{
	// only respond if selected, and a clock edge
	if (m_ss && state != m_clk_state)
	{
		// We implement SPI Mode 3 signalling, in which we latch the data on
		// rising clock edges, and shift the data on falling clock edges.
		// See http://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf for details
		// on the 4 SPI signalling modes. SD Cards can work in either Mode 0 or Mode 3,
		// both of which shift on the falling edge and latch on the rising edge but
		// have opposite CLK polarity.
		if (state)
			latch_in();
		else
			shift_out();
	}
	m_clk_state = state;
}

void spi_sdcard_device::latch_in()
{
	m_in_latch &= ~0x01;
	m_in_latch |= m_in_bit;
	LOGMASKED(LOG_SPI, "\tsdcard: L %02x (%d) (out %02x)\n", m_in_latch, m_cur_bit, m_out_latch);
	m_cur_bit++;
	if (m_cur_bit == 8)
	{
		LOGMASKED(LOG_SPI, "SDCARD: got %02x\n", m_in_latch);
		if (m_state == SD_STATE_WRITE_WAITFE)
		{
			if (m_in_latch == 0xfe)
			{
				m_state = SD_STATE_WRITE_DATA;
				m_out_latch = 0xff;
				m_write_ptr = 0;
			}
		}
		else if (m_state == SD_STATE_WRITE_DATA)
		{
			m_data[m_write_ptr++] = m_in_latch;
			if (m_write_ptr == (m_xferblk + 2))
			{
				LOG("writing LBA %x, data %02x %02x %02x %02x\n", m_blknext, m_data[0], m_data[1], m_data[2], m_data[3]);
				// TODO: this is supposed to be a CRC response, the actual write will take some time
				if (m_image->write(m_blknext, &m_data[0]))
				{
					m_data[0] = DATA_RESPONSE_OK;
				}
				else
				{
					m_data[0] = DATA_RESPONSE_IO_ERROR;
				}
				m_data[1] = 0x01;

				send_data(2, SD_STATE_IDLE, 0); // zero delay - must immediately follow the data
			}
		}
		else // receive CMD
		{
			std::memmove(m_cmd, m_cmd + 1, 5);
			m_cmd[5] = m_in_latch;

			if (m_state == SD_STATE_DATA_MULTI)
			{
				do_command();
				if (m_state == SD_STATE_DATA_MULTI && m_out_count == 0)
				{
					// FIXME: support multi-block read when transfer size is smaller than block size
					m_data[0] = 0xfe; // data token
					m_image->read(m_blknext++, &m_data[1]);
					util::crc16_t crc16 = util::crc16_creator::simple(&m_data[1], m_blksize);
					put_u16be(&m_data[m_blksize + 1], crc16);
					LOG("reading LBA %x: [0] %02x %02x .. [%d] %02x %02x [crc16] %04x\n", m_blknext - 1, m_data[1], m_data[2], m_blksize - 2, m_data[m_blksize - 1], m_data[m_blksize], crc16);
					send_data(1 + m_blksize + 2, SD_STATE_DATA_MULTI);
				}
			}
			else if ((m_state == SD_STATE_IDLE) || (((m_cmd[0] & 0x70) == 0x40) || (m_out_count == 0))) // CMD0 - GO_IDLE_STATE
			{
				do_command();
			}
		}
	}
}

void spi_sdcard_device::shift_out()
{
	m_in_latch <<= 1;
	m_out_latch <<= 1;
	m_out_latch |= 1;
	LOGMASKED(LOG_SPI, "\tsdcard: S %02x %02x (%d)\n", m_in_latch, m_out_latch, m_cur_bit);

	m_cur_bit &= 0x07;
	if (m_cur_bit == 0)
	{
		if (m_out_ptr < m_out_delay)
		{
			m_out_ptr++;
		}
		else if (m_out_count > 0)
		{
			m_out_latch = m_data[m_out_ptr - m_out_delay];
			m_out_ptr++;
			LOGMASKED(LOG_SPI, "SDCARD: latching %02x (start of shift)\n", m_out_latch);
			m_out_count--;
		}
	}
	write_miso(BIT(m_out_latch, 7));
}

void spi_sdcard_device::do_command()
{
	if (((m_cmd[0] & 0xc0) == 0x40) && ((m_cmd[5] & 1) || m_ignore_stop_bit))
	{
		LOGMASKED(LOG_COMMAND, "SDCARD: cmd %02d %02x %02x %02x %02x %02x\n", m_cmd[0] & 0x3f, m_cmd[1], m_cmd[2], m_cmd[3], m_cmd[4], m_cmd[5]);
		bool clean_cmd = true;
		switch (m_cmd[0] & 0x3f)
		{
		case 0: // CMD0 - GO_IDLE_STATE
			if (m_image->exists())
			{
				m_data[0] = 0x01;
				send_data(1, SD_STATE_IDLE);
			}
			else
			{
				m_data[0] = 0x00;
				send_data(1, SD_STATE_INA);
			}
			break;

		case 1: // CMD1 - SEND_OP_COND
			m_data[0] = 0x00;
			send_data(1, SD_STATE_READY);
			break;

		case 8: // CMD8 - SEND_IF_COND (SD v2 only)
			m_data[0] = 0x01;
			m_data[1] = 0;
			m_data[2] = 0;
			m_data[3] = 0x01;
			m_data[4] = 0xaa;
			send_data(5, SD_STATE_IDLE);
			break;

		case 9: // CMD9 - SEND_CSD
			m_data[0] = 0x00;
			m_data[1] = 0xff;
			m_data[2] = 0xfe;
			std::copy(std::begin(m_csd), std::end(m_csd), &m_data[3]);

			send_data(3 + std::size(m_csd), SD_STATE_STBY);
			break;

		case 10: // CMD10 - SEND_CID
			m_data[0]  = 0x00; // initial R1 response
			m_data[1]  = 0xff; // throwaway byte before data transfer
			m_data[2]  = 0xfe; // data token
			m_data[3]  = 'M';  // Manufacturer ID - we'll use M for MAME
			m_data[4]  = 'M';  // OEM ID - MD for MAMEdev
			m_data[5]  = 'D';
			m_data[6]  = 'M'; // Product Name - "MCARD"
			m_data[7]  = 'C';
			m_data[8]  = 'A';
			m_data[9]  = 'R';
			m_data[10] = 'D';
			m_data[11] = 0x10; // Product Revision in BCD (1.0)
			{
				u32 uSerial = 0x12345678;
				put_u32be(&m_data[12], uSerial); // PSN - Product Serial Number
			}
			m_data[16] = 0x01; // MDT - Manufacturing Date
			m_data[17] = 0x59; // 0x15 9 = 2021, September
			m_data[18] = 0x00; // CRC7, bit 0 is always 0
			{
				util::crc16_t crc16 = util::crc16_creator::simple(&m_data[3], 16);
				put_u16be(&m_data[19], crc16);
			}
			send_data(3 + 16 + 2, SD_STATE_STBY);
			break;

		case 12: // CMD12 - STOP_TRANSMISSION
			m_data[0] = 0;
			send_data(1, (m_state == SD_STATE_RCV) ? SD_STATE_PRG : SD_STATE_TRAN);
			break;

		case 13: // CMD13 - SEND_STATUS
			m_data[0] = 0; // TODO
			m_data[1] = 0;
			send_data(2, SD_STATE_STBY);
			break;

		case 16: // CMD16 - SET_BLOCKLEN
			if (m_image->exists())
			{
				u16 const blocklen = get_u16be(&m_cmd[3]);
				if (blocklen && ((m_type == SD_TYPE_V2) || (blocklen == m_blksize)) && (blocklen <= m_blksize))
				{
					m_xferblk = blocklen;
					m_data[0] = 0x00;
				}
				else
				{
					m_data[0] = 0x40; // parameter error
				}
			}
			else
			{
				m_data[0] = 0xff; // show an error
			}
			send_data(1, SD_STATE_TRAN);
			break;

		case 17: // CMD17 - READ_SINGLE_BLOCK
			if (m_image->exists())
			{
				m_data[0] = 0x00; // initial R1 response
				// data token occurs some time after the R1 response.  A2SD expects at least 1
				// byte of space between R1 and the data packet.
				m_data[1] = 0xff;
				m_data[2] = 0xfe; // data token
				u32 blk = get_u32be(&m_cmd[1]);
				if ((m_type == SD_TYPE_V2) && ((blk / m_blksize) != ((blk + (m_xferblk - 1)) / m_blksize)))
				{
					LOG("rejecting read of %u bytes at %u that crosses %u-byte block boundary\n", m_xferblk, blk, m_blksize);
					m_data[0] = 0x40; // parameter error
					send_data(1, SD_STATE_TRAN);
				}
				else if (m_xferblk == m_blksize)
				{
					// optimise for reading an entire block
					if (m_type == SD_TYPE_V2)
					{
						blk /= m_blksize;
					}
					m_image->read(blk, &m_data[3]);
					util::crc16_t crc16 = util::crc16_creator::simple(&m_data[3], m_xferblk);
					put_u16be(&m_data[m_xferblk + 3], crc16);
					LOG("reading LBA %x: [0] %02x %02x .. [%d] %02x %02x [crc16] %04x\n", blk, m_data[3], m_data[4], m_xferblk - 2, m_data[m_xferblk + 1], m_data[m_xferblk + 2], crc16);
					send_data(3 + m_xferblk + 2, SD_STATE_DATA);
				}
				else
				{
					assert(m_type == SD_TYPE_V2);
					m_image->read(blk / m_blksize, &m_sectorbuf[0]);
					std::copy_n(&m_sectorbuf[blk % m_blksize], m_xferblk, &m_data[3]);
					util::crc16_t crc16 = util::crc16_creator::simple(&m_data[3], m_xferblk);
					put_u16be(&m_data[m_xferblk + 3], crc16);
					LOG("reading LBA %x+%x: [0] %02x %02x .. [%d] %02x %02x [crc16] %04x\n", blk / m_blksize, blk % m_blksize, m_data[3], m_data[4], m_xferblk - 2, m_data[m_xferblk + 1], m_data[m_xferblk + 2], crc16);
					send_data(3 + m_xferblk + 2, SD_STATE_DATA);
				}
			}
			else
			{
				m_data[0] = 0xff; // show an error
				send_data(1, SD_STATE_TRAN);
			}
			break;

		case 18: // CMD18 - CMD_READ_MULTIPLE_BLOCK
			if (m_image->exists())
			{
				if (m_xferblk == m_blksize)
				{
					m_data[0] = 0x00; // initial R1 response
					// data token occurs some time after the R1 response.  A2SD
					// expects at least 1 byte of space between R1 and the data
					// packet.
					m_blknext = get_u32be(&m_cmd[1]);
					if (m_type == SD_TYPE_V2)
					{
						m_blknext /= m_xferblk;
					}
					send_data(1, SD_STATE_DATA_MULTI);
				}
				else
				{
					// FIXME: support multi-block read when transfer size is smaller than block size
					m_data[0] = 0x40; // parameter error
					send_data(1, SD_STATE_TRAN);
				}
			}
			else
			{
				m_data[0] = 0xff; // show an error
				send_data(1, SD_STATE_TRAN);
			}
			break;

		case 24: // CMD24 - WRITE_BLOCK
			if (m_xferblk != m_blksize)
			{
				// partial block write not supported
				LOG("rejecting write of %u bytes that is not a full %u-byte block\n", m_xferblk, m_blksize);
				m_data[0] = 0x40; // parameter error
				send_data(1, SD_STATE_TRAN);
			}
			else
			{
				m_blknext = get_u32be(&m_cmd[1]);
				if ((m_type == SD_TYPE_V2) && (m_blknext % m_blksize))
				{
					// misaligned write not supported
					LOG("rejecting write of %u bytes at %u that crosses %u-byte block boundary\n", m_xferblk, m_blknext, m_blksize);
					m_data[0] = 0x40; // parameter error
					send_data(1, SD_STATE_TRAN);
				}
				else
				{
					if (m_type == SD_TYPE_V2)
					{
						m_blknext /= m_xferblk;
					}
					m_data[0] = 0;
					send_data(1, SD_STATE_WRITE_WAITFE);
				}
			}
			break;

		case 41:
			if (m_bACMD) // ACMD41 - SD_SEND_OP_COND
			{
				m_data[0] = 0;
				send_data(1, SD_STATE_READY); // + SD_STATE_IDLE
			}
			else // CMD41 - illegal
			{
				m_data[0] = 0xff;
				send_data(1, SD_STATE_INA);
			}
			break;

		case 55: // CMD55 - APP_CMD
			m_data[0] = 0x01;
			send_data(1, SD_STATE_IDLE);
			break;

		case 58: // CMD58 - READ_OCR
			m_data[0] = 0;
			m_data[1] = 0x80; // Busy Status: 1b - Initialization Complete
			m_data[1] |= (m_type == SD_TYPE_V2) ? 0 : 0x40; // Card Capacity Status: 0b - SDCS, 1b SDHC, SDXC
			m_data[2] = 0;
			m_data[3] = 0;
			m_data[4] = 0;
			send_data(5, SD_STATE_DATA);
			break;

		case 59: // CMD59 - CRC_ON_OFF
			m_data[0] = 0;
			// TODO CRC 1-on, 0-off
			send_data(1, SD_STATE_STBY);
			break;

		default:
			LOGMASKED(LOG_COMMAND, "SDCARD: Unsupported CMD%02d\n", m_cmd[0] & 0x3f);
			clean_cmd = false;
			break;
		}

		// if this is command 55, that's a prefix indicating the next command is an "app command" or "ACMD"
		m_bACMD = (m_cmd[0] & 0x3f) == 55;

		if (clean_cmd)
			memset(m_cmd, 0xff, 6);
	}
}

void spi_sdcard_device::change_state(sd_state new_state)
{
	// TODO validate if transition is valid using refs below.
	// REF Figure 4-13:SD Memory Card State Diagram (Transition Mode)
	// REF Table 4-35:Card State Transition Table
	m_state = new_state;
}
