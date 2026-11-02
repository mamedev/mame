// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Curt Coder
/*

Xebec S1410

The Z80 firmware is not executed; the controller is emulated at the SASI command level.

PCB Layout
----------

ASSY 104527 REV E04 SN 127623

|-------------------------------------------|
|                                           |
|CN1                                        |
|                                           |
|                                           |
|CN2                                        |
|                   XEBEC2               CN5|
|   PROM                        2114        |
|CN3                XEBEC1      2114        |
|                                           |
|CN4                Z80         ROM         |
|   20MHz                             16MHz |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    Z80     - Zilog Z8400APS Z80A CPU
    ROM     - 2732 pinout ROM "XEBEC 104521G 2155008 M460949"
    PROM    - National Semiconductor DM74S288N "103911" 32x8 TTL PROM
    2114    - National Semiconductor NMC2148HN-3 1Kx4 RAM
    XEBEC1  - Xebec 3198-0009
    XEBEC2  - Xebec 3198-0045 "T20"
    CN1     - 4-pin Molex, drive power
    CN2     - 34-pin PCB edge, ST-506 drive 0/1 control
    CN3     - 2x10 PCB header, ST-506 drive 0 data
    CN4     - 2x10 PCB header, ST-506 drive 1 data
    CN5     - 2x25 PCB header, SASI host interface


ASSY 104766 REV C02 SN 231985P

|-------------------------------------------|
|                                           |
| CN1                           SY2158      |
|                               CN7     CN6 |
|                               ROM         |
| CN2                                       |
|           XEBEC1          Z80             |
|                                       CN5 |
| CN3       XEBEC2   20MHz      XEBEC3      |
|                                           |
| CN4       XEBEC4              XEBEC5      |
|                                           |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    Z80     - Zilog Z8400APS Z80A CPU
    ROM     - 2732 pinout ROM "104788D"
    SY2158  - Synertek SY2158A-3 1Kx8 RAM
    XEBEC1  - Xebec 3198-0046N8445
    XEBEC2  - Xebec 3198-0009
    XEBEC3  - Xebec 3198-0057
    XEBEC4  - Xebec 3198-0058
    XEBEC5  - Xebec 3198-0056
    CN1     - 4-pin Molex, drive power
    CN2     - 34-pin PCB edge, ST-506 drive 0/1 control
    CN3     - 2x10 PCB header, ST-506 drive 0 data
    CN4     - 2x10 PCB header, ST-506 drive 1 data
    CN5     - 2x25 PCB header, SASI host interface
    CN6     - 2x8 PCB header
    CN7     - 2x10 PCB header (test only)

*/


#include "emu.h"
#include "bus/nscsi/s1410.h"

#include "cpu/z80/z80.h"

#include "multibyte.h"

#define LOG_COMMAND (1U << 1)
#define LOG_DATA    (1U << 2)

#define VERBOSE 0

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_S1410, nscsi_s1410_device, "s1410", "Xebec S1410 5.25 Inch Winchester Disk Controller")

ROM_START( s1410 )
	ROM_REGION( 0x1000, "z80", 0 )
	ROM_DEFAULT_BIOS("104521g")
	ROM_SYSTEM_BIOS( 0, "104521f", "104521F" )
	ROMX_LOAD( "104521f", 0x0000, 0x1000, CRC(305b8e76) SHA1(9efaa53ae86bc111bd263ad433e083f78a000cab), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "104521g", "104521G" )
	ROMX_LOAD( "104521g", 0x0000, 0x1000, CRC(24385115) SHA1(c389f6108cd5ed798a090acacce940ee43d77042), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "104788d", "104788D (BASF 6188)" )
	ROMX_LOAD( "104788d", 0x0000, 0x1000, CRC(2e385e2d) SHA1(7e2c349b2b6e95f2134f82cffc38d86b8a68390d), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "104788e", "104788E (Seagate ST-251)" )
	ROMX_LOAD( "104788e", 0x0000, 0x1000, CRC(40a7195d) SHA1(bf68e99dad182bfcbb5d488aeb440cba572a6ef8), ROM_BIOS(3) )

	ROM_REGION( 0x20, "103911", 0 )
	ROM_LOAD( "103911", 0x00, 0x20, NO_DUMP ) // DM74S288N
ROM_END

nscsi_s1410_device::nscsi_s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_S1410, tag, owner, clock)
{
}

const tiny_rom_entry *nscsi_s1410_device::device_rom_region() const
{
	return ROM_NAME( s1410 );
}

void nscsi_s1410_device::z80_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("z80", 0);
	map(0x1000, 0x13ff).ram();
}

void nscsi_s1410_device::z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x66, 0x66).nopw();
	map(0x67, 0x67).nopw();
	map(0x68, 0x68).nopr();
	map(0x69, 0x69).nopw();
	map(0x6a, 0x6a).nopw();
	map(0x6b, 0x6b).nopw();
	map(0x6c, 0x6c).nopw();
	map(0xa0, 0xa0).noprw();
	map(0xc1, 0xc1).nopw();
	map(0xc2, 0xc2).nopw();
	map(0xc3, 0xc3).nopw();
}

void nscsi_s1410_device::device_add_mconfig(machine_config &config)
{
	nscsi_harddisk_device::device_add_mconfig(config);

	z80_device &z80(Z80(config, "z80", 16_MHz_XTAL / 4));
	z80.set_addrmap(AS_PROGRAM, &nscsi_s1410_device::z80_mem);
	z80.set_addrmap(AS_IO, &nscsi_s1410_device::z80_io);
	z80.set_disable();
}

