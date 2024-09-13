// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Apple II bus slot cards

    All of these cards are electrically compatible, but for compatibility
    reasons we divide them by machine type.

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "4play.h"
#include "a2alfam2.h"
#include "a2applicard.h"
#include "a2arcadebd.h"
#include "a2cffa.h"
#include "a2corvus.h"
#include "a2diskiing.h"
#include "a2dx1.h"
#include "a2echoii.h"
#include "a2hsscsi.h"
#include "a2iwm.h"
#include "a2mcms.h"
#include "a2memexp.h"
#include "a2midi.h"
#include "a2mockingboard.h"
#include "a2parprn.h"
#include "a2pic.h"
#include "a2sam.h"
#include "a2scsi.h"
#include "a2sd.h"
#include "a2softcard.h"
#include "a2ssc.h"
#include "a2superdrive.h"
#include "a2swyft.h"
#include "a2themill.h"
#include "a2thunderclock.h"
#include "a2ultraterm.h"
#include "a2videoterm.h"
#include "a2vulcan.h"
#include "a2wico_trackball.h"
#include "a2zipdrive.h"
#include "booti.h"
#include "byte8251.h"
#include "ccs7710.h"
#include "cmsscsi.h"
#include "computereyes2.h"
#include "excel9.h"
#include "ezcgi.h"
#include "grafex.h"
#include "grappler.h"
#include "lancegs.h"
#include "laser128.h"
#include "mouse.h"
#include "noisemaker.h"
#include "pc_xporter.h"
#include "prodosromdrive.h"
#include "q68.h"
#include "ramcard128k.h"
#include "ramcard16k.h"
#include "romcard.h"
#include "sider.h"
#include "snesmax.h"
#include "softcard3.h"
#include "ssbapple.h"
#include "ssprite.h"
#include "suprterminal.h"
#include "timemasterho.h"
#include "titan3plus2.h"
#include "transwarp.h"
#include "uniprint.h"
#include "uthernet.h"
#include "vistaa800.h"


void apple2_slot0_cards(device_slot_interface &device)
{
	device.option_add("lang", A2BUS_RAMCARD16K);      // Apple II RAM Language Card
	device.option_add("ssram", A2BUS_RAMCARD128K);    // Saturn Systems 128K extended language card
	device.option_add("romcard", A2BUS_ROMCARDUSER);    // Apple II ROM Card that loads a custom ROM image
	device.option_add("romcardfp", A2BUS_ROMCARDFP);    // Apple II ROM Card with Autostart Monitor + Applesoft BASIC
	device.option_add("romcardint", A2BUS_ROMCARDINT);  // Apple II ROM Card with Autostart Monitor + Integer BASIC
}

