#include "Behavior.h"
#include"Components.h"

void EnemyBehaviorSystem::Run(float dt, sl::Scene& scene)
{
	sl::Vec2f playerPos = GetWorldCollider(&scene, GameContext::player).GetCenter();

	scene.ForEach<EnemyBehaviorComponent, MovementComponent, PathfindingComponent, WeaponComponent>([&](sl::EntityId id, EnemyBehaviorComponent& behav, MovementComponent& movement, PathfindingComponent& pathComp, WeaponComponent& weapon)
		{		
			if (behav.behavior == BehaviorStage::Idle) return;
			behav.target = playerPos;

			bool inRange = IsPathClear(id, playerPos) && behav.approachRange <= (playerPos - GetWorldCollider(&scene, id).GetCenter()).GetLength();
			if (!inRange)
			{
				behav.behavior = BehaviorStage::Approach;
			}
			if (inRange)
			{
				behav.behavior = BehaviorStage::Attack;
			}
				switch (behav.behavior)
				{
				case BehaviorStage::Idle:
					pathComp.path.clear();
						pathComp.suspended = true;
						break;
				case BehaviorStage::Approach:
					pathComp.suspended = false;
					pathComp.target = playerPos;
						break;
				case BehaviorStage::Attack:
					WeaponAttack(id, behav.target);
					pathComp.suspended = true;
					pathComp.path.clear();
						break;
				case BehaviorStage::Retreat:
					break;
				default:
					break;
				}
			});
}