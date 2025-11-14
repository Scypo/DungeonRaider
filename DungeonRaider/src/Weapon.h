#pragma once
#include"ScypEngine/Engine.h"
#include"Components.h"

struct WeaponComponent
{
    //weapon data
    sl::Texture* texture = nullptr;
    sl::Sound* shotSound = nullptr;
    int height = 16;
    int width = 32;
    sl::Vec2i origin = { 10,8 };
    float damage = 30.0f;
    float cooldown = 0.4f;
    float remainingTime = 0.0f;
    //projectile data
    sl::Texture* projTexture = nullptr;
    sl::Sound* projCollisionSound = nullptr;
    float projHeight = 10;
    float projWidth = 10;
    float projSpeed = 250.0f;
};

struct Projectile
{
    float damage = 0;
};

sl::EntityId CreateProjectile(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f dir, sl::EntityId weapon, TagComponent immune);
bool WeaponAttack(sl::Scene& scene, sl::EntityId id, sl::Vec2f target, TagComponent immune = {});

class ProjectileCollisionSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override;
};

class WeaponSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override;
};