// license:BSD-3-Clause
// copyright-holders:Ville Linde, Eduardo Cruz
/*
    Rolling Extreme
    Gaelco, 1999

    PCB Layout
    ----------

    REF.991015
      |--------------------------------------------------|
      |TL074 TDA1543  LP61256         ROE.45     ROE.58  |
      |                                                  |
      |TL074 TDA1543  LP61256         ROE.47     ROE.60  |-|
      |                                                  | |DB9
      |              |--------| KM4216C258  KM4216C258   |-|
      |              |ALTERA  |                          |
      |      LP61256 |FLEX    | KM4216C258  KM4216C258   |
      |      LP61256 |EPF10K50|                          |
    |-|      LP61256 |--------| KM4216C258  KM4216C258   |
    | |2444CS           CY2308                           |
    | |           |--------|    KM4216C258  KM4216C258   |
    | |  L4955    |ALTERA  |                             |
    | |           |FLEX    |    |------------|           |
    | |LED1       |EPF10K100    |CHIP EXPRESS|KM4216C258 |
    | |LED2 60MHz |--------|    |D3PLUS-4    |           |
    |-|    |----------|         |T2032-01    |KM4216C258 |
      |    |DSP       |         |N9G1554     |           |
      |    |TMS320C82 |         |9939 CA TWN |           |
      |    |GGP60     |         |------------|           |
      |    |1997 TI   |      93C66                       |
      |    |----------|         |-------|                |-|
      |          ROE.17         |ALTERA |         90MHz  | |DB9
      |K4S643232-TC80           |MAX    |KM718V895T-72   |-|
      |          ROE.18   ROE.38|EPM7160|                |
      |K4S643232-TC80           |-------|                |
      |--------------------------------------------------|
*/

/*

    MP Interrupts:

        External Interrupt 1:       0x4003ef78
        External Interrupt 2:       -
        External Interrupt 3:       0x40041d30
        Memory Fault:               0x40043ae8

    PP0 Interrupts:
        Task:                       0x400010a0 / 0x400001a0

    PP1 Interrupts:
        Task:                       0x4004c6e8

    PP0 runs the polygon queue and the graphics FIFO transfers; PP1 mixes
    the sound.


    Memory locations:

        [0x00000084] PP0 ready flag
        [0x00000090] MP submitted packet count
        [0x00000094] PP0 consumed packet count
        [0x00000320] 2000000-TCOUNT in XINT3 handler
        [0x01010668] copied from (word)0xb0000004 in XINT3 handler

        ROM [0xc0050000] 0x10000 floats copied to [0x40180000]

        ROM [0xff800000 - 0xff80171f] sound data headers, 32 bytes each
            Word 0: sound data pointer (offset from 0xff800000)
            Word 1: uncompressed length?
            Word 2: ?
            Word 3: same as word 1?
            Word 4: ?
            Word 5: ?
            Word 6: ?
            Word 7: 0

        ROM [0xff801720 -> ] compressed audio data

        [0x0000100c] bitmask of active sound channels (max 16 channels?)
        [0x00001018 -> ] sound channel data, 64 bytes each
            +0x00: sound data pointer in ROM
            +0x04: 0
            +0x08: uncompressed length
            +0x0c: uncompressed length
            +0x10: 0?
            +0x14: 0 - sample rate?
            +0x18: 0?
            +0x1c: sample rate? (halfword)
            +0x1e: ?? (halfword)
            +0x20: ?
            +0x24: ?
            +0x28: ?
            +0x2c: ?
            +0x30: ?
            +0x34: ?
            +0x36: ? (halfword)
            +0x38: ?
            +0x3c: ?



    Texture ROM decode:

      {ic45}     {ic47}     {ic58}     {ic60}
    [2,0][0,0] [2,1][0,1] [3,0][1,0] [3,1][1,1]



    TODO:

    - Complete TMS320C82 PP instruction, interrupt and transfer timing support
    - Verify the inferred audio clock and the assignment of the four DAC outputs
    - Verify header translucency modes/coefficients and translucent Z writes
    - Minor Z-buffer issues
    - Wrong textures in a few places (could be a CPU core bug)
    - Link word period: the IC44 serial divider isn't dumped
    - Verify long-packet link framing against a CPLD dump or a real cabinet
      link (sender 0053, receiver expects 4053)

*/

#include "emu.h"

#include "cpu/tms320c82/tms320c82.h"
#include "machine/eepromser.h"
#include "sound/dac.h"
#include "sound/flt_biquad.h"
#include "video/poly.h"

#include "emuopts.h"
#include "screen.h"
#include "speaker.h"
#include "video/rgbutil.h"

#include "corefloat.h"
#include "endianness.h"
#include "multibyte.h"

#include "asio.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

namespace {

constexpr bool BILINEAR = true;

// Float-to-integer conversion is undefined for NaN and for values outside the
// int32_t range, which an infinite 1/Z produces. Saturate, mapping NaN to zero.
int32_t saturate_int32(float value)
{
	if (std::isnan(value))
		return 0;
	if (value >= 2147483648.0f)
		return std::numeric_limits<int32_t>::max();
	if (value < -2147483648.0f)
		return std::numeric_limits<int32_t>::min();
	return int32_t(value);
}

// Cabinet link over TCP, carrying the words written to the link registers.
// The socket is polled from the emulation thread. With -comm_framesync, the
// peers exchange a clock marker every interval and release received words at
// a common barrier.
class rollext_link_socket
{
public:
	rollext_link_socket(emu_options const &options, std::function<void(uint16_t)> receive)
		: m_acceptor(m_context)
		, m_socket(m_context)
		, m_receive(std::move(receive))
		, m_framesync(options.comm_framesync())
	{
		std::error_code error;
		m_server = !*options.comm_remotehost();
		auto const address = asio::ip::make_address(m_server ? options.comm_localhost() : options.comm_remotehost(), error);
		if (error)
		{
			osd_printf_error("Rolling Extreme link: invalid numeric address\n");
			return;
		}
		unsigned long const value = std::strtoul(m_server ? options.comm_localport() : options.comm_remoteport(), nullptr, 10);
		if (!value || value > 65535)
		{
			osd_printf_error("Rolling Extreme link: invalid port\n");
			return;
		}
		uint16_t const port = value;
		m_endpoint = asio::ip::tcp::endpoint(address, port);
		if (m_server)
		{
			m_acceptor.open(m_endpoint.protocol(), error);
			if (!error)
				m_acceptor.set_option(asio::socket_base::reuse_address(true), error);
			if (!error)
				m_acceptor.bind(m_endpoint, error);
			if (!error)
				m_acceptor.listen(1, error);
			if (error)
			{
				osd_printf_error("Rolling Extreme link: %s\n", error.message());
				return;
			}
		}
		m_valid = true;
		retry();
	}

