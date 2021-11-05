CC:=/opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
LD:=/opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-ld

CXX:=/opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-g++

CFLAGS+=-I/opt/aarch64-linux-gnu/include -L/opt/aarch64-linux-gnu/lib
CXXFLAGS+=-I/opt/aarch64-linux-gnu/include -L/opt/aarch64-linux-gnu/lib

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
