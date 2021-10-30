#pragma once
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

class camera : public glm::vec3
{
public:
    camera(float x, float y, float z) : glm::vec3{x, y, z}, cam_dir{0, 0, 0}, cam_right{1, 0, 0}, cam_up{0, 1, 0} {}

    camera &operator=(const glm::vec3 &new_pos)
    {
        this->x = new_pos.x;
        this->y = new_pos.y;
        this->z = new_pos.z;
        return *this;
    }

    void update_view_mat()
    {
        view = glm::lookAt(*this, *this - cam_dir, glm::vec3(0, 1, 0));
    }

    void update_proj_mat(int scrn_width, int scrn_height)
    {
        aspect = (float)scrn_width / scrn_height;
        proj = glm::perspective(fov, aspect, clip_near, clip_far);
    }

    glm::vec3 near_off() const
    {
        return cam_dir * clip_near;
    }

    glm::vec3 far_off() const
    {
        return cam_dir * clip_far;
    }

    float near_dist() const
    {
        return clip_near;
    }

    float far_dist() const
    {
        return clip_far;
    }

    float near_height() const
    {
        return 2 * tan(fov / 2) * near_dist();
    }

    float near_width() const
    {
        return near_height() * aspect;
    }

    float far_height() const
    {
        return 2 * tan(fov / 2) * far_dist();
    }

    float far_width() const
    {
        return far_height() * aspect;
    }

    void look_at(const glm::vec3 &pos)
    {
        set_dir(*this - pos);
    }

    void look_at(float x, float y, float z)
    {
        set_dir(*this - glm::vec3(x, y, z));
    }

    void set_dir(const glm::vec3 &new_dir)
    {
        cam_dir = new_dir / glm::length(new_dir);
        update_axes();
    }

    void set_dir(float x, float y, float z)
    {
        cam_dir = glm::vec3(x, y, z);
        cam_dir /= glm::length(cam_dir);
        update_axes();
    }

    void add_to_dir(float x, float y, float z)
    {
        set_dir(cam_dir + glm::vec3(x, y, z));
    }

    void add_to_dir(const glm::vec3 &a)
    {
        set_dir(cam_dir + a);
    }

    void rotate(float radians, const glm::vec3 &axis)
    {
        cam_dir = glm::rotate(cam_dir, radians, axis);
        update_axes();
    }

    const glm::vec3 &dir() const
    {
        return cam_dir;
    }

    const glm::vec3 &right() const
    {
        return cam_right;
    }

    const glm::vec3 &camera_up() const
    {
        return cam_up;
    }

    static inline constexpr glm::vec3 up()
    {
        return glm::vec3(0, 1, 0);
    }

    const glm::mat4 &view_matrix() const
    {
        return view;
    }

    const glm::mat4 &proj_matrix() const
    {
        return proj;
    }

public:
    float clip_near = .1f;
    float clip_far = 1000.f;

    float fov = glm::pi<float>() / 2;

private:
    float aspect;

    glm::vec3 cam_dir;
    glm::vec3 cam_right;
    glm::vec3 cam_up;

    glm::mat4 view;
    glm::mat4 proj;

    void update_axes()
    {
        cam_right = glm::normalize(glm::cross(up(), cam_dir));
        cam_up = glm::cross(cam_dir, cam_right);
    }
};