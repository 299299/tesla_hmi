-- Console input example.
-- This sample demonstrates:
--     - Implementing a crude text adventure game, which accepts input both through the engine console,
--       and standard input.
--     - Disabling default execution of console commands as immediate mode Lua script.
--     - Adding autocomplete options to the engine console.

require "LuaScripts/Utilities/Sample"

local gameOn = false
local foodAvailable = false
local eatenLastTurn = false
local numTurns = 0
local hunger = 0
local urhoThreat = 0

-- Hunger level descriptions
local hungerLevels = {
    "bursting",
    "well-fed",
    "fed",
    "hungry",
    "very hungry",
    "starving"
}

-- Urho threat level descriptions
local urhoThreatLevels = {
    "Suddenly Urho appears from a dark corner of the fish tank",
    "Urho seems to have his eyes set on you",
    "Urho is homing in on you mercilessly"
}

function Start()
    -- Execute the common startup for samples
    SampleStart()

    -- Disable default execution of Lua from the console
    SetExecuteConsoleCommands(false)

    -- Subscribe to console commands and the frame update
    SubscribeToEvent("ConsoleCommand", "HandleConsoleCommand")
    SubscribeToEvent("Update", "HandleUpdate")

    -- Subscribe key down event
    SubscribeToEvent("KeyDown", "HandleEscKeyDown")

    -- Hide logo to make room for the console
    SetLogoVisible(false)

    -- Show the console by default, make it large
    console.numRows = graphics.height / 16
    console.numBufferedRows = 2 * console.numRows
    console.commandInterpreter = "LuaScriptEventInvoker"
    console.visible = true
    console.closeButton.visible = false
    console:AddAutoComplete("help")
    console:AddAutoComplete("eat")
    console:AddAutoComplete("hide")
    console:AddAutoComplete("wait")
    console:AddAutoComplete("score")
    console:AddAutoComplete("quit")

    -- Show OS mouse cursor
    input.mouseVisible = true

    -- Set the mouse mode to use in the sample
    SampleInitMouseMode(MM_FREE)

    -- Open the operating system console window (for stdin / stdout) if not open yet
    -- Do not open in fullscreen, as this would cause constant device loss
    if not graphics.fullscreen then
        OpenConsoleWindow()
    end

    -- Initialize game and print the welcome message
    StartGame()

    -- Randomize from system clock
    SetRandomSeed(Time:GetSystemTime())
end

function HandleConsoleCommand(eventType, eventData)
    if eventData["Id"]:GetString() == "LuaScriptEventInvoker" then
        HandleInput(eventData["Command"]:GetString())
    end
end

function HandleUpdate(eventType, eventData)
    -- Check if there is input from stdin
    local input = GetConsoleInput()
    if input:len() > 0 then
        HandleInput(input)
    end
end

function HandleEscKeyDown(eventType, eventData)
    -- Unlike the other samples, exiting the engine when ESC is pressed instead of just closing the console
    if eventData["Key"]:GetInt() == KEY_ESCAPE then
        engine:Exit()
    end
end

function StartGame()
    print("Welcome to the Urho adventure game! You are the newest fish in the tank your\n" ..
          "objective is to survive as long as possible. Beware of hunger and the merciless\n" ..
          "predator cichlid Urho, who appears from time to time. Evading Urho is easier\n" ..
          "with an empty stomach. Type 'help' for available commands.")

    gameOn = true
    foodAvailable = false
    eatenLastTurn = false
    numTurns = 0
    hunger = 2
    urhoThreat = 0
end

function EndGame(message)
    print(message)
    print("Game over! You survived " .. numTurns .. " turns.\n" ..
          "Do you want to play again (Y/N)?")

    gameOn = false
end

function Advance()
    if urhoThreat > 0 then
        urhoThreat = urhoThreat + 1
        if urhoThreat > 3 then
            EndGame("Urho has eaten you!")
            return
        end
    elseif urhoThreat < 0 then
        urhoThreat = urhoThreat + 1
    elseif urhoThreat == 0 and Random() < 0.2 then
        urhoThreat = urhoThreat + 1
    end

    if urhoThreat > 0 then
        print(urhoThreatLevels[urhoThreat] .. ".")
    end

    if (numTurns % 4) == 0 and not eatenLastTurn then
        hunger = hunger + 1
        if hunger > 5 then
            EndGame("You have died from starvation!")
            return
        else
            print("You are " .. hungerLevels[hunger + 1] .. ".")
        end
    end

    eatenLastTurn = false

    if foodAvailable then
        print("The floating pieces of fish food are quickly eaten by other fish in the tank.")
        foodAvailable = false
    elseif Random() < 0.15 then
        print("The overhead dispenser drops pieces of delicious fish food to the water!")
        foodAvailable = true
    end

    numTurns = numTurns + 1
end

function TrimInput(input)
  return input:gsub("^%s*(.-)%s*$", "%1")
end

function HandleInput(input)
    local inputLower = TrimInput(input:lower())
    if inputLower:len() == 0 then
        print("Empty input given!")
        return
    end

    if inputLower == "quit" or inputLower == "exit" then
        engine:Exit()
    elseif gameOn then
        -- Game is on
        if inputLower == "help" then
            print("The following commands are available: 'eat', 'hide', 'wait', 'score', 'quit'.")
        elseif inputLower == "score" then
            print("You have survived " .. numTurns .. " turns.")
        elseif inputLower == "eat" then
            if foodAvailable then
                print("You eat several pieces of fish food.")
                foodAvailable = false
                eatenLastTurn = true
                if hunger > 3 then
                    hunger = hunger - 2
                else
                    hunger = hunger - 1
                end
                if hunger < 0 then
                    EndGame("You have killed yourself by over-eating!")
                    return
                else
                    print("You are now " .. hungerLevels[hunger + 1] .. ".")
                end
            else
                print("There is no food available.")
            end

            Advance()
        elseif inputLower == "wait" then
            print("Time passes...")
            Advance()
        elseif inputLower == "hide" then
            if urhoThreat > 0 then
                local evadeSuccess = hunger > 2 or Random() < 0.5
                if evadeSuccess then
                    print("You hide behind the thick bottom vegetation, until Urho grows bored.")
                    urhoThreat = -2
                else
                    print("Your movements are too slow you are unable to hide from Urho.")
                end
            else
                print("There is nothing to hide from.")
            end

            Advance()
        else
            print("Cannot understand the input '" .. input .. "'.")
        end
    else
        -- Game is over, wait for (y)es or (n)o reply
        local c = inputLower:sub(1, 1)
        if c == 'y' then
            StartGame()
        elseif c == 'n' then
            engine:Exit()
        else
            print("Please answer 'y' or 'n'.")
        end
    end
end

-- Create XML patch instructions for screen joystick layout specific to this sample app
function GetScreenJoystickPatchString()
    return
        "<patch>" ..
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button2']]\">" ..
        "        <attribute name=\"Is Visible\" value=\"false\" />" ..
        "    </add>" ..
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Hat0']]\">" ..
        "        <attribute name=\"Is Visible\" value=\"false\" />" ..
        "    </add>" ..
        "</patch>"
end
