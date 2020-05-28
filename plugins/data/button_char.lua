local default_text =
{
	-- Alphabetic Buttons (NeoGeo): A~D,H,Z
	 ["A"] = 1,     -- BTN_A
	 ["B"] = 2,     -- BTN_B
	 ["C"] = 3,     -- BTN_C
	 ["D"] = 4,     -- BTN_D
	 ["H"] = 8,     -- BTN_H
	 ["Z"] = 26,    -- BTN_Z
	-- Numerical Buttons (Capcom): 1~10
	 ["a"] = 27,    -- BTN_1
	 ["b"] = 28,    -- BTN_2
	 ["c"] = 29,    -- BTN_3
	 ["d"] = 30,    -- BTN_4
	 ["e"] = 31,    -- BTN_5
	 ["f"] = 32,    -- BTN_6
	 ["g"] = 33,    -- BTN_7
	 ["h"] = 34,    -- BTN_8
	 ["i"] = 35,    -- BTN_9
	 ["j"] = 36,    -- BTN_10
	-- Directions of Arrow, Joystick Ball
	 ["+"] = 39,    -- BTN_+
	 ["."] = 40,    -- DIR_...
	 ["1"] = 41,    -- DIR_1
	 ["2"] = 42,    -- DIR_2
	 ["3"] = 43,    -- DIR_3
	 ["4"] = 44,    -- DIR_4
	 ["5"] = 45,    -- Joystick Ball
	 ["6"] = 46,    -- DIR_6
	 ["7"] = 47,    -- DIR_7
	 ["8"] = 48,    -- DIR_8
	 ["9"] = 49,    -- DIR_9
	 ["N"] = 50,    -- DIR_N
	-- Special Buttons
	 ["S"] = 51,    -- BTN_START
	 ["P"] = 53,    -- BTN_PUNCH
	 ["K"] = 54,    -- BTN_KICK
	 ["G"] = 55,    -- BTN_GUARD
	-- Composition of Arrow Directions
	 ["!"] = 90,   -- Arrow
	 ["k"] = 100,   -- Half Circle Back
	 ["l"] = 101,   -- Half Circle Front Up
	 ["m"] = 102,   -- Half Circle Front
	 ["n"] = 103,   -- Half Circle Back Up
	 ["o"] = 104,   -- 1/4 Cir For 2 Down
	 ["p"] = 105,   -- 1/4 Cir Down 2 Back
	 ["q"] = 106,   -- 1/4 Cir Back 2 Up
	 ["r"] = 107,   -- 1/4 Cir Up 2 For
	 ["s"] = 108,   -- 1/4 Cir Back 2 Down
	 ["t"] = 109,   -- 1/4 Cir Down 2 For
	 ["u"] = 110,   -- 1/4 Cir For 2 Up
	 ["v"] = 111,   -- 1/4 Cir Up 2 Back
	 ["w"] = 112,   -- Full Clock Forward
	 ["x"] = 113,   -- Full Clock Back
	 ["y"] = 114,   -- Full Count Forward
	 ["z"] = 115,   -- Full Count Back
	 ["L"] = 116,   -- 2x Forward
	 ["M"] = 117,   -- 2x Back
	 ["Q"] = 118,   -- Dragon Screw Forward
	 ["R"] = 119,   -- Dragon Screw Back
	-- Big letter Text
	 ["^"] = 121,   -- AIR
	 ["?"] = 122,   -- DIR
	 ["X"] = 124,   -- TAP
	-- Condition of Positions
	 ["|"] = 125,   -- Jump
	 ["O"] = 126,   -- Hold
	 ["-"] = 127,   -- Air
	 ["="] = 128,   -- Squatting
	 ["~"] = 131,   -- Charge
	-- Special Character Text
	 ["`"] = 135,   -- Small Dot
	 ["@"] = 136,   -- Double Ball
	 [")"] = 137,   -- Single Ball
	 ["("] = 138,   -- Solid Ball
	 ["*"] = 139,   -- Star
	 ["&"] = 140,   -- Solid star
	 ["%"] = 141,   -- Triangle
	 ["$"] = 142,   -- Solid Triangle
	 ["#"] = 143,   -- Double Square
	 ["]"] = 144,   -- Single Square
	 ["["] = 145,   -- Solid Square
	 ["{"] = 146,   -- Down Triangle
	 ["}"] = 147,   -- Solid Down Triangle
	 ["<"] = 148,   -- Diamond
	 [">"] = 149,   -- Solid Diamond
}

