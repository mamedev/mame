// license:BSD-3-Clause
// copyright-holders: Xing Xing, David Haywood

/*

IGS ARM7 (IGS027A) based Mahjong / Gambling platform(s) with XA sub-cpu
These games use the IGS027A processor.

Triple Fever (V105US) (tripfevb) hangs after paying out tickets, with the MCU
apparently attempting serial communication with something.

*/

#include "emu.h"

#include "igs017_igs031.h"
#include "igs027a.h"
#include "pgmcrypt.h"
#include "xamcu.h"

#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"

#include "sound/okim6295.h"

#include "screen.h"
#include "speaker.h"

#include "crzybugs.lh"
#include "tripfev.lh"

#define LOG_DEBUG       (1U << 1)
//#define VERBOSE         (LOG_DEBUG)
#include "logmacro.h"

namespace {

class igs_m027xa_state : public driver_device
{
public:
	igs_m027xa_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_xa(*this, "xa"),
		m_ppi(*this, "ppi8255"),
		m_igs017_igs031(*this, "igs017_igs031"),
		m_oki(*this, "oki"),
		m_screen(*this, "screen"),
		m_ticket(*this, "ticket"),
		m_external_rom(*this, "user1"),
		m_io_test(*this, "TEST%u", 0U),
		m_io_dsw(*this, "DSW%u", 1U),
		m_out_lamps(*this, "lamp%u", 1U)
	{ }

	void igs_mahjong_xa(machine_config &config);
	void igs_mahjong_xa_xor(machine_config &config);

	void init_crzybugs();
	void init_crzybugsj();
	void init_hauntedh();
	void init_tripfev();
	void init_wldfruit();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<igs027a_cpu_device> m_maincpu;
	required_device<igs_xa_mcu_subcpu_device> m_xa;
	required_device<i8255_device> m_ppi;
	required_device<igs017_igs031_device> m_igs017_igs031;
	required_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	optional_device<ticket_dispenser_device> m_ticket;
	required_region_ptr<u32> m_external_rom;

	optional_ioport_array<3> m_io_test;
	optional_ioport_array<3> m_io_dsw;

	output_finder<8> m_out_lamps;

	u32 m_xor_table[0x100];
	u8 m_io_select[2];

	bool m_irq_from_igs031;

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void pgm_create_dummy_internal_arm_region();
	void main_map(address_map &map) ATTR_COLD;
	void main_xor_map(address_map &map) ATTR_COLD;

	u32 external_rom_r(offs_t offset);

	void xor_table_w(offs_t offset, u8 data);

	u16 xa_r(offs_t offset, u16 mem_mask);
	void xa_w(offs_t offset, u16 data, u16 mem_mask);

	void output_w(u8 data);
	void lamps_w(u8 data);

	void xa_irq(int state);

	u32 gpio_r();
	void oki_bank_w(offs_t offset, u8 data);
	template <unsigned Select, unsigned First> u8 dsw_r();
	template <unsigned Select> void io_select_w(u8 data);
};


void igs_m027xa_state::machine_reset()
{
	m_irq_from_igs031 = false;
}

void igs_m027xa_state::machine_start()
{
	m_out_lamps.resolve();

	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);

	save_item(NAME(m_xor_table));
	save_item(NAME(m_io_select));

	save_item(NAME(m_irq_from_igs031));
}

void igs_m027xa_state::video_start()
{
	m_igs017_igs031->video_start();
}

/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027xa_state::main_map(address_map &map)
{
	map(0x08000000, 0x0807ffff).rom().region("user1", 0); // Game ROM

	map(0x18000000, 0x18007fff).ram().mirror(0xf8000).share("nvram");

	map(0x38000000, 0x38007fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x38008000, 0x38008003).umask32(0x000000ff).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x38009000, 0x38009003).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x3800c000, 0x3800c003).umask32(0x000000ff).w(FUNC(igs_m027xa_state::oki_bank_w));

	map(0x50000000, 0x500003ff).umask32(0x000000ff).w(FUNC(igs_m027xa_state::xor_table_w));

	map(0x58000000, 0x58000003).umask32(0x0000ffff).r(m_xa, FUNC(igs_xa_mcu_subcpu_device::response_r));
	map(0x58000000, 0x58000003).w(m_xa, FUNC(igs_xa_mcu_subcpu_device::cmd_w));
}

