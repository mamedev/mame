// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tomasz Slanina
/***************************************************************************

    Zoran ZR36050 JPEG Image Compression Processor

    Expansion only, decoded with libjpeg once the whole code stream has
    arrived.  Outputs a decoded Y/Cb/Cr image for whatever additional
    hardware/software to figure out how to display.

    TODO:
    - compression
    - streams that rely on tables loaded into the marker memory

***************************************************************************/

#include "emu.h"
#include "zr36050.h"

#include "jpeglib.h"

#include <algorithm>
#include <csetjmp>

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ZR36050, zr36050_device, "zr36050", "Zoran ZR36050 JPEG image compression processor")

namespace {

struct jpeg_setjmp_error_mgr : public jpeg_error_mgr
{
	jpeg_setjmp_error_mgr(device_t &device) noexcept
		: m_device(device)
	{
		jpeg_std_error(this);
		error_exit = [] (j_common_ptr cinfo) { std::longjmp(static_cast<jpeg_setjmp_error_mgr *>(cinfo->err)->m_jump_buffer, 1); };
		output_message =
				[] (j_common_ptr cinfo)
				{
					char buffer[JMSG_LENGTH_MAX];
					(*cinfo->err->format_message)(cinfo, buffer);
					static_cast<jpeg_setjmp_error_mgr *>(cinfo->err)->m_device.logerror("libjpeg: %s\n", buffer);
				};
	}

	device_t &m_device;
	std::jmp_buf m_jump_buffer;
};

} // anonymous namespace


zr36050_device::zr36050_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ZR36050, tag, owner, clock)
	, m_code_read_cb(*this, 0xff)
	, m_end_cb(*this)
	, m_code(*this, "code", CODE_SIZE, ENDIANNESS_LITTLE)
	, m_regs{}
	, m_code_length(0)
	, m_code_state(CODE_IDLE)
	, m_code_marker(0)
	, m_code_remaining(0)
	, m_running(false)
	, m_width(0)
	, m_height(0)
{
}

void zr36050_device::device_start()
{
	save_item(NAME(m_regs));
	save_item(NAME(m_code_length));
	save_item(NAME(m_code_state));
	save_item(NAME(m_code_marker));
	save_item(NAME(m_code_remaining));
	save_item(NAME(m_running));
}

void zr36050_device::device_reset()
{
	// the marker memory keeps its contents
	std::fill_n(&m_regs[0], 0x40, 0);
	m_code_length = 0;
	m_code_state = CODE_IDLE;
	m_running = false;
	m_end_cb(0);
}

void zr36050_device::device_post_load()
{
	// the decoded image isn't saved, so rebuild it from the code stream
	if (BIT(m_regs[STATUS_1], 2) && m_code_length)
	{
		decode();
	}
}

u8 zr36050_device::read(offs_t offset)
{
	return m_regs[offset & 0x3ff];
}

void zr36050_device::write(offs_t offset, u8 data)
{
	offset &= 0x3ff;
	LOG("%s: write %03x = %02x\n", machine().describe_context(), offset, data);

	switch (offset)
	{
		case GO:
			if (BIT(data, 0))
			{
				go();
			}
			break;

		case STATUS_0:
		case STATUS_1:
			break;

		default:
			m_regs[offset] = data;
			break;
	}
}

void zr36050_device::code_w(u8 data)
{
	if (m_running && !BIT(m_regs[HARDWARE], 6))
	{
		code_byte(data);
	}
}

void zr36050_device::go()
{
	m_regs[STATUS_1] &= ~0x04;
	m_end_cb(0);

	// table preload passes complete at once
	if (BIT(m_regs[MODE], 7) || BIT(m_regs[MODE], 4))
	{
		if (BIT(m_regs[MODE], 7))
		{
			logerror("%s: compression is not supported\n", machine().describe_context());
		}
		finish();
		return;
	}

	m_code_length = 0;
	m_code_state = CODE_IDLE;
	m_running = true;

	// as code bus master, pull the stream in now
	if (BIT(m_regs[HARDWARE], 6))
	{
		for (u32 count = 0; m_running && (count < CODE_SIZE * 2); count++)
		{
			code_byte(m_code_read_cb());
		}

		if (m_running)
		{
			logerror("%s: no JPEG stream found\n", machine().describe_context());
			finish();
		}
	}
}

