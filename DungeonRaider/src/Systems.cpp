#include<algorithm>

#include "Systems.h"
#include"Components.h"
#include"GameObjects.h"
#include"Weapon.h"
#include"Pathfinder.h"
#include"FloatingText.h"

void MovementSystem::Run(float dt, sl::Scene& scene)
{	
	scene.ForEach<TransformComponent, MovementComponent>([&](sl::EntityId id, TransformComponent& transform, MovementComponent& movement)
		{
            movement.proposedPos = transform.pos + movement.dir * movement.speed * dt;
			if (!scene.HasComponent<ColliderComponent>(id)) transform.pos = movement.proposedPos;
		});
}

void InputReadSystem::Run(float dt, sl::Scene& scene)
{
	auto& kbd = se::Engine::GetKeyboard();
	auto& mouse = se::Engine::GetMouse();

	if (kbd.KeyIsPressed(GLFW_KEY_ESCAPE))
	{
		se::Engine::GetECS().SwitchScenes("BuyScreen");
		kbd.Flush();
	}

	MovementComponent& movement = scene.GetComponent<MovementComponent>(GameGlobals::player);
	movement.dir = { 0.0f,0.0f };
	if (kbd.KeyIsPressed('W')) movement.dir.y -= 1.0f;
	if (kbd.KeyIsPressed('S')) movement.dir.y += 1.0f;
	if (kbd.KeyIsPressed('A')) movement.dir.x -= 1.0f;
	if (kbd.KeyIsPressed('D')) movement.dir.x += 1.0f;

	WeaponComponent& weapon = scene.GetComponent<WeaponComponent>(GameGlobals::player);
	if(mouse.LeftIsPressed())
	{
		Camera& cam = scene.GetComponent<Camera>(GameGlobals::camera);
		sl::Vec2f mouseScreen = mouse.GetPos();
		sl::Vec2f mouseCanvas = sl::Vec2f{
			mouseScreen.x * (se::Engine::GetGraphics().GetCanvasWidth() / se::Engine::GetWindow().GetWidth()),
			mouseScreen.y * (se::Engine::GetGraphics().GetCanvasHeight() / se::Engine::GetWindow().GetHeight())
		};
		sl::Vec2f mouseWorldPos = mouseCanvas + cam.pos;

		WeaponAttack(&scene, GameGlobals::player, mouseWorldPos, TagComponent{ 0 | uint32_t(Tags::player) });
	}
	weapon.remainingTime -= dt;
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
	DrawLevel(&scene);
	gfx.SetDrawLayer(2.0f);
	DrawSprites(&scene);
	gfx.SetDrawLayer(3.0f);
	DrawHealthBars(&scene);
	gfx.SetDrawLayer(4.0f);
	DrawFloatingText(&scene);
	//scene.ForEach<PathfindingComponent>([&](sl::EntityId id, PathfindingComponent& pathComp)//Draw Paths
	//	{
	//		for (auto& pos : pathComp.path)
	//		{
	//			gfx.DrawRect(pos, { 10,10 }, sl::Colors::Green);
	//		}
	//	});
	gfx.EndView();
}

void DrawSprites(sl::Scene* scene)
{
	sl::Graphics& gfx = se::Engine::GetGraphics();
	scene->ForEach<TransformComponent, SpriteComponent>([&](sl::EntityId id, TransformComponent& transform, SpriteComponent& sprite)
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