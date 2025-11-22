#pragma once
#include"ScypEngine/Engine.h"
#include"Components.h"

enum class WeaponType
{
    AssaultRiffle,
    SMG,
    DMR,
    Snipier,
    None
};

struct WeaponComponent
{
    //weapon data
    sl::Texture* texture = nullptr;
    sl::Sound* shotSound = nullptr;
    int height = 0;
    int width = 0;
    sl::Vec2i origin = {};
    float damage = 0.0f;
    float cooldown = 0.0f;
    float remainingTime = 0.0f;
    int magSize = 0;
    int ammoLeft = 0;
    float reloadTime = 0.0f;
    float reloadTimeLeft = 0.0f;
    //projectile data
    sl::Texture* projTexture = nullptr;
    sl::Sound* projCollisionSound = nullptr;
    float projHeight = 0.0f;
    float projWidth = 0.0f;
    float projSpeed = 0.0f;
};

struct Projectile
{
    float damage = 0;
};

sl::EntityId CreateProjectile(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f dir, sl::EntityId weapon, TagComponent immune);
bool WeaponAttack(sl::Scene& scene, sl::EntityId id, sl::Vec2f target, TagComponent immune = {});
void AttachWeapon(sl::Scene& scene, sl::EntityId id, WeaponType type);
class ProjectileCollisionSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override;
};

class WeaponSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override;
};