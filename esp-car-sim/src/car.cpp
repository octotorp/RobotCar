#include "car.h"

// --- TEST ---
#include <godot_cpp/classes/input.hpp>
#include <iostream>
#include <fstream>
// --- TEST ---

using namespace godot;

Car::Car() {};
Car::~Car() {};

void Car::_bind_methods() {}

void Car::_ready()
{
	if (Engine::get_singleton()->is_editor_hint()) return;

	ray_front = get_node<RayCast2D>("Rays/RayFront");
	ray_back = get_node<RayCast2D>("Rays/RayBack");
	ray_left = get_node<RayCast2D>("Rays/RayLeft");
	ray_right = get_node<RayCast2D>("Rays/RayRight");
	ray_front->add_exception(this);
	ray_back->add_exception(this);
	ray_left->add_exception(this);
	ray_right->add_exception(this);

	target_marker = get_node<ColorRect>("TargetMarker");
}

void Car::_physics_process(double delta)
{
	if (Engine::get_singleton()->is_editor_hint()) return;

	double d_front = get_sensor_distance(ray_front);
	double d_back = get_sensor_distance(ray_back);
	double d_left = get_sensor_distance(ray_left);
	double d_right = get_sensor_distance(ray_right);
	// --- TEST ---
	Input* input = Input::get_singleton();
	double drive = input->get_axis("ui_down", "ui_up");
	double turn = input->get_axis("ui_left", "ui_right");

	if (target_marker) {
		// -4.0 робимо для того, щоб крапка розміром 8х8 була відцентрована:
		Vector2 marker_pos = Vector2(drive * 60.0 - 4.0, turn * 60.0 - 4.0);
		target_marker->set_position(marker_pos);
	}

	double left_power = Math::clamp(drive + turn, -1.0, 1.0);
	double right_power = Math::clamp(drive - turn, -1.0, 1.0);

	apply_wheel_force(POS_FL, left_power);
	apply_wheel_force(POS_BL, left_power);

	apply_wheel_force(POS_FR, right_power);
	apply_wheel_force(POS_BR, right_power);


	if (input->is_key_pressed(Key::KEY_G))
	{
		std::ofstream file("output.txt");
		if (!(file.is_open())) return;

		file << "DATA:\n"
			<< "d_front: " << d_front << ";\n"
			<< "d_back: " << d_back << ";\n"
			<< "d_left: " << d_left << ";\n"
			<< "d_right: " << d_right << ";\n"
			;
		file.close();
	}
	// --- TEST --
}

double Car::get_sensor_distance(RayCast2D* ray)
{
	if (!ray || !(ray->is_colliding())) return 999.;
	return ray->get_global_position().distance_to(ray->get_collision_point());
}

void Car::apply_wheel_force(Vector2 pos, double throttle)
{
	if (abs(throttle) > 1. || throttle == 0.0) return;

	Vector2 forward_dir = get_transform()[0];
	Vector2 force = forward_dir * (throttle * motor_force);
	Vector2 offset = pos.rotated(get_rotation());

	apply_force(force, offset);
}