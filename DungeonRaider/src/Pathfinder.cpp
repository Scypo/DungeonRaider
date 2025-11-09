#include "Pathfinder.h"
#include"ScypEngine/Engine.h"
#include"Level.h"
#include"Components.h"
#include"ScypLib/Logger.h"

static Pathfinder pathfinder;

void Pathfinder::FindPath(sl::EntityId id, sl::Vec2f goalWorld)
{
    sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
    
    sl::RectF entityWorldRect = GetWorldCollider(id);
    scene.ForEach<TilesetChunk>([&](sl::EntityId chunkId, TilesetChunk& chunk)
        {
            if (!entityWorldRect.IsContainedBy(sl::RectF(chunk.worldRect))) return;
            if (!chunk.worldRect.Contains(sl::Vec2i(goalWorld))) return;

            sl::Vec2i entitySize = sl::Vec2i(int(entityWorldRect.GetWidth()), int(entityWorldRect.GetHeight()));
            sl::Vec2i start = sl::Vec2i(entityWorldRect.GetCenter());
            sl::Vec2i goal = sl::Vec2i(goalWorld);

            if ((start - goal).GetLength() < eps)
            {
                LOG_DEBUG("start equal to goal " + std::to_string(id));
                return;
            }

            while (!openList.empty()) openList.pop();
            closedList.clear();
            parentMap.clear();
            costMap.clear();

            sl::Vec2i startGrid = chunk.WorldToGrid(float(start.x), float(start.y));
            if (chunk.collisionGrid[startGrid.y * chunk.width + startGrid.x])
            {
                LOG_DEBUG("pathfinding failed due to blocked start " + std::to_string(id));
                return;
            }
            sl::Vec2i goalGrid = chunk.WorldToGrid(float(goalWorld.x), float(goalWorld.y));
            if (chunk.collisionGrid[goalGrid.y * chunk.width + goalGrid.x])
            {
                LOG_DEBUG("pathfinding failed due to blocked goal" + std::to_string(id));
                return;
            }

            se::Engine::GetECS().GetCurrentScene()->GetComponent<PathfindingComponent>(id).path.clear();
            
            Cell startCell(start, start, 0, GetH(start, goal));

            openList.push(startCell);
            costMap[startCell.pos] = startCell.G;

            while (!openList.empty())
            {
                Cell currentCell = openList.top();
                openList.pop();

                if ((currentCell.pos - goal).GetLength() < eps)
                {
                    SetPath(start, currentCell.pos, chunk, id);
                    return;
                }

                if (closedList.find(currentCell.pos) != closedList.end()) continue;

                closedList.insert(currentCell.pos);

                for (const auto& dir : directions)
                {
                    sl::Vec2i neighborPos = { currentCell.pos.x + dir.x * entitySize.x / accuracy, currentCell.pos.y + dir.y * entitySize.y / accuracy };

                    if (!IsValid(neighborPos, chunk, entitySize)) continue;

                    bool isDiagonal = (dir.x != 0 && dir.y != 0);
                    
                    if (isDiagonal)
                    {
                        sl::Vec2i adj1 = { currentCell.pos.x + dir.x, currentCell.pos.y };
                        sl::Vec2i adj2 = { currentCell.pos.x, currentCell.pos.y + dir.y };
                        if (!IsValid(adj1, chunk, entitySize) || !IsValid(adj2, chunk, entitySize)) continue;
                    }

                    int movementCost = isDiagonal ? 14 : 10;

                    int neighborG = currentCell.G + movementCost;

                    if (closedList.find(neighborPos) != closedList.end()) continue;

                    if (costMap.find(neighborPos) == costMap.end() || neighborG < costMap[neighborPos])
                    {
                        parentMap[neighborPos] = currentCell.pos;
                        costMap[neighborPos] = neighborG;

                        Cell neighborCell(neighborPos, currentCell.pos, neighborG, GetH(neighborPos, goal));
                        openList.push(neighborCell);
                    }
                }
            }
            LOG_DEBUG("path not found" + std::to_string(id));
        });
}

int Pathfinder::GetH(sl::Vec2i pos, sl::Vec2i goal)
{
    int dx = pos.x - goal.x;
    int dy = pos.y - goal.y;

    return dx*dx + dy*dy;
}

bool Pathfinder::IsValid(sl::Vec2i worldPos, TilesetChunk& chunk, sl::Vec2i size)
{
    if (closedList.contains(worldPos)) return false;

    int halfW = size.x / 2;
    int halfH = size.y / 2;

    sl::Vec2i topLeft = chunk.WorldToGrid(worldPos.x - halfW, worldPos.y - halfH);
    sl::Vec2i bottomRight = chunk.WorldToGrid(worldPos.x + halfW, worldPos.y + halfH);

    for (int y = topLeft.y; y <= bottomRight.y; y++)
    {
        for (int x = topLeft.x; x <= bottomRight.x; x++)
        {
            if (chunk.collisionGrid[y * chunk.width + x]) return false;
        }
    }

    return true;
}

