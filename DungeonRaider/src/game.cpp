#include "game.h"
#include"Level.h"
#include"GameObjects.h"
#include"Systems.h"
#include"Events.h"
#include"Pathfinder.h"
#include"Behavior.h"
#include"FloatingText.h"

void DungeonRaider::OnBegin()
{
    sl::Logger::GetInstance().SetMinimumLogLevel(sl::LogLevel::Info);
    sl::Graphics& gfx = se::Engine::GetGraphics();
    gfx.SetVSyncInterval(1);
    gfx.SetCanvasSize(sl::Vec2f(640,360));
    auto& ecs = se::Engine::GetECS();
    ecs.CreateScene("level");
    ecs.SetCurrentScene("level");

    sl::Scene& scene = *ecs.GetCurrentScene();
    assert(&scene);

    sl::Texture* atlas = gfx.LoadTexture("assets/images/tile_set8.png");

    CreateLevel();
    CreatePlayer({ 0.0f,0.0f }, 40.0f, 40.0f, gfx.LoadTexture("assets/images/lessShittyCharacter.png"));
    CreateCamera();

    scene.RegisterSystem<InputReadSystem>();
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

    scene.GetEventBus().Subscribe<DealDamageEvent>([&scene](DealDamageEvent& e)
        {
            assert(scene.HasComponent<HealthComponent>(e.target));
            float& health = scene.GetComponent<HealthComponent>(e.target).health;
            health -= e.damage;
            if (health <= 0.0f)
            {
                #ifndef NDEBUG
                if(e.target == GameGlobals::player) return;
                #endif
                scene.DestroyEntity(e.target);
            }
        });
}
