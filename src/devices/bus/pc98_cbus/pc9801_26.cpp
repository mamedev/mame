// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC PC-9801-26(K) sound card

Legacy sound card for PC-98xx family, composed by a single YM2203

NOTES:
- The only known difference between -26 and -26K is that former draws its clock from C-Bus root.
  This makes the card to misbehave with 286+ machines.

TODO:
- DE-9 output writes (cfr. page 419 of PC-9801 Bible, needs software testing the functionality)
- understand if dips can be read by SW;
- configurable irq level needs a binding flush in C-bus handling;
- Eventually emulate -26 clock style, currently hardwired as -26K;

**************************************************************************************************/

#include "emu.h"
#include "pc9801_26.h"

#include "sound/ymopn.h"
#include "speaker.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PC9801_26, pc9801_26_device, "pc9801_26", "NEC PC-9801-26/K sound card")

pc9801_26_device::pc9801_26_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_26, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_opn(*this, "opn")
	, m_joy(*this, "joy_p%u", 1U)
	, m_bios(*this, "bios")
	, m_irq_jp(*this, "JP6A1_JP6A3")
{
}

void pc9801_26_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	YM2203(config, m_opn, 15.9744_MHz_XTAL / 4); // divider not verified
	m_opn->irq_handler().set([this] (int state) {
		const int int_levels[4] = { 0, 4, 6, 5 };
		m_bus->int_w(int_levels[m_int_level & 3], state);
	});
	/*
	 * xx-- ---- IRSTx interrupt status 0/1
	 * --xx ---- TRIGx Trigger 1/2
	 * ---- xxxx <directions>
	 */
	m_opn->port_a_read_callback().set([this] () {
		u8 res = (BIT(m_joy_sel, 7)) ? m_joy[BIT(m_joy_sel, 6)]->read() : 0x3f;
		res   |= m_int_level << 6;
		return (u8)res;
	});
	/*
	 * x--- ---- OUTE Output Enable (DDR?)
	 * -x-- ---- INSL Input Select
	 * --21 2211 OUTxy x = port / y (2-1) = number (1-3)
	 */
	m_opn->port_b_write_callback().set([this] (u8 data) {
		m_joy_sel = data;
		// TODO: guesswork, verify
		if (BIT(data, 7))
			return;
		m_joy[0]->pin_6_w(BIT(~data, 0));
		m_joy[0]->pin_7_w(BIT(~data, 1));
		m_joy[1]->pin_6_w(BIT(~data, 2));
		m_joy[1]->pin_7_w(BIT(~data, 3));
		m_joy[0]->pin_8_w(BIT(~data, 4));
		m_joy[1]->pin_8_w(BIT(~data, 5));
	});

	// TODO: verify mixing on HW
	// emerald stage 1 BGM uses ch. 3 for bassline, which sounds way more prominent
	// than the others combined (0.25 1/4 ratio even?)
	m_opn->add_route(0, "mono", 0.50);
	m_opn->add_route(1, "mono", 0.50);
	m_opn->add_route(2, "mono", 0.50);
	m_opn->add_route(3, "mono", 1.00);

	MSX_GENERAL_PURPOSE_PORT(config, m_joy[0], msx_general_purpose_port_devices, "joystick");
	MSX_GENERAL_PURPOSE_PORT(config, m_joy[1], msx_general_purpose_port_devices, "joystick");
}

