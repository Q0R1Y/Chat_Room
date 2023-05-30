CFLAG:=-g -Wall -I$(shell pwd)/include -lpthread
CC:=g++
TAR_DIR:=$(shell pwd)
export

all:
	make -C src
test:
	make -C src test

.PHONY:clean
clean:
	rm -rf s_log c_log server client test
	make -C src clean
