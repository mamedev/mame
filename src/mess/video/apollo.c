/*
 * video/apollo.c
 *
 *  Created on: May 12, 2010
 *      Author: Hans Ostermeyer
 *
 *  Released for general non-commercial use under the MAME license
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 *
 *  see also:
 *  - Domain Series 3000/Series 4000 Hardware Architecture Handbook (Order No. 007861 Rev. 02)
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf (page 12-16 ...)
 */

#define VERBOSE 0

#include "includes/apollo.h"

#include "apollo.lh"
#include "apollo_15i.lh"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// monochrome 1280x1024
#define SCREEN_DEVICE_ID_19I 0x09

// monochrome 1024x800
#define SCREEN_DEVICE_ID_15I 0x0b

#define VIDEO_SCREEN_TAG "screen"

// status register
#define SR_BLANK        0x80
#define SR_V_BLANK      0x40
#define SR_H_SYNC       0x20
#define SR_R_M_W        0x10
#define SR_ALT          0x08
#define SR_V_SYNC       0x04
#define SR_H_CK         0x02
#define SR_V_DATA       0x01

// control register 0
#define CR0_MODE(a)     ((a) >> 5)
#define CR0_MODE_0      0
#define CR0_MODE_1      1
#define CR0_MODE_VECTOR 2
#define CR0_MODE_3      3
#define CR0_MODE_BLT    4
#define CR0_MODE_NORMAL 7
#define CR0_SHIFT(a)    ((a) & 0x1f)

// control register 1
#define CR1_INV         0x80
#define CR1_DADDR_16    0x40
#define CR1_DV_CK       0x40
#define CR1_DH_CK       0x20
#define CR1_ROP_EN      0x10
#define CR1_RESET       0x08
#define CR1_DP_CK       0x04
#define CR1_SYNC_EN     0x02
#define CR1_DISP_EN     0x01

// control register 2
#define CR2_S_DATA(a)   ((a) >> 6)
#define CR2_CONST_ACCESS 0x00
#define CR2_PIXEL_ACCESS 0x01
#define CR2_SHIFT_ACCESS 0x02
#define CR2_PLANE_ACCESS 0x03

struct screen_data_t {
	UINT16 width;
	UINT16 height;
	UINT16 buffer_width;
	UINT16 buffer_height;

	UINT8 status_register;
	UINT8 device_id;
	UINT16 write_enable_register;
	UINT16 rop_register_0;
	UINT16 diag_mem_request;
	UINT8 cr0;
	UINT8 cr1;
	UINT8 cr2;
	UINT8 cr3a;

	UINT8 update_flag;
	UINT8 update_pending;

	UINT8 blt_cycle_count;
	UINT32 guard_latch;
	offs_t image_offset;

	int h_clock;
	int v_clock;
	int pixel_clock;
	int data_clock;

	UINT16 *image_memory;
	int image_memory_size;

	screen_device *screen;
};

/*****************************************************************************
 INLINE FUNCTIONS
 *****************************************************************************/

INLINE screen_data_t *get_safe_token(device_t *device) {
	assert(device != NULL);
	assert(device->type() == APOLLO_MONO15I || device->type() == APOLLO_MONO19I );
	return (screen_data_t *)downcast<apollo_mono_device *>(device)->token();
}

/***************************************************************************
 Monochrome Controller Registers at 0x5d800 - 0x5dc07
 ***************************************************************************/

static void log_cr1(const char * text, device_t *device, screen_data_t *screen_data) {

	DLOG1(("%s: cr0=%02x cr1=%02x sr=%02x pixel_clock=%3d/%3d bl=%d vb=%d vs=%d hs=%d hc=%d vck=%d hck=%d pck=%d vd=%d",
			text,
			screen_data->cr0,
			screen_data->cr1,
			screen_data->status_register,
			screen_data->pixel_clock,
			screen_data->data_clock,
			screen_data->status_register & SR_BLANK ? 1 : 0,
			screen_data->status_register & SR_V_BLANK ? 1 : 0,
			screen_data->status_register & SR_V_SYNC ? 1 : 0,
			screen_data->status_register & SR_H_SYNC ? 1 : 0,
			screen_data->status_register & SR_H_CK ? 1 : 0,
			screen_data->cr1 & CR1_DV_CK ? 1 : 0,
			screen_data->cr1 & CR1_DH_CK ? 1 : 0,
			screen_data->cr1 & CR1_DP_CK ? 1 : 0,
			screen_data->status_register & SR_V_DATA ? 1 : 0));
}

