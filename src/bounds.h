#pragma once
#include <glm/vec3.hpp>
#include <limits>
#include <cmath>

enum axis
{
    x = 0,
    y = 1,
    z = 2
};

class bounding_box
{
public:
    template <typename C>
    bounding_box(const C &pts) : min{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}, max{-min}
    {
        for (const auto &pt : pts)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (pt[i] < min[i])
                    min[i] = pt[i];
                if (pt[i] > max[i])
                    max[i] = pt[i];
            }
        }
    }

    bounding_box(const glm::vec3 &pt, const glm::vec3 &dimens) : min{pt}
    {
        for (int i = 0; i < 3; ++i)
        {
            if (dimens[i] < 0)
                min[i] += dimens[i];
        }
        max = min + glm::vec3(std::abs(dimens.x), std::abs(dimens.y), std::abs(dimens.z));
    }

    bounding_box(const bounding_box &o) : min{o.min}, max{o.max} {}

    bounding_box &operator=(const bounding_box &o)
    {
        min = o.min;
        max = o.max;
        return *this;
    }

    void scale(const glm::vec3 &mult)
    {
        max = glm::scale(glm::mat4(1.0), mult) * glm::vec4(max, 1.0f);
        min = glm::scale(glm::mat4(1.0), mult) * glm::vec4(max, 1.0f);
    }

    bounding_box &operator+=(const glm::vec3 &off)
    {
        min += off;
        max += off;
        return *this;
    }

    bounding_box &operator-=(const glm::vec3 &off)
    {
        min -= off;
        max -= off;
        return *this;
    }

    bounding_box &operator/=(float s)
    {
        min /= s;
        max /= s;
        return *this;
    }

    bounding_box &operator*=(float s)
    {
        min *= s;
        max *= s;
        return *this;
    }

    bounding_box operator+(const glm::vec3 &off) const
    {
        return bounding_box{*this} += off;
    }

    bounding_box operator-(const glm::vec3 &off) const
    {
        return bounding_box{*this} -= off;
    }

    bounding_box operator*(float off) const
    {
        return bounding_box{*this} *= off;
    }

    bounding_box operator/(float off) const
    {
        return bounding_box{*this} /= off;
    }

    bounding_box &operator=(const glm::vec3 &new_min)
    {
        glm::vec3 dims = max - min;
        min = new_min;
        max = min + dims;
        return *this;
    }

    bounding_box at(const glm::vec3 &new_min) const
    {
        return bounding_box{*this} = new_min;
    }

    //min is 0, max is 7
    glm::vec3 operator[](int i) const
    {
        switch (i)
        {
        case 0:
            return min;
        case 1:
            return min + glm::vec3(max.x - min.x, 0, 0);
        case 2:
            return min + glm::vec3(max.x - min.x, max.y - min.y, 0);
        case 3:
            return min + glm::vec3(0, max.y - min.y, 0);
        case 4:
            return min + glm::vec3(0, max.y - min.y, max.z - min.z);
        case 5:
            return min + glm::vec3(0, 0, max.z - min.z);
        case 6:
            return min + glm::vec3(max.x - min.x, 0, max.z - min.z);
        case 7:
            return max;
        default:
            throw std::out_of_range("bounding box only has 8 vertices");
        }
    }

    glm::vec3 center() const
    {
        return (min + max) / 2.f;
    }

    //signed overlap
    static float overlap(float min1, float max1, float min2, float max2)
    {
        if (max1 > max2)
            return min1 - max2;
        return max1 - min2;
    }

    static glm::vec3 intersection(const bounding_box &a, const bounding_box &b)
    {
        float min_overlap = INFINITY;
        int min_axis;

        float ap_min;
        float ap_max;
        float bp_min;
        float bp_max;

        float f;

        for (int i = 0; i < 3; ++i)
        {
            //project a and b onto axis, store min and max
            ap_min = INFINITY;
            ap_max = -INFINITY;
            bp_min = INFINITY;
            bp_max = -INFINITY;
            for (int c = 0; c < 8; ++c)
            {
                f = glm::dot(a[c], axes[i]);

                if (f < ap_min)
                    ap_min = f;
                if (f > ap_max)
                    ap_max = f;

                f = glm::dot(b[c], axes[i]);

                if (f < bp_min)
                    bp_min = f;
                if (f > bp_max)
                    bp_max = f;
            }

            f = overlap(ap_min, ap_max, bp_min, bp_max);
            if (std::abs(f) < std::abs(min_overlap))
            {
                min_overlap = f;
                min_axis = i;
            }
        }

        return axes[min_axis] * min_overlap;
    }

    static inline bool collides(float min1, float max1, float min2, float max2)
    {
        return std::max(min1, min2) <= std::min(max1, max2);
    }

    static inline bool collides(const bounding_box &a, const bounding_box &b)
    {
        return collides(a.min.x, a.max.x, b.min.x, b.max.x) &&
               collides(a.min.y, a.max.y, b.min.y, b.max.y) &&
               collides(a.min.z, a.max.z, b.min.z, b.max.z);
    }

    static inline bool collides(const bounding_box &a, const glm::vec3 &pt)
    {
        return (pt.x >= a.min.x && pt.x <= a.max.x) &&
               (pt.y >= a.min.y && pt.y <= a.max.y) &&
               (pt.z >= a.min.z && pt.z <= a.max.z);
    }

public:
    glm::vec3 min;
    glm::vec3 max;

    static constexpr glm::vec<3, glm::vec3> axes{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
};