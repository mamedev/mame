// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#include "emu.h"
#include "pc_vga_s3.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

// TODO: remove this enum
enum
{
	IBM8514_IDLE = 0,
	IBM8514_DRAWING_RECT,
	IBM8514_DRAWING_LINE,
	IBM8514_DRAWING_BITBLT,
	IBM8514_DRAWING_PATTERN,
	IBM8514_DRAWING_SSV_1,
	IBM8514_DRAWING_SSV_2,
	MACH8_DRAWING_SCAN
};


DEFINE_DEVICE_TYPE(S3_VGA,     s3_vga_device,     "s3_vga",     "S3 Graphics VGA")

s3_vga_device::s3_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s3_vga_device(mconfig, S3_VGA, tag, owner, clock)
{
}

s3_vga_device::s3_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
{
}

void s3_vga_device::device_add_mconfig(machine_config &config)
{
	IBM8514A(config, "8514a", 0).set_vga_owner();
}

TIMER_CALLBACK_MEMBER(s3_vga_device::vblank_timer_cb)
{
	// not sure if this is correct, but XF86_S3 seems to expect the viewport scrolling to be faster
	if(s3.memory_config & 0x08)
		vga.crtc.start_addr = vga.crtc.start_addr_latch << 2;
	else
		vga.crtc.start_addr = vga.crtc.start_addr_latch;
	vga.attribute.pel_shift = vga.attribute.pel_shift_latch;
	m_vblank_timer->adjust( screen().time_until_pos(vga.crtc.vert_blank_start + vga.crtc.vert_blank_end) );
}

void s3_vga_device::device_start()
{
	svga_device::device_start();
	memset(&s3, 0, sizeof(s3));

	// Initialise hardware graphics cursor colours, Windows 95 doesn't touch the registers for some reason
	for(int x = 0; x < 4; x++)
	{
		s3.cursor_fg[x] = 0xff;
		s3.cursor_bg[x] = 0x00;
	}
	m_8514 = subdevice<ibm8514a_device>("8514a");
	// set device ID
	s3.id_high = 0x88;  // CR2D
	s3.id_low = 0x11;   // CR2E
	s3.revision = 0x00; // CR2F
	s3.id_cr30 = 0xe1;  // CR30
}

void s3_vga_device::device_reset()
{
	vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are just assumed defaults.
	s3.strapping = 0x000f0b1e;
	s3.sr10 = 0x42;
	s3.sr11 = 0x41;
}

uint16_t s3_vga_device::offset()
{
	//popmessage("Offset: %04x  %s %s %s",vga.crtc.offset,vga.crtc.dw?"DW":"--",vga.crtc.word_mode?"BYTE":"WORD",(s3.memory_config & 0x08)?"31":"--");
	if(s3.memory_config & 0x08)
		return vga.crtc.offset << 3;
	return vga_device::offset();
}

void s3_vga_device::s3_define_video_mode()
{
	int divisor = 1;
	int xtal = ((vga.miscellaneous_output & 0xc) ? XTAL(28'636'363) : XTAL(25'174'800)).value();
	double freq;

	if((vga.miscellaneous_output & 0xc) == 0x0c)
	{
		// DCLK calculation
		freq = ((double)(s3.clk_pll_m+2) / (double)((s3.clk_pll_n+2)*(pow(2.0,s3.clk_pll_r)))) * 14.318; // clock between XIN and XOUT
		xtal = freq * 1000000;
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
			default: fatalerror("TODO: S3 colour mode not implemented %02x\n",((s3.ext_misc_ctrl_2) >> 4));
		}
	}
	else
	{
		svga.rgb8_en = (s3.memory_config & 8) >> 3;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb32_en = 0;
	}
	recompute_params_clock(divisor, xtal);
}

