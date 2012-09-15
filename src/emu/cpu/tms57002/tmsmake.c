#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include "osdcomm.h"

enum {
  I_POST  = 0x00000001,
  I_REP   = 0x00000002,
  I_CMODE = 0x00000004,
  I_DMODE = 0x00000008,
  I_SFAI  = 0x00000010,
  I_CRM   = 0x00000020,
  I_DBP   = 0x00000040,
  I_SFAO  = 0x00000080,
  I_SFMO  = 0x00000100,
  I_RND   = 0x00000200,
  I_MOVM  = 0x00000400,
  I_SFMA  = 0x00000800
};

enum {
  T_STD,
  T_BRANCH,
  T_CBRANCH,
  T_IDLE,
  T_FLAGS
};

enum { PA, PC, PD, PD24, PI, PML, PMO, PMV, PB, PWA, PWC, PWD, SFAI };

struct instr {
  char *name;
  char *dasm;
  char *run;
  int line, cycles, type, baseval, variants;
  unsigned int flags;
};

struct pdesc {
  const char *opt;
  int pcount, id;
};

struct pinf {
  const char *start;
  int size;
  int id;
  int pcount;
  int ppos[4];
};

static const pdesc pp_r[] = {
  { "a",    0, PA   },
  { "c",    0, PC   },
  { "d",    0, PD   },
  { "d24",  0, PD24 },
  { "i",    0, PI   },
  { "ml",   0, PML  },
  { "mo",   0, PMO  },
  { "mv",   0, PMV  },
  { "wa",   1, PWA  },
  { "wc",   1, PWC  },
  { "wd",   1, PWD  },
  { "b",    1, PB   },
  { "sfai", 2, SFAI },
  { 0 }
};

static instr cat1[0x40], cat2[0x80], cat3[0x80];

static pinf parse_res[4096];
static int parse_count;

struct vinfo {
  unsigned int mask;
  int variants;
  const char *name;
  const char *getter;
};

enum { IxCMODE, IxDMODE, IxSFAI, IxCRM, IxDBP, IxSFAO, IxSFMO, IxRND, IxMOVM, IxSFMA, IxCOUNT };

static const vinfo vinf[] = {
  { I_CMODE, 3, "cmode", "xmode(opcode, 'c')" },
  { I_DMODE, 3, "dmode", "xmode(opcode, 'd')" },
  { I_SFAI,  2, "sfai",  "sfai(st1)"       },
  { I_CRM,   4, "crm",   "crm(st1)"        },
  { I_DBP,   2, "dbp",   "dbp(st1)"        },
  { I_SFAO,  2, "sfao",  "sfao(st1)"       },
  { I_SFMO,  4, "sfmo",  "sfmo(st1)"       },
  { I_RND,   8, "rnd",   "rnd(st1)"        },
  { I_MOVM,  2, "movm",  "movm(st1)"       },
  { I_SFMA,  4, "sfma",  "sfma(st1)"       },
  { 0 }
};

static char *xstrdup(const char *str)
{
  char *cpy = NULL;
  if (str != NULL) {
    cpy = (char *)malloc(strlen(str) + 1);
    if (cpy != NULL)
      strcpy(cpy, str);
  }
  return cpy;
}

static char *sconcat(char *dest, const char *src)
{
  char *r;
  int len = strlen(src);
  int pos = dest ? strlen(dest) : 0;
  r = (char *)realloc(dest, pos+len+2);
  memcpy(r + pos, src, len);
  r[pos+len] = '\n';
  r[pos+len+1] = 0;
  return r;
}

static void next(char **q)
{
  char *p = *q;
  while(*p && *p != ' ')
    p++;
  while(*p == ' ')
    *p++ = 0;
  *q = p;
}

