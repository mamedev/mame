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

#define LOG_COMMAND (1U << 1)
#define LOG_SPI     (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_COMMAND)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

static constexpr u8 DATA_RESPONSE_OK        = 0x05;
static constexpr u8 DATA_RESPONSE_IO_ERROR  = 0x0d;

DEFINE_DEVICE_TYPE(SPI_SDCARD, spi_sdcard_sdhc_device, "spi_sdhccard", "SDHC Card (SPI Interface)")
DEFINE_DEVICE_TYPE(SPI_SDCARDV2, spi_sdcard_sdv2_device, "spi_sdv2card", "SDV2 Card (SPI Interface)")

spi_sdcard_device::spi_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	write_miso(*this),
	m_image(*this, "image"),
	m_state(SD_STATE_IDLE),
	m_ss(0), m_in_bit(0), m_clk_state(0),
	m_in_latch(0), m_out_latch(0xff), m_cur_bit(0),
	m_out_count(0), m_out_ptr(0), m_write_ptr(0), m_blksize(512), m_blknext(0),
	m_bACMD(false)
{
}

spi_sdcard_sdv2_device::spi_sdcard_sdv2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spi_sdcard_device(mconfig, SPI_SDCARDV2, tag, owner, clock)
{
	m_type = SD_TYPE_V2;
}

spi_sdcard_sdhc_device::spi_sdcard_sdhc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spi_sdcard_device(mconfig, SPI_SDCARD, tag, owner, clock)
{
	m_type = SD_TYPE_HC;
}

ALLOW_SAVE_TYPE(spi_sdcard_device::sd_state);
ALLOW_SAVE_TYPE(spi_sdcard_device::sd_type);

void spi_sdcard_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_out_ptr));
	save_item(NAME(m_out_count));
	save_item(NAME(m_ss));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_cur_bit));
	save_item(NAME(m_write_ptr));
	save_item(NAME(m_blksize));
	save_item(NAME(m_blknext));
	save_item(NAME(m_type));
	save_item(NAME(m_cmd));
	save_item(NAME(m_data));
	save_item(NAME(m_bACMD));
}

void spi_sdcard_device::device_reset()
{
}

void spi_sdcard_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, m_image).set_interface("spi_sdcard");
}

void spi_sdcard_device::send_data(u16 count, sd_state new_state)
{
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
			if (m_write_ptr == (m_blksize + 2))
			{
				LOG("writing LBA %x, data %02x %02x %02x %02x\n", m_blknext, m_data[0], m_data[1], m_data[2], m_data[3]);
				if (m_image->write(m_blknext, &m_data[0]))
				{
					m_data[0] = DATA_RESPONSE_OK;
				}
				else
				{
					m_data[0] = DATA_RESPONSE_IO_ERROR;
				}
				m_data[1] = 0x01;

				send_data(2, SD_STATE_IDLE);
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
		if (m_out_ptr < SPI_DELAY_RESPONSE)
		{
			m_out_ptr++;
		}
		else if (m_out_count > 0)
		{
			m_out_latch = m_data[m_out_ptr - SPI_DELAY_RESPONSE];
			m_out_ptr++;
			LOGMASKED(LOG_SPI, "SDCARD: latching %02x (start of shift)\n", m_out_latch);
			m_out_count--;
		}
	}
	write_miso(BIT(m_out_latch, 7));
}

