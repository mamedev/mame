// license:GPL2+
// copyright-holders:Felipe Sanches

// KN7000 control panel: matrix geometry and LED decode.

#include "emu.h"
#include "kn7000_cpanel.h"

#define LOG_LEDS (1U << 4)

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(KN7000_CPANEL, kn7000_cpanel_device, "kn7000_cpanel", "KN7000 Control Panel HLE")

static const uint8_t PORT_SEG[22] = {
	0x00,0x01,0x02,0x03,0x04,0x06,0x07,   // CPL_SEG0,1,2,3,4,6,7
	0x05,0x08,0x09,0x0a,0x0b,             // CPC_SEG5,8,9,10,11
	0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15  // CPR_SEG0..9  (wire ADDR 0x00..0x09)
};

static INPUT_PORTS_START(kn7000_cpanel)
	PORT_START("CPL_SEG0")   // CPL scan seg 0x00  (wire ADDR 0xC0)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDL 4 (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDL 1 (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDL 5 (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDL 2 (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("START/STOP (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDL 3 (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTRO & ENDING 2 (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SYNCHRO & BREAK (SW7)")
	PORT_START("CPL_SEG1")   // CPL scan seg 0x01  (wire ADDR 0xC1)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MEMORY/LOAD (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ORGANIST (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CUSTOM (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTERTAINER (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LATIN & WORLD (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MOVIE SHOW (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MARCH (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BALLROOM (SW7)")
	PORT_START("CPL_SEG2")   // CPL scan seg 0x02  (wire ADDR 0xC2)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COUNTRY (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("JAZZ COMBO (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUL & FUNK (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BALLAD (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MODERN DANCE (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ROCK & POP (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("60's & 70's (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 & 16 BEAT (SW7)")
	PORT_START("CPL_SEG3")   // CPL scan seg 0x03  (wire ADDR 0xC3)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTRO & ENDING 1 (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAP TEMPO (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FILL IN 2 (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FADE OUT (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FILL IN 1 (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FADE IN (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARIATION & MSA 4 (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPLIT POINT (SW7)")
	PORT_START("CPL_SEG4")   // CPL scan seg 0x04  (wire ADDR 0xC4)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARIATION & MSA 3 (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ONE TOUCH PLAY (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARIATION & MSA 2 (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUSIC STYLE ARRANGER (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARIATION & MSA 1 (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAD 3 (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAD 6/SOLO (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAD 2 (SW7)")
	PORT_START("CPL_SEG6")   // CPL scan seg 0x06  (wire ADDR 0xC6)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAD 5/SOLO (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PERFORMANCE PADS/STOP (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAD 4 (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PERFORMANCE PADS/BANK (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAD 1 (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PERFORMANCE PADS/AUTO (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEMO (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("CPL_SEG7")   // CPL scan seg 0x07  (wire ADDR 0xC7)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUSIC STYLIST (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AUTO MODE (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND SET (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PLAY CHORD OFF/ON (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ARRANGER OFF/ON (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("CPC_SEG5")   // CPC scan seg 0x05  (wire ADDR 0xC5)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("OTHER PARTS/TG (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HELP (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONTRAST UP (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONTRAST DOWN (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 1 (PART 1 ON) (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 1 (PART 1 OFF) (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 2 (PART 2 ON) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 2 (PART 2 OFF) (SW7)")
	PORT_START("CPC_SEG8")   // CPC scan seg 0x08  (wire ADDR 0xC8)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 3 (PART 3 ON) (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 3 (PART 3 OFF) (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 4 (PART 4 ON) (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 4 (PART 4 OFF) (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 5 (PART 5 ON) (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 5 (PART 5 OFF) (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 6 (PART 6 ON) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 6 (PART 6 OFF) (SW7)")
	PORT_START("CPC_SEG9")   // CPC scan seg 0x09  (wire ADDR 0xC9)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 7 (PART 7 ON) (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 7 (PART 7 OFF) (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 8 (PART 8 ON) (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 8 (PART 8 OFF) (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 9 (PART 9 ON) (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 9 (PART 9 OFF) (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 10 (PART 10 ON) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 10 (PART 10 OFF) (SW7)")
	PORT_START("CPC_SEG10")   // CPC scan seg 0x0a  (wire ADDR 0xCA)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 11 (PART 11 ON) (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 11 (PART 11 OFF) (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 12 (PART 12 ON) (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 12 (PART 12 OFF) (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 13 (PART 13 ON) (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 13 (PART 13 OFF) (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 14 (PART 14 ON) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 14 (PART 14 OFF) (SW7)")
	PORT_START("CPC_SEG11")   // CPC scan seg 0x0b  (wire ADDR 0xCB)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 15 (PART 15 ON) (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 15 (PART 15 OFF) (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE UP 16 (PART 16 ON) (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE DOWN 16 (PART 16 OFF) (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAGE UP (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAGE DOWN (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DISPLAY HOLD (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EXIT (SW7)")
	PORT_START("CPR_SEG0")   // CPR scan seg 0x0c  (wire ADDR 0x00)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOLO (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 1 (PANEL MEMORY 1) (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PROGRAM MENUS (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DISK EASY REC (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STRINGS & VOCAL (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SYNTH (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CUSTOMIZE (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("CPR_SEG1")   // CPR scan seg 0x0d  (wire ADDR 0x01)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TECHNI-CHORD (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PART SELECT RIGHT 1 (SW1)")   // panel silk: RIGHT 1 (schematic mislabels this D1069 as LEFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DISK MENU LOAD (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DISK PLAY (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("WORLD (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PAD (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CUSTOM PANEL (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SD CARD LOAD (SW7)")
	PORT_START("CPR_SEG2")   // CPR scan seg 0x0e  (wire ADDR 0x02)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PART SELECT LEFT (SW0)")   // panel silk: LEFT (schematic mislabels this D1055 as RIGHT 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PART SELECT RIGHT 2 (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EFFECT MIC (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARIATION (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MALLET & ORCH PERC (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ACCORDION REGISTER (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FAVORITES (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("CPR_SEG3")   // CPR scan seg 0x0f  (wire ADDR 0x03)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANSPOSE R1 (+) (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONDUCTOR RIGHT 1 (SW1)")   // panel silk: RIGHT 1 (schematic mislabels this D1107 as RIGHT 2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REVERB (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND DSP (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GUITAR (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAB ORGAN (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 5 (PANEL MEMORY 5) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NEXT BANK (SW7)")
	PORT_START("CPR_SEG4")   // CPR scan seg 0x10  (wire ADDR 0x04)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANSPOSE R1 (-) (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONDUCTOR RIGHT 2 (SW1)")   // panel silk: RIGHT 2 (schematic mislabels this D1114 as RIGHT 1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MULTI (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DIGITAL EFFECT (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PIANO (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DIGITAL DRAWBAR (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 4 (PANEL MEMORY 4) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BANK VIEW (PANEL MEMORY BANK SELECT) (SW7)")
	PORT_START("CPR_SEG5")   // CPR scan seg 0x11  (wire ADDR 0x05)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDR 5 (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONDUCTOR LEFT (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CHORUS (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SUSTAIN (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDR 1 (SW4)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDR 2 (SW5)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 3 (PANEL MEMORY 3) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 6 (PANEL MEMORY 6) (SW7)")
	PORT_START("CPR_SEG6")   // CPR scan seg 0x12  (wire ADDR 0x06)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDR 4 (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANSPOSE R2 (+) (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EW EXPANSION (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND EXPLORER (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 2 (PANEL MEMORY 2) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 7 (PANEL MEMORY 7) (SW7)")
	PORT_START("CPR_SEG7")   // CPR scan seg 0x13  (wire ADDR 0x07)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCDR 3 (SW0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRANSPOSE R2 (-) (SW1)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MEMORY (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ORGAN & ACCORDION (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND SET (PANEL MEMORY SET) (SW6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SOUND GROUP 8 (PANEL MEMORY 8) (SW7)")
	PORT_START("CPR_SEG8")   // CPR scan seg 0x14  (wire ADDR 0x08)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DRUM KITS (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SAX & WOODWIND (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("CPR_SEG9")   // CPR scan seg 0x15  (wire ADDR 0x09)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BASS (SW2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BRASS (SW3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor kn7000_cpanel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(kn7000_cpanel);
}

