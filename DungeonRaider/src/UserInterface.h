#pragma once
#include<ScypEngine/Engine.h>

sl::EntityId CreateButton(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f size, sl::Texture* texture, const sl::RectF& uv, std::function<void()> callback);
sl::EntityId CreateText(sl::Scene& scene, sl::Vec2f pos, const std::string& text, float height, sl::Color c, sl::Font* font);
class ButtonSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class UIRenderSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

struct TextComponent
{
	std::string text = "";
	float height = 0.0f;
	sl::Color color = sl::Colors::White;
	sl::Font* font = nullptr;
};