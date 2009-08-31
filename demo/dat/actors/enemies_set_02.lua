------------------------------------------------------------------------------[[
-- Filename: enemies_set02.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 101-200.
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
if (_G.enemies == nil) then
   enemies = {}
end

enemies[101] = {
	name = "Daemarbora",
	filename = "daemarbora",
	sprite_width = 128,
	sprite_height = 128,

	initial_stats = {
		hit_points = 80,
		skill_points = 20,
		experience_points = 15,
		strength = 12,
		vigor = 20,
		fortitude = 15,
		protection = 7,
		agility = 8,
		evade = 1.0,
		drunes = 55
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 1.8,
		strength = 7.0,
		vigor = 0.0,
		fortitude = 4.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0,
		drunes = 5.0
	},

	attack_points = {
		[1] = {
			name = "Trunk",
			x_position = 58,
			y_position = 36,
			fortitude_modifier = 0,
			protection_modifier = 0,
			evade_modifier = 0.0
		},
		[2] = {
			name = "Branches",
			x_position = 64,
			y_position = 80,
			fortitude_modifier = 0,
			protection_modifier = 0,
			evade_modifier = 1.0
		}
	},

	skills = {
		[1] = 103
	},

	drop_objects = {
		{ 10002, 0.05, 1 } -- Iron sword
	}
}


enemies[102] = {
	name = "Aerocephal",
	filename = "aerocephal",
	sprite_width = 192,
	sprite_height = 192,

	
	initial_stats = {
		hit_points = 90,
		skill_points = 10,
		experience_points = 12,
		strength = 8,
		vigor = 0,
		fortitude = 7,
		protection = 4,
		agility = 20,
		evade = 10.0,
		drunes = 60
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 2.0,
		strength = 5.0,
		vigor = 0.0,
		fortitude = 3.0,
		protection = 4.0,
		agility = 7.5,
		evade = 2.5,
		drunes = 6.0
	},

	attack_points = {
		[1] = {
			name = "Forehead",
			x_position = 90,
			y_position = 127,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = "Orifice",
			x_position = 95,
			y_position = 77,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
	},

	skills = {
		[1] = 103
	},

	drop_objects = {
		{ 20502, 0.10, 1 } -- Winged Circlet
	}
}


enemies[103] = {
	name = "Arcana Drake",
	filename = "arcana_drake",
	sprite_width = 192,
	sprite_height = 256,

	
	initial_stats = {
		hit_points = 85,
		skill_points = 10,
		experience_points = 45,
		strength = 20,
		vigor = 0,
		fortitude = 8,
		protection = 5,
		agility = 15,
		evade = 2.0,
		drunes = 80
	},
	
	growth_stats = {
		hit_points = 15.0,
		skill_points = 10.0,
		experience_points = 2.0,
		strength = 15.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 3.0,
		agility = 6.0,
		evade = 2.0,
		drunes = 5.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 26,
			y_position = 23,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = "Body",
			x_position = 95,
			y_position = 111,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = "Tail",
			x_position = 22,
			y_position = 146,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		[1] = 104
	},

	drop_objects = {
		{ 20002, 0.05, 1 } -- Cobalt Helm
	}
}


enemies[104] = {
	name = "Nagaruda",
	filename = "nagaruda",
	sprite_width = 192,
	sprite_height = 256,

	initial_stats = {
		hit_points = 90,
		skill_points = 10,
		experience_points = 18,
		strength = 10,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 12,
		evade = 2.0,
		drunes = 70
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 2.1,
		strength = 6.0,
		vigor = 0.0,
		fortitude = 5.0,
		protection = 4.0,
		agility = 8.0,
		evade = 2.0,
		drunes = 7.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 70,
			y_position = 165,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = "Abdomen",
			x_position = 60,
			y_position = 115,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = "Tail",
			x_position = 70,
			y_position = 65,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		[1] = 106
	},

	drop_objects = {
		{ 40001, 0.15, 1 } -- Karlate shield
	}
}


enemies[105] = {
	name = "Deceleon",
	filename = "deceleon",
	sprite_width = 256,
	sprite_height = 256,

	initial_stats = {
		hit_points = 100,
		skill_points = 10,
		experience_points = 18,
		strength = 15,
		vigor = 0,
		fortitude = 20,
		protection = 4,
		agility = 4,
		evade = 1.0,
		drunes = 85
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 2.2,
		strength = 7.0,
		vigor = 0.0,
		fortitude = 5.0,
		protection = 4.0,
		agility = 4.0,
		evade = 1.5,
		drunes = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 152,
			y_position = 226,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 150,
			y_position = 190,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = "Arm",
			x_position = 200,
			y_position = 155,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[4] = {
			name = "Legs",
			x_position = 150,
			y_position = 105,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		[1] = 105
	},

	drop_objects = {
		{ 10503, 0.05, 1 } -- Reinforced crossbow
	}
}


enemies[106] = {
	name = "Aurum Drakueli",
	filename = "aurum-drakueli",
	sprite_width = 320,
	sprite_height = 256,

	
	initial_stats = {
		hit_points = 120,
		skill_points = 10,
		experience_points = 20,
		strength = 22,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 18,
		evade = 2.0,
		drunes = 100
	},
	
	growth_stats = {
		hit_points = 15.0,
		skill_points = 10.0,
		experience_points = 2.2,
		strength = 9.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 7.5,
		evade = 2.0,
		drunes = 4.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 156,
			y_position = 222,
			fortitude_modifier = 0,
			protection_modifier = 0,
			evade_modifier = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 199,
			y_position = 155,
			fortitude_modifier = 0,
			protection_modifier = 0,
			evade_modifier = 0.0
		},
		[3] = {
			name = "Arm",
			x_position = 242,
			y_position = 143,
			fortitude_modifier = 0,
			protection_modifier = 0,
			evade_modifier = 0.0
		}
	},

	skills = {
		[1] = 107
	},

	drop_objects = {
		{ 30001, 0.15, 1 } -- Karlate breastplate
	}
}

enemies[107] = {
	name = "Kyle",
	filename = "kyle",
	sprite_width = 48,
	sprite_height = 96,

	
	initial_stats = {
		hit_points = 55,
		skill_points = 19,
		experience_points = 20,
		strength = 24,
		vigor = 5,
		fortitude = 14,
		protection = 6,
		agility = 36,
		evade = 5.0,
		drunes = 5
	},
	
	growth_stats = {
		hit_points = 9.0,
		skill_points = 10.0,
		experience_points = 1.5,
		strength = 8.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0,
		drunes = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 45,
			y_position = 80,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 45,
			y_position = 46,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = "Legs",
			x_position = 45,
			y_position = 14,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		[1] = 21
	},

	drop_objects = {
		{ 1, 0.15, 1 } -- Healing potion
	}
}
