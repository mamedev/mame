// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Sega 315-6289 "ELAN"

    VideoLogic/NEC transform & lighting processor used by NAOMI 2

***************************************************************************/
#ifndef MAME_SEGA_315_6289_H
#define MAME_SEGA_315_6289_H

#pragma once

#include "powervr2.h"

#include "cpu/sh/sh4.h"

#include "corefloat.h"

#include <vector>


class sega_315_6289_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	sega_315_6289_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu_space(T &&tag, int no) { m_cpu_space.set_tag(std::forward<T>(tag), no); }
	template <typename T> void set_ram(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_pvr(int which, T &&tag) { m_pvr[which].set_tag(std::forward<T>(tag)); }

	// where the SH-4 sees the texture memory of each CLX2
	void set_vram(int which, offs_t base, offs_t size) { m_vram_base[which] = base; m_vram_size = size; }

	void map(address_map &map) ATTR_COLD;

	void command_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// SH-4 writes to the memory of either CLX2 go to both
	bool broadcast() const { return BIT(m_sh4_ctrl, 0); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : uint32_t
	{
		IRQ_DMA_DONE = 0x01,
		IRQ_COMMAND_DONE = 0x02,
		IRQ_ERROR = 0x10
	};

	enum : uint32_t
	{
		CMD_NULL = 0x0,
		CMD_PROJECTION = 0x3,
		CMD_MATRIX_LIGHT = 0x4,
		CMD_GMP = 0x5,
		CMD_ICH = 0x7,
		CMD_MODEL = 0x8,
		CMD_REGISTER_WAIT = 0xe,
		CMD_LINK = 0xf
	};

	// optional parts of an ICH list vertex
	enum : uint32_t
	{
		VTX_NORMAL = 0x004,
		VTX_UV = 0x008,
		VTX_COLOR = 0x040,
		VTX_BUMP = 0x100
	};

	// light routing
	enum : uint8_t
	{
		ROUTE_SPECULAR_OFFSET = 0x1,
		ROUTE_DIFFUSE_OFFSET = 0x2,
		ROUTE_ALPHA = 0x4,
		ROUTE_SUBTRACT = 0x8
	};

	static constexpr int MAX_LIGHTS = 16;
	static constexpr int MAX_DEPTH = 8;
	static constexpr int MAX_LINKS = 0x100000;

	struct light
	{
		bool parallel = false;
		float color[3] = { };
		float direction[3] = { };
		float position[3] = { };
		uint8_t routing = 0;
		uint8_t dmode = 0;
		uint8_t smode = 0;
		bool dist_inverse = false;
		bool dist_attenuation = false;
		bool angle_attenuation = false;
		float dist_a = 0.0f;
		float dist_b = 0.0f;
		float angle_a = 0.0f;
		float angle_b = 0.0f;
	};

	struct vertex
	{
		float view[3];      // view space
		float uv[2];
		float base[4];      // ARGB
		float offs[4];
	};

	uint32_t id_r();
	uint32_t revision_r();
	void reset_w(uint32_t data);
	uint32_t queue_r();
	uint32_t sh4_ctrl_r();
	void sh4_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sdram_refresh_r();
	uint32_t sdram_config_r();
	uint32_t tiler_r();
	void tiler_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t irq_status_r();
	void irq_status_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t irq_mask_r();
	void irq_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t unknown_r(offs_t offset);
	void unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TIMER_CALLBACK_MEMBER(dma_done);

	// command streams come from the FIFO or from our RAM
	uint32_t ram_r(offs_t address) const { return uint32_t(m_ram[(address & m_ram_mask) >> 3] >> ((address & 4) << 3)); }
	uint32_t stream_r(bool fifo, offs_t address) const { return fifo ? m_fifo[(address >> 2) & 7] : ram_r(address); }
	float stream_float_r(bool fifo, offs_t address) const { return u2f(stream_r(fifo, address)); }

	bool execute(bool fifo, offs_t address, uint32_t size, int depth);
	template <typename... Params> void error(const char *format, Params &&... args);
	void reset_state();
	void set_projection(bool fifo, offs_t address);
	void set_matrix(bool fifo, offs_t address);
	void set_light_model(bool fifo, offs_t address);
	void set_light(bool fifo, offs_t address);
	void set_gmp(bool fifo, offs_t address);
	void texture_dma(uint32_t source, uint32_t destination, uint32_t size);
	void texture_copy(uint32_t source, uint32_t destination, uint32_t size);
	uint32_t pass_through(bool fifo, offs_t address, uint32_t size);
	uint32_t parameter_words(uint32_t pcw);
	uint32_t polygon(bool fifo, offs_t address);

	void fetch_position(bool fifo, offs_t address, float view[3]) const;
	void fetch_vertex(bool fifo, offs_t address, uint32_t format, vertex &v) const;
	void light_vertex(vertex &v, const float normal[3]) const;
	void flush_strip();
	void flush_volume();
	void emit_vertex(const vertex &v, bool last);
	void emit_triangle(const vertex &a, const vertex &b, const vertex &c);
	int clip_triangle(const vertex &a, const vertex &b, const vertex &c, vertex result[4]) const;
	void project(const float view[3], float screen[3]) const;

	void ta_w(const uint32_t *data, int count);

	required_device<sh7091_device> m_cpu;
	required_address_space m_cpu_space;
	required_shared_ptr<uint64_t> m_ram;
	required_device_array<powervr2_device, 2> m_pvr;

	offs_t m_vram_base[2];
	offs_t m_vram_size;
	offs_t m_ram_mask;
	emu_timer *m_dma_timer;

	// registers
	uint32_t m_sh4_ctrl;
	uint32_t m_tiler;
	uint32_t m_irq_status;
	uint32_t m_irq_mask;
	uint32_t m_regs[0x100 / 4];

	// command FIFO
	uint32_t m_fifo[8];
	bool m_dma_pending;

	// tile accelerator parameters passing through
	uint32_t m_pass_remaining;
	uint32_t m_pass_vertex_words;

	// geometry state
	uint32_t m_user_clip;
	float m_projection[4];          // fx, tx, fy, ty
	bool m_matrix_valid;
	float m_matrix[4][3];           // model to view space
	float m_normal_matrix[3][3];
	float m_near, m_far;
	float m_envmap_offset[2];

	bool m_gmp_valid;
	uint32_t m_gmp_select;
	float m_gmp_gloss;
	float m_gmp_diffuse[4];
	float m_gmp_specular[4];

	bool m_light_model_valid;
	uint32_t m_light_model_flags;
	uint16_t m_diffuse_mask;
	uint16_t m_specular_mask;
	float m_ambient_base[3];
	float m_ambient_offset[3];
	light m_lights[MAX_LIGHTS];
	uint16_t m_lights_valid;

	// model in progress
	bool m_open_volume;
	bool m_shadow_volume;
	uint32_t m_model_tsp;

	// polygon in progress
	bool m_textured;
	bool m_envmap;
	std::vector<vertex> m_strip;
};

DECLARE_DEVICE_TYPE(SEGA_315_6289, sega_315_6289_device)

#endif // MAME_SEGA_315_6289_H
