#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "sobel.h"

#include <likwid-marker.h>

#define GAUSSIAN_KERNEL_SIZE 3
#define SOBEL_KERNEL_SIZE 3
#define SOBEL_BINARY_THRESHOLD 22500 // 150 * 150

#define CONVOLUTION_1D(img, kernel, position, width)                                                                   \
	(img->data[position - width - 1] * kernel[0] + img->data[position - width] * kernel[1] +                           \
	 img->data[position - width + 1] * kernel[2] + img->data[position - 1] * kernel[3] +                               \
	 img->data[position] * kernel[4] + img->data[position + 1] * kernel[5] +                                           \
	 img->data[position + width - 1] * kernel[6] + img->data[position + width] * kernel[7] +                           \
	 img->data[position + width + 1] * kernel[8])

#define CONVOLUTION_CHAINED(cache, kernel, position, width)                                                            \
	(cache[position - width - 1]->pixel_val[0] * kernel[0] + cache[position - width]->pixel_val[0] * kernel[1] +       \
	 cache[position - width + 1]->pixel_val[0] * kernel[2] + cache[position - 1]->pixel_val[0] * kernel[3] +           \
	 cache[position]->pixel_val[0] * kernel[4] + cache[position + 1]->pixel_val[0] * kernel[5] +                       \
	 cache[position + width - 1]->pixel_val[0] * kernel[6] + cache[position + width]->pixel_val[0] * kernel[7] +       \
	 cache[position + width + 1]->pixel_val[0] * kernel[8])

#define IS_BORDER(position, width)                                                                            \
	(position % width == 0 || position % width == width - 1)

#define OFFSETS(width) 																							   \
	{                                                                                                                  \
		-width - 1, -width, -width + 1, -1, 0, 1, width - 1, width, width + 1                                              \
	}

#define PIXEL_BLACK 0
#define PIXEL_WHITE 255

const int16_t sobel_v_kernel[SOBEL_KERNEL_SIZE * SOBEL_KERNEL_SIZE] = {
	-1, -2, -1, 0, 0, 0, 1, 2, 1,
};

const int16_t sobel_h_kernel[SOBEL_KERNEL_SIZE * SOBEL_KERNEL_SIZE] = {
	-1, 0, 1, -2, 0, 2, -1, 0, 1,
};

const uint16_t gauss_kernel[GAUSSIAN_KERNEL_SIZE * GAUSSIAN_KERNEL_SIZE] = {
	1, 2, 1, 2, 4, 2, 1, 2, 1,
};

struct img_1D_t *edge_detection_1D(const struct img_1D_t *input_img) {
	struct img_1D_t *res_img;
	struct img_1D_t *temp_img;

	res_img = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	temp_img = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	LIKWID_MARKER_INIT;
	LIKWID_MARKER_START("edge_detection_1d");
	rgb_to_grayscale_1D(input_img, res_img);
	gaussian_filter_1D(res_img, temp_img, gauss_kernel);
	sobel_filter_1D(temp_img, res_img, sobel_v_kernel, sobel_h_kernel);
	LIKWID_MARKER_STOP("edge_detection_1d");
	LIKWID_MARKER_CLOSE;
	free_image(temp_img);

	return res_img;
}

void rgb_to_grayscale_1D(const struct img_1D_t *img, struct img_1D_t *result) {
	uint8_t r, g, b;
	size_t i;

	size_t img_size = img->width * img->height * img->components;
	size_t num_components = img->components;

	LIKWID_MARKER_START("rgb_grayscale_1d");
	for (i = 0; i < img_size; i += num_components) {
		r = img->data[i + R_OFFSET] * FACTOR_R;
		g = img->data[i + G_OFFSET] * FACTOR_G;
		b = img->data[i + B_OFFSET] * FACTOR_B;
		result->data[i / num_components] = r + g + b;
	}
	LIKWID_MARKER_STOP("rgb_grayscale_1d");
}

void gaussian_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const uint16_t *kernel) {
	const uint16_t gauss_ponderation = 16;
	size_t i;
	size_t j;

	const size_t width = img->width;
	const size_t height = img->height;
	const size_t img_size = width * height;
	const int32_t offsets[] = OFFSETS(width);

	int32_t sum;

	LIKWID_MARKER_START("gaussian_1d");
	// skip bottom and top borders
	for (i = width; i < img_size - width; ++i) {
		if (IS_BORDER(i, width)) {
			res_img->data[i] = img->data[i];
			continue;
		}

		sum = 0;
		for (j = 0; j < 9; ++j) {
			sum += img->data[i + offsets[j]] * kernel[j];
		}
		res_img->data[i] = sum / gauss_ponderation;

	}
	LIKWID_MARKER_STOP("gaussian_1d");
}

void sobel_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const int16_t *v_kernel,
					 const int16_t *h_kernel) {
	size_t i;
	int16_t gx, gy;
	double_t magnitude;

	const size_t width = img->width;
	const size_t height = img->height;
	const size_t img_size = width * height;
	const int32_t offsets[] = OFFSETS(width);

	LIKWID_MARKER_START("sobel_1d");
	for (i = width; i < img_size - width; ++i) {
		if (IS_BORDER(i, width)) {
			res_img->data[i] = img->data[i];
			continue;
		}		

		gx = 0;
		gy = 0;
		for (size_t j = 0; j < 9; ++j) {
			gx += img->data[i + offsets[j]] * h_kernel[j];
			gy += img->data[i + offsets[j]] * v_kernel[j];
		}

		magnitude = gx * gx + gy * gy;
		
		res_img->data[i] = magnitude > SOBEL_BINARY_THRESHOLD ? PIXEL_WHITE : PIXEL_BLACK;
	}
	LIKWID_MARKER_STOP("sobel_1d");
}

