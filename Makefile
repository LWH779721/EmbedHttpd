LINK_FLAG = -I ./include -L ./libs


cc = $(CC) 

SRC_PATH = ./src
OBJECT_PATH = object
BIN_PATH = bin 

vpath %.h include
vpath %.c src
vpath %.o object

SRCS = $(wildcard *.c $(SRC_PATH)/*.c)
OBJS = $(patsubst %c, %o, $(notdir $(SRCS)))
 
.PHONY: ALL clean
ALL: httpd $(OBJS)

httpd: $(OBJS)
	$(cc) $(OBJECT_PATH)/*.o $(LINK_FLAG) -o2 -o $@
	
%.o:%.c
	@$(cc) $(LINK_FLAG) -c $< -o $(OBJECT_PATH)/$@
	
clean:
	@-rm httpd
	@-rm $(OBJECT_PATH)/*.o