static void load(const char *fname)
{
  char buf[4096];
  FILE *f;
  instr *i = 0;
  int line = 0;

  sprintf(buf, "Error opening %s for reading\n", fname);
  f = fopen(fname, "r");
  if(!f) {
    perror(buf);
    exit(1);
  }

  while(fgets(buf, sizeof(buf), f)) {
    char *p = buf;

    while(*p && *p != '\r' && *p != '\n')
      p++;
    *p = 0;

    line++;

    if(!buf[0]) {
      i = 0;
      continue;
    }
    if(buf[0] == ' ' || buf[0] == '\t') {
      if(!i) {
	fprintf(stderr, "%s:%d: Text line without an instruction.\n", fname, line);
	exit(1);
      }
      if(i->dasm)
	i->run = sconcat(i->run, buf);
      else {
	p = buf;
	while(*p == ' ' || *p == '\t')
	  p++;
	i->dasm = xstrdup(p);
      }
    } else {
      char *name=0, *cat=0, *id=0, *cyc=0, *rep=0, *type=0;
      p = buf;
      int pid, limit;
      instr *bi;
      unsigned int flags;

      name = p;
      next(&p);
      cat = p;
      next(&p);
      id = p;
      next(&p);
      cyc = p;
      next(&p);
      rep = p;
      next(&p);
      type = p;
      next(&p);
      if(*p) {
	fprintf(stderr, "%s:%d: Extra text at end of instruction description.\n", fname, line);
	exit(1);
      }
      if(!rep[0]) {
	fprintf(stderr, "%s:%d: Missing columns in instruction description.\n", fname, line);
	exit(1);
      }

      flags = 0;
      pid = strtol(id, 0, 16);
      if(!strcmp(cat, "1")) {
	bi = cat1;
	limit = 0x40;
      } else if(!strcmp(cat, "2a")) {
	bi = cat2;
	limit = 0x80;
      } else if(!strcmp(cat, "2b")) {
	bi = cat2;
	limit = 0x80;
	flags = I_POST;
      } else if(!strcmp(cat, "3")) {
	bi = cat3;
	limit = 0x80;
      } else {
	fprintf(stderr, "%s:%d: Unknown category '%s'.\n", fname, line, cat);
	exit(1);
      }

      if(pid<0 || pid >= limit) {
	fprintf(stderr, "%s:%d: Index %s out of range for category %s.\n", fname, line, id, cat);
	exit(1);
      }

      i = bi + pid;
      if(i->name) {
	fprintf(stderr, "%s:%d: Conflict with %s instruction line %d.\n", fname, line, i->name, i->line);
	exit(1);
      }

      if(!strcmp(rep, "y"))
	flags |= I_REP;
      else if(strcmp(rep, "n")) {
	fprintf(stderr, "%s:%d: Unknown repetition mode '%s'.\n", fname, line, rep);
	exit(1);
      }

      i->name = xstrdup(name);
      i->line = line;
      i->cycles = strtol(cyc, 0, 10);
      i->flags = flags;

      if(type[0] == 0)
	i->type = T_STD;
      else if(!strcmp(type, "b"))
	i->type = T_BRANCH;
      else if(!strcmp(type, "cb"))
	i->type = T_CBRANCH;
      else if(!strcmp(type, "i"))
	i->type = T_IDLE;
      else if(!strcmp(type, "f"))
	i->type = T_FLAGS;
      else {
	fprintf(stderr, "%s:%d: Unknown type '%s'.\n", fname, line, type);
	exit(1);
      }
    }
  }
  fclose(f);
}

static void pstr(const char *s, const char *e)
{
  parse_res[parse_count].start = s;
  parse_res[parse_count].size = e-s;
  parse_res[parse_count].id = -1;
  parse_res[parse_count].pcount = 0;
  parse_count++;
}

