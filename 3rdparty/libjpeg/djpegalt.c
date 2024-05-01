/*
 * alternate djpeg.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 2009-2020 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains an alternate user interface for the JPEG decompressor.
 * One or more input files are named on the command line, and output file
 * names are created by substituting an appropriate extension.
 */

#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */

#include <ctype.h>		/* to declare isprint() */

#ifdef USE_CCOMMAND		/* command-line reader for Macintosh */
#ifdef __MWERKS__
#include <SIOUX.h>              /* Metrowerks needs this */
#include <console.h>		/* ... and this */
#endif
#ifdef THINK_C
#include <console.h>		/* Think declares it here */
#endif
#endif

#ifndef PATH_MAX		/* ANSI maximum-pathname-length constant */
#define PATH_MAX 256
#endif


/* Create the add-on message string table. */

#define JMESSAGE(code,string)	string ,

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};


/*
 * Automatic determination of available memory.
 */

static long default_maxmem;	/* saves value determined at startup, or 0 */

#ifndef FREE_MEM_ESTIMATE	/* may be defined from command line */

#ifdef MSDOS			/* For MS-DOS (unless flat-memory model) */

#include <dos.h>		/* for access to intdos() call */

LOCAL(long)
unused_dos_memory (void)
/* Obtain total amount of unallocated DOS memory */
{
  union REGS regs;
  long nparas;

  regs.h.ah = 0x48;		/* DOS function Allocate Memory Block */
  regs.x.bx = 0xFFFF;		/* Ask for more memory than DOS can have */
  (void) intdos(&regs, &regs);
  /* DOS will fail and return # of paragraphs actually available in BX. */
  nparas = (unsigned int) regs.x.bx;
  /* Times 16 to convert to bytes. */
  return nparas << 4;
}

/* The default memory setting is 95% of the available space. */
#define FREE_MEM_ESTIMATE  ((unused_dos_memory() * 95L) / 100L)

#endif /* MSDOS */

#ifdef ATARI			/* For Atari ST/STE/TT, Pure C or Turbo C */

#include <ext.h>

/* The default memory setting is 90% of the available space. */
#define FREE_MEM_ESTIMATE  (((long) coreleft() * 90L) / 100L)

#endif /* ATARI */

/* Add memory-estimation procedures for other operating systems here,
 * with appropriate #ifdef's around them.
 */

#endif /* !FREE_MEM_ESTIMATE */


/*
 * This list defines the known output image formats
 * (not all of which need be supported by a given version).
 * You can change the default output format by defining DEFAULT_FMT;
 * indeed, you had better do so if you undefine PPM_SUPPORTED.
 */

typedef enum {
	FMT_BMP,		/* BMP format (Windows flavor) */
	FMT_GIF,		/* GIF format (LZW compressed) */
	FMT_GIF0,		/* GIF format (uncompressed) */
	FMT_OS2,		/* BMP format (OS/2 flavor) */
	FMT_PPM,		/* PPM/PGM (PBMPLUS formats) */
	FMT_RLE,		/* RLE format */
	FMT_TARGA,		/* Targa format */
	FMT_TIFF		/* TIFF format */
} IMAGE_FORMATS;

#ifndef DEFAULT_FMT		/* so can override from CFLAGS in Makefile */
#define DEFAULT_FMT	FMT_BMP
#endif

static IMAGE_FORMATS requested_fmt;


/*
 * Argument-parsing code.
 * The switch parser is designed to be useful with DOS-style command line
 * syntax, ie, intermixed switches and file names, where only the switches
 * to the left of a given file name affect processing of that file.
 */


static const char * progname;	/* program name for error messages */
static char * outfilename;	/* for -outfile switch */


