#include "Level.h"
#include<random>

#include<ScypEngine/Engine.h>

#include"Components.h"
#include"GameObjects.h"

TilesetChunk::TilesetChunk(sl::Vec2i pos, int width, int height, int tileSize, sl::Texture* texture)
	:
	width(width), height(height), tileSize(tileSize), atlas(texture), worldRect(pos, width * tileSize, height * tileSize)
{
    collisionGrid.resize(size_t(height * width), false);
    uvIndex.resize(size_t(height * width), TileSprite::temp);
    tileTypes.resize(size_t(height * width), TileType::floor);
    uvs.reserve(size_t(TileSprite::temp) + 1);

    int atlasWidth = texture->GetWidth();
    int atlasHeight = texture->GetHeight();

    int numSpritesX = atlasWidth / tileSize;
    int numSpritesY = atlasHeight / tileSize;

    for (int y = 0; y < numSpritesY; ++y)
    {
        for (int x = 0; x < numSpritesX; ++x)
        {
            sl::RectF uv(sl::Vec2f(float(x * tileSize), float(y * tileSize)), float(tileSize), float(tileSize));
            uvs.push_back(uv);
        }
    }
    std::reverse(uvs.begin(), uvs.end());

    sl::RectF& wallUV = uvs[size_t(TileSprite::fullWall)];
    wallUV = uvs[size_t(TileSprite::wallTop)];
    wallUV.top -= tileSize / 2;

    sl::RectF& shutGateUV = uvs[size_t(TileSprite::shutGate)];
    shutGateUV = uvs[size_t(TileSprite::openGate)];
    shutGateUV.top -= tileSize / 2;
}

TilesetChunk::TilesetChunk(TilesetChunk&& other) noexcept
    : collisionGrid(std::move(other.collisionGrid)), uvIndex(std::move(other.uvIndex)),
    uvs(std::move(other.uvs)),
    atlas(other.atlas), worldRect(other.worldRect),
    width(other.width), height(other.height), tileSize(other.tileSize),
    tileTypes(std::move(other.tileTypes))
{
    other.width = 0;
    other.height = 0;
    other.tileSize = 0;
    other.atlas = nullptr;
}

TilesetChunk::TilesetChunk(const TilesetChunk& other)
    : collisionGrid(other.collisionGrid),
    uvIndex(other.uvIndex), uvs(other.uvs),
    atlas(other.atlas),
    worldRect(other.worldRect), width(other.width),
    height(other.height), tileSize(other.tileSize),
    tileTypes(other.tileTypes)
{}

