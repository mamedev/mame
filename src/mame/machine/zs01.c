/*
 * zs01.c
 *
 * Secure SerialFlash
 *
 * This is a high level emulation of the PIC used in some of the System 573 security cartridges.
 *
 */

#include "emu.h"
#include "machine/zs01.h"
#include "machine/ds2401.h"

#define VERBOSE_LEVEL 0

inline void ATTR_PRINTF(3,4) zs01_device::verboselog(int n_level, const char *s_fmt, ...)
{
	if(VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("zs01 %s %s: %s", tag(), machine().describe_context(), buf);
	}
}

// device type definition
const device_type ZS01 = &device_creator<zs01_device>;

void zs01_device::static_set_ds2401_tag(device_t &device, const char *ds2401_tag)
{
	zs01_device &zs01 = downcast<zs01_device &>(device);
	zs01.ds2401_tag = ds2401_tag;
}

zs01_device::zs01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_secure_serial_flash(mconfig, ZS01, "ZS01", tag, owner, clock)
{
}

void zs01_device::device_start()
{
	device_secure_serial_flash::device_start();
	save_item(NAME(state));
	save_item(NAME(shift));
	save_item(NAME(bit));
	save_item(NAME(byte));
	save_item(NAME(write_buffer));
	save_item(NAME(read_buffer));
	save_item(NAME(response_key));
	save_item(NAME(response_to_reset));
	save_item(NAME(command_key));
	save_item(NAME(data_key));
}

void zs01_device::device_reset()
{
	device_secure_serial_flash::device_reset();
	state = STATE_STOP;
	shift = 0;
	bit = 0;
	byte = 0;
	memset(write_buffer, 0, SIZE_WRITE_BUFFER);
	memset(read_buffer, 0, SIZE_READ_BUFFER);
	memset(response_key, 0, SIZE_KEY);
}

void zs01_device::nvram_default()
{
	// region always wins
	if(m_region)
	{
		// Ensure the size is correct though
		if(m_region->bytes() != SIZE_RESPONSE_TO_RESET+SIZE_KEY+SIZE_KEY+SIZE_DATA)
			logerror("zs01 %s: Wrong region length for initialization data, expected 0x%x, got 0x%x\n",
						tag(),
						SIZE_RESPONSE_TO_RESET+SIZE_KEY+SIZE_KEY+SIZE_DATA,
						m_region->bytes());
		else {
			UINT8 *rb = m_region->base();
			int offset = 0;
			memcpy(response_to_reset, rb + offset, SIZE_RESPONSE_TO_RESET); offset += SIZE_RESPONSE_TO_RESET;
			memcpy(command_key,       rb + offset, SIZE_KEY); offset += SIZE_KEY;
			memcpy(data_key,          rb + offset, SIZE_KEY); offset += SIZE_KEY;
			memcpy(data,              rb + offset, SIZE_DATA); offset += SIZE_DATA;
			return;
		}
	}

	// That chip isn't really usable without the passwords, so bitch
	// if there's no region
	logerror("zs01 %s: Warning, no default data provided, chip is unusable.\n", tag());
	memset(response_to_reset, 0, SIZE_RESPONSE_TO_RESET);
	memset(command_key,       0, SIZE_KEY);
	memset(data_key,          0, SIZE_KEY);
	memset(data,              0, SIZE_DATA);
}

void zs01_device::cs_0()
{
}

void zs01_device::cs_1()
{
}

void zs01_device::rst_0()
{
}

void zs01_device::rst_1()
{
	if(!cs) {
		verboselog(1, "goto response to reset\n");
		state = STATE_RESPONSE_TO_RESET;
		bit = 0;
		byte = 0;
	}
}

void zs01_device::decrypt(UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte)
{
	UINT32 a0;
	UINT32 v1;
	UINT32 v0;
	UINT32 a1;
	UINT32 t1;
	UINT32 t0;

	length--;
	if( length >= 0 )
	{
		do
		{
			t1 = source[ length ];
			a1 = 7;
			t0 = t1;

			do
			{
				v1 = key[ a1 ];
				a1--;
				v0 = v1 & 0x1f;
				v0 = t0 - v0;
				v1 >>= 5;
				v0 &= 0xff;
				a0 = (signed)v0 >> v1;
				v1 = 8 - v1;
				v1 &= 7;
				v0 = (signed)v0 << v1;
				t0 = a0 | v0;
			} while( a1 > 0 );

			v1 = key[ 0 ];
			a0 = previous_byte;
			v0 = t0 & 0xff;
			previous_byte = t1;
			v0 = v0 - v1;
			v0 = v0 ^ a0;

			destination[ length ] = v0;
			length--;
		}
		while( length >= 0 );
	}
}