kn7000_cpanel_device::kn7000_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	kn_cpanel_base_device(mconfig, KN7000_CPANEL, tag, owner, clock),
	// Bind to this device's OWN button ports (declared in device_input_ports() above), in
	// PORT_SEG[] index order (CPL_SEG0,1,2,3,4,6,7 / CPC_SEG5,8,9,10,11 / CPR_SEG0..9).
	m_phys(*this, { "CPL_SEG0", "CPL_SEG1", "CPL_SEG2", "CPL_SEG3", "CPL_SEG4", "CPL_SEG6", "CPL_SEG7",
					"CPC_SEG5", "CPC_SEG8", "CPC_SEG9", "CPC_SEG10", "CPC_SEG11",
					"CPR_SEG0", "CPR_SEG1", "CPR_SEG2", "CPR_SEG3", "CPR_SEG4", "CPR_SEG5", "CPR_SEG6", "CPR_SEG7", "CPR_SEG8", "CPR_SEG9" }),
	m_cpl_leds(*this, "cpl_led%u", 0U),
	m_cpc_leds(*this, "cpc_led%u", 0U),
	m_cpr_leds(*this, "cpr_led%u", 0U)
{
}

void kn7000_cpanel_device::device_start()
{
	kn_cpanel_base_device::device_start();
	save_item(NAME(m_chorus_led));
	save_item(NAME(m_multi_led));
}

