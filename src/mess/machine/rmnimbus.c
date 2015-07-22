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

#include "debugger.h"
#include "debug/debugcon.h"
#include "imagedev/flopdrv.h"
#include "includes/rmnimbus.h"



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

#define IPC_OUT_ADDR        0X01
#define IPC_OUT_READ_PEND   0X02
#define IPC_OUT_BYTE_AVAIL  0X04

#define IPC_IN_ADDR         0X01
#define IPC_IN_BYTE_AVAIL   0X02
#define IPC_IN_READ_PEND    0X04

/* IO unit */

#define DISK_INT_ENABLE         0x01
#define MSM5205_INT_ENABLE      0x04
#define MOUSE_INT_ENABLE        0x08
#define PC8031_INT_ENABLE       0x10

enum
{
	MOUSE_PHASE_STATIC = 0,
	MOUSE_PHASE_POSITIVE,
	MOUSE_PHASE_NEGATIVE
};

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

#define DEBUG_SET(flags)    ((m_debug_machine & (flags))==(flags))
#define DEBUG_SET_STATE(flags)    ((state->m_debug_machine & (flags))==(flags))

#define DEBUG_NONE          0x0000000
#define DECODE_BIOS         0x0000002
#define DECODE_BIOS_RAW     0x0000004
#define DECODE_DOS21        0x0000008

/* Nimbus sub-bios structures for debugging */

struct t_area_params
{
	UINT16  ofs_brush;
	UINT16  seg_brush;
	UINT16  ofs_data;
	UINT16  seg_data;
	UINT16  count;
};

struct t_plot_string_params
{
	UINT16  ofs_font;
	UINT16  seg_font;
	UINT16  ofs_data;
	UINT16  seg_data;
	UINT16  x;
	UINT16  y;
	UINT16  length;
};

struct t_nimbus_brush
{
	UINT16  style;
	UINT16  style_index;
	UINT16  colour1;
	UINT16  colour2;
	UINT16  transparency;
	UINT16  boundary_spec;
	UINT16  boundary_colour;
	UINT16  save_colour;
};


static void nimbus_debug(running_machine &machine, int ref, int params, const char *param[]);

