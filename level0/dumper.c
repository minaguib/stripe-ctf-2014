#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>

#include "murmur3.h"

uint16_t buckets[BUCKETS][BUCKET_SLOTS];

inline void load_dictionary_word(char *word) {
	char *c;
	int i;
	uint16_t h1, h2;
	uint32_t hash[4];

	for (c = word; *c; c++) {
		if (isupper(*c)) {
			//fprintf(stderr, "Skipping capital word %s\n", word);
			return;
		}
	}

	MurmurHash3_x64_128(word, strlen(word), MURMUR_SEED, hash);
	h1 = hash[0] % BUCKETS;
	h2 = hash[1] % (UINT16_MAX - 1) + 1;

	//fprintf(stderr, "Adding %" PRIu16 " %" PRIu16 " %s\n", h1, h2, word);
	for (i = 0; i < BUCKET_SLOTS && buckets[h1][i]; i++) {
		if (buckets[h1][i] == h2) {
			fprintf(stderr, "COLLISSION at [%s] = %"PRIu16"\n", word, h2);
			exit(1);
		}
	}
	if (i >= BUCKET_SLOTS) {
		fprintf(stderr, "REACHED BUCKET_SLOTS\n");
		exit(1);
	}

	buckets[h1][i] = h2;

}

inline void load_dictionary(int argc, char ** argv) {
	char *filename = "/usr/share/dict/words";
	char word[BUFFLEN];
	char *temp;
	FILE *f = NULL;

	if (argc >= 2)
		filename = argv[1];

	fprintf(stderr, "Opening %s\n", filename);
	f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
		exit(1);
	}

	while (fgets(word, BUFFLEN, f) != NULL && *word != '\0') {
		if ((temp = strchr(word, '\n')) != NULL)
			*temp = '\0';
		load_dictionary_word(word);
	}

	fclose(f);
}

int cmp(const void *a, const void *b) {
	uint16_t aa, bb;
	aa = *(uint16_t*)a;
	bb = *(uint16_t*)b;
	if (aa < bb)
		return 1;
	else if (aa == bb)
		return 0;
	else
		return -1;
};

void dump() {
	uint16_t h1, h2, i;
	printf("uint16_t buckets[BUCKETS][BUCKET_SLOTS] = {\n");
	for (h1 = 0; h1 < BUCKETS; h1++) {
		qsort(buckets[h1], BUCKET_SLOTS, sizeof(uint16_t), cmp);
		printf("{");
		for (i = 0; i < BUCKET_SLOTS && (h2 = buckets[h1][i]); i++) {
			printf("%"PRIu16",", h2);
		}
		printf("},\n");
	}
	printf("};\n");

};


int main(int argc, char ** argv) {
	load_dictionary(argc, argv);
	dump();
	return 0;
}

