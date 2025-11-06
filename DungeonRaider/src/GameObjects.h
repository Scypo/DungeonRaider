#pragma once
#include<ScypLib/ScypLib.h>
#include"Weapon.h"
#include"Level.h"

sl::EntityId CreatePlayer(sl::Vec2f pos, float width, float height, sl::Texture* texture);
void CreateCamera();
sl::EntityId CreateEnemy(sl::Vec2f pos, float width, float height, sl::Texture* texture);
sl::EntityId CreateProjectile(sl::Vec2f pos, sl::Vec2f dir, const ProjectileData* data, sl::EntityId creator);
sl::EntityId CreateTileChunk(sl::Vec2i pos, int width, int height, int tileSize, sl::Texture* texture);
void CreateEnemiesInRoom(const TilesetChunk& chunk, const RoomTrigger& trigger, int least, int most);
void DrawGameObjects();