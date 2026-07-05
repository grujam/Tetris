#pragma once

#include <memory>
#include <type_traits>
#include "Renderer.h"

class IDrawable abstract
{
public:
	IDrawable() {};
	virtual ~IDrawable() {}

public:
	virtual void OnDraw() = 0;
};

namespace Draw
{
	template<typename T>
	concept DrawableClass = std::is_base_of_v<IDrawable, T>;

	template<DrawableClass T, typename... Args>
	std::shared_ptr<T> CreateDrawable(Args&&... args)
	{
		auto drawobj = std::make_shared<T>(std::forward<Args>(args)...);
		GET_RENDER()->AddDrawTargets(drawobj);
		return drawobj;
	};

	void EraseDrawable(std::shared_ptr<IDrawable> InDrawable);
	
};