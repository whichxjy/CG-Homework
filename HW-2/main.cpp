#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "rasterizer.hpp"
#include "triangle.hpp"

constexpr double MY_PI = 3.1415926;

Matrix4f get_view_matrix(Vector3f eye_pos) {
    Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    return translate;
}

Matrix4f get_model_matrix(float rotation_angle) {
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    float rotation_rad = rotation_angle * MY_PI / 180;

    Matrix4f model;
    model << cos(rotation_rad), -sin(rotation_rad), 0, 0, sin(rotation_rad),
        cos(rotation_rad), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;

    return model;
}

Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear,
                               float zFar) {
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    // projection = (persp -> ortho) + ortho

    // persp -> ortho
    Matrix4f p2o;
    p2o << zNear, 0, 0, 0, 0, zNear, 0, 0, 0, 0, zNear + zFar, -zNear * zFar, 0,
        0, 1, 0;

    // ortho = shift + scale

    // shift
    Matrix4f shift;
    shift << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, -(zNear + zFar) / 2, 0, 0, 0, 1;

    // scale
    float eye_rad = eye_fov * MY_PI / 180;

    float height = 2 * zNear * tan(eye_rad / 2);
    float width = height * aspect_ratio;

    Matrix4f scale;
    scale << 2 / width, 0, 0, 0, 0, 2 / height, 0, 0, 0, 0, 2 / (zFar - zNear),
        0, 0, 0, 0, 1;

    return scale * shift * p2o;
}

int main(int argc, const char** argv) {
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]);  // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Vector3f eye_pos = {0, 0, 5};

    std::vector<Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << std::endl;

        if (key == 'a') {
            angle += 10;
        } else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
