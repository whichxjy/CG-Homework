#include <cmath>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>

using namespace Eigen;

int main() {
    // Basic Example of cpp
    std::cout << "Example of cpp" << std::endl;
    float a = 1.0;
    float b = 2.0;
    std::cout << a << std::endl;
    std::cout << (a / b) << std::endl;
    std::cout << std::sqrt(b) << std::endl;
    std::cout << std::acos(-1) << std::endl;
    std::cout << std::sin(30.0 / 180.0 * acos(-1)) << std::endl;

    // Example of vector
    std::cout << "Example of vector" << std::endl;

    // vector definition
    Vector3f v(1.0f, 2.0f, 3.0f);
    Vector3f w(1.0f, 0.0f, 0.0f);
    // vector output
    std::cout << "Example of output" << std::endl;

    std::cout << v << std::endl;
    // vector add
    std::cout << "Example of add" << std::endl;

    std::cout << (v + w) << std::endl;
    // vector scalar multiply
    std::cout << "Example of scalar multiply" << std::endl;

    std::cout << (v * 3.0f) << std::endl;
    std::cout << (2.0f * v) << std::endl;

    // Job 1：
    // 实现 v 和 w 的向量点积并输出结果。

    // vector dot product
    std::cout << "Dot product of v and w" << std::endl;
    std::cout << v.dot(w) << std::endl;

    // Example of matrix
    std::cout << "Example of matrix" << std::endl;

    // matrix definition
    Matrix3f i;
    i << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0;
    Matrix3f j;
    j << 2.0, 3.0, 1.0, 4.0, 6.0, 5.0, 9.0, 7.0, 8.0;
    // matrix output
    std::cout << "Example of output" << std::endl;
    std::cout << i << std::endl;

    // Job 2：
    // 实现 i 与 j 的矩阵相加、i 与 2.0 的数乘、i 与 j 的矩阵相乘、i 与 v
    // 的矩阵乘向量，并输出相应的结果。

    // matrix add i + j
    std::cout << "i + j =" << std::endl;
    std::cout << (i + j) << std::endl;

    // matrix scalar multiply i * 2.0
    std::cout << "i * 2.0 =" << std::endl;
    std::cout << (i * 2.0) << std::endl;

    // matrix multiply i * j
    std::cout << "i * j =" << std::endl;
    std::cout << (i * j) << std::endl;

    // matrix multiply vector i * v
    std::cout << "i * v =" << std::endl;
    std::cout << (i * v) << std::endl;

    return 0;
}