#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "sobel.h"

#include <immintrin.h>

#define GAUSSIAN_KERNEL_SIZE 3
#define SOBEL_KERNEL_SIZE 3
#define SOBEL_BINARY_THRESHOLD 22500 // 150 * 150

#define IS_BORDER(position, width) (position == 0 || position == width - 1)

#define EXPAND_u8_float(arr, scale, offset)                                                                            \
	(float)arr[7 * scale + offset], (float)arr[6 * scale + offset], (float)arr[5 * scale + offset],                    \
		(float)arr[4 * scale + offset], (float)arr[3 * scale + offset], (float)arr[2 * scale + offset],                \
		(float)arr[1 * scale + offset], (float)arr[0 * scale + offset]

#define EXPAND(arr, scale, offset)                                                                                     \
	arr[7 * scale + offset], arr[6 * scale + offset], arr[5 * scale + offset], arr[4 * scale + offset],                \
		arr[3 * scale + offset], arr[2 * scale + offset], arr[1 * scale + offset], arr[0 * scale + offset]

#define EXPAND_u8_u32(arr, scale, offset)                                                                              \
	(uint32_t) arr[7 * scale + offset], (uint32_t)arr[6 * scale + offset], (uint32_t)arr[5 * scale + offset],          \
		(uint32_t)arr[4 * scale + offset], (uint32_t)arr[3 * scale + offset], (uint32_t)arr[2 * scale + offset],       \
		(uint32_t)arr[1 * scale + offset], (uint32_t)arr[0 * scale + offset]

#define OFFSETS(width)                                                                                                 \
	{ -width - 1, -width, -width + 1, -1, 0, 1, width - 1, width, width + 1 }

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

	rgb_to_grayscale_1D_avx(input_img, res_img);
	gaussian_filter_1D(res_img, temp_img, gauss_kernel);
	sobel_filter_1D(temp_img, res_img, sobel_v_kernel, sobel_h_kernel);
	free_image(temp_img);

	return res_img;
}

void rgb_to_grayscale_1D(const struct img_1D_t *img, struct img_1D_t *result) {
	uint8_t r, g, b;
	size_t i;

	size_t img_size = img->width * img->height * img->components;
	size_t num_components = img->components;

	for (i = 0; i < img_size; i += num_components) {
		r = img->data[i + R_OFFSET] * FACTOR_R;
		g = img->data[i + G_OFFSET] * FACTOR_G;
		b = img->data[i + B_OFFSET] * FACTOR_B;
		result->data[i / num_components] = r + g + b;
	}
}

void rgb_to_grayscale_1D_avx(const struct img_1D_t *img, struct img_1D_t *result) {
	size_t i;
	size_t img_size = img->width * img->height;
	size_t num_comp = img->components;
	size_t rem;

	__m256 factor_r = _mm256_set1_ps(FACTOR_R);
	__m256 factor_g = _mm256_set1_ps(FACTOR_G);
	__m256 factor_b = _mm256_set1_ps(FACTOR_B);

	rem = img_size - (img_size % 8);
	for (i = 0; i < rem; i += 8) {
		// load 8 pixels
		uint8_t *base = img->data + i * num_comp;
		uint8_t *base_res = result->data + i;

		__m256 p_r = _mm256_set_ps(EXPAND_u8_float(base, num_comp, R_OFFSET));
		__m256 p_g = _mm256_set_ps(EXPAND_u8_float(base, num_comp, G_OFFSET));
		__m256 p_b = _mm256_set_ps(EXPAND_u8_float(base, num_comp, B_OFFSET));

		__m256 r_res = _mm256_mul_ps(p_r, factor_r);
		__m256 g_res = _mm256_mul_ps(p_g, factor_g);
		__m256 b_res = _mm256_mul_ps(p_b, factor_b);

		__m256 sums = _mm256_add_ps(r_res, g_res);
		sums = _mm256_add_ps(sums, b_res);
		float res[8] = {0};
		_mm256_storeu_ps(res, sums);

		base_res[0] = res[0];
		base_res[1] = res[1];
		base_res[2] = res[2];
		base_res[3] = res[3];
		base_res[4] = res[4];
		base_res[5] = res[5];
		base_res[6] = res[6];
		base_res[7] = res[7];
	}

	for (i = rem; i < img_size; ++i) {
		float r, g, b;
		r = img->data[i * num_comp + R_OFFSET] * FACTOR_R;
		g = img->data[i * num_comp + G_OFFSET] * FACTOR_G;
		b = img->data[i * num_comp + B_OFFSET] * FACTOR_B;
		result->data[i] = (uint8_t)(r + g + b);
	}
}

