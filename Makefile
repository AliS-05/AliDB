all:
	g++ main.cpp network.cpp -o main
	g++ client.cpp network.cpp -o client
run:	
	g++ main.cpp -o main
	g++ client.cpp -o client
	./main
	./client
