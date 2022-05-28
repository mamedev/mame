// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith, Carl
/*
    machine/rmnimbus.c

    Machine driver for the Research Machines Nimbus.

    Phill Harvey-Smith
    2009-11-29.

*/

/*

    SCSI/SASI drives supported by RM Nimbus machines

Native SCSI - format with HDFORM.EXE

Drive           Capacity    Tracks  Heads   Sec/Track       Blocks
RO652-20        20MB        306     4       34              41616
ST225N          20MB        615     4       17              41721
ST125N          20MB        407     4       26              41921
8425S-30        20MB                                        41004
CP3020          20MB        623     2       33              41118
ST225NP         20MB        615     4       17              41720
CP3040          40MB        1026    2       40              82080

Via Xebec S1410 SASI to MFM bridge board - format with WINFORM.EXE
NP05-10S         8MB        160     6       17              16320
NP04-20T        16MB        320     6       17              32640
NP03-20         15MB        306     6       17              31212
R352-10         10MB        306     4       17              20808
NP04-50         40MB        699     7       17              83181
NP04-55         44MB        754     7       17              89726

Via Adaptec ACB4070 SCSI to RLL bridge board - format with ADAPT.EXE
NEC D5147       60MB        615     8       26              127920
ST227R          60MB        820     6       26              127920

After formating, the drives need to have a partition table put on them with
STAMP.EXE and then formatted in the normal way for a dos system drive with
Format /s.

The tracks, heads and sectors/track can be used with chdman createhd
to create a blank hard disk which can then be formatted with the RM tools.
The important thing when doing this is to make sure that if using the Native
SCSI tools, that the disk has the  same number of blocks as specified above,
even if you have to use unusual geometry to do so !
Currently, only the ST225N and ST125N can be formatted as the other native
drives and Xebec board expect the WRITE BUFFER (0x3B) and READ BUFFER (0x3C)
with mode 0 commands to be implemented and the Adaptec board uses unknown
command 0xE4.


for example:

chdman createhd -o ST125N.chd -chs 41921,1,1 -ss 512
(the actual geometry can't be used because the block count won't match)

*/

#include "emu.h"
#include <functional>

#include "includes/rmnimbus.h"
#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "imagedev/floppy.h"



/*-------------------------------------------------------------------------*/
/* Defines, constants, and global variables                                */
/*-------------------------------------------------------------------------*/

/* External int vectors for chained interrupts */
#define EXTERNAL_INT_DISK       0x80
#define EXTERNAL_INT_MSM5205    0x84
#define EXTERNAL_INT_MOUSE_YU   0x88
#define EXTERNAL_INT_MOUSE_YD   0x89
#define EXTERNAL_INT_MOUSE_XL   0x8A
#define EXTERNAL_INT_MOUSE_XR   0x8B
#define EXTERNAL_INT_PC8031_8C  0x8c
#define EXTERNAL_INT_PC8031_8E  0x8E
#define EXTERNAL_INT_PC8031_8F  0x8F

#define HDC_DRQ_MASK        0x40
#define FDC_SIDE()          ((m_nimbus_drives.reg400 & 0x10) >> 4)
#define FDC_MOTOR()         ((m_nimbus_drives.reg400 & 0x20) >> 5)
#define FDC_DRIVE()         (fdc_driveno(m_nimbus_drives.reg400 & 0x0f))
#define HDC_DRQ_ENABLED()   ((m_nimbus_drives.reg400 & 0x40) ? 1 : 0)
#define FDC_DRQ_ENABLED()   ((m_nimbus_drives.reg400 & 0x80) ? 1 : 0)

/* 8031/8051 Peripheral controller */

#define IPC_OUT_ADDR        0x01
#define IPC_OUT_READ_PEND   0x02
#define IPC_OUT_BYTE_AVAIL  0x04

#define IPC_IN_ADDR         0x01
#define IPC_IN_BYTE_AVAIL   0x02
#define IPC_IN_READ_PEND    0x04

/* IO unit */

#define DISK_INT_ENABLE         0x01
#define MSM5205_INT_ENABLE      0x04
#define MOUSE_INT_ENABLE        0x08
#define PC8031_INT_ENABLE       0x10

#define MOUSE_NONE      0x00
#define MOUSE_LEFT      0x01
#define MOUSE_RIGHT     0x02
#define MOUSE_DOWN      0x04
#define MOUSE_UP        0x08
#define MOUSE_LBUTTON   0x10
#define MOUSE_RBUTTON   0x20

// Frequency in Hz to poll for mouse movement.
#define MOUSE_POLL_FREQUENCY    500

#define MOUSE_INT_ENABLED(state)     (((state)->m_iou_reg092 & MOUSE_INT_ENABLE) ? 1 : 0)

#define LINEAR_ADDR(seg,ofs)    ((seg<<4)+ofs)

#define OUTPUT_SEGOFS(mess,seg,ofs)  logerror("%s=%04X:%04X [%08X]\n",mess,seg,ofs,((seg<<4)+ofs))

#define LOG_SIO             0
#define LOG_DISK_HDD        0
#define LOG_DISK            0
#define LOG_PC8031          0
#define LOG_PC8031_186      0
#define LOG_PC8031_PORT     0
#define LOG_IOU             0
#define LOG_RAM             0

/* Debugging */

#define DEBUG_SET(flags)        ((m_debug_machine & (flags))==(flags))
#define DEBUG_SET_STATE(flags)  ((state->m_debug_machine & (flags))==(flags))

#define DEBUG_NONE          0x0000000
#define DECODE_BIOS         0x0000002
#define DECODE_BIOS_RAW     0x0000004
#define DECODE_DOS21        0x0000008

/* Nimbus sub-bios structures for debugging */

struct t_area_params
{
	uint16_t  ofs_brush;
	uint16_t  seg_brush;
	uint16_t  ofs_data;
	uint16_t  seg_data;
	uint16_t  count;
};

struct t_plot_string_params
{
	uint16_t  ofs_font;
	uint16_t  seg_font;
	uint16_t  ofs_data;
	uint16_t  seg_data;
	uint16_t  x;
	uint16_t  y;
	uint16_t  length;
};

struct t_nimbus_brush
{
	uint16_t  style;
	uint16_t  style_index;
	uint16_t  colour1;
	uint16_t  colour2;
	uint16_t  transparency;
	uint16_t  boundary_spec;
	uint16_t  boundary_colour;
	uint16_t  save_colour;
};


static int instruction_hook(device_t &device, offs_t curpc);

void rmnimbus_state::external_int(uint8_t vector, bool state)
{
	if(!state && (vector != m_vector))
		return;

	m_vector = vector;

	m_maincpu->int0_w(state);
}

uint8_t rmnimbus_state::cascade_callback()
{
	m_maincpu->int0_w(0);
	return !m_vector ? m_z80sio->m1_r() : m_vector;
}

void rmnimbus_state::machine_reset()
{
	/* CPU */
	iou_reset();
	fdc_reset();
	hdc_reset();
	pc8031_reset();
	rmni_sound_reset();
	memory_reset();
	mouse_js_reset();

	/* USER VIA 6522 port B is connected to the BBC user port */
	m_via->write_pb(0xff);
}

