#include "Entities.h"
#include<random>

#include"Components.h"
#include"Pathfinder.h"
#include"Behavior.h"
#include"Scenes.h"


sl::EntityId CreatePlayer(sl::Scene& scene, sl::Vec2f pos, float width, float height, sl::Texture* texture)
{
	sl::EntityId player = scene.CreateEntity();
	scene.AddComponent<TagComponent>(player, TagComponent{ 0 | uint32_t(Tags::player) });
	scene.AddComponent<HealthComponent>(player, HealthComponent{ 200.0f, 200.0f });
	scene.AddComponent<ShieldComponent>(player, ShieldComponent{ 200.0f, 200.0f, 5.0f, 1.5f,1.5f });
	scene.AddComponent<TransformComponent>(player, TransformComponent{ sl::Vec2f(450,350), 0.0f });
	scene.AddComponent<MovementComponent>(player, MovementComponent{});
	scene.AddComponent<ColliderComponent>(player, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene.AddComponent<SpriteComponent>(player, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Colors::White, sl::Vec2f(width, height), nullptr));

	AttachWeapon(scene, player, WeaponType::AssaultRiffle);
	GameGlobals::player = player;

	return player;
}

sl::EntityId CreateEnemyBase(sl::Scene& scene, sl::Vec2f pos, float width, float height, float health, sl::Texture* texture)
{
	sl::EntityId enemy = scene.CreateEntity();
	scene.AddComponent<TagComponent>(enemy, TagComponent{ 0 | uint32_t(Tags::enemy) });
	scene.AddComponent<PathfindingComponent>(enemy, PathfindingComponent{});
	scene.AddComponent<EnemyBehaviorComponent>(enemy, EnemyBehaviorComponent{ BehaviorStage::Spawn, 400.0f, 0.0f, 0.0f, 5, 5, {} });
	scene.AddComponent<HealthComponent>(enemy, HealthComponent{ health, health });
	scene.AddComponent<TransformComponent>(enemy, TransformComponent{ pos, 0.0f });
	scene.AddComponent<MovementComponent>(enemy, MovementComponent{});
	scene.AddComponent<ColliderComponent>(enemy, ColliderComponent{ sl::RectF(0, width, 0, height), ColliderComponent::CollisionLayer::World });
	scene.AddComponent<SpriteComponent>(enemy, SpriteComponent(sl::Vec2f(0, 0), sl::Vec2f(width / 2, height / 2), texture,
		sl::RectF(0.0f, width, 0.0f, height), sl::Vec2<bool>(false, false), 0.0f, sl::Color(1.0f,0.5f,0.5f,0.0f), sl::Vec2f(width, height), nullptr));
	return enemy;
}

