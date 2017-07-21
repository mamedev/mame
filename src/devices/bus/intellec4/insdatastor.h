// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
imm4-22 Instruction/Data Storage Module

This card has four 4002 RAMs configured as a page, sockets for four
1702A or similar 256×8 PROMs configured as a ROM page, buffers for four
4-bit input ports, and latches for four 4-bit output ports.

The 4002 RAMs may be jumpered to be selected by CM-RAM1, CM-RAM2 or
CM-RAM3.  Note that the CM-RAM lines are not decoded, so the 4002 RAMs
will be mirrored.  (With two or more imm4-22 cards are installed, the
4002 RAMs will fight for the bus if pages 4-7 are selected.)

The PROMs and I/O ports may be jumpered to be mapped at 0x04xx...0x07xx,
0x08xx...0x0bff or 0x0cff...0x0fff.  Additionally, the card may be
jumpered to enable the PROMs and I/O ports only when /ENABLE MON PROM is
asserted, or at all times.

This card may seem like a good way to expand three aspects of the system
at once, but in practice the PROMs are of limited use:
* If jumpered to enable the PROMs at all times, the PROMs will fight for
  the bus if console RAM card program storage is selected.
* If jumpered to enable the PROMs only when /ENABLE MON PROM is
  asserted, the PROMs become effectively inaccessible as the boot vector
  is in the onboard monitor PROM region, and the onboard monitor PROM
  program provides no way to jump to or read from arbitrary locations.

This card was probably most useful as the cheapest way to double RAM and
I/O ports.  Without PROMs installed, mapping I/O at 0x0400...0x07ff only
when /ENABLE MON PROM is asserted would be a convenient way to provide
the necessary connections for an imm4-90 high-speed paper tape reader.

Image file for the PROMs must be an exact multiple of 256 bytes.  PROMs
are assumed to be filled in order from lowest to highest address.
Jumper changes only take effect on hard reset.


P1 Universal Slot edge connector

                         1    2
                  GND    3    4  GND
                         5    6
                         7    8
                         9   10
                  MA0   11   12  MA1
                  MA2   13   14  MA3
                  MA4   15   16  MA5
                  MA6   17   18  MA7
                   C0   19   20  C1
                        21   22
                /MDI0   23   24
                /MDI1   25   26
                /MDI3   27   28
                /MDI2   29   30
                /MDI5   31   32
                /MDI4   33   34
                /MDI7   35   36
                /MDI6   37   38
                        39   40
                 /OUT   41   42  /ENABLE MON PROM
                 -10V   43   44  -10V
                        45   46
             /CM-RAM2   47   48  /CM-RAM3
                        49   50  /CM-RAM1
                I/O 1   51   52  I/O 0
                I/O 2   53   54  /IN
                        55   56  I/O 3
                        57   58
                        59   60
                        61   62
                        63   64
                        65   66
                        67   68
                        69   70
                        71   72  /D3
                        73   74
                        75   76  /D2
                        77   78
                /SYNC   79   80  /D1
                        81   82
                  /D0   83   84
                        85   86
                        87   88  /RESET-4002
                        89   90
                        91   92
              /CM-ROM   93   94  C3
                        95   96  C2
              PHASE 2   97   98  PHASE 1
                  +5V   99  100  +5V


P1 inputs (40-pin IDC)

       ROM 4/8/12 IN0    1    2  ROM 4/8/12 IN1
       ROM 4/8/12 IN2    3    4  ROM 4/8/12 IN3
                  GND    5    6  ROM 5/9/13 IN0
       ROM 5/9/13 IN1    7    8  ROM 5/9/13 IN2
       ROM 5/9/13 IN3    9   10  GND
      ROM 6/10/14 IN0   11   12  ROM 6/10/14 IN1
      ROM 6/10/14 IN2   13   14  ROM 6/10/14 IN3
                  GND   15   16  ROM 7/11/15 IN0
      ROM 7/11/15 IN1   17   18  ROM 7/11/15 IN2
      ROM 7/11/15 IN3   19   20  GND
                        21   22
                        23   24
                        25   26
                        27   28
                        29   30
                        31   32
                        33   34
                        35   36
                        37   38
                        39   40


P2 outputs (40-pin IDC)

      ROM 4/8/12 OUT0    1    2  ROM 4/8/12 OUT1
      ROM 4/8/12 OUT2    3    4  ROM 4/8/12 OUT3
                  GND    5    6  ROM 5/9/13 OUT0
      ROM 5/9/13 OUT1    7    8  ROM 5/9/13 OUT2
      ROM 5/9/13 OUT3    9   10  GND
     ROM 6/10/14 OUT0   11   12  ROM 6/10/14 OUT1
     ROM 6/10/14 OUT2   13   14  ROM 6/10/14 OUT3
                  GND   15   16  ROM 7/11/15 OUT0
     ROM 7/11/15 OUT1   17   18  ROM 7/11/15 OUT2
     ROM 7/11/15 OUT3   19   20  GND
      RAM 4/8/12 OUT0   21   22  RAM 4/8/12 OUT1
      RAM 4/8/12 OUT2   23   24  RAM 4/8/12 OUT3
                  GND   25   26  RAM 5/9/13 OUT0
      RAM 5/9/13 OUT1   27   28  RAM 5/9/13 OUT2
      RAM 5/9/13 OUT3   29   30  GND
     RAM 6/10/14 OUT0   31   32  RAM 6/10/14 OUT1
     RAM 6/10/14 OUT2   33   34  RAM 6/10/14 OUT3
                  GND   35   36  RAM 7/11/15 OUT0
     RAM 7/11/15 OUT1   37   38  RAM 7/11/15 OUT2
     RAM 7/11/15 OUT3   39   40  GND
*/
#ifndef MAME_BUS_INTELLEC4_INSDATASTOR_H
#define MAME_BUS_INTELLEC4_INSDATASTOR_H

#pragma once

#include "intellec4.h"

namespace bus { namespace intellec4 {

class imm4_22_device : public device_t, public device_univ_card_interface, public device_image_interface
{
public:
	imm4_22_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual iodevice_t  image_type()                    const override { return IO_ROM; }
	virtual bool        is_readable()                   const override { return true; }
	virtual bool        is_writeable()                  const override { return false; }
	virtual bool        is_creatable()                  const override { return false; }
	virtual bool        must_be_loaded()                const override { return false; }
	virtual bool        is_reset_on_load()              const override { return false; }
	virtual char const *file_extensions()               const override { return "rom,bin"; }
	virtual char const *custom_instance_name()          const override { return "promimage"; }
	virtual char const *custom_brief_instance_name()    const override { return "prom"; }

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_WRITE_LINE_MEMBER(reset_4002_in) override;

private:
	DECLARE_WRITE8_MEMBER(ram_out);
	DECLARE_WRITE8_MEMBER(rom_out);
	DECLARE_READ8_MEMBER(rom_in);

	void allocate();
	void map_ram_io();
	void map_prom();
	void unmap_prom();

	required_ioport m_jumpers;

	bool    m_ram_io_mapped;

	u8      m_ram_page, m_rom_page;
	bool    m_rom_mirror;

	u8                      m_memory[256], m_status[64];
	std::unique_ptr<u8 []>  m_prom;
};

} } // namespace bus::intellec4

DECLARE_DEVICE_TYPE_NS(INTELLEC4_INST_DATA_STORAGE, bus::intellec4, imm4_22_device)

#endif // MAME_BUS_INTELLEC4_INSDATASTOR_H
