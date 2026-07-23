// license:BSD-3-Clause
// copyright-holders:Hans Andersson
//============================================================
//
//  vectorrenderer.cpp - Persistent GPU vector CRT simulation
//
//============================================================

#include "vectorrenderer.h"

#include "effect.h"
#include "effectmanager.h"
#include "uniform.h"
#include "vertex.h"

#include "../frontend/mame/ui/menuitem.h"
#include "../frontend/mame/ui/slider.h"

#include "modules/lib/osdobj_common.h"

#include "emu.h"
#include "osdcore.h"
#include "render.h"
#include "strformat.h"

#include <bx/math.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>


namespace {

constexpr unsigned BLOOM_PASSES = 2;

// Empirical conversion from MAME's target-pixel line width to Gaussian sigma.
// This controls profile calibration; it is not a resolution-scaling factor.
constexpr float BEAM_SIGMA_SCALE = 0.085f;

constexpr uint64_t TARGET_FLAGS =
		BGFX_TEXTURE_RT |
		BGFX_SAMPLER_U_CLAMP |
		BGFX_SAMPLER_V_CLAMP |
		BGFX_SAMPLER_MIP_POINT;

constexpr uint32_t SAMPLE_FLAGS =
		BGFX_SAMPLER_U_CLAMP |
		BGFX_SAMPLER_V_CLAMP |
		BGFX_SAMPLER_MIP_POINT;

struct beam_vertex
{
	float x;
	float y;
};

struct beam_instance
{
	float x0;
	float y0;
	float x1;
	float y1;
	float red;
	float green;
	float blue;
	float sigma;
	float start;
	float duration;
	float intensity;
	float unused;
};

static_assert(sizeof(beam_instance) == sizeof(float) * 12);

} // anonymous namespace


bgfx_vector_renderer::bgfx_vector_renderer(effect_manager &effects, osd_options const &options)
	: m_decay_effect(effects.get_or_load_effect(options, "vector-crt/decay"))
	, m_beam_effect(effects.get_or_load_effect(options, "vector-crt/beam"))
	, m_downsample_effect(effects.get_or_load_effect(options, "vector-crt/downsample"))
	, m_blur_effect(effects.get_or_load_effect(options, "vector-crt/blur"))
	, m_composite_effect(effects.get_or_load_effect(options, "vector-crt/composite"))
	, m_post_vertices(BGFX_INVALID_HANDLE)
	, m_beam_vertices(BGFX_INVALID_HANDLE)
	, m_width(0)
	, m_height(0)
	, m_bloom_width(0)
	, m_bloom_height(0)
	, m_current_accumulation(0)
	, m_last_emu_time(0.0)
	, m_have_time(false)
	, m_reset_accumulation(true)
	, m_available(false)
	, m_present(false)
	, m_persistence(0.0f)
	, m_beam_width(0.0f)
	, m_beam_intensity(0.0f)
	, m_halo(0.0f)
	, m_bloom_strength(0.0f)
	, m_bloom_radius(0.0f)
	, m_exposure(0.0f)
{

	if (!(m_decay_effect && m_beam_effect && m_downsample_effect && m_blur_effect && m_composite_effect))
	{
		osd_printf_verbose("BGFX: Vector CRT renderer: failed to create effects\n");
		return;
	}

	if (!create_geometry())
	{
		osd_printf_verbose("BGFX: Vector CRT renderer: failed to create geometry\n");
		return;
	}
	create_sliders();
	m_available = true;
	osd_printf_verbose("BGFX: Vector CRT renderer initialized\n");
}


bgfx_vector_renderer::~bgfx_vector_renderer()
{
	destroy_targets();
	destroy_geometry();
}


