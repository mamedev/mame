// license:BSD-3-Clause
// copyright-holders:smf, Angelo Salese
/***************************************************************************

 gdrom.c - Implementation of the Sega GD-ROM device

***************************************************************************/

#include "emu.h"
#include "gdrom.h"
#include "coreutil.h"

#include <iostream>

#define LOG_WARN    (1U << 1)
#define LOG_CMD     (1U << 2)
#define LOG_CMD_RAW (1U << 3) // bare command IDs (temp until all commands are in)
#define LOG_XFER    (1U << 4)
#define LOG_TOC     (1U << 5) // TOC frame offsets

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_CMD | LOG_TOC)
//#define LOG_OUTPUT_STREAM std::cout
#include "logmacro.h"

#define LOGWARN(...)      LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGCMD(...)       LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGCMDRAW(...)    LOGMASKED(LOG_CMD_RAW, __VA_ARGS__)
#define LOGXFER(...)      LOGMASKED(LOG_XFER, __VA_ARGS__)
#define LOGTOC(...)       LOGMASKED(LOG_TOC, __VA_ARGS__)

#define GDROM_BUSY_STATE    0x00
#define GDROM_PAUSE_STATE   0x01
#define GDROM_STANDBY_STATE 0x02
#define GDROM_PLAY_STATE    0x03
#define GDROM_SEEK_STATE    0x04
#define GDROM_SCAN_STATE    0x05
#define GDROM_OPEN_STATE    0x06
#define GDROM_NODISC_STATE  0x07
#define GDROM_RETRY_STATE   0x08
#define GDROM_ERROR_STATE   0x09

// CD status readback/subchannel Q
#define LIVE_REQ_STAT 0

/*
 Officially not documented security-related packet commands:

 SYS_CHK_SECU (70h) Media security check
    Parameters: 1byte - R0011111, R - "recheck" (it seems actual security check performed automatically at drive power on or when disc was inserted, so normally this and next command returns result of already performed check.
                                      however, when MSB R bit is 1 will be forced media security recheck)
    Result: none
 SYS_REQ_SECU (71h) Request security data
    Parameters: 1byte - always 0x1f
    Result: a bit less than 1Kbyte chunk of data (length vary each time), contains real command reply obfuscated, which is extracted by such code:

   u8 reply[], real_reply[];
   for (u32 i = 0, offset = 0; i < length; i++)
   {
     offset++;
     u32 skip = reply[offset] - 2; // normally skip value is < 0x10, might be used to identify real reply length
     offset += skip;
     real_reply[i] = reply[offset++];
   }

   Real reply is 43 byte:
    struct sec_reply {
        u8 check_result;       // 0x1f - valid GD-ROM, 0x10 - valid Mil-CD (in this case following char[] fields is empty, 0x00-filled)
        char key_id[10];       // presumable Disc ID (T-xxxxxx or HDR-xxxxx etc)
        char key_maker_id[16]; // presumable "SEGA ENTERPRISES"
        char hard_id[16];      // presumable "SEGA SEGAKATANA "
    };

    *_id fields names came from Dev.box "Checker BIOS" disassembly, contents meaning is guesswork because all the reply dumps we have now was dumped with Mil-CD disc inserted but not GD-ROM.
    Presumable these data somehow encoded in GD-ROM disc HD area Lead-in (or Security Ring area ?), and compared with data in LD area IP.BIN by GD-drive firmware,
    as described in Sega patent EP0935242A1 https://patents.google.com/patent/EP0935242A1

    Dreamcast BIOS code verify only 1st result byte, if it's 5th bit (0x10) == 1.
    Naomi DIMM firmware verify if result byte equal to 0x1f.

 SYS_CHG_COMD (72h) ??? Authentication for next command ?
    Parameters: 1byte, probably key/password, in retail Dreamcast - 5th byte of unit SN# (located in flash ROM at 1A05Ah), 0 in Dev.box checker BIOS.
    Result: none

 SYS_REQ_COMD (73h) Request command list
    Parameters: none
    Result: chunk of data where obfuscated real reply, see command 71.
            real result: 14 bytes - codes of all regular (not security) packet commands supported by drive (00 10 11 12 13 14 15 16 20 21 22 30 31 40).

  Dreamcast BIOS SysCalls contain commands 0x72/73 routine, but it seems not used at practice.
*/