static int instruction_hook(device_t &device, offs_t curpc);
static void decode_subbios(device_t *device,offs_t pc, UINT8 raw_flag);
static void decode_dos21(device_t *device,offs_t pc);
static void decode_dssi_generic(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_fill_area(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_plot_character_string(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_set_new_clt(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_plonk_char(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_rw_sectors(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);

void rmnimbus_state::external_int(UINT8 vector, bool state)
{
	if(!state && (vector != m_vector))
		return;

	m_vector = vector;

	m_maincpu->int0_w(state);
}

READ8_MEMBER(rmnimbus_state::cascade_callback)
{
	m_maincpu->int0_w(0);
	return m_vector;
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
	m_via->write_pb0(1);
	m_via->write_pb1(1);
	m_via->write_pb2(1);
	m_via->write_pb3(1);
	m_via->write_pb4(1);
	m_via->write_pb5(1);
	m_via->write_pb6(1);
	m_via->write_pb7(1);
}

void rmnimbus_state::machine_start()
{
	m_nimbus_mouse.m_mouse_timer=timer_alloc(TIMER_MOUSE);

	/* setup debug commands */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine(), "nimbus_debug", CMDFLAG_NONE, 0, 0, 1, nimbus_debug);

		/* set up the instruction hook */
		m_maincpu->debug()->set_instruction_hook(instruction_hook);
	}

	m_debug_machine=DEBUG_NONE;
	m_fdc->dden_w(0);
}

static void nimbus_debug(running_machine &machine, int ref, int params, const char *param[])
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	if(params>0)
	{
		int temp;
		sscanf(param[0],"%d",&temp); state->m_debug_machine = temp;
	}
	else
	{
		debug_console_printf(machine,"Error usage : nimbus_debug <debuglevel>\n");
		debug_console_printf(machine,"Current debuglevel=%02X\n",state->m_debug_machine);
	}
}

/*-----------------------------------------------
    instruction_hook - per-instruction hook
-----------------------------------------------*/

static int instruction_hook(device_t &device, offs_t curpc)
{
	rmnimbus_state  *state = device.machine().driver_data<rmnimbus_state>();
	address_space   &space = device.memory().space(AS_PROGRAM);
	UINT8           *addr_ptr;

	addr_ptr = (UINT8*)space.get_read_ptr(curpc);

	if ((addr_ptr !=NULL) && (addr_ptr[0]==0xCD))
	{
		if(DEBUG_SET_STATE(DECODE_BIOS) && (addr_ptr[1]==0xF0))
		{
			if(DEBUG_SET_STATE(DECODE_BIOS_RAW))
				decode_subbios(&device,curpc,1);
			else
				decode_subbios(&device,curpc,0);
		}

		if(DEBUG_SET_STATE(DECODE_DOS21) && (addr_ptr[1]==0x21))
			decode_dos21(&device,curpc);
	}

	return 0;
}

#define set_type(type_name)     sprintf(type_str,type_name)
#define set_drv(drv_name)       sprintf(drv_str,drv_name)
#define set_func(func_name)     sprintf(func_str,func_name)

static void decode_subbios(device_t *device,offs_t pc, UINT8 raw_flag)
{
	char    type_str[80];
	char    drv_str[80];
	char    func_str[80];

	void (*dump_dssi)(device_t *,UINT16, UINT16 ,UINT8) = NULL;

	device_t *cpu = device->machine().device(MAINCPU_TAG);

	UINT16  ax = cpu->state().state_int(I8086_AX);
	UINT16  bx = cpu->state().state_int(I8086_BX);
	UINT16  cx = cpu->state().state_int(I8086_CX);
	UINT16  ds = cpu->state().state_int(I8086_DS);
	UINT16  si = cpu->state().state_int(I8086_SI);

	// *** TEMP Don't show f_enquire_display_line calls !
	if((cx==6) && (ax==43))
		return;
	// *** END TEMP

	if(!raw_flag)
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

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_initialise_unit"); break;
				case 2  : set_func("f_pseudo_init_unit"); break;
				case 3  : set_func("f_get_device_status"); break;
				case 4  : set_func("f_read_n_sectors"); dump_dssi=&decode_dssi_f_rw_sectors; break;
				case 5  : set_func("f_write_n_sectors"); dump_dssi=&decode_dssi_f_rw_sectors;  break;
				case 6  : set_func("f_verify_n_sectors"); break;
				case 7  : set_func("f_media_check"); break;
				case 8  : set_func("f_recalibrate"); break;
				case 9  : set_func("f_motors_off"); break;
			}
			dump_dssi=&decode_dssi_f_rw_sectors;

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
				case 6  : set_func("f_polyline"); dump_dssi=&decode_dssi_f_fill_area;   break;
				case 7  : set_func("f_fill_area"); dump_dssi=&decode_dssi_f_fill_area; break;
				case 8  : set_func("f_flood_fill_area"); break;
				case 9  : set_func("f_plot_character_string"); dump_dssi=&decode_dssi_f_plot_character_string; break;
				case 10 : set_func("f_define_graphics_clipping_area"); break;
				case 11 : set_func("f_enquire_clipping_area_limits"); break;
				case 12 : set_func("f_select_graphics_clipping_area"); break;
				case 13 : set_func("f_enq_selctd_graphics_clip_area"); break;
				case 14 : set_func("f_set_clt_element"); break;
				case 15 : set_func("f_enquire_clt_element"); break;
				case 16 : set_func("f_set_new_clt"); dump_dssi=&decode_dssi_f_set_new_clt; break;
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
						case 1  : set_func("f_plonk_char"); dump_dssi=decode_dssi_f_plonk_char; break;
						case 2  : set_func("f_plonk_cursor"); break;
						case 3  : set_func("f_kill_cursor"); break;
						case 4  : set_func("f_scroll"); break;
						case 5  : set_func("f_width"); dump_dssi=decode_dssi_generic;break;
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

	if(raw_flag)
	{
		if(dump_dssi!=NULL)
			dump_dssi(device,ds,si,raw_flag);
	}
	else
	{
		logerror("Type=%s, Driver=%s, Function=%s\n",type_str,drv_str,func_str);

		if(dump_dssi!=NULL)
			dump_dssi(device,ds,si,raw_flag);
		logerror("=======================================================================\n");
	}
}

static inline void *get_dssi_ptr(address_space &space, UINT16   ds, UINT16 si)
{
	int             addr;

	addr=((ds<<4)+si);
//    OUTPUT_SEGOFS("DS:SI",ds,si);

	return space.get_read_ptr(addr);
}

static void decode_dssi_generic(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	rmnimbus_state  *state = device->machine().driver_data<rmnimbus_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);
	UINT16  *params;
	int     count;

	if(raw_flag)
		return;

	params=(UINT16  *)get_dssi_ptr(space,ds,si);

	for(count=0; count<10; count++)
		logerror("%04X ",params[count]);

	logerror("\n");
}


