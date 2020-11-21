N64KITDIR	= c:/nintendo/n64kit

TARGET = 64noid
CODEFILES = main.c pad.c save.c render.c text.c sprite.c \
hash.c file.c game.c map.c mapdata.c mapsel.c nextmap.c stageend.c \
title.c \

OTHERFILES = 

############################################################################
# DO NOT CHANGE ANYTHING BELOW THIS LINE UNLESS YOU KNOW WHAT YOU'RE DOING #
############################################################################

include $(ROOT)/usr/include/make/PRdefs

DEBUGSYM	= -g
OPTIMIZER	= -O1

NUSYSDIR = $(N64KITDIR)/nusys
NUSYSINC = $(NUSYSDIR)/include
NUSYSLIB = $(NUSYSDIR)/lib
NUSTDINC = $(N64KITDIR)/nustd/include
NUSTDLIB = $(N64KITDIR)/nustd/lib

NUOBJ = $(NUSYSLIB)/nusys.o
CODEOBJECTS	= $(CODEFILES:.c=.o) $(NUOBJ)
OTHEROBJECTS = $(OTHERFILES:.c=.o)

CUSTFLAGS = 
LCDEFS = -DF3DEX_GBI_2
LCINCS = -I$(NUSYSINC) -I$(NUSTDINC)
LCOPTS = -G 0 $(DEBUGSYM) $(CUSTFLAGS)
LDFLAGS = -L$(ROOT)/usr/lib -L$(ROOT)/usr/lib/PR -L$(NUSYSLIB) -L$(NUSTDLIB) -lnusys_d -lnustd_d -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc

CODESEGMENT	= codesegment.o
OBJECTS	= $(CODESEGMENT) $(OTHEROBJECTS)

SYMBOL = $(TARGET).out
ROM	= $(TARGET).n64
FILE_PACKER = tools\bin\buildfilepack.exe
FILE_PACK = file_pack.bin
FILE_DIR = files

default: $(ROM)

$(FILE_PACK): FORCE
	$(FILE_PACKER) $(FILE_DIR) $(FILE_PACK)
	
FORCE:
	
$(CODESEGMENT):	$(CODEOBJECTS)
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(ROM): $(OBJECTS) $(FILE_PACK)
	$(MAKEROM) spec -I$(NUSYSINC) -r $(ROM) -e $(SYMBOL)
	makemask $(ROM)
	
include $(ROOT)/usr/include/make/commonrules
# This space is needed or makefile errors