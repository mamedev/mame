// license:BSD-3-Clause
// copyright-holders:Antoine Mine, Olivier Galibert

// The "Nanoreseau" was a proprietary networking for MO/TO Thomson
// computers using rs-485.  A PC is supposed to be used as a network
// head.

#include "emu.h"
#include "nanoreseau.h"

DEFINE_DEVICE_TYPE(NANORESEAU_TO, nanoreseau_to_device, "nanoreseau_to", "Nanoreseau controller (TO rom)")
DEFINE_DEVICE_TYPE(NANORESEAU_MO, nanoreseau_mo_device, "nanoreseau_mo", "Nanoreseau controller (MO rom)")

	nanoreseau_device::nanoreseau_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool no_id) :
	device_t(mconfig, type, tag, owner, clock),
	thomson_extension_interface(mconfig, *this),
	m_mc6854(*this, "mc6854"),
	m_rom(*this, "rom"),
	m_id(*this, "id"),
	m_no_id(no_id)
{
}

nanoreseau_to_device::nanoreseau_to_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool no_id) :
	nanoreseau_device(mconfig, NANORESEAU_TO, tag, owner, clock)
{
}

nanoreseau_mo_device::nanoreseau_mo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool no_id) :
	nanoreseau_device(mconfig, NANORESEAU_MO, tag, owner, clock)
{
}

ROM_START(nanoreseau_to)
	ROM_REGION( 0x7c0, "rom", 0 )
	ROM_LOAD ( "nano7.rom", 0x000, 0x7c0, CRC(42a1d1a6) SHA1(973209f4baa5e81bf7885c0602949e064bac7862) )
ROM_END

ROM_START(nanoreseau_mo)
	ROM_REGION( 0x7c0, "rom", 0 )
	ROM_LOAD ( "nano5.rom", 0x000, 0x7c0, CRC(2f756868) SHA1(b5b7cb6d12493d849330b6b5628efd1a83a4bbf5) )
ROM_END

static INPUT_PORTS_START(nanoreseau_config)
	PORT_START("id")
	PORT_CONFNAME(0x1f, 0x01, "Network ID")
	PORT_CONFSETTING(0x00, "0 (Master)")
	PORT_CONFSETTING(0x01, "1")
	PORT_CONFSETTING(0x02, "2")
	PORT_CONFSETTING(0x03, "3")
	PORT_CONFSETTING(0x04, "4")
	PORT_CONFSETTING(0x05, "5")
	PORT_CONFSETTING(0x06, "6")
	PORT_CONFSETTING(0x07, "7")
	PORT_CONFSETTING(0x08, "8")
	PORT_CONFSETTING(0x09, "9")
	PORT_CONFSETTING(0x0a, "10")
	PORT_CONFSETTING(0x0b, "11")
	PORT_CONFSETTING(0x0c, "12")
	PORT_CONFSETTING(0x0d, "13")
	PORT_CONFSETTING(0x0e, "14")
	PORT_CONFSETTING(0x0f, "15")
	PORT_CONFSETTING(0x10, "16")
	PORT_CONFSETTING(0x11, "17")
	PORT_CONFSETTING(0x12, "18")
	PORT_CONFSETTING(0x13, "19")
	PORT_CONFSETTING(0x14, "20")
	PORT_CONFSETTING(0x15, "21")
	PORT_CONFSETTING(0x16, "22")
	PORT_CONFSETTING(0x17, "23")
	PORT_CONFSETTING(0x18, "24")
	PORT_CONFSETTING(0x19, "25")
	PORT_CONFSETTING(0x1a, "26")
	PORT_CONFSETTING(0x1b, "27")
	PORT_CONFSETTING(0x1c, "28")
	PORT_CONFSETTING(0x1d, "29")
	PORT_CONFSETTING(0x1e, "30")
	PORT_CONFSETTING(0x1f, "31")
INPUT_PORTS_END

void nanoreseau_device::rom_map(address_map &map)
{
	map(0x000, 0x7bf).rom().region(m_rom, 0);
}

