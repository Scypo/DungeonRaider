#pragma once
#include<vector>

#include<ScypLib/Graphics.h>
#include<ScypLib/EntityComponentSystem.h>

enum class TileSprite
{
	floor,
	wallTop,
	fullWall,
	openGate,
	shutGate,
	speedTile,
	slowTile,
	temp
};
enum class TileType
{
	floor,
	fullWall,
	topWall
};

struct RoomEncounter
{
	int wavesLeft = 0;
	int leastEnemies = 0;
	int mostEnemies = 0;
	float deley = 0.0f;
	float deleyLeft = 0.0f;
};

struct RoomTrigger
{
	enum class State
	{
		Unexplored,
		Exploring,
		Explored
	};
	RoomTrigger() = default;
	RoomTrigger(RoomTrigger&& other) noexcept;
	RoomTrigger& operator=(const RoomTrigger& other);
	RoomTrigger(const RoomTrigger& other);
	State state = State::Unexplored;
	std::vector<size_t> gateIndexes;
	sl::RectF worldBounds = sl::RectF(0.0f, 0.0f, 0.0f, 0.0f);
};

struct TilesetChunk
{
	TilesetChunk() = default;
	TilesetChunk(sl::Vec2i pos, int width, int height, int tileSize, sl::Texture* texture);
	TilesetChunk(TilesetChunk&& other) noexcept;
	TilesetChunk(const TilesetChunk& other);
	TilesetChunk& operator=(const TilesetChunk& other);
	~TilesetChunk() = default;
	void SetTile(int x, int y, bool collides, TileSprite sprite, TileType type);
	sl::Vec2f GridToWorld(int x, int y) const;
	sl::Vec2i WorldToGrid(float x, float y) const;
	std::vector<bool> collisionGrid;
	std::vector<TileSprite> uvIndex;
	std::vector<TileType> tileTypes;
	std::vector<sl::RectF> uvs;
	sl::RectI worldRect{};
	sl::Texture* atlas = nullptr;
	int width = 0;
	int height = 0;
	int tileSize = 0;
};

sl::EntityId CreateTileChunk(sl::Scene* scene, sl::Vec2i pos, int width, int height, int tileSize, sl::Texture* texture);
void SpawnObstacles(TilesetChunk& chunk, RoomTrigger& trigger);
void SpawnRoom(TilesetChunk& chunk, RoomTrigger& trigger);
void CreateEnemiesInRoom(sl::Scene* scene, const TilesetChunk& chunk, const RoomTrigger& trigger, int least, int most);
void CreateLevel(sl::Scene* scene);
void DrawLevel(sl::Scene* scene);

class LevelSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class TileCollisionSystem : public sl::System
{
	void Run(float dt, sl::Scene& scene) override;
};

class DifficultyManager
{
public:
	float GetDifficultyScale() { return scale; };
	void UpdateDifficultyScale();
private:
	float scale = 1.0f;
	float rate = 0.05f;
};