void rmnimbus_state::machine_start()
{
	m_nimbus_mouse.m_mouse_timer = timer_alloc(FUNC(rmnimbus_state::do_mouse), this);

	/* setup debug commands */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("nimbus_debug", CMDFLAG_NONE, 0, 1, std::bind(&rmnimbus_state::debug_command, this, _1));

		/* set up the instruction hook */
		m_maincpu->debug()->set_instruction_hook(instruction_hook);
	}

	m_debug_machine=DEBUG_NONE;
	m_debug_trap=0;
	m_voice_enabled=false;
	m_fdc->dden_w(0);
	//m_fdc->overide_delays(64,m_fdc->get_cmd_delay());
}

void rmnimbus_state::debug_command(const std::vector<std::string> &params)
{
	if (params.size() > 0)
	{
		int temp;
		sscanf(params[0].c_str(), "%d", &temp);
		m_debug_machine = temp;
	}
	else
	{
		machine().debugger().console().printf("Error usage : nimbus_debug <debuglevel>\n");
		machine().debugger().console().printf("Current debuglevel=%02X\n", m_debug_machine);
	}
}

/*-----------------------------------------------
    instruction_hook - per-instruction hook
-----------------------------------------------*/

static int instruction_hook(device_t &device, offs_t curpc)
{
	rmnimbus_state  *state = device.machine().driver_data<rmnimbus_state>();
	address_space   &space = device.memory().space(AS_PROGRAM);
	uint8_t         *addr_ptr;
	uint8_t         first;

	addr_ptr = (uint8_t*)space.get_read_ptr(curpc);

	first = (curpc & 0x01) ? 1 : 0;

	if(DEBUG_SET_STATE(DECODE_BIOS) && (curpc == state->m_debug_trap) && (0 != state->m_debug_trap))
	{
		state->decode_subbios_return(&device,curpc);
	}

	if ((addr_ptr !=nullptr) && (addr_ptr[first]==0xCD))
	{
		if(DEBUG_SET_STATE(DECODE_BIOS) && (addr_ptr[first+1]==0xF0))
			state->decode_subbios(&device,curpc);

		if(DEBUG_SET_STATE(DECODE_DOS21) && (addr_ptr[first+1]==0x21))
			state->decode_dos21(&device,curpc);
	}

	return 0;
}

void rmnimbus_state::decode_subbios_return(device_t *device, offs_t pc)
{
	uint16_t  ax = m_maincpu->state_int(I8086_AX);
	uint16_t  ds = m_maincpu->state_int(I8086_DS);
	uint16_t  si = m_maincpu->state_int(I8086_SI);

	if(!DEBUG_SET(DECODE_BIOS_RAW))
	{
		logerror("at %05X sub-bios return code : %04X\n",pc,ax);
		decode_dssi_generic(ds,si);
		logerror("=======================================================================\n");
	}
	else
		logerror("%05X :: %04X\n",pc,ax);

	m_debug_trap=0;
}

#define set_type(type_name)     sprintf(type_str,type_name)
#define set_drv(drv_name)       sprintf(drv_str,drv_name)
#define set_func(func_name)     sprintf(func_str,func_name)

