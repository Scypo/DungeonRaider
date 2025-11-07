#include<random>

#include "Behavior.h"
#include"Components.h"

static std::random_device dev;
static std::mt19937 rng(dev());


void EnemyBehaviorSystem::Run(float dt, sl::Scene& scene)
{
	sl::Vec2f playerPos = GetWorldCollider(&scene, GameContext::player).GetCenter();
	
	scene.ForEach<EnemyBehaviorComponent, MovementComponent, PathfindingComponent, WeaponComponent>([&](sl::EntityId id, EnemyBehaviorComponent& behav, MovementComponent& movement, PathfindingComponent& pathComp, WeaponComponent& weapon)
		{		
			if (behav.behavior == BehaviorStage::Idle) return;
			behav.target = playerPos;

			bool inRange = behav.approachRange >= (playerPos - GetWorldCollider(&scene, id).GetCenter()).GetLength();
			bool clearPath = IsPathClear(id, playerPos);
				switch (behav.behavior)
				{
				case BehaviorStage::Idle:
					pathComp.path.clear();
					pathComp.suspended = true;
						break;
				case BehaviorStage::Approach:
					pathComp.target = playerPos;
					if (inRange && clearPath)
					{
						pathComp.suspended = true;
						behav.behavior = BehaviorStage::Attack;
						pathComp.path.clear();
					}
						break;
				case BehaviorStage::Attack:
					movement.dir = { 0.0f, 0.0f };
					if (WeaponAttack(id, behav.target))
					{
						behav.attacksLeft--;
					}
					if (!clearPath)
					{
						pathComp.suspended = false;
						behav.behavior = BehaviorStage::Approach;
					}
					else if (behav.attacksLeft == 0)
					{
						std::uniform_int_distribution<std::mt19937::result_type> attacks(behav.attackLimit / 2, behav.attackLimit);
						behav.attacksLeft = attacks(rng);
						behav.behavior = BehaviorStage::Retreat;
						pathComp.target = FindRetreatPos(id, 70.0f);
						pathComp.suspended = false;
					}
						break;
				case BehaviorStage::Retreat:
					if (pathComp.path.empty() || !clearPath)
					{
						pathComp.path.clear();
						behav.behavior = BehaviorStage::Approach;
					}
					break;
				default:
					break;
				}
			});
}

sl::Vec2f FindRetreatPos(sl::EntityId id, float range)
{
	std::uniform_real_distribution<float> dirX(-1.0f, 1.0f);
	std::uniform_real_distribution<float> dirY(-1.0f, 1.0f);

	sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();

	TilesetChunk* chunk = nullptr;
	sl::RectF collider = GetWorldCollider(&scene, id);
	int halfW = collider.GetWidth() / 2;
	int halfH = collider.GetHeight() / 2;
	scene.ForEach<TilesetChunk>([&](sl::EntityId checkId, TilesetChunk& checkChunk)
		{
			if (!chunk && collider.IsContainedBy(sl::RectF(checkChunk.worldRect))) chunk = &checkChunk;
		});
	assert(chunk);

	sl::Vec2f dir{};
	do
	{
		dir = { dirX(rng), dirY(rng) };
	} while (dir.GetLength() < 0.0001f);
	
	sl::Vec2f pos = collider.GetCenter();

	int steps = int(range) / chunk->tileSize;

	for (int curStep = 1; curStep <= steps; curStep++)
	{
		sl::Vec2f checkPos = pos + dir * (float(chunk->tileSize) * curStep);
		bool valid = true;

		for (int y = -halfH + checkPos.y; y <= halfH + checkPos.y; y += chunk->tileSize)
		{
			for (int x = -halfW + checkPos.x; x <= halfW + checkPos.x; x += chunk->tileSize)
			{
				sl::Vec2i checkPosGrid = chunk->WorldToGrid(float(x), float(y));

				if (checkPosGrid.x < 0 || checkPosGrid.x >= chunk->width || checkPosGrid.y < 0 || checkPosGrid.y >= chunk->height 
					||  chunk->collisionGrid[checkPosGrid.y * chunk->width + checkPosGrid.x])
				{
					valid = false;
					break;
				}
			}
			if (!valid) break;
		}

		if (!valid) break;

		pos = checkPos;
	}
	return pos;
}
