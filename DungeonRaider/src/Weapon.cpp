#include "Weapon.h"
#include"Level.h"
#include"Components.h"
#include"FloatingText.h"
#include"Entities.h"

bool WeaponAttack(sl::Scene& scene, sl::EntityId id, sl::Vec2f target, TagComponent immune)
{
    WeaponComponent& weapon = scene.GetComponent<WeaponComponent>(id);
    if (weapon.remainingTime > 0.0f || weapon.ammoLeft == 0 || weapon.reloadTimeLeft > 0.0f) return false;
    weapon.ammoLeft--;
    if (weapon.ammoLeft == 0)
    {
        weapon.reloadTimeLeft = weapon.reloadTime;
        weapon.remainingTime = 0.0f;
    }
    else
    {
        weapon.remainingTime = weapon.cooldown;
    }
    sl::RectF entityRect = GetWorldCollider(scene, id);
    sl::Vec2f weaponOrigin = sl::Vec2f(entityRect.GetCenter());

    sl::Vec2f dir = (target - weaponOrigin).GetNormalized();
    sl::Vec2f bulletSpawn = weaponOrigin - sl::Vec2f(weapon.projWidth / 2, weapon.projHeight / 2);

    CreateProjectile(scene, bulletSpawn, dir, id, immune);
    se::Engine::GetAudio().PlaySound(weapon.shotSound);
    return true;
}

void AttachWeapon(sl::Scene& scene, sl::EntityId id, WeaponType type)
{
    WeaponComponent weapon{};
    static sl::Sound* arShotSound = se::Engine::GetAudio().LoadSound("assets/sounds/shot.wav");
    static sl::Sound* smgShotSound = se::Engine::GetAudio().LoadSound("assets/sounds/shot.wav");
    static sl::Sound* dmrShotSound = se::Engine::GetAudio().LoadSound("assets/sounds/shot.wav");
    static sl::Sound* sniperShotSound = se::Engine::GetAudio().LoadSound("assets/sounds/shot.wav");

    switch (type)
    {
    case WeaponType::AssaultRiffle:
    {
        weapon.texture = nullptr;
        weapon.shotSound = arShotSound;
        weapon.height = 12;
        weapon.width = 24;
        weapon.origin = { 10, 6 };
        weapon.damage = 30.0f;
        weapon.cooldown = 0.3f;
        weapon.magSize = 28;
        weapon.ammoLeft = 28;
        weapon.reloadTime = 2.0f;

        weapon.projTexture = nullptr;
        weapon.projCollisionSound = nullptr;
        weapon.projHeight = 7;
        weapon.projWidth = 7;
        weapon.projSpeed = 300.0f;

        break;
    }

    case WeaponType::SMG:
    {
        weapon.texture = nullptr;
        weapon.shotSound = smgShotSound;
        weapon.height = 10;
        weapon.width = 20;
        weapon.origin = { 8, 5 };
        weapon.damage = 15.0f;
        weapon.cooldown = 0.1f;
        weapon.magSize = 40;
        weapon.ammoLeft = 40;
        weapon.reloadTime = 1.5f;

        weapon.projTexture = nullptr;
        weapon.projCollisionSound = nullptr;
        weapon.projHeight = 5;
        weapon.projWidth = 5;
        weapon.projSpeed = 350.0f;

        break;
    }

    case WeaponType::DMR:
    {
        weapon.texture = nullptr;
        weapon.shotSound = dmrShotSound;
        weapon.height = 14;
        weapon.width = 28;
        weapon.origin = { 11, 7 };
        weapon.damage = 45.0f;
        weapon.cooldown = 0.5f;
        weapon.magSize = 7;
        weapon.ammoLeft = 7;
        weapon.reloadTime = 3.0f;

        weapon.projTexture = nullptr;
        weapon.projCollisionSound = nullptr;
        weapon.projHeight = 10;
        weapon.projWidth = 10;
        weapon.projSpeed = 250.0f;

        break;
    }

    case WeaponType::Snipier:
    {
        weapon.texture = nullptr;
        weapon.shotSound = sniperShotSound;
        weapon.height = 16;
        weapon.width = 32;
        weapon.origin = { 13, 8 };
        weapon.damage = 60.0f;
        weapon.cooldown = 1.0f;
        weapon.magSize = 4;
        weapon.ammoLeft = 4;
        weapon.reloadTime = 4.0f;

        weapon.projTexture = nullptr;
        weapon.projCollisionSound = nullptr;
        weapon.projHeight = 12;
        weapon.projWidth = 12;
        weapon.projSpeed = 200.0f;

        break;
    }

    case WeaponType::None:
        break;
    }

    scene.AddComponent<WeaponComponent>(id, std::move(weapon));
}

