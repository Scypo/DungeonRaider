#pragma once
#include"ScypEngine/Engine.h"
#include"Components.h"

struct FloatingText
{
	float fadeRate = 1.0f;
	sl::Color color = sl::Colors::White;
};

inline void CreateFloatingText(sl::Scene& scene, const std::string& text, sl::Vec2f pos, sl::Color color, float fadeRate, float speed = 10.0f, sl::Vec2f dir = { 0.0f,0.0f })
{
	sl::EntityId floatingText = scene.CreateEntity();
	scene.AddComponent<FloatingText>(floatingText, FloatingText{ fadeRate, color });
	scene.AddComponent<TransformComponent>(floatingText, TransformComponent{ pos, 0.0f });
	scene.AddComponent<MovementComponent>(floatingText, MovementComponent{ dir, pos, speed });
}

inline void DrawFloatingText(sl::Scene& scene)
{
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene.ForEach<FloatingText, TransformComponent>([&](sl::EntityId id, FloatingText& text, TransformComponent& transform)
		{
			gfx.DrawRect(transform.pos, sl::Vec2f(10.0f, 10.0f), text.color);
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