static void set_cr1(device_t *device, screen_data_t *screen_data,
		UINT8 data) {
	UINT8 diffs = screen_data->cr1 ^ data;
	screen_data->cr1 = data;

//  if (screen_data->cr1 & CR1_SYNC_EN) {
//      // normal mode
//  } else

	if ((screen_data->cr1 & CR1_RESET) == 0) {
		if (diffs & CR1_RESET) {
			screen_data->blt_cycle_count = 0;
			screen_data->image_offset = 0;
			screen_data->guard_latch = 0;

			screen_data->h_clock = 0;
			screen_data->v_clock = 0;
			screen_data->pixel_clock = 0;
			if (screen_data->device_id == SCREEN_DEVICE_ID_19I) {
				screen_data->data_clock = -11; // TODO: why not 0 ????
				screen_data->status_register = SR_H_CK | SR_V_BLANK | SR_H_SYNC
						| SR_V_SYNC;
			} else {
				screen_data->data_clock = -9; // TODO: why not 0 ????
				screen_data->status_register = SR_V_BLANK | SR_V_SYNC;
			}
		}
		log_cr1("CR1_RESET", device, screen_data);
	} else {
		if ((diffs & CR1_RESET) && (screen_data->cr1 & CR1_RESET) != 0) {
			log_cr1("CR1_RESET", device, screen_data);
		}

		if ((diffs & CR1_DH_CK) && (screen_data->cr1 & CR1_DH_CK) == 0) {
			if (screen_data->device_id == SCREEN_DEVICE_ID_19I) {
				switch (screen_data->h_clock %= 108) {
				case 8:
					screen_data->status_register |= SR_BLANK;
					break;
				case 88:
					screen_data->status_register &= ~SR_BLANK;
					break;
				case 93:
					screen_data->status_register &= ~SR_H_SYNC;
						// trigger Dp_Ck
						diffs |= CR1_DP_CK;
						screen_data->cr1 &= ~CR1_DP_CK;
					break;
				case 104:
					screen_data->status_register |= SR_H_SYNC;
					break;
				}
			} else {
				switch (screen_data->h_clock %= 84) {
				case 1:
					screen_data->status_register |= SR_H_SYNC;
					break;
				case 8:
					screen_data->status_register |= SR_BLANK;
					break;
				case 72:
					screen_data->status_register &= ~SR_BLANK;
					break;
				case 77:
					screen_data->status_register &= ~SR_H_SYNC;
					diffs |= CR1_DV_CK;
					data &= ~CR1_DV_CK;
					break;
				}
			}
			screen_data->h_clock++;
			log_cr1("CR1_DH_CK",device, screen_data);
		}

		if ((diffs & CR1_DV_CK) && (screen_data->cr1 & CR1_DV_CK) == 0) {
			// this is used for disp.dex Test 19: Video RAM Shift Reg. Test
			if (screen_data->device_id == SCREEN_DEVICE_ID_15I) {
				switch (screen_data->v_clock %= 842) {
				case 799:
					screen_data->status_register &= ~SR_V_BLANK;
					break;
				case 804:
					screen_data->status_register &= ~SR_V_SYNC;
					break;
				case 808:
					screen_data->status_register |= SR_V_SYNC;
					break;
				case 841:
					screen_data->status_register |= SR_V_BLANK;
					break;
				}
				screen_data->v_clock++;
				log_cr1("CR1_DV_CK",device, screen_data);
			}
		}

		if ((diffs & CR1_DP_CK) && (screen_data->cr1 & CR1_DP_CK) == 0) {
			if (screen_data->device_id == SCREEN_DEVICE_ID_19I) {
				switch (screen_data->pixel_clock %= 1066) {
				case 1023:
					screen_data->status_register &= ~SR_V_BLANK;
					break;
				case 1028:
					screen_data->status_register &= ~SR_V_SYNC;
					break;
				case 1032:
					screen_data->status_register |= SR_V_SYNC;
					break;
				case 1065:
					screen_data->status_register |= SR_V_BLANK;
					break;
				}
			} else /*if (screen_data->pixel_clock == 0)*/ {
				// this is used for disp.dex Test 6: Vertical Counter Test
				switch (screen_data->pixel_clock %= 842) {
				case 799:
					screen_data->status_register &= ~SR_V_BLANK;
					break;
				case 804:
					screen_data->status_register &= ~SR_V_SYNC;
					break;
				case 808:
					screen_data->status_register |= SR_V_SYNC;
					break;
				case 841:
					screen_data->status_register |= SR_V_BLANK;
					break;
				}
			}

			if ((screen_data->cr1 & CR1_DISP_EN) == 0) {
				screen_data->status_register &= ~SR_V_DATA;
			}else			{
				UINT16 pixel = screen_data->image_memory[screen_data->data_clock / 16]
								& (0x8000 >> (screen_data->data_clock % 16));
				pixel = (pixel ? 1 : 0)	^ ((screen_data->cr1 & CR1_INV) ? 0 : 1);

				if (pixel) {
					screen_data->status_register |= SR_V_DATA;
				} else {
					screen_data->status_register &= ~SR_V_DATA;
				}
				screen_data->data_clock++;
			}

			screen_data->pixel_clock++;
			if ((screen_data->pixel_clock % 8) == 0) {
				screen_data->status_register ^= SR_H_CK;
			}

			log_cr1("CR1_DP_CK", device, screen_data);
		}

		if ((screen_data->status_register & SR_V_BLANK) == 0) {
			screen_data->status_register &= ~SR_BLANK;
		}

		if (diffs & CR1_DISP_EN) {
			// update screen
			screen_data->update_flag = 1;
		}
	}
}

