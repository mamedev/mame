// license:BSD-3-Clause
// copyright-holders:Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen
/*************************************************************************

    atarixga.h

    Atari XGA encryption FPGAs

*************************************************************************/

#ifndef MAME_ATARI_ATARIXGA_H
#define MAME_ATARI_ATARIXGA_H

DECLARE_DEVICE_TYPE(ATARI_136094_0072, atari_136094_0072_device)
DECLARE_DEVICE_TYPE(ATARI_136095_0072, atari_136095_0072_device)

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

	virtual void device_start() override = 0;
	virtual void device_reset() override = 0;

	std::unique_ptr<uint16_t[]> m_ram; // CY7C185-45PC, only 16-Kbit used
};

class atari_136094_0072_device : public atari_xga_device
{
public:
	atari_136094_0072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const size_t RAM_WORDS = 2048;

	uint16_t powers2(uint8_t k, uint16_t x);
	uint16_t lfsr2(uint16_t x);
	uint16_t lfsr1(uint16_t x);
	uint16_t decipher(uint8_t k, uint16_t c);

	enum fpga_mode
	{
		FPGA_RESET,
		FPGA_SETKEY,
		FPGA_DECIPHER
	};

	fpga_mode m_mode{};
	uint16_t m_address = 0;    // last written address
	uint16_t m_ciphertext = 0; // last written ciphertext
};

class atari_136095_0072_device : public atari_xga_device
{
public:
	atari_136095_0072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void polylsb_write(offs_t offset, uint32_t data);
	uint32_t polylsb_read(offs_t offset, uint32_t mem_mask = ~0);

	virtual void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	virtual uint32_t read(offs_t offset, uint32_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const size_t RAM_WORDS = 4096;

	uint16_t powers2(uint8_t k, uint16_t x);
	uint16_t lfsr2(uint16_t x);
	uint16_t lfsr1(uint16_t x);
	uint16_t decipher(uint8_t k, uint16_t c);

	enum fpga_mode
	{
		FPGA_SETKEY,
		FPGA_DECIPHER,
		FPGA_PROCESS,
		FPGA_RESULT
	};

	struct
	{
		uint16_t addr = 0;
		uint32_t data[64]{};
	} m_update;

	fpga_mode m_mode{};
	uint8_t m_poly_lsb = 0;
	uint16_t m_reply = 0;
};


#endif // MAME_ATARI_ATARIXGA_H