static void decode_dssi_f_fill_area(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	rmnimbus_state  *state = device->machine().driver_data<rmnimbus_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);

	UINT16          *addr_ptr;
	t_area_params   *area_params;
	t_nimbus_brush  *brush;
	int             cocount;

	area_params = (t_area_params   *)get_dssi_ptr(space,ds,si);

	if (!raw_flag)
		OUTPUT_SEGOFS("SegBrush:OfsBrush",area_params->seg_brush,area_params->ofs_brush);

	brush=(t_nimbus_brush  *)space.get_read_ptr(LINEAR_ADDR(area_params->seg_brush,area_params->ofs_brush));

	if(raw_flag)
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
		logerror("transparency=%04X,   boundry_spec=%04X\n",brush->transparency,brush->boundary_spec);
		logerror("boundry colour=%04X, save colour=%04X\n",brush->boundary_colour,brush->save_colour);


		OUTPUT_SEGOFS("SegData:OfsData",area_params->seg_data,area_params->ofs_data);
	}

	addr_ptr = (UINT16 *)space.get_read_ptr(LINEAR_ADDR(area_params->seg_data,area_params->ofs_data));
	for(cocount=0; cocount < area_params->count; cocount++)
	{
		if(raw_flag)
		{
			if(cocount!=(area_params->count-1))
				logerror("%04X, %04X, ",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
			else
				logerror("%04X, %04X ",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
		}
		else
			logerror("x=%d y=%d\n",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
	}

	if(raw_flag)
		logerror("\n");
}

static void decode_dssi_f_plot_character_string(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	rmnimbus_state  *state = device->machine().driver_data<rmnimbus_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);

	UINT8                   *char_ptr;
	t_plot_string_params    *plot_string_params;
	int                     charno;

	if(raw_flag)
		return;

	plot_string_params=(t_plot_string_params   *)get_dssi_ptr(space,ds,si);

	OUTPUT_SEGOFS("SegFont:OfsFont",plot_string_params->seg_font,plot_string_params->ofs_font);
	OUTPUT_SEGOFS("SegData:OfsData",plot_string_params->seg_data,plot_string_params->ofs_data);

	logerror("x=%d, y=%d, length=%d\n",plot_string_params->x,plot_string_params->y,plot_string_params->length);

	char_ptr=(UINT8*)space.get_read_ptr(LINEAR_ADDR(plot_string_params->seg_data,plot_string_params->ofs_data));

	if (plot_string_params->length==0xFFFF)
		logerror("%s",char_ptr);
	else
		for(charno=0;charno<plot_string_params->length;charno++)
			logerror("%c",char_ptr[charno]);

	logerror("\n");
}

static void decode_dssi_f_set_new_clt(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	rmnimbus_state  *state = device->machine().driver_data<rmnimbus_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);
	UINT16  *new_colours;
	int     colour;
	new_colours=(UINT16  *)get_dssi_ptr(space,ds,si);

	if(raw_flag)
		return;

	OUTPUT_SEGOFS("SegColours:OfsColours",ds,si);

	for(colour=0;colour<16;colour++)
		logerror("colour #%02X=%04X\n",colour,new_colours[colour]);

}

static void decode_dssi_f_plonk_char(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	rmnimbus_state  *state = device->machine().driver_data<rmnimbus_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);
	UINT16  *params;
	params=(UINT16  *)get_dssi_ptr(space,ds,si);

	if(raw_flag)
		return;

	OUTPUT_SEGOFS("SegParams:OfsParams",ds,si);

	logerror("plonked_char=%c\n",params[0]);
}

