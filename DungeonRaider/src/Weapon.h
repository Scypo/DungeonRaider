#pragma once
#include"ScypEngine/Engine.h"


struct ProjectileData
{
    sl::Texture* texture = nullptr;
    float height = 10;
    float width = 10;
    float speed = 250.0f;
    float damage = 30.0f;
};

struct WeaponComponent
{
    sl::Texture* texture = nullptr;
    sl::Sound* shotSound = nullptr;
    int height = 16;
    int width = 32;
    sl::Vec2i origin = { 10,8 };
    float damage = 10.0f;
    float cooldown = 0.4f;
    float remainingTime = 0.0f;
    ProjectileData projectileData {};
};

struct Projectile
{
    sl::EntityId creator = 0;
    float damage = 0;
};

void WeaponAttack(sl::EntityId id, sl::Vec2f target);

class ProjectileCollisionSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override;
};

class WeaponSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override;
};