static void parse_1(const char *start, const pdesc *inf, const char **str, int emode)
{
  const char *p = *str;
  const char *s = p;
  int depth = 0;
  while(*p) {
    char c = *p++;
    if(!depth && ((c == ',' && emode == ',') || (c == ')' && emode == ')'))) {
      if(s != p-1)
	pstr(s, p-1);
      *str = p;
      return;
    }
    if(c == '(')
      depth++;
    else if(c == ')') {
      depth--;
      if(depth < 0) {
	if(emode == ',')
	  fprintf(stderr, "Parse error, missing parameter in '%s'.\n", start);
	else
	  fprintf(stderr, "Parse error, unbalanced parenthesis in '%s'.\n", start);
	exit(1);
      }
    } else if(c == '%' && ((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9'))) {
      char buf[4096];
      char *pp = buf;
      int i, j;
      int pos;

      if(s != p-1)
	pstr(s, p-1);

      while((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9'))
	*pp++ = *p++;
      *pp = 0;
      for(i=0; inf[i].opt && strcmp(buf, inf[i].opt); i++);
      if(!inf[i].opt) {
	fprintf(stderr, "Parse error, unhandled parameter %%%s\n", buf);
	exit(1);
      }

      pos = parse_count++;
      parse_res[pos].start = 0;
      parse_res[pos].size = 0;
      parse_res[pos].id = inf[i].id;
      parse_res[pos].pcount = inf[i].pcount;

      if(inf[i].pcount) {
	if(*p != '(') {
	  fprintf(stderr, "Parse error, missing opening parenthesis on %%%s\n", buf);
	  exit(1);
	} else
	  p++;
      }

      *str = p;

      for(j=0; j != inf[i].pcount; j++) {
	parse_res[pos].ppos[j] = parse_count;
	parse_1(start, inf, str, j == inf[i].pcount-1 ? ')' : ',');
      }
      p = *str;
      parse_res[pos].ppos[j] = parse_count;
      s = p;
    }
  }
  if(s != p)
    pstr(s, p);
  *str = p;
  switch(emode) {
  case 0:
    return;
  case ',':
    fprintf(stderr, "Parse error, missing parameter at end of string in '%s'.\n", start);
    exit(1);
  case ')':
    fprintf(stderr, "Parse error, missing closing parenthesis at end of string in '%s'.\n", start);
    exit(1);
  }
}

static void parse(const char *str, const pdesc *inf)
{
  parse_count = 0;
  parse_1(str, inf, &str, 0);
}

static void compute_cache_ids_cat(instr *il, int count, int *baseval)
{
  int i, j;
  for(i=0; i != count; i++)
    if(il[i].run) {
      unsigned int flags = il[i].flags;
      int variants = 1;
      parse(il[i].run, pp_r);
      for(j=0; j != parse_count; j++) {
	switch(parse_res[j].id) {
	case -1:   break;
	case PA:   flags |= I_SFAO; break;
	case PC:   flags |= I_CMODE|I_CRM; break;
	case PD:   flags |= I_DMODE|I_DBP; break;
	case PI:   break;
	case PD24: flags |= I_DMODE|I_DBP; break;
	case PML:  flags |= I_SFMA; break;
	case PMO:  flags |= I_SFMO|I_RND|I_MOVM; break;
	case PMV:  flags |= I_SFMO|I_MOVM; break;
	case PB:   break;
	case PWA:  break;
	case PWC:  flags |= I_CMODE; break;
	case PWD:  flags |= I_DMODE|I_DBP; break;
	case SFAI: flags |= I_SFAI; break;
	}
      }
      il[i].flags = flags;
      il[i].baseval = *baseval;

      for(j=0; vinf[j].mask; j++)
	if(flags & vinf[j].mask)
	  variants *= vinf[j].variants;
      il[i].variants = variants;

      *baseval += variants;
    }
}


static void scp(char **str, int idx)
{
  memcpy(*str, parse_res[idx].start, parse_res[idx].size);
  (*str) += parse_res[idx].size;
}

static void scs(char **str, const char *s)
{
  int len = strlen(s);
  memcpy(*str, s, len);
  (*str) += len;
}

static void save_dasm_cat(FILE *f, const char *def, instr *il, int count)
{
  static const pdesc pp[] = {
    { "c", 0, PC },
    { "d", 0, PD },
    { "i", 0, PI },
    { 0 }
  };

  int i;
  fprintf(f, "#ifdef %s\n", def);
  for(i=0; i != count; i++)
    if(il[i].name) {
      int par[3];
      int pc = 0;
      int j;
      char buf[4096], *p = buf;
      parse(il[i].dasm, pp);
      for(j=0; j<parse_count;j++) {
	switch(parse_res[j].id) {
	case -1:
	  scp(&p, j);
	  break;
	case PC:
	  scs(&p, "%s");
	  par[pc++] = PC;
	  break;
	case PD:
	  scs(&p, "%s");
	  par[pc++] = PD;
	  break;
	case PI:
	  scs(&p, "%02x");
	  par[pc++] = PI;
	  break;
	}
      }
      *p = 0;

      fprintf(f, "    case 0x%02x:\n", i);
      fprintf(f, "      sprintf(buf, \"%s\"", buf);
      for(j=0; j != pc; j++)
	switch(par[j]) {
	case PC:
	  fprintf(f, ", get_memadr(opcode, 'c')");
	  break;
	case PD:
	  fprintf(f, ", get_memadr(opcode, 'd')");
	  break;
	case PI:
	  fprintf(f, ", opcode & 0xff");
	  break;
	}


      fprintf(f, ");\n");
      fprintf(f, "      break;\n");
      fprintf(f, "\n");
    }

  fprintf(f, "#endif\n\n");
}

static void intrp_expand(char **p, int s, int e)
{
  int i;
  for(i=s; i<e;) {
    switch(parse_res[i].id) {
    case -1:
      scp(p, i);
      break;

    case PA:
      scs(p, "aacc_to_output()");
      break;

    case PC:
      scs(p, "opc_read_c(opcode)");
      break;

    case PD:
      scs(p, "(opc_read_d(opcode) << 8)");
      break;

    case PI:
      scs(p, "(opcode & 0xff)");
      break;

    case PD24:
      scs(p, "opc_read_d(opcode)");
      break;

    case PML:
      scs(p, "macc_to_loop()");
      break;

    case PMO:
      scs(p, "macc_to_output()");
      break;

    case PMV:
      scs(p, "check_macc_overflow()");
      break;

    case PB:
      scs(p, "pc = ");
      intrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1]);
      scs(p, ";\n");
      scs(p, "  sti |= S_BRANCH");
      break;

    case PWA:
      scs(p, "r = ");
      intrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1]);
      scs(p, ";\n");
      scs(p, "  if(r < -0x80000000 || r > 0x7fffffff)\n");
      scs(p, "    st1 |= ST1_AOV;\n");
      scs(p, "  aacc = r");
      break;

    case PWC:
      scs(p, "opc_write_c(opcode, ");
      intrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1]);
      scs(p, ")");
      break;

    case PWD:
      scs(p, "opc_write_d(opcode, ");
      intrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1]);
      scs(p, ")");
      break;

    case SFAI:
      intrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1]);
      scs(p, " = ");
      intrp_expand(p, parse_res[i].ppos[1], parse_res[i].ppos[2]);
      scs(p, ";\n");
      scs(p, "  if(st1 & ST1_SFAI)\n");
      scs(p, "    ");
      intrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1]);
      scs(p, " = ((INT32)");
      intrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1]);
      scs(p, ") >> 1");
      break;
    }

    if(parse_res[i].id == -1)
      i++;
    else
      i = parse_res[i].ppos[parse_res[i].pcount];
  }
}

