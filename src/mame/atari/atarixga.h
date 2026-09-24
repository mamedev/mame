// license:BSD-3-Clause
// copyright-holders:Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen, Andrea Bogazzi
/*************************************************************************

    atarixga.h

    Atari XGA encryption FPGAs

*************************************************************************/

#ifndef MAME_ATARI_ATARIXGA_H
#define MAME_ATARI_ATARIXGA_H

DECLARE_DEVICE_TYPE(ATARI_136094_0072, atari_136094_0072_device)
DECLARE_DEVICE_TYPE(ATARI_136095_0072, atari_136095_0072_device)
DECLARE_DEVICE_TYPE(ATARI_136094_0004A, atari_136094_0004a_device)
DECLARE_DEVICE_TYPE(ATARI_TMEK_XGA, atari_tmek_xga_device)

class atari_xga_device : public device_t
{
public:
	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) = 0;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) = 0;

protected:
	// construction/destruction
	atari_xga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
	{
	}

	std::unique_ptr<uint16_t []> m_ram; // CY7C185-45PC, only 16-Kbit used
};

class atari_136094_0072_device : public atari_xga_device
{
public:
	atari_136094_0072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr size_t RAM_WORDS = 2048;

	enum fpga_mode : uint8_t
	{
		FPGA_RESET,
		FPGA_SETKEY,
		FPGA_DECIPHER,
		FPGA_RESULT
	};

	uint16_t decipher(uint8_t k, uint16_t c);

	fpga_mode m_mode;
	uint16_t m_address;    // last written address
	uint16_t m_ciphertext; // last written ciphertext
};

class atari_136095_0072_device : public atari_xga_device
{
public:
	atari_136095_0072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Fixed upper feedback bits; Road Riot's Revenge uses 0xf000.
	void set_polynomial_high(uint16_t value) { m_poly_high = value & 0xff00; }

	void polylsb_write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t polylsb_read(offs_t offset, uint32_t mem_mask = ~0);

	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr size_t RAM_WORDS = 4096;

	enum fpga_mode : uint8_t
	{
		FPGA_SETKEY,
		FPGA_DECIPHER,
		FPGA_PROCESS,
		FPGA_RESULT
	};

	uint16_t powers2(uint8_t k, uint16_t x);
	uint16_t lfsr2(uint16_t x);
	uint16_t lfsr1(uint16_t x);
	uint16_t decipher(uint8_t k, uint16_t c);

	struct
	{
		uint16_t addr = 0;
		uint32_t data[64]{};
	} m_update;

	fpga_mode m_mode;
	uint16_t m_poly_high;
	uint8_t m_poly_lsb;
	uint16_t m_reply;
};


// GT devices share the color-RAM bus. Offsets are bytes from 0xD80000;
// read16 leaves the color-RAM value alone unless the FPGA drives the bus.
class atari_gt_xga_device : public atari_xga_device
{
public:
	virtual void write16(offs_t offset, uint16_t data) = 0;
	virtual bool read16(offs_t offset, uint16_t &data) = 0;

	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;

protected:
	atari_gt_xga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: atari_xga_device(mconfig, type, tag, owner, clock)
	{
	}
};


class atari_tmek_xga_device : public atari_gt_xga_device
{
public:
	atari_tmek_xga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void write16(offs_t offset, uint16_t data) override;
	virtual bool read16(offs_t offset, uint16_t &data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr size_t RAM_WORDS = 2048;

	enum fpga_mode : uint8_t
	{
		FPGA_IDLE,
		FPGA_SETKEY,
		FPGA_DECIPHER
	};

	fpga_mode m_mode;
	bool m_select_pending;
	uint16_t m_taps;
	uint16_t m_reply;
};


class atari_136094_0004a_device : public atari_gt_xga_device
{
public:
	atari_136094_0004a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void write16(offs_t offset, uint16_t data) override;
	virtual bool read16(offs_t offset, uint16_t &data) override;

	uint16_t decipher(offs_t index, uint16_t c) const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr size_t RAM_WORDS = 2048;

	enum fpga_mode : uint8_t
	{
		FPGA_IDLE,
		FPGA_SETKEY,
		FPGA_DECIPHER
	};

	static uint16_t key_offset(offs_t index);
	void set_character(uint16_t data);

	fpga_mode m_mode;
	uint16_t m_taps;
	uint16_t m_reply;
};

#endif // MAME_ATARI_ATARIXGA_H
