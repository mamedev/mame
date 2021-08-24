// license:BSD-3-Clause
// copyright-holders:Mario Montminy
#include "emu.h"
#include "emuopts.h"
#include "rendutil.h"
#include "video/alt_vector.h"

#include <rapidjson/document.h>

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ALT_VECTOR_USB_DVG, alt_vector_device_usb_dvg, "alt_vector_usb_dvg", "ALT_VECTOR_USB_DVG")

// 0-15
#define DVG_RELEASE             0
#define DVG_BUILD               1
#define CMD_BUF_SIZE            0x20000
#define FLAG_COMPLETE           0x0
#define FLAG_RGB                0x1
#define FLAG_XY                 0x2
#define FLAG_EXIT               0x7
#define FLAG_CMD                0x5
#define FLAG_CMD_GET_DVG_INFO   0x1

#define FLAG_COMPLETE_MONOCHROME (1 << 28)

#define DVG_RES_MIN              0
#define DVG_RES_MAX              4095

#define SAVE_TO_FILE             0
#define SORT_VECTORS             0
#define MAX_VECTORS              0x10000
#define MAX_JSON_SIZE            512

// Defining region codes 
#define LEFT                     0x1
#define RIGHT                    0x2
#define BOTTOM                   0x4
#define TOP                      0x8

#define GAME_NONE                0
#define GAME_ARMORA              1
#define GAME_WARRIOR             2

const alt_vector_device_usb_dvg::game_info_t alt_vector_device_usb_dvg::s_games[] = {
	{"armora",   false, GAME_ARMORA,  true},
	{"armorap",  false, GAME_ARMORA,  true},
	{"armorar",  false, GAME_ARMORA,  true},
	{"asteroid", false, GAME_NONE,    true},  
	{"asteroi1", false, GAME_NONE,    true},   
	{"astdelux", false, GAME_NONE,    true},   
	{"astdelu1", false, GAME_NONE,    true},  
	{"llander",  false, GAME_NONE,    true},
	{"llander1", false, GAME_NONE,    true},
	{"cchasm",   false, GAME_NONE,    true},  
	{"cchasm1",  false, GAME_NONE,    true},      
	{"barrier",  false, GAME_NONE,    true},
	{"bzone",    false, GAME_NONE,    true},   
	{"bzone2",   false, GAME_NONE,    true},  
	{"bzonec",   false, GAME_NONE,    true},  
	{"redbaron", false, GAME_NONE,    true},
	{"omegrace", false, GAME_NONE,    true},        
	{"ripoff",   false, GAME_NONE,    true},   
	{"solarq",   false, GAME_NONE,    true},
	{"speedfrk", false, GAME_NONE,    true},  
	{"starhawk", false, GAME_NONE,    true},   
	{"sundance", false, GAME_NONE,    true},   
	{"tailg",    false, GAME_NONE,    true},  
	{"warrior",  false, GAME_WARRIOR, true}, 
	{"wotw",     false, GAME_NONE,    true},  
	{"spacewar", false, GAME_NONE,    true},      
	{"starcas",  false, GAME_NONE,    true},  
	{"starcas1", false, GAME_NONE,    true},   
	{"starcasp", false, GAME_NONE,    true},  
	{"starcase", false, GAME_NONE,    true},  
	{"starwars", true, GAME_NONE,     false},
	{"starwar1", true, GAME_NONE,     false},
	{"esb",      true, GAME_NONE,     false},
	{ 0 }
};

using namespace rapidjson;

//
// Function to compute region code for a point(x, y) 
//
uint32_t alt_vector_device_usb_dvg::compute_code(int32_t x, int32_t y)
{
	// initialized as being inside 
	uint32_t code = 0;

	if (x < m_clipx_min)      // to the left of rectangle 
		code |= LEFT;
	else if (x > m_clipx_max) // to the right of rectangle 
		code |= RIGHT;
	if (y < m_clipy_min)      // below the rectangle 
		code |= BOTTOM;
	else if (y > m_clipy_max) // above the rectangle 
		code |= TOP;

	return code;
}

