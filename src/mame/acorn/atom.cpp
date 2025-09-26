// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Nigel Barnes
/***************************************************************************

Acorn Atom:
  http://chrisacorns.computinghistory.org.uk/Computers/Atom.html
  http://chrisacorns.computinghistory.org.uk/Computers/Busicomputers_Prophet2.html
  http://chrisacorns.computinghistory.org.uk/Computers/Busicomputers_Prophet3.html

Memory map.

CPU: 6502
        0000-00ff Zero page
        0100-01ff Stack
        0200-1fff RAM (expansion)
        0a00-0a04 FDC 8271
        2000-21ff RAM (dos catalogue buffer)
        2200-27ff RAM (dos seq file buffer)
        2800-28ff RAM (float buffer)
        2900-7fff RAM (text RAM)
        8000-97ff VDG 6847
        9800-9fff RAM (expansion)
        a000-afff ROM (extension)
        b000-b003 PPIA 8255
        b003-b7ff NOP
        b800-bbff VIA 6522
        bc00-bfdf NOP
        bfe0-bfe2 MOUSE - extension??
        bfe3-bfff NOP
        c000-cfff ROM (basic)
        d000-dfff ROM (float)
        e000-efff ROM (dos)
        f000-ffff ROM (kernel)

Video:      MC6847

Sound:      Buzzer
Floppy:     FDC8271

Hardware:   PPIA 8255

    output  b000    0 - 3 keyboard row, 4 - 7 graphics mode
            b002    0 cas output, 1 enable 2.4kHz, 2 buzzer, 3 colour set

    input   b001    0 - 5 keyboard column, 6 CTRL key, 7 SHIFT key
            b002    4 2.4kHz input, 5 cas input, 6 REPT key, 7 60 Hz input

            VIA 6522


    DOS:

    The original location of the 8271 memory mapped registers is 0xa00-0x0a04.
    (This is the memory range assigned by Acorn in their design.)

    This is in the middle of the area for expansion RAM. Many Atom owners
    thought this was a bad design and have modified their Atom's and dos rom
    to use a different memory area.

    The atom driver in MAME uses the original memory area.

    ---

    The Econet card for the ATOM is decoded on the ATOM PCB at memory address B400 (hex). The Econet Eurocard has decoding circuits on it which select memory address 1940 (hex).
    There are then five significant addresses above these bases which contain the following registers: -

                ATOM card   Eurocard
    6854    register 1  B400        1940
    6854    register 2  B401        1941
    6854    register 3  B402        1942
    6854    Tx/Rx Data reg. B403        1943
    Station identification  B404        1944

    Station identification

    The identity number of each station is set up in hardware by links to IC 8. IC 8 is an octal buffer which when enabled feeds the cards station ID to the computer bus.
    Each link codes a bit in an eight bit binary number allowing any station ID in the range 0 to 255 to be set up. if a link is left open then the bit is a one, when a
    link is made the bit is a zero. Hence all links open corresponds to station ID 255, and all links made to station ID 0. Each station must have a unique identity and
    some identities are associated with specific functions on the network. Station ID zero is reserved for broadcast signals and should not be used. Station ID 255 is
    reserved at present for the file server, and 235 for the printer server. Wire links must be soldered to each network station card during installation, a suggested
    scheme for number allocation is to number normal user stations from one upwards and to number special stations and servers from 255 downwards.


***************************************************************************/

/*

    TODO:

    - display should be monochrome -- Should be optional, Acorn produced a Colour Card, and there is
        at least one after market Colour card.
    - tap files
    - mouse
    - color card
    - CP/M card
    - econet
    - Busicomputers Prophet 2
        * The Shift and Return keys are orange and the Return key is large,
        * There is a MODE switch to the top right of the keyboard,
        * There is a VIDEO port in addition to the TV output,
        * An Acorn AtomCalc ROM PCB is installed (is this standard on the Prophet2 or an upgrade?),
        * An Acorn 32K dynamic RAM card is installed,
        * A 5v DC input is added in addition to the standard power in (but this may be a later upgrade),
        * The Utility ROM is labelled P2/FP is installed

    - Cassette UEF not working - all data blocks overwrite each other at 0000 in ram
    - move internal expansion cards (BBC Basic, RAMROM, etc.) to slot devices.

*/

#include "emu.h"

#include "bus/acorn/bus.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
//#include "video/es5700.h"
#include "video/mc6847.h"

#include "formats/atom_dsk.h"
#include "formats/atom_tap.h"
#include "formats/imageutl.h"
#include "formats/uef_cas.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "multibyte.h"

#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

class atom_state : public driver_device
{
public:
	atom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_text_ram(*this, "textram", 0x1400, ENDIANNESS_LITTLE)
		, m_video_ram(*this, "videoram", 0x1800, ENDIANNESS_LITTLE)
		, m_vdg(*this, "vdg")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_bus(*this, "bus")
		, m_io_keyboard(*this, "Y%u", 0U)
		, m_cfg_ram(*this, "CFG_RAM")
	{ }

	void atom_base(machine_config &config);
	void atom(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);

protected:
	required_device<cpu_device> m_maincpu;
	memory_share_creator<uint8_t> m_text_ram;
	memory_share_creator<uint8_t> m_video_ram;
	required_device<mc6847_base_device> m_vdg;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	optional_device<acorn_bus_device> m_bus;
	required_ioport_array<12> m_io_keyboard;
	required_ioport m_cfg_ram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t textram_r(offs_t offset);
	void textram_w(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);