	void poll()
	{
		poll_io();
		if (m_framesync && m_valid && m_initial_connection)
		{
			m_initial_connection = false;
			// Wait for the peer before the first interval so both clocks start
			// together; a client may start before the listener.
			using clock = std::chrono::steady_clock;
			clock::duration remaining = std::chrono::seconds(2);
			clock::duration const max_slice = std::chrono::milliseconds(100);
			while (!m_connected && remaining > clock::duration::zero())
			{
				auto const started = clock::now();
				retry();
				std::this_thread::sleep_for(std::chrono::microseconds(100));
				poll_io();
				remaining -= std::min(clock::now() - started, max_slice);
			}
			if (!m_connected)
				osd_printf_error("Rolling Extreme link: initial connection timed out; start both synchronized cabinets together\n");
		}
		if (!m_connected || !m_framesync)
			return;

		// Send this interval's marker, wait for the peer's, then release the
		// words received in intervals up to this barrier.
		++m_local_tick;
		queue(0x53, 2, m_local_tick);
		poll_io();
		// Give up after two seconds without the peer's marker. Each poll counts
		// at most 100 ms, so a suspended host doesn't use up the wait.
		using clock = std::chrono::steady_clock;
		clock::duration remaining = std::chrono::seconds(2);
		clock::duration const max_slice = std::chrono::milliseconds(100);
		while (m_connected && int16_t(m_local_tick - m_remote_tick) > 0)
		{
			auto const started = clock::now();
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			poll_io();
			remaining -= std::min(clock::now() - started, max_slice);
			if (m_connected && int16_t(m_local_tick - m_remote_tick) > 0 && remaining <= clock::duration::zero())
			{
				if (m_clock_seen)
					osd_printf_error("Rolling Extreme link: clock sync timed out after peer stopped advancing; restart both cabinets\n");
				else
					osd_printf_error("Rolling Extreme link: clock sync timed out before first marker; enable -comm_framesync on both peers and restart both cabinets\n");
				close();
				break;
			}
		}
		while (m_connected && !m_input_batches.empty() && int16_t(m_local_tick - m_input_batches.front().tick) >= 0)
		{
			for (uint16_t word : m_input_batches.front().words)
				m_receive(word);
			m_input_batches.pop_front();
		}
	}

	void retry()
	{
		if (!m_valid || m_connecting || m_connected)
			return;
		m_connecting = true;
		auto connected = [this] (std::error_code error)
		{
			m_connecting = false;
			if (error)
			{
				close();
				return;
			}
			m_socket.set_option(asio::ip::tcp::no_delay(true), error);
			m_connected = true;
			m_initial_connection = false;
			m_local_tick = m_remote_tick = 0;
			m_clock_seen = false;
			read();
		};
		if (m_server)
			m_acceptor.async_accept(m_socket, std::move(connected));
		else
			m_socket.async_connect(m_endpoint, std::move(connected));
	}

	void send(uint16_t data, uint8_t node)
	{
		if (!m_connected)
			return; // an unplugged RS-485 cable cannot retain words
		if (m_pending.size() >= 16384 || m_output.size() >= 32)
		{
			close();
			return;
		}
		queue(0x52, node & 3, data);
	}

private:
	struct input_batch
	{
		uint16_t tick = 0;
		std::vector<uint16_t> words;
	};

	// Messages are a type byte, a node or marker byte and a big-endian word.
	void queue(uint8_t type, uint8_t node, uint16_t data)
	{
		uint8_t message[4] = { type, node };
		put_u16be(&message[2], data);
		m_pending.insert(m_pending.end(), std::begin(message), std::end(message));
	}

	void close()
	{
		std::error_code ignored;
		m_socket.close(ignored);
		m_connected = false;
		m_pending.clear();
		m_input_pending.clear();
		m_input_batches.clear();
		// Keep any outstanding async_write buffer alive until its callback.
	}
	void read()
	{
		asio::async_read(m_socket, asio::buffer(m_input), [this] (std::error_code error, size_t length)
		{
			if (error || length != 4)
			{
				close();
				return;
			}
			uint16_t const data = get_u16be(&m_input[2]);
			if (m_input[0] == 0x52 && m_input[1] <= 3)
			{
				if (!m_framesync)
				{
					m_receive(data);
				}
				else
				{
					if (m_input_pending.size() >= 4096)
					{
						close();
						return;
					}
					m_input_pending.push_back(data);
				}
			}
			else if (m_input[0] == 0x53 && m_input[1] == 2)
			{
				m_remote_tick = data;
				m_clock_seen = true;
				if (m_framesync)
				{
					if (m_input_batches.size() >= 32)
					{
						close();
						return;
					}
					m_input_batches.push_back({ data, std::move(m_input_pending) });
					m_input_pending.clear();
				}
			}
			else
			{
				close();
				return;
			}
			read();
		});
	}

	void write()
	{
		asio::async_write(m_socket, asio::buffer(m_output.front()), [this] (std::error_code error, size_t length)
		{
			if (error || length != m_output.front().size())
			{
				close();
				m_output.clear();
				return;
			}
			m_output.pop_front();
			if (!m_output.empty())
				write();
		});
	}

	void poll_io()
	{
		// Flush pending words on every poll, also during continuous serial DMA.
		if (m_connected && !m_pending.empty())
		{
			bool const idle = m_output.empty();
			m_output.push_back(std::move(m_pending));
			m_pending.clear();
			if (idle)
				write();
		}
		m_context.restart();
		m_context.poll();
	}