//
// Cohen-Sutherland line-clipping algorithm.  Some games (such as starwars)
// generate coordinates outside the view window, so we need to clip them here.
//
uint32_t alt_vector_device_usb_dvg::line_clip(int32_t *pX1, int32_t *pY1, int32_t *pX2, int32_t *pY2)
{
	int32_t x = 0, y = 0, x1, y1, x2, y2;
	uint32_t accept, code1, code2, code_out;

	x1 = *pX1;
	y1 = *pY1;
	x2 = *pX2;
	y2 = *pY2;

	accept = 0;
	// Compute region codes for P1, P2 
	code1 = compute_code(x1, y1);
	code2 = compute_code(x2, y2);

	while (1) 
	{
		if ((code1 == 0) && (code2 == 0)) 
		{
			// If both endpoints lie within rectangle 
			accept = 1;
			break;
		}
		else if (code1 & code2) 
		{
			// If both endpoints are outside rectangle, 
			// in same region 
			break;
		}
		else {
			// Some segment of line lies within the 
			// rectangle 
			// At least one endpoint is outside the 
			// rectangle, pick it. 
			if (code1 != 0) 
			{
			    code_out = code1;
			}
			else 
			{
			    code_out = code2;
			}

			// Find intersection point; 
			// using formulas y = y1 + slope * (x - x1), 
			// x = x1 + (1 / slope) * (y - y1) 
			if (code_out & TOP) 
			{
			    // point is above the clip rectangle 
			    x = x1 + (x2 - x1) * (m_clipy_max - y1) / (y2 - y1);
			    y = m_clipy_max;
			}
			else if (code_out & BOTTOM) 
			{
			    // point is below the rectangle 
			    x = x1 + (x2 - x1) * (m_clipy_min - y1) / (y2 - y1);
			    y = m_clipy_min;
			}
			else if (code_out & RIGHT) 
			{
			    // point is to the right of rectangle 
			    y = y1 + (y2 - y1) * (m_clipx_max - x1) / (x2 - x1);
			    x = m_clipx_max;
			}
			else if (code_out & LEFT) 
			{
			    // point is to the left of rectangle 
			    y = y1 + (y2 - y1) * (m_clipx_min - x1) / (x2 - x1);
			    x = m_clipx_min;
			}

			// Now intersection point x, y is found 
			// We replace point outside rectangle 
			// by intersection point 
			if (code_out == code1) 
			{
			    x1 = x;
			    y1 = y;
			    code1 = compute_code(x1, y1);
			}
			else 
			{
			    x2 = x;
			    y2 = y;
			    code2 = compute_code(x2, y2);
			}
		}
	}
	*pX1 = x1;
	*pY1 = y1;
	*pX2 = x2;
	*pY2 = y2;
	return accept;
}  



void alt_vector_device_usb_dvg::cmd_vec_postproc()
{
	int32_t  last_x = 0;
	int32_t  last_y = 0;
	int32_t  x0, y0, x1, y1;
	uint32_t add;
	uint32_t i;

	m_out_vec_cnt = 0;

	for (i = 0; i < m_in_vec_cnt ; i++) 
	{
		x0 = m_in_vec_list[i].x0;
		y0 = m_in_vec_list[i].y0;
		x1 = m_in_vec_list[i].x1;
		y1 = m_in_vec_list[i].y1;

		if (m_in_vec_list[i].screen_coords) 
		{
			transform_and_scale_coords(&x0, &y0);
			transform_and_scale_coords(&x1, &y1);
		}
		add = line_clip(&x0, &y0, &x1, &y1);
		if (add) {
			if (last_x != x0 || last_y != y0) 
			{
			    // Disconnect detected. Insert a blank vector.
			    m_out_vec_list[m_out_vec_cnt].x0 = last_x;
			    m_out_vec_list[m_out_vec_cnt].y0 = last_y;
			    m_out_vec_list[m_out_vec_cnt].x1 = x0;
			    m_out_vec_list[m_out_vec_cnt].y1 = y0;
			    m_out_vec_list[m_out_vec_cnt].color = rgb_t(0, 0, 0);
			    m_out_vec_cnt++;
			}
			m_out_vec_list[m_out_vec_cnt].x0 = last_x;
			m_out_vec_list[m_out_vec_cnt].y0 = last_y;
			m_out_vec_list[m_out_vec_cnt].x1 = x1;
			m_out_vec_list[m_out_vec_cnt].y1 = y1;
			m_out_vec_list[m_out_vec_cnt].color = m_in_vec_list[i].color;
			m_out_vec_cnt++;
		}
		last_x = x1;
		last_y = y1;
	}
}



