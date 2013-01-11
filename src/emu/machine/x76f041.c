/*
 * x76f041.c
 *
 * Secure SerialFlash
 *
 * The X76F041 is a Password Access Security Supervisor, containing four 128 x 8 bit SecureFlash arrays.
 * Access can be controlled by three 64-bit programmable passwords, one for read operations, one for write
 * operations and one for device configuration.
 *
 * The data sheet has an incorrect diagrams for sequential read with password, there shouldn't be an extra address after the 0xc0 command.
 *
 */

#include "emu.h"
#include "machine/x76f041.h"

#define VERBOSE_LEVEL 0

inline void ATTR_PRINTF(3,4) x76f041_device::verboselog(int n_level, const char *s_fmt, ...)
{
	if(VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("x76f041 %s %s: %s", tag(), machine().describe_context(), buf);
	}
}

// device type definition
const device_type X76F041 = &device_creator<x76f041_device>;

x76f041_device::x76f041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_secure_serial_flash(mconfig, X76F041, "X76F041", tag, owner, clock)
{
}

void x76f041_device::device_start()
{
	device_secure_serial_flash::device_start();
	save_item(NAME(state));
	save_item(NAME(shift));
	save_item(NAME(bit));
	save_item(NAME(byte));
	save_item(NAME(command));
	save_item(NAME(address));
	save_item(NAME(write_buffer));
	save_item(NAME(response_to_reset));
	save_item(NAME(write_password));
	save_item(NAME(read_password));
	save_item(NAME(configuration_password));
	save_item(NAME(configuration_registers));
	save_item(NAME(data));
}

void x76f041_device::device_reset()
{
	device_secure_serial_flash::device_reset();
	state = STATE_STOP;
	shift = 0;
	bit = 0;
	byte = 0;
	command = 0;
	address = 0;
	memset(write_buffer, 0, SIZE_WRITE_BUFFER);
}

void x76f041_device::nvram_default()
{
	// region always wins
	if(m_region)
	{
		// Ensure the size is correct though
		if(m_region->bytes() != SIZE_RESPONSE_TO_RESET+SIZE_WRITE_PASSWORD+
			SIZE_READ_PASSWORD+SIZE_CONFIGURATION_PASSWORD+SIZE_CONFIGURATION_REGISTERS+SIZE_DATA)
			logerror("X76F041: Wrong region length for initialization data, expected 0x%x, got 0x%x\n",
						SIZE_RESPONSE_TO_RESET+SIZE_WRITE_PASSWORD+
						SIZE_READ_PASSWORD+SIZE_CONFIGURATION_PASSWORD+SIZE_CONFIGURATION_REGISTERS+SIZE_DATA,
						m_region->bytes());
		else {
			UINT8 *rb = m_region->base();
			int offset = 0;
			memcpy(response_to_reset,       rb + offset, SIZE_RESPONSE_TO_RESET); offset += SIZE_RESPONSE_TO_RESET;
			memcpy(write_password,          rb + offset, SIZE_WRITE_PASSWORD); offset += SIZE_WRITE_PASSWORD;
			memcpy(read_password,           rb + offset, SIZE_READ_PASSWORD); offset += SIZE_READ_PASSWORD;
			memcpy(configuration_password,  rb + offset, SIZE_CONFIGURATION_PASSWORD); offset += SIZE_CONFIGURATION_PASSWORD;
			memcpy(configuration_registers, rb + offset, SIZE_CONFIGURATION_REGISTERS); offset += SIZE_CONFIGURATION_REGISTERS;
			memcpy(data,                    rb + offset, SIZE_DATA); offset += SIZE_DATA;
			return;
		}
	}

	// That chip isn't really usable without the passwords, so bitch
	// if there's no region
	logerror("X76F041: Warning, no default data provided, chip is unusable.\n");
	memset(response_to_reset,       0, SIZE_RESPONSE_TO_RESET);
	memset(write_password,          0, SIZE_WRITE_PASSWORD);
	memset(read_password,           0, SIZE_READ_PASSWORD);
	memset(configuration_password,  0, SIZE_CONFIGURATION_PASSWORD);
	memset(configuration_registers, 0, SIZE_CONFIGURATION_REGISTERS);
	memset(data,                    0, SIZE_DATA);
}

void x76f041_device::cs_0()
{
	/* enable chip */
	state = STATE_STOP;
}

void x76f041_device::cs_1()
{
	/* disable chip */
	state = STATE_STOP;
	/* high impendence? */
	sdar = false;
}

