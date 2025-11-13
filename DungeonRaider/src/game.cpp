#include "game.h"
#include"Level.h"
#include"GameObjects.h"
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
    ecs.SwitchScenes("Level", false);
    sl::Scene* scene = ecs.GetScene("Level");

    assert(scene);

    gfx.LoadTexture("assets/images/tile_set8.png");

    CreateLevel(scene);
    CreatePlayer(scene, { 0.0f,0.0f }, 40.0f, 40.0f, gfx.LoadTexture("assets/images/lessShittyCharacter.png"));
    CreateCamera(scene);

    scene->RegisterSystem<InputReadSystem>();
    scene->RegisterSystem<WeaponSystem>();
    scene->RegisterSystem<EnemyBehaviorSystem>();
    scene->RegisterSystem<PathfindingSystem>();
    scene->RegisterSystem<MovementSystem>();
    scene->RegisterSystem<TileCollisionSystem>();
    scene->RegisterSystem<ProjectileCollisionSystem>();
    scene->RegisterSystem<FloatingTextSystem>();
    scene->RegisterSystem<LevelSystem>();
    scene->RegisterSystem<CameraSystem>();
    scene->RegisterSystem<ExecuteEventSystem>();
    scene->RegisterSystem<RenderSystem>();
}

void CreateBuyScreen()
{
    auto& ecs = se::Engine::GetECS();
    sl::Graphics& gfx = se::Engine::GetGraphics();
    ecs.CreateScene("BuyScreen");
    ecs.SwitchScenes("BuyScreen", false);
    sl::Scene* scene = ecs.GetScene("BuyScreen");
    assert(&scene);

    sl::RectF uv(0.0f, 10.0f, 0.0f, 10.0f);
    CreateButton(scene, sl::Vec2f(0.0f, 0.0f), sl::Vec2f(100.0f, 100.0f), gfx.LoadTexture("assets/images/medkit.png"), uv, []() {
        std::cout << "nigger" << std::endl;
        });

    scene->RegisterSystem<ButtonSystem>();
    scene->RegisterSystem<ExecuteEventSystem>();
    scene->RegisterSystem<UIRenderSystem>();
}

void DungeonRaider::OnBegin()
{
    sl::Logger::GetInstance().SetMinimumLogLevel(sl::LogLevel::Info);
    sl::Graphics& gfx = se::Engine::GetGraphics();
    gfx.SetVSyncInterval(1);
    gfx.SetCanvasSize(sl::Vec2f(640,360));
    
    CreateBuyScreen();
    CreateSimulation();
    se::Engine::GetECS().SwitchScenes("Level", false);
}
