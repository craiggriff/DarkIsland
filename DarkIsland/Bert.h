#pragma once

#include "AnimChar.h"
#include "level.h"

namespace Game
{
	class Bert : public AnimChar
	{
	public:
		Bert(AllResources* p_Resources, Level* pp_Level);

		void Initialize(int char_num);
	};
};