void ProjectileCollisionSystem::Run(float dt, sl::Scene& scene)
{
    scene.ForEach<Projectile, TransformComponent, ColliderComponent, MovementComponent, TagComponent>([&](sl::EntityId id, Projectile& projectile, TransformComponent& transform, ColliderComponent& collider, MovementComponent& movement, TagComponent& tag)
        {
            sl::RectF projectileWorldRect = GetWorldCollider(scene, id);
            static sl::Sound* hitSound = se::Engine::GetAudio().LoadSound("assets/sounds/hit.wav");
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
                            se::Engine::GetAudio().PlaySound(hitSound);

                            return;
                        }
                    }
                });
            scene.ForEach<ColliderComponent, HealthComponent>([&](sl::EventId target, ColliderComponent& targetCollider, HealthComponent& targetHealth)
                {
                    if ( targetCollider.layer == ColliderComponent::CollisionLayer::Nothing) return;
                    if (scene.HasComponent<TagComponent>(target))
                    {
                        if (scene.GetComponent<TagComponent>(target).tag & tag.tag) return;
                    }
                    sl::RectF targetWorldRect = GetWorldCollider(scene, target);
                    if (targetWorldRect.IsOverlappingWith(projectileWorldRect))
                    {
                        CreateFloatingText(scene, std::to_string(int(projectile.damage)), 20.0f, nullptr, projectileWorldRect.GetCenter(), sl::Colors::Red, 1.5f, 50.0f, movement.dir);

                        float& health = scene.GetComponent<HealthComponent>(target).health;
                        if (health <= 0.0f) return;
                        if (scene.HasComponent<ShieldComponent>(target))
                        {
                            ShieldComponent& shield = scene.GetComponent<ShieldComponent>(target);
                            if (shield.shield <= 0.0f)
                            {
                                health -= projectile.damage;
                            }
                            else
                            {
                                shield.shield = std::max(0.0f, shield.shield - projectile.damage);
                            }
                            shield.cooldownLeft = shield.regenCooldown;
                            if (target == GameGlobals::player)
                            {
                                ShakeCamera(scene.GetComponent<Camera>(GameGlobals::camera), projectile.damage * 0.25f * (shield.maxShield - shield.shield) / shield.maxShield );
                            }
                        }
                        else
                        {
                            health -= projectile.damage;
                        }
                        scene.DestroyEntity(id);
                        if (health <= 0.0f)
                        {
                            if (target == GameGlobals::player)
                            {
                                CreateDeathAnimation(scene, target, 3.0f);
                                scene.GetComponent<SpriteComponent>(target).tint.a = 0.0f;
                                scene.GetComponent<ColliderComponent>(target).layer = ColliderComponent::CollisionLayer::Nothing;
                                scene.GetComponent<MovementComponent>(target).dir = {};
                            }
                            else
                            {
                                if (scene.HasComponent<TagComponent>(target))
                                {
                                    if (scene.GetComponent<TagComponent>(target).tag & uint32_t(Tags::enemy)) GameGlobals::killedEnemies++;;
                                }
                                CreateDeathAnimation(scene, target, 3.0f);
                                scene.DestroyEntity(target);
                            }
                        }
                    }
                });
        });
}

void WeaponSystem::Run(float dt, sl::Scene& scene)
{
    scene.ForEach<WeaponComponent>([&](sl::EntityId id, WeaponComponent& weapon)
        {
            weapon.remainingTime -= dt;
            weapon.reloadTimeLeft -= dt;
            if (weapon.ammoLeft == 0 && weapon.reloadTimeLeft <= 0.0f) weapon.ammoLeft = weapon.magSize;
        });
}

sl::EntityId CreateProjectile(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f dir, sl::EntityId weapon, TagComponent immune)
{
    assert(scene.HasComponent<WeaponComponent>(weapon));
    WeaponComponent& wpn = scene.GetComponent<WeaponComponent>(weapon);

    sl::EntityId projectile;
    projectile = scene.CreateEntity();
    scene.AddComponent<Projectile>(projectile, Projectile{ wpn.damage });
    scene.AddComponent<TagComponent>(projectile, std::move(immune));
    scene.AddComponent<TransformComponent>(projectile, TransformComponent{ pos, 0.0f });
    scene.AddComponent<MovementComponent>(projectile, MovementComponent{ dir, {}, wpn.projSpeed });
    scene.AddComponent<ColliderComponent>(projectile, ColliderComponent{ sl::RectF(0, wpn.projWidth, 0, wpn.projHeight), ColliderComponent::CollisionLayer::GameObjects });
    scene.AddComponent<SpriteComponent>(projectile, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(wpn.projWidth / 2, wpn.projHeight / 2), wpn.projTexture,
        sl::RectF(0.0f, wpn.projWidth, 0.0f, wpn.projHeight), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::White, sl::Vec2f(wpn.projWidth, wpn.projHeight), nullptr));

    return projectile;
}