#pragma once
#include <rgde/engine.h>
#include "glass.h"

//����
class PlayLevelObject : public game::ILevelObject, public event::CSender
{
public:
	PlayLevelObject();
	~PlayLevelObject();

private:
	game::Glass m_glass;
};