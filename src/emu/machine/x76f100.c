/*
 * x76f100.c
 *
 * Secure SerialFlash
 *
 * The X76F100 is a Password Access Security Supervisor, containing one 896-bit Secure SerialFlash array.
 * Access to the memory array can be controlled by two 64-bit passwords. These passwords protect read and
 * write operations of the memory array.
 *
 */

#include "emu.h"
#include "machine/x76f100.h"

#define VERBOSE_LEVEL 0

inline void ATTR_PRINTF(3,4) x76f100_device::verboselog(int n_level, const char *s_fmt, ...)
{
	if(VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("x76f100 %s %s: %s", tag(), machine().describe_context(), buf);
	}
}

// device type definition
const device_type X76F100 = &device_creator<x76f100_device>;

x76f100_device::x76f100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_secure_serial_flash(mconfig, X76F100, "X76F100", tag, owner, clock)
{
}

void x76f100_device::device_start()
{
	device_secure_serial_flash::device_start();
	save_item(NAME(state));
	save_item(NAME(shift));
	save_item(NAME(bit));
	save_item(NAME(byte));
	save_item(NAME(command));
	save_item(NAME(write_buffer));
	save_item(NAME(response_to_reset));
	save_item(NAME(write_password));
	save_item(NAME(read_password));
	save_item(NAME(data));
}

void x76f100_device::device_reset()
{
	device_secure_serial_flash::device_reset();
	state = STATE_STOP;
	shift = 0;
	bit = 0;
	byte = 0;
	command = 0;
	memset(write_buffer, 0, SIZE_WRITE_BUFFER);
}

void x76f100_device::nvram_default()
{
	// region always wins
	if(m_region)
	{
		// Ensure the size is correct though
		if(m_region->bytes() != SIZE_RESPONSE_TO_RESET+SIZE_WRITE_PASSWORD+SIZE_READ_PASSWORD+SIZE_DATA)
			logerror("x76f100 %s: Wrong region length for initialization data, expected 0x%x, got 0x%x\n",
						tag(),
						SIZE_RESPONSE_TO_RESET+SIZE_WRITE_PASSWORD+SIZE_READ_PASSWORD+SIZE_DATA,
						m_region->bytes());
		else {
			UINT8 *rb = m_region->base();
			int offset = 0;
			memcpy(response_to_reset, rb + offset, SIZE_RESPONSE_TO_RESET); offset += SIZE_RESPONSE_TO_RESET;
			memcpy(write_password,    rb + offset, SIZE_WRITE_PASSWORD); offset += SIZE_WRITE_PASSWORD;
			memcpy(read_password,     rb + offset, SIZE_READ_PASSWORD); offset += SIZE_READ_PASSWORD;
			memcpy(data,              rb + offset, SIZE_DATA); offset += SIZE_DATA;
			return;
		}
	}

	// That chip isn't really usable without the passwords, so bitch
	// if there's no region
	logerror("x76f100 %s: Warning, no default data provided, chip is unusable.\n", tag());
	memset(response_to_reset, 0, SIZE_RESPONSE_TO_RESET);
	memset(write_password,    0, SIZE_WRITE_PASSWORD);
	memset(read_password,     0, SIZE_READ_PASSWORD);
	memset(data,              0, SIZE_DATA);
}

void x76f100_device::cs_0()
{
	/* enable chip */
	state = STATE_STOP;
}

void x76f100_device::cs_1()
{
	/* disable chip */
	state = STATE_STOP;
	/* high impendence? */
	sdar = 0;
}

void x76f100_device::rst_0()
{
}

void x76f100_device::rst_1()
{
	if(!cs) {
		verboselog(1, "goto response to reset\n");
		state = STATE_RESPONSE_TO_RESET;
		bit = 0;
		byte = 0;
	}
}

UINT8 *x76f100_device::password()
{
	if((command & 0xe1) == COMMAND_READ)
	{
		return read_password;
	}

	return write_password;
}

void x76f100_device::password_ok()
{
	if((command & 0xe1) == COMMAND_READ)
	{
		state = STATE_READ_DATA;
	}
	else if((command & 0xe1) == COMMAND_WRITE)
	{
		state = STATE_WRITE_DATA;
	}
	else
	{
		/* TODO: */
	}
}