TilesetChunk& TilesetChunk::operator=(const TilesetChunk& other)
{
    if (this != &other) 
    {
        collisionGrid = other.collisionGrid;
        uvIndex = other.uvIndex;
        uvs = other.uvs;
        atlas = other.atlas;
        worldRect = other.worldRect;
        width = other.width;
        height = other.height;
        tileSize = other.tileSize;
        tileTypes = other.tileTypes;
    }
    return *this;
}
void TilesetChunk::SetTile(int x, int y, bool collides, TileSprite sprite, TileType type)
{
    assert(x >= 0 && y >= 0 && x < width && y < height);
    size_t index = size_t(y * width + x);
    collisionGrid[index] = collides;
    uvIndex[index] = sprite;
    tileTypes[index] = type;
}
sl::Vec2f TilesetChunk::GridToWorld(int x, int y) const
{
    float wx = float(worldRect.left + x * tileSize);
    float wy = float(worldRect.top + y * tileSize);
    return { wx, wy };
}
sl::Vec2i TilesetChunk::WorldToGrid(float x, float y) const
{
    int gx = int((x - worldRect.left) / tileSize);
    int gy = int((y - worldRect.top) / tileSize);
    if (gx < 0 || gx >= width || gy < 0 || gy >= height) 
    {
        LOG_WARN("coord conversion out of bounds");
        gx = std::clamp(gx, 0, width);
        gy = std::clamp(gx, 0, height);
    }
    return { gx, gy };
}
void SpawnRoom(TilesetChunk& chunk, RoomTrigger& trigger)
{
    int roomSize = int(std::min(chunk.width, chunk.height) * 0.75f);
    int corridorSize = int(roomSize * 0.4f);

    sl::Vec2i roomCenter = sl::Vec2i(chunk.width / 2, chunk.height / 2);
    sl::Vec2i roomStart = roomCenter - sl::Vec2i(roomSize / 2, roomSize / 2);
   
    int corridorX = chunk.width / 2 - corridorSize / 2;
    int corridorY = chunk.height / 2 - corridorSize / 2;

    sl::Vec2f chunkPos = sl::Vec2f(float(chunk.worldRect.left), float(chunk.worldRect.top));
    trigger.worldBounds = sl::RectF(chunk.GridToWorld(roomStart.x, roomStart.y), float(roomSize * chunk.tileSize), float(roomSize * chunk.tileSize));

    for (int y = corridorY; y < corridorY + corridorSize; y++)
    {
        for (int x = 0; x < chunk.width; x++)
        {
            if (y == corridorY || y == corridorY + corridorSize - 1)
            {
                chunk.SetTile(x, y, true, TileSprite::fullWall, TileType::fullWall);
            }
            else
            {
                chunk.SetTile(x, y, false, TileSprite::floor, TileType::floor);
            }
        }
    }

    for (int y = 0; y < chunk.height; y++)
    {
        for (int x = corridorX; x < corridorX + corridorSize; x++)
        {
            if (x == corridorX || x == corridorX + corridorSize - 1)
            {
                chunk.SetTile(x, y, true, TileSprite::wallTop, TileType::topWall);
            }
            else
            {
                chunk.SetTile(x, y, false, TileSprite::floor, TileType::floor);
            }
        }
    }

    for (int y = (chunk.height - roomSize) / 2; y < (chunk.height - roomSize) / 2 + roomSize; y++)
    {
        for (int x = (chunk.width - roomSize) / 2; x < (chunk.width - roomSize) / 2 + roomSize; x++)
        {
            if (y == (chunk.height - roomSize) / 2 || y == (chunk.height - roomSize) / 2 + roomSize - 1)
            {
                chunk.SetTile(x, y, true, TileSprite::fullWall, TileType::fullWall);
            }
            else if (x == (chunk.width - roomSize) / 2 || x == (chunk.width - roomSize) / 2 + roomSize - 1)
            {
                chunk.SetTile(x, y, true, TileSprite::wallTop, TileType::topWall);
            }
            else
            {
                chunk.SetTile(x, y, false, TileSprite::floor, TileType::floor);
            }
        }
    }

    for (int x = roomStart.x + roomSize / 2 - corridorSize / 2 + 1; x < roomStart.x + roomSize / 2 + corridorSize / 2 - 1; x++)
    {
        chunk.SetTile(x, roomStart.y, false, TileSprite::openGate, TileType::floor);
        chunk.SetTile(x, roomStart.y + roomSize - 1, false, TileSprite::openGate, TileType::floor);

        trigger.gateIndexes.push_back((roomStart.y + roomSize - 1) * chunk.width + x);
        trigger.gateIndexes.push_back(roomStart.y * chunk.width + x);
    }

    chunk.SetTile(roomStart.x, roomStart.y + roomSize / 2 - corridorSize / 2, true, TileSprite::fullWall, TileType::fullWall);
    chunk.SetTile(roomStart.x + roomSize - 1, roomStart.y + roomSize / 2 - corridorSize / 2, true, TileSprite::fullWall, TileType::fullWall);
    for (int y = roomStart.y + roomSize / 2 - corridorSize / 2 + 1; y < roomStart.y + roomSize / 2 + corridorSize / 2 - 1; y++)
    {
        chunk.SetTile(roomStart.x, y, false, TileSprite::openGate, TileType::floor);
        chunk.SetTile(roomStart.x + roomSize - 1, y, false, TileSprite::openGate,TileType::floor);

        trigger.gateIndexes.push_back(y * chunk.width + roomStart.x);
        trigger.gateIndexes.push_back(y * chunk.width + roomStart.x + roomSize - 1);
    }
}

void CreateLevel()
{
    auto& ecs = se::Engine::GetECS();
    sl::Scene& scene = *ecs.GetCurrentScene();

    sl::Texture* atlas = se::Engine::GetGraphics().LoadTexture("assets/images/tile_set8.png");

    int width = 30;
    int height = 30;
    int tileSize = 32;

    sl::Vec2i positions[5] = 
    {
        {0, -height * tileSize},        // bottom
        {width * tileSize, 0},          // right
        {0, height * tileSize},         // top
        {-width * tileSize, 0},         // left
        {0, 0}                          // center
    };

    for (int i = 0; i < 5; i++)
    {
        sl::EntityId chunkId = CreateTileChunk(positions[i], width, height, tileSize, atlas);

        if (i == 4)
        {
            GameGlobals::currentRoom = chunkId;
            scene.GetComponent<RoomTrigger>(chunkId).state = RoomTrigger::State::Explored;
            scene.GetComponent<RoomEncounter>(chunkId) = RoomEncounter{};
        }
    }
}