	asio::io_context m_context;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_socket;
	asio::ip::tcp::endpoint m_endpoint;
	std::function<void(uint16_t)> m_receive;
	std::array<uint8_t, 4> m_input{};
	std::deque<std::vector<uint8_t>> m_output;
	std::vector<uint8_t> m_pending;
	std::vector<uint16_t> m_input_pending;
	std::deque<input_batch> m_input_batches;
	bool m_valid = false, m_server = false, m_connecting = false, m_connected = false;
	bool const m_framesync;
	bool m_clock_seen = false, m_initial_connection = true;
	uint16_t m_local_tick = 0, m_remote_tick = 0;
};

struct rollext_polydata
{
	uint32_t tex_origin_y;
	uint32_t tex_origin_x;
	uint32_t pal;
	float uoz_dx;
	float uoz_dy;
	float voz_dx;
	float voz_dy;
	float ooz_dx;
	float ooz_dy;
	float baseu;
	float basev;
	float basez;
	uint32_t zmul;
	uint16_t alpha;
};

class rollext_renderer : public poly_manager<float, rollext_polydata, 4>
{
	friend class rollext_state;

public:
	rollext_renderer(screen_device &screen)
		: poly_manager<float, rollext_polydata, 4>(screen.machine())
		, m_screen(screen)
	{
		m_fb[0].allocate(512, 384);
		m_fb[1].allocate(512, 384);

		m_zb.allocate(512, 384);

		m_palette = std::make_unique<rgb_t []>(32768);
		m_texture_mask = std::make_unique<uint8_t []>(TEXTURE_MASK_SIZE);

		m_fb_current = 0;
		m_fb[0].fill(0xff000000);
		m_fb[1].fill(0xff000000);
		m_zb.fill(0xffffffff);

		// Graphics device memories; nothing rebuilds them from CPU-visible state.
		auto &save = screen.machine().save();
		save.save_item(NAME(m_fb[0]));
		save.save_item(NAME(m_fb[1]));
		save.save_item(NAME(m_zb));
		save.save_pointer(NAME(m_palette), 32768);
		save.save_pointer(NAME(m_texture_mask), TEXTURE_MASK_SIZE);
		save.save_item(NAME(m_fb_current));
		save.save_item(NAME(m_clip.min_x));
		save.save_item(NAME(m_clip.max_x));
		save.save_item(NAME(m_clip.min_y));
		save.save_item(NAME(m_clip.max_y));
	}

	template<bool TestZ>
	void render_texture_scan(int32_t scanline, const extent_t &extent, const rollext_polydata &extradata, int threadid);

	void set_texture_ram(uint8_t* texture_ram) { m_texture_ram = texture_ram; }
	void process_packet(const uint32_t *packet);

	void palette_write(int index, uint16_t data);
	void texture_mask_write(int index, uint8_t data);

	void clear_fb();
	void display(bitmap_rgb32 *bitmap, const rectangle &cliprect);
private:
	// One bit per texel, LSB first, for the first 2048 rows of 2048 texels
	static constexpr uint32_t TEXTURE_MASK_SIZE = 2048 * 2048 / 8;

	bool texel_masked(uint32_t x, uint32_t y) const;

	bitmap_rgb32 m_fb[2];
	std::unique_ptr<rgb_t []> m_palette;
	std::unique_ptr<uint8_t []> m_texture_mask;

	bitmap_ind32 m_zb;

	uint8_t m_fb_current;
	rectangle m_clip = rectangle(0, 511, 0, 383);

	uint8_t *m_texture_ram = nullptr;

	screen_device &m_screen;
	uint32_t m_logged_blend[16]{}; // header mode/coefficient pairs already logged
};

// FIXME: the mask RAM only covers the first 2048 of the 16384 texture rows.
// The others are drawn unmasked; how the graphics ASIC handles them is unknown.
inline bool rollext_renderer::texel_masked(uint32_t x, uint32_t y) const
{
	x &= 0x7ff;
	y &= 0x3fff;
	return y < 2048 && BIT(m_texture_mask[(y * 2048 + x) / 8], x & 7);
}

template<bool TestZ>
void rollext_renderer::render_texture_scan(int32_t scanline, const extent_t &extent, const rollext_polydata &extradata, int threadid)
{
	uint32_t *fb = &m_fb[m_fb_current].pix(scanline);
	uint32_t *const zb = &m_zb.pix(scanline);

	uint32_t tex_origin_y = extradata.tex_origin_y;
	uint32_t tex_origin_x = extradata.tex_origin_x;

	float baseu = extradata.baseu;
	float basev = extradata.basev;
	float basez = extradata.basez;
	float uoz_dx = extradata.uoz_dx;
	float uoz_dy = extradata.uoz_dy;
	float voz_dx = extradata.voz_dx;
	float voz_dy = extradata.voz_dy;
	float ooz_dx = extradata.ooz_dx;
	float ooz_dy = extradata.ooz_dy;

	uint32_t zmul = extradata.zmul;

	int palnum = extradata.pal << 8;

	float uoz = baseu + (uoz_dx * (extent.startx - 256)) + (-uoz_dy * (scanline - 192));
	float voz = basev + (voz_dx * (extent.startx - 256)) + (-voz_dy * (scanline - 192));
	float ooz = basez + (ooz_dx * (extent.startx - 256)) + (-ooz_dy * (scanline - 192));

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		// FIXME: Verify the graphics ASIC's reciprocal precision.
		// Depth truncates, allowing for single-precision roundoff just below an
		// integer, so coplanar surfaces get the same value.
		float const z = 1.0f / ooz;
		float const depth = z * zmul;
		uint32_t const zbufval = saturate_int32(depth + std::abs(depth) * std::numeric_limits<float>::epsilon());

		if (!TestZ || zbufval <= zb[x])
		{
			float u = uoz * z;
			float v = voz * z;

			// Floor keeps bilinear weights nonnegative at wrapped texture edges.
			// Texel coordinates wrap, so they're unsigned.
			float const intu = std::floor(u), intv = std::floor(v);
			uint32_t const tx = tex_origin_x + saturate_int32(intu);
			uint32_t const ty = tex_origin_y + saturate_int32(intv);

			if (BILINEAR)
			{
				// u - floor(u) rounds up to 1.0 just below an integer.
				uint8_t const fracu = std::min<int32_t>(saturate_int32((u - intu) * 256.0f), 255);
				uint8_t const fracv = std::min<int32_t>(saturate_int32((v - intv) * 256.0f), 255);

				// The texture mask is a one-bit visibility test, separate from
				// colour interpolation (as in the related Gaelco 3D renderer).
				// TODO: Verify this sampling rule on the graphics ASIC.
				if (!texel_masked(tx, ty))
				{
					uint32_t const tex00 = m_palette[palnum + m_texture_ram[(ty & 0x3fff) * 2048 + (tx & 0x7ff)]];
					uint32_t const tex01 = m_palette[palnum + m_texture_ram[(ty & 0x3fff) * 2048 + ((tx + 1) & 0x7ff)]];
					uint32_t const tex10 = m_palette[palnum + m_texture_ram[((ty + 1) & 0x3fff) * 2048 + (tx & 0x7ff)]];
					uint32_t const tex11 = m_palette[palnum + m_texture_ram[((ty + 1) & 0x3fff) * 2048 + ((tx + 1) & 0x7ff)]];
					rgbaint_t color(rgbaint_t::bilinear_filter(tex00, tex01, tex10, tex11, fracu, fracv));
					if (extradata.alpha < 256)
						color.blend(rgbaint_t(fb[x]), extradata.alpha);
					fb[x] = color.to_rgba_clamp();
					zb[x] = zbufval;
				}
			}
			else
			{
				if (!texel_masked(tx, ty))
				{
					uint8_t pen = m_texture_ram[((ty & 0x3fff) * 2048) + (tx & 0x7ff)];
					uint32_t texel = m_palette[palnum + (pen)];
					rgbaint_t color(texel);
					if (extradata.alpha < 256)
						color.blend(rgbaint_t(fb[x]), extradata.alpha);
					fb[x] = color.to_rgba_clamp();
					zb[x] = zbufval;
				}
			}
		}

		uoz += uoz_dx;
		voz += voz_dx;
		ooz += ooz_dx;
	}
}

