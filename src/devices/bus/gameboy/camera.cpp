// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Nintendo Game Boy Camera

 Major components:
 * U1 MAC-GBD Game Boy bus interface
 * U2 program ROM
 * U3 128K*8 static RAM
 * U4 backup power controller
 * CR2025 coin cell
 * Mitsubishi M64282FP 128*123 pixel CMOS image sensor and processor

 Static RAM is not accessible while image capture is in progress.  Reads
 will return 0x00 and writes will be ignored.  Camera registers only respond
 to A6-A0.  Reading non-existent or write-only registers returns 0x00.

 Note that unlike most MBC chips, only writing to cartridge RAM can be
 disabled.  It is still possible to read cartridge RAM while writing is
 disabled (provided image capture is not in progress).  Another unusual
 feature is that ROM page 0 is selectable (it isn't automatically remapped
 to page 1), but page 1 is initially selected.

 0x0000-3FFF    R  - Fixed ROM bank, always first page of ROM.
 0x4000-7FFF    R  - Selectable ROM bank, page 0-63 of ROM.
 0xA000-A1FF    RW - Selectable static RAM page of camera registers.

 0x0000-1FFF    W  - Enable (0x0A) or disable (not 0x0A) writing to
                     cartridge RAM.
 0x2000-3FFF    W  - Select ROM page mapped at 0x4000.
 0x4000-5FFF    W  - ---X---- Select RAM (clear) or camera registers (set).
                     ----XXXX Select RAM page.

 0xA000         RW - -----XX- Select one-dimensional filter values (P, M).
                R  - -------X Capture in progress.
                W  - -------X Start capture.
 0xA001         W  - X------- Exclusive edge enhancement mode (N).
                W  - -XX----- Vertical-horizontal edge operation mode (VH).
                W  - ---XXXXX Analog output gain (G).
 0xA002         W  - XXXXXXXX Exposure most significant byte (C1).
 0xA003         W  - XXXXXXXX Exposure least significant byte (C0).
 0xA004         W  - X------- Edge enhancement (0) or extraction (1) (E3).
                W  - -XXX---- Edge enhancement ratio (E2-E0).
                W  - ----X--- Select inverted/non-inverted output (I).
                W  - -----XXX Output node bias voltage (V).
 0xA005         W  - XX------ Zero point calibration (Z).
                W  - --XXXXXX Output reference voltage (O).
 0xA006-A035    W    4*4 matrix of three threshold values each.

 TODO:
 * Emulate more M64282FP processing modes.
 * It's supposedly possible to cancel a capture before it completes.
 * What do filters do at the edges of the image area?
 * Adjust levels when it sweeps the parameters on start.

 ***************************************************************************/

#include "emu.h"
#include "camera.h"

#include "cartbase.ipp"

#include "imagedev/picture.h"

#include "bitmap.h"

