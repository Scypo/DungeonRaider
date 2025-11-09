#include "Weapon.h"
#include"Level.h"
#include"Components.h"
#include"Events.h"

bool WeaponAttack(sl::EntityId id, sl::Vec2f target, TagComponent immune)
{
    sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
    WeaponComponent& weapon = scene.GetComponent<WeaponComponent>(id);
    if (weapon.remainingTime > 0) return false;
    ProjectileData& data = weapon.projectileData;
    weapon.remainingTime = weapon.cooldown;
    sl::RectF entityRect = GetWorldCollider(id);
    sl::Vec2f weaponOrigin = sl::Vec2f(entityRect.GetCenter());


    sl::Vec2f dir = (target - weaponOrigin).GetNormalized();
    sl::Vec2f bulletSpawn = weaponOrigin - sl::Vec2f(data.width / 2, data.height / 2);

    CreateProjectile(bulletSpawn, dir, &data, immune);
    return true;
}

void ProjectileCollisionSystem::Run(float dt, sl::Scene& scene)
{
    scene.ForEach<Projectile, TransformComponent, ColliderComponent, MovementComponent, TagComponent>([&](sl::EntityId id, Projectile& projectile, TransformComponent& transform, ColliderComponent& collider, MovementComponent& movement, TagComponent& tag)
        {
            sl::RectF projectileWorldRect = GetWorldCollider(id);

            scene.ForEach<TilesetChunk>([&](sl::EntityId tilesetId, TilesetChunk& chunk)
                {

                    if (!chunk.worldRect.IsOverlappingWith(sl::RectI(projectileWorldRect))) return;

                    auto blocks([&](int x, int y) -> bool
                        {
                            return chunk.collisionGrid[size_t(y * chunk.width + x)];
                        });


                    sl::Vec2i gridTL = chunk.WorldToGrid(projectileWorldRect.left, projectileWorldRect.top);
                    sl::Vec2i gridBR = chunk.WorldToGrid(projectileWorldRect.right, projectileWorldRect.bottom);

                    if (gridTL.x < 0 || gridTL.x >= chunk.width || gridTL.y < 0 || gridTL.y >= chunk.height) return;
                    if (gridBR.x < 0 || gridBR.x >= chunk.width || gridBR.y < 0 || gridBR.y >= chunk.height) return;

                    for (int y = gridTL.y; y <= gridBR.y; y++)
                    {
                        for (int x = gridTL.x; x <= gridBR.x; x++)
                        {
                            if (!blocks(x, y)) continue;

                            sl::RectF tileRect;
                            sl::Vec2f tilePos = chunk.GridToWorld(x, y);
                            tileRect.left = tilePos.x;
                            tileRect.top = tilePos.y;
                            tileRect.right = tilePos.x + chunk.tileSize;
                            tileRect.bottom = tilePos.y + chunk.tileSize;

                            if (!projectileWorldRect.IsOverlappingWith(tileRect)) continue;

                            scene.DestroyEntity(id);

                            return;
                        }
                    }
                });
            scene.ForEach<ColliderComponent, HealthComponent>([&](sl::EventId target, ColliderComponent& targetCollider, HealthComponent& targetHealth)
                {
                    if (scene.HasComponent<TagComponent>(target))
                    {
                        if (scene.GetComponent<TagComponent>(target).tag & tag.tag) return;
                    }
                    sl::RectF targetWorldRect = GetWorldCollider(target);
                    if (targetWorldRect.IsOverlappingWith(projectileWorldRect))
                    {
                        if (target == GameGlobals::player) return;//TEMPORARY SO PLAYER DOESNT DIE
                        scene.GetEventBus().Emit<DealDamageEvent>(DealDamageEvent{ target, projectile.damage });
                        scene.DestroyEntity(id);
                    }
                });
        });
}

void WeaponSystem::Run(float dt, sl::Scene& scene)
{
    scene.ForEach<WeaponComponent>([&](sl::EntityId id, WeaponComponent& weapon)
        {
            weapon.remainingTime -= dt;
        });
}

sl::EntityId CreateProjectile(sl::Vec2f pos, sl::Vec2f dir, const ProjectileData* data, TagComponent immune)
{
    sl::EntityComponentSystem& ecs = se::Engine::GetECS();
    sl::Scene* scene = ecs.GetCurrentScene();

    sl::EntityId projectile;
    projectile = scene->CreateEntity();
    scene->AddComponent<Projectile>(projectile, Projectile{ data->damage });
    scene->AddComponent<TagComponent>(projectile, std::move(immune));
    scene->AddComponent<TransformComponent>(projectile, TransformComponent{ pos, 0.0f });
    scene->AddComponent<MovementComponent>(projectile, MovementComponent{ dir, {}, data->speed });
    scene->AddComponent<ColliderComponent>(projectile, ColliderComponent{ sl::RectF(0, data->width, 0, data->height), ColliderComponent::CollisionLayer::GameObjects });
    scene->AddComponent<SpriteComponent>(projectile, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(data->width / 2, data->height / 2), data->texture,
        sl::RectF(0.0f, data->width, 0.0f, data->height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::White, sl::Vec2f(data->width, data->height), nullptr));

    return projectile;
}