void rollext_renderer::process_packet(const uint32_t *packet)
{
	// PP0 rewrites bits 31:30: 00 = triangle with an extra raster parameter,
	// 01 = ordinary triangle, 10 = setup, 11 = quad.
	// Low bits still hold the MP's original vertex count, even when PP0
	// splits a five-vertex polygon into a quad followed by a triangle.
	uint8_t const primitive = packet[0] >> 30;
	if (primitive == 2)
	{
		// MP routine 40040e4c packs signed nine-bit clip limits. X grows
		// rightward and Y upward around the display centre (256, 192).
		m_clip = rectangle(
				256 + util::sext(packet[1] >> 23, 9),
				256 + util::sext(packet[1] >> 11, 9),
				192 - util::sext(packet[3] >> 23, 9),
				192 - util::sext(packet[3] >> 11, 9));
		m_clip &= rectangle(0, 511, 0, 383);
		return;
	}
	uint8_t const num_verts = primitive == 3 ? 4 : 3;
	rectangle const visarea = m_clip;
	if (visarea.empty())
		return;

	vertex_t vert[4];

	// Poly data:
	// Word 0:   xx------ -------- -------- --------   Primitive: 00 special tri, 01 tri, 10 setup, 11 quad
	//           --x----- -------- -------- --------   Affine/overlay mode (depth compare disabled; inferred)
	//           ---x---- -------- -------- --------   ? always 1?
	//           ----x--- -------- -------- --------   ? always 1?
	//           -----x-- -------- -------- --------   the PP transfer code checks for 1
	//           ------x- -------- -------- --------   always 0?
	//           -------x -------- -------- --------   Texture page
	//           -------- xxxx---- -------- --------   Header mode (inferred)
	//           -------- ----xxxx x------- --------   Header coefficient (inferred)
	//           -------- -------- -xxxxxxx --------   Palette
	//           -------- -------- -------- -----xxx   Original MP vertex count (not the submitted primitive size)
	// Word 1:   (float) Vertex 1 X
	// Word 2:   xxxxxxxx xxxxx--- -------- --------   Texture Origin Bottom
	//           -------- -----xxx xxxxxxxx --------   Texture Origin Left
	// Word 3:   (float) Vertex 1 Y
	// Word 4:   xxxxxxxx xxxxxxxx xxxxxxxx x-------   Z buffer multiplier (int)
	// Word 5:   (float) Vertex 2 X
	// Word 6:   (float) U/Z per X pixel increment
	// Word 7:   (float) Vertex 2 Y
	// Word 8:   (float) U/Z per Y pixel increment
	// Word 9:   (float) Vertex 3 X
	// Word 10:  (float) 1/Z per X pixel increment
	// Word 11:  (float) Vertex 3 Y
	// Word 12:  (float) 1/Z per Y pixel increment
	// Word 13:  (float) Vertex 4 X (if quad)
	// Word 14:  (float) V/Z per X pixel increment
	// Word 15:  (float) Vertex 4 Y (if quad)
	// Word 16:  (float) V/Z per Y pixel increment
	// Word 17:  (float) Vertex 5 X (if 5 verts)
	// Word 18:  (float) Base U coordinate
	// Word 19:  (float) Vertex 5 Y (if 5 verts)
	// Word 20:  (float) Base Z coordinate
	// Word 21:  Mode 00: six-bit translucency coefficient in bits 21:16
	// Word 22:  (float) Base V coordinate
	// Word 23:  (float) ? Seems to be a copy of the last X coordinate

	for (int j = 0; j < num_verts; j++)
	{
		float fx = u2f(packet[(j * 4) + 1]);
		float fy = u2f(packet[(j * 4) + 3]);

		// Packet coordinates address integer pixel centres. poly_manager
		// uses half-integer centres, as in the related Gaelco 3D renderer.
		vert[j].x = fx + 256.5f;
		vert[j].y = -fy + 192.5f;
	}

	rollext_polydata &extra = object_data().next();

	extra.tex_origin_y = (packet[2] >> 19) & 0x1fff;
	extra.tex_origin_x = (packet[2] >> 8) & 0x7ff;
	extra.pal = (packet[0] >> 8) & 0x7f;

	extra.tex_origin_y |= (packet[0] & 0x01000000) ? 0x2000 : 0;

	extra.uoz_dx = u2f(packet[6]);
	extra.uoz_dy = u2f(packet[8]);
	extra.voz_dx = u2f(packet[14]);
	extra.voz_dy = u2f(packet[16]);
	extra.ooz_dx = u2f(packet[10]);
	extra.ooz_dy = u2f(packet[12]);
	extra.baseu = u2f(packet[18]);
	extra.basev = u2f(packet[22]);
	extra.basez = u2f(packet[20]);

	extra.zmul = packet[4] >> 7;
	// Primitive 00 carries a six-bit translucency coefficient in word 21 (only
	// 32, half opacity, has been seen). Header mode 0xe with coefficient 15
	// (rider shadows, translucent scenery) is drawn at half opacity, as the
	// reference video suggests. Modes 1, 2, 7, 0xa and 0xb also occur with
	// coefficient 15; they're drawn opaque.
	// FIXME: decode the other modes and coefficients, and translucent Z writes
	uint8_t const mode = BIT(packet[0], 20, 4), coefficient = BIT(packet[0], 15, 5);
	bool const decoded = (!mode && !coefficient) || (mode == 0xe && coefficient == 15);
	if (primitive == 0)
		extra.alpha = BIT(packet[21], 16, 6) * 4;
	else
		extra.alpha = (mode == 0xe && coefficient == 15) ? 128 : 256;
	if (!decoded && !BIT(m_logged_blend[mode], coefficient))
	{
		m_logged_blend[mode] |= 1U << coefficient;
		m_screen.logerror("Undecoded header mode %x coefficient %d (primitive %d)\n", mode, coefficient, primitive);
	}

	// The MP's affine/2D path sets bit 29. These packets skip the depth test
	// (the large road titles lie behind the background) but still write depth,
	// as on gaelco3d.
	// FIXME: verify the flag and its interaction with translucency
	render_delegate const callback = BIT(packet[0], 29)
			? render_delegate(&rollext_renderer::render_texture_scan<false>, this)
			: render_delegate(&rollext_renderer::render_texture_scan<true>, this);
	if (num_verts == 4)
		render_polygon<4, 0>(visarea, callback, vert);
	else
		render_triangle<0>(visarea, callback, vert[0], vert[1], vert[2]);
}