void spi_sdcard_device::do_command()
{
	if (((m_cmd[0] & 0xc0) == 0x40) && (m_cmd[5] & 1))
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

			//if (m_type == SD_TYPE_V2) // CSD Version 1.0
			{
				u8 block_len = 9;
				for (auto i = m_blksize >> 10; i; i >>= 1, ++block_len);

				m_data[3]  = 0x00; // 127: CSD_STRUCTURE:2 (00b) 0:6
				m_data[4]  = 0x0e; // 119: TAAC:8
				m_data[5]  = 0x00; // 111: NSAC:8
				m_data[6]  = 0x32; // 103: TRAN_SPEED:8 (32h or 5Ah)
				m_data[7]  = 0x5b; //  95: CCC:12 (01x110110101b)
				m_data[8]  = 0x50; //      .. READ_BL_LN:4
				m_data[8]  |= block_len;
				m_data[9]  = 0x83; //  79: READ_BL_PARTIAL:1 (1b) WRITE_BLK_MISALIGN:1 READ_BLK_MISALIGN:1 DSR_IMP:1 0:2 C_SIZE:12
				m_data[10] = 0xff; //      ..
				m_data[11] = 0xed; //      .. VDD_R_CURR_MIN:3 VDD_R_CURR_MAX:3
				m_data[12] = 0xb7; //  55: VDD_W_CURR_MIN:3 VDD_W_CURR_MAX:3 C_SIZE_MUL:3
				m_data[13] = 0xbf; //      .. ERASE_BLK_EN:1 SECTOR_SIZE:7
				m_data[14] = 0xbf; //      .. WP_GRP_SIZE:7
				m_data[15] = 0x04; //  31: WP_GRP_ENABLE:1 0:2 R2W_FACTOR:3 WRITE_BL_LEN:4
				m_data[15] |= (block_len >> 2);
				m_data[16] = 0x00; ///     .. WRITE_BL_PARTIAL:1 0:5
				m_data[16] |= (block_len & 3) << 6;
				m_data[17] = 0x00; //  15: FILE_FORMAT_GRP:1 COPY:1 PERM_WRITE_PROTECT:1 TMP_WRITE_PROTECT:1 FILE_FORMAT:2 WP_UPC:1 0:1
				m_data[18] = 0x01; //   7: CRC7 1:1
			}
			/*
			else // SD_TYPE_HC: CSD Version 2.0
			{
			    m_data[3]  = 0x40;
			}
			*/

			send_data(3 + 16, SD_STATE_STBY);
			break;

		case 10: // CMD10 - SEND_CID
			m_data[0] = 0x00; // initial R1 response
			m_data[1] = 0xff; // throwaway byte before data transfer
			m_data[2] = 0xfe; // data token
			m_data[3] = 'M';  // Manufacturer ID - we'll use M for MAME
			m_data[4] = 'M';  // OEM ID - MD for MAMEdev
			m_data[5] = 'D';
			m_data[6] = 'M'; // Product Name - "MCARD"
			m_data[7] = 'C';
			m_data[8] = 'A';
			m_data[9] = 'R';
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
			m_blksize = get_u16be(&m_cmd[3]);
			if (m_image->exists() && m_image->set_block_size(m_blksize))
			{
				m_data[0] = 0;
			}
			else
			{
				m_data[0] = 0xff; // indicate an error
				// if false was returned, it means the hard disk is a CHD file, and we can't resize the
				// blocks on CHD files.
				logerror("spi_sdcard: Couldn't change block size to %d, wrong CHD file?", m_blksize);
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
				if (m_type == SD_TYPE_V2)
				{
					blk /= m_blksize;
				}
				m_image->read(blk, &m_data[3]);
				{
					util::crc16_t crc16 = util::crc16_creator::simple(&m_data[3], m_blksize);
					put_u16be(&m_data[m_blksize + 3], crc16);
					LOG("reading LBA %x: [0] %02x %02x .. [%d] %02x %02x [crc16] %04x\n", blk, m_data[3], m_data[4], m_blksize - 2, m_data[m_blksize + 1], m_data[m_blksize + 2], crc16);
				}
				send_data(3 + m_blksize + 2, SD_STATE_DATA);
			}
			else
			{
				m_data[0] = 0xff; // show an error
				send_data(1, SD_STATE_DATA);
			}
			break;

		case 18: // CMD18 - CMD_READ_MULTIPLE_BLOCK
			if (m_image->exists())
			{
				m_data[0] = 0x00; // initial R1 response
				// data token occurs some time after the R1 response.  A2SD
				// expects at least 1 byte of space between R1 and the data
				// packet.
				m_blknext = get_u32be(&m_cmd[1]);
				if (m_type == SD_TYPE_V2)
				{
					m_blknext /= m_blksize;
				}
			}
			else
			{
				m_data[0] = 0xff; // show an error
			}
			send_data(1, SD_STATE_DATA_MULTI);
			break;

		case 24: // CMD24 - WRITE_BLOCK
			m_data[0] = 0;
			m_blknext = get_u32be(&m_cmd[1]);
			if (m_type == SD_TYPE_V2)
			{
				m_blknext /= m_blksize;
			}
			send_data(1, SD_STATE_WRITE_WAITFE);
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
