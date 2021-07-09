// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<uint8_t>

#include "fsmgr.h"

class fsblk_vec_t : public fsblk_t {
private:
	class blk_t : public iblock_t {
	public:
		blk_t(uint8_t *data, uint32_t size) : iblock_t(size), m_data(data) {}
		virtual ~blk_t() = default;

		virtual const uint8_t *rodata() override;
		virtual uint8_t *data() override;
		virtual void drop_weak_references() override;

	private:
		uint8_t *m_data;
	};

public:
	fsblk_vec_t(std::vector<uint8_t> &data) : m_data(data) {}
	virtual ~fsblk_vec_t() = default;

	virtual uint32_t block_count() const override;
	virtual block_t get(uint32_t id) override;
	virtual void fill(uint8_t data) override;

private:
	std::vector<uint8_t> &m_data;
};
