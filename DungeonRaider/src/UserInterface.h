#pragma once
#include<ScypEngine/Engine.h>

sl::EntityId CreateButton(sl::Scene& scene, sl::Vec2f pos, sl::Vec2f size, sl::Texture* texture, sl::RectF& uv, std::function<void()> callback);
class ButtonSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class UIRenderSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

void CreateBuyScreen();