void igs_m027xa_state::main_xor_map(address_map &map)
{
	main_map(map);

	map(0x08000000, 0x0807ffff).r(FUNC(igs_m027xa_state::external_rom_r)); // Game ROM
}


INPUT_PORTS_START( base )
	PORT_START("TEST0")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )          // COINA
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TEST1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x7c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COINC

	PORT_START("TEST2")
	PORT_BIT( 0x001ff, IP_ACTIVE_LOW, IPT_UNUSED )      // IGS031 interrupt in bit 8 - see gpio_r
	PORT_BIT( 0x00200, IP_ACTIVE_LOW, IPT_CUSTOM )      PORT_READ_LINE_DEVICE_MEMBER("xa", igs_xa_mcu_subcpu_device, irq_r)
	PORT_BIT( 0xffc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )
INPUT_PORTS_END

INPUT_PORTS_START( crzybugs )
	PORT_INCLUDE(base)

	PORT_MODIFY("TEST0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Play")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )    PORT_NAME("Big")

	PORT_MODIFY("TEST1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Stop All Reels")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_NAME("Ticket")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM )         PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)

	PORT_MODIFY("TEST2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )     PORT_NAME("Stop Reel 2 / Small")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )     PORT_NAME("Stop Reel 3 / Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )     PORT_NAME("Stop Reel 1 / Double Up")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR(Demo_Sounds) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x01, DEF_STR(On) )
	PORT_DIPNAME( 0x02, 0x02, "Non Stop" )                 PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x04, 0x04, "Password" )                 PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x04, DEF_STR(Yes) )
	PORT_DIPNAME( 0x08, 0x08, "Odds Table" )               PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x08, DEF_STR(Yes) )
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game" )           PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x10, DEF_STR(On) )
	PORT_DIPNAME( 0x60, 0x60, "Symbol" )                   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "Both" )
	PORT_DIPSETTING(    0x20, "Both (duplicate)" )
	PORT_DIPSETTING(    0x40, "Fruit" )
	PORT_DIPSETTING(    0x60, "Bug" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Score Box" )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "10X" )
	PORT_DIPSETTING(    0x01, "10X (duplicate)" )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPSETTING(    0x03, DEF_STR(No) )
	PORT_DIPNAME( 0x04, 0x04, "Play Score" )               PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x08, 0x08, "Hand Count" )               PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x30, 0x30, "Hold Pair" )                PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, "Regular" )
	PORT_DIPSETTING(    0x10, "Georgia" )
	PORT_DIPSETTING(    0x00, "Georgia (duplicate)" )
	PORT_DIPNAME( 0x40, 0x40, "Auto Hold" )                PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
INPUT_PORTS_END

INPUT_PORTS_START( tripfev )
	PORT_INCLUDE(base)

	PORT_MODIFY("TEST0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP5 )     PORT_NAME("Stop Reel 5 / Play")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )     PORT_NAME("Stop Reel 1 / Big")

	PORT_MODIFY("TEST1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Stop All Reels")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_NAME("Ticket")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM )         PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)

	PORT_MODIFY("TEST2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )     PORT_NAME("Stop Reel 3 / Small")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP4 )     PORT_NAME("Stop Reel 4 / Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )     PORT_NAME("Stop Reel 2 / Double Up")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR(Demo_Sounds) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x01, DEF_STR(On) )
	PORT_DIPNAME( 0x02, 0x02, "Non Stop" )                 PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x04, 0x04, "Password" )                 PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x08, 0x08, "Odds Table" )               PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x30, 0x30, "Score Box" )                PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "10X" )
	PORT_DIPSETTING(    0x10, "10X (duplicate)" )
	PORT_DIPSETTING(    0x20, DEF_STR(Yes) )
	PORT_DIPSETTING(    0x30, DEF_STR(No) )
	PORT_DIPNAME( 0x40, 0x40, "Play Score" )               PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x80, 0x80, "Auto Take" )                PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Hand Count" )               PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x06, 0x06, "Hold Pair" )                PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR(Off) )
	PORT_DIPSETTING(    0x04, "Regular" )
	PORT_DIPSETTING(    0x02, "Georgia" )
	PORT_DIPSETTING(    0x00, "Georgia (duplicate)" )
	PORT_DIPNAME( 0x08, 0x08, "Auto Ticket" )              PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
