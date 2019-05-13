obj=ComThread.o main.o Socket.o globle.o Base64.o WebClient.o BaseConfig.o GateClient.o IFSC_079.o  IFMC_055.o IFSC_085.o IFSC_080.o IFSC_062.o IFSC_066.o IFAL_011.o\
IFAL_074.o IFSC_061.o IFSC_071.o IFSC_086.o
THIS_MAKEFILE=$(shell pwd)
INCLUDE=-I$(THIS_MAKEFILE) -I$(THIS_MAKEFILE)/jsonCpp -I$(THIS_MAKEFILE)/inconv
LIBPATH=-L$(THIS_MAKEFILE)/json -L$(THIS_MAKEFILE)/iconv
trans:$(obj)
	g++ -o gate_server $(obj) $(LIBPATH) -lrt -lm -lpthread -ljson -liconv
$(obj):%.o:%.cpp
	g++ -O2 -fvisibility=hidden -s -D__linux -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE $(INCLUDE) -c $< -o $@
clean:
	reset
	-rm -f *.o
