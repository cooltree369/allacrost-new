--------------------------------------------------------------------------------
-- a01_harrvah_capital_attack.lua
--
-- A script specific to the main storyline events. The Harrvah Capital is under
-- attack by demons and the player has to navigate his party through the chaos
-- to find the king.
--------------------------------------------------------------------------------
local ns = {}
setmetatable(ns, {__index = _G})
a01_harrvah_capital_attack = ns;
setfenv(1, ns);

-- Set to true to turn on debugging messages generated by this map script
DEBUG = false; 

data_file = "lua/data/maps/harrvah_capital.lua";
location_filename = "img/portraits/locations/blank.png";
map_name = "Harrvah Capital";

sound_filenames = {};

music_filenames = {};
music_filenames[1] = "mus/Claudius.ogg";

-- Primary Map Classes
Map = {};
ObjectManager = {};
DialogueManager = {};
EventManager = {};
TreasureManager = {};
GlobalEvents = {};

enemy_ids = { }

-- Containers used to hold pointers to various class objects.
contexts = {};
zones = {};
objects = {};
sprites = {};
dialogues = {};
events = {};

-- All custom map functions are contained within the following table.
-- String keys in this table serves as the names of these functions. 
functions = {};

-- Shorthand names for map contexts
contexts["exterior"] = hoa_map.MapMode.CONTEXT_01; -- Displays the exterior of the town
contexts["interior"] = hoa_map.MapMode.CONTEXT_03; -- Displays the interiors of various structures



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
	Map:AddTileLayerToOrder(3);

	CreateZones();
	CreateObjects();
	CreateSprites();
	CreateEnemies();
	CreateDialogues();
	CreateEvents();

	-- Visuals: night lightning
	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.3, 0.6));

	Map:SetCurrentTrack(0);

	-- TODO: figure out if visuals should be disabled normally, or wait for control to be given to the player before they are displayed
	-- Map:DisableIntroductionVisuals();
	Map.unlimited_stamina = DEBUG;
	Map:ShowStaminaBar(not DEBUG);
	Map:ShowDialogueIcons(true);

	-- All character sprites are initially uncollidable, since they will "merge" into one sprite at the end of the opening scene
	Map:SetCamera(sprites["claudius"]);
	sprites["claudius"]:SetNoCollision(true);
	sprites["mark"]:SetNoCollision(true);
	sprites["lukar"]:SetNoCollision(true);

	EventManager:StartEvent(event_chains["intro_scene"]);
	IfPrintDebug(DEBUG, "Map loading complete");
end -- Load(m)



function Update()
	-- Nothing special required
end



function Draw()
	Map:DrawMapLayers();
end