static const uint8_t GDROM_Cmd71_Reply[] =
{
	0x96, 0x0B, 0x45, 0xF0, 0x7E, 0xFF, 0x3D, 0x06, 0x4D, 0x7D, 0x10, 0xBF, 0x07, 0x00, 0x73, 0xCF,
	0x9C, 0x00, 0xBC, 0x0C, 0x1C, 0xAF, 0x1C, 0x30, 0xE7, 0xA7, 0x03, 0xA8, 0x98, 0x00, 0xBD, 0x0F,
	0xBD, 0x5B, 0xAA, 0x50, 0x23, 0x39, 0x31, 0x10, 0x0E, 0x69, 0x13, 0xE5, 0x00, 0xD2, 0x0D, 0x66,
	0x54, 0xBF, 0x5F, 0xFD, 0x37, 0x74, 0xF4, 0x5B, 0x22, 0x00, 0xC6, 0x09, 0x0F, 0xCA, 0x93, 0xE8,
	0xA4, 0xAB, 0x00, 0x61, 0x0E, 0x2E, 0xE1, 0x4B, 0x76, 0x8B, 0x6A, 0xA5, 0x9C, 0xE6, 0x23, 0xC4,
	0x00, 0x4B, 0x06, 0x1B, 0x91, 0x01, 0x00, 0xE2, 0x0D, 0xCF, 0xCA, 0x38, 0x3A, 0xB9, 0xE7, 0x91,
	0xE5, 0xEF, 0x4B, 0x00, 0xD6, 0x09, 0xD3, 0x68, 0x3E, 0xC4, 0xAF, 0x2D, 0x00, 0x2A, 0x0D, 0xF9,
	0xFC, 0x78, 0xED, 0xAE, 0x99, 0xB3, 0x32, 0x5A, 0xE7, 0x00, 0x4C, 0x0A, 0x22, 0x97, 0x5B, 0x82,
	0x06, 0x7A, 0x4C, 0x00, 0x42, 0x0E, 0x57, 0x78, 0x46, 0xF5, 0x20, 0xFC, 0x6B, 0xCB, 0x01, 0x5B,
	0x86, 0x00, 0xE4, 0x0E, 0xB2, 0x26, 0xCD, 0x71, 0xE3, 0xA5, 0x33, 0x06, 0x8E, 0x9A, 0x50, 0x00,
	0x07, 0x07, 0xF5, 0x34, 0xEF, 0xE6, 0x00, 0x32, 0x0F, 0x13, 0x41, 0x59, 0x56, 0x0F, 0x02, 0x38,
	0x2A, 0x64, 0x2A, 0x07, 0x3E, 0x00, 0x52, 0x11, 0x2A, 0x1D, 0x5F, 0x76, 0x66, 0xA0, 0xB2, 0x2F,
	0x97, 0xC7, 0x5E, 0x6E, 0x52, 0xE2, 0x00, 0x58, 0x09, 0xCA, 0x89, 0xA5, 0xDF, 0x0A, 0xDE, 0x00,
	0x50, 0x06, 0x49, 0xB8, 0xB4, 0x00, 0x77, 0x05, 0x24, 0xE8, 0x00, 0xBB, 0x0C, 0x91, 0x89, 0xA2,
	0x8B, 0x62, 0xDE, 0x6A, 0xC6, 0x60, 0x00, 0xE7, 0x0F, 0x0F, 0x11, 0x96, 0x55, 0xD2, 0xBF, 0xE6,
	0x48, 0x0B, 0x5C, 0xAB, 0xDC, 0x00, 0xBA, 0x0A, 0x30, 0xD7, 0x48, 0x0E, 0x78, 0x63, 0x0C, 0x00,
	0xD2, 0x0D, 0xFB, 0x8A, 0xA3, 0xFE, 0xF8, 0x3A, 0xDD, 0x88, 0xA9, 0x4B, 0x00, 0xA2, 0x0A, 0x75,
	0x5D, 0x0D, 0x37, 0x24, 0xC5, 0x9D, 0x00, 0xF7, 0x0B, 0x25, 0xEF, 0xDB, 0x41, 0xE0, 0x52, 0x3E,
	0x4E, 0x00, 0xB7, 0x03, 0x00, 0xE5, 0x11, 0xB9, 0xDE, 0x5A, 0x57, 0xCF, 0xB9, 0x1A, 0xFC, 0x7F,
	0x26, 0xEE, 0x7B, 0xCD, 0x2B, 0x00, 0x4B, 0x08, 0xB8, 0x09, 0x70, 0x6A, 0x9F, 0x00, 0x4B, 0x11,
	0x8C, 0x15, 0x87, 0xA3, 0x05, 0x4F, 0x37, 0x8E, 0x63, 0xDE, 0xEF, 0x39, 0xFC, 0x4B, 0x00, 0xAB,
	0x10, 0x0B, 0x91, 0xAA, 0x0F, 0xE1, 0xE9, 0xAE, 0x69, 0x3A, 0xF8, 0x03, 0x69, 0xD2, 0x00, 0xE2,
	0x07, 0xC1, 0x5C, 0x3D, 0x82, 0x00, 0xA9, 0x08, 0x68, 0xC4, 0xAD, 0x2E, 0xD1, 0x00, 0xF7, 0x0E,
	0xC6, 0x47, 0xC8, 0xCD, 0x8E, 0x7C, 0x00, 0x5C, 0x95, 0xB9, 0xF4, 0x00, 0xE3, 0x04, 0x5B, 0x00,
	0x74, 0x07, 0x65, 0xC7, 0x84, 0x8E, 0x00, 0xC6, 0x07, 0x61, 0x80, 0x44, 0x3F, 0x00, 0xC8, 0x0E,
	0x72, 0x78, 0x47, 0xD3, 0xC2, 0x4D, 0xAF, 0xC0, 0x54, 0x13, 0x31, 0x00, 0xF7, 0x0D, 0x48, 0xD8,
	0xE2, 0x92, 0x9F, 0x7F, 0x2F, 0x44, 0x68, 0x33, 0x00, 0x0D, 0x10, 0xAB, 0xFE, 0xEA, 0x8E, 0x19,
	0x81, 0xF8, 0x6F, 0x7C, 0xDE, 0xE1, 0xB3, 0x06, 0x00, 0x4D, 0x11, 0x66, 0xAE, 0x4C, 0xF9, 0xB7,
	0x2F, 0xEE, 0xB0, 0x8E, 0x7E, 0xE1, 0x8D, 0x95, 0x6F, 0x00, 0xF4, 0x0D, 0x88, 0x9D, 0xCA, 0xE3,
	0xC4, 0xB2, 0x47, 0xBB, 0xA0, 0x69, 0x00, 0xF3, 0x0B, 0x48, 0x17, 0x41, 0x64, 0xA0, 0x0E, 0x71,
	0x82, 0x00, 0x34, 0x1E, 0x18, 0x4D, 0x85, 0x80, 0x4C, 0xA9, 0x0B, 0x66, 0x9B, 0x75, 0x13, 0x61,
	0x70, 0x27, 0x81, 0x7A, 0x02, 0xCD, 0x57, 0xAB, 0xDF, 0x02, 0x93, 0x52, 0x83, 0xDF, 0x48, 0xA8,
	0xA6, 0x9E, 0x74, 0x6F, 0x89, 0x03, 0x28, 0x25, 0x52, 0x96, 0xFF, 0x67, 0x7A, 0xD8, 0x3C, 0xB1,
	0x2C, 0x46, 0x84, 0xEF, 0xE1, 0xC1, 0xC6, 0xC9, 0xDC, 0x96, 0xAA, 0xA9, 0xC4, 0x82, 0x58, 0x27,
	0x57, 0x75, 0x67, 0x34, 0xFB, 0x3B, 0x25, 0xBF, 0xFB, 0x3B, 0xF6, 0x13, 0xEC, 0x96, 0xE5, 0x16,
	0x26, 0xFD, 0xA8, 0xDA, 0x1B, 0xC6, 0x50, 0x7F, 0x47, 0xFF, 0x08, 0x55, 0x08, 0xED, 0x00, 0x93,
	0x9B, 0xC4, 0x71, 0x67, 0xEC, 0xA6, 0xCC, 0x16, 0x20, 0x87, 0x47, 0x07, 0xA6, 0x00, 0x79, 0x5D,
	0x4F, 0xAB, 0xA1, 0x6F, 0x7A, 0x6B, 0x27, 0xC4, 0xDA, 0xA3, 0xC3, 0x94, 0x4F, 0x7F, 0xF3, 0xE5,
	0x1B, 0x6F, 0xCC, 0xE5, 0xF0, 0xE5, 0x9D, 0xC9, 0xAE, 0xFD, 0x39, 0xAC, 0x4C, 0xE5, 0x58, 0x83,
	0x25, 0x65, 0x92, 0x74, 0x9E, 0x81, 0xA0, 0xB6, 0xA9, 0x02, 0x9B, 0x07, 0xB6, 0xE7, 0x79, 0x57,
	0xD9, 0x4A, 0xCE, 0xFA, 0xB4, 0x94, 0x05, 0xCC, 0x86, 0x3C, 0xDD, 0x06, 0xCD, 0xA6, 0x24, 0x24,
	0xFA, 0xC1, 0xF9, 0x48, 0xC9, 0x0C, 0x6C, 0xC4, 0x96, 0x82, 0x17, 0xF6, 0x31, 0x09, 0xC4, 0xE2,
	0x77, 0xFD, 0xCF, 0x46, 0x18, 0xB2, 0x5F, 0x01, 0x6B, 0xD1, 0x7B, 0x56, 0xB8, 0x94, 0x4A, 0xE5,
	0x6C, 0x19, 0xF0, 0xC0, 0xB6, 0x70, 0x93, 0xF7, 0xD3, 0xD1, 0x2B, 0x6E, 0x7C, 0x53, 0x6D, 0x85,
	0xD1, 0x0C, 0x8B, 0x77, 0xEE, 0x90, 0xDA, 0x15, 0x55, 0xE0, 0x58, 0x09, 0x56, 0xFC, 0x31, 0x9F,
	0xAF, 0x46, 0xCB, 0xC3, 0x8D, 0x71, 0x75, 0xF2, 0x2C, 0xC3, 0xBB, 0xA1, 0xC4, 0xCF, 0x27, 0x56,
	0x7C, 0x9B, 0xFE, 0xAF, 0x3E, 0x4E, 0xB4, 0xCD, 0x6A, 0xAA, 0xF5, 0xF3, 0xE3, 0x22, 0x82, 0xE1,
	0xA5, 0x68, 0xB3, 0xDB, 0x8F, 0x9E, 0x5E, 0x7B, 0x90, 0xF0, 0x79, 0x3F, 0x52, 0x8C, 0x61, 0x88,
	0x76, 0xAE, 0x14, 0x63, 0x19, 0x0F, 0x1D, 0xCE, 0xA1, 0x63, 0x10, 0xB2, 0xE2, 0xD7, 0x94, 0xB1,
	0x33, 0xCB, 0x28, 0x85, 0x7D, 0x9B, 0xF5, 0xF4, 0x25, 0x50, 0x9B, 0xDB, 0x35, 0xA5, 0xB0, 0x9C,
	0x09, 0x92, 0xE3, 0x31, 0x40, 0xAB, 0x4D, 0xF4, 0x35, 0xE8, 0xB3, 0x0A, 0x21, 0xC3, 0x86, 0x9C,
	0xCB, 0x29, 0xA4, 0x77, 0x57, 0xBC, 0xD8, 0xDA, 0xA5, 0x82, 0x80, 0xE8, 0xCF, 0x72, 0x81, 0xAD,
	0x2E, 0x28, 0xFF, 0xD8, 0xB6, 0xD1, 0x2B, 0x97, 0x00, 0xFF, 0xE1, 0x06, 0x44, 0x39, 0x1C, 0x4B,
	0xAB, 0x19, 0x5B, 0x4D, 0xD6, 0x3E, 0x1B, 0x5C, 0x64, 0xBB, 0x32, 0x68, 0xF5, 0x7C, 0xC9, 0x9E,
	0xE8, 0xB4, 0x29, 0x1B, 0x7F, 0x4D, 0x80, 0x80, 0x7E, 0x8B, 0x1C, 0x0A, 0xE6, 0x9A, 0xBF, 0x49,
	0x1E, 0xC5, 0xB6, 0x67, 0x7D, 0x05, 0xE4, 0x90, 0x40, 0x4B, 0xAF, 0x9B, 0x52, 0xDE, 0x17, 0x80,
	0x81, 0x56, 0xEA, 0x3A, 0x53, 0x82, 0x8C, 0x62, 0xFB, 0x96, 0x97, 0x6F, 0xC1, 0x16, 0x78, 0xD4,
	0x7B, 0xE7, 0xB9, 0x5A, 0x2A, 0xEB, 0x87, 0x68, 0x33, 0xD3, 0x31, 0x45, 0xFA, 0xFE, 0xF4, 0x1C,
	0x90, 0x86, 0x73, 0x77, 0xD9, 0xA9, 0xD1, 0x4A, 0x4A, 0xCF, 0xAE, 0x23, 0xDB, 0xF9, 0x09, 0xD8,
	0x18, 0xDC, 0x6A, 0x0D, 0xE4, 0x19, 0x8C, 0x65, 0xC6, 0x64, 0xC7, 0xDC, 0xA9, 0xE3, 0x91, 0xB1,
	0x4C, 0xC8, 0xC1, 0x9E, 0x3B, 0x7F, 0xCB, 0xA3, 0xCF, 0xDD, 0xF0, 0x1D, 0x07, 0x6E, 0xDC, 0xCE,
	0x0D, 0xCD, 0x7E, 0x1E, 0x55, 0x11, 0x8B, 0xDF, 0x3A, 0xAB, 0xB6, 0x3B, 0x6E, 0x52, 0x7F, 0xA7,
	0x00, 0xD1, 0x33, 0xBE, 0xF2, 0x9B, 0xFC, 0x4A, 0xCF, 0x9D, 0x8F, 0xC6, 0xC4, 0x7B, 0xDA, 0xE7,
	0x2A, 0x1C, 0x26, 0x6E
};


