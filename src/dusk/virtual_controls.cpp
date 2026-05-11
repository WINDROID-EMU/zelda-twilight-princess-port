#include "dusk/virtual_controls.hpp"
#include "dusk/settings.h"
#include "aurora/lib/logging.hpp"
#include "dusk/ui/ui.hpp"
#include "dusk/ui/menu_bar.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <cmath>

namespace dusk::virtual_controls {

static VirtualControlsState s_state;
static SDL_Gamepad* s_virtualGamepad = nullptr;
static SDL_Joystick* s_virtualJoystick = nullptr;
static aurora::Module s_logModule("dusk::virtual_controls");

constexpr float BUTTON_SIZE = 60.0f;
constexpr float BUTTON_SPACING = 10.0f;
constexpr float JOYSTICK_SIZE = 120.0f;
constexpr float JOYSTICK_DEADZONE = 0.1f;

void init() {
    s_logModule.info("Initializing virtual controls");
    
    // Initialize button positions (will be updated based on screen size)
    // Default positions for portrait mobile layout
    float yPos = 0.0f;
    
    // D-Pad (left side)
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadUp)].rect = {50.0f, 200.0f, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadDown)].rect = {50.0f, 270.0f + BUTTON_SPACING, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadLeft)].rect = {20.0f, 235.0f + BUTTON_SPACING, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadRight)].rect = {80.0f + BUTTON_SPACING, 235.0f + BUTTON_SPACING, BUTTON_SIZE, BUTTON_SIZE};
    
    // Action buttons (right side)
    s_state.buttons[static_cast<size_t>(VirtualButton::A)].rect = {0.0f, 0.0f, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::B)].rect = {0.0f, 0.0f, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::X)].rect = {0.0f, 0.0f, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::Y)].rect = {0.0f, 0.0f, BUTTON_SIZE, BUTTON_SIZE};
    
    // Shoulder buttons
    s_state.buttons[static_cast<size_t>(VirtualButton::L)].rect = {50.0f, 50.0f, BUTTON_SIZE * 2, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::R)].rect = {0.0f, 50.0f, BUTTON_SIZE * 2, BUTTON_SIZE};
    
    // Start and Z
    s_state.buttons[static_cast<size_t>(VirtualButton::Start)].rect = {0.0f, 0.0f, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::Z)].rect = {0.0f, 0.0f, BUTTON_SIZE, BUTTON_SIZE};
    
    // Left joystick
    s_state.leftStick.baseRect = {50.0f, 400.0f, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.leftStick.stickRect = {50.0f + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 400.0f + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE};
    
    // Right joystick
    s_state.rightStick.baseRect = {0.0f, 400.0f, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.rightStick.stickRect = {0.0f + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 400.0f + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE};
    
    s_state.enabled = dusk::getSettings().game.enableVirtualControls.getValue();
}

void shutdown() {
    s_logModule.info("Shutting down virtual controls");
    if (s_virtualGamepad) {
        SDL_CloseGamepad(s_virtualGamepad);
        s_virtualGamepad = nullptr;
    }
    if (s_virtualJoystick) {
        SDL_CloseJoystick(s_virtualJoystick);
        s_virtualJoystick = nullptr;
    }
}

void applyDolphinLayout(int screenWidth, int screenHeight) {
    // Dolphin Emulator / GameCube Controller Layout
    const float margin = 20.0f;
    const float bottomMargin = 40.0f;
    const float topMargin = 60.0f;
    
    // ═══════════════════════════════════════════════════════════════════════════
    // LADO ESQUERDO (Control Stick + D-Pad) - Estilo Dolphin
    // ═══════════════════════════════════════════════════════════════════════════
    
    // Control Stick (Joystick Esquerdo) - Posição central-esquerda, igual ao GameCube
    float leftStickX = margin + 20.0f;
    float leftStickY = screenHeight - 200.0f - bottomMargin;
    s_state.leftStick.baseRect = {leftStickX, leftStickY, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.leftStick.stickRect = {leftStickX + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                    leftStickY + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                    BUTTON_SIZE, BUTTON_SIZE};
    
    // D-Pad - Abaixo do Control Stick (estilo GameCube)
    float dpadX = leftStickX + (JOYSTICK_SIZE - BUTTON_SIZE * 3 - BUTTON_SPACING * 2) / 2;
    float dpadY = leftStickY + JOYSTICK_SIZE + BUTTON_SPACING * 2;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadUp)].rect = 
        {dpadX + BUTTON_SIZE + BUTTON_SPACING, dpadY, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadDown)].rect = 
        {dpadX + BUTTON_SIZE + BUTTON_SPACING, dpadY + BUTTON_SIZE * 2 + BUTTON_SPACING, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadLeft)].rect = 
        {dpadX, dpadY + BUTTON_SIZE + BUTTON_SPACING, BUTTON_SIZE, BUTTON_SIZE};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadRight)].rect = 
        {dpadX + BUTTON_SIZE * 2 + BUTTON_SPACING * 2, dpadY + BUTTON_SIZE + BUTTON_SPACING, BUTTON_SIZE, BUTTON_SIZE};
    
    // ═══════════════════════════════════════════════════════════════════════════
    // LADO DIREITO (Botões de Ação + C-Stick) - Estilo Dolphin/GameCube
    // ═══════════════════════════════════════════════════════════════════════════
    
    // Botões de Ação - Layout em diamante estilo GameCube
    //      [Y]
    //  [X]     [A]
    //      [B]
    float actionCenterX = screenWidth - margin - 100.0f;
    float actionCenterY = screenHeight - 220.0f - bottomMargin;
    float buttonOffset = BUTTON_SIZE + BUTTON_SPACING;
    
    // A - Centro (maior destaque no GameCube)
    s_state.buttons[static_cast<size_t>(VirtualButton::A)].rect = 
        {actionCenterX, actionCenterY, BUTTON_SIZE * 1.2f, BUTTON_SIZE * 1.2f};
    
    // B - Abaixo e à esquerda de A
    s_state.buttons[static_cast<size_t>(VirtualButton::B)].rect = 
        {actionCenterX - buttonOffset * 0.8f, actionCenterY + buttonOffset * 0.8f, 
         BUTTON_SIZE, BUTTON_SIZE};
    
    // X - À esquerda de A
    s_state.buttons[static_cast<size_t>(VirtualButton::X)].rect = 
        {actionCenterX - buttonOffset, actionCenterY, BUTTON_SIZE, BUTTON_SIZE};
    
    // Y - Acima de A
    s_state.buttons[static_cast<size_t>(VirtualButton::Y)].rect = 
        {actionCenterX, actionCenterY - buttonOffset, BUTTON_SIZE, BUTTON_SIZE};
    
    // C-Stick (Joystick Direito) - Abaixo dos botões de ação, igual ao GameCube
    float rightStickX = actionCenterX - JOYSTICK_SIZE / 2 + BUTTON_SIZE * 0.6f;
    float rightStickY = actionCenterY + buttonOffset * 1.8f;
    s_state.rightStick.baseRect = {rightStickX, rightStickY, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.rightStick.stickRect = {rightStickX + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                     rightStickY + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                     BUTTON_SIZE, BUTTON_SIZE};
    
    // ═══════════════════════════════════════════════════════════════════════════
    // BOTÕES DE OMBRO (L, R, Z) - Topo da tela
    // ═══════════════════════════════════════════════════════════════════════════
    
    // L - Ombro esquerdo (maior, como no GameCube)
    s_state.buttons[static_cast<size_t>(VirtualButton::L)].rect = 
        {margin, topMargin, BUTTON_SIZE * 3, BUTTON_SIZE * 1.2f};
    
    // R - Ombro direito (maior, como no GameCube)
    s_state.buttons[static_cast<size_t>(VirtualButton::R)].rect = 
        {screenWidth - BUTTON_SIZE * 3 - margin, topMargin, 
         BUTTON_SIZE * 3, BUTTON_SIZE * 1.2f};
    
    // Z - Acima dos botões de ação (estilo GameCube)
    s_state.buttons[static_cast<size_t>(VirtualButton::Z)].rect = 
        {actionCenterX + BUTTON_SIZE * 1.5f, actionCenterY - buttonOffset * 0.5f, 
         BUTTON_SIZE * 1.5f, BUTTON_SIZE * 0.8f};
    
    // Start - Centro inferior
    s_state.buttons[static_cast<size_t>(VirtualButton::Start)].rect = 
        {screenWidth / 2.0f - BUTTON_SIZE / 2.0f, 
         screenHeight - BUTTON_SIZE - bottomMargin, 
         BUTTON_SIZE, BUTTON_SIZE};
}