static void save_intrp_cat(FILE *f, const char *def, instr *il, int count, unsigned int fmask, unsigned int fval)
{
  int i;
  fprintf(f, "#ifdef %s\n", def);
  for(i=0; i != count; i++)
    if(il[i].run) {
      fprintf(f, "case 0x%02x: // %s\n", i, il[i].name);
      if((il[i].flags & fmask) == fval) {
	char buf[4096], *p = buf;
	parse(il[i].run, pp_r);
	intrp_expand(&p, 0, parse_count);
	*p = 0;

	fprintf(f, "%s", buf);
      }
      fprintf(f, "  break;\n\n");
    }

  fprintf(f, "#endif\n\n");
}

static void save_cdec_cat(FILE *f, const char *def, instr *il, int count, unsigned int fmask, unsigned int fval)
{
  int i;
  fprintf(f, "#ifdef %s\n", def);
  for(i=0; i != count; i++)
    if(il[i].run) {
      int m=1, j;
      unsigned int flags = il[i].flags;
      fprintf(f, "case 0x%02x: // %s\n", i, il[i].name);
      if((il[i].flags & fmask) == fval) {
	fprintf(f, "  *op = %d", il[i].baseval);
	for(j=0; vinf[j].mask; j++)
	  if(flags & vinf[j].mask) {
	    if(m != 1)
	      fprintf(f, " + %d*%s", m, vinf[j].getter);
	    else
	      fprintf(f, " + %s", vinf[j].getter);
	    m *= vinf[j].variants;
	  }
	fprintf(f, ";\n");
	switch(il[i].type) {
	case T_FLAGS:
	  fprintf(f, "%s", il[i].run);
	  break;
	case T_BRANCH:
	  fprintf(f, "  cs->branch = BR_UB;\n");
	  break;
	case T_CBRANCH:
	  fprintf(f, "  cs->branch = BR_CB;\n");
	  break;
	case T_IDLE:
	  fprintf(f, "  cs->branch = BR_IDLE;\n");
	  break;
	}
      }
      fprintf(f, "  break;\n\n");
    }

  fprintf(f, "#endif\n\n");
}


