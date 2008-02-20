
STAMPDIR = stamps
CONFDIR = configs

TARGET = mame
SUBTARGET = mame

MAME_TOOLS = chdman src2html jedutil regrep srcclean makemeta romcmp 

PATH_SEP = /
PWD = $(shell pwd)
ECHO = echo
TOUCH = touch
MKDIRP = mkdir -p
RMF = rm -f
MVF = mv -f

COMMA := ,
EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)

PRINTHELP = @$(ECHO) $(CONFIG_NAME):

CONFIG_NAME = $(HOST_OS)-$(TARGET_OS)-$(TARGET_OSD)
