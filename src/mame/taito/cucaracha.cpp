// license:BSD-3-Clause
// copyright-holders:AJR, Angelo Salese
/**************************************************************************************************

    La Cucaracha  (c) 1992 Taito

    Mechanical whack-a-mole with a dot-matrix LED screen.
    Available in English and Spanish (latter is undumped).
    Released in Japan as Gokidetor (ゴキデター) (undumped).

    Up to 6 machines can be linked in a "competition mode" (fastest wins).

    TODO:
    - cockroach motor;
    - artwork layout, feasible but actual dimensions are unknown;
    - PWM sound;
    - lamp on freeze SW;
    - comms with other machines, bitbanger?
    - hunt for a pinout sheet: game employs 9 connectors labeled from A to G then N1 and N2,
      no JAMMA. Available manual just have 3 of them at last page, and with completely different
      labels.

    NOTES:
    - to enter service mode hold 9 at startup;

**************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
//#include "machine/m66240.h"
#include "machine/te7750.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "taitosnd.h"

#include "speaker.h"

#include "cucaracha.lh"

namespace {

class cucaracha_state : public driver_device
{
public:
	cucaracha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_program_bank(*this, "program_bank")
		, m_vram(*this, "vram")
		, m_led_matrix(*this, "ledmatrix%u", 0U)
	{ }

	void cucaracha(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void out4_w(uint8_t data);
	void out5_w(uint8_t data);
	void out6_w(uint8_t data);
	void out7_w(uint8_t data);
	void out8_w(uint8_t data);
	void out9_w(uint8_t data);
	void ym_porta_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_program_bank;
	required_shared_ptr<uint8_t> m_vram;
	output_finder<16 * 64> m_led_matrix;

	void led_transfer_w(offs_t offset, u8 data);

	u8 m_scrollx = 0;
	u8 m_scrolly = 0;
};

void cucaracha_state::led_transfer_w(offs_t offset, u8 data)
{
	if (data)
		logerror("$d101: write %02x\n", data);

	// attract mode uses negative Y on cockroach display
	const u8 start_y = m_scrolly & 0x1f;
	const u8 start_x = m_scrollx >> 3;
	// TODO: fractional X scrolling however could be a possibility (currently unused by the game)
	if (m_scrollx & 7)
		popmessage("scroll frac %02x", m_scrollx);
	const u8 Y_SIZE = 16;
	const u8 X_SIZE = 64;

	for(int y = 0; y < Y_SIZE; y++)
	{
		const u32 dst_offset = y * X_SIZE;
		const u32 src_offset = (y + start_y) * 0x20;
		for(int x = 0; x < X_SIZE; x+= 4)
		{
			const u32 x_address = src_offset + (((x >> 2) + start_x) & 0x1f);
			for(int xi = 0; xi < 4; xi++)
			{
				const u8 pen = (m_vram[x_address & 0x3ff] >> ((3 - xi) * 2)) & 3;
				m_led_matrix[dst_offset + x + xi] = pen;
			}
		}
	}
}


void cucaracha_state::machine_start()
{
	m_program_bank->configure_entries(0, 8, memregion("program_rom")->base(), 0x2000);

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

void cucaracha_state::machine_reset()
{
	m_program_bank->set_entry(4);
}

void cucaracha_state::out4_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 4\n", data);
}

// cockroach control starting from here up to out7_w?
void cucaracha_state::out5_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 5\n", data);
}

void cucaracha_state::out6_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 6\n", data);
}

// ---- --x- coin lockout?
void cucaracha_state::out7_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 7\n", data);
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));
}

// serial comms?
void cucaracha_state::out8_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 8\n", data & 0x3f);
}

void cucaracha_state::out9_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 9\n", data);
	// assumed, writes a 4 when displaying ERR 5
	m_program_bank->set_entry(data & 0xf);
}

void cucaracha_state::ym_porta_w(uint8_t data)
{
	if (data != 0x40)
		logerror("Writing %02X to YM2203 port A\n", data);
}


void cucaracha_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program_rom", 0);
	map(0x8000, 0x9fff).bankr(m_program_bank);
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xc3ff).ram().share("vram");
	// LED config, at start of irq service
	map(0xd000, 0xd000).lw8(NAME([this] (offs_t offset, u8 data) { m_scrollx = data; }));
	map(0xd001, 0xd001).lw8(NAME([this] (offs_t offset, u8 data) { m_scrolly = data; }));
	map(0xd101, 0xd101).w(FUNC(cucaracha_state::led_transfer_w));
	// d1c0 = ?output
	map(0xd800, 0xd80f).rw("te7750", FUNC(te7750_device::read), FUNC(te7750_device::write));
	//map(0xda00, 0xda01).w("pwm", FUNC(m66240_device::write));
	// de00 ?input
	map(0xdf00, 0xdf00).portr("DSW3");
	map(0xe000, 0xe000).portr("DSW1");
	map(0xe001, 0xe001).portr("DSW2");
	map(0xe002, 0xe003).nopr(); // ?input
	map(0xf000, 0xf000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xf001, 0xf001).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	// f600 ?output
	// f700 ?output
}


void cucaracha_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	// $b001 accessed as mirror for game start samples
	map(0xb000, 0xb001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

INPUT_PORTS_START( cucaracha )
	PORT_START("IN1")
	// cockroach 1 strike ON/front sensor/<spare>/rear sensor
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	// cockroach 2 strike ON/front sensor/<spare>/rear sensor
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	// cockroach 3 strike ON/front sensor/<spare>/rear sensor
	PORT_DIPNAME( 0x01, 0x00, "IN2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	// cockroach 4 strike ON/front sensor/<spare>/rear sensor
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	// cockroach 5 strike ON/front sensor/<spare>/rear sensor
	PORT_DIPNAME( 0x01, 0x00, "IN3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) // insecticide/freeze switch

	PORT_START("IN8")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, "IN8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) // Ticket dispenser
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	// "Ticket Vendor"
	PORT_DIPNAME( 0x04, 0x04, "Ticket Dispenser" ) PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x18, 0x18, "Game Type" ) PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x18, "1-Player Game" )
	PORT_DIPSETTING(    0x10, "2-Players Game" )
	PORT_DIPSETTING(    0x08, "2-to-6 Players Game" )
	// TODO: translate from Taito-ese these two lines
	PORT_DIPSETTING(    0x00, "Ageing" ) // free play?
	PORT_DIPNAME( 0x60, 0x60, "Specified Score of Communication Game" ) PORT_DIPLOCATION("SW1:!6,!7")
	PORT_DIPSETTING(    0x60, "60" )
	PORT_DIPSETTING(    0x40, "80" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Ticket Rate" ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x03, "x 1" )
	PORT_DIPSETTING(    0x02, "x 0.7" )
	PORT_DIPSETTING(    0x01, "x 1.5" )
	PORT_DIPSETTING(    0x00, "x 2" )
	// labeled from A to D
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	// "The appearing quantity of cockroach mechanisms when Freeze SW is pressed"
	PORT_DIPNAME( 0xe0, 0xe0, "Cockroach Quantity on Freeze SW" ) PORT_DIPLOCATION("SW2:!6,!7,!8")
	PORT_DIPSETTING(    0xe0, "3" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0xa0, "2" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "3 (duplicate)" )
	PORT_DIPSETTING(    0x20, "3 (duplicate)" )
	PORT_DIPSETTING(    0x00, "3 (duplicate)" )

	PORT_START("DSW3")
	// TODO: rotary switch
	PORT_DIPNAME( 0x0f, 0x0f, "Communication ID" ) PORT_DIPLOCATION("SW3:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x0f, "0" )
	PORT_DIPSETTING(    0x0e, "1" )
	PORT_DIPSETTING(    0x0d, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x0b, "4" )
	PORT_DIPSETTING(    0x0a, "5" )
	PORT_DIPSETTING(    0x09, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x07, "8 (unused)" )
	PORT_DIPSETTING(    0x06, "9 (unused)" )
	PORT_DIPSETTING(    0x05, "A (unused)" )
	PORT_DIPSETTING(    0x04, "B (unused)" )
	PORT_DIPSETTING(    0x03, "C (unused)" )
	PORT_DIPSETTING(    0x02, "D (unused)" )
	PORT_DIPSETTING(    0x01, "E (unused)" )
	PORT_DIPSETTING(    0x00, "F (unused)" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void cucaracha_state::cucaracha(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(16'000'000) / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &cucaracha_state::main_map);
	// TODO: pinpoint IRQ sources
	// game engine runs with this irq, ~100 Hz for lip sync with the cockroach voice at game start
	m_maincpu->set_periodic_int(FUNC(cucaracha_state::irq0_line_hold), attotime::from_hz(100));
	// NMI related to E002 input and TE7750 port 7

	te7750_device &te7750(TE7750(config, "te7750"));
	te7750.ios_cb().set_constant(3);
	te7750.in_port1_cb().set_ioport("IN1");
	te7750.in_port2_cb().set_ioport("IN2");
	te7750.in_port3_cb().set_ioport("IN3");
	te7750.out_port4_cb().set(FUNC(cucaracha_state::out4_w));
	te7750.out_port5_cb().set(FUNC(cucaracha_state::out5_w));
	te7750.out_port6_cb().set(FUNC(cucaracha_state::out6_w));
	te7750.out_port7_cb().set(FUNC(cucaracha_state::out7_w));
	te7750.in_port8_cb().set_ioport("IN8");
	te7750.out_port8_cb().set(FUNC(cucaracha_state::out8_w));
	te7750.out_port9_cb().set(FUNC(cucaracha_state::out9_w));

	z80_device &soundcpu(Z80(config, "soundcpu", 4000000));
	soundcpu.set_addrmap(AS_PROGRAM, &cucaracha_state::sound_map);

	pc060ha_device &ciu(PC060HA(config, "ciu"));
	ciu.nmi_callback().set_inputline("soundcpu", INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline("soundcpu", INPUT_LINE_RESET);

	config.set_default_layout(layout_cucaracha);

	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 3000000));
	ymsnd.irq_handler().set_inputline("soundcpu", 0);
	ymsnd.port_a_write_callback().set(FUNC(cucaracha_state::ym_porta_w));
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.80);

	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // clock frequency & pin 7 not verified
}

ROM_START( cucaracha )
	ROM_REGION( 0x20000, "program_rom", 0 )
	ROM_LOAD( "ic2", 0, 0x20000, CRC(f9dbca28) SHA1(b2f6d6b66bfa5e5ca7c26a0709f7136bf9e1a42e) )
	// 8000-FFFF are graphics; 10000-1FFFF is unused

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic34", 0, 0x10000, CRC(fd06305d) SHA1(7889f0c360650bfd0fe593c522685a978879bfee) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "ic87", 0, 0x40000, CRC(adb9fd16) SHA1(59aae5ac26cae30b961b30e17fd494094aa7aa15) )

	ROM_REGION( 0x8000, "pals", 0 ) // unprotected
	ROM_LOAD( "d33-01.pal16l8a.ic20", 0x0000, 0x104, CRC(6d6a8601) SHA1(b3e343358f8f9334b0befefef80f2a0b53ae0cc7) )
	ROM_LOAD( "d33-02.pal16l8b.ic62", 0x1000, 0x104, CRC(1e2d2e73) SHA1(3cbb95f26daed685ac8ad3d324e2c10ff444378a) )
	ROM_LOAD( "d33-03.pal16l8a.ic70", 0x2000, 0x104, CRC(f18b8ad0) SHA1(eabaccd50e72520c5fd91fb27cbd88962e8e82c2) )
	ROM_LOAD( "d33-04.pal16l8b.ic81", 0x3000, 0x104, CRC(b5690dd6) SHA1(0118d272b2c66e3faade1f4f7328167697fd0649) )
	ROM_LOAD( "d33-05.pal16l8b.ic93", 0x4000, 0x104, CRC(81b5ce19) SHA1(7602c8aa22ea3b3a64633ce9b55a45cfd20167cb) )
	ROM_LOAD( "d33-06.pal20l8b.ic44", 0x5000, 0x144, CRC(bf4eeb17) SHA1(d78f65eacf1c7893f87b9fe8be0e5e1b28af7c7a) )
ROM_END

ROM_START( cucaracha2 )
	ROM_REGION( 0x20000, "program_rom", 0 )
	ROM_LOAD( "ic2.rom", 0, 0x20000, CRC(03bf24d1) SHA1(ef63a5be25d77ac20984402cc45137d292a9fa1d) )
	// 8000-FFFF are graphics; 10000-1FFFF is unused

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic34.rom", 0, 0x10000, CRC(fd06305d) SHA1(7889f0c360650bfd0fe593c522685a978879bfee) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "ic87.rom", 0, 0x40000, CRC(adb9fd16) SHA1(59aae5ac26cae30b961b30e17fd494094aa7aa15) )

	ROM_REGION( 0x8000, "pals", 0 ) // PALs missing for this board, using the ones from the parent.
	ROM_LOAD( "d33-01.pal16l8a.ic20", 0x0000, 0x104, CRC(6d6a8601) SHA1(b3e343358f8f9334b0befefef80f2a0b53ae0cc7) )
	ROM_LOAD( "d33-02.pal16l8b.ic62", 0x1000, 0x104, CRC(1e2d2e73) SHA1(3cbb95f26daed685ac8ad3d324e2c10ff444378a) )
	ROM_LOAD( "d33-03.pal16l8a.ic70", 0x2000, 0x104, CRC(f18b8ad0) SHA1(eabaccd50e72520c5fd91fb27cbd88962e8e82c2) )
	ROM_LOAD( "d33-04.pal16l8b.ic81", 0x3000, 0x104, CRC(b5690dd6) SHA1(0118d272b2c66e3faade1f4f7328167697fd0649) )
	ROM_LOAD( "d33-05.pal16l8b.ic93", 0x4000, 0x104, CRC(81b5ce19) SHA1(7602c8aa22ea3b3a64633ce9b55a45cfd20167cb) )
	ROM_LOAD( "d33-06.pal20l8b.ic44", 0x5000, 0x144, CRC(bf4eeb17) SHA1(d78f65eacf1c7893f87b9fe8be0e5e1b28af7c7a) )
ROM_END

} // Anonymous namespace

GAME( 1992, cucaracha,  0,         cucaracha, cucaracha, cucaracha_state, empty_init, ROT0, "Taito", "La Cucaracha (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1992, cucaracha2, cucaracha, cucaracha, cucaracha, cucaracha_state, empty_init, ROT0, "Taito", "La Cucaracha (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
