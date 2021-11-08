TOOLCHAIN_DIR:=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu

CC:=$(TOOLCHAIN_DIR)/bin/aarch64-linux-gnu-gcc
LD:=$(TOOLCHAIN_DIR)/bin/aarch64-linux-gnu-ld

CXX:=$(TOOLCHAIN_DIR)/bin/aarch64-linux-gnu-g++

CFLAGS+=-I$(TOOLCHAIN_DIR)/include -L$(TOOLCHAIN_DIR)/lib
CXXFLAGS+=-I$(TOOLCHAIN_DIR)/include -L$(TOOLCHAIN_DIR)/lib

LDFLAGS+=-lrga

CSRCS+=rga_test.c
CXXSRCS+=RkRga_test.cxx

OUTPUT:=demo

default:clean $(OUTPUT) scp
#default:clean test scp

$(OUTPUT):
	$(CC) $(CSRCS) $(CFLAGS) $(LDFLAGS) -o $@

test:clean
	$(CXX) $(CXXSRCS) $(CFLAGS) $(LDFLAGS) -o $@
	scp test root@192.168.100.100:/root
	
.PHONY:scp
scp:
	scp $(OUTPUT) root@192.168.100.100:/root
	scp ./librga.so root@192.168.100.100:/root

.PHONY:clean
clean:
	rm -f $(OUTPUT)
	rm -f test

