#include<algorithm>

#include "Systems.h"
#include"Components.h"
#include"Weapon.h"
#include"Pathfinder.h"
#include"FloatingText.h"
#include"Entities.h"
#include"UserInterface.h"

void MovementSystem::Run(float dt, sl::Scene& scene)
{	
	scene.ForEach<TransformComponent, MovementComponent>([&](sl::EntityId id, TransformComponent& transform, MovementComponent& movement)
		{
            movement.proposedPos = transform.pos + movement.dir * movement.speed * dt;
			if (!scene.HasComponent<ColliderComponent>(id)) transform.pos = movement.proposedPos;
		});
}

void CameraSystem::Run(float dt, sl::Scene& scene)
{
	scene.ForEach<Camera>([&](sl::EntityId id, Camera& cam)
		{
			if(cam.active)
			{
				cam.pos = scene.GetComponent<TransformComponent>(GameGlobals::player).pos;

				cam.pos.x -= se::Engine::GetGraphics().GetCanvasWidth() * 0.5f;
				cam.pos.y -= se::Engine::GetGraphics().GetCanvasHeight() * 0.5f;

				float scroll = se::Engine::GetMouse().GetScrollOffsetY();
				if (scroll > 0) cam.zoom += 0.05f;
				if (scroll < 0) cam.zoom = std::max(0.05f, cam.zoom - 0.05f);
			}
		});
}

void ExecuteEventSystem::Run(float dt, sl::Scene& scene)
{
	scene.GetEventBus().DispatchAll();
}

void RenderSystem::Run(float dt, sl::Scene& scene)
{
	sl::Graphics& gfx = se::Engine::GetGraphics();
	Camera& cam = scene.GetComponent<Camera>(GameGlobals::camera);
	gfx.BeginView(cam.pos, cam.zoom);
	gfx.SetDrawLayer(0.0f);
	DrawLevel(scene);
	gfx.SetDrawLayer(2.0f);
	DrawSprites(scene);
	gfx.SetDrawLayer(3.0f);
	DrawHealthBars(scene);
	gfx.SetDrawLayer(4.0f);
	DrawFloatingText(scene);
	//scene.ForEach<PathfindingComponent>([&](sl::EntityId id, PathfindingComponent& pathComp)//Draw Paths
	//	{
	//		for (auto& pos : pathComp.path)
	//		{
	//			gfx.DrawRect(pos, { 10,10 }, sl::Colors::Green);
	//		}
	//	});
	gfx.EndView();
	gfx.BeginView();
	gfx.SetDrawLayer(6.0f);
	DrawHUD(scene);
	gfx.DrawText(sl::Vec2f(0.0f, -25.0f), "FPS: " + std::to_string(int(1 / dt)), nullptr, 50.0f, sl::Colors::White);
	if (scene.GetComponent<HealthComponent>(GameGlobals::player).health <= 0.0f)
	{
		gfx.SetDrawLayer(7.0f);
		DrawDeathScreen(scene, dt);
	}
	gfx.EndView();
}

void DrawSprites(sl::Scene& scene)
{
	sl::Graphics& gfx = se::Engine::GetGraphics();
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
}