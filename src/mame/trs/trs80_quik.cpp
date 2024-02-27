// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Quickload code for TRS-80 /CMD files

***************************************************************************/

#include "emu.h"
#include "trs80_quik.h"

#include "cpu/z80/z80.h"

#include <tuple>

#define VERBOSE 1
#include "logmacro.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

enum : uint8_t
{
	CMD_TYPE_OBJECT_CODE                            = 0x01,
	CMD_TYPE_TRANSFER_ADDRESS                       = 0x02,
	CMD_TYPE_END_OF_PARTITIONED_DATA_SET_MEMBER     = 0x04,
	CMD_TYPE_LOAD_MODULE_HEADER                     = 0x05,
	CMD_TYPE_PARTITIONED_DATA_SET_HEADER            = 0x06,
	CMD_TYPE_PATCH_NAME_HEADER                      = 0x07,
	CMD_TYPE_ISAM_DIRECTORY_ENTRY                   = 0x08,
	CMD_TYPE_END_OF_ISAM_DIRECTORY_ENTRY            = 0x0a,
	CMD_TYPE_PDS_DIRECTORY_ENTRY                    = 0x0c,
	CMD_TYPE_END_OF_PDS_DIRECTORY_ENTRY             = 0x0e,
	CMD_TYPE_YANKED_LOAD_BLOCK                      = 0x10,
	CMD_TYPE_COPYRIGHT_BLOCK                        = 0x1f
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

DEFINE_DEVICE_TYPE(TRS80_QUICKLOAD, trs80_quickload_device, "trs80_quickload", "TRS-80 /CMD quickload")

trs80_quickload_device::trs80_quickload_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: snapshot_image_device(mconfig, TRS80_QUICKLOAD, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
{
	set_load_callback(load_delegate(*this, FUNC(trs80_quickload_device::quickload_cb)));
}


// TODO: If you get "Attempting to write outside of RAM" enough times in succession, MAME will exit unexpectedly (no error).
QUICKLOAD_LOAD_MEMBER(trs80_quickload_device::quickload_cb)
{
	util::core_file &file = image_core_file();
	address_space &program = m_maincpu->space(AS_PROGRAM);

	uint8_t type, length;
	uint8_t data[0x100];
	uint8_t addr[2];
	void *ptr;

	std::error_condition err;
	size_t actual;
	while (true)
	{
		std::tie(err, actual) = read(file, &type, 1);
		if (actual != 1)
			break;
		std::tie(err, actual) = read(file, &length, 1);
		if (actual != 1)
			break;

		switch (type)
		{
		case CMD_TYPE_OBJECT_CODE:  // 01 - block of data
			{
				length -= 2;
				u16 block_length = length ? length : 256;
				std::tie(err, actual) = read(file, &addr, 2);
				if (actual != 2)
				{
					logerror("/CMD error reading address of data block\n");
					return std::make_pair(err ? err : image_error::INVALIDLENGTH, std::string());
				}
				u16 address = (addr[1] << 8) | addr[0];
				LOG("/CMD object code block: address %04x length %u\n", address, block_length);
				ptr = program.get_write_ptr(address);
				if (!ptr)
				{
					return std::make_pair(
							image_error::INVALIDIMAGE,
							util::string_format("Object code block at address %04x is outside RAM", address));
				}
				std::tie(err, actual) = read(file, ptr, block_length);
				if (actual != block_length)
				{
					logerror("/CMD error reading data block at address %04x\n", address);
					return std::make_pair(err ? err : image_error::INVALIDLENGTH, std::string());
				}
			}
			break;

		case CMD_TYPE_TRANSFER_ADDRESS: // 02 - go address
			{
				std::tie(err, actual) = read(file, &addr, 2);
				if (actual != 2)
				{
					logerror("/CMD error reading transfer address\n");
					return std::make_pair(err ? err : image_error::INVALIDLENGTH, std::string());
				}
				u16 address = (addr[1] << 8) | addr[0];
				LOG("/CMD transfer address %04x\n", address);
				m_maincpu->set_state_int(Z80_PC, address);
			}
			return std::make_pair(std::error_condition(), std::string());

		case CMD_TYPE_LOAD_MODULE_HEADER: // 05 - name
			std::tie(err, actual) = read(file, &data, length);
			if (actual != length)
			{
				logerror("/CMD error reading block type %02x\n", type);
				return std::make_pair(err ? err : image_error::INVALIDLENGTH, std::string());
			}
			LOG("/CMD load module header '%s'\n", data);
			break;

		case CMD_TYPE_COPYRIGHT_BLOCK: // 1F - copyright info           std::tie(err, actual) = read(file, &data, length);
			if (actual != length)
			{
				logerror("/CMD error reading block type %02x\n", type);
				return std::make_pair(err ? err : image_error::INVALIDLENGTH, std::string());
			}
			LOG("/CMD copyright block '%s'\n", data);
			break;

		default:
			std::tie(err, actual) = read(file, &data, length);
			if (actual != length)
			{
				logerror("/CMD error reading block type %02x\n", type);
				return std::make_pair(err ? err : image_error::INVALIDLENGTH, std::string());
			}
			logerror("/CMD unsupported block type %u!\n", type);
			return std::make_pair(
					image_error::INVALIDIMAGE,
					util::string_format("Unsupported or invalid block type %u", type));
		}
	}

	return std::make_pair(err, std::string());
}