sl::EntityId CreateRandomEnemy(sl::Scene& scene, sl::Vec2f pos, float difficulty)
{
	static std::random_device dev;
	static std::mt19937 rng(dev());
	static std::uniform_int_distribution<int> typeDist(0, int(WeaponType::None) - 1);
	WeaponType type = WeaponType(typeDist(rng));
	sl::EntityId enemy{};
	switch (type)
	{
	case WeaponType::AssaultRiffle:
	{
		enemy = CreateEnemyBase(scene, pos, 40.0f, 40.0f, 100.0f * difficulty, nullptr);
		AttachWeapon(scene, enemy, WeaponType::AssaultRiffle);
		scene.GetComponent<SpriteComponent>(enemy).tint = sl::Colors::Red;
		scene.AddComponent<ShieldComponent>(enemy, ShieldComponent{ 100.0f * difficulty, 100.0f * difficulty, 5.0f, 0.0f,1.5f });
		scene.GetComponent<WeaponComponent>(enemy).damage *= difficulty;
		break;
	}
	case WeaponType::SMG:
	{
		enemy = CreateEnemyBase(scene, pos, 30.0f, 30.0f, 90.0f, nullptr);
		AttachWeapon(scene, enemy, WeaponType::SMG);
		scene.GetComponent<SpriteComponent>(enemy).tint = sl::Colors::Green;
		scene.AddComponent<ShieldComponent>(enemy, ShieldComponent{ 120.0f * difficulty, 120.0f * difficulty, 4.5f, 0.0f,1.3f });
		scene.GetComponent<WeaponComponent>(enemy).damage *= difficulty;
		break;
	}
	case WeaponType::DMR:
	{
		enemy = CreateEnemyBase(scene, pos, 40.0f, 40.0f, 80.0f, nullptr);
		AttachWeapon(scene, enemy, WeaponType::DMR);
		scene.GetComponent<SpriteComponent>(enemy).tint = sl::Colors::Magenta;
		scene.AddComponent<ShieldComponent>(enemy, ShieldComponent{ 30.0f * difficulty, 30.0f * difficulty, 5.0f, 0.0f,1.55f });
		scene.GetComponent<WeaponComponent>(enemy).damage *= difficulty;
		break;
	}
	case WeaponType::Snipier:
	{
		enemy = CreateEnemyBase(scene, pos, 30.0f, 30.0f, 70.0f, nullptr);
		AttachWeapon(scene, enemy, WeaponType::Snipier);
		scene.GetComponent<SpriteComponent>(enemy).tint = sl::Colors::Cyan;
		scene.AddComponent<ShieldComponent>(enemy, ShieldComponent{ 10.0f * difficulty, 10.0f * difficulty, 5.5f, 0.0f,1.75f });
		scene.GetComponent<WeaponComponent>(enemy).damage *= difficulty;
		break;
	}
	default:
		break;
	}
	return enemy;
}

sl::EntityId CreateDeathAnimation(sl::Scene& scene, sl::EntityId id, float fadeRate)
{
	sl::EntityId animation = scene.CreateEntity();
	SpriteComponent sprite = scene.GetComponent<SpriteComponent>(id);
	scene.AddComponent<SpriteComponent>(animation, std::move(sprite));
	TransformComponent transform = scene.GetComponent<TransformComponent>(id);
	scene.AddComponent<TransformComponent>(animation, std::move(transform));
	scene.AddComponent<DeathComponent>(animation, DeathComponent{ fadeRate });
	return animation;
}

void DrawBar(const sl::Vec2f& pos, float width, float height, float fraction, sl::Color filledColor)
{
	sl::Graphics& gfx = se::Engine::GetGraphics();
	fraction = std::clamp(fraction, 0.0f, 1.0f);

	sl::RectF filled(pos, width * fraction, height);
	sl::RectF empty = filled;
	empty.left = filled.right;
	empty.right = filled.left + width;

	gfx.DrawRect(filled, filledColor);
	gfx.DrawRect(empty, sl::Colors::Black);
}

void DrawHealthBars(sl::Scene& scene)
{
	float height = 5.0f;
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene.ForEach<HealthComponent>([&](sl::EntityId id, HealthComponent& health)
		{
			if (id == GameGlobals::player) return;
			sl::RectF worldRect = GetWorldCollider(scene, id);
			float barWidth = worldRect.GetWidth();
			sl::Vec2f barPos(worldRect.left, worldRect.top - height);

			float fraction = health.health / health.maxHealth;
			sl::Color barColor = sl::Colors::Red;

			if (scene.HasComponent<ShieldComponent>(id))
			{
				ShieldComponent& shield = scene.GetComponent<ShieldComponent>(id);
				if (shield.shield > 0.0f)
				{
					fraction = shield.shield / shield.maxShield;
					barColor = sl::Colors::Gray;
					if (fraction >= 0.95f) return;
				}
			}

			DrawBar(barPos, barWidth, height, fraction, barColor);
		});
}

void DrawReloadBars(sl::Scene& scene)
{
	float height = 3.0f;
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene.ForEach<WeaponComponent>([&](sl::EntityId id, WeaponComponent& weapon)
		{
			if (weapon.reloadTimeLeft <= 0.0f) return;
			sl::RectF worldRect = GetWorldCollider(scene, id);
			float barWidth = worldRect.GetWidth();
			sl::Vec2f barPos(worldRect.left, worldRect.top - height - 7);

			float fraction = weapon.reloadTimeLeft/ weapon.reloadTime;
			sl::Color barColor = sl::Colors::White;

			DrawBar(barPos, barWidth, height, fraction, barColor);
		});
}