LOCAL(void)
usage (void)
/* complain about bad command line */
{
  fprintf(stderr, "usage: %s [switches] inputfile(s)\n", progname);
  fprintf(stderr, "List of input files may use wildcards (* and ?)\n");
  fprintf(stderr, "Output filename is same as input filename except for extension\n");

  fprintf(stderr, "Switches (names may be abbreviated):\n");
  fprintf(stderr, "  -colors N      Reduce image to no more than N colors\n");
  fprintf(stderr, "  -fast          Fast, low-quality processing\n");
  fprintf(stderr, "  -grayscale     Force grayscale output\n");
  fprintf(stderr, "  -rgb           Force RGB output\n");
#ifdef IDCT_SCALING_SUPPORTED
  fprintf(stderr, "  -scale M/N     Scale output image by fraction M/N, eg, 1/8\n");
#endif
#ifdef BMP_SUPPORTED
  fprintf(stderr, "  -bmp           Select BMP output format (Windows style)%s\n",
	  (DEFAULT_FMT == FMT_BMP ? " (default)" : ""));
#endif
#ifdef GIF_SUPPORTED
  fprintf(stderr, "  -gif           Select GIF output format (LZW compressed)%s\n",
	  (DEFAULT_FMT == FMT_GIF ? " (default)" : ""));
  fprintf(stderr, "  -gif0          Select GIF output format (uncompressed)%s\n",
	  (DEFAULT_FMT == FMT_GIF0 ? " (default)" : ""));
#endif
#ifdef BMP_SUPPORTED
  fprintf(stderr, "  -os2           Select BMP output format (OS/2 style)%s\n",
	  (DEFAULT_FMT == FMT_OS2 ? " (default)" : ""));
#endif
#ifdef PPM_SUPPORTED
  fprintf(stderr, "  -pnm           Select PBMPLUS (PPM/PGM) output format%s\n",
	  (DEFAULT_FMT == FMT_PPM ? " (default)" : ""));
#endif
#ifdef RLE_SUPPORTED
  fprintf(stderr, "  -rle           Select Utah RLE output format%s\n",
	  (DEFAULT_FMT == FMT_RLE ? " (default)" : ""));
#endif
#ifdef TARGA_SUPPORTED
  fprintf(stderr, "  -targa         Select Targa output format%s\n",
	  (DEFAULT_FMT == FMT_TARGA ? " (default)" : ""));
#endif
  fprintf(stderr, "Switches for advanced users:\n");
#ifdef DCT_ISLOW_SUPPORTED
  fprintf(stderr, "  -dct int       Use integer DCT method%s\n",
	  (JDCT_DEFAULT == JDCT_ISLOW ? " (default)" : ""));
#endif
#ifdef DCT_IFAST_SUPPORTED
  fprintf(stderr, "  -dct fast      Use fast integer DCT (less accurate)%s\n",
	  (JDCT_DEFAULT == JDCT_IFAST ? " (default)" : ""));
#endif
#ifdef DCT_FLOAT_SUPPORTED
  fprintf(stderr, "  -dct float     Use floating-point DCT method%s\n",
	  (JDCT_DEFAULT == JDCT_FLOAT ? " (default)" : ""));
#endif
  fprintf(stderr, "  -dither fs     Use F-S dithering (default)\n");
  fprintf(stderr, "  -dither none   Don't use dithering in quantization\n");
  fprintf(stderr, "  -dither ordered  Use ordered dither (medium speed, quality)\n");
#ifdef QUANT_2PASS_SUPPORTED
  fprintf(stderr, "  -map FILE      Map to colors used in named image file\n");
#endif
  fprintf(stderr, "  -nosmooth      Don't use high-quality upsampling\n");
#ifdef QUANT_1PASS_SUPPORTED
  fprintf(stderr, "  -onepass       Use 1-pass quantization (fast, low quality)\n");
#endif
#ifndef FREE_MEM_ESTIMATE
  fprintf(stderr, "  -maxmemory N   Maximum memory to use (in kbytes)\n");
#endif
  fprintf(stderr, "  -outfile name  Specify name for output file\n");
  fprintf(stderr, "  -verbose  or  -debug   Emit debug output\n");
  exit(EXIT_FAILURE);
}


LOCAL(int)
parse_switches (j_decompress_ptr cinfo, int argc, char **argv,
		int last_file_arg_seen, boolean for_real)
