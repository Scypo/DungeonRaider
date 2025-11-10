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
	scene->AddComponent<TagComponent>(player, TagComponent{ 0 | uint32_t(Tags::player) });
	scene->AddComponent<HealthComponent>(player, HealthComponent{ 100.0f, 100.0f });
	scene->AddComponent<TransformComponent>(player, TransformComponent{ sl::Vec2f(450,350), 0.0f });
	scene->AddComponent<MovementComponent>(player, MovementComponent{});
	scene->AddComponent<ColliderComponent>(player, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene->AddComponent<SpriteComponent>(player, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::White, sl::Vec2f(width, height), nullptr));

	scene->AddComponent<WeaponComponent>(player, WeaponComponent{});
	GameGlobals::player = player;
	
	return player;
}

sl::EntityId CreateEnemy(sl::Vec2f pos, float width, float height, float health, float damage, sl::Texture* texture)
{
	sl::EntityComponentSystem& ecs = se::Engine::GetECS();
	sl::Scene* scene = ecs.GetCurrentScene();

	sl::EntityId enemy = scene->CreateEntity();
	scene->AddComponent<TagComponent>(enemy, TagComponent{ 0 | uint32_t(Tags::enemy) });
	scene->AddComponent<PathfindingComponent>(enemy, PathfindingComponent{});
	scene->AddComponent<EnemyBehaviorComponent>(enemy, EnemyBehaviorComponent{ BehaviorStage::Approach, 200.0f, 1.0f, 0.0f, 5, 5, {} });
	scene->AddComponent<HealthComponent>(enemy, HealthComponent{ health, health });
	scene->AddComponent<TransformComponent>(enemy, TransformComponent{ pos, 0.0f });
	scene->AddComponent<MovementComponent>(enemy, MovementComponent{});
	scene->AddComponent<ColliderComponent>(enemy, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene->AddComponent<SpriteComponent>(enemy, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::Orange, sl::Vec2f(width, height), nullptr));
	scene->AddComponent<WeaponComponent>(enemy, WeaponComponent{});
	scene->GetComponent<WeaponComponent>(enemy).damage = damage;
	return enemy;
}

void DrawHealthBars()
{
	float height = 5.0f;
	sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene.ForEach<HealthComponent>([&](sl::EntityId id, HealthComponent& health)
		{
			if (id == GameGlobals::player) return;
			sl::RectF worldRect = GetWorldCollider(id);

			float filled = health.health / health.maxHealth * worldRect.GetWidth();

			sl::RectF filledBar = worldRect;
			filledBar.bottom = filledBar.top - height;
			filledBar.top = filledBar.bottom - height;
			filledBar.right = filledBar.left + filled;
			sl::RectF emptyBar = worldRect;
			emptyBar.bottom = emptyBar.top - height;
			emptyBar.top = emptyBar.bottom - height;
			emptyBar.left += filled;
			gfx.DrawRect(filledBar, sl::Colors::Red);
			gfx.DrawRect(emptyBar, sl::Colors::Black);
		});
}