void rmnimbus_state::decode_subbios(device_t *device,offs_t pc)
{
	char    type_str[80];
	char    drv_str[80];
	char    func_str[80];

	void (rmnimbus_state::*dump_dssi)(uint16_t, uint16_t) = &rmnimbus_state::decode_dssi_none;

	uint16_t  ax = m_maincpu->state_int(I8086_AX);
	uint16_t  bx = m_maincpu->state_int(I8086_BX);
	uint16_t  cx = m_maincpu->state_int(I8086_CX);
	uint16_t  ds = m_maincpu->state_int(I8086_DS);
	uint16_t  si = m_maincpu->state_int(I8086_SI);

	// Set the address to trap after the sub-bios call.
	m_debug_trap=pc+2;

	// *** TEMP Don't show f_enquire_display_line calls !
	if((cx==6) && (ax==43))
		return;
	// *** END TEMP

	if(!DEBUG_SET(DECODE_BIOS_RAW))
	{
		logerror("=======================================================================\n");
		logerror("Sub-bios call at %08X, AX=%04X, BX=%04X, CX=%04X, DS:SI=%04X:%04X\n",pc,ax,bx,cx,ds,si);
	}

	set_type("invalid");
	set_drv("invalid");
	set_func("invalid");

	switch (cx)
	{
		case 0   :
		{
			set_type("t_mummu");
			set_drv("d_mummu");

			switch (ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_add_type_code"); break;
				case 2  : set_func("f_del_typc_code"); break;
				case 3  : set_func("f_get_TCB"); break;
				case 4  : set_func("f_add_driver_code"); break;
				case 5  : set_func("f_del_driver_code"); break;
				case 6  : set_func("f_get_DCB"); break;
				case 7  : set_func("f_get_copyright"); break;
			}
		}; break;

		case 1   :
		{
			set_type("t_character");
			set_drv("d_printer");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_get_output_status"); break;
				case 2  : set_func("f_output_character"); break;
				case 3  : set_func("f_get_input_status"); break;
				case 4  : set_func("f_get_and_remove"); break;
				case 5  : set_func("f_get_no_remove"); break;
				case 6  : set_func("f_get_last_and_remove"); break;
				case 7  : set_func("f_get_last_no_remove"); break;
				case 8  : set_func("f_set_IO_parameters"); break;
			}
		}; break;

		case 2   :
		{
			set_type("t_disk");

			switch(bx)
			{
				case 0  : set_drv("d_floppy"); break;
				case 1  : set_drv("d_winchester"); break;
				case 2  : set_drv("d_tape"); break;
				case 3  : set_drv("d_rompack"); break;
				case 4  : set_drv("d_eeprom"); break;
			}

			dump_dssi = &rmnimbus_state::decode_dssi_generic;

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_initialise_unit"); break;
				case 2  : set_func("f_pseudo_init_unit"); break;
				case 3  : set_func("f_get_device_status"); break;
				case 4  : set_func("f_read_n_sectors"); dump_dssi = &rmnimbus_state::decode_dssi_f_rw_sectors; break;
				case 5  : set_func("f_write_n_sectors"); dump_dssi = &rmnimbus_state::decode_dssi_f_rw_sectors; break;
				case 6  : set_func("f_verify_n_sectors"); dump_dssi = &rmnimbus_state::decode_dssi_f_rw_sectors; break;
				case 7  : set_func("f_media_check"); break;
				case 8  : set_func("f_recalibrate"); break;
				case 9  : set_func("f_motors_off"); break;
			}

		}; break;

		case 3   :
		{
			set_type("t_piconet");
			set_drv("d_piconet");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_get_slave_status"); break;
				case 2  : set_func("f_get_slave_map"); break;
				case 3  : set_func("f_change_slave_addr"); break;
				case 4  : set_func("f_read_slave_control"); break;
				case 5  : set_func("f_write_slave_control"); break;
				case 6  : set_func("f_send_data_byte"); break;
				case 7  : set_func("f_request_data_byte"); break;
				case 8  : set_func("f_send_data_block"); break;
				case 9  : set_func("f_request_data_block"); break;
				case 10 : set_func("f_reset_slave"); break;

			}
		}; break;

		case 4   :
		{
			set_type("t_tick");
			set_drv("d_tick");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_ticks_per_second"); break;
				case 2  : set_func("f_link_tick_routine"); break;
				case 3  : set_func("f_unlink_tick_routine"); break;
			}
		}; break;

		case 5   :
		{
			set_type("t_graphics_input");

			switch(bx)
			{
				case 0  : set_drv("d_mouse"); break;
				case 1  : set_drv("d_joystick_1"); break;
				case 2  : set_drv("d_joystick_2"); break;
			}


			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_graphics_input_cold_start"); break;
				case 2  : set_func("f_graphics_input_device_off"); break;
				case 3  : set_func("f_return_button_status"); break;
				case 4  : set_func("f_return_switch_and_button_stat"); break;
				case 5  : set_func("f_start_tracking"); break;
				case 6  : set_func("f_stop_tracking"); break;
				case 7  : set_func("f_enquire_position"); break;
				case 8  : set_func("f_set_position"); break;

				case 10 : set_func("f_return_button_press_info"); break;
				case 11 : set_func("f_return_button_release_info"); break;
				case 12 : set_func("f_set_gain/f_set_squeaks_per_pixel_ratio"); break;
				case 13 : set_func("f_enquire_graphics_in_misc_data"); break;
			}
		}; break;

		case 6   :
		{
			set_type("t_graphics_output");
			set_drv("d_ngc_screen");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number");                 break;
				case 1  : set_func("f_graphics_output_cold_start");         break;
				case 2  : set_func("f_graphics_output_warm_start");         break;
				case 3  : set_func("f_graphics_output_off");                break;
				case 4  : set_func("f_reinit_graphics_output");             break;
				case 5  : set_func("f_polymarker");                         break;
				case 6  : set_func("f_polyline"); dump_dssi = &rmnimbus_state::decode_dssi_f_fill_area; break;
				case 7  : set_func("f_fill_area"); dump_dssi = &rmnimbus_state::decode_dssi_f_fill_area; break;
				case 8  : set_func("f_flood_fill_area"); break;
				case 9  : set_func("f_plot_character_string"); dump_dssi = &rmnimbus_state::decode_dssi_f_plot_character_string; break;
				case 10 : set_func("f_define_graphics_clipping_area"); break;
				case 11 : set_func("f_enquire_clipping_area_limits"); break;
				case 12 : set_func("f_select_graphics_clipping_area"); break;
				case 13 : set_func("f_enq_selctd_graphics_clip_area"); break;
				case 14 : set_func("f_set_clt_element"); break;
				case 15 : set_func("f_enquire_clt_element"); break;
				case 16 : set_func("f_set_new_clt"); dump_dssi = &rmnimbus_state::decode_dssi_f_set_new_clt; break;
				case 17 : set_func("f_enquire_clt_contents"); break;
				case 18 : set_func("f_define_dithering_pattern"); break;
				case 19 : set_func("f_enquire_dithering_pattern"); break;
				case 20 : set_func("f_draw_sprite"); break;
				case 21 : set_func("f_move_sprite"); break;
				case 22 : set_func("f_erase_sprite"); break;
				case 23 : set_func("f_read_pixel"); break;
				case 24 : set_func("f_read_to_limit"); break;
				case 25 : set_func("f_read_area_pixel"); break;
				case 26 : set_func("f_write_area_pixel"); break;
				case 27 : set_func("f_copy_area_pixel"); break;

				case 29 : set_func("f_read_area_word"); break;
				case 30 : set_func("f_write_area_word"); break;
				case 31 : set_func("f_copy_area_word"); break;
				case 32 : set_func("f_swap_area_word"); break;
				case 33 : set_func("f_set_border_colour"); break;
				case 34 : set_func("f_enquire_border_colour"); break;
				case 35 : set_func("f_enquire_miscellaneous_data"); break;
				case 36  : set_func("f_circle"); break;

				case 38 : set_func("f_arc_of_ellipse"); break;
				case 39 : set_func("f_isin"); break;
				case 40 : set_func("f_icos"); break;
				case 41 : set_func("f_define_hatching_pattern"); break;
				case 42 : set_func("f_enquire_hatching_pattern"); break;
				case 43 : set_func("f_enquire_display_line"); break;
				case 44 : set_func("f_plonk_logo"); break;
			}
		}; break;

		case 7   :
		{
			set_type("t_zend");
			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
			}
		}; break;

		case 8   :
		{
			set_type("t_zep");
			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
			}
		}; break;

		case 9   :
		{
			set_type("t_raw_console");

			switch(bx)
			{
				case 0  :
				{
					set_drv("d_screen");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_plonk_char"); dump_dssi = &rmnimbus_state::decode_dssi_f_plonk_char; break;
						case 2  : set_func("f_plonk_cursor"); break;
						case 3  : set_func("f_kill_cursor"); break;
						case 4  : set_func("f_scroll"); break;
						case 5  : set_func("f_width"); dump_dssi = &rmnimbus_state::decode_dssi_generic; break;
						case 6  : set_func("f_get_char_set"); break;
						case 7  : set_func("f_set_char_set"); break;
						case 8  : set_func("f_reset_char_set"); break;
						case 9  : set_func("f_set_plonk_parameters"); break;
						case 10 : set_func("f_set_cursor_flash_rate"); break;
					}
				}; break;

				case 1  :
				{
					set_drv("d_keyboard");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_init_keyboard"); break;
						case 2  : set_func("f_get_last_key_code"); break;
						case 3  : set_func("f_get_bitmap"); break;
					}
				}; break;
			}
		}; break;

		case 10   :
		{
			set_type("t_acoustics");

			switch(bx)
			{
				case 0  :
				{
					set_drv("d_sound");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_sound_enable"); break;
						case 2  : set_func("f_play_note"); break;
						case 3  : set_func("f_get_queue_status"); break;
					}
				}; break;

				case 1  :
				{
					set_drv("d_voice");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_talk"); break;
						case 2  : set_func("f_wait_and_talk"); break;
						case 3  : set_func("f_test_talking"); break;
					}
				}
			}
		}; break;

		case 11   :
		{
			set_type("t_hard_sums");
			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
			}
		}; break;
	}

	if(DEBUG_SET(DECODE_BIOS_RAW))
	{
		(this->*dump_dssi)(ds, si);
	}
	else
	{
		logerror("Type=%s, Driver=%s, Function=%s\n",type_str,drv_str,func_str);

		(this->*dump_dssi)(ds, si);
		logerror("=======================================================================\n");
	}
}

static inline void *get_regpair_ptr(address_space &space, uint16_t   segment, uint16_t offset)
{
	int             addr;

	addr=((segment<<4)+offset);

	return space.get_read_ptr(addr);
}

void rmnimbus_state::decode_dssi_none(uint16_t ds, uint16_t si)
{
}

void rmnimbus_state::decode_dssi_generic(uint16_t ds, uint16_t si)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t  *params;
	int     count;

	if(DEBUG_SET(DECODE_BIOS_RAW))
		return;

	params=(uint16_t  *)get_regpair_ptr(space,ds,si);

	for(count=0; count<10; count++)
		logerror("%04X ",params[count]);

	logerror("\n");
}