	void ppi_pa_w(uint8_t data);
	uint8_t ppi_pb_r();
	uint8_t ppi_pc_r();
	void ppi_pc_w(uint8_t data);
	uint8_t vdg_videoram_r(offs_t offset);

	/* keyboard state */
	u8 m_keylatch = 0U;

	/* cassette state */
	bool m_hz2400 = 0;
	bool m_pc0 = 0;
	bool m_pc1 = 0;

	static void floppy_formats(format_registration &fr);
	void cassette_output_tick(int state);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void atom_mem(address_map &map) ATTR_COLD;
};


class atombbc_state : public atom_state
{
public:
	atombbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: atom_state(mconfig, type, tag)
		, m_mode(*this, "mode")
	{
	}

	void atombbc(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	memory_view m_mode;

	void atombbc_mem(address_map &map) ATTR_COLD;
};


class prophet_state : public atom_state
{
public:
	prophet_state(const machine_config &mconfig, device_type type, const char *tag)
		: atom_state(mconfig, type, tag)
		, m_cfg_mode(*this, "CFG_MODE")
	{
	}

	void prophet2(machine_config &config);
	void atomes(machine_config &config);

protected:
	void machine_reset() override;

private:
	required_ioport m_cfg_mode;

	void prophet_mem(address_map &map) ATTR_COLD;
	void atomes_mem(address_map &map) ATTR_COLD;
};


class atomrr_state : public atom_state
{
public:
	atomrr_state(const machine_config &mconfig, device_type type, const char *tag)
		: atom_state(mconfig, type, tag)
		, m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE)
		, m_flash(*this, "flash")
		, m_mode(*this, "mode")
		, m_jumper(*this, "JUMPER")
		, m_rom_latch(0)
		, m_switch_latch(0)
	{
	}

	void atomrr(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( clock_boost );

protected:
	void machine_start() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_ram;
	required_region_ptr<uint8_t> m_flash;
	memory_view m_mode;
	required_ioport m_jumper;

	void atomrr_mem(address_map &map) ATTR_COLD;

	uint8_t dskram_r(offs_t offset);
	void dskram_w(offs_t offset, uint8_t data);
	uint8_t topram_r(offs_t offset);
	void topram_w(offs_t offset, uint8_t data);
	uint8_t dskrom_r(offs_t offset);
	uint8_t extram_r(offs_t offset);
	void extram_w(offs_t offset, uint8_t data);
	uint8_t extram1_r(offs_t offset);
	void extram1_w(offs_t offset, uint8_t data);
	uint8_t extram2_r(offs_t offset);
	void extram2_w(offs_t offset, uint8_t data);

	uint8_t switch_r();
	void switch_w(uint8_t data);

	uint8_t m_rom_latch = 0;
	uint8_t m_switch_latch = 0;
};


/*-------------------------------------------------
    QUICKLOAD_LOAD_MEMBER(atom_state::quickload_cb)
-------------------------------------------------*/

QUICKLOAD_LOAD_MEMBER(atom_state::quickload_cb)
{
	/*

	    The format for the .ATM files is as follows:

	    Offset Size     Description
	    ------ -------- -----------------------------------------------------------
	    0000h  16 BYTEs ATOM filename (if less than 16 BYTEs, rest is 00h bytes)
	    0010h  WORD     Start address for load
	    0012h  WORD     Execution address
	    0014h  WORD     Size of data in BYTEs
	    0016h  Size     Data

	*/

	uint8_t header[0x16] = { 0 };

	image.fread(header, 0x16);

	uint16_t start_address = get_u16le(&header[0x10]);
	uint16_t run_address = get_u16le(&header[0x12]);
	uint16_t size = get_u16le(&header[0x14]);

	char pgmname[17];
	for (int i = 0; i < 16; i++)
		pgmname[i] = header[i];
	pgmname[16] = 0;
	LOG("ATM filename: %s\n", pgmname);
	LOG("ATM start address: %04X\n", start_address);
	LOG("ATM run address: %04X\n", run_address);
	LOG("ATM size: %04X\n", size);

	address_space &space = m_maincpu->space(AS_PROGRAM);
	for (int i = 0; i < size; i++)
	{
		uint8_t data;
		image.fread(&data, 1);
		space.write_byte(start_address + i, data);
	}

	if (run_address == 0xc2b2)
		space.write_word(0x0c, start_address + size);
	else
		m_maincpu->set_state_int(M6502_PC, run_address);   // if not basic, autostart program (set_pc doesn't work)

	return std::make_pair(std::error_condition(), std::string());
}

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

uint8_t atom_state::textram_r(offs_t offset)
{
	uint16_t textram_size = BIT(m_cfg_ram->read(), 0, 4) * 1024;

	if (offset < textram_size)
		return m_text_ram[offset];
	else
		return 0x00;
}

void atom_state::textram_w(offs_t offset, uint8_t data)
{
	uint16_t textram_size = BIT(m_cfg_ram->read(), 0, 4) * 1024;

	if (offset < textram_size)
		m_text_ram[offset] = data;
}


uint8_t atom_state::videoram_r(offs_t offset)
{
	uint16_t videoram_size = BIT(m_cfg_ram->read(), 4, 4) * 1024;

	if (offset < videoram_size)
		return m_video_ram[offset];
	else
		return 0x00;
}

void atom_state::videoram_w(offs_t offset, uint8_t data)
{
	uint16_t videoram_size = BIT(m_cfg_ram->read(), 4, 4) * 1024;

	if (offset < videoram_size)
		m_video_ram[offset] = data;
}


uint8_t atomrr_state::dskram_r(offs_t offset)
{
	if (BIT(m_switch_latch, 1))
		return m_ram[offset | 0x0a00];
	else
		return m_bus->read(offset | 0x0a00);
}

void atomrr_state::dskram_w(offs_t offset, uint8_t data)
{
	if (BIT(m_switch_latch, 1))
		m_ram[offset | 0x0a00] = data;
	else
		m_bus->write(offset | 0x0a00, data);
}


uint8_t atomrr_state::dskrom_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (BIT(m_switch_latch, 2))
	{
		data = m_flash[offset | 0x10000];
	}
	else
	{
		data = m_flash[offset | 0x14000];

		// external pl6/7 buffers enabled
		if ((offset & 0x3000) == 0x2000)
			data = m_bus->read(offset | 0xe000);
	}