//
// Reset the indexes to the vector list and command buffer.
//
void alt_vector_device_usb_dvg::cmd_reset(uint32_t initial)
{
	m_in_vec_last_x  = 0;
	m_in_vec_last_y  = 0;
	m_in_vec_cnt     = 0;
	m_out_vec_cnt    = 0;
	m_cmd_offs       = 0;
}



//
// Add a vector to the input vector list.  We don't keep
// blank vectors.  They will be added later.
//
void alt_vector_device_usb_dvg::cmd_add_vec(int x, int y, rgb_t color, bool screen_coords) 
{
	uint32_t   blank, add;
	int32_t    x0, y0, x1, y1;

	x0 = m_in_vec_last_x;
	y0 = m_in_vec_last_y;
	x1 = x;
	y1 = y;

    add = 1;
	blank = (color.r() == 0) && (color.g() == 0) && (color.b() == 0);
	if ((x1 == x0) && (y1 == y0) && blank)
	{
		add = 0;
	}
	if (m_exclude_blank_vectors) 
	{
		if (add) 
		{
			add = !blank;
		}
	}
	if (add) 
	{
		if (m_in_vec_cnt < MAX_VECTORS) 
		{
			if (m_in_vec_cnt) 
			{
			    m_in_vec_list[m_in_vec_cnt - 1].next = &m_in_vec_list[m_in_vec_cnt];
			}
			m_in_vec_list[m_in_vec_cnt].x0            = x0;
			m_in_vec_list[m_in_vec_cnt].y0            = y0; 
			m_in_vec_list[m_in_vec_cnt].x1            = x1;
			m_in_vec_list[m_in_vec_cnt].y1            = y1;
			m_in_vec_list[m_in_vec_cnt].color         = color;
			m_in_vec_list[m_in_vec_cnt].screen_coords = screen_coords;
			m_in_vec_cnt++;
		}
	}
	m_in_vec_last_x = x;
	m_in_vec_last_y = y;
}


//
// Add commands to the serial buffer to send.  When we detect
// color changes, we add a command to update it.
// As an optimization there is a blank flag in the XY coord which
// allows USB-DVG to blank the beam without updating the RGB color DACs.
//
void alt_vector_device_usb_dvg::cmd_add_point(int x, int y, rgb_t color)
{
	uint32_t   cmd;
	uint32_t   color_change;
	uint32_t   blank;

	blank = (color.r() == 0) && (color.g() == 0) && (color.b() == 0);
	if (!blank) 
	{
		color_change = ((m_last_r != color.r()) || (m_last_g != color.g()) || (m_last_b != color.b()));        
		if (color_change) 
		{
			m_last_r = color.r();
			m_last_g = color.g();
			m_last_b = color.b();
			cmd = (FLAG_RGB << 29) | ((m_last_r & 0xff) << 16) | ((m_last_g & 0xff) << 8)| (m_last_b & 0xff);
			if (m_cmd_offs <= (CMD_BUF_SIZE - 8)) 
			{
			    m_cmd_buf[m_cmd_offs++] = cmd >> 24;               
			    m_cmd_buf[m_cmd_offs++] = cmd >> 16;
			    m_cmd_buf[m_cmd_offs++] = cmd >>  8;
			    m_cmd_buf[m_cmd_offs++] = cmd >>  0;
			}
		}
	}
	transform_final(&x, &y);
	cmd = (FLAG_XY << 29) | ((blank & 0x1) << 28) | ((x & 0x3fff) << 14) | (y & 0x3fff); 
	if (m_cmd_offs <= (CMD_BUF_SIZE - 8)) 
	{
		m_cmd_buf[m_cmd_offs++] = cmd >> 24;   
		m_cmd_buf[m_cmd_offs++] = cmd >> 16;
		m_cmd_buf[m_cmd_offs++] = cmd >>  8;
		m_cmd_buf[m_cmd_offs++] = cmd >>  0;
	}
}


