#include "UserInterface.h"
#include"Components.h"
#include"Entities.h"
#include"Systems.h"
#include"Weapon.h"

sl::EntityId CreateButton(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f size, sl::Texture* texture, sl::RectF& uv, std::function<void()> callback)
{
	sl::EntityId button = scene.CreateEntity();
	scene.AddComponent<TransformComponent>(button, TransformComponent{ pos, 0.0f });
	SpriteComponent sprite{};
	sprite.uv = uv;
	sprite.texture = texture;
	sprite.size = size;
	scene.AddComponent<SpriteComponent>(button, std::move(sprite));
	ColliderComponent collider{};
	collider.bounds = sl::RectF(0, size.x, 0, size.y);
	scene.AddComponent<ColliderComponent>(button, std::move(collider));
	scene.AddComponent<CallbackComponent>(button, CallbackComponent{ callback });
	return button;
}

sl::EntityId CreateText(sl::Scene& scene, sl::Vec2f pos, const std::string& text, float height, sl::Color c, sl::Font* font)
{
	sl::EntityId txt = scene.CreateEntity();
	scene.AddComponent<TransformComponent>(txt, TransformComponent{ pos,0.0f });
	scene.AddComponent<TextComponent>(txt, TextComponent{ text,height,c,font });
	return txt;
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
	CreateText(scene, sl::Vec2f(50.0f, 190.0f), "Time Played: " + std::to_string(GameGlobals::timePlayed), 50, sl::Colors::White, nullptr);


	scene.RegisterSystem<ButtonSystem>();
	scene.RegisterSystem<ExecuteEventSystem>();
	scene.RegisterSystem<UIRenderSystem>();
	return scene;
}

void ButtonSystem::Run(float dt, sl::Scene& scene)
{
	if (se::Engine().GetKeyboard().KeyIsPressed(GLFW_KEY_ESCAPE))
	{
		se::Engine::GetECS().SwitchScenes("Level", true);
		se::Engine::GetKeyboard().Flush();
	}
	sl::Graphics& gfx = se::Engine::GetGraphics();
	sl::Vec2f mouseScreenPos = se::Engine::GetMouse().GetPos();
	sl::Vec2f mouseWorldPos = { mouseScreenPos.x / se::Engine::GetWindow().GetWidth() * gfx.GetCanvasWidth(),
		mouseScreenPos.y / se::Engine::GetWindow().GetHeight() * gfx.GetCanvasHeight() };

	bool leftPressed = se::Engine::GetMouse().LeftIsPressed();
	scene.ForEach<CallbackComponent, ColliderComponent, SpriteComponent>([&](sl::EntityId id, CallbackComponent& button, ColliderComponent& collider, SpriteComponent& sprite)
		{
			sl::RectF worldRect = GetWorldCollider(scene, id);
			if (worldRect.Contains(mouseWorldPos))
			{
				sprite.tint.a = 0.7f;
				if (leftPressed)
				{
					button.callback();
					se::Engine::GetMouse().Flush();
				}
			}
			else
			{
				sprite.tint.a = 1.0f;
			}
		});
}

void UIRenderSystem::Run(float dt, sl::Scene& scene)
{
	sl::Graphics& gfx = se::Engine::GetGraphics();
	gfx.BeginFrame();
	gfx.BeginView();

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
	scene.ForEach<TransformComponent, TextComponent>([&](sl::EntityId id, TransformComponent& transform, TextComponent& text)
		{
			gfx.DrawText(transform.pos.x, transform.pos.y, text.text, text.font, text.height, text.color);
		});

	gfx.EndView();
	gfx.EndFrame();
}
