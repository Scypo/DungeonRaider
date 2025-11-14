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
    ecs.SwitchScenes("Level", false);
    sl::Scene& scene = *ecs.GetScene("Level");

    gfx.LoadTexture("assets/images/tile_set8.png");

    CreateLevel(scene);
    CreatePlayer(scene, { 0.0f,0.0f }, 40.0f, 40.0f, gfx.LoadTexture("assets/images/lessShittyCharacter.png"));
    CreateCamera(scene);

    scene.RegisterSystem<InputReadSystem>();
    scene.RegisterSystem<ShieldSystem>();
    scene.RegisterSystem<WeaponSystem>();
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

void CreateBuyScreen()
{
    auto& ecs = se::Engine::GetECS();
    sl::Graphics& gfx = se::Engine::GetGraphics();
    ecs.CreateScene("BuyScreen");
    ecs.SwitchScenes("BuyScreen", false);
    sl::Scene& scene = *ecs.GetScene("BuyScreen");

    sl::Texture* texture = gfx.LoadTexture("assets/images/powerupButtons.png");
    sl::RectF uv(0.0f, 10.0f, 10.0f, 0.0f);
    std::vector<sl::RectF> uvs = {
        sl::RectF(0.0f, 39, 0.0f, 64.0f),
        sl::RectF(40.0f, 79, 0.0f, 64.0f),
        sl::RectF(80.0f, 119, 0.0f, 64.0f) };
        
    CreateButton(scene, sl::Vec2f(40.0f, 50.0f), sl::Vec2f(160.0f, 260.0f), texture, uvs[0], [&]() 
        {
            if (GameGlobals::spendingPoints > 0)
            {
                GameGlobals::spendingPoints--;
                ShieldComponent& shield = ecs.GetScene("Level")->GetComponent<ShieldComponent>(GameGlobals::player);
                shield.maxShield *= 1.05f;
                shield.regenRate *= 0.8f;
            }
        });

    CreateButton(scene, sl::Vec2f(240.0f, 50.0f), sl::Vec2f(160.0f, 260.0f), texture, uvs[1], [&]() 
        {
            if (GameGlobals::spendingPoints > 0)
            {
                GameGlobals::spendingPoints--;
                HealthComponent& health = ecs.GetScene("Level")->GetComponent<HealthComponent>(GameGlobals::player);
                health.maxHealth = 1.1f * health.maxHealth;
            }
        });

    CreateButton(scene, sl::Vec2f(440.0f, 50.0f), sl::Vec2f(160.0f, 260.0f), texture, uvs[2], [&]() 
        {
            if (GameGlobals::spendingPoints > 0)
            {
                //GameGlobals::spendingPoints--;
                //HealthComponent& health = ecs.GetScene("Level")->GetComponent<HealthComponent>(GameGlobals::player);
                //health.maxHealth = 1.05f * health.maxHealth;
            }
        
        });

    scene.RegisterSystem<ButtonSystem>();
    scene.RegisterSystem<ExecuteEventSystem>();
    scene.RegisterSystem<UIRenderSystem>();
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