//
//  Convert the MAME-supplied coordinates to USB-DVG-compatible coordinates.
// 
void  alt_vector_device_usb_dvg::transform_and_scale_coords(int *px, int *py)
{
	float x, y;

	x = (*px >> 16) + (*px & 0xffff) / 65536.0;
	x *= m_xscale;
	y = (*py >> 16) + (*py & 0xffff) / 65536.0;
	y *= m_yscale;
	*px = x;
	*py = y;
}


//
// Determine game type, orientation
//
int alt_vector_device_usb_dvg::determine_game_settings() 
{
	uint32_t i;
	Document d;

	get_dvg_info();

	d.Parse(reinterpret_cast<const char *>(&m_json_buf[0]));
	m_vertical_display = d["vertical"].GetBool();

	if (machine().config().gamedrv().flags & machine_flags::SWAP_XY) 
	{
		m_swap_xy = true;
	}
	if (machine().config().gamedrv().flags & machine_flags::FLIP_Y) 
	{
		m_flip_y = false;
	}
	if (machine().config().gamedrv().flags & machine_flags::FLIP_X) 
	{
		m_flip_x = true;
	}  

	m_bw_game               = false;
	m_artwork               = GAME_NONE;
	m_exclude_blank_vectors = false;
	if (!strcmp(emulator_info::get_appname_lower(), "mess"))
	{
		m_bw_game = 1;
	}
	else
	{
		for (i = 0 ; s_games[i].name ; i++) 
		{
			if (!strcmp(machine().system().name, s_games[i].name)) 
			{
			    m_artwork               = s_games[i].artwork;
			    m_exclude_blank_vectors = s_games[i].exclude_blank_vectors;
			    m_bw_game               = s_games[i].bw_game;
			    break;
			}
		}
	}
	return 0;    
}


//
//  Compute a final transformation to coordinates (flip and swap).
//
void alt_vector_device_usb_dvg::transform_final(int *px, int *py)
{
	int x, y, tmp;
	x = *px;
	y = *py;

	if (m_swap_xy) 
	{
		tmp = x;
		x = y;
		y = tmp;
	}
	if (m_flip_x) 
	{
		x = DVG_RES_MAX - x;
	}
	if (m_flip_y) 
	{
		y = DVG_RES_MAX - y;
	}
	x = std::clamp(x, 0, DVG_RES_MAX); 
	y = std::clamp(y, 0, DVG_RES_MAX);

	if (m_vertical_display) 
	{
		if (m_swap_xy) 
		{
			// Vertical on vertical display
		}
		else 
		{
			// Horizontal on vertical display
			y = 512 + (0.75 * y);
		}
	}
	else 
	{
		if (m_swap_xy) 
		{
			// Vertical on horizontal display
			x = 512 + (0.75 * x);
		}
		else 
		{
			// Horizontal on horizontal display
		}
	}

	*px = x;
	*py = y;
}

//
//  Read responses from USB-DVG via the virtual serial port over USB.
// 
int alt_vector_device_usb_dvg::serial_read(uint8_t *buf, int size)
{
	int result = size;
	uint32_t read = 0;

	m_serial->read(buf, 0, size, read);
	if (read != size) {
		result = -1;
	}
	return result;
}

//
//  Send commands to USB-DVG via the virtual serial port over USB.
// 
int alt_vector_device_usb_dvg::serial_write(uint8_t *buf, int size) 
{
	int result = -1;
	uint32_t written = 0, chunk, total;
	
	total = size;
	while (size) 
	{
		chunk = std::min(size, 512);
		m_serial->write(buf, 0, chunk, written);
		if (written != chunk) 
		{
			goto END;
		}
		buf  += chunk;
		size -= chunk;
	}
	result = total;
END:
   return result;
}