void updateLayout(int screenWidth, int screenHeight) {
    applyDolphinLayout(screenWidth, screenHeight);
}

void injectSDLEvent(SDL_Event& event) {
    SDL_PushEvent(&event);
}

void update() {
    s_state.enabled = dusk::getSettings().game.enableVirtualControls.getValue();
    
    if (!s_state.enabled) {
        return;
    }
    
    // Generate SDL events based on virtual control state
    SDL_Event event = {};
    
    // Process buttons
    for (size_t i = 0; i < static_cast<size_t>(VirtualButton::Count); ++i) {
        auto& button = s_state.buttons[i];
        
        if (button.pressed != button.wasPressed) {
            // Special handling for Back button - open menu
            if (static_cast<VirtualButton>(i) == VirtualButton::Back) {
                if (button.pressed) {
                    // Find and open the MenuBar from document stack
                    auto& docs = dusk::ui::get_document_stack();
                    for (auto& doc : docs) {
                        if (auto* menuBar = dynamic_cast<dusk::ui::MenuBar*>(doc.get())) {
                            if (!menuBar->visible()) {
                                menuBar->show();
                            }
                            break;
                        }
                    }
                }
                button.wasPressed = button.pressed;
                continue;
            }
            
            event.type = button.pressed ? SDL_EVENT_GAMEPAD_BUTTON_DOWN : SDL_EVENT_GAMEPAD_BUTTON_UP;
            event.gbutton.timestamp = SDL_GetTicks();
            event.gbutton.which = 0; // Virtual controller index
            
            // Map virtual button to SDL gamepad button
            SDL_GamepadButton sdlButton;
            switch (static_cast<VirtualButton>(i)) {
                case VirtualButton::A: sdlButton = SDL_GAMEPAD_BUTTON_SOUTH; break;
                case VirtualButton::B: sdlButton = SDL_GAMEPAD_BUTTON_EAST; break;
                case VirtualButton::X: sdlButton = SDL_GAMEPAD_BUTTON_WEST; break;
                case VirtualButton::Y: sdlButton = SDL_GAMEPAD_BUTTON_NORTH; break;
                case VirtualButton::Start: sdlButton = SDL_GAMEPAD_BUTTON_START; break;
                case VirtualButton::Z: sdlButton = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER; break;
                case VirtualButton::L: sdlButton = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER; break;
                case VirtualButton::R: sdlButton = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER; break;
                case VirtualButton::DPadUp: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_UP; break;
                case VirtualButton::DPadDown: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_DOWN; break;
                case VirtualButton::DPadLeft: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_LEFT; break;
                case VirtualButton::DPadRight: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_RIGHT; break;
                default: continue;
            }
            
            event.gbutton.button = sdlButton;
            injectSDLEvent(event);
            
            button.wasPressed = button.pressed;
        }
    }
    
    // Process joysticks
    event.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
    event.gaxis.timestamp = SDL_GetTicks();
    event.gaxis.which = 0;
    
    // Left stick X
    if (std::abs(s_state.leftStick.x) > JOYSTICK_DEADZONE) {
        event.gaxis.axis = SDL_GAMEPAD_AXIS_LEFTX;
        event.gaxis.value = static_cast<s16>(s_state.leftStick.x * 32767.0f);
        injectSDLEvent(event);
    }
    
    // Left stick Y
    if (std::abs(s_state.leftStick.y) > JOYSTICK_DEADZONE) {
        event.gaxis.axis = SDL_GAMEPAD_AXIS_LEFTY;
        event.gaxis.value = static_cast<s16>(s_state.leftStick.y * 32767.0f);
        injectSDLEvent(event);
    }
    
    // Right stick X
    if (std::abs(s_state.rightStick.x) > JOYSTICK_DEADZONE) {
        event.gaxis.axis = SDL_GAMEPAD_AXIS_RIGHTX;
        event.gaxis.value = static_cast<s16>(s_state.rightStick.x * 32767.0f);
        injectSDLEvent(event);
    }
    
    // Right stick Y
    if (std::abs(s_state.rightStick.y) > JOYSTICK_DEADZONE) {
        event.gaxis.axis = SDL_GAMEPAD_AXIS_RIGHTY;
        event.gaxis.value = static_cast<s16>(s_state.rightStick.y * 32767.0f);
        injectSDLEvent(event);
    }
}

