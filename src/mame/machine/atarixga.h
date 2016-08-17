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
	atari_xga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE32_MEMBER(write);
	DECLARE_READ32_MEMBER(read);

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

	UINT16 powers2(UINT8 k, UINT16 x);
	UINT16 lfsr2(UINT16 x);
	UINT16 lfsr1(UINT16 x);
	UINT16 parity(UINT16 x);
	size_t popcount(UINT16 x);
	UINT16 ctz(UINT16 x);
	UINT16 decipher(UINT8 k, UINT16 c);

	fpga_mode m_mode;
	UINT16 m_address;    // last written address
	UINT16 m_ciphertext; // last written ciphertext
	std::unique_ptr<UINT16[]> m_ram; // CY7C185-45PC, only 16-Kbit used
};


#endif
