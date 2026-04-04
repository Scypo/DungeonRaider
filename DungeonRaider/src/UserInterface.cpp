#include "UserInterface.h"
#include"Components.h"
#include"Entities.h"
#include"Systems.h"
#include"Weapon.h"
#include "Scenes.h"

sl::EntityId CreateButton(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f size, sl::Texture* texture, const sl::RectF& uv, std::function<void()> callback)
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

void ButtonSystem::Run(float dt, sl::Scene& scene)
{
	if (se::Engine().GetKeyboard().KeyIsPressed(sl::Key::Escape))
	{
		se::Engine::GetECS().SwitchScenes("Level", true);
		se::Engine::GetKeyboard().Flush();
	}
	sl::Graphics& gfx = se::Engine::GetGraphics();
	sl::Vec2f mouseScreenPos = se::Engine::GetMouse().GetPos();
	sl::Vec2f mouseWorldPos = { mouseScreenPos.x / se::Engine::GetWindow().GetWidth() * gfx.GetCanvasWidth(),
		mouseScreenPos.y / se::Engine::GetWindow().GetHeight() * gfx.GetCanvasHeight() };

	bool action = se::Engine::GetMouse().LeftIsPressed() || se::Engine::GetKeyboard().KeyIsPressed(sl::Key::Space) || se::Engine::GetKeyboard().KeyIsPressed(sl::Key::Enter);
	scene.ForEach<CallbackComponent, ColliderComponent, SpriteComponent>([&](sl::EntityId id, CallbackComponent& button, ColliderComponent& collider, SpriteComponent& sprite)
		{
			sl::RectF worldRect = GetWorldCollider(scene, id);
			if (worldRect.Contains(mouseWorldPos))
			{
				sprite.tint.a = 0.7f;
				if (action)
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

	gfx.SetDrawDepth(0.0f);
	scene.ForEach<TransformComponent, SpriteComponent>([&](sl::EntityId id, TransformComponent& transform, SpriteComponent& sprite)
		{
			if (sprite.texture)
			{
				gfx.DrawTexture(transform.pos + sprite.offset, sprite.size, sprite.texture, sprite.shader,
					sprite.flip.x, sprite.flip.y, sprite.angle, &sprite.pivot, &sprite.uv, sprite.tint);
			}
			else
			{
				gfx.DrawRect(transform.pos + sprite.offset, sprite.size, sprite.tint);
			}
		});
	gfx.SetDrawDepth(1.0f);
	scene.ForEach<TransformComponent, TextComponent>([&](sl::EntityId id, TransformComponent& transform, TextComponent& text)
		{
			gfx.DrawText(transform.pos, text.text, text.font, text.height, text.color);
		});

	gfx.EndView();
	gfx.EndFrame();
}
