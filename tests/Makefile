CC:=/opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
LD:=/opt/aarch64-linux-gnu/bin/aarch64-linux-gnu-ld

CFLAGS+=-Irockchip_rga
CXXFLAGS+=

LDFLAGS+=

OUTPUT:=demo

include test.mk
include rockchip_rga/rockchip_rga.mk

default:clean $(OUTPUT) scp

$(OUTPUT):
	$(CC) $(CSRCS) $(CXX_SRCS) -o $@
	
.PHONY:scp
scp:
	scp $(OUTPUT) root@192.168.100.100:/root

.PHONY:clean
clean:
	rm -f $(OUTPUT)
