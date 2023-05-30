pwd:=$(shell pwd)/
cur_obj:=$(addprefix $(pwd),$(patsubst %.cpp,%.o,$(wildcard *.cpp)))
sub_dir:=client server
obj+=$(cur_obj)
export

all:$(cur_obj) $(sub_dir)

$(sub_dir):ECHO
	make -C $@
ECHO:
	@echo $(sub_dir)

$(cur_obj):%.o:%.cpp
	$(CC) $^ $(CFLAG) -c -o $@

test:
	make -C debug test

.PHONY:clean
clean:
	rm -rf *.o
	make -C client clean
	make -C server clean
