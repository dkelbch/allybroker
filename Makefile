################################################################################
#
#	Copyright 2009 DOSCH & AMAND Research GmbH&Co.KG
#
#	Author	:	Ralf Stein, Dirk Kelbch
#	Date	:	09-Feb-2009
#
#	History
#	----------------------------------------------------------------------------
#
#################################################################################

.KEEP_STATE:
.SUFFIXES:
.SUFFIXES: .c .cpp .h .hpp .asm .s .inc .o .elf .d .dep .def .lex .y .ypp

libraries :=
symfiles  :=
objects   :=
linkpath  :=

APPNAME := allyBroker

ifeq ($(OS),Windows_NT)
HOST_OS = WIN32
RM := del /Q
SL := \$(EMPTY)
else
HOST_OS = UNIX
RM := rm -f
SL := /
endif

################################################################################
#
# Version control
#
HAPPL_VERSION:=0x0100
HAPPL_BUILD:=001

################################################################################
#
# Directory
#
# define output,intermediate, object, project and base directory
#
BASE :=.
PROJDIR := $(BASE)$(SL)src
MQTTBROKER_LIB := $(PROJDIR)$(SL)PicoMQTT
VMFRAME_LIB := $(PROJDIR)$(SL)vmframe

OUTDIR := $(BASE)$(SL)out
OBJDIR := $(BASE)$(SL)obj
LIBDIR := $(PROJDIR)$(SL)lib
################################################################################
#
# DNA C-Option definition
#
#
#
coptions := -DAPPL_VERSION=$(HAPPL_VERSION) -DAPPL_BUILD=$(HAPPL_BUILD) -DMAC_OS

################################################################################
#
# DNA include
#
#
includes := -I. -I$(PROJDIR)$(SL)include 

includes += -I $(PROJDIR)
################################################################################

objects := $(OBJDIR)$(SL)main.o

vpath %.c $(PROJDIR)

##########
# frame
##########
includes += -I$(PROJDIR)$(SL)vmframe

objects += $(OBJDIR)$(SL)vmsrv.o
objects += $(OBJDIR)$(SL)linklist.o
objects += $(OBJDIR)$(SL)vmdebug.o


vpath %.c $(PROJDIR)$(SL)vmframe

##########
# mqtt
##########
includes += -I$(PROJDIR)$(SL)mqtt

objects += $(OBJDIR)$(SL)mqttBroker.o
objects += $(OBJDIR)$(SL)mqtt.o
objects += $(OBJDIR)$(SL)mqttTract.o
objects += $(OBJDIR)$(SL)mqttTopic.o

vpath %.c $(PROJDIR)$(SL)mqtt

#vpath %.c $(MQTTBROKER_LIB)
#vpath %.cpp $(MQTTBROKER_LIB)

#ifeq ($(HOST_OS),WIN32)
#CMBS_LIBNAME := cmbs_host_win32.lib
#CMBS_LIB := $(LIBDIR)$(SL)$(CMBS_LIBNAME)
#else
#CMBS_LIBNAME := cmbs_host_lnx
#CMBS_LIB := $(LIBDIR)/lib$(CMBS_LIBNAME).a
#linkpath += -L$(LIBDIR)
#endif

################################################################################
# host cmbs application
#include $(BASE)$(SL)appcmbs$(SL)happcmbs.mak

################################################################################
# framework
#ifeq ($(HOST_OS),WIN32)
#include $(PROJDIR)$(SL)frame$(SL)win32$(SL)frame.mak
#else
#include $(PROJDIR)/frame/linux/frame.mak
#endif

################################################################################
# rules

ifeq ($(HOST_OS),WIN32)
include  win32.mak
else
include  linux.mak
endif

all:  dirs $(OUTDIR)/$(APPNAME)

dirs: $(OBJDIR) $(OUTDIR) $(LIBDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OUTDIR):
	mkdir $(OUTDIR)

$(LIBDIR):
	mkdir $(LIBDIR)

#$(CMBS_LIB):	$(cfr_objects) $(cmbs_objects)
#	$(ARCHIEVE) $(cfr_objects) $(cmbs_objects)

clean:
	$(RM) $(OBJDIR)$(SL)*
ifeq ($(HOST_OS),WIN32)
	$(RM) $(OUTDIR)$(SL)$(APPNAME).exe
else
	$(RM) $(OUTDIR)$(SL)$(APPNAME)
endif

$(OUTDIR)/$(APPNAME): $(CMBS_LIB) $(objects) 
ifeq ($(HOST_OS),WIN32)
	$(LINK)  $(LFLAGS) $(objects) $(linkpath)
else
	$(LINK)  $(objects) $(LFLAGS) $(linkpath)
endif


################################################################################
#
# END