void renderButtonImGui(const VirtualButtonState& button, const ImVec4& color) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    ImVec2 p1(button.rect.x, button.rect.y);
    ImVec2 p2(button.rect.x + button.rect.w, button.rect.y + button.rect.h);
    
    ImVec4 bgColor = button.pressed ? 
        ImVec4(color.x, color.y, color.z, 0.8f) : 
        ImVec4(color.x, color.y, color.z, 0.4f);
    
    drawList->AddRectFilled(p1, p2, ImGui::ColorConvertFloat4ToU32(bgColor), 8.0f);
    drawList->AddRect(p1, p2, ImGui::ColorConvertFloat4ToU32(color), 8.0f, 0, 2.0f);
}

void renderJoystickImGui(const VirtualJoystick& joystick, const ImVec4& color) {
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    ImVec2 baseP1(joystick.baseRect.x, joystick.baseRect.y);
    ImVec2 baseP2(joystick.baseRect.x + joystick.baseRect.w, joystick.baseRect.y + joystick.baseRect.h);
    
    ImVec4 baseColor = ImVec4(color.x, color.y, color.z, 0.4f);
    drawList->AddRectFilled(baseP1, baseP2, ImGui::ColorConvertFloat4ToU32(baseColor), 16.0f);
    drawList->AddRect(baseP1, baseP2, ImGui::ColorConvertFloat4ToU32(color), 16.0f, 0, 2.0f);
    
    ImVec2 stickP1(joystick.stickRect.x, joystick.stickRect.y);
    ImVec2 stickP2(joystick.stickRect.x + joystick.stickRect.w, joystick.stickRect.y + joystick.stickRect.h);
    
    ImVec4 stickColor = joystick.touched ? 
        ImVec4(color.x, color.y, color.z, 0.8f) : 
        ImVec4(color.x, color.y, color.z, 0.6f);
    
    drawList->AddRectFilled(stickP1, stickP2, ImGui::ColorConvertFloat4ToU32(stickColor), 8.0f);
    drawList->AddRect(stickP1, stickP2, ImGui::ColorConvertFloat4ToU32(color), 8.0f, 0, 2.0f);
}