// Physical scan matrix == firmware normalized-segment space: port n -> normSeg PORT_SEG[n],
// SW bit forwarded unchanged (a pure identity -- each scan column is exactly one wire ADDR).
uint8_t kn7000_cpanel_device::port_seg(int port) const
{
	return PORT_SEG[port];
}

uint8_t kn7000_cpanel_device::seg_wire_addr(int seg) const
{
	static const uint8_t seg_to_addr[0x21] = {
		0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb, // normSeg 0x00-0x0B
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,           // normSeg 0x0C-0x15
		0xd0,0xd1,0xd2,0xd3,0xff,0xff,0xff,0xff,0xff,0xff,0x17,      // normSeg 0x16-0x20 (0x1A=dial, own path)
	};
	return seg_to_addr[seg];
}

void kn7000_cpanel_device::panel_led_frame(uint8_t addr, uint8_t data)
{
	const int reg = addr & 0x3f;
	if ((addr & 0xc0) == 0)
	{
		switch (reg)
		{
		case 0x00:
			m_cpr_leds[0]  = BIT(data, 0);   // D1009 CUSTOMIZE (green)
			m_cpr_leds[1]  = BIT(data, 1);   // D1023 CUSTOM PANEL (green)
			m_cpr_leds[2]  = BIT(data, 2);   // D1037 FAVORITES (green)
			m_cpr_leds[3]  = BIT(data, 3);   // D1051 SOUND GROUP 5 (PANEL MEMORY 5) (amber)
			m_cpr_leds[4]  = BIT(data, 4);   // D1065 SOUND GROUP 4 (PANEL MEMORY 4) (amber)
			m_cpr_leds[5]  = BIT(data, 5);   // D1079 SOUND GROUP 3 (PANEL MEMORY 3) (amber)
			m_cpr_leds[6]  = BIT(data, 6);   // D1093 SOUND GROUP 2 (PANEL MEMORY 2) (amber)
			m_cpr_leds[7]  = BIT(data, 7);   // D1107 CONDUCTOR RIGHT 1 (red)   (lights while the RIGHT 1 conductor is active)
			break;
		case 0x01:
			m_cpr_leds[8]  = BIT(data, 0);   // (unmapped)
			m_cpr_leds[9]  = BIT(data, 1);   // D1024 SD CARD LOAD (green)
			m_cpr_leds[10] = BIT(data, 2);   // (unmapped)
			m_cpr_leds[11] = BIT(data, 3);   // D1052 DISK IN USE (red)
			m_cpr_leds[12] = BIT(data, 4);   // D1066 BANK VIEW (PANEL MEMORY BANK SELECT) (green)
			m_cpr_leds[13] = BIT(data, 5);   // D1080 SOUND GROUP 6 (PANEL MEMORY 6) (amber)
			m_cpr_leds[14] = BIT(data, 6);   // D1094 SOUND GROUP 7 (PANEL MEMORY 7) (amber)
			m_cpr_leds[15] = BIT(data, 7);   // D1108 SOUND GROUP 8 (PANEL MEMORY 8) (red)
			break;
		case 0x02:
			m_cpr_leds[16] = BIT(data, 0);   // D1011 DISK EASY REC (green)
			m_cpr_leds[17] = BIT(data, 1);   // D1025 DISK PLAY (green)
			m_cpr_leds[18] = BIT(data, 2);   // D1039 VARIATION (red)
			m_cpr_leds[19] = BIT(data, 3);   // D1053 SOUND DSP (red)
			m_cpr_leds[20] = BIT(data, 4);   // D1067 DIGITAL EFFECT (red)
			m_cpr_leds[21] = BIT(data, 5);   // D1081 SUSTAIN (red)
			m_cpr_leds[22] = BIT(data, 6);   // D1095 SOUND EXPLORER (red)
			m_cpr_leds[23] = BIT(data, 7);   // D1116 ORGAN & ACCORDION (red)
			break;
		case 0x03:
			m_cpr_leds[24] = BIT(data, 0);   // D1012 PROGRAM MENUS (green)
			m_cpr_leds[25] = BIT(data, 1);   // D1026 DISK MENU LOAD (green)
			m_cpr_leds[26] = BIT(data, 2);   // D1040 EFFECT MIC (red)
			m_cpr_leds[27] = BIT(data, 3);   // D1068 REVERB (red)
			m_cpr_leds[28] = BIT(data, 4);   // D1054 MULTI (red)
			m_cpr_leds[29] = BIT(data, 5);   // D1082 CHORUS (red)
			// Shadow the global-effect ON state for the tone generator (the firmware's
			// only cold-toggle announcement of CHORUS is this LED -- see chorus_led()).
			m_multi_led  = BIT(data, 4);
			m_chorus_led = BIT(data, 5);
			m_cpr_leds[30] = BIT(data, 6);   // D1096 EW EXPANSION (red)
			m_cpr_leds[31] = BIT(data, 7);   // D1110 MEMORY (green)
			break;
		case 0x04:
			m_cpr_leds[32] = BIT(data, 0);   // D1027 SOLO (red)
			m_cpr_leds[33] = BIT(data, 1);   // D1013 TECHNI-CHORD (red)
			m_cpr_leds[34] = BIT(data, 2);   // D1069 PART SELECT RIGHT 1 (red)   [schematic labels D1069 LEFT; panel silk = RIGHT 1]
			m_cpr_leds[35] = BIT(data, 3);   // D1041 PART SELECT RIGHT 2 (red)
			m_cpr_leds[36] = BIT(data, 4);   // D1055 PART SELECT LEFT (red)   [schematic labels D1055 RIGHT 1; panel silk = LEFT]
			m_cpr_leds[37] = BIT(data, 5);   // D1083 TRANSPOSE R2 (+) (red)
			m_cpr_leds[38] = BIT(data, 6);   // D1097 TRANSPOSE R2 (-) (red)
			m_cpr_leds[39] = BIT(data, 7);   // D1111 TRANSPOSE R1 (+) (green)
			break;
		case 0x05:
			m_cpr_leds[40] = BIT(data, 0);   // D1042 STRINGS & VOCAL (red)
			m_cpr_leds[41] = BIT(data, 1);   // D1122 WORLD (red)
			m_cpr_leds[42] = BIT(data, 2);   // D1014 MALLET & ORCH PERC (red)
			m_cpr_leds[43] = BIT(data, 3);   // D1056 GUITAR (red)
			m_cpr_leds[44] = BIT(data, 4);   // D1070 PIANO (red)
			m_cpr_leds[45] = BIT(data, 5);   // D1098 SYNTH (red)
			m_cpr_leds[46] = BIT(data, 6);   // D1112 PAD (green)
			m_cpr_leds[47] = BIT(data, 7);   // D1117 ACCORDION REGISTER (red)
			break;
		case 0x07:
			m_cpr_leds[56] = BIT(data, 0);   // (unmapped)
			m_cpr_leds[57] = BIT(data, 1);   // (unmapped)
			m_cpr_leds[58] = BIT(data, 2);   // (unmapped)
			m_cpr_leds[59] = BIT(data, 3);   // (unmapped)
			m_cpr_leds[60] = BIT(data, 4);   // (unmapped)
			m_cpr_leds[61] = BIT(data, 5);   // (unmapped)
			m_cpr_leds[62] = BIT(data, 6);   // (unmapped)
			m_cpr_leds[63] = BIT(data, 7);   // (unmapped)
			break;
		case 0x08:
			m_cpr_leds[64] = BIT(data, 0);   // D1114 CONDUCTOR RIGHT 2 (red)   [schematic labels D1114 RIGHT 1; panel silk = RIGHT 2]
			m_cpr_leds[65] = BIT(data, 1);   // D1120 CONDUCTOR LEFT (red)
			m_cpr_leds[66] = BIT(data, 2);   // (unmapped)
			m_cpr_leds[67] = BIT(data, 3);   // (unmapped)
			m_cpr_leds[68] = BIT(data, 4);   // (unmapped)
			m_cpr_leds[69] = BIT(data, 5);   // (unmapped)
			m_cpr_leds[70] = BIT(data, 6);   // (unmapped)
			m_cpr_leds[71] = BIT(data, 7);   // (unmapped)
			break;
		case 0x09:
			m_cpr_leds[72] = BIT(data, 0);   // D1115 SOUND GROUP 1 (PANEL MEMORY 1) (red)
			m_cpr_leds[73] = BIT(data, 1);   // (unmapped)
			m_cpr_leds[74] = BIT(data, 2);   // (unmapped)
			m_cpr_leds[75] = BIT(data, 3);   // (unmapped)
			m_cpr_leds[76] = BIT(data, 4);   // (unmapped)
			m_cpr_leds[77] = BIT(data, 5);   // (unmapped)
			m_cpr_leds[78] = BIT(data, 6);   // (unmapped)
			m_cpr_leds[79] = BIT(data, 7);   // (unmapped)
			break;
		case 0x0a:
			m_cpr_leds[80] = BIT(data, 0);   // D1109 SAX & WOODWIND (red)
			m_cpr_leds[81] = BIT(data, 1);   // D1028 BRASS (red)
			m_cpr_leds[82] = BIT(data, 2);   // (unmapped)
			m_cpr_leds[83] = BIT(data, 3);   // (unmapped)
			m_cpr_leds[84] = BIT(data, 4);   // (unmapped)
			m_cpr_leds[85] = BIT(data, 5);   // (unmapped)
			m_cpr_leds[86] = BIT(data, 6);   // (unmapped)
			m_cpr_leds[87] = BIT(data, 7);   // (unmapped)
			break;
		case 0x0b:
			m_cpr_leds[88] = BIT(data, 0);   // D1123 DRUM KITS (red)
			m_cpr_leds[89] = BIT(data, 1);   // D1084 BASS (red)
			m_cpr_leds[90] = BIT(data, 2);   // (unmapped)
			m_cpr_leds[91] = BIT(data, 3);   // (unmapped)
			m_cpr_leds[92] = BIT(data, 4);   // (unmapped)
			m_cpr_leds[93] = BIT(data, 5);   // (unmapped)
			m_cpr_leds[94] = BIT(data, 6);   // (unmapped)
			m_cpr_leds[95] = BIT(data, 7);   // (unmapped)
			break;
		case 0x0c:
			m_cpr_leds[96] = BIT(data, 0);   // D1118 TRANSPOSE R1 (-) (red)
			m_cpr_leds[97] = BIT(data, 1);   // TEMPO/PROGRAM   (the only unnamed CPR LED that PANEL MEMORY SET lights)
			m_cpr_leds[98] = BIT(data, 2);   // (unmapped)
			m_cpr_leds[99] = BIT(data, 3);   // (unmapped)
			m_cpr_leds[100]= BIT(data, 4);   // (unmapped)
			m_cpr_leds[101]= BIT(data, 5);   // (unmapped)
			m_cpr_leds[102]= BIT(data, 6);   // (unmapped)
			m_cpr_leds[103]= BIT(data, 7);   // (unmapped)
			break;
		case 0x0d:
			m_cpr_leds[104]= BIT(data, 0);   // D1119 TAB ORGAN (red)
			m_cpr_leds[105]= BIT(data, 1);   // D1121 DIGITAL DRAWBAR (red)
			m_cpr_leds[106]= BIT(data, 2);   // (unmapped)
			m_cpr_leds[107]= BIT(data, 3);   // (unmapped)
			m_cpr_leds[108]= BIT(data, 4);   // (unmapped)
			m_cpr_leds[109]= BIT(data, 5);   // (unmapped)
			m_cpr_leds[110]= BIT(data, 6);   // (unmapped)
			m_cpr_leds[111]= BIT(data, 7);   // (unmapped)
			break;
		default:
			for (int bit = 0; bit < 8; bit++) { const int led = reg * 8 + bit; if (led < 512) m_cpr_leds[led] = BIT(data, bit); }
			break;
		}
	}
	else if ((addr & 0xe0) == 0xe0)
	{
		const int creg = addr & 0x1f;
		for (int bit = 0; bit < 8; bit++)
		{
			const int led = creg * 8 + bit;   // creg is 5 bits, so 0..255
			if (led < 256)
				m_cpc_leds[led] = BIT(data, bit);
		}
	}
	else
	{
		switch (reg)
		{
		case 0x00:
			m_cpl_leds[0]  = BIT(data, 0);   // D1116 INTRO & ENDING 1 (red)
			m_cpl_leds[1]  = BIT(data, 1);   // D1147 BALLROOM (red)
			m_cpl_leds[2]  = BIT(data, 2);   // 8 & 16 BEAT
			m_cpl_leds[3]  = BIT(data, 3);   // D1164 VARIATION & MSA 1 (red)
			m_cpl_leds[4]  = BIT(data, 4);   // D1180 MUSIC STYLE ARRANGER (red)
			m_cpl_leds[5]  = BIT(data, 5);   // DISPLAY HOLD
			m_cpl_leds[6]  = BIT(data, 6);   // (unmapped)
			m_cpl_leds[7]  = BIT(data, 7);   // (unmapped)
			break;
		case 0x01:
			m_cpl_leds[8]  = BIT(data, 0);   // D1115 INTRO & ENDING 2 (red)
			m_cpl_leds[9]  = BIT(data, 1);   // D1142 MARCH (red)
			m_cpl_leds[10] = BIT(data, 2);   // 60s & 70s
			m_cpl_leds[11] = BIT(data, 3);   // D1163 VARIATION & MSA 2 (red)
			m_cpl_leds[12] = BIT(data, 4);   // D1179 PERFORMANCE PADS/AUTO (green)
			m_cpl_leds[13] = BIT(data, 5);   // SPLIT POINT G3   (keyboard split-point indicator)
			m_cpl_leds[14] = BIT(data, 6);   // (unmapped)
			m_cpl_leds[15] = BIT(data, 7);   // (unmapped)
			break;
		case 0x02:
			m_cpl_leds[16] = BIT(data, 0);   // D1114 SYNCHRO & BREAK (red)
			m_cpl_leds[17] = BIT(data, 1);   // D1141 MOVIE SHOW (red)
			m_cpl_leds[18] = BIT(data, 2);   // D1130 ROCK & POP (red)
			m_cpl_leds[19] = BIT(data, 3);   // D1162 VARIATION & MSA 3 (red)
			m_cpl_leds[20] = BIT(data, 4);   // D1178 APC/SEQ VOLUME (state LED, no button) (green)
			m_cpl_leds[21] = BIT(data, 5);   // SPLIT POINT C3   (keyboard split-point indicator)
			m_cpl_leds[22] = BIT(data, 6);   // (unmapped)
			m_cpl_leds[23] = BIT(data, 7);   // (unmapped)
			break;
		case 0x03:
			m_cpl_leds[24] = BIT(data, 0);   // BEAT 1
			m_cpl_leds[25] = BIT(data, 1);   // D1145 LATIN & WORLD (red)
			m_cpl_leds[26] = BIT(data, 2);   // D1148 MODERN DANCE (red)
			m_cpl_leds[27] = BIT(data, 3);   // D1161 VARIATION & MSA 4 (red)
			m_cpl_leds[28] = BIT(data, 4);   // D1177 MUSIC STYLIST (green)
			m_cpl_leds[29] = BIT(data, 5);   // OTHER PART & FR   (the OTHER PARTS/TG button's indicator, on the CPL board)
			m_cpl_leds[30] = BIT(data, 6);   // (unmapped)
			m_cpl_leds[31] = BIT(data, 7);   // (unmapped)
			break;
		case 0x04:
			m_cpl_leds[32] = BIT(data, 0);   // BEAT 2
			m_cpl_leds[33] = BIT(data, 1);   // D1143 ENTERTAINER (red)
			m_cpl_leds[34] = BIT(data, 2);   // D1128 BALLAD (red)
			m_cpl_leds[35] = BIT(data, 3);   // D1160 FADE IN (red)
			m_cpl_leds[36] = BIT(data, 4);   // D1176 AUTO MODE (green)
			m_cpl_leds[37] = BIT(data, 5);   // (unmapped)
			m_cpl_leds[38] = BIT(data, 6);   // (unmapped)
			m_cpl_leds[39] = BIT(data, 7);   // (unmapped)
			break;
		case 0x05:
			m_cpl_leds[40] = BIT(data, 0);   // BEAT 3
			m_cpl_leds[41] = BIT(data, 1);   // D1127 CUSTOM (green)
			m_cpl_leds[42] = BIT(data, 2);   // D1126 SOUL & FUNK (red)
			m_cpl_leds[43] = BIT(data, 3);   // D1159 FILL IN 1 (red)
			m_cpl_leds[44] = BIT(data, 4);   // D1175 SOUND SET (green)
			m_cpl_leds[45] = BIT(data, 5);   // (unmapped)
			m_cpl_leds[46] = BIT(data, 6);   // (unmapped)
			m_cpl_leds[47] = BIT(data, 7);   // (unmapped)
			break;
		case 0x06:
			m_cpl_leds[48] = BIT(data, 0);   // BEAT 4
			m_cpl_leds[49] = BIT(data, 1);   // ORGANIST
			m_cpl_leds[50] = BIT(data, 2);   // D1129 JAZZ COMBO (red)
			m_cpl_leds[51] = BIT(data, 3);   // D1158 FADE OUT (red)
			m_cpl_leds[52] = BIT(data, 4);   // D1174 PLAY CHORD OFF/ON (red)
			m_cpl_leds[53] = BIT(data, 5);   // (unmapped)
			m_cpl_leds[54] = BIT(data, 6);   // (unmapped)
			m_cpl_leds[55] = BIT(data, 7);   // (unmapped)
			break;
		case 0x07:
			m_cpl_leds[56] = BIT(data, 0);   // SPLIT POINT G2   (split-point indicator; the manual has SPLIT POINT cycle G2->C3->G3->off)
			m_cpl_leds[57] = BIT(data, 1);   // D1125 MEMORY/LOAD (green)
			m_cpl_leds[58] = BIT(data, 2);   // D1144 COUNTRY (red)
			m_cpl_leds[59] = BIT(data, 3);   // D1157 FILL IN 2 (red)
			m_cpl_leds[60] = BIT(data, 4);   // D1173 ARRANGER OFF/ON (green)
			m_cpl_leds[61] = BIT(data, 5);   // (unmapped)
			m_cpl_leds[62] = BIT(data, 6);   // (unmapped)
			m_cpl_leds[63] = BIT(data, 7);   // (unmapped)
			break;
		default:
			for (int bit = 0; bit < 8; bit++) { const int led = reg * 8 + bit; if (led < 512) m_cpl_leds[led] = BIT(data, bit); }
			break;
		}
	}
	LOGMASKED(LOG_LEDS, "panel LED frame addr=%02X data=%02X\n", addr, data);
}
