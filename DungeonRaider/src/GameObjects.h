#pragma once
#include<ScypLib/ScypLib.h>
#include"Weapon.h"
#include"Level.h"

sl::EntityId CreatePlayer(sl::Vec2f pos, float width, float height, sl::Texture* texture);
sl::EntityId CreateEnemy(sl::Vec2f pos, float width, float height, sl::Texture* texture);
void DrawHealthBars();