static void set_cr3(device_t *device, screen_data_t *screen_data, UINT8 data) {
	screen_data->cr3a = data;
	if ((data & 0x80) == 0) {
		int shift = (data & 0x0f) >> 1;
		UINT8 bit_mask = 1 << shift;
		if (data & 0x01) {
			set_cr1(device, screen_data, screen_data->cr1 | bit_mask);
		} else {
			set_cr1(device, screen_data, screen_data->cr1 & ~bit_mask);
		}
	}
}

READ16_DEVICE_HANDLER( apollo_mcr_r ) {
	screen_data_t *screen_data = get_safe_token(device);
	UINT16 data;
	switch (offset & 0x203) {
	case 0:
	case 1:
	case 2:
	case 3:
		data = (screen_data->status_register << 8)
				| screen_data->device_id;
		break;
	case 0x200:
		data = screen_data->cr0 << 8 | 0xff;
		break;
	case 0x201:
		data = screen_data->cr1 << 8 | 0xff;
		break;
	case 0x202:
		data = screen_data->cr2 << 8 | 0xff;
		break;
	case 0x203:
		data = screen_data->cr3a << 8 | 0xff;
		break;
	default:
		data = screen_data->device_id;
		break;
	}
	DLOG1(("reading Monochrome Controller at offset %03x = %04x and %04x", offset, data, mem_mask));
	return data;
}

WRITE16_DEVICE_HANDLER(apollo_mcr_w ) {
	screen_data_t *screen_data = get_safe_token(device);
	if (offset != 0 && data != 0)
		DLOG1(("writing Monochrome Controller at offset %03x = %04x and %04x", offset, data, mem_mask));

	switch (offset & 0x203) {
	case 0:
		screen_data->write_enable_register = data;
		screen_data->blt_cycle_count = 0;
		screen_data->status_register &= ~SR_ALT;
		break;
	case 1:
		screen_data->rop_register_0 = data;
		switch (data & 0x0f) {
		case 0: // zero
		case 3: // Source
		case 0x0c: // ~Source
		case 0x0f: // one
			screen_data->status_register &= ~SR_R_M_W;
			break;
		default:
			screen_data->status_register |= SR_R_M_W;
			break;
		}
		break;
	case 2:
		// trigger memory refresh in diagnostic mode
		screen_data->diag_mem_request = data;
		break;
	case 0x200:
		screen_data->cr0 = data >> 8;
		screen_data->blt_cycle_count = 0;
		break;
	case 0x201:
		set_cr1(device, screen_data, data >> 8);
		break;
	case 0x202:
		screen_data->cr2 = data >> 8;
		break;
	case 0x203:
		set_cr3(device, screen_data, data >> 8);
		break;
	}
}

/***************************************************************************
 Monochrome graphics memory space at FA0000 - FDFFFF
 ***************************************************************************/