/* Parse optional switches.
 * Returns argv[] index of first file-name argument (== argc if none).
 * Any file names with indexes <= last_file_arg_seen are ignored;
 * they have presumably been processed in a previous iteration.
 * (Pass 0 for last_file_arg_seen on the first or only iteration.)
 * for_real is FALSE on the first (dummy) pass; we may skip any expensive
 * processing.
 */
{
  int argn;
  char * arg;

  /* Set up default JPEG parameters. */
  requested_fmt = DEFAULT_FMT;	/* set default output file format */
  outfilename = NULL;
  cinfo->err->trace_level = 0;
  if (default_maxmem > 0)	/* override library's default value */
    cinfo->mem->max_memory_to_use = default_maxmem;

  /* Scan command line options, adjust parameters */

  for (argn = 1; argn < argc; argn++) {
    arg = argv[argn];
    if (*arg != '-') {
      /* Not a switch, must be a file name argument */
      if (argn <= last_file_arg_seen) {
	outfilename = NULL;	/* -outfile applies to just one input file */
	continue;		/* ignore this name if previously processed */
      }
      break;			/* else done parsing switches */
    }
    arg++;			/* advance past switch marker character */

    if (keymatch(arg, "bmp", 1)) {
      /* BMP output format (Windows flavor). */
      requested_fmt = FMT_BMP;

    } else if (keymatch(arg, "colors", 1) || keymatch(arg, "colours", 1) ||
	       keymatch(arg, "quantize", 1) || keymatch(arg, "quantise", 1)) {
      /* Do color quantization. */
      int val;

      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (sscanf(argv[argn], "%d", &val) != 1)
	usage();
      cinfo->desired_number_of_colors = val;
      cinfo->quantize_colors = TRUE;

    } else if (keymatch(arg, "dct", 2)) {
      /* Select IDCT algorithm. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (keymatch(argv[argn], "int", 1)) {
	cinfo->dct_method = JDCT_ISLOW;
      } else if (keymatch(argv[argn], "fast", 2)) {
	cinfo->dct_method = JDCT_IFAST;
      } else if (keymatch(argv[argn], "float", 2)) {
	cinfo->dct_method = JDCT_FLOAT;
      } else
	usage();

    } else if (keymatch(arg, "dither", 2)) {
      /* Select dithering algorithm. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (keymatch(argv[argn], "fs", 2)) {
	cinfo->dither_mode = JDITHER_FS;
      } else if (keymatch(argv[argn], "none", 2)) {
	cinfo->dither_mode = JDITHER_NONE;
      } else if (keymatch(argv[argn], "ordered", 2)) {
	cinfo->dither_mode = JDITHER_ORDERED;
      } else
	usage();

    } else if (keymatch(arg, "debug", 1) || keymatch(arg, "verbose", 1)) {
      /* Enable debug printouts. */
      /* On first -d, print version identification */
      static boolean printed_version = FALSE;

      if (! printed_version) {
	fprintf(stderr, "Independent JPEG Group's DJPEG, version %s\n%s\n",
		JVERSION, JCOPYRIGHT);
	printed_version = TRUE;
      }
      cinfo->err->trace_level++;

    } else if (keymatch(arg, "fast", 1)) {
      /* Select recommended processing options for quick-and-dirty output. */
      cinfo->two_pass_quantize = FALSE;
      cinfo->dither_mode = JDITHER_ORDERED;
      if (! cinfo->quantize_colors) /* don't override an earlier -colors */
	cinfo->desired_number_of_colors = 216;
      cinfo->dct_method = JDCT_FASTEST;
      cinfo->do_fancy_upsampling = FALSE;

    } else if (keymatch(arg, "gif", 1)) {
      /* GIF output format (LZW compressed). */
      requested_fmt = FMT_GIF;

    } else if (keymatch(arg, "gif0", 4)) {
      /* GIF output format (uncompressed). */
      requested_fmt = FMT_GIF0;

    } else if (keymatch(arg, "grayscale", 2) || keymatch(arg, "greyscale",2)) {
      /* Force monochrome output. */
      cinfo->out_color_space = JCS_GRAYSCALE;

    } else if (keymatch(arg, "rgb", 3)) {
      /* Force RGB output. */
      cinfo->out_color_space = JCS_RGB;

    } else if (keymatch(arg, "map", 3)) {
      /* Quantize to a color map taken from an input file. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (for_real) {		/* too expensive to do twice! */
#ifdef QUANT_2PASS_SUPPORTED	/* otherwise can't quantize to supplied map */
	FILE * mapfile;

	if ((mapfile = fopen(argv[argn], READ_BINARY)) == NULL) {
	  fprintf(stderr, "%s: can't open %s\n", progname, argv[argn]);
	  exit(EXIT_FAILURE);
	}
	read_color_map(cinfo, mapfile);
	fclose(mapfile);
	cinfo->quantize_colors = TRUE;
#else
	ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
      }

    } else if (keymatch(arg, "maxmemory", 3)) {
      /* Maximum memory in Kb (or Mb with 'm'). */
      long lval;
      char ch = 'x';

      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (sscanf(argv[argn], "%ld%c", &lval, &ch) < 1)
	usage();
      if (ch == 'm' || ch == 'M')
	lval *= 1000L;
      cinfo->mem->max_memory_to_use = lval * 1000L;

    } else if (keymatch(arg, "nosmooth", 3)) {
      /* Suppress fancy upsampling. */
      cinfo->do_fancy_upsampling = FALSE;

    } else if (keymatch(arg, "onepass", 3)) {
      /* Use fast one-pass quantization. */
      cinfo->two_pass_quantize = FALSE;

    } else if (keymatch(arg, "os2", 3)) {
      /* BMP output format (OS/2 flavor). */
      requested_fmt = FMT_OS2;

    } else if (keymatch(arg, "outfile", 4)) {
      /* Set output file name. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      outfilename = argv[argn];	/* save it away for later use */

    } else if (keymatch(arg, "pnm", 1) || keymatch(arg, "ppm", 1)) {
      /* PPM/PGM output format. */
      requested_fmt = FMT_PPM;

    } else if (keymatch(arg, "rle", 1)) {
      /* RLE output format. */
      requested_fmt = FMT_RLE;

    } else if (keymatch(arg, "scale", 1)) {
      /* Scale the output image by a fraction M/N. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (sscanf(argv[argn], "%u/%u",
		 &cinfo->scale_num, &cinfo->scale_denom) < 1)
	usage();

    } else if (keymatch(arg, "targa", 1)) {
      /* Targa output format. */
      requested_fmt = FMT_TARGA;

    } else {
      usage();			/* bogus switch */
    }
  }

  return argn;			/* return index of next arg (file name) */
}