void rollext_renderer::clear_fb()
{
	wait();
	rectangle visarea(0, 511, 0, 383);

	m_zb.fill(0xffffffff, visarea);
	m_fb[m_fb_current].fill(0xff000000, visarea);
}

void rollext_renderer::palette_write(int index, uint16_t data)
{
	wait();
	m_palette[index] = rgb_t(0xff, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data));
}

void rollext_renderer::texture_mask_write(int index, uint8_t data)
{
	wait();
	m_texture_mask[index] = data;
}

void rollext_renderer::display(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	// Each graphics task flips the buffers and draws into the new current one,
	// so the other buffer holds the last complete frame.
	wait();
	copybitmap_trans(*bitmap, m_fb[m_fb_current ^ 1], 0, 0, 0, 0, cliprect, 0);
}





class rollext_state : public driver_device
{
public:
	rollext_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac%u", 0U)
		, m_screen(*this, "screen")
		, m_in(*this, "INPUTS%u", 1U)
		, m_analog(*this, "ANALOG%u", 1U)
		, m_eeprom_in(*this, "EEPROMIN")
		, m_eeprom(*this, "eeprom")
		, m_link_config(*this, "LINK")
	{
	}

	void rollext(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// FIXME: the IC44 serial divider isn't dumped, so the word period is a guess
	static constexpr attotime LINK_WORD_PERIOD = attotime::from_usec(32);

	required_device<tms320c82_mp_device> m_maincpu;
	required_device_array<dac_word_interface, 4> m_dac;
	required_device<screen_device> m_screen;
	required_ioport_array<3> m_in;
	required_ioport_array<1> m_analog;
	required_ioport m_eeprom_in;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_ioport m_link_config;

	emu_timer *m_audio_timer = nullptr;
	std::unique_ptr<rollext_link_socket> m_link_socket;
	emu_timer *m_link_poll_timer = nullptr;
	emu_timer *m_link_tx_timer = nullptr;
	emu_timer *m_link_rx_timer = nullptr;
	uint16_t m_link_control = 0;
	uint16_t m_link_rx_word = 0;
	uint16_t m_video_timing = 0;
	std::array<uint16_t, 2> m_link_tx_words{};
	std::array<bool, 2> m_link_tx_dma{};
	uint8_t m_link_tx_head = 0;
	uint8_t m_link_tx_count = 0;
	uint16_t m_link_tx_shift_word = 0;
	bool m_link_tx_shifting = false;
	bool m_link_tx_shift_dma = false;
	bool m_link_rx_ready = false;
	bool m_link_rx_armed = true;
	bool m_link_rx_requested = false;
	std::array<uint16_t, 4096> m_link_input{};
	uint16_t m_link_head = 0;
	uint16_t m_link_count = 0;
	uint32_t m_link_retry = 0;

	uint32_t m_graphics_packet[24]{};
	uint8_t m_graphics_words = 0;
	std::unique_ptr<uint8_t []> m_texture;
	std::unique_ptr<rollext_renderer> m_renderer;
	uint8_t m_adc_reg;
	uint8_t m_adc_input;
	uint8_t m_adc_readbit;

	void memmap(address_map &map) ATTR_COLD;
	void preprocess_texture_data() ATTR_COLD;
	void link_postload() ATTR_COLD;
	void link_reset() ATTR_COLD;

	uint32_t a0000000_r(offs_t offset, uint32_t mem_mask = ~0);
	void a0000000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t link_status_r();
	void link_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <bool Dma> uint16_t link_data_r();
	template <bool Dma> void link_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t link_ack_r();
	uint16_t video_timing_r() { return m_video_timing; }
	void video_timing_w(uint16_t data) { m_video_timing = data; }
	void link_tx_start();
	void audio_w(offs_t offset, uint16_t data) { m_dac[offset]->write(data); }
	void palette_w(offs_t offset, uint16_t data) { m_renderer->palette_write(offset, data); }
	// Bytes are swapped within each halfword.
	void texture_mask_w(offs_t offset, uint8_t data) { m_renderer->texture_mask_write(offset ^ 1, data); }
	void graphics_transfer(offs_t address, uint32_t data, uint32_t mem_mask);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(audio_tick);
	TIMER_CALLBACK_MEMBER(link_poll);
	TIMER_CALLBACK_MEMBER(link_tx);
	TIMER_CALLBACK_MEMBER(link_rx);
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
};

void rollext_state::preprocess_texture_data()
{
	uint8_t const *rom = memregion("texture")->base();

	for (int j=0; j < 16384; j+=2)
	{
		for (int i=0; i < 2048; i+=4)
		{
			m_texture[((j + 0) * 2048) + i + 1] = rom[BYTE4_XOR_BE(3)];
			m_texture[((j + 1) * 2048) + i + 1] = rom[BYTE4_XOR_BE(2)];
			m_texture[((j + 0) * 2048) + i + 0] = rom[BYTE4_XOR_BE(1)];
			m_texture[((j + 1) * 2048) + i + 0] = rom[BYTE4_XOR_BE(0)];
			m_texture[((j + 0) * 2048) + i + 3] = rom[BYTE4_XOR_BE(7)];
			m_texture[((j + 1) * 2048) + i + 3] = rom[BYTE4_XOR_BE(6)];
			m_texture[((j + 0) * 2048) + i + 2] = rom[BYTE4_XOR_BE(5)];
			m_texture[((j + 1) * 2048) + i + 2] = rom[BYTE4_XOR_BE(4)];
			rom += 8;
		}
	}
}

void rollext_state::video_start()
{
	m_texture = std::make_unique<uint8_t []>(0x2000000);

	preprocess_texture_data();

	m_renderer = std::make_unique<rollext_renderer>(*m_screen);
	m_renderer->set_texture_ram(m_texture.get());
}

uint32_t rollext_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_renderer->display(&bitmap, cliprect);
	return 0;
}