static void cintrp_expand(char **p, int s, int e, const int *cv)
{
  int i;
  for(i=s; i<e;) {
    switch(parse_res[i].id) {
    case -1:
      scp(p, i);
      break;

    case PA:
      if(cv[IxSFAO])
	scs(p, "(aacc << 7)");
      else
	scs(p, "aacc");
      break;

    case PC: {
      const char *r = NULL;
      if(cv[IxCMODE] == 0)
	r = "cmem[i->param]";
      else if(cv[IxCMODE] == 1)
	r = "cmem[ca]";
      else if(cv[IxCMODE] == 2)
	r = "cmem[ca++]";
      else
	abort();

      if(cv[IxCRM] == 0 || cv[IxCRM] == 3)
	scs(p, r);
      else if(cv[IxCRM] == 1) {
	scs(p, "(");
	scs(p, r);
	scs(p, " & 0xffff0000)");
      } else if(cv[IxCRM] == 2) {
	scs(p, "(");
	scs(p, r);
	scs(p, " << 16)");
      } else
	abort();
      break;
    }

    case PD:
      if(cv[IxDMODE] == 0)
	if(cv[IxDBP])
	  scs(p, "(dmem1[(i->param + ba1) & 0x1f] << 8)");
	else
	  scs(p, "(dmem0[(i->param + ba0) & 0xff] << 8)");
      else if(cv[IxDMODE] == 1)
	if(cv[IxDBP])
	  scs(p, "(dmem1[(id + ba1) & 0x1f] << 8)");
	else
	  scs(p, "(dmem0[(id + ba0) & 0xff] << 8)");
      else if(cv[IxDMODE] == 2)
	if(cv[IxDBP])
	  scs(p, "(dmem1[((id++) + ba1) & 0x1f] << 8)");
	else
	  scs(p, "(dmem0[((id++) + ba0) & 0xff] << 8)");
      else
	abort();
      break;

    case PI:
      scs(p, "i->param");
      break;

    case PD24:
      if(cv[IxDMODE] == 0)
	if(cv[IxDBP])
	  scs(p, "dmem1[(i->param + ba1) & 0x1f]");
	else
	  scs(p, "dmem0[(i->param + ba0) & 0xff]");
      else if(cv[IxDMODE] == 1)
	if(cv[IxDBP])
	  scs(p, "dmem1[(id + ba1) & 0x1f]");
	else
	  scs(p, "dmem0[(id + ba0) & 0xff]");
      else if(cv[IxDMODE] == 2)
	if(cv[IxDBP])
	  scs(p, "dmem1[((id++) + ba1) & 0x1f]");
	else
	  scs(p, "dmem0[((id++) + ba0) & 0xff]");
      else
	abort();
      break;

    case PML:
      if(cv[IxSFMA] == 0)
	scs(p, "macc");
      else if(cv[IxSFMA] == 1)
	scs(p, "(macc << 2)");
      else if(cv[IxSFMA] == 2)
	scs(p, "(macc << 4)");
      else if(cv[IxSFMA] == 3)
	scs(p, "(macc >> 16)");
      else
	abort();
      break;

    case PMO: {
      static const long long rounding[8] = {
	0,
	1LL << (48-32-1),
	1LL << (48-24-1),
	1LL << (48-30-1),
	1LL << (48-16-1),
	0, 0, 0
      };
      static const unsigned long long rmask[8] = {
	~0ULL,
	(~0ULL) << (48-32),
	(~0ULL) << (48-24),
	(~0ULL) << (48-30),
	(~0ULL) << (48-16),
	~0ULL, ~0ULL, ~0ULL
      };

      char r[256];
      sprintf(r, "macc_to_output_%d%s(0x%016" I64FMT "xULL, 0x%016" I64FMT "xULL)", cv[IxSFMO], cv[IxMOVM] ? "s" : "", rounding[cv[IxRND]], rmask[cv[IxRND]]);
      scs(p, r);
      break;
    }

    case PMV: {
      char r[256];
      sprintf(r, "check_macc_overflow_%d%s()", cv[IxSFMO], cv[IxMOVM] ? "s" : "");
      scs(p, r);
      break;
    }

    case PB:
      scs(p, "pc = ");
      cintrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1], cv);
      scs(p, ";\n");
      scs(p, "  sti |= S_BRANCH");
      break;

    case PWA:
      scs(p, "r = ");
      cintrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1], cv);
      scs(p, ";\n");
      scs(p, "  if(r < -0x80000000 || r > 0x7fffffff)\n");
      scs(p, "    st1 |= ST1_AOV;\n");
      scs(p, "  aacc = r");
      break;

    case PWC:
      if(cv[IxCMODE] == 0)
	scs(p, "cmem[i->param] = ");
      else if(cv[IxCMODE] == 1)
	scs(p, "cmem[ca] = ");
      else if(cv[IxCMODE] == 2)
	scs(p, "cmem[ca++] = ");
      else
	abort();

      cintrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1], cv);
      break;

    case PWD:
      if(cv[IxDMODE] == 0)
	if(cv[IxDBP])
	  scs(p, "dmem1[(i->param + ba1) & 0x1f] = ");
	else
	  scs(p, "dmem0[(i->param + ba0) & 0xff] = ");
      else if(cv[IxDMODE] == 1)
	if(cv[IxDBP])
	  scs(p, "dmem1[(id + ba1) & 0x1f] = ");
	else
	  scs(p, "dmem0[(id + ba0) & 0xff] = ");
      else if(cv[IxDMODE] == 2)
	if(cv[IxDBP])
	  scs(p, "dmem1[((id++) + ba1) & 0x1f] = ");
	else
	  scs(p, "dmem0[((id++) + ba0) & 0xff] = ");
      else
	abort();

      cintrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1], cv);
      break;

    case SFAI:
      cintrp_expand(p, parse_res[i].ppos[0], parse_res[i].ppos[1], cv);
      scs(p, " = ");
      if(cv[IxSFAI])
	scs(p, "((INT32)(");
      cintrp_expand(p, parse_res[i].ppos[1], parse_res[i].ppos[2], cv);
      if(cv[IxSFAI])
	scs(p, ")) >> 1");
      break;
    }

    if(parse_res[i].id == -1)
      i++;
    else
      i = parse_res[i].ppos[parse_res[i].pcount];
  }
}