void DrawLevel()
{
    sl::Scene& scene = *se::Engine::GetECS().GetCurrentScene();
    sl::Graphics& gfx = se::Engine::GetGraphics();
    Camera& cam = scene.GetComponent<Camera>(GameGlobals::camera);

    scene.ForEach<TilesetChunk, TransformComponent>([&](sl::EntityId id, TilesetChunk& chunk, TransformComponent& transform)
        {
            float worldLeft = cam.pos.x;
            float worldRight = cam.pos.x + gfx.GetCanvasWidth() / cam.zoom;
            float worldTop = cam.pos.y;
            float worldBottom = cam.pos.y + gfx.GetCanvasHeight() / cam.zoom;

            int left = std::max(0, int((worldLeft - chunk.worldRect.left) / chunk.tileSize));
            int right = std::min(chunk.width - 1, int(std::ceil((worldRight - chunk.worldRect.left) / chunk.tileSize)));
            int bottom = std::max(0, int((worldTop - chunk.worldRect.top) / chunk.tileSize));
            int top = std::min(chunk.height - 1, int(std::ceil((worldBottom - chunk.worldRect.top) / chunk.tileSize)));

            left = 0;                   //TEMPORARY SO YOU CAN USE ZOOM ON CAMERA (IM TO LAZY TO IMPLEMENT IT PROPERLY ) TO BE REMOVED IN LATER VERSIONS
            right = chunk.width - 1;    //TEMPORARY SO YOU CAN USE ZOOM ON CAMERA (IM TO LAZY TO IMPLEMENT IT PROPERLY ) TO BE REMOVED IN LATER VERSIONS
            top = chunk.height - 1;                    //TEMPORARY SO YOU CAN USE ZOOM ON CAMERA (IM TO LAZY TO IMPLEMENT IT PROPERLY ) TO BE REMOVED IN LATER VERSIONS
            bottom = 0;  //TEMPORARY SO YOU CAN USE ZOOM ON CAMERA (IM TO LAZY TO IMPLEMENT IT PROPERLY ) TO BE REMOVED IN LATER VERSIONS

            for (int y = top; y >= bottom; y--)
            {
                for (int x = left; x <= right; x++)
                {
                    size_t index = y * chunk.width + x;
                    sl::Vec2f tilePos = transform.pos + sl::Vec2f(float(x), float(y)) * float(chunk.tileSize);
                    TileSprite type = chunk.uvIndex[index];
                    sl::Vec2f size = { float(chunk.tileSize), float(chunk.tileSize) };
                    gfx.SetDrawLayer(0.0f);
                    if (type != TileSprite::temp)
                    {
                        sl::Vec2f offset = { 0.0f, 0.0f };
                        if (chunk.tileTypes[index] == TileType::fullWall)
                        {
                            gfx.SetDrawLayer(5.0f);
                            offset.y -= chunk.tileSize / 2;
                            size.y *= 1.5f;
                        }
                        else if (chunk.tileTypes[index] == TileType::topWall)
                        {
                            gfx.SetDrawLayer(5.0f);
                            offset.y -= chunk.tileSize / 2;
                        }
                        const sl::RectF* uv = &chunk.uvs[size_t(size_t(type))];
                        gfx.DrawTexture(tilePos + offset, size, chunk.atlas, nullptr, false, false, 0.0f, sl::Vec2f(0, 0), uv);
                    }
                }
            }
        });
}