uint8_t s3_vga_device::crtc_reg_read(uint8_t index)
{
	uint8_t res;

	if(index <= 0x18)
		res = svga_device::crtc_reg_read(index);
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
				res = s3.strapping & 0x000000ff;  // PCI (not really), Fast Page Mode DRAM
				break;
			case 0x37:  // Configuration register 2
				res = (s3.strapping & 0x0000ff00) >> 8;  // enable chipset, 64k BIOS size, internal DCLK/MCLK
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
				s3.cursor_fg_ptr = 0;
				s3.cursor_bg_ptr = 0;
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
				res = s3.cursor_fg[s3.cursor_fg_ptr++];
				s3.cursor_fg_ptr %= 4;
				break;
			case 0x4b:
				res = s3.cursor_bg[s3.cursor_bg_ptr++];
				s3.cursor_bg_ptr %= 4;
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
				res |= ((vga.crtc.offset & 0x0300) >> 4);
				break;
			case 0x53:
				res = s3.cr53;
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
				res = (s3.strapping & 0x00ff0000) >> 16;  // no /CAS,/OE stretch time, 32-bit data bus size
				break;
			case 0x69:
				res = vga.crtc.start_addr_latch >> 16;
				break;
			case 0x6a:
				res = svga.bank_r & 0x7f;
				break;
			case 0x6f: // Configuration register 4 (Trio64V+)
				res = (s3.strapping & 0xff000000) >> 24;  // LPB(?) mode, Serial port I/O at port 0xe8, Serial port I/O disabled (MMIO only), no WE delay
				break;
			default:
				res = vga.crtc.data[index];
				//machine().debug_break();
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void s3_vga_device::crtc_reg_write(uint8_t index, uint8_t data)
{
	if(index <= 0x18)
		svga_device::crtc_reg_write(index,data);
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
					LOG("CR36: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x37:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xffff00ff) | (data << 8);
					LOG("CR37: Strapping data = %08x\n",s3.strapping);
				}
				break;
			case 0x38:
				s3.reg_lock1 = data;
				break;
			case 0x39:
				/* TODO: reg lock mechanism */
				s3.reg_lock2 = data;
				break;
			case 0x40:
				s3.enable_8514 = data & 0x01;  // enable 8514/A registers (x2e8, x6e8, xae8, xee8)
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
				popmessage("HW Cursor Data Address %04x\n",s3.cursor_start_addr);
				break;
			case 0x4d:
				s3.cursor_start_addr = (s3.cursor_start_addr & 0xff00) | data;
				popmessage("HW Cursor Data Address %04x\n",s3.cursor_start_addr);
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
				break;
			case 0x68:
				if(s3.reg_lock2 == 0xa5)
				{
					s3.strapping = (s3.strapping & 0xff00ffff) | (data << 16);
					LOG("CR68: Strapping data = %08x\n",s3.strapping);
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
					LOG("CR6F: Strapping data = %08x\n",s3.strapping);
				}
				break;
			default:
				LOG( "S3: 3D4 index %02x write %02x\n",index,data);
				break;
		}
	}
}

uint8_t s3_vga_device::s3_seq_reg_read(uint8_t index)
{
	uint8_t res = 0xff;

	if(index <= 0x0c)
		res = vga.sequencer.data[index];
	else
	{
		switch(index)
		{
		case 0x10:
			res = s3.sr10;
			break;
		case 0x11:
			res = s3.sr11;
			break;
		case 0x12:
			res = s3.sr12;
			break;
		case 0x13:
			res = s3.sr13;
			break;
		case 0x15:
			res = s3.sr15;
			break;
		case 0x17:
			res = s3.sr17;  // CLKSYN test register
			s3.sr17--;  // who knows what it should return, docs only say it defaults to 0, and is reserved for testing of the clock synthesiser
			break;
		}
	}

	return res;
}

void s3_vga_device::s3_seq_reg_write(uint8_t index, uint8_t data)
{
	if(index <= 0x0c)
	{
		vga.sequencer.data[vga.sequencer.index] = data;
		seq_reg_write(vga.sequencer.index,data);
	}
	else
	{
		switch(index)
		{
		// Memory CLK PLL
		case 0x10:
			s3.sr10 = data;
			break;
		case 0x11:
			s3.sr11 = data;
			break;
		// Video CLK PLL
		case 0x12:
			s3.sr12 = data;
			break;
		case 0x13:
			s3.sr13 = data;
			break;
		case 0x15:
			if(data & 0x02)  // load DCLK frequency (would normally have a small variable delay)
			{
				s3.clk_pll_n = s3.sr12 & 0x1f;
				s3.clk_pll_r = (s3.sr12 & 0x60) >> 5;
				s3.clk_pll_m = s3.sr13 & 0x7f;
				s3_define_video_mode();
			}
			if(data & 0x20)  // immediate DCLK/MCLK load
			{
				s3.clk_pll_n = s3.sr12 & 0x1f;
				s3.clk_pll_r = (s3.sr12 & 0x60) >> 5;
				s3.clk_pll_m = s3.sr13 & 0x7f;
				s3_define_video_mode();
			}
			s3.sr15 = data;
		}
	}
}