void apple2_cards(device_slot_interface &device)
{
	device.option_add("diskiing", A2BUS_DISKIING);             // Disk II Controller Card, cycle-accurate version
	device.option_add("diskiing13", A2BUS_DISKIING13);         // Disk II Controller Card, cycle-accurate version
	device.option_add("diskiiiwm", A2BUS_IWM_CARD);            // IWM Disk II Controller Card
	device.option_add("mockingboard", A2BUS_MOCKINGBOARD);     // Sweet Micro Systems Mockingboard
	device.option_add("phasor", A2BUS_PHASOR);                 // Applied Engineering Phasor
	device.option_add("cffa2", A2BUS_CFFA2);                   // CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware
	device.option_add("cffa202", A2BUS_CFFA2_6502);            // CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware
	device.option_add("memexp", A2BUS_MEMEXP);                 // Apple II Memory Expansion Card
	device.option_add("ramfactor", A2BUS_RAMFACTOR);           // Applied Engineering RamFactor
	device.option_add("thclock", A2BUS_THUNDERCLOCK);          // ThunderWare ThunderClock Plus
	device.option_add("softcard", A2BUS_SOFTCARD);             // Microsoft SoftCard
	device.option_add("videoterm", A2BUS_VIDEOTERM);           // Videx VideoTerm
	device.option_add("ssc", A2BUS_SSC);                       // Apple Super Serial Card
	device.option_add("ssi", APRICORN_SSI);                    // Apricorn Super Serial Imager
	device.option_add("swyft", A2BUS_SWYFT);                   // IAI SwyftCard
	device.option_add("themill", A2BUS_THEMILL);               // Stellation Two The Mill (6809 card)
	device.option_add("sam", A2BUS_SAM);                       // SAM Software Automated Mouth (8-bit DAC + speaker)
	device.option_add("alfam2", A2BUS_ALFAM2);                 // ALF Apple Music II
	device.option_add("echoii", A2BUS_ECHOII);                 // Street Electronics Echo II
	device.option_add("ap16", A2BUS_IBSAP16);                  // IBS AP16 (German VideoTerm clone)
	device.option_add("ap16alt", A2BUS_IBSAP16ALT);            // IBS AP16 (German VideoTerm clone), alternate revision
	device.option_add("vtc1", A2BUS_VTC1);                     // Unknown VideoTerm clone
	device.option_add("arcbd", A2BUS_ARCADEBOARD);             // Third Millenium Engineering Arcade Board
	device.option_add("midi", A2BUS_MIDI);                     // Generic 6840+6850 MIDI board
	device.option_add("zipdrive", A2BUS_ZIPDRIVE);             // ZIP Technologies IDE card
	device.option_add("echoiiplus", A2BUS_ECHOPLUS);           // Street Electronics Echo Plus (Echo II + Mockingboard clone)
	device.option_add("scsi", A2BUS_SCSI);                     // Apple II SCSI Card
	device.option_add("applicard", A2BUS_APPLICARD);           // PCPI Applicard
	device.option_add("aesms", A2BUS_AESMS);                   // Applied Engineering Super Music Synthesizer
	device.option_add("ultraterm", A2BUS_ULTRATERM);           // Videx UltraTerm (original)
	device.option_add("ultratermenh", A2BUS_ULTRATERMENH);     // Videx UltraTerm (enhanced //e)
	device.option_add("aevm80", A2BUS_AEVIEWMASTER80);         // Applied Engineering ViewMaster 80
	device.option_add("parprn", A2BUS_PARPRN);                 // Apple II Parallel Printer Interface Card
	device.option_add("4dparprn", A2BUS_4DPARPRN);             // Fourth Dimension Parallel Printer Interface
	device.option_add("parallel", A2BUS_PIC);                  // Apple II Parallel Interface Card
	device.option_add("grappler", A2BUS_GRAPPLER);             // Orange Micro Grappler Printer Interface card
	device.option_add("grapplus", A2BUS_GRAPPLERPLUS);         // Orange Micro Grappler+ Printer Interface card
	device.option_add("bufgrapplus", A2BUS_BUFGRAPPLERPLUS);   // Orange Micro Buffered Grappler+ Printer Interface card
	device.option_add("bufgrapplusa", A2BUS_BUFGRAPPLERPLUSA); // Orange Micro Buffered Grappler+ (rev A) Printer Interface card
	device.option_add("corvus", A2BUS_CORVUS);                 // Corvus flat-cable HDD interface (see notes in a2corvus.c)
	device.option_add("mcms1", A2BUS_MCMS1);                   // Mountain Computer Music System, card 1 of 2
	device.option_add("mcms2", A2BUS_MCMS2);                   // Mountain Computer Music System, card 2 of 2.  must be in card 1's slot + 1!
	device.option_add("dx1", A2BUS_DX1);                       // Decillonix DX-1 sampler card
	device.option_add("tm2ho", A2BUS_TIMEMASTERHO);            // Applied Engineering TimeMaster II H.O.
	device.option_add("mouse", A2BUS_MOUSE);                   // Apple II Mouse Card
	device.option_add("ezcgi", A2BUS_EZCGI);                   // E-Z Color Graphics Interface
	device.option_add("ezcgi9938", A2BUS_EZCGI_9938);          // E-Z Color Graphics Interface (TMS9938)
	device.option_add("ezcgi9958", A2BUS_EZCGI_9958);          // E-Z Color Graphics Interface (TMS9958)
	device.option_add("ssprite", A2BUS_SSPRITE);               // Synetix SuperSprite Board
	device.option_add("ssbapple", A2BUS_SSBAPPLE);             // SSB Apple speech board
	device.option_add("4play", A2BUS_4PLAY);                   // 4Play Joystick Card (Rev. B)
	device.option_add("snesmax", A2BUS_SNES_MAX);              // SNES MAX controller adapter
	device.option_add("ceyes2", A2BUS_COMPUTEREYES2);          // ComputerEyes/2 Video Digitizer
	device.option_add("twarp", A2BUS_TRANSWARP);               // AE TransWarp accelerator
	device.option_add("applesurance", A2BUS_APPLESURANCE);     // Applesurance Diagnostic Controller
//  device.option_add("magicmusician", A2BUS_MAGICMUSICIAN);   // Magic Musician Card
	device.option_add("byte8251", A2BUS_BYTE8251);             // BYTE Magazine 8251 serial card
	device.option_add("noisemaker", A2BUS_NOISEMAKER);         // ADS Noisemaker II
	device.option_add("suprterm", A2BUS_SUPRTERMINAL);         // M&R Enterprises SUP'R'TERMINAL 80-column card
	device.option_add("uniprint", A2BUS_UNIPRINT);             // Videx Uniprint parallel printer card
	device.option_add("ccs7710", A2BUS_CCS7710);               // California Computer Systems Model 7710 Asynchronous Serial Interface
	device.option_add("booti", A2BUS_BOOTI);                   // Booti Card
	device.option_add("q68", A2BUS_Q68);                       // Stellation Q68 68000 card
	device.option_add("q68plus", A2BUS_Q68PLUS);               // Stellation Q68 Plus 68000 card
	device.option_add("grafex", A2BUS_GRAFEX);                 // Grafex card (uPD7220 graphics)
	device.option_add("excel9", A2BUS_EXCEL9);                 // Excel-9 (6809 coprocessor)
	device.option_add("vistaa800", A2BUS_VISTAA800);           // Vista A800 8" Disk Controller Card
	device.option_add("wicotrackball", A2BUS_WICOTRACKBALL);   // Wico Trackball
}

