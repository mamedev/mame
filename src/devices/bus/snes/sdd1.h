// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_SDD1_H
#define MAME_BUS_SNES_SDD1_H

#pragma once

#include "snes_slot.h"



// ======================> sns_rom_sdd1_device

class sns_rom_sdd1_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_rom_sdd1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	sns_rom_sdd1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// misc classes for the S-DD1

	class SDD1_IM //Input Manager
	{
	public:
		SDD1_IM() { }

		void IM_prepareDecomp(uint32_t in_buf);
		uint8_t IM_getCodeword(uint8_t *ROM, uint32_t *mmc, const uint8_t code_len);

		uint32_t m_byte_ptr = 0;
		uint8_t m_bit_count = 0;
	};

	class SDD1_GCD //Golomb-Code Decoder
	{
	public:
		SDD1_GCD(SDD1_IM* associatedIM) : m_IM(associatedIM) { }

		void GCD_getRunCount(uint8_t *ROM, uint32_t *mmc, uint8_t code_num, uint8_t* MPScount, uint8_t* LPSind);

		SDD1_IM* m_IM;
	};

	class SDD1_BG // Bits Generator
	{
	public:
		SDD1_BG(SDD1_GCD* associatedGCD, uint8_t code) : m_code_num(code), m_GCD(associatedGCD) { }

		void BG_prepareDecomp();
		uint8_t BG_getBit(uint8_t *ROM, uint32_t *mmc, uint8_t* endOfRun);

		uint8_t m_code_num;
		uint8_t m_MPScount = 0;
		uint8_t m_LPSind = 0;
		SDD1_GCD* m_GCD;
	};

	struct SDD1_PEM_ContextInfo
	{
		uint8_t status;
		uint8_t MPS;
	};

	class SDD1_PEM //Probability Estimation Module
	{
	public:
		SDD1_PEM(
				SDD1_BG* associatedBG0,
				SDD1_BG* associatedBG1,
				SDD1_BG* associatedBG2,
				SDD1_BG* associatedBG3,
				SDD1_BG* associatedBG4,
				SDD1_BG* associatedBG5,
				SDD1_BG* associatedBG6,
				SDD1_BG* associatedBG7)
			: m_BG{ associatedBG0, associatedBG1, associatedBG2, associatedBG3, associatedBG4, associatedBG5, associatedBG6, associatedBG7 }
		{
		}

		void PEM_prepareDecomp();
		uint8_t PEM_getBit(uint8_t *ROM, uint32_t *mmc, uint8_t context);

		SDD1_PEM_ContextInfo m_contextInfo[32];
		SDD1_BG* m_BG[8];
	};


	class SDD1_CM
	{
	public:
		SDD1_CM(SDD1_PEM* associatedPEM) : m_PEM(associatedPEM) { }

		void CM_prepareDecomp(uint8_t *ROM, uint32_t *mmc, uint32_t first_byte);
		uint8_t CM_getBit(uint8_t *ROM, uint32_t *mmc);

		uint8_t m_bitplanesInfo = 0;
		uint8_t m_contextBitsInfo = 0;
		uint8_t m_bit_number = 0;
		uint8_t m_currBitplane = 0;
		uint16_t m_prevBitplaneBits[8];
		SDD1_PEM* m_PEM;
	};


	class SDD1_OL
	{
	public:
		SDD1_OL(SDD1_CM* associatedCM) : m_CM(associatedCM) { }

		void OL_prepareDecomp(uint8_t *ROM, uint32_t *mmc, uint32_t first_byte, uint16_t out_len, uint8_t *out_buf);
		void OL_launch(uint8_t *ROM, uint32_t *mmc);

		uint8_t m_bitplanesInfo = 0;
		uint16_t m_length = 0;
		uint8_t* m_buffer = nullptr;
		SDD1_CM* m_CM;
	};

	class SDD1_emu
	{
	public:
		SDD1_emu(running_machine &machine);

		running_machine &machine() const { return m_machine; }

		void SDD1emu_decompress(uint8_t *ROM, uint32_t *mmc, uint32_t in_buf, uint16_t out_len, uint8_t *out_buf);

		std::unique_ptr<SDD1_IM> m_IM;
		std::unique_ptr<SDD1_GCD> m_GCD;
		std::unique_ptr<SDD1_BG> m_BG0;
		std::unique_ptr<SDD1_BG> m_BG1;
		std::unique_ptr<SDD1_BG> m_BG2;
		std::unique_ptr<SDD1_BG> m_BG3;
		std::unique_ptr<SDD1_BG> m_BG4;
		std::unique_ptr<SDD1_BG> m_BG5;
		std::unique_ptr<SDD1_BG> m_BG6;
		std::unique_ptr<SDD1_BG> m_BG7;
		std::unique_ptr<SDD1_PEM> m_PEM;
		std::unique_ptr<SDD1_CM> m_CM;
		std::unique_ptr<SDD1_OL> m_OL;

	private:
		running_machine& m_machine;
	};

	uint8_t read_helper(uint32_t offset);

	uint8_t m_sdd1_enable;  // channel bit-mask
	uint8_t m_xfer_enable;  // channel bit-mask
	uint32_t m_mmc[4];      // memory map controller ROM indices

	struct
	{
		uint32_t addr;    // $43x2-$43x4 -- DMA transfer address
		uint16_t size;    // $43x5-$43x6 -- DMA transfer size
	} m_dma[8];

	std::unique_ptr<SDD1_emu> m_sdd1emu;

	struct
	{
		std::unique_ptr<uint8_t[]> data;    // pointer to decompressed S-DD1 data (65536 bytes)
		uint16_t offset;  // read index into S-DD1 decompression buffer
		uint32_t size;    // length of data buffer; reads decrement counter, set ready to false at 0
		uint8_t ready;    // 1 when data[] is valid; 0 to invoke sdd1emu.decompress()
	} m_buffer;
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_LOROM_SDD1, sns_rom_sdd1_device)

#endif // MAME_BUS_SNES_SDD1_H
