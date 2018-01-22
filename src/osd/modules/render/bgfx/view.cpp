#include "emu.h"
#include "window.h"
#include "rendutil.h"
#include "../drawbgfx.h"

#include <bx/math.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "target.h"
#include "view.h"

void bgfx_view::update() {
	std::shared_ptr<osd_window> win = m_renderer->assert_window();

	const uint32_t window_index = win->m_index;
	const uint32_t width = m_renderer->get_window_width(window_index);
	const uint32_t height = m_renderer->get_window_height(window_index);

	if (window_index != m_window_index)
		m_window_index = window_index;
	if (width != m_view_width)
		m_view_width = width;
	if (height != m_view_height)
		m_view_height = height;
}

void bgfx_ortho_view::setup() {
	if (m_window_index != 0)
	{
		bgfx::setViewFrameBuffer(m_index, m_backbuffer->target());
	}

	bgfx::setViewRect(m_index, 0, 0, m_view_width, m_view_height);

	while ((m_index + 1) > m_seen_views.size())
	{
		m_seen_views.push_back(false);
	}

	if (!m_seen_views[m_index])
	{
		m_seen_views[m_index] = true;
		uint16_t flags = 0;
		flags |= (m_do_clear_color ? BGFX_CLEAR_COLOR : 0);
		flags |= (m_do_clear_depth ? BGFX_CLEAR_DEPTH : 0);
		flags |= (m_do_clear_stencil ? BGFX_CLEAR_STENCIL : 0);
		bgfx::setViewClear(m_index, flags, m_clear_color, m_clear_depth, m_clear_stencil);
		bgfx::setViewMode(m_index, bgfx::ViewMode::Sequential);
	}

	setup_matrices();
}

void bgfx_ortho_view::setup_matrices() {
	float proj[16];
	float view[16];
	const bgfx::Caps* caps = bgfx::getCaps();

	bx::mtxIdentity(view);
	bx::mtxOrtho(proj, 0.0f, m_view_width, m_view_height, 0.0f, m_z_near, m_z_far, 0.0f, caps->homogeneousDepth);

	bgfx::setViewTransform(m_index, view, proj);
}
