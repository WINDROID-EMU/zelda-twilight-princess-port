#ifndef DUSK_VIRTUAL_CONTROLS_HPP
#define DUSK_VIRTUAL_CONTROLS_HPP

#include <SDL3/SDL.h>
#include <array>
#include <string>

namespace dusk::virtual_controls {

enum class VirtualButton {
    A,
    B,
    X,
    Y,
    Start,
    Z,
    L,
    R,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    Back,  // Android back button / Menu button
    Count
};

enum class VirtualAxis {
    LeftStickX,
    LeftStickY,
    RightStickX,
    RightStickY,
    Count
};

// Preset layouts
enum class LayoutPreset {
    Dolphin,      // GameCube/Dolphin style (default)
    Xbox,         // Xbox controller style
    PlayStation,  // PlayStation style
    Mobile,       // Mobile-optimized style
    Custom,       // User-defined custom layout
    Count
};

struct VirtualButtonState {
    bool pressed = false;
    bool wasPressed = false;
    SDL_FRect rect = {0, 0, 0, 0};
};

struct VirtualJoystick {
    SDL_FRect baseRect = {0, 0, 0, 0};
    SDL_FRect stickRect = {0, 0, 0, 0};
    float x = 0.0f;
    float y = 0.0f;
    bool touched = false;
    int touchId = -1;
};

// Configuration for a single button
struct ButtonConfig {
    float x = 0.0f;
    float y = 0.0f;
    float width = 60.0f;
    float height = 60.0f;
    bool visible = true;
};

// Configuration for a joystick
struct JoystickConfig {
    float x = 0.0f;
    float y = 0.0f;
    float size = 120.0f;
    bool visible = true;
};

// Complete layout configuration
struct LayoutConfig {
    std::array<ButtonConfig, static_cast<size_t>(VirtualButton::Count)> buttons;
    JoystickConfig leftStick;
    JoystickConfig rightStick;
    float opacity = 0.7f;
    float buttonScale = 1.0f;
    bool showLabels = true;
    LayoutPreset preset = LayoutPreset::Dolphin;
};

struct VirtualControlsState {
    std::array<VirtualButtonState, static_cast<size_t>(VirtualButton::Count)> buttons;
    VirtualJoystick leftStick;
    VirtualJoystick rightStick;
    bool enabled = false;
    bool editMode = false;  // Layout editing mode
    LayoutConfig layout;
};

void init();
void shutdown();
void update();
void renderImGui(int screenWidth, int screenHeight);
bool handleTouchEvent(const SDL_TouchFingerEvent& event);
VirtualControlsState& getState();

// Layout management
void setLayoutPreset(LayoutPreset preset);
void loadCustomLayout(const std::string& configJson);
std::string saveCustomLayout();
void resetToDefaultLayout();
void enterEditMode();
void exitEditMode();
bool isInEditMode();

// Layout presets
void applyDolphinLayout(int screenWidth, int screenHeight);
void applyXboxLayout(int screenWidth, int screenHeight);
void applyPlayStationLayout(int screenWidth, int screenHeight);
void applyMobileLayout(int screenWidth, int screenHeight);
void applyCustomLayout(int screenWidth, int screenHeight);

// Individual button/joystick editing
void setButtonPosition(VirtualButton button, float x, float y);
void setButtonSize(VirtualButton button, float width, float height);
void setJoystickPosition(bool left, float x, float y);
void setJoystickSize(bool left, float size);
void setButtonVisible(VirtualButton button, bool visible);
void setJoystickVisible(bool left, bool visible);

const char* getLayoutPresetName(LayoutPreset preset);

} // namespace dusk::virtual_controls

#endif // DUSK_VIRTUAL_CONTROLS_HPP
