// Sound effects example
// This sample demonstrates:
//     - Playing sound effects and music
//     - Controlling sound and music master volume

#include "Scripts/Utilities/Sample.as"

Array<String> soundNames = {
    "Fist",
    "Explosion",
    "Power-up"
};

Array<String> soundResourceNames = {
    "Sounds/PlayerFistHit.wav",
    "Sounds/BigExplosion.wav",
    "Sounds/Powerup.wav"
};

SoundSource@ musicSource;

void Start()
{
    // Execute the common startup for samples
    SampleStart();

    // Create a scene which will not be actually rendered, but is used to hold SoundSource components while they play sounds
    scene_ = Scene();

    // Create music sound source
    @musicSource = scene_.CreateComponent("SoundSource");
    // Set the sound type to music so that master volume control works correctly
    musicSource.soundType = SOUND_MUSIC;

    // Enable OS cursor
    input.mouseVisible = true;

    // Create the user interface
    CreateUI();

    // Set the mouse mode to use in the sample
    SampleInitMouseMode(MM_FREE);
}

void CreateUI()
{
    XMLFile@ uiStyle = cache.GetResource("XMLFile", "UI/DefaultStyle.xml");
    // Set style to the UI root so that elements will inherit it
    ui.root.defaultStyle = uiStyle;

    // Create buttons for playing back sounds
    for (uint i = 0; i < soundNames.length; ++i)
    {
        Button@ button = CreateButton(i * 140 + 20, 20, 120, 40, soundNames[i]);
        // Store the sound effect resource name as a custom variable into the button
        button.vars["SoundResource"] = soundResourceNames[i];
        SubscribeToEvent(button, "Pressed", "HandlePlaySound");
    }

    // Create buttons for playing/stopping music
    Button@ button = CreateButton(20, 80, 120, 40, "Play Music");
    SubscribeToEvent(button, "Released", "HandlePlayMusic");

    button = CreateButton(160, 80, 120, 40, "Stop Music");
    SubscribeToEvent(button, "Released", "HandleStopMusic");

    // Create sliders for controlling sound and music master volume
    Slider@ slider = CreateSlider(20, 140, 200, 20, "Sound Volume");
    slider.value = audio.masterGain[SOUND_EFFECT];
    SubscribeToEvent(slider, "SliderChanged", "HandleSoundVolume");

    slider = CreateSlider(20, 200, 200, 20, "Music Volume");
    slider.value = audio.masterGain[SOUND_MUSIC];
    SubscribeToEvent(slider, "SliderChanged", "HandleMusicVolume");
}

Button@ CreateButton(int x, int y, int xSize, int ySize, const String&in text)
{
    Font@ font = cache.GetResource("Font", "Fonts/Anonymous Pro.ttf");

    // Create the button and center the text onto it
    Button@ button = ui.root.CreateChild("Button");
    button.SetStyleAuto();
    button.SetPosition(x, y);
    button.SetSize(xSize, ySize);

    Text@ buttonText = button.CreateChild("Text");
    buttonText.SetAlignment(HA_CENTER, VA_CENTER);
    buttonText.SetFont(font, 12);
    buttonText.text = text;

    return button;
}

Slider@ CreateSlider(int x, int y, int xSize, int ySize, const String& text)
{
    Font@ font = cache.GetResource("Font", "Fonts/Anonymous Pro.ttf");

    // Create text and slider below it
    Text@ sliderText = ui.root.CreateChild("Text");
    sliderText.SetPosition(x, y);
    sliderText.SetFont(font, 12);
    sliderText.text = text;

    Slider@ slider = ui.root.CreateChild("Slider");
    slider.SetStyleAuto();
    slider.SetPosition(x, y + 20);
    slider.SetSize(xSize, ySize);
    // Use 0-1 range for controlling sound/music master volume
    slider.range = 1.0f;

    return slider;
}

void HandlePlaySound(StringHash eventType, VariantMap& eventData)
{
    Button@ button = GetEventSender();
    String soundResourceName = button.vars["SoundResource"].GetString();

    // Get the sound resource
    Sound@ sound = cache.GetResource("Sound", soundResourceName);

    if (sound !is null)
    {
        // Create a SoundSource component for playing the sound. The SoundSource component plays
        // non-positional audio, so its 3D position in the scene does not matter. For positional sounds the
        // SoundSource3D component would be used instead
        SoundSource@ soundSource = scene_.CreateComponent("SoundSource");
        soundSource.autoRemoveMode = REMOVE_COMPONENT;
        soundSource.Play(sound);
        // In case we also play music, set the sound volume below maximum so that we don't clip the output
        soundSource.gain = 0.7f;
    }
}

void HandlePlayMusic(StringHash eventType, VariantMap& eventData)
{
    Sound@ music = cache.GetResource("Sound", "Music/Ninja Gods.ogg");
    // Set the song to loop
    music.looped = true;

    musicSource.Play(music);
}

void HandleStopMusic(StringHash eventType, VariantMap& eventData)
{
    musicSource.Stop();
}

void HandleSoundVolume(StringHash eventType, VariantMap& eventData)
{
    float newVolume = eventData["Value"].GetFloat();
    audio.masterGain[SOUND_EFFECT] = newVolume;
}

void HandleMusicVolume(StringHash eventType, VariantMap& eventData)
{
    float newVolume = eventData["Value"].GetFloat();
    audio.masterGain[SOUND_MUSIC] = newVolume;
}

// Create XML patch instructions for screen joystick layout specific to this sample app
String patchInstructions =
        "<patch>" +
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button2']]\">" +
        "        <attribute name=\"Is Visible\" value=\"false\" />" +
        "    </add>" +
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Hat0']]\">" +
        "        <attribute name=\"Is Visible\" value=\"false\" />" +
        "    </add>" +
        "</patch>";
