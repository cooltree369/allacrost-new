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
DEBUG = true;

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
contexts["exterior"] = hoa_map.MapMode.CONTEXT_02; -- This is the "default" context for this map. CONTEXT_01 is not used
contexts["interior_a"] = hoa_map.MapMode.CONTEXT_03;
contexts["interior_b"] = hoa_map.MapMode.CONTEXT_04;
contexts["interior_c"] = hoa_map.MapMode.CONTEXT_05;
contexts["interior_d"] = hoa_map.MapMode.CONTEXT_06;
contexts["interior_e"] = hoa_map.MapMode.CONTEXT_07;



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
	Map.virtual_focus:SetContext(contexts["exterior"]);
	Map.unlimited_stamina = true;
	Map:ShowStaminaBar(true);
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
	zones["throne_room"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior_a"]);
	zones["throne_room"]:AddSection(96, 100, 62, 63, false);
	zones["throne_room"]:AddSection(96, 100, 63, 64, true);
	Map:AddZone(zones["throne_room"]);

	zones["left_tower_entrance"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior_b"]);
	zones["left_tower_entrance"]:AddSection(72, 76, 70, 71, false);
	zones["left_tower_entrance"]:AddSection(72, 76, 71, 72, true);
	Map:AddZone(zones["left_tower_entrance"]);

	zones["right_tower_entrance"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior_c"]);
	zones["right_tower_entrance"]:AddSection(120, 124, 70, 71, false);
	zones["right_tower_entrance"]:AddSection(120, 124, 71, 72, true);
	Map:AddZone(zones["right_tower_entrance"]);

	zones["left_tower_balcony"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior_b"]);
	zones["left_tower_balcony"]:AddSection(84, 85, 62, 66, false);
	zones["left_tower_balcony"]:AddSection(85, 86, 62, 66, true);
	Map:AddZone(zones["left_tower_balcony"]);

	zones["right_tower_balcony"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior_c"]);
	zones["right_tower_balcony"]:AddSection(110, 111, 62, 66, true);
	zones["right_tower_balcony"]:AddSection(111, 112, 62, 66, false);
	Map:AddZone(zones["right_tower_balcony"]);
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
	sprites["claudius"]:SetContext(contexts["exterior"]);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["claudius"]);

	sprites["mark"] = ConstructSprite("Knight01", 2, entrance_x + 2, entrance_y - 2);
	sprites["mark"]:SetContext(contexts["exterior"]);
	sprites["mark"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["mark"]:SetName(hoa_system.Translate("Mark"));
	ObjectManager:AddObject(sprites["mark"]);

	sprites["lukar"] = ConstructSprite("Knight01", 3, entrance_x, entrance_y - 4);
	sprites["lukar"]:SetContext(contexts["exterior"]);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	ObjectManager:AddObject(sprites["lukar"]);

	-- Create the captain and the NPCs fighting the battle straight ahead of the city entrance
	-- Coordinates for NPC battle that lies straight ahead on the road after entering the town
	local straight_battle_x = 98;
	local straight_battle_y = 175;

	sprites["captain"] = ConstructSprite("Knight06", 10, straight_battle_x, straight_battle_y + 4);
	sprites["captain"]:SetContext(contexts["exterior"]);
	sprites["captain"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["captain"]:SetName(hoa_system.Translate("Captain Bravis"));
	ObjectManager:AddObject(sprites["captain"]);

	sprites["knight01"] = ConstructSprite("Knight01", 11, straight_battle_x - 3, straight_battle_y);
	sprites["knight01"]:SetContext(contexts["exterior"]);
	sprites["knight01"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["knight01"]);

	sprites["knight02"] = ConstructSprite("Knight02", 12, straight_battle_x + 4, straight_battle_y - 2);
	sprites["knight02"]:SetContext(contexts["exterior"]);
	sprites["knight02"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["knight02"]);

	-- Create the senior knight and others fighting near the inn
	-- Coordinates for NPC battle that lies near the entrance to the inn
	local inn_battle_x = 123;
	local inn_battle_y = 188;

	sprites["senior_knight"] = ConstructSprite("Knight04", 12, inn_battle_x - 1, inn_battle_y);
	sprites["senior_knight"]:SetContext(contexts["exterior"]);
	sprites["senior_knight"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["senior_knight"]);

	sprites["knight03"] = ConstructSprite("Knight03", 15, inn_battle_x, inn_battle_y + 2);
	sprites["knight03"]:SetContext(contexts["exterior"]);
	sprites["knight03"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["knight03"]);

	sprites["knight04"] = ConstructSprite("Knight01", 16, inn_battle_x + 2, inn_battle_y - 2);
	sprites["knight04"]:SetContext(contexts["exterior"]);
	sprites["knight04"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["knight04"]);

	-- Create the sergeant and others fighting near the item shop entrance
	-- Coordinates for NPC battle that takes place near the entrance to the item shop
	local item_shop_battle_x = 84;
	local item_shop_battle_y = 184;
	sprites["sergeant"] = ConstructSprite("Knight05", 11, item_shop_battle_x + 3, item_shop_battle_y - 2);
	sprites["sergeant"]:SetContext(contexts["exterior"]);
	sprites["sergeant"]:SetDirection(hoa_map.MapMode.WEST);
	sprites["sergeant"]:SetName(hoa_system.Translate("Sergeant Methus"));
	ObjectManager:AddObject(sprites["sergeant"]);
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

	event_dialogues["demon_spawn"] = 1001;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["demon_spawn"]);
		text = hoa_system.Translate("I don't believe what I just saw.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("That demon just emerged from the shadows.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Well that's just great! How the hell are we supposed to stop an invasion that comes through shadows?");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("Here it comes!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

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
