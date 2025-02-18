// license:BSD-3-Clause
// copyright-holders: Jaume López

//#define VERBOSE 1

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/pic8259.h"
#include "video/i8275.h"
#include "machine/ram.h"
#include "screen.h"
#include "speaker.h"
#include "sound/spkrdev.h"
#include "machine/clock.h"
#include "system23_kbd.h"
#include "logmacro.h"

#include "ibmsystem23.lh"

namespace
{
	class system23_state: public driver_device
	{
		public:
			system23_state(const machine_config &mconfig, device_type type, const char *tag)
				: driver_device(mconfig, type, tag),
				m_maincpu(*this, "maincpu"),
				m_ros(*this, "ros"),
				m_ppi_crtc(*this, "ppi_kbd"),
				m_ppi_diag(*this, "ppi_diag"),
				m_ppi_settings(*this, "ppi_settings"),
				m_dmac(*this,"dma"),
				m_crtc(*this, "crtc"),
				m_pit(*this, "pit"),
				m_pic(*this, "pic"),
				m_usart(*this, "usart"),
				m_chargen(*this, "chargen"),
				m_ram(*this, RAM_TAG),
				m_ram_bank_r(*this, "ram_bank_read"),
				m_ram_bank_w(*this, "ram_bank_write"),
				//m_ram_bank_dma(*this, "ram_bank_dma"),
				m_screen(*this, "screen"),
				m_speaker(*this,"beeper"),
				m_keyboard(*this, "kbd"),
				m_pit_clock(*this, "pit_clk"),
				m_language(*this,"lang"),
				m_cedip(*this,"ce"),
				m_j1(*this,"j1"),
				m_diag_digits(*this, "digit%u", 0U)
			{

			}

			void system23(machine_config &config);

		protected:
			virtual void machine_start() override ATTR_COLD;
			virtual void machine_reset() override ATTR_COLD;

		private:
			required_device<i8085a_cpu_device> m_maincpu;
			required_memory_bank m_ros;
			required_device<i8255_device> m_ppi_crtc;
			required_device<i8255_device> m_ppi_diag;
			required_device<i8255_device> m_ppi_settings;
			required_device<i8257_device> m_dmac;
			required_device<i8275_device> m_crtc;
			required_device<pit8253_device> m_pit;
			required_device<pic8259_device> m_pic;
			required_device<i8251_device> m_usart;
			required_region_ptr<uint8_t> m_chargen;
			required_device<ram_device> m_ram;
			required_memory_bank m_ram_bank_r;
			required_memory_bank m_ram_bank_w;
			//required_memory_bank m_ram_bank_dma;
			required_device<screen_device> m_screen;
			required_device<speaker_sound_device> m_speaker;
			required_device<system23_kbd_device> m_keyboard;
			required_device<clock_device> m_pit_clock;
			required_ioport m_language;
			required_ioport m_cedip;
			required_ioport m_j1;
			output_finder<2> m_diag_digits;

			uint8_t m_bus_test_register = 0;
			uint8_t hex_seven_segment[16] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

			uint8_t lpen_ct = 0;

			uint8_t m_hrtc = 0;
			uint8_t m_vrtc = 0;
			uint8_t m_pixel = 0;

			uint8_t m_ros_page;
			uint8_t m_ram_r_page;
			uint8_t m_ram_w_page;
			uint8_t m_dma_page;

			uint8_t m_port_4e;

			//uint8_t ram_r(offs_t offset);
			//void ram_w(offs_t offset, uint8_t data);
			uint8_t *m_ram_ptr;
			uint32_t m_ram_size;

			uint32_t m_rst75_enabled;

			int m_sod;
			int m_ros_w;

			void diag_digits_w(uint8_t data);

			void port_4e_w(uint8_t data);
			uint8_t port_4e_r();

			void cpu_test_register_w(uint8_t data);
			uint8_t cpu_test_register_r();

			uint8_t memory_settings_r();

