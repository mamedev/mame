// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
imm4-22 Instruction/Data Storage Module

This card has four 4002 RAMs configured as a page, sockets for four
1702A or similar 256×8 PROMs configured as a ROM page, buffers for four
4-bit input ports, and latches for four 4-bit output ports.

The 4002 RAMs may be jumpered to be selected by CM-RAM1, CM-RAM2 or
CM-RAM3.  Note that the CM-RAM lines are not decoded, so the 4002 RAMs
will be mirrored.  (With two or more imm4-22 cards installed, the 4002
RAMs will fight for the bus if pages 4-7 are selected.)

The PROMs and I/O ports may be jumpered to be mapped at 0x04xx...0x07xx,
0x08xx...0x0bff or 0x0cff...0x0fff.  Additionally, the card may be
jumpered to enable the PROMs and I/O ports only when /ENABLE MON PROM is
asserted, or at all times.

This card may seem like a good way to expand three aspects of the system
at once, but in practice the PROMs are of limited use:
* If jumpered to enable the PROMs at all times, the PROMs will fight for
  the bus if console RAM card program storage is selected.
* If jumpered to enable the PROMs only when /ENABLE MON PROM is
  asserted, the PROMs become effectively inaccessible as the boot vector
  is in the onboard monitor PROM region, and the onboard monitor PROM
  program provides no way to jump to or read from arbitrary locations.

This card was probably most useful as the cheapest way to double RAM and
I/O ports.  Without PROMs installed, mapping I/O at 0x0400...0x07ff only
when /ENABLE MON PROM is asserted would be a convenient way to provide
the necessary connections for an imm4-90 high-speed paper tape reader.

Image file for the PROMs must be an exact multiple of 256 bytes.  PROMs
are assumed to be filled in order from lowest to highest address.
Jumper changes only take effect on hard reset.


P1 Universal Slot edge connector

                         1    2
                  GND    3    4  GND
                         5    6
                         7    8
                         9   10
                  MA0   11   12  MA1
                  MA2   13   14  MA3
                  MA4   15   16  MA5
                  MA6   17   18  MA7
                   C0   19   20  C1
                        21   22
                /MDI0   23   24
                /MDI1   25   26
                /MDI3   27   28
                /MDI2   29   30
                /MDI5   31   32
                /MDI4   33   34
                /MDI7   35   36
                /MDI6   37   38
                        39   40
                 /OUT   41   42  /ENABLE MON PROM
                 -10V   43   44  -10V
                        45   46
             /CM-RAM2   47   48  /CM-RAM3
                        49   50  /CM-RAM1
                I/O 1   51   52  I/O 0
                I/O 2   53   54  /IN
                        55   56  I/O 3
                        57   58
                        59   60
                        61   62
                        63   64
                        65   66
                        67   68
                        69   70
                        71   72  /D3
                        73   74
                        75   76  /D2
                        77   78
                /SYNC   79   80  /D1
                        81   82
                  /D0   83   84
                        85   86
                        87   88  /RESET-4002
                        89   90
                        91   92
              /CM-ROM   93   94  C3
                        95   96  C2
              PHASE 2   97   98  PHASE 1
                  +5V   99  100  +5V


P1 inputs (40-pin IDC)

       ROM 4/8/12 IN0    1    2  ROM 4/8/12 IN1
       ROM 4/8/12 IN2    3    4  ROM 4/8/12 IN3
                  GND    5    6  ROM 5/9/13 IN0
       ROM 5/9/13 IN1    7    8  ROM 5/9/13 IN2
       ROM 5/9/13 IN3    9   10  GND
      ROM 6/10/14 IN0   11   12  ROM 6/10/14 IN1
      ROM 6/10/14 IN2   13   14  ROM 6/10/14 IN3
                  GND   15   16  ROM 7/11/15 IN0
      ROM 7/11/15 IN1   17   18  ROM 7/11/15 IN2
      ROM 7/11/15 IN3   19   20  GND
                        21   22
                        23   24
                        25   26
                        27   28
                        29   30
                        31   32
                        33   34
                        35   36
                        37   38
                        39   40