//
// Preprocess and send commands to USB-DVG over the virtual serial port.
//
int alt_vector_device_usb_dvg::serial_send()
{
	int      result = -1;
	uint32_t cmd;
	uint32_t i;

	cmd_vec_postproc();

	for (i = 0 ; i < m_out_vec_cnt ; i++) 
	{
		cmd_add_point(m_out_vec_list[i].x1, m_out_vec_list[i].y1, m_out_vec_list[i].color);
	}   
	cmd = (FLAG_COMPLETE << 29); 
	if (m_bw_game) 
	{
		cmd |= FLAG_COMPLETE_MONOCHROME;
	}
	m_cmd_buf[m_cmd_offs++] = cmd >> 24;
	m_cmd_buf[m_cmd_offs++] = cmd >> 16;
	m_cmd_buf[m_cmd_offs++] = cmd >>  8;
	m_cmd_buf[m_cmd_offs++] = cmd >>  0;     

	result  = serial_write(&m_cmd_buf[0], m_cmd_offs);
	cmd_reset(0);
	m_last_r = m_last_g = m_last_b = -1;
	return result;
}



alt_vector_device_usb_dvg::alt_vector_device_usb_dvg(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) 
   : alt_vector_device_base(mconfig, ALT_VECTOR_USB_DVG, tag, owner, clock),
	m_exclude_blank_vectors(false),
	m_xmin(0),
	m_xmax(0),
	m_ymin(0),
	m_ymax(0),
	m_xscale(1.0),
	m_yscale(1.0),
	m_cmd_offs(0), 
	m_swap_xy(false),
	m_flip_x(false),
	m_flip_y(true),
	m_clipx_min(DVG_RES_MIN),
	m_clipx_max(DVG_RES_MAX),
	m_clipy_min(DVG_RES_MIN),
	m_clipy_max(DVG_RES_MAX),
	m_last_r(-1),
	m_last_g(-1),
	m_last_b(-1),
	m_artwork(0),
	m_bw_game(false),
	m_in_vec_cnt(0),
	m_in_vec_last_x(0),
	m_in_vec_last_y(0),
	m_out_vec_cnt(0),
	m_vertical_display(0),
	m_dual(false),
	m_json_length(0)
{
}



void alt_vector_device_usb_dvg::device_start()
{
	m_dual = machine().config().options().alt_vector_dual();

	int i;
	uint64_t size = 0;
	std::error_condition filerr = osd_file::open(machine().config().options().alt_vector_port(), OPEN_FLAG_READ | OPEN_FLAG_WRITE, m_serial, size);
	if (filerr)
	{
		fprintf(stderr, "alt_vector_device_usb_dvg: error: osd_file::open failed.\n");
	    ::exit(1);
	}

	m_cmd_buf      = std::make_unique<uint8_t[]>(CMD_BUF_SIZE);
	m_in_vec_list  = std::make_unique<vector_t[]>(MAX_VECTORS);
	m_out_vec_list = std::make_unique<vector_t[]>(MAX_VECTORS);
	m_json_buf     = std::make_unique<uint8_t[]>(MAX_JSON_SIZE);
	m_json_length  = 0;
	m_gamma_table  = std::make_unique<float[]>(256);

	for (i = 0; i < 256; i++)
	{
	    m_gamma_table[i] = apply_brightness_contrast_gamma_fp(i, machine().options().brightness(), machine().options().contrast(), machine().options().gamma());
	}

	determine_game_settings();     
	m_clipx_min = DVG_RES_MIN;
	m_clipy_min = DVG_RES_MIN;
	m_clipx_max = DVG_RES_MAX;
	m_clipy_max = DVG_RES_MAX;
}