static struct pixel_t **cache = NULL;

struct img_chained_t *edge_detection_chained(const struct img_chained_t *input_img) {
	
	struct img_chained_t *res_img;
	struct img_chained_t *temp_img;

	res_img = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	temp_img = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	cache = (struct pixel_t **)malloc(sizeof(struct pixel_t *) * input_img->width * input_img->height *
									  input_img->components);

	if (!cache) {
		fprintf(stderr, "[%s] cache allocation error\n", __func__);
		perror(__func__);
		exit(EXIT_FAILURE);
	}
	LIKWID_MARKER_INIT;
	LIKWID_MARKER_START("edge_detection_chained");
	rgb_to_grayscale_chained(input_img, res_img);
	gaussian_filter_chained(res_img, temp_img, gauss_kernel);
	sobel_filter_chained(temp_img, res_img, sobel_v_kernel, sobel_h_kernel);
	LIKWID_MARKER_STOP("edge_detection_chained");
	LIKWID_MARKER_CLOSE;
	free(cache);

	return res_img;
}

void rgb_to_grayscale_chained(const struct img_chained_t *img, struct img_chained_t *result) {
	struct pixel_t *cur_img_pixel, *cur_res_pixel;
	uint8_t r, g, b;

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = result->first_pixel;
	LIKWID_MARKER_START("grayscale_chained");
	do {
		r = cur_img_pixel->pixel_val[R_OFFSET] * FACTOR_R;
		g = cur_img_pixel->pixel_val[G_OFFSET] * FACTOR_G;
		b = cur_img_pixel->pixel_val[B_OFFSET] * FACTOR_B;

		cur_res_pixel->pixel_val[0] = r + g + b;

		cur_img_pixel = cur_img_pixel->next_pixel;
		cur_res_pixel = cur_res_pixel->next_pixel;

	} while (cur_img_pixel != NULL);
	LIKWID_MARKER_STOP("grayscale_chained");
}

void gaussian_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const uint16_t *kernel) {

	const uint16_t gauss_ponderation = 16;
	struct pixel_t *cur_img_pixel, *cur_res_pixel;
	size_t i;

	const size_t width = img->width;
	const size_t height = img->height;
	const size_t img_size = width * height * img->components;
	const int32_t offsets[] = OFFSETS(width);
	int32_t sum;

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = res_img->first_pixel;

	LIKWID_MARKER_START("gaussian_chained");
	// populate cache
	for (i = 0; i < img_size; ++i) {
		cache[i] = cur_img_pixel;
		cur_img_pixel = cur_img_pixel->next_pixel;
	}

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = res_img->first_pixel;

	for (i = width; i < img_size - width; ++i) {
		if (IS_BORDER(i, width)) {
			cur_res_pixel->pixel_val[0] = cur_img_pixel->pixel_val[0];
			cur_img_pixel = cur_img_pixel->next_pixel;
			cur_res_pixel = cur_res_pixel->next_pixel;
			continue;
		}

		// apply gaussian filter
		sum = 0;
		for (size_t j = 0; j < 9; ++j) {
			sum += cache[i + offsets[j]]->pixel_val[0] * kernel[j];
		}

		cur_res_pixel->pixel_val[0] = sum / gauss_ponderation;

		cur_img_pixel = cur_img_pixel->next_pixel;
		cur_res_pixel = cur_res_pixel->next_pixel;
	}
	LIKWID_MARKER_STOP("gaussian_chained");
}

void sobel_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const int16_t *v_kernel,
						  const int16_t *h_kernel) {
	struct pixel_t *cur_img_pixel, *cur_res_pixel;
	int16_t gx, gy;
	double_t magnitude;

	size_t i;

	const size_t width = img->width;
	const size_t height = img->height;
	const size_t img_size = width * height;
	const int32_t offsets[] = OFFSETS(width);

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = res_img->first_pixel;

	LIKWID_MARKER_START("sobel_chained");
	// populate cache
	for (i = 0; i < img_size; ++i) {
		cache[i] = cur_img_pixel;
		cur_img_pixel = cur_img_pixel->next_pixel;
	}

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = res_img->first_pixel;

	for (i = width; i < img_size - width; ++i) {
		if (IS_BORDER(i, width)) {
			cur_res_pixel->pixel_val[0] = cur_img_pixel->pixel_val[0];
			cur_img_pixel = cur_img_pixel->next_pixel;
			cur_res_pixel = cur_res_pixel->next_pixel;
			continue;
		}

		gx = 0;
		gy = 0;
		for (size_t j = 0; j < 9; ++j) {
			gx += cache[i + offsets[j]]->pixel_val[0] * h_kernel[j];
			gy += cache[i + offsets[j]]->pixel_val[0] * v_kernel[j];
		}

		magnitude = gx * gx + gy * gy;
		cur_res_pixel->pixel_val[0] = magnitude > SOBEL_BINARY_THRESHOLD ? PIXEL_WHITE : PIXEL_BLACK;

		cur_img_pixel = cur_img_pixel->next_pixel;
		cur_res_pixel = cur_res_pixel->next_pixel;
	}
	LIKWID_MARKER_STOP("sobel_chained");
}