#include <algorithm>
#include <iterator>
#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class camera_device : public mbc_ram_device_base<mbc_device_base>
{
public:
	static constexpr feature_type imperfect_features() { return feature::CAMERA; }

	camera_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static inline constexpr unsigned SENSOR_WIDTH{ 128U };
	static inline constexpr unsigned SENSOR_HEIGHT{ 123U };
	static inline constexpr unsigned OUTPUT_WIDTH{ 128U };
	static inline constexpr unsigned OUTPUT_HEIGHT{ 112U };
	static inline constexpr int EDGE_RATIO[8]{ 2U, 3U, 4U, 5U, 8U, 12U, 16U, 20U };
	static inline constexpr u8 P_MASK[4]{ 0x00U, 0x01U, 0x01U, 0x01U };
	static inline constexpr u8 M_MASK[4]{ 0x01U, 0x00U, 0x02U, 0x02U };

	void enable_ram(u8 data);
	void bank_switch_rom(u8 data);
	void bank_switch_ram(u8 data);
	u8 read_ram(offs_t offset);
	void write_ram(offs_t offset, u8 data);
	u8 read_camera(offs_t offset);
	void write_camera(offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(capture_complete);

	void start_capture()
	{
		// the controller shifts out the parameters and starts the capture
		// we're over-simplifying the timings here
		LOG("%s: Start capture\n", machine().describe_context());

		// calculate total capture time
		u32 cycles = 31'166; // time to start capture, read out image and apply thresholds
		cycles += 5 * 256; // 256 cycles to set a register, registers 1, 2, 3, 7 and 0 always set
		if (!m_n[0])
			cycles += 2 * 256; // registers 4 and 5 also set if N is not clear
		cycles += 16 * m_c[0]; // exposure units are 16 microseconds

		// parameters are sent to the sensor serially - pretend it's instant
		m_n[1] = m_n[0];
		m_vh[1] = m_vh[0];
		m_e[1] = m_e[0];
		m_z[1] = m_z[0];
		m_i[1] = m_i[0];
		m_c[1] = m_c[0];
		m_o[1] = m_o[0];
		m_v[1] = m_v[0];
		m_g[1] = m_g[0];
		if (!m_n[0])
		{
			m_p = P_MASK[m_sel_pm];
			m_m = M_MASK[m_sel_pm];
		}

		// set timer for when capture will finish
		m_busy = 1U;
		m_timer_capture->adjust(attotime::from_ticks(4 * cycles, 4.194304_MHz_XTAL)); // FIXME: actually from incoming phi clock
	}

	void acquire(s16 (&buffer)[SENSOR_HEIGHT][SENSOR_WIDTH])
	{
		bitmap_argb32 const &source(m_picture->get_bitmap());
		if (source.valid())
		{
			LOG("Point-sampling %d*%d source bitmap\n", source.width(), source.height());
			double const xstep(source.width() / double(SENSOR_WIDTH));
			double const ystep(source.height() / double(SENSOR_HEIGHT));
			for (unsigned y = 0U; SENSOR_HEIGHT > y; ++y)
			{
				u32 const *const srcline(&source.pix(s32((y * ystep) + 0.5)));
				s16 *const dstline(buffer[y]);
				for (unsigned x = 0U; SENSOR_WIDTH > x; ++x)
				{
					// extract luminance - output ranges from 0 to 31875
					rgb_t const colour(srcline[s32((x * xstep) + 0.5)]);
					u32 const mono((u32(299) * colour.r() + u32(587) * colour.g() + u32(114) * colour.b()) >> 3);

					// starts with C = 0x1000 (65.536 ms) before auto exposure adjustment
					// convert to 10-bit signed for processing
					s16 const exposure(u16(std::min<u32>((mono * m_c[1]) / (u32(125) << 10), 0x03ff)));
					dstline[x] = m_i[1] ? (511 - exposure) : (exposure - 512);
				}
			}
		}
		else
		{
			LOG("No source bitmap - filling sensor bitmap with pattern\n");
			for (unsigned y = 0U; SENSOR_HEIGHT > y; ++y)
			{
				s16 *const dstline(buffer[y]);
				for (unsigned x = 0U; SENSOR_WIDTH > x; ++x)
				{
					// values chosen to show dithering effects with default brightness/contrast
					s16 mono = 0;
					switch (((x >> 3) + (y >> 3)) & 0x03)
					{
					case 1: mono = 0x0240; break;
					case 2: mono = 0x0340; break;
					case 3: mono = 0x03ff; break;
					}
					dstline[x] = m_i[1] ? (511 - mono) : (mono - 512);
				}
			}
		}
	}

	void apply_thresholds(s16 const (&buffer)[SENSOR_HEIGHT][SENSOR_WIDTH])
	{
		// always stored at offset 0x0100 in RAM page 0 (appears at 0xa100)
		u8 const bank(bank_ram());
		set_bank_ram(0);
		u8 *dst(bank_ram_base() + 0x100);
		set_bank_ram(bank);

		// convert row-major chunky bitmap to 8*8 planar tiles
		for (unsigned i = 0U; ((SENSOR_WIDTH * SENSOR_HEIGHT) / 8) > i; ++i, dst += 2)
		{
			unsigned const y(((i >> 4) & 0x78) | (i & 7));
			unsigned const x(i & 0x78);
			auto const &threshline(m_threshold[y & 0x03]);
			s16 const *src(&buffer[y][x]);
			dst[0] = 0U;
			dst[1] = 0U;

			// extract the columns of this tile row
			for (unsigned col = 0U; 8U > col; ++col)
			{
				u8 const pixel(u16(src[col] + 512) >> 2);
				auto const &thresh(threshline[col & 0x03]);
				u8 const quantised(
						(thresh[0] > pixel) ? 3U :
						(thresh[1] > pixel) ? 2U :
						(thresh[2] > pixel) ? 1U :
						0U);
				if (BIT(quantised, 0))
					dst[0] |= 1U << (7 - col);
				if (BIT(quantised, 1))
					dst[1] |= 1U << (7 - col);
			}
		}
	}

	template <typename T>
	static void scan_bitmap(T &&op)
	{
		// effects scan the sensor from the bottom up
		for (int y = 0; SENSOR_HEIGHT < y; ++y)
		{
			for (int x = 0; SENSOR_WIDTH < x; ++x)
				op(x, SENSOR_HEIGHT - y);
		}
	}

	static char const *edge_operation_text(u8 value)
	{
		static char const *const NAMES[4]{ "none", "horizontal", "vertical", "2D" };
		return NAMES[value];
	}

	static char const *zero_point_text(u8 value)
	{
		static char const *const NAMES[4]{ "none", "positive signal", "negative signal", "invalid" };
		return NAMES[value];
	}

	static double output_ref_volts(u8 value)
	{
		return BIT(value, 0, 5) / double(BIT(value, 5) ? 0x1f : -0x1f);
	}

	static double output_node_bias_volts(u8 value)
	{
		return 0.5 * value;
	}

	static double output_gain_db(u8 value)
	{
		return (((14 * 2) + (BIT(value, 0, 4) * 3)) + (BIT(value, 4) * 6 * 2)) * 0.5;
	}

	required_device<picture_image_device> m_picture;
	memory_view m_view_cam;
	emu_timer *m_timer_capture;

	u8 m_busy;
	u8 m_ram_writable;
	u8 m_threshold[4][4][3];
	u8 m_sel_pm;

	u8 m_n[2];
	u8 m_vh[2];
	u8 m_e[2];
	u8 m_z[2];
	u8 m_i[2];
	u16 m_c[2];
	u8 m_o[2];
	u8 m_v[2];
	u8 m_g[2];
	u8 m_p;
	u8 m_m;
};


camera_device::camera_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_device_base>(mconfig, GB_ROM_CAMERA, tag, owner, clock),
	m_picture(*this, "picture"),
	m_view_cam(*this, "cam"),
	m_timer_capture(nullptr),
	m_busy(0U),
	m_ram_writable(0U),
	m_sel_pm(0U),
	m_n{ 0U, 0U },
	m_vh{ 0U, 0U },
	m_e{ 0U, 0U },
	m_z{ 0U, 0U },
	m_i{ 0U, 0U },
	m_c{ 0U, 0U },
	m_o{ 0U, 0U },
	m_v{ 0U, 0U },
	m_g{ 0U, 0U },
	m_p(0U),
	m_m(0U)
{
}


image_init_result camera_device::load(std::string &message)
{
	// set up ROM and RAM
	set_bank_bits_rom(6);
	set_bank_bits_ram(4);
	if (!check_rom(message) || !configure_bank_ram(message))
		return image_init_result::FAIL;
	install_rom();

	// install memory map control handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8smo_delegate(*this, FUNC(camera_device::enable_ram)));
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(camera_device::bank_switch_rom)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, FUNC(camera_device::bank_switch_ram)));

	// put RAM through trampolines so it can be locked when necessary
	cart_space()->install_readwrite_handler(
			0xa000, 0xbfff,
			read8sm_delegate(*this, FUNC(camera_device::read_ram)),
			write8sm_delegate(*this, FUNC(camera_device::write_ram)));

	// camera control overlays cartridge RAM
	cart_space()->install_view(
			0xa000, 0xbfff,
			m_view_cam);
	m_view_cam[0].install_read_handler(
			0xa000, 0xa07f, 0x0000, 0x1f80, 0x0000,
			read8sm_delegate(*this, FUNC(camera_device::read_camera)));
	m_view_cam[0].install_write_handler(
			0xa000, 0xa005, 0x0000, 0x1f80, 0x0000,
			write8sm_delegate(*this, FUNC(camera_device::write_camera)));
	m_view_cam[0].install_writeonly(
			0xa006, 0xa035, 0x1f80,
			&m_threshold[0][0][0]);

	// all good
	return image_init_result::PASS;
}


