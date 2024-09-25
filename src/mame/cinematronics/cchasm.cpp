// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

    Cinematronics Cosmic Chasm hardware

    driver by Mathis Rosenhauer

    Games supported:
        * Cosmic Chasm

    Known bugs:
        * none at this time

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/6840ptm.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/vector.h"

#include "screen.h"
#include "speaker.h"


namespace {

class cchasm_state : public driver_device
{
public:
	cchasm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ctc(*this, "ctc"),
		m_audiocpu(*this, "audiocpu"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch3(*this, "soundlatch3"),
		m_soundlatch4(*this, "soundlatch4"),
		m_ram(*this, "ram"),
		m_inputs(*this, "IN%u", 1U)
	{
	}

	void cchasm(machine_config &config);

	INPUT_CHANGED_MEMBER(set_coin_flag);

protected:
	virtual void machine_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(refresh_end);

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80_device> m_audiocpu;
	required_device<dac_bit_interface> m_dac1;
	required_device<dac_bit_interface> m_dac2;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_device<generic_latch_8_device> m_soundlatch4;

	required_shared_ptr<uint16_t> m_ram;

	required_ioport_array<3> m_inputs;

	int m_sound_flags = 0;
	int m_coin_flag = 0;
	int m_output[2]{};
	int m_xcenter = 0;
	int m_ycenter = 0;
	emu_timer *m_refresh_end_timer = nullptr;

	void led_w(offs_t offset, uint16_t data);
	void refresh_control_w(offs_t offset, uint8_t data);
	void reset_coin_flag_w(uint8_t data);
	uint8_t coin_sound_r();
	uint8_t soundlatch2_r();
	void soundlatch4_w(uint8_t data);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t io_r(offs_t offset);
	void ctc_timer_1_w(int state);
	void ctc_timer_2_w(int state);

	void refresh();

	void memmap(address_map &map) ATTR_COLD;
	void sound_memmap(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void cchasm_state::memmap(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x040000, 0x04000f).rw("6840ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x050000, 0x050000).w(FUNC(cchasm_state::refresh_control_w));
	map(0x060000, 0x060001).portr("DSW").w(FUNC(cchasm_state::led_w));
	map(0x070000, 0x070001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0xf80000, 0xf800ff).rw(FUNC(cchasm_state::io_r), FUNC(cchasm_state::io_w));
	map(0xffb000, 0xffffff).ram().share(m_ram);
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void cchasm_state::sound_memmap(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x5000, 0x53ff).ram();
	map(0x6000, 0x6001).mirror(0xf9e).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x6000, 0x6000).mirror(0xf9e).r(FUNC(cchasm_state::coin_sound_r));
	map(0x6001, 0x6001).mirror(0xf9e).r("ay1", FUNC(ay8910_device::data_r));
	map(0x6020, 0x6021).mirror(0xf9e).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x6021, 0x6021).mirror(0xf9e).r("ay2", FUNC(ay8910_device::data_r));
	map(0x6040, 0x6040).mirror(0xf9e).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(m_soundlatch3, FUNC(generic_latch_8_device::write));
	map(0x6041, 0x6041).mirror(0xf9e).rw(FUNC(cchasm_state::soundlatch2_r), FUNC(cchasm_state::soundlatch4_w));
	map(0x6061, 0x6061).mirror(0xf9e).w(FUNC(cchasm_state::reset_coin_flag_w));
	map(0x7041, 0x7041).noprw(); // TODO
}

void cchasm_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cchasm )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "40000" )
	PORT_DIPSETTING(    0x04, "60000" )
	PORT_DIPSETTING(    0x02, "80000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Frequency" )
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPSETTING(    0x10, "Every" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cchasm_state, set_coin_flag, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cchasm_state, set_coin_flag, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cchasm_state, set_coin_flag, 0)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test 1") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* Test 2, not used in cchasm */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) /* Test 3, not used in cchasm */
INPUT_PORTS_END



