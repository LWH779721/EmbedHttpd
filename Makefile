ALL:
	gcc httpd.c -o httpd -lpthread

clean:
	rm httpd
