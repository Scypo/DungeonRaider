#pragma once
#include"ScypEngine/Engine.h"
#include"Components.h"

struct FloatingText
{
	float fadeRate = 1.0f;
	sl::Color color = sl::Colors::White;
	std::string text = "";
	float height = 0.0f;
	sl::Font* font = nullptr;
};

inline void CreateFloatingText(sl::Scene& scene, const std::string& text, float height, sl::Font* font, sl::Vec2f pos, sl::Color color, float fadeRate, float speed = 10.0f, sl::Vec2f dir = { 0.0f,0.0f })
{
	sl::EntityId floatingText = scene.CreateEntity();
	scene.AddComponent<FloatingText>(floatingText, FloatingText{ fadeRate, color, text, height, font });
	scene.AddComponent<TransformComponent>(floatingText, TransformComponent{ pos, 0.0f });
	scene.AddComponent<MovementComponent>(floatingText, MovementComponent{ dir, pos, speed });
}

inline void DrawFloatingText(sl::Scene& scene)
{
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene.ForEach<FloatingText, TransformComponent>([&](sl::EntityId id, FloatingText& text, TransformComponent& transform)
		{
			gfx.DrawText(transform.pos, text.text, text.font, text.height, text.color);

		});
}

class FloatingTextSystem : public sl::System
{
	inline void Run(float dt, sl::Scene& scene) override
	{
		scene.ForEach<FloatingText>([&](sl::EntityId id, FloatingText& text)
			{
				text.color.a -= text.fadeRate * dt;
				if (text.color.a < 0.1f)
				{
					scene.DestroyEntity(id);
				}
			});
	}
};