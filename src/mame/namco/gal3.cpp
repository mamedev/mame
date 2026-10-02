// license:BSD-3-Clause
// copyright-holders:  John & Phil Bennett (2026), Naibo Zhang (April 2009)
/* Galaxian ^3


  System Overview:
  This is a Scalable, Multi-CPU and Multi-User System.
  The largest scale configuration known so far was capable of 28(!) players and 16 screens wrapped around. (retired in the early 2000's)

  System has one Master 68020 CPU Board for game play, and one or more Slave 68020 CPU Boards for graphics.

  The Master CPU communicates with (one or more)Slave CPUs via multi-port shared RAM, the "C-RAM board".
  Each C-RAM board is configured as 2 banks of Triple-Port Static RAM Units. The 2 Master-CPU-Port of the 2 RAM banks
  are joined together, leaving 4 Slave-CPU-Ports. More than one C-RAM boards can be chained together.

  The Master CPU controls a Sound Board and a RSO board. RSO board contains Namco 139 serial communication ICs (x9 pieces),
  connecting to several "Personal Boards" (Player-Terminals), which handle players' input and vibration/lamp feedback.
  Each of the Sound board, the RSO board, and the Personal board has their own 68000 CPU.

  Slave CPU connects to a cluster of namcos21-type video board sets: DSP-PGN-OBJ. A DSP board has 5x TMS320C25 running at 40MHz.

  Every 68020 CPU board has 512KB of local Hi-Speed SRAM. Most code/data required by the Slave CPU sub-system are
  uploaded from the Master CPU's ROM. Among them are:
  Slave CPU program, DSP program, Calculation Tables, Vertex Array Index, Palette, 2D Sprite Data, etc.

 

  System Diagram:
  ---------------
                                                                    Serial Comm Cable
     Master 68020 -------- RSO 68000, 9x Namco 139 SCI ICs <==============================> Personal Board 68000
          |                     |                                       ........ more personal boards.........
          |                     |                                       ........ more personal boards.........
          |                     |________ LD Players UART bus
          |                     |-------- Sound 68000, 4x Namco 140 ICs
     |---------|
     |  C-RAM  |
     | #-# #-# |
     |---------|
       | | | |
       | | | |------- Slave 68020
       | | |                |-------- 1x master DSP, 4x slave DSPs, Polygon, 2D Sprite ------> V-MIX board -----> SCREEN
       | | |                |-------- ........ more video boards ......... (max 2 per slave?)     
       | | |                                                                                   
       | | |
       | | |------- ........ more slave 68020s .........
       | |
       | |------- ........ more slave 68020s .........
       |
       |------- ........ more slave 68020s .........


The versions dumped/emulated here are for the Theatre 6 with 2 video outputs, driving 2x 15kHz CRT projectors (480i), with 6 players.
Two Pioneer LD-V8000 laserdiscs are used, one for each screen.

  PCBs:

	2x backplane pcbs
	2x CPU pcbs (one with master MST roms, one with slave SLV roms)
	1x CRAM pcb
	1x RS (RS0) pcb
	1x SOUND pcb
	2x OBJ pcb
	2x PGN pcb
	2x DSP pcb
	1x VMIX pcb
	3x PERSONAL pcbs

	(17 pcbs in total needed for a complete set).

	There are two 6-player game variants, differing in laserdisc and ROMs:

		- Galaxian3 Project Dragoon (1992)
		- Attack of the Zolgear (1994)

	Sep 26 

	Emulation status:
	Dragoon and Zolgear playable

	Notes/TODO:
	MAME doesn't support digital laserdisc audio, can get quadrophonic by overlaying digital PCM to the right laserdisc audio in the .chd file.
		-Left laserdisc = analogue stereo, right laserdisc = digital audio
	C140 audio not quite there
	PSN audio not implemented. Just DACs on each board.
	Palette issues - namcos21_3d not right and sprite file hack needs a tidy
	Apallingly CPU intensive - entirely due to overhead of 2 screen's worth of video DSPs
	Outputs not implemented - just lamps and LEDs anyway.
	LDV8000 needs a bit more work, otherwise might as well be a clone of LDV4200
	C139 not quite right - update isn't handling coing inputs properly as they have a time period to register.
	C139 file needs a tidy and some thought to avoid clashing with networking C139 games
	
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "cpu/tms320c2x/tms320c2x.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/mc68681.h"
#include "machine/ldv8000hle.h"
#include "sound/c140.h"
#include "layout/generic.h"
#include "speaker.h"
#include "namco_c355spr.h"
#include "namcos21_dsp_c67.h"
#include "namcos21_3d.h"
#include "namco_c139_internal.h"
#include "namco_c148.h"

#include "emupal.h"

#define LEDS
//#define DEBUG_SCREEN

#ifdef DEBUG_SCREEN
#define NUMSCREENS 3
#else
#define NUMSCREENS 2

#endif

namespace {

class gal3_state : public driver_device
{
public:
	gal3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_c355spr(*this, "c355spr_%u", 1U),
		m_palette(*this, "palette_%u", 1U),
		m_rso_shared_ram(*this, "rso_shared_ram"),
		m_c140_16a(*this, "c140_16a"),
		m_c140_16c(*this, "c140_16c"),
		m_c140_16g(*this, "c140_16g"),
		m_c140a_region(*this, "c140_16a"),
		m_c140c_region(*this, "c140_16c"),
		m_c140g_region(*this, "c140_16g"),
		m_namcos21_3d(*this, "namcos21_3d_%u", 1U),
		m_namcos21_dsp_c67(*this, "namcos21dsp_c67_%u", 1U),
#ifdef DEBUG_SCREEN
		m_debug_screen(*this, "screen_dbg"),
#endif
		m_screen_l(*this, "screen_l"),
		m_screen_r(*this, "screen_r"),
		m_maincpu(*this, "main_cpu"),
		m_subcpu(*this, "slv_cpu"),
		m_rso_cpu(*this, "rso_cpu"),
		m_sound_cpu(*this, "sound_cpu"),
		m_psn_cpu(*this, "psn_cpu_%u",1U),
		m_rso_intc(*this, "rso_intc"),
		m_laserdisc(*this, "laserdisc_%u", 1U),
		m_uart(*this, "uart_%u",1u),
		m_sci_rso(*this, "sci_rso_%u",1U),
		m_sci_psn(*this, "sci_psn_%u", 1U),
		m_sci_sound(*this, "sci_sound"),
		m_adc_ports_psn0(*this, "ADC_psn0.%u", 0),
		m_adc_ports_psn1(*this, "ADC_psn1.%u", 0),
		m_adc_ports_psn2(*this, "ADC_psn2.%u", 0)

	{ }

	void gal3(machine_config &config);
	void gal3_init();
	void gal3zlgr_init();


protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device_array<namco_c355spr_device, NUMSCREENS> m_c355spr;
	required_device_array<palette_device, NUMSCREENS> m_palette;
	uint16_t m_video_enable[NUMSCREENS];
	required_shared_ptr<uint16_t> m_rso_shared_ram;
	required_device<c140_device> m_c140_16a;
	required_device<c140_device> m_c140_16c;
	required_device<c140_device> m_c140_16g;
	required_region_ptr<u16> m_c140a_region;
	required_region_ptr<u16> m_c140c_region;
	required_region_ptr<u16> m_c140g_region;

	required_device_array<namcos21_3d_device, 2> m_namcos21_3d;
	required_device_array<namcos21_dsp_c67_device, 2> m_namcos21_dsp_c67;
	
#ifdef DEBUG_SCREEN
	required_device<screen_device> m_debug_screen;
#endif
	required_device<screen_device> m_screen_l;
	required_device<screen_device> m_screen_r;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_rso_cpu;
	required_device<cpu_device> m_sound_cpu;
	required_device_array<cpu_device, 3> m_psn_cpu;
	required_device<namco_c148_device> m_rso_intc;
	required_device_array<pioneer_ldv8000hle_device, 2> m_laserdisc;
	required_device_array<mc68681_device,6> m_uart;
	required_device_array<namco_c139_local_device,4> m_sci_rso;
	required_device_array<namco_c139_local_device, 3> m_sci_psn;
	required_device<namco_c139_local_device> m_sci_sound;
	optional_ioport_array<8> m_adc_ports_psn0;
	optional_ioport_array<8> m_adc_ports_psn1;
	optional_ioport_array<8> m_adc_ports_psn2;

	uint16_t m_psn0_adc_mux; // ADC mux settings
	uint16_t m_psn1_adc_mux; 
	uint16_t m_psn2_adc_mux; 
	uint32_t m_led_mst = 0;
	uint32_t m_led_slv = 0;

	uint32_t m_serialprod = 0;

	uint16_t m_rso_c139_irq = 0;

	bool m_is_dragoon; // hack for original game as some differences in emulation (I need to close)
	uint16_t m_sci_flags; // For identifying SCI interrupt sources as not via interrupt controller IC

	int m_framedelay;

	uint32_t led_mst_r();
	void led_mst_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t led_slv_r();
	void led_slv_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Screen> uint16_t video_enable_r();
	
	uint16_t video_enable_r1();
	uint16_t video_enable_r2();

	template<int Screen> void video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rso_r(offs_t offset);
	void rso_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

#ifdef DEBUG_SCREEN
	uint32_t screen_update_debug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
#endif
	uint32_t screen_update_ld1(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);
	uint32_t screen_update_ld2(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);
	uint32_t screen_update_laserdiscs(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect, uint8_t screen_index);

	void mst_cpu_map(address_map &map) ATTR_COLD;
	void slv_cpu_map(address_map &map) ATTR_COLD;
	void psn_cpu_map_0(address_map &map) ATTR_COLD;
	void psn_cpu_map_1(address_map& map) ATTR_COLD;
	void psn_cpu_map_2(address_map& map) ATTR_COLD;
	void psn_cpu_common(address_map& map);

	void rso_cpu_map(address_map &map) ATTR_COLD;
	void sound_cpu_map(address_map &map) ATTR_COLD;
	void c140a_map(address_map& map) ATTR_COLD;
	void c140c_map(address_map& map) ATTR_COLD;
	void c140g_map(address_map& map) ATTR_COLD;


	void maincpu_wipeirq1(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_maincpu->set_input_line(1, CLEAR_LINE);
	}
	void maincpu_wipeirq3(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_maincpu->set_input_line(3, CLEAR_LINE);
	}

	uint16_t  maincpu_readirq1(offs_t offset)
	{
		m_maincpu->set_input_line(1, CLEAR_LINE);
		return 0;
	}
	uint16_t  maincpu_readirq3(offs_t offset)
	{
		m_maincpu->set_input_line(3, CLEAR_LINE);
		return 0;
	}

	void subcpu_wipeirq1(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_subcpu->set_input_line(1, CLEAR_LINE);
	}
	uint16_t  subcpu_readirq1(offs_t offset)
	{
		m_subcpu->set_input_line(1, CLEAR_LINE);
		return 0;
	}
	void subcpu_wipeirq3(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_subcpu->set_input_line(3, CLEAR_LINE);
	}

	void reset_all_dsps(int state)
	{

		m_namcos21_dsp_c67[0]->reset_dsps(state);
		m_namcos21_dsp_c67[1]->reset_dsps(state);

	}

	void dsp_reset_w(uint8_t data)
	{
		reset_all_dsps(data & 1 ? CLEAR_LINE : ASSERT_LINE);

	}

	void rso_reset_w(uint16_t data)
	{
		m_rso_cpu->set_input_line(INPUT_LINE_RESET, data & 1 ? CLEAR_LINE : ASSERT_LINE);
	}


	uint16_t  rso_read_c139_irqs(offs_t offset)
	{
		//Bit 0-2 are PSN
		//Bit 8 is sound
		if (m_is_dragoon)
		{
			return m_sci_flags;
		}

		return 0x100;
	}
	void rso_write_c139_irqs(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_rso_c139_irq = data;
	}

	uint16_t rso_uart_irq_flag_read(offs_t offset)
	{
		uint16_t result = 0xFFFF;
		if (m_serialprod)
		{
			//Only lowest UART used
			result = 0xfffe;
			m_serialprod = 0;
		}


		return result;
	}
	void rso_uart_irq_write(offs_t offset, uint16_t data, uint16_t mem_mask)
	{

	}

	void uart_irq(offs_t offset, uint16_t data, uint16_t mem_mask)
	{

	}

	// TODO - these are overkill as it's just a bit to set - maybe dont' need to handle both read and write
	// NEED TO BE BITWISE TOO, not just wipe the lot for every C138 upon reading
	uint16_t psn_iack_r(offs_t offset)
	{
		uint16_t result = m_sci_flags;
		m_sci_flags = 0x1ff;
		return result;

	}
	void psn_iack_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_sci_flags = 0x1ff;
	}

	

	// Trigger RSO when any of the C139's to PSN boards send data
	void sci_to_rso_irq_trig_1(int state)
	{
		m_sci_flags = 0x1FE;
		m_rso_intc->ex_irq_trigger();
	}
	void sci_to_rso_irq_trig_2(int state)
	{
		m_sci_flags = 0x1FD;
		m_rso_intc->ex_irq_trigger();
	}
	void sci_to_rso_irq_trig_3(int state)
	{
		m_sci_flags = 0x1FB;
		m_rso_intc->ex_irq_trigger();
	}

	void psn1_sci_rx_irq(int state)
	{
		m_psn_cpu[0]->set_input_line(3, ASSERT_LINE);

	}
	void psn2_sci_rx_irq(int state)
	{
		m_psn_cpu[1]->set_input_line(3, ASSERT_LINE);

	}
	void psn3_sci_rx_irq(int state)
	{
		m_psn_cpu[2]->set_input_line(3, ASSERT_LINE);

	}

	void sound_sci_rx_irq(int state)
	{
		//osd_printf_verbose("Sound SCI IRQ: %x, %x\n", state, 0);
		m_sound_cpu->set_input_line(1, HOLD_LINE);
	}


	uint16_t sound_sci_irq_clear_r(offs_t offset)
	{
	//	osd_printf_verbose("Sound SCI IRQ Clear: %x,\n", 0);
		m_sound_cpu->set_input_line(1, CLEAR_LINE);
		return 0;
	}

	void sound_sci_irq_clear_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
	//	osd_printf_verbose("Sound SCI IRQ Clear: %x,\n", 0);
		m_sound_cpu->set_input_line(1, CLEAR_LINE);
	}


	uint16_t sound_irq2_clear_r(offs_t offset)
	{
		m_sound_cpu->set_input_line(2, CLEAR_LINE);
		return 0;
	}

	void sound_irq2_clear_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_sound_cpu->set_input_line(2, CLEAR_LINE);
	}


	//SCI IRQ 3?
	void psn0_sci_ack(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_psn_cpu[0]->set_input_line(3, CLEAR_LINE);
	}
	void psn1_sci_ack(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_psn_cpu[1]->set_input_line(3, CLEAR_LINE);
	}
	void psn2_sci_ack(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_psn_cpu[2]->set_input_line(3, CLEAR_LINE);
	}

	uint16_t psn0_sci_read(offs_t offset)
	{
		m_psn_cpu[0]->set_input_line(3, CLEAR_LINE);
		return (0);
	}

	uint16_t psn1_sci_read(offs_t offset)
	{
		m_psn_cpu[1]->set_input_line(3, CLEAR_LINE);
		return (0);
	}	
	
	uint16_t psn2_sci_read(offs_t offset)
	{
		m_psn_cpu[2]->set_input_line(3, CLEAR_LINE);
		return (0);
	}


	void psn0_adc_mux_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_psn0_adc_mux = data & 0x7;
	}

	void psn1_adc_mux_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_psn1_adc_mux = data & 0x7;
	}

	void psn2_adc_mux_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		m_psn2_adc_mux = data & 0x7;
	}

	uint16_t psn0_adc_read(offs_t offset)
	{
		uint16_t port = m_psn0_adc_mux;
		if (port < 4)
		{
			return m_adc_ports_psn0[port].read_safe(0);
		}
		else if (port == 4)
		{
			return 0x800; //centre
		}
		else if (port == 5)
		{
			return 0x800;//0x7ff;
		}
		else if (port == 6)
		{
			return 0x800;
		}
		else
		{
			return 0x800;//0x7ff;
		}
				
	}
	uint16_t psn1_adc_read(offs_t offset)
	{
		uint16_t port = m_psn1_adc_mux;
		if (port < 4)
		{
			return m_adc_ports_psn1[port].read_safe(0);
		}
		else if (port == 4)
		{
			return 0x800; //centre
		}
		else if (port == 5)
		{
			return 0x800;//0x7ff;
		}
		else if (port == 6)
		{
			return 0x800;
		}
		else
		{
			return 0x800;//0x7ff;
		}

	}
	uint16_t psn2_adc_read(offs_t offset)
	{
		uint16_t port = m_psn2_adc_mux;
		if (port < 4)
		{
			return m_adc_ports_psn2[port].read_safe(0);
		}
		else if (port == 4)
		{
			return 0x800; //centre
		}
		else if (port == 5)
		{
			return 0x800;//0x7ff;
		}
		else if (port == 6)
		{
			return 0x800;
		}
		else
		{
			return 0x800;//0x7ff;
		}

	}

	
	uint16_t psn_read_inputs_fudge(offs_t offset)
	{
		if (offset == 0)
		{
			return 0xffff;
		}
		else
		{
			return 0x8001;
		}
	}


	uint16_t psn_acia_r(offs_t offset)
	{
		
		return 0xff;
	}
	


	uint16_t sound_dip(offs_t offset)
	{

			return 0xffff;
		
	}


	u16 c140a_rom_r(offs_t offset)
	{

		uint16_t ret = 0x00;

		if (m_is_dragoon)
		{
			const bool play = BIT(offset, 20);

			

			if (play)
			{
				ret = m_c140a_region[offset & 0x7FFFF]; // voice1 or voice2

				ret |= (ret >> 8);
			}
		}
		else
		{
			const bool play = BIT(offset, 19);


			if (!play)
			{
				ret = m_c140a_region[((offset & 0x300000) >> 1) | (offset & 0x7ffff)];
			}
		
		}

		return ret;
	}

	u16 c140c_rom_r(offs_t offset)
	{

		uint16_t ret = 0x00;

		if (m_is_dragoon)
		{
			const bool play = BIT(offset, 20);



			if (play)
			{
				ret = m_c140c_region[offset & 0x7FFFF]; // voice1 or voice2

				ret |= (ret >> 8);
			}
		}
		else
		{
			const bool play = BIT(offset, 19);


			if (!play)
			{
				ret = m_c140c_region[((offset & 0x300000) >> 1) | (offset & 0x7ffff)];
			}

		}

		return ret;
	}

	u16 c140g_rom_r(offs_t offset)
	{
		//TODO
		uint16_t ret = 0x00;

		if (m_is_dragoon)
		{
			const bool play = BIT(offset, 20);



			if (play)
			{
				ret = m_c140g_region[offset & 0x7FFFF]; // voice1 or voice2

				ret |= (ret >> 8);
			}
		}
		else
		{
			const bool play = BIT(offset, 19);


			if (!play)
			{
				ret = m_c140g_region[((offset & 0x300000) >> 1) | (offset & 0x7ffff)];
			}

		}

		return ret;
	}

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

};




TIMER_DEVICE_CALLBACK_MEMBER(gal3_state::screen_scanline)
{
	// Hack to delay the kickstart (TODO: just change the DSP kickstart code to remove this)
	int scanline = param;


	if (scanline == 1)
	{

		m_rso_intc->vblank_irq_trigger();


		m_serialprod = 1;
		m_rso_cpu->set_input_line(4, HOLD_LINE); // UART

		m_psn_cpu[0]->set_input_line(4, HOLD_LINE); // ACIA (Coin inputs)
		m_psn_cpu[1]->set_input_line(4, HOLD_LINE); // ACIA (Coin inputs)
		m_psn_cpu[2]->set_input_line(4, HOLD_LINE); // ACIA (Coin inputs)
		

		if (m_framedelay == 0)
		{
			m_framedelay = 600;

			m_namcos21_dsp_c67[0]->reset_kickstart();
			m_namcos21_dsp_c67[1]->reset_kickstart();
		}
		else
		{
			m_framedelay--;
		}
		
	}
	

}


void gal3_state::machine_start()
{
	save_item(NAME(m_led_mst));
	save_item(NAME(m_led_slv));
	m_framedelay = 600;
}

void gal3_state::video_start()
{
	save_item(NAME(m_video_enable));
}

/* One function for both sides, just index it with 0 or 1 */
uint32_t gal3_state::screen_update_laserdiscs(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect, uint8_t screen_index)
{
	bitmap_ind16 tmp_bitmap;
	tmp_bitmap.allocate(768, 528);

	tmp_bitmap.fill(0x0000, cliprect); // TODO : actually laserdisc layer

	m_c355spr[screen_index]->build_sprite_list_and_render_sprites(cliprect); // TODO : buffered?

	m_namcos21_3d[screen_index]->copy_visible_poly_framebuffer(tmp_bitmap, cliprect);



	for (int pri = 0; pri < 15; pri++)
	{
		m_c355spr[screen_index]->draw(screen, tmp_bitmap, cliprect, pri);
	}
	const rgb_t* palette = m_palette[screen_index]->palette()->entry_list_raw();


	/* loop over rows and copy to the destination */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{

		uint16_t* src = &tmp_bitmap.pix(y, cliprect.min_x);
		uint32_t* dest = &bitmap.pix(y, cliprect.min_x);
		/* loop over columns */
		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{

			uint32_t data = *src;
			uint32_t col = palette[data & 0xFFFF];
			if (col != 0xff009200)
			{
				*dest = col;
			}
			else
			{
				*dest = 00000000;
			}

			src++;
			dest++;

		}
	}

	return 0;
}

