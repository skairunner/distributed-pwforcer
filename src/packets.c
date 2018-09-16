#include <packets.h>
#include <stdio.h>
#include <string.h>

int packettypeFromString(char c) {
	switch(c) {
		case 'a': return PACKET_ANSWER;
		case 'w': return PACKET_WORK;
		case 's': return PACKET_START;
		default: return -1;
	}
}

void serialize_start(struct packet_start* packet, char** serialized, int* len) {
	*serialized = (char*)malloc(sizeof(char) * 6);
	strcpy(*serialized, "   0s");
	*len = PACKET_HEADER_SIZE;
}

void serialize_answer(struct packet_answer* packet, char** serialized, int* len) {
	asprintf(serialized, "    a%d %hi %s", packet->workindex, packet->pwdlen, packet->pwd);
	*len = strlen(*serialized + sizeof(char) * 5);
	char* tempstr;
	asprintf(&tempstr, "%4d", *len);
	// copy excluding terminating byte
	strncpy(*serialized, tempstr, 4);
	free(tempstr);
	*len += 5;
}

void serialize_work(struct packet_work* packet, char** serialized, int* len) {
	asprintf(serialized, "    w%d %d %d %hi %s",
				packet->workindex,
				packet->nworkers,
				packet->rangenum,
				packet->pwdlen,
				packet->hash);
	// must find str len excluding header
	*len = strlen(*serialized + sizeof(char) * 5);
	char* tempstr;
	asprintf(&tempstr, "%4d", *len);
	// copy excluding terminating byte
	strncpy(*serialized, tempstr, 4);
	free(tempstr);
	*len += 5;
}

void serialize_packet(void* packet, int packettype, char** serialized, int* len) {
	if (packettype == PACKET_WORK)
		serialize_work((struct packet_work*)packet, serialized, len);
	else if (packettype == PACKET_ANSWER)
		serialize_answer((struct packet_answer*)packet, serialized, len);
	else if (packettype == PACKET_START)
		serialize_start((struct packet_start*)packet, serialized, len);
	else {
		printf("Unknown packet type %d\n", packettype);
		exit(1);
	}
}

void deserialize_start(struct packet_start** packet, char* serialized) {
	*packet = malloc(sizeof(struct packet_start));
	// end of function
}

// string in the form of
// "4 3 8 1709BC8CCB278EAF47CE59C271217866DA96BE38F52BD95EC798C29BA71C96C2"
void deserialize_work(struct packet_work** packet, char* serialized) {
	*packet = malloc(sizeof(struct packet_work));
	char* ptr1;
	char* ptr2 = serialized;
	(*packet)->workindex = strtol(ptr2, &ptr1, 10);
	(*packet)->nworkers = strtol(ptr1+1, &ptr2, 10);
	(*packet)->rangenum = strtol(ptr2+1, &ptr1, 10);
	(*packet)->pwdlen = strtol(ptr1+1, &ptr2, 10);
	(*packet)->hash = malloc(sizeof(char) * 65);
	strcpy((*packet)->hash, ptr2+1);
}

void deserialize_answer(struct packet_answer** packet, char* serialized) {
	*packet = malloc(sizeof(struct packet_answer));
	char* ptr1;
	char* ptr2 = serialized;
	(*packet)->workindex = strtol(ptr2, &ptr1, 10);
	(*packet)->pwdlen = strtol(ptr1+1, &ptr2, 10);
	(*packet)->pwd = malloc(sizeof(char) * 65);
	strcpy((*packet)->pwd, ptr2+1);
}

void deserialize_packet(void** packet, int packettype, char* serialized) {
	if (packettype == PACKET_WORK)
		deserialize_work((struct packet_work**)packet, serialized);
	else if (packettype == PACKET_ANSWER)
		deserialize_answer((struct packet_answer**)packet, serialized);
	else if (packettype == PACKET_START)
		deserialize_start((struct packet_start**)packet, serialized);
	else {
		printf("Unknown packet type %d\n", packettype);
		exit(1);
	}
}

void printpacket_start(struct packet_start* packet) {
	printf("<Start Packet>\n");
}

void printpacket_work(struct packet_work* packet) {
	printf("<Work Packet {\n");
	printf("\tWork index : %d\n", packet->workindex);
	printf("\tNum workers: %d\n", packet->nworkers);
	printf("\tRange num  : %d\n", packet->rangenum);
	printf("\tHash       : %s\n", packet->hash);
	printf("\tPwd len    : %hi\n\t}>\n", packet->pwdlen);
}

// why the hell do we need this why doesn't send send the entire buffer ugh
int send_all(int socket, char* buffer, int len) {
	char* ptr = buffer;
	int nbytes;
	while (len > 0) {
		nbytes = send(socket, buffer, len, 0);
		if (nbytes < 1)
			return nbytes;
		ptr += nbytes;
		len -= nbytes;
	}
	return 0;
}

int recv_all(int socket, char* buffer, int len) {
	char* ptr = buffer;
	int nbytes;
	while (len > 0) {
		nbytes = recv(socket, buffer, len, 0);
		if (nbytes < 1)
			return nbytes;
		ptr += nbytes;
		len -= nbytes;
	}
	return 0;
}

char recv_packet_buffer[1000];
void recv_packet(int sockfrom, int* packettype, void** packet) {
	// First, recv 4 byte header
	memset(recv_packet_buffer, 0, 1000);
	int nbytes = recv(sockfrom, recv_packet_buffer, PACKET_HEADER_SIZE, 0);
	if (nbytes <= 0) {
		// error or connection closed
		if (nbytes == 0) {
			// connection closed
			printf("Client %d hung up.\n", sockfrom);
			*packettype = PACKET_CLOSED;
			*packet = NULL;
		} else {
			perror("recv");
			exit(1);
		}
		return;
	}
	// next, find message length and packet type
	*packettype = packettypeFromString(recv_packet_buffer[4]);
	if (*packettype == -1) {
		printf("%c is not a valid packet type", recv_packet_buffer[4]);
	}
	//printf("Received packet header[%s]\n", recv_packet_buffer);
	recv_packet_buffer[4] = 0;
	int msglen = (int)atol(recv_packet_buffer);
	if (msglen != 0) {
		if (recv_all(sockfrom, recv_packet_buffer, msglen) == -1) {
			perror("recv_all");
			exit(1);
		}
		//printf("Packet[%s]\n", recv_packet_buffer);
	}
	deserialize_packet(packet, *packettype, recv_packet_buffer);
}

void send_packet(int sockto, int packettype, void* packet) {
	int strlen = 0;
	char* msg = 0;

	serialize_packet(packet, packettype, &msg, &strlen);
	if (send_all(sockto, msg, strlen) == -1) {
		perror("send all");
		exit(1);
	}
	free(msg);
}