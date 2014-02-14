#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>

#include "murmur3.h"
#include "dump.c"

inline bool has_word(char *word) {
	int i;
	char *c, word2[BUFFLEN];
	size_t len;
	uint32_t hash[4];
	uint16_t h1, h2;

	for (len = 0, c = word; *c; c++) {
		word2[len++] = tolower(*c);
	}
	word2[len] = '\0';

	MurmurHash3_x64_128(word2, len, MURMUR_SEED, hash);
	h1 = hash[0] % BUCKETS;
	h2 = hash[1] % (UINT16_MAX - 1) + 1;

	for (i = 0; i < BUCKET_SLOTS; i++) {
		if (buckets[h1][i] < h2) 
			break;
		else if (buckets[h1][i] == h2)
			return true;
	}

	return false;
}

inline void parse_line(char *line) {
	char *word;
	bool has;
	int i = 0;
	while ((word = strsep(&line, " \n")) != NULL && *word != '\0') {
		if ((i++) > 0) {
			fputc(' ', stdout);
		}
		has = has_word(word);
		if (!has)
			fputc('<', stdout);
		fputs(word, stdout);
		if (!has)
			fputc('>', stdout);
	}
	fputc('\n', stdout);
}

inline void loop() {
	char line[BUFFLEN];
	setvbuf(stdin, NULL, _IOFBF, (1024 * 1024));
	setvbuf(stdout, NULL, _IOFBF, (1024 * 1024));
	while (fgets(line, BUFFLEN, stdin) != NULL) {
		parse_line(line);
	}
}

int main(int argc, char ** argv) {

#ifdef __gnu_linux__
	struct sched_param param;
	sched_setscheduler(0, SCHED_FIFO, &param);
#endif

	setpriority(PRIO_PROCESS, 0, -20);

	loop();
	return 0;
}