void apple2e_cards(device_slot_interface &device)
{
	device.option_add("diskiing", A2BUS_DISKIING);             // Disk II Controller Card, cycle-accurate version
	device.option_add("diskiing13", A2BUS_DISKIING13);         // Disk II Controller Card, cycle-accurate version
	device.option_add("diskiiiwm", A2BUS_IWM_CARD);            // IWM Disk II Controller Card
	device.option_add("mockingboard", A2BUS_MOCKINGBOARD);     // Sweet Micro Systems Mockingboard
	device.option_add("phasor", A2BUS_PHASOR);                 // Applied Engineering Phasor
	device.option_add("cffa2", A2BUS_CFFA2);                   // CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware
	device.option_add("cffa202", A2BUS_CFFA2_6502);            // CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware
	device.option_add("memexp", A2BUS_MEMEXP);                 // Apple II Memory Expansion Card
	device.option_add("ramfactor", A2BUS_RAMFACTOR);           // Applied Engineering RamFactor
	device.option_add("thclock", A2BUS_THUNDERCLOCK);          // ThunderWare ThunderClock Plus
	device.option_add("softcard", A2BUS_SOFTCARD);             // Microsoft SoftCard
	device.option_add("videoterm", A2BUS_VIDEOTERM);           // Videx VideoTerm
	device.option_add("ssc", A2BUS_SSC);                       // Apple Super Serial Card
	device.option_add("ssi", APRICORN_SSI);                    // Apricorn Super Serial Imager
	device.option_add("swyft", A2BUS_SWYFT);                   // IAI SwyftCard
	device.option_add("themill", A2BUS_THEMILL);               // Stellation Two The Mill (6809 card)
	device.option_add("sam", A2BUS_SAM);                       // SAM Software Automated Mouth (8-bit DAC + speaker)
	device.option_add("alfam2", A2BUS_ALFAM2);                 // ALF Apple Music II
	device.option_add("echoii", A2BUS_ECHOII);                 // Street Electronics Echo II
	device.option_add("ap16", A2BUS_IBSAP16);                  // IBS AP16 (German VideoTerm clone)
	device.option_add("ap16alt", A2BUS_IBSAP16ALT);            // IBS AP16 (German VideoTerm clone), alternate revision
	device.option_add("vtc1", A2BUS_VTC1);                     // Unknown VideoTerm clone
	device.option_add("arcbd", A2BUS_ARCADEBOARD);             // Third Millenium Engineering Arcade Board
	device.option_add("midi", A2BUS_MIDI);                     // Generic 6840+6850 MIDI board
	device.option_add("zipdrive", A2BUS_ZIPDRIVE);             // ZIP Technologies IDE card
	device.option_add("focusdrive", A2BUS_FOCUSDRIVE);         // Focus Drive IDE card
	device.option_add("echoiiplus", A2BUS_ECHOPLUS);           // Street Electronics Echo Plus (Echo II + Mockingboard clone)
	device.option_add("scsi", A2BUS_SCSI);                     // Apple II SCSI Card
	device.option_add("hsscsi", A2BUS_HSSCSI);                 // Apple II High-Speed SCSI Card
	device.option_add("applicard", A2BUS_APPLICARD);           // PCPI Applicard
	device.option_add("aesms", A2BUS_AESMS);                   // Applied Engineering Super Music Synthesizer
	device.option_add("ultraterm", A2BUS_ULTRATERM);           // Videx UltraTerm (original)
	device.option_add("ultratermenh", A2BUS_ULTRATERMENH);     // Videx UltraTerm (enhanced //e)
	device.option_add("aevm80", A2BUS_AEVIEWMASTER80);         // Applied Engineering ViewMaster 80
	device.option_add("parprn", A2BUS_PARPRN);                 // Apple II Parallel Printer Interface Card
	device.option_add("4dparprn", A2BUS_4DPARPRN);             // Fourth Dimension Parallel Printer Interface
	device.option_add("parallel", A2BUS_PIC);                  // Apple II Parallel Interface Card
	device.option_add("grappler", A2BUS_GRAPPLER);             // Orange Micro Grappler Printer Interface card
	device.option_add("grapplus", A2BUS_GRAPPLERPLUS);         // Orange Micro Grappler+ Printer Interface card
	device.option_add("bufgrapplus", A2BUS_BUFGRAPPLERPLUS);   // Orange Micro Buffered Grappler+ Printer Interface card
	device.option_add("bufgrapplusa", A2BUS_BUFGRAPPLERPLUSA); // Orange Micro Buffered Grappler+ (rev A) Printer Interface card
	device.option_add("corvus", A2BUS_CORVUS);                 // Corvus flat-cable HDD interface (see notes in a2corvus.c)
	device.option_add("mcms1", A2BUS_MCMS1);                   // Mountain Computer Music System, card 1 of 2
	device.option_add("mcms2", A2BUS_MCMS2);                   // Mountain Computer Music System, card 2 of 2.  must be in card 1's slot + 1!
	device.option_add("dx1", A2BUS_DX1);                       // Decillonix DX-1 sampler card
	device.option_add("tm2ho", A2BUS_TIMEMASTERHO);            // Applied Engineering TimeMaster II H.O.
	device.option_add("mouse", A2BUS_MOUSE);                   // Apple II Mouse Card
	device.option_add("ezcgi", A2BUS_EZCGI);                   // E-Z Color Graphics Interface
	device.option_add("ezcgi9938", A2BUS_EZCGI_9938);          // E-Z Color Graphics Interface (TMS9938)
	device.option_add("ezcgi9958", A2BUS_EZCGI_9958);          // E-Z Color Graphics Interface (TMS9958)
//  device.option_add("magicmusician", A2BUS_MAGICMUSICIAN);   // Magic Musician Card
	device.option_add("pcxport", A2BUS_PCXPORTER);             // Applied Engineering PC Transporter
	device.option_add("ssprite", A2BUS_SSPRITE);               // Synetix SuperSprite Board
	device.option_add("ssbapple", A2BUS_SSBAPPLE);             // SSB Apple speech board
	device.option_add("twarp", A2BUS_TRANSWARP);               // AE TransWarp accelerator
	device.option_add("vulcan", A2BUS_VULCANIIE);              // Applied Engineering Vulcan IDE drive
	device.option_add("4play", A2BUS_4PLAY);                   // 4Play Joystick Card (Rev. B)
	device.option_add("snesmax", A2BUS_SNES_MAX);              // SNES MAX controller adapter
	device.option_add("ceyes2", A2BUS_COMPUTEREYES2);          // ComputerEyes/2 Video Digitizer
	device.option_add("applesurance", A2BUS_APPLESURANCE);     // Applesurance Diagnostic Controller
	device.option_add("byte8251", A2BUS_BYTE8251);             // BYTE Magazine 8251 serial card
	device.option_add("noisemaker", A2BUS_NOISEMAKER);         // ADS Noisemaker II
	device.option_add("cmsscsi", A2BUS_CMSSCSI);               // CMS Apple II SCSI Card
	device.option_add("uthernet", A2BUS_UTHERNET);             // A2RetroSystems Uthernet card
	device.option_add("sider2", A2BUS_SIDER2);                 // Advanced Tech Systems / First Class Peripherals Sider 2 SASI card
	device.option_add("sider1", A2BUS_SIDER1);                 // Advanced Tech Systems / First Class Peripherals Sider 1 SASI card
	device.option_add("uniprint", A2BUS_UNIPRINT);             // Videx Uniprint parallel printer card
	device.option_add("ccs7710", A2BUS_CCS7710);               // California Computer Systems Model 7710 Asynchronous Serial Interface
	device.option_add("booti", A2BUS_BOOTI);                   // Booti Card
	device.option_add("lancegs", A2BUS_LANCEGS);               // ///SHH SYSTEME LANceGS Card
	device.option_add("q68", A2BUS_Q68);                       // Stellation Q68 68000 card
	device.option_add("q68plus", A2BUS_Q68PLUS);               // Stellation Q68 Plus 68000 card
	device.option_add("a2sd", A2BUS_A2SD);                     // Florian Reitz AppleIISD
	device.option_add("grafex", A2BUS_GRAFEX);                 // Grafex card (uPD7220 graphics)
	device.option_add("pdromdrive", A2BUS_PRODOSROMDRIVE);     // ProDOS ROM Drive
	device.option_add("superdrive", A2BUS_SUPERDRIVE);         // Apple II 3.5" Disk Controller
	device.option_add("vistaa800", A2BUS_VISTAA800);           // Vista A800 8" Disk Controller Card
	device.option_add("wicotrackball", A2BUS_WICOTRACKBALL);   // Wico Trackball
}