uint32_t gal3_state::screen_update_ld1(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{

	screen_update_laserdiscs(screen, bitmap, cliprect, 0);

	return 0;

}

uint32_t gal3_state::screen_update_ld2(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{

	screen_update_laserdiscs(screen, bitmap, cliprect, 1);	
	return 0;
}

#ifdef DEBUG_SCREEN
uint32_t gal3_state::screen_update_debug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x77, cliprect); 
	screen.priority().fill(0, cliprect);
	m_c355spr[2]->build_sprite_list_and_render_sprites(cliprect); // TODO : buffered?

	int pri;

	
	for( pri=0; pri<15; pri++ )
	{
		m_c355spr[2]->draw(screen, bitmap, cliprect, pri);

	}


	// CPU Diag LEDs.
// Should match service manual order and colours.

	char led_array_m[]{ "GRYGRYGRYGRYGRYG" };
	char led_array_s[]{ "GRYGRYGRYGRYGRYG" };

	uint16_t led_mst = (uint16_t)(m_led_mst >> 16);
	uint16_t led_sub = (uint16_t)(m_led_slv >> 16);
	led_mst ^= 0xffff;
	led_sub ^= 0xffff;

	for (int i = 0; i < 16; i++)
	{
		if ((led_mst & (0x8000 >> i)) == 0)
		{
			led_array_m[i] = '_';
		}
		if ((led_sub & (0x8000 >> i)) == 0)
		{
			led_array_s[i] = '_';
		}
	}

#ifdef LEDS
	popmessage("LED_MST 16-1:  %x\nLED_SLV 16-1:  %x\n \n", led_array_m, led_array_s);
#endif
	return 0;

//	return UPDATE_HAS_NOT_CHANGED;
}
#endif