void renderImGui(int screenWidth, int screenHeight) {
    if (!s_state.enabled) {
        return;
    }
    
    // Get current layout preset from settings
    int presetInt = dusk::getSettings().game.virtualControlsLayoutPreset.getValue();
    LayoutPreset preset = static_cast<LayoutPreset>(presetInt);
    
    // Apply the appropriate layout
    switch (preset) {
        case LayoutPreset::Dolphin:
            applyDolphinLayout(screenWidth, screenHeight);
            break;
        case LayoutPreset::Xbox:
            applyXboxLayout(screenWidth, screenHeight);
            break;
        case LayoutPreset::PlayStation:
            applyPlayStationLayout(screenWidth, screenHeight);
            break;
        case LayoutPreset::Mobile:
            applyMobileLayout(screenWidth, screenHeight);
            break;
        case LayoutPreset::Custom:
            applyCustomLayout(screenWidth, screenHeight);
            break;
        default:
            applyDolphinLayout(screenWidth, screenHeight);
            break;
    }
    
    // Get opacity from settings
    float opacity = dusk::getSettings().game.virtualControlsOpacity.getValue();
    
    // Render buttons with opacity
    ImVec4 buttonColor = ImVec4(1.0f, 1.0f, 1.0f, opacity);
    
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::DPadUp)], buttonColor);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::DPadDown)], buttonColor);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::DPadLeft)], buttonColor);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::DPadRight)], buttonColor);
    
    ImVec4 aColor = ImVec4(1.0f, 0.4f, 0.4f, opacity);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::A)], aColor);
    
    ImVec4 bColor = ImVec4(0.4f, 1.0f, 0.4f, opacity);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::B)], bColor);
    
    ImVec4 xColor = ImVec4(0.4f, 0.4f, 1.0f, opacity);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::X)], xColor);
    
    ImVec4 yColor = ImVec4(1.0f, 1.0f, 0.4f, opacity);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::Y)], yColor);
    
    ImVec4 shoulderColor = ImVec4(0.8f, 0.8f, 0.8f, opacity);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::L)], shoulderColor);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::R)], shoulderColor);
    
    ImVec4 startColor = ImVec4(0.6f, 0.6f, 0.6f, opacity);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::Start)], startColor);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::Z)], startColor);
    
    // Back button (Menu) - typically in Mobile layout
    ImVec4 backColor = ImVec4(0.5f, 0.5f, 0.5f, opacity);
    renderButtonImGui(s_state.buttons[static_cast<size_t>(VirtualButton::Back)], backColor);
    
    // Render joysticks with opacity
    ImVec4 joystickColor = ImVec4(0.8f, 0.8f, 0.8f, opacity);
    renderJoystickImGui(s_state.leftStick, joystickColor);
    renderJoystickImGui(s_state.rightStick, joystickColor);
    
    // Show labels if enabled
    if (dusk::getSettings().game.virtualControlsShowLabels.getValue()) {
        // Button labels would be rendered here (simplified for now)
    }
}

