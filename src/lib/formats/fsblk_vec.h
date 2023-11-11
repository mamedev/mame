// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Block device on vector<uint8_t>

#include "fsblk.h"

namespace fs {

class fsblk_vec_base_t : public fsblk_t {
private:
	class blk_t : public iblock_t {
	public:
		blk_t(u8 *data, u32 size) : iblock_t(size), m_data(data) {}
		virtual ~blk_t() = default;

		virtual const u8 *rodata() override;
		virtual u8 *data() override;
		virtual void drop_weak_references() override;

	private:
		u8 *m_data;
	};

public:
	fsblk_vec_base_t() { };
	virtual ~fsblk_vec_base_t() = default;

	virtual u32 block_count() const override;
	virtual block_t get(u32 id) override;
	virtual void fill(u8 data) override;

protected:
	virtual std::vector<u8> &vec() = 0;
	virtual const std::vector<u8> &vec() const = 0;
};

class fsblk_vec_t : public fsblk_vec_base_t {
public:
	fsblk_vec_t(std::vector<u8> &data) : m_data(data) {}

protected:
	virtual std::vector<u8> &vec() override { return m_data; }
	virtual const std::vector<u8> &vec() const override { return m_data; }

private:
	std::vector<u8> &m_data;
};


class fsblk_vec_owned_t : public fsblk_vec_base_t {
public:
	fsblk_vec_owned_t(std::vector<u8> &&data) : m_data(std::move(data)) {}

protected:
	virtual std::vector<u8> &vec() override { return m_data; }
	virtual const std::vector<u8> &vec() const override { return m_data; }

private:
	std::vector<u8> m_data;
};

} // namespace fs
