LINK_FLAG = -I ./include -L ./libs -lmlog

ALL:
	@gcc httpd.c $(LINK_FLAG) -o2 -o httpd
	
clean:
	@rm httpd
