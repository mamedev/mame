###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
#   Copyright (c) Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

OBJ = obj
SRC = ..

#-fuse-ld=gold -Wpedantic -march=native -march=native

# LTO = -flto=4  -fuse-linker-plugin -flto-partition=balanced -Wodr

CDEFS = -DPSTANDALONE=1 -DPTR64=1
#-Werror
CFLAGS =  $(LTO) -g -O3 -std=c++98  -march=native -msse4.2 -Wall -Wpedantic -Wsign-compare -Wextra -Wno-long-long -Wno-unused-parameter -Wno-unused-result -Wno-variadic-macros -I..
LDFLAGS = $(LTO) -g -O3 -std=c++98 
#CFLAGS =  $(LTO) -g -O3 -std=c++11   -Wall -Wpedantic -Wsign-compare -Wextra -Isrc
#LDFLAGS = $(LTO) -g -O3 -std=c++11 	

CC = @g++-5
LD = @g++-5
MD = @mkdir
RM = @rm

TARGETS = nltool nlwav

NLOBJ = $(OBJ)
POBJ = $(OBJ)/plib

OBJDIRS = $(OBJ) \
		  $(OBJ)/analog \
		  $(OBJ)/solver \
		  $(OBJ)/devices \
		  $(OBJ)/plib \
		  $(OBJ)/devices \
		  $(OBJ)/macro \
		  $(OBJ)/tools \
		  $(OBJ)/prg \
		  

OBJS = $(POBJS) $(NLOBJS)

POBJS := \
	$(POBJ)/pstring.o \
	$(POBJ)/palloc.o \
	$(POBJ)/pparser.o \
	$(POBJ)/pstate.o \
	$(POBJ)/pstream.o \

NLOBJS := \
	$(NLOBJ)/nl_base.o \
	$(NLOBJ)/nl_parser.o \
	$(NLOBJ)/nl_setup.o \
	$(NLOBJ)/nl_factory.o \
	$(NLOBJ)/analog/nld_bjt.o \
	$(NLOBJ)/analog/nld_fourterm.o \
	$(NLOBJ)/analog/nld_switches.o \
	$(NLOBJ)/analog/nld_twoterm.o \
	$(NLOBJ)/analog/nld_opamps.o \
	$(NLOBJ)/devices/nld_4020.o \
	$(NLOBJ)/devices/nld_4066.o \
	$(NLOBJ)/devices/nld_7400.o \
	$(NLOBJ)/devices/nld_7402.o \
	$(NLOBJ)/devices/nld_7404.o \
	$(NLOBJ)/devices/nld_7408.o \
	$(NLOBJ)/devices/nld_7410.o \
	$(NLOBJ)/devices/nld_7411.o \
	$(NLOBJ)/devices/nld_7420.o \
	$(NLOBJ)/devices/nld_7425.o \
	$(NLOBJ)/devices/nld_7427.o \
	$(NLOBJ)/devices/nld_7430.o \
	$(NLOBJ)/devices/nld_7432.o \
	$(NLOBJ)/devices/nld_7437.o \
	$(NLOBJ)/devices/nld_7448.o \
	$(NLOBJ)/devices/nld_7450.o \
	$(NLOBJ)/devices/nld_7474.o \
	$(NLOBJ)/devices/nld_7483.o \
	$(NLOBJ)/devices/nld_7486.o \
	$(NLOBJ)/devices/nld_7490.o \
	$(NLOBJ)/devices/nld_7493.o \
	$(NLOBJ)/devices/nld_74107.o \
	$(NLOBJ)/devices/nld_74123.o \
	$(NLOBJ)/devices/nld_74153.o \
	$(NLOBJ)/devices/nld_74175.o \
	$(NLOBJ)/devices/nld_74192.o \
	$(NLOBJ)/devices/nld_74193.o \
	$(NLOBJ)/devices/nld_74279.o \
	$(NLOBJ)/devices/nld_74ls629.o \
	$(NLOBJ)/devices/nld_82S16.o \
	$(NLOBJ)/devices/nld_9310.o \
	$(NLOBJ)/devices/nld_9312.o \
	$(NLOBJ)/devices/nld_9316.o \
	$(NLOBJ)/devices/nld_mm5837.o \
	$(NLOBJ)/devices/nld_ne555.o \
	$(NLOBJ)/devices/nld_r2r_dac.o \
	$(NLOBJ)/devices/nld_legacy.o \
	$(NLOBJ)/devices/net_lib.o \
	$(NLOBJ)/devices/nld_log.o \
	$(NLOBJ)/devices/nld_system.o \
	$(NLOBJ)/devices/nld_truthtable.o \
	$(NLOBJ)/macro/nlm_cd4xxx.o \
	$(NLOBJ)/macro/nlm_opamp.o \
	$(NLOBJ)/macro/nlm_other.o \
	$(NLOBJ)/macro/nlm_ttl74xx.o \
	$(NLOBJ)/solver/nld_solver.o \
	$(NLOBJ)/tools/nl_convert.o \

all:	maketree $(TARGETS)

#-------------------------------------------------
# clean
#-------------------------------------------------

clean:
	$(RM) -rf $(OBJS) $(TARGETS)

#-------------------------------------------------
# nltool
#-------------------------------------------------

nltool: $(OBJ)/prg/nltool.o $(OBJS)
	@echo Linking $@...
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

nlwav: $(OBJ)/prg/nlwav.o $(OBJS)
	@echo Linking $@...
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

#-------------------------------------------------
# directories
#-------------------------------------------------

$(sort $(OBJDIRS)):
	$(MD) -p $@

maketree: $(sort $(OBJDIRS))


#-------------------------------------------------
# generic rules
#-------------------------------------------------

$(OBJ)/%.o: $(SRC)/%.c 
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(OBJ)/%.pp: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -E $< -o $@

$(OBJ)/%.s: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -S $< -o $@

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $^


