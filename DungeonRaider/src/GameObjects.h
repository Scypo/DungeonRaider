#pragma once
#include<ScypLib/ScypLib.h>
#include"Weapon.h"
#include"Level.h"

sl::EntityId CreatePlayer(sl::Scene* scene, sl::Vec2f pos, float width, float height, sl::Texture* texture);
sl::EntityId CreateEnemy(sl::Scene* scene, sl::Vec2f pos, float width, float height, float health, float damage, sl::Texture* texture);
void DrawHealthBars(sl::Scene* scene);