	return data;
}


uint8_t atomrr_state::topram_r(offs_t offset)
{
	if (!BIT(m_switch_latch, 0))
		return m_ram[offset | 0x7000];
	else
		return 0xff;
}

void atomrr_state::topram_w(offs_t offset, uint8_t data)
{
	if (!BIT(m_switch_latch, 0))
		m_ram[offset | 0x7000] = data;
}


uint8_t atomrr_state::extram_r(offs_t offset)
{
	if (BIT(m_switch_latch, 0))
		return m_ram[offset | 0x7000];
	else
		return m_flash[offset | (m_rom_latch << 12)];
}

void atomrr_state::extram_w(offs_t offset, uint8_t data)
{
	if (BIT(m_switch_latch, 0))
		m_ram[offset | 0x7000] = data;
}


uint8_t atomrr_state::extram1_r(offs_t offset)
{
	if (BIT(m_switch_latch, 0))
		return m_ram [offset | 0x6000];
	else
		return m_flash[offset | (m_rom_latch << 12)];
}

void atomrr_state::extram1_w(offs_t offset, uint8_t data)
{
	if (BIT(m_switch_latch, 0))
		m_ram[offset | 0x6000] = data;
}


uint8_t atomrr_state::extram2_r(offs_t offset)
{
	if (BIT(m_switch_latch, 1))
		return m_ram[offset | 0x7000];
	else
		return m_flash[offset | 0x19000];
}

void atomrr_state::extram2_w(offs_t offset, uint8_t data)
{
	if (BIT(m_switch_latch, 1))
		m_ram[offset | 0x7000] = data;
}

uint8_t atomrr_state::switch_r()
{
	return 0xb0 | m_switch_latch;
}

void atomrr_state::switch_w(uint8_t data)
{
	/*

	    bit     description
	            In Atom Mode:
	    0       ExtRamEn controls whether #A000 bank0 is ROM (0) or RAM (1)
	    1       DskRamEn controls whether #0A00 is RAM
	    2       DskRomEn controls which OS set is selected
	    3       Mode = 0 (AtomMode)
	            In Beeb Mode:
	    0       ExtRamEn1 controls whether #6000 bank 0 is ROM (0) or RAM (1)
	    1       ExtRamEn2 controls whether #7000 is ROM (0) or RAM (1)
	    2       unused
	    3       Mode = 1 (BeebMode)

	*/

	m_switch_latch = data & 0x0f;

	m_mode.select(BIT(data, 3));
}


/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( atom_mem )
-------------------------------------------------*/

void atom_state::atom_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(m_bus, FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x0000, 0x03ff).ram();
	map(0x2800, 0x3bff).rw(FUNC(atom_state::textram_r), FUNC(atom_state::textram_w));
	map(0x8000, 0x97ff).rw(FUNC(atom_state::videoram_r), FUNC(atom_state::videoram_w));
	map(0xa000, 0xafff).r("romslot", FUNC(generic_slot_device::read_rom));
	map(0xb000, 0xb003).mirror(0x3fc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb800, 0xb80f).mirror(0x3f0).m("via", FUNC(via6522_device::map));
	map(0xc000, 0xdfff).rom().region("maincpu", 0x0000);
	map(0xf000, 0xffff).rom().region("maincpu", 0x3000);
}

/*-------------------------------------------------
    ADDRESS_MAP( atombbc_mem )
-------------------------------------------------*/

