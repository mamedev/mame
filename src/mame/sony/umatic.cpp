// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/****************************************************************************

    Skeleton driver for Sony U-Matic Videocassette Recorder

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"


namespace {

class umatic_state : public driver_device
{
public:
	umatic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
	{
	}

	void umatic(machine_config &config);
	uint8_t io_read(offs_t offset);
	void io_write(offs_t offset, uint8_t data);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
};

void umatic_state::io_write(offs_t offset, uint8_t data)
{
	//FIXME!
}

uint8_t umatic_state::io_read(offs_t offset)
{
	switch(offset & 7){
		case 0: return 0; //FIXME!
		case 1: return 0; //FIXME!
		case 2: return 0; //FIXME!
		case 3: return 0; //FIXME!
		case 4: return 0x04; //FIXME!
		case 5: return 0; //FIXME!
		case 6: return 0; //FIXME!
		case 7: return 0; //FIXME!
	}

	return 0;
}

void umatic_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0x7c).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x80, 0x87).mirror(0x78).rw(FUNC(umatic_state::io_read), FUNC(umatic_state::io_write));
}

void umatic_state::mem_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();  // 8k-byte EPROM at IC26, but only the first 6kb are mapped.
					// And remaining unmapped content is all 0xFF.
	map(0x1800, 0x1fff).ram();  // 2k-byte CXK5816PN-15L at IC17
}

static INPUT_PORTS_START(umatic)
INPUT_PORTS_END

void umatic_state::umatic(machine_config &config)
{
	Z80(config, m_maincpu, 4.9152_MHz_XTAL / 2); // LH0080 SHARP
	m_maincpu->set_addrmap(AS_PROGRAM, &umatic_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &umatic_state::io_map);

	// peripheral hardware
	Z80CTC(config, m_ctc, 4.9152_MHz_XTAL / 16); // LH0082 SHARP
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	//TODO: m_ctc->zc_callback<2>().set(...); // "search freq out" ?
}

ROM_START(vo5850pm)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("2764_s68_ev1-25.ic26", 0x0000, 0x2000, CRC(7f3c191d) SHA1(4843399f86a15133e966c9e8992eafac03818916))
ROM_END

} // anonymous namespace


//   YEAR  NAME   PARENT/COMPAT MACHINE  INPUT    CLASS             INIT COMPANY  FULLNAME                                    FLAGS
SYST(19??, vo5850pm,    0, 0,   umatic, umatic, umatic_state, empty_init, "Sony", "U-Matic Videocassette Recorder VO-5850PM",  MACHINE_IS_SKELETON)
