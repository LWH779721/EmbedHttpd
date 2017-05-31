ALL:
	gcc httpd.c util/print_util.c -lpthread -o2 -I util -o httpd
	
clean:
	rm httpd