static void save_cintrp_cat(FILE *f, instr *il, int count)
{
  int i;
  for(i=0; i != count; i++)
    if(il[i].run) {
      int cv[IxCOUNT];
      int j, k = 0;
      unsigned int flags = il[i].flags;
      char buf[16384];

      memset(cv, 0, sizeof(cv));
      parse(il[i].run, pp_r);

      for(j=0; j != il[i].variants; j++) {
	char *p = buf;
	fprintf(f, "case %d: // %s", il[i].baseval + j, il[i].name);
	for(k=0; k != IxCOUNT; k++)
	  if(flags & vinf[k].mask)
	    fprintf(f, " %s=%d", vinf[k].name, cv[k]);
	fprintf(f, "\n");
	cintrp_expand(&p, 0, parse_count, cv);
	*p = 0;
	fprintf(f, "%s", buf);
	fprintf(f, "  break;\n\n");

	for(k=0; k != IxCOUNT; k++)
	  if(flags & vinf[k].mask) {
	    cv[k]++;
	    if(cv[k] != vinf[k].variants)
	      break;
	    cv[k] = 0;
	  }
      }
      assert(k == IxCOUNT);
    }
}

static void compute_cache_ids(void)
{
  int baseval = 1;
  compute_cache_ids_cat(cat1, 0x40, &baseval);
  compute_cache_ids_cat(cat2, 0x80, &baseval);
  compute_cache_ids_cat(cat3, 0x80, &baseval);
}

