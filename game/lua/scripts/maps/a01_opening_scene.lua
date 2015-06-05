--------------------------------------------------------------------------------
-- 01_opening_scene.lua
--
-- This script controls the events that happen on the map after a player starts
-- a new game. It is heavy on dialogue and the user has no control over the characters.
--------------------------------------------------------------------------------
local ns = {}
setmetatable(ns, {__index = _G})
a01_opening_scene = ns;
setfenv(1, ns);

data_file = "lua/data/maps/harrvah_desert_cave_path.lua";
location_filename = "img/portraits/locations/blank.png";
map_name = "";

sound_filenames = {};

music_filenames = {};
music_filenames[1] = "snd/wind.ogg";

Map = {};

function Load(m)
	Map = m;

	ObjectManager = Map.object_supervisor;
	DialogueManager = Map.dialogue_supervisor;
	EventManager = Map.event_supervisor;
	TreasureManager = Map.treasure_supervisor;
	GlobalEvents = Map.map_event_group;

	-- Setup the order in which we wish to draw the tile and object layers
	Map:ClearLayerOrder();
	Map:AddTileLayerToOrder(0);
	Map:AddTileLayerToOrder(1);
	Map:AddObjectLayerToOrder(0);
	Map:AddTileLayerToOrder(2);

	-- Visuals: night lightning, during a sand storm, with occasional lightning. The "music" is nothing more than constant wind
	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.3, 0.6));
	VideoManager:EnableAmbientOverlay("img/effects/sand_storm.png", -450.0, 25.0);
	VideoManager:LoadLightningEffect("lua/graphics/lightning.lua", 2);
	VideoManager:EnableLightning(true);
	Map:SetCurrentTrack(0);
	
	CreateSprites();
	CreateDialogue();
	CreateEvents();
	EventManager:StartEvent(10);

	local event = hoa_map.CustomSpriteEvent.Create(1000, 10, "FocusCameraOnSprite", "");

	-- This entire map is played out in scene state. As soon as the map is loaded, we start the chain of events.
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	Map:DisableIntroductionVisuals();
	Map:ShowStaminaBar(false);
	Map:ShowDialogueIcons(false);
end -- Load(m)



function Update()
	-- Nothing special required
end



function Draw()
	Map:DrawMapLayers();
end


-- Creates all sprites for the characters, knights, and hounds
function CreateSprites()
	local sprite;
	local animation;

	
	-- Global starting coordinates for the center of the group of knights. All sprites
	-- use these coordinates in determining their initial positions.
	local group_start_x = 95;
	local group_start_y = 22;

	-- Create sprites for all three playable characters. They are at the back of the unit of troops
	claudius = {};
	mark = {};
	lukar = {};

	claudius = ConstructSprite("Claudius", 1, group_start_x - 12, group_start_y);
	claudius:SetDirection(hoa_map.MapMode.EAST);
	claudius:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	claudius:SetNoCollision(true);
	animation = claudius:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(claudius);

	mark = ConstructSprite("Knight01", 2, group_start_x - 10.5, group_start_y + 2.5);
	mark:SetDirection(hoa_map.MapMode.EAST);
	mark:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	mark:SetName(hoa_system.Translate("Mark"));
	mark:SetNoCollision(true);
	animation = mark:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(mark);

	lukar = ConstructSprite("Knight01", 3, group_start_x - 9.5, group_start_y - 1);
	lukar:SetDirection(hoa_map.MapMode.EAST);
	lukar:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	lukar:SetName(hoa_system.Translate("Lukar"));
	lukar:SetNoCollision(true);
	animation = lukar:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(lukar);

	-- Create the captain, his sergeant, and one senior knight leading the troop heading due East
	sprite = ConstructSprite("Knight06", 10, group_start_x + 12, group_start_y + 1);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetName(hoa_system.Translate("Captain Bravis"));
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight05", 11, group_start_x + 10.5, group_start_y + 0.25);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetName(hoa_system.Translate("Sergeant Methus"));
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight04", 12, group_start_x + 10, group_start_y + 2.5);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	-- Create the hounds and their knight pairs in the middle of the pack (sorted by top to bottom)
	sprite = ConstructSprite("Mak Hound", 20, group_start_x - 1, group_start_y - 1);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight01", 21, group_start_x + 3, group_start_y - 1);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight02", 22, group_start_x - 1, group_start_y - 4);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Mak Hound", 30, group_start_x - 3, group_start_y + 7);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight03", 31, group_start_x + 1, group_start_y + 7);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight02", 32, group_start_x - 5, group_start_y + 9);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	-- Create the rest of the knight squads
	-- Squad 1: Upper right of formation
	sprite = ConstructSprite("Knight01", 40, group_start_x + 5.5, group_start_y - 3);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight02", 41, group_start_x + 7.5, group_start_y - 1);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight03", 42, group_start_x + 5, group_start_y + 0.5);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	-- Squad 2: Lower right of formation
	sprite = ConstructSprite("Knight02", 50, group_start_x + 7.25, group_start_y + 4);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight01", 51, group_start_x + 5.5, group_start_y + 5.75);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight02", 52, group_start_x + 4, group_start_y + 8);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	-- Squad 3: Middle horizontal of formation
	sprite = ConstructSprite("Knight03", 60, group_start_x + 2.5, group_start_y + 4);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight02", 61, group_start_x, group_start_y + 2);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight01", 62, group_start_x - 3.5, group_start_y + 3.5);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	-- Squad 4: Rear vertical of formation
	sprite = ConstructSprite("Knight02", 70, group_start_x - 6, group_start_y - 2.5);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight03", 71, group_start_x - 7.25, group_start_y + 0.5);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	sprite = ConstructSprite("Knight02", 72, group_start_x - 8, group_start_y + 4.5);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite);

	-- TEMP: an invisible sprite used as a narrator and camera focus point
	sprite = ConstructSprite("Knight01", 100, group_start_x, group_start_y);
	sprite:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	sprite:SetName("");
	sprite:SetVisible(false);
	ObjectManager:AddObject(sprite);
	Map:SetCamera(sprite);
