extends Node2D

var current_delta: float = 0.0

const car_scene = preload("res://scenes/car.tscn")
const population = 200
const round_time = 55.
const start_pos = Vector2(140.,300.)
const auto_save_interval_ms: int = 20 * 60 * 1000 # 20 хвилин у мілісекундах реального часу

enum Stage {move_forward, stay_still, move_reverse}

var once = 1
var is_rendering_disabled: bool = false

var last_best_car = null
var last_best_score = -9999999999.

var cars: Array = []
var timer: float = 0.
var generation: int = 0
var current_stage: Stage

var last_save_time: int = 0

@onready var obstacle: Node2D = $Obstacle
var isDown: bool = true

func _ready() -> void:
	last_save_time = Time.get_ticks_msec()
	Engine.time_scale = 5.0
	current_stage = Stage.move_forward
	for i in range(population):
		var car = car_scene.instantiate()
		car.set_meta("crashed", false)
		car.position = start_pos
		add_child(car)
		car.reset_state(start_pos, 0.)
		car.load_brain("brain/22-08-06 -M- 12684.bin")
		if i != 1:
			car.mutate_brain(0.2, 0.33)
		car.set_target(Vector2(1., 0.))
		cars.append(car)
		
func _physics_process(delta: float) -> void:
	timer += delta
			
	if timer < 25.0 and current_stage != Stage.move_forward:
		current_stage = Stage.move_forward
		for car in cars:
			car.set_target(Vector2(1., 0.))
			
	if timer > 25.0 and timer <= 30.0 and current_stage != Stage.stay_still:
		current_stage = Stage.stay_still 
		for car in cars:
			car.set_target(Vector2(0., 0.))
			
	if timer > 30.0 and current_stage != Stage.move_reverse:
		current_stage = Stage.move_reverse
		for car in cars:
			car.set_target(Vector2(-1., 0.))
	
	current_delta = delta
	for i in range(cars.size()):
		process_car_fitness(i)
	
	
	
	#if obstacle.position.y < -10:
	#	isDown = true
	#if obstacle.position.y > 660:
	#	isDown = false
	
	#if isDown:
	#	obstacle.position.y += 5.
	#else:
	#	obstacle.position.y -= 5.

	if timer > 10.0 and once == 1:
		once = 2
		obstacle.position.y = -1000
	
	if timer > 25.0 and once == 2:
		once = 3
		obstacle.position.y = -10
		
	if timer > 50.0 and once == 3:
		once = 4
		obstacle.position.y = -1000
	
	
	if timer >= round_time:
		evolve_population()
		timer = 0.


func process_car_fitness( car_index: int) -> void:
	var car = cars[car_index]
	var delta = current_delta
	var is_crashed = car.get_meta("crashed", false)
	if is_crashed:
		car.add_fitness(-100)
		return
	
	if not is_crashed:
		@warning_ignore("confusable_local_declaration")
		var d_front = car.get_front_distance()
		@warning_ignore("confusable_local_declaration")
		var d_back = car.get_back_distance()
		if d_front < 15.0 or d_back < 15.0:
			car.set_meta("crashed", true)
			car.add_fitness(-3000.0)
			return
			
	var speed = car.linear_velocity
	var bad_speed = speed.length() - max(0.0, speed.x)
	var d_front = car.get_front_distance()
	var d_back = car.get_back_distance()
	var forward_speed = speed.dot(car.transform.x)
	
	match current_stage:
		Stage.move_forward:
			car.add_fitness(-bad_speed * 2.0 * delta)
			
			var safe_factor = clamp(d_front / 150.0, 0.0, 1.0)
			
			if forward_speed > 0:
				car.add_fitness(forward_speed * delta * 3 * safe_factor)
			else:
				car.add_fitness(forward_speed * delta * 15)
			
		Stage.stay_still:
			car.add_fitness(-bad_speed * 32.0 * delta)
			
			car.add_fitness(abs(car.get_angular_velocity()) * -30 * delta)
			if speed.length() > 0.1:
				car.add_fitness(abs(speed.length()) * -40.0 * delta)
			else:
				car.add_fitness(200 * delta)
			
			
		Stage.move_reverse:
			car.add_fitness(-bad_speed * 16.0 * delta)
			car.add_fitness(abs(car.get_angular_velocity()) * -30 * delta)
			
			var safe_factor = clamp(d_back / 150.0, 0.0, 1.0)
			var reverse_speed = -speed.x
			if reverse_speed < 15.0 and d_back > 100.:
				car.add_fitness(-100.0 * delta)
			
			if d_back < 150.:
				car.add_fitness(abs(car.get_angular_velocity()) * -64 * delta)
			
			if d_back > 20.0 and d_back < 100.0:
				car.add_fitness((150.0 - speed.length() * 150) * delta)
			
			if forward_speed > 0:
				car.add_fitness(forward_speed * delta * -15)
			else:
				car.add_fitness(forward_speed * delta * -8 * safe_factor)
			
	#MultiStage fitness
	car.add_fitness(-abs(car.rotation) * 120.0 * delta)
	var y_drift = abs(car.global_position.y - start_pos.y)
	car.add_fitness(-y_drift * 0.4 * delta)
	
	