void gdrom_device::device_reset()
{
	static const uint8_t GDROM_Def_Cmd11_Reply[32] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0x19, 0x00, 0x00, 0x08, 0x53, 0x45, 0x20, 0x20, 0x20, 0x20,
		0x20, 0x20, 0x52, 0x65, 0x76, 0x20, 0x36, 0x2E, 0x34, 0x32, 0x39, 0x39, 0x30, 0x33, 0x31, 0x36
	};

	for(int i = 0;i<32;i++)
		GDROM_Cmd11_Reply[i] = GDROM_Def_Cmd11_Reply[i];

	atapi_cdrom_device::device_reset();
}

// scsicd_exec_command
//
// Execute a SCSI command.

void gdrom_device::ExecCommand()
{
	LOGCMDRAW("%02x\n", command[0]);
	switch ( command[0] )
	{
		case 0x00:
			// TEST_UNIT
			// TODO: verify if t10mmc use is enough
			// loopchk returns OK in Packet cmnd (0201)
			t10mmc::ExecCommand();
			break;

		case 0x10:
		{
			transferOffset = command[2];
			u8 allocation_length = SCSILengthFromUINT8( &command[4] );
			// any game that enables [redbook]
			LOGCMD("REQ_STAT 10h offset %02x length %02x\n", transferOffset, allocation_length);

			if (transferOffset || allocation_length != 0xa)
				throw emu_fatalerror("GDROM: REQ_STAT with unsupported offset %02x length %02x", transferOffset, allocation_length);

			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = allocation_length;
			break;
		}

		case 0x11:
			LOGCMD("REQ_MODE 11h %02x %02x\n", command[2], command[4]);
			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;

//          if (SCSILengthFromUINT8( &command[4] ) < 32) return -1;
			transferOffset = command[2];
			m_transfer_length = SCSILengthFromUINT8( &command[4] );
			if (transferOffset & 1)
				throw emu_fatalerror("GDROM: REQ_MODE with odd offset %02x %02x", transferOffset, m_transfer_length);
			break;

		case 0x12: // SET_MODE
			LOGCMD("SET_MODE 12h %02x %02x\n", command[2], command[4]);

			m_phase = SCSI_PHASE_DATAOUT;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			//transferOffset = command[2];
			m_transfer_length = SCSILengthFromUINT8( &command[4] );
			if (command[2])
				throw emu_fatalerror("GDROM: SET_MODE with offset %02x %02x", transferOffset, m_transfer_length);

			if (m_transfer_length > 0xa)
			{
				LOGWARN("SET_MODE attempt to write on read only regs %02x\n", command[4]);
				m_transfer_length = 0xa;
			}
			break;

		case 0x14:
		{
			if (!m_image->exists())
			{
				m_phase = SCSI_PHASE_STATUS;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_transfer_length = 0;
				break;
			}

			// TODO: it's supposed to write a single and a double density TOC request
			//if (command[1])
			//  throw emu_fatalerror("Double density unsupported");
			u16 allocation_length = SCSILengthFromUINT16( &command[3] );
			LOGCMD("READ_TOC 14h %02x %02x %d\n",
				command[1], command[2], allocation_length
			);

			if (allocation_length != 408)
				throw emu_fatalerror("TOC with allocation length != 408 (%d)", allocation_length);

			if (m_cdda != nullptr)
			{
				m_cdda->stop_audio();
				m_sector_number = (m_sector_number & 0xf0) | GDROM_STANDBY_STATE;
			}

			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = allocation_length;
			break;
		}

		// accessed for audio CDs
		// TODO: needed for Mil CD support
		case 0x15:
		{
			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = SCSILengthFromUINT8( &command[4] );
			LOGCMD("REQ_SES 15h %02x %02x\n", command[2], m_transfer_length);
			break;
		}

		case 0x20: // CD_PLAY
		{
			if (!m_image->exists())
			{
				m_phase = SCSI_PHASE_STATUS;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_transfer_length = 0;
				break;
			}

			const u32 start_offs = (command[2]<<16 | command[3]<<8 | command[4]);
			const u32 end_offs = (command[8]<<16 | command[9]<<8 | command[10]);
			//(command[8] % 75) + ((command[7] * 75) % (60*75)) + (command[6] * (75*60)) - m_lba;

			const u8 play_mode = command[1] & 7;

			auto trk = m_image->get_track(start_offs);
			const u8 repeat = command[6] & 0xf;
			LOGCMD("CD_PLAY 20h track %d FAD %d blocks %d type %02x repeat %01x\n"
				, trk + 1
				, start_offs
				, end_offs - start_offs
				, play_mode
				, repeat
			);

			if (play_mode == 7 && m_audio_sense == SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_PAUSED)
			{
				m_cdda->pause_audio(0);
				m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS;
				LOGCMD("\tPlayback resume\n");
				m_sector_number = (m_sector_number & 0xf0) | GDROM_PLAY_STATE;
			}
			else if (m_image->get_track_type(trk) == cdrom_file::CD_TRACK_AUDIO && end_offs > start_offs)
			{
				// TODO: check end > start assertion
				m_cd_status.cdda_fad = start_offs - 150;
				m_cd_status.cdda_blocks = (end_offs - start_offs);

				m_cdda->start_audio(m_cd_status.cdda_fad, m_cd_status.cdda_blocks);
				m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS;
				m_cd_status.repeat_current = 0;
				m_cd_status.repeat_count = repeat;
				m_sector_number = (m_sector_number & 0xf0) | GDROM_PLAY_STATE;

				LOGCMD("\tPlayback started\n");
			}
			else
			{
				LOGWARN("track is NOT audio!\n");
				set_sense(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_SENSE_ASC_ASCQ_ILLEGAL_MODE_FOR_THIS_TRACK);
				// TODO: unconfirmed state
				m_sector_number = (m_sector_number & 0xf0) | GDROM_ERROR_STATE;
				return;
			}

			m_phase = SCSI_PHASE_STATUS;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = 0;
			break;
		}

		case 0x21:
		{
			if (!m_image->exists())
			{
				m_phase = SCSI_PHASE_STATUS;
				m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
				m_transfer_length = 0;
				break;
			}

			const u32 start_seek = (command[2]<<16 | command[3]<<8 | command[4]);
			const u8 seek_mode = command[1] & 0xf;
			LOGCMD("CD_SEEK 21h FAD %d type %02x\n", start_seek + 150, seek_mode);
			switch(seek_mode)
			{
				// TODO: implement SEEK & standby
				// 1 FAD seek
				// 2 MSF seek
				// 3 stop audio, go to Home FAD
				case 4:
					// 4 pause audio
					LOGCMD("\tPlayback paused\n");
					m_cdda->pause_audio(1);
					m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_PAUSED;
					m_sector_number = (m_sector_number & 0xf0) | GDROM_PAUSE_STATE;
					break;
			}

			m_phase = SCSI_PHASE_STATUS;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = 0;
			break;
		}

		case 0x30: // CD_READ
			if (command[1] & 1)
			{
				m_transfer_length = 0;
				throw emu_fatalerror("GDROM: MSF mode used for CD_READ, unsupported");
			}
			else
			{
				m_lba = (command[2]<<16 | command[3]<<8 | command[4]) - 150;
				m_blocks = command[8]<<16 | command[9]<<8 | command[10];

				read_type = (command[1] >> 1) & 7;
				data_select = (command[1] >> 4) & 0xf;
				//m_status |= IDE_STATUS_DSC;

				// TODO: any other value is a non-Mode 1 attempt basically
				if (read_type != 0 && read_type != 2)
				{
					throw emu_fatalerror("GDROM: Unhandled read_type %d", read_type);
				}

				if (data_select != 2)   // just sector data
				{
					throw emu_fatalerror("GDROM: Unhandled data_select %d", data_select);
				}

				// LBA 45000 is start of double density GD-ROM
				LOGCMD("CD_READ 30h %02x %02x\n", command[2], command[4]);
				LOGCMD("   LBA %d (%x) for %d blocks (%d bytes, read type %d, data select %d)\n",
					m_lba + 150, m_lba, m_blocks,
					m_blocks * m_sector_bytes, read_type, data_select
				);

				if (m_num_subblocks > 1)
				{
					m_cur_subblock = m_lba % m_num_subblocks;
					m_lba /= m_num_subblocks;
				}
				else
				{
					m_cur_subblock = 0;
				}

				if (m_cdda != nullptr)
				{
					m_cdda->stop_audio();
					m_audio_sense = SCSI_SENSE_ASC_ASCQ_NO_SENSE;
					m_sector_number = (m_sector_number & 0xf0) | GDROM_STANDBY_STATE;
				}

				m_phase = SCSI_PHASE_DATAIN;
				m_status_code = SCSI_STATUS_CODE_GOOD;
				m_transfer_length = m_blocks * m_sector_bytes;
			}
			break;

		case 0x40:
		{
			m_transfer_length = SCSILengthFromUINT8( &command[4] );
			//LOGCMD("CD_SCD 40h %02x %d\n", command[1] & 0xf, m_transfer_length);

			switch(command[1] & 0xf)
			{
				case 0x00:
					m_phase = SCSI_PHASE_DATAIN;
					m_status_code = SCSI_STATUS_CODE_GOOD;
					m_transfer_length = SCSILengthFromUINT8( &command[4] );
					break;
				case 0x01:
					m_phase = SCSI_PHASE_DATAIN;
					m_status_code = SCSI_STATUS_CODE_GOOD;
					m_transfer_length = 0xe;
					break;
				default:
					LOGWARN("command 0x40: unhandled subchannel request\n");
					break;
			}
			break;
		}

		// security check, return no data, always followed by cmd 0x71, command[1] parameter can be 0x1f or 0x9f
		case 0x70:
			m_phase = SCSI_PHASE_STATUS;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = 0;
			break;

		case 0x71:
			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = sizeof(GDROM_Cmd71_Reply);
			break;

		// case 0x22: CD_SCAN
		// case 0x31: CD_READ2
		// case 0x13: REQ_ERROR
		// case 0x16: CD_OPEN [Tray]

		// case 0x08: ??? loopchk uses it in one of the Packet cmnd tests
		//            (check for unsupported command?)

		default:
			throw emu_fatalerror("GDROM: unhandled command %02x", command[0]);
	}
}