			int sid_r();

			uint8_t dmac_mem_r(offs_t offset);
			void dmac_mem_w(offs_t offset, uint8_t data);

			void dmac_hrq_w(int state);

			void crtc_dack_w(offs_t offset, uint8_t data);
			I8275_DRAW_CHARACTER_MEMBER(display_pixels);
			uint8_t crtc_test_vars_r();
			void hrtc_r(uint8_t data);
			void vrtc_r(uint8_t data);

			void update_speaker(uint32_t state);
			void rst75(uint32_t state);
			void rst55(uint32_t state);

			uint8_t ros_page_r();
			void ros_page_w(uint8_t data);
			uint8_t ram_read_page_r();
			void ram_read_page_w(uint8_t data);
			uint8_t ram_write_page_r();
			void ram_write_page_w(uint8_t data);
			uint8_t dma_page_r();
			void dma_page_w(uint8_t data);

			void reset_keyboard(uint8_t data);
			uint8_t read_keyboard();
			void data_strobe_w(int state);

			void usart_ck_w(int state);

			void trap();
			void sod_trap(int state);
			void ros_w_trap(uint8_t data);

			void pit_clk2(int state);
			void rst75_enable(uint8_t data);

			void system23_io(address_map &map) ATTR_COLD;
			void system23_mem(address_map &map) ATTR_COLD;
	};

	//This routine deals with the diagnostics "miniprobe" that signals the test where the computer remains stuck

	void system23_state::diag_digits_w(uint8_t data)
	{
		m_diag_digits[0] = system23_state::hex_seven_segment[data & 0x0f];
		m_diag_digits[1] = system23_state::hex_seven_segment[(data >> 4) & 0x0f];
	}

	//Those routines deal with a register used only to verify the data bus

	void system23_state::cpu_test_register_w(uint8_t data)
	{
		m_bus_test_register = data;
	}

	uint8_t system23_state::cpu_test_register_r()
	{
		return m_bus_test_register;
	}

	//This routine deals with an unknown register whose purpose is still to be discovered. The memory test fails if it does not return 0

	uint8_t system23_state::memory_settings_r()
	{
		return 0;//return 0;	//Patch to make the memory test work while port 2e is being studied
	}

	//This routine deals with the SID signal being read. The CPU test fails if the line is not asserted high

	int system23_state::sid_r()
	{
		return ASSERT_LINE; // Actually, SID is tied to VCC at the motherboard
	}

	//Those routines deal with the DMA controller accesses to memory and I/O devices

	uint8_t system23_state::dmac_mem_r(offs_t offset)
	{
		return m_maincpu->space(AS_PROGRAM).read_byte(offset);
	}

	void system23_state::dmac_mem_w(offs_t offset, uint8_t data)
	{
		m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
	}

