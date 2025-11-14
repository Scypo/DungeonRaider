#include "Entities.h"
#include<random>

#include"Components.h"
#include"Pathfinder.h"
#include"Behavior.h"

sl::EntityId CreatePlayer(sl::Scene& scene, sl::Vec2f pos, float width, float height, sl::Texture* texture)
{
	sl::EntityId player = scene.CreateEntity();
	scene.AddComponent<TagComponent>(player, TagComponent{ 0 | uint32_t(Tags::player) });
	scene.AddComponent<HealthComponent>(player, HealthComponent{ 100.0f, 100.0f });
	scene.AddComponent<ShieldComponent>(player, ShieldComponent{ 100.0f, 100.0f, 5.0f, 1.5f,1.5f });
	scene.AddComponent<TransformComponent>(player, TransformComponent{ sl::Vec2f(450,350), 0.0f });
	scene.AddComponent<MovementComponent>(player, MovementComponent{});
	scene.AddComponent<ColliderComponent>(player, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene.AddComponent<SpriteComponent>(player, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::White, sl::Vec2f(width, height), nullptr));

	scene.AddComponent<WeaponComponent>(player, WeaponComponent{});
	GameGlobals::player = player;

	return player;
}

sl::EntityId CreateEnemy(sl::Scene& scene, sl::Vec2f pos, float width, float height, float health, float damage, sl::Texture* texture)
{
	sl::EntityId enemy = scene.CreateEntity();
	scene.AddComponent<TagComponent>(enemy, TagComponent{ 0 | uint32_t(Tags::enemy) });
	scene.AddComponent<PathfindingComponent>(enemy, PathfindingComponent{});
	scene.AddComponent<EnemyBehaviorComponent>(enemy, EnemyBehaviorComponent{ BehaviorStage::Approach, 200.0f, 1.0f, 0.0f, 5, 5, {} });
	scene.AddComponent<ShieldComponent>(enemy, ShieldComponent{ 100.0f, 100.0f, 5.0f, 1.5f,1.5f });
	scene.AddComponent<HealthComponent>(enemy, HealthComponent{ health, health });
	scene.AddComponent<TransformComponent>(enemy, TransformComponent{ pos, 0.0f });
	scene.AddComponent<MovementComponent>(enemy, MovementComponent{});
	scene.AddComponent<ColliderComponent>(enemy, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene.AddComponent<SpriteComponent>(enemy, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::Orange, sl::Vec2f(width, height), nullptr));
	scene.AddComponent<WeaponComponent>(enemy, WeaponComponent{});
	scene.GetComponent<WeaponComponent>(enemy).damage = damage;
	return enemy;
}

void DrawHealthBars(sl::Scene& scene)
{
	float height = 5.0f;
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene.ForEach<HealthComponent>([&](sl::EntityId id, HealthComponent& health)
		{
			if (id == GameGlobals::player) return;
			sl::RectF worldRect = GetWorldCollider(scene, id);

			sl::Color c = sl::Colors::Red;
			float filled = health.health / health.maxHealth * worldRect.GetWidth();
			if (scene.HasComponent<ShieldComponent>(id))
			{
				ShieldComponent& shield = scene.GetComponent<ShieldComponent>(id);
				if (shield.shield > 0.0f)
				{
					filled = shield.shield / shield.maxShield * worldRect.GetWidth();
					c = sl::Colors::Gray;
				}
			}

			if (filled > 0.95f * worldRect.GetWidth()) return;

			sl::RectF filledBar = worldRect;
			filledBar.bottom = filledBar.top - height;
			filledBar.top = filledBar.bottom - height;
			filledBar.right = filledBar.left + filled;
			sl::RectF emptyBar = worldRect;
			emptyBar.bottom = emptyBar.top - height;
			emptyBar.top = emptyBar.bottom - height;
			emptyBar.left += filled;
			gfx.DrawRect(filledBar, c);
			gfx.DrawRect(emptyBar, sl::Colors::Black);
		});
}

void ShieldSystem::Run(float dt, sl::Scene& scene)
{
	scene.ForEach<ShieldComponent>([&](sl::EntityId id, ShieldComponent& shield)
		{
			shield.cooldownLeft -= dt;
			if (shield.cooldownLeft <= 0.0f)
			{
				shield.shield = std::min(shield.shield + shield.regenRate * dt, shield.maxShield);
			}
		});
}