// scsicd_read_data
//
// Read data from the device resulting from the execution of a command

void gdrom_device::ReadData( uint8_t *data, int dataLength )
{
	int i;
	uint8_t tmp_buffer[2048];

	switch ( command[0] )
	{
		case 0x10: // REQ_STAT
		{
			data[0] = m_sector_number & 0xf; // CD status
			data[1] = (m_sector_number & 0xf0) | (m_cd_status.repeat_current & 0xf);
			auto fad = m_cdda->get_audio_lba();
			auto trk = m_image->get_track(fad);
			data[2] = m_image->get_adr_control(trk - 1);
			data[3] = trk;
			// TODO: index
			data[4] = 1;
			// FAD, in binary format
			data[5] = (fad >> 16) & 0xff;
			data[6] = (fad >> 8) & 0xff;
			data[7] = fad & 0xff;
			// Max Read Error Retry Times
			data[8] = 0;
			// <reserved>
			data[9] = 0;
			if (LIVE_REQ_STAT)
			{
				popmessage("REQ_STAT STATUS %02x track %d index %d adr %02x fad %06d"
					, m_sector_number
					, data[3]
					, data[4]
					, data[2]
					, fad
				);
			}
			break;
		}

		case 0x11: // REQ_MODE
			//LOGCMD("REQ_MODE dataLength %d\n", dataLength);
			memcpy(data, &GDROM_Cmd11_Reply[transferOffset], (dataLength >= 32-transferOffset) ? 32-transferOffset : dataLength);
			// TODO: reading this while playing a redbook likely hardwires CD speed to 75 Hz
			//data[1] = 0x01;
			break;

		case 0x13: // REQ_ERROR
			// cfr. Appendix I for possible error types
			data[0] = 0xf0; // fixed?
			data[1] = 0;
			data[2] = 6; // Sense Key
			data[3] = 0;
			// 4-7 "specific command details" (?) or FAD
			data[4] = 0;
			data[5] = 0;
			data[6] = 0;
			data[7] = 0;
			data[8] = 0x29; // ASC
			data[9] = 0; // ASCQ
			break;

		case 0x14: // READ TOC (GD-ROM ver.)
			/*
			    Track numbers are problematic here: 0 = lead-in, 0xaa = lead-out.
			    That makes sense in terms of how real-world CDs are referred to, but
			    our internal routines for tracks use "0" as track 1.  That probably
			    should be fixed...
			*/
			LOGCMD("READ_TOC format %d time %d\n",
				command[2] & 0xf, (command[1] >> 1) & 1
			);
			switch (command[2] & 0x0f)
			{
				case 0:     // normal
				{
					int start_trk = 1;
					int end_trk = m_image->get_last_track();
					int len = 408;
					//int in_len;
					int dptr = 0;
					uint32_t tstart;

					// Non-standard, doesn't want in_len readback
					//dptr = 0;
					//data[dptr++] = (len>>8) & 0xff;
					//data[dptr++] = (len & 0xff);

					// pre-fill with 0xffs, audio CD player counts number of tracks by checking the
					// first EOF it encounters (and don't care about the 396-407 dataset below)
					memset(data, 0xff, len);
					dptr = 0;
					LOGTOC("TOC: Start track %d end track %d\n", start_trk, end_trk);
					for (i = start_trk; i <= end_trk; i++)
					{
						u8 adr = m_image->get_adr_control(i - 1) | 1;
						data[dptr++] = adr;

						tstart = m_image->get_track_start(i - 1) + 150;
						//if ((command[1]&2)>>1)
						//  tstart = cdrom_file::lba_to_msf(tstart);
						data[dptr++] = (tstart>>16) & 0xff;
						data[dptr++] = (tstart>>8) & 0xff;
						data[dptr++] = (tstart & 0xff);
						LOGTOC("\t%d: FAD %d %02x\n", i, tstart, adr);
					}

					dptr = 396;
					data[dptr++] = m_image->get_adr_control(0) | 1;
					data[dptr++] = start_trk;
					data[dptr++] = 0;
					data[dptr++] = 0;
					data[dptr++] = m_image->get_adr_control(end_trk) | 1;
					data[dptr++] = end_trk;
					data[dptr++] = 0;
					data[dptr++] = 0;
					const u32 tend = m_image->get_track_start(0xaa) + 150;
					//if ((command[1]&2)>>1)
					//  tstart = cdrom_file::lba_to_msf(tstart);
					data[dptr++] = m_image->get_adr_control(0xaa) | 1;
					data[dptr++] = (tend>>16) & 0xff;
					data[dptr++] = (tend>>8) & 0xff;
					data[dptr++] = (tend & 0xff);
					LOGTOC("\t0xaa: FAD %d\n", tstart);
					break;
				}
				default:
					LOGWARN("Unhandled READ_TOC format %d\n", command[2]&0xf);
					break;
			}
			break;

		case 0x15:
		{
			// REQ_SES
			data[0] = m_sector_number & 0xf; // CD status, stripped by type?
			data[1] = 0; // <reserved>, zeroed
			data[2] = 1; // number of sessions

			const u8 session_num = command[2] & 0xff;
			u32 fad;
			if (session_num == 0)
				fad = m_image->get_track_start(0xaa) + 150;
			else
				fad = m_image->get_track_start(0) + 150;
			LOGTOC("SESSION %d: %06d\n", session_num, fad);
			data[3] = (fad >> 16) & 0xff;
			data[4] = (fad >> 8) & 0xff;
			data[5] = fad & 0xff;
			break;
		}

		case 0x30: // CD_READ
			LOGXFER("CD_READ read %x dataLength,\n", dataLength);
			if ((m_image->exists()) && (m_blocks))
			{
				while (dataLength > 0)
				{
					if (!m_image->read_data(m_lba, tmp_buffer, cdrom_file::CD_TRACK_MODE1))
					{
						LOGWARN("CD read error!\n");
						return;
					}

					LOGXFER("True LBA: %d, buffer half: %d\n", m_lba, m_cur_subblock * m_sector_bytes);

					memcpy(data, &tmp_buffer[m_cur_subblock * m_sector_bytes], m_sector_bytes);

					m_cur_subblock++;
					if (m_cur_subblock >= m_num_subblocks)
					{
						m_cur_subblock = 0;

						m_lba++;
						m_blocks--;
					}

					m_last_lba = m_lba;
					dataLength -= m_sector_bytes;
					data += m_sector_bytes;
				}
			}
			break;

		case 0x40: // Get Subchannel status
			switch (command[2] & 0x0f)
			{
				case 0: // Subcode P-W
				{
					data[0] = 0; // Reserved
					data[1] = m_audio_sense;
					data[2] = 0;
					data[3] = m_transfer_length; // header size
					auto fad = m_cdda->get_audio_lba();
					if (!m_image->read_subcode( fad, &data[4 + m_transfer_length]))
					{
						// attempt to provide Q channel
						// TODO: audio CD player definitely reads here but never updates GFXs?
						u8 subqbuf[12]{};
						const u32 msf_abs = cdrom_file::lba_to_msf_alt( fad );
						const auto trk = m_image->get_track( fad );
						const u32 msf_rel = cdrom_file::lba_to_msf_alt( fad - m_image->get_track_start( trk ) );

						subqbuf[0] = 0x01 | ((m_image->get_track_type(m_image->get_track(trk + 1)) == cdrom_file::CD_TRACK_AUDIO) ? 0x00 : 0x40);
						subqbuf[1] = dec_2_bcd(trk + 1);
						subqbuf[2] = 1;
						subqbuf[3] = dec_2_bcd((msf_rel >> 16) & 0xff);
						subqbuf[4] = dec_2_bcd((msf_rel >> 8) & 0xff);
						subqbuf[5] = dec_2_bcd((msf_rel >> 0) & 0xff);
						subqbuf[6] = 0;
						subqbuf[7] = dec_2_bcd((msf_abs >> 16) & 0xff);
						subqbuf[8] = dec_2_bcd((msf_abs >> 8) & 0xff);
						subqbuf[9] = dec_2_bcd((msf_abs >> 0) & 0xff);
						subqbuf[10] = 0xff; //machine().rand(); // CRC
						subqbuf[11] = 0xff; //machine().rand();

						if (LIVE_REQ_STAT)
						{
							popmessage("Qchan STATUS %02x TRACK %d ABS %02x:%02x:%02x REL %02x:%02x:%02x"
								, subqbuf[0]
								, subqbuf[1]
								, subqbuf[7], subqbuf[8], subqbuf[9]
								, subqbuf[3], subqbuf[4], subqbuf[5]
							);
						}

						for (int i = 0; i < m_transfer_length - 4; i++)
							data[i + 4] = BIT(subqbuf[i >> 3], 7 - (i & 7)) ? 0x40 : 0x00;
					}

					break;
				}
				case 1: // Subcode Q
					// TODO: unread by audio CD player (?)
					LOGWARN("CD_SCD: unhandled Subcode Q path\n");
					data[0] = 0; // Reserved
					data[1] = m_audio_sense;
					data[2] = 0;
					data[3] = m_transfer_length; // header size
					data[4] = 0; // ADR
					data[5] = 1; // Track Number
					data[6] = 1; // index #1
					data[7] = 0; // ?
					data[8] = 0; // ?
					data[9] = 0; // elapsed FAD
					data[0xa] = 0; // <reserved>, zeroed
					data[0xb] = 0; // FAD >> 16
					data[0xc] = 0; // FAD >> 8
					data[0xd] = 0x96; // FAD >> 0
					break;
			}
			break;

		case 0x71:
			LOGCMD("SYS_REQ_SECU\n");
			memcpy(data, &GDROM_Cmd71_Reply[0], sizeof(GDROM_Cmd71_Reply));
			if (m_image->is_gd())
				data[10] = 0x1f; // needed by dimm board firmware
			break;

		default:
			t10mmc::ReadData( data, dataLength );
			break;
	}
}