void SpawnObstacles(TilesetChunk& chunk, RoomTrigger& trigger)
{
    sl::Vec2i roomPos = sl::Vec2i(int(trigger.worldBounds.left - chunk.worldRect.left), int(trigger.worldBounds.top - chunk.worldRect.top)) / chunk.tileSize;
    int roomWidth = int(trigger.worldBounds.GetWidth() / chunk.tileSize);
    int roomHeight = int(trigger.worldBounds.GetHeight() / chunk.tileSize);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dirDist(0, 1); // 1 = vertical; 0 = horizontal
    std::uniform_int_distribution<int> hLengthDist(static_cast<int>( roomWidth * 0.3f), static_cast<int>(roomWidth * 0.5f));  // Horizontal wall length
    std::uniform_int_distribution<int> vLengthDist(static_cast<int>(roomHeight * 0.3f), static_cast<int>(roomHeight * 0.5f)); // Vertical wall length
    std::uniform_int_distribution<int> xDist(static_cast<int>(roomPos.x + 3), static_cast<int>(roomPos.x + roomWidth * 0.60f)); // X start position
    std::uniform_int_distribution<int> yDist(static_cast<int>(roomPos.y + 3), static_cast<int>(roomPos.y + roomHeight * 0.5f)); // Y start position
    std::uniform_int_distribution<int> chance(0, 100);

    std::vector<int> HwallPositions;
    std::vector<int> VwallPositions;

    for (int i = 0; i < roomWidth * roomHeight * 0.0035f; i++)
    {
        int xStart = xDist(rng);
        int yStart = yDist(rng);
        assert(xStart < chunk.width && yStart < chunk.height && xStart >= 0 && yStart >= 0);

        bool isHorizontal = (dirDist(rng) == 1);
        bool valid = true;
        int tries = 0;
        const int maxTries = 2000;
        const int distance = 3;

        auto checkPosition = [&](int& position, std::vector<int>& wallPositions, bool isY)
            {
                do
                {
                    valid = true;
                    position = (isY ? yDist(rng) : xDist(rng));
                    for (const auto& wallPos : wallPositions)
                    {
                        if (std::abs(wallPos - position) < distance)
                        {
                            valid = false;
                            break;
                        }
                    }
                    tries++;
                } while (!valid && tries < maxTries);
            };

        if (isHorizontal)
        {
            checkPosition(yStart, HwallPositions, true);
        }
        else
        {
            checkPosition(xStart, VwallPositions, false);
        }

        // Calculate wall length and limits based on direction
        int wallLength = isHorizontal ? hLengthDist(rng) : vLengthDist(rng);
        int xEnd = std::min(roomPos.x + roomWidth - 4, xStart + wallLength);
        int yEnd = std::min(roomPos.y + roomHeight - 4, yStart + wallLength);

        if (isHorizontal)
        {
            // Place the horizontal wall
            for (int x = xStart; x < xEnd - 1; x++)
            {
                assert(x < chunk.width);
                if (!chunk.collisionGrid[size_t(yStart * chunk.width + x)])
                {
                    chunk.collisionGrid[size_t(yStart * chunk.width + x)] = true;
                    chunk.uvIndex[size_t(yStart * chunk.width + x)] = TileSprite::wallTop;
                }
            }
            HwallPositions.push_back(yStart);
        }
        else
        {
            for (int y = yStart; y < yEnd; y++)
            {
                assert(y < chunk.width);
                chunk.collisionGrid[size_t(y * chunk.width + xStart)] = 1;
                chunk.uvIndex[size_t(y * chunk.width + xStart)] = TileSprite::wallTop;
            }

            if (!chunk.collisionGrid[size_t(yEnd * chunk.width + xStart)]) //if there is a wall above it makes sure that it is connected in T shape
            {
                chunk.collisionGrid[size_t(yEnd * chunk.width + xStart)] = 1;
                chunk.uvIndex[size_t(yEnd * chunk.width + xStart)] = TileSprite::wallTop;
            }
            else
            {
                chunk.collisionGrid[size_t(yEnd * chunk.width + xStart)] = 1;
                chunk.uvIndex[size_t(yEnd * chunk.width + xStart)] = TileSprite::wallTop;
            }
            VwallPositions.push_back(xStart);
        }
    }

}

RoomTrigger::RoomTrigger(RoomTrigger&& other) noexcept
    : gateIndexes(std::move(other.gateIndexes)), state(other.state),
    worldBounds(other.worldBounds)
{
}

RoomTrigger& RoomTrigger::operator=(const RoomTrigger& other)
{
    state = other.state;
    worldBounds = other.worldBounds;
    gateIndexes = other.gateIndexes;
    return *this;
}

RoomTrigger::RoomTrigger(const RoomTrigger& other)
{
    state = other.state;
    worldBounds = other.worldBounds;
    gateIndexes = other.gateIndexes;
}

