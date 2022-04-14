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

    Refrences:
    https://www.sdcard.org/downloads/pls/ (Physical Layer Simplified Specification)
    REF: tags are refering to the spec form above. 'Physical Layer Simplified Specification v8.00'

    http://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf
    https://embdev.net/attachment/39390/TOSHIBA_SD_Card_Specification.pdf
    http://elm-chan.org/docs/mmc/mmc_e.html
*/

#include "emu.h"
#include "spi_sdcard.h"
#include "imagedev/harddriv.h"

#define LOG_GENERAL (1U << 0)
#define LOG_COMMAND (1U << 1)
#define LOG_SPI     (1U << 2)

//#define VERBOSE (LOG_COMMAND)
#define LOG_OUTPUT_FUNC osd_printf_info

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
	m_harddisk(nullptr),
	m_ss(0), m_in_bit(0),
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
	write_miso.resolve_safe();
	save_item(NAME(m_state));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_out_ptr));
	save_item(NAME(m_out_count));
	save_item(NAME(m_ss));
	save_item(NAME(m_in_bit));
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
	m_harddisk = m_image->get_hard_disk_file();
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
	// only respond if selected
	if (m_ss)
	{
		// We implmement SPI Mode 3 signalling, in which we latch the data on
		// rising clock edges, and shift the data on falling clock edges.
		// See http://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf for details
		// on the 4 SPI signalling modes.  SD Cards can work in ether Mode 0 or Mode 3,
		// both of which shift on the falling edge and latch on the rising edge but
		// have opposite CLK polarity.

		if (state)
		{
			m_in_latch &= ~0x01;
			m_in_latch |= m_in_bit;
			LOGMASKED(LOG_SPI, "\tsdcard: L %02x (%d) (out %02x)\n", m_in_latch, m_cur_bit, m_out_latch);
			m_cur_bit++;
			if (m_cur_bit == 8)
			{
				LOGMASKED(LOG_SPI, "SDCARD: got %02x\n", m_in_latch);
				for (u8 i = 0; i < 5; i++)
				{
					m_cmd[i] = m_cmd[i + 1];
				}
				m_cmd[5] = m_in_latch;

				switch (m_state)
				{
				case SD_STATE_IDLE:
					do_command();
					break;

				case SD_STATE_WRITE_WAITFE:
					if (m_in_latch == 0xfe)
					{
						m_state = SD_STATE_WRITE_DATA;
						m_out_latch = 0xff;
						m_write_ptr = 0;
						change_state(SD_STATE_RCV);
					}
					break;

				case SD_STATE_WRITE_DATA:
					m_data[m_write_ptr++] = m_in_latch;
					if (m_write_ptr == (m_blksize + 2))
					{
						u32 blk = (u32(m_cmd[1]) << 24) | (u32(m_cmd[2]) << 16) | (u32(m_cmd[3]) << 8) | u32(m_cmd[4]);
						if (m_type == SD_TYPE_V2)
						{
							blk /= m_blksize;
						}

						LOGMASKED(LOG_GENERAL, "writing LBA %x, data %02x %02x %02x %02x\n", blk, m_data[0], m_data[1], m_data[2], m_data[3]);
						if (m_harddisk->write(blk, &m_data[0]))
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
					break;

				case SD_STATE_DATA_MULTI:
					do_command();
					if (m_state == SD_STATE_DATA_MULTI && m_out_count == 0)
					{
						m_data[0] = 0xfe; // data token
						m_harddisk->read(m_blknext++, &m_data[1]);
						util::crc16_t crc16 = util::crc16_creator::simple(
							&m_data[1], m_blksize);
						m_data[m_blksize + 1] = (crc16 >> 8) & 0xff;
						m_data[m_blksize + 2] = (crc16 & 0xff);
						send_data(1 + m_blksize + 2, SD_STATE_DATA_MULTI);
					}
					break;

				default:
					if (((m_cmd[0] & 0x70) == 0x40) || (m_out_count == 0)) // CMD0 - GO_IDLE_STATE
					{
						do_command();
					}
					break;
				}
			}
		}
		else
		{
			m_in_latch <<= 1;
			m_out_latch <<= 1;
			m_out_latch |= 1;
			LOGMASKED(LOG_SPI, "\tsdcard: S %02x %02x (%d)\n", m_in_latch,
					  m_out_latch, m_cur_bit);

			m_cur_bit &= 0x07;
			if (m_cur_bit == 0)
			{
				if (m_out_count > 0)
				{
					m_out_latch = m_data[m_out_ptr++];
					LOGMASKED(LOG_SPI, "SDCARD: latching %02x (start of shift)\n", m_out_latch);
					m_out_count--;
				}
			}
			write_miso(BIT(m_out_latch, 7));
		}
	}
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
			if (m_harddisk)
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

		case 8:                   // CMD8 - SEND_IF_COND (SD v2 only)
			m_data[0] = 0x01;
			m_data[1] = 0;
			m_data[2] = 0;
			m_data[3] = 0x01;
			m_data[4] = 0xaa;
			send_data(5, SD_STATE_IDLE);
			break;

		case 10:              // CMD10 - SEND_CID
			m_data[0] = 0x01; // initial R1 response
			m_data[1] = 0x00; // throwaway byte before data transfer
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
				m_data[12] = (uSerial >> 24) & 0xff; // PSN - Product Serial Number
				m_data[13] = (uSerial >> 16) & 0xff;
				m_data[14] = (uSerial >> 8) & 0xff;
				m_data[15] = (uSerial & 0xff);
			}
			m_data[16] = 0x01; // MDT - Manufacturing Date
			m_data[17] = 0x59; // 0x15 9 = 2021, September
			m_data[18] = 0x00; // CRC7, bit 0 is always 0
			{
				util::crc16_t crc16 = util::crc16_creator::simple(&m_data[3], 16);
				m_data[19] = (crc16 >> 8) & 0xff;
				m_data[20] = (crc16 & 0xff);
			}
			send_data(3 + 16 + 2, SD_STATE_STBY);
			break;

		case 12: // CMD12 - STOP_TRANSMISSION
			m_data[0] = 0;
			send_data(1,
					  m_state == SD_STATE_RCV ? SD_STATE_PRG : SD_STATE_TRAN);
			break;

		case 16: // CMD16 - SET_BLOCKLEN
			m_blksize = (u16(m_cmd[3]) << 8) | u16(m_cmd[4]);
			if (m_harddisk->set_block_size(m_blksize))
			{
				m_data[0] = 0;
			}
			else
			{
				m_data[0] = 0xff; // indicate an error
				// if false was returned, it means the hard disk is a CHD file, and we can't resize the
				// blocks on CHD files.
				logerror("spi_sdcard: Couldn't change block size to %d, wrong "
						 "CHD file?",
						 m_blksize);
			}
			send_data(1, SD_STATE_TRAN);
			break;

		case 17: // CMD17 - READ_SINGLE_BLOCK
			if (m_harddisk)
			{
				m_data[0] = 0x00; // initial R1 response
				// data token occurs some time after the R1 response.  A2SD expects at least 1
				// byte of space between R1 and the data packet.
				m_data[1] = 0xff;
				m_data[2] = 0xfe; // data token
				u32 blk = (u32(m_cmd[1]) << 24) | (u32(m_cmd[2]) << 16) | (u32(m_cmd[3]) << 8) | u32(m_cmd[4]);
				if (m_type == SD_TYPE_V2)
				{
					blk /= m_blksize;
				}
				LOGMASKED(LOG_GENERAL, "reading LBA %x\n", blk);
				m_harddisk->read(blk, &m_data[3]);
				{
					util::crc16_t crc16 = util::crc16_creator::simple(&m_data[3], m_blksize);
					m_data[m_blksize + 3] = (crc16 >> 8) & 0xff;
					m_data[m_blksize + 4] = (crc16 & 0xff);
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
			if (m_harddisk)
			{
				m_data[0] = 0x00; // initial R1 response
				// data token occurs some time after the R1 response.  A2SD
				// expects at least 1 byte of space between R1 and the data
				// packet.
				m_blknext = (u32(m_cmd[1]) << 24) | (u32(m_cmd[2]) << 16) | (u32(m_cmd[3]) << 8) | u32(m_cmd[4]);
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
			send_data(1, SD_STATE_RCV);
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
			if (m_type == SD_TYPE_HC)
			{
				m_data[1] = 0x40; // indicate SDHC support
			}
			else
			{
				m_data[1] = 0;
			}
			m_data[2] = 0;
			m_data[3] = 0;
			m_data[4] = 0;
			send_data(5, SD_STATE_DATA);
			break;

		default:
			LOGMASKED(LOG_COMMAND, "SDCARD: Unsupported %02x\n", m_cmd[0] & 0x3f);
			clean_cmd = false;
			break;
		}

		// if this is command 55, that's a prefix indicating the next command is an "app command" or "ACMD"
		if ((m_cmd[0] & 0x3f) == 55)
		{
			m_bACMD = true;
		}
		else
		{
			m_bACMD = false;
		}

		if (clean_cmd)
		{
			for (u8 i = 0; i < 6; i++)
			{
				m_cmd[i] = 0xff;
			}
		}
	}
}

void spi_sdcard_device::change_state(sd_state new_state)
{
	// TODO validate if transition is valid using refs below.
	// REF Figure 4-13:SD Memory Card State Diagram (Transition Mode)
	// REF Table 4-35:Card State Transition Table
	m_state = new_state;
}
