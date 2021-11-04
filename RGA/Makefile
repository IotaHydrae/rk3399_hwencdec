CC:=/opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
LD:=/opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-ld

CFLAGS+=-I/opt/aarch64-linux-gnu/include -L/opt/aarch64-linux-gnu/lib
CXXFLAGS+=

LDFLAGS+=-lrga

CSRCS+=rga_test.c

OUTPUT:=demo

default:clean $(OUTPUT) scp

$(OUTPUT):
	$(CC) $(CSRCS) $(CFLAGS) $(LDFLAGS) -o $@ 
	
.PHONY:scp
scp:
	scp $(OUTPUT) root@192.168.100.100:/root

.PHONY:clean
clean:
	rm -f $(OUTPUT)