INPUT_PORTS_END


void igs_m027xa_state::output_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // one pulse per COINA accepted
	machine().bookkeeping().coin_counter_w(2, BIT(data, 1)); // one pulse per key-in accepted
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2)); // one pulse per COINC accepted
	// bits 3 and 4 have something to do with counting credits paid out by ticket and key-out
	if (m_ticket)
		m_ticket->motor_w(BIT(data, 6));
}

void igs_m027xa_state::lamps_w(u8 data)
{
	// active high outputs
	// +------+----------------+----------------+
	// | lamp | crzybugs       | tripfev        |
	// +------+----------------+----------------+
	// |  1   | start/stop all | start/stop all |
	// |  2   | stop 2/small   | stop 3/small   |
	// |  3   | bet            | stop 5/play    |
	// |  4   | stop 3/take    | stop 4/take    |
	// |  5   | stop 1/double  | stop 2/double  |
	// |  6   | big            | stop 1/big     |
	// |  7   |                |                |
	// |  8   |                |                |
	// +------+----------------+----------------+
	for (unsigned i = 0; 8 > i; ++i)
		m_out_lamps[i] = BIT(data, i);
}


u32 igs_m027xa_state::gpio_r()
{
	u32 ret = m_io_test[2].read_safe(0xfffff);
	if (m_irq_from_igs031 && m_igs017_igs031->get_irq_enable())
		ret &= ~(u32(1) << 8);
	return ret;
}

void igs_m027xa_state::oki_bank_w(offs_t offset, u8 data)
{
	m_oki->set_rom_bank(data & 7);
}

template <unsigned Select, unsigned First>
u8 igs_m027xa_state::dsw_r()
{
	u8 data = 0xff;

	for (int i = First; i < m_io_dsw.size(); i++)
		if (!BIT(m_io_select[Select], i - First))
			data &= m_io_dsw[i].read_safe(0xff);
	return data;
}

template <unsigned Select>
void igs_m027xa_state::io_select_w(u8 data)
{
	m_io_select[Select] = data;
}


void igs_m027xa_state::xa_irq(int state)
{
	if (state)
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time()); // TODO: when is the IRQ line cleared?
}

u32 igs_m027xa_state::external_rom_r(offs_t offset)
{
	return m_external_rom[offset] ^ m_xor_table[offset & 0x00ff];
}


void igs_m027xa_state::xor_table_w(offs_t offset, u8 data)
{
	m_xor_table[offset] = (u32(data) << 24) | (u32(data) << 8);
}



TIMER_DEVICE_CALLBACK_MEMBER(igs_m027xa_state::interrupt)
{
	int scanline = param;

	switch (scanline)
	{
	case 0:
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time()); // vbl?
		m_irq_from_igs031 = false;
		break;
	case 240:
		if (m_igs017_igs031->get_irq_enable())
		{
			m_irq_from_igs031 = true;
			m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time()); // TODO: when is the IRQ line cleared?
		}
		break;
	}
}


