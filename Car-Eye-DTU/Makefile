##########################################################################
# File:             Mailefile 
# Version:          0.1 
# Author:           Maobin
# DATE              NAME                        DESCRIPTION
# 2011-04-10        Maobin                      Initial Version 0.1
##########################################################################
# started
include $(PROJ_HOME)/build/option.mak
OBJDIR=./
MODULE_NAME = $(PARAM)
LIB_RESULE=$(MODULE_NAME).a

#Two mothods#
#First mothod#
S1 := $(wildcard *.c)
SOURCE:=$(S1)

#Second mothod#
#SOURCE := app_demo_adc.c \
#		   main.c

#头文件目录设置
LOCAL_INCLUDE := -I./

#正常情况下使用下面一句就可以编译，生成.a文件
include $(PROJ_HOME)/build/app_build.mak

#示例中只有一个文件，所以编译成main.o
#OBJECT := main.o

#all:$(OBJECT)

#$(OBJECT):$(SOURCE)
#	$(CC) -c $(CFLAGS) $(LOCAL_INCLUDE) -o $(LIB_DIR)/$@ $< 
#
#.PHONY:
#   clean:
#	rm -fr $(OBJECT) $(LIB_DIR)/$(LIB_RESULE)