bool bgfx_vector_renderer::create_geometry()
{
	ScreenVertex post[6];
	float const vtop = ((bgfx::getRendererType() == bgfx::RendererType::OpenGL) || (bgfx::getRendererType() == bgfx::RendererType::OpenGLES)) ? 1.0f : 0.0f;
	float const vbottom = 1.0f - vtop;
	auto set_post_vertex = [] (ScreenVertex &vertex, float x, float y, float u, float v)
	{
		vertex.m_x = x;
		vertex.m_y = y;
		vertex.m_z = 0.0f;
		vertex.m_rgba = 0xffffffffU;
		vertex.m_u = u;
		vertex.m_v = v;
	};
	set_post_vertex(post[0], 0.0f, 0.0f, 0.0f, vtop);
	set_post_vertex(post[1], 1.0f, 0.0f, 1.0f, vtop);
	set_post_vertex(post[2], 1.0f, 1.0f, 1.0f, vbottom);
	set_post_vertex(post[3], 1.0f, 1.0f, 1.0f, vbottom);
	set_post_vertex(post[4], 0.0f, 1.0f, 0.0f, vbottom);
	set_post_vertex(post[5], 0.0f, 0.0f, 0.0f, vtop);
	m_post_vertices = bgfx::createVertexBuffer(bgfx::copy(post, sizeof(post)), ScreenVertex::ms_decl);

	beam_vertex const beam[6] =
	{
		{ 0.0f, -1.0f },
		{ 1.0f, -1.0f },
		{ 1.0f,  1.0f },
		{ 1.0f,  1.0f },
		{ 0.0f,  1.0f },
		{ 0.0f, -1.0f }
	};
	m_beam_layout.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.end();
	m_beam_vertices = bgfx::createVertexBuffer(bgfx::copy(beam, sizeof(beam)), m_beam_layout);

	if (!bgfx::isValid(m_post_vertices) || !bgfx::isValid(m_beam_vertices))
	{
		osd_printf_warning("BGFX: Unable to create vector CRT geometry; using normal vector rendering\n");
		destroy_geometry();
		return false;
	}
	return true;
}


void bgfx_vector_renderer::destroy_geometry()
{
	if (bgfx::isValid(m_beam_vertices))
	{
		bgfx::destroy(m_beam_vertices);
		m_beam_vertices = BGFX_INVALID_HANDLE;
	}
	if (bgfx::isValid(m_post_vertices))
	{
		bgfx::destroy(m_post_vertices);
		m_post_vertices = BGFX_INVALID_HANDLE;
	}
}


bool bgfx_vector_renderer::create_targets(uint16_t width, uint16_t height)
{
	destroy_targets();

	if (!bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::RGBA16F, TARGET_FLAGS))
	{
		osd_printf_warning("BGFX: RGBA16F render targets are unavailable; using normal vector rendering\n");
		m_available = false;
		return false;
	}

	m_width = width;
	m_height = height;
	m_bloom_width = std::max<uint16_t>(1, (width + 1) / 2);
	m_bloom_height = std::max<uint16_t>(1, (height + 1) / 2);

	auto create_target = [] (target &output, uint16_t target_width, uint16_t target_height)
	{
		output.texture = bgfx::createTexture2D(target_width, target_height, false, 1, bgfx::TextureFormat::RGBA16F, TARGET_FLAGS);
		if (bgfx::isValid(output.texture))
			output.framebuffer = bgfx::createFrameBuffer(1, &output.texture, false);
		return bgfx::isValid(output.texture) && bgfx::isValid(output.framebuffer);
	};

	bool valid = true;
	for (target &accumulation : m_accumulation)
		valid = create_target(accumulation, m_width, m_height) && valid;
	for (target &bloom : m_bloom)
		valid = create_target(bloom, m_bloom_width, m_bloom_height) && valid;

	if (!valid)
	{
		osd_printf_warning("BGFX: Unable to create vector CRT render targets; using normal vector rendering\n");
		destroy_targets();
		m_available = false;
		return false;
	}

	m_current_accumulation = 0;
	m_reset_accumulation = true;
	m_have_time = false;
	osd_printf_verbose("BGFX: Created %ux%u RGBA16F vector phosphor buffers and %ux%u bloom buffers\n", m_width, m_height, m_bloom_width, m_bloom_height);
	return true;
}


void bgfx_vector_renderer::destroy_targets()
{
	auto destroy_target = [] (target &value)
	{
		if (bgfx::isValid(value.framebuffer))
		{
			bgfx::destroy(value.framebuffer);
			value.framebuffer = BGFX_INVALID_HANDLE;
		}
		if (bgfx::isValid(value.texture))
		{
			bgfx::destroy(value.texture);
			value.texture = BGFX_INVALID_HANDLE;
		}
	};
	for (target &accumulation : m_accumulation)
		destroy_target(accumulation);
	for (target &bloom : m_bloom)
		destroy_target(bloom);
	m_width = m_height = m_bloom_width = m_bloom_height = 0;
}


