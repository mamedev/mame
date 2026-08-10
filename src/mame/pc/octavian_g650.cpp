// license:BSD-3-Clause
// copyright-holders:
/*********************************************************************************

Skeleton driver for Octavian G650 hardware (PC-based).

Unknown exact hardware configuration, but from the "Sweet Street" image the
following setup can be figured out:
- VIA CPU 32 bits.
- AC'97 audio.
- Moxa Smartio/Industio PCI multiport.
- ITE IT8212.
- 640×480 VESA (but some games on this hardware are advertised as 1024x768 24bit).
- Linux 2.6.20.1
Some games on this hardware are known to use a touch screen.

There is an external PCB (USB) for managing the security encryption with Atmel Atmega 64/128 for the
encryption keys and a FT232BM/245BM for the USB.

Known games on this hardware:
  2014 - Slot Club Gold
  2012 - Templari
  2012 - Corrida
  2011 - American Life
  2011 - Betty Bones
  2011 - Circus Wild
  2011 - Commandos: Jungle Raid
  2011 - Don Milionario
  2010 - Space of Magic
  2010 - Stargazer
  2010 - Submarine
  2009 - Cocktail Bar
  2009 - Paris Paris
  2008 - Black Jack Club
  2008 - Chicago Chicago
  2008 - Freewild
  2008 - Hot Race
  2008 - Roulette Club
  2007 - Admiral Parrot
  2007 - Billy Bones
  2007 - Black Light
  2007 - Chicago Night
  2007 - Commander Parrot
  2007 - Crazy Garden
  2007 - Grand Cruise
  2007 - Lucky Twins
  2007 - Night Taxi
  2007 - Pilot School
  2007 - Sweet Street
  2006 - Cinemania
  2005 - Cool Heat
  2005 - Draw Master
  2005 - Dream Factory

*********************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class octavian_g650_state : public driver_device
{
public:
	octavian_g650_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void octavian_g650(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void octavian_g650_io(address_map &map) ATTR_COLD;
	void octavian_g650_map(address_map &map) ATTR_COLD;
};


void octavian_g650_state::octavian_g650_map(address_map &map)
{
	// map(0x00000000, 0x0009ffff).ram();
	// map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	// map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

void octavian_g650_state::octavian_g650_io(address_map &map)
{
}

static INPUT_PORTS_START(octavian_g650)
INPUT_PORTS_END

void octavian_g650_state::octavian_g650(machine_config &config)
{
	PENTIUM4(config, m_maincpu, 100'000'000); // Unknown VIA 32 bits CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &octavian_g650_state::octavian_g650_map);
	m_maincpu->set_addrmap(AS_IO, &octavian_g650_state::octavian_g650_io);
	m_maincpu->set_disable();

	PCI_ROOT(config, "pci");
	// ...
}


/* The first HDD has three partitions, the second one is encrypted and is mounted as a RAM disc. The third one seems encrypted too.
   The second HDD have references to both "Admiral Parrot" and "Sweet Street" games (both from Octavian and both from 2007), but 
   seems to be "Sweet Street", and that the "Admiral Parrot" stuff is just a leftover.

   Some partitions use a custom filesystem (or not filesystem at all), but thankfully it also uses grub:
     title      system
     root       (hd0,0)
     kernel     /vmlinuz root=/dev/ram0 vga=769 console=/dev/tty2 CONSOLE=/dev/tty2 ro it821x_noraid=1 ide=nodma
     initrd     /initrd.gz
     boot

   It also prints messages in tty2:
     ./vmlinuz: Linux kernel x86 boot executable, bzImage, version 2.6.20.1 (root@dgsan) #58 PREEMPT Tue Dec 2 16:16:14 MSK 2008,
                RO-rootFS, root_dev 0X303, Normal VGA, setup size 512*14, syssize 0x25693,
                jump 0x238 0xe8c50c908db40000 instruction, protocol 2.5

   769 means it wants 640x480x8
   201:002 = missing network adaptor?
   201:004 = ?

   The encryption is a pre-block Blowfish CBC and 128 bits keys, with the IV depending on the offset.
   Linux uses cryptoloop + Blowfish with the values readed from the Atmel on the USB external board. It skips all-zero blocks.

   First partition initrd is decrypted performing a MD5 of /boot/vmlinuz
     key = RIPEMD160( ascii_hex( MD5(/boot/vmlinuz) ) )

   Then, the second partition is descrypted using the key at /sbin/init
     #!/bin/sh
     /bin/mount -n -o remount,rw /dev/loop0 /  > /dev/null 2>&1
     /bin/mount -n /proc /proc -t proc  > /dev/null 2>&1
     /sbin/losetup  -e blowfish /dev/loop1 /dev/hda2 -m 6c228c3c296d > /dev/null 2>&1
     /sbin/e2fsck -a /dev/loop1 <dev/tty2 >dev/tty2 2>&1
     /bin/mount   -n -o ro -t ext2 /dev/loop1 /mnt  > /dev/null 2>&1
     [ -d /mnt/dgt/ram ] || exit 0
     /bin/umount -n /proc  > /dev/null 2>&1
     cd /mnt  > /dev/null 2>&1
     /sbin/pivot_root . initrd  > /dev/null 2>&1
     cd /
     exec /usr/sbin/chroot . bin/sh -c \
      '/bin/mount -n /proc /proc -t proc;umount -n -lf /initrd; /bin/umount -n /proc; exec /sbin/init' \
      <dev/tty2 >dev/tty2 2>&1

   Finally, /etc/init.d/mountall.sh does the final step. and /etc/init.d/mountdg.sh mount the third
   partition (the game itself) at /dgt/distr

   /usr/sbin/key_find is used to look up for the USB external board, sending "test", and receiving
   "3dd0de425ec68a50d498f30cc66b78c79880d0c2" from the MCU.

   At boot, it shows a splash screen from "Dream Games - Gaming System". */
ROM_START(sweetstr)
	// ROM_REGION32_LE(0x80000, "bios", 0)
	// ROM_LOAD("bios.bin", 0x00000, 0x80000, ...) // Unknown PCB

	DISK_REGION("ide:0:hdd")
	DISK_IMAGE("sk_6181_sys", 0, SHA1(3c89e1e65617ae502156f662c05119c74aaee5be))

	DISK_REGION("ide:1:hdd")
	DISK_IMAGE("sk_6181_dat", 0, SHA1(9612c2733678a3c8fa354793a2055a9b37c936fc))
ROM_END

} // anonymous namespace


GAME(2007, sweetstr, 0, octavian_g650, octavian_g650, octavian_g650_state, empty_init, ROT0, "Octavian", "Sweet Street", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