// scsicd_write_data
//
// Write data to the CD-ROM device as part of the execution of a command

void gdrom_device::WriteData( uint8_t *data, int dataLength )
{
	switch (command[0])
	{
		case 0x12: // SET_MODE
			memcpy(&GDROM_Cmd11_Reply[transferOffset], data, (dataLength >= 32-transferOffset) ? 32-transferOffset : dataLength);
			if (data[2] != 0 && data[2] != 7)
				throw emu_fatalerror("GDROM: Unsupported CD speed setting %02x\n", data[2]);
			// ---- -000 Max speed (x12)
			// ---- -xxx [x1 (75 Hz), x2, x4, x6, x8, x10, x12]
			LOGCMD("\tCD speed: %02x\n", data[2]);
			// Standby Time:
			// - 0 = no standby
			// any other value = number of seconds for pause -> standby transition
			LOGCMD("\tStandby Time: %02x%02x\n", data[4], data[5]);

			LOGCMD("\tRead Continous: %s\n",    BIT(data[6], 5) ? "enable" : "disable");
			LOGCMD("\tECC: %s\n",               BIT(data[6], 4) ? "enable" : "disable");
			LOGCMD("\tRead Retry: %s\n",        BIT(data[6], 3) ? "enable" : "disable");
			LOGCMD("\tForm 2 Read Retry: %s\n", BIT(data[6], 0) ? "enable" : "disable");

			LOGCMD("\tRead Retry Times %d\n", data[9]);
			break;

		default:
			t10mmc::WriteData( data, dataLength );
			break;
	}
}

