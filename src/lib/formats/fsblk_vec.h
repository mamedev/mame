// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<uint8_t>

#include "fsblk.h"

namespace fs {

class fsblk_vec_t : public fsblk_t {
private:
	class blk_t : public block_t {
	public:
		blk_t(u8 *data, u32 size) : block_t(size), m_data(data) {}
		virtual ~blk_t() = default;

		virtual const u8 *rodata() const override;
		virtual u8 *data() override;

	private:
		u8 *m_data;
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