void alt_vector_device_usb_dvg::device_stop()
{
	uint32_t cmd;

    cmd = (FLAG_EXIT << 29); 
    m_cmd_offs = 0;
	m_cmd_buf[m_cmd_offs++] = cmd >> 24;    
	m_cmd_buf[m_cmd_offs++] = cmd >> 16;
	m_cmd_buf[m_cmd_offs++] = cmd >>  8;
	m_cmd_buf[m_cmd_offs++] = cmd >>  0;
	serial_write(&m_cmd_buf[0], m_cmd_offs);
}

void alt_vector_device_usb_dvg::device_reset()
{
}

int alt_vector_device_usb_dvg::add_point(int x, int y, rgb_t color, int intensity)
{ 
	intensity = std::clamp(intensity, 0, 255);
	if (intensity == 0) {
		color.set_r(0);
		color.set_g(0);
		color.set_b(0);
	}
	else {
		float cscale = m_gamma_table[intensity];
		color.set_r(cscale * color.r());
		color.set_g(cscale * color.g());
		color.set_b(cscale * color.b());
	}
	cmd_add_vec(x, y, color, true);    
	return m_dual ? 0 : 1;
}

void alt_vector_device_usb_dvg::get_dvg_info()
{
	uint32_t cmd;
	uint8_t  cmd_buf[4];
	int      result;
	uint32_t version, major, minor;

	if (m_json_length) {
		return;
	}
	cmd = (FLAG_CMD << 29) | FLAG_CMD_GET_DVG_INFO;
	sscanf(emulator_info::get_build_version(), "%u.%u", &major, &minor);
	version = (((minor / 1000) % 10) << 12) | (((minor / 100) % 10) << 8) | (((minor / 10) % 10) << 4) | (minor % 10);
	cmd |= version << 8;
	cmd_buf[0] = cmd >> 24;
	cmd_buf[1] = cmd >> 16;
	cmd_buf[2] = cmd >> 8;
	cmd_buf[3] = cmd >> 0;
	serial_write(cmd_buf, 4);
	result = serial_read(reinterpret_cast<uint8_t *>(&cmd), sizeof(cmd));
	if (result < 0) goto END;
	result = serial_read(reinterpret_cast<uint8_t *>(&m_json_length), sizeof(m_json_length));
	if (result < 0) goto END;
	m_json_length = std::min(m_json_length, MAX_JSON_SIZE - 1);
	result = serial_read(&m_json_buf[0], m_json_length);
	if (result < 0) goto END;
END:;
}

