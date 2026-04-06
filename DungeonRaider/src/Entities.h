#pragma once
#include<ScypLib/EntityComponentSystem.h>
#include<ScypLib/Graphics.h>
sl::EntityId CreatePlayer(sl::Scene& scene, sl::Vec2f pos, float width, float height, sl::Texture* texture);
sl::EntityId CreateEnemyBase(sl::Scene& scene, sl::Vec2f pos, float width, float height, float health, sl::Texture* texture);
sl::EntityId CreateRandomEnemy(sl::Scene& scene, sl::Vec2f pos, float difficulty);
sl::EntityId CreateDeathAnimation(sl::Scene& scene, sl::EntityId id, float fadeRate);
void DrawBar(const sl::Vec2f& pos, float width, float height, float fraction, sl::Color filledColor);
void DrawHealthBars(sl::Scene& scene);
void DrawReloadBars(sl::Scene& scene);
void DrawHUD(sl::Scene& scene);
void DrawDeathScreen(sl::Scene& scene, float dt);

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
	float timeSinceHit = 1.0f;
};

class PlayerSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class ShieldSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class DeathSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

struct DeathComponent
{
	float fadeRate = 0.0f;
};