// device type definition
DEFINE_DEVICE_TYPE(ATAPI_GDROM, gdrom_device, "gdrom", "GD-ROM")

gdrom_device::gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, ATAPI_GDROM, tag, owner, clock)
{
}

void gdrom_device::device_start()
{
	save_item(NAME(read_type));
	save_item(NAME(data_select));
	save_item(NAME(transferOffset));

	/// TODO: split identify buffer into another method as device_start() should be called after it's filled in, but the atapi_cdrom_device has it's own.
	atapi_cdrom_device::device_start();

	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));

	m_identify_buffer[0] = 0x8600; // ATAPI device, cmd set 6 compliant, DRQ within 3 ms of PACKET command

	// non-standard identify returns
	// TODO: should be centralized in command 11h
	// loopchk: identify device Maker test (0104) PC=c04694a
	// Was 23
	int dptr = 8;
	m_identify_buffer[dptr++] = 'S' | ('E' << 8);
	m_identify_buffer[dptr++] = ' ' | (' ' << 8);
	m_identify_buffer[dptr++] = ' ' | (' ' << 8);
	m_identify_buffer[dptr++] = ' ' | (' ' << 8);

	// loopchk: identify device Model test (0104) PC=c046acc
	// Was 27
	dptr = 16;
	m_identify_buffer[dptr++] = 'C' | ('D' << 8);
	m_identify_buffer[dptr++] = '-' | ('R' << 8);
	m_identify_buffer[dptr++] = 'O' | ('M' << 8);
	m_identify_buffer[dptr++] = ' ' | ('D' << 8);
	m_identify_buffer[dptr++] = 'R' | ('I' << 8);
	m_identify_buffer[dptr++] = 'V' | ('E' << 8);
	m_identify_buffer[dptr++] = ' ' | (' ' << 8);
	m_identify_buffer[dptr++] = ' ' | (' ' << 8);
	// TODO: versioning is unchecked by loopchk
	m_identify_buffer[dptr++] = '6' | ('.' << 8);
	m_identify_buffer[dptr++] = '4' | ('2' << 8);
	// FIXME: doc mentions a "System Date" readback
	for (; dptr < 47; dptr ++)
		m_identify_buffer[dptr] = ' ' | (' ' << 8);

	m_identify_buffer[49] = 0x0400; // IORDY may be disabled

	m_identify_buffer[63]=7; // multi word dma mode 0-2 supported
	m_identify_buffer[64]=1; // PIO mode 3 supported
}