void zs01_device::decrypt2(UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte)
{
	UINT32 a0;
	UINT32 v1;
	UINT32 v0;
	UINT32 a1;
	UINT32 t2;
	UINT32 t1;
	UINT32 t0;

	t2 = 0;
	if( length >= 0 )
	{
		do
		{
			t1 = source[ t2 ];
			a1 = 7;
			t0 = t1;

			do
			{
				v1 = key[ a1 ];
				a1--;
				v0 = v1 & 0x1f;
				v0 = t0 - v0;
				v1 >>= 5;
				v0 &= 0xff;
				a0 = (signed)v0 >> v1;
				v1 = 8 - v1;
				v1 &= 7;
				v0 = (signed)v0 << v1;
				t0 = a0 | v0;
			} while( a1 > 0 );

			v1 = key[ 0 ];
			a0 = previous_byte;
			v0 = t0 & 0xff;
			previous_byte = t1;
			v0 = v0 - v1;
			v0 = v0 ^ a0;

			destination[ t2 ] = v0;
			t2++;

		} while( t2 < length );
	}
}

void zs01_device::encrypt(UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT32 previous_byte)
{
	UINT32 t0;
	UINT32 v0;
	UINT32 v1;
	UINT32 a0;
	UINT32 a1;

	length--;
	if( length >= 0 )
	{
		do
		{
			t0 = 1;
			v0 = source[ length ];
			v1 = previous_byte;
			a0 = key[ 0 ];
			v0 ^= v1;
			a0 += v0;

			do
			{
				a1 = key[ t0 ];
				t0++;
				a0 &= 0xff;
				v0 = a1 >> 5;
				v1 = a0 << v0;
				v0 = 8 - v0;
				v0 &= 7;
				a0 = (signed) a0 >> v0;
				v1 |= a0;
				v1 &= 0xff;
				a1 &= 0x1f;
				v1 += a1;
				v0 = (signed) t0 < 8;
				a0 = v1;

			} while( v0 != 0 );

			previous_byte = v1;

			destination[ length ] = a0;
			length--;

		} while( length >= 0 );
	}
}

UINT16 zs01_device::do_crc(UINT8 *buffer, UINT32 length)
{
	UINT32 v1;
	UINT32 a3;
	UINT32 v0;
	UINT32 a2;

	v1 = 0xffff;
	a3 = 0;

	if( length > 0 )
	{
		do
		{
			v0 = buffer[ a3 ];
			a2 = 7;
			v0 = v0 << 8;
			v1 = v1 ^ v0;
			v0 = v1 & 0x8000;

			do
			{
				if( v0 != 0 )
				{
					v0 = v1 << 1;
					v1 = v0 ^ 0x1021;
				}
				else
				{
					v0 = v1 << 1;
					v1 = v1 << 1;
				}

				a2--;
				v0 = v1 & 0x8000;
			} while( (signed) a2 >= 0 );

			a3++;
			v0 = (signed) a3 < (signed) length;
		} while ( v0 != 0 );
	}

	v0 = ~v1 ;
	v0 = v0 & 0xffff;

	return v0;
}

int zs01_device::data_offset()
{
	int block = ( (write_buffer[0] & 2 ) << 7 ) | write_buffer[1];

	return block * SIZE_DATA_BUFFER;
}

void zs01_device::scl_0()
{
	if(!cs) {
		switch(state) {
		case STATE_STOP:
			break;

		case STATE_RESPONSE_TO_RESET:
			if(!bit) {
				shift = response_to_reset[byte];
				verboselog(1, "<- response_to_reset[%d]: %02x\n", byte, shift);
			}

			sdar = (shift >> 7) & 1;
			shift <<= 1;
			bit++;

			if( bit == 8 ) {
				bit = 0;
				byte++;
				if( byte == 4 ) {
					sdar = true;
					verboselog(1, "goto stop\n");
					state = STATE_STOP;
				}
			}
			break;

		case STATE_LOAD_COMMAND:
			break;

		case STATE_READ_DATA:
			break;
		}
	}
}