void x76f041_device::rst_0()
{
}

void x76f041_device::rst_1()
{
	if(!cs) {
		verboselog(1, "goto response to reset\n");
		state = STATE_RESPONSE_TO_RESET;
		bit = 0;
		byte = 0;
	}
}

UINT8 *x76f041_device::password()
{
	switch(command & 0xe0) {
	case COMMAND_WRITE:
		return write_password;
	case COMMAND_READ:
		return read_password;
	default:
		return configuration_password;
	}
}

void x76f041_device::password_ok()
{
	switch(command & 0xe0) {
	case COMMAND_WRITE:
		state = STATE_WRITE_DATA;
		break;
	case COMMAND_READ:
		state = STATE_READ_DATA;
		break;
	case COMMAND_WRITE_USE_CONFIGURATION_PASSWORD:
		state = STATE_WRITE_DATA;
		break;
	case COMMAND_READ_USE_CONFIGURATION_PASSWORD:
		state = STATE_READ_DATA;
		break;
	case COMMAND_CONFIGURATION:
		switch( address ) {
		case CONFIGURATION_PROGRAM_WRITE_PASSWORD:
			break;
		case CONFIGURATION_PROGRAM_READ_PASSWORD:
			break;
		case CONFIGURATION_PROGRAM_CONFIGURATION_PASSWORD:
			break;
		case CONFIGURATION_RESET_WRITE_PASSWORD:
			break;
		case CONFIGURATION_RESET_READ_PASSWORD:
			break;
		case CONFIGURATION_PROGRAM_CONFIGURATION_REGISTERS:
			state = STATE_WRITE_CONFIGURATION_REGISTERS;
			byte = 0;
			break;
		case CONFIGURATION_READ_CONFIGURATION_REGISTERS:
			state = STATE_READ_CONFIGURATION_REGISTERS;
			byte = 0;
			break;
		case CONFIGURATION_MASS_PROGRAM:
			break;
		case CONFIGURATION_MASS_ERASE:
			break;
		default:
			break;
		}
	}
}

void x76f041_device::load_address()
{
	/* todo: handle other bcr bits */
	int bcr;

	address = shift;

	verboselog(1, "-> address: %02x\n", address);

	if(!(command & 1 ))
		bcr = configuration_registers[CONFIG_BCR1];
	else
		bcr = configuration_registers[CONFIG_BCR2];

	if(address & 0x80)
		bcr >>= 4;

	if(((command & 0xe0) == COMMAND_READ && (bcr & BCR_Z) && (bcr & BCR_T)) ||
		((command & 0xe0) == COMMAND_WRITE && (bcr & BCR_Z))) {
		/* todo: find out when this is really checked. */
		verboselog(1, "command not allowed\n");
		state = STATE_STOP;
		sdar = false;

	} else if(((command & 0xe0) == COMMAND_WRITE && !(bcr & BCR_X)) ||
				((command & 0xe0) == COMMAND_READ && !(bcr & BCR_Y))) {
		verboselog(1, "password not required\n");
		password_ok();

	} else {
		verboselog(1, "send password\n");
		state = STATE_LOAD_PASSWORD;
		byte = 0;
	}
}

int x76f041_device::data_offset()
{
	int block_offset = ((command & 1) << 8) + address;

	// TODO: confirm block_start doesn't wrap.

	return (block_offset & 0x180) | ((block_offset + byte) & 0x7f);
}

void x76f041_device::scl_0()
{
	if(!cs) {
		switch(state) {
		case STATE_RESPONSE_TO_RESET:
			sdar = (response_to_reset[byte] >> bit) & 1;
			verboselog(2, "in response to reset %d (%d/%d)\n", sdar, byte, bit);
			bit++;
			if(bit == 8) {
				bit = 0;
				byte++;
				if( byte == 4 )
					byte = 0;
			}
			break;
		}
	}
}

