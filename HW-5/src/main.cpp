#include <iostream>

#include "camera.hpp"
#include "color.hpp"
#include "hittable.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "util.hpp"

HittableList random_scene() {
    HittableList world;

    auto ground_material = std::make_shared<Lambertian>(Vec3(0.5, 0.5, 0.5));
    world.add(
        std::make_shared<Sphere>(Vec3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            Vec3 center(a + 0.9 * random_double(), 0.2,
                        b + 0.9 * random_double());

            if ((center - Vec3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = Vec3::random() * Vec3::random();
                    sphere_material = std::make_shared<Lambertian>(albedo);
                    world.add(
                        std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Vec3::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<Metal>(albedo, fuzz);
                    world.add(
                        std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<Dielectric>(1.5);
                    world.add(
                        std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<Dielectric>(1.5);
    world.add(std::make_shared<Sphere>(Vec3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<Lambertian>(Vec3(0.4, 0.2, 0.1));
    world.add(std::make_shared<Sphere>(Vec3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<Metal>(Vec3(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<Sphere>(Vec3(4, 1, 0), 1.0, material3));

    return world;
}

Vec3 ray_color(const Ray& ray, const Hittable& world, int depth) {
    if (depth <= 0) {
        return Vec3(0, 0, 0);
    }

    HitRecord record;

    if (world.hit(ray, 0.001, infinity, record)) {
        Ray scattered;
        Vec3 attenuation;

        if (record.material->scatter(ray, record, attenuation, scattered)) {
            return attenuation * ray_color(scattered, world, depth - 1);
        }

        return Vec3(0, 0, 0);
    }

    Vec3 unit_direction = unit_vector(ray.direction);
    double t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
}

int main() {
    double aspect_ratio = 16.0 / 9.0;

    Vec3 lookfrom(13, 2, 3);
    Vec3 lookat(0, 0, 0);
    Vec3 vup(0, 1, 0);
    double dist_to_focus = 10.0;
    double aperture = 0.1;

    Camera camera(lookfrom, lookat, vup, 20, aspect_ratio, aperture,
                  dist_to_focus);

    int image_width = 384;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 100;
    int max_depth = 50;

    std::cout << "P3" << std::endl
              << image_width << ' ' << image_height << std::endl
              << "255" << std::endl;

    HittableList world = random_scene();

    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            Vec3 pixel_color(0, 0, 0);

            for (int s = 0; s < samples_per_pixel; ++s) {
                double u = (i + random_double()) / (image_width - 1);
                double v = (j + random_double()) / (image_height - 1);
                Ray ray = camera.get_ray(u, v);
                pixel_color += ray_color(ray, world, max_depth);
            }

            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }
}