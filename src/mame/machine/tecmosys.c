/* tecmosys protection related functions */

/*
	The device validates a password,
	then uploads the size of a code upload followed by the code upload itself.
	After that, it uploads 4 ranges of code to checksum, followed by the checksums.

	Apart from inital protection calls and code upload, both games in the main game loop
	write to the protection but they seem to ignore the returned data so maybe it is tied to something else too.

	It looks like the code upload is very plain, it can only be 0xff bytes long, and not contain the byte 0xff.
	The checksum ranges can't contain 0xff either, although the checksum values can.
	I'd be very interested in putting some trojan ROMs together if anyone has a board to run them on.
	You can get me at nuapete@hotmail.com
*/

#include "driver.h"
#include "tecmosys.h"

UINT8 device_read_ptr;
UINT8 device_status;
struct prot_data* device_data;
static UINT8 device_value = 0xff;

// deroon prot data
static UINT8 deroon_passwd[] = {'L','U','N','A',0};
static UINT8 deroon_upload[] = {0x02, 0x4e, 0x75, 0x00 }; // code length, code, 0x00 trailer
struct prot_data deroon_data =
{
	5,
	deroon_passwd,
	deroon_upload,
	{
		0x10,0x11,0x12,0x13,	// range 1 using static ranges from the ROM to avoid calculating sums.
		0x24,0x25,0x26,0x27,	// range 2
		0x38,0x39,0x3a,0x3b,	// range 3
		0x4c,0x4d,0x4e,0x4f,	// range 4
		0x00,					// trailer
	},
	{ 0xa6, 0x29, 0x4b, 0x3f }
};

// tkdensho prot data
static UINT8 tkdensho_passwd[] = {'A','G','E','P','R','O','T','E','C','T',' ','S','T','A','R','T',0};
static UINT8 tkdensho_upload[] = {0x06, 0x4e, 0xf9, 0x00, 0x00, 0x22, 0xc4,0x00};
struct prot_data tkdensho_data =
{
	0x11,
	tkdensho_passwd,
	tkdensho_upload,
	{
		0x10,0x11,0x12,0x13,	// range 1
		0x24,0x25,0x26,0x27,	// range 2
		0x38,0x39,0x3a,0x3b,	// range 3
		0x4c,0x4d,0x4e,0x4f,	// range 4
		0x00,					// trailer
	},
	{ 0xbf, 0xfa, 0xda, 0xda }
};

struct prot_data tkdensha_data =
{
	0x11,
	tkdensho_passwd,
	tkdensho_upload,
	{
		0x10,0x11,0x12,0x13,	// range 1
		0x24,0x25,0x26,0x27,	// range 2
		0x38,0x39,0x3a,0x3b,	// range 3
		0x4c,0x4d,0x4e,0x4f,	// range 4
		0x00,					// trailer
	},
	{ 0xbf, 0xfa, 0x21, 0x5d }
};


READ16_HANDLER(prot_status_r)
{
	if (ACCESSING_BITS_8_15)
	{
		// Bit 7: 0 = ready to write
		// Bit 6: 0 = ready to read
		return 0;
	}

	return 0xc0; // simulation is always ready
}

WRITE16_HANDLER(prot_status_w)
{
	// deroon clears the status in one place.
}


READ16_HANDLER(prot_data_r)
{
	// prot appears to be read-ready for two consecutive reads
	// but returns 0xff for subsequent reads.
	UINT8 ret = device_value;
	device_value = 0xff;
	//logerror("- prot_r = 0x%02x\n", ret );
	return ret << 8;
}


WRITE16_HANDLER(prot_data_w)
{
	// Only LSB
	data >>= 8;
	//logerror("+ prot_w( 0x%02x )\n", data);

	switch( device_status )
	{
		case DS_IDLE:
			if( data == 0x13 )
			{
				device_status = DS_LOGIN;
				device_value = device_data->passwd_len;
				device_read_ptr = 0;
				break;
			}
			break;

		case DS_LOGIN:
			if( device_read_ptr >= device_data->passwd_len)
			{
				device_status = DS_SEND_CODE;
				device_value = device_data->code[0];
				device_read_ptr = 1;
			}
			else
				device_value = device_data->passwd[device_read_ptr++] == data ? 0 : 0xff;
			break;

		case DS_SEND_CODE:
			if( device_read_ptr >= device_data->code[0]+2 ) // + code_len + trailer
			{
				device_status = DS_SEND_ADRS;
				device_value = device_data->checksum_ranges[0];
				device_read_ptr = 1;
			}
			else
				device_value = data == device_data->code[device_read_ptr-1] ? device_data->code[device_read_ptr++] : 0xff;
			break;

		case DS_SEND_ADRS:
			if( device_read_ptr >= 16+1 ) //+ trailer
			{
				device_status = DS_SEND_CHKSUMS;
				device_value = 0;
				device_read_ptr = 0;
			}
			else
			{
				device_value = data == device_data->checksum_ranges[device_read_ptr-1] ? device_data->checksum_ranges[device_read_ptr++] : 0xff;
			}
			break;

		case DS_SEND_CHKSUMS:
			if( device_read_ptr >= 5 )
			{
				device_status = DS_DONE;
				device_value = 0;
			}
			else
				device_value = data == device_data->checksums[device_read_ptr] ? device_data->checksums[device_read_ptr++] : 0xff;
			break;

		case DS_DONE:
			switch( data )
			{
				case 0x00:
				case 0x20:
				case 0xbf:
				case 0xff:
					break;

				default:
					logerror( "Protection still in use??? w=%02x\n", data );
					break;
			}
			break;
	}
}
