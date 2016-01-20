// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s3virge.c
 *
 * Implementation of the S3 Virge series of video card
 *
 * Current status:
 *  - Working on getting VESA video modes working better - 800x600 and higher skip every other line at
 *    8-bit depth, but are fine at 15/16-bit depth.
 *  - S3D is not implemented at all, so no 2D/3D acceleration yet.
 */

#include "s3virge.h"

#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)
#define LOG_REG        1

const device_type S3VIRGE = &device_creator<s3virge_vga_device>;
const device_type S3VIRGEDX = &device_creator<s3virgedx_vga_device>;
const device_type S3VIRGEDX1 = &device_creator<s3virgedx_rev1_vga_device>;

s3virge_vga_device::s3virge_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: s3_vga_device(mconfig, S3VIRGE, "S3 86C325", tag, owner, clock, "virge_vga", __FILE__)
{
}

s3virge_vga_device::s3virge_vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: s3_vga_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

s3virgedx_vga_device::s3virgedx_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: s3virge_vga_device(mconfig, S3VIRGEDX, "S3 86C375", tag, owner, clock, "virgedx_vga", __FILE__)
{
}

s3virgedx_vga_device::s3virgedx_vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: s3virge_vga_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

s3virgedx_rev1_vga_device::s3virgedx_rev1_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: s3virgedx_vga_device(mconfig, S3VIRGEDX1, "S3 86C375 (rev 1)", tag, owner, clock, "virgedx_r1", __FILE__)
{
}

void s3virge_vga_device::device_start()
{
	zero();

	int x;
	int i;
	for (i = 0; i < 0x100; i++)
		m_palette->set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.read_dipswitch = read8_delegate(); //read_dipswitch;
	vga.svga_intf.seq_regcount = 0x1c;
	vga.svga_intf.crtc_regcount = 0x19;
	vga.svga_intf.vram_size = 0x400000;
	vga.memory.resize(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);
	save_item(vga.memory,"Video RAM");
	save_pointer(vga.crtc.data,"CRTC Registers",0x100);
	save_pointer(vga.sequencer.data,"Sequencer Registers",0x100);
	save_pointer(vga.attribute.data,"Attribute Registers", 0x15);

	m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vga_device::vblank_timer_cb),this));

	memset(&s3, 0, sizeof(s3));
	// Initialise hardware graphics cursor colours, Windows 95 doesn't touch the registers for some reason
	for(x=0;x<4;x++)
	{
		s3.cursor_fg[x] = 0xff;
		s3.cursor_bg[x] = 0x00;
	}
	// set device ID
	s3.id_high = 0x56;  // CR2D
	s3.id_low = 0x31;   // CR2E
	s3.revision = 0x00; // CR2F  (value unknown)
	s3.id_cr30 = 0xe1;  // CR30
}

void s3virgedx_vga_device::device_start()
{
	s3virge_vga_device::device_start();

	// set device ID
	s3.id_high = 0x8a;  // CR2D
	s3.id_low = 0x01;   // CR2E
	s3.revision = 0x00; // CR2F  (value unknown)
	s3.id_cr30 = 0xe1;  // CR30
}

void s3virgedx_rev1_vga_device::device_start()
{
	s3virge_vga_device::device_start();

	// set device ID
	s3.id_high = 0x8a;  // CR2D
	s3.id_low = 0x01;   // CR2E
	s3.revision = 0x01; // CR2F
	s3.id_cr30 = 0xe1;  // CR30
}

void s3virge_vga_device::device_reset()
{
	vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are just assumed defaults.
	s3.strapping = 0x000f0912;
}

void s3virgedx_vga_device::device_reset()
{
	vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are just assumed defaults.
	s3.strapping = 0x000f0912;
}

void s3virgedx_rev1_vga_device::device_reset()
{
	vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are based on results from a Diamond Stealth 3D 2000 Pro (Virge/DX based)
	// bits 8-15 are still unknown, S3ID doesn't show config register 2 (CR37)
	s3.strapping = 0x0aff0912;
}

