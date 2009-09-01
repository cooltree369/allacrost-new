------------------------------------------------------------------------------[[
-- Filename: defense.lua
--
-- Description: This file contains the definitions of all defense skills that
-- exist in Hero of Allacrost. Each defense skill has a unique integer identifier
-- that is used as its key in the skills table below.
------------------------------------------------------------------------------]]

-- All defense skills definitions are stored in this table
if (skills == nil) then
   skills = {}
end


-- -----------------------------------------------------------------------------
-- IDs 10001-20000 are reserved for defense skills
-- -----------------------------------------------------------------------------

skills[10001] = {
	name = "Karlate Guard",
	description = "Take a strong defensive stance.",
	sp_required = 3,
	warmup_time = 300,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_SELF,
	target_ally = true,

	BattleExecute = function(target, instigator)
		instigator:AddNewEffect(2);
	end
}

skills[10002] = {
	name = "Dodge Enemies",
	description = "Increases evasion ability for a brief period.",
	sp_required = 1,
	warmup_time = 300,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_SELF,
	target_ally = true,

	BattleExecute = function(target, instigator)
		instigator:AddNewEffect(4);
	end
}
