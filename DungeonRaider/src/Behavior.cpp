#include<random>

#include "Behavior.h"
#include"Components.h"

static std::random_device dev;
static std::mt19937 rng(dev());


void EnemyBehaviorSystem::Run(float dt, sl::Scene& scene)
{
	sl::Vec2f playerPos = GetWorldCollider(GameGlobals::player).GetCenter();
	
	scene.ForEach<EnemyBehaviorComponent, MovementComponent, PathfindingComponent, WeaponComponent>([&](sl::EntityId id, EnemyBehaviorComponent& behav, MovementComponent& movement, PathfindingComponent& pathComp, WeaponComponent& weapon)
		{		
			if (behav.behavior == BehaviorStage::Idle) return;
			behav.target = playerPos;

			bool inRange = behav.approachRange >= (playerPos - GetWorldCollider(id).GetCenter()).GetLength();
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
					if (WeaponAttack(id, behav.target, TagComponent { 0 | uint32_t(Tags::enemy) }))
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
						pathComp.target = FindRetreatPos(id, 70.0f, playerPos);
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

sl::Vec2f FindRetreatPos(sl::EntityId id, float range, const sl::Vec2f playerPos)
{
	std::uniform_real_distribution<float> dirX(-1.0f, 1.0f);
	std::uniform_real_distribution<float> dirY(-1.0f, 1.0f);

	sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();

	TilesetChunk* chunk = nullptr;
	sl::RectF collider = GetWorldCollider(id);
	int halfW = int(collider.GetWidth()) / 2;
	int halfH = int(collider.GetHeight()) / 2;
	scene.ForEach<TilesetChunk>([&](sl::EntityId checkId, TilesetChunk& checkChunk)
		{
			if (!chunk && collider.IsContainedBy(sl::RectF(checkChunk.worldRect))) chunk = &checkChunk;
		});
	assert(chunk);

	sl::Vec2f dir{};
	float distFromPlayer = 0.0f;
	float minDist = float(std::max(collider.GetWidth(), collider.GetHeight())) * 2.0f;
	minDist *= minDist;

	for (int tries = 0; tries < 100 && dir.GetLength() < 0.0001f && distFromPlayer < minDist; tries++)
	{
		dir = { dirX(rng), dirY(rng) };
		dir.Normalize();

		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		if (len < 0.0001f) continue;
		dir.x /= len;
		dir.y /= len;

		sl::Vec2f toPlayer = { playerPos.x - collider.GetCenter().x, playerPos.y - collider.GetCenter().y };
		float proj = toPlayer.x * dir.x + toPlayer.y * dir.y;
		sl::Vec2f closestPoint = { collider.GetCenter().x + dir.x * proj, collider.GetCenter().y + dir.y * proj };

		float distToLine = std::sqrt((closestPoint.x - playerPos.x) * (closestPoint.x - playerPos.x) + (closestPoint.y - playerPos.y) * (closestPoint.y - playerPos.y));
	}
	
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
