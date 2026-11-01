// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
#ifndef MAME_VIDEO_MALI200_H
#define MAME_VIDEO_MALI200_H

#pragma once

#include <unordered_map>
#include <unordered_set>

// Mali-200 pixel processor, MaliGP2 and their shared MMU.  The register layout
// is the original 16 KiB integration, not the later Mali-400 multi-core layout.
class mali200_device : public device_t
{
public:
	mali200_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }

	// Physical bus access for an entire aligned 4 KiB page: bit 0 read,
	// bit 1 write. The integration must keep it stable within a synchronous
	// GPU callback; the CPU cannot change mappings during that callback.
	auto dma_page_permissions_callback() { return m_dma_page_permissions.bind(); }
	auto pp_irq_callback() { return m_pp_irq.bind(); }
	auto gp_irq_callback() { return m_gp_irq.bind(); }
	auto mmu_irq_callback() { return m_mmu_irq.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	struct pp_fragment
	{
		// Only configured components are interpolated. Shader fetches return
		// zero for the others without clearing all 64 entries for every pixel.
		float varying[64], coordinate[4]{}, color[4]{};
		unsigned varying_count = 0;
		float depth = 0;
		bool front = true, discard = false;
	};

	struct pp_instruction
	{
		u32 address = 0;
		u64 field[12]{};
		u32 upper[12]{};
		float constant[2][4]{};
		u32 control = 0;
		unsigned count = 0, fields = 0;
		bool texture_copy = false;
		unsigned texture_sampler = 0, texture_varying = 0, texture_components = 0, texture_destination = 0;
	};

	struct pp_texture_state
	{
		u32 address = 0;
		u32 base[11]{};
		unsigned format = 0, layout = 0, max_level = 0, width = 0, height = 0;
		unsigned size = 0, stride = 0, wrap_s = 0, wrap_t = 0, range = 0;
		bool compressed = false, explicit_stride = false, unnormalized = false;
		bool reverse = false, swap_rb = false, nearest_min = false, nearest_mag = false;
		float border[4]{};
	};

	struct pp_tile
	{
		unsigned x = 0, y = 0;
		u32 scissor[4] = { 0, 32767, 0, 32767 };
		float color[256][4]{}, depth[256]{};
		u8 stencil[256]{};
		bool dirty[256]{};
	};

	u32 pp_r(offs_t offset);
	void pp_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 gp_r(offs_t offset);
	void gp_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 mmu_r(offs_t offset);
	void mmu_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void reset_pp();
	void reset_gp();
	void reset_plb();
	void reset_mmu();
	void update_irqs();
	void begin_dma_cache();
	void pp_command(u32 command);
	void gp_command(u32 command);
	bool translate(u32 address, bool write, u32 &physical, unsigned bus);
	bool read_word(u32 address, u32 &data, unsigned bus);
	bool write_word(u32 address, u32 data, unsigned bus);
	bool read_bytes(u32 address, unsigned size, u32 &data, unsigned bus);
	bool write_bytes(u32 address, unsigned size, u32 data, unsigned bus);
	bool vs_commands();
	bool plb_commands();
	bool vs_vertex(u32 vertex);
	bool vs_input(unsigned stream, u32 vertex, float *value);
	bool vs_output(unsigned stream, u32 vertex, float const *value);

	bool pp_shader(u32 const *state, pp_fragment &fragment, bool &constant);
	bool pp_texture(u32 const *state, unsigned sampler, float const *coordinate, float lod, float *color);
	bool pp_render();
	bool pp_writeback(pp_tile const &tile);
	bool pp_primitive(pp_tile &tile, u32 state_address, u32 vertex_address, u32 const *indices, bool rectangle);
	void trace_list(char const *name, u32 start, u32 end, unsigned bus);
	TIMER_CALLBACK_MEMBER(pp_run);
	TIMER_CALLBACK_MEMBER(gp_run);

	required_address_space m_dma;
	// Framework address-handler cache: mappings, taps and device accesses
	// retain their normal semantics; this does not cache the memory data.
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_dma_access;
	devcb_read8 m_dma_page_permissions;
	devcb_write_line m_pp_irq, m_gp_irq, m_mmu_irq;
	emu_timer *m_pp_timer = nullptr, *m_gp_timer = nullptr;
	u32 m_pp[0x1100 / 4]{};
	u32 m_gp[0x100 / 4]{};
	u32 m_mmu[9]{};
	u32 m_gp_heap[2]{};
	u32 m_gp_started = 0;
	u32 m_pp_jobs = 0, m_gp_jobs = 0;
	u32 m_vs_config[0x50]{};
	u32 m_vs_shader[512 * 4]{};
	u32 m_vs_uniform[304 * 4]{};
	u32 m_plb_config[16]{}, m_plb_bins[1024][2]{};
	u32 m_plb_array = 0, m_plb_stride = 0, m_plb_state = 0, m_plb_vertex = 0;
	u32 m_plb_scissor[2]{};
	bool m_plb_scissor_set = false;
	u64 m_plb_append_done = 0;
	// Host-side decode caches live only during one synchronous PP callback.
	// They are rebuilt for each job and therefore are not save-state data.
	std::unordered_map<u32, pp_instruction> m_pp_instructions;
	std::unordered_map<u32, pp_texture_state> m_pp_textures;
	// Small front caches avoid hashing the same shader/descriptor for every
	// fragment. The maps still own entries for the whole job; rehashing does
	// not invalidate references, and collisions only replace these pointers.
	std::array<pp_instruction *, 64> m_pp_instruction_cache{};
	std::array<pp_texture_state *, 64> m_pp_texture_cache{};
	// CPU execution cannot interleave with a synchronous rendering callback.
	// Cache its page walks, invalidating if GPU writes touch a walked table.
	bool m_dma_cache_active = false;
	std::array<u32, 1024> m_dma_page_tags{}, m_dma_page_entries{};
	std::array<u8, 1024> m_dma_page_access{};
	std::unordered_set<u32> m_dma_table_pages;
};

DECLARE_DEVICE_TYPE(MALI200, mali200_device)

#endif // MAME_VIDEO_MALI200_H
