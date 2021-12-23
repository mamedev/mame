// license:BSD-3-Clause
// copyright-holders: smf
/***************************************************************************

      Z80 based, triple time zone clock

      https://github.com/tomstorey/Z80_clock

TODO:
      set rtc to utc time instead of local time at startup
      implement pwm brightness & remove clear_display code
      improve led driver selection

****************************************************************************/

#include "emu.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/snapquik.h"
#include "machine/bq4847.h"
#include "machine/clock.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "softlist.h"
#include "speaker.h"
#include "z80clock.lh"

class z80clock_state : public driver_device
{
public:
	z80clock_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_ne555(*this, "ne555"),
		m_ctc(*this, "ctc"),
		m_ctc_clock(*this, "ctc_clock"),
		m_sio(*this, "sio"),
		m_sio_clock(*this, "sio_clock"),
		m_sv2(*this, "sv2"),
		m_sv3(*this, "sv3"),
		m_rtc(*this, "rtc"),
		m_u12(*this, "u12"),
		m_u13(*this, "u13"),
		m_u14(*this, "u14"),
		m_u15(*this, "u15"),
		m_col(*this, "row%u.col%u", 0U, 0U),
		m_dp(*this, "row%u.dp%u", 0U, 0U),
		m_debug(*this, "debug%u", 0U),
		m_jp1(*this, "JP1"),
		m_jp2(*this, "JP2"),
		m_jp3(*this, "JP3"),
		m_jp4(*this, "JP4"),
		m_jp5(*this, "JP5"),
		m_jp6(*this, "JP6")
	{
	}

	void z80clock(machine_config& config)
	{
		static const z80_daisy_config z80clock_daisy_chain[] =
		{
			{ "ctc" },
			{ "sio" },
			{ nullptr }
		};

		Z80(config, m_maincpu, 6_MHz_XTAL);
		m_maincpu->set_daisy_config(z80clock_daisy_chain);
		m_maincpu->set_addrmap(AS_PROGRAM, &z80clock_state::mem_map);
		m_maincpu->set_addrmap(AS_IO, &z80clock_state::io_map);

		SPEAKER(config, "mono").front_center();
		SPEAKER_SOUND(config, m_speaker);
		m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

		CLOCK(config, m_ne555, 8589).signal_handler().set(FUNC(z80clock_state::ne555));
		m_ne555->set_clock_scale(0.0f);
		m_ne555->set_duty_cycle(2/3.0f);

		Z80CTC(config, m_ctc, 6_MHz_XTAL);
		m_ctc->intr_callback().set_inputline(m_maincpu, 0);
		m_ctc->zc_callback<0>().set(FUNC(z80clock_state::ctc_sound<0>));
		m_ctc->zc_callback<1>().set(FUNC(z80clock_state::ctc_sound<1>));
		m_ctc->zc_callback<2>().set(FUNC(z80clock_state::ctc_sound<2>));
		m_ctc->zc_callback<3>().set(FUNC(z80clock_state::ctc_sound<3>));

		BQ4845(config, m_rtc, 32.768_kHz_XTAL);
		m_rtc->int_handler().set(m_ctc, FUNC(z80ctc_device::trg3));
		m_rtc->rst_handler().set([this](int state) { if (!state && started() && m_jp2->read()) machine().schedule_soft_reset(); }); // HACK: inputs cannot be read during startup & can't hold machine reset low
		m_rtc->write_wdi(1);

		CLOCK(config, m_ctc_clock, 4.096_MHz_XTAL).signal_handler().set(FUNC(z80clock_state::prescaler));

		Z80SIO(config, m_sio, 6_MHz_XTAL);
		RS232_PORT(config, m_sv2, default_rs232_devices, nullptr);
		RS232_PORT(config, m_sv3, default_rs232_devices, nullptr);
		CLOCK(config, m_sio_clock, 3.6864_MHz_XTAL).signal_handler().set([this](int state) { m_sio->rxca_w(state); m_sio->txca_w(state); m_sio->rxtxcb_w(state); });

		m_sio->out_rtsa_callback().set(m_sv2, FUNC(rs232_port_device::write_rts));
		m_sv2->rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
		m_sio->out_txda_callback().set(m_sv2, FUNC(rs232_port_device::write_txd));
		m_sv2->cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
		m_sio->out_dtra_callback().set(m_sio, FUNC(z80sio_device::dcda_w));

		m_sio->out_rtsb_callback().set(m_sv3, FUNC(rs232_port_device::write_rts));
		m_sv3->rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
		m_sio->out_txdb_callback().set(m_sv3, FUNC(rs232_port_device::write_txd));
		m_sv3->cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
		m_sio->out_dtrb_callback().set(m_sio, FUNC(z80sio_device::dcdb_w));

		config.set_default_layout(layout_z80clock);

		GENERIC_CARTSLOT(config, m_u12, generic_plain_slot, "u12").set_must_be_loaded(true);
		GENERIC_CARTSLOT(config, m_u14, generic_plain_slot, "u14");
		GENERIC_CARTSLOT(config, m_u15, generic_plain_slot, "u15");

		auto& quickload(QUICKLOAD(config, "u13", "rom"));
		quickload.set_load_callback(FUNC(z80clock_state::quickload_u13));
		quickload.set_interface("u13");

		SOFTWARE_LIST(config, "rom_list").set_original("z80clock");
	}

protected:
	virtual void machine_start() override
	{
		m_col.resolve();
		m_dp.resolve();
		m_debug.resolve();

		m_shift.resize(17);

		save_item(NAME(m_shift));

		m_clear_display_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z80clock_state::clear_display), this));
		m_clear_display_timer->adjust(attotime::zero, 0, attotime::from_hz(240));
		m_clear_display_count.resize(3);

		save_item(NAME(m_clear_display_count));

		save_item(NAME(m_ctc_sound_state));
		save_item(NAME(m_ctc_sound_flipflop));
	}

	virtual void machine_reset() override
	{
		/// HACK: start the sio clock on first write for speed
		m_sio_clock->set_clock_scale(0.0f);
	}