uint32_t rollext_state::a0000000_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0:         // inputs
		{
			uint32_t data = 0;

			if (ACCESSING_BITS_16_23)
			{
				// -------- ---x---- -------- -------- ADC channel 0
				// -------- --x----- -------- -------- ADC channel 1?
				// -------- -x------ -------- -------- ADC channel 2?
				// -------- x------- -------- -------- ADC channel 3?

				data |= (m_adc_readbit & 1) ? 0x100000 : 0;

				data |= m_in[0]->read() << 16;
			}
			if (ACCESSING_BITS_8_15)
			{
				data |= 0x600; // transfer ready (bit 9), graphics FIFO ready (bit 10)
				data |= m_eeprom_in->read() << 8;
			}
			if (ACCESSING_BITS_0_7)
			{
				data |= m_in[1]->read();
			}

			return data;
		}

		case 1:
			// Sheet 1: /CSR2 is the unused second control bank (IC7),
			// pulled high. /CSR3 is the coin bank (IC10). The MP packs
			// 0xa0000006 into input byte 3, used by its credit routine.
			return 0x00ff0000 | m_in[2]->read();
	}

	return 0xffffffff;
}

void rollext_state::a0000000_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		// PP0 starts each graphics task with a halfword write here.
		if (ACCESSING_BITS_16_31)
		{
			m_renderer->wait();
			m_renderer->m_fb_current ^= 1;
			m_renderer->clear_fb();
		}
		if (ACCESSING_BITS_8_15)
		{
			m_eeprom->di_write(BIT(data, 8));
			m_eeprom->cs_write(BIT(data, 9));
			m_eeprom->clk_write(BIT(data, 10));
		}
	}
	else if (offset == 1)
	{
		if (ACCESSING_BITS_16_23)
		{
			uint8_t newdata = data >> 16;
			if ((newdata & 0x20) == 0 && (m_adc_reg & 0x20) != 0)
			{
				m_adc_input = m_analog[0]->read();
			}

			if (newdata & 0x10)
			{
				m_adc_readbit = (m_adc_input >> 7) & 1;
				m_adc_input <<= 1;
			}

			m_adc_reg = newdata;
			// bit 6: unknown (video enable or coin lockout)
			// bit 1: start lamp
			machine().bookkeeping().coin_counter_w(0, BIT(newdata, 0));
		}

		// if (ACCESSING_BITS_0_7) ADC force feedback?
	}
}

// IC44 (EPM7160) drives four 75LBC176 RS-485 lanes, selected by control
// bits 15:14. Register/DMA behavior is inferred from the ROM routines at
// 40041d30..400429a8. Serial word timing and the other control bits remain
// unverified.
uint16_t rollext_state::link_status_r()
{
	return (m_link_tx_count == m_link_tx_words.size() ? 0 : 0x100) | (m_link_rx_ready ? 1 : 0);
}

void rollext_state::link_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_link_control);
}

// The CPU and XPT (DMA) data registers share the received word.
template <bool Dma>
uint16_t rollext_state::link_data_r()
{
	uint16_t const data = m_link_rx_ready ? m_link_rx_word : 0xffff;
	if (!machine().side_effects_disabled())
	{
		m_link_rx_ready = false;
		if (m_link_rx_requested && !Dma)
		{
			m_link_rx_armed = false;
			m_maincpu->set_input_line(tms320c82_mp_device::INPUT_X3, ASSERT_LINE);
		}
		m_link_rx_requested = false;
	}
	return data;
}

template <bool Dma>
void rollext_state::link_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask != 0xffff)
	{
		logerror("%s: Unhandled link write data %04x mask %04x\n", machine().describe_context(), data, mem_mask);
		return;
	}
	if (m_link_tx_count < m_link_tx_words.size())
	{
		uint8_t const tail = (m_link_tx_head + m_link_tx_count) % m_link_tx_words.size();
		m_link_tx_words[tail] = data;
		m_link_tx_dma[tail] = Dma;
		++m_link_tx_count;
		link_tx_start();
	}
}

uint16_t rollext_state::link_ack_r()
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->set_input_line(tms320c82_mp_device::INPUT_X3, CLEAR_LINE);
		m_link_rx_armed = true;
	}
	return 0;
}

void rollext_state::link_tx_start()
{
	if (m_link_tx_shifting || !m_link_tx_count)
		return;
	// Status bit 8 reports space in the holding FIFO, two words deep as
	// inferred from the ROM's paired writes. The shifter frees its slot as soon
	// as it starts on a word.
	m_link_tx_shift_word = m_link_tx_words[m_link_tx_head];
	m_link_tx_shift_dma = m_link_tx_dma[m_link_tx_head];
	m_link_tx_head = (m_link_tx_head + 1) % m_link_tx_words.size();
	--m_link_tx_count;
	m_link_tx_shifting = true;
	m_link_tx_timer->adjust(LINK_WORD_PERIOD);
}

TIMER_CALLBACK_MEMBER(rollext_state::link_tx)
{
	if (m_link_socket)
		m_link_socket->send(m_link_tx_shift_word, m_link_control >> 14);
	m_link_tx_shifting = false;
	// XPT1 is requested once the word has gone out.
	if (m_link_tx_shift_dma)
		m_maincpu->external_transfer(1);
	link_tx_start();
}

