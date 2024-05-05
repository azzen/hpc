#include "k-means.h"
#include <immintrin.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPAND_u8_u32(arr)                                                                                             \
	(uint32_t) arr[0], (uint32_t)arr[1], (uint32_t)arr[2], (uint32_t)arr[3], (uint32_t)arr[4], (uint32_t)arr[5],       \
		(uint32_t)arr[6], (uint32_t)arr[7]

// This function will calculate the euclidean distance between two pixels.
// Instead of using coordinates, we use the RGB value for evaluating distance.
float distance(uint8_t *p1, uint8_t *p2) {
	float r_diff = p1[0] - p2[0];
	float g_diff = p1[1] - p2[1];
	float b_diff = p1[2] - p2[2];
	return sqrt(r_diff * r_diff + g_diff * g_diff + b_diff * b_diff);
}

float distance_sqr(uint8_t *p1, uint8_t *p2) {
	float r_diff = p1[0] - p2[0];
	float g_diff = p1[1] - p2[1];
	float b_diff = p1[2] - p2[2];
	return r_diff * r_diff + g_diff * g_diff + b_diff * b_diff;
}

void distance_sqr_avx(uint8_t *p1, uint8_t *p2, uint32_t *res, int single) {
	uint8_t *p1_r = p1 + R_OFFSET;
	uint8_t *p1_g = p1 + G_OFFSET;
	uint8_t *p1_b = p1 + B_OFFSET;

	uint8_t *p2_r = p2 + R_OFFSET;
	uint8_t *p2_g = p2 + G_OFFSET;
	uint8_t *p2_b = p2 + B_OFFSET;

	__m256i p1r_v = _mm256_set_epi32(EXPAND_u8_u32(p1_r));
	__m256i p1g_v = _mm256_set_epi32(EXPAND_u8_u32(p1_g));
	__m256i p1b_v = _mm256_set_epi32(EXPAND_u8_u32(p1_b));

	__m256i p2r_v = _mm256_set1_epi32((uint32_t)(p2_r[0]));
	__m256i p2g_v = _mm256_set1_epi32((uint32_t)(p2_g[0]));
	__m256i p2b_v = _mm256_set1_epi32((uint32_t)(p2_b[0]));

	__m256i r_diff = _mm256_sub_epi32(p1r_v, p2r_v);
	__m256i g_diff = _mm256_sub_epi32(p1g_v, p2g_v);
	__m256i b_diff = _mm256_sub_epi32(p1b_v, p2b_v);

	__m256i r_diff_sqr = _mm256_mullo_epi32(r_diff, r_diff);
	__m256i g_diff_sqr = _mm256_mullo_epi32(g_diff, g_diff);
	__m256i b_diff_sqr = _mm256_mullo_epi32(b_diff, b_diff);

	__m256i sums = _mm256_add_epi32(r_diff_sqr, g_diff_sqr);
	sums = _mm256_add_epi32(sums, b_diff_sqr);
	_mm256_storeu_si256((__m256i *)res, sums);
}