uint8_t s3_vga_device::port_03b0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (get_crtc_port() == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				res = s3_vga_device::crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03b0_r(offset);
				break;
		}
	}

	return res;
}

void s3_vga_device::port_03b0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				s3_vga_device::crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03b0_w(offset,data);
				break;
		}
	}
}

uint8_t s3_vga_device::port_03c0_r(offs_t offset)
{
	uint8_t res;

	switch(offset)
	{
		case 5:
			res = s3_seq_reg_read(vga.sequencer.index);
			break;
		default:
			res = vga_device::port_03c0_r(offset);
			break;
	}

	return res;
}

void s3_vga_device::port_03c0_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 5:
			s3_seq_reg_write(vga.sequencer.index,data);
			break;
		default:
			vga_device::port_03c0_w(offset,data);
			break;
	}
}

uint8_t s3_vga_device::port_03d0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (get_crtc_port() == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = s3_vga_device::crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03d0_r(offset);
				break;
		}
	}

	return res;
}

void s3_vga_device::port_03d0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03d0_w(offset,data);
				break;
		}
	}
}

uint8_t s3_vga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		int data;
		if(offset & 0x10000)
			return 0;
		data = 0;
		if(vga.sequencer.data[4] & 0x8)
		{
			if((offset + (svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
				data = vga.memory[(offset + (svga.bank_r*0x10000))];
		}
		else
		{
			int i;

			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if((offset*4+i+(svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
						data |= vga.memory[(offset*4+i+(svga.bank_r*0x10000))];
				}
			}
		}
		return data;
	}
	if((offset + (svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
		return vga_device::mem_r(offset);
	else
		return 0xff;
}

void s3_vga_device::mem_w(offs_t offset, uint8_t data)
{
	ibm8514a_device* dev = get_8514();
	// bit 4 of CR53 enables memory-mapped I/O
	// 0xA0000-0xA7fff maps to port 0xE2E8 (pixel transfer)
	if(s3.cr53 & 0x10)
	{
		if(offset < 0x8000)
		{
			// pass through to the pixel transfer register (DirectX 5 wants this)
			if(dev->ibm8514.bus_size == 0)
			{
				dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
				dev->ibm8514_wait_draw();
			}
			if(dev->ibm8514.bus_size == 1)
			{
				switch(offset & 0x0001)
				{
				case 0:
				default:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
					break;
				case 1:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffff00ff) | (data << 8);
					dev->ibm8514_wait_draw();
					break;
				}
			}
			if(dev->ibm8514.bus_size == 2)
			{
				switch(offset & 0x0003)
				{
				case 0:
				default:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
					break;
				case 1:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffff00ff) | (data << 8);
					break;
				case 2:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xff00ffff) | (data << 16);
					break;
				case 3:
					dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0x00ffffff) | (data << 24);
					dev->ibm8514_wait_draw();
					break;
				}
			}
			return;
		}
		switch(offset)
		{
		case 0x8100:
		case 0x82e8:
			dev->ibm8514.curr_y = (dev->ibm8514.curr_y & 0xff00) | data;
			dev->ibm8514.prev_y = (dev->ibm8514.prev_y & 0xff00) | data;
			break;
		case 0x8101:
		case 0x82e9:
			dev->ibm8514.curr_y = (dev->ibm8514.curr_y & 0x00ff) | (data << 8);
			dev->ibm8514.prev_y = (dev->ibm8514.prev_y & 0x00ff) | (data << 8);
			break;
		case 0x8102:
		case 0x86e8:
			dev->ibm8514.curr_x = (dev->ibm8514.curr_x & 0xff00) | data;
			dev->ibm8514.prev_x = (dev->ibm8514.prev_x & 0xff00) | data;
			break;
		case 0x8103:
		case 0x86e9:
			dev->ibm8514.curr_x = (dev->ibm8514.curr_x & 0x00ff) | (data << 8);
			dev->ibm8514.prev_x = (dev->ibm8514.prev_x & 0x00ff) | (data << 8);
			break;
		case 0x8108:
		case 0x8ae8:
			dev->ibm8514.line_axial_step = (dev->ibm8514.line_axial_step & 0xff00) | data;
			dev->ibm8514.dest_y = (dev->ibm8514.dest_y & 0xff00) | data;
			break;
		case 0x8109:
		case 0x8ae9:
			dev->ibm8514.line_axial_step = (dev->ibm8514.line_axial_step & 0x00ff) | ((data & 0x3f) << 8);
			dev->ibm8514.dest_y = (dev->ibm8514.dest_y & 0x00ff) | (data << 8);
			break;
		case 0x810a:
		case 0x8ee8:
			dev->ibm8514.line_diagonal_step = (dev->ibm8514.line_diagonal_step & 0xff00) | data;
			dev->ibm8514.dest_x = (dev->ibm8514.dest_x & 0xff00) | data;
			break;
		case 0x810b:
		case 0x8ee9:
			dev->ibm8514.line_diagonal_step = (dev->ibm8514.line_diagonal_step & 0x00ff) | ((data & 0x3f) << 8);
			dev->ibm8514.dest_x = (dev->ibm8514.dest_x & 0x00ff) | (data << 8);
			break;
		case 0x8118:
		case 0x9ae8:
			s3.mmio_9ae8 = (s3.mmio_9ae8 & 0xff00) | data;
			break;
		case 0x8119:
		case 0x9ae9:
			s3.mmio_9ae8 = (s3.mmio_9ae8 & 0x00ff) | (data << 8);
			dev->ibm8514_cmd_w(s3.mmio_9ae8);
			break;
		case 0x8120:
		case 0xa2e8:
			dev->ibm8514.bgcolour = (dev->ibm8514.bgcolour & 0xff00) | data;
			break;
		case 0x8121:
		case 0xa2e9:
			dev->ibm8514.bgcolour = (dev->ibm8514.bgcolour & 0x00ff) | (data << 8);
			break;
		case 0x8124:
		case 0xa6e8:
			dev->ibm8514.fgcolour = (dev->ibm8514.fgcolour & 0xff00) | data;
			break;
		case 0x8125:
		case 0xa6e9:
			dev->ibm8514.fgcolour = (dev->ibm8514.fgcolour & 0x00ff) | (data << 8);
			break;
		case 0x8128:
		case 0xaae8:
			dev->ibm8514.write_mask = (dev->ibm8514.write_mask & 0xff00) | data;
			break;
		case 0x8129:
		case 0xaae9:
			dev->ibm8514.write_mask = (dev->ibm8514.write_mask & 0x00ff) | (data << 8);
			break;
		case 0x812c:
		case 0xaee8:
			dev->ibm8514.read_mask = (dev->ibm8514.read_mask & 0xff00) | data;
			break;
		case 0x812d:
		case 0xaee9:
			dev->ibm8514.read_mask = (dev->ibm8514.read_mask & 0x00ff) | (data << 8);
			break;
		case 0xb6e8:
		case 0x8134:
			dev->ibm8514.bgmix = (dev->ibm8514.bgmix & 0xff00) | data;
			break;
		case 0x8135:
		case 0xb6e9:
			dev->ibm8514.bgmix = (dev->ibm8514.bgmix & 0x00ff) | (data << 8);
			break;
		case 0x8136:
		case 0xbae8:
			dev->ibm8514.fgmix = (dev->ibm8514.fgmix & 0xff00) | data;
			break;
		case 0x8137:
		case 0xbae9:
			dev->ibm8514.fgmix = (dev->ibm8514.fgmix & 0x00ff) | (data << 8);
			break;
		case 0x8138:
			dev->ibm8514.scissors_top = (dev->ibm8514.scissors_top & 0xff00) | data;
			break;
		case 0x8139:
			dev->ibm8514.scissors_top = (dev->ibm8514.scissors_top & 0x00ff) | (data << 8);
			break;
		case 0x813a:
			dev->ibm8514.scissors_left = (dev->ibm8514.scissors_left & 0xff00) | data;
			break;
		case 0x813b:
			dev->ibm8514.scissors_left = (dev->ibm8514.scissors_left & 0x00ff) | (data << 8);
			break;
		case 0x813c:
			dev->ibm8514.scissors_bottom = (dev->ibm8514.scissors_bottom & 0xff00) | data;
			break;
		case 0x813d:
			dev->ibm8514.scissors_bottom = (dev->ibm8514.scissors_bottom & 0x00ff) | (data << 8);
			break;
		case 0x813e:
			dev->ibm8514.scissors_right = (dev->ibm8514.scissors_right & 0xff00) | data;
			break;
		case 0x813f:
			dev->ibm8514.scissors_right = (dev->ibm8514.scissors_right & 0x00ff) | (data << 8);
			break;
		case 0x8140:
			dev->ibm8514.pixel_control = (dev->ibm8514.pixel_control & 0xff00) | data;
			break;
		case 0x8141:
			dev->ibm8514.pixel_control = (dev->ibm8514.pixel_control & 0x00ff) | (data << 8);
			break;
		case 0x8146:
			dev->ibm8514.multifunc_sel = (dev->ibm8514.multifunc_sel & 0xff00) | data;
			break;
		case 0x8148:
			dev->ibm8514.rect_height = (dev->ibm8514.rect_height & 0xff00) | data;
			break;
		case 0x8149:
			dev->ibm8514.rect_height = (dev->ibm8514.rect_height & 0x00ff) | (data << 8);
			break;
		case 0x814a:
			dev->ibm8514.rect_width = (dev->ibm8514.rect_width & 0xff00) | data;
			break;
		case 0x814b:
			dev->ibm8514.rect_width = (dev->ibm8514.rect_width & 0x00ff) | (data << 8);
			break;
		case 0x8150:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0x8151:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffff00ff) | (data << 8);
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0x8152:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xff00ffff) | (data << 16);
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0x8153:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0x00ffffff) | (data << 24);
			if(dev->ibm8514.state == IBM8514_DRAWING_RECT)
				dev->ibm8514_wait_draw();
			break;
		case 0xbee8:
			s3.mmio_bee8 = (s3.mmio_bee8 & 0xff00) | data;
			break;
		case 0xbee9:
			s3.mmio_bee8 = (s3.mmio_bee8 & 0x00ff) | (data << 8);
			dev->ibm8514_multifunc_w(s3.mmio_bee8);
			break;
		case 0x96e8:
			s3.mmio_96e8 = (s3.mmio_96e8 & 0xff00) | data;
			break;
		case 0x96e9:
			s3.mmio_96e8 = (s3.mmio_96e8 & 0x00ff) | (data << 8);
			dev->ibm8514_width_w(s3.mmio_96e8);
			break;
		case 0xe2e8:
			dev->ibm8514.pixel_xfer = (dev->ibm8514.pixel_xfer & 0xffffff00) | data;
			dev->ibm8514_wait_draw();
			break;
		default:
			LOG( "S3: MMIO offset %05x write %02x\n",offset+0xa0000,data);
			break;
		}
		return;
	}

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		//printf("%08x %02x (%02x %02x) %02X\n",offset,data,vga.sequencer.map_mask,svga.bank_w,(vga.sequencer.data[4] & 0x08));
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
		vga_device::mem_w(offset,data);
}