void igs_m027xa_state::igs_mahjong_xa(machine_config &config)
{
	IGS027A(config, m_maincpu, 22'000'000); // Crazy Bugs has a 22MHz crystal, what about the others?
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027xa_state::main_map);
	m_maincpu->in_port().set(FUNC(igs_m027xa_state::gpio_r));
	m_maincpu->out_port().set(FUNC(igs_m027xa_state::io_select_w<1>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	IGS_XA_SUBCPU(config, m_xa, 10'000'000); // MX10EXAQC (Philips 80C51 XA) unknown frequency
	m_xa->irq().set(FUNC(igs_m027xa_state::xa_irq));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update("igs017_igs031", FUNC(igs017_igs031_device::screen_update));
	m_screen->set_palette("igs017_igs031:palette");

	TIMER(config, "scantimer").configure_scanline(FUNC(igs_m027xa_state::interrupt), "screen", 0, 1);

	// crzybugs: PPI port A = input, port B = output, port C = output
	I8255A(config, m_ppi);
	m_ppi->tri_pa_callback().set_constant(0x00);
	m_ppi->tri_pb_callback().set_constant(0x00);
	m_ppi->tri_pc_callback().set_constant(0x00);
	m_ppi->out_pb_callback().set(FUNC(igs_m027xa_state::output_w));
	m_ppi->out_pc_callback().set(FUNC(igs_m027xa_state::lamps_w));

	IGS017_IGS031(config, m_igs017_igs031, 0);
	m_igs017_igs031->set_text_reverse_bits(true);
	m_igs017_igs031->in_pa_callback().set(NAME((&igs_m027xa_state::dsw_r<1, 0>)));
	m_igs017_igs031->in_pb_callback().set_ioport("TEST0");
	m_igs017_igs031->in_pc_callback().set_ioport("TEST1");

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(200));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void igs_m027xa_state::igs_mahjong_xa_xor(machine_config &config)
{
	igs_mahjong_xa(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027xa_state::main_xor_map);
}

// prg at u34
// text at u15
// cg at u32 / u12
// samples at u3

ROM_START( haunthig )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "haunthig_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "hauntedhouse_ver-109us.u34", 0x000000, 0x80000, CRC(300fed78) SHA1(afa4c8855cd780c57d4f92ea6131ed4e77063268) )

	ROM_REGION( 0x10000, "xa:mcu", 0 )
	ROM_LOAD( "hauntedhouse.u17", 0x000000, 0x10000, BAD_DUMP CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // not dumped for this set

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "haunted-h_text.u15", 0x000000, 0x80000, CRC(c23f48c8) SHA1(0cb1b6c61611a081ae4a3c0be51812045ff632fe) )

	ROM_REGION( 0x800000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "haunted-h_cg.u32",  0x000000, 0x400000, BAD_DUMP CRC(e0ea10e6) SHA1(e81be78fea93e72d4b1f4c0b58560bda46cf7948) ) // not dumped for this set, FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "haunted-h_ext.u12", 0x400000, 0x400000, BAD_DUMP CRC(662eb883) SHA1(831ebe29e1e7a8b2c2fff7fbc608975771c3486c) ) // not dumped for this set, FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 ) // Oki M6295 samples
	ROM_LOAD( "haunted-h_sp.u3", 0x00000, 0x200000,  BAD_DUMP CRC(fe3fcddf) SHA1(ac57ab6d4e4883747c093bd19d0025cf6588cb2c) ) // not dumped for this set

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "hu_u38a.u38", 0x000, 0x117, NO_DUMP ) // ATF16V8B, protected
	ROM_LOAD( "hu_u39.u39",  0x200, 0x2dd, CRC(75f58b46) SHA1(7cb136a41899ddd50c95a67ca6353ce5d8d92149) ) // AT22V10
ROM_END

ROM_START( haunthiga ) // IGS PCB-0575-04-HU - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 2x 8-dip banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "h2_igs027a", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'H2'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "hauntedhouse_ver-101us.u34", 0x000000, 0x80000, CRC(4bf045d4) SHA1(78c848fd69961df8d9b75f92ad57c3534fbf08db) )

	ROM_REGION( 0x10000, "xa:mcu", 0 )
	ROM_LOAD( "hauntedhouse.u17", 0x000000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked J9, not read protected?

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "haunted-h_text.u15", 0x000000, 0x80000, CRC(c23f48c8) SHA1(0cb1b6c61611a081ae4a3c0be51812045ff632fe) )

	ROM_REGION( 0x800000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "haunted-h_cg.u32",  0x000000, 0x400000, CRC(e0ea10e6) SHA1(e81be78fea93e72d4b1f4c0b58560bda46cf7948) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "haunted-h_ext.u12", 0x400000, 0x400000, CRC(662eb883) SHA1(831ebe29e1e7a8b2c2fff7fbc608975771c3486c) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 ) // Oki M6295 samples
	ROM_LOAD( "haunted-h_sp.u3", 0x00000, 0x200000, CRC(fe3fcddf) SHA1(ac57ab6d4e4883747c093bd19d0025cf6588cb2c) )

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "hu_u38a.u38", 0x000, 0x117, NO_DUMP ) // ATF16V8B, protected
	ROM_LOAD( "hu_u39.u39",  0x200, 0x2dd, CRC(75f58b46) SHA1(7cb136a41899ddd50c95a67ca6353ce5d8d92149) ) // AT22V10