void cchasm_state::led_w(offs_t offset, uint16_t data)
{
	/*logerror("LED write %x to %x\n", data, offset);*/
}


void cchasm_state::reset_coin_flag_w(uint8_t data)
{
	if (m_coin_flag)
	{
		m_coin_flag = 0;
		m_ctc->trg0(m_coin_flag);
	}
}

INPUT_CHANGED_MEMBER(cchasm_state::set_coin_flag )
{
	if (!newval && !m_coin_flag)
	{
		m_coin_flag = 1;
		m_ctc->trg0(m_coin_flag);
	}
}

uint8_t cchasm_state::coin_sound_r()
{
	uint8_t coin = (m_inputs[2]->read() >> 4) & 0x7;
	return m_sound_flags | (m_coin_flag << 3) | coin;
}

uint8_t cchasm_state::soundlatch2_r()
{
	m_sound_flags &= ~0x80;
	m_ctc->trg2(0);
	return m_soundlatch2->read();
}

void cchasm_state::soundlatch4_w(uint8_t data)
{
	m_sound_flags |= 0x40;
	m_soundlatch4->write(data);
	m_maincpu->set_input_line(1, HOLD_LINE);
}

void cchasm_state::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//static int led;

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;
		switch (offset & 0xf)
		{
		case 0:
			m_soundlatch->write(data);
			break;
		case 1:
			m_sound_flags |= 0x80;
			m_soundlatch2->write(data);
			m_ctc->trg2(1);
			m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			break;
		case 2:
			//led = data;
			break;
		}
	}
}

uint16_t cchasm_state::io_r(offs_t offset)
{
	switch (offset & 0xf)
	{
	case 0x0:
		return m_soundlatch3->read() << 8;
	case 0x1:
		m_sound_flags &= ~0x40;
		return m_soundlatch4->read() << 8;
	case 0x2:
		return (m_sound_flags| (ioport("IN3")->read() & 0x07) | 0x08) << 8;
	case 0x5:
		return m_inputs[1]->read() << 8;
	case 0x8:
		return m_inputs[0]->read() << 8;
	default:
		return 0xff << 8;
	}
}


void cchasm_state::ctc_timer_1_w(int state)
{
	if (state) /* rising edge */
	{
		m_output[0] = !m_output[0];
		m_dac1->write(m_output[0]);
	}
}

void cchasm_state::ctc_timer_2_w(int state)
{
	if (state) /* rising edge */
	{
		m_output[1] = !m_output[1];
		m_dac2->write(m_output[1]);
	}
}


TIMER_CALLBACK_MEMBER(cchasm_state::refresh_end)
{
	m_maincpu->set_input_line(2, ASSERT_LINE);
}


void cchasm_state::refresh()
{
	constexpr int HALT   = 0;
	constexpr int JUMP   = 1;
	constexpr int COLOR  = 2;
	constexpr int SCALEY = 3;
	constexpr int POSY   = 4;
	constexpr int SCALEX = 5;
	constexpr int POSX   = 6;
	constexpr int LENGTH = 7;

	int pc = 0;
	int done = 0;
	int currentx = 0, currenty = 0;
	int scalex = 0, scaley = 0;
	int color = 0;
	int total_length = 1;   /* length of all lines drawn in a frame */
	int move = 0;

	m_vector->clear_list();

	while (!done)
	{
		int data = m_ram[pc];
		const int opcode = data >> 12;
		data &= 0xfff;
		if ((opcode > COLOR) && (data & 0x800))
			data |= 0xfffff000;

		pc++;

		switch (opcode)
		{
		case HALT:
			done=1;
			break;
		case JUMP:
			pc = data - 0xb00;
			logerror("JUMP to %x\n", data);
			break;
		case COLOR:
			color = vector_device::color444(data ^ 0xfff);
			break;
		case SCALEY:
			scaley = data << 5;
			break;
		case POSY:
			move = 1;
			currenty = m_ycenter + (data << 16);
			break;
		case SCALEX:
			scalex = data << 5;
			break;
		case POSX:
			move = 1;
			currentx = m_xcenter - (data << 16);
			break;
		case LENGTH:
			if (move)
			{
				m_vector->add_point (currentx, currenty, 0, 0);
				move = 0;
			}

			currentx -= data * scalex;
			currenty += data * scaley;

			total_length += abs(data);

			if (color)
				m_vector->add_point (currentx, currenty, color, 0xff);
			else
				move = 1;
			break;
		default:
			logerror("Unknown refresh proc opcode %x with data %x at pc = %x\n", opcode, data, pc-2);
			done = 1;
			break;
		}
	}
	/* Refresh processor runs with 6 MHz */
	m_refresh_end_timer->adjust(attotime::from_hz(6000000) * total_length);
}