void atombbc_state::atombbc_mem(address_map &map)
{
	map(0x0000, 0xffff).view(m_mode);
	/* Atom */
	m_mode[0](0x0000, 0xffff).rw(m_bus, FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	m_mode[0](0x0000, 0x03ff).ram().share("ram00");
	m_mode[0](0x2000, 0x33ff).ram().share("textram");
	m_mode[0](0x3400, 0x37ff).ram().share("ram34");
	m_mode[0](0x3800, 0x3bff).ram().share("ram38");
	m_mode[0](0x8000, 0x97ff).rw(FUNC(atombbc_state::videoram_r), FUNC(atombbc_state::videoram_w));
	m_mode[0](0xa000, 0xafff).r("romslot", FUNC(generic_slot_device::read_rom));
	m_mode[0](0xb000, 0xb003).mirror(0x3fc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	m_mode[0](0xb800, 0xb80f).mirror(0x3f0).m("via", FUNC(via6522_device::map));
	m_mode[0](0xc000, 0xdfff).rom().region("maincpu", 0x0000);
	m_mode[0](0xf000, 0xffff).rom().region("maincpu", 0x3000);
	/* BBC BASIC */
	m_mode[1](0x0000, 0xffff).rw(m_bus, FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	m_mode[1](0x0000, 0x13ff).ram().share("textram");
	m_mode[1](0x1400, 0x17ff).ram().share("ram34");
	m_mode[1](0x1800, 0x1bff).ram().share("ram38");
	m_mode[1](0x1c00, 0x1fff).ram().share("ram00");
	m_mode[1](0x4000, 0x57ff).rw(FUNC(atombbc_state::videoram_r), FUNC(atombbc_state::videoram_w));
	m_mode[1](0x6000, 0x6fff).r("romslot", FUNC(generic_slot_device::read_rom));
	m_mode[1](0x7000, 0x7003).mirror(0x3fc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	m_mode[1](0x7800, 0x780f).mirror(0x3f0).m("via", FUNC(via6522_device::map));
	m_mode[1](0x8000, 0xbfff).rom().region("basic", 0);
	m_mode[1](0xc000, 0xdfff).rom().region("maincpu", 0x4000);
	m_mode[1](0xf000, 0xffff).rom().region("maincpu", 0x7000);
}

/*-------------------------------------------------
    ADDRESS_MAP( prophet_mem )
-------------------------------------------------*/

void prophet_state::prophet_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(m_bus, FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x0000, 0x03ff).ram();
	//map(0x2800, 0x3bff) // RAM not installed, external bus buffered.
	map(0x8000, 0x97ff).rw(FUNC(prophet_state::videoram_r), FUNC(prophet_state::videoram_w));
	map(0xa000, 0xafff).rom().region("atomcalc", 0x0000);
	map(0xb000, 0xb003).mirror(0x3fc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb800, 0xb80f).mirror(0x3f0).m("via", FUNC(via6522_device::map));
	map(0xc000, 0xdfff).rom().region("maincpu", 0x0000);
	map(0xe000, 0xefff).rom().region("atomcalc", 0x1000);
	map(0xf000, 0xffff).rom().region("maincpu", 0x3000);
}

/*-------------------------------------------------
    ADDRESS_MAP( atomes_mem )
-------------------------------------------------*/

void prophet_state::atomes_mem(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	//map(0x2800, 0x3bff) // RAM not installed, external bus buffered.
	map(0x8000, 0x97ff).rw(FUNC(prophet_state::videoram_r), FUNC(prophet_state::videoram_w));
	map(0xa000, 0xafff).rom().region("ext", 0x0000);
	map(0xb000, 0xb003).mirror(0x3fc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb800, 0xb80f).mirror(0x3f0).m("via", FUNC(via6522_device::map));
	map(0xc000, 0xdfff).rom().region("maincpu", 0x0000);
	map(0xf000, 0xffff).rom().region("maincpu", 0x3000);
}

/*-------------------------------------------------
    ADDRESS_MAP( atomrr_mem )
-------------------------------------------------*/

void atomrr_state::atomrr_mem(address_map &map)
{
	map(0x0000, 0xffff).view(m_mode);
	/* Atom */
	m_mode[0](0x0000, 0x6fff).lrw8(
		NAME([this](offs_t offset) { return m_ram[offset]; }),
		NAME([this](offs_t offset, uint8_t data) { m_ram[offset] = data; })
	);
	m_mode[0](0x0a00, 0x0aff).rw(FUNC(atomrr_state::dskram_r), FUNC(atomrr_state::dskram_w));
	m_mode[0](0x7000, 0x7fff).rw(FUNC(atomrr_state::topram_r), FUNC(atomrr_state::topram_w));
	m_mode[0](0x8000, 0x97ff).rw(FUNC(atomrr_state::videoram_r), FUNC(atomrr_state::videoram_w));
	m_mode[0](0x9800, 0x9fff).ram().share("vram_ext");
	m_mode[0](0xa000, 0xafff).rw(FUNC(atomrr_state::extram_r), FUNC(atomrr_state::extram_w));
	m_mode[0](0xc000, 0xffff).r(FUNC(atomrr_state::dskrom_r));
	/* BBC BASIC */
	m_mode[1](0x0000, 0x5fff).lrw8(
		NAME([this](offs_t offset) { return m_ram[offset]; }),
		NAME([this](offs_t offset, uint8_t data) { m_ram[offset] = data; })
	);
	m_mode[1](0x6000, 0x6fff).rw(FUNC(atomrr_state::extram1_r), FUNC(atomrr_state::extram1_w));
	m_mode[1](0x7000, 0x7fff).rw(FUNC(atomrr_state::extram2_r), FUNC(atomrr_state::extram2_w));
	m_mode[1](0x8000, 0x97ff).rw(FUNC(atomrr_state::videoram_r), FUNC(atomrr_state::videoram_w));
	m_mode[0](0x9800, 0x9fff).ram().share("vram_ext");
	m_mode[1](0xa000, 0xffff).rom().region("flash", 0x1a000);
	/* I/O */
	map(0xb000, 0xb003).mirror(0x3fc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb400, 0xbff0).lrw8(
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xb400); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xb400, data); })
	);
	map(0xb800, 0xb80f).mirror(0x3f0).m("via", FUNC(via6522_device::map));
	map(0xbffd, 0xbffd).portr("JUMPER");
	map(0xbffe, 0xbffe).rw(FUNC(atomrr_state::switch_r), FUNC(atomrr_state::switch_w));
	map(0xbfff, 0xbfff).lrw8(NAME([this]() { return 0xb0 | m_rom_latch; }), NAME([this](uint8_t data) { m_rom_latch = data & 0x0f; }));
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( trigger_reset )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( atom_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	if (newval)
	{
		machine().schedule_soft_reset();
	}
}