ROM_END

ROM_START( crzybugs ) // IGS PCB-0447-05-GM - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 3x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m7_igs27a.u37", 0x00000, 0x4000, CRC(1b20532c) SHA1(e08d0110a843915a8ba8627ae6d3947cccc22048) ) // sticker marked 'M7'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-204us.u23", 0x000000, 0x80000, CRC(d1232462) SHA1(685a292f39bf57a80d6ef31289cf9f673ba06dd4) ) // MX27C4096

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked J9
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u10.u10", 0x000000, 0x80000, CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) )

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_cg.u19",  0x000000, 0x200000, CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazybugs_sp.u15", 0x000000, 0x200000, CRC(591b315b) SHA1(fda1816d83e202170dba4afc6e7898b706a76087) ) // M27C160
ROM_END

ROM_START( crzybugsa )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m7_igs27a.u37", 0x00000, 0x4000, CRC(1b20532c) SHA1(e08d0110a843915a8ba8627ae6d3947cccc22048) ) // sticker marked 'M7' (not verified for this set)

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-202us.u23", 0x000000, 0x80000, CRC(210da1e6) SHA1(c726497bebd25d6a9053e331b4c26acc7e2db0b2) ) // MX27C4096

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU)
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u10.u10", 0x000000, 0x80000, CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) ) // M27C4002

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_cg.u19",  0x000000, 0x200000, CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // M27C160, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazy_bugs_sp.u15", 0x000000, 0x200000, CRC(591b315b) SHA1(fda1816d83e202170dba4afc6e7898b706a76087) ) // M27C160
ROM_END

ROM_START( crzybugsb )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m7_igs27a.u37", 0x00000, 0x4000, CRC(1b20532c) SHA1(e08d0110a843915a8ba8627ae6d3947cccc22048) ) // sticker marked 'M7' (not verified for this set)

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-202us.u23", 0x000000, 0x80000, CRC(129e36e9) SHA1(53f20bc3792249de8ef276f84283baa9abd30acd) ) // MX27C4096

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU)
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u10.u10", 0x000000, 0x80000, BAD_DUMP CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) ) // not dumped for this set

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_cg.u19",  0x000000, 0x200000, BAD_DUMP CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // not dumped for this set, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazy_bugs_sp.u15", 0x000000, 0x200000, BAD_DUMP CRC(591b315b) SHA1(fda1816d83e202170dba4afc6e7898b706a76087) ) // not dumped for this set
ROM_END

ROM_START( crzybugsj ) // IGS PCB-0575-04-HU - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 2x 8-dip banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m6.u42", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'M6'

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-103jp.u34", 0x000000, 0x200000, CRC(1e35ed79) SHA1(0e4f8b706cdfcaf2aacdc40eec422df9d865b311) )

	ROM_REGION( 0x10000, "xa:mcu", 0 )
	ROM_LOAD( "e9.u17", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked E9, same as haunthig

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u15.u15", 0x000000, 0x80000, CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) )
	// u14 not populated

	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_ani-cg-u32.u32",  0x000000, 0x200000, CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	// u12 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazy_bugs_sp_u3.u3", 0x000000, 0x200000,  CRC(b15974a1) SHA1(82509902bbb33a2120d815e7879b9b8591a29976) )

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "hu_u38.u38", 0x000, 0x117, NO_DUMP ) // ATF16V8B, protected
	ROM_LOAD( "hu_u39.u39", 0x200, 0x2dd, CRC(75f58b46) SHA1(7cb136a41899ddd50c95a67ca6353ce5d8d92149) ) // AT22V10
ROM_END