bool isPointInRect(float x, float y, const SDL_FRect& rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

bool handleTouchEvent(const SDL_TouchFingerEvent& event) {
    if (!s_state.enabled) {
        return false;
    }
    
    float x = event.x * 1280.0f; // Assuming 1280 width, will be updated in render
    float y = event.y * 720.0f;  // Assuming 720 height, will be updated in render
    
    if (event.type == SDL_EVENT_FINGER_DOWN) {
        // Check buttons
        for (size_t i = 0; i < static_cast<size_t>(VirtualButton::Count); ++i) {
            if (isPointInRect(x, y, s_state.buttons[i].rect)) {
                s_state.buttons[i].pressed = true;
                return true;
            }
        }
        
        // Check joysticks
        if (isPointInRect(x, y, s_state.leftStick.baseRect)) {
            s_state.leftStick.touched = true;
            s_state.leftStick.touchId = event.fingerID;
            return true;
        }
        
        if (isPointInRect(x, y, s_state.rightStick.baseRect)) {
            s_state.rightStick.touched = true;
            s_state.rightStick.touchId = event.fingerID;
            return true;
        }
    }
    else if (event.type == SDL_EVENT_FINGER_UP) {
        // Release buttons
        for (size_t i = 0; i < static_cast<size_t>(VirtualButton::Count); ++i) {
            if (s_state.buttons[i].pressed) {
                s_state.buttons[i].pressed = false;
                s_state.buttons[i].wasPressed = false;
            }
        }
        
        // Release joysticks
        if (s_state.leftStick.touched && s_state.leftStick.touchId == event.fingerID) {
            s_state.leftStick.touched = false;
            s_state.leftStick.touchId = -1;
            s_state.leftStick.x = 0.0f;
            s_state.leftStick.y = 0.0f;
            
            // Reset stick position
            s_state.leftStick.stickRect.x = s_state.leftStick.baseRect.x + JOYSTICK_SIZE/2 - BUTTON_SIZE/2;
            s_state.leftStick.stickRect.y = s_state.leftStick.baseRect.y + JOYSTICK_SIZE/2 - BUTTON_SIZE/2;
        }
        
        if (s_state.rightStick.touched && s_state.rightStick.touchId == event.fingerID) {
            s_state.rightStick.touched = false;
            s_state.rightStick.touchId = -1;
            s_state.rightStick.x = 0.0f;
            s_state.rightStick.y = 0.0f;
            
            // Reset stick position
            s_state.rightStick.stickRect.x = s_state.rightStick.baseRect.x + JOYSTICK_SIZE/2 - BUTTON_SIZE/2;
            s_state.rightStick.stickRect.y = s_state.rightStick.baseRect.y + JOYSTICK_SIZE/2 - BUTTON_SIZE/2;
        }
    }
    else if (event.type == SDL_EVENT_FINGER_MOTION) {
        // Update joystick positions
        if (s_state.leftStick.touched && s_state.leftStick.touchId == event.fingerID) {
            float centerX = s_state.leftStick.baseRect.x + JOYSTICK_SIZE / 2;
            float centerY = s_state.leftStick.baseRect.y + JOYSTICK_SIZE / 2;
            
            float deltaX = x - centerX;
            float deltaY = y - centerY;
            
            // Clamp to joystick radius
            float maxDist = JOYSTICK_SIZE / 2 - BUTTON_SIZE / 2;
            float dist = std::sqrt(deltaX * deltaX + deltaY * deltaY);
            
            if (dist > maxDist) {
                deltaX = deltaX / dist * maxDist;
                deltaY = deltaY / dist * maxDist;
            }
            
            s_state.leftStick.x = deltaX / maxDist;
            s_state.leftStick.y = deltaY / maxDist;
            
            s_state.leftStick.stickRect.x = centerX + deltaX - BUTTON_SIZE / 2;
            s_state.leftStick.stickRect.y = centerY + deltaY - BUTTON_SIZE / 2;
            
            return true;
        }
        
        if (s_state.rightStick.touched && s_state.rightStick.touchId == event.fingerID) {
            float centerX = s_state.rightStick.baseRect.x + JOYSTICK_SIZE / 2;
            float centerY = s_state.rightStick.baseRect.y + JOYSTICK_SIZE / 2;
            
            float deltaX = x - centerX;
            float deltaY = y - centerY;
            
            // Clamp to joystick radius
            float maxDist = JOYSTICK_SIZE / 2 - BUTTON_SIZE / 2;
            float dist = std::sqrt(deltaX * deltaX + deltaY * deltaY);
            
            if (dist > maxDist) {
                deltaX = deltaX / dist * maxDist;
                deltaY = deltaY / dist * maxDist;
            }
            
            s_state.rightStick.x = deltaX / maxDist;
            s_state.rightStick.y = deltaY / maxDist;
            
            s_state.rightStick.stickRect.x = centerX + deltaX - BUTTON_SIZE / 2;
            s_state.rightStick.stickRect.y = centerY + deltaY - BUTTON_SIZE / 2;
            
            return true;
        }
    }
    
    return false;
}

VirtualControlsState& getState() {
    return s_state;
}

// Layout preset names
const char* getLayoutPresetName(LayoutPreset preset) {
    switch (preset) {
        case LayoutPreset::Dolphin: return "Dolphin (GameCube)";
        case LayoutPreset::Xbox: return "Xbox";
        case LayoutPreset::PlayStation: return "PlayStation";
        case LayoutPreset::Mobile: return "Mobile";
        case LayoutPreset::Custom: return "Custom";
        default: return "Unknown";
    }
}

// Apply layout preset
void setLayoutPreset(LayoutPreset preset) {
    s_state.layout.preset = preset;
    s_logModule.info("Setting virtual controls layout to: {}", getLayoutPresetName(preset));
    
    // Save to settings
    dusk::getSettings().game.virtualControlsLayoutPreset.setValue(static_cast<int>(preset));
}

// Layout editing mode
void enterEditMode() {
    s_state.editMode = true;
    s_logModule.info("Entered virtual controls edit mode");
}

void exitEditMode() {
    s_state.editMode = false;
    s_logModule.info("Exited virtual controls edit mode");
}

bool isInEditMode() {
    return s_state.editMode;
}

// Individual button/joystick editing
void setButtonPosition(VirtualButton button, float x, float y) {
    size_t idx = static_cast<size_t>(button);
    if (idx < static_cast<size_t>(VirtualButton::Count)) {
        s_state.layout.buttons[idx].x = x;
        s_state.layout.buttons[idx].y = y;
    }
}

void setButtonSize(VirtualButton button, float width, float height) {
    size_t idx = static_cast<size_t>(button);
    if (idx < static_cast<size_t>(VirtualButton::Count)) {
        s_state.layout.buttons[idx].width = width;
        s_state.layout.buttons[idx].height = height;
    }
}

void setJoystickPosition(bool left, float x, float y) {
    if (left) {
        s_state.layout.leftStick.x = x;
        s_state.layout.leftStick.y = y;
    } else {
        s_state.layout.rightStick.x = x;
        s_state.layout.rightStick.y = y;
    }
}

void setJoystickSize(bool left, float size) {
    if (left) {
        s_state.layout.leftStick.size = size;
    } else {
        s_state.layout.rightStick.size = size;
    }
}

void setButtonVisible(VirtualButton button, bool visible) {
    size_t idx = static_cast<size_t>(button);
    if (idx < static_cast<size_t>(VirtualButton::Count)) {
        s_state.layout.buttons[idx].visible = visible;
    }
}

void setJoystickVisible(bool left, bool visible) {
    if (left) {
        s_state.layout.leftStick.visible = visible;
    } else {
        s_state.layout.rightStick.visible = visible;
    }
}

// Xbox layout
void applyXboxLayout(int screenWidth, int screenHeight) {
    const float margin = 20.0f;
    const float bottomMargin = 40.0f;
    const float buttonSize = BUTTON_SIZE;
    const float spacing = BUTTON_SPACING;
    
    // Left stick (left side, lower)
    float leftStickX = margin + 20.0f;
    float leftStickY = screenHeight - 200.0f - bottomMargin;
    s_state.leftStick.baseRect = {leftStickX, leftStickY, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.leftStick.stickRect = {leftStickX + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                    leftStickY + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                    BUTTON_SIZE, BUTTON_SIZE};
    
    // D-Pad (left side, above left stick)
    float dpadX = leftStickX + (JOYSTICK_SIZE - buttonSize * 3 - spacing * 2) / 2;
    float dpadY = leftStickY - buttonSize * 3 - spacing * 2;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadUp)].rect = 
        {dpadX + buttonSize + spacing, dpadY, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadDown)].rect = 
        {dpadX + buttonSize + spacing, dpadY + buttonSize * 2 + spacing, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadLeft)].rect = 
        {dpadX, dpadY + buttonSize + spacing, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadRight)].rect = 
        {dpadX + buttonSize * 2 + spacing * 2, dpadY + buttonSize + spacing, buttonSize, buttonSize};
    
    // Right stick (right side, lower)
    float rightStickX = screenWidth - JOYSTICK_SIZE - margin - 20.0f;
    float rightStickY = screenHeight - 200.0f - bottomMargin;
    s_state.rightStick.baseRect = {rightStickX, rightStickY, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.rightStick.stickRect = {rightStickX + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                     rightStickY + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                     BUTTON_SIZE, BUTTON_SIZE};
    
    // ABXY buttons (right side, above right stick) - Xbox diamond layout
    float abX = rightStickX + JOYSTICK_SIZE/2 - buttonSize * 1.5f;
    float abY = rightStickY - buttonSize * 3.5f - spacing * 2;
    float offset = buttonSize + spacing;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::Y)].rect = {abX + offset, abY, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::X)].rect = {abX, abY + offset, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::A)].rect = {abX + offset, abY + offset * 2, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::B)].rect = {abX + offset * 2, abY + offset, buttonSize, buttonSize};
    
    // Shoulder buttons (top)
    s_state.buttons[static_cast<size_t>(VirtualButton::L)].rect = 
        {margin, 80.0f, buttonSize * 3, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::R)].rect = 
        {screenWidth - buttonSize * 3 - margin, 80.0f, buttonSize * 3, buttonSize};
    
    // Start and Z (center)
    s_state.buttons[static_cast<size_t>(VirtualButton::Start)].rect = 
        {screenWidth / 2.0f - buttonSize / 2.0f, screenHeight - buttonSize - bottomMargin, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::Z)].rect = 
        {screenWidth / 2.0f + buttonSize, screenHeight - buttonSize - bottomMargin, buttonSize, buttonSize};
}