func evolve_population() -> void:
	var best_car = null
	var best_score = -9999999999.
	isDown = true
	once = 1
	obstacle.position.y = -10
	
	cars.sort_custom(func(a, b): return a.get_fitness() > b.get_fitness())
	var top_parents = cars.slice(0, 5)
	best_car = cars[0]
	best_score = cars[0].get_fitness()
	
	
	
	print("--- ПОКОЛІННЯ ", generation, " --- Рекорд: ", int(best_score))
	last_best_car = best_car
	last_best_score = best_score
	
	# Автозбереження найкращого мозку раз на 20 хвилин реального часу
	var current_time = Time.get_ticks_msec()
	if current_time - last_save_time >= auto_save_interval_ms:
		last_save_time = current_time
		var time_str = Time.get_time_string_from_system().replace(":", "-")
		var filename = "brain/%s -- %d.bin" % [time_str, int(best_score)]
		if best_car:
			best_car.save_brain(filename)
			print(">>> [АВТОЗБЕРЕЖЕННЯ]: ", filename)

	generation += 1
	for i in range(cars.size()):
		
		if cars[i] in top_parents:
			pass
		elif i < 20:
			cars[i].copy_brain_from(best_car)
			cars[i].mutate_brain(0.3, 0.4)
		else:
			var parent = top_parents.pick_random()
			cars[i].copy_brain_from(parent)
			cars[i].mutate_brain(0.15, 0.2)
		cars[i].set_meta("crashed", false)
		cars[i].reset_state(start_pos, 0.)






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
		elif event.keycode == KEY_B:
			if last_best_car:
				var time_str = Time.get_time_string_from_system().replace(":", "-")
				var filename = "brain/%s -M- %d.bin" % [time_str, int(last_best_score)]
				last_best_car.save_brain(filename)
				print(">>> [РУЧНЕ ЗБЕРЕЖЕННЯ]: ", filename)
		elif event.keycode == KEY_H:
			is_rendering_disabled = !is_rendering_disabled
			for car in cars:
				car.visible = !is_rendering_disabled
			$Background.visible = !is_rendering_disabled
	# Знижуємо частоту оновлення екрана до 5 FPS (фізика працюватиме на повну!):
			Engine.max_fps = 5 if is_rendering_disabled else 0
			print("Графіка: ", "ВИМКНЕНО (турбо-режим)" if is_rendering_disabled else "УВІМКНЕНО")
			
		elif event.keycode == KEY_S:
			if cars.size() > 0:
				var time_str = Time.get_time_string_from_system().replace(":", "-")
				var filename = "%s -- manual.bin" % [time_str]
				cars[0].save_brain(filename)
				print(">>> [РУЧНЕ ЗБЕРЕЖЕННЯ]: ", filename)