void rmnimbus_state::decode_dssi_f_fill_area(uint16_t ds, uint16_t si)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	uint16_t          *addr_ptr;
	t_area_params   *area_params;
	t_nimbus_brush  *brush;
	int             cocount;

	area_params = (t_area_params   *)get_regpair_ptr(space,ds,si);

	if (!DEBUG_SET(DECODE_BIOS_RAW))
		OUTPUT_SEGOFS("SegBrush:OfsBrush",area_params->seg_brush,area_params->ofs_brush);

	brush=(t_nimbus_brush  *)space.get_read_ptr(LINEAR_ADDR(area_params->seg_brush,area_params->ofs_brush));

	if(DEBUG_SET(DECODE_BIOS_RAW))
	{
		logerror("\tdw\t%04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X, ",
					brush->style,brush->style_index,brush->colour1,brush->colour2,
					brush->transparency,brush->boundary_spec,brush->boundary_colour,brush->save_colour,
					area_params->count);
	}
	else
	{
		logerror("Brush params\n");
		logerror("Style=%04X,          StyleIndex=%04X\n",brush->style,brush->style_index);
		logerror("Colour1=%04X,        Colour2=%04X\n",brush->colour1,brush->colour2);
		logerror("transparency=%04X,   boundary_spec=%04X\n",brush->transparency,brush->boundary_spec);
		logerror("boundary colour=%04X, save colour=%04X\n",brush->boundary_colour,brush->save_colour);


		OUTPUT_SEGOFS("SegData:OfsData",area_params->seg_data,area_params->ofs_data);
	}

	addr_ptr = (uint16_t *)space.get_read_ptr(LINEAR_ADDR(area_params->seg_data,area_params->ofs_data));
	for(cocount=0; cocount < area_params->count; cocount++)
	{
		if(DEBUG_SET(DECODE_BIOS_RAW))
		{
			if(cocount!=(area_params->count-1))
				logerror("%04X, %04X, ",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
			else
				logerror("%04X, %04X ",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
		}
		else
			logerror("x=%d y=%d\n",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
	}

	if(DEBUG_SET(DECODE_BIOS_RAW))
		logerror("\n");
}

void rmnimbus_state::decode_dssi_f_plot_character_string(uint16_t ds, uint16_t si)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	uint8_t                   *char_ptr;
	t_plot_string_params    *plot_string_params;
	int                     charno;

	if(DEBUG_SET(DECODE_BIOS_RAW))
		return;

	plot_string_params=(t_plot_string_params   *)get_regpair_ptr(space,ds,si);

	OUTPUT_SEGOFS("SegFont:OfsFont",plot_string_params->seg_font,plot_string_params->ofs_font);
	OUTPUT_SEGOFS("SegData:OfsData",plot_string_params->seg_data,plot_string_params->ofs_data);

	logerror("x=%d, y=%d, length=%d\n",plot_string_params->x,plot_string_params->y,plot_string_params->length);

	char_ptr=(uint8_t*)space.get_read_ptr(LINEAR_ADDR(plot_string_params->seg_data,plot_string_params->ofs_data));

	if (plot_string_params->length==0xFFFF)
		logerror("%s",char_ptr);
	else
		for(charno=0;charno<plot_string_params->length;charno++)
			logerror("%c",char_ptr[charno]);

	logerror("\n");
}

void rmnimbus_state::decode_dssi_f_set_new_clt(uint16_t ds, uint16_t si)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t  *new_colours;
	int     colour;
	new_colours=(uint16_t  *)get_regpair_ptr(space,ds,si);

	if(DEBUG_SET(DECODE_BIOS_RAW))
		return;

	OUTPUT_SEGOFS("SegColours:OfsColours",ds,si);

	for(colour=0;colour<16;colour++)
		logerror("colour #%02X=%04X\n",colour,new_colours[colour]);

}

void rmnimbus_state::decode_dssi_f_plonk_char(uint16_t ds, uint16_t si)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t  *params;
	params=(uint16_t  *)get_regpair_ptr(space,ds,si);

	if(DEBUG_SET(DECODE_BIOS_RAW))
		return;

	OUTPUT_SEGOFS("SegParams:OfsParams",ds,si);

	logerror("plonked_char=%c\n",params[0]);
}

void rmnimbus_state::decode_dssi_f_rw_sectors(uint16_t ds, uint16_t si)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t  *params;
	int     param_no;

	if(DEBUG_SET(DECODE_BIOS_RAW))
		return;

	params=(uint16_t  *)get_regpair_ptr(space,ds,si);

	logerror("unitno=%04X, count=%02X, first_sector=%08X buffer=%04X:%04X (%05X)\n",
			 params[0],
			 params[1],
			 ((params[3] * 65536)+params[2]),
			 params[5],params[4],
			 ((params[5]*16)+params[4])
			 );

	for(param_no=0;param_no<16;param_no++)
		logerror("%04X ",params[param_no]);

	logerror("\n");
}

void rmnimbus_state::decode_dos21(device_t *device,offs_t pc)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	//uint16_t  *params;
	char    *path;

	uint16_t  ax = m_maincpu->state_int(I8086_AX);
	uint16_t  bx = m_maincpu->state_int(I8086_BX);
	uint16_t  cx = m_maincpu->state_int(I8086_CX);
	uint16_t  dx = m_maincpu->state_int(I8086_DX);
	uint16_t  cs = m_maincpu->state_int(I8086_CS);
	uint16_t  ds = m_maincpu->state_int(I8086_DS);
	uint16_t  es = m_maincpu->state_int(I8086_ES);
	uint16_t  ss = m_maincpu->state_int(I8086_SS);

	uint16_t  si = m_maincpu->state_int(I8086_SI);
	uint16_t  di = m_maincpu->state_int(I8086_DI);
	uint16_t  bp = m_maincpu->state_int(I8086_BP);

	uint8_t dosfn = ax >> 8;    // Dos function is AH, upper half of AX.

	logerror("=======================================================================\n");
	logerror("DOS Int 0x21 call at %05X\n",pc);
	logerror("AX=%04X, BX=%04X, CX=%04X, DX=%04X\n",ax,bx,cx,dx);
	logerror("CS=%04X, DS=%04X, ES=%04X, SS=%04X\n",cs,ds,es,ss);
	logerror("SI=%04X, DI=%04X, BP=%04X\n",si,di,bp);
	logerror("=======================================================================\n");

	if (((dosfn >= 0x39)  && (dosfn <= 0x3d))
		|| (0x43 == dosfn)
		|| (0x4e == dosfn)
		|| (0x56 == dosfn)
		|| ((dosfn >= 0x5a) && (dosfn <= 0x5b)) )
	{
		path=(char *)get_regpair_ptr(space,ds,dx);
		logerror("Path at DS:DX=%s\n",path);

		if (0x56 == dosfn)
		{
			path=(char *)get_regpair_ptr(space,es,di);
			logerror("Path at ES:DI=%s\n",path);
		}
		logerror("=======================================================================\n");
	}
}

#define CBUFLEN 32

offs_t rmnimbus_state::dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	unsigned call;
	char    callname[CBUFLEN];
	offs_t result = 0;

	// decode and document (some) INT XX calls
	if (opcodes.r8(pc) == 0xCD)
	{
		call = opcodes.r8(pc+1);
		switch (call)
		{
			case 0x20 :
				strcpy(callname, "(dos terminate)");
				break;

			case 0x21 :
				strcpy(callname, "(dos function)");
				break;

			case 0xf0 :
				strcpy(callname, "(sub_bios)");
				break;

			case 0xf3 :
				strcpy(callname, "(dispatch handler)");
				break;

			case 0xf5 :
				strcpy(callname, "(event handler)");
				break;

			case 0xf6 :
				strcpy(callname, "(resource message)");
				break;

			default :
				strcpy(callname, "");
		}
		util::stream_format(stream, "int   %02xh %s",call,callname);
		result = 2;
	}
	return result;
}