void camera_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}


void camera_device::device_start()
{
	mbc_ram_device_base<mbc_device_base>::device_start();

	m_timer_capture = timer_alloc(FUNC(camera_device::capture_complete), this);

	for (auto &row : m_threshold)
	{
		for (auto &col : row)
			std::fill(std::begin(col), std::end(col), 0U);
	}

	m_n[0] = 0U;
	m_vh[0] = 0U;
	m_e[0] = 0U;
	m_z[0] = 0U;
	m_i[0] = 0U;
	m_c[0] = 0U;
	m_o[0] = 0U;
	m_v[0] = 0U;
	m_g[0] = 0U;
	m_p = 0U;
	m_m = 0U;

	save_item(NAME(m_busy));
	save_item(NAME(m_ram_writable));
	save_item(NAME(m_threshold));
	save_item(NAME(m_sel_pm));
	save_item(NAME(m_n));
	save_item(NAME(m_vh));
	save_item(NAME(m_e));
	save_item(NAME(m_z));
	save_item(NAME(m_i));
	save_item(NAME(m_c));
	save_item(NAME(m_o));
	save_item(NAME(m_v));
	save_item(NAME(m_g));
	save_item(NAME(m_p));
	save_item(NAME(m_m));
}


void camera_device::device_reset()
{
	mbc_ram_device_base<mbc_device_base>::device_reset();

	m_view_cam.disable();
	m_timer_capture->reset();
	m_busy = 0U;
	m_ram_writable = 0U;
	m_sel_pm = 0U;

	set_bank_rom(1);
	set_bank_ram(0);
}