/***************************************************************************************/

uint32_t gal3_state::led_mst_r()
{
	return m_led_mst;
}

void gal3_state::led_mst_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_led_mst);
}

uint32_t gal3_state::led_slv_r()
{
	return m_led_slv;
}

void gal3_state::led_slv_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_led_slv);
}


uint16_t gal3_state::video_enable_r1()
{
	static uint16_t m_venabletest{ 0 };
	m_venabletest ^= 0x1;
	return m_venabletest;

}

uint16_t gal3_state::video_enable_r2()
{
	static uint16_t m_venabletest{ 0 };
	m_venabletest ^= 0x1;
	return m_venabletest;

}

template<int Screen>
uint16_t gal3_state::video_enable_r()
{
	uint16_t data = m_video_enable[Screen];


	return rand() & 1;

	return data;
}

template<int Screen>
void gal3_state::video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_enable[Screen]); // 0xff53, instead of 0x40 in namcos21
}

uint16_t gal3_state::rso_r(offs_t offset)
{

	return m_rso_shared_ram[offset];
}

void gal3_state::rso_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rso_shared_ram[offset]);
}


void gal3_state::mst_cpu_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x20000000, 0x20001fff).ram().share("nvmem");   //NVRAM
	map(0x40000000, 0x40000001).w(FUNC(gal3_state::rso_reset_w));
	map(0x44000000, 0x44000003).portr("DSW_CPU_mst");
	map(0x44800000, 0x44800003).r(FUNC(gal3_state::led_mst_r)).w(FUNC(gal3_state::led_mst_w)); //LEDs
	map(0x48000000, 0x48000003).rw(FUNC(gal3_state::maincpu_readirq1), FUNC(gal3_state::maincpu_wipeirq1));
	map(0x4c000000, 0x4c000003).rw(FUNC(gal3_state::maincpu_readirq3), FUNC(gal3_state::maincpu_wipeirq3));
	map(0x5fff0000, 0x5fff7fff).mirror(0x10000).ram().share("share1");  //CRAM (TODO - whats pointing it here?)
	map(0x60000000, 0x60007fff).mirror(0x10000).ram().share("share1");  //CRAM
	map(0x80000000, 0x8007ffff).ram(); //512K Local RAM
/// map(0xc0000000, 0xc000000b).nopw();    //upload?
	map(0xc000000c, 0xc000000f).nopr(); //irq2 ack
/// map(0xd8000000, 0xd800000f).ram(); // protection or 68681?
	map(0xf2800000, 0xf2800fff).rw(FUNC(gal3_state::rso_r), FUNC(gal3_state::rso_w)); //RSO PCB

#ifdef DEBUG_SCREEN
	//Debug screen (zolgear)	
	map(0xf4700000, 0xf471ffff).rw(m_c355spr[2], FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w)).share("objram_3");
	map(0xf4720000, 0xf4720007).rw(m_c355spr[2], FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0xf4740000, 0xf474ffff).rw(m_palette[2], FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette_3");
	map(0xf4750000, 0xf475ffff).rw(m_palette[2], FUNC(palette_device::read16_ext), FUNC(palette_device::write16_ext)).share("palette_3_ext");
	map(0xf4760000, 0xf4760001).rw(FUNC(gal3_state::video_enable_r<2>), FUNC(gal3_state::video_enable_w<2>));
#else
	map(0xf4700000, 0xf47FFFFF).nopr();
#endif
}

void gal3_state::slv_cpu_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x40000000, 0x40000003).w(FUNC(gal3_state::dsp_reset_w));
	map(0x44000000, 0x44000003).portr("DSW_CPU_slv");
	map(0x44800000, 0x44800003).r(FUNC(gal3_state::led_slv_r)).w(FUNC(gal3_state::led_slv_w)); //LEDs
	map(0x48000000, 0x48000003).rw(FUNC(gal3_state::subcpu_readirq1), FUNC(gal3_state::subcpu_wipeirq1));
    map(0x50000000, 0x50000003).ram();//?? reads 16 bits from here
	map(0x54000000, 0x54000003).ram(); // Flashy something? TODO.
	map(0x60000000, 0x60007fff).mirror(0x10000).ram().share("share1");
	map(0x80000000, 0x8007ffff).ram(); //512K Local RAM

	// Video chain 1
	map(0xf1200000, 0xf120ffff).rw(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_w));
	map(0xf1400000, 0xf1400003).w(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::pointram_control_w));
	map(0xf1440000, 0xf1440003).rw(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::pointram_data_r), FUNC(namcos21_dsp_c67_device::pointram_data_w));
	map(0xf1440004, 0xf147ffff).nopw();
	map(0xf1480000, 0xf14807ff).rw(m_namcos21_dsp_c67[0], FUNC(namcos21_dsp_c67_device::namcos21_depthcue_r), FUNC(namcos21_dsp_c67_device::namcos21_depthcue_w));

	map(0xf1700000, 0xf171ffff).rw(m_c355spr[0], FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w)).share("objram_1");
	map(0xf1720000, 0xf1720007).rw(m_c355spr[0], FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0xf1740000, 0xf174ffff).rw(m_palette[0], FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette_1");
	map(0xf1750000, 0xf175ffff).rw(m_palette[0], FUNC(palette_device::read16_ext), FUNC(palette_device::write16_ext)).share("palette_1_ext");
	map(0xf1760000, 0xf1760003).rw(FUNC(gal3_state::video_enable_r1), FUNC(gal3_state::video_enable_w<0>));


	// Video chain 2
	map(0xf2200000, 0xf220ffff).rw(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::dspram16_r), FUNC(namcos21_dsp_c67_device::dspram16_w));
	map(0xf2400000, 0xf2400003).w(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::pointram_control_w));
	map(0xf2440000, 0xf2440003).rw(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::pointram_data_r), FUNC(namcos21_dsp_c67_device::pointram_data_w));
	map(0xf2440004, 0xf247ffff).nopw();
	map(0xf2480000, 0xf24807ff).rw(m_namcos21_dsp_c67[1], FUNC(namcos21_dsp_c67_device::namcos21_depthcue_r), FUNC(namcos21_dsp_c67_device::namcos21_depthcue_w));

	map(0xf2700000, 0xf271ffff).rw(m_c355spr[1], FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w)).share("objram_2");
	map(0xf2720000, 0xf2720007).rw(m_c355spr[1], FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0xf2740000, 0xf274ffff).rw(m_palette[1], FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette_2");
	map(0xf2750000, 0xf275ffff).rw(m_palette[1], FUNC(palette_device::read16_ext), FUNC(palette_device::write16_ext)).share("palette_2_ext");
	map(0xf2760000, 0xf2760003).rw(FUNC(gal3_state::video_enable_r2), FUNC(gal3_state::video_enable_w<1>));

}

/*
	RSO ISRs (dragoon)
	IRQ0 : 13DA - C148
	IRQ1 : 105E - not used/trap
	IRQ2 : 1064 - not used/trap
	IRQ3 : 106A - C139 RX interrupt?
	IRQ4 : 1226 - Laserdisc
	IRQ5 : 13B0 - C148 (wipe 1de000)
	IRQ6 : 13C2 - not used/trap
	IRQ7 : 13C6 - not used/trap

*/
void gal3_state::rso_cpu_map(address_map &map)
{
	
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); //64K working RAM