void bgfx_vector_renderer::setup_view(uint16_t view, bgfx::FrameBufferHandle framebuffer, uint16_t width, uint16_t height, bool clear)
{
	bgfx::setViewFrameBuffer(view, framebuffer);
	bgfx::setViewRect(view, 0, 0, width, height);
	bgfx::setViewClear(view, clear ? BGFX_CLEAR_COLOR : BGFX_CLEAR_NONE, 0x00000000U, 1.0f, 0);
	bgfx::setViewMode(view, bgfx::ViewMode::Sequential);

	float projection[16];
	bx::mtxOrtho(projection, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(view, nullptr, projection);
}


void bgfx_vector_renderer::bind_post_geometry()
{
	bgfx::setVertexBuffer(0, m_post_vertices);
}


void bgfx_vector_renderer::set_uniform(bgfx_effect *effect, char const *name, float x, float y, float z, float w)
{
	if (bgfx_uniform *const uniform = effect->uniform(name))
	{
		float values[4] = { x, y, z, w };
		uniform->set(values, sizeof(values));
	}
}


void bgfx_vector_renderer::draw_post(bgfx_effect *effect, uint16_t view)
{
	bind_post_geometry();
	effect->submit(view);
}


void bgfx_vector_renderer::draw_beams(uint16_t view, double frame_time)
{
	if (m_vectors.empty())
		return;

	double total_length = 0.0;
	for (render_primitive const *const primitive : m_vectors)
	{
		double const dx = primitive->bounds.x1 - primitive->bounds.x0;
		double const dy = primitive->bounds.y1 - primitive->bounds.y0;
		total_length += std::max(std::sqrt((dx * dx) + (dy * dy)), std::max<double>(primitive->width, 1.0));
	}
	total_length = std::max(total_length, std::numeric_limits<double>::epsilon());

	set_uniform(m_beam_effect, "u_vector_params", float(frame_time), m_persistence, m_beam_intensity, m_halo);
	set_uniform(m_beam_effect, "u_target_dims", float(m_width), float(m_height), 1.0f / float(m_width), 1.0f / float(m_height));

	uint32_t offset = 0;
	double elapsed_length = 0.0;
	while (offset < m_vectors.size())
	{
		uint32_t const remaining = uint32_t(m_vectors.size() - offset);
		uint32_t const count = bgfx::getAvailInstanceDataBuffer(remaining, sizeof(beam_instance));
		if (!count)
		{
			osd_printf_warning("BGFX: Transient instance buffer exhausted while rendering vectors\n");
			break;
		}

		bgfx::InstanceDataBuffer instances;
		bgfx::allocInstanceDataBuffer(&instances, count, sizeof(beam_instance));
		beam_instance *const data = reinterpret_cast<beam_instance *>(instances.data);
		for (uint32_t index = 0; index < count; ++index)
		{
			render_primitive const &primitive = *m_vectors[offset + index];
			double const dx = primitive.bounds.x1 - primitive.bounds.x0;
			double const dy = primitive.bounds.y1 - primitive.bounds.y0;
			double const length = std::max(std::sqrt((dx * dx) + (dy * dy)), std::max<double>(primitive.width, 1.0));

			beam_instance &instance = data[index];
			instance.x0 = primitive.bounds.x0;
			instance.y0 = primitive.bounds.y0;
			instance.x1 = primitive.bounds.x1;
			instance.y1 = primitive.bounds.y1;
			instance.red = primitive.color.r;
			instance.green = primitive.color.g;
			instance.blue = primitive.color.b;
			instance.sigma = primitive.width * m_beam_width * BEAM_SIGMA_SCALE;
			instance.start = float(elapsed_length / total_length);
			instance.duration = float(length / total_length);
			instance.intensity = primitive.color.a;
			instance.unused = 0.0f;
			elapsed_length += length;
		}

		bgfx::setVertexBuffer(0, m_beam_vertices);
		bgfx::setInstanceDataBuffer(&instances, 0, count);
		m_beam_effect->submit(view);
		offset += count;
	}
}


void bgfx_vector_renderer::prepare(uint32_t &view, render_primitive *first, uint16_t width, uint16_t height, double emu_time)
{
	m_vectors.clear();
	m_present = false;
	for (render_primitive *primitive = first; primitive; primitive = primitive->next())
	{
		m_present = m_present || bool(PRIMFLAG_GET_VECTORBUF(primitive->flags));
		if ((primitive->type == render_primitive::LINE) && PRIMFLAG_GET_VECTOR(primitive->flags))
			m_vectors.push_back(primitive);
	}

	if (!m_available || !m_present || !width || !height)
		return;

	if ((width != m_width) || (height != m_height))
	{
		if (!create_targets(width, height))
			return;
	}

	double frame_time = 1.0 / 60.0;
	if (m_have_time)
	{
		frame_time = emu_time - m_last_emu_time;

		// Time can move backwards after a reset, state load, rewind, or
		// machine restart. Clear the phosphor instead of retaining an image
		// from the previous timeline.
		if (frame_time < 0.0)
		{
			m_reset_accumulation = true;
			frame_time = 1.0 / 60.0;
		}
		else if (frame_time <= 0.000001)
		{
			// Emulation is paused or this display list has already been
			// processed at the same emulated time. Do not excite it again.
			return;
		}

		// Avoid an unusually large elapsed time causing undesirable behaviour.
		frame_time = std::min(frame_time, 0.100);
	}
	m_last_emu_time = emu_time;
	m_have_time = true;

	uint8_t const previous = m_current_accumulation;
	uint8_t const current = previous ^ 1;

	setup_view(
			uint16_t(view),
			m_accumulation[current].framebuffer,
			m_width,
			m_height,
			m_reset_accumulation);

	if (m_reset_accumulation)
	{
		bgfx::touch(uint16_t(view));
		m_reset_accumulation = false;
	}
	else
	{
		// Exponential phosphor decay:
		//
		//     I(t + dt) = I(t) * exp(-dt / tau)
		//
		// m_persistence is tau in seconds.
		float const tau = std::max(m_persistence, 0.001f);
		float const decay = std::exp(-float(frame_time) / tau);

		bgfx_uniform *const sampler = m_decay_effect->uniform("s_accum");
		if (!sampler)
		{
			osd_printf_warning("BGFX: Vector CRT decay effect has no s_accum uniform\n");
			m_available = false;
			return;
		}

		bgfx::setTexture(
				0,
				sampler->handle(),
				m_accumulation[previous].texture,
				SAMPLE_FLAGS);

		set_uniform(m_decay_effect, "u_decay", decay);
		draw_post(m_decay_effect, uint16_t(view));
	}
	++view;
	m_current_accumulation = current;

	if (!m_vectors.empty())
	{
		setup_view(uint16_t(view), m_accumulation[current].framebuffer, m_width, m_height, false);
		float projection[16];
		bx::mtxOrtho(projection, 0.0f, float(m_width), float(m_height), 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(uint16_t(view), nullptr, projection);
		draw_beams(uint16_t(view), frame_time);
		++view;
	}

	setup_view(uint16_t(view), m_bloom[0].framebuffer, m_bloom_width, m_bloom_height, false);
	bgfx::setTexture(0, m_downsample_effect->uniform("s_accum")->handle(), m_accumulation[current].texture, SAMPLE_FLAGS);
	set_uniform(m_downsample_effect, "u_source_texel", 1.0f / float(m_width), 1.0f / float(m_height));
	draw_post(m_downsample_effect, uint16_t(view));
	++view;

	// Scale the output-pixel bloom radius relative to a 1080-line reference.
	float const bloom_scale = std::clamp(float(m_height) / 1080.0f, 0.25f, 2.0f);
	float const bloom_radius = m_bloom_radius * bloom_scale;

	for (unsigned pass = 0; pass < BLOOM_PASSES; ++pass)
	{
		setup_view(uint16_t(view), m_bloom[1].framebuffer, m_bloom_width, m_bloom_height, false);
		bgfx::setTexture(0, m_blur_effect->uniform("s_tex")->handle(), m_bloom[0].texture, SAMPLE_FLAGS);
		set_uniform(m_blur_effect, "u_blur", bloom_radius / float(m_width), 0.0f);
		draw_post(m_blur_effect, uint16_t(view));
		++view;

		setup_view(uint16_t(view), m_bloom[0].framebuffer, m_bloom_width, m_bloom_height, false);
		bgfx::setTexture(0, m_blur_effect->uniform("s_tex")->handle(), m_bloom[1].texture, SAMPLE_FLAGS);
		set_uniform(m_blur_effect, "u_blur", 0.0f, bloom_radius / float(m_height));
		draw_post(m_blur_effect, uint16_t(view));
		++view;
	}
}


void bgfx_vector_renderer::composite(uint16_t view)
{
	if (!m_available || !m_present || !m_width || !m_height)
		return;

	bgfx::setTexture(0, m_composite_effect->uniform("s_accum")->handle(), m_accumulation[m_current_accumulation].texture, SAMPLE_FLAGS);
	bgfx::setTexture(1, m_composite_effect->uniform("s_bloom")->handle(), m_bloom[0].texture, SAMPLE_FLAGS);
	set_uniform(m_composite_effect, "u_composite", m_bloom_strength, m_exposure, 2.2f, 0.0f);
	draw_post(m_composite_effect, view);
}


void bgfx_vector_renderer::create_sliders()
{
	struct slider_description
	{
		char const *name;
		int32_t minimum;
		int32_t value;
		int32_t maximum;
		int32_t increment;
	};
	static constexpr slider_description descriptions[SLIDER_COUNT] =
	{
		{ "Vector phosphor persistence",   1,   2, 100, 1 },   // 0.02
		{ "Vector beam width",            30,  75, 400, 1 },   // 0.75
		{ "Vector beam intensity",        10, 400, 500, 1 },   // 4.00
		{ "Vector beam halo",              0,   4, 100, 1 },   // 0.04
		{ "Vector bloom strength",         0,  12, 300, 1 },   // 0.12
		{ "Vector bloom radius",          20, 195, 400, 1 },   // 1.95
		{ "Vector exposure",              10,  78, 400, 1 },   // 0.78
	};

	m_sliders.reserve(SLIDER_COUNT);
	for (uint8_t id = 0; id < SLIDER_COUNT; ++id)
	{
		slider_description const &description = descriptions[id];
		m_sliders.emplace_back(std::make_unique<slider_state>(
				description.name,
				description.minimum,
				description.value,
				description.maximum,
				description.increment,
				[this, id] (std::string *text, int32_t value) { return slider_changed(slider_id(id), text, value); }));
		slider_changed(slider_id(id), nullptr, description.value);
	}
}


int32_t bgfx_vector_renderer::slider_changed(slider_id id, std::string *text, int32_t new_value)
{
	float scale = 1.0f;
	float *setting = nullptr;
	char const *format = "%1.2f";
	switch (id)
	{
		case SLIDER_PERSISTENCE:
			setting = &m_persistence;
			scale = 0.001f;
			format = "%1.0f ms";
			break;
		case SLIDER_BEAM_WIDTH:
			setting = &m_beam_width;
			scale = 0.01f;
			format = "%1.2fx";
			break;
		case SLIDER_BEAM_INTENSITY:
			setting = &m_beam_intensity;
			scale = 0.01f;
			format = "%1.2fx";
			break;
		case SLIDER_HALO:
			setting = &m_halo;
			scale = 0.01f;
			format = "%1.2fx";
			break;
		case SLIDER_BLOOM_STRENGTH:
			setting = &m_bloom_strength;
			scale = 0.01f;
			format = "%1.2fx";
			break;
		case SLIDER_BLOOM_RADIUS:
			setting = &m_bloom_radius;
			scale = 0.01f;
			format = "%1.2f px @ 1080p";
			break;
		case SLIDER_EXPOSURE:
			setting = &m_exposure;
			scale = 0.01f;
			format = "%1.2fx";
			break;
		default:
			return 0;
	}

	if (new_value != SLIDER_NOCHANGE)
		*setting = float(new_value) * scale;
	if (text)
	{
		float const display_value = (id == SLIDER_PERSISTENCE) ? (*setting * 1000.0f) : *setting;
		*text = util::string_format(format, display_value);
	}
	return int32_t(std::floor((*setting / scale) + 0.5f));
}


void bgfx_vector_renderer::append_sliders(std::vector<ui::menu_item> &items)
{
	if (!m_available || !m_present)
		return;
	for (std::unique_ptr<slider_state> const &slider : m_sliders)
	{
		ui::menu_item item(ui::menu_item_type::SLIDER, slider.get());
		item.set_text(slider->description);
		items.emplace_back(std::move(item));
	}
}