TIMER_CALLBACK_MEMBER(rollext_state::link_rx)
{
	if (!(m_link_config->read() & 1))
		return;
	m_link_rx_timer->adjust(LINK_WORD_PERIOD);
	if (!m_link_rx_ready && m_link_count)
	{
		m_link_rx_word = m_link_input[m_link_head];
		m_link_head = (m_link_head + 1) % m_link_input.size();
		--m_link_count;
		m_link_rx_ready = true;
	}
	if (m_link_rx_ready && m_link_rx_armed && !m_link_rx_requested)
	{
		m_link_rx_requested = m_maincpu->external_transfer(3);
	}
}

TIMER_CALLBACK_MEMBER(rollext_state::link_poll)
{
	if (!(m_link_config->read() & 1))
	{
		m_link_socket.reset();
		m_link_rx_timer->adjust(attotime::never);
		return;
	}
	if (!m_link_socket)
	{
		m_link_rx_timer->adjust(LINK_WORD_PERIOD);
		m_link_socket = std::make_unique<rollext_link_socket>(machine().options(), [this] (uint16_t data)
		{
			if (m_link_count < m_link_input.size())
			{
				m_link_input[(m_link_head + m_link_count) % m_link_input.size()] = data;
				++m_link_count;
			}
		});
	}
	m_link_socket->poll();
	if (++m_link_retry == (machine().options().comm_framesync() ? 10000 : 1000))
	{
		m_link_retry = 0;
		m_link_socket->retry();
	}
}

void rollext_state::link_postload()
{
	// A save file cannot rewind another process. Drop the host connection;
	// linked cabinets need a coordinated restart after loading either end.
	m_link_socket.reset();
	m_link_retry = 0;
}

void rollext_state::link_reset()
{
	m_link_control = m_link_tx_shift_word = m_link_rx_word = m_video_timing = 0;
	m_link_tx_words.fill(0);
	m_link_tx_dma.fill(false);
	m_link_tx_head = m_link_tx_count = 0;
	m_link_tx_shifting = m_link_tx_shift_dma = false;
	m_link_rx_ready = m_link_rx_requested = false;
	m_link_rx_armed = true;
	m_link_socket.reset();
	m_link_retry = 0;
	m_link_head = m_link_count = 0;
	m_link_tx_timer->adjust(attotime::never);
	m_link_rx_timer->adjust(attotime::never);
	attotime const interval = attotime::from_usec(machine().options().comm_framesync() ? 100 : 1000);
	m_link_poll_timer->adjust(interval, 0, interval);
}

void rollext_state::graphics_transfer(offs_t address, uint32_t data, uint32_t mem_mask)
{
	// A28 selects the graphics FIFO while the RAM sees the same address.
	// PP0's TC peripheral-read descriptor uses the 0x50000000 alias.
	if ((address & 0xff000000) != 0x50000000)
		return;
	if (mem_mask != 0xffffffff)
	{
		logerror("%s: Unhandled graphics transfer at %08x mask %08x\n", machine().describe_context(), address, mem_mask);
		return;
	}
	m_graphics_packet[m_graphics_words++] = data;
	if (m_graphics_words == std::size(m_graphics_packet))
	{
		m_renderer->process_packet(m_graphics_packet);
		m_graphics_words = 0;
	}
}

// Master Processor memory map
void rollext_state::memmap(address_map &map)
{
	map(0x40000000, 0x40ffffff).mirror(0x10000000).ram().share("main_ram");
	map(0x60000000, 0x60000007).w(FUNC(rollext_state::audio_w));
	map(0x80000000, 0x8000ffff).w(FUNC(rollext_state::palette_w));
	map(0x80010000, 0x80010001).rw(FUNC(rollext_state::video_timing_r), FUNC(rollext_state::video_timing_w));
	map(0x90000000, 0x9007ffff).w(FUNC(rollext_state::texture_mask_w));
	map(0xa0000000, 0xa00000ff).rw(FUNC(rollext_state::a0000000_r), FUNC(rollext_state::a0000000_w));
	map(0xb0000000, 0xb0000001).rw(FUNC(rollext_state::link_status_r), FUNC(rollext_state::link_control_w));
	map(0xb0000002, 0xb0000003).rw(FUNC(rollext_state::link_data_r<false>), FUNC(rollext_state::link_data_w<false>));
	map(0xb0000004, 0xb0000005).r(FUNC(rollext_state::link_ack_r));
	map(0xb0000006, 0xb0000007).rw(FUNC(rollext_state::link_data_r<true>), FUNC(rollext_state::link_data_w<true>));
	map(0xc0000000, 0xc03fffff).rom().region("rom1", 0);
	map(0xff000000, 0xffffffff).rom().region("rom0", 0);
}