/*-------------------------------------------------
    INPUT_PORTS( atom )
-------------------------------------------------*/

static INPUT_PORTS_START( atom )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC")          PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC),27)

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2195")     PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2194")     PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_X)          PORT_CHAR('X')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK")         PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_W)          PORT_CHAR('W')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2191")     PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR('^') // U+2191 = ↑
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COPY")         PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_L)          PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_V)          PORT_CHAR('V')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_A)          PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_U)          PORT_CHAR('U')

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_T)          PORT_CHAR('T')

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('[')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_S)          PORT_CHAR('S')

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(32)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_R)          PORT_CHAR('R')

	PORT_START("Y10")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL")         PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT")        PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y11")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REPT")         PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))

	PORT_START("BRK")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK")        PORT_CODE(KEYCODE_F12)   PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(atom_state::trigger_reset), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( atom_ram )
	PORT_INCLUDE(atom)

	PORT_START("CFG_RAM")
	PORT_CONFNAME(0x0f, 0x05, "Lower Text Space RAM")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "1K #2800-#2BFF")
	PORT_CONFSETTING(0x02, "2K #2800-#2FFF")
	PORT_CONFSETTING(0x03, "3K #2800-#33FF")
	PORT_CONFSETTING(0x04, "4K #2800-#37FF")
	PORT_CONFSETTING(0x05, "5K #2800-#3BFF")
	PORT_CONFNAME(0xf0, 0x60, "Video Graphics RAM")
	PORT_CONFSETTING(0x10, "1K #8000-#83FF")
	PORT_CONFSETTING(0x20, "2K #8000-#87FF")
	PORT_CONFSETTING(0x30, "3K #8000-#8BFF")
	PORT_CONFSETTING(0x40, "4K #8000-#8FFF")
	PORT_CONFSETTING(0x50, "5K #8000-#93FF")
	PORT_CONFSETTING(0x60, "6K #8000-#97FF")
INPUT_PORTS_END


/*-------------------------------------------------
    INPUT_PORTS( atombbc )
-------------------------------------------------*/

static INPUT_PORTS_START( atombbc )
	PORT_INCLUDE(atom)

	PORT_START("CFG_RAM")
	PORT_CONFNAME(0x0f, 0x05, "Lower Text Space RAM")
	PORT_CONFSETTING(0x05, "5K #2800-#3BFF")
	PORT_CONFNAME(0xf0, 0x60, "Video Graphics RAM")
	PORT_CONFSETTING(0x60, "6K #8000-#97FF")
INPUT_PORTS_END


/*-------------------------------------------------
    INPUT_PORTS( prophet )
-------------------------------------------------*/

static INPUT_PORTS_START( prophet )
	PORT_INCLUDE(atom)

	PORT_MODIFY("Y0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2191")     PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP)) // U+2191 = ↑

	PORT_MODIFY("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2193")     PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // U+2193 = ↓
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR('+') PORT_CHAR(';')

	PORT_MODIFY("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2190")     PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // U+2190 = ←
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('*') PORT_CHAR(':')

	PORT_MODIFY("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SCREEN")       PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_MODIFY("Y5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2192")     PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // U+2192 = →

	PORT_MODIFY("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HERE")         PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')

	PORT_MODIFY("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC")          PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC),27)

	PORT_MODIFY("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COPY")         PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('[')

	PORT_START("CFG_RAM")
	PORT_CONFNAME(0x0f, 0x00, "Lower Text Space RAM")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFNAME(0xf0, 0x10, "Video Graphics RAM")
	PORT_CONFSETTING(0x10, "1K #8000-#83FF")

	PORT_START("CFG_MODE")
	PORT_CONFNAME(0x01, 0x01, "Mode")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x01, "Autoboot")
INPUT_PORTS_END


/*-------------------------------------------------
    INPUT_PORTS( atomes )
-------------------------------------------------*/

static INPUT_PORTS_START( atomes )
	PORT_INCLUDE(atom)

	PORT_START("CFG_RAM")
	PORT_CONFNAME(0x0f, 0x00, "Lower Text Space RAM")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFNAME(0xf0, 0x10, "Video Graphics RAM")
	PORT_CONFSETTING(0x10, "1K #8000-#83FF")

	PORT_START("CFG_MODE")
	PORT_CONFNAME(0x01, 0x01, "Mode")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x01, "Sign")
INPUT_PORTS_END


/*-------------------------------------------------
    INPUT_PORTS( atomrr )
-------------------------------------------------*/

static INPUT_PORTS_START( atomrr )
	PORT_INCLUDE(atom)

	PORT_START("CFG_RAM")
	PORT_CONFNAME(0x0f, 0x00, "Lower Text Space RAM")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFNAME(0xf0, 0x60, "Video Graphics RAM")
	PORT_CONFSETTING(0x60, "6K #8000-#97FF")

	PORT_START("JUMPER")
	PORT_CONFNAME(0x04, 0x04, "DSKROMEN")
	PORT_CONFSETTING(0x00, "External")
	PORT_CONFSETTING(0x04, "Flash (AtoMMC)")
	PORT_CONFNAME(0x08, 0x00, "Clock Boost") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(atomrr_state::clock_boost), 0)
	PORT_CONFSETTING(0x00, DEF_STR( No ))
	PORT_CONFSETTING(0x08, DEF_STR( Yes ))
INPUT_PORTS_END


