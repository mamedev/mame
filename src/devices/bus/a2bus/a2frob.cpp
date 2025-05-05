// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol, David Shah, Golden Child
/***************************************************************************

  Apple 2 Frob Card by Frobco

  The Frob Card is a ROM emulator device for the Atari 2600 that operates
  under the control of an Apple II.  It also allows the Apple II and Atari 2600
  to exchange information with a bidirectional port.

  The Atari 2600 code comes directly from a2600.cpp and has been converted
  to be a device instead of a driver.

  The slot interface for the a2600 has been removed for simplicity as well as
  the PAL a2600 variant.

  Note: Once the Frob memory is loaded, you can reset the 2600 with Numpad Minus

***************************************************************************/


#include "emu.h"

#include "a2bus.h"
#include "a2frobtia.h"
#include "a2frob.h"

#include "bus/vcs/rom.h"
#include "bus/vcs/vcs_slot.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6507.h"
#include "machine/mos6530.h"
#include "sound/tiaintf.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

static const uint16_t supported_screen_heights[4] = { 262, 312, 328, 342 };

class a2600_frob_base_device :
	public device_t,
	public device_a2bus_card_interface
{

public:
	a2600_frob_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	INPUT_CHANGED_MEMBER(reset_a2600);

protected:
	a2600_frob_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_riot_ram(*this, "riot_ram"),
		m_tia(*this, "tia_video"),
		m_maincpu(*this, "maincpu"),
		m_riot(*this, "riot"),
		m_joy1(*this, "joyport1"),
		m_joy2(*this, "joyport2"),
		m_screen(*this, "tia_screen"),
		m_xtal(3.579575_MHz_XTAL)
	{ }

	virtual void device_start() override ATTR_COLD;

	void a2600_mem(address_map &map) ATTR_COLD;

	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;

	virtual u8 read_cnxx(u8 offset) override
	{ return !m_frob_apple_control ? m_frob_mem[(offset & 0xff)|(m_frob_page << 8)] : 0; }

	virtual void write_cnxx(u8 offset, u8 data) override
	{ if (!m_frob_apple_control) m_frob_mem[(offset & 0xff)|(m_frob_page << 8)] = data; }

	virtual bool take_c800() override { return false; }


	u8 read_frob_mem(offs_t offset) { return !m_frob_apple_control ? 0 : m_frob_mem[offset & 0xfff]; }

	void write_frob_mem(offs_t offset, u8 data) { m_frob_mem[offset & 0xfff] = data; }


	void a2600_write_fff0(offs_t offset, u8 data)
	{
		m_byte_for_apple = data;
		machine().scheduler().synchronize(
			timer_expired_delegate(FUNC(a2600_frob_base_device::sync_write_byte_waiting_flag_apple), this), 1);
	}

	u8 a2600_read_fff2(offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			machine().scheduler().synchronize(
				timer_expired_delegate(FUNC(a2600_frob_base_device::sync_write_byte_waiting_flag_vcs), this), 0);
		}
		return m_byte_for_vcs;
	}

	u8 a2600_read_fff1(offs_t offset)
	{
		return (m_byte_waiting_flag_apple ? 0 : 1) << 7 | (m_byte_waiting_flag_vcs << 6);
	}

	void switch_A_w(uint8_t data);
	uint8_t switch_A_r();
	void switch_B_w(uint8_t data);
	void irq_callback(int state);
	uint16_t a2600_read_input_port(offs_t offset);
	uint8_t a2600_get_databus_contents(offs_t offset);
	void a2600_tia_vsync_callback(uint16_t data);

	TIMER_CALLBACK_MEMBER(sync_write_byte_waiting_flag_vcs) { m_byte_waiting_flag_vcs = (u8) param; };
	TIMER_CALLBACK_MEMBER(sync_write_byte_waiting_flag_apple) { m_byte_waiting_flag_apple = (u8) param;};

	required_shared_ptr<uint8_t> m_riot_ram;
	required_device<a2frobtia_video_device> m_tia;
	required_device<m6507_device> m_maincpu;
	required_device<mos6532_device> m_riot;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<screen_device> m_screen;

	u8 m_frob_mem[0x1000] = { 0 };  // 4k frob memory
	u8 m_frob_apple_control = 0;  // 0 means that it's under apple control
	u8 m_byte_waiting_flag_apple = 0;
	u8 m_byte_waiting_flag_vcs = 0;
	u8 m_byte_for_vcs = 0;
	u8 m_byte_for_apple = 0;
	u8 m_bidirectional_active = 0;
	u8 m_frob_page = 0;
	u8 m_frob_control_reg = 0;

private:
	uint16_t m_current_screen_height = 0U;
	XTAL m_xtal;

};