static void decode_dssi_f_rw_sectors(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	rmnimbus_state  *state = device->machine().driver_data<rmnimbus_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);
	UINT16  *params;
	int     param_no;

	if(raw_flag)
		return;

	params=(UINT16  *)get_dssi_ptr(space,ds,si);

	for(param_no=0;param_no<16;param_no++)
		logerror("%04X ",params[param_no]);

	logerror("\n");
}

static void decode_dos21(device_t *device,offs_t pc)
{
	device_t *cpu = device->machine().device(MAINCPU_TAG);

	UINT16  ax = cpu->state().state_int(I8086_AX);
	UINT16  bx = cpu->state().state_int(I8086_BX);
	UINT16  cx = cpu->state().state_int(I8086_CX);
	UINT16  dx = cpu->state().state_int(I8086_DX);
	UINT16  cs = cpu->state().state_int(I8086_CS);
	UINT16  ds = cpu->state().state_int(I8086_DS);
	UINT16  es = cpu->state().state_int(I8086_ES);
	UINT16  ss = cpu->state().state_int(I8086_SS);

	UINT16  si = cpu->state().state_int(I8086_SI);
	UINT16  di = cpu->state().state_int(I8086_DI);
	UINT16  bp = cpu->state().state_int(I8086_BP);

	logerror("=======================================================================\n");
	logerror("DOS Int 0x21 call at %05X\n",pc);
	logerror("AX=%04X, BX=%04X, CX=%04X, DX=%04X\n",ax,bx,cx,dx);
	logerror("CS=%04X, DS=%04X, ES=%04X, SS=%04X\n",cs,ds,es,ss);
	logerror("SI=%04X, DI=%04X, BP=%04X\n",si,di,bp);
	logerror("=======================================================================\n");
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
	address_space &space = machine().device( MAINCPU_TAG)->memory().space( AS_PROGRAM );
	int     ramsize = m_ram->size();
	int     ramblock = 0;
	int     blockno;
	char    bank[10];
	UINT8   *ram    = &m_ram->pointer()[0];
	UINT8   *map_blocks[3];
	UINT8   *map_base;
	int     map_blockno;
	int     block_ofs;

	UINT8   ramsel = (m_mcu_reg080 & 0x1F);

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
	map_blocks[1]  = (ramblocks[ramblock][1].blocksize==0) ? NULL : &ram[ramblocks[ramblock][1].blockbase*1024];
	map_blocks[2]  = (ramblocks[ramblock][2].blocksize==0) ? NULL : &ram[ramblocks[ramblock][2].blockbase*1024];

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
			(map_blocks[map_blockno]!=NULL))
		{
			map_base=(ramsel==0x07) ? map_blocks[map_blockno] : &map_blocks[map_blockno][block_ofs*1024];

			membank(bank)->set_base(map_base);
			space.install_readwrite_bank(memmap[blockno].start, memmap[blockno].end, bank);
			//if(LOG_RAM) logerror(", base=%X\n",(int)map_base);
		}
		else
		{
			space.nop_readwrite(memmap[blockno].start, memmap[blockno].end);
			if(LOG_RAM) logerror("NOP\n");
		}
	}
}

READ8_MEMBER(rmnimbus_state::nimbus_mcu_r)
{
	return m_mcu_reg080;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_mcu_w)
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

	external_int(m_z80sio->m1_r(), state);
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

UINT8 rmnimbus_state::fdc_driveno(UINT8 drivesel)
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