end -- function CreateSprites()


-- Creates all dialogue that takes place through characters and events
function CreateDialogue()
	local dialogue;
	local text;

	-- Introduction Dialogue
	dialogue = hoa_map.MapDialogue.Create(10);
		dialogue:SetInputBlocked(true);
		text = hoa_system.Translate("Behold, the barren desert that stretches out across the kingdom of Harrvah.");
		dialogue:AddLine(text, 100);
		dialogue:AddLineTiming(8000);
		text = hoa_system.Translate("The Harrvah Knights, though formidable in their great city of stone, were powerless against the might of this great desert.");
		dialogue:AddLine(text, 100);
		dialogue:AddLineTiming(8000);
		text = hoa_system.Translate("On this auspicious night, a large unit of knights is marching through the desert, making their way toward a cave a fair distance from the city walls.");
		dialogue:AddLine(text, 100);
		dialogue:AddLineTiming(8000);
		text = hoa_system.Translate("Although the great sand storms that had swept through the lands for the past several days had finally weakened, the winds still howled throughout the desert...");
		dialogue:AddLine(text, 100);
		dialogue:AddLineTiming(8000);	

	-- Captain's orders after troop movement
	dialogue = hoa_map.MapDialogue.Create(20);
		text = hoa_system.Translate("Alright, listen up!");
		dialogue:AddLine(text, 10);

	dialogue = hoa_map.MapDialogue.Create(21);
		text = hoa_system.Translate("Our intelligence reports that somewhere in this cave, the underground river vein that supplies our city has been blocked. Our mission is to remove the obstruction and restore our water supply.");
		dialogue:AddLine(text, 10);
		text = hoa_system.Translate("I don't need to remind you all of how important this mission is. That monster of a storm has prevented us from achieving this objective sooner, and our local reserves of water are nearly dry. If we fail to succeed here, our people will perish.");
		dialogue:AddLine(text, 10);
		text = hoa_system.Translate("The passages in this cave are too narrow for our entire unit to move through. We'll make our way through one squad at a time and re-assemble at the river bed.");
		dialogue:AddLine(text, 10);
		text = hoa_system.Translate("Now form your squads and get moving. I'll see you all underground.");
		dialogue:AddLine(text, 10);

	-- Character party after captain's orders
	dialogue = hoa_map.MapDialogue.Create(30);
		text = hoa_system.Translate("Mark. Claudius. Let's go.");
		dialogue:AddLine(text, 3);

	dialogue = hoa_map.MapDialogue.Create(31);
		text = hoa_system.Translate("The creatures that inhabit this cave are not friendly, so be on your guard at all times. Now let's get moving.");
		dialogue:AddLine(text, 3);