/*
 * Marker processor for COM and interesting APPn markers.
 * This replaces the library's built-in processor, which just skips the marker.
 * We want to print out the marker as text, to the extent possible.
 * Note this code relies on a non-suspending data source.
 */

LOCAL(unsigned int)
jpeg_getc (j_decompress_ptr cinfo)
/* Read next byte */
{
  struct jpeg_source_mgr * datasrc = cinfo->src;

  if (datasrc->bytes_in_buffer == 0) {
    if (! (*datasrc->fill_input_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
  }
  datasrc->bytes_in_buffer--;
  return GETJOCTET(*datasrc->next_input_byte++);
}


METHODDEF(boolean)
print_text_marker (j_decompress_ptr cinfo)
{
  boolean traceit = (cinfo->err->trace_level >= 1);
  INT32 length;
  unsigned int ch;
  unsigned int lastch = 0;

  length = jpeg_getc(cinfo) << 8;
  length += jpeg_getc(cinfo);
  length -= 2;			/* discount the length word itself */

  if (traceit) {
    if (cinfo->unread_marker == JPEG_COM)
      fprintf(stderr, "Comment, length %ld:\n", (long) length);
    else			/* assume it is an APPn otherwise */
      fprintf(stderr, "APP%d, length %ld:\n",
	      cinfo->unread_marker - JPEG_APP0, (long) length);
  }

  while (--length >= 0) {
    ch = jpeg_getc(cinfo);
    if (traceit) {
      /* Emit the character in a readable form.
       * Nonprintables are converted to \nnn form,
       * while \ is converted to \\.
       * Newlines in CR, CR/LF, or LF form will be printed as one newline.
       */
      if (ch == '\r') {
	fprintf(stderr, "\n");
      } else if (ch == '\n') {
	if (lastch != '\r')
	  fprintf(stderr, "\n");
      } else if (ch == '\\') {
	fprintf(stderr, "\\\\");
      } else if (isprint(ch)) {
	putc(ch, stderr);
      } else {
	fprintf(stderr, "\\%03o", ch);
      }
      lastch = ch;
    }
  }

  if (traceit)
    fprintf(stderr, "\n");

  return TRUE;
}


/*
 * Check for overwrite of an existing file; clear it with user
 */

#ifndef NO_OVERWRITE_CHECK

LOCAL(boolean)
is_write_ok (char * outfname)
{
  FILE * ofile;
  int ch;

  ofile = fopen(outfname, READ_BINARY);
  if (ofile == NULL)
    return TRUE;		/* not present */
  fclose(ofile);		/* oops, it is present */

  for (;;) {
    fprintf(stderr, "%s already exists, overwrite it? [y/n] ",
	    outfname);
    fflush(stderr);
    ch = getc(stdin);
    if (ch != '\n')		/* flush rest of line */
      while (getc(stdin) != '\n')
	/* nothing */;

    switch (ch) {
    case 'Y':
    case 'y':
      return TRUE;
    case 'N':
    case 'n':
      return FALSE;
    /* otherwise, ask again */
    }
  }
}

#endif


/*
 * Process a single input file name, and return its index in argv[].
 * File names at or to left of old_file_index have been processed already.
 */

LOCAL(int)
process_one_file (int argc, char **argv, int old_file_index)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  char *infilename;
  char workfilename[PATH_MAX];
  const char *default_extension = NULL;
#ifdef PROGRESS_REPORT
  struct cdjpeg_progress_mgr progress;
#endif
  int file_index;
  djpeg_dest_ptr dest_mgr = NULL;
  FILE * input_file = NULL;
  FILE * output_file = NULL;
  JDIMENSION num_scanlines;

  /* Initialize the JPEG decompression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  /* Add some application-specific error messages (from cderror.h) */
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;

  /* Insert custom marker processor for COM and APP12.
   * APP12 is used by some digital camera makers for textual info,
   * so we provide the ability to display it as text.
   * If you like, additional APPn marker types can be selected for display,
   * but don't try to override APP0 or APP14 this way (see libjpeg.txt).
   */
  jpeg_set_marker_processor(&cinfo, JPEG_COM, print_text_marker);
  jpeg_set_marker_processor(&cinfo, JPEG_APP0+12, print_text_marker);

  /* Now safe to enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
  enable_signal_catcher((j_common_ptr) &cinfo);
#endif

  /* Scan command line to find next file name.
   * It is convenient to use just one switch-parsing routine, but the switch
   * values read here are ignored; we will rescan the switches after opening
   * the input file.
   * (Exception: tracing level set here controls verbosity for COM markers
   * found during jpeg_read_header...)
   */

  file_index = parse_switches(&cinfo, argc, argv, old_file_index, FALSE);
  if (file_index >= argc) {
    fprintf(stderr, "%s: missing input file name\n", progname);
    usage();
  }

  /* Open the input file. */
  infilename = argv[file_index];
  if ((input_file = fopen(infilename, READ_BINARY)) == NULL) {
    fprintf(stderr, "%s: can't open %s\n", progname, infilename);
    goto fail;
  }

#ifdef PROGRESS_REPORT
  start_progress_monitor((j_common_ptr) &cinfo, &progress);
#endif

  /* Specify data source for decompression */
  jpeg_stdio_src(&cinfo, input_file);

  /* Read file header, set default decompression parameters */
  (void) jpeg_read_header(&cinfo, TRUE);

  /* Adjust default decompression parameters by re-parsing the options */
  file_index = parse_switches(&cinfo, argc, argv, old_file_index, TRUE);

  /* Initialize the output module now to let it override any crucial
   * option settings (for instance, GIF wants to force color quantization).
   */
  switch (requested_fmt) {
#ifdef BMP_SUPPORTED
  case FMT_BMP:
    dest_mgr = jinit_write_bmp(&cinfo, FALSE);
    default_extension = ".bmp";
    break;
  case FMT_OS2:
    dest_mgr = jinit_write_bmp(&cinfo, TRUE);
    default_extension = ".bmp";
    break;
#endif
#ifdef GIF_SUPPORTED
  case FMT_GIF:
    dest_mgr = jinit_write_gif(&cinfo, TRUE);
    default_extension = ".gif";
    break;
  case FMT_GIF0:
    dest_mgr = jinit_write_gif(&cinfo, FALSE);
    default_extension = ".gif";
    break;
#endif
#ifdef PPM_SUPPORTED
  case FMT_PPM:
    dest_mgr = jinit_write_ppm(&cinfo);
    default_extension = ".ppm";
    break;
#endif
#ifdef RLE_SUPPORTED
  case FMT_RLE:
    dest_mgr = jinit_write_rle(&cinfo);
    default_extension = ".rle";
    break;
#endif
#ifdef TARGA_SUPPORTED
  case FMT_TARGA:
    dest_mgr = jinit_write_targa(&cinfo);
    default_extension = ".tga";
    break;
#endif
  default:
    ERREXIT(&cinfo, JERR_UNSUPPORTED_FORMAT);
  }

  /* If user didn't supply -outfile switch, select output file name. */
  if (outfilename == NULL) {
    int i;

    outfilename = workfilename;
    /* Make outfilename be infilename with appropriate extension */
    strcpy(outfilename, infilename);
    for (i = (int)strlen(outfilename)-1; i >= 0; i--) {
      switch (outfilename[i]) {
      case ':':
      case '/':
      case '\\':
	i = 0;			/* stop scanning */
	break;
      case '.':
	outfilename[i] = '\0';	/* lop off existing extension */
	i = 0;			/* stop scanning */
	break;
      default:
	break;			/* keep scanning */
      }
    }
    strcat(outfilename, default_extension);
  }

  fprintf(stderr, "Decompressing %s => %s\n", infilename, outfilename);
#ifndef NO_OVERWRITE_CHECK
  if (! is_write_ok(outfilename))
    goto fail;
#endif

  /* Open the output file. */
  if ((output_file = fopen(outfilename, WRITE_BINARY)) == NULL) {
    fprintf(stderr, "%s: can't create %s\n", progname, outfilename);
    goto fail;
  }
  dest_mgr->output_file = output_file;

  /* Start decompressor */
  (void) jpeg_start_decompress(&cinfo);

  /* Write output file header */
  (*dest_mgr->start_output) (&cinfo, dest_mgr);

  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }

#ifdef PROGRESS_REPORT
  /* Hack: count final pass as done in case finish_output does an extra pass.
   * The library won't have updated completed_passes.
   */
  progress.pub.completed_passes = progress.pub.total_passes;
#endif

  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
  (*dest_mgr->finish_output) (&cinfo, dest_mgr);
  (void) jpeg_finish_decompress(&cinfo);

  /* Clean up and exit */
fail:
  jpeg_destroy_decompress(&cinfo);

  if (input_file != NULL) fclose(input_file);
  if (output_file != NULL) fclose(output_file);

#ifdef PROGRESS_REPORT
  end_progress_monitor((j_common_ptr) &cinfo);
#endif

  /* Disable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
  enable_signal_catcher((j_common_ptr) NULL);
#endif

  return file_index;
}


/*
 * The main program.
 */

int
main (int argc, char **argv)
{
  int file_index;

  /* On Mac, fetch a command line. */
#ifdef USE_CCOMMAND
  argc = ccommand(&argv);
#endif

#ifdef MSDOS
  progname = "djpeg";		/* DOS tends to be too verbose about argv[0] */
#else
  progname = argv[0];
  if (progname == NULL || progname[0] == 0)
    progname = "djpeg";		/* in case C library doesn't provide it */
#endif

  /* The default maxmem must be computed only once at program startup,
   * since releasing memory with free() won't give it back to the OS.
   */
#ifdef FREE_MEM_ESTIMATE
  default_maxmem = FREE_MEM_ESTIMATE;
#else
  default_maxmem = 0;
#endif

  /* Scan command line, parse switches and locate input file names */

  if (argc < 2)
    usage();			/* nothing on the command line?? */

  file_index = 0;

  while (file_index < argc-1)
    file_index = process_one_file(argc, argv, file_index);

  /* All done. */
  exit(EXIT_SUCCESS);
  return 0;			/* suppress no-return-value warnings */
}