P2 outputs (40-pin IDC)

      ROM 4/8/12 OUT0    1    2  ROM 4/8/12 OUT1
      ROM 4/8/12 OUT2    3    4  ROM 4/8/12 OUT3
                  GND    5    6  ROM 5/9/13 OUT0
      ROM 5/9/13 OUT1    7    8  ROM 5/9/13 OUT2
      ROM 5/9/13 OUT3    9   10  GND
     ROM 6/10/14 OUT0   11   12  ROM 6/10/14 OUT1
     ROM 6/10/14 OUT2   13   14  ROM 6/10/14 OUT3
                  GND   15   16  ROM 7/11/15 OUT0
     ROM 7/11/15 OUT1   17   18  ROM 7/11/15 OUT2
     ROM 7/11/15 OUT3   19   20  GND
      RAM 4/8/12 OUT0   21   22  RAM 4/8/12 OUT1
      RAM 4/8/12 OUT2   23   24  RAM 4/8/12 OUT3
                  GND   25   26  RAM 5/9/13 OUT0
      RAM 5/9/13 OUT1   27   28  RAM 5/9/13 OUT2
      RAM 5/9/13 OUT3   29   30  GND
     RAM 6/10/14 OUT0   31   32  RAM 6/10/14 OUT1
     RAM 6/10/14 OUT2   33   34  RAM 6/10/14 OUT3
                  GND   35   36  RAM 7/11/15 OUT0
     RAM 7/11/15 OUT1   37   38  RAM 7/11/15 OUT2
     RAM 7/11/15 OUT3   39   40  GND
*/

#include "emu.h"
#include "insdatastor.h"


namespace {

INPUT_PORTS_START(imm4_22)
	PORT_START("JUMPERS")
	PORT_CONFNAME( 0x03, 0x01, "4002 RAM page" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFNAME( 0x0c, 0x04, "PROM/GPIO page" )
	PORT_CONFSETTING(    0x04, "0x400-0x7FF" )
	PORT_CONFSETTING(    0x08, "0x800-0xBFF" )
	PORT_CONFSETTING(    0x0c, "0xC00-0xFFF" )
	PORT_CONFNAME( 0x10, 0x00, "PROM/GPIO mode" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "MON only" )
INPUT_PORTS_END


class imm4_22_device
		: public device_t
		, public bus::intellec4::device_univ_card_interface
		, public device_image_interface
{
public:
	imm4_22_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool        is_readable()                   const noexcept override { return true; }
	virtual bool        is_writeable()                  const noexcept override { return false; }
	virtual bool        is_creatable()                  const noexcept override { return false; }
	virtual bool        is_reset_on_load()              const noexcept override { return false; }
	virtual char const *file_extensions()               const noexcept override { return "rom,bin"; }
	virtual char const *image_type_name()               const noexcept override { return "promimage"; }
	virtual char const *image_brief_type_name()         const noexcept override { return "prom"; }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void reset_4002_in(int state) override;

private:
	void ram_out(offs_t offset, u8 data);
	void rom_out(offs_t offset, u8 data);
	u8 rom_in(offs_t offset);

	void allocate();
	void map_ram_io();
	void map_prom();
	void unmap_prom();

	required_ioport m_jumpers;

	bool    m_ram_io_mapped;

	u8      m_ram_page, m_rom_page;
	bool    m_rom_mirror;

	u8                      m_memory[256], m_status[64];
	std::unique_ptr<u8 []>  m_prom;
};


imm4_22_device::imm4_22_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INTELLEC4_INST_DATA_STORAGE, tag, owner, clock)
	, bus::intellec4::device_univ_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_jumpers(*this, "JUMPERS")
	, m_ram_io_mapped(false)
	, m_ram_page(0U), m_rom_page(0U), m_rom_mirror(false)
	, m_prom()
{
	std::fill(std::begin(m_memory), std::end(m_memory), 0U);
	std::fill(std::begin(m_status), std::end(m_status), 0U);
}


std::pair<std::error_condition, std::string> imm4_22_device::call_load()
{
	if ((length() > 1024U) || (length() % 256U))
	{
		return std::make_pair(
				image_error::INVALIDLENGTH,
				"Invalid PROM image size (must be a multiple of 256 bytes no larger than 1K)");
	}

	allocate();
	if (fread(m_prom.get(), length()) != length())
		return std::make_pair(image_error::UNSPECIFIED, "Error reading file");

	map_prom();

	return std::make_pair(std::error_condition(), std::string());
}

void imm4_22_device::call_unload()
{
	unmap_prom();
}


ioport_constructor imm4_22_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(imm4_22);
}

void imm4_22_device::device_start()
{
	m_ram_io_mapped = false;

	allocate();

	save_item(NAME(m_ram_page));
	save_item(NAME(m_rom_page));
	save_item(NAME(m_rom_mirror));
	save_item(NAME(m_memory));
	save_item(NAME(m_status));
	save_pointer(NAME(m_prom), 1024U);
}