void nscsi_s1410_device::device_reset()
{
	nscsi_harddisk_device::device_reset();

	// initialize drive characteristics
	params[0] = 0;
	params[1] = 153;
	params[2] = 4;
	params[3] = 0;
	params[4] = 128;
	params[5] = 0;
	params[6] = 64;
	params[7] = 11;
}

bool nscsi_s1410_device::scsi_command_done(uint8_t command, uint8_t length)
{
	if(!length)
		return false;
	switch(command >> 5) {
	case 0: return length == 6;
	case 1: return true;
	case 2: return true;
	case 3: return true;
	case 4: return true;
	case 5: return true;
	case 6: return true;
	case 7: return length == 6;
	}
	return true;
}

void nscsi_s1410_device::scsi_command()
{
	memset(m_scsi_sense_buffer, 0, sizeof(m_scsi_sense_buffer));

	switch(m_scsi_cmdbuf[0]) {
	case SC_TEST_UNIT_READY:
	case SC_REZERO_UNIT:
	case SC_REASSIGN_BLOCKS:
	case SC_READ:
	case SC_WRITE:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
		} else {
			nscsi_harddisk_device::scsi_command();
		}
		break;

	case SC_SEEK:
		{
			const auto &info = image->get_info();
			int max_lba = (info.cylinders * info.heads * info.sectors) - 1;
			lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			if (lba <= max_lba) {
				scsi_status_complete(SS_GOOD);
			} else {
				scsi_status_complete(SS_SEEK_ERROR);
				m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			}
		}
		break;

	case SC_REQUEST_SENSE:
		scsi_data_in(SBUF_SENSE, 4);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_UNIT:
		LOG("command FORMAT UNIT\n");
		{
			const auto &info = image->get_info();
			auto block = std::make_unique<uint8_t[]>(info.sectorbytes);
			memset(&block[0], 0x6c, info.sectorbytes);
			lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
			for(; lba < (info.cylinders * info.heads * info.sectors); lba++) {
				image->write(lba, block.get());
			}
		}
		scsi_status_complete(SS_GOOD);
		break;

	case SC_FORMAT_TRACK: {
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			return;
		}

		lba = get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff;
		blocks = (bytes_per_sector == 256) ? 32 : 17;

		int track_length = blocks*bytes_per_sector;
		auto block = std::make_unique<uint8_t[]>(track_length);
		memset(&block[0], 0x6c, track_length);

		if(!image->write(lba, &block[0])) {
			logerror("%s: HD WRITE ERROR !\n", tag());
			scsi_status_complete(SS_FORMAT_ERROR);
			m_scsi_sense_buffer[0] = SK_FORMAT_ERROR;
		} else {
			scsi_status_complete(SS_GOOD);
		}
		}
		break;

	case SC_FORMAT_ALT_TRACK:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			return;
		}

		scsi_data_out(2, 3);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_INIT_DRIVE_PARAMS:
		scsi_data_out(2, 8);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_WRITE_SECTOR_BUFFER:
		scsi_data_out(2, bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_SECTOR_BUFFER:
		scsi_data_in(2, bytes_per_sector);
		scsi_status_complete(SS_GOOD);
		break;

	case SC_CHECK_TRACK_FORMAT:
		if (m_scsi_cmdbuf[1] >> 5) {
			scsi_status_complete(SS_NOT_READY);
			m_scsi_sense_buffer[0] = SK_DRIVE_NOT_READY;
			return;
		}
		scsi_status_complete(SS_GOOD);
		break;

	case SC_READ_ECC_BURST:
	case SC_RAM_DIAG:
	case SC_DRIVE_DIAG:
	case SC_CONTROLLER_DIAG:
	case SC_READ_LONG:
	case SC_WRITE_LONG:
		scsi_status_complete(SS_GOOD);
		break;

	default:
		logerror("%s: command %02x ***UNKNOWN***\n", tag(), m_scsi_cmdbuf[0]);
		break;
	}
}

uint8_t nscsi_s1410_device::scsi_get_data(int id, int pos)
{
	switch(m_scsi_cmdbuf[0]) {
	case SC_READ_SECTOR_BUFFER:
		return block[pos];

	default:
		return nscsi_harddisk_device::scsi_get_data(id, pos);
	}
}

void nscsi_s1410_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if(id != 2 && !pos) {
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}

	switch(m_scsi_cmdbuf[0]) {
	case SC_FORMAT_ALT_TRACK:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		break;

	case SC_INIT_DRIVE_PARAMS:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		params[pos] = data;
		break;

	case SC_WRITE_SECTOR_BUFFER:
		LOGMASKED(LOG_DATA, "s1410: scsi_put_data, id:%d pos:%d data:%02x %c\n", id, pos, data, data >= 0x20 && data < 0x7f ? (char)data : ' ');
		block[pos] = data;
		break;

	default:
		return nscsi_harddisk_device::scsi_put_data(id, pos, data);
	}
}

// Byte transfer rate (5Mb/s)
attotime nscsi_s1410_device::scsi_data_byte_period()
{
	return attotime::from_nsec(1600);
}

// Command execution delay
attotime nscsi_s1410_device::scsi_data_command_delay()
{
	switch(m_scsi_cmdbuf[0]) {
	case SC_READ:
	case SC_WRITE:
	case SC_SEEK:
		// Legacy default (unchanged unless a driver calls set_seek_timing()): a
		// flat average-seek delay charged on every command, matching the older
		// NEC D5126A approximation the existing s1410 users rely on.  With a
		// model configured, defer to the base's cylinder-aware timing.
		if (!m_seek_model)
			return attotime::from_msec(85);
		return seek_time(get_u24be(&m_scsi_cmdbuf[1]) & 0x1fffff);

	default:
		return attotime::zero;
	}
}
