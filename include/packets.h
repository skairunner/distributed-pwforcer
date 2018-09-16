#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define PACKET_HEADER_SIZE 5

enum packettypes {PACKET_NULL=0, PACKET_CLOSED, PACKET_START, PACKET_WORK, PACKET_ANSWER};

int packettypeFromString(char c);

// packet>   0s
//        ^^^ care of leading spaces
struct packet_start {
};

// packet>MMMMwW+ N+ B+ L+ H64
// eg>  74w3 4 3 8 1709BC8CCB278EAF47CE59C271217866DA96BE38F52BD95EC798C29BA71C96C2
//    ^^ care of leading spaces
// eg is last batch out of 4 for pw len 8
struct packet_work {
	int workindex;
	int nworkers;
	int rangenum; // which range
	char* hash;
	short pwdlen;
};

// packet>MMMMaW+ L+ P+
// eg:  15a 3 10 helloworld
//    ^^ care of leading spaces
struct packet_answer {
	int workindex;
	short pwdlen;
	char* pwd;
};

// mallocs a new string into serialized.
/*
	MSGLEN    : M
	MSGTYP    : T
	WORK INDEX: W
	NUMWORKERS: N
	BATCH     : B
	HASH      : H
	PWDLEN    : L
	PWD       : P
*/
void serialize_start(struct packet_start* packet, char** serialized, int* len);
void serialize_answer(struct packet_answer* packet, char** serialized, int* len);
void serialize_work(struct packet_work* packet, char** serialized, int* len);
void serialize_packet(void* packet, int packettype, char** serialized, int* len);

void deserialize_start(struct packet_start** packet, char* serialized);
void deserialize_work(struct packet_work** packet, char* serialized);
void deserialize_answer(struct packet_answer** packet, char* serialized);
void deserialize_packet(void** packet, int packettype, char* serialized);

void printpacket_start(struct packet_start* packet);
void printpacket_work(struct packet_work* packet);
void printpacket_answer(struct packet_answer* packet);

void recv_packet(int sockfrom, int* packettype, void** packet);

void send_packet(int sockto, int packettype, void* packet);