void zr36050_device::code_byte(u8 data)
{
	if (m_code_state == CODE_IDLE)
	{
		// skip anything before the SOI marker
		if (data == 0xff)
		{
			m_code_state = CODE_SOI;
		}
		return;
	}
	else if (m_code_state == CODE_SOI)
	{
		if (data == 0xd8)
		{
			m_code[0] = 0xff;
			m_code[1] = 0xd8;
			m_code_length = 2;
			m_code_state = CODE_MARKER;
		}
		else if (data != 0xff)
		{
			m_code_state = CODE_IDLE;
		}
		return;
	}

	m_code[m_code_length++] = data;

	switch (m_code_state)
	{
		case CODE_MARKER:
			if (data == 0xff)
			{
				m_code_state = CODE_MARKER_ID;
			}
			break;

		case CODE_MARKER_ID:
			if (data == 0xd9)
			{
				m_code_state = CODE_DONE;
			}
			else if (data == 0x01 || (data >= 0xd0 && data <= 0xd7))
			{
				m_code_state = CODE_MARKER;
			}
			else if (data != 0xff)
			{
				m_code_marker = data;
				m_code_state = CODE_LENGTH_HI;
			}
			break;

		case CODE_LENGTH_HI:
			m_code_remaining = data << 8;
			m_code_state = CODE_LENGTH_LO;
			break;

		case CODE_LENGTH_LO:
			m_code_remaining = std::max<int>((m_code_remaining | data) - 2, 0);
			if (!m_code_remaining)
			{
				m_code_state = (m_code_marker == 0xda) ? CODE_ENTROPY : CODE_MARKER;
			}
			else
			{
				m_code_state = CODE_SEGMENT;
			}
			break;

		case CODE_SEGMENT:
			if (!--m_code_remaining)
			{
				m_code_state = (m_code_marker == 0xda) ? CODE_ENTROPY : CODE_MARKER;
			}
			break;

		case CODE_ENTROPY:
			if (data == 0xff)
			{
				m_code_state = CODE_ENTROPY_FF;
			}
			break;

		case CODE_ENTROPY_FF:
			if (data == 0xd9)
			{
				m_code_state = CODE_DONE;
			}
			else if (data == 0x00 || (data >= 0xd0 && data <= 0xd7))
			{
				m_code_state = CODE_ENTROPY;
			}
			else if (data != 0xff)
			{
				m_code_marker = data;
				m_code_state = CODE_LENGTH_HI;
			}
			break;
	}

	if (m_code_state == CODE_DONE || m_code_length == CODE_SIZE)
	{
		if (m_code_state != CODE_DONE)
		{
			logerror("code stream too long, decoding the first %u bytes\n", CODE_SIZE);
		}
		decode();
		finish();
	}
}

void zr36050_device::finish()
{
	m_running = false;
	m_regs[STATUS_1] |= 0x04;
	m_end_cb(1);
}

void zr36050_device::decode()
{
	jpeg_decompress_struct cinfo;
	jpeg_setjmp_error_mgr jerr(*this);
	cinfo.err = &jerr;
	JSAMPARRAY buffer;
	int components;

	if (setjmp(jerr.m_jump_buffer)) // setjmp must be used in control expression
	{
		char msg[JMSG_LENGTH_MAX];
		(*cinfo.err->format_message)(reinterpret_cast<j_common_ptr>(&cinfo), msg);
		logerror("JPEG error: %s\n", msg);
		goto cleanup; // use goto to ensure longjmp can't cross an initialisation
	}

	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, m_code.target(), m_code_length);
	jpeg_read_header(&cinfo, TRUE);

	components = (cinfo.num_components == 1) ? 1 : 3;
	cinfo.out_color_space = (components == 1) ? JCS_GRAYSCALE : JCS_YCbCr;
	jpeg_start_decompress(&cinfo);

	m_width = cinfo.output_width;
	m_height = cinfo.output_height;
	m_image.assign(size_t(m_width) * m_height * 3, 0);
	for (size_t i = 0; i < m_image.size(); i += 3)
	{
		m_image[i + 1] = m_image[i + 2] = 0x80;
	}

	buffer = (*cinfo.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&cinfo), JPOOL_IMAGE, m_width * components, 1);
	while (cinfo.output_scanline < cinfo.output_height)
	{
		u8 *const dest = &m_image[size_t(cinfo.output_scanline) * m_width * 3];
		jpeg_read_scanlines(&cinfo, buffer, 1);
		if (components == 1)
		{
			for (int x = 0; x < m_width; x++)
			{
				dest[x * 3] = buffer[0][x];
			}
		}
		else
		{
			std::copy_n(buffer[0], m_width * 3, dest);
		}
	}

	jpeg_finish_decompress(&cinfo);
cleanup:
	jpeg_destroy_decompress(&cinfo);
}

rgb_t zr36050_device::ycbcr_to_rgb(u8 y, u8 cb, u8 cr)
{
	// the same fixed point arithmetic as libjpeg, so results match its RGB output
	const s32 b_diff = s32(cb) - 0x80;
	const s32 r_diff = s32(cr) - 0x80;
	const s32 r = y + ((91881 * r_diff + 0x8000) >> 16);
	const s32 g = y + ((-22553 * b_diff - 46802 * r_diff + 0x8000) >> 16);
	const s32 b = y + ((116130 * b_diff + 0x8000) >> 16);
	return rgb_t(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255));
}