// PlayStation layout
void applyPlayStationLayout(int screenWidth, int screenHeight) {
    const float margin = 20.0f;
    const float bottomMargin = 40.0f;
    const float buttonSize = BUTTON_SIZE;
    const float spacing = BUTTON_SPACING;
    
    // Left stick (left side, lower)
    float leftStickX = margin + 20.0f;
    float leftStickY = screenHeight - 200.0f - bottomMargin;
    s_state.leftStick.baseRect = {leftStickX, leftStickY, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.leftStick.stickRect = {leftStickX + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                    leftStickY + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                    BUTTON_SIZE, BUTTON_SIZE};
    
    // D-Pad (left side, above left stick)
    float dpadX = leftStickX + (JOYSTICK_SIZE - buttonSize * 3 - spacing * 2) / 2;
    float dpadY = leftStickY - buttonSize * 3 - spacing * 2;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadUp)].rect = 
        {dpadX + buttonSize + spacing, dpadY, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadDown)].rect = 
        {dpadX + buttonSize + spacing, dpadY + buttonSize * 2 + spacing, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadLeft)].rect = 
        {dpadX, dpadY + buttonSize + spacing, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadRight)].rect = 
        {dpadX + buttonSize * 2 + spacing * 2, dpadY + buttonSize + spacing, buttonSize, buttonSize};
    
    // Right stick (right side, lower)
    float rightStickX = screenWidth - JOYSTICK_SIZE - margin - 20.0f;
    float rightStickY = screenHeight - 200.0f - bottomMargin;
    s_state.rightStick.baseRect = {rightStickX, rightStickY, JOYSTICK_SIZE, JOYSTICK_SIZE};
    s_state.rightStick.stickRect = {rightStickX + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                     rightStickY + JOYSTICK_SIZE/2 - BUTTON_SIZE/2, 
                                     BUTTON_SIZE, BUTTON_SIZE};
    
    // PlayStation shape buttons (△ ○ × □)
    float shapeX = rightStickX + JOYSTICK_SIZE/2 - buttonSize * 1.5f;
    float shapeY = rightStickY - buttonSize * 3.5f - spacing * 2;
    float offset = buttonSize + spacing;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::Y)].rect = {shapeX + offset, shapeY, buttonSize, buttonSize};  // Triangle
    s_state.buttons[static_cast<size_t>(VirtualButton::X)].rect = {shapeX, shapeY + offset, buttonSize, buttonSize};  // Square
    s_state.buttons[static_cast<size_t>(VirtualButton::A)].rect = {shapeX + offset, shapeY + offset * 2, buttonSize, buttonSize};  // Circle
    s_state.buttons[static_cast<size_t>(VirtualButton::B)].rect = {shapeX + offset * 2, shapeY + offset, buttonSize, buttonSize};  // X
    
    // Shoulder buttons (top)
    s_state.buttons[static_cast<size_t>(VirtualButton::L)].rect = 
        {margin, 80.0f, buttonSize * 3, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::R)].rect = 
        {screenWidth - buttonSize * 3 - margin, 80.0f, buttonSize * 3, buttonSize};
    
    // Start and Z (center)
    s_state.buttons[static_cast<size_t>(VirtualButton::Start)].rect = 
        {screenWidth / 2.0f - buttonSize / 2.0f, screenHeight - buttonSize - bottomMargin, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::Z)].rect = 
        {screenWidth / 2.0f + buttonSize, screenHeight - buttonSize - bottomMargin, buttonSize, buttonSize};
}