private:
	required_device<z80_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<clock_device> m_ne555;
	required_device<z80ctc_device> m_ctc;
	required_device<clock_device> m_ctc_clock;
	required_device<z80sio_device> m_sio;
	required_device<clock_device> m_sio_clock;
	required_device<rs232_port_device> m_sv2;
	required_device<rs232_port_device> m_sv3;
	required_device<bq4845_device> m_rtc;
	required_device<generic_slot_device> m_u12;
	required_shared_ptr<uint8_t> m_u13;
	required_device<generic_slot_device> m_u14;
	required_device<generic_slot_device> m_u15;
	output_finder<3, 8> m_col;
	output_finder<3, 8> m_dp;
	output_finder<8> m_debug;
	required_ioport m_jp1;
	required_ioport m_jp2;
	required_ioport m_jp3;
	required_ioport m_jp4;
	required_ioport m_jp5;
	required_ioport m_jp6;
	std::vector<uint8_t> m_shift;
	emu_timer* m_clear_display_timer;
	std::vector<uint8_t> m_clear_display_count;
	int m_ctc_sound_state;
	bool m_ctc_sound_flipflop;

	void mem_map(address_map& map)
	{
		map.unmap_value_high();
		map(0x0000, 0x5fff).rom().r(m_u12, FUNC(generic_slot_device::read_rom));
		map(0x6000, 0x9fff).ram().share("u13");
		map(0xc000, 0xdfff).rom().r(m_u14, FUNC(generic_slot_device::read_rom));
		map(0xe000, 0xffff).rom().r(m_u15, FUNC(generic_slot_device::read_rom));
	}

	void io_map(address_map& map)
	{
		map.unmap_value_high();
		map.global_mask(0xff);
		map(0x00, 0x03).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
		map(0x04, 0x04).w(FUNC(z80clock_state::wdt_poke_w));
		map(0x08, 0x08).portr("BTN");
		map(0x09, 0x09).portr("TZ_SW1");
		map(0x0a, 0x0a).portr("TZ_SW2");
		map(0x0b, 0x0b).portr("TZ_SW3");
		map(0x0c, 0x0c).w(FUNC(z80clock_state::outputs_w));
		map(0x10, 0x1f).rw(m_rtc, FUNC(bq4845_device::read), FUNC(bq4845_device::write));
		map(0x20, 0x20).w(FUNC(z80clock_state::disp_data_w));
		map(0x21, 0x21).w(FUNC(z80clock_state::disp_ctrl_w));
		map(0xde, 0xde).w(FUNC(z80clock_state::debug_port_w));
		map(0xfc, 0xff).r(m_sio, FUNC(z80sio_device::cd_ba_r));
		map(0xfc, 0xff).w(FUNC(z80clock_state::sio_cd_ba_w));
	}

	void wdt_poke_w(uint8_t data)
	{
		m_rtc->write_wdi(0);
		m_rtc->write_wdi(1);
	}

	void outputs_w(uint8_t data)
	{
		m_ne555->set_clock_scale(BIT(data, 7) ? 1.0f : 0.0f);
	}

	void disp_data_w(uint8_t data)
	{
		std::copy_backward(m_shift.begin(), m_shift.end() - 1, m_shift.end());

		m_shift[0] = data;
	}

	void disp_ctrl_w(uint8_t data)
	{
		int row = (data & 3) - 1;
		if (row >= 0)
		{
			const char* led_driver = m_u15->get_feature("led_driver");
			uint16_t invert = (led_driver && !strcmp(led_driver, "74hc595")) ? 0xffff : 0x0000;

			for (int i = 0; i < 8; i++)
			{
				m_col[row][i] = bitswap<16>((m_shift[(i * 2) + 1] << 8) | m_shift[(i * 2) + 2], 3, 6, 10, 14, 13, 9, 4, 12, 11, 15, 0, 1, 2, 5, 7, 8) ^ invert;
				m_dp[row][i] = BIT(m_shift[0] ^ invert, i);
			}

			m_clear_display_count[row] = 2;
		}
	}

	TIMER_CALLBACK_MEMBER(clear_display)
	{
		for (int row = 0; row < 3; row++)
			if (m_clear_display_count[row] != 0)
			{
				m_clear_display_count[row]--;
				if (m_clear_display_count[row] == 0)
				{
					for (int i = 0; i < 8; i++)
					{
						m_col[row][i] = 0;
						m_dp[row][i] = 0;
					}
				}
			}
	}

	void ne555(int state)
	{
		if (m_jp3->read())
			m_speaker->level_w(state);
	}

	template<int channel>
	void ctc_sound(int state)
	{
		if (!BIT(m_jp5->read(), channel))
		{
			if (!m_jp3->read() && m_jp6->read())
				m_speaker->level_w(state);

			if (state && !m_ctc_sound_state)
			{
				m_ctc_sound_flipflop = !m_ctc_sound_flipflop;

				if (!m_jp3->read() && !m_jp6->read())
					m_speaker->level_w(m_ctc_sound_flipflop);
			}

			m_ctc_sound_state = state;
		}
	}

	void debug_port_w(uint8_t data)
	{
		for (int i = 0; i < 8; i++)
			m_debug[i] = BIT(data, i);
	}

	void sio_cd_ba_w(offs_t offset, uint8_t data)
	{
		/// HACK: start the sio clock on first write for speed
		m_sio_clock->set_clock_scale(1.0f);
		m_sio->cd_ba_w(offset, data);
	}

	void prescaler(int state)
	{
		auto ch = m_jp4->read();
		m_ctc->trg0(state | BIT(ch, 0));
		m_ctc->trg1(state | BIT(ch, 1));
		m_ctc->trg2(state | BIT(ch, 2));
		m_ctc_clock->set_clock_scale(1.0 / m_jp1->read());
	}

	QUICKLOAD_LOAD_MEMBER(quickload_u13)
	{
		auto length = image.length();
		if (length > m_u13.bytes())
			return image_init_result::FAIL;

		if (image.fread(m_u13, length) != length)
			return image_init_result::FAIL;

		m_maincpu->set_pc(0x6000);
		return image_init_result::PASS;
	}
};