	void system23_state::dmac_hrq_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
		m_dmac->hlda_w(state);
	}

	//This routine deals with the responsability of writing a character to the screen

	I8275_DRAW_CHARACTER_MEMBER( system23_state::display_pixels )
	{
		//const rgb_t *palette = m_palette->palette()->entry_list_raw();
		uint8_t gfx = 0;
		//uint8_t chr_bank = m_bus_test_register & 0x03;

		using namespace i8275_attributes;

		if (!BIT(attrcode, VSP))
			gfx = m_chargen[(linecount & 15) | (charcode << 4) /*| (chr_bank << 10)*/];

		if (BIT(attrcode, LTEN))
		 	gfx = 0xff;

		if (BIT(attrcode, RVV))
			gfx ^= 0xff;

		//if (BIT(attrcode, GPA0) || BIT(attrcode, GPA1)) printf("GPA0: %u GPA1: %u\n", BIT(attrcode, GPA0), BIT(attrcode, GPA1));
		if (BIT(attrcode, GPA0) && BIT(attrcode, GPA1) && !lpen_ct)
		{
			//LOG("Sytem/23: Light pen asserted\n");
			lpen_ct = 1;
		}
		else if(lpen_ct == 1)
		{
			m_crtc->lpen_w(ASSERT_LINE);
			lpen_ct = 0;
		}
		else
		{
			m_crtc->lpen_w(CLEAR_LINE);
		}

		//Pixel reference saved for test 06 usage
		m_pixel = BIT(gfx, 1);
		//LOG("x=%x y=%x attrcode=%x\n",x,y,attrcode);

		// Highlight not used
		bitmap.pix(y, x++) = (BIT(gfx, 7)? rgb_t::green() : rgb_t::black());
		bitmap.pix(y, x++) = (BIT(gfx, 6)? rgb_t::green() : rgb_t::black());
		bitmap.pix(y, x++) = (BIT(gfx, 5)? rgb_t::green() : rgb_t::black());
		bitmap.pix(y, x++) = (BIT(gfx, 4)? rgb_t::green() : rgb_t::black());
		bitmap.pix(y, x++) = (BIT(gfx, 3)? rgb_t::green() : rgb_t::black());
		bitmap.pix(y, x++) = (BIT(gfx, 2)? rgb_t::green() : rgb_t::black());
		bitmap.pix(y, x++) = (BIT(gfx, 1)? rgb_t::green() : rgb_t::black());
		bitmap.pix(y, x++) = (BIT(gfx, 0)? rgb_t::green() : rgb_t::black());
	}

	//This routine compiles the retraces and pixel data from the CRTC into a single byte
	uint8_t system23_state::crtc_test_vars_r()
	{
		return m_hrtc << 4 | m_vrtc << 3 | m_pixel << 2;
	}

	void system23_state::hrtc_r(uint8_t data)
	{
		m_hrtc = data;
	}

	void system23_state::vrtc_r(uint8_t data)
	{
		m_vrtc = data;
	}

	uint8_t system23_state::ros_page_r()
	{
		return m_ros_page;
	}

	void system23_state::ros_page_w(uint8_t data)
	{
		LOG("ROS page: %x\n", data);
		m_ros_page = data | 0xf0;
		m_ros->set_entry(data & 0xf);
	}


	uint8_t system23_state::ram_read_page_r()
	{
		return m_ram_r_page;
	}

	void system23_state::ram_read_page_w(uint8_t data)
	{
		LOG("RAM read page: %x\n",data);
		m_ram_r_page = data | 0xf0;
		m_ram_bank_r->set_entry(data & 0xf);
	}

	uint8_t system23_state::ram_write_page_r()
	{
		return m_ram_w_page;
	}

	void system23_state::ram_write_page_w(uint8_t data)
	{
		LOG("RAM write page: %x\n", data);
		m_ram_w_page = data | 0xf0;
		m_ram_bank_w->set_entry(data & 0xf);
	}

	uint8_t system23_state::dma_page_r()
	{
		return m_dma_page;
	}

	void system23_state::dma_page_w(uint8_t data)
	{
		LOG("DMA page: %x\n",data);
		m_dma_page = data | 0xf0;
		//m_ram_bank_dma->set_entry(data & 0xf);
	}

	void system23_state::port_4e_w(uint8_t data)
	{
		m_port_4e = data;
	}

	uint8_t system23_state::port_4e_r()
	{
		return m_port_4e;
	}

	void system23_state::update_speaker(uint32_t state)
	{
		uint32_t speaker_enable = (m_port_4e && BIT(m_port_4e, 1)) ? CLEAR_LINE : ASSERT_LINE;
		uint32_t speaker_state = (speaker_enable ? CLEAR_LINE : ASSERT_LINE) & state;
		m_speaker->level_w(speaker_state);
	}

	void system23_state::rst75(uint32_t state)
	{
		LOG("RST7.5\n");
		m_maincpu->set_input_line(I8085_RST75_LINE, state);
	}

	void system23_state::rst55(uint32_t state)
	{
		LOG("RST5.5\n");
		m_maincpu->set_input_line(I8085_RST55_LINE, state & m_j1->read());
	}

	void system23_state::reset_keyboard(uint8_t data)
	{
		if(BIT(data,7))
		{
			m_keyboard->reset_w(CLEAR_LINE);
		}
		else
		{
			m_keyboard->reset_w(ASSERT_LINE);
		}
		m_pic->ir0_w(BIT(data,3));
		m_keyboard->t0_w(BIT(data,5));
	}

	uint8_t system23_state::read_keyboard()
	{
		return m_keyboard->read_keyboard() & 0x7f;
	}

	void system23_state::usart_ck_w(int state)
	{
		m_usart->write_rxc(state);
		m_usart->write_txc(state);
	}

	void system23_state::data_strobe_w(int state)
	{
		//LOG("KBD Data strobe: %d\n", state);
		m_ppi_diag->pc4_w(state);
	}

	void system23_state::trap()
	{
		int error = !(m_ros_w);
		int state = m_sod & error;//SOD gates a 4-input NAND output (RESET OUT, Memory Parity Error, ROS write Error)
		LOG("TRAP: %d", state);
		m_maincpu->set_input_line(I8085_RST75_LINE, state);
	}

	void system23_state::sod_trap(int state)
	{
		m_sod = state;
		trap();
	}

	void system23_state::ros_w_trap(uint8_t data)
	{
		m_ros_w = CLEAR_LINE;
		trap();
	}

	void system23_state::pit_clk2(int state)
	{
    	m_pit->write_clk2(!(state & m_rst75_enabled));
	}

	void system23_state::rst75_enable(uint8_t data)
	{
		m_rst75_enabled = BIT(data,3);
	}

	//This routine describes the computer's I/O map

	void system23_state::system23_io(address_map &map)
	{
		map.unmap_value_high();
		map(0x00, 0x0f).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
		map(0x22, 0x22).rw(FUNC(system23_state::ram_read_page_r),FUNC(system23_state::ram_read_page_w));
		map(0x21, 0x21).rw(FUNC(system23_state::ram_write_page_r),FUNC(system23_state::ram_write_page_w));
		map(0x20, 0x20).rw(FUNC(system23_state::dma_page_r),FUNC(system23_state::dma_page_w));
		map(0x23, 0x23).rw(FUNC(system23_state::ros_page_r),FUNC(system23_state::ros_page_w));
		map(0x24, 0x27).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
		map(0x28, 0x2b).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
		map(0x2c, 0x2f).rw(m_ppi_settings, FUNC(i8255_device::read), FUNC(i8255_device::write));
		map(0x40, 0x43).rw(m_ppi_diag, FUNC(i8255_device::read), FUNC(i8255_device::write));
		map(0x44, 0x45).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
		map(0x48, 0x49).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
		map(0x4c, 0x4f).rw(m_ppi_crtc, FUNC(i8255_device::read), FUNC(i8255_device::write));
	}

	//This routine describes the computer's memory map

	void system23_state::system23_mem(address_map &map)
	{
		map.unmap_value_high();
		map(0x0000, 0x3fff).rom().w(FUNC(system23_state::ros_w_trap));
		map(0x4000, 0x7fff).bankr(m_ros).w(FUNC(system23_state::ros_w_trap));
		map(0x8000, 0xbfff).ram();
		map(0xc000, 0xffff).bankr(m_ram_bank_r);
		map(0xc000, 0xffff).bankw(m_ram_bank_w);
	}

	//This is the constructor of the class

	void system23_state::system23(machine_config &config)
	{
		I8085A(config, m_maincpu, (18'432'000 / 3));
		m_maincpu->set_addrmap(AS_PROGRAM, &system23_state::system23_mem);
		m_maincpu->set_addrmap(AS_IO, &system23_state::system23_io);
		m_maincpu->in_sid_func().set(FUNC(system23_state::sid_r));
		m_maincpu->out_sod_func().set(FUNC(system23_state::sod_trap));
		m_maincpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

		I8255(config, m_ppi_crtc);
		m_ppi_crtc->in_pa_callback().set(FUNC(system23_state::cpu_test_register_r));
		m_ppi_crtc->out_pa_callback().set(FUNC(system23_state::cpu_test_register_w));
		m_ppi_crtc->in_pb_callback().set(FUNC(system23_state::crtc_test_vars_r));
		m_ppi_crtc->in_pc_callback().set(FUNC(system23_state::port_4e_r));
		m_ppi_crtc->out_pc_callback().set(FUNC(system23_state::port_4e_w));

		I8255(config, m_ppi_diag);
		m_ppi_diag->in_pa_callback().set(FUNC(system23_state::read_keyboard));
		m_ppi_diag->out_pb_callback().set(FUNC(system23_state::diag_digits_w));
		m_ppi_diag->out_pc_callback().set(FUNC(system23_state::reset_keyboard));

		I8255(config, m_ppi_settings);
		m_ppi_settings->in_pc_callback().set(FUNC(system23_state::memory_settings_r));
		m_ppi_settings->in_pa_callback().set_ioport(m_language);
		m_ppi_settings->in_pb_callback().set_ioport(m_cedip);
		m_ppi_settings->out_pc_callback().set(FUNC(system23_state::rst75_enable));

		I8257(config, m_dmac, (18'432'000 / 6)); //frequency needs to be adjusted
		m_dmac->out_memw_cb().set(FUNC(system23_state::dmac_mem_w));
		m_dmac->in_memr_cb().set(FUNC(system23_state::dmac_mem_r));
		m_dmac->out_iow_cb<2>().set(m_crtc, FUNC(i8275_device::dack_w));
		m_dmac->out_hrq_cb().set(FUNC(system23_state::dmac_hrq_w));

		SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
		m_screen->set_raw(18'432'000, 800, 0, 640, 324, 0, 300);
		m_screen->set_screen_update(m_crtc, FUNC(i8275_device::screen_update));

		I8275(config, m_crtc, (18'432'000 / 8));
		m_crtc->set_character_width(8);
		m_crtc->set_screen(m_screen);
		m_crtc->set_display_callback(FUNC(system23_state::display_pixels));
		m_crtc->drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq2_w));
		m_crtc->irq_wr_callback().set(FUNC(system23_state::rst55));
		m_crtc->hrtc_wr_callback().set(FUNC(system23_state::hrtc_r));
		m_crtc->vrtc_wr_callback().set(FUNC(system23_state::vrtc_r));

		I8251(config, m_usart, 0);
		m_usart->rxrdy_handler().set(m_pic, FUNC(pic8259_device::ir1_w));
		m_usart->txrdy_handler().set(m_pic, FUNC(pic8259_device::ir2_w));

		SPEAKER(config, "mono").front_center();
		SPEAKER_SOUND(config, m_speaker);
		m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

		PIT8253(config, m_pit, 0);
		m_pit->set_clk<0>(18'432'000 / 12);
		m_pit->set_clk<1>(18'432'000 / 12);
		//m_pit->set_clk<2>(18'432'000 / 12);
		m_pit->out_handler<0>().set(FUNC(system23_state::usart_ck_w));
		m_pit->out_handler<1>().set(FUNC(system23_state::update_speaker));
		m_pit->out_handler<2>().set(FUNC(system23_state::rst75));

		PIC8259(config, m_pic);
		m_pic->out_int_callback().set_inputline(m_maincpu, I8085_INTR_LINE);

		RAM(config, m_ram).set_default_size("128k");

		SYSTEM23_KEYBOARD(config, m_keyboard, 0);
		m_keyboard->scancode_export().set(FUNC(system23_state::data_strobe_w));

		CLOCK(config, m_pit_clock, 18'432'000 / 12);
		m_pit_clock->signal_handler().set(FUNC(system23_state::pit_clk2));

		config.set_perfect_quantum(m_maincpu);
		config.set_default_layout(layout_ibmsystem23);

	}

	void system23_state::machine_start()
	{
		m_diag_digits.resolve();
		m_ros->configure_entries(0, 16, memregion("maincpu")->base() + 0x4000, 0x4000);
		m_ram_ptr = m_ram->pointer();
		m_ram_size = m_ram->size();
		m_ram_bank_r->configure_entries(0, 16, m_ram->pointer() + 0x4000, 0x4000);
		m_ram_bank_w->configure_entries(0, 16, m_ram->pointer() + 0x4000, 0x4000);
		//m_ram_bank_dma->configure_entries(0, 16, m_ram->pointer() + 0x4000, 0x4000);
		m_sod = CLEAR_LINE;
		m_ros_w = ASSERT_LINE;
	}

	void system23_state::machine_reset()
	{

	}

	static INPUT_PORTS_START(system23)
		PORT_START("j1")
			PORT_DIPNAME(0x01, 0x00, "J1")
			PORT_DIPSETTING(    0x00, DEF_STR( Off ))
			PORT_DIPSETTING(    0x01, DEF_STR( On ))

		PORT_START("ce")
			PORT_DIPNAME( 0x01, 0x00, "RAM Base installed")
			PORT_DIPSETTING(    0x01, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x02, 0x00, "RAM Expansion installed")
			PORT_DIPSETTING(    0x02, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x04, 0x00, "RAM Base 32/64KB")
			PORT_DIPSETTING(    0x04, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x08, 0x00, "RAM Expansion 32/64KB")
			PORT_DIPSETTING(    0x08, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x10, 0x00, "A1 - CE Loop on test")
			PORT_DIPSETTING(    0x10, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x20, 0x00, "A2 - CE Stop on error")
			PORT_DIPSETTING(    0x20, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x40, 0x00, "A3 - Machine update card installed")
			PORT_DIPSETTING(    0x40, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x80, 0x00, "A4")
			PORT_DIPSETTING(    0x80, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))

		PORT_START("lang")
			PORT_DIPNAME( 0x01, 0x00, "B1")
			PORT_DIPSETTING(    0x01, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x02, 0x00, "B2")
			PORT_DIPSETTING(    0x02, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x04, 0x00, "B3")
			PORT_DIPSETTING(    0x04, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x08, 0x00, "B4")
			PORT_DIPSETTING(    0x08, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x10, 0x00, "B5")
			PORT_DIPSETTING(    0x10, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x20, 0x00, "B6")
			PORT_DIPSETTING(    0x20, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x40, 0x00, "B7")
			PORT_DIPSETTING(    0x40, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x80, 0x00, "B8")
			PORT_DIPSETTING(    0x80, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
	INPUT_PORTS_END


	ROM_START( system23 )
		ROM_SYSTEM_BIOS(0, "ros_104", "ROS 1.04 - 1982?")
		ROM_SYSTEM_BIOS(1, "ros_101", "ROS 1.01 - 1980")

		ROM_REGION(0x24000, "maincpu", ROMREGION_ERASEFF)
		//ROS 1.04 (1982?)
		ROMX_LOAD("02_61c9866a_4481186.bin", 0x00000, 0x2000, CRC(61c9866a) SHA1(43f2bed5cc2374c7fde4632948329062e57e994b), ROM_BIOS(0))
		ROMX_LOAD("09_07843020_8493747.bin", 0x02000, 0x2000, CRC(07843020) SHA1(828ca0199af1246f6caf58bcb785f791c3a7e34e), ROM_BIOS(0))
		ROMX_LOAD("0a_b9569153_8519402.bin", 0x04000, 0x2000, CRC(b9569153) SHA1(92ccf91766557bc565ad36fc131396aff28b8999), ROM_BIOS(0))
		ROMX_LOAD("0b_4f631183_8519404.bin", 0x06000, 0x2000, CRC(4f631183) SHA1(5f7b011129616bae46c70133309a70c29cc0f127), ROM_BIOS(0))
		ROMX_LOAD("0c_48646293_8519403.bin", 0x08000, 0x2000, CRC(48646293) SHA1(3d7eaf2c143499757681fbedbc3716829ef9bd25), ROM_BIOS(0))
		ROMX_LOAD("0d_bea5a812_8519405.bin", 0x0a000, 0x2000, CRC(bea5a812) SHA1(5da3a9231c5d456fa7a26ab36b9d5380e096af59), ROM_BIOS(0))
		ROMX_FILL(0xc000, 0x2000, 0xff, ROM_BIOS(0))//ROM E is empty
		ROMX_FILL(0xe000, 0x2000, 0xff, ROM_BIOS(0))//ROM F is empty
		ROMX_LOAD("10_41e6c232_8519411.bin", 0x10000, 0x2000, CRC(41e6c232) SHA1(6711000a0c6836a411997de15274b990dd2c0ed0), ROM_BIOS(0))
		ROMX_LOAD("11_b17f5c6e_8519407.bin", 0x12000, 0x2000, CRC(b17f5c6e) SHA1(1d5c2f33de6d1efa2b27b7c43b30268946ef5920), ROM_BIOS(0))
		ROMX_LOAD("12_04dcc52f_8519408.bin", 0x14000, 0x2000, CRC(04dcc52f) SHA1(feba4f189a8bb442c241dadb1fa1f2cb4f344fa3), ROM_BIOS(0))
		ROMX_LOAD("13_9a3f70c7_8519414.bin", 0x16000, 0x2000, CRC(9a3f70c7) SHA1(2dc509b8fa0f84a12df2ee6e91e5fbb29ce7c541), ROM_BIOS(0))
		ROMX_LOAD("14_91b2969e_8519406.bin", 0x18000, 0x2000, CRC(91b2969e) SHA1(dedac0b9b3e607bcb03bc16653c2c002eb67b633), ROM_BIOS(0))
		ROMX_LOAD("15_0f9a99fa_8519416.bin", 0x1a000, 0x2000, CRC(0f9a99fa) SHA1(ba174188167482997694b84f68b9d45e55187212), ROM_BIOS(0))
		ROMX_LOAD("16_e451a5e2_8519409.bin", 0x1c000, 0x2000, CRC(e451a5e2) SHA1(f41d2493f696387a2c729420ad8beafac96e604d), ROM_BIOS(0))
		ROMX_LOAD("17_b9fb8bf1_8519410.bin", 0x1e000, 0x2000, CRC(b9fb8bf1) SHA1(e5de770db73ac4c30fad5c03445b5239c133d84a), ROM_BIOS(0))
		ROMX_LOAD("18_22cb6de4_8519417.bin", 0x20000, 0x2000, CRC(22cb6de4) SHA1(ae050de1dd20afc25fe97012f6b088be0ae47878), ROM_BIOS(0))
		ROMX_LOAD("19_2e665945_4481711.bin", 0x22000, 0x2000, CRC(2e665945) SHA1(4ae61c13786b44b28a02055c104ef63355a629b9), ROM_BIOS(0))

		//ROS 1.01 (1980)
		ROMX_LOAD("02_765abd93_8493746.bin", 0x00000, 0x2000, CRC(765abd93) SHA1(1ec489f1d2f72bf7e9ddc5ef642a8336b3ff67e3), ROM_BIOS(1))
		ROMX_LOAD("09_07843020_8493747.bin", 0x02000, 0x2000, CRC(07843020) SHA1(828ca0199af1246f6caf58bcb785f791c3a7e34e), ROM_BIOS(1))
		ROMX_LOAD("0a_b9569153_8493748.bin", 0x04000, 0x2000, CRC(b9569153) SHA1(92ccf91766557bc565ad36fc131396aff28b8999), ROM_BIOS(1))
		ROMX_LOAD("0b_4f631183_8493754.bin", 0x06000, 0x2000, CRC(4f631183) SHA1(5f7b011129616bae46c70133309a70c29cc0f127), ROM_BIOS(1))
		ROMX_LOAD("0c_48646293_8493749.bin", 0x08000, 0x2000, CRC(48646293) SHA1(3d7eaf2c143499757681fbedbc3716829ef9bd25), ROM_BIOS(1))
		ROMX_LOAD("0d_bea5a812_8493755.bin", 0x0a000, 0x2000, CRC(bea5a812) SHA1(5da3a9231c5d456fa7a26ab36b9d5380e096af59), ROM_BIOS(1))
		ROMX_FILL(0x0c000, 0x2000, 0xff, ROM_BIOS(1))//ROM E is empty
		ROMX_FILL(0x0e000, 0x2000, 0xff, ROM_BIOS(1))//ROM F is empty
		ROMX_FILL(0x10000, 0x2000, 0xff, ROM_BIOS(1))//ROM 10 is empty
		ROMX_FILL(0x10000, 0x2000, 0xff, ROM_BIOS(1))//ROM 11 is empty
		ROMX_LOAD("12_04dcc52f_8493760.bin", 0x14000, 0x2000, CRC(04dcc52f) SHA1(feba4f189a8bb442c241dadb1fa1f2cb4f344fa3), ROM_BIOS(1))
		ROMX_LOAD("13_26869666_8493761.bin", 0x16000, 0x2000, CRC(26869666) SHA1(9fae28fe3613218a6f8d7fb7a88bbc29a3f75a0f), ROM_BIOS(1))
		ROMX_LOAD("14_91b2969e_8493756.bin", 0x18000, 0x2000, CRC(91b2969e) SHA1(dedac0b9b3e607bcb03bc16653c2c002eb67b633), ROM_BIOS(1))
		ROMX_LOAD("15_269db39d_8493762.bin", 0x1a000, 0x2000, CRC(269db39d) SHA1(f32c3754cff5a8bbfc76fc23040731cb7ff343fa), ROM_BIOS(1))
		ROMX_LOAD("16_e451a5e2_8493763.bin", 0x1c000, 0x2000, CRC(e451a5e2) SHA1(f41d2493f696387a2c729420ad8beafac96e604d), ROM_BIOS(1))
		ROMX_LOAD("17_b9fb8bf1_8493764.bin", 0x1e000, 0x2000, CRC(b9fb8bf1) SHA1(e5de770db73ac4c30fad5c03445b5239c133d84a), ROM_BIOS(1))
		ROMX_LOAD("18_41e6c232_8493765.bin", 0x20000, 0x2000, CRC(41e6c232) SHA1(6711000a0c6836a411997de15274b990dd2c0ed0), ROM_BIOS(1))
		ROMX_LOAD("19_73aeeb56_8493727.bin", 0x22000, 0x2000, CRC(73aeeb56) SHA1(abfce66d49662731d9e41e06c54c63cd40f9caac), ROM_BIOS(1))

		ROM_REGION(0x2000, "chargen", 0)
		ROMX_LOAD("chr_73783bc7_8519412.bin", 0x0000, 0x2000, CRC(73783bc7) SHA1(45ee2a9acbb577b281ad8181b7ec0c5ef05c346a),ROM_BIOS(0))
		ROMX_LOAD("chr_73783bc7_6842372.bin", 0x0000, 0x2000, CRC(73783bc7) SHA1(45ee2a9acbb577b281ad8181b7ec0c5ef05c346a),ROM_BIOS(1))
	ROM_END

}

COMP( 1981, system23, 0,      0,      system23, system23,     system23_state, empty_init, "IBM",   "IBM System/23 Datamaster", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_GRAPHICS)
