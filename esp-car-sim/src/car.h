#pragma once

#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/classes/ray_cast2d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/color_rect.hpp>

#include "neural_net.h"

namespace godot
{

	class Car : public RigidBody2D
	{
		GDCLASS(Car, RigidBody2D);

	private:
		NeuralNet brain;

		double motor_force = 150.;

		RayCast2D* ray_front = nullptr;
		RayCast2D* ray_back = nullptr;
		RayCast2D* ray_left = nullptr;
		RayCast2D* ray_right = nullptr;

		ColorRect* target_marker = nullptr;

		// Координати кріплення 4 коліс відносно центру маси:
		const Vector2 POS_FL = Vector2(18.0f, -20.0f);  // Переднє ліве
		const Vector2 POS_BL = Vector2(-18.0f, -20.0f); // Заднє ліве
		const Vector2 POS_FR = Vector2(18.0f, 20.0f);   // Переднє праве
		const Vector2 POS_BR = Vector2(-18.0f, 20.0f);   // Заднє праве

	protected:
		static void _bind_methods();

	public:
		Car();
		~Car();

		void _ready() override;
		void _physics_process(double delta) override;

		void apply_wheel_force(Vector2 local_pos, double throttle);
		double get_sensor_distance(RayCast2D* ray);
	};

}