static UINT32 get_source_data(screen_data_t *screen_data, UINT32 src_data) {
	switch (CR2_S_DATA(screen_data->cr2)) {
	case CR2_CONST_ACCESS: // 0x00
		// set source to all ones (used for vectors)
		src_data= 0xffff;
		break;
	case CR2_PIXEL_ACCESS: // 0x01
		// replicate 4 LSB of data bus
		src_data= src_data & 1 ? 0xffff : 0;
		break;
	case CR2_SHIFT_ACCESS: // 0x02
		// replicate LSB of shifter
		src_data = src_data & 0xffff;
		break;
	case CR2_PLANE_ACCESS: // 0x03
		// use source data unchanged (normal use)
		if (CR0_SHIFT(screen_data->cr0) >= 16) {
			src_data = (src_data << 16) | (src_data >> 16);
		}
		src_data >>= (CR0_SHIFT(screen_data->cr0) & 0x0f);
		break;
	}
	return src_data;
}

static UINT16 rop(screen_data_t *screen_data, UINT16 dest_data) {

	UINT32 src_data = get_source_data(screen_data, screen_data->guard_latch);

	if ((screen_data->cr1 & CR1_ROP_EN)
			/*&& (CR2_S_DATA(screen_data->cr2) == CR2_PLANE_ACCESS)*/) {
		switch (screen_data->rop_register_0 & 0x0f) {
		case 0: // zero
			src_data = 0;
			break;
		case 1: // Source AND Destination
			src_data = src_data & dest_data;
			break;
		case 2: // Source AND ~Destination
			src_data = src_data & (~dest_data);
			break;
		case 3: // Source
			break;
		case 4: // ~Source AND Destination
			src_data = (~src_data) & dest_data;
			break;
		case 5: // Destination
			src_data = dest_data;
			break;
		case 6: // Source XOR Destination
			src_data = src_data ^ dest_data;
			break;
		case 7: // Source OR Destination
			src_data = src_data | dest_data;
			break;
		case 8: // Source NOR Destination
			src_data = ~(src_data | dest_data);
			break;
		case 9: // Source XNOR Destination
			src_data = ~(src_data ^ dest_data);
			break;
		case 0x0a: // ~Destination
			src_data = ~dest_data;
			break;
		case 0x0b: // Source OR ~Destination
			src_data = src_data | (~dest_data);
			break;
		case 0x0c: // ~Source
			src_data = ~src_data;
			break;
		case 0x0d: // ~Source OR Destination
			src_data = (~src_data) | dest_data;
			break;
		case 0x0e: // Source NAND Destination
			src_data = ~(src_data & dest_data);
			break;
		case 0x0f: // One
			src_data = 0xffff;
			break;
		}
	}

	return src_data & 0xffff;
}

READ16_DEVICE_HANDLER( apollo_mgm_r ) {
	screen_data_t *screen_data = get_safe_token(device);
	UINT16 data;

	if (CR0_MODE(screen_data->cr0) == CR0_MODE_0
			&& screen_data->blt_cycle_count > 0) {
		offset = screen_data->image_offset;
		screen_data->blt_cycle_count = 0;
	}

	if (offset >= screen_data->image_memory_size) {
		// 128 kB display buffer of 15" screen seems to be shadowed from $fa0000 to $fc0000
		DLOG1(("reading Monochrome Graphics Memory at invalid offset %05x", offset));
		offset %= screen_data->image_memory_size;
	}

	switch (CR0_MODE(screen_data->cr0)) {
	case CR0_MODE_VECTOR:
		// vector or fill mode
		UINT16 src_data, dest_data;
		screen_data->status_register &= ~SR_ALT;

		dest_data = screen_data->image_memory[offset];
		src_data = rop(screen_data, dest_data);

		src_data &= ~screen_data->write_enable_register;
		dest_data &= (screen_data->write_enable_register | ~mem_mask);
		screen_data->image_memory[offset] = dest_data | src_data;
		data = screen_data->image_memory[offset];
		break;
	case CR0_MODE_3:
		// CPU source BLT: read internal data bus
		data = screen_data->guard_latch;
		break;
	default:
		data = screen_data->image_memory[offset];
		screen_data->guard_latch <<= 16;
		screen_data->guard_latch |= data;
		break;
	}
	DLOG1(("reading Monochrome Graphics Memory with mode %d: offset %05x = %04x & %04x", CR0_MODE(screen_data->cr0), offset, data, mem_mask));
	return data;
}

