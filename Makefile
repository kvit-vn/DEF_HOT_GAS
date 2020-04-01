MCU=stm8
DEVICE=stm8s103f3
FLASHER=stlinkv2
CFLAGS=-I ../lib/STM8S_StdPeriph_Lib_V2.1.0/Libraries/STM8S_StdPeriph_Driver/inc/ -DSTM8S103 -c
CC=sdcc
LIB=stm8s_gpio.rel stm8s_clk.rel stm8s_tim4.rel
OBJ=main.rel
TARGET=def_hot_gas

.PHONY: all clean

%.rel:	%.c
	$(CC) -m$(MCU)  $(CFLAGS) $(LDFLAGS) $<

all:	$(OBJ) $(LIB)
	$(CC) -m$(MCU) -o $(TARGET).ihx $(OBJ) $(LIB) 

download:
	sudo stm8flash -c $(FLASHER) -p $(DEVICE) -w $(TARGET).ihx
clean:
	@rm -v *.sym *.asm *.lst *.rel *.ihx *lk *.rst *.cdb *.map
