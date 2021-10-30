#pragma once
#include <array>
#include <glm/vec3.hpp>
#include <glm/gtx/normal.hpp>
#include <limits>
#include <numeric>
#include "object.h"

class quad : public mesh
{
public:
	template <typename C>
	quad(const C &pts)
	{
		vs.resize(24);
		is.resize(36);

		c = std::accumulate(pts.begin(), pts.end(), glm::vec3(0, 0, 0)) / static_cast<float>(pts.size());

		glm::vec3 min(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
		glm::vec3 max = -min;
		for (int i = 0; i < pts.size(); ++i)
		{
			if (pts[i].x < min.x)
				min.x = pts[i].x;
			if (pts[i].y < min.y)
				min.y = pts[i].y;
			if (pts[i].z < min.z)
				min.z = pts[i].z;

			if (pts[i].x > max.x)
				max.x = pts[i].x;
			if (pts[i].y > max.y)
				max.y = pts[i].y;
			if (pts[i].z > max.z)
				max.z = pts[i].z;
		}

		//faces[0] smallest x's
		//faces[1] largest x's
		//faces[2] smallest y's
		//faces[3] largest y's
		//faces[4] smallest z's
		//faces[5] largest z's

		std::array<std::array<int, 4>, 6> faces;
		std::array<int, 6> count;
		count.fill(0);

		for (int i = 0; i < pts.size(); ++i)
		{
			if (pts[i].x == min.x)
			{
				faces[0][count[0]] = i;
				++count[0];
			}
			if (pts[i].x == max.x)
			{
				faces[1][count[1]] = i;
				++count[1];
			}
			if (pts[i].y == min.y)
			{
				faces[2][count[2]] = i;
				++count[2];
			}
			if (pts[i].y == max.y)
			{
				faces[3][count[3]] = i;
				++count[3];
			}
			if (pts[i].z == min.z)
			{
				faces[4][count[4]] = i;
				++count[4];
			}
			if (pts[i].z == max.z)
			{
				faces[5][count[5]] = i;
				++count[5];
			}

			//put vs in vs array
			vs[i * 3 + 0] = pts[i].x;
			vs[i * 3 + 1] = pts[i].y;
			vs[i * 3 + 2] = pts[i].z;
		}
		int cur_ind;
		for (int f = 0; f < faces.size(); ++f)
		{
			cur_ind = f * 6;
			//if the starting point of the triangle (0) is in the middle of the other points, swap it
			if (is_in_middle(pts[faces[f][0]], pts[faces[f][1]], pts[faces[f][2]]))
			{
				std::swap(faces[f][0], faces[f][2]);
			}

			//if the middle point of the triangle (1) is not in the middle of the other two points, swap it
			if (!is_in_middle(pts[faces[f][1]], pts[faces[f][0]], pts[faces[f][2]]))
			{
				std::swap(faces[f][1], faces[f][2]);
			}

			is[cur_ind + 0] = faces[f][0];
			is[cur_ind + 1] = faces[f][1];
			is[cur_ind + 2] = faces[f][2];
			if (glm::dot(
					glm::triangleNormal(pts[faces[f][0]], pts[faces[f][1]], pts[faces[f][2]]),
					(pts[faces[f][0]] + pts[faces[f][1]] + pts[faces[f][2]]) / 3.0f - c) < 0)
			{
				std::swap(is[cur_ind], is[cur_ind + 2]);
			}

			is[cur_ind + 3] = faces[f][2];
			is[cur_ind + 4] = faces[f][3];
			is[cur_ind + 5] = faces[f][0];

			if (glm::dot(
					glm::triangleNormal(pts[faces[f][2]], pts[faces[f][3]], pts[faces[f][0]]),
					(pts[faces[f][2]] + pts[faces[f][3]] + pts[faces[f][0]]) / 3.0f - c) < 0)
			{
				std::swap(is[cur_ind + 3], is[cur_ind + 5]);
			}
		}
	}

	quad(const glm::vec3 &pt, float xwidth, float yheight, float zlength)
	{
		std::array<glm::vec3, 8> pts;
		pts[0] = pt;
		pts[1] = pt + glm::vec3{xwidth, 0, 0};
		pts[2] = pt + glm::vec3{xwidth, yheight, 0};
		pts[3] = pt + glm::vec3{0, yheight, 0};

		pts[4] = pt + glm::vec3{0, 0, zlength};
		pts[5] = pt + glm::vec3{xwidth, 0, zlength};
		pts[6] = pt + glm::vec3{xwidth, yheight, zlength};
		pts[7] = pt + glm::vec3{0, yheight, zlength};

		*this = quad{pts};
	}

private:
	static bool is_in_middle(const glm::vec3 &mid, const glm::vec3 &c1, const glm::vec3 &c2)
	{
		if (mid.x == c1.x && mid.x == c2.x)
			return (mid.z == c1.z && mid.y == c2.y) || (mid.z == c2.z && mid.y == c1.y);
		else if (mid.y == c1.y && mid.y == c2.y)
			return (mid.x == c1.x && mid.z == c2.z) || (mid.x == c2.x && mid.z == c1.z);
		else if (mid.z == c1.z && mid.z == c2.z)
			return (mid.x == c1.x && mid.y == c2.y) || (mid.x == c2.x && mid.y == c1.y);
		throw std::logic_error{"No common axes"};
	}
};