WRITE16_DEVICE_HANDLER( apollo_mgm_w ) {
	screen_data_t *screen_data = get_safe_token(device);
	UINT16 src_data, dest_data;
	UINT32 dest_addr;

	if (offset >= screen_data->image_memory_size) {
		// 128 kB display buffer of 15" screen seems to be shadowed from $fa0000 to $fc0000
		DLOG1(("writing Monochrome Graphics Memory at invalid offset %05x = %04x & %04x ", offset, data, mem_mask));
		offset %= screen_data->image_memory_size;
	}

	DLOG1(("writing Monochrome Graphics Memory with mode %d: offset=%04x data=%04x mask=%04x", CR0_MODE(screen_data->cr0), offset, data, mem_mask));
	switch (CR0_MODE(screen_data->cr0)) {
	case CR0_MODE_0:
		// CPU destination BLT
		// bus write to provide display memory address
		// bus read to get data
		screen_data->image_offset = offset;
		screen_data->blt_cycle_count = 1;
		break;
	case CR0_MODE_1:
		// Alternating BLT
		// alternating bus writes provide src/dest address
		// second write provides Write-enables
		if (++screen_data->blt_cycle_count == 1) {
			screen_data->status_register |= SR_ALT;
			screen_data->guard_latch <<= 16;
			screen_data->guard_latch |= screen_data->image_memory[offset];
		} else {
			screen_data->blt_cycle_count = 0;
			screen_data->status_register &= ~SR_ALT;

			dest_data = screen_data->image_memory[offset];
			src_data = rop(screen_data, dest_data);

			src_data &= (~data & mem_mask);
			dest_data &= (data | ~mem_mask);
			screen_data->image_memory[offset] = dest_data | src_data;
		}
		break;
	case CR0_MODE_VECTOR:
		// Vector or fill mode
		// write provides Write-enables and address
		screen_data->status_register &= ~SR_ALT;

		dest_data = screen_data->image_memory[offset];
		src_data = rop(screen_data, dest_data);

		src_data &= (~data & mem_mask);
		dest_data &= (data | ~mem_mask);
		screen_data->image_memory[offset] = dest_data | src_data;
		break;
	case CR0_MODE_3:
		// CPU source BLT
		// bus write to provide src data
		// bus write to provide Write-enables and address
		if (++screen_data->blt_cycle_count == 1) {
			screen_data->status_register |= SR_ALT;

			// strange: must fix byte access for /systest/grtest on sr10.2
			if (mem_mask == 0xff00)
			{
				data >>=8;
				mem_mask >>= 8;
			}

			screen_data->guard_latch <<= 16;
			screen_data->guard_latch |= data;

		} else {
			screen_data->blt_cycle_count = 0;
			screen_data->status_register &= ~SR_ALT;

			dest_data = screen_data->image_memory[offset];
			dest_data &= (data | ~mem_mask);

			src_data = rop(screen_data, dest_data);
			src_data &= (~data & mem_mask);

			screen_data->image_memory[offset] = dest_data | src_data;
		}
		break;
	case CR0_MODE_BLT:
		// Double access BLT
		// bus write to provide src addr on address lines
		// dest addr on data lines (16-bit WORD Offset)
		screen_data->guard_latch <<= 16;
		screen_data->guard_latch |= screen_data->image_memory[offset];

		dest_addr = (data & mem_mask);
		if (screen_data->device_id == SCREEN_DEVICE_ID_19I && (screen_data->cr1
				& CR1_DADDR_16)) {
			dest_addr += 0x10000;
		}
		dest_data = screen_data->image_memory[dest_addr];

		src_data = rop(screen_data, dest_data);
		src_data &= ~screen_data->write_enable_register;

		dest_data &= (screen_data->write_enable_register | ~mem_mask);
		screen_data->image_memory[dest_addr] = dest_data | src_data;
		break;
	case CR0_MODE_NORMAL:
		screen_data->guard_latch <<= 16;
		screen_data->guard_latch |= (data & mem_mask);;
		dest_data = screen_data->image_memory[offset];
		src_data = rop(screen_data, dest_data);

		src_data &= ~screen_data->write_enable_register;
		dest_data &= (screen_data->write_enable_register | ~mem_mask);
		screen_data->image_memory[offset] = dest_data | src_data;
		break;
	default:
		DLOG(("writing Monochrome Graphics Memory - unexpected cr0 mode %d", CR0_MODE(screen_data->cr0)));
	}
	screen_data->update_flag = 1;
}

