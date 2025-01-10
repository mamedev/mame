// license:BSD-3-Clause
// copyright-holders: Jaume López

#define VERBOSE 1

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "video/i8275.h"
#include "machine/ram.h"
#include "screen.h"
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
				m_ppi_kbd(*this, "ppi_kbd"),
				m_ppi_diag(*this, "ppi_diag"),
				m_ppi_settings(*this, "ppi_settings"),
				m_dmac(*this,"dma"),
				m_crtc(*this, "crtc"),
				m_chargen(*this, "chargen"),
				m_ram(*this, RAM_TAG),
				m_screen(*this, "screen"),
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
			required_device<i8255_device> m_ppi_kbd;
			required_device<i8255_device> m_ppi_diag;
			required_device<i8255_device> m_ppi_settings;
			required_device<i8257_device> m_dmac;
			required_device<i8275_device> m_crtc;
			required_region_ptr<uint8_t> m_chargen;
			required_device<ram_device> m_ram;
			required_device<screen_device> m_screen;
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

			void diag_digits_w(uint8_t data);

			void cpu_test_register_w(uint8_t data);
			uint8_t cpu_test_register_r();

			uint8_t memory_settings_r();

			void sod_w(int state);
			int sid_r();

			uint8_t dmac_mem_r(offs_t offset);
			void dmac_mem_w(offs_t offset, uint8_t data);

			void dmac_hrq_w(int state);

			void crtc_dack_w(offs_t offset, uint8_t data);
			I8275_DRAW_CHARACTER_MEMBER(display_pixels);
			uint8_t crtc_test_vars_r();
			void hrtc_r(uint8_t data);
			void vrtc_r(uint8_t data);

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
		return 0;	//Patch to make the memory test work while port 2e is being studied
	}

	//This routine deals with the SID signal being read. The CPU test fails if the line is not asserted high

	int system23_state::sid_r()
	{
		return ASSERT_LINE; // Actually, SID is tied to VCC at the motherboard
	}

	//This routine deals with the SOD signal being written. As of writting this comment there is no known purpose for SOD

	void system23_state::sod_w(int state)
	{
		// At this point, it is unknown what SOD is attached to
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

		using namespace i8275_attributes;

		if (!BIT(attrcode, VSP))
			gfx = m_chargen[(linecount & 15) | (charcode << 4)];

		// if (BIT(attrcode, LTEN))
		// 	gfx = 0xff;

		if (BIT(attrcode, RVV))
			gfx ^= 0xff;

		//if (BIT(attrcode, GPA0) || BIT(attrcode, GPA1)) printf("GPA0: %u GPA1: %u\n", BIT(attrcode, GPA0), BIT(attrcode, GPA1));
		if (BIT(attrcode, GPA0) && BIT(attrcode, GPA1) && !lpen_ct)
		{
			LOG("Sytem/23: Light pen asserted\n");
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

	//This routine describes the computer's I/O map

	void system23_state::system23_io(address_map &map)
	{
		map.unmap_value_high();
		map(0x00, 0x08).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
		map(0x2c, 0x2f).rw(m_ppi_settings, FUNC(i8255_device::read), FUNC(i8255_device::write));
		map(0x40, 0x43).rw(m_ppi_diag, FUNC(i8255_device::read), FUNC(i8255_device::write));
		map(0x44, 0x45).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
		map(0x4c, 0x4f).rw(m_ppi_kbd, FUNC(i8255_device::read), FUNC(i8255_device::write));
	}

	//This routine describes the computer's memory map

	void system23_state::system23_mem(address_map &map)
	{
		map.unmap_value_high();
		map(0x0000, 0x3fff).rom();
		map(0x8000, 0xbfff).ram();

	}

	//This is the constructor of the class

	void system23_state::system23(machine_config &config)
	{
		I8085A(config, m_maincpu, (18'432'000 / 3));
		m_maincpu->set_addrmap(AS_PROGRAM, &system23_state::system23_mem);
		m_maincpu->set_addrmap(AS_IO, &system23_state::system23_io);
		m_maincpu->in_sid_func().set(FUNC(system23_state::sid_r));
		m_maincpu->out_sod_func().set(FUNC(system23_state::sod_w));

		I8255(config, m_ppi_kbd);
		m_ppi_kbd->in_pa_callback().set(FUNC(system23_state::cpu_test_register_r));
		m_ppi_kbd->out_pa_callback().set(FUNC(system23_state::cpu_test_register_w));
		m_ppi_kbd->in_pb_callback().set(FUNC(system23_state::crtc_test_vars_r));

		I8255(config, m_ppi_diag);
		m_ppi_diag->out_pb_callback().set(FUNC(system23_state::diag_digits_w));

		I8255(config, m_ppi_settings);
		m_ppi_settings->in_pc_callback().set(FUNC(system23_state::memory_settings_r));
		m_ppi_settings->in_pa_callback().set_ioport(m_language);
		m_ppi_settings->in_pb_callback().set_ioport(m_cedip);

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
		//m_crtc->irq_wr_callback().set_inputline(m_maincpu, I8085_RST55_LINE); // Only when jumper J1 is bridged
		m_crtc->hrtc_wr_callback().set(FUNC(system23_state::hrtc_r));
		m_crtc->vrtc_wr_callback().set(FUNC(system23_state::vrtc_r));


		RAM(config, m_ram).set_default_size("16k");

		config.set_perfect_quantum(m_maincpu);
		config.set_default_layout(layout_ibmsystem23);
	}

	void system23_state::machine_start()
	{
		m_diag_digits.resolve();
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
			PORT_DIPNAME( 0x01, 0x00, "A1")
			PORT_DIPSETTING(    0x01, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x02, 0x00, "A2")
			PORT_DIPSETTING(    0x02, DEF_STR( Off ))
			PORT_DIPSETTING(    0x00, DEF_STR( On ))
			PORT_DIPNAME( 0x04, 0x00, "A3")
			PORT_DIPSETTING(    0x04, DEF_STR( Off ))
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
	INPUT_PORTS_END


	ROM_START( system23 )
		ROM_SYSTEM_BIOS(0, "r_set", "\"R\" Set - 1982?")
		ROM_SYSTEM_BIOS(1, "tm_set", "\"TM\" Set - 1981?")

		ROM_REGION(0x4000, "maincpu", 0)
		ROMX_LOAD("02_61c9866a_4481186.bin", 0x0000, 0x2000, CRC(61c9866a) SHA1(43f2bed5cc2374c7fde4632948329062e57e994b),ROM_BIOS(0))
		ROMX_LOAD("09_07843020_8493747.bin", 0x2000, 0x2000, CRC(07843020) SHA1(828ca0199af1246f6caf58bcb785f791c3a7e34e),ROM_BIOS(0))

		ROMX_LOAD("02_765abd93_8493746.bin", 0x0000, 0x2000, CRC(765abd93) SHA1(1ec489f1d2f72bf7e9ddc5ef642a8336b3ff67e3),ROM_BIOS(1))
		ROMX_LOAD("09_07843020_8493747.bin", 0x2000, 0x2000, CRC(07843020) SHA1(828ca0199af1246f6caf58bcb785f791c3a7e34e),ROM_BIOS(1))

		ROM_REGION(0x2000, "chargen", 0)
		ROMX_LOAD("chr_73783bc7_8519412.bin", 0x0000, 0x2000, CRC(73783bc7) SHA1(45ee2a9acbb577b281ad8181b7ec0c5ef05c346a),ROM_BIOS(0))
		ROMX_LOAD("chr_73783bc7_6842372.bin", 0x0000, 0x2000, CRC(73783bc7) SHA1(45ee2a9acbb577b281ad8181b7ec0c5ef05c346a),ROM_BIOS(1))
	ROM_END

}

COMP( 1981, system23, 0,      0,      system23, system23,     system23_state, empty_init, "IBM",   "IBM System/23 Datamaster", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