static INPUT_PORTS_START(rollext)
	PORT_START("LINK")
	PORT_CONFNAME(0x01, 0x00, "Cabinet Link (experimental)")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))

	PORT_START("INPUTS1")
	PORT_SERVICE_NO_TOGGLE(0x8, IP_ACTIVE_LOW)      // test button
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_VOLUME_DOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_VOLUME_UP)

	PORT_START("INPUTS2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("View Change")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Right Brake")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Left Brake")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Left Smash")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Right Smash")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUTS3")
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("EEPROMIN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("ANALOG1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_NAME("Seat Tilt")
INPUT_PORTS_END


TIMER_CALLBACK_MEMBER(rollext_state::audio_tick)
{
	// SO-REQ from IC25 requests XPT2: four samples, followed by cursor update.
	m_maincpu->external_transfer(2);
}

void rollext_state::machine_reset()
{
	link_reset();
	m_graphics_words = 0;
	std::fill(std::begin(m_graphics_packet), std::end(m_graphics_packet), 0);
	m_adc_reg = 0;
	m_adc_input = 0;
	m_adc_readbit = 0;
	// Inferred FPGA divider: the ROM's 12-bit pitch steps for 8/16/22.05/
	// 32/44.1 kHz sources agree with 90 MHz / 2048 (43945.3125 Hz).
	attotime const period = attotime::from_ticks(2048, 90'000'000);
	m_audio_timer->adjust(period, 0, period);
}

void rollext_state::machine_start()
{
	m_link_poll_timer = timer_alloc(FUNC(rollext_state::link_poll), this);
	m_link_tx_timer = timer_alloc(FUNC(rollext_state::link_tx), this);
	m_link_rx_timer = timer_alloc(FUNC(rollext_state::link_rx), this);
	save_item(NAME(m_link_control));
	save_item(NAME(m_link_tx_words));
	save_item(NAME(m_link_tx_head));
	save_item(NAME(m_link_tx_count));
	save_item(NAME(m_link_tx_shift_word));
	save_item(NAME(m_link_tx_shifting));
	save_item(NAME(m_link_tx_shift_dma));
	save_item(NAME(m_link_rx_word));
	save_item(NAME(m_video_timing));
	save_item(NAME(m_link_tx_dma));
	save_item(NAME(m_link_rx_ready));
	save_item(NAME(m_link_rx_armed));
	save_item(NAME(m_link_rx_requested));
	save_item(NAME(m_link_input));
	save_item(NAME(m_link_head));
	save_item(NAME(m_link_count));
	machine().save().register_postload(save_prepost_delegate(FUNC(rollext_state::link_postload), this));
	m_audio_timer = timer_alloc(FUNC(rollext_state::audio_tick), this);
	for (auto &dac : m_dac)
		dac->write(0);
	save_item(NAME(m_graphics_packet));
	save_item(NAME(m_graphics_words));
	save_item(NAME(m_adc_reg));
	save_item(NAME(m_adc_input));
	save_item(NAME(m_adc_readbit));
}


void rollext_state::rollext(machine_config &config)
{
	TMS320C82_MP(config, m_maincpu, 60_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rollext_state::memmap);
	m_maincpu->set_vblank_int("screen", FUNC(rollext_state::vblank_interrupt));
	m_maincpu->peripheral_read_cb().set(FUNC(rollext_state::graphics_transfer));

	config.set_maximum_quantum(attotime::from_hz(100));

	EEPROM_93C66_16BIT(config, m_eeprom);

	// IC12/IC11 TDA1543 outputs CANAL1..4 (sheet 6), which the ROM's output
	// test calls left, right, bass and rear. The 3.3k (left/right) and 4.7k
	// I/V resistors set their relative levels.
	SPEAKER(config, "speaker", 2).front();
	SPEAKER(config, "subwoofer").lfe();
	SPEAKER(config, "rear").rear_center();

	// Each I/V amplifier (Rf || 1 nF) is followed by a third-order low-pass
	// filter: three 56k resistors, Ca and Cc to ground at the first and third
	// nodes, and Cb from the second node to the unity-gain TL074 follower
	// output. Its denominator,
	//   1 + s R (Ca + 3 Cc) + s^2 2 R^2 Cc (Ca + Cb) + s^3 R^3 Ca Cb Cc,
	// factors into a complex pair and a real pole:
	//   left/right (330p, 680p, 100p): 8.25 kHz with Q 0.78, and 15.0 kHz
	//   bass/rear (8.2n, 15n, 2.2n): 393 Hz with Q 0.75, and 549 Hz
	// The I/V poles (48 and 34 kHz) are above the audio band.
	struct output_filter { double fc, q, pole; };
	constexpr output_filter FILTERS[2] = { { 8'252.7, 0.7813, 15'020.6 }, { 393.2, 0.7513, 548.8 } };
	for (int i = 0; i < 4; i++)
	{
		output_filter const &f = FILTERS[i / 2];
		filter_biquad_device &pair = FILTER_BIQUAD(config, util::string_format("lpf%d_pair", i).c_str())
				.setup(filter_biquad_device::biquad_type::LOWPASS, f.fc, f.q, 1.0);
		filter_biquad_device &pole = FILTER_BIQUAD(config, util::string_format("lpf%d_pole", i).c_str())
				.setup(filter_biquad_device::biquad_type::LOWPASS1P, f.pole, 1.0, 1.0);
		DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[i], 0).add_route(ALL_OUTPUTS, pair, 1.0);
		pair.add_route(ALL_OUTPUTS, pole, 1.0);
		if (i < 2)
			pole.add_route(ALL_OUTPUTS, "speaker", 3'300.0 / 4'700.0, i);
		else
			pole.add_route(ALL_OUTPUTS, (i == 2) ? "subwoofer" : "rear", 1.0);
	}

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 384);
	m_screen->set_visarea(0, 511, 0, 383);
	m_screen->set_screen_update(FUNC(rollext_state::screen_update));
}


INTERRUPT_GEN_MEMBER(rollext_state::vblank_interrupt)
{
	m_maincpu->set_input_line(tms320c82_mp_device::INPUT_X1, ASSERT_LINE);
}

ROM_START(rollext)
	ROM_REGION32_BE(0x1000000, "rom0", 0)
	ROM_LOAD64_DWORD_SWAP("roe.ic17", 0x000000, 0x800000, CRC(30f51686) SHA1(70761fc713b5ca0e488dfccf89b64cc6ddd05217))
	ROM_LOAD64_DWORD_SWAP("roe.ic18", 0x000004, 0x800000, CRC(1a4e65d6) SHA1(69fc7747d8e64d0b397c676bebaedc9e6122fe89))

	ROM_REGION32_BE(0x400000, "rom1", 0)
	ROM_LOAD32_DWORD("roe.ic38", 0x000000, 0x400000, CRC(d9bcfb7c) SHA1(eda9870881732d4dc7cdacb65c6d40af3451dc9d))

	ROM_REGION32_BE(0x2000000, "texture", 0)            // Texture ROMs
	ROM_LOAD32_BYTE("roe.ic45", 0x000000, 0x800000, CRC(0f7fe365) SHA1(ed50fd2b76840eac6ce394a0c748109f615b775a))
	ROM_LOAD32_BYTE("roe.ic47", 0x000001, 0x800000, CRC(44d7ccee) SHA1(2fec682396e4cca704bd1237016acec0e7b4b428))
	ROM_LOAD32_BYTE("roe.ic58", 0x000002, 0x800000, CRC(67ad4561) SHA1(56f41b4ebd827fec49902f377c5ed054c02d9e6c))
	ROM_LOAD32_BYTE("roe.ic60", 0x000003, 0x800000, CRC(a64524af) SHA1(31bef17656ab025f90cd222d3d6d0cb62dee29ee))
ROM_END

} // anonymous namespace

GAME( 1999, rollext, 0, rollext, rollext, rollext_state, empty_init, ROT0, "Gaelco (Namco America license)", "ROLLing eX.tre.me (US)", MACHINE_SUPPORTS_SAVE )