function CreateZones()
	IfPrintDebug(DEBUG, "Creating zones...");
	---------- Context Zones

	-- The following zone implements the context switching for all structures found in the town.
	-- The zone sections correspond to the area just outside of the doors to these buildings.
	-- They are ordered starting from the bottom left of the map, going toward the right.
	zones["town_doors"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior"]);
	-- Southwest home 1
	zones["town_doors"]:AddSection(22, 26, 176, 177, true);
	zones["town_doors"]:AddSection(22, 26, 175, 176, false);
	-- Soutwest home 2
	zones["town_doors"]:AddSection(48, 52, 180, 181, true);
	zones["town_doors"]:AddSection(48, 52, 179, 180, false);
	-- Item Shop
	zones["town_doors"]:AddSection(80, 84, 180, 181, true);
	zones["town_doors"]:AddSection(80, 84, 179, 180, false);
	-- Inn
	zones["town_doors"]:AddSection(116, 120, 182, 1835, true);
	zones["town_doors"]:AddSection(116, 120, 181, 182, false);

	-- West home 1
	zones["town_doors"]:AddSection(20, 24, 152, 153, true);
	zones["town_doors"]:AddSection(20, 24, 151, 152, false);
	-- West home 2
	zones["town_doors"]:AddSection(48, 52, 150, 151, true);
	zones["town_doors"]:AddSection(48, 52, 149, 150, false);
	-- West home 3
	zones["town_doors"]:AddSection(78, 82, 152, 153, true);
	zones["town_doors"]:AddSection(78, 82, 151, 152, false);
	-- Weapon Shop
	zones["town_doors"]:AddSection(148, 152, 154, 155, true);
	zones["town_doors"]:AddSection(148, 152, 153, 154, false);
	-- East home
	zones["town_doors"]:AddSection(178, 182, 148, 149, true);
	zones["town_doors"]:AddSection(178, 182, 147, 148, false);

	-- Northwest home 1
	zones["town_doors"]:AddSection(12, 16, 122, 123, true);
	zones["town_doors"]:AddSection(12, 16, 121, 122, false);
	-- Northwest home 2
	zones["town_doors"]:AddSection(50, 54, 126, 127, true);
	zones["town_doors"]:AddSection(50, 54, 125, 126, false);
	-- Northeast home 1
	zones["town_doors"]:AddSection(116, 120, 126, 127, true);
	zones["town_doors"]:AddSection(116, 120, 125, 126, false);
	-- Northwest home 2
	zones["town_doors"]:AddSection(140, 144, 122, 123, true);
	zones["town_doors"]:AddSection(140, 144, 121, 122, false);
	-- Claudius' home
	zones["town_doors"]:AddSection(168, 172, 124, 125, true);
	zones["town_doors"]:AddSection(168, 172, 123, 124, false);

	Map:AddZone(zones["town_doors"]);
end



function CreateObjects()
	IfPrintDebug(DEBUG, "Creating objects...");
end



function CreateSprites()
	IfPrintDebug(DEBUG, "Creating sprites...");

	local sprite;
	local animation;

	-- Create sprites for the three playable characters
	-- This X/Y position that represents the south middle point of the map, just outside the city wall gates
	local entrance_x = 98;
	local entrance_y = 208;

	sprites["claudius"] = ConstructSprite("Claudius", 1, entrance_x - 2, entrance_y - 2);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["claudius"]);

	sprites["mark"] = ConstructSprite("Knight01", 2, entrance_x + 2, entrance_y - 2);
	sprites["mark"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["mark"]:SetName(hoa_system.Translate("Mark"));
	ObjectManager:AddObject(sprites["mark"]);

	sprites["lukar"] = ConstructSprite("Knight01", 3, entrance_x, entrance_y - 4);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	ObjectManager:AddObject(sprites["lukar"]);

	-- Create the captain and the NPCs fighting the battle straight ahead of the city entrance
	-- Coordinates for NPC battle that lies straight ahead on the road after entering the town
	local straight_battle_x = 98;
	local straight_battle_y = 175;

	sprites["captain"] = ConstructSprite("Knight06", 10, straight_battle_x, straight_battle_y + 4);
	sprites["captain"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["captain"]:SetName(hoa_system.Translate("Captain Bravis"));
	ObjectManager:AddObject(sprites["captain"]);

	sprites["knight01"] = ConstructSprite("Knight01", 11, straight_battle_x - 3, straight_battle_y);
	sprites["knight01"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["knight01"]);

	sprites["knight02"] = ConstructSprite("Knight02", 12, straight_battle_x + 4, straight_battle_y - 2);
	sprites["knight02"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["knight02"]);

	-- Create the senior knight and others fighting near the inn
	-- Coordinates for NPC battle that lies near the entrance to the inn
	local inn_battle_x = 123;
	local inn_battle_y = 188;

	sprites["senior_knight"] = ConstructSprite("Knight04", 12, inn_battle_x - 1, inn_battle_y);
	sprites["senior_knight"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["senior_knight"]);

	sprites["knight03"] = ConstructSprite("Knight03", 15, inn_battle_x, inn_battle_y + 2);
	sprites["knight03"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["knight03"]);

	sprites["knight04"] = ConstructSprite("Knight01", 16, inn_battle_x + 2, inn_battle_y - 2);
	sprites["knight04"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["knight04"]);

	-- Create the sergeant and others fighting near the item shop entrance
	-- Coordinates for NPC battle that takes place near the entrance to the item shop
	local item_shop_battle_x = 84;
	local item_shop_battle_y = 184;
	sprites["sergeant"] = ConstructSprite("Knight05", 11, item_shop_battle_x + 3, item_shop_battle_y - 2);
	sprites["sergeant"]:SetDirection(hoa_map.MapMode.WEST);
	sprites["sergeant"]:SetName(hoa_system.Translate("Sergeant Methus"));
	ObjectManager:AddObject(sprites["sergeant"]);

	-- TEMP: townspeople that should eventually be moved to the map script for the attack aftermath
	sprites["laila"] = ConstructSprite("Laila", 500, 180, 118);
	sprites["laila"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["laila"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["laila"]);

	sprites["marcus"] = ConstructSprite("Marcus", 501, 186, 112);
	sprites["marcus"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["marcus"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["marcus"]);

	sprites["vanica"] = ConstructSprite("Vanica", 502, 166, 112);
	sprites["vanica"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["vanica"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["vanica"]);

	sprites["weapon_merchant"] = ConstructSprite("Alexander", 510, 144, 140);
	sprites["weapon_merchant"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["weapon_merchant"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["weapon_merchant"]);

	sprites["item_merchant"] = ConstructSprite("Female Merchant", 520, 73, 173);
	sprites["item_merchant"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["item_merchant"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["item_merchant"]);

	sprites["inn_keeper"] = ConstructSprite("Octavia", 530, 118, 168);
	sprites["inn_keeper"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["inn_keeper"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["inn_keeper"]);

	sprites["inn_worker"] = ConstructSprite("Livia", 531, 113, 174);
	sprites["inn_worker"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["inn_worker"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["inn_worker"]);
end -- function CreateSprites()



function CreateEnemies()
	IfPrintDebug(DEBUG, "Creating enemies...");
end



function CreateDialogues()
	IfPrintDebug(DEBUG, "Creating dialogues...");

	event_dialogues = {}; -- Holds IDs of the dialogues used during events

	local dialogue;
	local text;

	----------------------------------------------------------------------------
	---------- Dialogues attached to characters
	----------------------------------------------------------------------------
	dialogue = hoa_map.MapDialogue.Create(10);
		text = hoa_system.Translate("The city is under attack by demons. We'll protect the citizens. Make your way to the castle with haste!");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
	sprites["captain"]:AddDialogueReference(10);

	dialogue = hoa_map.MapDialogue.Create(11);
		text = hoa_system.Translate("Go! We'll manage here.");
		dialogue:AddLine(text, sprites["sergeant"]:GetObjectID());
	sprites["sergeant"]:AddDialogueReference(11);

	-- TEMP: dialogues below should eventually be moved to the map script for the attack aftermath
	dialogue = hoa_map.MapDialogue.Create(100);
		text = hoa_system.Translate("Welcome home, Claudius. What's the matter?");
		dialogue:AddLine(text, sprites["laila"]:GetObjectID());
		text = hoa_system.Translate("It's...nothing. I'm just glad you're okay.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
	sprites["laila"]:AddDialogueReference(100);

	dialogue = hoa_map.MapDialogue.Create(101);
		text = hoa_system.Translate("Oh dear, oh dear.");
		dialogue:AddLine(text, sprites["vanica"]:GetObjectID());
	sprites["vanica"]:AddDialogueReference(101);

	dialogue = hoa_map.MapDialogue.Create(102);
		text = hoa_system.Translate("Your mother and I are doing what we can for those who lost their homes in the attack.");
		dialogue:AddLine(text, sprites["marcus"]:GetObjectID());
		text = hoa_system.Translate("Let me help. Show me where I can make myself useful.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("No, you need to rest. We may be attacked again you know, and how are you going to defend our city if you're exhausted?");
		dialogue:AddLine(text, sprites["marcus"]:GetObjectID());
	sprites["marcus"]:AddDialogueReference(102);

	dialogue = hoa_map.MapDialogue.Create(103);
		text = hoa_system.Translate("Sorry, this inn has converted into a shelter after the attack and we're completely full.");
		dialogue:AddLine(text, sprites["inn_keeper"]:GetObjectID());
	sprites["inn_keeper"]:AddDialogueReference(103);

	dialogue = hoa_map.MapDialogue.Create(104);
		text = hoa_system.Translate("So much work to do. I'm exhausted.");
		dialogue:AddLine(text, sprites["inn_worker"]:GetObjectID());
	sprites["inn_worker"]:AddDialogueReference(104);

	dialogue = hoa_map.MapDialogue.Create(105);
		text = hoa_system.Translate("Most of my inventory was destroyed in the attack.");
		dialogue:AddLine(text, sprites["item_merchant"]:GetObjectID());
		dialogue:AddLineEventAtEnd(1000); -- event_chains["item_shop"]
	sprites["item_merchant"]:AddDialogueReference(105);

	dialogue = hoa_map.MapDialogue.Create(106);
		text = hoa_system.Translate("Demand has skyrocketed after our city was attacked. I've sold most of my armaments, but I have a few selections remaining.");
		dialogue:AddLine(text, sprites["weapon_merchant"]:GetObjectID());
		dialogue:AddLineEventAtEnd(1001); -- event_chains["weapon_armor_shop"] = 1001;
	sprites["weapon_merchant"]:AddDialogueReference(106);

	----------------------------------------------------------------------------
	---------- Dialogues triggered by events
	----------------------------------------------------------------------------
	event_dialogues["opening"] = 1000;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["opening"]);
		text = hoa_system.Translate("Wh...what the hell is going on? What the hell are they?!");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("Captain!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Lukar! Take your squad and make your way to the castle. Inform the captain of the royal guard of our return and ask for orders.");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
		text = hoa_system.Translate("The rest of us will repel these demons! Now go! Defend our people and our homes!");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
		-- TEMP: Remove after development release is published
		text = hoa_system.Translate("The rest of these events are not yet finished. You may explore this map, enter the various structures, and speak with NPCs. Thanks for playing!");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		
end -- function CreateDialogues()


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	IfPrintDebug(DEBUG, "Creating events...");

	event_chains = {}; -- Holds IDs of the starting event for each event chain
	local event = {};

	-- Event Group #1: Initial map scene -- camera pans across a stretch of the city under attack before focusing on the captain
	event_chains["intro_scene"] = 10;
	event = hoa_map.CustomEvent.Create(event_chains["intro_scene"], "StartIntroScene", "");
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 1, 2000);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 1, Map.virtual_focus, 98, 130);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 2);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 2, Map.virtual_focus, sprites["captain"].x_position, sprites["captain"].y_position);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 3);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 4);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 5);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 3, sprites["claudius"], 0, -22);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 4, sprites["mark"], 0, -22);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 5, sprites["lukar"], 0, -22);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 6);
	event = hoa_map.DialogueEvent.Create(event_chains["intro_scene"] + 6, event_dialogues["opening"]);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 7);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 8);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 9);
	-- Move Claudius, Lukar and Mark to the same position
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 8, sprites["claudius"], 98, 185);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 8, sprites["mark"], 98, 185);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["intro_scene"] + 9, sprites["lukar"], 98, 185);
	event:AddEventLinkAtEnd(event_chains["intro_scene"] + 10);
	event = hoa_map.CustomEvent.Create(event_chains["intro_scene"] + 10, "", "EndIntroScene");

	----------------------------------------------------------------------------
	---------- Miscellaneous Events
	----------------------------------------------------------------------------
	event_chains["item_shop"] = 1000;
	event = hoa_map.CustomEvent.Create(event_chains["item_shop"], "LoadItemShop", "");

	event_chains["weapon_armor_shop"] = 1001;
	event = hoa_map.CustomEvent.Create(event_chains["weapon_armor_shop"], "LoadWeaponArmorShop", "");