local expand_text =
{
	-- Alphabetic Buttons (NeoGeo): S (Slash Button)
	 ["s"] = 19,    -- BTN_S
	-- Special Buttons
	 ["S"] = 52,    -- BTN_SELECT
	-- Multiple Punches & Kicks
	 ["E"] = 57,    -- Light  Punch
	 ["F"] = 58,    -- Middle Punch
	 ["G"] = 59,    -- Strong Punch
	 ["H"] = 60,    -- Light  Kick
	 ["I"] = 61,    -- Middle Kick
	 ["J"] = 62,    -- Strong Kick
	 ["T"] = 63,    -- 3 Kick
	 ["U"] = 64,    -- 3 Punch
	 ["V"] = 65,    -- 2 Kick
	 ["W"] = 66,    -- 2 Pick
	-- Composition of Arrow Directions
	 ["!"] = 91,    -- Continue Arrow
	-- Charge of Arrow Directions
	 ["1"] = 92,    -- Charge DIR_1
	 ["2"] = 93,    -- Charge DIR_2
	 ["3"] = 94,    -- Charge DIR_3
	 ["4"] = 95,    -- Charge DIR_4
	 ["6"] = 96,    -- Charge DIR_6
	 ["7"] = 97,    -- Charge DIR_7
	 ["8"] = 98,    -- Charge DIR_8
	 ["9"] = 99,    -- Charge DIR_9
	-- Big letter Text
	 ["M"] = 123,   -- MAX
	-- Condition of Positions
	 ["-"] = 129,   -- Close
	 ["="] = 130,   -- Away
	 ["*"] = 132,   -- Serious Tap
	 ["?"] = 133,   -- Any Button
}

