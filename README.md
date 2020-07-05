# simple-TCP

This project contains material that I used to teach a basic networking class at the University of Strasbourg.
The goal is to implement a simpler TCP using UDP sockets in C.
The code is in French because the classes during the Bachelor's are in French.

In the following, I will briefly discuss how things work, and why.

## Go-back-N

We ask them to use the following structure to simulate TCP's behavior:

``` C
struct message
{
	int seq;
	int ack;
	char buffer[BUFFER_LENGTH];
};
```

`seq` is a sequence number used to indicate the number of the packet they are sending,
`ack` is the number of the packet the sender is acknowledged (piggybacked information),
and the buffer contains the information we wish to send.
This is akin to the behavior of "Go-Back-N".

## Code behavior

The way the code works is that two actors (the client and the server) send packets through the medium,
a program that introduces random packet loss and random latency to test the resliency of the students' programs.

``` Makefile
make
bin/medium 4 10000 127.0.0.1 5555 127.0.0.1 6666 X Y &
bin/server util/pika.jpeg bin/pika1.jpeg 5555 &
bin/client util/pika.jpeg bin/pika2.jpeg 127.0.0.1 10000 6666
```

Medium parameters:
* X: packet loss rate
* Y: delay

The files for the client and server are FILE_SENT, and FILE_RECEIVED.
In this case, both actos are sending the same image, but they can be of different sizes.

### Discussion

There are certain conditions where this program does not work.
What are the combinations of parameters that cause this behavior? Why?