INPUT_PORTS_START(z80clock)
	PORT_START("JP1")
	PORT_CONFNAME(0x700, 0x0100, "JP1 Timer Prescaler (4.096mhz/256)")
	PORT_CONFSETTING(   0x0400, "5-6 /4 (4khz)")
	PORT_CONFSETTING(   0x0200, "3-4 /2 (8khz)")
	PORT_CONFSETTING(   0x0100, "1-2 /1 (16khz)")

	PORT_START("JP2")
	PORT_CONFNAME(0x1, 0x01, "JP2 Watchdog Reset Enable")
	PORT_CONFSETTING(0x01, DEF_STR(On))
	PORT_CONFSETTING(0x00, DEF_STR(Off))

	PORT_START("JP3")
	PORT_CONFNAME(0x1, 0x00, "JP3 Sound Source")
	PORT_CONFSETTING(0x01, "1-2 555")
	PORT_CONFSETTING(0x00, "2-3 CTC")

	PORT_START("JP4")
	PORT_CONFNAME(0x1, 0x00, "JP4 1-2 Timer Prescaler CTC CH0")
	PORT_CONFSETTING(0x01, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFNAME(0x2, 0x00, "JP4 3-4 Timer Prescaler CTC CH1")
	PORT_CONFSETTING(0x02, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFNAME(0x4, 0x00, "JP4 5-6 Timer Prescaler CTC CH2")
	PORT_CONFSETTING(0x04, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))

	PORT_START("JP5")
	PORT_CONFNAME(0x1, 0x00, "JP5 1-2 Sound CTC CH0")
	PORT_CONFSETTING(0x01, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFNAME(0x2, 0x02, "JP5 3-4 Sound CTC CH1")
	PORT_CONFSETTING(0x02, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFNAME(0x4, 0x04, "JP5 5-6 Sound CTC CH2")
	PORT_CONFSETTING(0x04, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFNAME(0x8, 0x08, "JP5 7-8 Sound CTC CH3")
	PORT_CONFSETTING(0x08, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))

	PORT_START("JP6")
	PORT_CONFNAME(0x1, 0x00, "JP6 1-2 Sound CTC flip flop")
	PORT_CONFSETTING(0x01, "1-2 Off")
	PORT_CONFSETTING(0x00, "2-3 On")

	PORT_START("BTN")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SW1 UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SW2 DN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SW3 ENT") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SW4 ESC") PORT_CODE(KEYCODE_ESC)
	PORT_DIPNAME(0x30, 0x20, "Dim 21:00 to 05:59") PORT_DIPLOCATION("SW5:1,2")
	PORT_DIPSETTING(0x30, "Disable")
	PORT_DIPSETTING(0x20, "Row 1 timezone")
	PORT_DIPSETTING(0x10, "Row 2 timezone")
	PORT_DIPSETTING(0x00, "Row 3 timezone")
	PORT_DIPNAME(0x40, 0x00, "Dim Duty Cycle") PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(0x40, "50%")
	PORT_DIPSETTING(0x00, "25%")
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unused)) PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	#define DIP_TIMEZONE(row, loc, def) \
		PORT_START("TZ_SW" #row) \
		PORT_DIPNAME(0x80, ~(def) & 0x80, "Row " #row " type") PORT_DIPLOCATION("SW" #loc ":8") \
		PORT_DIPSETTING(0x80, "Time") \
		PORT_DIPSETTING(0x00, "Date") \
		\
		PORT_DIPNAME(0x70, ~(def) & 0x70, "Row " #row " timezone high") PORT_DIPLOCATION("SW" #loc ":5,6,7") \
		PORT_DIPSETTING(0x70, "0_") \
		PORT_DIPSETTING(0x60, "1_") \
		PORT_DIPSETTING(0x50, "2_") \
		PORT_DIPSETTING(0x40, "3_") \
		PORT_DIPSETTING(0x30, "4_") \
		PORT_DIPSETTING(0x20, "5_") \
		PORT_DIPSETTING(0x10, "6_") \
		PORT_DIPSETTING(0x00, "7_") \
		\
		PORT_DIPNAME(0x0f, ~(def) & 0xf, "Row " #row " timezone low") PORT_DIPLOCATION("SW" #loc ":1,2,3,4") \
		PORT_DIPSETTING(0x0f, "_0") \
		PORT_DIPSETTING(0x0e, "_1") \
		PORT_DIPSETTING(0x0d, "_2") \
		PORT_DIPSETTING(0x0c, "_3") \
		PORT_DIPSETTING(0x0b, "_4") \
		PORT_DIPSETTING(0x0a, "_5") \
		PORT_DIPSETTING(0x09, "_6") \
		PORT_DIPSETTING(0x08, "_7") \
		PORT_DIPSETTING(0x07, "_8") \
		PORT_DIPSETTING(0x06, "_9") \
		PORT_DIPSETTING(0x05, "_a") \
		PORT_DIPSETTING(0x04, "_b") \
		PORT_DIPSETTING(0x03, "_c") \
		PORT_DIPSETTING(0x02, "_d") \
		PORT_DIPSETTING(0x01, "_e") \
		PORT_DIPSETTING(0x00, "_f")

	DIP_TIMEZONE(1, 6, 0x02)
	DIP_TIMEZONE(2, 7, 0x82)
	DIP_TIMEZONE(3, 8, 0x03)
INPUT_PORTS_END

ROM_START( z80clock )
ROM_END

COMP(2020, z80clock, 0, 0, z80clock, z80clock, z80clock_state, empty_init, "Tom Storey", "Z80 based, triple time zone clock", MACHINE_SUPPORTS_SAVE)
