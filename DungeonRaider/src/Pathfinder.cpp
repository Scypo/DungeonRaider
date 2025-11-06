#include "Pathfinder.h"
#include"ScypEngine/Engine.h"
#include"Level.h"
#include"Components.h"

static Pathfinder pathfinder;

void Pathfinder::FindPath(sl::EntityId id, sl::Vec2f goalWorld)
{
    sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
    
    sl::RectF entityWorldRect = GetWorldCollider(&scene, id);
    scene.ForEach<TilesetChunk>([&](sl::EntityId chunkId, TilesetChunk& chunk)
        {
            if (!entityWorldRect.IsContainedBy(sl::RectF(chunk.worldRect))) return;
            if (!chunk.worldRect.Contains(sl::Vec2i(goalWorld))) return;

            sl::Vec2i start = chunk.WorldToGrid(entityWorldRect.GetCenter().x, entityWorldRect.GetCenter().y);
            sl::Vec2i goal = chunk.WorldToGrid(goalWorld.x, goalWorld.y);

            if (start == goal)
            {
                LOG_DEBUG("start equal to goal " + std::to_string(id));
                return;
            }

            while (!openList.empty()) openList.pop();
            closedList.clear();
            parentMap.clear();
            costMap.clear();


            if (!IsValid(start, chunk, id) || !IsValid(goal, chunk, id))
            {
                LOG_DEBUG("pathfinding failed due to incorrect goal or start " + std::to_string(id));
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

                if (currentCell.pos == goal)
                {
                    SetPath(start, goal, chunk, id);
                    return;
                }

                if (closedList.find(currentCell.pos) != closedList.end()) continue;

                closedList.insert(currentCell.pos);

                for (const auto& dir : directions)
                {
                    sl::Vec2i neighborPos = { currentCell.pos.x + dir.x, currentCell.pos.y + dir.y };

                    if (!IsValid(neighborPos, chunk, id)) continue;

                    bool isDiagonal = (dir.x != 0 && dir.y != 0);
                    
                    if (isDiagonal)
                    {
                        sl::Vec2i adj1 = { currentCell.pos.x + dir.x, currentCell.pos.y };
                        sl::Vec2i adj2 = { currentCell.pos.x, currentCell.pos.y + dir.y };
                        if (!IsValid(adj1, chunk, id) || !IsValid(adj2, chunk, id)) continue;
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
            LOG_DEBUG("path not found");
        });
}

bool Pathfinder::IsPathClear(sl::EntityId id, TilesetChunk& chunk, sl::Vec2i start, sl::Vec2i goal)
{
    int dx = std::abs(goal.x - start.x);
    int dy = std::abs(goal.y - start.y);

    int sx = (start.x < goal.x) ? 1 : -1;
    int sy = (start.y < goal.y) ? 1 : -1;

    int err = dx - dy;

    sl::Vec2i pos = start;

    while (true)
    {
        if (!IsValid(pos, chunk, id)) return false;

        if (pos == goal)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; pos.x += sx; }
        if (e2 < dx) { err += dx; pos.y += sy; }
    }

    return true;
}

int Pathfinder::GetH(sl::Vec2i pos, sl::Vec2i goal)
{
    int dx = pos.x - goal.x;
    int dy = pos.y - goal.y;

    return dx*dx + dy*dy;
}

bool Pathfinder::IsValid(sl::Vec2i pos, TilesetChunk& chunk, sl::EntityId id)
{
    if (pos.x < 0 || pos.x >= chunk.width || pos.y < 0 || pos.y >= chunk.height) return false;

    if (closedList.contains(pos)) return false;
    ColliderComponent& collider = se::Engine::GetECS().GetCurrentScene()->GetComponent<ColliderComponent>(id);
    
    int widthInTiles = int(std::ceil(collider.bounds.GetWidth() / float(chunk.tileSize)));
    int heightInTiles = int(std::ceil(collider.bounds.GetHeight() / float(chunk.tileSize)));

    int halfW = widthInTiles / 2;
    int halfH = heightInTiles / 2;

    for (int y = -halfH; y <= halfH; ++y)
    {
        for (int x = -halfW; x <= halfW; ++x)
        {
            int checkX = pos.x + x;
            int checkY = pos.y + y;

            if (checkX < 0 || checkX >= chunk.width || checkY < 0 || checkY >= chunk.height) return false;
            if (chunk.collisionGrid[checkY * chunk.width + checkX]) return false;
        }
    }

    return true;
}

void Pathfinder::SetPath(sl::Vec2i startPos, sl::Vec2i goalPos, TilesetChunk& chunk, sl::EntityId id)
{
    std::vector<sl::Vec2f> path;
    
    sl::Vec2i currentPos = goalPos;
    assert(parentMap.contains(goalPos));
    while (currentPos != startPos)
    {
        sl::Vec2f pointPos = chunk.GridToWorld(currentPos.x, currentPos.y);
        pointPos.x += chunk.tileSize / 2;
        pointPos.y += chunk.tileSize / 2;
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
    path.clear();

    size_t current = 0;
    size_t next = 1;

    while (next < rawPath.size())
    {
        while (next + 1 < rawPath.size() && IsPathClear(id, chunk, chunk.WorldToGrid(rawPath[current].x, rawPath[current].y), chunk.WorldToGrid(rawPath[next + 1].x, rawPath[next + 1].y)))
        {
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
            sl::RectF worldCollider = GetWorldCollider(&scene, id);
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
                movement.dir = sl::Vec2f(targetPos - GetWorldCollider(&scene, id).GetCenter()).GetNormalized();
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

bool IsPathClear(sl::EntityId id, sl::Vec2f goalWorld)
{
    sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
    TilesetChunk* curChunk = nullptr;
    sl::RectF worldCollider = GetWorldCollider(&scene, id);
    
    scene.ForEach<TilesetChunk>([&](sl::EntityId chunkId, TilesetChunk& chunk)
        {
            if (curChunk) return;
            if (worldCollider.IsContainedBy(sl::RectF(chunk.worldRect)) && chunk.worldRect.Contains(sl::Vec2i(goalWorld)))
            {
                curChunk = &chunk;
            }
        });

    if (!curChunk) return false;

    sl::Vec2i start = curChunk->WorldToGrid(worldCollider.GetCenter().x, worldCollider.GetCenter().y);
    sl::Vec2i goal = curChunk->WorldToGrid(goalWorld.x, goalWorld.y);

    int colliderTileWidth = std::max(1, int(std::ceil(worldCollider.GetWidth() / curChunk->tileSize)));
    int colliderTileHeight = std::max(1, int(std::ceil(worldCollider.GetHeight() / curChunk->tileSize)));

    int dx = std::abs(goal.x - start.x);
    int dy = -std::abs(goal.y - start.y);
    int sx = start.x < goal.x ? 1 : -1;
    int sy = start.y < goal.y ? 1 : -1;
    int err = dx + dy;

    sl::Vec2i tile = start;
    while (true)
    {
        for (int y = 0; y < colliderTileHeight; y++)
        {
            for (int x = 0; x < colliderTileWidth; x++)
            {
                int checkX = tile.x + x - colliderTileWidth / 2;
                int checkY = tile.y + y - colliderTileHeight / 2;

                if (checkX < 0 || checkY < 0 || checkX >= curChunk->width || checkY >= curChunk->height) return false;

                size_t index = checkY * curChunk->width + checkX;
                if (curChunk->collisionGrid[index]) return false;
            }
        }

        if (tile == goal) break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            tile.x += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            tile.y += sy;
        }
    }
    
    return true;
}
