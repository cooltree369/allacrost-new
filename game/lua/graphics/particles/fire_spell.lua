-- Fire spell particle effect
-- Last modified on November 10th, 2005
-- Author: roos, modifications by Bertram
-- Modified from fire.lua to pop the fire only once

systems = {}

systems[0] =
{
	emitter =
	{
		x=0,
		y=-20,
		x2=0,
		y2=0,
		center_x=0,
		center_y=0,
		x_variation=0,
		y_variation=0,
		radius=30,
		shape='CIRCLE',
		omnidirectional=true,
		orientation=0,
		outer_cone=0,
		inner_cone=0,
		initial_speed=200,
		initial_speed_variation=0,
		emission_rate=100,
		start_time=0,
		emitter_mode='burst',
		spin='RANDOM'
	},

	keyframes =
	{
		{  -- keyframe 1
			size_x=0.9,
			size_y=0.9,
			color={1,1,1,.8},
			rotation_speed=1,
			size_variation_x=0,
			size_variation_y=0,
			rotation_speed_variation=.5,
			color_variation={0,0,0,0},
			time=0
		},

		{  -- keyframe 2
			size_x=0.8,
			size_y=0.8,
			color={1,1,1,0},
			rotation_speed=1,
			size_variation_x=0,
			size_variation_y=0,
			rotation_speed_variation=.5,
			color_variation={0,0,0,0},
			time=1.0
		}

	},

	animation_frames =
	{
		'img/effects/flame1.png',
		'img/effects/flame2.png',
		'img/effects/flame3.png',
		'img/effects/flame4.png',
		'img/effects/flame5.png',
		'img/effects/flame6.png',
		'img/effects/flame7.png',
		'img/effects/flame8.png'
	},
	animation_frame_times =
	{
		16, 16, 16, 16, 16, 16, 16, 16
	},
	enabled = true,
	blend_mode = 13,
	system_lifetime = 3,
	particle_lifetime = 0.4,
	particle_lifetime_variation = 0.00,
	max_particles = 50,
	damping = 0.01,
	damping_variation = 0,
	acceleration_x = 0,
	acceleration_y = 0,
	acceleration_variation_x = 0,
	acceleration_variation_y = 0,
	wind_velocity_x = 0,
	wind_velocity_y = -155,
	wind_velocity_variation_x = 0,
	wind_velocity_variation_y = 0,
	wave_motion_used = false,
	wave_length = .5,
	wave_length_variation = 0,
	wave_amplitude = 0,
	wave_amplitude_variation = 0,
	tangential_acceleration = 0,
	tangential_acceleration_variation = 0,
	radial_acceleration = 0,
	radial_acceleration_variation = 0,
	user_defined_attractor = false,
	attractor_falloff = 0,
	rotation_used = true,
	rotate_to_velocity = false,
	speed_scale_used = false,
	speed_scale = 0.005,
	min_speed_scale = 1.0,
	max_speed_scale = 20.0,
	smooth_animation = true,
	modify_stencil = false,
	stencil_op = 'INCR',
	use_stencil = false,
	scene_lighting = 0.0,
	random_initial_angle = true
}