a2600_frob_base_device::a2600_frob_base_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	a2600_frob_base_device(mconfig, A2BUS_FROB, tag, owner, clock)
{
}


u8 a2600_frob_base_device::read_c0nx(u8 offset)
{
	// offset 0 is read from frob status  (bits 7 = write ok, 6 = read ok) (other bits will float)
	switch (offset & 0x1)
	{
		case 0:
			return ((m_byte_waiting_flag_vcs ? 0 : 1) << 7) |
					(m_byte_waiting_flag_apple << 6);
			break;
		case 1:
			if (!machine().side_effects_disabled())
			{
				machine().scheduler().synchronize(
					timer_expired_delegate(FUNC(a2600_frob_base_device::sync_write_byte_waiting_flag_apple), this), 0);
			}
			return m_byte_for_apple;
		default:
			return 0;
	}
}

void a2600_frob_base_device::write_c0nx(u8 offset, u8 data)
{
	// offset 0 is write to frob control register
	// offset 1 is write to bidirectional register
	switch (offset & 0x1)
	{
		case 0:
			m_frob_control_reg = data;
			m_frob_apple_control = BIT(data, 4);
			m_bidirectional_active = BIT(data, 5);
			m_frob_page = BIT(data, 0, 4);
			LOG("m_frob_apple_control = %x  m_frob_page = %x\n",m_frob_apple_control, m_frob_page);
			break;
		case 1:
			machine().scheduler().synchronize(
				timer_expired_delegate(FUNC(a2600_frob_base_device::sync_write_byte_waiting_flag_vcs), this), 1);
			m_byte_for_vcs = data;
			break;
		default:
			;
	}
}

void a2600_frob_base_device::a2600_mem(address_map &map) // 6507 has 13-bit address space, 0x0000 - 0x1fff
{
	map(0x0000, 0x007f).mirror(0x0f00).rw(m_tia, FUNC(a2frobtia_video_device::read), FUNC(a2frobtia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0d00).ram().share("riot_ram");
	map(0x0280, 0x029f).mirror(0x0d00).m("riot", FUNC(mos6532_device::io_map));
	map(0x1000, 0x1fff).r(FUNC(a2600_frob_base_device::read_frob_mem));
	map(0x1ff0, 0x1ff0).w(FUNC(a2600_frob_base_device::a2600_write_fff0));
	map(0x1ff1, 0x1ff1).r(FUNC(a2600_frob_base_device::a2600_read_fff1));
	map(0x1ff2, 0x1ff2).r(FUNC(a2600_frob_base_device::a2600_read_fff2));
}

void a2600_frob_base_device::switch_A_w(uint8_t data)
{
	/* Left controller port */
	m_joy1->joy_w(data >> 4);

	/* Right controller port */
	m_joy2->joy_w(data & 0x0f);
}

uint8_t a2600_frob_base_device::switch_A_r()
{
	uint8_t val = 0;

	// Left controller port PINs 1-4 ( 4321 )
	val |= (m_joy1->read_joy() & 0x0f) << 4;

	// Right controller port PINs 1-4 ( 4321 )
	val |= m_joy2->read_joy() & 0x0f;

	return val;
}

void a2600_frob_base_device::switch_B_w(uint8_t data)
{
}

void a2600_frob_base_device::irq_callback(int state)
{
}

uint16_t a2600_frob_base_device::a2600_read_input_port(offs_t offset)
{
	switch (offset)
	{
	case 0: // Left controller port PIN 5
		return m_joy1->read_pot_x();

	case 1: // Left controller port PIN 9
		return m_joy1->read_pot_y();

	case 2: // Right controller port PIN 5
		return m_joy2->read_pot_x();

	case 3: // Right controller port PIN 9
		return m_joy2->read_pot_y();

	case 4: // Left controller port PIN 6
		return (m_joy1->read_joy() & 0x20) ? 0xff : 0x7f;

	case 5: // Right controller port PIN 6
		return (m_joy2->read_joy() & 0x20) ? 0xff : 0x7f;
	}
	return 0xff;
}

/* There are a few games that do an LDA ($80-$FF),Y instruction.
   The contents off the databus then depend on whatever was read
   from the RAM. To do this really properly the 6502 core would
   need to keep track of the last databus contents so we can query
   that. For now this is a quick hack to determine that value anyway.
   Examples:
   Q-Bert's Qubes (NTSC,F6) at 0x1594
   Berzerk at 0xF093.
*/
uint8_t a2600_frob_base_device::a2600_get_databus_contents(offs_t offset)
{
	uint16_t  last_address, prev_address;
	uint8_t   last_byte, prev_byte;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	last_address = m_maincpu->pc() + 1;
	if ( ! ( last_address & 0x1080 ) )
	{
		return offset;
	}
	last_byte = prog_space.read_byte(last_address );
	if ( last_byte < 0x80 || last_byte == 0xFF )
	{
		return last_byte;
	}
	prev_address = last_address - 1;
	if ( ! ( prev_address & 0x1080 ) )
	{
		return last_byte;
	}
	prev_byte = prog_space.read_byte(prev_address );
	if ( prev_byte == 0xB1 )
	{   /* LDA (XX),Y */
		return prog_space.read_byte(last_byte + 1 );
	}
	return last_byte;
}

#if 0
static const rectangle visarea[4] = {
	{ 26, 26 + 160 + 16, 24, 24 + 192 + 31 },   /* 262 */
	{ 26, 26 + 160 + 16, 32, 32 + 228 + 31 },   /* 312 */
	{ 26, 26 + 160 + 16, 45, 45 + 240 + 31 },   /* 328 */
	{ 26, 26 + 160 + 16, 48, 48 + 240 + 31 }    /* 342 */
};
#endif

void a2600_frob_base_device::a2600_tia_vsync_callback(uint16_t data)
{
	for (int i = 0; i < std::size(supported_screen_heights); i++)
	{
		if (data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3)
		{
			if (supported_screen_heights[i] != m_current_screen_height)
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, visarea[i], HZ_TO_ATTOSECONDS(m_xtal) * 228 * m_current_screen_height);
			}
		}
	}
}

