/***************************************************************************

    pc_vga.h

    PC standard VGA adaptor

***************************************************************************/

#ifndef PC_VGA_H
#define PC_VGA_H

MACHINE_CONFIG_EXTERN( pcvideo_vga );
MACHINE_CONFIG_EXTERN( pcvideo_vga_isa );

VIDEO_START( vga );

struct pc_svga_interface
{
	size_t vram_size;
	int seq_regcount;
	int gc_regcount;
	int crtc_regcount;
	void (*choosevideomode)(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, const UINT8 *sequencer, const UINT8 *crtc, int *width, int *height);
};

void pc_vga_init(running_machine &machine, read8_space_func read_dipswitch, const struct pc_svga_interface *svga_intf);
void pc_vga_io_init(running_machine &machine, address_space *mem_space, offs_t mem_offset, address_space *io_space, offs_t port_offset);
void pc_vga_gamtor_io_init(running_machine &machine, address_space *mem_space, offs_t mem_offset, address_space *io_space, offs_t port_offset);
void pc_svga_trident_io_init(running_machine &machine, address_space *mem_space, offs_t mem_offset, address_space *io_space, offs_t port_offset);
void pc_vga_reset(running_machine &machine);
void *pc_vga_memory(void);
size_t pc_vga_memory_size(void);
void pc_video_start(running_machine &machine);

READ8_HANDLER(vga_port_03b0_r);
READ8_HANDLER(vga_port_03c0_r);
READ8_HANDLER(vga_port_03d0_r);
READ8_HANDLER(vga_mem_r);
WRITE8_HANDLER(vga_port_03b0_w);
WRITE8_HANDLER(vga_port_03c0_w);
WRITE8_HANDLER(vga_port_03d0_w);
WRITE8_HANDLER(vga_mem_w);

/* per-device implementations */
READ8_HANDLER(tseng_et4k_03b0_r);
WRITE8_HANDLER(tseng_et4k_03b0_w);
READ8_HANDLER(tseng_et4k_03c0_r);
WRITE8_HANDLER(tseng_et4k_03c0_w);
READ8_HANDLER(tseng_et4k_03d0_r);
WRITE8_HANDLER(tseng_et4k_03d0_w);
READ8_HANDLER(tseng_mem_r);
WRITE8_HANDLER(tseng_mem_w);

READ8_HANDLER(trident_03c0_r);
WRITE8_HANDLER(trident_03c0_w);
READ8_HANDLER(trident_03d0_r);
WRITE8_HANDLER(trident_03d0_w);
READ8_HANDLER(trident_mem_r);
WRITE8_HANDLER(trident_mem_w);

READ8_HANDLER(s3_port_03b0_r);
WRITE8_HANDLER(s3_port_03b0_w);
READ8_HANDLER(s3_port_03c0_r);
WRITE8_HANDLER(s3_port_03c0_w);
READ8_HANDLER(s3_port_03d0_r);
WRITE8_HANDLER(s3_port_03d0_w);
READ16_HANDLER(s3_port_9ae8_r);
WRITE16_HANDLER(s3_port_9ae8_w);
READ8_HANDLER(s3_mem_r);
WRITE8_HANDLER(s3_mem_w);

/*
  pega notes (paradise)
  build in amstrad pc1640

  ROM_LOAD("40100", 0xc0000, 0x8000, CRC(d2d1f1ae))

  4 additional dipswitches
  seems to have emulation modes at register level
  (mda/hgc lines bit 8 not identical to ega/vga)

  standard ega/vga dipswitches
  00000000  320x200
  00000001  640x200 hanging
  00000010  640x200 hanging
  00000011  640x200 hanging

  00000100  640x350 hanging
  00000101  640x350 hanging EGA mono
  00000110  320x200
  00000111  640x200

  00001000  640x200
  00001001  640x200
  00001010  720x350 partial visible
  00001011  720x350 partial visible

  00001100  320x200
  00001101  320x200
  00001110  320x200
  00001111  320x200

*/

/*
  oak vga (oti 037 chip)
  (below bios patch needed for running)

  ROM_LOAD("oakvga.bin", 0xc0000, 0x8000, CRC(318c5f43))
*/

#endif /* PC_VGA_H */

