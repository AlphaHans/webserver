OBJETS=wrapper.o jhhttp.o

main: $(OBJETS)
	g++ -o webserver $(OBJETS)

wrapper.o: wrapper.h wrapper.cc
	g++ -c wrapper.cc -std=c++11
jhhttp.o: jhhttp.h jhhttp.cc httpapi.h
	g++ -c jhhttp.cc -std=c++11

clean:
	rm -f webserver $(OBJETS)
