#include "GameObjects.h"
#include<random>

#include<ScypEngine/Engine.h>

#include"Components.h"
#include"Pathfinder.h"
#include"Behavior.h"

sl::EntityId CreatePlayer(sl::Vec2f pos, float width, float height, sl::Texture* texture)
{
	sl::EntityComponentSystem& ecs = se::Engine::GetECS();
	sl::Scene* scene = ecs.GetCurrentScene();

	sl::EntityId player = scene->CreateEntity();
	scene->AddComponent<PlayerTag>(player, PlayerTag{});
	scene->AddComponent<HealthComponent>(player, HealthComponent{ 100.0f });
	scene->AddComponent<TransformComponent>(player, TransformComponent{ sl::Vec2f(450,350), 0.0f });
	scene->AddComponent<MovementComponent>(player, MovementComponent{});
	scene->AddComponent<ColliderComponent>(player, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene->AddComponent<SpriteComponent>(player, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::White, sl::Vec2f(width, height), nullptr));

	scene->AddComponent<WeaponComponent>(player, WeaponComponent{});
	GameContext::player = player;
	
	return player;
}

void CreateCamera()
{
	sl::EntityComponentSystem& ecs = se::Engine::GetECS();
	sl::Scene* scene = ecs.GetCurrentScene();

	sl::EntityId camera = scene->CreateEntity();
	scene->AddComponent<Camera>(camera, Camera{});
	GameContext::camera = camera;
}

sl::EntityId CreateEnemy(sl::Vec2f pos, float width, float height, sl::Texture* texture)
{
	sl::EntityComponentSystem& ecs = se::Engine::GetECS();
	sl::Scene* scene = ecs.GetCurrentScene();

	sl::EntityId enemy = scene->CreateEntity();
	scene->AddComponent<EnemyTag>(enemy, EnemyTag{});
	scene->AddComponent<PathfindingComponent>(enemy, PathfindingComponent{});
	scene->AddComponent<EnemyBehaviorComponent>(enemy, EnemyBehaviorComponent{ BehaviorStage::Approach, 200.0f, 1.0f, 0.0f, 5, 5, {} });
	scene->AddComponent<HealthComponent>(enemy, HealthComponent{ 100.0f });
	scene->AddComponent<TransformComponent>(enemy, TransformComponent{ pos, 0.0f });
	scene->AddComponent<MovementComponent>(enemy, MovementComponent{});
	scene->AddComponent<ColliderComponent>(enemy, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene->AddComponent<SpriteComponent>(enemy, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::Orange, sl::Vec2f(width, height), nullptr));
	scene->AddComponent<WeaponComponent>(enemy, WeaponComponent{});
	return enemy;
}

sl::EntityId CreateProjectile(sl::Vec2f pos, sl::Vec2f dir, const ProjectileData* data, sl::EntityId creator)
{
	sl::EntityComponentSystem& ecs = se::Engine::GetECS();
	sl::Scene* scene = ecs.GetCurrentScene();

	sl::EntityId projectile;
	projectile = scene->CreateEntity();
	scene->AddComponent<Projectile>(projectile, Projectile{ creator, data->damage });
	scene->AddComponent<TransformComponent>(projectile, TransformComponent{ pos, 0.0f });
	scene->AddComponent<MovementComponent>(projectile, MovementComponent{ dir, {}, data->speed });
	scene->AddComponent<ColliderComponent>(projectile, ColliderComponent{ sl::RectF(0, data->width, 0, data->height), ColliderComponent::CollisionLayer::GameObjects });
	scene->AddComponent<SpriteComponent>(projectile, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(data->width / 2, data->height/ 2), data->texture,
		sl::RectF(0.0f, data->width, 0.0f, data->height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::White, sl::Vec2f(data->width, data->height), nullptr));

	return projectile;
}

sl::EntityId CreateTileChunk(sl::Vec2i pos, int width, int height, int tileSize, sl::Texture* texture)
{
	sl::EntityComponentSystem& ecs = se::Engine::GetECS();
	sl::Scene* scene = ecs.GetCurrentScene();
	sl::EntityId chunkId = scene->CreateEntity();

	TilesetChunk chunk(pos, width, height, tileSize, texture);
	RoomTrigger trigger;

	SpawnRoom(chunk, trigger);
	SpawnObstacles(chunk, trigger);

	scene->AddComponent<TilesetChunk>(chunkId, std::move(chunk));
	scene->AddComponent<RoomTrigger>(chunkId, std::move(trigger));
	scene->AddComponent<TransformComponent>(chunkId, TransformComponent{ sl::Vec2f(pos), 0.0f });
	scene->AddComponent<RoomEncounter>(chunkId, RoomEncounter{1,2,5,1.0f,1.0f});
	return chunkId;
}

void CreateEnemiesInRoom(const TilesetChunk& chunk, const RoomTrigger& trigger, int least, int most)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> countDist(least, most);

	constexpr float biggestEnemySize = 40.0f;
	constexpr float smallestDistance = 2.0f * biggestEnemySize;

	std::uniform_real_distribution<float> xDist(trigger.worldBounds.left + 2.0f * biggestEnemySize, trigger.worldBounds.right -  2.0f * biggestEnemySize);
	std::uniform_real_distribution<float> yDist(trigger.worldBounds.top + 2.0f * biggestEnemySize, trigger.worldBounds.bottom - 2.0f * biggestEnemySize);

	int count = countDist(rng);

	int enemyWidthInTiles = 3;
	int enemyHeightInTiles = 3;

	auto blocks([&](int x, int y) -> bool
		{
			if (x < 0 || y < 0 || x >= chunk.width || y >= chunk.height) return true;
			return chunk.collisionGrid[y * chunk.width + x];
		});
	std::vector<sl::Vec2f> usedPos;
	usedPos.reserve(count);

	for (int i = 0; i < count; i++)
	{
		sl::Vec2f spawnPos{};
		bool valid = true;

		for (int attempts = 0; attempts < 100; attempts++)
		{
			valid = true;
			spawnPos = { xDist(rng), yDist(rng) };

			for (auto& pos : usedPos)
			{
				if ((pos - spawnPos).GetLength() < smallestDistance) valid = false;
			}

			sl::Vec2i gridTL = chunk.WorldToGrid(spawnPos.x, spawnPos.y);
			sl::Vec2i gridBR = chunk.WorldToGrid(spawnPos.x + biggestEnemySize, spawnPos.y + biggestEnemySize);

			if (gridTL.x < 0 || gridTL.y < 0 || gridBR.x >= chunk.width || gridBR.y >= chunk.height)
			{
				valid = false;
				continue;
			}

			for (int y = gridTL.y; y <= gridBR.y && valid; y++)
			{
				for (int x = gridTL.x; x <= gridBR.x; x++)
				{
					if (blocks(x, y)) valid = false;
				}
			}
			if (valid) break;
		}
		if (valid)
		{
			usedPos.push_back(spawnPos);
			CreateEnemy(spawnPos, 40.0f, 40.0f, nullptr);
		}
	}
}

void DrawGameObjects()
{
	sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene.ForEach<TransformComponent, SpriteComponent>([&](sl::EntityId id, TransformComponent& transform, SpriteComponent& sprite)
		{
			if (sprite.texture)
			{
				gfx.DrawTexture(transform.pos + sprite.offset, sprite.size, sprite.texture, sprite.shader,
					sprite.flip.x, sprite.flip.y, sprite.angle, sprite.pivot, &sprite.uv, sprite.tint);
			}
			else
			{
				gfx.DrawRect(transform.pos + sprite.offset, sprite.size, sprite.tint);
			}
		});
}
