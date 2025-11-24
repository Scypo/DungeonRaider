#include"Scenes.h"
#include"Systems.h"
#include"Entities.h"
#include"Components.h"
#include"Level.h"
#include"Weapon.h"
#include"FloatingText.h"
#include"Pathfinder.h"
#include"Behavior.h"
#include"UserInterface.h"

sl::Scene& CreateSimulation()
{
	auto& ecs = se::Engine::GetECS();
	sl::Graphics& gfx = se::Engine::GetGraphics();
	ecs.CreateScene("Level");
	sl::Scene& scene = *ecs.GetScene("Level");

	gfx.LoadTexture("assets/images/tile_set8.png");

	CreateLevel(scene);
	CreatePlayer(scene, { 0.0f,0.0f }, 40.0f, 40.0f, gfx.LoadTexture("assets/images/lessShittyCharacter.png"));
	CreateCamera(scene);

	scene.RegisterSystem<PlayerSystem>();
	scene.RegisterSystem<ShieldSystem>();
	scene.RegisterSystem<WeaponSystem>();
	scene.RegisterSystem<DeathSystem>();
	scene.RegisterSystem<EnemyBehaviorSystem>();
	scene.RegisterSystem<PathfindingSystem>();
	scene.RegisterSystem<MovementSystem>();
	scene.RegisterSystem<TileCollisionSystem>();
	scene.RegisterSystem<ProjectileCollisionSystem>();
	scene.RegisterSystem<FloatingTextSystem>();
	scene.RegisterSystem<LevelSystem>();
	scene.RegisterSystem<CameraSystem>();
	scene.RegisterSystem<ExecuteEventSystem>();
	scene.RegisterSystem<RenderSystem>();
	return scene;
}

sl::Scene& CreateBuyScreen()
{
	auto& ecs = se::Engine::GetECS();
	sl::Graphics& gfx = se::Engine::GetGraphics();
	ecs.CreateScene("BuyScreen");
	sl::Scene& scene = *ecs.GetScene("BuyScreen");

	sl::Texture* texture = gfx.LoadTexture("assets/images/shopicons.png");
	sl::RectF uv(0.0f, 10.0f, 10.0f, 0.0f);
	std::vector<sl::RectF> uvs = {
		sl::RectF(0.0f, 19, 0.0f, 45.0f),
		sl::RectF(20.0f, 39, 0.0f, 45.0f),
		sl::RectF(40.0f, 59, 0.0f, 45.0f),
		sl::RectF(60.0f, 79, 0.0f, 45.0f) };

	CreateButton(scene, sl::Vec2f(64.0f, 90.0f), sl::Vec2f(80.0f, 180.0f), texture, uvs[0], [&]()
		{
			if (GameGlobals::spendingPoints > 0)
			{
				GameGlobals::spendingPoints--;
				ShieldComponent& shield = ecs.GetScene("Level")->GetComponent<ShieldComponent>(GameGlobals::player);
				shield.maxShield *= 1.05f;
				shield.regenRate *= 1.1f;
			}
		});

	CreateButton(scene, sl::Vec2f(208.0f, 90.0f), sl::Vec2f(80.0f, 180.0f), texture, uvs[1], [&]()
		{
			if (GameGlobals::spendingPoints > 0)
			{
				GameGlobals::spendingPoints--;
				HealthComponent& health = ecs.GetScene("Level")->GetComponent<HealthComponent>(GameGlobals::player);
				health.maxHealth = 1.1f * health.maxHealth;
			}
		});

	CreateButton(scene, sl::Vec2f(352.0f, 90.0f), sl::Vec2f(80.0f, 180.0f), texture, uvs[2], [&]()
		{
			if (GameGlobals::spendingPoints > 0)
			{
				GameGlobals::spendingPoints--;
				HealthComponent& health = ecs.GetScene("Level")->GetComponent<HealthComponent>(GameGlobals::player);
				health.health = health.maxHealth;
			}

		});

	CreateButton(scene, sl::Vec2f(496.0f, 90.0f), sl::Vec2f(80.0f, 180.0f), texture, uvs[3], [&]()
		{
			if (GameGlobals::spendingPoints > 0)
			{
				GameGlobals::spendingPoints--;
				WeaponComponent& weapon = ecs.GetScene("Level")->GetComponent<WeaponComponent>(GameGlobals::player);
				weapon.damage *= 1.1f;
			}

		});

	scene.RegisterSystem<ButtonSystem>();
	scene.RegisterSystem<ExecuteEventSystem>();
	scene.RegisterSystem<UIRenderSystem>();

	return scene;
}

