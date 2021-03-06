#include "rasterizer.hpp"

#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <vector>

rst::pos_buf_id rst::rasterizer::load_positions(
    const std::vector<Vector3f>& positions) {
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(
    const std::vector<Vector3i>& indices) {
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(
    const std::vector<Vector3f>& cols) {
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Vector3f& v3, float w = 1.0f) {
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

static float cross2D(float x1, float y1, float x2, float y2) {
    return x1 * y2 - x2 * y1;
}

static float sign(int x, int y, const Vector3f& p2, const Vector3f& p3) {
    return (x - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (y - p3.y());
}

// https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
static bool insideTriangle(int x, int y, const Vector3f* _v) {
    // TODO 2: Implement this function to check if the point (x, y) is inside
    // the triangle represented by _v[0], _v[1], _v[2]

    float d1 = sign(x, y, _v[0], _v[1]);
    float d2 = sign(x, y, _v[1], _v[2]);
    float d3 = sign(x, y, _v[2], _v[0]);

    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);

    return !(has_pos && has_neg);
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y,
                                                            const Vector3f* v) {
    float c1 =
        (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y +
         v[1].x() * v[2].y() - v[2].x() * v[1].y()) /
        (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() +
         v[1].x() * v[2].y() - v[2].x() * v[1].y());
    float c2 =
        (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y +
         v[2].x() * v[0].y() - v[0].x() * v[2].y()) /
        (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() +
         v[2].x() * v[0].y() - v[0].x() * v[2].y());
    float c3 =
        (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y +
         v[0].x() * v[1].y() - v[1].x() * v[0].y()) /
        (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() +
         v[0].x() * v[1].y() - v[1].x() * v[0].y());
    return {c1, c2, c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer,
                           col_buf_id col_buffer, Primitive type) {
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Matrix4f mvp = projection * view * model;
    for (auto& i : ind) {
        Triangle t;
        Vector4f v[] = {mvp * to_vec4(buf[i[0]], 1.0f),
                        mvp * to_vec4(buf[i[1]], 1.0f),
                        mvp * to_vec4(buf[i[2]], 1.0f)};
        // Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        // Viewport transformation
        for (auto& vert : v) {
            vert.x() = 0.5 * width * (vert.x() + 1.0);
            vert.y() = 0.5 * height * (vert.y() + 1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i) {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

// Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();

    float xmin = FLT_MAX, xmax = FLT_MIN;
    float ymin = FLT_MAX, ymax = FLT_MIN;

    // TODO 1: Find out the bounding box of current triangle.
    for (int i = 0; i < 3; i++) {
        xmin = fmin(xmin, t.v[i].x());
        xmax = fmax(xmax, t.v[i].x());
        ymin = fmin(ymin, t.v[i].y());
        ymax = fmax(ymax, t.v[i].y());
    }

    // iterate through the pixel and find if the current pixel is inside the
    // triangle
    for (int x = static_cast<int>(xmin); x <= xmax; ++x) {
        for (int y = static_cast<int>(ymin); y <= ymax; ++y) {
            // if it's not in the area of current triangle, just do nothing.
            if (!insideTriangle(x, y, t.v)) continue;
            // otherwise we need to do z-buffer testing.

            // use the following code to get the depth value of pixel (x,y),
            // it's stored in z_interpolated
            auto [alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
            float w_reciprocal =
                1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            float z_interpolated = alpha * v[0].z() / v[0].w() +
                                   beta * v[1].z() / v[1].w() +
                                   gamma * v[2].z() / v[2].w();
            z_interpolated *= w_reciprocal;

            // TODO 3: Perform Z-buffer algorithm here.
            int index = get_index(x, y);

            if (-z_interpolated < depth_buf[index]) {
                // set the pixel color to frame buffer.
                frame_buf[index] = 255.0f * t.color[0];
                depth_buf[index] = -z_interpolated;
            }
        }
    }
}

void rst::rasterizer::set_model(const Matrix4f& m) { model = m; }

void rst::rasterizer::set_view(const Matrix4f& v) { view = v; }

void rst::rasterizer::set_projection(const Matrix4f& p) { projection = p; }

void rst::rasterizer::clear(rst::Buffers buff) {
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
        std::fill(depth_buf.begin(), depth_buf.end(),
                  std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h) {
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y) {
    return (height - 1 - y) * width + x;
}

void rst::rasterizer::set_pixel(const Vector3f& point, const Vector3f& color) {
    // old index: auto ind = point.y() + point.x() * width;
    auto ind = (height - 1 - point.y()) * width + point.x();
    frame_buf[ind] = color;
}
