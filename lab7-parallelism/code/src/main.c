#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int find_sequence_in_file(const char *filename, const char *sequence) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		perror("Could not open file");
		return 0;
	}

	size_t sequence_length = strlen(sequence);
	size_t match_count = 0;
	int ch;

	while ((ch = fgetc(file)) != EOF) {
		if (ch == sequence[match_count]) {
			match_count++;
			if (match_count == sequence_length) {
				fclose(file);
				return 1;
			}
		} else {
			match_count = (ch == sequence[0]) ? 1 : 0;
		}
	}

	fclose(file);
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <filename> <sequence>\n", argv[0]);
		return EXIT_FAILURE;
	}

	const char *filename = argv[1];
	const char *sequence = argv[2];

	int found = find_sequence_in_file(filename, sequence);
	if (found) {
		printf("Sequence found\n");
	} else {
		printf("Sequence not found\n");
	}

	return EXIT_SUCCESS;
}