UINT8 s3virge_vga_device::s3_crtc_reg_read(UINT8 index)
{
	UINT8 res;

	if(index <= 0x18)
		res = crtc_reg_read(index);
	else
	{
		switch(index)
		{
			case 0x2d:
				res = s3.id_high;
				break;
			case 0x2e:
				res = s3.id_low;
				break;
			case 0x2f:
				res = s3.revision;
				break;
			case 0x30: // CR30 Chip ID/REV register
				res = s3.id_cr30;
				break;
			case 0x31:
				res = s3.memory_config;
				break;
			case 0x35:
				res = s3.crt_reg_lock;
				break;
			case 0x36:  // Configuration register 1
				res = s3.strapping & 0x000000ff;
				break;
			case 0x37:  // Configuration register 2
				res = (s3.strapping & 0x0000ff00) >> 8;
				break;
			case 0x38:
				res = s3.reg_lock1;
				break;
			case 0x39:
				res = s3.reg_lock2;
				break;
			case 0x42: // CR42 Mode Control
				res = s3.cr42 & 0x0f;  // bit 5 set if interlaced, leave it unset for now.
				break;
			case 0x43:
				res = s3.cr43;
				break;
			case 0x45:
				res = s3.cursor_mode;
				break;
			case 0x46:
				res = (s3.cursor_x & 0xff00) >> 8;
				break;
			case 0x47:
				res = s3.cursor_x & 0x00ff;
				break;
			case 0x48:
				res = (s3.cursor_y & 0xff00) >> 8;
				break;
			case 0x49:
				res = s3.cursor_y & 0x00ff;
				break;
			case 0x4a:
				res = s3.cursor_fg[s3.cursor_fg_ptr];
				s3.cursor_fg_ptr = 0;
				break;
			case 0x4b:
				res = s3.cursor_bg[s3.cursor_bg_ptr];
				s3.cursor_bg_ptr = 0;
				break;
			case 0x4c:
				res = (s3.cursor_start_addr & 0xff00) >> 8;
				break;
			case 0x4d:
				res = s3.cursor_start_addr & 0x00ff;
				break;
			case 0x4e:
				res = s3.cursor_pattern_x;
				break;
			case 0x4f:
				res = s3.cursor_pattern_y;
				break;
			case 0x51:
				res = (vga.crtc.start_addr_latch & 0x0c0000) >> 18;
				res |= ((svga.bank_w & 0x30) >> 2);
				break;
			case 0x55:
				res = s3.extended_dac_ctrl;
				break;
			case 0x5c:
				// if VGA dot clock is set to 3 (misc reg bits 2-3), then selected dot clock is read, otherwise read VGA clock select
				if((vga.miscellaneous_output & 0xc) == 0x0c)
					res = s3.cr42 & 0x0f;
				else
					res = (vga.miscellaneous_output & 0xc) >> 2;
				break;
			case 0x67:
				res = s3.ext_misc_ctrl_2;
				break;
			case 0x68:  // Configuration register 3
				res = (s3.strapping & 0x00ff0000) >> 16;
				break;
			case 0x69:
				res = vga.crtc.start_addr_latch >> 16;
				break;
			case 0x6a:
				res = svga.bank_r & 0x7f;
				break;
			case 0x6f: // Configuration register 4
				res = (s3.strapping & 0xff000000) >> 24;
				break;
			default:
				res = vga.crtc.data[index];
				//debugger_break(machine);
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void s3virge_vga_device::s3_define_video_mode()
{
	int divisor = 1;
	int xtal = (vga.miscellaneous_output & 0xc) ? XTAL_28_63636MHz : XTAL_25_1748MHz;
	double freq;

	if((vga.miscellaneous_output & 0xc) == 0x0c)
	{
		// Dot clock is set via SR12 and SR13
		// DCLK calculation
		freq = ((double)(s3.clk_pll_m+2) / (double)((s3.clk_pll_n+2)*(pow(2.0,s3.clk_pll_r)))) * 14.318; // clock between XIN and XOUT
		xtal = freq * 1000000;
		//printf("DCLK set to %dHz M=%i N=%i R=%i\n",xtal,s3.clk_pll_m,s3.clk_pll_n,s3.clk_pll_r);
	}

	if((s3.ext_misc_ctrl_2) >> 4)
	{
		svga.rgb8_en = 0;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb32_en = 0;
		switch((s3.ext_misc_ctrl_2) >> 4)
		{
			case 0x01: svga.rgb8_en = 1; break;
			case 0x03: svga.rgb15_en = 1; divisor = 2; break;
			case 0x05: svga.rgb16_en = 1; divisor = 2; break;
			case 0x0d: svga.rgb32_en = 1; divisor = 1; break;
			default: fatalerror("TODO: s3 video mode not implemented %02x\n",((s3.ext_misc_ctrl_2) >> 4));
		}
	}
	else
	{
		svga.rgb8_en = (s3.cr3a & 0x10) >> 4;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb32_en = 0;
	}
	if(s3.cr43 & 0x80)  // Horizontal clock doubling (techincally, doubles horizontal CRT parameters)
		divisor *= 2;
	recompute_params_clock(divisor, xtal);
}

void s3virge_vga_device::s3_crtc_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x18)
	{
		crtc_reg_write(index,data);
		s3_define_video_mode();
	}
	else
	{
		switch(index)
		{
			case 0x31: // CR31 Memory Configuration Register
				s3.memory_config = data;
				vga.crtc.start_addr_latch &= ~0x30000;
				vga.crtc.start_addr_latch |= ((data & 0x30) << 12);
				s3_define_video_mode();
				break;
			case 0x35:
				if((s3.reg_lock1 & 0xc) != 8 || ((s3.reg_lock1 & 0xc0) == 0)) // lock register
					return;
				s3.crt_reg_lock = data;
				svga.bank_w = data & 0xf;
				svga.bank_r = svga.bank_w;
				break;
			case 0x36:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xffffff00) | data;
					logerror("CR36: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x37:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xffff00ff) | (data << 8);
					logerror("CR37: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x38:
				s3.reg_lock1 = data;
				break;
			case 0x39:
				/* TODO: reg lock mechanism */
				s3.reg_lock2 = data;
				break;
			case 0x3a:
				s3.cr3a = data;
				break;
			case 0x40:
				s3.enable_s3d = data & 0x01;  // enable S3D registers
				break;
			case 0x42:
				s3.cr42 = data;  // bit 5 = interlace, bits 0-3 = dot clock (seems to be undocumented)
				break;
			case 0x43:
				s3.cr43 = data;  // bit 2 = bit 8 of offset register, but only if bits 4-5 of CR51 are 00h.
				vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x04) << 6);
				s3_define_video_mode();
				break;
/*
3d4h index 45h (R/W):  CR45 Hardware Graphics Cursor Mode
bit    0  HWGC ENB. Hardware Graphics Cursor Enable. Set to enable the
          HardWare Cursor in VGA and enhanced modes.
       1  (911/24) Delay Timing for Pattern Data Fetch
       2  (801/5,928) Hardware Cursor Horizontal Stretch 2. If set the cursor
           pixels are stretched horizontally to two bytes and items 0 and 1 of
           the fore/background stacks in 3d4h index 4Ah/4Bh are used.
       3  (801/5,928) Hardware Cursor Horizontal Stretch 3. If set the cursor
           pixels are stretched horizontally to three bytes and items 0,1 and
           2 of the fore/background stacks in 3d4h index 4Ah/4Bh are used.
     2-3  (805i,864/964) HWC-CSEL. Hardware Cursor Color Select.
            0: 4/8bit, 1: 15/16bt, 2: 24bit, 3: 32bit
          Note: So far I've had better luck with: 0: 8/15/16bit, 1: 32bit??
       4  (80x +) Hardware Cursor Right Storage. If set the cursor data is
           stored in the last 256 bytes of 4 1Kyte lines (4bits/pixel) or the
           last 512 bytes of 2 2Kbyte lines (8bits/pixel). Intended for
           1280x1024 modes where there are no free lines at the bottom.
       5  (928) Cursor Control Enable for Brooktree Bt485 DAC. If set and 3d4h
           index 55h bit 5 is set the HC1 output becomes the ODF and the HC0
           output becomes the CDE
          (964) BT485 ODF Selection for Bt485A RAMDAC. If set pin 185 (RS3
           /ODF) is the ODF output to a Bt485A compatible RamDAC (low for even
           fields and high for odd fields), if clear pin185 is the RS3 output.
 */
			case 0x45:
				s3.cursor_mode = data;
				break;
/*
3d4h index 46h M(R/W):  CR46/7 Hardware Graphics Cursor Origin-X
bit 0-10  The HardWare Cursor X position. For 64k modes this value should be
          twice the actual X co-ordinate.
 */
			case 0x46:
				s3.cursor_x = (s3.cursor_x & 0x00ff) | (data << 8);
				break;
			case 0x47:
				s3.cursor_x = (s3.cursor_x & 0xff00) | data;
				break;
/*
3d4h index 48h M(R/W):  CR48/9 Hardware Graphics Cursor Origin-Y
bit  0-9  (911/24) The HardWare Cursor Y position.
    0-10  (80x +) The HardWare Cursor Y position.
Note: The position is activated when the high byte of the Y coordinate (index
      48h) is written, so this byte should be written last (not 911/924 ?)
 */
			case 0x48:
				s3.cursor_y = (s3.cursor_y & 0x00ff) | (data << 8);
				break;
			case 0x49:
				s3.cursor_y = (s3.cursor_y & 0xff00) | data;
				break;

/*
3d4h index 4Ah (R/W):  Hardware Graphics Cursor Foreground Stack       (80x +)
bit  0-7  The Foreground Cursor color. Three bytes (4 for the 864/964) are
          stacked here. When the Cursor Mode register (3d4h index 45h) is read
          the stackpointer is reset. When a byte is written the byte is
          written into the current top of stack and the stackpointer is
          increased. The first byte written (item 0) is allways used, the
          other two(3) only when Hardware Cursor Horizontal Stretch (3d4h
          index 45h bit 2-3) is enabled.
 */
			case 0x4a:
				s3.cursor_fg[s3.cursor_fg_ptr++] = data;
				s3.cursor_fg_ptr %= 4;
				break;
/*
3d4h index 4Bh (R/W):  Hardware Graphics Cursor Background Stack       (80x +)
bit  0-7  The Background Cursor color. Three bytes (4 for the 864/964) are
          stacked here. When the Cursor Mode register (3d4h index 45h) is read
          the stackpointer is reset. When a byte is written the byte is
          written into the current top of stack and the stackpointer is
          increased. The first byte written (item 0) is allways used, the
          other two(3) only when Hardware Cursor Horizontal Stretch (3d4h
          index 45h bit 2-3) is enabled.
 */
			case 0x4b:
				s3.cursor_bg[s3.cursor_bg_ptr++] = data;
				s3.cursor_bg_ptr %= 4;
				break;
/*
3d4h index 4Ch M(R/W):  CR4C/D Hardware Graphics Cursor Storage Start Address
bit  0-9  (911,924) HCS_STADR. Hardware Graphics Cursor Storage Start Address
    0-11  (80x,928) HWGC_STA. Hardware Graphics Cursor Storage Start Address
    0-12  (864,964) HWGC_STA. Hardware Graphics Cursor Storage Start Address
          Address of the HardWare Cursor Map in units of 1024 bytes (256 bytes
          for planar modes). The cursor map is a 64x64 bitmap with 2 bits (A
          and B) per pixel. The map is stored as one word (16 bits) of bit A,
          followed by one word with the corresponding 16 B bits.
          The bits are interpreted as:
             A    B    MS-Windows:         X-11:
             0    0    Background          Screen data
             0    1    Foreground          Screen data
             1    0    Screen data         Background
             1    1    Inverted screen     Foreground
          The Windows/X11 switch is only available for the 80x +.
          (911/24) For 64k color modes the cursor is stored as one byte (8
            bits) of A bits, followed by the 8 B-bits, and each bit in the
            cursor should be doubled to provide a consistent cursor image.
          (801/5,928) For Hi/True color modes use the Horizontal Stretch bits
            (3d4h index 45h bits 2 and 3).
 */
		case 0x4c:
				s3.cursor_start_addr = (s3.cursor_start_addr & 0x00ff) | (data << 8);
				break;
			case 0x4d:
				s3.cursor_start_addr = (s3.cursor_start_addr & 0xff00) | data;
				break;
/*
3d4h index 4Eh (R/W):  CR4E HGC Pattern Disp Start X-Pixel Position
bit  0-5  Pattern Display Start X-Pixel Position.
 */
			case 0x4e:
				s3.cursor_pattern_x = data;
				break;
/*
3d4h index 4Fh (R/W):  CR4F HGC Pattern Disp Start Y-Pixel Position
bit  0-5  Pattern Display Start Y-Pixel Position.
 */
			case 0x4f:
				s3.cursor_pattern_y = data;
				break;
			case 0x51:
				vga.crtc.start_addr_latch &= ~0xc0000;
				vga.crtc.start_addr_latch |= ((data & 0x3) << 18);
				svga.bank_w = (svga.bank_w & 0xcf) | ((data & 0x0c) << 2);
				svga.bank_r = svga.bank_w;
				if((data & 0x30) != 0x00)
					vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x30) << 4);
				else
					vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((s3.cr43 & 0x04) << 6);
				s3_define_video_mode();
				break;
			case 0x53:
				s3.cr53 = data;
				break;
/*
3d4h index 55h (R/W):  Extended Video DAC Control Register             (80x +)
bit 0-1  DAC Register Select Bits. Passed to the RS2 and RS3 pins on the
         RAMDAC, allowing access to all 8 or 16 registers on advanced RAMDACs.
         If this field is 0, 3d4h index 43h bit 1 is active.
      2  Enable General Input Port Read. If set DAC reads are disabled and the
         STRD strobe for reading the General Input Port is enabled for reading
         while DACRD is active, if clear DAC reads are enabled.
      3  (928) Enable External SID Operation if set. If set video data is
           passed directly from the VRAMs to the DAC rather than through the
           VGA chip
      4  Hardware Cursor MS/X11 Mode. If set the Hardware Cursor is in X11
         mode, if clear in MS-Windows mode
      5  (80x,928) Hardware Cursor External Operation Mode. If set the two
          bits of cursor data ,is output on the HC[0-1] pins for the video DAC
          The SENS pin becomes HC1 and the MID2 pin becomes HC0.
      6  ??
      7  (80x,928) Disable PA Output. If set PA[0-7] and VCLK are tristated.
         (864/964) TOFF VCLK. Tri-State Off VCLK Output. VCLK output tri
          -stated if set
 */
			case 0x55:
				s3.extended_dac_ctrl = data;
				break;
/*
3d4h index 5Dh (R/W):  Extended Horizontal Overflow Register           (80x +)
bit    0  Horizontal Total bit 8. Bit 8 of the Horizontal Total register (3d4h
          index 0)
       1  Horizontal Display End bit 8. Bit 8 of the Horizontal Display End
          register (3d4h index 1)
       2  Start Horizontal Blank bit 8. Bit 8 of the Horizontal Start Blanking
          register (3d4h index 2).
       3  (864,964) EHB+64. End Horizontal Blank +64. If set the /BLANK pulse
           is extended by 64 DCLKs. Note: Is this bit 6 of 3d4h index 3 or
           does it really extend by 64 ?
       4  Start Horizontal Sync Position bit 8. Bit 8 of the Horizontal Start
          Retrace register (3d4h index 4).
       5  (864,964) EHS+32. End Horizontal Sync +32. If set the HSYNC pulse
           is extended by 32 DCLKs. Note: Is this bit 5 of 3d4h index 5 or
           does it really extend by 32 ?
       6  (928,964) Data Transfer Position bit 8. Bit 8 of the Data Transfer
            Position register (3d4h index 3Bh)
       7  (928,964) Bus-Grant Terminate Position bit 8. Bit 8 of the Bus Grant
            Termination register (3d4h index 5Fh).
*/
			case 0x5d:
				vga.crtc.horz_total = (vga.crtc.horz_total & 0xfeff) | ((data & 0x01) << 8);
				vga.crtc.horz_disp_end = (vga.crtc.horz_disp_end & 0xfeff) | ((data & 0x02) << 7);
				vga.crtc.horz_blank_start = (vga.crtc.horz_blank_start & 0xfeff) | ((data & 0x04) << 6);
				vga.crtc.horz_blank_end = (vga.crtc.horz_blank_end & 0xffbf) | ((data & 0x08) << 3);
				vga.crtc.horz_retrace_start = (vga.crtc.horz_retrace_start & 0xfeff) | ((data & 0x10) << 4);
				vga.crtc.horz_retrace_end = (vga.crtc.horz_retrace_end & 0xffdf) | (data & 0x20);
				s3_define_video_mode();
				break;
/*
3d4h index 5Eh (R/W):  Extended Vertical Overflow Register             (80x +)
bit    0  Vertical Total bit 10. Bit 10 of the Vertical Total register (3d4h
          index 6). Bits 8 and 9 are in 3d4h index 7 bit 0 and 5.
       1  Vertical Display End bit 10. Bit 10 of the Vertical Display End
          register (3d4h index 12h). Bits 8 and 9 are in 3d4h index 7 bit 1
          and 6
       2  Start Vertical Blank bit 10. Bit 10 of the Vertical Start Blanking
          register (3d4h index 15h). Bit 8 is in 3d4h index 7 bit 3 and bit 9
          in 3d4h index 9 bit 5
       4  Vertical Retrace Start bit 10. Bit 10 of the Vertical Start Retrace
          register (3d4h index 10h). Bits 8 and 9 are in 3d4h index 7 bit 2
          and 7.
       6  Line Compare Position bit 10. Bit 10 of the Line Compare register
          (3d4h index 18h). Bit 8 is in 3d4h index 7 bit 4 and bit 9 in 3d4h
          index 9 bit 6.
 */
			case 0x5e:
				vga.crtc.vert_total = (vga.crtc.vert_total & 0xfbff) | ((data & 0x01) << 10);
				vga.crtc.vert_disp_end = (vga.crtc.vert_disp_end & 0xfbff) | ((data & 0x02) << 9);
				vga.crtc.vert_blank_start = (vga.crtc.vert_blank_start & 0xfbff) | ((data & 0x04) << 8);
				vga.crtc.vert_retrace_start = (vga.crtc.vert_retrace_start & 0xfbff) | ((data & 0x10) << 6);
				vga.crtc.line_compare = (vga.crtc.line_compare & 0xfbff) | ((data & 0x40) << 4);
				s3_define_video_mode();
				break;
			case 0x67:
				s3.ext_misc_ctrl_2 = data;
				s3_define_video_mode();
				//printf("%02x X\n",data);
				break;
			case 0x68:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xff00ffff) | (data << 16);
					logerror("CR68: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x69:
				vga.crtc.start_addr_latch &= ~0x1f0000;
				vga.crtc.start_addr_latch |= ((data & 0x1f) << 16);
				s3_define_video_mode();
				break;
			case 0x6a:
				svga.bank_w = data & 0x3f;
				svga.bank_r = svga.bank_w;
				break;
			case 0x6f:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0x00ffffff) | (data << 24);
					logerror("CR6F: Strapping data = %08x\n",s3.strapping);
				}
				break;
			default:
				if(LOG_REG) logerror("S3: CR%02X write %02x\n",index,data);
				break;
		}
	}
}


