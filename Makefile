all:
	g++ main.cpp -o main
	g++ client.cpp -o client
run:	
	g++ main.cpp -o main
	g++ client.cpp -o client
	./main
	./client