ROM_START( tripfev ) // IGS PCB-0575-02-HU PCB
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "w1_igs027a.u42", 0x00000, 0x4000, CRC(a40ec1f8) SHA1(f6f7005d61522934758fd0a98bf383c6076b6afe) ) // sticker marked 'W1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v108.u34", 0x000000, 0x80000, CRC(f0ad18ed) SHA1(95239e7b9925f12008051140afb74d47a5da4a3a) ) // 27C4096

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked P7
	ROM_LOAD( "p7.u17", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked P7, but same as haunthig, crzybugsj

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "triple_fever_text.u15", 0x000000, 0x80000, CRC(522a1030) SHA1(9a7a5ba9b26bceb0d251be6139c10e4655fc19ec) ) // M27C4002

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "triple_fever_cg.u32",  0x000000, 0x400000, CRC(cd45bbf2) SHA1(7f1cf270245bbe4604de2cacade279ab13584dbd) ) // M27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "triplef_sp.u3", 0x000000, 0x200000, CRC(98b9cafd) SHA1(3bf3971f0d9520c98fc6b1c2e77ab9c178d21c62) ) // M27C160
ROM_END

ROM_START( tripfeva ) // IGS PCB-0447-05-GM - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 3x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "w1_igs027a.u37", 0x00000, 0x4000, CRC(a40ec1f8) SHA1(f6f7005d61522934758fd0a98bf383c6076b6afe) ) // sticker marked 'W1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "triple_fever_u23_v107_us.u23", 0x000000, 0x80000, CRC(aa56d888) SHA1(0b8b2765079259b76ea803289841d867c33c8cb2) ) // 27C4096

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked P7
	ROM_LOAD( "p7.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked P7, but same as haunthig, crzybugsj

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "triple_fever_u10_text.u10", 0x000000, 0x80000, CRC(522a1030) SHA1(9a7a5ba9b26bceb0d251be6139c10e4655fc19ec) ) // M27C4002

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "triple_fever_u19_cg.u19",  0x000000, 0x400000, CRC(cd45bbf2) SHA1(7f1cf270245bbe4604de2cacade279ab13584dbd) ) // M27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "triplef_sp_u15.u15", 0x000000, 0x200000, CRC(98b9cafd) SHA1(3bf3971f0d9520c98fc6b1c2e77ab9c178d21c62) ) // M27C160
ROM_END

ROM_START( tripfevb ) // IGS PCB-0447-05-GM - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 3x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "w1_igs027a.u37", 0x00000, 0x4000, CRC(a40ec1f8) SHA1(f6f7005d61522934758fd0a98bf383c6076b6afe) ) // sticker marked 'W1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "u23 27c4096.bin", 0x000000, 0x80000, CRC(f870edda) SHA1(30d1c2d4c575749adbbf28b64eca1f35bcf7dfca) ) // 27C4096, unreadable label

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked P7
	ROM_LOAD( "p7.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked P7, but same as haunthig, crzybugsj

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "triple_fever_u10_text.u10", 0x000000, 0x80000, CRC(522a1030) SHA1(9a7a5ba9b26bceb0d251be6139c10e4655fc19ec) ) // M27C4002

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "triple_fever_u19_cg.u19",  0x000000, 0x400000, CRC(cd45bbf2) SHA1(7f1cf270245bbe4604de2cacade279ab13584dbd) ) // M27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "triplef_sp_u15.u15", 0x000000, 0x200000, CRC(98b9cafd) SHA1(3bf3971f0d9520c98fc6b1c2e77ab9c178d21c62) ) // M27C160
ROM_END

ROM_START( wldfruit ) // IGS PCB-0447-05-GM - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 3x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "w1.u37", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'W1?' (same label, but not the same as tripfev? or an error)

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "wild_fruit_v-208us.u23", 0x000000, 0x80000, CRC(d43398f1) SHA1(ecc4bd5cb6da16b35c63b843cf7beec1ab84ed9d) ) // M27C4002

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) marked J9
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "wild_fruit_text.u10", 0x000000, 0x80000, CRC(d6f0fd58) SHA1(5ddae5d4df53504dbb2e0fe9f7caea961c961ef8) ) // 27C4096

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "wild_fruit_cg.u19",  0x000000, 0x400000, CRC(119686a8) SHA1(22583c1a1018cfdd20f0ef696d91fa1f6e01ab00) ) // M27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "oki", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "wild_fruit_sp.u15", 0x000000, 0x200000, CRC(9da3e9dd) SHA1(7e447492713549e6be362d4aca6d223dad20771a) ) // M27C160
ROM_END