READ8_MEMBER(s3virge_vga_device::port_03b0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				res = s3_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03b0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(s3virge_vga_device::port_03b0_w)
{
	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				s3_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03b0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(s3virge_vga_device::port_03c0_r)
{
	UINT8 res;

	switch(offset)
	{
		default:
			res = s3_vga_device::port_03c0_r(space,offset,mem_mask);
			break;
	}

	return res;
}

WRITE8_MEMBER(s3virge_vga_device::port_03c0_w)
{
	switch(offset)
	{
		default:
			s3_vga_device::port_03c0_w(space,offset,data,mem_mask);
			break;
	}
}

READ8_MEMBER(s3virge_vga_device::port_03d0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = s3_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03d0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(s3virge_vga_device::port_03d0_w)
{
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				s3_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03d0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(s3virge_vga_device::mem_r)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		UINT8 data;
		if(offset & 0x10000)
			return 0;
		data = 0xff;
		if(vga.sequencer.data[4] & 0x8)
		{
			if(offset + (svga.bank_r*0x10000) < vga.svga_intf.vram_size)
				data = vga.memory[offset + (svga.bank_r*0x10000)];
		}
		else
		{
			int i;

			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if(offset*4+i+(svga.bank_r*0x10000) < vga.svga_intf.vram_size)
						data |= vga.memory[offset*4+i+(svga.bank_r*0x10000)];
				}
			}
		}
		return data;
	}
	if((offset + (svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
		return vga_device::mem_r(space,offset,mem_mask);
	else
		return 0xff;
}

WRITE8_MEMBER(s3virge_vga_device::mem_w)
{
	// bit 4 of CR53 enables memory-mapped I/O
	if(s3.cr53 & 0x10)
	{
		// TODO
	}

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
	//  printf("%08x %02x (%02x %02x) %02X\n",offset,data,vga.sequencer.map_mask,svga.bank_w,(vga.sequencer.data[4] & 0x08));
		if(offset & 0x10000)
			return;
		if(vga.sequencer.data[4] & 0x8)
		{
			if((offset + (svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
				vga.memory[(offset + (svga.bank_w*0x10000))] = data;
		}
		else
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if((offset*4+i+(svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
						vga.memory[(offset*4+i+(svga.bank_w*0x10000))] = data;
				}
			}
		}
		return;
	}

	if((offset + (svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
		vga_device::mem_w(space,offset,data,mem_mask);
}