void apple2gs_cards(device_slot_interface &device)
{
	device.option_add("diskiing", A2BUS_DISKIING);             // Disk II Controller Card, cycle-accurate version
	device.option_add("mockingboard", A2BUS_MOCKINGBOARD);     // Sweet Micro Systems Mockingboard
	device.option_add("phasor", A2BUS_PHASOR);                 // Applied Engineering Phasor
	device.option_add("cffa2", A2BUS_CFFA2);                   // CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware
	device.option_add("cffa202", A2BUS_CFFA2_6502);            // CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware
	device.option_add("memexp", A2BUS_MEMEXP);                 // Apple II Memory Expansion Card
	device.option_add("ramfactor", A2BUS_RAMFACTOR);           // Applied Engineering RamFactor
	device.option_add("thclock", A2BUS_THUNDERCLOCK);          // ThunderWare ThunderClock Plus
	device.option_add("softcard", A2BUS_SOFTCARD);             // Microsoft SoftCard
	device.option_add("videoterm", A2BUS_VIDEOTERM);           // Videx VideoTerm
	device.option_add("ssc", A2BUS_SSC);                       // Apple Super Serial Card
	device.option_add("ssi", APRICORN_SSI);                    // Apricorn Super Serial Imager
	device.option_add("swyft", A2BUS_SWYFT);                   // IAI SwyftCard
	device.option_add("themill", A2BUS_THEMILL);               // Stellation Two The Mill (6809 card)
	device.option_add("sam", A2BUS_SAM);                       // SAM Software Automated Mouth (8-bit DAC + speaker)
	device.option_add("alfam2", A2BUS_ALFAM2);                 // ALF Apple Music II
	device.option_add("echoii", A2BUS_ECHOII);                 // Street Electronics Echo II
	device.option_add("ap16", A2BUS_IBSAP16);                  // IBS AP16 (German VideoTerm clone)
	device.option_add("ap16alt", A2BUS_IBSAP16ALT);            // IBS AP16 (German VideoTerm clone), alternate revision
	device.option_add("vtc1", A2BUS_VTC1);                     // Unknown VideoTerm clone
	device.option_add("arcbd", A2BUS_ARCADEBOARD);             // Third Millenium Engineering Arcade Board
	device.option_add("midi", A2BUS_MIDI);                     // Generic 6840+6850 MIDI board
	device.option_add("zipdrive", A2BUS_ZIPDRIVE);             // ZIP Technologies IDE card
	device.option_add("focusdrive", A2BUS_FOCUSDRIVE);         // Focus Drive IDE card
	device.option_add("echoiiplus", A2BUS_ECHOPLUS);           // Street Electronics Echo Plus (Echo II + Mockingboard clone)
	device.option_add("scsi", A2BUS_SCSI);                     // Apple II SCSI Card
	device.option_add("hsscsi", A2BUS_HSSCSI);                 // Apple II High-Speed SCSI Card
	device.option_add("applicard", A2BUS_APPLICARD);           // PCPI Applicard
	device.option_add("aesms", A2BUS_AESMS);                   // Applied Engineering Super Music Synthesizer
	device.option_add("ultraterm", A2BUS_ULTRATERM);           // Videx UltraTerm (original)
	device.option_add("ultratermenh", A2BUS_ULTRATERMENH);     // Videx UltraTerm (enhanced //e)
	device.option_add("aevm80", A2BUS_AEVIEWMASTER80);         // Applied Engineering ViewMaster 80
	device.option_add("parprn", A2BUS_PARPRN);                 // Apple II Parallel Printer Interface Card
	device.option_add("4dparprn", A2BUS_4DPARPRN);             // Fourth Dimension Parallel Printer Interface
	device.option_add("parallel", A2BUS_PIC);                  // Apple Parallel Interface Card
	device.option_add("grappler", A2BUS_GRAPPLER);             // Orange Micro Grappler Printer Interface card
	device.option_add("grapplus", A2BUS_GRAPPLERPLUS);         // Orange Micro Grappler+ Printer Interface card
	device.option_add("bufgrapplus", A2BUS_BUFGRAPPLERPLUS);   // Orange Micro Buffered Grappler+ Printer Interface card
	device.option_add("bufgrapplusa", A2BUS_BUFGRAPPLERPLUSA); // Orange Micro Buffered Grappler+ (rev A) Printer Interface card
	device.option_add("corvus", A2BUS_CORVUS);                 // Corvus flat-cable HDD interface (see notes in a2corvus.c)
	device.option_add("mcms1", A2BUS_MCMS1);                   // Mountain Computer Music System, card 1 of 2
	device.option_add("mcms2", A2BUS_MCMS2);                   // Mountain Computer Music System, card 2 of 2.  must be in card 1's slot + 1!
	device.option_add("dx1", A2BUS_DX1);                       // Decillonix DX-1 sampler card
	device.option_add("tm2ho", A2BUS_TIMEMASTERHO);            // Applied Engineering TimeMaster II H.O.
	device.option_add("mouse", A2BUS_MOUSE);                   // Apple II Mouse Card
	device.option_add("ezcgi", A2BUS_EZCGI);                   // E-Z Color Graphics Interface
	device.option_add("ezcgi9938", A2BUS_EZCGI_9938);          // E-Z Color Graphics Interface (TMS9938)
	device.option_add("ezcgi9958", A2BUS_EZCGI_9958);          // E-Z Color Graphics Interface (TMS9958)
	device.option_add("vulcan", A2BUS_VULCAN);                 // Applied Engineering Vulcan IDE drive
	device.option_add("vulcangold", A2BUS_VULCANGOLD);         // Applied Engineering Vulcan Gold IDE drive
	device.option_add("4play", A2BUS_4PLAY);                   // 4Play Joystick Card (Rev. B)
	device.option_add("snesmax", A2BUS_SNES_MAX);              // SNES MAX controller adapter
//  device.option_add("magicmusician", A2BUS_MAGICMUSICIAN);   // Magic Musician Card
//  device.option_add("pcxport", A2BUS_PCXPORTER);             // Applied Engineering PC Transporter
	device.option_add("byte8251", A2BUS_BYTE8251);             // BYTE Magazine 8251 serial card
	device.option_add("noisemaker", A2BUS_NOISEMAKER);         // ADS Noisemaker II
//  device.option_add("hostram", A2BUS_HOSTRAM);               // Slot 7 RAM for GS Plus host protocol
//  device.option_add("ramfast", A2BUS_RAMFAST);               // C.V. Technologies RAMFast SCSI card
	device.option_add("cmsscsi", A2BUS_CMSSCSI);               // CMS Apple II SCSI Card
	device.option_add("uthernet", A2BUS_UTHERNET);             // A2RetroSystems Uthernet card
	device.option_add("sider2", A2BUS_SIDER2);                 // Advanced Tech Systems / First Class Peripherals Sider 2 SASI card
	device.option_add("sider1", A2BUS_SIDER1);                 // Advanced Tech Systems / First Class Peripherals Sider 1 SASI card
	device.option_add("uniprint", A2BUS_UNIPRINT);             // Videx Uniprint parallel printer card
	device.option_add("ccs7710", A2BUS_CCS7710);               // California Computer Systems Model 7710 Asynchronous Serial Interface
	device.option_add("booti", A2BUS_BOOTI);                   // Booti Card
	device.option_add("lancegs", A2BUS_LANCEGS);               // ///SHH SYSTEME LANceGS Card
	device.option_add("q68", A2BUS_Q68);                       // Stellation Q68 68000 card
	device.option_add("q68plus", A2BUS_Q68PLUS);               // Stellation Q68 Plus 68000 card
	device.option_add("grafex", A2BUS_GRAFEX);                 // Grafex card (uPD7220 graphics)
	device.option_add("pdromdrive", A2BUS_PRODOSROMDRIVE);     // ProDOS ROM Drive
	device.option_add("superdrive", A2BUS_SUPERDRIVE);         // Apple II 3.5" Disk Controller
	device.option_add("wicotrackball", A2BUS_WICOTRACKBALL);   // Wico Trackball
}

void apple3_cards(device_slot_interface &device)
{
	device.option_add("cffa2", A2BUS_CFFA2_6502);          // CFFA2.0 Compact Flash for Apple II (www.dreher.net), 6502 firmware
	device.option_add("applicard", A2BUS_APPLICARD);       // PCPI Applicard
	device.option_add("thclock", A2BUS_THUNDERCLOCK);      // ThunderWare ThunderClock Plus - driver assumes slot 2 by default
	device.option_add("mouse", A2BUS_MOUSE);               // Apple II Mouse Card
	device.option_add("focusdrive", A2BUS_FOCUSDRIVE);     // Focus Drive IDE card
	device.option_add("cmsscsi", A2BUS_CMSSCSI);           // CMS Apple II SCSI Card
	device.option_add("titan3plus2", A2BUS_TITAN3PLUS2);   // Titan /// Plus 2 card
	device.option_add("mockingboard", A2BUS_MOCKINGBOARD); // Sweet Micro Systems Mockingboard (experimental on ///)
	device.option_add("softcard3", A2BUS_SOFTCARD3);       // Microsoft SoftCard ///
	device.option_add("grafex", A2BUS_GRAFEX);             // Grafex card (µPD7220 graphics)
}
