#pragma once
#include<ScypEngine/Engine.h>
#include"Components.h"

inline sl::EntityId CreateButton(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f size, sl::Texture* texture, sl::RectF& uv, std::function<void()> callback)
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

class ButtonSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override
	{
		if (se::Engine().GetKeyboard().KeyIsPressed(GLFW_KEY_ESCAPE))
		{
			se::Engine::GetECS().SwitchScenes("Level", false);
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
};

class UIRenderSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override
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

		gfx.EndView();
		gfx.EndFrame();
	}
};