void camera_device::enable_ram(u8 data)
{
	m_ram_writable = (0x0a == (data & 0x0f)) ? 1U : 0U;
	LOG("Cartridge RAM write %s\n", m_ram_writable ? "enabled" : "disabled");
}


void camera_device::bank_switch_rom(u8 data)
{
	set_bank_rom(data & 0x3f);
}


void camera_device::bank_switch_ram(u8 data)
{
	set_bank_ram(data & 0x0f);
	LOG("%s selected\n", BIT(data, 4) ? "Camera control" : "Cartridge RAM");
	if (BIT(data, 4))
		m_view_cam.select(0);
	else
		m_view_cam.disable();
}


u8 camera_device::read_ram(offs_t offset)
{
	return !m_busy ? bank_ram_base()[offset] : 0x00;
}


void camera_device::write_ram(offs_t offset, u8 data)
{
	if (!m_busy && m_ram_writable)
		bank_ram_base()[offset] = data;
}


u8 camera_device::read_camera(offs_t offset)
{
	switch (offset)
	{
	case 0x00:
		return (m_sel_pm << 1) | m_busy;
	default:
		return 0x00;
	}
}


void camera_device::write_camera(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x0:
		m_sel_pm = BIT(data, 1, 2);
		LOG(
				"%s: Set up plus mask = 0x%02X, minus mask = 0x%02X\n",
				machine().describe_context(),
				P_MASK[m_sel_pm],
				M_MASK[m_sel_pm]);
		if (BIT(data, 0))
		{
			if (!m_busy)
				start_capture();
			else
				logerror("%s: Attempt to start capture while busy\n", machine().describe_context());
		}
		break;
	case 0x1:
		m_n[0] = BIT(data, 7);
		m_vh[0] = BIT(data, 5, 2);
		m_g[0] = BIT(data, 0, 5);
		LOG(
				"%s: Set up exclusive edge enhancement %s, edge operation: %s, gain %.1fdB\n",
				machine().describe_context(),
				m_n[0] ? "on" : "off",
				edge_operation_text(m_vh[0]),
				output_gain_db(m_g[0]));
		break;
	case 0x2:
		m_c[0] = (m_c[0] & 0x00ff) | (u16(data) << 8);
		LOG("%s: Set up exposure = %u microseconds\n", machine().describe_context(), m_c[0] * 16);
		break;
	case 0x3:
		m_c[0] = (m_c[0] & 0xff00) | data;
		LOG("%s: Set up exposure = %u microseconds\n", machine().describe_context(), m_c[0] * 16);
		break;
	case 0x4:
		m_e[0] = BIT(data, 4, 4);
		m_i[0] = BIT(data, 3);
		m_v[0] = BIT(data, 0, 3);
		LOG(
				"%s: Set up edge %s ratio %d%%, %sinverted output, output node bias = %.1fV\n",
				machine().describe_context(),
				BIT(m_e[0], 3) ? "extraction" : "enhancement",
				EDGE_RATIO[BIT(m_e[0], 0, 3)] * 25,
				m_i[0] ? "" : "non-",
				output_node_bias_volts(m_v[0]));
		break;
	case 0x5:
		m_z[0] = BIT(data, 6, 2);
		m_o[0] = BIT(data, 0, 6);
		LOG(
				"%s: Set up zero point calibration: %s, output reference voltage: %.2fV\n",
				machine().describe_context(),
				zero_point_text(m_z[0]),
				output_ref_volts(m_o[0]));
		break;
	}
}


