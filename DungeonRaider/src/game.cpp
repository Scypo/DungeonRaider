#include "game.h"
#include<ScypLib/Logger.h>
#include"Level.h"
#include"Entities.h"
#include"Systems.h"
#include"Pathfinder.h"
#include"Behavior.h"
#include"FloatingText.h"
#include"Scenes.h"


void DungeonRaider::OnBegin()
{
    sl::Logger::GetInstance().SetMinimumLogLevel(sl::LogLevel::Info);
    sl::Graphics& gfx = se::Engine::GetGraphics();
    gfx.SetVSyncInterval(1);
    gfx.SetCanvasSize(sl::Vec2f(640,360));
    
    gfx.SetDefaultFont(gfx.LoadFont("assets/fonts/Splatfont.ttf", ' ', '~'));
    CreateMainMenu();
    se::Engine::GetECS().SwitchScenes("MainMenu", false);
}

void DungeonRaider::OnUpdate(float dt)
{
    if (se::Engine::GetKeyboard().KeyIsPressed(sl::Key::F11))
    {
        se::Engine::GetWindow().ToggleFullscreen();
    }
}
