/***************************************************************************

    pc_vga.h

    PC standard VGA adaptor

***************************************************************************/

#ifndef PC_VGA_H
#define PC_VGA_H

typedef void (*pc_video_update_proc)(bitmap_t *bitmap);

MACHINE_CONFIG_EXTERN( pcvideo_vga );

struct pc_vga_interface
{
	/* VGA memory mapper */
	const char *vga_memory_bank;
	void (*map_vga_memory)(running_machine &machine, offs_t begin, offs_t end, read8_space_func rh, const char *rh_name, write8_space_func wh, const char *wh_name);

	/* VGA dipswitch (???) */
	read8_space_func read_dipswitch;

	/* where the RAM go */
	int mem_addressspace;
	offs_t mem_offset;

	/* where the ports go */
	int port_addressspace;
	offs_t port_offset;
};

struct pc_svga_interface
{
	size_t vram_size;
	int seq_regcount;
	int gc_regcount;
	int crtc_regcount;
	pc_video_update_proc (*choosevideomode)(const UINT8 *sequencer, const UINT8 *crtc, const UINT8 *gc, int *width, int *height);
};

void pc_vga_init(running_machine &machine, const struct pc_vga_interface *vga_intf, const struct pc_svga_interface *svga_intf);
void pc_vga_reset(running_machine &machine);
void *pc_vga_memory(void);
size_t pc_vga_memory_size(void);

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
#if 0
        int i;
        UINT8 *memory=machine.region("maincpu")->base()+0xc0000;
        UINT8 chksum;

		/* oak vga */
        /* plausibility check of retrace signals goes wrong */
        memory[0x00f5]=memory[0x00f6]=memory[0x00f7]=0x90;
        memory[0x00f8]=memory[0x00f9]=memory[0x00fa]=0x90;
        for (chksum=0, i=0;i<0x7fff;i++) {
                chksum+=memory[i];
        }
        memory[i]=0x100-chksum;
#endif

#endif /* PC_VGA_H */

