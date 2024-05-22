#include <stdio.h>

// Size = 100 MiB
#define SIZE 0x6400000

float a[SIZE];

void fill(void) {
	for (int i = 0; i < SIZE; i++) {
		a[i] = 1.0f;
	}
}

void sum(float *a) {
	float sum = 0.0f;
	for (int i = 0; i < SIZE; ++i) {
		sum += a[i];
	}
}

int main(void) {
	fill();
	sum(a);
}