void igs_m027xa_state::pgm_create_dummy_internal_arm_region()
{
	u16 *temp16 = (u16 *)memregion("maincpu")->base();

	// fill with RX 14
	for (int i = 0; i < 0x4000 / 2; i += 2)
	{
		temp16[i] = 0xff1e;
		temp16[i +1] = 0xe12f;

	}

	// jump straight to external area
	temp16[(0x0000) / 2] = 0xd088;
	temp16[(0x0002) / 2] = 0xe59f;
	temp16[(0x0004) / 2] = 0x0680;
	temp16[(0x0006) / 2] = 0xe3a0;
	temp16[(0x0008) / 2] = 0xff10;
	temp16[(0x000a) / 2] = 0xe12f;
	temp16[(0x0090) / 2] = 0x0400;
	temp16[(0x0092) / 2] = 0x1000;
}


void igs_m027xa_state::init_hauntedh()
{
	hauntedh_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027xa_state::init_crzybugs()
{
	crzybugs_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}

void igs_m027xa_state::init_crzybugsj()
{
	crzybugsj_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027xa_state::init_tripfev()
{
	tripfev_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}

void igs_m027xa_state::init_wldfruit()
{
	wldfruit_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

} // anonymous namespace

// These use the MX10EXAQC (80c51XA from Philips)
// the PCBs are closer to igs_fear.cpp in terms of layout
GAME(  2008, haunthig,  0,        igs_mahjong_xa,     base,     igs_m027xa_state, init_hauntedh,  ROT0, "IGS", "Haunted House (IGS, V109US)", MACHINE_NOT_WORKING ) // IGS FOR V109US 2008 10 14
GAME(  2006, haunthiga, haunthig, igs_mahjong_xa,     base,     igs_m027xa_state, init_hauntedh,  ROT0, "IGS", "Haunted House (IGS, V101US)", MACHINE_NOT_WORKING ) // IGS FOR V101US 2006 08 23

GAMEL( 2009, crzybugs,  0,        igs_mahjong_xa_xor, crzybugs, igs_m027xa_state, init_crzybugs,  ROT0, "IGS", "Crazy Bugs (V204US)", 0, layout_crzybugs ) // IGS FOR V204US 2009 5 19
GAMEL( 2006, crzybugsa, crzybugs, igs_mahjong_xa_xor, crzybugs, igs_m027xa_state, init_crzybugs,  ROT0, "IGS", "Crazy Bugs (V202US)", 0, layout_crzybugs ) // IGS FOR V100US 2006 3 29 but also V202US string
GAMEL( 2005, crzybugsb, crzybugs, igs_mahjong_xa_xor, crzybugs, igs_m027xa_state, init_crzybugs,  ROT0, "IGS", "Crazy Bugs (V200US)", 0, layout_crzybugs ) // FOR V100US 2005 7 20 but also V200US string

GAME(  2007, crzybugsj, crzybugs, igs_mahjong_xa,     crzybugs, igs_m027xa_state, init_crzybugsj, ROT0, "IGS", "Crazy Bugs (V103JP)", MACHINE_NOT_WORKING ) // IGS FOR V101JP 2007 06 08

GAMEL( 2006, tripfev,   0,        igs_mahjong_xa_xor, tripfev,  igs_m027xa_state, init_tripfev,   ROT0, "IGS", "Triple Fever (V108US)", 0, layout_tripfev )
GAMEL( 2006, tripfeva,  tripfev,  igs_mahjong_xa_xor, tripfev,  igs_m027xa_state, init_tripfev,   ROT0, "IGS", "Triple Fever (V107US)", 0, layout_tripfev ) // IGS FOR V107US 2006 09 07
GAMEL( 2006, tripfevb,  tripfev,  igs_mahjong_xa_xor, tripfev,  igs_m027xa_state, init_tripfev,   ROT0, "IGS", "Triple Fever (V105US)", MACHINE_NOT_WORKING, layout_tripfev )

GAME(  200?, wldfruit,  0,        igs_mahjong_xa,     base,     igs_m027xa_state, init_wldfruit,  ROT0, "IGS", "Wild Fruit (V208US)", MACHINE_NOT_WORKING ) // IGS-----97----V208US
