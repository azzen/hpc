#include <coz.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	const char *filename;
	const char *sequence;
	size_t start;
	size_t end;
	int found;
	pthread_mutex_t *mutex;
} thread_args_t;

void *search_chunk(void *arg) {
	thread_args_t *args = (thread_args_t *)arg;

	FILE *fp = fopen(args->filename, "r");
	if (!fp) {
		perror("Could not open file");
		pthread_exit(NULL);
	}

	fseek(fp, args->start, SEEK_SET);

	size_t sequence_length = strlen(args->sequence);
	size_t buffer_size = args->end - args->start + sequence_length;

	// printf("Thread %lu searching from %lu to %lu\n", pthread_self(), args->start, args->end);

	char *buffer = malloc(buffer_size);

	if (!buffer) {
		perror("Could not allocate buffer");
		fclose(fp);
		pthread_exit(NULL);
	}

	size_t bytes_read = fread(buffer, 1, buffer_size, fp);
	fclose(fp);

	size_t match_count = 0;
	for (size_t i = 0; i < bytes_read; ++i) {
		if (buffer[i] == args->sequence[match_count]) {
			match_count++;
			if (match_count == sequence_length) {
				pthread_mutex_lock(args->mutex);
				args->found = 1;
				pthread_mutex_unlock(args->mutex);
				free(buffer);
				// printf("Thread %lu found sequence\n", pthread_self());
				pthread_exit(NULL);
			}
		} else {
			match_count = (buffer[i] == args->sequence[0]) ? 1 : 0;
		}
		COZ_PROGRESS;
	}

	free(buffer);
	pthread_exit(NULL);
}

int find_sequence_in_file_omp(const char *filename, const char *sequence) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		perror("Could not open file");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	fclose(fp);

	// printf("File size: %lu\n", file_size);

	size_t sequence_length = strlen(sequence);
	int found = 0;

	// clang-format off

	#pragma omp parallel shared(found)
	{
		int tid = omp_get_thread_num();
		int nthreads = omp_get_num_threads();
		int start = tid * (file_size / nthreads);
		int end = (tid == nthreads - 1) ? file_size : (tid + 1) * (file_size / nthreads);
		//printf("Thread %d searching from %d to %d\n", tid, start, end);
		
		FILE *fp = fopen(filename, "r");
		
		if (!fp) {
			perror("Could not open file");
		}

		fseek(fp, start, SEEK_SET);
		size_t buffer_size = end - start + sequence_length;
		char *buffer = malloc(buffer_size);

		if (!buffer) {
			perror("Could not allocate buffer");
			fclose(fp);
			#pragma omp critical
			{ found = 0; }
			#pragma omp cancel parallel
		}

		size_t bytes_read = fread(buffer, 1, buffer_size, fp);
		fclose(fp);

		size_t match_count = 0;
		for (size_t i = 0; i < bytes_read; ++i) {
			if (buffer[i] == sequence[match_count]) {
				match_count++;

				if (match_count == sequence_length) {
					#pragma omp critical
					{ 
						found = 1; 
					}
					free(buffer);
					buffer = NULL;
					break;
				}

			} else {
				match_count = (buffer[i] == sequence[0]) ? 1 : 0;
			}
			COZ_PROGRESS;
		}

		// clang-format on
		if (buffer != NULL)
			free(buffer);
	}
	return found;
}

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
		COZ_PROGRESS;
	}

	fclose(file);
	return 0;
}

int find_sequence_in_file_multi(const char *filename, const char *sequence, int nthreads) {
	FILE *fp = fopen(filename, "r");

	if (!fp) {
		perror("Could not open file");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	fclose(fp);

	printf("File size: %lu\n", file_size);

	pthread_t *threads = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
	thread_args_t *args = (thread_args_t *)malloc(nthreads * sizeof(thread_args_t));
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	int found = 0;

	for (int i = 0; i < nthreads; ++i) {
		args[i].filename = filename;
		args[i].sequence = sequence;
		args[i].start = i * (file_size / nthreads);
		args[i].end = (i + 1) * (file_size / nthreads);

		if (args[i].end > file_size) {
			args[i].end = file_size;
		}

		args[i].found = 0;
		args[i].mutex = &mutex;

		pthread_create(&threads[i], NULL, search_chunk, &args[i]);
	}

	for (int i = 0; i < nthreads; ++i) {
		pthread_join(threads[i], NULL);
		if (args[i].found) {
			printf("Sequence found by thread %d\n", i);
			found = 1;
			break;
		}
	}

	return found;
}

int main(int argc, char *argv[]) {
	int found = 0;

	if (argc == 3) {
		const char *filename = argv[1];
		const char *sequence = argv[2];
		found = find_sequence_in_file(filename, sequence);
	} else if (argc == 4) {
		const char *filename = argv[1];
		const char *sequence = argv[2];
		if (strncmp(argv[3], "--nthreads=", 11) == 0) {
			int nb_threads = atoi(argv[3] + 11);
			found = find_sequence_in_file_multi(filename, sequence, nb_threads);
		} else if (strncmp(argv[3], "--omp", 5) == 0) {
			found = find_sequence_in_file_omp(filename, sequence);
		}
	} else {
		fprintf(stderr, "Usage: %s <filename> <sequence> --nthreads=<num_thread>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (found) {
		printf("Sequence found\n");
	} else {
		printf("Sequence not found\n");
	}

	return EXIT_SUCCESS;
}