/*
    The Nimbus has 3 banks of memory each of which can be either 16x4164 or 16x41256 giving
    128K or 512K per bank. These banks are as follows :

    bank0   on nimbus motherboard.
    bank1   first half of expansion card.
    bank2   second half of expansion card.

    The valid combinations are :

    bank0       bank1       bank2       total
    128K                                128K
    128K        128K                    256K
    128K        128K        128K        384K
    128K        512K                    640K (1)
    512K        128K                    640K (2)
    512K        512K                    1024K
    512K        512K        512K        1536K

    It will be noted that there are two possible ways of getting 640K, we emulate method 2
    (above).

    To allow for the greatest flexibility, the Nimbus allows 4 methods of mapping the
    banks of ram into the 1M addressable by the 81086.

    With only 128K banks present, they are mapped into the first 3 blocks of 128K in
    the memory map giving a total of up to 384K.

    If any of the blocks are 512K, then the block size is set to 512K and the map arranged
    so that the bottom block is a 512K block (if both 512K and 128K blocks are available).

    This is all determined by the value written to port 80 :-

    port80 = 0x07   start       end
        block0      0x00000     0x1FFFF
        block1      0x20000     0x3FFFF
        block2      0x40000     0x5FFFF

    port80 = 0x1F
        block0      0x00000     0x7FFFF
        block1      0x80000     0xEFFFF (0x9FFFF if 128K (2))

    port80 = 0x0F
        block1      0x00000     0x7FFFF
        block0      0x80000     0xEFFFF (0x9FFFF if 128K (1))

    port80 = 0x17
        block1      0x00000     0x7FFFF
        block2      0x80000     0xEFFFF

*/

struct nimbus_meminfo
{
	offs_t  start;      /* start address of bank */
	offs_t  end;        /* End address of bank */
};

static const struct nimbus_meminfo memmap[] =
{
	{ 0x00000, 0x1FFFF },
	{ 0x20000, 0x3FFFF },
	{ 0x40000, 0x5FFFF },
	{ 0x60000, 0x7FFFF },
	{ 0x80000, 0x9FFFF },
	{ 0xA0000, 0xBFFFF },
	{ 0xC0000, 0xDFFFF },
	{ 0xE0000, 0xEFFFF }
};

struct nimbus_block
{
	int     blockbase;
	int     blocksize;
};

typedef nimbus_block nimbus_blocks[3];

static const nimbus_blocks ramblocks[] =
{
	{{ 0, 128 },    { 000, 000 },   { 000, 000 }} ,
	{{ 0, 128 },    { 128, 128 },   { 000, 000 }} ,
	{{ 0, 128 },    { 128, 128 },   { 256, 128 }} ,
	{{ 0, 512 },    { 000, 000 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 128 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 512 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 512 },   { 1024, 512 } }
};

void rmnimbus_state::nimbus_bank_memory()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int     ramsize = m_ram->size();
	int     ramblock = 0;
	int     blockno;
	char    bank[10];
	uint8_t   *ram    = &m_ram->pointer()[0];
	uint8_t   *map_blocks[3];
	uint8_t   *map_base;
	int     map_blockno;
	int     block_ofs;

	uint8_t   ramsel = (m_mcu_reg080 & 0x1F);

	// Invalid ramsel, return.
	if((ramsel & 0x07)!=0x07)
		return;

	switch (ramsize / 1024)
	{
		case 128    : ramblock=0; break;
		case 256    : ramblock=1; break;
		case 384    : ramblock=2; break;
		case 512    : ramblock=3; break;
		case 640    : ramblock=4; break;
		case 1024   : ramblock=5; break;
		case 1536   : ramblock=6; break;
	}

	map_blocks[0]  = ram;
	map_blocks[1]  = (ramblocks[ramblock][1].blocksize==0) ? nullptr : &ram[ramblocks[ramblock][1].blockbase*1024];
	map_blocks[2]  = (ramblocks[ramblock][2].blocksize==0) ? nullptr : &ram[ramblocks[ramblock][2].blockbase*1024];

	//if(LOG_RAM) logerror("\n\nmcu_reg080=%02X, ramblock=%d, map_blocks[0]=%X, map_blocks[1]=%X, map_blocks[2]=%X\n",m_mcu_reg080,ramblock,(int)map_blocks[0],(int)map_blocks[1],(int)map_blocks[2]);

	for(blockno=0;blockno<8;blockno++)
	{
		sprintf(bank,"bank%d",blockno);

		switch (ramsel)
		{
			case 0x07   : (blockno<3) ? map_blockno=blockno : map_blockno=-1; break;
			case 0x1F   : (blockno<4) ? map_blockno=0 : map_blockno=1; break;
			case 0x0F   : (blockno<4) ? map_blockno=1 : map_blockno=0; break;
			case 0x17   : (blockno<4) ? map_blockno=1 : map_blockno=2; break;
			default     : map_blockno=-1;
		}
		block_ofs=(ramsel==0x07) ? 0 : ((blockno % 4)*128);


		if(LOG_RAM) logerror("mapped %s",bank);

		if((map_blockno>-1) && (block_ofs < ramblocks[ramblock][map_blockno].blocksize) &&
			(map_blocks[map_blockno]!=nullptr))
		{
			map_base=(ramsel==0x07) ? map_blocks[map_blockno] : &map_blocks[map_blockno][block_ofs*1024];

			membank(bank)->set_base(map_base);
			space.install_readwrite_bank(memmap[blockno].start, memmap[blockno].end, membank(bank));
			//if(LOG_RAM) logerror(", base=%X\n",(int)map_base);
		}
		else
		{
			space.nop_readwrite(memmap[blockno].start, memmap[blockno].end);
			if(LOG_RAM) logerror("NOP\n");
		}
	}
}

uint8_t rmnimbus_state::nimbus_mcu_r()
{
	return m_mcu_reg080;
}

void rmnimbus_state::nimbus_mcu_w(uint8_t data)
{
	m_mcu_reg080=data;

	nimbus_bank_memory();
}

void rmnimbus_state::memory_reset()
{
	m_mcu_reg080=0x07;
	nimbus_bank_memory();
}

/*

Z80SIO, used for the keyboard interface

*/

/* Z80 SIO/2 */

WRITE_LINE_MEMBER(rmnimbus_state::sio_interrupt)
{
	if(LOG_SIO)
		logerror("SIO Interrupt state=%02X\n",state);

	external_int(0, state);
}

/* Floppy disk */

void rmnimbus_state::fdc_reset()
{
	m_nimbus_drives.reg400=0;
	m_scsi_ctrl_out->write(0);
}

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_fdc_intrq_w)
{
	if(LOG_DISK)
		logerror("nimbus_drives_intrq = %d\n",state);

	if(m_iou_reg092 & DISK_INT_ENABLE)
	{
		external_int(EXTERNAL_INT_DISK,state);
	}
}

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_fdc_drq_w)
{
	if(LOG_DISK)
		logerror("nimbus_drives_drq_w(%d)\n", state);

	m_maincpu->drq1_w(state && FDC_DRQ_ENABLED());
}

READ_LINE_MEMBER(rmnimbus_state::nimbus_fdc_enmf_r)
{
	return false;
}

