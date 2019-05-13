obj=json_reader.o json_value.o json_writer.o
THIS_MAKEFILE=$(shell pwd)
INCLUDE=-I$(THIS_MAKEFILE)
json:$(obj)
	ar rc libjson.a $(obj)
$(obj):%.o:%.cpp
	g++  -c $< -o $@
clean:
	-rm -f *.o
