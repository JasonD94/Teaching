CC		= g++
CFLAGS = -O0 -g -Wall -Wextra -pedantic-errors  -w -std=c++11

all:
	$(CC) $(CFLAGS) POST_simple.cpp -o simple.out -lcurl
	$(CC) $(CFLAGS) POST_contributor.cpp API.cpp -o key.out  -lcurl
	$(CC) $(CFLAGS) POST_email.cpp API.cpp -o email.out  -lcurl
	$(CC) $(CFLAGS) POST_API_Test.cpp API.cpp -o api.out  -lcurl