uint8_t rmnimbus_state::fdc_driveno(uint8_t drivesel)
{
	switch (drivesel)
	{
		case 0x01: return 0;
		case 0x02: return 1;
		case 0x04: return 2;
		case 0x08: return 3;
		case 0x10: return 4;
		case 0x20: return 5;
		case 0x40: return 6;
		case 0x80: return 7;
		default: return 0;
	}
}

/*
    0x410 read bits

    0   Ready from floppy
    1   Index pulse from floppy
    2   Motor on from floppy
    3   MSG from HDD
    4   !BSY from HDD
    5   !I/O from HDD
    6   !C/D
    7   !REQ from HDD
*/

uint8_t rmnimbus_state::scsi_r(offs_t offset)
{
	int result = 0;

	int pc=m_maincpu->pc();
	char drive[5];
	floppy_image_device *floppy;

	sprintf(drive, "%d", FDC_DRIVE());
	floppy = m_fdc->subdevice<floppy_connector>(drive)->get_device();

	switch(offset*2)
	{
		case 0x00 :
			result |= m_scsi_req << 7;
			result |= m_scsi_cd << 6;
			result |= m_scsi_io << 5;
			result |= m_scsi_bsy << 4;
			result |= m_scsi_msg << 3;
			if(floppy)
			{
				result |= FDC_MOTOR() << 2;
				result |= (!floppy->idx_r()) << 1;
				result |= (floppy->dskchg_r()) << 0;
			}
			break;
		case 0x08 :
			result = m_scsi_data_in->read();
			hdc_post_rw();
			break;
		default:
			break;
	}

	if(LOG_DISK_HDD)
		logerror("Nimbus HDCR at pc=%08X from %04X data=%02X\n",pc,(offset*2)+0x410,result);

	return result;
}

/*
    0x400 write bits

    0   drive 0 select
    1   drive 1 select
    2   drive 2 select
    3   drive 3 select
    4   side select
    5   fdc motor on
    6   hdc drq enabled
    7   fdc drq enabled
*/
void rmnimbus_state::fdc_ctl_w(uint8_t data)
{
	uint8_t old_drq = m_nimbus_drives.reg400 & HDC_DRQ_MASK;
	char drive[5];
	floppy_image_device *floppy;

	m_nimbus_drives.reg400 = data;

	sprintf(drive, "%d", FDC_DRIVE());
	floppy = m_fdc->subdevice<floppy_connector>(drive)->get_device();

	m_fdc->set_floppy(floppy);
	if(floppy)
	{
		floppy->ss_w(FDC_SIDE());
		floppy->mon_w(!FDC_MOTOR());
	}

	// if we enable hdc drq with a pending condition, act on it
	if((data & HDC_DRQ_MASK) && (!old_drq))
		set_scsi_drqlat(false, false);
}

/*
    0x410 write bits

    0   SCSI reset
    1   SCSI SEL
    2   SCSI IRQ Enable
*/

void rmnimbus_state::scsi_w(offs_t offset, uint8_t data)
{
	int pc=m_maincpu->pc();

	if(LOG_DISK_HDD)
		logerror("Nimbus HDCW at %05X write of %02X to %04X\n",pc,data,(offset*2)+0x410);

	switch(offset*2)
	{
		case 0x00 :
			m_scsi_ctrl_out->write(data);
			break;

		case 0x08 :
			m_scsi_data_out->write(data);
			hdc_post_rw();
			break;
	}
}

void rmnimbus_state::hdc_reset()
{
	m_scsi_iena = 0;
	m_scsi_msg = 0;
	m_scsi_bsy = 0;
	m_scsi_io = 0;
	m_scsi_cd = 0;
	m_scsi_req = 0;

	// Latched req, IC11b
	m_scsi_reqlat = 0;
}

/*
    The SCSI code outputs a 1 to indicate an active line, even though it is active low
    The inputs on the RM schematic are fed through inverters, but because of the above
    we don't need to invert them, unless the schematic uses the signal directly
    For consistency we will invert msg before latching.
*/

void rmnimbus_state::check_scsi_irq()
{
	nimbus_fdc_intrq_w(m_scsi_io && m_scsi_cd && m_scsi_req && m_scsi_iena);
}

WRITE_LINE_MEMBER(rmnimbus_state::write_scsi_iena)
{
	m_scsi_iena = state;
	check_scsi_irq();
}

// This emulates the 74LS74 latched version of req
void rmnimbus_state::set_scsi_drqlat(bool   clock, bool clear)
{
	if (clear)
		m_scsi_reqlat = 0;
	else if (clock)
		m_scsi_reqlat = 1;

	if(m_scsi_reqlat)
		hdc_drq(true);
	else
		hdc_drq(false);
}

void rmnimbus_state::hdc_post_rw()
{
	if(m_scsi_req)
		m_scsibus->write_ack(1);

	// IC17A, IC17B, latched req cleared by SCSI data read or write, or C/D= command
	set_scsi_drqlat(false, true);
}

void rmnimbus_state::hdc_drq(bool state)
{
	m_maincpu->drq1_w(HDC_DRQ_ENABLED() && !m_scsi_cd && state);
}

WRITE_LINE_MEMBER( rmnimbus_state::write_scsi_bsy )
{
	m_scsi_bsy = state;
}

WRITE_LINE_MEMBER( rmnimbus_state::write_scsi_cd )
{
	m_scsi_cd = state;

	// IC17A, IC17B, latched req cleared by SCSI data read or write, or C/D= command
	set_scsi_drqlat(false, !m_scsi_cd);

	check_scsi_irq();
}

WRITE_LINE_MEMBER( rmnimbus_state::write_scsi_io )
{
	m_scsi_io = state;

	if (m_scsi_io)
	{
		m_scsi_data_out->write(0);
	}
	check_scsi_irq();
}

WRITE_LINE_MEMBER( rmnimbus_state::write_scsi_msg )
{
	m_scsi_msg = !state;
}

WRITE_LINE_MEMBER( rmnimbus_state::write_scsi_req )
{
	// Detect rising edge on req, IC11b, clock
	int rising = ((m_scsi_req == 0) && (state == 1));

	// This is the state of the actual line from the SCSI
	m_scsi_req = state;

	// Latched req, is forced low by C/D being set to command
	set_scsi_drqlat(rising, m_scsi_cd);

	if (!m_scsi_reqlat)
		m_scsibus->write_ack(0);

	check_scsi_irq();
}

void rmnimbus_state::nimbus_voice_w(offs_t offset, uint8_t data)
{
	if (offset == 0xB0)
		m_voice_enabled = true;
	else if (offset == 0xB2)
		m_voice_enabled = false;
}

/* 8031/8051 Peripheral controller 80186 side */

void rmnimbus_state::pc8031_reset()
{
	logerror("peripheral controller reset\n");

	memset(&m_ipc_interface,0,sizeof(m_ipc_interface));
}


#if 0
void rmnimbus_state::ipc_dumpregs()
{
	logerror("in_data=%02X, in_status=%02X, out_data=%02X, out_status=%02X\n",
				m_ipc_interface.ipc_in, m_ipc_interface.status_in,
				m_ipc_interface.ipc_out, m_ipc_interface.status_out);

}
#endif

uint8_t rmnimbus_state::nimbus_pc8031_r(offs_t offset)
{
	int pc=m_maincpu->pc();
	uint8_t   result;

	switch(offset*2)
	{
		case 0x00   : result=m_ipc_interface.ipc_out;
						m_ipc_interface.status_in   &= ~IPC_IN_READ_PEND;
						m_ipc_interface.status_out  &= ~IPC_OUT_BYTE_AVAIL;
						break;

		case 0x02   : result=m_ipc_interface.status_out;
						break;

		default : result=0; break;
	}

	if(LOG_PC8031_186)
		logerror("Nimbus PCIOR %08X read of %04X returns %02X\n",pc,(offset*2)+0xC0,result);

	return result;
}