/***************************************************************************
 VIDEO HARDWARE
 ***************************************************************************/

static void apollo_screen_update(device_t *device, bitmap_ind16 &bitmap,
		const rectangle &cliprect) {
	screen_data_t *screen_data = get_safe_token(device);

	UINT16 *source_ptr = screen_data->image_memory;
	int x, y;
	UINT16 data, mask;
	UINT16 inverse = (screen_data->cr1 & CR1_INV) ? 0xffff : 0;

	DLOG1(("apollo_screen_update: size=%0x rowpixels=%d", screen_data->image_memory_size, bitmap.rowpixels()));

	if ((screen_data->cr1 & CR1_DISP_EN) == 0) {
		// display is disabled
		for (y = 0; y < screen_data->height; y++) {
			int dest = 0;
			for (x = 0; x < screen_data->width; x += 16) {
				for (mask = 0x8000; mask; mask >>= 1) {
					bitmap.pix16(y, dest++) = 0;
				}
			}
			source_ptr += (screen_data->buffer_width - screen_data->width) / 16;
		}
	} else {
		for (y = 0; y < screen_data->height; y++) {
			int dest = 0;
			for (x = 0; x < screen_data->width; x += 16) {
				data = *source_ptr++ ^ inverse;
				for (mask = 0x8000; mask; mask >>= 1) {
					bitmap.pix16(y, dest++) = data & mask ? 0 : 1;
				}
			}
			source_ptr += (screen_data->buffer_width - screen_data->width) / 16;
		}
	}
}

/*-------------------------------------------------
    vblank_state_changed -
   called on each state change of the VBLANK signal
-------------------------------------------------*/

static void vblank_state_changed(device_t *device, screen_device &screen, bool vblank_state)
{
	screen_data_t *screen_data = get_safe_token(device);

	if ((screen_data->cr1 & CR1_RESET) && (screen_data->cr1 & CR1_SYNC_EN)) {
		if (vblank_state) {
			screen_data->status_register &= ~(SR_V_BLANK | SR_BLANK);
			// faking V_DATA for disp.dex test 16
			if (screen_data->image_memory[0]) {
				screen_data->status_register |= SR_V_DATA;
			}
		} else {
			screen_data->status_register |= (SR_V_BLANK | SR_BLANK);
			screen_data->status_register &= ~SR_V_DATA;
		}
	}
}

VIDEO_START( apollo_screen ) {
}

SCREEN_UPDATE_IND16( apollo_screen ) {
	// FIXME: omit using APOLLO_SCREEN_TAG
	device_t *apollo_screen = screen.machine().device( APOLLO_SCREEN_TAG );
	screen_data_t *screen_data = get_safe_token(apollo_screen);

	int has_changed = 0;

	if (screen_data->update_flag && !screen_data->update_pending) {
		has_changed = 1;
		screen_data->update_flag = 0;
		screen_data->update_pending = 1;
		apollo_screen_update(apollo_screen, bitmap, cliprect);
		screen_data->update_pending = 0;
	}
	return has_changed ? 0 : UPDATE_HAS_NOT_CHANGED;
}

/***************************************************************************
 MACHINE DRIVERS
 ***************************************************************************/
MACHINE_CONFIG_FRAGMENT( apollo_mono19i )
		MCFG_DEFAULT_LAYOUT( layout_apollo )
		MCFG_SCREEN_ADD(VIDEO_SCREEN_TAG, RASTER)
		MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
//      MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
		MCFG_PALETTE_LENGTH(2)
		MCFG_PALETTE_INIT(black_and_white)
		// dot clock, htotal, hstart, hend, vtotal, vstart, vend
		// MCFG_SCREEN_RAW_PARAMS(118000000, 1280, 0, 1728, 1024, 0, 1065)
		MCFG_SCREEN_REFRESH_RATE(64)
		MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(616))
		MCFG_SCREEN_SIZE(1280, 1024)
		MCFG_SCREEN_VISIBLE_AREA(0, 1279, 0, 1023)
		MCFG_VIDEO_START(apollo_screen)
		MCFG_SCREEN_UPDATE_STATIC(apollo_screen)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( apollo_mono15i )
		MCFG_DEFAULT_LAYOUT( layout_apollo_15i )
		MCFG_SCREEN_ADD(VIDEO_SCREEN_TAG, RASTER)
		MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
