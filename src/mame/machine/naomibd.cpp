// license:BSD-3-Clause
// copyright-holders:ElSemi

#include "emu.h"
#include "naomibd.h"
#include "machine/x76f100.h"

/*
    Naomi ROM board info from ElSemi:

    NAOMI_ROM_OFFSETH = 0x5f7000,
    NAOMI_ROM_OFFSETL = 0x5f7004,
    NAOMI_ROM_DATA = 0x5f7008,
    NAOMI_DMA_OFFSETH = 0x5f700C,
    NAOMI_DMA_OFFSETL = 0x5f7010,
    NAOMI_DMA_COUNT = 0x5f7014,
    NAOMI_COMM_OFFSET = 0x5F7050,
    NAOMI_COMM_DATA = 0x5F7054,
    NAOMI_BOARDID_WRITE = 0x5F7078,
    NAOMI_BOARDID_READ = 0x5F707C,
    each port is 16 bit wide, to access the rom in PIO mode, just set an offset in ROM_OFFSETH/L and read from ROM_DATA, each access reads 2 bytes and increases the offset by 2.

    the BOARDID regs access the password protected eeprom in the game board. the main board eeprom is read through port 0x1F800030

    To access the board using DMA, use the DMA_OFFSETL/H. DMA_COUNT is in units of 0x20 bytes. Then trigger a GDROM DMA request.

    Cartridge protection info from Deunan Knute:

    NAOMI cart can hold up to 512MB of data, so the highest bits are used for other, dark and scary purposes.
    I call those bits "mode selector".

    First it's important to note that DMA and PIO seem to have separate address counters, as well as separate mode selector registers.

    * bit 31 (mode bit 3) is auto-advance bit
    When set to one the address will be automatically incremented when data is read, so you need only set it once and can just keep polling
    the PIO port. When zero it will stay on current address.  Now this works exactly the same for DMA, and even if DMA engine is 32-byte
    per block it will repeatedly read only the first 16-bit word.

    * bit 30 (mode bit 2) is most often as special mode switch
    DMA transfer with this bit set will hang. PIO will return semi-random data (floating bus?). So one function of that bit is "disable".
    PIO read will return all ones if DMA mode has this bit cleared, so it seems you can do either PIO or DMA but not both at the same time.
    In other words, disable DMA once before using PIO (most games using both access types do that when the DMA terminates).
    This bit is also used to reset the chip's internal protection mechanism on "Oh! My Goddess" to a known state.
    "M4" type carts: ROM_OFFSET bit 30 enables data decryption, for both PIO and DMA.

    * bit 29 (mode bit 1) is "M1" compression bit on Actel carts, other functions on others
    It's actually the opposite, when set the addressing is following the chip layout and when cleared the protection chip will have it's fun
    doing a decompression + XOR on the data for Actel carts.
    "M2" type carts: ROM size/mapping select, 0 - 4MB ROM-mode, 1 - 8MB ROM mode. ROM_OFFSET bit 29 select cart mapping for both PIO and DMA, DMA_OFFSET bit 29 looks have no any effect.
    "M4" type carts: no effect

    Normal address starts with 0xa0000000 to enable auto-advance and standard addressing mode.
*/