void rmnimbus_state::nimbus_pc8031_w(offs_t offset, uint8_t data)
{
	int pc=m_maincpu->pc();

	switch(offset*2)
	{
		case 0x00   : m_ipc_interface.ipc_in=data;
						m_ipc_interface.status_in   |= IPC_IN_BYTE_AVAIL;
						m_ipc_interface.status_in   &= ~IPC_IN_ADDR;
						m_ipc_interface.status_out  |= IPC_OUT_READ_PEND;
						break;

		case 0x02   : m_ipc_interface.ipc_in=data;
						m_ipc_interface.status_in   |= IPC_IN_BYTE_AVAIL;
						m_ipc_interface.status_in   |= IPC_IN_ADDR;
						m_ipc_interface.status_out  |= IPC_OUT_READ_PEND;
						break;
	}

	if(LOG_PC8031_186)
		logerror("Nimbus PCIOW %08X write of %02X to %04X\n",pc,data,(offset*2)+0xC0);

}

/* 8031/8051 Peripheral controller 8031/8051 side */

uint8_t rmnimbus_state::nimbus_pc8031_iou_r(offs_t offset)
{
	int pc=m_iocpu->pc();
	uint8_t   result = 0;

	switch (offset & 0x01)
	{
		case 0x00   : result=m_ipc_interface.ipc_in;
						m_ipc_interface.status_out  &= ~IPC_OUT_READ_PEND;
						m_ipc_interface.status_in   &= ~IPC_IN_BYTE_AVAIL;
						break;

		case 0x01   : result=m_ipc_interface.status_in;
						break;
	}

	if(((offset==2) || (offset==3)) && (m_iou_reg092 & PC8031_INT_ENABLE))
		external_int(EXTERNAL_INT_PC8031_8C, true);

	if(LOG_PC8031)
		logerror("8031: PCIOR %04X read of %04X returns %02X\n",pc,offset,result);

	return result;
}

void rmnimbus_state::nimbus_pc8031_iou_w(offs_t offset, uint8_t data)
{
	int pc=m_iocpu->pc();

	if(LOG_PC8031)
		logerror("8031 PCIOW %04X write of %02X to %04X\n",pc,data,offset);

	switch(offset & 0x03)
	{
		case 0x00   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out  |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out  &= ~IPC_OUT_ADDR;
						m_ipc_interface.status_in   |= IPC_IN_READ_PEND;
						break;

		case 0x01   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out   |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out   |= IPC_OUT_ADDR;
						m_ipc_interface.status_in    |= IPC_IN_READ_PEND;
						break;

		case 0x02   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out  |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out  &= ~IPC_OUT_ADDR;
						m_ipc_interface.status_in   |= IPC_IN_READ_PEND;
						if(m_iou_reg092 & PC8031_INT_ENABLE)
							external_int(EXTERNAL_INT_PC8031_8F, true);
						break;

		case 0x03   : m_ipc_interface.ipc_out=data;
						//m_ipc_interface.status_out   |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out   |= IPC_OUT_ADDR;
						m_ipc_interface.status_in    |= IPC_IN_READ_PEND;
						if(m_iou_reg092 & PC8031_INT_ENABLE)
							external_int(EXTERNAL_INT_PC8031_8E, true);
						break;
	}
}

uint8_t rmnimbus_state::nimbus_pc8031_port1_r()
{
	int pc=m_iocpu->pc();
	uint8_t   result = (m_eeprom_bits & ~4) | (m_eeprom->do_read() << 2);

	if(LOG_PC8031_PORT)
		logerror("8031: PCPORTR %04X read of P1 returns %02X\n",pc,result);

	return result;
}

uint8_t rmnimbus_state::nimbus_pc8031_port3_r()
{
	int pc=m_iocpu->pc();
	uint8_t   result = 0;

	if(LOG_PC8031_PORT)
		logerror("8031: PCPORTR %04X read of P3 returns %02X\n",pc,result);

	return result;
}

void rmnimbus_state::nimbus_pc8031_port1_w(uint8_t data)
{
	int pc=m_iocpu->pc();

	m_eeprom->cs_write((data & 8) ? 1 : 0);

	if(!(data & 8))
		m_eeprom_state = 0;
	else if(!(data & 2) || (m_eeprom_state == 2))
		m_eeprom_state = 2;
	else if((data & 8) && (!(m_eeprom_bits & 8)))
		m_eeprom_state = 1;
	else if((!(data & 1)) && (m_eeprom_bits & 1) && (m_eeprom_state == 1))
		m_eeprom_state = 2; //wait until 1 clk after cs rises to set di else it's seen as a start bit

	m_eeprom->di_write(((data & 2) && (m_eeprom_state == 2)) ? 1 : 0);
	m_eeprom->clk_write((data & 1) ? 1 : 0);
	m_eeprom_bits = data;

	if(LOG_PC8031_PORT)
		logerror("8031 PCPORTW %04X write of %02X to P1\n",pc,data);
}

void rmnimbus_state::nimbus_pc8031_port3_w(uint8_t data)
{
	int pc=m_iocpu->pc();

	if(LOG_PC8031_PORT)
		logerror("8031 PCPORTW %04X write of %02X to P3\n",pc,data);
}


/* IO Unit */
uint8_t rmnimbus_state::nimbus_iou_r(offs_t offset)
{
	int pc=m_maincpu->pc();
	uint8_t   result=0;

	if(offset==0)
	{
		result=m_iou_reg092;
	}

	if(LOG_IOU)
		logerror("Nimbus IOUR %08X read of %04X returns %02X\n",pc,(offset*2)+0x92,result);

	return result;
}

void rmnimbus_state::nimbus_iou_w(offs_t offset, uint8_t data)
{
	int pc=m_maincpu->pc();

	if(LOG_IOU)
		logerror("Nimbus IOUW %08X write of %02X to %04X\n",pc,data,(offset*2)+0x92);

	if(offset==0)
	{
		m_iou_reg092=data;
		m_msm->reset_w((data & MSM5205_INT_ENABLE) ? 0 : 1);
	}
}

void rmnimbus_state::iou_reset()
{
	m_iou_reg092=0x00;
	m_eeprom_state = 0;
}

/* Rompacks, not completely implemented */

uint8_t rmnimbus_state::nimbus_rompack_r(offs_t offset)
{
	logerror("Rompack read offset %02X, rompack address=%04X\n",offset,(m_ay8910_b*256)+m_ay8910_a);

	return 0;
}

void rmnimbus_state::nimbus_rompack_w(offs_t offset, uint8_t data)
{
	logerror("Rompack write offset %02X, data=%02X, rompack address=%04X\n",offset,data,(m_ay8910_b*256)+m_ay8910_a);
}

/*
    Sound hardware : AY8910

    I believe that the IO ports of the 8910 are used to control the ROMPack ports, however
    this is currently un-implemented (and may never be as I don't have any rompacks!).

    The registers are mapped as so :

    Address     0xE0                0xE2
    Read        Data                ????
    Write       Register Address    Data

*/