void LevelSystem::Run(float dt, sl::Scene& scene)
{
    sl::RectF playerWorldRect;
    TransformComponent& transform = scene.GetComponent<TransformComponent>(GameGlobals::player);
    ColliderComponent& collider = scene.GetComponent<ColliderComponent>(GameGlobals::player);
    
    playerWorldRect.left = transform.pos.x + collider.bounds.left;
    playerWorldRect.top = transform.pos.y + collider.bounds.top;
    playerWorldRect.right = playerWorldRect.left + collider.bounds.GetWidth();
    playerWorldRect.bottom = playerWorldRect.top + collider.bounds.GetHeight();

    bool newRoomFound = false;

    scene.ForEach<TilesetChunk, RoomTrigger, TransformComponent>([&](sl::EntityId triggerId, TilesetChunk& chunk, RoomTrigger& trigger, TransformComponent& transform)
        {
            if (newRoomFound) return;

            if (triggerId != GameGlobals::currentRoom && playerWorldRect.IsContainedBy(trigger.worldBounds))
            {
                sl::EntityId prevRoom = GameGlobals::currentRoom;
                sl::EntityId curRoom = triggerId;
                TransformComponent& prevTransform = scene.GetComponent<TransformComponent>(prevRoom);
                scene.DestroyEntity(prevRoom);

                if (RoomTrigger::State::Unexplored == trigger.state)
                {
                    for (size_t index : trigger.gateIndexes)
                    {
                        chunk.collisionGrid[index] = true;
                        chunk.tileTypes[index] = TileType::fullWall;
                        chunk.uvIndex[index] = TileSprite::shutGate;
                    }
                    trigger.state = RoomTrigger::State::Exploring;
                }

                sl::Vec2f delta = transform.pos - prevTransform.pos;
                if (transform.pos.y == prevTransform.pos.y)
                {
                    scene.ForEach<TilesetChunk, TransformComponent, RoomTrigger>([&](sl::EntityId movedId, TilesetChunk& moved, TransformComponent& movedTransform, RoomTrigger& movedTrig)
                        {
                            if (movedId == curRoom) return;
                            if (transform.pos.y == movedTransform.pos.y)
                            {
                                scene.DestroyEntity(movedId);
                            }
                            else
                            {
                                movedTransform.pos += delta;
                                moved.worldRect = sl::RectI(sl::Vec2i(movedTransform.pos), moved.width * moved.tileSize, moved.height * moved.tileSize);
                                movedTrig.worldBounds.Translate(delta);
                                
                            }
                        });
                    CreateTileChunk(sl::Vec2i(transform.pos - delta), chunk.width, chunk.height, chunk.tileSize, chunk.atlas);
                    CreateTileChunk(sl::Vec2i(transform.pos + delta), chunk.width, chunk.height, chunk.tileSize, chunk.atlas);
                }
                else if (transform.pos.x == prevTransform.pos.x)
                {
                    scene.ForEach<TilesetChunk, TransformComponent, RoomTrigger>([&](sl::EntityId movedId, TilesetChunk& moved, TransformComponent& movedTransform, RoomTrigger& movedTrig)
                        {
                            if (movedId == curRoom) return;
                            if (transform.pos.x == movedTransform.pos.x)
                            {
                                scene.DestroyEntity(movedId);
                            }
                            else
                            {
                                movedTransform.pos += delta;
                                moved.worldRect = sl::RectI(sl::Vec2i(movedTransform.pos), moved.width * moved.tileSize, moved.height * moved.tileSize);
                                movedTrig.worldBounds.Translate(delta);
                            }
                        });
                    CreateTileChunk(sl::Vec2i(transform.pos - delta), chunk.width, chunk.height, chunk.tileSize, chunk.atlas);
                    CreateTileChunk(sl::Vec2i(transform.pos + delta), chunk.width, chunk.height, chunk.tileSize, chunk.atlas);
                }

                GameGlobals::currentRoom = curRoom;
                newRoomFound = true;
            }
        });

    sl::RectF& roomBounds = scene.GetComponent<RoomTrigger>(GameGlobals::currentRoom).worldBounds;
    bool roomCleared = true;
    scene.ForEach<TagComponent, TransformComponent, ColliderComponent>([&](sl::EntityId, TagComponent& tag, TransformComponent& transform, ColliderComponent& collider)
        {
            if (!roomCleared || (tag.tag & uint32_t(Tags::enemy)) == 0) return;
            sl::RectF enemyWorldRect;
            enemyWorldRect.left = transform.pos.x + collider.bounds.left;
            enemyWorldRect.top = transform.pos.y + collider.bounds.top;
            enemyWorldRect.right = enemyWorldRect.left + collider.bounds.GetWidth();
            enemyWorldRect.bottom = enemyWorldRect.top + collider.bounds.GetHeight();
            if (enemyWorldRect.IsContainedBy(roomBounds)) roomCleared = false;
        });

    if (roomCleared)
    {
        RoomTrigger& trigger = scene.GetComponent<RoomTrigger>(GameGlobals::currentRoom);
        if (trigger.state == RoomTrigger::State::Exploring)
        {
            TilesetChunk& chunk = scene.GetComponent<TilesetChunk>(GameGlobals::currentRoom);
            RoomEncounter& encounter = scene.GetComponent<RoomEncounter>(GameGlobals::currentRoom);
            
            if (encounter.wavesLeft > 0 && encounter.deleyLeft <= 0.0f)
            {
                CreateEnemiesInRoom(chunk, trigger, encounter.leastEnemies, encounter.mostEnemies);
                encounter.deleyLeft = encounter.deley;
                encounter.wavesLeft--;
            }
            else if (encounter.wavesLeft > 0 && encounter.deleyLeft > 0.0f)
            {
                encounter.deleyLeft -= dt;
            }
            else
            {
                for (size_t index : trigger.gateIndexes)
                {
                    chunk.collisionGrid[index] = false;
                    chunk.tileTypes[index] = TileType::floor;
                    chunk.uvIndex[index] = TileSprite::openGate;
                }
                trigger.state = RoomTrigger::State::Explored;
            }
        }
    }
}

