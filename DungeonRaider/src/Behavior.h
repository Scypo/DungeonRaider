#pragma once
#include"ScypLib/EntityComponentSystem.h"
#include"Components.h"
#include"Pathfinder.h"
#include"Weapon.h"

enum class BehaviorStage
{
	Idle,
	Approach,
	Attack,
	Retreat
};

sl::Vec2f FindRetreatPos(sl::Scene* scene, sl::EntityId id, float range, const sl::Vec2f playerPos);

struct EnemyBehaviorComponent
{
	BehaviorStage behavior = BehaviorStage::Idle;
	float approachRange = 0.0f;
	float stageSwitchCooldown = 0.0f;
	float cooldowndLeft = 0.0f;
	int attacksLeft = 1;
	int attackLimit = 1;
	sl::Vec2f target{};
};

class EnemyBehaviorSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};