void gaussian_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const uint16_t *kernel) {
	const uint16_t gauss_ponderation = 16;
	size_t i;

	const size_t width = img->width;
	const size_t height = img->height;
	const size_t img_size = width * height;
	const int32_t offsets[] = OFFSETS(width);

	// skip bottom and top borders
	for (i = width; i < img_size - width; ++i) {
		uint32_t pos = i % width;
		if (IS_BORDER(pos, width)) {
			res_img->data[i] = img->data[i];
			continue;
		}

		// res_img->data[i] = (val[0] + img->data[i + offsets[8]] * kernel[8]) / gauss_ponderation;
		res_img->data[i] = (img->data[i + offsets[0]] * kernel[0] + img->data[i + offsets[1]] * kernel[1] +
							img->data[i + offsets[2]] * kernel[2] + img->data[i + offsets[3]] * kernel[3] +
							img->data[i + offsets[4]] * kernel[4] + img->data[i + offsets[5]] * kernel[5] +
							img->data[i + offsets[6]] * kernel[6] + img->data[i + offsets[7]] * kernel[7] +
							img->data[i + offsets[8]] * kernel[8]) /
						   gauss_ponderation;
	}
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

	for (i = width; i < img_size - width; ++i) {
		uint32_t pos = i % width;
		if (IS_BORDER(pos, width)) {
			res_img->data[i] = img->data[i];
			continue;
		}

		gx = img->data[i + offsets[0]] * h_kernel[0] + img->data[i + offsets[1]] * h_kernel[1] +
			 img->data[i + offsets[2]] * h_kernel[2] + img->data[i + offsets[3]] * h_kernel[3] +
			 img->data[i + offsets[4]] * h_kernel[4] + img->data[i + offsets[5]] * h_kernel[5] +
			 img->data[i + offsets[6]] * h_kernel[6] + img->data[i + offsets[7]] * h_kernel[7] +
			 img->data[i + offsets[8]] * h_kernel[8];

		gy = img->data[i + offsets[0]] * v_kernel[0] + img->data[i + offsets[1]] * v_kernel[1] +
			 img->data[i + offsets[2]] * v_kernel[2] + img->data[i + offsets[3]] * v_kernel[3] +
			 img->data[i + offsets[4]] * v_kernel[4] + img->data[i + offsets[5]] * v_kernel[5] +
			 img->data[i + offsets[6]] * v_kernel[6] + img->data[i + offsets[7]] * v_kernel[7] +
			 img->data[i + offsets[8]] * v_kernel[8];

		magnitude = gx * gx + gy * gy;

		res_img->data[i] = magnitude > SOBEL_BINARY_THRESHOLD ? PIXEL_WHITE : PIXEL_BLACK;
	}
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
	rgb_to_grayscale_chained(input_img, res_img);
	gaussian_filter_chained(res_img, temp_img, gauss_kernel);
	sobel_filter_chained(temp_img, res_img, sobel_v_kernel, sobel_h_kernel);
	free(cache);

	return res_img;
}

void rgb_to_grayscale_chained(const struct img_chained_t *img, struct img_chained_t *result) {
	struct pixel_t *cur_img_pixel, *cur_res_pixel;

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = result->first_pixel;

	do {
		cur_res_pixel->pixel_val[0] = cur_img_pixel->pixel_val[R_OFFSET] * FACTOR_R +
									  cur_img_pixel->pixel_val[G_OFFSET] * FACTOR_G +
									  cur_img_pixel->pixel_val[B_OFFSET] * FACTOR_B;

		cur_img_pixel = cur_img_pixel->next_pixel;
		cur_res_pixel = cur_res_pixel->next_pixel;

	} while (cur_img_pixel != NULL);
}

