// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "jvs13551.h"

#include "cpu/tlcs90/tlcs90.h"

DEFINE_DEVICE_TYPE(SEGA_837_13551, sega_837_13551_device, "jvs13551", "Sega 837-13551 I/O Board")

void sega_837_13551_device::jvs13551_coin_1_w(int state)
{
	if(state)
		inc_coin(0);
}

void sega_837_13551_device::jvs13551_coin_2_w(int state)
{
	if(state)
		inc_coin(1);
}

static INPUT_PORTS_START(sega_837_13551_coins)
	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_WRITE_LINE_MEMBER(FUNC(sega_837_13551_device::jvs13551_coin_1_w))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_WRITE_LINE_MEMBER(FUNC(sega_837_13551_device::jvs13551_coin_2_w))
INPUT_PORTS_END

ROM_START( jvs13551 )
	// TMP90PH44N firmwares
	ROM_REGION( 0x4000, "iomcu", ROMREGION_ERASE )
	// Sega 838-13683-93 (838-13683 ;Ver1.04;98/12)
	ROM_LOAD( "sp5001.bin",   0x0000, 0x4000, CRC(3456c8cc) SHA1(f3b66ab1d2eab32e97b46077e3ed2ab5b2982325) )
	// Sega 838-13683-91 Rev.A (838-13683A ;Ver1.06;99/03)
	ROM_LOAD( "sp5001-a.bin", 0x0000, 0x4000, CRC(b52d3777) SHA1(eb882a0d4fde5d8a9fb118cb6e3547b0e9f7bfea) )
	// Sega 838-13683-93 Rev.B (838-13683B ;Ver1.07;99/06)
	ROM_LOAD( "sp5001-b.bin", 0x0000, 0x4000, CRC(28b5fb84) SHA1(8784024548d24b6a43057f06de1d53ce3a34eb12) )
	// Sega 838-13683-02 (838-13683D ;Ver1.09;99/11)
	ROM_LOAD( "sp5002-a.bin", 0x0000, 0x4000, CRC(72983a0f) SHA1(aa13276347bc643ef93e81e9ab7c905deb16c415) )
	// Sega 837-13551-92 0007 Type1 (837-13551 ;Ver1.00;98/10)
	ROM_LOAD( "315-6215.bin", 0x0000, 0x4000, CRC(98202738) SHA1(8c4dc85438298e31e25f69542804a78ff0e20962) )
ROM_END

const tiny_rom_entry *sega_837_13551_device::device_rom_region() const
{
	return ROM_NAME(jvs13551);
}

void sega_837_13551_device::device_add_mconfig(machine_config &config)
{
	TMP90PH44(config, "iomcu", 10000000); // unknown clock
}

ioport_constructor sega_837_13551_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sega_837_13551_coins);
}

sega_837_13551_device::sega_837_13551_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : jvs_device(mconfig, SEGA_837_13551, tag, owner, clock)
, port(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
{
}

const char *sega_837_13551_device::device_id()
{
	return "SEGA ENTERPRISES,LTD.;I/O BD JVS;837-13551";
}

uint8_t sega_837_13551_device::command_format_version()
{
	return 0x11;
}

uint8_t sega_837_13551_device::jvs_standard_version()
{
	return 0x20;
}

uint8_t sega_837_13551_device::comm_method_version()
{
	return 0x10;
}

void sega_837_13551_device::device_start()
{
	jvs_device::device_start();
	save_item(NAME(coin_counter));
}

void sega_837_13551_device::device_reset()
{
	jvs_device::device_reset();
	coin_counter[0] = 0;
	coin_counter[1] = 0;
}

void sega_837_13551_device::inc_coin(int coin)
{
	coin_counter[coin]++;
	if(coin_counter[coin] == 16384)
		coin_counter[coin] = 0;
}


void sega_837_13551_device::function_list(uint8_t *&buf)
{
	// SW input - 2 players, 13 bits
	*buf++ = 0x01; *buf++ = 2; *buf++ = 13; *buf++ = 0;

	// Coin input - 2 slots
	*buf++ = 0x02; *buf++ = 2; *buf++ = 0; *buf++ = 0;

	// Analog input - 8 channels
	*buf++ = 0x03; *buf++ = 8; *buf++ = 16; *buf++ = 0;

	// Driver out - 6 channels
	*buf++ = 0x12; *buf++ = 6; *buf++ = 0; *buf++ = 0;
}

bool sega_837_13551_device::coin_counters(uint8_t *&buf, uint8_t count)
{
	if(count > 2)
		return false;

	*buf++ = coin_counter[0] >> 8; *buf++ = coin_counter[0];

	if(count > 1) {
		*buf++ = coin_counter[1] >> 8; *buf++ = coin_counter[1];
	}

	return true;
}

bool sega_837_13551_device::coin_add(uint8_t slot, int32_t count)
{
	if(slot < 1 || slot > 2)
		return false;

	coin_counter[slot-1] += count;

	return true;
}

bool sega_837_13551_device::switches(uint8_t *&buf, uint8_t count_players, uint8_t bytes_per_switch)
{
	if(count_players > 2 || bytes_per_switch > 2)
		return false;

	*buf++ = port[0] ? port[0]->read() : 0;
	for(int i=0; i<count_players; i++) {
		uint32_t val = port[1+i] ? port[1+i]->read() : 0;
		for(int j=0; j<bytes_per_switch; j++)
			*buf++ = val >> ((1-j) << 3);
	}
	return true;

}

bool sega_837_13551_device::analogs(uint8_t *&buf, uint8_t count)
{
	if(count > 8)
		return false;
	for(int i=0; i<count; i++) {
		uint16_t val = port[3+i] ? port[3+i]->read() : 0x8000;
		*buf++ = val >> 8;
		*buf++ = val;
	}
	return true;
}

bool sega_837_13551_device::swoutputs(uint8_t count, const uint8_t *vals)
{
	// WARNING! JVS standard have reversed bits count order
	// so "board have 6 output bits" means 6 MSB bits is used, the same rules for input too
	if(count > 1)
		return false;
	jvs_outputs = vals[0] & 0xfc;
	logerror("837-13551: output %02x\n", jvs_outputs);
	if (port[11])
	{
		port[11]->write(jvs_outputs, 0xfc);
	}
	return true;
}

bool sega_837_13551_device::swoutputs(uint8_t id, uint8_t val)
{
	if(id > 6)
		return false;
	handle_output(port[11], id, val);
	logerror("837-13551: output %d, %d\n", id, val);
	return true;
}