void zs01_device::scl_1()
{
	if(!cs) {
		switch(state) {
		case STATE_STOP:
			break;

		case STATE_RESPONSE_TO_RESET:
			break;

		case STATE_LOAD_COMMAND:
			if(bit < 8) {
				shift <<= 1;
				if(sdaw)
					shift |= 1;
				bit++;
				verboselog(2, "clock %d %02x\n", bit, shift);
			} else {
				sdar = false;

				switch(state) {
				case STATE_LOAD_COMMAND:
					write_buffer[byte] = shift;
					verboselog(2, "-> write_buffer[%d]: %02x\n", byte, write_buffer[byte]);
					byte++;
					if(byte == SIZE_WRITE_BUFFER) {
						UINT16 crc;

						decrypt(write_buffer, write_buffer, SIZE_WRITE_BUFFER, command_key, 0xff);

						if(write_buffer[0] & 4)
							decrypt2(&write_buffer[2], &write_buffer[2], SIZE_DATA_BUFFER, data_key, 0x00);

						crc = do_crc(write_buffer, 10);

						if(crc == ((write_buffer[10] << 8) | write_buffer[11])) {
							verboselog(1, "-> command: %02x\n", write_buffer[0]);
							verboselog(1, "-> address: %02x\n", write_buffer[1]);
							verboselog(1, "-> data: %02x%02x%02x%02x%02x%02x%02x%02x\n",
										write_buffer[2], write_buffer[3], write_buffer[4], write_buffer[5],
										write_buffer[6], write_buffer[7], write_buffer[8], write_buffer[9]);
							verboselog(1, "-> crc: %02x%02x\n", write_buffer[10], write_buffer[11]);
							switch(write_buffer[0] & 1) {
							case COMMAND_WRITE:
								memcpy(&data[data_offset()], &write_buffer[2], SIZE_DATA_BUFFER);

								/* todo: find out what should be returned. */
								memset(&read_buffer[0] , 0, SIZE_WRITE_BUFFER);
								break;

							case COMMAND_READ:
								/* todo: find out what should be returned. */
								memset(&read_buffer[0], 0, 2);

								switch(write_buffer[1]) {
								case 0xfd: {
									/* TODO: use read/write to talk to the ds2401, which will require a timer. */
									ds2401_device *ds2401 = machine().device<ds2401_device>(ds2401_tag);
									for(int i = 0; i < SIZE_DATA_BUFFER; i++)
										read_buffer[2+i] = ds2401->direct_read(SIZE_DATA_BUFFER-i-1);
									break;
								}
								default:
									memcpy(&read_buffer[2], &data[data_offset()], SIZE_DATA_BUFFER);
									break;
								}

								memcpy(response_key, &write_buffer[2], SIZE_KEY);
								break;
							}
						} else {
							verboselog(0, "bad crc\n");
							/* todo: find out what should be returned. */
							memset(&read_buffer[0], 0xff, 2 );
						}

						verboselog(1, "<- status: %02x%02x\n",
									read_buffer[0], read_buffer[1]);

						verboselog(1, "<- data: %02x%02x%02x%02x%02x%02x%02x%02x\n",
									read_buffer[2], read_buffer[3], read_buffer[4], read_buffer[5],
									read_buffer[6], read_buffer[7], read_buffer[8], read_buffer[9]);

						crc = do_crc(read_buffer, 10);
						read_buffer[10] = crc >> 8;
						read_buffer[11] = crc & 255;

						encrypt(read_buffer, read_buffer, SIZE_READ_BUFFER, response_key, 0xff);

						byte = 0;
						state = STATE_READ_DATA;
					}
					break;
				}

				bit = 0;
				shift = 0;
			}
			break;

		case STATE_READ_DATA:
			if(bit < 8) {
				if(bit == 0) {
					switch(state) {
					case STATE_READ_DATA:
						shift = read_buffer[byte];
						verboselog(2, "<- read_buffer[%d]: %02x\n", byte, shift);
						break;
					}
				}
				sdar = (shift >> 7) & 1;
				shift <<= 1;
				bit++;
			} else {
				bit = 0;
				sdar = false;
				if(!sdaw) {
					verboselog(2, "ack <-\n");
					byte++;
					if(byte == SIZE_READ_BUFFER) {
						byte = 0;
						sdar = true;
						state = STATE_LOAD_COMMAND;
					}
				} else {
					verboselog(2, "nak <-\n");
				}
			}
			break;
		}
	}
}

void zs01_device::sda_1()
{
	if(!cs && scl){
//      state = STATE_STOP;
//      sdar = false;
	}
}

void zs01_device::sda_0()
{
	if(!cs && scl) {
		switch(state) {
		case STATE_STOP:
			verboselog(1, "goto start\n");
			state = STATE_LOAD_COMMAND;
			break;
			//  default:
			//      verboselog(1, "skipped start (default)\n");
			//      break;
		}

		bit = 0;
		byte = 0;
		shift = 0;
		sdar = false;
	}
}

void zs01_device::nvram_read(emu_file &file)
{
	file.read(response_to_reset, SIZE_RESPONSE_TO_RESET);
	file.read(command_key,       SIZE_KEY);
	file.read(data_key,          SIZE_KEY);
	file.read(data,              SIZE_DATA);
}

void zs01_device::nvram_write(emu_file &file)
{
	file.write(response_to_reset, SIZE_RESPONSE_TO_RESET);
	file.write(command_key,       SIZE_KEY);
	file.write(data_key,          SIZE_KEY);
	file.write(data,              SIZE_DATA);
}
