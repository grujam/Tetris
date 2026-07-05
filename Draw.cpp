#include "stdafx.h"
#include "Draw.h"

namespace Draw
{
	void EraseDrawable(std::shared_ptr<IDrawable> InDrawable)
	{
		GET_RENDER()->RemoveDrawTargets(InDrawable);
	}
}

