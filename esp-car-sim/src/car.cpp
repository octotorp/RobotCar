#include "car.h"
#include <godot_cpp/classes/physics_server2d.hpp>

// --- TEST ---
#include <godot_cpp/classes/input.hpp>
#include <iostream>
#include <fstream>
// --- TEST ---

using namespace godot;

Car::Car() {};
Car::~Car() {};

void Car::_bind_methods() 
{
	ClassDB::bind_method(D_METHOD("reset_state", "start_pos", "start_rot"), &Car::reset_state);
	ClassDB::bind_method(D_METHOD("get_fitness"), &Car::get_fitness);
	ClassDB::bind_method(D_METHOD("add_fitness", "amount"), &Car::add_fitness);
	ClassDB::bind_method(D_METHOD("set_target", "target"), &Car::set_target);
	ClassDB::bind_method(D_METHOD("copy_brain_from", "other"), &Car::copy_brain_from);
	ClassDB::bind_method(D_METHOD("mutate_brain", "rate", "strength"), &Car::mutate_brain);

	ClassDB::bind_method(D_METHOD("save_brain", "path"), &Car::save_brain);
	ClassDB::bind_method(D_METHOD("load_brain", "path"), &Car::load_brain);
}

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

	const double SAFE_DIST = 75.0; // Межа безпечної дистанції
	double wall_penalty = 0.0;
	// Якщо будь-який сенсор заліз у зону небезпеки:
	if (d_front < SAFE_DIST) wall_penalty += (SAFE_DIST - d_front);
	if (d_back < SAFE_DIST) wall_penalty += (SAFE_DIST - d_back);
	if (d_left < SAFE_DIST) wall_penalty += (SAFE_DIST - d_left);
	if (d_right < SAFE_DIST) wall_penalty += (SAFE_DIST - d_right);
	// Знімаємо бали пропорційно часу кадру:
	if (wall_penalty > 0.0) {
		add_fitness(-wall_penalty * 4096.0 * delta);
	}

	// --- TEST ---
	Input* input = Input::get_singleton();
	/*double drive = input->get_axis("ui_down", "ui_up");
	double turn = input->get_axis("ui_left", "ui_right");

	target.x = drive;
	target.y = turn;*/

	Matrix<float, 6, 1> inputs;
	inputs << (float)(d_front / 150.0),
		(float)(d_back / 150.0),
		(float)(d_left / 150.0),
		(float)(d_right / 150.0),
		(float)target.x,
		(float)target.y;

	auto motors_output = brain.forward(inputs);

	if (target_marker) {
		// -4.0 робимо для того, щоб крапка розміром 8х8 була відцентрована:
		Vector2 marker_pos = Vector2(target.x * 60.0 - 4.0, target.y * 60.0 - 4.0);
		target_marker->set_position(marker_pos);
	}

	/*double left_power = Math::clamp(drive + turn, -1.0, 1.0);
	double right_power = Math::clamp(drive - turn, -1.0, 1.0);

	apply_wheel_force(POS_FL, left_power);
	apply_wheel_force(POS_BL, left_power);

	apply_wheel_force(POS_FR, right_power);
	apply_wheel_force(POS_BR, right_power);*/

	apply_wheel_force(POS_FL, motors_output(0));
	apply_wheel_force(POS_BL, motors_output(1));
	apply_wheel_force(POS_FR, motors_output(2));
	apply_wheel_force(POS_BR, motors_output(3));


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
	if (input->is_key_pressed(Key::KEY_R))
	{
		brain = NeuralNet();
	}
	// --- TEST --
}

double Car::get_sensor_distance(RayCast2D* ray)
{
	if (!ray || !(ray->is_colliding())) return 150.;
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

void Car::reset_state(Vector2 start_pos, double start_rot)
{
	PhysicsServer2D* ps = PhysicsServer2D::get_singleton();

	// Телепортуємо фізичне тіло:
	ps->body_set_state(get_rid(), PhysicsServer2D::BODY_STATE_TRANSFORM, Transform2D(start_rot, start_pos));

	// Зупиняємо рух та обертання:
	ps->body_set_state(get_rid(), PhysicsServer2D::BODY_STATE_LINEAR_VELOCITY, Vector2(0, 0));
	ps->body_set_state(get_rid(), PhysicsServer2D::BODY_STATE_ANGULAR_VELOCITY, 0.0);

	fitness = 0.0;
}

void Car::set_target(Vector2 pos)
{
	target = pos;
}