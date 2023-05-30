cur_obj=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
obj+=$(cur_obj)
server:$(cur_obj)
	$(CC) $(obj) $(CFLAG) -o $(TAR_DIR)/$@
$(cur_obj):%.o:%.cpp
	$(CC) $^ $(CFLAG) -c -o $@


.PHONY:clean
clean:
	rm -rf *.o