// Mobile-optimized layout
void applyMobileLayout(int screenWidth, int screenHeight) {
    const float margin = 20.0f;
    const float bottomMargin = 50.0f;
    const float buttonSize = BUTTON_SIZE * 1.1f;  // Larger buttons for touch
    const float spacing = BUTTON_SPACING * 1.5f;
    
    // D-Pad (left side, bottom)
    float dpadX = margin;
    float dpadY = screenHeight - 250.0f - bottomMargin;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadUp)].rect = 
        {dpadX + buttonSize, dpadY, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadDown)].rect = 
        {dpadX + buttonSize, dpadY + buttonSize * 2 + spacing, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadLeft)].rect = 
        {dpadX, dpadY + buttonSize + spacing, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::DPadRight)].rect = 
        {dpadX + buttonSize * 2 + spacing, dpadY + buttonSize + spacing, buttonSize, buttonSize};
    
    // Left joystick (left side, above D-Pad)
    float leftStickX = margin;
    float leftStickY = screenHeight - 420.0f - bottomMargin;
    s_state.leftStick.baseRect = {leftStickX, leftStickY, JOYSTICK_SIZE * 1.1f, JOYSTICK_SIZE * 1.1f};
    s_state.leftStick.stickRect = {leftStickX + JOYSTICK_SIZE*1.1f/2 - buttonSize/2, 
                                    leftStickY + JOYSTICK_SIZE*1.1f/2 - buttonSize/2, 
                                    buttonSize, buttonSize};
    
    // ABXY buttons (right side, bottom) - larger for touch
    float abX = screenWidth - buttonSize * 3 - margin - spacing * 2;
    float abY = screenHeight - 250.0f - bottomMargin;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::A)].rect = 
        {abX + buttonSize + spacing, abY + buttonSize + spacing, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::B)].rect = 
        {abX + buttonSize * 2 + spacing * 2, abY + buttonSize, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::X)].rect = 
        {abX, abY + buttonSize, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::Y)].rect = 
        {abX + buttonSize + spacing, abY, buttonSize, buttonSize};
    
    // Right joystick (right side, above buttons)
    float rightStickX = screenWidth - JOYSTICK_SIZE * 1.1f - margin;
    float rightStickY = screenHeight - 420.0f - bottomMargin;
    s_state.rightStick.baseRect = {rightStickX, rightStickY, JOYSTICK_SIZE * 1.1f, JOYSTICK_SIZE * 1.1f};
    s_state.rightStick.stickRect = {rightStickX + JOYSTICK_SIZE*1.1f/2 - buttonSize/2, 
                                     rightStickY + JOYSTICK_SIZE*1.1f/2 - buttonSize/2, 
                                     buttonSize, buttonSize};
    
    // Shoulder buttons (top)
    s_state.buttons[static_cast<size_t>(VirtualButton::L)].rect = 
        {margin, 80.0f, buttonSize * 2.5f, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::R)].rect = 
        {screenWidth - buttonSize * 2.5f - margin, 80.0f, buttonSize * 2.5f, buttonSize};
    
    // Start, Z, and Back (center)
    float centerX = screenWidth / 2.0f;
    float bottomY = screenHeight - buttonSize - bottomMargin;
    
    s_state.buttons[static_cast<size_t>(VirtualButton::Start)].rect = 
        {centerX - buttonSize * 1.5f - spacing, bottomY, buttonSize, buttonSize};
    s_state.buttons[static_cast<size_t>(VirtualButton::Z)].rect = 
        {centerX - buttonSize / 2.0f, bottomY - buttonSize - spacing, buttonSize, buttonSize};
    // Back button - opens menu (Android style)
    s_state.buttons[static_cast<size_t>(VirtualButton::Back)].rect = 
        {centerX + buttonSize * 0.5f + spacing, bottomY, buttonSize, buttonSize};
}