void x76f041_device::scl_1()
{
	if(!cs) {
		switch(state) {
		case STATE_STOP:
			break;
		case STATE_RESPONSE_TO_RESET:
			break;
		case STATE_LOAD_COMMAND:
		case STATE_LOAD_ADDRESS:
		case STATE_LOAD_PASSWORD:
		case STATE_VERIFY_PASSWORD:
		case STATE_WRITE_DATA:
		case STATE_WRITE_CONFIGURATION_REGISTERS:
			if(bit < 8) {
				verboselog(2, "clock\n");
				shift <<= 1;
				if(sdaw)
					shift |= 1;
				bit++;
			} else {
				sdar = false;

				switch(state) {
				case STATE_LOAD_COMMAND:
					command = shift;
					verboselog(1, "-> command: %02x\n", command);
					/* todo: verify command is valid? */
					state = STATE_LOAD_ADDRESS;
					break;
				case STATE_LOAD_ADDRESS:
					load_address();
					break;
				case STATE_LOAD_PASSWORD:
					verboselog(1, "-> password: %02x\n", shift );
					write_buffer[byte++] = shift;
					if(byte == SIZE_WRITE_BUFFER)
						state = STATE_VERIFY_PASSWORD;
					break;
				case STATE_VERIFY_PASSWORD:
					verboselog(1, "-> verify password: %02x\n", shift);
					/* todo: this should probably be handled as a command */
					if(shift == 0xc0) {
						/* todo: this should take 10ms before it returns ok. */
						if(!memcmp(password(), write_buffer, SIZE_WRITE_BUFFER))
							password_ok();
						else
							sdar = true;
					}
					break;
				case STATE_WRITE_DATA:
					verboselog(1, "-> data: %02x\n", shift);
					write_buffer[byte++] = shift;
					if(byte == SIZE_WRITE_BUFFER) {
						for(byte = 0; byte < SIZE_WRITE_BUFFER; byte++)
							data[data_offset()] = write_buffer[byte];
						byte = 0;
						verboselog(1, "data flushed\n");
					}
					break;
				case STATE_WRITE_CONFIGURATION_REGISTERS:
					verboselog(1, "-> configuration register: %02x\n", shift);
					/* todo: write after all bytes received? */
					configuration_registers[byte++] = shift;
					if(byte == SIZE_CONFIGURATION_REGISTERS)
						byte = 0;
					break;
				}

				bit = 0;
				shift = 0;
			}
			break;
		case STATE_READ_DATA:
		case STATE_READ_CONFIGURATION_REGISTERS:
			if(bit < 8) {
				if(bit == 0) {
					switch(state) {
					case STATE_READ_DATA:
						shift = data[data_offset()];
						verboselog(1, "<- data: %02x\n", shift);
						break;
					case STATE_READ_CONFIGURATION_REGISTERS:
						shift = configuration_registers[byte & 7];
						verboselog(1, "<- configuration register: %02x\n", shift );
						break;
					}
				}
				sdar = ( shift >> 7 ) & 1;
				shift <<= 1;
				bit++;
			} else {
				bit = 0;
				sdar = false;
				if(!sdaw) {
					verboselog(2, "ack <-\n");
					byte++;
				} else {
					verboselog(2, "nak <-\n");
				}
			}
			break;
		}
	}
}

void x76f041_device::sda_0()
{
	if(!cs && scl) {
		switch(state) {
		case STATE_STOP:
			verboselog(1, "goto start (1)\n");
			state = STATE_LOAD_COMMAND;
			break;
		case STATE_LOAD_PASSWORD:
			/* todo: this will be the 0xc0 command, but it's not handled as a command yet. */
			verboselog(1, "goto start (2)\n");
			break;
		case STATE_READ_DATA:
			verboselog(1, "goto load address\n");
			state = STATE_LOAD_ADDRESS;
			break;
		default:
			verboselog(1, "skipped start (default)\n");
			break;
		}

		bit = 0;
		byte = 0;
		shift = 0;
		sdar = false;
	}
}

void x76f041_device::sda_1()
{
	if(!cs && scl) {
		verboselog(1, "goto stop\n");
		state = STATE_STOP;
		sdar = false;
	}
}

void x76f041_device::nvram_read(emu_file &file)
{
	file.read(response_to_reset, SIZE_RESPONSE_TO_RESET);
	file.read(write_password, SIZE_WRITE_PASSWORD);
	file.read(read_password, SIZE_READ_PASSWORD);
	file.read(configuration_password, SIZE_CONFIGURATION_PASSWORD);
	file.read(configuration_registers, SIZE_CONFIGURATION_REGISTERS);
	file.read(data, SIZE_DATA);
}

void x76f041_device::nvram_write(emu_file &file)
{
	file.write(response_to_reset, SIZE_RESPONSE_TO_RESET);
	file.write(write_password, SIZE_WRITE_PASSWORD);
	file.write(read_password, SIZE_READ_PASSWORD);
	file.write(configuration_password, SIZE_CONFIGURATION_PASSWORD);
	file.write(configuration_registers, SIZE_CONFIGURATION_REGISTERS);
	file.write(data, SIZE_DATA);
}
