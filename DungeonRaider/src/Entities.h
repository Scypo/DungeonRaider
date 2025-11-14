#pragma once
#include<ScypLib/EntityComponentSystem.h>
#include<ScypLib/Graphics.h>
sl::EntityId CreatePlayer(sl::Scene& scene, sl::Vec2f pos, float width, float height, sl::Texture* texture);
sl::EntityId CreateEnemy(sl::Scene& scene, sl::Vec2f pos, float width, float height, float health, float damage, sl::Texture* texture);
void DrawHealthBars(sl::Scene& scene);

struct ShieldComponent
{
	float shield = 0.0f;
	float maxShield = 0.0f;
	float regenRate = 0.0f;
	float cooldownLeft = 0.0f;
	float regenCooldown = 0.0f;
};

struct HealthComponent
{
	float health = 0.0f;
	float maxHealth = 0.0f;
};

class ShieldSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};