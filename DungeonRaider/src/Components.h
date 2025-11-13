#pragma once
#include<ScypEngine/Engine.h>

struct GameGlobals
{
	inline static float score = 0.0f;
	inline static sl::EntityId player = -1;
	inline static sl::EntityId camera = -1;
	inline static sl::EntityId currentRoom = -1;
	inline static float elapsedTime = 0.0f;
	inline static int spendingPoints = 0;
};

struct TransformComponent
{
	sl::Vec2f pos = { 0.0f,0.0f };
	float angle = 0.0f;
};

struct MovementComponent
{
	sl::Vec2f dir = { 0.0f, 0.0f };
	sl::Vec2f proposedPos = { 0.0f, 0.0f };
	float speed = 250.0f;
};

struct ColliderComponent
{
	enum class CollisionLayer
	{
		Everything,
		World,
		GameObjects
	};
	sl::RectF bounds;
	CollisionLayer layer = CollisionLayer::Everything;
};

struct TriggerComponent
{
	sl::RectF bounds;
};

struct SpriteComponent
{
	SpriteComponent() = default;
	SpriteComponent(sl::Texture* texture)
		: uv(0.0f, float(texture->GetWidth()), float(texture->GetHeight()), 0.0f) {}
	SpriteComponent(const sl::Vec2f& offset, const sl::Vec2f& pivot,
		sl::Texture* texture, const sl::RectF& uv, const sl::Vec2<bool>& flip,
		float angle, const sl::Color& tint, const sl::Vec2f& size, sl::Shader* shader)
		: offset(offset), pivot(pivot), texture(texture), uv(uv),
		flip(flip), angle(angle), tint(tint), size(size), shader(shader) {}

	sl::Vec2f offset = { 0.0f,0.0f };
	sl::Vec2f pivot = { 0.0f,0.0f };
	sl::Vec2f size = { 0.0f,0.0f };
	sl::Texture* texture = nullptr;
	sl::RectF uv = sl::RectF(0, 0, 0, 0);
	sl::Vec2<bool> flip = { false, false };
	float angle = 0.0f;
	sl::Color tint = sl::Colors::White;
	sl::Shader* shader = nullptr;
};

struct HealthComponent
{
	float health = 0.0f;
	float maxHealth = 0.0f;
};

enum class Tags
{
	player = 1 << 0,
	enemy = 1 << 1
};

struct TagComponent
{
	uint32_t tag = 0;
};

struct Camera
{
	sl::Vec2f pos = { 0.0f,0.0f };
	float zoom = 1.0f;
	bool active = true;
};

struct CallbackComponent
{
	std::function<void()> callback;
};

inline sl::EntityId CreateCamera(sl::Scene* scene)
{
	assert(scene);
	sl::EntityId camera = scene->CreateEntity();
	scene->AddComponent<Camera>(camera, Camera{});
	GameGlobals::camera = camera;
	return camera;
}

inline sl::RectF GetWorldCollider(sl::Scene* scene, sl::EntityId id)
{
	assert(scene);
	assert(scene->HasComponent<TransformComponent>(id) && scene->HasComponent<ColliderComponent>(id));
	TransformComponent& transform = scene->GetComponent<TransformComponent>(id);
	ColliderComponent& collider = scene->GetComponent<ColliderComponent>(id);
	sl::RectF entityWorldRect;
	entityWorldRect.left = transform.pos.x + collider.bounds.left;
	entityWorldRect.top = transform.pos.y + collider.bounds.top;
	entityWorldRect.right = entityWorldRect.left + collider.bounds.GetWidth();
	entityWorldRect.bottom = entityWorldRect.top + collider.bounds.GetHeight();

	return entityWorldRect;
}
