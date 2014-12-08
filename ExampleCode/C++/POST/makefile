CC		= g++
CFLAGS = -O0 -g -Wall -Wextra -pedantic-errors  -w -std=c++11

all:
	#$(CC) $(CFLAGS) POST_curl.cpp -o post.out -lcurl
	$(CC) $(CFLAGS) POST_curl_isenseupload.cpp -o post_json.out  -lcurl
	$(CC) $(CFLAGS) upload_simple.cpp -o simple_post.out  -lcurl
