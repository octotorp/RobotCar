extends Node2D

const car_scene = preload("res://scenes/car.tscn")
const population = 100
const round_time = 20.
const start_pos = Vector2(140.,300.)
const auto_save_interval_ms: int = 20 * 60 * 1000 # 20 хвилин у мілісекундах реального часу

var cars: Array = []
var timer: float = 0.
var generation: int = 0
var isStop: bool = false
var last_save_time: int = 0

func _ready() -> void:
	last_save_time = Time.get_ticks_msec()
	for i in range(population):
		var car = car_scene.instantiate()
		car.position = start_pos
		add_child(car)
		car.reset_state(start_pos, 0.)
		car.mutate_brain(0.5, 0.5)
		car.set_target(Vector2(1., 0.))
		cars.append(car)
		
func _physics_process(delta: float) -> void:
	timer += delta
	if timer > round_time - 3.:
		isStop = true;
		for car in cars:
			car.set_target(Vector2(0., 0.))
	
	for car in cars:
		
		if isStop:
			var speed = car.linear_velocity.length()
			if speed > 0.1:
				car.add_fitness(abs(speed) * -0.5 * delta)
			else:
				car.add_fitness(10 * delta)
		else:
			var forward_speed = car.linear_velocity.dot(car.transform.x)
			if forward_speed > 0:
				car.add_fitness(forward_speed * delta * 0.3)
			else:
				car.add_fitness(forward_speed * delta * 0.05)
		car.add_fitness(abs(car.get_angular_velocity()) * -10000 * delta)
		
	
	if timer >= round_time:
		evolve_population()
		timer = 0.

func evolve_population() -> void:
	var best_car = null
	var best_score = -9999999.
	isStop = false
	
	for car in cars:
		car.set_target(Vector2(1., 0.))
		if car.get_fitness() > best_score:
			best_score = car.get_fitness()
			best_car = car
	print("--- ПОКОЛІННЯ ", generation, " --- Рекорд: ", int(best_score))
	
	# Автозбереження найкращого мозку раз на 20 хвилин реального часу
	var current_time = Time.get_ticks_msec()
	if current_time - last_save_time >= auto_save_interval_ms:
		last_save_time = current_time
		var time_str = Time.get_time_string_from_system().replace(":", "-")
		var filename = "%s -- %d.bin" % [time_str, int(best_score)]
		if best_car:
			best_car.save_brain(filename)
			print(">>> [АВТОЗБЕРЕЖЕННЯ]: ", filename)

	generation += 1
	for car in cars:
		if car != best_car:
			car.copy_brain_from(best_car)
			car.mutate_brain(0.1, 0.15)
		car.reset_state(start_pos, 0.)






func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed:
		if event.keycode == KEY_1:
			Engine.time_scale = 1.0  # Звичайна швидкість
			print("Швидкість: 1x")
		elif event.keycode == KEY_2:
			Engine.time_scale = 5.0  # 5x
			print("Швидкість: 5x")
		elif event.keycode == KEY_3:
			Engine.time_scale = 15.0 # Турбо 15x (покоління пролітатимуть за секунду!)
			print("Швидкість: 15x")
		elif event.keycode == KEY_4:
			Engine.time_scale = 30.0 # Турбо 15x (покоління пролітатимуть за секунду!)
			print("Швидкість: 30x")
		elif event.keycode == KEY_S:
			if cars.size() > 0:
				var time_str = Time.get_time_string_from_system().replace(":", "-")
				var filename = "%s -- manual.bin" % [time_str]
				cars[0].save_brain(filename)
				print(">>> [РУЧНЕ ЗБЕРЕЖЕННЯ]: ", filename)