void TileCollisionSystem::Run(float dt, sl::Scene& scene)
{
    scene.ForEach<TransformComponent, ColliderComponent, MovementComponent>(
        [&](sl::EntityId id, TransformComponent& transform, ColliderComponent& collider, MovementComponent& movement)
        {
            if (movement.dir.x == 0 && movement.dir.y == 0) return;
            sl::RectF entityWorldRect;
            entityWorldRect.left = movement.proposedPos.x + collider.bounds.left;
            entityWorldRect.top = movement.proposedPos.y + collider.bounds.top;
            entityWorldRect.right = entityWorldRect.left + collider.bounds.GetWidth();
            entityWorldRect.bottom = entityWorldRect.top + collider.bounds.GetHeight();

            if (collider.layer == ColliderComponent::CollisionLayer::Everything || collider.layer == ColliderComponent::CollisionLayer::World)
            {
                scene.ForEach<TilesetChunk>([&](sl::EntityId tilesetId, TilesetChunk& chunk)
                    {
                        if (!chunk.worldRect.IsOverlappingWith(sl::RectI(entityWorldRect))) return;

                        auto blocks([&](int x, int y) -> bool
                            {
                                if (x < 0 || y < 0 || x >= chunk.width || y >= chunk.height) return true;
                                return chunk.collisionGrid[y * chunk.width + x];
                            });


                        sl::Vec2i gridTL = chunk.WorldToGrid(entityWorldRect.left, entityWorldRect.top);
                        sl::Vec2i gridBR = chunk.WorldToGrid(entityWorldRect.right, entityWorldRect.bottom);

                        if (gridTL.x < 0 || gridTL.x >= chunk.width || gridTL.y < 0 || gridTL.y >= chunk.height) return;
                        if (gridBR.x < 0 || gridBR.x >= chunk.width || gridBR.y < 0 || gridBR.y >= chunk.height) return;

                        for (int y = gridTL.y; y <= gridBR.y; y++)
                        {
                            for (int x = gridTL.x; x <= gridBR.x; x++)
                            {
                                if (!blocks(x, y)) continue;

                                sl::RectF tileRect;
                                sl::Vec2f tilePos = chunk.GridToWorld(x, y);
                                tileRect.left = tilePos.x;
                                tileRect.top = tilePos.y;
                                tileRect.right = tilePos.x + chunk.tileSize;
                                tileRect.bottom = tilePos.y + chunk.tileSize;

                                if (!entityWorldRect.IsOverlappingWith(tileRect)) continue;

                                float dxLeft = tileRect.right - entityWorldRect.left;
                                float dxRight = tileRect.left - entityWorldRect.right;
                                float dyTop = tileRect.bottom - entityWorldRect.top;
                                float dyBottom = tileRect.top - entityWorldRect.bottom;

                                float mtvX = (std::abs(dxLeft) < std::abs(dxRight)) ? dxLeft : dxRight;
                                float mtvY = (std::abs(dyTop) < std::abs(dyBottom)) ? dyTop : dyBottom;

                                if (std::abs(mtvX) < std::abs(mtvY)) movement.proposedPos.x += mtvX;
                                else movement.proposedPos.y += mtvY;

                                entityWorldRect.left = movement.proposedPos.x + collider.bounds.left;
                                entityWorldRect.top = movement.proposedPos.y + collider.bounds.top;
                                entityWorldRect.right = entityWorldRect.left + collider.bounds.GetWidth();
                                entityWorldRect.bottom = entityWorldRect.top + collider.bounds.GetHeight();
                            }
                        }
                    });
            }
            transform.pos = movement.proposedPos;
        });

}