void DrawHUD(sl::Scene& scene)
{
	float barWidth = 150.0f;
	float barHeight = 20.0f;
	sl::Vec2f shieldPos(50.0f, 50.0f);
	sl::Vec2f healthPos(50.0f, 80.0f);

	auto& shield = scene.GetComponent<ShieldComponent>(GameGlobals::player);
	DrawBar(shieldPos, barWidth, barHeight, shield.shield / shield.maxShield, sl::Colors::Gray);

	auto& health = scene.GetComponent<HealthComponent>(GameGlobals::player);
	DrawBar(healthPos, barWidth, barHeight, health.health / health.maxHealth, sl::Colors::Red);
}

void DrawDeathScreen(sl::Scene& scene, float dt)
{
	static sl::Color c = sl::Color(0.0f, 0.0f, 0.0f, 0.0f);
	float fadeRate = 0.7f;
	c.a += fadeRate * dt;
	sl::Graphics& gfx = se::Engine::GetGraphics();
	gfx.DrawRect(gfx.GetCanvasRect(), c);
	if (c.a >= 1.2f)
	{
		sl::Scene* newScene = &CreateReportScreen();
		se::Engine::GetECS().SwitchScenes(newScene, true);
	}
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

void DeathSystem::Run(float dt, sl::Scene& scene)
{
	scene.ForEach<SpriteComponent, DeathComponent>([&](sl::EntityId id, SpriteComponent& sprite, DeathComponent& death)
		{
			sprite.tint.a -= death.fadeRate * dt;
			if (sprite.tint.a < 0.1f) scene.DestroyEntity(id);
		});
}

void PlayerSystem::Run(float dt, sl::Scene& scene)
{
	GameGlobals::timePlayed += dt;
	if (scene.GetComponent<HealthComponent>(GameGlobals::player).health <= 0.0f) return;
	auto& kbd = se::Engine::GetKeyboard();
	auto& mouse = se::Engine::GetMouse();

	if (kbd.KeyIsPressed(sl::Key::Escape))
	{
		CreateBuyScreen();
		se::Engine::GetECS().SwitchScenes("BuyScreen", false);
		kbd.Flush();
	}

	MovementComponent& movement = scene.GetComponent<MovementComponent>(GameGlobals::player);
	movement.dir = { 0.0f,0.0f };
	if (kbd.KeyIsPressed('W')) movement.dir.y -= 1.0f;
	if (kbd.KeyIsPressed('S')) movement.dir.y += 1.0f;
	if (kbd.KeyIsPressed('A')) movement.dir.x -= 1.0f;
	if (kbd.KeyIsPressed('D')) movement.dir.x += 1.0f;

	WeaponComponent& weapon = scene.GetComponent<WeaponComponent>(GameGlobals::player);
	if (mouse.LeftIsPressed())
	{
		Camera& cam = scene.GetComponent<Camera>(GameGlobals::camera);
		sl::Vec2f mouseScreen = mouse.GetPos();
		sl::Vec2f mouseCanvas = sl::Vec2f{
			mouseScreen.x * (se::Engine::GetGraphics().GetCanvasWidth() / se::Engine::GetWindow().GetWidth()),
			mouseScreen.y * (se::Engine::GetGraphics().GetCanvasHeight() / se::Engine::GetWindow().GetHeight())
		};
		sl::Vec2f mouseWorldPos = mouseCanvas + cam.pos;

		WeaponAttack(scene, GameGlobals::player, mouseWorldPos, TagComponent{ 0 | uint32_t(Tags::player) });
	}
	if (kbd.KeyIsPressed('R') && weapon.reloadTimeLeft <= 0.0f)
	{
		weapon.ammoLeft = 0;
		weapon.reloadTimeLeft = weapon.reloadTime;
	}
	weapon.remainingTime -= dt;
}