uint32_t s3_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	svga_device::screen_update(screen, bitmap, cliprect);

	uint8_t cur_mode = pc_vga_choosevideomode();

	// draw hardware graphics cursor
	// TODO: support 16 bit and greater video modes
	// TODO: should be a derived function from svga_device
	if(s3.cursor_mode & 0x01)  // if cursor is enabled
	{
		uint16_t cx = s3.cursor_x & 0x07ff;
		uint16_t cy = s3.cursor_y & 0x07ff;

		if(cur_mode == SCREEN_OFF || cur_mode == TEXT_MODE || cur_mode == MONO_MODE || cur_mode == CGA_MODE || cur_mode == EGA_MODE)
			return 0;  // cursor only works in VGA or SVGA modes

		uint32_t src = s3.cursor_start_addr * 1024;  // start address is in units of 1024 bytes

		uint32_t bg_col;
		uint32_t fg_col;
		int r,g,b;
		uint32_t datax;
		switch(cur_mode)
		{
		case RGB15_MODE:
		case RGB16_MODE:
			datax = s3.cursor_bg[0]|s3.cursor_bg[1]<<8;
			r = (datax&0xf800)>>11;
			g = (datax&0x07e0)>>5;
			b = (datax&0x001f)>>0;
			r = (r << 3) | (r & 0x7);
			g = (g << 2) | (g & 0x3);
			b = (b << 3) | (b & 0x7);
			bg_col = (0xff<<24)|(r<<16)|(g<<8)|(b<<0);

			datax = s3.cursor_fg[0]|s3.cursor_fg[1]<<8;
			r = (datax&0xf800)>>11;
			g = (datax&0x07e0)>>5;
			b = (datax&0x001f)>>0;
			r = (r << 3) | (r & 0x7);
			g = (g << 2) | (g & 0x3);
			b = (b << 3) | (b & 0x7);
			fg_col = (0xff<<24)|(r<<16)|(g<<8)|(b<<0);
			break;
		case RGB24_MODE:
			datax = s3.cursor_bg[0]|s3.cursor_bg[1]<<8|s3.cursor_bg[2]<<16;
			r = (datax&0xff0000)>>16;
			g = (datax&0x00ff00)>>8;
			b = (datax&0x0000ff)>>0;
			bg_col = (0xff<<24)|(r<<16)|(g<<8)|(b<<0);

			datax = s3.cursor_fg[0]|s3.cursor_fg[1]<<8|s3.cursor_fg[2]<<16;
			r = (datax&0xff0000)>>16;
			g = (datax&0x00ff00)>>8;
			b = (datax&0x0000ff)>>0;
			fg_col = (0xff<<24)|(r<<16)|(g<<8)|(b<<0);
			break;
		case RGB8_MODE:
		default:
			bg_col = pen(s3.cursor_bg[0]);
			fg_col = pen(s3.cursor_fg[0]);
			break;
		}

		//popmessage("%08x %08x",(s3.cursor_bg[0])|(s3.cursor_bg[1]<<8)|(s3.cursor_bg[2]<<16)|(s3.cursor_bg[3]<<24)
		//                    ,(s3.cursor_fg[0])|(s3.cursor_fg[1]<<8)|(s3.cursor_fg[2]<<16)|(s3.cursor_fg[3]<<24));
//      for(x=0;x<64;x++)
//          printf("%08x: %02x %02x %02x %02x\n",src+x*4,vga.memory[src+x*4],vga.memory[src+x*4+1],vga.memory[src+x*4+2],vga.memory[src+x*4+3]);
		for(int y=0;y<64;y++)
		{
			if(cy + y < cliprect.max_y && cx < cliprect.max_x)
			{
				uint32_t *const dst = &bitmap.pix(cy + y, cx);
				for(int x=0;x<64;x++)
				{
					uint16_t bita = (vga.memory[(src+1) % vga.svga_intf.vram_size] | ((vga.memory[(src+0) % vga.svga_intf.vram_size]) << 8)) >> (15-(x % 16));
					uint16_t bitb = (vga.memory[(src+3) % vga.svga_intf.vram_size] | ((vga.memory[(src+2) % vga.svga_intf.vram_size]) << 8)) >> (15-(x % 16));
					uint8_t val = ((bita & 0x01) << 1) | (bitb & 0x01);
					if(s3.extended_dac_ctrl & 0x10)
					{  // X11 mode
						switch(val)
						{
						case 0x00:
							// no change
							break;
						case 0x01:
							// no change
							break;
						case 0x02:
							dst[x] = bg_col;
							break;
						case 0x03:
							dst[x] = fg_col;
							break;
						}
					}
					else
					{  // Windows mode
						switch(val)
						{
						case 0x00:
							dst[x] = bg_col;
							break;
						case 0x01:
							dst[x] = fg_col;
							break;
						case 0x02:  // screen data
							// no change
							break;
						case 0x03:  // inverted screen data
							dst[x] = ~(dst[x]);
							break;
						}
					}
					if(x % 16 == 15)
						src+=4;
				}
			}
		}
	}
	return 0;
}