void cchasm_state::refresh_control_w(offs_t offset, uint8_t data)
{
	switch (data)
	{
	case 0x37:
		refresh();
		break;
	case 0xf7:
		m_maincpu->set_input_line(2, CLEAR_LINE);
		break;
	}
}

void cchasm_state::machine_start()
{
	const rectangle &visarea = m_screen->visible_area();

	m_xcenter = visarea.xcenter() << 16;
	m_ycenter = visarea.ycenter() << 16;

	m_refresh_end_timer = timer_alloc(FUNC(cchasm_state::refresh_end), this);

	m_coin_flag = 0;
	m_sound_flags = 0;
	m_output[0] = 0;
	m_output[1] = 0;

	save_item(NAME(m_sound_flags));
	save_item(NAME(m_coin_flag));
	save_item(NAME(m_output));
}



/*************************************
 *
 *  CPU config
 *
 *************************************/

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void cchasm_state::cchasm(machine_config &config)
{
	constexpr XTAL CCHASM_68K_CLOCK(8'000'000);

	/* basic machine hardware */
	M68000(config, m_maincpu, CCHASM_68K_CLOCK);    /* 8 MHz (from schematics) */
	m_maincpu->set_addrmap(AS_PROGRAM, &cchasm_state::memmap);

	Z80(config, m_audiocpu, 3584229);       /* 3.58  MHz (from schematics) */
	m_audiocpu->set_daisy_config(daisy_chain);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cchasm_state::sound_memmap);
	m_audiocpu->set_addrmap(AS_IO, &cchasm_state::sound_portmap);

	Z80CTC(config, m_ctc, 3584229 /* same as "audiocpu" */);
	m_ctc->intr_callback().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<1>().set(FUNC(cchasm_state::ctc_timer_1_w));
	m_ctc->zc_callback<2>().set(FUNC(cchasm_state::ctc_timer_2_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	VECTOR(config, m_vector, 0);
	SCREEN(config, m_screen, SCREEN_TYPE_VECTOR);
	m_screen->set_refresh_hz(40);
	m_screen->set_size(400, 300);
	m_screen->set_visarea(0, 1024-1, 0, 768-1);
	m_screen->set_screen_update("vector", FUNC(vector_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);
	GENERIC_LATCH_8(config, m_soundlatch3);
	GENERIC_LATCH_8(config, m_soundlatch4);

	AY8910(config, "ay1", 1818182).add_route(ALL_OUTPUTS, "speaker", 0.15);

	AY8910(config, "ay2", 1818182).add_route(ALL_OUTPUTS, "speaker", 0.15);

	DAC_1BIT(config, m_dac1, 0).add_route(ALL_OUTPUTS, "speaker", 0.375);
	DAC_1BIT(config, m_dac2, 0).add_route(ALL_OUTPUTS, "speaker", 0.375);

	/* 6840 PTM */
	ptm6840_device &ptm(PTM6840(config, "6840ptm", CCHASM_68K_CLOCK/10));
	ptm.set_external_clocks(0, (CCHASM_68K_CLOCK / 10).value(), 0);
	ptm.irq_callback().set_inputline("maincpu", 4);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( cchasm )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chasm.u4",  0x000000, 0x001000, CRC(19244f25) SHA1(79deaae82da8d1b16d05bbac43ba900c4b1d9f26) )
	ROM_LOAD16_BYTE( "chasm.u12", 0x000001, 0x001000, CRC(5d702c7d) SHA1(cbdceed45a1112594fbcbeb6976edc932b32d518) )
	ROM_LOAD16_BYTE( "chasm.u8",  0x002000, 0x001000, CRC(56a7ce8a) SHA1(14c790dcddb78d3b81b5a65fe3529e42c9708273) )
	ROM_LOAD16_BYTE( "chasm.u16", 0x002001, 0x001000, CRC(2e192db0) SHA1(1a8ff983295ab52b5099c089b3142cdc56d28aee) )
	ROM_LOAD16_BYTE( "chasm.u3",  0x004000, 0x001000, CRC(9c71c600) SHA1(900526eaff7483fc478ebfb3f14796ff8fd1d01f) )
	ROM_LOAD16_BYTE( "chasm.u11", 0x004001, 0x001000, CRC(a4eb59a5) SHA1(a7bb3ca8f1f000f224def6342ca9d1eabcb210e6) )
	ROM_LOAD16_BYTE( "chasm.u7",  0x006000, 0x001000, CRC(8308dd6e) SHA1(82ad7c27e9a41af5280ecd975d3530ff2ed27ad4) )
	ROM_LOAD16_BYTE( "chasm.u15", 0x006001, 0x001000, CRC(9d3abf97) SHA1(476d684182d92d66263df82e1b5c4ff24b6814e8) )
	ROM_LOAD16_BYTE( "u2",        0x008000, 0x001000, CRC(4e076ae7) SHA1(a72f5425b256785b810ee5f23917b44f778cfcd3) )
	ROM_LOAD16_BYTE( "u10",       0x008001, 0x001000, CRC(cc9e19ca) SHA1(6c46ec265c2cc0683470ed1df978b96b577c5ca1) )
	ROM_LOAD16_BYTE( "chasm.u6",  0x00a000, 0x001000, CRC(a96525d2) SHA1(1c41bc3bf051cf1830182cbde6fba4e56db7e431) )
	ROM_LOAD16_BYTE( "chasm.u14", 0x00a001, 0x001000, CRC(8e426628) SHA1(2d70a7717b18cc892332b9d5d2de3ceba6c1481d) )
	ROM_LOAD16_BYTE( "u1",        0x00c000, 0x001000, CRC(88b71027) SHA1(49fa676d7838c643d642fbc70579ce29e76ba724) )
	ROM_LOAD16_BYTE( "chasm.u9",  0x00c001, 0x001000, CRC(d90c9773) SHA1(4033f0579f0782db2157f6cbece53b0d74e61d4f) )
	ROM_LOAD16_BYTE( "chasm.u5",  0x00e000, 0x001000, CRC(e4a58b7d) SHA1(0e5f948cd110804e6119fafb4e3fa5904dd1390f) )
	ROM_LOAD16_BYTE( "chasm.u13", 0x00e001, 0x001000, CRC(877e849c) SHA1(bdeb97fcb7488e7f0866dd651204c362d2ec9f4f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2732.bin", 0x0000, 0x1000, CRC(715adc4a) SHA1(426be4f3334ef7f2e8eb4d533e64276c30812aa3) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal12l6.u76", 0x0000, 0x0034, CRC(a30e02b7) SHA1(572f6d3f03e559f12e3bd5e087d7680ac69e9182) )
	ROM_LOAD( "pal12l6.u77", 0x0100, 0x0034, CRC(458b9cdb) SHA1(a3bff56d805f6dc494d294f079c3580430acf317) )
ROM_END

ROM_START( cchasm1 )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chasm.u4",  0x000000, 0x001000, CRC(19244f25) SHA1(79deaae82da8d1b16d05bbac43ba900c4b1d9f26) )
	ROM_LOAD16_BYTE( "chasm.u12", 0x000001, 0x001000, CRC(5d702c7d) SHA1(cbdceed45a1112594fbcbeb6976edc932b32d518) )
	ROM_LOAD16_BYTE( "chasm.u8",  0x002000, 0x001000, CRC(56a7ce8a) SHA1(14c790dcddb78d3b81b5a65fe3529e42c9708273) )
	ROM_LOAD16_BYTE( "chasm.u16", 0x002001, 0x001000, CRC(2e192db0) SHA1(1a8ff983295ab52b5099c089b3142cdc56d28aee) )
	ROM_LOAD16_BYTE( "chasm.u3",  0x004000, 0x001000, CRC(9c71c600) SHA1(900526eaff7483fc478ebfb3f14796ff8fd1d01f) )
	ROM_LOAD16_BYTE( "chasm.u11", 0x004001, 0x001000, CRC(a4eb59a5) SHA1(a7bb3ca8f1f000f224def6342ca9d1eabcb210e6) )
	ROM_LOAD16_BYTE( "chasm.u7",  0x006000, 0x001000, CRC(8308dd6e) SHA1(82ad7c27e9a41af5280ecd975d3530ff2ed27ad4) )
	ROM_LOAD16_BYTE( "chasm.u15", 0x006001, 0x001000, CRC(9d3abf97) SHA1(476d684182d92d66263df82e1b5c4ff24b6814e8) )
	ROM_LOAD16_BYTE( "chasm.u2",  0x008000, 0x001000, CRC(008b26ef) SHA1(6758d77bf48f466b8692bf7c678a597792d8cfdb) )
	ROM_LOAD16_BYTE( "chasm.u10", 0x008001, 0x001000, CRC(c2c532a3) SHA1(d29d40d42a2f69de0b1e2ee6a32633468a94fd85) )
	ROM_LOAD16_BYTE( "chasm.u6",  0x00a000, 0x001000, CRC(a96525d2) SHA1(1c41bc3bf051cf1830182cbde6fba4e56db7e431) )
	ROM_LOAD16_BYTE( "chasm.u14", 0x00a001, 0x001000, CRC(8e426628) SHA1(2d70a7717b18cc892332b9d5d2de3ceba6c1481d) )
	ROM_LOAD16_BYTE( "chasm.u1",  0x00c000, 0x001000, CRC(e02293f8) SHA1(136757b3c9e0ebfde6c13c57ac52f5fdbf5fd65b) )
	ROM_LOAD16_BYTE( "chasm.u9",  0x00c001, 0x001000, CRC(d90c9773) SHA1(4033f0579f0782db2157f6cbece53b0d74e61d4f) )
	ROM_LOAD16_BYTE( "chasm.u5",  0x00e000, 0x001000, CRC(e4a58b7d) SHA1(0e5f948cd110804e6119fafb4e3fa5904dd1390f) )
	ROM_LOAD16_BYTE( "chasm.u13", 0x00e001, 0x001000, CRC(877e849c) SHA1(bdeb97fcb7488e7f0866dd651204c362d2ec9f4f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2732.bin", 0x0000, 0x1000, CRC(715adc4a) SHA1(426be4f3334ef7f2e8eb4d533e64276c30812aa3) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, cchasm,  0,      cchasm, cchasm, cchasm_state, empty_init, ROT270, "Cinematronics / GCE", "Cosmic Chasm (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, cchasm1, cchasm, cchasm, cchasm, cchasm_state, empty_init, ROT270, "Cinematronics / GCE", "Cosmic Chasm (set 2)", MACHINE_SUPPORTS_SAVE )