// Function to initialize cluster centers using the k-means++ algorithm
void kmeans_pp(struct img_1D_t *image, int num_clusters, uint8_t *centers) {
	// Select a random pixel as the first cluster center
	int first_center = (rand() % (image->width * image->height)) * image->components;
	size_t img_dim = image->width * image->height;
	size_t i, j, k, rem;

	// Set the RGB values of the first center
	centers[0 + R_OFFSET] = image->data[first_center + R_OFFSET];
	centers[0 + G_OFFSET] = image->data[first_center + G_OFFSET];
	centers[0 + B_OFFSET] = image->data[first_center + B_OFFSET];

	float *distances = (float *)malloc(img_dim * sizeof(float));

	// Calculate distances from each pixel to the first center
	uint8_t *dest = malloc(image->components * sizeof(uint8_t));
	memcpy(dest, centers, image->components * sizeof(uint8_t));

	rem = img_dim - (img_dim % 8);
	for (i = 0; i < rem; i += 8) {
		uint32_t res[8] = {0};
		uint8_t *src = image->data + i * image->components;
		uint8_t *dst = dest;
		distance_sqr_avx(src, dst, res, 1);
		for (j = 0; j < 8; ++j) {
			distances[i + j] = res[j];
		}
	}

	for (i = rem; i < img_dim; ++i) {
		uint8_t *src = malloc(image->components * sizeof(uint8_t));
		memcpy(src, image->data + i * image->components, image->components * sizeof(uint8_t));
		distances[i] = distance_sqr(src, dest);
		free(src);
	}

	free(dest);

	// Loop to find remaining cluster centers
	for (int i = 1; i < num_clusters; ++i) {
		float total_weight = 0.0;
		__m256 dist_v;

		rem = img_dim - ((img_dim) % 8); // 8 pixels per iteration
		for (j = 0; j < rem; j += 8) {
			dist_v = _mm256_loadu_ps(distances + j);
			dist_v = _mm256_hadd_ps(dist_v, dist_v);
			total_weight += dist_v[0];
		}

		for (j = rem; j < img_dim; ++j) {
			total_weight += distances[j];
		}

		float r = ((float)rand() / (float)RAND_MAX) * total_weight;
		int index = 0;

		// Choose the next center based on weighted probability
		while (index < img_dim && r > distances[index]) {
			r -= distances[index];
			index++;
		}

		// Set the RGB values of the selected center
		centers[i * image->components + R_OFFSET] = image->data[index * image->components + R_OFFSET];
		centers[i * image->components + G_OFFSET] = image->data[index * image->components + G_OFFSET];
		centers[i * image->components + B_OFFSET] = image->data[index * image->components + B_OFFSET];

		// Update distances based on the new center
		uint8_t *new_center = malloc(image->components * sizeof(uint8_t));
		memcpy(new_center, centers + i * image->components, image->components * sizeof(uint8_t));

		rem = img_dim - (img_dim % 8);
		for (j = 0; j < rem; j += 8) {
			uint32_t res[8] = {0};
			uint8_t *src = image->data + j * image->components;
			uint8_t *dst = new_center;
			distance_sqr_avx(src, dst, res, 1);
			for (k = 0; k < 8; ++k) {
				if (res[k] < distances[j + k]) {
					distances[j + k] = res[k];
				}
			}
		}
		for (j = rem; j < img_dim; ++j) {
			uint8_t *src = malloc(image->components * sizeof(uint8_t));
			memcpy(src, image->data + j * image->components, image->components * sizeof(uint8_t));
			uint16_t dist = distance_sqr(src, new_center);
			if (dist < distances[j]) {
				distances[j] = dist;
			}

			free(src);
		}
		free(new_center);
	}
	free(distances);
}

// This function performs k-means clustering on an image.
// It takes as input the image, its dimensions (width and height), and the
// number of clusters to find.
void kmeans(struct img_1D_t *image, int num_clusters) {
	uint8_t *centers = calloc(num_clusters * image->components, sizeof(uint8_t));

	// image dimensions without components
	size_t img_dim;
	size_t comp_size = image->components * sizeof(uint8_t);
	size_t i, c;

	// Initialize the cluster centers using the k-means++ algorithm.
	kmeans_pp(image, num_clusters, centers);
	img_dim = image->width * image->height;

	int *assignments = (int *)malloc(img_dim * sizeof(int));

	// Assign each pixel in the image to its nearest cluster.
	for (i = 0; i < img_dim; ++i) {
		float min_dist = INFINITY;
		int best_cluster = -1;

		uint8_t *src = malloc(comp_size);
		memcpy(src, image->data + i * image->components, comp_size);

		for (c = 0; c < num_clusters; c++) {
			uint8_t *dest = malloc(comp_size);
			memcpy(dest, centers + c * image->components, comp_size);

			uint16_t dist = distance(src, dest);

			if (dist < min_dist) {
				min_dist = dist;
				best_cluster = c;
			}

			assignments[i] = best_cluster;
			free(dest);
		}
		free(src);
	}

	ClusterData *cluster_data = (ClusterData *)calloc(num_clusters, sizeof(ClusterData));

	// Compute the sum of the pixel values for each cluster.
	for (i = 0; i < img_dim; ++i) {
		int cluster = assignments[i];
		cluster_data[cluster].count++;
		cluster_data[cluster].sum_r += (int)image->data[i * image->components + R_OFFSET];
		cluster_data[cluster].sum_g += (int)image->data[i * image->components + G_OFFSET];
		cluster_data[cluster].sum_b += (int)image->data[i * image->components + B_OFFSET];
	}

	// Update cluster centers based on the computed sums
	for (c = 0; c < num_clusters; ++c) {
		if (cluster_data[c].count > 0) {
			centers[c * image->components + R_OFFSET] = cluster_data[c].sum_r / cluster_data[c].count;
			centers[c * image->components + G_OFFSET] = cluster_data[c].sum_g / cluster_data[c].count;
			centers[c * image->components + B_OFFSET] = cluster_data[c].sum_b / cluster_data[c].count;
		}
	}

	free(cluster_data);

	// Update image data with the cluster centers
	for (i = 0; i < img_dim; ++i) {
		int cluster = assignments[i];
		image->data[i * image->components + R_OFFSET] = centers[cluster * image->components + R_OFFSET];
		image->data[i * image->components + G_OFFSET] = centers[cluster * image->components + G_OFFSET];
		image->data[i * image->components + B_OFFSET] = centers[cluster * image->components + B_OFFSET];
	}

	free(assignments);
	free(centers);
}