READ8_MEMBER(rmnimbus_state::scsi_r)
{
	int result = 0;

	int pc=space.device().safe_pc();
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
			result |= !m_scsi_msg << 3;
			if(floppy)
			{
				result |= FDC_MOTOR() << 2;
				result |= (!floppy->idx_r()) << 1;
				result |= floppy->ready_r() << 0;
			}
			break;
		case 0x08 :
			result = m_scsi_data_in->read();
			hdc_post_rw();
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
WRITE8_MEMBER(rmnimbus_state::fdc_ctl_w)
{
	UINT8 reg400_old = m_nimbus_drives.reg400;
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
	if((data & HDC_DRQ_MASK) && (~reg400_old & HDC_DRQ_MASK))
		hdc_drq(true);
}

/*
    0x410 write bits

    0   SCSI reset
    1   SCSI SEL
    2   SCSI IRQ Enable
*/

WRITE8_MEMBER(rmnimbus_state::scsi_w)
{
	int pc=space.device().safe_pc();

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
}

void rmnimbus_state::check_scsi_irq()
{
	nimbus_fdc_intrq_w(m_scsi_io && m_scsi_cd && m_scsi_req && m_scsi_iena);
}

WRITE_LINE_MEMBER(rmnimbus_state::write_scsi_iena)
{
	m_scsi_iena = state;
	check_scsi_irq();
}

void rmnimbus_state::hdc_post_rw()
{
	if(m_scsi_req)
		m_scsibus->write_ack(1);
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
	m_scsi_msg = state;
}

WRITE_LINE_MEMBER( rmnimbus_state::write_scsi_req )
{
	int last = m_scsi_req;
	m_scsi_req = state;

	if (state)
	{
		if (!m_scsi_cd && !last)
		{
			hdc_drq(true);
		}
	}
	else
	{
		hdc_drq(false);
		m_scsibus->write_ack(0);
	}
	check_scsi_irq();
}

/* 8031/8051 Peripheral controler 80186 side */

void rmnimbus_state::pc8031_reset()
{
	logerror("peripheral controler reset\n");

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

READ8_MEMBER(rmnimbus_state::nimbus_pc8031_r)
{
	int pc=space.device().safe_pc();
	UINT8   result;

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

WRITE8_MEMBER(rmnimbus_state::nimbus_pc8031_w)
{
	int pc=space.device().safe_pc();

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

/* 8031/8051 Peripheral controler 8031/8051 side */

READ8_MEMBER(rmnimbus_state::nimbus_pc8031_iou_r)
{
	int pc=space.device().safe_pc();
	UINT8   result = 0;

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

WRITE8_MEMBER(rmnimbus_state::nimbus_pc8031_iou_w)
{
	int pc=space.device().safe_pc();

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

READ8_MEMBER(rmnimbus_state::nimbus_pc8031_port_r)
{
	int pc=space.device().safe_pc();
	UINT8   result = 0;

	if(LOG_PC8031_PORT)
		logerror("8031: PCPORTR %04X read of %04X returns %02X\n",pc,offset,result);

	switch(offset)
	{
		case 0x01:
			result = (m_eeprom_bits & ~4) | (m_eeprom->do_read() << 2);
			break;
	}

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_pc8031_port_w)
{
	int pc=space.device().safe_pc();

	switch (offset)
	{
		case 0x01:
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
			break;
	}

	if(LOG_PC8031_PORT)
		logerror("8031 PCPORTW %04X write of %02X to %04X\n",pc,data,offset);
}



/* IO Unit */
READ8_MEMBER(rmnimbus_state::nimbus_iou_r)
{
	int pc=space.device().safe_pc();
	UINT8   result=0;

	if(offset==0)
	{
		result=m_iou_reg092;
	}

	if(LOG_IOU)
		logerror("Nimbus IOUR %08X read of %04X returns %02X\n",pc,(offset*2)+0x92,result);

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_iou_w)
{
	int pc=space.device().safe_pc();

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

	m_last_playmode = MSM5205_S48_4B;
	m_msm->playmode_w(m_last_playmode);

	m_ay8910_a=0;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_sound_ay8910_porta_w)
{
	m_msm->data_w(data);

	// Mouse code needs a copy of this.
	m_ay8910_a=data;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_sound_ay8910_portb_w)
{
	if ((data & 0x07) != m_last_playmode)
	{
		m_last_playmode = (data & 0x07);
		m_msm->playmode_w(m_last_playmode);
	}
}

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_msm5205_vck)
{
	if(m_iou_reg092 & MSM5205_INT_ENABLE)
		external_int(EXTERNAL_INT_MSM5205,state);
}

static const int MOUSE_XYA[3][4] = { { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 } };
static const int MOUSE_XYB[3][4] = { { 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 1, 1, 0, 0 } };
//static const int MOUSE_XYA[4] = { 1, 1, 0, 0 };
//static const int MOUSE_XYB[4] = { 0, 1, 1, 0 };

void rmnimbus_state::mouse_js_reset()
{
	m_nimbus_mouse.m_mouse_px=0;
	m_nimbus_mouse.m_mouse_py=0;
	m_nimbus_mouse.m_mouse_x=128;
	m_nimbus_mouse.m_mouse_y=128;
	m_nimbus_mouse.m_mouse_pc=0;
	m_nimbus_mouse.m_mouse_pcx=0;
	m_nimbus_mouse.m_mouse_pcy=0;
	m_nimbus_mouse.m_intstate_x=0;
	m_nimbus_mouse.m_intstate_y=0;
	m_nimbus_mouse.m_reg0a4=0xC0;

	// Setup timer to poll the mouse
	m_nimbus_mouse.m_mouse_timer->adjust(attotime::zero, 0, attotime::from_hz(1000));
}

void rmnimbus_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	UINT8   x = 0;
	UINT8   y = 0;
//  int     pc=m_maincpu->pc();

	UINT8   intstate_x;
	UINT8   intstate_y;
	int     xint;
	int     yint;

	m_nimbus_mouse.m_reg0a4 = ioport(MOUSE_BUTTON_TAG)->read() | 0xC0;
	x = ioport(MOUSEX_TAG)->read();
	y = ioport(MOUSEY_TAG)->read();

	UINT8   mxa;
	UINT8   mxb;
	UINT8   mya;
	UINT8   myb;

	//logerror("poll_mouse()\n");

	if (x == m_nimbus_mouse.m_mouse_x)
	{
		m_nimbus_mouse.m_mouse_px = MOUSE_PHASE_STATIC;
	}
	else if (x > m_nimbus_mouse.m_mouse_x)
	{
		m_nimbus_mouse.m_mouse_px = MOUSE_PHASE_POSITIVE;
	}
	else if (x < m_nimbus_mouse.m_mouse_x)
	{
		m_nimbus_mouse.m_mouse_px = MOUSE_PHASE_NEGATIVE;
	}

	if (y == m_nimbus_mouse.m_mouse_y)
	{
		m_nimbus_mouse.m_mouse_py = MOUSE_PHASE_STATIC;
	}
	else if (y > m_nimbus_mouse.m_mouse_y)
	{
		m_nimbus_mouse.m_mouse_py = MOUSE_PHASE_POSITIVE;
	}
	else if (y < m_nimbus_mouse.m_mouse_y)
	{
		m_nimbus_mouse.m_mouse_py = MOUSE_PHASE_NEGATIVE;
	}

	switch (m_nimbus_mouse.m_mouse_px)
	{
		case MOUSE_PHASE_STATIC     : break;
		case MOUSE_PHASE_POSITIVE   : m_nimbus_mouse.m_mouse_pcx++; break;
		case MOUSE_PHASE_NEGATIVE   : m_nimbus_mouse.m_mouse_pcx--; break;
	}
	m_nimbus_mouse.m_mouse_pcx &= 0x03;

	switch (m_nimbus_mouse.m_mouse_py)
	{
		case MOUSE_PHASE_STATIC     : break;
		case MOUSE_PHASE_POSITIVE   : m_nimbus_mouse.m_mouse_pcy++; break;
		case MOUSE_PHASE_NEGATIVE   : m_nimbus_mouse.m_mouse_pcy--; break;
	}
	m_nimbus_mouse.m_mouse_pcy &= 0x03;

//  mxb = MOUSE_XYB[state.m_mouse_px][state->m_mouse_pcx]; // XB
//  mxa = MOUSE_XYA[state.m_mouse_px][state->m_mouse_pcx]; // XA
//  mya = MOUSE_XYA[state.m_mouse_py][state->m_mouse_pcy]; // YA
//  myb = MOUSE_XYB[state.m_mouse_py][state->m_mouse_pcy]; // YB

	mxb = MOUSE_XYB[1][m_nimbus_mouse.m_mouse_pcx]; // XB
	mxa = MOUSE_XYA[1][m_nimbus_mouse.m_mouse_pcx]; // XA
	mya = MOUSE_XYA[1][m_nimbus_mouse.m_mouse_pcy]; // YA
	myb = MOUSE_XYB[1][m_nimbus_mouse.m_mouse_pcy]; // YB

	if ((m_nimbus_mouse.m_mouse_py!=MOUSE_PHASE_STATIC) || (m_nimbus_mouse.m_mouse_px!=MOUSE_PHASE_STATIC))
	{
//        logerror("mouse_px=%02X, mouse_py=%02X, mouse_pcx=%02X, mouse_pcy=%02X\n",
//              state.m_mouse_px,state->m_mouse_py,state->m_mouse_pcx,state->m_mouse_pcy);

//        logerror("mxb=%02x, mxa=%02X (mxb ^ mxa)=%02X, (ay8910_a & 0xC0)=%02X, (mxb ^ mxa) ^ ((ay8910_a & 0x80) >> 7)=%02X\n",
//              mxb,mxa, (mxb ^ mxa) , (state.m_ay8910_a & 0xC0), (mxb ^ mxa) ^ ((state->m_ay8910_a & 0x40) >> 6));
	}

	intstate_x = (mxb ^ mxa) ^ ((m_ay8910_a & 0x40) >> 6);
	intstate_y = (myb ^ mya) ^ ((m_ay8910_a & 0x80) >> 7);

	if (MOUSE_INT_ENABLED(this))
	{
		if ((intstate_x==1) && (m_nimbus_mouse.m_intstate_x==0))
//        if (intstate_x!=state.m_intstate_x)
		{
			xint=mxa ? EXTERNAL_INT_MOUSE_XR : EXTERNAL_INT_MOUSE_XL;

			external_int(xint, true);

//            logerror("Xint:%02X, mxb=%02X\n",xint,mxb);
		}

		if ((intstate_y==1) && (m_nimbus_mouse.m_intstate_y==0))
//        if (intstate_y!=state.m_intstate_y)
		{
			yint=myb ? EXTERNAL_INT_MOUSE_YU : EXTERNAL_INT_MOUSE_YD;

			external_int(yint, true);
//            logerror("Yint:%02X, myb=%02X\n",yint,myb);
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

	m_nimbus_mouse.m_mouse_x = x;
	m_nimbus_mouse.m_mouse_y = y;

	if ((m_nimbus_mouse.m_mouse_py!=MOUSE_PHASE_STATIC) || (m_nimbus_mouse.m_mouse_px!=MOUSE_PHASE_STATIC))
	{
//        logerror("pc=%05X, reg0a4=%02X, reg092=%02X, ay_a=%02X, x=%02X, y=%02X, px=%02X, py=%02X, intstate_x=%02X, intstate_y=%02X\n",
//                 pc,state.m_reg0a4,state->m_iou_reg092,state->m_ay8910_a,state->m_mouse_x,state->m_mouse_y,state->m_mouse_px,state->m_mouse_py,intstate_x,intstate_y);
	}

	m_nimbus_mouse.m_intstate_x=intstate_x;
	m_nimbus_mouse.m_intstate_y=intstate_y;
}

READ8_MEMBER(rmnimbus_state::nimbus_mouse_js_r)
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
	UINT8 result;
	//int pc=m_maincpu->_pc();

	if (ioport("config")->read() & 0x01)
	{
		result=m_nimbus_mouse.m_reg0a4;
		//logerror("mouse_js_r: pc=%05X, result=%02X\n",pc,result);
	}
	else
	{
		result=ioport(JOYSTICK0_TAG)->read_safe(0xff);
	}

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_mouse_js_w)
{
}

/**********************************************************************
Paralell printer / User port.
The Nimbus paralell printer port card is almost identical to the circuit
in the BBC micro, so I have borrowed the driver code from the BBC :)

Port A output is buffered before being connected to the printer connector.
This means that they can only be operated as output lines.
CA1 is pulled high by a 4K7 resistor. CA1 normally acts as an acknowledge
line when a printer is used. CA2 is buffered so that it has become an open
collector output only. It usially acts as the printer strobe line.
***********************************************************************/

/* USER VIA 6522 port B is connected to the BBC user port */
WRITE8_MEMBER(rmnimbus_state::nimbus_via_write_portb)
{
}