/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( clock_boost )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( atomrr_state::clock_boost )
{
	m_maincpu->set_unscaled_clock(newval ? 3.579545_MHz_XTAL/2 : 4_MHz_XTAL/4);
}


/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    I8255 interface
-------------------------------------------------*/

void atom_state::ppi_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       keyboard column 0
	    1       keyboard column 1
	    2       keyboard column 2
	    3       keyboard column 3
	    4       MC6847 A/G
	    5       MC6847 GM0
	    6       MC6847 GM1
	    7       MC6847 GM2

	*/

	/* keyboard column */
	m_keylatch = data & 0x0f;

	/* MC6847 */
	m_vdg->ag_w(BIT(data, 4));
	m_vdg->gm0_w(BIT(data, 5));
	m_vdg->gm1_w(BIT(data, 6));
	m_vdg->gm2_w(BIT(data, 7));
}

uint8_t atom_state::ppi_pb_r()
{
	/*

	    bit     description

	    0       keyboard row 0
	    1       keyboard row 1
	    2       keyboard row 2
	    3       keyboard row 3
	    4       keyboard row 4
	    5       keyboard row 5
	    6       keyboard CTRL
	    7       keyboard SHFT

	*/

	uint8_t data = 0x3f;

	if (m_keylatch < 10)
		data = m_io_keyboard[m_keylatch]->read() & 0x3f;

	data |= m_io_keyboard[10]->read() & 0xc0;

	return data;
}

uint8_t atom_state::ppi_pc_r()
{
	/*

	    bit     description

	    0       O/P 1, cassette output 0
	    1       O/P 2, cassette output 1
	    2       O/P 3, speaker output
	    3       O/P 4, MC6847 CSS
	    4       2400 Hz input
	    5       cassette input
	    6       keyboard RPT
	    7       MC6847 FS

	*/

	/* 2400 Hz input */
	u8 data = m_hz2400 ? 0x10 : 0;

	/* cassette input */
	data |= (m_cassette->input() > 0.02) << 5;

	/* keyboard RPT */
	data |= m_io_keyboard[11]->read() & 0x40;

	/* MC6847 FS */
	data |= m_vdg->fs_r() ? 0x80 : 0;

	return data;
}

void atom_state::ppi_pc_w(uint8_t data)
{
	/*

	    bit     description

	    0       O/P 1, cassette output 0
	    1       O/P 2, cassette output 1
	    2       O/P 3, speaker output
	    3       O/P 4, MC6847 CSS
	    4       2400 Hz input
	    5       cassette input
	    6       keyboard RPT
	    7       MC6847 FS

	*/

	/* cassette output */
	m_pc0 = BIT(data, 0);
	m_pc1 = BIT(data, 1);

	/* speaker output */
	m_speaker->level_w(BIT(data, 2));

	/* MC6847 CSS */
	m_vdg->css_w(BIT(data, 3));
}

void atom_state::cassette_output_tick(int state)
{
	m_hz2400 = state;

	bool level = m_pc0 && !(m_pc1 && !m_hz2400);

	m_cassette->output(level ? -1.0 : +1.0);
}

/*-------------------------------------------------
    mc6847 interface
-------------------------------------------------*/

uint8_t atom_state::vdg_videoram_r(offs_t offset)
{
	if (offset == ~0) return 0xff;

	m_vdg->as_w(BIT(m_video_ram[offset], 6));
	m_vdg->intext_w(BIT(m_video_ram[offset], 6));
	m_vdg->inv_w(BIT(m_video_ram[offset], 7));

	return m_video_ram[offset];
}

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

void atom_state::machine_start()
{
	/* Kees van Oss mentions that address 8-b are used for the random number
	generator. I don't know if this is hardware, or random data because the
	ram chips are not cleared at start-up. So at this time, these numbers
	are poked into the memory to simulate it. */
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.write_byte(0x08, machine().rand() & 0xff);
	space.write_byte(0x09, machine().rand() & 0xff);
	space.write_byte(0x0a, machine().rand() & 0xff);
	space.write_byte(0x0b, machine().rand() & 0xff);

	save_item(NAME(m_keylatch));
	save_item(NAME(m_hz2400));
	save_item(NAME(m_pc0));
	save_item(NAME(m_pc1));
}

void atom_state::machine_reset()
{
	m_keylatch = 0;
	m_hz2400 = 0;
	m_pc0 = 0;
	m_pc1 = 0;
}


void atombbc_state::machine_start()
{
	atom_state::machine_start();

	/* select BBC BASIC by default */
	m_mode.select(1);
}

void atombbc_state::machine_reset()
{
	atom_state::machine_reset();

	/* mode is selected with SHIFT+BREAK (Atom) or CTRL+BREAK (BBC BASIC) */
	switch (m_io_keyboard[10]->read())
	{
	case 0x40: m_mode.select(0); break; /* Atom */
	case 0x80: m_mode.select(1); break; /* BBC BASIC */
	}
}


void prophet_state::machine_reset()
{
	atom_state::machine_reset();

	if (m_cfg_mode->read())
	{
		/* Autoboot into ROM */
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	}
}


void atomrr_state::machine_start()
{
	atom_state::machine_start();

	switch_w(6);

	save_item(NAME(m_rom_latch));
	save_item(NAME(m_switch_latch));
}


/***************************************************************************
    DEFAULT CARD SETTINGS
***************************************************************************/

