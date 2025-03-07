#include "pch.h"
#include "MigBase.h"

using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// MigBase

void MigBase::create()
{
}

void MigBase::createGraphics()
{
}

void MigBase::windowSizeChanged()
{
}

void MigBase::visibilityChanged(bool vis)
{
}

void MigBase::suspend()
{
}

void MigBase::resume()
{
}

void MigBase::destroyGraphics()
{
}

void MigBase::destroy()
{
}

///////////////////////////////////////////////////////////////////////////
// LifeCycleCollection

void LifeCycleCollection::create()
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->create();
		iter++;
	}
}

void LifeCycleCollection::createGraphics()
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->createGraphics();
		iter++;
	}
}

void LifeCycleCollection::windowSizeChanged()
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->windowSizeChanged();
		iter++;
	}
}

void LifeCycleCollection::visibilityChanged(bool vis)
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->visibilityChanged(vis);
		iter++;
	}
}

void LifeCycleCollection::suspend()
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->suspend();
		iter++;
	}
}

void LifeCycleCollection::resume()
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->resume();
		iter++;
	}
}

void LifeCycleCollection::destroyGraphics()
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->destroyGraphics();
		iter++;
	}
}

void LifeCycleCollection::destroy()
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter != nullptr)
			(*iter)->destroy();
		iter++;
	}
}

bool LifeCycleCollection::addToList(MigBase& item)
{
	_lcList.push_back(&item);
	return true;
}

bool LifeCycleCollection::removeFromList(MigBase& item)
{
	std::vector<MigBase*>::iterator iter = _lcList.begin();
	while (iter != _lcList.end())
	{
		if (*iter == &item)
			break;
		iter++;
	}
	if (iter != _lcList.end())
	{
		_lcList.erase(iter);
		return true;
	}
	return false;
}

void LifeCycleCollection::removeAll()
{
	_lcList.clear();
}