//  map(0x180000, 0x183fff).ram(); // NVRAM?

	map(0x1c0000, 0x1fffff).m(m_rso_intc, FUNC(namco_c148_device::map));
	map(0x200000, 0x200001).portr("DSW_RSO").nopw(); // LEDs and DIP switches
	map(0x240000, 0x244001).rw(FUNC(gal3_state::rso_read_c139_irqs), FUNC(gal3_state::rso_write_c139_irqs)); //C139 IRQ status 
	map(0x280000, 0x280001).rw(FUNC(gal3_state::rso_uart_irq_flag_read), FUNC(gal3_state::rso_uart_irq_write)); // Dragoon looks here for some UART ACK? Fill with 0xFFFF. BREAKS ZOLGEAR!
	
	map(0x2c0000, 0x2c0001).rw(FUNC(gal3_state::psn_iack_r), FUNC(gal3_state::psn_iack_w)); // C139 20P INT ACK
	map(0x2c0800, 0x2c0801).rw(FUNC(gal3_state::psn_iack_r), FUNC(gal3_state::psn_iack_w)); // C139 20K INT ACK
	map(0x2c1000, 0x2c1001).rw(FUNC(gal3_state::psn_iack_r), FUNC(gal3_state::psn_iack_w)); // C139 18P INT ACK
	map(0x2c1800, 0x2c1801).ram(); // C139 18K INT ACK
	map(0x2c2000, 0x2c2001).ram(); // C139 15P INT ACK
	map(0x2c2800, 0x2c2801).ram(); // C139 15K INT ACK
	map(0x2c3000, 0x2c3001).ram(); // C139 11P INT ACK
	map(0x2c3800, 0x2c3801).ram(); // C139 11K INT ACK
	map(0x2c4000, 0x2c4001).ram(); // C139 11S INT ACK
	
	map(0x300000, 0x300fff).ram().share("rso_shared_ram"); //TODO: is the upper half for another RSO? Main writes, but we don't read

	map(0x400000, 0x40001f).rw(m_uart[0], FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00FF); // MC68681 7L
	map(0x480000, 0x48001f).ram(); // MC68681 8L
	map(0x500000, 0x500017).ram(); // MC68681 4L
	map(0x580000, 0x580017).ram(); // MC68681 6L
	map(0x600000, 0x600017).ram(); // MC68681 1L
	map(0x680000, 0x680017).ram(); // MC68681 3L

	map(0x800000, 0x80000f).m(m_sci_rso[0], FUNC(namco_c139_local_device::regs_map));  // C139 20P
	map(0x840000, 0x843fff).m(m_sci_rso[0], FUNC(namco_c139_local_device::data_map));

	map(0x880000, 0x88000f).m(m_sci_rso[1], FUNC(namco_c139_local_device::regs_map));  // C139 20K
	map(0x8c0000, 0x8c3fff).m(m_sci_rso[1], FUNC(namco_c139_local_device::data_map));

	map(0x900000, 0x90000f).m(m_sci_rso[2], FUNC(namco_c139_local_device::regs_map));  // C139 18P
	map(0x940000, 0x943fff).m(m_sci_rso[2], FUNC(namco_c139_local_device::data_map));


	map(0x980000, 0x98000f).ram(); // C139 18K
	map(0x9c0000, 0x9c3fff).ram();

	map(0xa00000, 0xa0000f).ram(); // C139 15P
	map(0xa40000, 0xa43fff).ram();

	map(0xa80000, 0xa8000f).ram(); // C139 15K
	map(0xac0000, 0xac3fff).ram();

	map(0xb00000, 0xb0000f).ram(); // C139 11P
	map(0xb40000, 0xb43fff).ram();

	map(0xb80000, 0xb8000f).ram(); // C139 11K
	map(0xbc0000, 0xbc3fff).ram();

	map(0xc00000, 0xc0000f).m(m_sci_rso[3], FUNC(namco_c139_local_device::regs_map));  	// C139 11S - sound
	map(0xc40000, 0xc43fff).m(m_sci_rso[3], FUNC(namco_c139_local_device::data_map));

}

/* Sound CPU ISRs
	0: 688  - trap (LED at C0000?)
	1: 81A - C139 RX
	2: 496 - ISR2 (vblank?)
	3: 694 -trap
	4: 670 - C140 G?
	5: 694 - trap

*/
void gal3_state::sound_cpu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom(); 
	map(0x080000, 0x08ffff).ram();
    map(0x0c0000, 0x0cffff).ram(); //00, 20, 30, 40, 50  LED?
//	map(0x0c0000, 0x0cffff).r(FUNC(gal3_state::sound_dip));

	map(0x0c0020, 0x0c0021).rw(FUNC(gal3_state::sound_sci_irq_clear_r), FUNC(gal3_state::sound_sci_irq_clear_w)); //RSO PCB
	map(0x0c0030, 0x0c0031).rw(FUNC(gal3_state::sound_irq2_clear_r), FUNC(gal3_state::sound_irq2_clear_w)); //RSO PCB


    map(0x100000, 0x10000f).m(m_sci_sound, FUNC(namco_c139_local_device::regs_map));
	map(0x110000, 0x113fff).m(m_sci_sound, FUNC(namco_c139_local_device::data_map));
//	map(0x110000, 0x113fff).ram();


	map(0x120000, 0x120003).ram(); //LED?
 
	//Only first, second and fifth C140 appear to be used across both games and only 5th has an ISR?
	map(0x200000, 0x2001ff).mirror(0x0e00).rw(m_c140_16a, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w));
	
	map(0x201000, 0x2011ff).mirror(0x0e00).rw(m_c140_16c, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w));
	
	map(0x202000, 0x202fff).ram(); // C140d
	map(0x203000, 0x203fff).ram(); //C140f
	
	map(0x204000, 0x2041ff).mirror(0x0e00).rw(m_c140_16g, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w));
}


/*
	PSN IRQs
	IRQ0: 19FE -> 20E4
	IRQ1: 23A0 -> 19FE (above)
	IRQ2: 232E //VBLANK
	IRQ3: 2384 //C139
	IRQ4: 2392 // Prod something at 0x900000 - sound?

*/
void gal3_state::psn_cpu_common(address_map& map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram();



	map(0x800042, 0x800043).ram();//portw("COUNTERS_PSN");
	map(0x800046, 0x800047).ram();//portw("GREEN_LED");
#if 0

	//map(0x800006, 0x800007).  vblank_ack
	map(0x800010, 0x800011).portw("LAMP_PSN");
	map(0x800014, 0x800014).portw("CONTROL_PSN");
	map(0x800020, 0x80002f).ram(); //IRQ 2 - ADC?

	map(0x800040, 0x800041).portr("BUTTONS_PSN");
	map(0x800042, 0x800043).portw("COUNTERS_PSN");
	map(0x800046, 0x800047).portw("GREEN_LED");
#endif
	map(0x900000, 0x900001).r(FUNC(gal3_state::psn_acia_r));
	map(0x900002, 0x900023).ram(); //IRQ4 touches this

	map(0x900040, 0x90005F).ram(); // sound


	map(0x900060, 0x90006F).ram(); // 

}


void gal3_state::psn_cpu_map_0(address_map &map)
{
	psn_cpu_common(map);
	map(0x800000, 0x800001).portr("INP_PSN0");
	map(0x800002, 0x800003).ram();//portr("DIP_PSN0");
	map(0x800004, 0x800039).ram();
	map(0x800040, 0x800041).portr("BUTTONS_PSN0");

	map(0x800014, 0x800015).w(FUNC(gal3_state::psn0_adc_mux_w));
	map(0x80004a, 0x80004b).rw(FUNC(gal3_state::psn0_sci_read), FUNC(gal3_state::psn0_sci_ack));
	map(0x900060, 0x900061).r(FUNC(gal3_state::psn0_adc_read));
	map(0x900080, 0x90008f).m(m_sci_psn[0], FUNC(namco_c139_local_device::regs_map));  // C139 18P
	map(0xb00000, 0xb03fff).m(m_sci_psn[0], FUNC(namco_c139_local_device::data_map));
}

void gal3_state::psn_cpu_map_1(address_map& map)
{
	psn_cpu_common(map);
	map(0x800000, 0x800001).portr("INP_PSN1");
	map(0x800002, 0x800003).ram();//portr("DIP_PSN0");
	map(0x800004, 0x800039).ram();
	map(0x800040, 0x800041).portr("BUTTONS_PSN1");
	map(0x800014, 0x800015).w(FUNC(gal3_state::psn1_adc_mux_w));
	map(0x80004a, 0x80004b).rw(FUNC(gal3_state::psn1_sci_read), FUNC(gal3_state::psn1_sci_ack));
	map(0x900060, 0x900061).r(FUNC(gal3_state::psn1_adc_read));
	map(0x900080, 0x90008f).m(m_sci_psn[1], FUNC(namco_c139_local_device::regs_map));
	map(0xb00000, 0xb03fff).m(m_sci_psn[1], FUNC(namco_c139_local_device::data_map));
}

void gal3_state::psn_cpu_map_2(address_map& map)
{
	psn_cpu_common(map);
	map(0x800000, 0x800001).portr("INP_PSN2");
	map(0x800002, 0x800003).ram();//portr("DIP_PSN0");
	map(0x800004, 0x800039).ram();
	map(0x800040, 0x800041).portr("BUTTONS_PSN2");
	map(0x800014, 0x800015).w(FUNC(gal3_state::psn2_adc_mux_w));
	map(0x80004a, 0x80004b).rw(FUNC(gal3_state::psn2_sci_read), FUNC(gal3_state::psn2_sci_ack));
	map(0x900060, 0x900061).r(FUNC(gal3_state::psn2_adc_read));
	map(0x900080, 0x90008f).m(m_sci_psn[2], FUNC(namco_c139_local_device::regs_map));  
	map(0xb00000, 0xb03fff).m(m_sci_psn[2], FUNC(namco_c139_local_device::data_map));
}



void gal3_state::c140a_map(address_map& map)
{

	map.global_mask(0x7fffff); 
	map(0x000000, 0x7fffff).r(FUNC(gal3_state::c140a_rom_r));


}

void gal3_state::c140c_map(address_map& map)
{
	map.global_mask(0x7fffff); 
	map(0x000000, 0x7fffff).r(FUNC(gal3_state::c140c_rom_r));

}



void gal3_state::c140g_map(address_map& map)
{

	map.global_mask(0x7fffff); 
	map(0x000000, 0x7fffff).r(FUNC(gal3_state::c140g_rom_r));

}

