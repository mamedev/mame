// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<uint8_t>

#include "fsblk.h"

namespace fs {

class fsblk_vec_t final : public fsblk_t {
private:
	class blk_t final : public block_t {
	public:
		blk_t(u8 *data, u32 size) : block_t(size), m_data(data) {}
		virtual ~blk_t() = default;

	protected:
		virtual void internal_write(u32 offset, const u8 *src, u32 size) override;
		virtual void internal_fill(u32 offset, u8 data, u32 size) override;
		virtual void internal_read(u32 offset, u8 *dst, u32 size) const override;
		virtual bool internal_eqmem(u32 offset, const u8 *src, u32 size) const override;

	private:
		u8 *const m_data;
	};

public:
	fsblk_vec_t(std::vector<u8> &data) : m_data(data) {}
	virtual ~fsblk_vec_t() = default;

	virtual u32 block_count() const override;
	virtual block_t::ptr get(u32 id) override;
	virtual void fill_all(u8 data) override;

private:
	std::vector<u8> &m_data;
};

} // namespace fs
