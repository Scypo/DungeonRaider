#pragma once
#include"vector"
#include"ScypLib/Vec2.h"
#include"ScypLib/EntityComponentSystem.h"
#include"Level.h"

struct PathfindingComponent
{
    PathfindingComponent() = default;
    PathfindingComponent(PathfindingComponent&& other) noexcept;
    PathfindingComponent(const PathfindingComponent& other);
    PathfindingComponent& operator=(const PathfindingComponent& other);
    std::vector<sl::Vec2f> path;
	float timeSincePathfining = 0.0f;
    float pathLifetime = 2.0f;
    bool suspended = false;
    sl::Vec2f target{};
};

bool IsPathClear(sl::Scene* scene, sl::EntityId id, sl::Vec2f goal);
bool IsPathClear(sl::Scene* scene, sl::Vec2f start, sl::Vec2f goal, sl::Vec2f size);

class Pathfinder
{
private:
	struct Cell
	{
        sl::Vec2i pos;
        sl::Vec2i parentPos;
        int G;
        int H;
        int F;
        bool operator<(const Cell& other) const
        {
            return F > other.F;
        }
        Cell(sl::Vec2i pos, sl::Vec2i parentPos, int G, int H)
            :
            pos(pos),
            parentPos(parentPos),
            G(G),
            H(H),
            F(G + H)
        {}
	};
private:
    std::priority_queue<Cell> openList;
    std::unordered_set<sl::Vec2i> closedList;
    std::unordered_map<sl::Vec2i, sl::Vec2i> parentMap;
    std::unordered_map<sl::Vec2i, int> costMap;
    std::vector<sl::Vec2i> directions =
    {
        { 1, 0 },   // Right
        { -1, 0 },  // Left
        { 0, 1 },   // Down
        { 0, -1 },  // Up
    };
    int eps = 10;
    int accuracy = 2;
public:
    Pathfinder() = default;
	void FindPath(sl::Scene* scene, sl::EntityId id, sl::Vec2f goalWorld);
private:
    int GetH(sl::Vec2i pos, sl::Vec2i goal);
    bool IsValid(sl::Vec2i pos, TilesetChunk& chunk, sl::Vec2i size);
    void SetPath(sl::Scene* scene, sl::Vec2i startPos, sl::Vec2i goalPos, TilesetChunk& chunk, sl::EntityId id);
    void SmoothPath(sl::Scene* scene, std::vector<sl::Vec2f>& path, TilesetChunk& chunk, sl::EntityId id);
};

class PathfindingSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override;
};