end -- function CreateDialogue()


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	local event = {};
	
	-- Event Group #1: Move all sprites toward the cave entrance. 
	-- The distance that the troops will march for. Note that some sprites will move slightly more or less
	local march_distance = 280;
	event = hoa_map.PathMoveSpriteEvent.Create(10, 1, march_distance - 0.5, 0);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(11);
	event:AddEventLinkAtStart(12);
	event:AddEventLinkAtStart(13);
	event:AddEventLinkAtStart(14);
	event:AddEventLinkAtStart(15);
	event:AddEventLinkAtStart(16);
	event:AddEventLinkAtStart(17);
	event:AddEventLinkAtStart(18);
	event:AddEventLinkAtStart(19);
	event:AddEventLinkAtStart(20);
	event:AddEventLinkAtStart(21);
	event:AddEventLinkAtStart(22);
	event:AddEventLinkAtStart(23);
	event:AddEventLinkAtStart(24);
	event:AddEventLinkAtStart(25);
	event:AddEventLinkAtStart(26);
	event:AddEventLinkAtStart(27);
	event:AddEventLinkAtStart(28);
	event:AddEventLinkAtStart(29);
	event:AddEventLinkAtStart(30);
	event:AddEventLinkAtStart(31);
	event:AddEventLinkAtStart(32);
	event:AddEventLinkAtStart(33);
	event:AddEventLinkAtStart(34);
	event:AddEventLinkAtStart(50, 5000); -- Narration dialogue starts 5 seconds after map begins
	event:AddEventLinkAtEnd(100, 250); -- Sprites begin turning in various directions after stopping to speak to one another
	event:AddEventLinkAtEnd(150, 2500); -- Captain's squad begins moving toward the top of the screen
	
	event = hoa_map.PathMoveSpriteEvent.Create(11, 2, march_distance - 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(12, 3, march_distance + 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(13, 10, march_distance + 0.5, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(14, 11, march_distance + 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(15, 12, march_distance + 0.4, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(16, 20, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(17, 21, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(18, 22, march_distance - 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(19, 30, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(20, 31, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(21, 32, march_distance + 1.5, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(22, 40, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(23, 41, march_distance + 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(24, 42, march_distance + 0.5, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(25, 50, march_distance + 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(26, 51, march_distance + 0.5, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(27, 52, march_distance + 0.75, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(28, 60, march_distance + 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(29, 61, march_distance + 0.5, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(30, 62, march_distance + 1, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(31, 70, march_distance - 0.25, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(32, 71, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(33, 72, march_distance + 0.25, 0);
	event:SetRelativeDestination(true);
	
	-- This event moves the invisible sprite serving as a focus for the map camera
	event = hoa_map.PathMoveSpriteEvent.Create(34, 100, march_distance, 0);
	event:SetRelativeDestination(true);

	-- Event Group #2: The narration dialogue
	event = hoa_map.DialogueEvent.Create(50, 10);

	-- Event Group #3: Various sprites begin turning toward each other as if they were talking
	-- This first event doesn't do anything since the sprite is already facing east. It is used as
	-- a base to set the timings of the other events in this group
	event = hoa_map.ChangeDirectionSpriteEvent.Create(100, 1, hoa_map.MapMode.EAST);
	
	event:AddEventLinkAtStart(101, 2400);
	event:AddEventLinkAtStart(102, 2600);
	event:AddEventLinkAtStart(103, 1720);
	event:AddEventLinkAtStart(104, 1880);
	event:AddEventLinkAtStart(105, 1250);
	event:AddEventLinkAtStart(106, 1400);
	event:AddEventLinkAtStart(107, 1650);
	event:AddEventLinkAtStart(108, 650);
	event:AddEventLinkAtStart(109, 900);
	event:AddEventLinkAtStart(110, 2800);
	event:AddEventLinkAtStart(111, 3000);
	event:AddEventLinkAtStart(112, 650);
	event:AddEventLinkAtStart(113, 750);
	event:AddEventLinkAtStart(114, 800);
	event:AddEventLinkAtStart(115, 550);
	event:AddEventLinkAtStart(116, 700);
	event:AddEventLinkAtStart(117, 1050);
	event:AddEventLinkAtStart(118, 3050);
	event:AddEventLinkAtStart(119, 3200);
	event:AddEventLinkAtStart(120, 4680);
	event:AddEventLinkAtStart(121, 4400);
	event:AddEventLinkAtStart(122, 3600);
	event:AddEventLinkAtStart(123, 3750);
	event:AddEventLinkAtStart(124, 5000);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(101, 2, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(102, 3, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(103, 70, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(104, 71, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(105, 72, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(106, 22, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(107, 21, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(108, 32, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(109, 31, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(110, 61, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(111, 60, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(112, 52, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(113, 51, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(114, 50, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(115, 40, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(116, 41, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(117, 42, hoa_map.MapMode.NORTH);
	
	-- These sprites change their directions more than once before the captain's speech
	event = hoa_map.ChangeDirectionSpriteEvent.Create(118, 2, hoa_map.MapMode.EAST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(119, 72, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(120, 31, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(121, 52, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(122, 41, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(123, 42, hoa_map.MapMode.EAST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(124, 3, hoa_map.MapMode.SOUTH);

	-- Event Group #4: The captain's squad turns as if discussing before moving to the top of the screen
	-- First change the directions of the sprites in the squad
	event = hoa_map.ChangeDirectionSpriteEvent.Create(150, 10, hoa_map.MapMode.WEST);
	event:AddEventLinkAtStart(151, 200);
	event:AddEventLinkAtStart(152, 450);
	event:AddEventLinkAtStart(153, 2400);
	event:AddEventLinkAtStart(156, 2600);
	event:AddEventLinkAtStart(159, 2800);
	event:AddEventLinkAtStart(162, 2000);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(151, 11, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(152, 12, hoa_map.MapMode.NORTH);
	
	-- Next begin moving the squad to the top of the screen
	local captain_move_x1 = 0;
	local captain_move_y1 = -6;
	local captain_move_x2 = -15;
	local captain_move_y2 = -6;
	-- Sergeant movements
	event = hoa_map.PathMoveSpriteEvent.Create(153, 11, captain_move_x1, captain_move_y1);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(154);
	event = hoa_map.PathMoveSpriteEvent.Create(154, 11, captain_move_x2 - 1, captain_move_y2);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(155);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(155, 11, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(163, 1500);
	
	-- Captain movements
	event = hoa_map.PathMoveSpriteEvent.Create(156, 10, captain_move_x1, captain_move_y1);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(157);
	event = hoa_map.PathMoveSpriteEvent.Create(157, 10, captain_move_x2, captain_move_y2);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(158);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(158, 10, hoa_map.MapMode.SOUTH);
	
	-- Senior knight movements
	event = hoa_map.PathMoveSpriteEvent.Create(159, 12, captain_move_x1, captain_move_y1);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(160, 500);
	event = hoa_map.PathMoveSpriteEvent.Create(160, 12, captain_move_x2 + 5, captain_move_y2 - 2);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(161);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(161, 12, hoa_map.MapMode.SOUTH);
	
	-- Center the camera on the captain while the squad is moving
	event = hoa_map.CustomSpriteEvent.Create(162, 10, "FocusCameraOnSprite", "");
	
	-- Finally have the sergeant step forward just before he is to speak
	event = hoa_map.PathMoveSpriteEvent.Create(163, 11, 0, 5);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(200);

	-- Event Group #5: The sergeant calls attention to the troops, who all look north toward him
	event = hoa_map.DialogueEvent.Create(200, 20);
	event:AddEventLinkAtEnd(201, 250);
	event:AddEventLinkAtEnd(202, 350);
	event:AddEventLinkAtEnd(203, 225);
	event:AddEventLinkAtEnd(204, 450);
	event:AddEventLinkAtEnd(205, 360);
	event:AddEventLinkAtEnd(206, 150);
	event:AddEventLinkAtEnd(207, 250);
	event:AddEventLinkAtEnd(208, 770);
	event:AddEventLinkAtEnd(209, 350);
	event:AddEventLinkAtEnd(210, 830);
	event:AddEventLinkAtEnd(211, 630);
	event:AddEventLinkAtEnd(212, 190);
	event:AddEventLinkAtEnd(213, 350);
	event:AddEventLinkAtEnd(214, 950);
	event:AddEventLinkAtEnd(215, 800);
	event:AddEventLinkAtEnd(216, 640);
	event:AddEventLinkAtEnd(217, 320);
	event:AddEventLinkAtEnd(218, 480);
	event:AddEventLinkAtEnd(219, 555);
	event:AddEventLinkAtEnd(220, 1250);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(201, 1, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(202, 2, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(203, 3, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(204, 21, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(205, 22, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(206, 31, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(207, 32, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(208, 40, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(209, 41, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(210, 42, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(211, 50, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(212, 51, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(213, 52, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(214, 60, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(215, 61, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(216, 62, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(217, 70, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(218, 71, hoa_map.MapMode.NORTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(219, 72, hoa_map.MapMode.NORTH);
	
	-- Sergeant moves northward back behind the captain, captain takes small step forward
	event = hoa_map.PathMoveSpriteEvent.Create(220, 11, 0, -5);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(221);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(221, 11, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(222, 250);
	event = hoa_map.PathMoveSpriteEvent.Create(222, 10, 0, 4);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(250, 250);

	-- Event Group #6: Dialogue where the captain gives out his orders
	local cave_entrance_x = 371;
	local cave_entrance_y = 5;
	event = hoa_map.DialogueEvent.Create(250, 21);
	event:AddEventLinkAtEnd(251, 500);
	event:AddEventLinkAtEnd(253, 500);
	event:AddEventLinkAtEnd(255, 500);
	event:AddEventLinkAtEnd(257, 500);
	event:AddEventLinkAtEnd(300, 1000);
	event:AddEventLinkAtEnd(320, 1250);
	event:AddEventLinkAtEnd(340, 1600);
	event:AddEventLinkAtEnd(360, 2200);
	event:AddEventLinkAtEnd(380, 2500);
	event:AddEventLinkAtEnd(400, 750);

	-- Captain's squad moves into the cave and disappears
	event = hoa_map.PathMoveSpriteEvent.Create(251, 10, cave_entrance_x, cave_entrance_y);
	event:AddEventLinkAtEnd(252);
	event = hoa_map.CustomSpriteEvent.Create(252, 10, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(253, 11, cave_entrance_x + 0.25, cave_entrance_y);
	event:AddEventLinkAtEnd(254);
	event = hoa_map.CustomSpriteEvent.Create(254, 11, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(255, 12, cave_entrance_x - 0.25, cave_entrance_y);
	event:AddEventLinkAtEnd(256);
	event = hoa_map.CustomSpriteEvent.Create(256, 12, "DisableCollisionAndVisibility", "");

	-- Event Group #7: Other squads turn toward each other again, then begin moving into the cave
	-- Rear squad
	event = hoa_map.ChangeDirectionSpriteEvent.Create(300, 70, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(301, 400);
	event:AddEventLinkAtEnd(304, 500);
	event:AddEventLinkAtEnd(307, 700);
	event = hoa_map.PathMoveSpriteEvent.Create(301, 70, cave_entrance_x - 5, cave_entrance_y + 7);
	event:AddEventLinkAtEnd(302);
	event = hoa_map.PathMoveSpriteEvent.Create(302, 70, cave_entrance_x + 0.5, cave_entrance_y);
	event:AddEventLinkAtEnd(303);
	event = hoa_map.CustomSpriteEvent.Create(303, 70, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(304, 71, cave_entrance_x - 5, cave_entrance_y + 7);
	event:AddEventLinkAtEnd(305);
	event = hoa_map.PathMoveSpriteEvent.Create(305, 71, cave_entrance_x, cave_entrance_y);
	event:AddEventLinkAtEnd(306);
	event = hoa_map.CustomSpriteEvent.Create(306, 71, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(307, 72, cave_entrance_x - 5, cave_entrance_y + 7);
	event:AddEventLinkAtEnd(308);
	event = hoa_map.PathMoveSpriteEvent.Create(308, 72, cave_entrance_x - 0.25, cave_entrance_y);
	event:AddEventLinkAtEnd(309);
	event = hoa_map.CustomSpriteEvent.Create(309, 72, "DisableCollisionAndVisibility", "");
	
	-- Hound squads
	event = hoa_map.ChangeDirectionSpriteEvent.Create(320, 22, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtStart(321, 500);
	event:AddEventLinkAtStart(322, 2500);
	event:AddEventLinkAtStart(323, 2200);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(321, 21, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(322, 32, hoa_map.MapMode.EAST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(323, 31, hoa_map.MapMode.WEST);
	
	-- Middle squad
	event = hoa_map.ChangeDirectionSpriteEvent.Create(340, 60, hoa_map.MapMode.WEST);
	event:AddEventLinkAtStart(341);
	event:AddEventLinkAtStart(342);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(341, 61, hoa_map.MapMode.SOUTH);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(342, 62, hoa_map.MapMode.EAST);
	
	-- Top right squad
	event = hoa_map.ChangeDirectionSpriteEvent.Create(360, 40, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtStart(361);
	event:AddEventLinkAtEnd(362, 400);
	event:AddEventLinkAtEnd(365, 500);
	event:AddEventLinkAtEnd(368, 700);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(361, 41, hoa_map.MapMode.WEST);
	event = hoa_map.PathMoveSpriteEvent.Create(362, 40, 0, -6);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(363);
	event = hoa_map.PathMoveSpriteEvent.Create(363, 40, cave_entrance_x, cave_entrance_y);
	event:AddEventLinkAtEnd(364);
	event = hoa_map.CustomSpriteEvent.Create(364, 40, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(365, 41, 0, -6);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(366);
	event = hoa_map.PathMoveSpriteEvent.Create(366, 41, cave_entrance_x, cave_entrance_y);
	event:AddEventLinkAtEnd(367);
	event = hoa_map.CustomSpriteEvent.Create(367, 41, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(368, 42, 0, -6);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(369);
	event = hoa_map.PathMoveSpriteEvent.Create(369, 42, cave_entrance_x, cave_entrance_y);
	event:AddEventLinkAtEnd(370);
	event = hoa_map.CustomSpriteEvent.Create(370, 42, "DisableCollisionAndVisibility", "");

	-- Bottom left squad
	event = hoa_map.ChangeDirectionSpriteEvent.Create(380, 52, hoa_map.MapMode.EAST);
	event:AddEventLinkAtStart(381);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(381, 50, hoa_map.MapMode.WEST);

	-- Event Group #8: Dialogue between the members of the character party, who then move to the cave
	-- Move the camera to focus on Lukar, turn him toward his party, and open his first dialogue
	event = hoa_map.CustomSpriteEvent.Create(400, 3, "FocusCameraOnSprite", "");
	event:AddEventLinkAtEnd(401);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(401, 3, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(402, 250);
	event = hoa_map.DialogueEvent.Create(402, 30);
	event:AddEventLinkAtEnd(403, 500);
	event:AddEventLinkAtEnd(404, 500);
	event:AddEventLinkAtEnd(406, 1000);
	-- Turn Claudius and Mark to face Lukar, who speaks again
	event = hoa_map.ChangeDirectionSpriteEvent.Create(403, 1, hoa_map.MapMode.EAST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(404, 2, hoa_map.MapMode.NORTH);
	event = hoa_map.DialogueEvent.Create(406, 31);
	event:AddEventLinkAtEnd(407, 500);
	event:AddEventLinkAtEnd(409, 500);
	event:AddEventLinkAtEnd(411, 1000);
	-- Move character squad into the cave and make the sprites disappear as they enter
	event = hoa_map.PathMoveSpriteEvent.Create(407, 3, cave_entrance_x, cave_entrance_y);
	event:AddEventLinkAtEnd(408);
	event = hoa_map.CustomSpriteEvent.Create(408, 3, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(409, 2, cave_entrance_x + 0.25, cave_entrance_y);
	event:AddEventLinkAtEnd(410);
	event = hoa_map.CustomSpriteEvent.Create(410, 2, "DisableCollisionAndVisibility", "");
	event = hoa_map.PathMoveSpriteEvent.Create(411, 1, cave_entrance_x - 0.25, cave_entrance_y);
	event:AddEventLinkAtEnd(412);
	event = hoa_map.CustomSpriteEvent.Create(412, 1, "DisableCollisionAndVisibility", "");
	event:AddEventLinkAtEnd(413, 1000);
	
	-- Once the cave has been reached, transition to the cave map
	event = hoa_map.MapTransitionEvent.Create(413, "lua/scripts/maps/a01_unblock_underground_river.lua");
end -- function CreateEvents()


-- Container for all map class event functions
functions = {};

-- Sprite function: Focus map camera on sprite
functions["FocusCameraOnSprite"] = function(sprite)
	Map:SetCamera(sprite, 1000);
end


-- Sprite function: Disable collision and visibility on sprite
functions["DisableCollisionAndVisibility"] = function(sprite)
	sprite:SetVisible(false);
	sprite:SetNoCollision(true);
end


-- Sprite function: Change movement speed of a sprite
functions["ChangeSpriteMovementSpeed"] = function(sprite)
	sprite:SetMovementSpeed(hoa_map.MapMode.VERY_FAST_SPEED);
end