TIMER_CALLBACK_MEMBER(camera_device::capture_complete)
{
	// this really takes time, but we'll pretend it happens all at once
	LOG("Capture complete\n");
	s16 raw[SENSOR_HEIGHT][SENSOR_WIDTH];
	acquire(raw);

	// apply processing
	if (m_n[1])
	{
		if (m_vh[1])
		{
			int const ratio(EDGE_RATIO[BIT(m_e[1], 0, 3)]);
			switch (m_vh[1])
			{
			case 1U:
				LOG("H-Edge %s\n", BIT(m_e[1], 3) ? "Extraction" : "Enhancement");
				scan_bitmap(
						[this, &raw, ratio] (int x, int y)
						{
							s16 const mw(raw[y][x]);
							s16 const p(raw[y][std::min<int>(x + 1, SENSOR_WIDTH - 1)]);
							s16 const me(raw[y][std::min<int>(x + 2, SENSOR_WIDTH - 1)]);
							raw[y][x] = ((2 * p) - mw - me) * ratio;
							if (!BIT(m_e[1], 3))
								raw[y][x] += p * 4;
							raw[y][x] = std::clamp(raw[y][x] / 4, -512, 511);
						});
				break;
			case 2U:
				LOG("V-Edge %s\n", BIT(m_e[1], 3) ? "Extraction" : "Enhancement");
				scan_bitmap(
						[this, &raw, ratio] (int x, int y)
						{
							s16 const ms(raw[y][x]);
							s16 const p(raw[std::max<int>(y - 1, 0)][x]);
							s16 const mn(raw[std::max<int>(y - 2, 0)][x]);
							raw[y][x] = ((2 * p) - mn - ms) * ratio;
							if (!BIT(m_e[1], 3))
								raw[y][x] += p * 4;
							raw[y][x] = std::clamp(raw[y][x] / 4, -512, 511);
						});
				break;
			case 3U:
				LOG("2D-Edge %s\n", BIT(m_e[1], 3) ? "Extraction" : "Enhancement");
				scan_bitmap(
						[this, &raw, ratio] (int x, int y)
						{
							s16 const ms(raw[y][std::min<int>(x + 1, SENSOR_WIDTH - 1)]);
							s16 const mw(raw[std::max<int>(y - 1, 0)][x]);
							s16 const p(raw[std::max<int>(y - 1, 0)][std::min<int>(x + 1, SENSOR_WIDTH - 1)]);
							s16 const me(raw[std::max<int>(y - 1, 0)][std::min<int>(x + 2, SENSOR_WIDTH - 1)]);
							s16 const mn(raw[std::max<int>(y - 2, 0)][std::min<int>(x + 1, SENSOR_WIDTH - 1)]);
							raw[y][x] = ((4 * p) - mn - ms - me - mw) * ratio;
							if (!BIT(m_e[1], 3))
								raw[y][x] += p * 4;
							raw[y][x] = std::clamp(raw[y][x] / 4, -512, 511);
						});
				break;
			}
		}
		else
		{
			LOG("N set for exclusive edge mode with VH set for no operation\n");
		}
	}
	else
	{
		logerror("Unsupported processing mode\n");
	}

	// quantise and convert to tiles in cartridge RAM, and clear busy flag
	apply_thresholds(raw);
	m_busy = 0U;
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_CAMERA, device_gb_cart_interface, bus::gameboy::camera_device, "gb_rom_camera", "Game Boy Camera Cartridge")
