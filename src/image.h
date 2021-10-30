#pragma once
#include <png.h>
#include <vector>
#include <cstdio>
#include <cmath>

class rgba_image
{
public:
	struct color
	{
		png_byte col;

		png_byte r() const
		{
			return col >> 6;
		}
		png_byte g() const
		{
			return col << 2 >> 6;
		}
		png_byte b() const
		{
			return col << 4 >> 6;
		}
		png_byte a() const
		{
			return col & 0b00000011;
		}
	};

public:
	rgba_image() {}

	rgba_image(const char *file)
	{
		read_from_file(file);
	}

	void read_from_file(const char *file)
	{
		png_struct *png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		png_info *info = png_create_info_struct(png);

		FILE *p = fopen(file, "rb");
		png_init_io(png, p);

		png_read_info(png, info);

		png_get_IHDR(png, info, &width, &height, &bpp, &color_type, nullptr, nullptr, nullptr);

		if (bpp == 16)
			png_set_strip_16(png);
		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png);
		if (color_type == PNG_COLOR_TYPE_GRAY && bpp < 8)
			png_set_expand_gray_1_2_4_to_8(png);
		if (png_get_valid(png, info, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png);
		if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(png);

		png_read_update_info(png, info);

		color_type = png_get_color_type(png, info);
		bpp = png_get_bit_depth(png, info);

		row_width = png_get_rowbytes(png, info);

		d = std::vector<color>(height * row_width);

		for (int i = 0; i < height; ++i)
		{
			png_read_row(png, reinterpret_cast<png_byte *>(d.data() + i * row_width), nullptr);
		}

		png_read_end(png, info);
		png_destroy_read_struct(&png, &info, nullptr);
		fclose(p);
	}

	size_t size() const
	{
		return d.size();
	}

	color *data()
	{
		return d.data();
	}

	const color *data() const
	{
		return d.data();
	}

	int bytes_per_pixel() const
	{
		return row_width / width;
	}

	color *operator[](unsigned int i)
	{
		return d.data() + i * row_width;
	}

	const color *operator[](unsigned int i) const
	{
		return d.data() + i * row_width;
	}

	int image_color_type() const
	{
		return color_type;
	}

	int bit_depth() const
	{
		return bpp;
	}

	png_uint_32 image_width() const
	{
		return width;
	}

	png_uint_32 image_height() const
	{
		return height;
	}

	size_t row_size() const
	{
		return row_width;
	}

private:
	png_uint_32 width, height;
	int bpp;
	int color_type;

	size_t row_width;

	std::vector<color> d;
};