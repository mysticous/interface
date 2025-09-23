# All rights reserved by HiRain Technologies.
# Auto generated, Do not edit.
# 
# Makefile for libtb2820.so.2.1.1
#

CFLAGS  +=  -O1 -DNDEBUG -Werror -Wall
LIBS    :=  -fPIC -shared

CC = gcc 

HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)
SOURCES += $(wildcard *.cpp)

libtb2820.so.2.1.1: $(SOURCES)
	$(CC) $(CFLAGS) -o $@  $(LIBS) $^
	@ln -s libtb2820.so.2.1.1 libtb2820.so.2
	@ln -s libtb2820.so.2 libtb2820.so
        
libtb2820-virt.so.2.1.1: $(SOURCES)
	$(CC) $(CFLAGS) -DTB_DEVICE_INTERFACE_VIRTUAL -o $@  $(LIBS) $^
	@ln -s libtb2820-virt.so.2.1.1 libtb2820-virt.so.2
	@ln -s libtb2820-virt.so.2 libtb2820-virt.so

libtb2820-dbg.so.2.1.1: $(SOURCES)
	$(CC) $(CFLAGS) -DTB_DEVICE_INTERFACE_DEBUG -o $@  $(LIBS) $^
	@ln -s libtb2820-dbg.so.2.1.1 libtb2820-dbg.so.2
	@ln -s libtb2820-dbg.so.2 libtb2820-dbg.so

.c.o: 
	$(CC) -c $(LIBS) $^

.PHONY : clean all install install64 compdb

all: libtb2820.so.2.1.1 libtb2820-dbg.so.2.1.1 libtb2820-virt.so.2.1.1

install64:
	cp -af libtb2820.so /usr/lib64
	cp -af libtb2820.so.* /usr/lib64
	cp -af $(HEADERS) /usr/include

install:
	cp -af libtb2820.so /usr/lib
	cp -af libtb2820.so.* /usr/lib
	cp -af $(HEADERS) /usr/include

clean:
	rm -f *.o *.so *.so.*

compdb:              ## 生成 compile_commands.json
	@echo ">> collecting compile commands ..."
	rm -f compile_commands.json
	bear  -- make clean all

