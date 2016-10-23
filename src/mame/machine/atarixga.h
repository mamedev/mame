// license:BSD-3-Clause
// copyright-holders:Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen
/*************************************************************************

    atarixga.h

    Atari XGA encryption FPGA

*************************************************************************/

#ifndef __MACHINE_ATARIXGA__
#define __MACHINE_ATARIXGA__

extern const device_type ATARI_XGA;

class atari_xga_device :  public device_t
{
public:
	// construction/destruction
	atari_xga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static const size_t RAM_WORDS = 2048;

	enum fpga_mode
	{
		FPGA_RESET,
		FPGA_SETKEY,
		FPGA_DECIPHER
	};

	uint16_t powers2(uint8_t k, uint16_t x);
	uint16_t lfsr2(uint16_t x);
	uint16_t lfsr1(uint16_t x);
	uint16_t parity(uint16_t x);
	size_t popcount(uint16_t x);
	uint16_t ctz(uint16_t x);
	uint16_t decipher(uint8_t k, uint16_t c);

	fpga_mode m_mode;
	uint16_t m_address;    // last written address
	uint16_t m_ciphertext; // last written ciphertext
	std::unique_ptr<uint16_t[]> m_ram; // CY7C185-45PC, only 16-Kbit used
};


#endif