static DEVICE_INPUT_DEFAULTS_START(32k_ram32)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x02)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(32k_ram32dos)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x03)
DEVICE_INPUT_DEFAULTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void atom_state::atom_base(machine_config &config)
{
	M6502(config, m_maincpu, 4_MHz_XTAL/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &atom_state::atom_mem);
	config.set_perfect_quantum(m_maincpu); // required for Tube interface

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	MC6847(config, m_vdg, 3.579545_MHz_XTAL);
	m_vdg->input_callback().set(FUNC(atom_state::vdg_videoram_r));
	m_vdg->set_screen("screen");

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	clock_device &cass_clock(CLOCK(config, "cass_clock", 4_MHz_XTAL/16/13/8));
	cass_clock.signal_handler().set(FUNC(atom_state::cassette_output_tick));  // 2403.846Hz

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(atom_state::ppi_pa_w));
	ppi.in_pb_callback().set(FUNC(atom_state::ppi_pb_r));
	ppi.in_pc_callback().set(FUNC(atom_state::ppi_pc_r));
	ppi.out_pc_callback().set(FUNC(atom_state::ppi_pc_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_formats(atom_cassette_formats);
	m_cassette->set_interface("atom_cass");
}

void atom_state::atom(machine_config &config)
{
	atom_base(config);

	via6522_device &via(MOS6522(config, "via", 4_MHz_XTAL/4));
	via.writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	via.readpb_handler().set(m_bus, FUNC(acorn_bus_device::pb_r));
	via.writepb_handler().set(m_bus, FUNC(acorn_bus_device::pb_w));
	via.ca2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	via.cb1_handler().set(m_bus, FUNC(acorn_bus_device::write_cb1));
	via.cb2_handler().set(m_bus, FUNC(acorn_bus_device::write_cb2));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set("via", FUNC(via6522_device::write_ca1));
	centronics.busy_handler().set("via", FUNC(via6522_device::write_pa7));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);

	QUICKLOAD(config, "quickload", "atm", attotime::from_seconds(2)).set_load_callback(FUNC(atom_state::quickload_cb));

	/* extension bus */
	ACORN_BUS(config, m_bus);
	m_bus->out_irq_callback().set_inputline("maincpu", M6502_IRQ_LINE);
	m_bus->out_nmi_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	m_bus->cb1_handler().set("via", FUNC(via6522_device::write_cb1));
	m_bus->cb2_handler().set("via", FUNC(via6522_device::write_cb2));
	ACORN_BUS_SLOT(config, "pl6", m_bus, atom_bus_devices, "discpack");
	ACORN_BUS_SLOT(config, "pl7", m_bus, atom_bus_devices, "32k").set_option_device_input_defaults("32k", DEVICE_INPUT_DEFAULTS_NAME(32k_ram32dos));
	ACORN_BUS_SLOT(config, "pl8", m_bus, atom_pl8_devices, nullptr);

	/* utility rom slot */
	GENERIC_SOCKET(config, "romslot", generic_linear_slot, "atom_cart", "bin,rom");

	/* software lists */
	SOFTWARE_LIST(config, "rom_list").set_original("atom_rom");
	SOFTWARE_LIST(config, "cass_list").set_original("atom_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("atom_flop");
}


void atombbc_state::atombbc(machine_config &config)
{
	atom(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &atombbc_state::atombbc_mem);
}


void prophet_state::prophet2(machine_config &config)
{
	atom(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &prophet_state::prophet_mem);

	/* extension bus */
	subdevice<acorn_bus_slot_device>("pl6")->set_default_option(nullptr);
	subdevice<acorn_bus_slot_device>("pl7")->set_default_option("32k").set_option_device_input_defaults("32k", DEVICE_INPUT_DEFAULTS_NAME(32k_ram32));
	subdevice<acorn_bus_slot_device>("pl7")->set_fixed(true);
}


//void atom_state::prophet3(machine_config &config)
//{
//  atom(config);
//  m_maincpu->set_addrmap(AS_PROGRAM, &atom_state::prophet_mem);
//
//  /* extension bus */
//  subdevice<acorn_bus_slot_device>("pl6")->set_default_option("discpack"); // Polebrook Computer Services Ltd FDC1 (missing ROM P3DOS + 4K RAM)
//  subdevice<acorn_bus_slot_device>("pl7")->set_default_option("32k");
//  subdevice<acorn_bus_slot_device>("pl7")->set_fixed(true);
//}


void prophet_state::atomes(machine_config &config)
{
	atom_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &prophet_state::atomes_mem);

	via6522_device &via(MOS6522(config, "via", 4_MHz_XTAL/4));
	via.writepa_handler().set_nop(); // ("es5700", FUNC(es5700_device::write));

	// TODO: add LED message device
	//ES5700(config, "es5700");
}


void atomrr_state::atomrr(machine_config &config)
{
	atom(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &atomrr_state::atomrr_mem);

	/* extension bus */
	subdevice<acorn_bus_slot_device>("pl6")->set_default_option("atomsid");
	subdevice<acorn_bus_slot_device>("pl7")->set_default_option("atomtube");
	subdevice<acorn_bus_slot_device>("pl8")->set_default_option(nullptr); // atommc (add default when AtoMMC is emulated)
}


/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( atom )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(            0x3000, 0x1000 )
	ROM_LOAD( "afloat.ic21", 0x1000, 0x1000, CRC(81d86af7) SHA1(ebcde5b36cb3a3344567cbba4c7b9fde015f4802) )
ROM_END

