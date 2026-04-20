/*
 * Copyright (C) 2026 Marcus Hirt
 *
 * This software is free:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	pragma comment(lib, "ws2_32.lib")
#	define CLOSE(s) closesocket(s)
#	define PATH_SEP "\\"
typedef int socklen_t;
typedef SOCKET sock_t;
#else
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <sys/socket.h>
#	include <unistd.h>
#	define CLOSE(s) close(s)
#	define INVALID_SOCKET (-1)
#	define PATH_SEP "/"
typedef int sock_t;
#endif

#define PACKET_LEN 102
#define WOL_FILE   ".wol"
#define MAX_LINE   256

static int parse_mac(const char *mac, unsigned char out[6]) {
	char hex[13];
	int j = 0;
	for (int i = 0; mac[i]; i++) {
		if (mac[i] != ':' && mac[i] != '-') hex[j++] = mac[i];
	}
	hex[j] = '\0';
	if (j != 12) return -1;
	for (int i = 0; i < 6; i++) {
		char b[3] = {hex[i * 2], hex[i * 2 + 1], 0};
		out[i] = (unsigned char) strtol(b, NULL, 16);
	}
	return 0;
}

static const char *get_home_dir(void) {
#ifdef _WIN32
	const char *home = getenv("USERPROFILE");
	if (!home) home = getenv("HOMEPATH");
	return home;
#else
	return getenv("HOME");
#endif
}

/*
 * Looks up 'name' in ~/.wol. File format (one entry per line):
 *   alias  MAC [broadcast-address [port]]
 * Lines starting with '#' and blank lines are ignored.
 * Returns 0 on match, filling mac_out (required), broadcast_out and
 * *port_out only when present in the file (caller supplies defaults).
 */
static int find_alias(const char *name, char *mac_out, char *broadcast_out, int *port_out) {
	const char *home = get_home_dir();
	if (!home) return -1;

	char path[512];
	snprintf(path, sizeof(path), "%s" PATH_SEP "%s", home, WOL_FILE);

	FILE *f = fopen(path, "r");
	if (!f) return -1;

	char line[MAX_LINE];
	int found = -1;
	while (fgets(line, sizeof(line), f)) {
		char *p = line;
		while (*p == ' ' || *p == '\t') p++;
		if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

		char alias[64], mac[32], broadcast[32];
		int  port = -1;
		int  n    = sscanf(p, "%63s %31s %31s %d", alias, mac, broadcast, &port);
		if (n < 2) continue;

		if (strcmp(alias, name) == 0) {
			strcpy(mac_out, mac);
			if (n >= 3) strcpy(broadcast_out, broadcast);
			if (n >= 4) *port_out = port;
			found = 0;
			break;
		}
	}
	fclose(f);
	return found;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: wol <MAC|alias> [broadcast-address] [port]\n");
		fprintf(stderr, "  MAC               AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF\n");
		fprintf(stderr, "  alias             name defined in ~/.wol\n");
		fprintf(stderr, "  broadcast-address IPv4 broadcast address (default: 255.255.255.255)\n");
		fprintf(stderr, "                    Use subnet-directed broadcast (e.g. 192.168.1.255)\n");
		fprintf(stderr, "                    if your router blocks 255.255.255.255 across VLANs.\n");
		fprintf(stderr, "  port              UDP port (default: 9)\n");
		fprintf(stderr, "Example: wol AA:BB:CC:DD:EE:FF 192.168.1.255 9\n");
		fprintf(stderr, "Example: wol macmini\n");
		fprintf(stderr, "\n~/.wol format:  alias  MAC [broadcast-address [port]]\n");
		return 1;
	}

	char alias_mac[32]       = {0};
	char alias_broadcast[32] = {0};
	int  alias_port          = -1;

	const char *mac       = argv[1];
	const char *broadcast = "255.255.255.255";
	int         port      = 9;

	unsigned char macb[6];
	if (parse_mac(mac, macb) < 0) {
		if (find_alias(argv[1], alias_mac, alias_broadcast, &alias_port) < 0) {
			fprintf(stderr, "Invalid MAC address and no alias '%s' found in ~/%s\n", argv[1], WOL_FILE);
			return 2;
		}
		mac = alias_mac;
		if (alias_broadcast[0]) broadcast = alias_broadcast;
		if (alias_port > 0)     port      = alias_port;
		if (parse_mac(mac, macb) < 0) {
			fprintf(stderr, "Invalid MAC address in alias '%s': %s\n", argv[1], mac);
			return 2;
		}
	}

	if (argc > 2) broadcast = argv[2];
	if (argc > 3) port      = atoi(argv[3]);

	unsigned char packet[PACKET_LEN];
	memset(packet, 0xFF, 6);
	for (int i = 6; i < PACKET_LEN; i += 6)
		memcpy(packet + i, macb, 6);

#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	sock_t sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) { perror("socket"); return 3; }

	int one = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *) &one, sizeof(one));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons((unsigned short) port);
	if (inet_pton(AF_INET, broadcast, &addr.sin_addr) != 1) {
		fprintf(stderr, "Invalid broadcast address: %s\n", broadcast);
		CLOSE(sock);
		return 4;
	}

	if (sendto(sock, (const char *) packet, PACKET_LEN, 0,
	           (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("sendto");
		CLOSE(sock);
		return 5;
	}
	CLOSE(sock);

#ifdef _WIN32
	WSACleanup();
#endif

	printf("Magic packet sent to %s via %s:%d\n", mac, broadcast, port);
	return 0;
}