end -- function CreateEvents()

----------------------------------------------------------------------------
---------- Event Functions
----------------------------------------------------------------------------


-- Move camera to just below the right staircase leading to the castle for the start of the scene
functions["StartIntroScene"] = function()
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 1.0), 0); -- Initially set the screen to black
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 0.0), 1000); -- Gradually fade the screen back in
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	Map:MoveVirtualFocus(130, 98);
	Map:SetCamera(Map.virtual_focus, 0);
	Map.virtual_focus:SetMovementSpeed(hoa_map.MapMode.VERY_FAST_SPEED - 25); -- Custom speed for fast camera panning
end


-- Hide all character sprites but Claudius and give control over to the player. Will not return success unless all
-- character sprites are not in motion
functions["EndIntroScene"] = function()
	if (sprites["claudius"]:IsMoving() == true or sprites["mark"]:IsMoving() == true or sprites["lukar"]:IsMoving() == true) then
		return false;
	end

	sprites["mark"]:SetVisible(false);
	sprites["lukar"]:SetVisible(false);
	sprites["claudius"]:SetNoCollision(false);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	Map:SetCamera(sprites["claudius"], 500);
	Map:PopState();
	return true;
end


-- Sprite function: Focus map camera on sprite
functions["FocusCameraOnSprite"] = function(sprite)
	Map:SetCamera(sprite, 1000);
end


-- Pop current map state
functions["PopState"] = function()
	Map:PopState();
end


-- Puts game state into shop mode with items
functions["LoadItemShop"] = function()
	LoadNewShop(
		1, 4 -- healing potions
	);
end


-- Puts game state into shop mode with items
functions["LoadWeaponArmorShop"] = function()
	LoadNewShop(
		10002, 1, -- iron sword
		20001, 2, -- karlate helm
		30001, 3, -- leather chain mail
		30002, 1  -- karlate breast plate
	);
end