// Custom layout (user-defined)
void applyCustomLayout(int screenWidth, int screenHeight) {
    // Apply positions and sizes from the custom layout configuration
    for (size_t i = 0; i < static_cast<size_t>(VirtualButton::Count); ++i) {
        const auto& config = s_state.layout.buttons[i];
        if (config.visible) {
            s_state.buttons[i].rect = {config.x, config.y, config.width, config.height};
        } else {
            // Hide off-screen
            s_state.buttons[i].rect = {-1000, -1000, 0, 0};
        }
    }
    
    // Apply joystick configurations
    if (s_state.layout.leftStick.visible) {
        float size = s_state.layout.leftStick.size;
        s_state.leftStick.baseRect = {s_state.layout.leftStick.x, s_state.layout.leftStick.y, size, size};
        s_state.leftStick.stickRect = {s_state.layout.leftStick.x + size/2 - BUTTON_SIZE/2, 
                                        s_state.layout.leftStick.y + size/2 - BUTTON_SIZE/2, 
                                        BUTTON_SIZE, BUTTON_SIZE};
    } else {
        s_state.leftStick.baseRect = {-1000, -1000, 0, 0};
    }
    
    if (s_state.layout.rightStick.visible) {
        float size = s_state.layout.rightStick.size;
        s_state.rightStick.baseRect = {s_state.layout.rightStick.x, s_state.layout.rightStick.y, size, size};
        s_state.rightStick.stickRect = {s_state.layout.rightStick.x + size/2 - BUTTON_SIZE/2, 
                                         s_state.layout.rightStick.y + size/2 - BUTTON_SIZE/2, 
                                         BUTTON_SIZE, BUTTON_SIZE};
    } else {
        s_state.rightStick.baseRect = {-1000, -1000, 0, 0};
    }
}

// Reset to default layout
void resetToDefaultLayout() {
    s_logModule.info("Resetting virtual controls to default layout");
    s_state.layout = LayoutConfig{};  // Reset to defaults
    setLayoutPreset(LayoutPreset::Dolphin);
}

// Save/load custom layout (simplified - just store in settings)
std::string saveCustomLayout() {
    // For now, just mark as custom - full JSON serialization can be added later
    s_state.layout.preset = LayoutPreset::Custom;
    return "custom";
}

void loadCustomLayout(const std::string& configJson) {
    if (configJson == "custom") {
        s_state.layout.preset = LayoutPreset::Custom;
    }
}

} // namespace dusk::virtual_controls
