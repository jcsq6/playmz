#pragma once
#include "image.h"
#include "quad.h"
#include <map>

class maze_loader
{
public:
	maze_loader(int x_radius, int y_radius, const rgba_image &maze_img)
		: radius{x_radius, y_radius}, mz{maze_img}
	{
	}

	const std::vector<quad> &meshes() const
	{
		return blocks;
	}

	void load(const glm::vec<2, int> &pos)
	{
		blocks.clear();

		glm::vec<2, int> plus = radius;
		glm::vec<2, int> minus = -radius;

		if (pos.x + radius.x >= mz.image_width())
			plus.x = mz.image_width() - pos.x;
		if (pos.y + radius.y >= mz.image_height())
			plus.y = mz.image_height() - pos.y;

		if (pos.x < radius.x)
			minus.x = -pos.x;
		if (pos.y < radius.y)
			minus.y = -pos.y;

		range horiz;
		std::map<int, range> vert;

		bool is_white;

		for (int y = minus.y; y < plus.y; ++y)
		{
			is_white = mz[pos.y + y][(pos.x + minus.x) * mz.bytes_per_pixel()].col;

			for (int x = minus.x; x < plus.x; ++x)
			{
				//if the current pixel is black
				if (!is_white)
				{
					if (pos.x + x < horiz.min)
						horiz.min = x;

					if (pos.x + x > horiz.max)
						horiz.max = x;

					if (vert.count(x) == 0)
						vert[x] = {};

					if (pos.y + y < vert[x].min)
						vert[x].min = y;

					if (pos.y + y > vert[x].max)
						vert[x].max = y;
				}

				is_white = x + 1 == plus.x || mz[pos.y + y][(pos.x + x + 1) * mz.bytes_per_pixel()].col;
				//if the next horizontal pixel is white and the current is black, then add the range to blocks and remove range (unless the width of the current range is 0)
				if (is_white && horiz.max != std::numeric_limits<int>::min())
				{
					if (horiz.min != horiz.max)
						blocks.emplace_back(glm::vec3(pos.x + horiz.min, 0, pos.y + y), horiz.max - horiz.min + 1, 1, 1);
					horiz = {};
				}

				//if the next vertical pixel is white and the current is black, remove range, also add the range to blocks and (unless the height of the current range is 0)
				if (vert.count(x) && (y + 1 == plus.y || mz[pos.y + y + 1][(pos.x + x) * mz.bytes_per_pixel()].col))
				{
					if (vert[x].min != vert[x].max)
						blocks.emplace_back(glm::vec3(pos.x + x, 0, pos.y + vert[x].min), 1, 1, vert[x].max - vert[x].min);
					vert.erase(x);
				}
			}
		}
	}

private:
	glm::vec<2, int> radius;
	const rgba_image &mz;

	std::vector<quad> blocks;

	struct range
	{
		int min = std::numeric_limits<int>::max();
		int max = std::numeric_limits<int>::min();
	};
};