int alt_vector_device_usb_dvg::update(screen_device &screen, const rectangle &cliprect)
{
	rgb_t color = rgb_t(108, 108, 108);
	rgb_t black = rgb_t(0, 0, 0);
	int x0, y0, x1, y1;

	m_xmin = screen.visible_area().min_x;
	m_xmax = screen.visible_area().max_x;
	m_ymin = screen.visible_area().min_y;
	m_ymax = screen.visible_area().max_y;

	if (m_xmax == 0) 
	{
		m_xmax = 1;
	}
	if (m_ymax == 0) 
	{
		m_ymax = 1;
	}
	m_xscale = (DVG_RES_MAX + 1.0) / (m_xmax - m_xmin);
	m_yscale = (DVG_RES_MAX + 1.0) / (m_ymax - m_ymin);  

	x0 = 0;
	y0 = 0;
	x1 = (cliprect.right() - cliprect.left()) * m_xscale;
	y1 = (cliprect.bottom() - cliprect.top()) * m_yscale;

	// printf("clip: (%d,%d)-(%d,%d)\n", x0, y0, x1, y1);
	// Make sure the clip coordinates fall within the display coordinates.
	x0 = std::clamp(x0, 0, DVG_RES_MAX);
	y0 = std::clamp(y0, 0, DVG_RES_MAX);	
	x1 = std::clamp(x1, 0, DVG_RES_MAX);
	y1 = std::clamp(y1, 0, DVG_RES_MAX);	

	m_clipx_min = x0;
	m_clipy_min = y0;
	m_clipx_max = x1;
	m_clipy_max = y1;

	if (m_in_vec_cnt) 
	{
		switch (m_artwork) 
		{
			case GAME_ARMORA:
				// Upper Right Quadrant
				// Outer structure
				cmd_add_vec(3446, 2048, black, false);
				cmd_add_vec(3958, 2224, color, false);
				cmd_add_vec(3958, 3059, color, false);
				cmd_add_vec(3323, 3059, color, false);
				cmd_add_vec(3323, 3225, color, false);
				cmd_add_vec(3194, 3225, color, false);
				cmd_add_vec(3194, 3393, color, false);
				cmd_add_vec(3067, 3393, color, false);
				cmd_add_vec(3067, 3901, color, false);
				cmd_add_vec(2304, 3901, color, false);
				cmd_add_vec(2304, 3225, color, false);
				cmd_add_vec(2048, 3225, color, false);
				// Center structure
				cmd_add_vec(2048, 2373, black, false);
				cmd_add_vec(2562, 2738, color, false);
				cmd_add_vec(2430, 2738, color, false);
				cmd_add_vec(2430, 2893, color, false);
				cmd_add_vec(2306, 2893, color, false);
				cmd_add_vec(2306, 3065, color, false);
				cmd_add_vec(2048, 3065, color, false);
				// Big structure
				cmd_add_vec(2938, 2209, black, false);
				cmd_add_vec(3198, 2383, color, false);
				cmd_add_vec(3706, 2383, color, false);
				cmd_add_vec(3706, 2738, color, false);
				cmd_add_vec(2938, 2738, color, false);
				cmd_add_vec(2938, 2209, color, false);
				// Small structure
				cmd_add_vec(2551, 3055, black, false);
				cmd_add_vec(2816, 3590, color, false);
				cmd_add_vec(2422, 3590, color, false);
				cmd_add_vec(2422, 3231, color, false);
				cmd_add_vec(2555, 3231, color, false);
				cmd_add_vec(2555, 3055, color, false);
				// Upper Left Quadrant
				// Outer structure
				cmd_add_vec(649, 2048, black, false);
				cmd_add_vec(137, 2224, color, false);
				cmd_add_vec(137, 3059, color, false);
				cmd_add_vec(772, 3059, color, false);
				cmd_add_vec(772, 3225, color, false);
				cmd_add_vec(901, 3225, color, false);
				cmd_add_vec(901, 3393, color, false);
				cmd_add_vec(1028, 3393, color, false);
				cmd_add_vec(1028, 3901, color, false);
				cmd_add_vec(1792, 3901, color, false);
				cmd_add_vec(1792, 3225, color, false);
				cmd_add_vec(2048, 3225, color, false);
				// Center structure
				cmd_add_vec(2048, 2373, black, false);
				cmd_add_vec(1533, 2738, color, false);
				cmd_add_vec(1665, 2738, color, false);
				cmd_add_vec(1665, 2893, color, false);
				cmd_add_vec(1789, 2893, color, false);
				cmd_add_vec(1789, 3065, color, false);
				cmd_add_vec(2048, 3065, color, false);
				// Big structure
				cmd_add_vec(1157, 2209, black, false);
				cmd_add_vec(897, 2383, color, false);
				cmd_add_vec(389, 2383, color, false);
				cmd_add_vec(389, 2738, color, false);
				cmd_add_vec(1157, 2738, color, false);
				cmd_add_vec(1157, 2209, color, false);
				// Small structure
				cmd_add_vec(1544, 3055, black, false);
				cmd_add_vec(1280, 3590, color, false);
				cmd_add_vec(1673, 3590, color, false);
				cmd_add_vec(1673, 3231, color, false);
				cmd_add_vec(1540, 3231, color, false);
				cmd_add_vec(1540, 3055, color, false);
				// Lower Right Quadrant
				// Outer structure
				cmd_add_vec(3446, 2048, black, false);
				cmd_add_vec(3958, 1871, color, false);
				cmd_add_vec(3958, 1036, color, false);
				cmd_add_vec(3323, 1036, color, false);
				cmd_add_vec(3323, 870, color, false);
				cmd_add_vec(3194, 870, color, false);
				cmd_add_vec(3194, 702, color, false);
				cmd_add_vec(3067, 702, color, false);
				cmd_add_vec(3067, 194, color, false);
				cmd_add_vec(2304, 194, color, false);
				cmd_add_vec(2304, 870, color, false);
				cmd_add_vec(2048, 870, color, false);
				// Center structure
				cmd_add_vec(2048, 1722, black, false);
				cmd_add_vec(2562, 1357, color, false);
				cmd_add_vec(2430, 1357, color, false);
				cmd_add_vec(2430, 1202, color, false);
				cmd_add_vec(2306, 1202, color, false);
				cmd_add_vec(2306, 1030, color, false);
				cmd_add_vec(2048, 1030, color, false);
				// Big structure
				cmd_add_vec(2938, 1886, black, false);
				cmd_add_vec(3198, 1712, color, false);
				cmd_add_vec(3706, 1712, color, false);
				cmd_add_vec(3706, 1357, color, false);
				cmd_add_vec(2938, 1357, color, false);
				cmd_add_vec(2938, 1886, color, false);
				// Small structure
				cmd_add_vec(2551, 1040, black, false);
				cmd_add_vec(2816, 505, color, false);
				cmd_add_vec(2422, 505, color, false);
				cmd_add_vec(2422, 864, color, false);
				cmd_add_vec(2555, 864, color, false);
				cmd_add_vec(2555, 1040, color, false);
				// Lower Left Quadrant
				// Outer structure
				cmd_add_vec(649, 2048, black, false);
				cmd_add_vec(137, 1871, color, false);
				cmd_add_vec(137, 1036, color, false);
				cmd_add_vec(772, 1036, color, false);
				cmd_add_vec(772, 870, color, false);
				cmd_add_vec(901, 870, color, false);
				cmd_add_vec(901, 702, color, false);
				cmd_add_vec(1028, 702, color, false);
				cmd_add_vec(1028, 194, color, false);
				cmd_add_vec(1792, 194, color, false);
				cmd_add_vec(1792, 870, color, false);
				cmd_add_vec(2048, 870, color, false);
				// Center structure
				cmd_add_vec(2048, 1722, black, false);
				cmd_add_vec(1533, 1357, color, false);
				cmd_add_vec(1665, 1357, color, false);
				cmd_add_vec(1665, 1202, color, false);
				cmd_add_vec(1789, 1202, color, false);
				cmd_add_vec(1789, 1030, color, false);
				cmd_add_vec(2048, 1030, color, false);
				// Big structure
				cmd_add_vec(1157, 1886, black, false);
				cmd_add_vec(897, 1712, color, false);
				cmd_add_vec(389, 1712, color, false);
				cmd_add_vec(389, 1357, color, false);
				cmd_add_vec(1157, 1357, color, false);
				cmd_add_vec(1157, 1886, color, false);
				// Small structure
				cmd_add_vec(1544, 1040, black, false);
				cmd_add_vec(1280, 505, color, false);
				cmd_add_vec(1673, 505, color, false);
				cmd_add_vec(1673, 864, color, false);
				cmd_add_vec(1540, 864, color, false);
				cmd_add_vec(1540, 1040, color, false);
				break;

			case GAME_WARRIOR:
			    cmd_add_vec(1187, 2232, black, false);
			    cmd_add_vec(1863, 2232, color, false);
			    cmd_add_vec(1187, 1372, black, false);
			    cmd_add_vec(1863, 1372, color, false);
			    cmd_add_vec(1187, 2232, black, false);
			    cmd_add_vec(1187, 1372, color, false);
			    cmd_add_vec(1863, 2232, black, false);
			    cmd_add_vec(1863, 1372, color, false);
			    cmd_add_vec(2273, 2498, black, false);
			    cmd_add_vec(2949, 2498, color, false);
			    cmd_add_vec(2273, 1658, black, false);
			    cmd_add_vec(2949, 1658, color, false);
			    cmd_add_vec(2273, 2498, black, false);
			    cmd_add_vec(2273, 1658, color, false);
			    cmd_add_vec(2949, 2498, black, false);
			    cmd_add_vec(2949, 1658, color, false);
			    break;
		}
		serial_send();
	}
	return 0;
}
