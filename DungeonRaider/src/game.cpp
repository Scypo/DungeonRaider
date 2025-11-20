#include "game.h"
#include<ScypLib/Logger.h>
#include"Level.h"
#include"Entities.h"
#include"Systems.h"
#include"Pathfinder.h"
#include"Behavior.h"
#include"FloatingText.h"
#include"UserInterface.h"

void CreateSimulation()
{
    auto& ecs = se::Engine::GetECS();
    sl::Graphics& gfx = se::Engine::GetGraphics();
    ecs.CreateScene("Level");
    sl::Scene& scene = *ecs.GetScene("Level");

    gfx.LoadTexture("assets/images/tile_set8.png");
    gfx.SetDefaultFont(gfx.LoadFont("assets/fonts/Splatfont.ttf", ' ', '~'));

    CreateLevel(scene);
    CreatePlayer(scene, { 0.0f,0.0f }, 40.0f, 40.0f, gfx.LoadTexture("assets/images/lessShittyCharacter.png"));
    CreateCamera(scene);

    scene.RegisterSystem<PlayerSystem>();
    scene.RegisterSystem<ShieldSystem>();
    scene.RegisterSystem<WeaponSystem>();
    scene.RegisterSystem<DeathSystem>();
    scene.RegisterSystem<EnemyBehaviorSystem>();
    scene.RegisterSystem<PathfindingSystem>();
    scene.RegisterSystem<MovementSystem>();
    scene.RegisterSystem<TileCollisionSystem>();
    scene.RegisterSystem<ProjectileCollisionSystem>();
    scene.RegisterSystem<FloatingTextSystem>();
    scene.RegisterSystem<LevelSystem>();
    scene.RegisterSystem<CameraSystem>();
    scene.RegisterSystem<ExecuteEventSystem>();
    scene.RegisterSystem<RenderSystem>();
}

void DungeonRaider::OnBegin()
{
    sl::Logger::GetInstance().SetMinimumLogLevel(sl::LogLevel::Info);
    sl::Graphics& gfx = se::Engine::GetGraphics();
    gfx.SetVSyncInterval(1);
    gfx.SetCanvasSize(sl::Vec2f(640,360));
    
    CreateSimulation();
    se::Engine::GetECS().SwitchScenes("Level", false);
}
