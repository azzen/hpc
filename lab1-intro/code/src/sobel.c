#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "sobel.h"

#define GAUSSIAN_KERNEL_SIZE 3
#define SOBEL_KERNEL_SIZE 3
#define SOBEL_BINARY_THRESHOLD 150 // in the range 0 to uint8_max (255)

#define CONVOLUTION_1D(img, kernel, position)                                                                          \
	(img->data[position - img->width - 1] * kernel[0] + img->data[position - img->width] * kernel[1] +                 \
	 img->data[position - img->width + 1] * kernel[2] + img->data[position - 1] * kernel[3] +                          \
	 img->data[position] * kernel[4] + img->data[position + 1] * kernel[5] +                                           \
	 img->data[position + img->width - 1] * kernel[6] + img->data[position + img->width] * kernel[7] +                 \
	 img->data[position + img->width + 1] * kernel[8])

#define IS_BORDER(img, position)                                                                                       \
	(position < img->width || position > img->width * (img->height - 1) || position % img->width == 0 ||               \
	 position % img->width == img->width - 1)

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
	struct img_1D_t *grayscale_img;
	struct img_1D_t *gauss_img;

	res_img = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	grayscale_img = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	gauss_img = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);

	rgb_to_grayscale_1D(input_img, grayscale_img);
	gaussian_filter_1D(grayscale_img, gauss_img, gauss_kernel);
	sobel_filter_1D(gauss_img, res_img, sobel_v_kernel, sobel_h_kernel);

	free_image(grayscale_img);
	free_image(gauss_img);

	return res_img;
}

void rgb_to_grayscale_1D(const struct img_1D_t *img, struct img_1D_t *result) {
	uint8_t r, g, b;
	size_t i;

	for (i = 0; i < img->width * img->height * img->components; i += img->components) {
		r = img->data[i + R_OFFSET] * FACTOR_R;
		g = img->data[i + G_OFFSET] * FACTOR_G;
		b = img->data[i + B_OFFSET] * FACTOR_B;
		result->data[i / img->components] = r + g + b;
	}
}

void gaussian_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const uint16_t *kernel) {
	const uint16_t gauss_ponderation = 16;
	size_t i;

	for (i = 0; i < img->width * img->height * img->components; ++i) {
		if (IS_BORDER(img, i)) {
			res_img->data[i] = img->data[i];
			continue;
		}
		// apply gaussian filter
		res_img->data[i] = CONVOLUTION_1D(img, kernel, i) / gauss_ponderation;
	}
}

void sobel_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const int16_t *v_kernel,
					 const int16_t *h_kernel) {
	size_t i;
	int16_t gx, gy;
	double_t magnitude;

	for (i = 0; i < img->height * img->width * img->components; ++i) {
		if (IS_BORDER(img, i)) {
			res_img->data[i] = img->data[i];
			continue;
		}

		gx = CONVOLUTION_1D(img, h_kernel, i);
		gy = CONVOLUTION_1D(img, v_kernel, i);
		magnitude = sqrt(gx * gx + gy * gy);
		res_img->data[i] = magnitude > SOBEL_BINARY_THRESHOLD ? PIXEL_WHITE : PIXEL_BLACK;
	}
}

struct img_chained_t *edge_detection_chained(const struct img_chained_t *input_img) {
	struct img_chained_t *res_img;
	struct img_chained_t *grayscale_img;
	struct img_chained_t *gauss_img;

	res_img = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	grayscale_img = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
	gauss_img = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);

	rgb_to_grayscale_chained(input_img, grayscale_img);
	gaussian_filter_chained(grayscale_img, gauss_img, gauss_kernel);

	return gauss_img;
}

void rgb_to_grayscale_chained(const struct img_chained_t *img, struct img_chained_t *result) {
	struct pixel_t *cur_img_pixel;
	struct pixel_t *cur_res_pixel;
	uint8_t r, g, b;

	cur_img_pixel = img->first_pixel;
	cur_res_pixel = result->first_pixel;

	do {
		r = cur_img_pixel->pixel_val[R_OFFSET] * FACTOR_R;
		g = cur_img_pixel->pixel_val[G_OFFSET] * FACTOR_G;
		b = cur_img_pixel->pixel_val[B_OFFSET] * FACTOR_B;

		cur_res_pixel->pixel_val[0] = r + g + b;

		cur_img_pixel = cur_img_pixel->next_pixel;
		cur_res_pixel = cur_res_pixel->next_pixel;

	} while (cur_img_pixel != NULL);
}

void gaussian_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const uint16_t *kernel) {

	const uint16_t gauss_ponderation = 16;
	struct pixel_t *cur_img_pixel, *cur_res_pixel;
	size_t i;
	// TODO
}

void sobel_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const int16_t *v_kernel,
						  const int16_t *h_kernel) {
	// TODO
}