void rmnimbus_state::rmni_sound_reset()
{
	m_msm->reset_w(1);

	m_last_playmode = msm5205_device::S48_4B;
	m_msm->playmode_w(m_last_playmode);

	m_ay8910_a=0;
	m_ay8910_b=0;
}

void rmnimbus_state::nimbus_sound_ay8910_porta_w(uint8_t data)
{
	m_msm->data_w(data);

	// Mouse code needs a copy of this.
	// ROMpack lower address lines
	m_ay8910_a=data;
}

void rmnimbus_state::nimbus_sound_ay8910_portb_w(uint8_t data)
{
	// Only update msm5205 if voice is enabled.....
	if (m_voice_enabled  && ((data & 0x07) != m_last_playmode))
	{
		m_last_playmode = (data & 0x07);
		m_msm->playmode_w(m_last_playmode);
	}

	// ROMpack upper address lines
	m_ay8910_b=data;
}

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_msm5205_vck)
{
	if(m_iou_reg092 & MSM5205_INT_ENABLE)
		external_int(EXTERNAL_INT_MSM5205,state);
}

static const int MOUSE_XYA[4] = { 1, 1, 0, 0 };
static const int MOUSE_XYB[4] = { 0, 1, 1, 0 };

TIMER_CALLBACK_MEMBER(rmnimbus_state::do_mouse)
{
	uint8_t mouse_x;        // Current mouse X and Y
	uint8_t mouse_y;
	int8_t  xdiff;          // Difference from previous X and Y
	int8_t  ydiff;

	uint8_t intstate_x;     // Used to calculate if we should trigger interrupt
	uint8_t intstate_y;
	int     xint;           // X and Y interrupts to trigger
	int     yint;

	uint8_t   mxa;          // Values of quadrature encoders for X and Y
	uint8_t   mxb;
	uint8_t   mya;
	uint8_t   myb;

	// Read mouse buttons
	m_nimbus_mouse.m_reg0a4 = m_io_mouse_button->read();

	// Read mose positions and calculate difference from previous value
	mouse_x = m_io_mousex->read();
	mouse_y = m_io_mousey->read();

	xdiff = m_nimbus_mouse.m_mouse_x - mouse_x;
	ydiff = m_nimbus_mouse.m_mouse_y - mouse_y;

	// convert movement into emulated movement of quadrature encoder in mouse.
	if (xdiff < 0)
		m_nimbus_mouse.m_mouse_pcx++;
	else if (xdiff > 0)
		m_nimbus_mouse.m_mouse_pcx--;

	if (ydiff < 0)
		m_nimbus_mouse.m_mouse_pcy++;
	else if (ydiff > 0)
		m_nimbus_mouse.m_mouse_pcy--;

	// Compensate for quadrature wrap.
	m_nimbus_mouse.m_mouse_pcx &= 0x03;
	m_nimbus_mouse.m_mouse_pcy &= 0x03;

	// get value of mouse quadrature encoders for this wheel position
	mxa = MOUSE_XYA[m_nimbus_mouse.m_mouse_pcx]; // XA
	mxb = MOUSE_XYB[m_nimbus_mouse.m_mouse_pcx]; // XB
	mya = MOUSE_XYA[m_nimbus_mouse.m_mouse_pcy]; // YA
	myb = MOUSE_XYB[m_nimbus_mouse.m_mouse_pcy]; // YB

	// calculate interrupt state
	intstate_x = (mxb ^ mxa) ^ ((m_ay8910_a & 0x40) >> 6);
	intstate_y = (myb ^ mya) ^ ((m_ay8910_a & 0x80) >> 7);

	// Generate interrupts if enabled, otherwise return values in
	// mouse register
	if (MOUSE_INT_ENABLED(this))
	{
		if ((intstate_x==1) && (m_nimbus_mouse.m_intstate_x==0))
		{
			xint=mxa ? EXTERNAL_INT_MOUSE_XL : EXTERNAL_INT_MOUSE_XR;

			external_int(xint, true);
		}

		if ((intstate_y==1) && (m_nimbus_mouse.m_intstate_y==0))
		{
			yint=myb ? EXTERNAL_INT_MOUSE_YD : EXTERNAL_INT_MOUSE_YU;

			external_int(yint, true);
		}
	}
	else
	{
		m_nimbus_mouse.m_reg0a4 &= 0xF0;
		m_nimbus_mouse.m_reg0a4 |= ( mxb & 0x01) << 3; // XB
		m_nimbus_mouse.m_reg0a4 |= (~mxb & 0x01) << 2; // XA
		m_nimbus_mouse.m_reg0a4 |= (~myb & 0x01) << 1; // YA
		m_nimbus_mouse.m_reg0a4 |= ( myb & 0x01) << 0; // YB
	}

	// Update current mouse position
	m_nimbus_mouse.m_mouse_x = mouse_x;
	m_nimbus_mouse.m_mouse_y = mouse_y;

	// and interrupt state
	m_nimbus_mouse.m_intstate_x=intstate_x;
	m_nimbus_mouse.m_intstate_y=intstate_y;
}

void rmnimbus_state::mouse_js_reset()
{
	m_nimbus_mouse.m_mouse_x=128;
	m_nimbus_mouse.m_mouse_y=128;
	m_nimbus_mouse.m_mouse_pcx=0;
	m_nimbus_mouse.m_mouse_pcy=0;
	m_nimbus_mouse.m_intstate_x=0;
	m_nimbus_mouse.m_intstate_y=0;
	m_nimbus_mouse.m_reg0a4=0xC0;

	// Setup timer to poll the mouse
	m_nimbus_mouse.m_mouse_timer->adjust(attotime::zero, 0, attotime::from_hz(MOUSE_POLL_FREQUENCY));
}

uint8_t rmnimbus_state::nimbus_mouse_js_r()
{
	/*

	    bit     description

	    0       JOY 0-Up    or mouse XB
	    1       JOY 0-Down  or mouse XA
	    2       JOY 0-Left  or mouse YA
	    3       JOY 0-Right or mouse YB
	    4       JOY 0-b0    or mouse rbutton
	    5       JOY 0-b1    or mouse lbutton
	    6       ?? always reads 1
	    7       ?? always reads 1

	*/
	uint8_t result;
	//int pc=m_maincpu->_pc();

	if (m_io_config->read() & 0x01)
	{
		result=m_nimbus_mouse.m_reg0a4 | 0xC0;
		//logerror("mouse_js_r: pc=%05X, result=%02X\n",pc,result);
	}
	else
	{
		result = m_io_joystick0->read() | 0xC0;
	}

	return result;
}

// Clear mose latches
void rmnimbus_state::nimbus_mouse_js_w(uint8_t data)
{
	m_nimbus_mouse.m_reg0a4 = 0x00;
	//logerror("clear mouse latches\n");
}


/**********************************************************************
Parallel printer / User port.
The Nimbus parallel printer port card is almost identical to the circuit
in the BBC micro, so I have borrowed the driver code from the BBC :)

Port A output is buffered before being connected to the printer connector.
This means that they can only be operated as output lines.
CA1 is pulled high by a 4K7 resistor. CA1 normally acts as an acknowledge
line when a printer is used. CA2 is buffered so that it has become an open
collector output only. It usually acts as the printer strobe line.
***********************************************************************/

/* USER VIA 6522 port B is connected to the BBC user port */
void rmnimbus_state::nimbus_via_write_portb(uint8_t data)
{
}