DEVICE_ADDRESS_MAP_START(submap, 16, naomi_board)
	AM_RANGE(0x00, 0x01) AM_WRITE(rom_offseth_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(rom_offsetl_w)
	AM_RANGE(0x04, 0x05) AM_READWRITE(rom_data_r, rom_data_w)
	AM_RANGE(0x06, 0x07) AM_WRITE(dma_offseth_w)
	AM_RANGE(0x08, 0x09) AM_WRITE(dma_offsetl_w)
	AM_RANGE(0x0a, 0x0b) AM_WRITE(dma_count_w)
	AM_RANGE(0x3c, 0x3d) AM_WRITE(boardid_w)
	AM_RANGE(0x3e, 0x3f) AM_READ(boardid_r)

	AM_RANGE(0x00, 0xff) AM_READ(default_r)
ADDRESS_MAP_END

naomi_board::naomi_board(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: naomi_g1_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
	eeprom_tag = nullptr;
	rombdid_tag = nullptr;
}

void naomi_board::static_set_eeprom_tag(device_t &device, const char *_eeprom_tag, const char *_actel_tag)
{
	naomi_board &dev = downcast<naomi_board &>(device);
	dev.eeprom_tag = _eeprom_tag;
	dev.rombdid_tag = _actel_tag;
}


void naomi_board::device_start()
{
	naomi_g1_device::device_start();

	save_item(NAME(rom_offset));
	save_item(NAME(dma_offset));
	save_item(NAME(dma_count));
	save_item(NAME(dma_cur_offset));
	save_item(NAME(pio_ready));
	save_item(NAME(dma_ready));

	if(eeprom_tag)
		eeprom = machine().device<x76f100_device>(eeprom_tag);
	else
		eeprom = nullptr;
}

void naomi_board::device_reset()
{
	naomi_g1_device::device_reset();
	rom_offset = 0;
	dma_offset = 0;
	dma_cur_offset = 0;
	pio_ready = false;
	dma_ready = false;
}

void naomi_board::dma_get_position(UINT8 *&base, UINT32 &limit, bool to_mainram)
{
	if(!to_mainram) {
		base = nullptr;
		limit = 0;
		return;
	}

	if(!dma_ready) {
		if(!(dma_offset & 0x80000000))
			throw emu_fatalerror("NAOMI BOARD: Unsupported, non-incrementing DMA.\n");
		board_setup_address(dma_offset, true);
		dma_cur_offset = 0;
		dma_ready = true;
	}

	board_get_buffer(base, limit);
	UINT32 blimit = 0x20*dma_count - dma_cur_offset;
	if(0 && limit > blimit)
		limit = blimit;
}

void naomi_board::dma_advance(UINT32 size)
{
	dma_cur_offset += size;
	board_advance(size);
}

WRITE16_MEMBER(naomi_board::rom_offseth_w)
{
	rom_offset = (rom_offset & 0x0000ffff) | (data << 16);
	pio_ready = false;
}

WRITE16_MEMBER(naomi_board::rom_offsetl_w)
{
	rom_offset = (rom_offset & 0xffff0000) | data;
	pio_ready = false;
}

READ16_MEMBER(naomi_board::rom_data_r)
{
	if(!pio_ready) {
		board_setup_address(rom_offset, false);
		pio_ready = true;
	}

	UINT8 *buffer;
	UINT32 size;
	UINT16 res;
	board_get_buffer(buffer, size);
	assert(size > 1);
	res = buffer[0] | (buffer[1] << 8);
	if(rom_offset & 0x80000000)
		board_advance(2);
	return res;
}

WRITE16_MEMBER(naomi_board::rom_data_w)
{
	board_write(rom_offset, data);

	if(rom_offset & 0x80000000)
		rom_offset += 2;
}

WRITE16_MEMBER(naomi_board::dma_offseth_w)
{
	dma_offset = (dma_offset & 0x0000ffff) | (data << 16);
	dma_ready = false;
}

WRITE16_MEMBER(naomi_board::dma_offsetl_w)
{
	dma_offset = (dma_offset & 0xffff0000) | data;
	dma_ready = false;
}

WRITE16_MEMBER(naomi_board::dma_count_w)
{
	dma_count = data;
}

WRITE16_MEMBER(naomi_board::boardid_w)
{
	eeprom->write_cs((data >> 2) & 1);
	eeprom->write_rst((data >> 3) & 1);
	eeprom->write_scl((data >> 1) & 1);
	eeprom->write_sda((data >> 0) & 1);
}

READ16_MEMBER(naomi_board::boardid_r)
{
	return eeprom->read_sda() << 15;
}

READ16_MEMBER(naomi_board::default_r)
{
	logerror("NAOMIBD: unmapped read at %02x\n", offset);
	return 0xffff;
}

void naomi_board::board_write(offs_t offset, UINT16 data)
{
	logerror("NAOMIBD: unhandled board write %08x, %04x\n", offset, data);
}