// to load a different bios for slots:
// -cbus0 pc9801_26,bios=N
ROM_START( pc9801_26 )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	// PC9801_26k is a minor change that applies to 286+ CPUs
	ROM_SYSTEM_BIOS( 0,  "26k",     "nec26k" )
	ROMX_LOAD( "26k_wyka01_00.bin", 0x0000, 0x2000, CRC(f071bf69) SHA1(f3cdef94e9fee116cf4a9b54881e77c6cd903815), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "26k_wyka02_00.bin", 0x0001, 0x2000, CRC(eaa01052) SHA1(5d47edae49aad591f139d5599fe04b61aefd5ecd), ROM_SKIP(1) | ROM_BIOS(0) )
	// regular BIOS, for V30 and downward CPUs
	ROM_SYSTEM_BIOS( 1,  "26",      "nec26" )
	ROMX_LOAD( "sound.rom",       0x0000, 0x4000, CRC(80eabfde) SHA1(e09c54152c8093e1724842c711aed6417169db23), ROM_BIOS(1) )
	// following rom is unchecked and of dubious quality
	// we also currently mark it based off where they originally belonged to, lacking a better info
	ROM_SYSTEM_BIOS( 2,  "26_9821", "nec26_9821" )
	ROMX_LOAD( "sound_9821.rom",  0x0000, 0x4000, BAD_DUMP CRC(a21ef796) SHA1(34137c287c39c44300b04ee97c1e6459bb826b60), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *pc9801_26_device::device_rom_region() const
{
	return ROM_NAME( pc9801_26 );
}

static INPUT_PORTS_START( pc9801_26 )
	// On-board jumpers
	PORT_START("JP6A1_JP6A3")
	PORT_CONFNAME( 0x03, 0x03, "PC-9801-26: Interrupt level")
	PORT_CONFSETTING(    0x00, "INT0 (IRQ3)" ) // 2-3, 2-3
	PORT_CONFSETTING(    0x02, "INT41 (IRQ10)" ) // 2-3, 1-2
	PORT_CONFSETTING(    0x03, "INT5 (IRQ12)" ) // 1-2, 1-2
	PORT_CONFSETTING(    0x01, "INT6 (IRQ13)" ) // 1-2, 2-3

	PORT_START("JP6A2")
	PORT_CONFNAME( 0x07, 0x01, "PC-9801-26: Sound ROM address")
	PORT_CONFSETTING(    0x00, "0xc8000" )    // 1-10
	PORT_CONFSETTING(    0x01, "0xcc000" )    // 2-9
	PORT_CONFSETTING(    0x02, "0xd0000" )    // 3-8
	PORT_CONFSETTING(    0x03, "0xd4000" )    // 4-7
	PORT_CONFSETTING(    0x04, "Disable ROM") // 5-6

	PORT_START("JP6A4")
	PORT_CONFNAME( 0x01, 0x01, "PC-9801-26: Port Base" )
	PORT_CONFSETTING(    0x00, "0x088" ) // 1-4
	PORT_CONFSETTING(    0x01, "0x188" ) // 2-3
INPUT_PORTS_END

ioport_constructor pc9801_26_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_26 );
}

u16 pc9801_26_device::read_io_base()
{
	return (ioport("JP6A4")->read() & 1) << 8;
}

void pc9801_26_device::device_start()
{
	m_rom_base = 0;

	save_item(NAME(m_joy_sel));
}

void pc9801_26_device::device_reset()
{
	// read INT line
	m_int_level = m_irq_jp->read() & 3;
}

void pc9801_26_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		const u8 rom_setting = ioport("JP6A2")->read() & 7;
		static const u32 rom_addresses[8] = { 0xc8000, 0xcc000, 0xd0000, 0xd4000, 0, 0, 0, 0 };
		const u32 start_address = rom_addresses[rom_setting & 7];

		if (start_address != 0)
		{
			const u32 end_address = start_address + 0x3fff;
			logerror("map ROM at 0x%08x-0x%08x\n", start_address, end_address);
			m_bus->space(AS_PROGRAM).install_rom(
				start_address,
				end_address,
				m_bios->base()
			);
		}
		else
		{
			logerror("ROM is disconnected\n");
		}
	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0x3fff, *this, &pc9801_26_device::io_map);
	}
}


void pc9801_26_device::io_map(address_map &map)
{
	const u16 io_base = read_io_base();
	map(0x0088 | io_base, 0x008b | io_base).rw(m_opn, FUNC(ym2203_device::read), FUNC(ym2203_device::write)).umask16(0x00ff);
}
