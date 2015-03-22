all:
	g++ -g main.cpp -o fat

clean:
	rm -f ./fat
	rm -f *.o
	cp ../sampledisk* ./