ROM_START( atombbc )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )
	/* Atom mode */
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(            0x3000, 0x1000 )
	ROM_LOAD( "afloat.ic21", 0x1000, 0x1000, CRC(81d86af7) SHA1(ebcde5b36cb3a3344567cbba4c7b9fde015f4802) )
	/* BBC mode */
	ROM_LOAD( "mos3.rom",    0x7000, 0x1000, CRC(20158bd8) SHA1(5ee4c0d2b65be72646e17d69b76fb00a0e5298df) )

	ROM_REGION( 0x4000, "basic", 0)
	ROM_LOAD( "bbcbasic.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281) )
ROM_END

ROM_START( prophet2 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(            0x3000, 0x1000)
	ROM_LOAD( "p2fp.ic21",   0x1000, 0x1000, CRC(8be45181) SHA1(b033e2b0b0690e3e4c1f1ec3036f8d83da619f68) )

	ROM_REGION( 0x2000, "atomcalc", 0 )
	ROM_LOAD( "a_69ed.rom", 0x0000, 0x1000, CRC(006010b7) SHA1(1e25c451baad92b74e946cb8dd48abfcaed59d91) )
	ROM_LOAD( "e_61e5.rom", 0x1000, 0x1000, CRC(ecd2d08b) SHA1(d3fe606db1c3372d1e2c62e1143682d33b9fff38) )
ROM_END

//#define rom_prophet3 rom_atom

ROM_START( atomes )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "abasic.ic20", 0x0000, 0x1000, CRC(289b7791) SHA1(0072c83458a9690a3ea1f6094f0f38cf8e96a445) )
	ROM_CONTINUE(            0x3000, 0x1000)

	ROM_REGION( 0x1000, "ext", 0 )
	ROM_LOAD( "pr2b.ic24",   0x0000, 0x1000, CRC(98ddae59) SHA1(92b0df53ab8275e4d7e120d495186f6fdefb0822) )
ROM_END

ROM_START( atomrr )
	/*
	  0x00000 - Atom #A000 Bank 0 - SDDOS v3.25
	  0x01000 - Atom #A000 Bank 1 - PCharme v1.73
	  0x02000 - Atom #A000 Bank 2 - Gags v2.3
	  0x03000 - Atom #A000 Bank 3 - AXR1
	  0x04000 - Atom #A000 Bank 4 - AtomFpga Utils v0.21 (was SALFAA v2.6)
	  0x05000 - Atom #A000 Bank 5 - Atomic Windows v1.1
	  0x06000 - Atom #A000 Bank 6 - WE ROM
	  0x07000 - Atom #A000 Bank 7 - Program Power Programmers Toolkit
	  0x08000 - BBC #6000 Bank 0 (ExtROM1)
	  0x09000 - BBC #6000 Bank 1 (ExtROM1)
	  0x0A000 - BBC #6000 Bank 2 (ExtROM1)
	  0x0B000 - BBC #6000 Bank 3 (ExtROM1)
	  0x0C000 - BBC #6000 Bank 4 (ExtROM1)
	  0x0D000 - BBC #6000 Bank 5 (ExtROM1)
	  0x0E000 - BBC #6000 Bank 6 (ExtROM1)
	  0x0F000 - BBC #6000 Bank 7 (ExtROM1)
	  0x10000 - Atom Basic (DskRomEn=1)
	  0x11000 - Atom FP (DskRomEn=1)
	  0x12000 - Atom MMC (DskRomEn=1)
	  0x13000 - Atom Kernel (DskRomEn=1)
	  0x14000 - Atom Basic (DskRomEn=0)
	  0x15000 - Atom FP (DskRomEn=0)
	  0x16000 - unused
	  0x17000 - Atom Kernel (DskRomEn=0)
	  0x18000 - unused
	  0x19000 - BBC #7000 (ExtROM2)
	  0x1A000 - BBC Basic 1/4
	  0x1B000 - unused
	  0x1C000 - BBC Basic 2/4
	  0x1D000 - BBC Basic 3/4
	  0x1E000 - BBC Basic 4/4
	  0x1F000 - BBC MOS 3.0
	*/
	ROM_REGION( 0x20000, "flash", 0 )
	ROM_LOAD( "ramrom.rom", 0x0000, 0x20000, CRC(250c5b3e) SHA1(ffc4a965ad002c082cd68cea9d2c73045bc9d073) )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS          INIT         COMPANY             FULLNAME                                   FLAGS
COMP( 1979, atom,     0,      0,      atom,     atom_ram, atom_state,    empty_init,  "Acorn Computers",  "Atom",                                    MACHINE_SUPPORTS_SAVE )
COMP( 1982, atombbc,  atom,   0,      atombbc,  atombbc,  atombbc_state, empty_init,  "Acorn Computers",  "Atom with BBC Basic",                     MACHINE_SUPPORTS_SAVE )
COMP( 1983, prophet2, atom,   0,      prophet2, prophet,  prophet_state, empty_init,  "Busicomputers",    "Prophet 2",                               MACHINE_SUPPORTS_SAVE )
//COMP( 1983, prophet3, atom,   0,      prophet3, prophet,  prophet_state, empty_init,  "Busicomputers",    "Prophet 3",                               MACHINE_SUPPORTS_SAVE )
COMP( 198?, atomes,   atom,   0,      atomes,   atomes,   prophet_state, empty_init,  "Pearce Signs",     "ES5700 (LED Electronic Message System)",  MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
COMP( 2014, atomrr,   atom,   0,      atomrr,   atomrr,   atomrr_state,  empty_init,  "Acorn Computers",  "Atom with RAMROM",                        MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