void gaussian_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const uint16_t *kernel) {

	const uint16_t gauss_ponderation = 16;
	struct pixel_t *cur_img_pixel, *cur_res_pixel;
	size_t i;

	const size_t width = img->width;
	const size_t height = img->height;
	const size_t img_size = width * height * img->components;
	const int32_t offsets[] = OFFSETS(width);

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = res_img->first_pixel;

	// populate cache
	for (i = 0; i < img_size; ++i) {
		cache[i] = cur_img_pixel;
		cur_img_pixel = cur_img_pixel->next_pixel;
	}

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = res_img->first_pixel;

	for (i = width; i < img_size - width; ++i) {
		uint32_t pos = i % width;
		if (IS_BORDER(pos, width)) {
			cur_res_pixel->pixel_val[0] = cur_img_pixel->pixel_val[0];
			cur_img_pixel = cur_img_pixel->next_pixel;
			cur_res_pixel = cur_res_pixel->next_pixel;
			continue;
		}

		// apply gaussian filter
		cur_res_pixel->pixel_val[0] =
			(cache[i + offsets[0]]->pixel_val[0] * kernel[0] + cache[i + offsets[1]]->pixel_val[0] * kernel[1] +
			 cache[i + offsets[2]]->pixel_val[0] * kernel[2] + cache[i + offsets[3]]->pixel_val[0] * kernel[3] +
			 cache[i + offsets[4]]->pixel_val[0] * kernel[4] + cache[i + offsets[5]]->pixel_val[0] * kernel[5] +
			 cache[i + offsets[6]]->pixel_val[0] * kernel[6] + cache[i + offsets[7]]->pixel_val[0] * kernel[7] +
			 cache[i + offsets[8]]->pixel_val[0] * kernel[8]) /
			gauss_ponderation;

		cur_img_pixel = cur_img_pixel->next_pixel;
		cur_res_pixel = cur_res_pixel->next_pixel;
	}
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

	// populate cache
	for (i = 0; i < img_size; ++i) {
		cache[i] = cur_img_pixel;
		cur_img_pixel = cur_img_pixel->next_pixel;
	}

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = res_img->first_pixel;

	for (i = width; i < img_size - width; ++i) {
		uint32_t pos = i % width;
		if (IS_BORDER(pos, width)) {
			cur_res_pixel->pixel_val[0] = cur_img_pixel->pixel_val[0];
			cur_img_pixel = cur_img_pixel->next_pixel;
			cur_res_pixel = cur_res_pixel->next_pixel;
			continue;
		}

		gx = cache[i + offsets[0]]->pixel_val[0] * h_kernel[0] + cache[i + offsets[1]]->pixel_val[0] * h_kernel[1] +
			 cache[i + offsets[2]]->pixel_val[0] * h_kernel[2] + cache[i + offsets[3]]->pixel_val[0] * h_kernel[3] +
			 cache[i + offsets[4]]->pixel_val[0] * h_kernel[4] + cache[i + offsets[5]]->pixel_val[0] * h_kernel[5] +
			 cache[i + offsets[6]]->pixel_val[0] * h_kernel[6] + cache[i + offsets[7]]->pixel_val[0] * h_kernel[7] +
			 cache[i + offsets[8]]->pixel_val[0] * h_kernel[8];

		gy = cache[i + offsets[0]]->pixel_val[0] * v_kernel[0] + cache[i + offsets[1]]->pixel_val[0] * v_kernel[1] +
			 cache[i + offsets[2]]->pixel_val[0] * v_kernel[2] + cache[i + offsets[3]]->pixel_val[0] * v_kernel[3] +
			 cache[i + offsets[4]]->pixel_val[0] * v_kernel[4] + cache[i + offsets[5]]->pixel_val[0] * v_kernel[5] +
			 cache[i + offsets[6]]->pixel_val[0] * v_kernel[6] + cache[i + offsets[7]]->pixel_val[0] * v_kernel[7] +
			 cache[i + offsets[8]]->pixel_val[0] * v_kernel[8];

		magnitude = gx * gx + gy * gy;
		cur_res_pixel->pixel_val[0] = magnitude > SOBEL_BINARY_THRESHOLD ? PIXEL_WHITE : PIXEL_BLACK;

		cur_img_pixel = cur_img_pixel->next_pixel;
		cur_res_pixel = cur_res_pixel->next_pixel;
	}
}