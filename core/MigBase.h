#pragma once

namespace MigTech
{
	// any class that needs lifecycle events should derive from this class
	class MigBase
	{
	public:
		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();
		virtual void visibilityChanged(bool vis);
		virtual void suspend();
		virtual void resume();
		virtual void destroyGraphics();
		virtual void destroy();
	};

	// holds a list of classes for lifecycle events
	class LifeCycleCollection
	{
	public:
		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();
		virtual void visibilityChanged(bool vis);
		virtual void suspend();
		virtual void resume();
		virtual void destroyGraphics();
		virtual void destroy();

	public:
		bool addToList(MigBase& item);
		bool removeFromList(MigBase& item);
		void removeAll();

	protected:
		std::vector<MigBase*> _lcList;
	};
}