void gdrom_device::process_buffer()
{
	atapi_hle_device::process_buffer();
	// HACK: find out when this should be updated
	// TODO: upper byte is CD type detection
	// 0000 CD-DA
	// 0001 CD-ROM
	// 0010 CD-ROM XA / CD Extra
	// 0011 CD-I
	// 1000 GD-ROM
	const u8 cd_type = m_image->is_gd() ? 0x80 : 0x00;
	m_sector_number = cd_type | (m_image->exists() ? m_sector_number & 0xf : GDROM_NODISC_STATE);
}

void gdrom_device::signature()
{
	atapi_hle_device::signature();

	// 0000 CD-DA
	// 0001 CD-ROM
	// 0010 CD-ROM XA, CD Extra
	// 0011 CD-i
	// 1000 GD-ROM
	const u8 cd_type = m_image->is_gd() ? 0x80 : 0x00;

	// naomi dimm board firmware needs the upper nibble to be 8 at the beginning
	m_sector_number = cd_type | (m_image->exists() ? m_sector_number & 0xf : GDROM_NODISC_STATE);
}

//bool gdrom_device::set_features()
//{
	// TODO: DSC, likely tested by Check-GD programs
//  m_status |= IDE_STATUS_DSC;
//  return atapi_cdrom_device::set_features();
//}

void gdrom_device::cdda_end_mark_cb(int state)
{
	if (state != ASSERT_LINE)
		return;

	m_cd_status.repeat_current ++;
	m_cd_status.repeat_current &= 0xf;
	LOG("end marker %d %d\n", m_cd_status.repeat_current, m_cd_status.repeat_count);

	// repeat_count = 0xf: infinite, not unlike Saturn CD Block
	// - ggx sound test track #20 (opening) is a good testing scenario.
	// - audio cd player don't care about this:
	//   it manually handle repeat modes depending on GD state alone.
	if (m_cd_status.repeat_current > m_cd_status.repeat_count)
	{
		m_cdda->pause_audio(1);
		m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_SUCCESSFULLY_COMPLETED;
		LOG("\tCompleted\n");
		m_sector_number = (m_sector_number & 0xf0) | GDROM_PAUSE_STATE;
	}
	else
	{
		m_cdda->start_audio(m_cd_status.cdda_fad, m_cd_status.cdda_blocks);
		LOG("\tRestart playback\n");
		m_sector_number = (m_sector_number & 0xf0) | GDROM_PLAY_STATE;
	}
}