void nanoreseau_device::io_map(address_map &map)
{
	map(0, 3).rw(m_mc6854, FUNC(mc6854_device::read), FUNC(mc6854_device::write));
	if (!m_no_id)
		map(8, 8).r(FUNC(nanoreseau_device::id_r));
}

const tiny_rom_entry *nanoreseau_to_device::device_rom_region() const
{
	return ROM_NAME(nanoreseau_to);
}

const tiny_rom_entry *nanoreseau_mo_device::device_rom_region() const
{
	return ROM_NAME(nanoreseau_mo);
}

void nanoreseau_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_mc6854);
	m_mc6854->set_out_frame_callback(FUNC(nanoreseau_device::got_frame));
}

void nanoreseau_device::device_start()
{
	m_timer = timer_alloc(FUNC(nanoreseau_device::answer_tick), this);
	save_item(NAME(m_answer_step));
}

void nanoreseau_device::device_reset()
{
	m_answer_step = 4;
	m_mc6854->set_cts(0);
	m_mc6854->set_cts(1);
}

ioport_constructor nanoreseau_device::device_input_ports() const
{
	return m_no_id ? nullptr : INPUT_PORTS_NAME(nanoreseau_config);
}

/*********************** Network ************************/

/* The network extension is built as an external floppy controller.
   It uses the same ROM and I/O space, so it is natural to have the
   top-level network emulation here.

   NOTE: This is work in progress.
   For the moment, only hand-checks work: the TO7 can take the line, then
   perform a DKBOOT request. We do not have the server emulated yet, so
   there is no way to answer the request.
*/

/* consigne DKBOOT

   MO5 BASIC
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $01 $00<$41 $00 $FF $20
   $3D $4C $01 $60 $20 $3C $4F $01 $05 $20 $3F $9C $19 $25 $03 $11
   $93 $15 $10 $25 $32 $8A $7E $FF $E1 $FD $E9 $41>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00

   TO7/70 BASIC
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $02 $00<$20 $42 $41 $53
   $49 $43 $20 $4D $49 $43 $52 $4F $53 $4F $46 $54 $20 $31 $2E $30
   $04 $00 $00 $00 $00 $00 $60 $FF $37 $9B $37 $9C>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00

   TO7 BASIC
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $00 $00<$20 $42 $41 $53
   $49 $43 $20 $4D $49 $43 $52 $4F $53 $4F $46 $54 $20 $31 $2E $30
   $04 $00 $00 $00 $00 $00 $60 $FF $37 $9B $37 $9C>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00

   TO7 LOGO
   $00 $00 $01 $00 $00 $00 $00 $00 $00 $00 $00 $00<$00 $00 $00 $00
   $00 $20 $4C $4F $47 $4F $04 $00 $00 $00 $00 $00 $00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $AA $FF $01 $16 $00 $C8>$00 $00 $00 $00
   $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00 $00


*/

TIMER_CALLBACK_MEMBER(nanoreseau_device::answer_tick)
{
	m_answer_step++;
	m_mc6854->set_cts(m_answer_step & 1);
	if (m_answer_step < 4)
		m_timer->adjust(attotime::from_usec(100));
}

void nanoreseau_device::got_frame(uint8_t *data, int length)
{
	std::string frame = util::string_format("%s: frame", machine().time().to_string());
	for(int i = 0; i < length; i++)
		frame += util::string_format(" %02x", data[i]);
	logerror("%s\n", frame);

	if (data[1] == 0xff) {
		logerror("frame: %d phones %d\n", data[2], data[0]);
		m_answer_step = 0;
		m_timer->adjust(attotime::from_usec(100));
		m_mc6854->set_cts(0);

	} else if (!data[1]) {
		char name[33];
		memcpy(name, data + 12, 32);
		name[32] = 0;
		for(int i=0; i<32; i++)
			if (name[i] < 32 || name[i] >= 127)
				name[i]='.';
		logerror("DKBOOT system %s appli %s\n", data[10] == 0 ? "TO7" : data[10] == 1 ? "MO5" : data[10] == 2 ? "TO7/70" : "?", name);
	}
}

u8 nanoreseau_device::id_r()
{
	// network ID of the computer
	return m_id->read();
}