void Pathfinder::SetPath(sl::Vec2i startPos, sl::Vec2i goalPos, TilesetChunk& chunk, sl::EntityId id)
{
    std::vector<sl::Vec2f> path;
    
    sl::Vec2i currentPos = goalPos;
    while (currentPos != startPos)
    {
        sl::Vec2f pointPos = sl::Vec2f(currentPos);
        path.push_back(pointPos);
        currentPos = parentMap.at(currentPos);
    }
    assert(!path.empty());
    SmoothPath(path, chunk, id);
}

void Pathfinder::SmoothPath(std::vector<sl::Vec2f>& rawPath, TilesetChunk& chunk, sl::EntityId id)
{
    if (rawPath.size() < 2) return;
    std::vector<sl::Vec2f>& path = se::Engine::GetECS().GetCurrentScene()->GetComponent<PathfindingComponent>(id).path;
    ColliderComponent& collider = se::Engine::GetECS().GetCurrentScene()->GetComponent<ColliderComponent>(id);
    sl::Vec2f size = sl::Vec2f(collider.bounds.GetWidth(), collider.bounds.GetHeight());
    path.clear();

    size_t current = 0;
    size_t next = 1;

    while (next < rawPath.size())
    {
        while (next + 1 < rawPath.size() && IsPathClear(rawPath[current], rawPath[next], size))
        {
            break;
            next++;
        }
        path.push_back(rawPath[next]);
        current = next;
        next++;
    }
    if (path.back() != rawPath.back()) path.push_back(rawPath.back());
}

void PathfindingSystem::Run(float dt, sl::Scene& scene)
{
    float treshold = 3;
    scene.ForEach<PathfindingComponent, MovementComponent, TransformComponent>([&](sl::EntityId id, PathfindingComponent& pathComp, MovementComponent& movement, TransformComponent& transform)
        {
            if (pathComp.suspended) return;
            pathComp.timeSincePathfining += dt;
            sl::RectF worldCollider = GetWorldCollider(id);
            if (pathComp.pathLifetime <= pathComp.timeSincePathfining || pathComp.path.empty())
            {
                pathComp.timeSincePathfining = 0.0f;
                pathfinder.FindPath(id, pathComp.target);
            }
            else if ((worldCollider.GetCenter() - pathComp.path.back()).GetLength() < treshold)
            {
                pathComp.path.pop_back();
            }
            if (!pathComp.path.empty())
            {
                sl::Vec2f targetPos = pathComp.path.back();
                movement.dir = sl::Vec2f(targetPos - GetWorldCollider(id).GetCenter()).GetNormalized();
            }
            else
            {
                movement.dir = { 0, 0 };
            }
        });
}

PathfindingComponent::PathfindingComponent(PathfindingComponent&& other) noexcept
    : path(std::move(other.path)), timeSincePathfining(other.timeSincePathfining), pathLifetime(other.pathLifetime), suspended(other.suspended) {}

PathfindingComponent::PathfindingComponent(const PathfindingComponent& other)
    : path(other.path), timeSincePathfining(other.timeSincePathfining), pathLifetime(other.pathLifetime), suspended(other.suspended) {}

PathfindingComponent& PathfindingComponent::operator=(const PathfindingComponent& other)
{
    path = other.path;
    timeSincePathfining = other.timeSincePathfining;
    pathLifetime = other.pathLifetime;
    suspended = other.suspended;
    return *this;
}

bool IsPathClear(sl::Vec2f start, sl::Vec2f goal, sl::Vec2f size)
{
    int eps = 10;
    sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
    TilesetChunk* curChunk = nullptr;

    scene.ForEach<TilesetChunk>([&](sl::EntityId chunkId, TilesetChunk& chunk)
        {
            if (curChunk) return;
            if (chunk.worldRect.Contains(sl::Vec2i(start)) && chunk.worldRect.Contains(sl::Vec2i(goal)))
            {
                curChunk = &chunk;
            }
        });

    if (!curChunk) return false;

    int jumpLen = 5;
    int dx = std::abs(int(goal.x - start.x));
    int dy = -std::abs(int(goal.y - start.y));
    int sx = start.x < goal.x ? jumpLen : -jumpLen;
    int sy = start.y < goal.y ? jumpLen : -jumpLen;
    int err = dx + dy;

    sl::Vec2f pos = start;
    int halfW = size.x / 2;
    int halfH = size.y / 2;

    while ((pos - goal).GetLength() > eps)
    {

        sl::Vec2i topLeft = curChunk->WorldToGrid(pos.x - halfW, pos.y - halfH);
        sl::Vec2i bottomRight = curChunk->WorldToGrid(pos.x + halfW, pos.y + halfH);

        for (int y = topLeft.y; y <= bottomRight.y; y++)
        {
            for (int x = topLeft.x; x <= bottomRight.x; x++)
            {
                if (curChunk->collisionGrid[y * curChunk->width + x]) return false;
            }
        }

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            pos.x += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            pos.y += sy;
        }
    }

    return true;
}

bool IsPathClear(sl::EntityId id, sl::Vec2f goalWorld)
{
    sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
    TilesetChunk* curChunk = nullptr;
    sl::RectF worldCollider = GetWorldCollider(id);
    sl::Vec2f size = sl::Vec2f(worldCollider.GetWidth(), worldCollider.GetHeight());
    
    return IsPathClear(worldCollider.GetCenter(), goalWorld, size);
}
