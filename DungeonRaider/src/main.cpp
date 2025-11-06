#include<ScypEngine/Engine.h>
#include"game.h"

int main()
{
	se::Engine::Init(1280, 720, "Dungeon Rider");
	se::Engine::Run<DungeonRaider>();
	se::Engine::Shutdown();
}