sl::Scene& CreateReportScreen()
{
	auto& ecs = se::Engine::GetECS();
	sl::Graphics& gfx = se::Engine::GetGraphics();
	ecs.CreateScene("ReportScreen");
	sl::Scene& scene = *ecs.GetScene("ReportScreen");

	CreateText(scene, sl::Vec2f(50.0f, 10.0f), "You Died!", 50, sl::Colors::White, nullptr);
	CreateText(scene, sl::Vec2f(50.0f, 70.0f), "Rooms Cleared: " + std::to_string(GameGlobals::roomsCleared), 50, sl::Colors::White, nullptr);
	CreateText(scene, sl::Vec2f(50.0f, 130.0f), "Enemies Killed: " + std::to_string(GameGlobals::killedEnemies), 50, sl::Colors::White, nullptr);
	CreateText(scene, sl::Vec2f(50.0f, 190.0f), "Time Played: " + std::to_string(int(GameGlobals::timePlayed / 60.0f)) + " minutes", 50, sl::Colors::White, nullptr);
	CreateText(scene, sl::Vec2f(50.0f, 300.0f), "Press Enter To Continue" , 25, sl::Colors::White, nullptr);
	sl::EntityId button = CreateButton(scene, sl::Vec2f(0.0f, 0.0f), sl::Vec2f(gfx.GetCanvasWidth(), gfx.GetCanvasHeight()), nullptr, sl::RectF{}, [&]()
		{
			CreateMainMenu();
			se::Engine::GetECS().SwitchScenes("MainMenu", true);
			se::Engine::GetKeyboard().Flush();
		});
	scene.GetComponent<SpriteComponent>(button).tint = sl::Color(0.0f,0.0f,0.0f,0.0f);

	scene.RegisterSystem<ButtonSystem>();
	scene.RegisterSystem<ExecuteEventSystem>();
	scene.RegisterSystem<UIRenderSystem>();
	return scene;
}

sl::Scene& CreateMainMenu()
{
	auto& ecs = se::Engine::GetECS();
	sl::Graphics& gfx = se::Engine::GetGraphics();
	ecs.CreateScene("MainMenu");
	sl::Scene& scene = *ecs.GetScene("MainMenu");

	sl::Texture* texture = gfx.LoadTexture("assets/images/buttons.png");
	sl::RectF uv(0.0f, 10.0f, 10.0f, 0.0f);
	std::vector<sl::RectF> uvs = {
		sl::RectF(0.0f, 59, 0.0f, 23.0f),
		sl::RectF(0.0f, 59, 24.0f, 47.0f),
		sl::RectF(0.0f, 59, 48.0f, 71.0f) };

	CreateText(scene, sl::Vec2f(245.0f, -25.0f), "Start", 100.0f, sl::Colors::White, nullptr);
	CreateButton(scene, sl::Vec2f(200.0f, 12.0f), sl::Vec2f(240.0f, 96.0f), texture, uvs[0], [&]()
		{
			CreateSimulation();
			se::Engine::GetECS().SwitchScenes("Level", true);
			se::Engine::GetKeyboard().Flush();
		});

	CreateText(scene, sl::Vec2f(225.0f, 119.0f), "Settings", 80.0f, sl::Colors::White, nullptr);
	CreateButton(scene, sl::Vec2f(200.0f, 132.0f), sl::Vec2f(240.0f, 96.0f), texture, uvs[2], [&]()
		{
		});

	CreateText(scene, sl::Vec2f(260.0f, 220.0f), "Quit", 100.0f, sl::Colors::White, nullptr);
	CreateButton(scene, sl::Vec2f(200.0f, 252.0f), sl::Vec2f(240.0f, 96.0f), texture, uvs[1], [&]()
		{
			se::Engine::Quit();
		});


	scene.RegisterSystem<ButtonSystem>();
	scene.RegisterSystem<ExecuteEventSystem>();
	scene.RegisterSystem<UIRenderSystem>();
	return scene;
}