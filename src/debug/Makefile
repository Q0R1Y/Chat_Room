cur_obj=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
tar=$(patsubst %.cpp,%,$(wildcard *.cpp))
obj+=$(cur_obj)

test:$(tar)
	@echo $(tar)

$(tar):$(cur_obj)
	@echo $(obj)
	$(CC) $^ $(CFLAG) -o $@

$(cur_obj):%.o:%.cpp
	@echo $(cur_obj)
	$(CC) $^ $(CFLAG) -c -o $@


.PHONY:clean
clean:
	rm -rf $(obj)