local convert_text =
{
	-- Alphabetic Buttons: A~Z
	 ["A-button"] = 1, -- BTN_A
	 ["B-button"] = 2, -- BTN_B
	 ["C-button"] = 3, -- BTN_C
	 ["D-button"] = 4, -- BTN_D
	 ["E-button"] = 5, -- BTN_E
	 ["F-button"] = 6, -- BTN_F
	 ["G-button"] = 7, -- BTN_G
	 ["H-button"] = 8, -- BTN_H
	 ["I-button"] = 9, -- BTN_I
	 ["J-button"] = 10, -- BTN_J
	 ["K-button"] = 11, -- BTN_K
	 ["L-button"] = 12, -- BTN_L
	 ["M-button"] = 13, -- BTN_M
	 ["N-button"] = 14, -- BTN_N
	 ["O-button"] = 15, -- BTN_O
	 ["P-button"] = 16, -- BTN_P
	 ["Q-button"] = 17, -- BTN_Q
	 ["R-button"] = 18, -- BTN_R
	 ["S-button"] = 19, -- BTN_S
	 ["T-button"] = 20, -- BTN_T
	 ["U-button"] = 21, -- BTN_U
	 ["V-button"] = 22, -- BTN_V
	 ["W-button"] = 23, -- BTN_W
	 ["X-button"] = 24, -- BTN_X
	 ["Y-button"] = 25, -- BTN_Y
	 ["Z-button"] = 26, -- BTN_Z
	-- Special Moves and Buttons
	 ["decrease"] = 37, -- BTN_DEC
	 ["increase"] = 38, -- BTN_INC
	 ["BALL"] = 45,  -- Joystick Ball
	 ["start"] = 51,   -- BTN_START
	 ["select"] = 52, -- BTN_SELECT
	 ["punch"] = 53,   -- BTN_PUNCH
	 ["kick"] = 54,  -- BTN_KICK
	 ["guard"] = 55,   -- BTN_GUARD
	 ["L-punch"] = 57, -- Light Punch
	 ["M-punch"] = 58, -- Middle Punch
	 ["S-punch"] = 59, -- Strong Punch
	 ["L-kick"] = 60, -- Light Kick
	 ["M-kick"] = 61, -- Middle Kick
	 ["S-kick"] = 62, -- Strong Kick
	 ["3-kick"] = 63, -- 3 Kick
	 ["3-punch"] = 64, -- 3 Punch
	 ["2-kick"] = 65, -- 2 Kick
	 ["2-punch"] = 66, -- 2 Pick
	-- Custom Buttons and Cursor Buttons
	 ["custom1"] = 67, -- CUSTOM_1
	 ["custom2"] = 68, -- CUSTOM_2
	 ["custom3"] = 69, -- CUSTOM_3
	 ["custom4"] = 70, -- CUSTOM_4
	 ["custom5"] = 71, -- CUSTOM_5
	 ["custom6"] = 72, -- CUSTOM_6
	 ["custom7"] = 73, -- CUSTOM_7
	 ["custom8"] = 74, -- CUSTOM_8
	 ["up"] = 75,    -- (Cursor Up)
	 ["down"] = 76,  -- (Cursor Down)
	 ["left"] = 77,  -- (Cursor Left)
	 ["right"] = 78,   -- (Cursor Right)
	-- Player Lever
	 ["lever"] = 79,   -- Non Player Lever
	 ["nplayer"] = 80, -- Gray Color Lever
	 ["1player"] = 81, -- 1 Player Lever
	 ["2player"] = 82, -- 2 Player Lever
	 ["3player"] = 83, -- 3 Player Lever
	 ["4player"] = 84, -- 4 Player Lever
	 ["5player"] = 85, -- 5 Player Lever
	 ["6player"] = 86, -- 6 Player Lever
	 ["7player"] = 87, -- 7 Player Lever
	 ["8player"] = 88, -- 8 Player Lever
	-- Composition of Arrow Directions
	 ["-->"] = 90, -- Arrow
	 ["==>"] = 91, -- Continue Arrow
	 ["hcb"] = 100, -- Half Circle Back
	 ["huf"] = 101, -- Half Circle Front Up
	 ["hcf"] = 102, -- Half Circle Front
	 ["hub"] = 103, -- Half Circle Back Up
	 ["qfd"] = 104, -- 1/4 Cir For 2 Down
	 ["qdb"] = 105, -- 1/4 Cir Down 2 Back
	 ["qbu"] = 106, -- 1/4 Cir Back 2 Up
	 ["quf"] = 107, -- 1/4 Cir Up 2 For
	 ["qbd"] = 108, -- 1/4 Cir Back 2 Down
	 ["qdf"] = 109, -- 1/4 Cir Down 2 For
	 ["qfu"] = 110, -- 1/4 Cir For 2 Up
	 ["qub"] = 111, -- 1/4 Cir Up 2 Back
	 ["fdf"] = 112, -- Full Clock Forward
	 ["fub"] = 113, -- Full Clock Back
	 ["fuf"] = 114, -- Full Count Forward
	 ["fdb"] = 115, -- Full Count Back
	 ["xff"] = 116, -- 2x Forward
	 ["xbb"] = 117, -- 2x Back
	 ["dsf"] = 118, -- Dragon Screw Forward
	 ["dsb"] = 119, -- Dragon Screw Back
	-- Big letter Text
	 ["AIR"] = 121, -- AIR
	 ["DIR"] = 122, -- DIR
	 ["MAX"] = 123, -- MAX
	 ["TAP"] = 124, -- TAP
	-- Condition of Positions
	 ["jump"] = 125,  -- Jump
	 ["hold"] = 126,  -- Hold
	 ["air"] = 127, -- Air
	 ["sit"] = 128, -- Squatting
	 ["close"] = 129, -- Close
	 ["away"] = 130,  -- Away
	 ["charge"] = 131, -- Charge
	 ["tap"] = 132, -- Serious Tap
	 ["button"] = 133, -- Any Button
}

local function convert_char(str)
	str = str:gsub("@(%g+)", function(s) if convert_text[s] then return utf8.char(convert_text[s] + 0xe000) end return s end)
	str = str:gsub("_(%g)", function(s) if default_text[s] then return utf8.char(default_text[s] + 0xe000) end return s end)
	str = str:gsub("%^(%g)", function(s) if expand_text[s] then return utf8.char(expand_text[s] + 0xe000) end return s end)
	return str
end

return convert_char