int x76f100_device::data_offset()
{
	int block_offset = (command >> 1) & 0x0f;

	return block_offset * SIZE_WRITE_BUFFER + byte;
}

void x76f100_device::scl_0()
{
	if(cs)
		return;

	switch(state) {
	case STATE_RESPONSE_TO_RESET:
		if(bit == 0) {
			shift = response_to_reset[byte];
			verboselog(1, "<- response_to_reset[%d]: %02x\n", byte, shift);
		}

		sdar = shift & 1;
		shift >>= 1;
		bit++;

		if(bit == 8) {
			bit = 0;
			byte++;
			if(byte == 4)
				byte = 0;
		}
		break;
	}
}

void x76f100_device::scl_1()
{
	if(cs)
		return;

	switch(state) {
	case STATE_RESPONSE_TO_RESET:
		break;

	case STATE_LOAD_COMMAND:
	case STATE_LOAD_PASSWORD:
	case STATE_VERIFY_PASSWORD:
	case STATE_WRITE_DATA:
		if(bit < 8) {
			verboselog(2, "clock\n");
			shift <<= 1;
			if(sdaw)
				shift |= 1;
			bit++;
		} else {
			switch(state) {
			case STATE_LOAD_COMMAND:
				verboselog(1, "-> command: %02x\n", command);
				sdar = false;
				command = shift;
				/* todo: verify command is valid? */
				state = STATE_LOAD_PASSWORD;
				bit = 0;
				shift = 0;
				break;

			case STATE_LOAD_PASSWORD:
				verboselog(1, "-> password: %02x\n", shift);
				sdar = false;
				write_buffer[byte++] = shift;
				if(byte == SIZE_WRITE_BUFFER)
					state = STATE_VERIFY_PASSWORD;
				bit = 0;
				shift = 0;
				break;

			case STATE_VERIFY_PASSWORD:
				verboselog(1, "-> verify password: %02x\n", shift);
				sdar = false;
				/* todo: this should probably be handled as a command */
				if(shift == COMMAND_ACK_PASSWORD) {
					/* todo: this should take 10ms before it returns ok. */
					if(!memcmp(password(), write_buffer, SIZE_WRITE_BUFFER))
						password_ok();
					else
						sdar = true;
				}
				bit = 0;
				shift = 0;
				break;

			case STATE_WRITE_DATA:
				verboselog(1, "-> data: %02x\n", shift );
				sdar = false;
				write_buffer[byte++] = shift;
				if(byte == SIZE_WRITE_BUFFER) {
					for(byte = 0; byte < SIZE_WRITE_BUFFER; byte++)
						data[data_offset()] = write_buffer[byte];
					byte = 0;
				}
				bit = 0;
				shift = 0;
				break;
			}
		}
		break;

	case STATE_READ_DATA:
		if(bit < 8) {
			if(bit == 0) {
				shift = data[data_offset()];
				verboselog(1, "<- data: %02x\n", shift );
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
			} else {
				verboselog(2, "nak <-\n");
			}
		}
		break;
	}
}

void x76f100_device::sda_0()
{
	if(cs || !scl)
		return;

	switch(state) {
	case STATE_STOP:
		state = STATE_LOAD_COMMAND;
		break;

	case STATE_LOAD_PASSWORD:
		/* todo: this will be the 0xc0 command, but it's not handled as a command yet. */
		break;

	case STATE_READ_DATA:
		//              c->state = STATE_LOAD_ADDRESS;
		break;

	default:
		break;
	}

	bit = 0;
	byte = 0;
	shift = 0;
	sdar = false;
}

void x76f100_device::sda_1()
{
	if(cs || !scl)
		return;

	state = STATE_STOP;
	sdar = false;
}

void x76f100_device::nvram_read(emu_file &file)
{
	file.read(response_to_reset, SIZE_RESPONSE_TO_RESET);
	file.read(write_password,    SIZE_WRITE_PASSWORD);
	file.read(read_password,     SIZE_READ_PASSWORD);
	file.read(data,              SIZE_DATA);
}

void x76f100_device::nvram_write(emu_file &file)
{
	file.write(response_to_reset, SIZE_RESPONSE_TO_RESET);
	file.write(write_password,    SIZE_WRITE_PASSWORD);
	file.write(read_password,     SIZE_READ_PASSWORD);
	file.write(data,              SIZE_DATA);
}