//void a2600_frob_base_device::machine_start()
void a2600_frob_base_device::device_start()
{
	m_current_screen_height = m_screen->height();
	memset(m_riot_ram, 0x00, 0x80);

	save_item(NAME(m_current_screen_height));
	save_pointer(NAME(m_frob_mem), 0x1000);

	save_item(NAME(m_frob_apple_control));
	save_item(NAME(m_byte_waiting_flag_apple));
	save_item(NAME(m_byte_waiting_flag_vcs));
	save_item(NAME(m_byte_for_vcs));
	save_item(NAME(m_byte_for_apple));
	save_item(NAME(m_bidirectional_active));
	save_item(NAME(m_frob_page));
	save_item(NAME(m_frob_control_reg));
}


INPUT_CHANGED_MEMBER( a2600_frob_base_device::reset_a2600 )
{
	 if (newval == 0)
	 {
		m_maincpu->reset(); // reset a2600 cpu
		m_tia->reset(); // need to reset tia or will segfault
//      this->reset(); // reset whole device, too much?
		m_byte_waiting_flag_apple = 0;
		m_byte_waiting_flag_vcs = 0;
	 }
}

static INPUT_PORTS_START( a2600_frob )
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_CONFSETTING(    0x08, "Color" )
	PORT_CONFSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_CONFSETTING(    0x40, "A" )
	PORT_CONFSETTING(    0x00, "B" )
	PORT_CONFNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_CONFSETTING(    0x80, "A" )
	PORT_CONFSETTING(    0x00, "B" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(a2600_frob_base_device::reset_a2600), 0x00) PORT_NAME("Reset 2600") PORT_CODE(KEYCODE_MINUS_PAD)
INPUT_PORTS_END


void a2600_frob_base_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, m_xtal / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_frob_base_device::a2600_mem);


	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(m_xtal, 228, 26, 26 + 160 + 16, 262, 24 , 24 + 192 + 31);
	m_screen->set_screen_update("tia_video", FUNC(a2frobtia_video_device::screen_update));

	/* video hardware */
	A2FROBTIA_NTSC_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(a2600_frob_base_device::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(a2600_frob_base_device::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(a2600_frob_base_device::a2600_tia_vsync_callback));
	m_tia->set_screen("tia_screen"); // need to specify screen

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", m_xtal/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
	MOS6532(config, m_riot, m_xtal / 3);
	m_riot->pa_rd_callback().set(FUNC(a2600_frob_base_device::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(a2600_frob_base_device::switch_A_w));
	m_riot->pb_rd_callback().set_ioport("SWB");
	m_riot->pb_wr_callback().set(FUNC(a2600_frob_base_device::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(a2600_frob_base_device::irq_callback));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");
}

// actually I think the a2600 doesn't have any rom in this context,
// since the frob supplies the ram (acting as ROM)
ROM_START(a2600_frob)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
ROM_END

tiny_rom_entry const * a2600_frob_base_device::device_rom_region() const
{
	return ROM_NAME(a2600_frob);
}

ioport_constructor a2600_frob_base_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a2600_frob);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_FROB, device_a2bus_card_interface, a2600_frob_base_device, "a2frob", "Apple II Frob Card")
