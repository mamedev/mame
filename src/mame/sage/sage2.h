// license:BSD-3-Clause
// copyright-holders:Curt Coder

#include "cpu/m68000/m68000.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "bus/ieee488/ieee488.h"
#include "imagedev/floppy.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/upd765.h"

#define M68000_TAG      "u68"
#define I8255A_0_TAG    "u22"
#define I8255A_1_TAG    "u39"
#define I8253_0_TAG     "u74"
#define I8253_1_TAG     "u75"
#define I8259_TAG       "u73"
#define I8251_0_TAG     "u58"
#define I8251_1_TAG     "u67"
#define UPD765_TAG      "u21"
#define TMS9914_TAG     "u6"
#define CENTRONICS_TAG  "centronics"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"

class sage2_state : public driver_device
{
public:
	sage2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, M68000_TAG)
		, m_pic(*this, I8259_TAG)
		, m_usart0(*this, I8251_0_TAG)
		, m_usart1(*this, I8251_1_TAG)
		, m_fdc(*this, UPD765_TAG)
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, M68000_TAG)
		, m_floppy0(*this, UPD765_TAG ":0")
		, m_floppy1(*this, UPD765_TAG ":1")
		, m_floppy(nullptr)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_ieee488(*this, IEEE488_TAG)
		, m_fdc_int(0)
		, m_fdie(0)
		, m_led(*this, "led0")
	{ }

	void sage2(machine_config &config);

	void init_sage2();

private:
	void update_fdc_int();

	uint16_t rom_r(offs_t offset);
	void br1_w(int state);
	void br2_w(int state);
	void ppi0_pc_w(uint8_t data);
	uint8_t ppi1_pb_r();
	void ppi1_pc_w(uint8_t data);

	void fdc_irq(int state);

	void write_centronics_ack(int state);
	void write_centronics_busy(int state);
	void write_centronics_perror(int state);
	void write_centronics_select(int state);
	void write_centronics_fault(int state);

	void sage2_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<i8251_device> m_usart0;
	required_device<i8251_device> m_usart1;
	required_device<upd765a_device> m_fdc;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<ieee488_device> m_ieee488;

	// floppy state
	int m_fdc_int = 0;
	int m_fdie = 0;

	int m_centronics_busy = 0;
	int m_centronics_perror = 0;
	int m_centronics_select = 0;
	int m_centronics_fault = 0;
	output_finder<> m_led;
};