//      MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
		MCFG_PALETTE_LENGTH(2)
		MCFG_PALETTE_INIT(black_and_white)
		// dot clock, htotal, hstart, hend, vtotal, vstart, vend
		// MCFG_SCREEN_RAW_PARAMS(85963000, 1024, 0, 1344, 800, 0, 842)
		MCFG_SCREEN_REFRESH_RATE(76)
		MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(657))
		MCFG_SCREEN_SIZE(1024, 800)
		MCFG_SCREEN_VISIBLE_AREA(0, 1023, 0, 799)
		MCFG_VIDEO_START(apollo_screen)
		MCFG_SCREEN_UPDATE_STATIC(apollo_screen)
MACHINE_CONFIG_END

/*-------------------------------------------------
 DEVICE_START( apollo_mono19i/15i )
 -------------------------------------------------*/

static DEVICE_START( apollo_mono ) {
	screen_data_t *screen_data = get_safe_token(device);

	/* get the video screen  */
	screen_data->screen = (screen_device *)device->machine().device(VIDEO_SCREEN_TAG);
	assert(screen_data->screen != NULL);

	/* allocate the memory image */
	screen_data->image_memory_size = screen_data->buffer_height
			* screen_data->buffer_width / 16;
//  screen_data->image_memory = (UINT16 *) malloc(screen_data->image_memory_size * 2);
	screen_data->image_memory = auto_alloc_array(device->machine(), UINT16, screen_data->image_memory_size);
	assert(screen_data->image_memory != NULL);

	DLOG1(("device start apollo screen buffer=%p size=%0x", screen_data->image_memory, screen_data->image_memory_size));
}

static DEVICE_START( apollo_mono19i ) {
	screen_data_t *screen_data = get_safe_token(device);

	memset(screen_data, 0, sizeof(screen_data_t));

	// monochrome 1280x1024
	screen_data->device_id = SCREEN_DEVICE_ID_19I;
	screen_data->width = 1280;
	screen_data->height = 1024;
	screen_data->buffer_width = 2048;
	screen_data->buffer_height = 1024;

	device_start_apollo_mono(device);
}

static DEVICE_START( apollo_mono15i ) {
	screen_data_t *screen_data = get_safe_token(device);

	memset(screen_data, 0, sizeof(screen_data_t));

	// monochrome 1024x800
	screen_data->device_id = SCREEN_DEVICE_ID_15I;
	screen_data->width = 1024;
	screen_data->height = 800;
	screen_data->buffer_width = 1024;
	screen_data->buffer_height = 1024;

	device_start_apollo_mono(device);
}

/*-------------------------------------------------
 DEVICE_RESET( apollo_mono19i/15i )
 -------------------------------------------------*/

static DEVICE_RESET( apollo_mono19i ) {
	screen_data_t *screen_data = get_safe_token(device);

	DLOG1(("device reset apollo screen"));

	memset(screen_data->image_memory, 0, screen_data->image_memory_size * 2);

	/* register for VBLANK callbacks */
	screen_data->screen->register_vblank_callback(vblank_state_delegate(FUNC(vblank_state_changed), device));
}

static DEVICE_RESET( apollo_mono15i ) {
	DEVICE_RESET_CALL(apollo_mono19i);
}

apollo_mono_device::apollo_mono_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_clear(screen_data_t);
}

const device_type APOLLO_MONO19I = &device_creator<apollo_mono19i_device>;

apollo_mono19i_device::apollo_mono19i_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: apollo_mono_device(mconfig, APOLLO_MONO19I, "Apollo 19\" Monochrome Screen", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void apollo_mono19i_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_mono19i_device::device_start()
{
	DEVICE_START_NAME( apollo_mono19i )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_mono19i_device::device_reset()
{
	DEVICE_RESET_NAME( apollo_mono19i )(this);
}


const device_type APOLLO_MONO15I = &device_creator<apollo_mono15i_device>;

apollo_mono15i_device::apollo_mono15i_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: apollo_mono_device(mconfig, APOLLO_MONO15I, "Apollo 15\" Monochrome Screen", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void apollo_mono15i_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_mono15i_device::device_start()
{
	DEVICE_START_NAME( apollo_mono15i )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_mono15i_device::device_reset()
{
	DEVICE_RESET_NAME( apollo_mono15i )(this);
}


