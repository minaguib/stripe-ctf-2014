#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

SHA_CTX ctx;

char *commit_body_prefix;
size_t commit_body_prefix_len;

#define NONCE_LEN 8
#define NONCE_LEN_STR "8"

void initialize_ctx(char * tree, char *parent, char *hostname, char *slot_id) {

	char * commit_header;
	size_t commit_header_len;
	size_t commit_body_len;
	SHA1_Init(&ctx);

	commit_body_prefix_len = asprintf(&commit_body_prefix,
		"tree %s\n"
		"parent %s\n"
		"author Mina Naguib <mina@naguib.ca> %lu -0500\n"
		"committer Mina Naguib <mina@naguib.ca> %lu -0500\n"
		"\n"
		"%s-%s-"
		,tree, parent, time(NULL), time(NULL), hostname, slot_id);

	commit_body_len = commit_body_prefix_len + NONCE_LEN;
	commit_header_len = asprintf(&commit_header, "commit %lu", commit_body_len);

	SHA1_Update(&ctx, (void*)commit_header, commit_header_len + 1);
	SHA1_Update(&ctx, (void*)commit_body_prefix, commit_body_prefix_len);

}

inline void increment_nonce(unsigned int n, char *nonce) {
	register char *c;
	for (c = nonce + NONCE_LEN - 1; c >= nonce; c--) {
		(*c)++;
		if (*c > '9' && *c < 'a')
		  (*c) = 'a';
		else if (*c > 'f') {
		  (*c) = '0';
		  continue;
		}
		return;
	}
	sprintf(nonce, "%0" NONCE_LEN_STR "x", n);
}

int brute(char *difficulty_hex, int slot_id) {
	SHA_CTX ctx2;
	unsigned char md[SHA_DIGEST_LENGTH];
	unsigned int i, d;
	unsigned char difficulty[SHA_DIGEST_LENGTH] = {};
	int found;
	struct timeval start_tv,now_tv;
	unsigned int elapsed_ms;
	char nonce[NONCE_LEN + 1];

	for (d = 0; d < SHA_DIGEST_LENGTH; d++) {
		sscanf(difficulty_hex + (d*2), "%02x", (unsigned int *)(difficulty + d));
	}

	gettimeofday(&start_tv, NULL);
	sprintf(nonce, "%0" NONCE_LEN_STR "x", 0);

	for(i=0; ;i++) {

		ctx2 = ctx;
		increment_nonce(i, nonce);
		SHA1_Update(&ctx2, (void*)nonce, NONCE_LEN);
		SHA1_Final(md, &ctx2);

		found = 1;
		for (d = 0; d < SHA_DIGEST_LENGTH; d++) {
			if (md[d] > difficulty[d]) {
				found = 0;
				break;
			}
		}

		if (found)
			break;

		if (i != 0 && (i % 5000000) == 0) {
			gettimeofday(&now_tv, NULL);
			elapsed_ms = (now_tv.tv_sec - start_tv.tv_sec) * 1000.0;
			elapsed_ms += (now_tv.tv_usec - start_tv.tv_usec) / 1000.0;
			fprintf(stderr, "%.24s Slot [%3d] at attempt %u nonce %s (%u/s)\n",
					ctime(&now_tv.tv_sec),
					slot_id,
					i,
					nonce,
					(unsigned)((float)i / (float)elapsed_ms * 1000.0)
					);
		}

	};

	fprintf(stderr, "Found at nonce %s:\n", nonce);
	for (d = 0; d < NONCE_LEN; d++) {
		fprintf(stderr, "%02x", md[d]);
	}
	fprintf(stderr, "\n");

	// Found - now we send back to output what's fed to git hash-object -t commit
	// If we have no bugs, it will re-calculate the same hash we did
	fputs(commit_body_prefix, stdout);
	fputs(nonce, stdout);

	return 1;
}

int main(int argc, char ** argv) {

	char *difficulty, *tree, *parent, *hostname, *slot_id;

	if (argc < 6) {
		fprintf(stderr, "Bad arguments %d\n", argc);
		exit(1);
	}
	difficulty = argv[1];
	tree = argv[2];
	parent = argv[3];
	hostname = argv[4];
	slot_id = argv[5];

	/*
	fprintf(stderr,
		"Solving for\n"
		"\tdifficulty=%s\n"
		"\ttree=%s\n"
		"\tparent=%s\n"
		difficulty, tree, parent);
	*/

	initialize_ctx(tree, parent, hostname, slot_id);

	return brute(
		difficulty,
		strtoul(slot_id, NULL, 10)
		) ? 0 : 1;

}