static INPUT_PORTS_START( gal3 )
	PORT_START("DSW_CPU_mst")   //
	PORT_DIPNAME( 0x00010000, 0x00010000, "DIPSW 1-1, test")
	PORT_DIPSETTING(      0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "DIPSW 1-2, debug")
	PORT_DIPSETTING(      0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "DIPSW 1-3")
	PORT_DIPSETTING(      0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "DIPSW 1-4")
	PORT_DIPSETTING(      0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "DIPSW 1-5")
	PORT_DIPSETTING(      0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "DIPSW 1-6")
	PORT_DIPSETTING(      0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "DIPSW 1-7")
	PORT_DIPSETTING(      0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "DIPSW 1-8")
	PORT_DIPSETTING(      0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x01000000, 0x01000000, "DIPSW 2-1, freeplay")
	PORT_DIPSETTING(      0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x02000000, "DIPSW 2-2, rainbow!")
	PORT_DIPSETTING(      0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "DIPSW 2-3")
	PORT_DIPSETTING(      0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "DIPSW 2-4")
	PORT_DIPSETTING(      0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x00000000, "DIPSW 2-5")  //on pour zolgear?
	PORT_DIPSETTING(      0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x00000000, "DIPSW 2-6")  //on pour zolgear?
	PORT_DIPSETTING(      0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80000000, 0x80000000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )

	PORT_START("DSW_CPU_slv")   //
	PORT_DIPNAME( 0x00010000, 0x00010000, "CPU_slv_DIPSW 3-1")
	PORT_DIPSETTING(      0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, "DIPSW 3-2")
	PORT_DIPSETTING(      0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, "DIPSW 3-3")
	PORT_DIPSETTING(      0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, "DIPSW 3-4")
	PORT_DIPSETTING(      0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, "DIPSW 3-5")
	PORT_DIPSETTING(      0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, "DIPSW 3-6")
	PORT_DIPSETTING(      0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, "DIPSW 3-7")
	PORT_DIPSETTING(      0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, "DIPSW 3-8")
	PORT_DIPSETTING(      0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x01000000, 0x00000000, "DIPSW 4-1")  //on
	PORT_DIPSETTING(      0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x00000000, "DIPSW 4-2")  //on
	PORT_DIPSETTING(      0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, "DIPSW 4-3")
	PORT_DIPSETTING(      0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, "DIPSW 4-4")
	PORT_DIPSETTING(      0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x00000000, "DIPSW 4-5")
	PORT_DIPSETTING(      0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x00000000, "DIPSW 4-6")
	PORT_DIPSETTING(      0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x40000000, "DIPSW 4-7")
	PORT_DIPSETTING(      0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80000000, 0x80000000, "DIPSW 4-8")
	PORT_DIPSETTING(      0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00000000, DEF_STR( On ) )

	PORT_START("DSW_RSO")
	PORT_DIPNAME( 0x0001, 0x0001, "RSO_DIP 1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "RSO_DIP 2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "RSO_DIP 3" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "RSO_DIP 4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "RSO_DIP 5" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "RSO_DIP 6" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "RSO_DIP 7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "RSO_DIP 8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


		/*
		.... .... .... ...x - Start button (P60_B23)
.... .... .... ..x. - Skip button (P60_A23)
.... .... .... .x.. - Reset button (P60_B24)
.... .... .... x... - ? (P60_A24)
.... .... ...x .... - 'Mode' PCB switch - also wired to (P60_B25)
.... .... ..x. .... - 'Test' PCB button - also wired to (P60_A25)
.... .... .x.. .... - 'Mood 1' (P60_B26)
.... .... x... .... - 'Mood 2' (P60_A26)
*/

	PORT_START("INP_PSN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_TILT) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNUSED)


		/*
		800040 - Buttons(R)
		Default read value is FFFF on 'bad' board

		.... .... .... ...x - CSR - Right Coin SW
		.... .... .... ..x. - SSR - Right Service SW
		.... .... .....x.. - T1R - Right Trigger 1 ?
		.... .... ....x... - T2R - Right Trigger 2
		.... ...x .... .... - CSL - Left Coin SW
		.... ..x. .... .... - SSL - Left Service SW
		.....x.. .... .... - T1L - Left Trigger 1 ?
		....x... .... .... - T2L - Left Trigger 2 ?
		*/


	PORT_START("BUTTONS_PSN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN2) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SERVICE3) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_COIN1) PORT_PLAYER(1)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_SERVICE4) PORT_PLAYER(1)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)


	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ADC_psn0.0")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player1 Left/Right") PORT_PLAYER(1)

	PORT_START("ADC_psn0.1")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player1 Up/Down") PORT_PLAYER(1)

	PORT_START("ADC_psn0.2")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player2 Left/Right") PORT_PLAYER(2)

	PORT_START("ADC_psn0.3")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player2 Up/Down") PORT_PLAYER(2)




	PORT_START("INP_PSN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_TILT) PORT_PLAYER(3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_PLAYER(3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_PLAYER(3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNUSED)


	PORT_START("BUTTONS_PSN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN2) PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SERVICE3) PORT_PLAYER(3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_COIN1) PORT_PLAYER(3)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_SERVICE4) PORT_PLAYER(3)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(3)


	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ADC_psn1.0")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player3 Left/Right") PORT_PLAYER(3)

	PORT_START("ADC_psn1.1")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player3 Up/Down") PORT_PLAYER(3)

	PORT_START("ADC_psn1.2")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player4 Left/Right") PORT_PLAYER(4)

	PORT_START("ADC_psn1.3")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player4 Up/Down") PORT_PLAYER(4)



	PORT_START("INP_PSN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_TILT) PORT_PLAYER(5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_PLAYER(5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_PLAYER(5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNUSED)



	PORT_START("BUTTONS_PSN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN2) PORT_PLAYER(5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SERVICE3) PORT_PLAYER(5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_COIN1) PORT_PLAYER(5)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_SERVICE4) PORT_PLAYER(5)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(5)
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(5)

	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ADC_psn2.0")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player6 Left/Right") PORT_PLAYER(6)

	PORT_START("ADC_psn2.1")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player6 Up/Down") PORT_PLAYER(6)

	PORT_START("ADC_psn2.2")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player5 Left/Right") PORT_PLAYER(5)

	PORT_START("ADC_psn2.3")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_MINMAX(0x000, 0xfFF) PORT_SENSITIVITY(25) PORT_KEYDELTA(160) PORT_CENTERDELTA(160) PORT_NAME("Player5 Up/Down") PORT_PLAYER(5)



INPUT_PORTS_END


void gal3_state::gal3(machine_config& config)
{
	config.set_maximum_quantum(attotime::from_hz(60 * 8000)); /* 8000 CPU slices per frame */


	SCREEN(config, m_screen_l);
	m_screen_l->set_refresh_hz(60);
	m_screen_l->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen_l->set_raw(1.0 * 49.152_MHz_XTAL / 4 * 2, 768, 0, 496, 264 * 2, 0, 480);
	m_screen_l->set_video_attributes(VIDEO_SELF_RENDER);
	m_screen_l->set_screen_update("laserdisc_1", FUNC(laserdisc_device::screen_update));
	m_screen_l->set_default_position(1.0, 0.042000, 1.0, 0.0);

	SCREEN(config, m_screen_r);
	m_screen_r->set_refresh_hz(60);
	m_screen_r->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen_r->set_raw(1.0 * 49.152_MHz_XTAL / 4 * 2, 768, 0, 496, 264 * 2, 0, 480);
	m_screen_r->set_video_attributes(VIDEO_SELF_RENDER);
	m_screen_r->set_screen_update("laserdisc_2", FUNC(laserdisc_device::screen_update));
	

#ifdef DEBUG_SCREEN
	// video chain 3 - DEBUG
	SCREEN(config, m_debug_screen);

	m_debug_screen->set_refresh_hz(60);
	m_debug_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_debug_screen->set_raw(0.99 * 49.152_MHz_XTAL / 4 * 2, 768, 0, 496, 264 * 2, 0, 480);
	m_debug_screen->set_screen_update(FUNC(gal3_state::screen_update_debug));
	m_debug_screen->set_palette(m_palette[2]);

	PALETTE(config, m_palette[2]).set_format(palette_device::xBRG_888, 0x10000 / 2);
	m_palette[2]->set_membits(16);
#endif


	M68020(config, m_maincpu, 49152000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gal3_state::mst_cpu_map);
	m_maincpu->set_vblank_int("screen_l", FUNC(gal3_state::irq1_line_assert));
	TIMER(config, "scantimer").configure_scanline(FUNC(gal3_state::screen_scanline), "screen_l", 0, 1);

	M68020(config, m_subcpu, 49152000 / 2);
	m_subcpu->set_addrmap(AS_PROGRAM, &gal3_state::slv_cpu_map);
	m_subcpu->set_vblank_int("screen_l", FUNC(gal3_state::irq1_line_assert));

	M68000(config, m_rso_cpu, 49152000 / 4);
	m_rso_cpu->set_addrmap(AS_PROGRAM, &gal3_state::rso_cpu_map);
	//m_rso_cpu->set_vblank_int("screen_l", FUNC(gal3_state::irq5_line_hold));  /// handled via C148 IC

	M68000(config, m_sound_cpu, 49152000 / 4);
	m_sound_cpu->set_addrmap(AS_PROGRAM, &gal3_state::sound_cpu_map);
	m_sound_cpu->set_vblank_int("screen_l", FUNC(gal3_state::irq2_line_hold));


	//oof, a bit of a mess with that vblank to psn. Also what's going on with RSO'
	M68000(config, m_psn_cpu[0], 16384000 / 2);
	m_psn_cpu[0]->set_addrmap(AS_PROGRAM, &gal3_state::psn_cpu_map_0);
	m_psn_cpu[0]->set_vblank_int("screen_l", FUNC(gal3_state::irq2_line_assert));
	M68000(config, m_psn_cpu[1], 16384000 / 2);
	m_psn_cpu[1]->set_addrmap(AS_PROGRAM, &gal3_state::psn_cpu_map_1);
	m_psn_cpu[1]->set_vblank_int("screen_l", FUNC(gal3_state::irq2_line_assert));
	M68000(config, m_psn_cpu[2], 16384000 / 2);
	m_psn_cpu[2]->set_addrmap(AS_PROGRAM, &gal3_state::psn_cpu_map_2);
	m_psn_cpu[2]->set_vblank_int("screen_l", FUNC(gal3_state::irq2_line_assert));

	
	NVRAM(config, "nvmem", nvram_device::DEFAULT_ALL_0);

	// Palette for left screen 2D/3D
	PALETTE(config, m_palette[0]).set_format(palette_device::xBRG_888, 0x10000 / 2);
	m_palette[0]->set_membits(16);

	// Palette for right screen 2D/3D
	PALETTE(config, m_palette[1]).set_format(palette_device::xBRG_888, 0x10000 / 2);
	m_palette[1]->set_membits(16);

	NAMCO_C355SPR(config, m_c355spr[0], 0);
	m_c355spr[0]->set_screen(m_screen_l);
	m_c355spr[0]->set_palette(m_palette[0]);
	m_c355spr[0]->set_scroll_offsets(0x10, 0x19);
	m_c355spr[0]->set_color_base(0x0); // TODO : verify palette offset
	m_c355spr[0]->set_external_prifill(true);

	NAMCOS21_3D(config, m_namcos21_3d[0], 0);
	//m_namcos21_3d[0]->set_zz_shift_mult(11, 0x200);
	m_namcos21_3d[0]->set_depth_reverse(false);
	m_namcos21_3d[0]->set_num_palettes(0x20);
//	m_namcos21_3d[0]->set_framebuffer_size(496, 480);
	m_namcos21_3d[0]->set_framebuffer_size(480, 480);


	NAMCOS21_DSP_C67(config, m_namcos21_dsp_c67[0], 0);
	m_namcos21_dsp_c67[0]->set_renderer_tag("namcos21_3d_1");




	NAMCO_C355SPR(config, m_c355spr[1], 0);
	m_c355spr[1]->set_screen(m_screen_l);
	m_c355spr[1]->set_palette(m_palette[1]);
	m_c355spr[1]->set_scroll_offsets(0x10, 0x19);
	m_c355spr[1]->set_color_base(0x0); // TODO : verify palette offset
	m_c355spr[1]->set_external_prifill(true);


	NAMCOS21_3D(config, m_namcos21_3d[1], 0);
	//m_namcos21_3d[1]->set_zz_shift_mult(11, 0x200);
	m_namcos21_3d[1]->set_depth_reverse(false);
	m_namcos21_3d[1]->set_num_palettes(0x10);
//	m_namcos21_3d[1]->set_framebuffer_size(496 , 480);
	m_namcos21_3d[1]->set_framebuffer_size(480, 480);

	NAMCOS21_DSP_C67(config, m_namcos21_dsp_c67[1], 0);
	m_namcos21_dsp_c67[1]->set_renderer_tag("namcos21_3d_2");

	m_namcos21_dsp_c67[0]->set_gametype(namcos21_dsp_c67_device::NAMCOS21_GALAXIAN3);
	m_namcos21_dsp_c67[1]->set_gametype(namcos21_dsp_c67_device::NAMCOS21_GALAXIAN3);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	//  5 C140s in sound board. Only 3 in use [0,3,5] at 16A,16C,16G
	C140(config, m_c140_16a, 49152000 / 2304);
	m_c140_16a->set_addrmap(0, &gal3_state::c140a_map);
	m_c140_16a->int1_callback().set_inputline(m_sound_cpu, 4);
	m_c140_16a->add_route(0, "lspeaker", 0.50);

	
	C140(config, m_c140_16c, 49152000 / 2304);
	m_c140_16c->set_addrmap(0, &gal3_state::c140c_map);   
	m_c140_16c->int1_callback().set_inputline(m_sound_cpu, 4);
	m_c140_16c->add_route(1, "rspeaker", 0.50);


	C140(config, m_c140_16g, 49152000 / 2304);
	m_c140_16g->set_addrmap(0, &gal3_state::c140g_map);
	m_c140_16g->int1_callback().set_inputline(m_sound_cpu, 4);
	m_c140_16g->add_route(0, "lspeaker", 0.50);
	m_c140_16g->add_route(1, "rspeaker", 0.50);


	// MC69681 UARTs on RSO
	// Just two  used for laserdisc on Theatre 6 (both in single channel mode)

	MC68681(config, m_uart[0], XTAL(3'686'400));
	MC68681(config, m_uart[1], XTAL(3'686'400));
	MC68681(config, m_uart[2], XTAL(3'686'400));
	MC68681(config, m_uart[3], XTAL(3'686'400));
	MC68681(config, m_uart[4], XTAL(3'686'400));
	MC68681(config, m_uart[5], XTAL(3'686'400));
	
	m_uart[0]->irq_cb().set_inputline(m_rso_cpu, 4);
	/* Laserdisc Left */
	m_uart[0]->a_tx_cb().set(m_laserdisc[0], FUNC(pioneer_ldv8000hle_device::rx_w));

	PIONEER_LDV8000HLE(config, m_laserdisc[0], 0);
	m_laserdisc[0]->set_overlay(496, 528 , FUNC(gal3_state::screen_update_ld1));

	m_laserdisc[0]->add_route(0, "lspeaker", 0.4);
	m_laserdisc[0]->add_route(1, "rspeaker", 0.4);
	m_laserdisc[0]->set_overlay_scale(1.0f, 1.1f);
	m_laserdisc[0]->set_overlay_position(-0.01f, 0.05f);

	m_laserdisc[0]->set_screen(m_screen_l);
	m_laserdisc[0]->serial_tx().set(m_uart[0], FUNC(mc68681_device::rx_a_w));


	/* Laserdisc Right */
	m_uart[0]->b_tx_cb().set(m_laserdisc[1], FUNC(pioneer_ldv8000hle_device::rx_w));

	PIONEER_LDV8000HLE(config, m_laserdisc[1], 0);
	m_laserdisc[1]->set_overlay(496, 528, FUNC(gal3_state::screen_update_ld2));

	m_laserdisc[1]->set_overlay_scale(1.0f, 1.1f);
	m_laserdisc[1]->set_overlay_position(0.000f, 0.05f);

	m_laserdisc[1]->add_route(0, "lspeaker", 0.4);
	m_laserdisc[1]->add_route(1, "rspeaker", 0.4);
	m_laserdisc[1]->set_screen(m_screen_r);
	m_laserdisc[1]->serial_tx().set(m_uart[0], FUNC(mc68681_device::rx_b_w));

	//C139 for remote boards from RS PCB

	NAMCO_C139_LOCAL(config, m_sci_rso[0], 0U);
	NAMCO_C139_LOCAL(config, m_sci_rso[1], 0U);
	NAMCO_C139_LOCAL(config, m_sci_rso[2], 0U);

	NAMCO_C139_LOCAL(config, m_sci_psn[0], 0U);
	NAMCO_C139_LOCAL(config, m_sci_psn[1], 0U);
	NAMCO_C139_LOCAL(config, m_sci_psn[2], 0U);

	NAMCO_C139_LOCAL(config, m_sci_rso[3], 0U); // Sound
	NAMCO_C139_LOCAL(config, m_sci_sound, 0U);


	
	m_sci_rso[0]->irq_cb().set(FUNC(gal3_state::sci_to_rso_irq_trig_1));
	m_sci_rso[1]->irq_cb().set(FUNC(gal3_state::sci_to_rso_irq_trig_2));
	m_sci_rso[2]->irq_cb().set(FUNC(gal3_state::sci_to_rso_irq_trig_3));
	m_sci_psn[0]->irq_cb().set(FUNC(gal3_state::psn1_sci_rx_irq));
	m_sci_psn[1]->irq_cb().set(FUNC(gal3_state::psn2_sci_rx_irq));
	m_sci_psn[2]->irq_cb().set(FUNC(gal3_state::psn3_sci_rx_irq));

	m_sci_sound->irq_cb().set(FUNC(gal3_state::sound_sci_rx_irq));

	m_sci_rso[0]->m_linkid = 1;
	m_sci_rso[1]->m_linkid = 2;
	m_sci_rso[2]->m_linkid = 3;
	m_sci_rso[3]->m_linkid = 4; // Sound

	m_sci_psn[0]->m_linkid = 0x10;
	m_sci_psn[1]->m_linkid = 0x20;
	m_sci_psn[2]->m_linkid = 0x30;

	m_sci_sound->m_linkid = 0x40;




	uint8_t* buff_ptr = NULL;
	uint16_t* count_ptr = NULL;
	m_sci_rso[0]->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_psn[0]->set_rx_ptr(buff_ptr, count_ptr);

	m_sci_psn[0]->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_rso[0]->set_rx_ptr(buff_ptr, count_ptr); 
	
	m_sci_rso[1]->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_psn[1]->set_rx_ptr(buff_ptr, count_ptr);

	m_sci_psn[1]->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_rso[1]->set_rx_ptr(buff_ptr, count_ptr);

	m_sci_rso[2]->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_psn[2]->set_rx_ptr(buff_ptr, count_ptr);

	m_sci_psn[2]->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_rso[2]->set_rx_ptr(buff_ptr, count_ptr);
	
	/* RSO to sound */
	m_sci_rso[3]->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_sound->set_rx_ptr(buff_ptr, count_ptr);
	m_sci_sound->get_rx_buffer(&buff_ptr, &count_ptr);
	m_sci_rso[3]->set_rx_ptr(buff_ptr, count_ptr);

	
	NAMCO_C148(config, m_rso_intc, 0, m_rso_cpu, true);
	//m_rso_intc->link_c148_device(m_slave_intc);
	//m_rso_intc->out_ext1_callback().set(FUNC(namcos21_state::sound_reset_w));
	//m_rso_intc->out_ext2_callback().set(FUNC(namcos21_state::system_reset_w));

#ifdef DEBUG_SCREEN
	NAMCO_C355SPR(config, m_c355spr[2], 0);
	m_c355spr[2]->set_screen(m_debug_screen);
	m_c355spr[2]->set_palette(m_palette[2]);
	m_c355spr[2]->set_scroll_offsets(0x26, 0x19);
	//m_c355spr[2]->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate());
	//m_c355spr[2]->set_palxor(0); // reverse mapping
	m_c355spr[2]->set_color_base(0x0); // TODO : verify palette offset
	m_c355spr[2]->set_external_prifill(true);
#endif


}
/*

**************************************************************************************************
MASTER CPU PCB
**************************************************************************************************

PCB markings:
screened : 8623961202 CPU020 
Etched   : (8623963202)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-MST-PRG0E  6B  27c4001     PRG0E
GLC1-MST-PRG1E  10B 27c4001     PRG1E
GLC1-MST-PRG2E  14B 27c4001     PRG2E
GLC1-MST-PRG3E  18B 27c4001     PRG3E

**************************************************************************************************
SLAVE CPU PCB
**************************************************************************************************

PCB markings:
screened : 8623961202 CPU020
Etched   : (8623963202)


label       loc.    Device      Filename
--------------------------------------------------------
GLC-SLV-PRG0    6B  27c010A     PRG0.bin
GLC-SLV-PRG1    10B 27c010A     PRG1.bin
GLC-SLV-PRG2    14B 27c010A     PRG2.bin
GLC-SLV-PRG3    18B 27c010A     PRG3.bin

**************************************************************************************************
DSP PCB
**************************************************************************************************


PCB markings:
screened : 8623961703 DSP
Etched   : (8623963703) TSK-A


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-DSP-PTOH   2F  27c040      PTOH.BIN
GLC1-DSP-PTOU   2K  27c040      PTOU.BIN
GLC1-DSP-PTOL   2N  27c040      PTOL.BIN

**************************************************************************************************
OBJ PCB
**************************************************************************************************

PCB markings:
screened : 8623962002
Etched   : (8623964002)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-OBJ-OBJ0   9T  27c040      OBJ0.BIN
GLC1-OBJ-OBJ2   9W  27c4000     OBJ2.BIN
GLC1-OBJ-OBJ1   9Y  27c040      OBJ1.BIN
GLC1-OBJ-OBJ3   9Z  27c4000     OBJ0.BIN

**************************************************************************************************
PSN PCB
**************************************************************************************************


PCB markings:
screened : V1079603
Etched   : (V1079703)


label       loc.    Device      Filename
--------------------------------------------------------
GLC-PSN-VOL IC100   27c010A     VOL.bin
GLC-PSN-PRG0B   IC22    27c010A     PRG0B.bin
GLC-PSN-PRG0B   IC23    27c010A     PRG1B.bin


PCB markings:
screened : V107960701
Etched   : (V107970701) TSK-A

**************************************************************************************************
RS PCB
**************************************************************************************************

label       loc.    Device      Filename
--------------------------------------------------------
GLC-RS-PRGLB    18B 27c010      PRGLB.BIN
GLC-RS-PRGUB    19B 27c010      PRGUB.BIN

**************************************************************************************************
SOUND PCB
**************************************************************************************************


PCB markings:
screened : V107965101
Etched   : (V107975101)


label       loc.    Device      Filename
--------------------------------------------------------
GLC1-SND-VOI0   13A 27c040      VOI0.BIN
GLC1-SND-VOI2   13C 27c040      VOI2.BIN
GLC1-SND-VOI8   10G 27c040      VOI8.BIN
GLC1-SND-VOI9   11/12G  27c040      VOI9.BIN
GLC1-SND-VOI10  13G 27c040      VOI10.BIN
GLC1-SND-VOI11  14G 27c040      VOI11.BIN

GLC1-SND-PRG0   1H  27c1000     PRG0.BIN
GLC1-SND-PRG1   2H  27c1000     PRG1.BIN
GLC1-SND-DATA1  4/5H    27c1000     DATA1.BIN


*/

ROM_START( gal3 )
	/********* CPU-MST board x1 *********/
	ROM_REGION( 0x200000, "main_cpu", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "glc1-mst-prg0e.6b", 0x00003, 0x80000, CRC(5deccd72) SHA1(8d50779221538cc171469a691fabb17b62a8e664) )
	ROM_LOAD32_BYTE( "glc1-mst-prg1e.10b", 0x00002, 0x80000, CRC(b6144e3b) SHA1(33f63d881e7012db7f971b074bc5f876a66198b7) )
	ROM_LOAD32_BYTE( "glc1-mst-prg2e.14b", 0x00001, 0x80000, CRC(13381084) SHA1(486c1e136e6594ba68858e40246c5fb9bef1c0d2) )
	ROM_LOAD32_BYTE( "glc1-mst-prg3e.18b", 0x00000, 0x80000, CRC(7917584a) SHA1(ec22de8a3751099d37e14cd05c736c33baa1ee1d) )

	/********* CPU-SLV board x1 *********/
	ROM_REGION( 0x080000, "slv_cpu", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "glc-slv-prg0.6b",  0x00003, 0x20000, CRC(75b2341a) SHA1(73616f5633f583b9ebfba2380fde3e7b08743e9f) )
	ROM_LOAD32_BYTE( "glc-slv-prg1.10b", 0x00002, 0x20000, CRC(f37ba6c0) SHA1(f8ee29ee4d341bfd6595e92c090865b8e5d9578c) )
	ROM_LOAD32_BYTE( "glc-slv-prg2.14b", 0x00001, 0x20000, CRC(c38a933e) SHA1(96c85db607d8527e927ef23fc53324172ddb861a) )
	ROM_LOAD32_BYTE( "glc-slv-prg3.18b", 0x00000, 0x20000, CRC(deae86d2) SHA1(1898955423b8da585b6319406566aad02db20d64) )

	/********* DSP board x2 *********/
	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67_1:point24", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )  /* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )  /* least significant */

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67_2:point24", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "glc1-dsp-ptoh.2f", 0x000001, 0x80000, CRC(b4213c83) SHA1(9d036b73149656fdc13eed38946a70f532bff3f1) )  /* most significant */
	ROM_LOAD32_BYTE( "glc1-dsp-ptou.2k", 0x000002, 0x80000, CRC(14877cef) SHA1(5ebdccd6db837ceb9473bd219eb211431944cbf0) )
	ROM_LOAD32_BYTE( "glc1-dsp-ptol.2n", 0x000003, 0x80000, CRC(b318534a) SHA1(6fcf2ead6dd0d5a6f22438520588ba4e33ca39a8) )  /* least significant */

	/********* OBJ board x2 *********/
	ROM_REGION( 0x200000, "c355spr_1", 0 )
	ROM_LOAD32_BYTE( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD32_BYTE( "glc1-obj-obj1.9w", 0x000001, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD32_BYTE( "glc1-obj-obj2.9y", 0x000002, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD32_BYTE( "glc1-obj-obj3.9z", 0x000003, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	ROM_REGION( 0x200000, "c355spr_2", 0 )
	ROM_LOAD32_BYTE( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD32_BYTE( "glc1-obj-obj1.9w", 0x000001, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD32_BYTE( "glc1-obj-obj2.9y", 0x000002, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD32_BYTE( "glc1-obj-obj3.9z", 0x000003, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	ROM_REGION( 0x200000, "c355spr_3", 0 )
	ROM_LOAD32_BYTE( "glc1-obj-obj0.9t", 0x000000, 0x80000, CRC(0fe98d33) SHA1(5cfefa342fe2fa278d010927d761cb51105a4a60) )
	ROM_LOAD32_BYTE( "glc1-obj-obj1.9w", 0x000001, 0x80000, CRC(660a4f6d) SHA1(c3c3525f51280e71f2d607649a6b5434cbd862c8) )
	ROM_LOAD32_BYTE( "glc1-obj-obj2.9y", 0x000002, 0x80000, CRC(90bcc5a3) SHA1(76cb23e295bb15279e046e83f8e4ab9f85f68243) )
	ROM_LOAD32_BYTE( "glc1-obj-obj3.9z", 0x000003, 0x80000, CRC(65244f07) SHA1(fd876ca5f198914f15864397b358e56fcaa41e90) )

	/********* PSN board x3 *********/
	ROM_REGION( 0x040000, "psn_cpu_1", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b1_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_cpu_2", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b2_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_cpu_3", 0 )
	ROM_LOAD16_BYTE( "glc-psn-prg0b.ic22", 0x000001, 0x20000, CRC(da8a74f8) SHA1(2826a55a4a0acec07ff760c7857da10c4ffaf7d0) )
	ROM_LOAD16_BYTE( "glc-psn-prg1b.ic23", 0x000000, 0x20000, CRC(978431c9) SHA1(7d631444f5844c55bea820507e34a17199f5da2e) )
	ROM_REGION( 0x020000, "psn_b3_vol", 0 )
	ROM_LOAD( "glc-psn-vol.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	/********* RS board x1 *********/
	ROM_REGION( 0x040000, "rso_cpu", 0 )
	ROM_LOAD16_BYTE( "glc-rs-prglb.18b", 0x000001, 0x20000, CRC(9d0c8d03) SHA1(8fffef622cd4440ea9f17882cd54a8a49fbbc148) )
	ROM_LOAD16_BYTE( "glc-rs-prgub.19b", 0x000000, 0x20000, CRC(125ad94c) SHA1(4e2e316b639e9a3a78ecd5c827f3309efa3bc78c) )

	/********* SOUND board x1 *********/
	ROM_REGION( 0x080000, "sound_cpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "glc1-snd-prg0.1h", 0x000000, 0x20000, CRC(4845481c) SHA1(3cf90b8b2351b2bc015bf273552e19e09f84ee70) )
	ROM_LOAD16_BYTE( "glc1-snd-prg1.2h", 0x000001, 0x20000, CRC(3b98c175) SHA1(26e59700347bab7fa10f029e781f993f3a97d257) )
	ROM_LOAD16_BYTE( "glc1-snd-data1.4h",0x040001, 0x20000, CRC(8c7135f5) SHA1(b8c3866c70ac1c431140d6cfe50d9273db7d9b68) )


	ROM_REGION16_BE(0x200000, "c140_16a", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE("glc1-snd-voi0.13a", 0x000000, 0x80000, CRC(ef3bda56) SHA1(2cdfec1860a6d2bd645d83b42cc232643818a699))

	ROM_REGION16_BE(0x200000, "c140_16c", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE("glc1-snd-voi2.13c", 0x000000, 0x80000, CRC(ef3bda56) SHA1(2cdfec1860a6d2bd645d83b42cc232643818a699))

	ROM_REGION(0x400000, "c140_16g", ROMREGION_ERASE00)
	ROM_LOAD("glc1-snd-voi8.10g", 0x000000, 0x80000, CRC(bba0c15b) SHA1(b0abc22fd1ae8a9970ad45d9ebdb38e6b06033a7))
	ROM_LOAD("glc1-snd-voi9.11g", 0x100000, 0x80000, CRC(dd1b1ee4) SHA1(b69af15acaa9c3d79d7758adc8722ff5c1129b76))
	ROM_LOAD("glc1-snd-voi10.13g", 0x200000, 0x80000, CRC(1c1dedf4) SHA1(b6b9dac68103ff2206d731d409a557a71afd98f7))
	ROM_LOAD("glc1-snd-voi11.14g", 0x300000, 0x80000, CRC(559e2a8a) SHA1(9a2f28305c6073a0b9b80a5d9617cc25a921e9d0))


	/********* Laserdiscs *********/

	DISK_REGION("laserdisc_1")
	DISK_IMAGE_READONLY("gal3", 0, SHA1(dfbd67f25b24996455f13c0ce5fdf8d20c30fc8a))

	DISK_REGION("laserdisc_2")
	DISK_IMAGE_READONLY("gal3", 0, SHA1(dfbd67f25b24996455f13c0ce5fdf8d20c30fc8a))


	
ROM_END

ROM_START( gal3zlgr )
	/********* CPU-MST board x1 *********/
	ROM_REGION( 0x200000, "main_cpu", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "gz2e-mst-prg0-e78f.6b",  0x00003, 0x80000, CRC(0d8760cc) SHA1(a57e07b3e8195779055e3ca3d8ba91488f5bb7da) )
	ROM_LOAD32_BYTE( "gz2e-mst-prg1-9baf.10b", 0x00002, 0x80000, CRC(fc1e7a06) SHA1(b495302d66ca99d98aeeff6685c05795e506d2de) )
	ROM_LOAD32_BYTE( "gz2e-mst-prg2-a7c6.14b", 0x00001, 0x80000, CRC(cbd4a7e5) SHA1(067bf775c683f28a09c8910824a8174377e5c57f) )
	ROM_LOAD32_BYTE( "gz2e-mst-prg3-0a57.18b", 0x00000, 0x80000, CRC(616d0133) SHA1(456cf35bc2b293ae0f1d48e6ac171e02b6df172b) )

	/********* CPU-SLV board x1 *********/
	ROM_REGION( 0x080000, "slv_cpu", 0 ) /* 68020 Code */
	ROM_LOAD32_BYTE( "gz1-slv-prg0.6b",  0x00003, 0x20000, CRC(58828a8f) SHA1(8d246c38907dbb1007aeba4485d71584b28c4f87) )
	ROM_LOAD32_BYTE( "gz1-slv-prg1.10b", 0x00002, 0x20000, CRC(4ec6f2fe) SHA1(83452d912fe02cd7dc29f1b83be40f63ac5c7d64) )
	ROM_LOAD32_BYTE( "gz1-slv-prg2.14b", 0x00001, 0x20000, CRC(133942c9) SHA1(12e0411e54057c4bc3eaf2f9f065d7c2b775dc06) )
	ROM_LOAD32_BYTE( "gz1-slv-prg3.18b", 0x00000, 0x20000, CRC(58bec4cb) SHA1(e7f6a62c6e500149cc10881f54b400cae0bc09d5) )

	/********* DSP board x2 *********/
	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67_1:point24", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "gz1-dsp-poth.2f", 0x000001, 0x80000, CRC(d0f6d9fe) SHA1(a8b56df54f4dfe0fd856803b7be0a34b9ec1af7e) )  /* most significant */
	ROM_LOAD32_BYTE( "gz1-dsp-potu.2k", 0x000002, 0x80000, CRC(072a99a4) SHA1(cdfac76b768d1b966e079d508784b9588c278335) )
	ROM_LOAD32_BYTE( "gz1-dsp-potl.2n", 0x000003, 0x80000, CRC(b318534a) CRC(2c528c1e) SHA1(8d9e0142376de15754dc11207e538f541b423497) )  /* least significant */

	ROM_REGION32_BE( 0x400000, "namcos21dsp_c67_2:point24", ROMREGION_ERASE ) /* 24bit signed point data */
	ROM_LOAD32_BYTE( "gz1-dsp-poth.2f", 0x000001, 0x80000, CRC(d0f6d9fe) SHA1(a8b56df54f4dfe0fd856803b7be0a34b9ec1af7e) )  /* most significant */
	ROM_LOAD32_BYTE( "gz1-dsp-potu.2k", 0x000002, 0x80000, CRC(072a99a4) SHA1(cdfac76b768d1b966e079d508784b9588c278335) )
	ROM_LOAD32_BYTE( "gz1-dsp-potl.2n", 0x000003, 0x80000, CRC(b318534a) CRC(2c528c1e) SHA1(8d9e0142376de15754dc11207e538f541b423497) )  /* least significant */

	/********* OBJ board x2 *********/
	ROM_REGION( 0x200000, "c355spr_1", 0 )
	ROM_LOAD32_BYTE( "gz1-obj-obj0.9t", 0x000000, 0x80000, CRC(8c6730b6) SHA1(3147d487dba352301148ab490a704516246cb057) )
	ROM_LOAD32_BYTE( "gz1-obj-obj1.9w", 0x000002, 0x80000, CRC(cba30c26) SHA1(5eb7ab0d9353e1e44a87a96f2ba5fbd22902ccc0) )
	ROM_LOAD32_BYTE( "gz1-obj-obj2.9y", 0x000001, 0x80000, CRC(c9a8abf3) SHA1(a7969a01b1174032e02cdd8f81850eb67d5e747f) )
	ROM_LOAD32_BYTE( "gz1-obj-obj3.9z", 0x000003, 0x80000, CRC(31d55cb8) SHA1(76f3841727e1244d4582559593e7ebff801273a1) )

	ROM_REGION( 0x200000, "c355spr_2", 0 )
	ROM_LOAD32_BYTE( "gz1-obj-obj0.9t", 0x000000, 0x80000, CRC(8c6730b6) SHA1(3147d487dba352301148ab490a704516246cb057) )
	ROM_LOAD32_BYTE( "gz1-obj-obj1.9w", 0x000002, 0x80000, CRC(cba30c26) SHA1(5eb7ab0d9353e1e44a87a96f2ba5fbd22902ccc0) )
	ROM_LOAD32_BYTE( "gz1-obj-obj2.9y", 0x000001, 0x80000, CRC(c9a8abf3) SHA1(a7969a01b1174032e02cdd8f81850eb67d5e747f) )
	ROM_LOAD32_BYTE( "gz1-obj-obj3.9z", 0x000003, 0x80000, CRC(31d55cb8) SHA1(76f3841727e1244d4582559593e7ebff801273a1) )

	ROM_REGION( 0x200000, "c355spr_3", 0 )
	ROM_LOAD32_BYTE( "gz1-obj-obj0.9t", 0x000000, 0x80000, CRC(8c6730b6) SHA1(3147d487dba352301148ab490a704516246cb057) )
	ROM_LOAD32_BYTE( "gz1-obj-obj1.9w", 0x000002, 0x80000, CRC(cba30c26) SHA1(5eb7ab0d9353e1e44a87a96f2ba5fbd22902ccc0) )
	ROM_LOAD32_BYTE( "gz1-obj-obj2.9y", 0x000001, 0x80000, CRC(c9a8abf3) SHA1(a7969a01b1174032e02cdd8f81850eb67d5e747f) )
	ROM_LOAD32_BYTE( "gz1-obj-obj3.9z", 0x000003, 0x80000, CRC(31d55cb8) SHA1(76f3841727e1244d4582559593e7ebff801273a1) )

	/********* PSN board x3 *********/
	ROM_REGION( 0x040000, "psn_cpu_1", 0 )
	ROM_LOAD16_BYTE( "glc1-psn-prg0c.ic22", 0x000001, 0x20000, CRC(3d6e22f7) SHA1(84278036bb474cd00157d809fea020eb847fccd4) )
	ROM_LOAD16_BYTE( "glc1-psn-prg1c.ic23", 0x000000, 0x20000, CRC(8dd45bce) SHA1(a1fc4ee2dac238e6ca66c16b5a5b2d681892bc91) )
	ROM_REGION( 0x020000, "psn_b1_vol", 0 )
	ROM_LOAD( "glc1-psn-v0l.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_cpu_2", 0 )
	ROM_LOAD16_BYTE( "glc1-psn-prg0c.ic22", 0x000001, 0x20000, CRC(3d6e22f7) SHA1(84278036bb474cd00157d809fea020eb847fccd4) )
	ROM_LOAD16_BYTE( "glc1-psn-prg1c.ic23", 0x000000, 0x20000, CRC(8dd45bce) SHA1(a1fc4ee2dac238e6ca66c16b5a5b2d681892bc91) )
	ROM_REGION( 0x020000, "psn_b2_vol", 0 )
	ROM_LOAD( "glc1-psn-v0l.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	ROM_REGION( 0x040000, "psn_cpu_3", 0 )
	ROM_LOAD16_BYTE( "glc1-psn-prg0c.ic22", 0x000001, 0x20000, CRC(3d6e22f7) SHA1(84278036bb474cd00157d809fea020eb847fccd4) )
	ROM_LOAD16_BYTE( "glc1-psn-prg1c.ic23", 0x000000, 0x20000, CRC(8dd45bce) SHA1(a1fc4ee2dac238e6ca66c16b5a5b2d681892bc91) )
	ROM_REGION( 0x020000, "psn_b3_vol", 0 )
	ROM_LOAD( "glc1-psn-v0l.ic100", 0x000000, 0x20000, CRC(9d49576c) SHA1(25c02d2cc171468711c71d8f2da0ea7d9b5f0c23) )

	/********* RS board x1 *********/
	ROM_REGION( 0x040000, "rso_cpu", 0 )
	ROM_LOAD16_BYTE( "gz1-rs-prgl.18b", 0x000001, 0x20000, CRC(4452046b) SHA1(e3754d625b3b32b72836fe0649ecb885d0c61dd6) )
	ROM_LOAD16_BYTE( "gz1-rs-prgu.19b", 0x000000, 0x20000, CRC(daa089f7) SHA1(5f2ad90a67068cb7986945144da0facefbdd4ebc) )

	/********* SOUND board x1 *********/
	ROM_REGION( 0x080000, "sound_cpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gz1-snd-prg0.1h", 0x000000, 0x20000, CRC(24f39552) SHA1(31d1d8248742ee767c0e390a67ce46974e31879e) )
	ROM_LOAD16_BYTE( "gz1-snd-prg1.2h", 0x000001, 0x20000, CRC(e06c6381) SHA1(12eb5c3f3bc4700ba39328e2d3aa5f6847b6ca08) )
	ROM_LOAD16_BYTE( "gz1-snd-data1b.5h", 0x040001, 0x20000, CRC(bf8a68b8) SHA1(6e8cfc1fb4463acd9135b94b258e92b13a09b21f) )

	
	ROM_REGION(0x200000, "c140_16a", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE("gz2-snd-voi0.13a", 0x000000, 0x80000, CRC(25443ea7) SHA1(75429504e2b42671e3cd20f9fab0b49d7749a5bf))
	ROM_LOAD16_BYTE("gz2-snd-voi1.14a", 0x100000, 0x80000, CRC(712cd42f) SHA1(74a2458f98dd2ae47c7dbe682207382205923860))

	ROM_REGION(0x200000, "c140_16c", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE("gz2-snd-voi2.13c", 0x000000, 0x80000, CRC(25443ea7) SHA1(75429504e2b42671e3cd20f9fab0b49d7749a5bf))
	ROM_LOAD16_BYTE("gz2-snd-voi3.14c", 0x100000, 0x80000, CRC(712cd42f) SHA1(74a2458f98dd2ae47c7dbe682207382205923860))

	ROM_REGION(0x400000, "c140_16g", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE("gz2-snd-voi8.10g",  0x000000, 0x80000, CRC(11c77554) SHA1(e4392b156ad5162f8848cc4947931d72df2faeae))
	ROM_LOAD16_BYTE("gz2-snd-voi9.11g",  0x100000, 0x80000, CRC(08112f11) SHA1(44931eb46c755ea6ae1c06129137d9a38b4c57cc))
	ROM_LOAD16_BYTE("gz2-snd-voi10.13g", 0x200000, 0x80000, CRC(11d08f85) SHA1(533a78d57821f0dfedc27bf1bd280339b3d1aeb4))
	ROM_LOAD16_BYTE("gz2-snd-voi11.14g", 0x300000, 0x80000, CRC(d6808d5b) SHA1(a9f8e29dab54bec5c37738a02782b923fc9bbe41))


	/********* Laserdiscs *********/
	
	DISK_REGION( "laserdisc_1" )
	DISK_IMAGE_READONLY("gal3zlgr", 0, SHA1(2355aa5dfdfd8c0a5cd1ef87d508c53e9fb52299))

	DISK_REGION( "laserdisc_2" )
	DISK_IMAGE_READONLY("gal3zlgr", 0, SHA1(2355aa5dfdfd8c0a5cd1ef87d508c53e9fb52299))

ROM_END


} // anonymous namespace


void gal3_state::gal3zlgr_init()
{
	uint8_t *rom  = (uint8_t *)memregion("slv_cpu")->base();

	rom[0x2016] = 0x16; // Timing hack to delay the slave 68020 for longer (was 0x4)

}


void gal3_state::gal3_init()
{
	m_is_dragoon = true;
}


/*     YEAR  NAME     PARENT  MACHINE  INPUT  CLASS       INIT           MONITOR  COMPANY  FULLNAME                                    FLAGS */
GAMEL( 1992, gal3,      0,     gal3,    gal3,  gal3_state, gal3_init,    ROT0,  "Namco", "Galaxian 3 - Theater 6 : Project Dragoon", MACHINE_IMPERFECT_SOUND| MACHINE_IMPERFECT_GRAPHICS, layout_dualhsxs )
GAMEL( 1994, gal3zlgr,  0,     gal3,    gal3,  gal3_state, gal3zlgr_init, ROT0, "Namco", "Galaxian 3 - Theater 6 : Attack of the Zolgear", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS, layout_dualhsxs) 