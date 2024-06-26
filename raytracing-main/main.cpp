//
// Created by Bardio on 22/05/2024.
//

#include <iostream>
#include <vector>

#ifdef _WIN32

#include "sphere.h"
#include "window.h"
#include "algorithm"

#include "bardrix/quaternion.h"
#include <bardrix/ray.h>
#include <bardrix/light.h>
#include <bardrix/camera.h>


///beeep beep beep beepo beepo beepo gleeby sqleeby
/// \brief Calculates the light intensity at a given intersection point
/// \param shape The shape that was intersected
/// \param light The light source
/// \param camera The camera
/// \param intersection_point The intersection point of an object
/// \return The light intensity at the intersection point
/// \example double intensity = calculate_light_intensity(shape, light, camera, intersection_point);
double calculate_light_intensity(const bardrix::shape& shape, const bardrix::light& light, const bardrix::camera& camera,
    const bardrix::point3& intersection_point) {
    const bardrix::vector3 light_intersection_vector = intersection_point.vector_to(light.position).normalized();

    // Angle between the normal and the light intersection vector
    const double angle = shape.normal_at(intersection_point).dot(light_intersection_vector);

    if (angle < 0) // This means the light is behind the intersection_point
        return 0;

    // Specular reflection
    bardrix::vector3 reflection = bardrix::quaternion::mirror(light_intersection_vector,
        shape.normal_at(intersection_point));
    double specular_angle = reflection.dot(camera.position.vector_to(intersection_point).normalized());
    double specular = std::pow(specular_angle, shape.get_material().get_shininess());

    // We're calculating phong shading (ambient + diffuse + specular)
    double intensity = shape.get_material().get_ambient();
    intensity += shape.get_material().get_diffuse() * angle;
    intensity += shape.get_material().get_specular() * specular;

    // Max intensity is 1
    return std::min(1.0, intensity * light.inverse_square_law(intersection_point));
}




int main() {
    int width = 600;
    int height = 600;
    // Create a window
    bardrix::window window("Raytracing", width, height);

    // Create a camera
    bardrix::camera camera = bardrix::camera({0,0,0}, {0,0,1}, width, height, 60);

    // Create a sphere
    std::vector<sphere> spheres;
    sphere s1(1.0, bardrix::point3(0.0, 0.0, 3.0));
    s1.set_material(bardrix::material(0.3, 1, 0.8, 20));

    sphere s2(1.0, { 0.0, 0.0, -3.0 });
    s2.set_material(bardrix::material(0.3, 1, 0.8, 20, bardrix::color::magenta()));
    spheres.push_back(s1);
    spheres.push_back(s2);


    std::vector<bardrix::light> lights{
    bardrix::light({2,1,1}, 1, bardrix::color::cyan()),
    bardrix::light({-2,-1,-1}, 5, bardrix::color::yellow()),
    bardrix::light({1,1,0}, 2, bardrix::color::cyan()),
    };





    window.on_paint = [&camera, &spheres, &lights](bardrix::window* window, std::vector<uint32_t>& buffer) {
        // Draw the sphere
        for (int y = 0; y < window->get_height(); y++) {
            for (int x = 0; x < window->get_width(); x++) {
                bardrix::ray ray = *camera.shoot_ray(x, y, 10);

                bardrix::color color = bardrix::color::black();

                for (sphere s : spheres) {
                    auto intersection = s.intersection(ray);
                    if (intersection.has_value()) {
                        bardrix::color tmpcolor = bardrix::color::black();
                        for (bardrix::light& l : lights) {
                            double intensity = calculate_light_intensity(s, l, camera, intersection.value());
                            tmpcolor += s.get_material().color.blended(l.color) * intensity;
                        }
                        color = tmpcolor;
                    }
                }
                // If the ray intersects the sphere, paint the pixel white
                
                buffer[y * window->get_width() + x] = color.argb(); // ARGB is the format used by Windows API
            }
        }
        //lights[0].position += 0.1;
        lights[1].position.x += 0.1; 
        lights[1].position.y += 0.05;
        lights[1].position.z -= 0.1;
        lights[1].set_intensity(lights[1].get_intensity() + 0.1);
        window->redraw();
    };

    window.on_keydown = [&camera](bardrix::window* window, WPARAM key) {
        constexpr double movement_speed = 0.1; // In units
        constexpr double rotation_speed = 2; // In degrees
        switch (key) {
        case VK_ESCAPE:
            window->close();
            break;
        case 0x57: // W
            camera.position += camera.get_direction() * movement_speed;
            break;
        case 0x41: // A
            camera.position -= camera.get_direction().cross({ 0,1,0 }).normalized() * movement_speed;
            break;
        case 0x53: // S
            camera.position -= camera.get_direction() * movement_speed;
            break;
        case 0x44: // D
            camera.position += camera.get_direction().cross({ 0,1,0 }).normalized() * movement_speed;
            break;
        case VK_SHIFT:
            camera.position -= {0, movement_speed, 0};
            break;
        case VK_SPACE:
            camera.position += {0, movement_speed, 0};
            break;
        case VK_UP:
            camera.set_direction(bardrix::quaternion::rotate_degrees(camera.get_direction(), { 1,0,0 }, rotation_speed));
            break;
        case VK_DOWN:
            camera.set_direction(bardrix::quaternion::rotate_degrees(camera.get_direction(), { 1,0,0 }, -rotation_speed));
            break;
        case VK_LEFT:
            camera.set_direction(bardrix::quaternion::rotate_degrees(camera.get_direction(), { 0,1,0 }, -rotation_speed));
            break;
        case VK_RIGHT:
            camera.set_direction(bardrix::quaternion::rotate_degrees(camera.get_direction(), { 0,1,0 }, rotation_speed));
            break;
        default:
            return;
        }
        window->redraw(); // Redraw the window (calls on_paint)
        };

    window.on_resize = [&camera](bardrix::window* window, int width, int height) {
        // Resize the camera
        camera.set_width(width);
        camera.set_height(height);

        window->redraw(); // Redraw the window (calls on_paint)
    };

    // Get width and height of the screen
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    // Show the window in the center of the screen
    if (!window.show(screen_width / 2 - width / 2, screen_height / 2 - height / 2)) {
        std::cout << GetLastError() << std::endl;
        return -1;
    }

    bardrix::window::run();
}

#else // _WIN32

int main() {
    std::cout << "This example is only available on Windows." << std::endl;
    return 0;
}

#endif // _WIN32