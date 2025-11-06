#pragma once
#include<ScypEngine/Engine.h>

class MovementSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class InputReadSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class CameraSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class ExecuteEventSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class RenderSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};