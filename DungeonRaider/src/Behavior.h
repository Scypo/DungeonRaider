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

void FindRetreatPos(sl::EntityId id);

struct EnemyBehaviorComponent
{
	BehaviorStage behavior = BehaviorStage::Idle;
	float approachRange = 0.0f;
	float stageSwitchCooldown = 0.0f;
	float cooldowndLeft = 0.0f;
	sl::Vec2f target{};
};

class EnemyBehaviorSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};
