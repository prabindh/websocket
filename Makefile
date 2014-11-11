serverobjs = localhost_server.o localhost_server_websocket.o sha1.o b64.o loadcheck.o

all: localserver

localserver: $(serverobjs)
	g++ -o localserver $(serverobjs)

clean:
	rm *.o localserver



