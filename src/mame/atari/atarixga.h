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
		FPGA_DECIPHER
	};

	uint16_t powers2(uint8_t k, uint16_t x);
	uint16_t lfsr2(uint16_t x);
	uint16_t lfsr1(uint16_t x);
	uint16_t decipher(uint8_t k, uint16_t c);

	fpga_mode m_mode;
	uint16_t m_address;    // last written address
	uint16_t m_ciphertext; // last written ciphertext
};

class atari_136095_0072_device : public atari_xga_device
{
public:
	atari_136095_0072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void polylsb_write(offs_t offset, uint32_t data);
	uint32_t polylsb_read(offs_t offset, uint32_t mem_mask = ~0);

	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const size_t RAM_WORDS = 4096;

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
	uint8_t m_poly_lsb;
	uint16_t m_reply;
};


class atari_136094_0004a_device : public atari_xga_device
{
public:
	atari_136094_0004a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// 16-bit access; offset is the byte offset inside the 0xD80000 color RAM window
	void write16(offs_t offset, uint16_t data);
	bool read16(offs_t offset, uint16_t &data);

	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;

	uint16_t decipher(offs_t index, uint16_t c) const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const size_t RAM_WORDS = 2048;

	enum fpga_mode : uint8_t
	{
		FPGA_IDLE,
		FPGA_SETKEY,
		FPGA_DECIPHER
	};

	static uint16_t key_offset(offs_t index);
	uint16_t lfsr(uint16_t x) const;
	void set_character(uint16_t data);

	fpga_mode m_mode;
	uint16_t m_taps;
	uint16_t m_reply;
};

#endif // MAME_ATARI_ATARIXGA_H