static void save_dasm(FILE *f)
{
  save_dasm_cat(f, "DASM1", cat1, 0x40);
  save_dasm_cat(f, "DASM2", cat2, 0x80);
  save_dasm_cat(f, "DASM3", cat3, 0x80);
}

void save_intrp(FILE *f)
{
  save_intrp_cat(f, "INTRP1",  cat1, 0x40, 0,      0);
  save_intrp_cat(f, "INTRP2A", cat2, 0x80, I_POST, 0);
  save_intrp_cat(f, "INTRP2B", cat2, 0x80, I_POST, I_POST);
  save_intrp_cat(f, "INTRP3",  cat3, 0x80, 0,      0);
}

static void save_cdec(FILE *f)
{
  save_cdec_cat(f, "CDEC1",  cat1, 0x40, 0,      0);
  save_cdec_cat(f, "CDEC2A", cat2, 0x80, I_POST, 0);
  save_cdec_cat(f, "CDEC2B", cat2, 0x80, I_POST, I_POST);
  save_cdec_cat(f, "CDEC3",  cat3, 0x80, 0,      0);
}

static void save_cintrp(FILE *f)
{
  fprintf(f, "#ifdef CINTRP\n");
  save_cintrp_cat(f, cat1, 0x40);
  save_cintrp_cat(f, cat2, 0x80);
  save_cintrp_cat(f, cat3, 0x80);
  fprintf(f, "#endif\n");
}

static void save(const char *fname)
{
  char buf[4096];
  FILE *f;

  sprintf(buf, "Error opening %s for writing\n", fname);
  f = fopen(fname, "w");
  if(!f) {
    perror(buf);
    exit(1);
  }

  compute_cache_ids();

  save_dasm(f);
  //  save_intrp(f);
  save_cdec(f);
  save_cintrp(f);

  fclose(f);
}

static void clear_cat(instr *il, int count)
{
  int i;
  for(i=0; i != count; i++) {
    free(il[i].name);
    free(il[i].dasm);
    free(il[i].run);
  }
}

static void clear(void)
{
  clear_cat(cat1, 0x40);
  clear_cat(cat2, 0x80);
  clear_cat(cat3, 0x80);
}

int main(int argc, char *argv[])
{
  if(argc != 3) {
    fprintf(stderr, "Usage:\n%s tmsinstr.lst tms57002.inc\n", argv[0]);
    exit(1);
  }

  memset(cat1, 0, sizeof(cat1));
  memset(cat2, 0, sizeof(cat2));
  memset(cat3, 0, sizeof(cat3));

  load(argv[1]);
  save(argv[2]);
  clear();

  return 0;
}