void imm4_22_device::device_reset()
{
	map_ram_io();
}


void imm4_22_device::reset_4002_in(int state)
{
	// FIXME: this takes several cycles to actually erase everything, and prevents writes while asserted
	if (!state)
	{
		std::fill(std::begin(m_memory), std::end(m_memory), 0U);
		std::fill(std::begin(m_status), std::end(m_status), 0U);
	}
}


void imm4_22_device::ram_out(offs_t offset, u8 data)
{
	// GPIO write - hooking this up would be a pain with MAME as it is
	logerror("4002 A%u out %X\n", 13U + (offset & 0x03U), data & 0x0fU);
}

void imm4_22_device::rom_out(offs_t offset, u8 data)
{
	// GPIO write - hooking this up would be a pain with MAME as it is
	logerror("ROM %u out %X\n", (m_rom_page << 2) | ((offset >> 4) & 0x03U), data & 0x0fU);
}

u8 imm4_22_device::rom_in(offs_t offset)
{
	// GPIO read - hooking this up would be a pain with MAME as it is
	if (!machine().side_effects_disabled())
		logerror("ROM %u in\n", (m_rom_page << 2) | ((offset >> 4) & 0x03U));
	return 0x0fU;
}


void imm4_22_device::allocate()
{
	if (!m_prom)
	{
		m_prom = std::make_unique<u8 []>(1024U);
		std::fill(m_prom.get(), m_prom.get() + 1024U, 0U);
	}
}

void imm4_22_device::map_ram_io()
{
	if (!m_ram_io_mapped)
	{
		static constexpr offs_t ram_start[3][4] = {
				{ 0x01U, 0x04U, 0x05U, 0x07U },
				{ 0x02U, 0x04U, 0x06U, 0x07U },
				{ 0x03U, 0x05U, 0x06U, 0x07U } };

		m_ram_io_mapped = true;

		ioport_value const jumpers(m_jumpers->read());
		m_ram_page = jumpers & 0x03U;
		m_rom_page = (jumpers >> 2) & 0x03U;
		m_rom_mirror = BIT(~jumpers, 4);

		if ((1U > m_ram_page) || (3U < m_ram_page))
			throw emu_fatalerror("imm4_22_device: invalid RAM page %u", m_ram_page);
		if ((1U > m_rom_page) || (3U < m_rom_page))
			throw emu_fatalerror("imm4_22_device: invalid PROM/GPIO page %u", m_rom_page);

		for (offs_t const start : ram_start[m_ram_page - 1U])
		{
			memory_space().install_ram(start << 8, (start << 8) | 0x00ffU, m_memory);
			status_space().install_ram(start << 6, (start << 6) | 0x003fU, m_status);
			ram_ports_space().install_write_handler(start << 2, (start << 2) | 0x03U, write8sm_delegate(*this, FUNC(imm4_22_device::ram_out)));
		}

		offs_t const rom_ports_start(offs_t(m_rom_page) << 6);
		offs_t const rom_ports_end(rom_ports_start | 0x003fU);
		offs_t const rom_ports_mirror(m_rom_mirror ? 0x1f00U : 0x0700U);
		rom_ports_space().install_readwrite_handler(
				rom_ports_start, rom_ports_end, 0U, rom_ports_mirror, 0U,
				read8sm_delegate(*this, FUNC(imm4_22_device::rom_in)), write8sm_delegate(*this, FUNC(imm4_22_device::rom_out)));

		if (is_loaded())
			map_prom();
	}
}

void imm4_22_device::map_prom()
{
	if (m_ram_io_mapped)
	{
		// FIXME: gimme a cookie!
		// FIXME: if mirror is on, this will fight console RAM for the bus in 0x2000 region and also appear in the "two switches" 0x3000 region
		rom_space().install_rom(
				offs_t(m_rom_page) << 10, offs_t((offs_t(m_rom_page) << 10) | length()), m_rom_mirror ? 0x1000U : 0x0000U,
				m_prom.get());
	}
}

void imm4_22_device::unmap_prom()
{
	// FIXME: where's my magic cookie?
	rom_space().unmap_read(offs_t(m_rom_page) << 10, (offs_t(m_rom_page) << 10) | 0x03ffU, m_rom_mirror ? 0x1000U : 0x0000U);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(INTELLEC4_INST_DATA_STORAGE, bus::intellec4::device_univ_card_interface, imm4_22_device, "intlc4_imm4_22", "Intel imm4-22 Instruction/Data Storage Module")