sl::EntityId CreateTileChunk(sl::Vec2i pos, int width, int height, int tileSize, sl::Texture* texture)
{
    sl::EntityComponentSystem& ecs = se::Engine::GetECS();
    sl::Scene* scene = ecs.GetCurrentScene();
    sl::EntityId chunkId = scene->CreateEntity();

    TilesetChunk chunk(pos, width, height, tileSize, texture);
    RoomTrigger trigger;

    SpawnRoom(chunk, trigger);
    SpawnObstacles(chunk, trigger);

    scene->AddComponent<TilesetChunk>(chunkId, std::move(chunk));
    scene->AddComponent<RoomTrigger>(chunkId, std::move(trigger));
    scene->AddComponent<TransformComponent>(chunkId, TransformComponent{ sl::Vec2f(pos), 0.0f });
    scene->AddComponent<RoomEncounter>(chunkId, RoomEncounter{ 1,2,5,1.0f,1.0f });
    return chunkId;
}

void CreateEnemiesInRoom(const TilesetChunk& chunk, const RoomTrigger& trigger, int least, int most)
{
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> countDist(least, most);

    constexpr float biggestEnemySize = 40.0f;
    constexpr float smallestDistance = 2.0f * biggestEnemySize;

    std::uniform_real_distribution<float> xDist(trigger.worldBounds.left + 2.0f * biggestEnemySize, trigger.worldBounds.right - 2.0f * biggestEnemySize);
    std::uniform_real_distribution<float> yDist(trigger.worldBounds.top + 2.0f * biggestEnemySize, trigger.worldBounds.bottom - 2.0f * biggestEnemySize);

    int count = countDist(rng);

    int enemyWidthInTiles = 3;
    int enemyHeightInTiles = 3;

    auto blocks([&](int x, int y) -> bool
        {
            if (x < 0 || y < 0 || x >= chunk.width || y >= chunk.height) return true;
            return chunk.collisionGrid[y * chunk.width + x];
        });
    std::vector<sl::Vec2f> usedPos;
    usedPos.reserve(count);

    for (int i = 0; i < count; i++)
    {
        sl::Vec2f spawnPos{};
        bool valid = true;

        for (int attempts = 0; attempts < 100; attempts++)
        {
            valid = true;
            spawnPos = { xDist(rng), yDist(rng) };

            for (auto& pos : usedPos)
            {
                if ((pos - spawnPos).GetLength() < smallestDistance) valid = false;
            }

            sl::Vec2i gridTL = chunk.WorldToGrid(spawnPos.x, spawnPos.y);
            sl::Vec2i gridBR = chunk.WorldToGrid(spawnPos.x + biggestEnemySize, spawnPos.y + biggestEnemySize);

            if (gridTL.x < 0 || gridTL.y < 0 || gridBR.x >= chunk.width || gridBR.y >= chunk.height)
            {
                valid = false;
                continue;
            }

            for (int y = gridTL.y; y <= gridBR.y && valid; y++)
            {
                for (int x = gridTL.x; x <= gridBR.x; x++)
                {
                    if (blocks(x, y)) valid = false;
                }
            }
            if (valid) break;
        }
        if (valid)
        {
            usedPos.push_back(spawnPos);
            CreateEnemy(spawnPos, 40.0f, 40.0f, nullptr);
        }
    }
}